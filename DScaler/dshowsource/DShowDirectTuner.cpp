/////////////////////////////////////////////////////////////////////////////
// $Id: DShowDirectTuner.cpp,v 1.1 2002-10-29 19:30:43 tobbej Exp $
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
// Change Log
//
// Date          Developer             Changes
//
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

/**
 * @file DShowDirectTuner.cpp implementation of the CDShowDirectTuner class.
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT

#include "dscaler.h"
#include "DShowDirectTuner.h"
#include "exception.h"
#include "debuglog.h"
#include <ks.h>
#include <ksmedia.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define INSTANCE_DATA_OF_PROPERTY_PTR(x) ( (PKSPROPERTY((x)) ) + 1 )
#define INSTANCE_DATA_OF_PROPERTY_SIZE(x) ( sizeof((x)) - sizeof(KSPROPERTY) )

//template function to get data from IKsPropertySet
template<typename DataType>
void GetKSData(CComPtr<IKsPropertySet> pKSProp,KSPROPERTY_TUNER Property,DataType &Data)
{
	DWORD dwSupported=0;
	HRESULT hr=pKSProp->QuerySupported(PROPSETID_TUNER,Property,&dwSupported);
	if(SUCCEEDED(hr) && dwSupported&KSPROPERTY_SUPPORT_GET)
	{
		DWORD cbBytes=0;
		hr=pKSProp->Get(PROPSETID_TUNER,
			Property,
			INSTANCE_DATA_OF_PROPERTY_PTR(&Data),
			INSTANCE_DATA_OF_PROPERTY_SIZE(Data),
			&Data,
			sizeof(Data),
			&cbBytes);
		if(SUCCEEDED(hr) && cbBytes>=sizeof(DataType))
		{
			return;
		}
		else
		{
			throw CDShowException(CString("GetKSData<")+typeid(Data).name()+">: Failed",hr);
		}
	}
	else
	{
		throw CDShowException(CString("GetKSData<")+typeid(Data).name()+">: Property is not supported by tuner",hr);
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDShowDirectTuner::CDShowDirectTuner(CComPtr<IAMTVTuner> pTVTuner,IGraphBuilder *pGraph)
:CDShowObject(pGraph),m_pTVTuner(pTVTuner)
{
	HRESULT hr=m_pTVTuner.QueryInterface(&m_pKSProp);
	if(FAILED(hr))
	{
		throw CDShowException("TVTuner does not support IKsPropertySet !!",hr);
	}
}

CDShowDirectTuner::~CDShowDirectTuner()
{

}

AMTunerModeType CDShowDirectTuner::GetTunerMode()
{
	AMTunerModeType mode;
	HRESULT hr=m_pTVTuner->get_Mode(&mode);
	if(FAILED(hr))
	{
		throw CDShowException("CDShowDirectTuner::GetTunerMode Failed",hr);
	}
	return mode;
}

void CDShowDirectTuner::SetTunerMode(AMTunerModeType Mode)
{
	if(Mode&GetAvailableModes())
	{
		HRESULT hr=m_pTVTuner->put_Mode(Mode);
		if(FAILED(hr))
		{
			throw CDShowException("CDShowDirectTuner::SetTunerMode Failed",hr);
		}
	}
	else
	{
		throw CDShowException("CDShowDirectTuner::SetTunerMode: Specified tuner mode is not supported");
	}
}

long CDShowDirectTuner::GetAvailableModes()
{
	long modes;
	HRESULT hr=m_pTVTuner->GetAvailableModes(&modes);
	if(FAILED(hr))
	{
		throw CDShowException("CDShowDirectTuner::GetAvailableModes Failed",hr);	
	}
	return modes;
}

long CDShowDirectTuner::GetFrequency()
{
	KSPROPERTY_TUNER_STATUS_S TunerStatus;
	memset(&TunerStatus,0,sizeof(KSPROPERTY_TUNER_STATUS_S));
	GetKSData(m_pKSProp,KSPROPERTY_TUNER_STATUS,TunerStatus);
	return TunerStatus.CurrentFrequency;
}

void CDShowDirectTuner::SetFrequency(long Freq,AMTunerModeType Mode,AnalogVideoStandard Format)
{
	SetTunerMode(Mode);
	
	//it doesn't matter if SetTVFormat fails
	try
	{
		SetTVFormat(Format);
	}
	catch(CDShowException e)
	{
		LOG(1,"CDShowDirectTuner::SetFrequency: Exception from SetTVFormat - %s",e.getErrorText());
	}
	
	KSPROPERTY_TUNER_MODE_CAPS_S ModeCaps;
	KSPROPERTY_TUNER_FREQUENCY_S Frequency;
	memset(&ModeCaps,0,sizeof(KSPROPERTY_TUNER_MODE_CAPS_S));
	memset(&Frequency,0,sizeof(KSPROPERTY_TUNER_FREQUENCY_S));
	ModeCaps.Mode=Mode;
	GetKSData(m_pKSProp,KSPROPERTY_TUNER_MODE_CAPS,ModeCaps);
	
	Frequency.Frequency=Freq;
	if(ModeCaps.Strategy==KS_TUNER_STRATEGY_DRIVER_TUNES)
	{
		Frequency.TuningFlags=KS_TUNER_TUNING_FINE;
	}
	else
	{
		Frequency.TuningFlags=KS_TUNER_TUNING_EXACT;
	}
	if(Freq>=ModeCaps.MinFrequency && Freq<=ModeCaps.MaxFrequency)
	{
		HRESULT hr=m_pKSProp->Set(PROPSETID_TUNER,
			KSPROPERTY_TUNER_FREQUENCY,
			INSTANCE_DATA_OF_PROPERTY_PTR(&Frequency),
			INSTANCE_DATA_OF_PROPERTY_SIZE(Frequency),
			&Frequency,
			sizeof(Frequency));
		if(FAILED(hr))
		{
			throw CDShowException("CDShowDirectTuner::SetFrequency failed to set frequency",hr);
		}
	}
	else
	{
		throw CDShowException("CDShowDirectTuner::SetFrequency: Frequency out of range");
	}
}

void CDShowDirectTuner::GetMinMaxFrequency(long &min,long &max,AMTunerModeType Mode)
{
	KSPROPERTY_TUNER_MODE_CAPS_S ModeCaps;
	memset(&ModeCaps,0,sizeof(KSPROPERTY_TUNER_MODE_CAPS_S));
	ModeCaps.Mode=Mode;
	GetKSData(m_pKSProp,KSPROPERTY_TUNER_MODE_CAPS,ModeCaps);
	min=ModeCaps.MinFrequency;
	max=ModeCaps.MaxFrequency;
}

long CDShowDirectTuner::GetSignalStrength(CDShowDirectTuner::eSignalType &type)
{
	AMTunerModeType Mode=GetTunerMode();

	KSPROPERTY_TUNER_MODE_CAPS_S ModeCaps;
	KSPROPERTY_TUNER_STATUS_S TunerStatus;
	memset(&ModeCaps,0,sizeof(KSPROPERTY_TUNER_MODE_CAPS_S));
	memset(&TunerStatus,0,sizeof(KSPROPERTY_TUNER_STATUS_S));
	try
	{
		ModeCaps.Mode=Mode;
		GetKSData(m_pKSProp,KSPROPERTY_TUNER_MODE_CAPS,ModeCaps);
	}
	catch(CDShowException e)
	{
		type=eSignalType::SIGNALTYPE_NONE;
		return 0;
	}

	if(ModeCaps.Strategy==KS_TUNER_STRATEGY_PLL)
	{
		type=eSignalType::SIGNALTYPE_PLL;
		GetKSData(m_pKSProp,KSPROPERTY_TUNER_STATUS,TunerStatus);
		return TunerStatus.PLLOffset;
	}
	else if(ModeCaps.Strategy==KS_TUNER_STRATEGY_SIGNAL_STRENGTH)
	{
		type=eSignalType::SIGNALTYPE_SIGNALSTRENGTH;
		GetKSData(m_pKSProp,KSPROPERTY_TUNER_STATUS,TunerStatus);
		return TunerStatus.SignalStrength;
	}
	else
	{
		type=eSignalType::SIGNALTYPE_NONE;
		return 0;
	}
}

long CDShowDirectTuner::GetInput()
{
	long index;
	HRESULT hr=m_pTVTuner->get_ConnectInput(&index);
	if(FAILED(hr))
	{
		throw CDShowException("CDShowDirectTuner::GetInput Failed",hr);
	}
	return index;
}

void CDShowDirectTuner::SetInput(long Input)
{
	HRESULT hr=m_pTVTuner->put_ConnectInput(Input);
	if(FAILED(hr))
	{
		throw CDShowException("CDShowDirectTuner::SetInput Failed",hr);
	}
}

long CDShowDirectTuner::GetNumInputs()
{
	long count;
	HRESULT hr=m_pTVTuner->get_NumInputConnections(&count);
	if(FAILED(hr))
	{
		throw CDShowException("CDShowDirectTuner::GetNumInputs Failed",hr);
	}
	return count;
}

AnalogVideoStandard CDShowDirectTuner::GetTVFormat()
{
	KSPROPERTY_TUNER_STANDARD_S Standard;
	memset(&Standard,0,sizeof(KSPROPERTY_TUNER_STANDARD_S));
	GetKSData(m_pKSProp,KSPROPERTY_TUNER_STANDARD,Standard);
	return (AnalogVideoStandard)Standard.Standard;
}

void CDShowDirectTuner::SetTVFormat(AnalogVideoStandard Format)
{
	KSPROPERTY_TUNER_STANDARD_S Standard;
	memset(&Standard,0,sizeof(KSPROPERTY_TUNER_STANDARD_S));
	Standard.Standard=Format;
	HRESULT hr=m_pKSProp->Set(PROPSETID_TUNER,
		KSPROPERTY_TUNER_STANDARD,
		INSTANCE_DATA_OF_PROPERTY_PTR(&Standard),
		INSTANCE_DATA_OF_PROPERTY_SIZE(Standard),
		&Standard,
		sizeof(Standard));
	if(FAILED(hr))
	{
		throw CDShowException("CDShowDirectTuner::PutTVFormat failed to set tvformat",hr);
	}
}

long CDShowDirectTuner::GetAvailableTVFormats()
{
	AMTunerModeType Mode=GetTunerMode();
	KSPROPERTY_TUNER_MODE_CAPS_S ModeCaps;
	memset(&ModeCaps,0,sizeof(KSPROPERTY_TUNER_MODE_CAPS_S));
	ModeCaps.Mode=Mode;

	GetKSData(m_pKSProp,KSPROPERTY_TUNER_MODE_CAPS,ModeCaps);
	return ModeCaps.StandardsSupported;
}
#endif