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
 * @file DSRendInPin.cpp Implementation of CDSRendInPin
 */

#include "StdAfx.h"
#include "DSRend.h"
#include "DSRendInPin.h"
#include "DSRendFilter.h"
#include "mediatypes.h"

#include <dvdmedia.h>

CDSRendInPin::CDSRendInPin(CDSRendFilter *pFilter)
:m_pFilter(pFilter),m_bFlushing(false),m_bForceYUY2(FALSE),m_FieldFormat(DSREND_FIELD_FORMAT_AUTO)
{
    //clear mediatype
    memset(&m_mt,0,sizeof(m_mt));
}

HRESULT CDSRendInPin::FinalConstruct()
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::FinalConstruct\n"),__FILE__,__LINE__);
    return S_OK;
}

HRESULT CDSRendInPin::FinalRelease()
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::FinalRelease\n"),__FILE__,__LINE__);
    if(m_pFilter!=NULL)
    {
        m_pFilter=NULL;
    }
    return S_OK;
}

// IDSRendSettings
STDMETHODIMP CDSRendInPin::get_Status(DSRendStatus * pVal)
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::get_GetStatus\n"),__FILE__,__LINE__);
    if (pVal == NULL)
        return E_POINTER;
    
    if(!isConnected())
    {
        return VFW_E_NOT_CONNECTED;
    }
    else
    {
        return m_pFilter->m_FieldBuffer.GetStatus(*pVal);
    }
}

STDMETHODIMP CDSRendInPin::get_SwapFields(BOOL * pVal)
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::get_SwapFields\n"),__FILE__,__LINE__);
    if (pVal == NULL)
        return E_POINTER;
    *pVal=m_pFilter->m_FieldBuffer.GetSwapFields();
    return S_OK;
}

STDMETHODIMP CDSRendInPin::put_SwapFields(BOOL pVal)
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::put_SwapFields\n"),__FILE__,__LINE__);
    m_pFilter->m_FieldBuffer.SetSwapFields(pVal!=FALSE);
    m_pFilter->SetDirty(true);
    return S_OK;
}

STDMETHODIMP CDSRendInPin::get_ForceYUY2(BOOL * pVal)
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::get_ForceYUY2\n"),__FILE__,__LINE__);
    if (pVal == NULL)
        return E_POINTER;
    
    *pVal=m_bForceYUY2;
    return S_OK;
}

STDMETHODIMP CDSRendInPin::put_ForceYUY2(BOOL pVal)
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::put_ForceYUY2\n"),__FILE__,__LINE__);
    m_bForceYUY2=pVal;
    m_pFilter->SetDirty(true);
    return S_OK;
}

STDMETHODIMP CDSRendInPin::get_FieldFormat(DSREND_FIELD_FORMAT * pVal)
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::get_FieldFormat\n"),__FILE__,__LINE__);
    if (pVal == NULL)
        return E_POINTER;
    *pVal=m_FieldFormat;
    return S_OK;
}

STDMETHODIMP CDSRendInPin::put_FieldFormat(DSREND_FIELD_FORMAT pVal)
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::put_FieldFormat\n"),__FILE__,__LINE__);
    m_FieldFormat=pVal;
    m_pFilter->SetDirty(true);
    return S_OK;
}

STDMETHODIMP CDSRendInPin::get_VertMirror(BOOL *pVal)
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::get_VertMirror\n"),__FILE__,__LINE__);
    if(pVal == NULL)
        return E_POINTER;
    *pVal=m_pFilter->m_FieldBuffer.GetVertMirror();
    return S_OK;
}

STDMETHODIMP CDSRendInPin::put_VertMirror(BOOL newVal)
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::put_VertMirror\n"),__FILE__,__LINE__);
    m_pFilter->m_FieldBuffer.SetVertMirror(newVal!=FALSE);
    m_pFilter->SetDirty(true);
    return S_OK;
}

