/////////////////////////////////////////////////////////////////////////////
// $Id: TDA9887.h,v 1.10 2004-11-23 20:24:18 to_see Exp $
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 John Adcock.  All rights reserved.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: not supported by cvs2svn $
// Revision 1.9  2004/11/23 18:19:29  to_see
// Created new class CTDA9887FromIni
//
// Revision 1.8  2004/09/29 20:36:02  to_see
// Added Card AverTV303, Thanks to Zbigniew Pluta
//
// Revision 1.7  2004/09/11 20:19:55  to_see
// Renamed variable name from Pal_L to Secam_L (Sorry)
//
// Revision 1.6  2004/08/27 13:12:41  to_see
// Added audio support for Ati Tv Wonder Pro
//
// Revision 1.5  2004/06/23 20:15:22  to_see
// Created an new struct TControlSettings for better handling
// more cards and deleted class CTDA9887MsiMaster. Thanks to atnak.
//
// Revision 1.4  2004/05/16 19:45:08  to_see
// Added an new class for Msi Master Card
//
// Revision 1.3  2004/02/11 20:34:00  adcockj
// Support multiple locations of TDA9887 (thanks to Pityu)
//
// Revision 1.2  2003/10/27 10:39:54  adcockj
// Updated files for better doxygen compatability
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file TDA9887.h CTDA9887 Header
 */

#ifndef _TDA_9887_H_
#define _TDA_9887_H_

#include "TVFormats.h"
#include "I2CDevice.h"
#include "ITuner.h"

//
// TDA defines
//

#define I2C_TDA9887_0					0x86
#define I2C_TDA9887_1					0x96

//// first reg
#define TDA9887_VideoTrapBypassOFF		0x00	// bit b0
#define TDA9887_VideoTrapBypassON		0x01	// bit b0

#define TDA9887_AutoMuteFmInactive		0x00	// bit b1
#define TDA9887_AutoMuteFmActive		0x02	// bit b1

#define TDA9887_Intercarrier			0x00	// bit b2
#define TDA9887_QSS						0x04	// bit b2

#define TDA9887_PositiveAmTV			0x00	// bit b3:4
#define TDA9887_FmRadio					0x08	// bit b3:4
#define TDA9887_NegativeFmTV			0x10	// bit b3:4

#define TDA9887_ForcedMuteAudioON		0x20	// bit b5
#define TDA9887_ForcedMuteAudioOFF		0x00	// bit b5

#define TDA9887_OutputPort1Active		0x00	// bit b6
#define TDA9887_OutputPort1Inactive		0x40	// bit b6
#define TDA9887_OutputPort2Active		0x00	// bit b7
#define TDA9887_OutputPort2Inactive		0x80	// bit b7

//// second reg
#define TDA9887_TakeOverPointMin		0x00	// bit c0:4
#define TDA9887_TakeOverPointDefault	0x10	// bit c0:4
#define TDA9887_TakeOverPointMax		0x1f	// bit c0:4
#define TDA9887_DeemphasisOFF			0x00	// bit c5
#define TDA9887_DeemphasisON			0x20	// bit c5
#define TDA9887_Deemphasis75			0x00	// bit c6
#define TDA9887_Deemphasis50			0x40	// bit c6
#define TDA9887_AudioGain0				0x00	// bit c7
#define TDA9887_AudioGain6				0x80	// bit c7

//// third reg
#define TDA9887_AudioIF_4_5				0x00	// bit e0:1
#define TDA9887_AudioIF_5_5				0x01	// bit e0:1
#define TDA9887_AudioIF_6_0				0x02	// bit e0:1
#define TDA9887_AudioIF_6_5				0x03	// bit e0:1

