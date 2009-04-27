/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Torbjörn Jansson.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//  This file is subject to the terms of the GNU General Public License as
//  published by the Free Software Foundation.  A copy of this license is
//  included with this software distribution in the file COPYING.  If you
//  do not have a copy, you may obtain a copy by writing to the Free
//  Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////

/**
 * @file DShowFileSource.cpp implementation of the CDShowFileSource class.
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT

#include "dscaler.h"
#include "DShowFileSource.h"
#include "exception.h"
#include "PinEnum.h"
#include "..\..\..\DSRend\DSRend.h"
#include "debuglog.h"

using namespace std;

CDShowFileSource::CDShowFileSource(IGraphBuilder *pGraph,string filename)
:CDShowBaseSource(pGraph),m_file(filename),m_bIsConnected(FALSE)
{
    USES_CONVERSION;
    HRESULT hr=m_pBuilder.CoCreateInstance(CLSID_CaptureGraphBuilder2);
    if(FAILED(hr))
    {
        throw CDShowException("Failed to create capture graph builder",hr);
    }
    hr=m_pBuilder->SetFiltergraph(m_pGraph);
    if(FAILED(hr))
    {
        throw CDShowException("SetFiltergraph failed on capture graph builder",hr);
    }

    string tmp = m_file.substr((m_file.size()<4 ? 0 : m_file.size()-4));
    if(!AreEqualInsensitive(tmp, ".grf"))
    {
        hr=m_pGraph->AddSourceFilter(A2W(filename.c_str()),NULL,&m_pFileSource);
        if(FAILED(hr))
        {
            throw CDShowException("Failed to add file",hr);
        }
    }
}

CDShowFileSource::~CDShowFileSource()
{

}

void CDShowFileSource::Connect(CComPtr<IBaseFilter> VideoFilter)
{
    USES_CONVERSION;
    HRESULT hr;

    //is this a grf file? grf files needs special handling
    string tmp = m_file.substr((m_file.size()<4 ? 0 : m_file.size()-4)).c_str();
    if(!AreEqualInsensitive(tmp, ".grf"))
    {
        //the simple case, RenderStream is able to properly connect the filters
        hr=m_pBuilder->RenderStream(NULL,NULL,m_pFileSource,NULL,VideoFilter);
        if(FAILED(hr))
        {
            //that didnt work, try to manualy connect the pins on the source filter
            CDShowPinEnum outPins(m_pFileSource,PINDIR_OUTPUT);
            CComPtr<IPin> outPin;
            BOOL bSucceeded=FALSE;
            while(outPin=outPins.next(),bSucceeded==FALSE && outPin!=NULL)
            {
                CDShowPinEnum inPins(VideoFilter,PINDIR_INPUT);
                CComPtr<IPin> inPin;
                while(inPin=inPins.next(),inPin!=NULL)
                {
                    hr=m_pGraph->Connect(outPin,inPin);
                    if(SUCCEEDED(hr))
                    {
                        //now the video is connected properly

                        //render the other pins if any, it is not a big problem if it fails,
                        //just continue with the stream we already got
                        outPins.reset();
                        CComPtr<IPin> pin;
                        while(pin=outPins.next(),pin!=NULL)
                        {
                            CComPtr<IPin> dest;
                            hr=pin->ConnectedTo(&dest);
                            if(FAILED(hr))
                            {
                                m_pGraph->Render(pin);
                            }
                        }

                        bSucceeded=TRUE;
                        break;
                    }
                }
            }
            if(!bSucceeded)
            {
                throw CDShowUnsupportedFileException("Can't connect filesource to renderer",hr);
            }
        }

        //try to render audio, if this fails then this file probably don't have any audio
        BOOL bAudioRendered=FALSE;
        int AudioStreamCount=0;
        /*
        Connect all audio streams, not sure if this is a good idea, but it
        looks like IGraphBuilder::RenderFile also tries to connect all audio
        streams.
        This will always add one extra unconnected audio renderer when using
        a user specified audio renderer
        */
        while(hr=m_pBuilder->RenderStream(NULL,&MEDIATYPE_Audio,m_pFileSource,NULL,GetNewAudioRenderer()),SUCCEEDED(hr))
        {
            bAudioRendered=TRUE;
            AudioStreamCount++;
        }
        if(bAudioRendered)
        {
            LOG(2,"DShowFileSource: %d Audio streams rendered",AudioStreamCount);
        }
        else
        {
            LOG(2,"DShowFileSource: Unsupported audio or no audio found, error code: 0x%x",hr);
        }
    }
    else
    {
        hr=m_pGraph->RenderFile(A2W(m_file.c_str()),NULL);
        if(FAILED(hr))
        {
            throw CDShowException("Faild to render grapedit .grf file",hr);
        }

        CDShowGenericEnum<IEnumFilters,IBaseFilter> filterEnum;
        HRESULT hr=m_pGraph->EnumFilters(&filterEnum.m_pEnum);
        if(FAILED(hr))
        {
            throw CDShowException("Failed to get filter enumerator!!!",hr);
        }

        BOOL bFound=FALSE;
        CComPtr<IDSRendFilter> pDSRend;
        CComPtr<IBaseFilter> pFilter;
        while(hr=filterEnum.next(&pFilter),hr==S_OK && pFilter!=NULL)
        {
            if(pFilter.IsEqualObject(VideoFilter))
            {
                pFilter.Release();
                continue;
            }

            hr=pFilter.QueryInterface(&pDSRend);
            if(SUCCEEDED(hr))
            {
                bFound=TRUE;
                //replace the dsrend filter in the grf file with our renderer.
                //this might need to be changed to allow settings on the
                //dsrend filter from the grf file to be preserved
                //(not implemented yet on the filter)

                CDShowPinEnum InPins(pFilter,PINDIR_INPUT);
                CComPtr<IPin> pInPin=InPins.next();
                if(pInPin==NULL)
                {
                    throw CDShowException("DSRend filter do not have an input pin!!! (bug)");
                }
                CComPtr<IPin> pOutPin;
                hr=pInPin->ConnectedTo(&pOutPin);
                if(FAILED(hr))
                {
                    if(hr==VFW_E_NOT_CONNECTED)
                    {
                        throw CDShowException("The dsrend filter is not connected in the grf file",hr);
                    }
                    else
                    {
                        throw CDShowException("Failed to find filter that is connected to dscaler renderer",hr);
                    }
                }

                //preserve the mediatype on the connection
                AM_MEDIA_TYPE mt;
                memset(&mt,0,sizeof(AM_MEDIA_TYPE));
                hr=pOutPin->ConnectionMediaType(&mt);
                _ASSERTE(SUCCEEDED(hr));

                //preserve dsrend filter settings
                CComPtr<IPersistStream> pPStrmOld;
                hr=pFilter.QueryInterface(&pPStrmOld);
                if(FAILED(hr))
                {
                    throw CDShowException("Coud not find IPersistStream on old dsrend (bug)",hr);
                }
                CComPtr<IPersistStream> pPStrmNew;
                hr=VideoFilter.QueryInterface(&pPStrmNew);
                if(FAILED(hr))
                {
                    throw CDShowException("Coud not find IPersistStream on new dsrend (bug)",hr);
                }

                ULARGE_INTEGER ulSize;
                hr=pPStrmOld->GetSizeMax(&ulSize);
                if(FAILED(hr))
                {
                    throw CDShowException("IPersistStream::GetSizeMax failed (bug)",hr);
                }

                //make sure that the new and old renderer filters is of the same type
                GUID OldGUID;
                GUID NewGUID;
                if(FAILED(pPStrmOld->GetClassID(&OldGUID)) || FAILED(pPStrmNew->GetClassID(&NewGUID)))
                {
                    LOG(2,"DShowFileSource: Failed to get ClassID of new or old renderer filter");
                }
                else
                {
                    if(IsEqualGUID(OldGUID,NewGUID))
                    {
                        CComPtr<IStream> pStream;
                        HGLOBAL hg=GlobalAlloc(GMEM_MOVEABLE,(SIZE_T)ulSize.QuadPart);
                        if(hg!=NULL)
                        {
                            if(CreateStreamOnHGlobal(hg,TRUE,&pStream)==S_OK)
                            {
                                hr=pPStrmOld->Save(pStream,FALSE);
                                if(SUCCEEDED(hr))
                                {
                                    LARGE_INTEGER pos;
                                    pos.QuadPart=0;
                                    hr=pStream->Seek(pos,STREAM_SEEK_SET,NULL);
                                    hr=pPStrmNew->Load(pStream);
                                }
                            }
                        }
                    }
                    else
                    {
                        LOG(2,"DShowFileSource: Old and new renderer filter is not of the same type, will not copy setting");
                    }
                }


                hr=m_pGraph->RemoveFilter(pFilter);

                //connect to the right dsrend filter
                CDShowPinEnum InPins2(VideoFilter,PINDIR_INPUT);
                CComPtr<IPin> pInPin2=InPins2.next();
                hr=pOutPin->Connect(pInPin2,&mt);
                if(mt.cbFormat>0 && mt.pbFormat!=NULL)
                {
                    CoTaskMemFree(mt.pbFormat);
                    mt.cbFormat=0;
                    mt.pbFormat=NULL;
                }
                if(mt.pUnk!=NULL)
                {
                    mt.pUnk->Release();
                    mt.pUnk=NULL;
                }
                if(FAILED(hr))
                {
                    throw CDShowException("Failed to connect dsrend filter",hr);
                }
                break;
            }
            pFilter.Release();
        }
        if(!bFound)
        {
            throw CDShowException("This filter graph file does not contain a dscaler renderer filter");
        }
    }

    m_bIsConnected=TRUE;
}

#endif