// IPin
HRESULT CDSRendInPin::Connect(IPin *pReceivePin,const AM_MEDIA_TYPE *pmt)
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::Connect\n"),__FILE__,__LINE__);
    return E_NOTIMPL;
    
    /*CAutoLockCriticalSection lock(&m_Lock);
    HRESULT hr;

    //dont know if this is realy needed in the renderer, it seems like the Connect method
    //is called only on output pins, and ReciveConnection is called on inputs

    if(pReceivePin==NULL)
    {
        return E_POINTER;
    }
    if(isConnected())
    {
        return VFW_E_ALREADY_CONNECTED;
    }
    if(!m_pFilter->isStopped())
    {
        return VFW_E_NOT_STOPPED;
    }

    if(pmt!=NULL)
    {
        //check if we accept the mediatype
        if(FAILED(CheckMediaType(pmt)))
        {
            return VFW_E_TYPE_NOT_ACCEPTED;
        }
        else
        {
            hr=pReceivePin->ReceiveConnection(this,pmt);
            if(SUCCEEDED(hr))
            {
                m_pConnected=pReceivePin;
                freeMediaType(&m_mt);
                copyMediaType(&m_mt,pmt);
            }
            return hr;
        }
    }
    else
    {
        //FIXME: enumerate all mediatypes on the receiveing pin and check if its
        //prosibel to connect using one of them
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    return S_OK;*/
}

HRESULT CDSRendInPin::ReceiveConnection(IPin *pConnector,const AM_MEDIA_TYPE *pmt)
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::ReceiveConnection\n"),__FILE__,__LINE__);
    CAutoLockCriticalSection lock(&m_Lock);

    if(pConnector==NULL || pmt==NULL)
        return E_POINTER;

    if(isConnected())
    {
        return VFW_E_ALREADY_CONNECTED;
    }
    //check if the filter is stopped
    //this might have to chage if we want to support dyanic format changes while the graph is running
    if(!m_pFilter->isStopped())
    {
        return VFW_E_NOT_STOPPED;
    }

    //check the mediatype
    if(FAILED(CheckMediaType(pmt)))
    {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }
    HRESULT hr=m_pFilter->m_FieldBuffer.SetMediaType(pmt);
    if(FAILED(hr))
    {
        ATLTRACE(_T(" m_FieldBuffer.SetMediaType failed!!\n"));
        return hr;
    }

    m_pConnected=pConnector;
    freeMediaType(&m_mt);
    copyMediaType(&m_mt,pmt);

    return S_OK;
}

HRESULT CDSRendInPin::Disconnect()
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::Disconnect\n"),__FILE__,__LINE__);
    CAutoLockCriticalSection lock(&m_Lock);
    if(!m_pFilter->isStopped())
    {
        return VFW_E_NOT_STOPPED;
    }
    HRESULT hr=m_pConnected==NULL ? S_FALSE : S_OK;
    m_pConnected=NULL;
    freeMediaType(&m_mt);
    return hr;
}

HRESULT CDSRendInPin::ConnectedTo(IPin **ppPin)
{
    //ATLTRACE(_T("%s(%d) : CDSRendInPin::ConnectedTo\n"),__FILE__,__LINE__);
    CAutoLockCriticalSection lock(&m_Lock);
    if(ppPin==NULL)
    {
        return E_POINTER;
    }

    //is this pin unconnected?
    if(!isConnected())
    {
        *ppPin=NULL;
        return VFW_E_NOT_CONNECTED;
    }
    *ppPin=m_pConnected;
    (*ppPin)->AddRef();
    return S_OK;
}

HRESULT CDSRendInPin::ConnectionMediaType(AM_MEDIA_TYPE *pmt)
{
    //ATLTRACE(_T("%s(%d) : CDSRendInPin::ConnectionMediaType\n"),__FILE__,__LINE__);
    CAutoLockCriticalSection lock(&m_Lock);
    if(pmt==NULL)
    {
        return E_POINTER;
    }
    if(m_pConnected==NULL)
    {
        /*
        memset(pmt,0,sizeof(AM_MEDIA_TYPE));
        pmt->bFixedSizeSamples=TRUE;
        pmt->lSampleSize=1;
        */
        return VFW_E_NOT_CONNECTED;
    }
    if(copyMediaType(pmt,&m_mt))
        return S_OK;
    else
        return E_FAIL;
}

