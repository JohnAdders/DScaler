/////////////////////////////////////////////////////////////////////////////
// $Id: debug.cpp,v 1.3 2003-03-05 21:44:22 tobbej Exp $
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
// Change Log
//
// Date          Developer             Changes
//
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.2  2001/12/16 16:31:43  adcockj
// Bug fixes
//
// Revision 1.1  2001/12/09 22:01:48  tobbej
// experimental dshow support, doesnt work yet
// define WANT_DSHOW_SUPPORT if you want to try it
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file debug.cpp some dshow debuging functions
 */

#include "stdafx.h"
#include "debug.h"
#include "pinenum.h"
#include <dvdmedia.h>
#ifdef WANT_DSHOW_SUPPORT

#ifdef _DEBUG


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
#endif

///@return name of the guid
char* GetGUIDName(GUID &guid)
{
	static char fourcc_buffer[20];
	struct TGUID2NAME
	{
		char *szName;
		GUID guid;
	};
	#define OUR_GUID_ENTRY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    { #name, { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } } },
	TGUID2NAME names[] =
	{
		#include <uuids.h>
	};

	if(guid==GUID_NULL)
	{
		return "GUID_NULL";
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
		char tmp[sizeof(DWORD)+1];
		memset(tmp,0,sizeof(DWORD)+1);
		memcpy(tmp,&guid.Data1,sizeof(DWORD));
		_snprintf(fourcc_buffer,20,"FOURCC '%s'",tmp);
		return fourcc_buffer;
	}
	return "Unknown GUID";
}

void DumpGraph(IFilterGraph *pGraph,std::string &text)
{
	stringstream str;
	CDShowGenericEnum<IEnumFilters,IBaseFilter> Filters;
	HRESULT hr=pGraph->EnumFilters(&Filters.m_pEnum);
	if(FAILED(hr))
	{
		str << "Failed to get filter enumerator!!!";
		return;
	}
	str << "---FilterGraph: 0x" << hex << pGraph << dec << "---" << endl;
	CComPtr<IBaseFilter> pFilter;
	while(hr=Filters.next(&pFilter),hr==S_OK && pFilter!=NULL)
	{
		std::string tmp;
		DumpFilter(pFilter,tmp);
		str << tmp << endl;
		pFilter.Release();
	}
	text=str.str();
}

