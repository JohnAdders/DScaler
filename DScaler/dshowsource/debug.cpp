/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Torbjörn Jansson.  All rights reserved.
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
 * @file debug.cpp some dshow debuging functions
 */

#include "stdafx.h"
#ifdef WANT_DSHOW_SUPPORT

#include "debug.h"
#include "pinenum.h"
#include <dvdmedia.h>

using namespace std;

HRESULT AddToRot(IUnknown *pUnkGraph, DWORD *pdwRegister)
{
    CComPtr<IMoniker> pMoniker;
    CComPtr<IRunningObjectTable> pROT;
    if(FAILED(GetRunningObjectTable(0, &pROT)))
    {
        return E_FAIL;
    }

    WCHAR wsz[256];
    wsprintfW(wsz, L"FilterGraph %08p pid %08x", (DWORD_PTR)pUnkGraph, GetCurrentProcessId());
    HRESULT hr = CreateItemMoniker(L"!", wsz, &pMoniker);
    if(SUCCEEDED(hr))
    {
        hr = pROT->Register(0, pUnkGraph, pMoniker, pdwRegister);
    }
    return hr;
}

void RemoveFromRot(DWORD pdwRegister)
{
    CComPtr<IRunningObjectTable> pROT;
    if(SUCCEEDED(GetRunningObjectTable(0, &pROT)))
    {
        pROT->Revoke(pdwRegister);
    }
}
//#endif

///@return name of the guid
TCHAR* GetGUIDName(GUID &guid)
{
    struct TGUID2NAME
    {
        TCHAR* szName;
        GUID guid;
    };
    #define OUR_GUID_ENTRY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    { _T(#name), { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } } },
    TGUID2NAME names[] =
    {
        #include <uuids.h>
    };

    if(guid==GUID_NULL)
    {
        return _T("GUID_NULL");
    }
    for(int i=0;i<(sizeof(names)/sizeof(names[0]));i++)
    {
        if(names[i].guid==guid)
        {
            return names[i].szName;
        }
    }
    if(guid.Data2==0 && guid.Data3==0x10 && ((DWORD *)guid.Data4)[0]==0xaa000080 && ((DWORD *)guid.Data4)[1]==0x719b3800)
    {
        static TCHAR fourcc_buffer[20];
        BYTE* pCC = (BYTE*)&guid.Data1;
        _stprintf(fourcc_buffer,_T("FOURCC '%c%c%c%c'"),pCC[3], pCC[2] ,pCC[1], pCC[0]);
        return fourcc_buffer;
    }
    return _T("Unknown GUID");
}

void DumpGraph(IFilterGraph *pGraph,tstring &text)
{
    tostringstream str;
    CDShowGenericEnum<IEnumFilters,IBaseFilter> Filters;
    HRESULT hr=pGraph->EnumFilters(&Filters.m_pEnum);
    if(FAILED(hr))
    {
        str << _T("Failed to get filter enumerator!!!");
        return;
    }
    str << _T("---FilterGraph: 0x") << hex << pGraph << dec << _T("---") << endl;
    CComPtr<IBaseFilter> pFilter;
    while(hr=Filters.next(&pFilter),hr==S_OK && pFilter!=NULL)
    {
        tstring tmp;
        DumpFilter(pFilter,tmp);
        str << tmp << endl;
        pFilter.Release();
    }
    text=str.str();
}