#define TDA9887_VideoIF_58_75			0x00	// bit e2:4
#define TDA9887_VideoIF_45_75			0x04	// bit e2:4
#define TDA9887_VideoIF_38_90			0x08	// bit e2:4
#define TDA9887_VideoIF_38_00			0x0C	// bit e2:4
#define TDA9887_VideoIF_33_90			0x10	// bit e2:4
#define TDA9887_VideoIF_33_40			0x14	// bit e2:4
#define TDA9887_RadioIF_45_75			0x18	// bit e2:4
#define TDA9887_RadioIF_38_90			0x1C	// bit e2:4

#define TDA9887_TunerGainNormal			0x00	// bit e5
#define TDA9887_TunerGainLow			0x20	// bit e5

#define TDA9887_Gating_18				0x00	// bit e6
#define TDA9887_Gating_36				0x40	// bit e6

#define TDA9887_AgcOutON				0x80	// bit e7
#define TDA9887_AgcOutOFF				0x00	// bit e7


// \TODO: delete this
enum eTDA9887Card
{
	TDA9887_DEFAULT = 0,
	TDA9887_MSI_TV_ANYWHERE_MASTER,
	TDA9887_LEADTEK_WINFAST_EXPERT,
	TDA9887_ATI_TV_WONDER_PRO,
	TDA9887_AVERTV_303,
	TDA9887_LASTONE,
};

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

class CTDA9887 : public IExternalIFDemodulator
{
public:
    CTDA9887();
	CTDA9887(eTDA9887Card TDA9887Card);
    ~CTDA9887();

    bool Detect();

    void Init(bool bPreInit, eVideoFormat videoFormat);
    void TunerSet(bool bPreSet, eVideoFormat videoFormat);
    
    eTunerAFCStatus GetAFCStatus(long &nFreqDeviation);
protected:
    // from CI2CDevice
    virtual BYTE GetDefaultAddress() const { return I2C_TDA9887_0; }

private:
	
	// \TODO: delete this
	typedef struct
	{
		DWORD eTDA9887Card;
		BYTE Pal_BG[3];
		BYTE Pal_I[3];
		BYTE Pal_DK[3];
		BYTE Secam_L[3];
		BYTE Ntsc[3];
		BYTE Ntsc_Jp[3];
		BYTE Fm_Radio[3];

	} TControlSettings;

    static const TControlSettings m_ControlSettings[TDA9887_LASTONE];
	eTDA9887Card m_eCardID;
};

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

class CTDA9887Pinnacle : public CTDA9887
{
public:
    CTDA9887Pinnacle(int CardId);
    ~CTDA9887Pinnacle();

    void Init(bool bPreInit, eVideoFormat videoFormat);
    void TunerSet(bool bPreSet, eVideoFormat videoFormat);    
private:
    int m_CardId;
    eVideoFormat m_LastVideoFormat;
};

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

class CTDA9887FromIni : public IExternalIFDemodulator
{
public:
    CTDA9887FromIni();
    ~CTDA9887FromIni();

    bool Detect();

    void Init(bool bPreInit, eVideoFormat videoFormat);
    void TunerSet(bool bPreSet, eVideoFormat videoFormat);
	void SetCardSpecific(eVideoFormat videoFormat, BYTE B, BYTE C = TDA9887_TakeOverPointDefault);   
    eTunerAFCStatus GetAFCStatus(long &nFreqDeviation);

protected:
    // from CI2CDevice
    virtual BYTE GetDefaultAddress() const { return I2C_TDA9887_0; }

private:
	typedef struct
    {
		BYTE B;
		BYTE C;
		BYTE E;
    } TDABytes;

	typedef struct
    {
		TDABytes Pal_BG;
		TDABytes Pal_I;
		TDABytes Pal_DK;
		TDABytes Pal_MN;
		TDABytes Secam_L;
		TDABytes Secam_DK;
		TDABytes Ntsc_M;
		TDABytes Ntsc_JP;
		TDABytes Fm_Radio;
    } TVStandards;

    static TVStandards m_TVStandards; // not constant

private:
	TDABytes* GetTDABytesFromVideoFormat(eVideoFormat VideoFormat);
};

#endif