HRESULT CDSRendInPin::QueryPinInfo(PIN_INFO *pInfo)
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::QueryPinInfo\n"),__FILE__,__LINE__);
    if(pInfo==NULL)
    {
        return E_POINTER;
    }

    if(m_pFilter==NULL)
    {
        return E_FAIL;
    }

    IBaseFilter *pFilter=NULL;
    HRESULT hr=m_pFilter->QueryInterface(IID_IBaseFilter,(void**)&pFilter);
    if(FAILED(hr))
        return hr;

    pInfo->dir=PINDIR_INPUT;
    pInfo->pFilter=pFilter;
    WCHAR name[]=L"Input";

    memcpy(pInfo->achName,name,sizeof(name));
    return S_OK;
}

HRESULT CDSRendInPin::QueryId(LPWSTR *Id)
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::QueryId\n"),__FILE__,__LINE__);
    if(Id==NULL)
    {
        return E_POINTER;
    }
    WCHAR name[]=L"Input";
    *Id=(LPWSTR)CoTaskMemAlloc(sizeof(name));
    if(*Id==NULL)
    {
        return E_OUTOFMEMORY;
    }

    wcscpy(*Id,name);
    return S_OK;
}

HRESULT CDSRendInPin::QueryAccept(const AM_MEDIA_TYPE *pmt)
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::QueryAccept\n"),__FILE__,__LINE__);

    return SUCCEEDED(CheckMediaType(pmt)) ? S_OK : S_FALSE;
}

HRESULT CDSRendInPin::EnumMediaTypes(IEnumMediaTypes **ppEnum)
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::EnumMediaTypes\n"),__FILE__,__LINE__);
    if(ppEnum==NULL)
    {
        return E_POINTER;
    }

    CComObject<CComEnum<IEnumMediaTypes,&IID_IEnumMediaTypes,AM_MEDIA_TYPE*,CopyMT > > *pEnum=NULL;
    HRESULT hr=CComObject<CComEnum<IEnumMediaTypes,&IID_IEnumMediaTypes,AM_MEDIA_TYPE*,CopyMT > >::CreateInstance(&pEnum);
    if(FAILED(hr))
        return hr;
    
    AM_MEDIA_TYPE mt1;
    memset(&mt1,0,sizeof(mt1));
    mt1.majortype=MEDIATYPE_Video;
    mt1.subtype=MEDIASUBTYPE_YUY2;
    mt1.lSampleSize=1;
    mt1.bFixedSizeSamples=TRUE;

    AM_MEDIA_TYPE mt2;
    memset(&mt2,0,sizeof(mt2));
    mt2.majortype=MEDIATYPE_Video;
    mt2.subtype=MEDIASUBTYPE_RGB24;
    mt2.lSampleSize=1;
    mt2.bFixedSizeSamples=TRUE;

    AM_MEDIA_TYPE *mediaTypes[2];
    mediaTypes[0]=&mt1;
    mediaTypes[1]=&mt2;

    //maybe the GetUnknown() call is the cause for the problems with _ATL_DEBUG_INTERFACES ?
    //GetUnknown() doesnt addref the returned pointer
    hr=pEnum->Init(&mediaTypes[0],&mediaTypes[2],GetUnknown(),AtlFlagCopy);
    if(FAILED(hr))
    {
        delete pEnum;
        return hr;
    }
    //addref the enumerator interface, nessesary since CreateInstance dont addref the new object
    pEnum->AddRef();
    *ppEnum=pEnum;

    return hr;
}

HRESULT CDSRendInPin::QueryInternalConnections(IPin **apPin,ULONG *nPin)
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::QueryInternalConnections\n"),__FILE__,__LINE__);
    //ok to return E_NOTIMPL here since we have IAMMiscFlaggs interface
    return E_NOTIMPL;
}

HRESULT CDSRendInPin::EndOfStream()
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::EndOfStream\n"),__FILE__,__LINE__);
    CAutoLockCriticalSection lock(&m_Lock);
    if(isFlushing())
    {
        return E_FAIL;
    }
    
    //need to sync with Recive calls

    //need to queue this if we buffer samples
    m_pFilter->sendNotifyMsg(EC_COMPLETE,S_OK,0);

    return S_OK;
}