void DumpPreferredMediaTypes(CComPtr<IBaseFilter> pFilter,tstring &text)
{
    USES_CONVERSION;
    tostringstream str;
    CDShowPinEnum pins(pFilter);

    FILTER_INFO FilterInfo;
    memset(&FilterInfo,0,sizeof(FILTER_INFO));

    str << _T("---Preferred media types for filter: ");
    HRESULT hr=pFilter->QueryFilterInfo(&FilterInfo);
    if(SUCCEEDED(hr))
    {
        str << W2A(FilterInfo.achName) << _T("---") << endl;
    }
    else
    {
        str << _T("Unknown filter 0x") << hex << pFilter.p << dec << _T("---") << endl;
    }
    if(FilterInfo.pGraph!=NULL)
    {
        FilterInfo.pGraph->Release();
        FilterInfo.pGraph=NULL;
    }


    CComPtr<IPin> pin;
    while(pin=pins.next(),pin!=NULL)
    {
        str << _T(" ---Pin: ");
        PIN_INFO PinInfo;
        memset(&PinInfo,0,sizeof(PIN_INFO));
        hr=pin->QueryPinInfo(&PinInfo);
        if(SUCCEEDED(hr))
        {
            str << W2A(PinInfo.achName);
        }
        else
        {
            str << _T("Unknown pin");
        }
        if(PinInfo.pFilter!=NULL)
        {
            PinInfo.pFilter->Release();
            PinInfo.pFilter=NULL;
        }
        str    << _T(" (0x") << hex << pin.p << dec << _T(") ");
        if(PinInfo.dir==PINDIR_OUTPUT)
        {
            str << _T("(Output)");
        }
        else
        {
            str << _T("(Input)");
        }
        str << _T("---") << endl;



        CDShowGenericEnum<IEnumMediaTypes,AM_MEDIA_TYPE> MediaTypes;
        HRESULT hr=pin->EnumMediaTypes(&MediaTypes.m_pEnum);
        if(FAILED(hr))
        {
            str << _T(" Failed to get mediatype enumerator!!!");
            continue;
        }
        AM_MEDIA_TYPE *mt=NULL;
        while(hr=MediaTypes.next(&mt),hr==S_OK,mt!=NULL)
        {
            tstring tmp;
            DumpMediaType(mt,tmp);
            str << tmp << endl << _T(" ---") << endl;
            if(mt->pUnk!=NULL)
            {
                mt->pUnk->Release();
                mt->pUnk=NULL;
            }
            if(mt->cbFormat>0 && mt->pbFormat!=NULL)
            {
                CoTaskMemFree(mt->pbFormat);
                mt->pbFormat=NULL;
            }
            CoTaskMemFree(mt);
            mt=NULL;
        }
    }
    text=str.str();
}

void DumpFilter(CComPtr<IBaseFilter> pFilter,tstring &text)
{
    USES_CONVERSION;
    tostringstream str;
    CDShowPinEnum pins(pFilter);
    FILTER_INFO FilterInfo;
    memset(&FilterInfo,0,sizeof(FILTER_INFO));

    HRESULT hr=pFilter->QueryFilterInfo(&FilterInfo);
    if(SUCCEEDED(hr))
    {
        str << _T("---") << W2A(FilterInfo.achName) << _T("---") << endl;
    }
    else
    {
        str << _T("---Unknown filter 0x") << hex << pFilter.p << dec << _T("---") << endl;
    }
    if(FilterInfo.pGraph!=NULL)
    {
        FilterInfo.pGraph->Release();
        FilterInfo.pGraph=NULL;
    }

    CComPtr<IPin> pin;
    while(pin=pins.next(),pin!=NULL)
    {
        PIN_INFO PinInfo;
        memset(&PinInfo,0,sizeof(PIN_INFO));
        hr=pin->QueryPinInfo(&PinInfo);
        if(SUCCEEDED(hr))
        {
            str << _T(" ---") << W2A(PinInfo.achName);
        }
        else
        {
            str << _T(" ---Unknown pin");
        }
        if(PinInfo.pFilter!=NULL)
        {
            PinInfo.pFilter->Release();
            PinInfo.pFilter=NULL;
        }
        str    << _T(" (0x") << hex << pin.p << dec << _T(") ");
        if(PinInfo.dir==PINDIR_OUTPUT)
        {
            str << _T("(Output)");
        }
        else
        {
            str << _T("(Input)");
        }
        str << _T("---") << endl;

        CComPtr<IPin> pConnectedTo;
        hr=pin->ConnectedTo(&pConnectedTo);
        if(SUCCEEDED(hr) && pConnectedTo!=NULL)
        {
            str << _T(" ConnectedTo: 0x") << hex << pConnectedTo.p << dec << endl;
            AM_MEDIA_TYPE mt;
            memset(&mt,0,sizeof(AM_MEDIA_TYPE));
            hr=pin->ConnectionMediaType(&mt);
            if(SUCCEEDED(hr))
            {
                tstring mttext;
                DumpMediaType(&mt,mttext);
                str << _T(" ConnectionMediaType:") << endl << mttext << endl;
                if(mt.cbFormat>0 && mt.pbFormat!=NULL)
                {
                    CoTaskMemFree(mt.pbFormat);
                    mt.pbFormat=NULL;
                    mt.cbFormat=0;
                }
                if(mt.pUnk!=NULL)
                {
                    mt.pUnk->Release();
                    mt.pUnk=NULL;
                }
            }
        }

    }
    text=str.str();
}