void DumpPreferredMediaTypes(CComPtr<IBaseFilter> pFilter,std::string &text)
{
	USES_CONVERSION;
	stringstream str;
	CDShowPinEnum pins(pFilter);
	
	FILTER_INFO FilterInfo;
	memset(&FilterInfo,0,sizeof(FILTER_INFO));
	
	str << "---Preferred media types for filter: ";
	HRESULT hr=pFilter->QueryFilterInfo(&FilterInfo);
	if(SUCCEEDED(hr))
	{
		str << W2A(FilterInfo.achName) << "---" << endl;
	}
	else
	{
		str << "Unknown filter 0x" << hex << pFilter.p << dec << "---" << endl;
	}
	if(FilterInfo.pGraph!=NULL)
	{
		FilterInfo.pGraph->Release();
		FilterInfo.pGraph=NULL;
	}
	
	
	CComPtr<IPin> pin;
	while(pin=pins.next(),pin!=NULL)
	{
		str << " ---Pin: ";
		PIN_INFO PinInfo;
		memset(&PinInfo,0,sizeof(PIN_INFO));
		hr=pin->QueryPinInfo(&PinInfo);
		if(SUCCEEDED(hr))
		{
			str << W2A(PinInfo.achName);
		}
		else
		{
			str << "Unknown pin";
		}
		if(PinInfo.pFilter!=NULL)
		{
			PinInfo.pFilter->Release();
			PinInfo.pFilter=NULL;
		}
		str	<< " (0x" << hex << pin.p << dec << ") ";
		if(PinInfo.dir==PINDIR_OUTPUT)
		{
			str << "(Output)";
		}
		else
		{
			str << "(Input)";
		}
		str << "---" << endl;



		CDShowGenericEnum<IEnumMediaTypes,AM_MEDIA_TYPE> MediaTypes;
		HRESULT hr=pin->EnumMediaTypes(&MediaTypes.m_pEnum);
		if(FAILED(hr))
		{
			str << " Failed to get mediatype enumerator!!!";
			continue;
		}
		AM_MEDIA_TYPE *mt=NULL;
		while(hr=MediaTypes.next(&mt),hr==S_OK,mt!=NULL)
		{
			std::string tmp;
			DumpMediaType(mt,tmp);
			str << tmp << endl << " ---" << endl;
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

void DumpFilter(CComPtr<IBaseFilter> pFilter,std::string &text)
{
	USES_CONVERSION;
	stringstream str;
	CDShowPinEnum pins(pFilter);
	FILTER_INFO FilterInfo;
	memset(&FilterInfo,0,sizeof(FILTER_INFO));
	
	HRESULT hr=pFilter->QueryFilterInfo(&FilterInfo);
	if(SUCCEEDED(hr))
	{
		str << "---" << W2A(FilterInfo.achName) << "---" << endl;
	}
	else
	{
		str << "---Unknown filter 0x" << hex << pFilter.p << dec << "---" << endl;
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
			str << " ---" << W2A(PinInfo.achName);
		}
		else
		{
			str << " ---Unknown pin";
		}
		if(PinInfo.pFilter!=NULL)
		{
			PinInfo.pFilter->Release();
			PinInfo.pFilter=NULL;
		}
		str	<< " (0x" << hex << pin.p << dec << ") ";
		if(PinInfo.dir==PINDIR_OUTPUT)
		{
			str << "(Output)";
		}
		else
		{
			str << "(Input)";
		}
		str << "---" << endl;
		
		CComPtr<IPin> pConnectedTo;
		hr=pin->ConnectedTo(&pConnectedTo);
		if(SUCCEEDED(hr) && pConnectedTo!=NULL)
		{
			str << " ConnectedTo: 0x" << hex << pConnectedTo.p << dec << endl;
			AM_MEDIA_TYPE mt;
			memset(&mt,0,sizeof(AM_MEDIA_TYPE));
			hr=pin->ConnectionMediaType(&mt);
			if(SUCCEEDED(hr))
			{
				std::string mttext;
				DumpMediaType(&mt,mttext);
				str << " ConnectionMediaType:" << endl << mttext << endl;
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

void DumpMediaType(AM_MEDIA_TYPE *mt,std::string &text)
{
	struct TFlag2String
	{
		DWORD dwFlag;
		char *szName;
	};
	stringstream str;
	ASSERT(mt!=NULL);
	str << " MajorType: " << GetGUIDName(mt->majortype) << endl;
	str << " SubType: " << GetGUIDName(mt->subtype) << endl;
	
	if(mt->bFixedSizeSamples)
	{
		str << " SampleSize: " << mt->lSampleSize;
	}
	else
	{
		str << " Variable size samples";
	}
	if(mt->bTemporalCompression)
	{
		str << " Temporal compression";
	}
	else
	{
		str << " No temporal compression";
	}
	str << endl << " FormatType: " << GetGUIDName(mt->formattype) << endl;

	if(mt->formattype==FORMAT_VideoInfo || mt->formattype==FORMAT_VideoInfo2)
	{
		//common parts for VIDEOINFOHEADER and VIDEOINFOHEADER2
		VIDEOINFOHEADER *pHeader=(VIDEOINFOHEADER *)mt->pbFormat;
		str << " Source RECT: (L: " << pHeader->rcSource.left << " T: " << pHeader->rcSource.top << " R: " << pHeader->rcSource.right << " B: " << pHeader->rcSource.bottom << ")" << endl;
		str << " Target RECT: (L: " << pHeader->rcTarget.left << " T: " << pHeader->rcTarget.top << " R: " << pHeader->rcTarget.right << " B: " << pHeader->rcTarget.bottom << ")" << endl;
		str << " BitRate: " << pHeader->dwBitRate << " ErrorRate: " << pHeader->dwBitErrorRate << endl;
		str	<< " AvgTimePerFrame: " << 1/(pHeader->AvgTimePerFrame/(double)10000000) << " fps" << endl;
		
		BITMAPINFOHEADER *pmbi=NULL;
		if(mt->formattype==FORMAT_VideoInfo)
		{
			pmbi=&pHeader->bmiHeader;
		}
		else if(mt->formattype==FORMAT_VideoInfo2)
		{
			VIDEOINFOHEADER2 *pHeader2=(VIDEOINFOHEADER2 *)mt->pbFormat;
			pmbi=&pHeader2->bmiHeader;
			str << " InterlaceFlags: " << pHeader2->dwInterlaceFlags << " (";
			
			TFlag2String flags[]=
			{
				{AMINTERLACE_IsInterlaced,"AMINTERLACE_IsInterlaced"},
				{AMINTERLACE_1FieldPerSample,"AMINTERLACE_1FieldPerSample"},
				{AMINTERLACE_Field1First,"AMINTERLACE_Field1First"},
				{AMINTERLACE_FieldPatField1Only,"AMINTERLACE_FieldPatField1Only"},
				{AMINTERLACE_FieldPatField2Only,"AMINTERLACE_FieldPatField2Only"},
				{AMINTERLACE_FieldPatBothRegular,"AMINTERLACE_FieldPatBothRegular"},
				{AMINTERLACE_DisplayModeBobOnly,"AMINTERLACE_DisplayModeBobOnly"},
				{AMINTERLACE_DisplayModeWeaveOnly,"AMINTERLACE_DisplayModeWeaveOnly"},
				{AMINTERLACE_DisplayModeBobOrWeave,"AMINTERLACE_DisplayModeBobOrWeave"}
			};
			
			for(int i=0;i<sizeof(flags)/sizeof(flags[0]);i++)
			{
				if(flags[i].dwFlag&pHeader2->dwInterlaceFlags)
				{
					if(i!=0)
					{
						str << "|";
					}
					str << flags[i].szName;
				}
			}
			str	<< ")" << endl;
			str << " AspectRatio: " << pHeader2->dwPictAspectRatioX << "x" << pHeader2->dwPictAspectRatioY << "(" << pHeader2->dwPictAspectRatioX/(double)pHeader2->dwPictAspectRatioY << ")" << endl;
			if(pHeader2->dwControlFlags&AMCONTROL_USED)
			{
				str << " ControllFlags: ";
				if(pHeader2->dwControlFlags&AMCONTROL_PAD_TO_4x3)
				{
					str << "Pad to 4x3";
				}
				else if(pHeader2->dwControlFlags&AMCONTROL_PAD_TO_16x9)
				{
					str << "Pad to 16x9";
				}
				else
				{
					str << "Unknown ControllFlag " << pHeader2->dwControlFlags;
				}
			}
		}
		
		//bitmapinfoheader
		ASSERT(pmbi!=NULL);
		str << " " << pmbi->biWidth << "x" << pmbi->biHeight << " " << pmbi->biBitCount << " bits";
	}
	else if(mt->formattype==FORMAT_WaveFormatEx)
	{
		WAVEFORMATEXTENSIBLE *pwfx=(WAVEFORMATEXTENSIBLE *)mt->pbFormat;
		str << " wFormatTag: " << pwfx->Format.wFormatTag << endl;
		str << " nChannels: " << pwfx->Format.nChannels << endl;
		str << " nSamplesPerSec: " << pwfx->Format.nSamplesPerSec << endl;
		str << " nAvgBytesPerSec: " << pwfx->Format.nAvgBytesPerSec << endl;
		str << " nBlockAlign: " << pwfx->Format.nBlockAlign << endl;
		str << " wBitsPerSample: " << pwfx->Format.wBitsPerSample << endl;
		str << " cbSize: " << pwfx->Format.cbSize << endl;
		if(pwfx->Format.wFormatTag==WAVE_FORMAT_EXTENSIBLE && pwfx->Format.cbSize>=22)
		{
			str << " dwChannelMask: ";
			
			TFlag2String flags[]=
			{
				{SPEAKER_FRONT_LEFT ,"SPEAKER_FRONT_LEFT"},
				{SPEAKER_FRONT_RIGHT,"SPEAKER_FRONT_RIGHT"},
				{SPEAKER_FRONT_CENTER,"SPEAKER_FRONT_CENTER"},
				{SPEAKER_LOW_FREQUENCY,"SPEAKER_LOW_FREQUENCY"},
				{SPEAKER_BACK_LEFT,"SPEAKER_BACK_LEFT"},
				{SPEAKER_BACK_RIGHT,"SPEAKER_BACK_RIGHT"},
				{SPEAKER_FRONT_LEFT_OF_CENTER,"SPEAKER_FRONT_LEFT_OF_CENTER"},
				{SPEAKER_FRONT_RIGHT_OF_CENTER,"SPEAKER_FRONT_RIGHT_OF_CENTER"},
				{SPEAKER_BACK_CENTER,"SPEAKER_BACK_CENTER"},
				{SPEAKER_SIDE_LEFT,"SPEAKER_SIDE_LEFT"},
				{SPEAKER_SIDE_RIGHT,"SPEAKER_SIDE_RIGHT"},
				{SPEAKER_TOP_CENTER,"SPEAKER_TOP_CENTER"},
				{SPEAKER_TOP_FRONT_LEFT,"SPEAKER_TOP_FRONT_LEFT"},
				{SPEAKER_TOP_FRONT_CENTER,"SPEAKER_TOP_FRONT_CENTER"},
				{SPEAKER_TOP_FRONT_RIGHT,"SPEAKER_TOP_FRONT_RIGHT"},
				{SPEAKER_TOP_BACK_LEFT,"SPEAKER_TOP_BACK_LEFT"},
				{SPEAKER_TOP_BACK_CENTER,"SPEAKER_TOP_BACK_CENTER"},
				{SPEAKER_TOP_BACK_RIGHT,"SPEAKER_TOP_BACK_RIGHT"}
			};
			
			for(int i=0;i<sizeof(flags)/sizeof(flags[0]);i++)
			{
				if(flags[i].dwFlag&pwfx->dwChannelMask)
				{
					if(i!=0)
					{
						str << "|";
					}
					str << flags[i].szName;
				}
			}
			str << " SubFormat: " << GetGUIDName(pwfx->SubFormat) << endl;
		}
		
	}
	text=str.str();
}
#endif