HRESULT CDSRendInPin::BeginFlush()
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::BeginFlush\n"),__FILE__,__LINE__);
    CAutoLockCriticalSection lock(&m_Lock);
    m_bFlushing=true;
    
    resumePause();

    //need to free our mediasamples here so we dont lock upstream filters
    //beginFlush in the filter will take care of that
    m_pFilter->beginFlush();
    
    return S_OK;
}

HRESULT CDSRendInPin::EndFlush()
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::EndFlush\n"),__FILE__,__LINE__);
    CAutoLockCriticalSection lock(&m_Lock);
    
    
    
    m_bFlushing=false;
    return S_OK;
}

HRESULT CDSRendInPin::NewSegment(REFERENCE_TIME tStart,REFERENCE_TIME tStop,double dRate)
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::NewSegment\n"),__FILE__,__LINE__);
    
    //maybe save the tStart ?
    
    return S_OK;
}

HRESULT CDSRendInPin::QueryDirection(PIN_DIRECTION *pPinDir)
{
    //ATLTRACE(_T("%s(%d) : CDSRendInPin::QueryDirection\n"),__FILE__,__LINE__);
    if(pPinDir==NULL)
        return E_POINTER;
    *pPinDir=PINDIR_INPUT;
    return S_OK;
}

// IMemInputPin
HRESULT CDSRendInPin::GetAllocator(IMemAllocator **ppAllocator)
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::GetAllocator\n"),__FILE__,__LINE__);
    
    /*CComPtr<IMemAllocator> pAlloc;
    HRESULT hr=pAlloc.CoCreateInstance(CLSID_MemoryAllocator);
    if(FAILED(hr))
        return hr;
    
    pAlloc->AddRef();
    *ppAllocator=pAlloc;
    return S_OK;*/

    //return VFW_E_NO_ALLOCATOR since we dont have any allocator
    //the upstream filter will provide one for us.
    //maybe the pin shoud provide its own memory allocator.
    return VFW_E_NO_ALLOCATOR;
}

HRESULT CDSRendInPin::NotifyAllocator(IMemAllocator *pAllocator,BOOL bReadOnly)
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::NotifyAllocator\n"),__FILE__,__LINE__);
    if(pAllocator==NULL)
    {
        return E_POINTER;
    }
    //ATLASSERT(bReadOnly==FALSE);

    ALLOCATOR_PROPERTIES prop;
    if(FAILED(pAllocator->GetProperties(&prop)))
    {
        return E_FAIL;
    }
    
    //check that the allocator properties is acceptable
    if(prop.cbAlign!=16 && FALSE)
    {
        return VFW_E_BADALIGN;
    }

    if(prop.cbPrefix!=0)
    {
        return E_FAIL;
    }
    ATLASSERT(prop.cBuffers>0);
    m_pFilter->m_FieldBuffer.SetMediaSampleCount(prop.cBuffers-1);
    return S_OK;
}

HRESULT CDSRendInPin::GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps)
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::GetAllocatorRequirements\n"),__FILE__,__LINE__);
    if(pProps==NULL)
        return E_POINTER;

    ATLASSERT(m_pFilter->m_FieldBuffer.GetFieldCount()>0);
    
    //request 16 byte aligned buffers. remeber that this is just a request,
    //need to check it in NotifyAllocator
    pProps->cbAlign=16;
    pProps->cbBuffer=0;
    pProps->cbPrefix=0;

    /**
     * @todo the number of extra buffers to request shod be user
     * configurable. if we dont request a few extra buffers and doing field
     * input, there will be a lot of dropped fields.
     * Maybe also check with CFieldBuffer if it can buffer samples directly
     * and only request extra fields if it can (field input)
     */
    pProps->cBuffers=m_pFilter->m_FieldBuffer.GetFieldCount()+8;
    return S_OK;
}