void DumpMediaType(AM_MEDIA_TYPE *mt,tstring &text)
{
    struct TFlag2String
    {
        DWORD dwFlag;
        TCHAR* szName;
    };
    tostringstream str;
    _ASSERTE(mt!=NULL);
    str << _T(" MajorType: ") << GetGUIDName(mt->majortype) << endl;
    str << _T(" SubType: ") << GetGUIDName(mt->subtype) << endl;

    if(mt->bFixedSizeSamples)
    {
        str << _T(" SampleSize: ") << mt->lSampleSize;
    }
    else
    {
        str << _T(" Variable size samples");
    }
    if(mt->bTemporalCompression)
    {
        str << _T(" Temporal compression");
    }
    else
    {
        str << _T(" No temporal compression");
    }
    str << endl << _T(" FormatType: ") << GetGUIDName(mt->formattype) << endl;

    if(mt->formattype==FORMAT_VideoInfo || mt->formattype==FORMAT_VideoInfo2)
    {
        //common parts for VIDEOINFOHEADER and VIDEOINFOHEADER2
        VIDEOINFOHEADER *pHeader=(VIDEOINFOHEADER *)mt->pbFormat;
        str << _T(" Source RECT: (L: ") << pHeader->rcSource.left << _T(" T: ") << pHeader->rcSource.top << _T(" R: ") << pHeader->rcSource.right << _T(" B: ") << pHeader->rcSource.bottom << _T(")") << endl;
        str << _T(" Target RECT: (L: ") << pHeader->rcTarget.left << _T(" T: ") << pHeader->rcTarget.top << _T(" R: ") << pHeader->rcTarget.right << _T(" B: ") << pHeader->rcTarget.bottom << _T(")") << endl;
        str << _T(" BitRate: ") << pHeader->dwBitRate << _T(" ErrorRate: ") << pHeader->dwBitErrorRate << endl;
        str    << _T(" AvgTimePerFrame: ") << 1/(pHeader->AvgTimePerFrame/(double)10000000) << _T(" fps") << endl;

        BITMAPINFOHEADER *pmbi=NULL;
        if(mt->formattype==FORMAT_VideoInfo)
        {
            pmbi=&pHeader->bmiHeader;
        }
        else if(mt->formattype==FORMAT_VideoInfo2)
        {
            VIDEOINFOHEADER2 *pHeader2=(VIDEOINFOHEADER2 *)mt->pbFormat;
            pmbi=&pHeader2->bmiHeader;
            str << _T(" InterlaceFlags: ") << pHeader2->dwInterlaceFlags << _T(" (");

            TFlag2String flags[]=
            {
                {AMINTERLACE_IsInterlaced,_T("AMINTERLACE_IsInterlaced")},
                {AMINTERLACE_1FieldPerSample,_T("AMINTERLACE_1FieldPerSample")},
                {AMINTERLACE_Field1First,_T("AMINTERLACE_Field1First")},
                {AMINTERLACE_FieldPatField1Only,_T("AMINTERLACE_FieldPatField1Only")},
                {AMINTERLACE_FieldPatField2Only,_T("AMINTERLACE_FieldPatField2Only")},
                {AMINTERLACE_FieldPatBothRegular,_T("AMINTERLACE_FieldPatBothRegular")},
                {AMINTERLACE_DisplayModeBobOnly,_T("AMINTERLACE_DisplayModeBobOnly")},
                {AMINTERLACE_DisplayModeWeaveOnly,_T("AMINTERLACE_DisplayModeWeaveOnly")},
                {AMINTERLACE_DisplayModeBobOrWeave,_T("AMINTERLACE_DisplayModeBobOrWeave")}
            };

            for(int i=0;i<sizeof(flags)/sizeof(flags[0]);i++)
            {
                if(flags[i].dwFlag&pHeader2->dwInterlaceFlags)
                {
                    if(i!=0)
                    {
                        str << _T("|");
                    }
                    str << flags[i].szName;
                }
            }
            str    << _T(")") << endl;
            str << _T(" AspectRatio: ") << pHeader2->dwPictAspectRatioX << _T("x") << pHeader2->dwPictAspectRatioY << _T("(") << pHeader2->dwPictAspectRatioX/(double)pHeader2->dwPictAspectRatioY << _T(")") << endl;
            if(pHeader2->dwControlFlags&AMCONTROL_USED)
            {
                str << _T(" ControllFlags: ");
                if(pHeader2->dwControlFlags&AMCONTROL_PAD_TO_4x3)
                {
                    str << _T("Pad to 4x3");
                }
                else if(pHeader2->dwControlFlags&AMCONTROL_PAD_TO_16x9)
                {
                    str << _T("Pad to 16x9");
                }
                else
                {
                    str << _T("Unknown ControllFlag ") << pHeader2->dwControlFlags;
                }
            }
        }

        //bitmapinfoheader
        _ASSERTE(pmbi!=NULL);
        str << _T(" ") << pmbi->biWidth << _T("x") << pmbi->biHeight << _T(" ") << pmbi->biBitCount << _T(" bits") << endl;
        str << _T(" biPlanes: ") << pmbi->biPlanes << endl;
        str << _T(" biSizeImage: ")<< pmbi->biSizeImage;

    }
    else if(mt->formattype==FORMAT_WaveFormatEx)
    {
        WAVEFORMATEXTENSIBLE *pwfx=(WAVEFORMATEXTENSIBLE *)mt->pbFormat;
        str << _T(" wFormatTag: ") << pwfx->Format.wFormatTag << endl;
        str << _T(" nChannels: ") << pwfx->Format.nChannels << endl;
        str << _T(" nSamplesPerSec: ") << pwfx->Format.nSamplesPerSec << endl;
        str << _T(" nAvgBytesPerSec: ") << pwfx->Format.nAvgBytesPerSec << endl;
        str << _T(" nBlockAlign: ") << pwfx->Format.nBlockAlign << endl;
        str << _T(" wBitsPerSample: ") << pwfx->Format.wBitsPerSample << endl;
        str << _T(" cbSize: ") << pwfx->Format.cbSize << endl;
        if(pwfx->Format.wFormatTag==WAVE_FORMAT_EXTENSIBLE && pwfx->Format.cbSize>=22)
        {
            str << _T(" dwChannelMask: ");

            TFlag2String flags[]=
            {
                {SPEAKER_FRONT_LEFT ,_T("SPEAKER_FRONT_LEFT")},
                {SPEAKER_FRONT_RIGHT,_T("SPEAKER_FRONT_RIGHT")},
                {SPEAKER_FRONT_CENTER,_T("SPEAKER_FRONT_CENTER")},
                {SPEAKER_LOW_FREQUENCY,_T("SPEAKER_LOW_FREQUENCY")},
                {SPEAKER_BACK_LEFT,_T("SPEAKER_BACK_LEFT")},
                {SPEAKER_BACK_RIGHT,_T("SPEAKER_BACK_RIGHT")},
                {SPEAKER_FRONT_LEFT_OF_CENTER,_T("SPEAKER_FRONT_LEFT_OF_CENTER")},
                {SPEAKER_FRONT_RIGHT_OF_CENTER,_T("SPEAKER_FRONT_RIGHT_OF_CENTER")},
                {SPEAKER_BACK_CENTER,_T("SPEAKER_BACK_CENTER")},
                {SPEAKER_SIDE_LEFT,_T("SPEAKER_SIDE_LEFT")},
                {SPEAKER_SIDE_RIGHT,_T("SPEAKER_SIDE_RIGHT")},
                {SPEAKER_TOP_CENTER,_T("SPEAKER_TOP_CENTER")},
                {SPEAKER_TOP_FRONT_LEFT,_T("SPEAKER_TOP_FRONT_LEFT")},
                {SPEAKER_TOP_FRONT_CENTER,_T("SPEAKER_TOP_FRONT_CENTER")},
                {SPEAKER_TOP_FRONT_RIGHT,_T("SPEAKER_TOP_FRONT_RIGHT")},
                {SPEAKER_TOP_BACK_LEFT,_T("SPEAKER_TOP_BACK_LEFT")},
                {SPEAKER_TOP_BACK_CENTER,_T("SPEAKER_TOP_BACK_CENTER")},
                {SPEAKER_TOP_BACK_RIGHT,_T("SPEAKER_TOP_BACK_RIGHT")}
            };

            for(int i=0;i<sizeof(flags)/sizeof(flags[0]);i++)
            {
                if(flags[i].dwFlag&pwfx->dwChannelMask)
                {
                    if(i!=0)
                    {
                        str << _T("|");
                    }
                    str << flags[i].szName;
                }
            }
            str << _T(" SubFormat: ") << GetGUIDName(pwfx->SubFormat) << endl;
        }

    }
    text=str.str();
}
#endif
