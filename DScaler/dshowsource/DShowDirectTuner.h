/////////////////////////////////////////////////////////////////////////////
// $Id: DShowDirectTuner.h,v 1.1 2002-10-29 19:30:43 tobbej Exp $
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
 * @file DShowDirectTuner.h interface for the CDShowDirectTuner class.
 */

#if !defined(AFX_DSHOWDIRECTTUNER_H__BCD904DA_6DA2_47DF_A759_09ABA502617F__INCLUDED_)
#define AFX_DSHOWDIRECTTUNER_H__BCD904DA_6DA2_47DF_A759_09ABA502617F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DSObject.h"

/**
 * DirectShow tuner class.
 * This class uses both the normal directshow IAMTVTuner and
 * also IKsPropertySet to be able to tune directly to a frequency.
 */
class CDShowDirectTuner : public CDShowObject  
{
public:
	///enum used by GetSignalStrength
	enum eSignalType
	{
		SIGNALTYPE_NONE,
		SIGNALTYPE_PLL,
		SIGNALTYPE_SIGNALSTRENGTH,
	};
	
	CDShowDirectTuner(CComPtr<IAMTVTuner> pTVTuner, IGraphBuilder *pGraph);
	virtual ~CDShowDirectTuner();
	eDSObjectType getObjectType() { return DSHOW_TYPE_TUNER;};
	
	///@return current tuner mode
	AMTunerModeType GetTunerMode();
	///change tuner mode (tv, fm/am radio)
	void SetTunerMode(AMTunerModeType Mode);
	///@return a combination of AMTunerModeType
	long GetAvailableModes();
	
	///@return current frequency in hz
	long GetFrequency();
	/**
	 * Sets tuner frequency.
	 * @param Freq frequency
	 * @param Mode Tuner mode to use (tv, fm/am radio)
	 * @param Format Video format to use
	 */
	void SetFrequency(long Freq,AMTunerModeType Mode,AnalogVideoStandard Format);
	/**
	 * Retrieves minumum and maximum allowed frequency for specified tuner mode.
	 * @param min returns minimum allowed frequency
	 * @param max returns maximum allowed frequency
	 * @param Mode Tuner mode
	 */
	void GetMinMaxFrequency(long &min,long &max,AMTunerModeType Mode=AMTUNER_MODE_TV);
	
	/**
	 * Gets current signal strength.
	 * The type of signal depends on the tuning strategy that the device 
	 * driver uses.
	 * If strategy is KS_TUNER_STRATEGY_DRIVER_TUNES then there is no way of 
	 * geting the signal status in that case IAMAnalogVideoDecoder::get_HorizontalLocked
	 * coud be used insted.
	 *
	 * @param type type of signal
	 * @return current signal, meaning of this value depends on the type parameter
	 */
	long GetSignalStrength(eSignalType &type);
	
	///@return currently selected tuner input
	long GetInput();
	///@param Input new tuner input to use
	void SetInput(long Input);
	///@return number of Tuner inputs
	long GetNumInputs();
	
	///@return current tvformat
	AnalogVideoStandard GetTVFormat();
	///@param Format new tvformat
	void SetTVFormat(AnalogVideoStandard Format);
	///@return a combination of AnalogVideoStandard
	long GetAvailableTVFormats();

private:

	CComPtr<IAMTVTuner> m_pTVTuner;
	CComPtr<IKsPropertySet> m_pKSProp;

};

#endif // !defined(AFX_DSHOWDIRECTTUNER_H__BCD904DA_6DA2_47DF_A759_09ABA502617F__INCLUDED_)