HRESULT CDSRendInPin::Receive(IMediaSample *pSample)
{
    //ATLTRACE(_T("%s(%d) : CDSRendInPin::Receive\n"),__FILE__,__LINE__);

    if(pSample==NULL)
    {
        return E_POINTER;
    }
    
    FILTER_STATE state;
    HRESULT hr=m_pFilter->GetState(0,&state);
    if(FAILED(hr))
        return hr;
    //isStopped must be synced with streaming thread,
    //but we cant hold the render lock here since that might cause a deadlock with stop
    if(state==State_Stopped)
    {
        return VFW_E_WRONG_STATE;
    }
    
    CAutoLockCriticalSection lock(&m_pFilter->m_renderLock);
    if(isFlushing())
    {
        return S_FALSE;
    }

    //check if the mediatype has changed
    ///@todo check for IMediaSample2
    AM_MEDIA_TYPE *pmt=NULL;
    if(pSample->GetMediaType(&pmt)==S_OK)
    {    
        /**
         * @todo if returning a failiure, send end of stream and post EC_ERRORABORT
         */
        if(FAILED(CheckMediaType(pmt)))
        {
            return VFW_E_TYPE_NOT_ACCEPTED;
        }
        m_pFilter->m_FieldBuffer.RemoveFields(INFINITE);
        hr=m_pFilter->m_FieldBuffer.SetMediaType(pmt);
        if(FAILED(hr))
        {
            ATLTRACE(_T(" m_FieldBuffer.SetMediaType failed!\n"));
            return hr;
        }
        
        freeMediaType(&m_mt);
        copyMediaType(&m_mt,pmt);
        freeMediaType(pmt);
    }

    //render the sample
    hr=m_pFilter->renderSample(pSample);
    
    //if the filter is paused, block
    if(state==State_Paused)
    {
        m_resumePauseEvent.Wait(INFINITE);
    }
    return hr;
}

HRESULT CDSRendInPin::ReceiveMultiple(IMediaSample **pSamples,long nSamples,long *nSamplesProcessed)
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::ReceiveMultiple\n"),__FILE__,__LINE__);
    HRESULT hr;
    if(pSamples==NULL || nSamples==NULL)
    {
        return E_POINTER;
    }
    ATLASSERT(nSamples>0);

    *nSamplesProcessed=0;
    for(long cnt=0;cnt<nSamples;cnt++)
    {
        hr=Receive(pSamples[*nSamplesProcessed]);
        if(hr!=S_OK)
            break;
        *nSamplesProcessed++;
    }
    return hr;
}

HRESULT CDSRendInPin::ReceiveCanBlock()
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::ReceiveCanBlock\n"),__FILE__,__LINE__);
    //S_OK == might block in Recive
    return S_OK;
}

// IAMFilterMiscFlags
ULONG CDSRendInPin::GetMiscFlags(void)
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::GetMiscFlags\n"),__FILE__,__LINE__);
    return AM_FILTER_MISC_FLAGS_IS_RENDERER;
}

HRESULT CDSRendInPin::CheckMediaType(const AM_MEDIA_TYPE *pmt)
{
    ATLTRACE(_T("%s(%d) : CDSRendInPin::CheckMediaType\n"),__FILE__,__LINE__);
    if(pmt==NULL)
    {
        return E_POINTER;
    }

    //we shoud probably make some more checks here
    //for example check the source and target rects
    if(pmt->majortype==MEDIATYPE_Video)
    {
        if(pmt->subtype!=MEDIASUBTYPE_YUY2)
        {
            if(m_bForceYUY2 || pmt->subtype!=MEDIASUBTYPE_RGB24)
            {
                return E_FAIL;
            }
        }/*
        if(pmt->subtype!=MEDIASUBTYPE_RGB24)
        {
            return E_FAIL;
        }*/

        bool bFieldInput;
        BITMAPINFOHEADER *pBmi=NULL;
        if(pmt->formattype==FORMAT_VideoInfo)
        {
            if(pmt->cbFormat>0)
            {
                VIDEOINFOHEADER *pHeader=(VIDEOINFOHEADER*)pmt->pbFormat;
                pBmi=&(pHeader->bmiHeader);
            }
            bFieldInput=false;
        }
        else if(pmt->formattype==FORMAT_VideoInfo2)
        {
            if(pmt->cbFormat>0)
            {
                VIDEOINFOHEADER2 *pHeader=(VIDEOINFOHEADER2*)pmt->pbFormat;
                pBmi=&(pHeader->bmiHeader);
                
                if(pHeader->dwInterlaceFlags&AMINTERLACE_1FieldPerSample)
                {
                    bFieldInput=true;
                    //must have both fields
                    switch(pHeader->dwInterlaceFlags&AMINTERLACE_FieldPatternMask)
                    {
                    case AMINTERLACE_FieldPatField1Only:
                    case AMINTERLACE_FieldPatField2Only:
                        return E_FAIL;
                    }
                    ATLASSERT((pHeader->dwInterlaceFlags&AMINTERLACE_FieldPatternMask)|(AMINTERLACE_FieldPatBothIrregular|AMINTERLACE_FieldPatBothRegular));
                    /*if((pHeader->dwInterlaceFlags&AMINTERLACE_DisplayModeMask)!=AMINTERLACE_DisplayModeBobOnly)
                    {
                        return E_FAIL;
                    }*/
                    //return E_FAIL;
                }
                else
                {
                    bFieldInput=false;
                }
            }
        }
        if((m_FieldFormat==DSREND_FIELD_FORMAT_FIELD && bFieldInput==false) || (m_FieldFormat==DSREND_FIELD_FORMAT_FRAME && bFieldInput==true))
        {
            return E_FAIL;  
        }

        //check that there is a size specified in the media type
        //and that the width is a multiple of 16 (alignment problems in dscaler)
        if(pBmi!=NULL && pBmi->biWidth!=0 && pBmi->biHeight!=0 && (pBmi->biWidth&0xf)==0)
        {
            return S_OK;
        }
    }
    return E_FAIL;
}

long CDSRendInPin::GetSize()
{
    return sizeof(m_FieldFormat)+sizeof(m_bForceYUY2)+sizeof(BOOL)*2;
}

HRESULT CDSRendInPin::SaveToStream(IStream *pStream)
{
    DSREND_FIELD_FORMAT fmt;
    BOOL bForceYUY2;
    BOOL bSwapFields;
    BOOL bVertMirror;
    
    HRESULT hr=get_FieldFormat(&fmt);
    if(FAILED(hr))
    {
        return hr;
    }
    hr=get_ForceYUY2(&bForceYUY2);
    if(FAILED(hr))
    {
        return hr;
    }
    hr=get_SwapFields(&bSwapFields);
    if(FAILED(hr))
    {
        return hr;
    }
    hr=get_VertMirror(&bVertMirror);
    if(FAILED(hr))
    {
        return hr;
    }

    hr=pStream->Write(&fmt,sizeof(fmt),NULL);
    if(FAILED(hr))
    {
        return hr;
    }
    hr=pStream->Write(&bForceYUY2,sizeof(bForceYUY2),NULL);
    if(FAILED(hr))
    {
        return hr;
    }
    hr=pStream->Write(&bSwapFields,sizeof(bSwapFields),NULL);
    if(FAILED(hr))
    {
        return hr;
    }
    hr=pStream->Write(&bVertMirror,sizeof(bVertMirror),NULL);
    if(FAILED(hr))
    {
        return hr;
    }
    return S_OK;
}

HRESULT CDSRendInPin::LoadFromStream(IStream *pStream,DWORD dwVersion)
{
    DSREND_FIELD_FORMAT fmt;
    BOOL bForceYUY2;
    BOOL bSwapFields;
    BOOL bVertMirror;

    if(dwVersion!=0)
    {
        return E_FAIL;
    }
    HRESULT hr=pStream->Read(&fmt,sizeof(fmt),NULL);
    if(FAILED(hr))
    {
        return hr;
    }
    hr=pStream->Read(&bForceYUY2,sizeof(bForceYUY2),NULL);
    if(FAILED(hr))
    {
        return hr;
    }
    hr=pStream->Read(&bSwapFields,sizeof(bSwapFields),NULL);
    if(FAILED(hr))
    {
        return hr;
    }
    hr=pStream->Read(&bVertMirror,sizeof(bVertMirror),NULL);
    if(FAILED(hr))
    {
        return hr;
    }

    hr=put_FieldFormat(fmt);
    if(FAILED(hr))
    {
        return hr;
    }
    hr=put_ForceYUY2(bForceYUY2);
    if(FAILED(hr))
    {
        return hr;
    }
    hr=put_SwapFields(bSwapFields);
    if(FAILED(hr))
    {
        return hr;
    }
    hr=put_VertMirror(bVertMirror);
    if(FAILED(hr))
    {
        return hr;
    }
    return S_OK;
}