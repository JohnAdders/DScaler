//
// $Id: TDA9887.cpp,v 1.15 2004-11-28 06:46:25 atnak Exp $
//
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
// Revision 1.14  2004/11/27 19:07:43  atnak
// Changed constant to more correct name.
//
// Revision 1.13  2004/11/27 14:15:56  atnak
// Changed variable name to fix conflict.
//
// Revision 1.12  2004/11/27 01:14:25  atnak
// Fixed comments.
//
// Revision 1.11  2004/11/27 00:57:35  atnak
// Completed changes for CTDA9887FromIni (and renamed to CTDA9887Ex).
//
// Revision 1.10  2004/11/23 18:19:28  to_see
// Created new class CTDA9887FromIni
//
// Revision 1.9  2004/09/29 20:36:02  to_see
// Added Card AverTV303, Thanks to Zbigniew Pluta
//
// Revision 1.8  2004/09/11 20:19:55  to_see
// Renamed variable name from Pal_L to Secam_L (Sorry)
//
// Revision 1.7  2004/08/27 13:12:41  to_see
// Added audio support for Ati Tv Wonder Pro
//
// Revision 1.6  2004/06/23 20:15:22  to_see
// Created an new struct TControlSettings for better handling
// more cards and deleted class CTDA9887MsiMaster. Thanks to atnak.
//
// Revision 1.5  2004/06/02 18:41:34  to_see
// Added settings in CTDA9887MsiMaster for Pal(I) from John
//
// Revision 1.4  2004/05/16 19:45:08  to_see
// Added an new class for Msi Master Card
//
// Revision 1.3  2003/10/27 10:39:54  adcockj
// Updated files for better doxygen compatability
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file TDA9887.cpp CTDA9887 Implementation
 */

#include "stdafx.h"

#include "tda9887.h"
#include "DebugLog.h"

// \TODO: delete this
const CTDA9887::TControlSettings CTDA9887::m_ControlSettings[TDA9887_LASTONE] =
{
    {
        TDA9887_DEFAULT,
		// B ,   C,    E
		{0x96, 0x70, 0x49}, // PAL_BG
		{0x96, 0x70, 0x4a}, // PAL_I
		{0x96, 0x70, 0x4b}, // PAL_DK
		{0x86, 0x50, 0x4b}, // SECAM_L
		{0x96, 0x70, 0x44}, // NTSC
		{0x96, 0x70, 0x40}, // NTSC_JP
		{0x8e, 0x0d, 0x77}, // FM_RADIO
    },
    {
        TDA9887_MSI_TV_ANYWHERE_MASTER,
        {0x56, 0x70, 0x49}, // PAL_BG		Working
        {0x56, 0x6e, 0x4a}, // PAL_I		Working
        {0x56, 0x70, 0x4b}, // PAL_DK		Working
        {0x86, 0x50, 0x4b}, // SECAM_L		?
        {0x92, 0x30, 0x04}, // NTSC			Working
        {0x92, 0x30, 0x40}, // NTSC_JP		?
        {0x8e, 0x0d, 0x77}, // FM_RADIO		?
    },
	{
        TDA9887_LEADTEK_WINFAST_EXPERT,
        {0x16, 0x70, 0x49}, // PAL_BG		Working
        {0x16, 0x6e, 0x4a}, // PAL_I		?
        {0x16, 0x70, 0x4b}, // PAL_DK		Working
        {0x86, 0x50, 0x4b}, // SECAM_L		?
        {0x92, 0x30, 0x04}, // NTSC			?
        {0x92, 0x30, 0x40}, // NTSC_JP		?
        {0x8e, 0x70, 0x49}, // FM_RADIO		? must work
    },
	{
        TDA9887_ATI_TV_WONDER_PRO,
        {0x16, 0x70, 0x49}, // PAL_BG		?
        {0x16, 0x6e, 0x4a}, // PAL_I		?
        {0x16, 0x70, 0x4b}, // PAL_DK		?
        {0x86, 0x50, 0x4b}, // SECAM_L		?
        {0x92, 0x30, 0x04}, // NTSC			Working
        {0x92, 0x30, 0x40}, // NTSC_JP		?
        {0x8e, 0x0d, 0x77}, // FM_RADIO		?
    },
	{
        TDA9887_AVERTV_303,
        {0x16, 0x70, 0x49}, // PAL_BG		?
        {0x16, 0x6e, 0x4a}, // PAL_I		?
        {0x16, 0x70, 0x4b}, // PAL_DK		Working
        {0x86, 0x50, 0x4b}, // SECAM_L		?
        {0x92, 0x30, 0x04}, // NTSC			?
        {0x92, 0x30, 0x40}, // NTSC_JP		?
        {0x8e, 0x0d, 0x77}, // FM_RADIO		?
    },
};


/// TDA9885 (PAL, NTSC)
/// TDA9886 (PAL, SECAM, NTSC)
/// TDA9887 (PAL, SECAM, NTSC, FM Radio)

// 1.st constructor, default settings.
CTDA9887::CTDA9887()
{
	m_eCardID = TDA9887_DEFAULT;
}

// \TODO: delete this
// 2.nd constructor, use it with your own settings.
CTDA9887::CTDA9887(eTDA9887Card TDA9887Card)
{
	m_eCardID = TDA9887Card;
}

CTDA9887::~CTDA9887()
{
}


bool CTDA9887::DetectAttach(CI2CBus* i2cBus)
{
	BYTE tda9887Addresses[4] = { I2C_TDA9887_0, I2C_TDA9887_1,
								 I2C_TDA9887_2, I2C_TDA9887_3 };
	return DetectAttach(i2cBus, tda9887Addresses, 4);
}

bool CTDA9887::DetectAttach(CI2CBus* i2cBus, BYTE* addresses, size_t count)
{
	for (size_t i = 0; i < count; ++i)
	{
		if (addresses[i] != 0x00)
		{
			Attach(i2cBus, addresses[i]);

			if (Detect())
			{
				return true;
			}
		}
	}
	return false;
}

bool CTDA9887::Detect()
{
    BYTE tda9887set[5] = { m_DeviceAddress, 0x00, 0x54, 0x70, 0x44 };
	return m_I2CBus->Write(tda9887set, 5);
}

void CTDA9887::Init(bool bPreInit, eVideoFormat videoFormat)
{
    TunerSet(bPreInit, videoFormat);
}

void CTDA9887::TunerSet(bool bPreSet, eVideoFormat VideoFormat)
{
   static BYTE tda9887set[5] = {m_DeviceAddress};

   if (!bPreSet)
   {
       // only setup before tuning
       return;
   }

   switch (VideoFormat)
   {
	case VIDEOFORMAT_PAL_B:
    case VIDEOFORMAT_PAL_G:
    case VIDEOFORMAT_PAL_H:
    case VIDEOFORMAT_PAL_N:
	case VIDEOFORMAT_SECAM_B:
    case VIDEOFORMAT_SECAM_G:
    case VIDEOFORMAT_SECAM_H:
		memcpy(&tda9887set[2], &m_ControlSettings[m_eCardID].Pal_BG, 3);
		break;

	case VIDEOFORMAT_PAL_I:
		memcpy(&tda9887set[2], &m_ControlSettings[m_eCardID].Pal_I, 3);
		break;

	case VIDEOFORMAT_PAL_D:
	case VIDEOFORMAT_SECAM_D:
    case VIDEOFORMAT_SECAM_K:
    case VIDEOFORMAT_SECAM_K1:
		memcpy(&tda9887set[2], &m_ControlSettings[m_eCardID].Pal_DK, 3);
		break;

	case VIDEOFORMAT_SECAM_L:
    case VIDEOFORMAT_SECAM_L1:
		memcpy(&tda9887set[2], &m_ControlSettings[m_eCardID].Secam_L, 3);
		break;

    case VIDEOFORMAT_PAL_60:
		///\todo Video bandwidth of PAL-60?
		break;

	case VIDEOFORMAT_PAL_M:
	case VIDEOFORMAT_PAL_N_COMBO:
	case VIDEOFORMAT_NTSC_M:
		memcpy(&tda9887set[2], &m_ControlSettings[m_eCardID].Ntsc, 3);
		break;

    case VIDEOFORMAT_NTSC_50:
    case VIDEOFORMAT_NTSC_M_Japan:
		memcpy(&tda9887set[2], &m_ControlSettings[m_eCardID].Ntsc_Jp, 3);
		break;

	case (VIDEOFORMAT_LASTONE+1):
		//radio
		memcpy(&tda9887set[2], &m_ControlSettings[m_eCardID].Fm_Radio, 3);
		break;
   }

   if (tda9887set != NULL)
   {
      LOG(2,"TDA9885/6/7: 0x%02x 0x%02x 0x%02x", tda9887set[2],tda9887set[3],tda9887set[4]);
      m_I2CBus->Write(tda9887set, 5);
   }
}

eTunerAFCStatus CTDA9887::GetAFCStatus(long &nFreqDeviation)
{
    /**
    bit 0: power on reset
    bit 1-4: AFC
    bit 5: FM carrier detection
    bit 6: VIF input level
    bit 7: 1=VCO in +-1.6MHz AFC window
    */
    BYTE addr[] = { m_DeviceAddress };
    BYTE result;
    if (m_I2CBus->Read(addr, 1, &result, 1))
    {
        nFreqDeviation = ((result>>1) & 0xF);
        if ((nFreqDeviation == 0x7) || (nFreqDeviation == 0x8))
        {
            // <= -187.5 kHz or >= 187.5 hKz
            //return TUNER_AFC_NOCARRIER;
        }
        if (nFreqDeviation&8)
        {
            nFreqDeviation |= (-8);
        }

        nFreqDeviation = -1 * ((nFreqDeviation *  25000) + 12500);
        return TUNER_AFC_CARRIER;
    }
    return TUNER_AFC_NOTSUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

CTDA9887Pinnacle::CTDA9887Pinnacle(int CardId) : CTDA9887()
{
    m_CardId = CardId;
    m_LastVideoFormat = (eVideoFormat)-1;
}

CTDA9887Pinnacle::~CTDA9887Pinnacle()
{
}


void CTDA9887Pinnacle::Init(bool bPreInit, eVideoFormat videoFormat)
{
    TunerSet(bPreInit, videoFormat);
}

void CTDA9887Pinnacle::TunerSet(bool bPreSet, eVideoFormat videoFormat)
{
    static BYTE bData[5];

    if (videoFormat != m_LastVideoFormat)
    {
        m_LastVideoFormat = videoFormat;

        BYTE   bVideoIF     = 0;
        BYTE   bAudioIF     = 0;
        BYTE   bDeEmphasis  = 0;
        BYTE   bDeEmphVal   = 0;
        BYTE   bModulation  = 0;
        BYTE   bCarrierMode = 0;
        BYTE   bOutPort1    = TDA9887_OutputPort1Inactive;
        BYTE   bOutPort2    = 0; //TDA9887_OutputPort2Inactive
        BYTE   bVideoTrap   = TDA9887_VideoTrapBypassOFF;
        // Noise fix supplied by anonymous
        BYTE   bTopAdjust   = 0x0e; //mbAGC;

        //if (mParams.fVideoTrap)
        //{
        //	bVideoTrap   = cVideoTrapBypassON;
        //}

        if (IsPALVideoFormat(videoFormat))
        {
            bDeEmphasis  = TDA9887_DeemphasisON;
	        bDeEmphVal   = TDA9887_Deemphasis50;
	        bModulation  = TDA9887_NegativeFmTV;
	        if (m_CardId == 1)
            {
		        bCarrierMode = TDA9887_Intercarrier;
	        }
            else  // stereo boards
            {
		        bCarrierMode = TDA9887_QSS;
	        }
            bOutPort1    = TDA9887_OutputPort1Inactive;

            if ((videoFormat == VIDEOFORMAT_PAL_M) || (videoFormat == VIDEOFORMAT_PAL_N))
            {
                bDeEmphVal = TDA9887_Deemphasis75;
                if ((m_CardId == 5) || (m_CardId == 6))
                {
			        bCarrierMode = TDA9887_Intercarrier;
		        }
                else  // stereo boards
                {
			        bCarrierMode = TDA9887_QSS;
		        }
            }
        }
        else if (IsSECAMVideoFormat(videoFormat))
        {
	        bDeEmphasis  = TDA9887_DeemphasisON;
	        bDeEmphVal   = TDA9887_Deemphasis50;
	        bModulation  = TDA9887_NegativeFmTV;
	        bCarrierMode = TDA9887_QSS;
	        bOutPort1    = TDA9887_OutputPort1Inactive;

            if ((videoFormat == VIDEOFORMAT_SECAM_L) || (videoFormat == VIDEOFORMAT_SECAM_L1))
            {
                bDeEmphasis  = TDA9887_DeemphasisOFF;
		        bDeEmphVal   = TDA9887_Deemphasis75;
		        bModulation  = TDA9887_PositiveAmTV;
            }
        }
        else if (IsNTSCVideoFormat(videoFormat))
        {
	        bDeEmphasis  = TDA9887_DeemphasisON;
            bDeEmphVal   = TDA9887_Deemphasis75;
            bModulation  = TDA9887_NegativeFmTV;
            if ((m_CardId == 5) || (m_CardId == 6))
            {
		        bCarrierMode = TDA9887_Intercarrier;
	        }
            else
            {
		        bCarrierMode = TDA9887_QSS;
            }
            bOutPort1    = TDA9887_OutputPort1Inactive;
        }
        else if (videoFormat == (VIDEOFORMAT_LASTONE+1)) //FM Radio
        {
	        bDeEmphasis  = TDA9887_DeemphasisON;
            if (m_CardId == 3)
            {
		        bDeEmphVal = TDA9887_Deemphasis75;
	        }
            else
            {
		        bDeEmphVal = TDA9887_Deemphasis50;
            }
            bModulation  = TDA9887_FmRadio;
            bCarrierMode = TDA9887_QSS;
	        bOutPort1    = TDA9887_OutputPort1Inactive;

            bVideoTrap   = TDA9887_VideoTrapBypassOFF;
        }

        //Set bandwidths

        bVideoIF     = TDA9887_VideoIF_38_90;
        bAudioIF     = TDA9887_AudioIF_5_5;
        switch (videoFormat)
        {
        case VIDEOFORMAT_PAL_B:
        case VIDEOFORMAT_PAL_G:
        case VIDEOFORMAT_PAL_H:
            bVideoIF     = TDA9887_VideoIF_38_90;
	        bAudioIF     = TDA9887_AudioIF_5_5;
            break;
        case VIDEOFORMAT_PAL_D:
            bVideoIF     = TDA9887_VideoIF_38_00;
	        bAudioIF     = TDA9887_AudioIF_6_5;
            break;
        case VIDEOFORMAT_PAL_I:
	        bVideoIF     = TDA9887_VideoIF_38_90;
	        bAudioIF     = TDA9887_AudioIF_6_0;
	        break;
        case VIDEOFORMAT_PAL_M:
        case VIDEOFORMAT_PAL_N:
        case VIDEOFORMAT_PAL_N_COMBO:   //Not sure about PAL-NC
            bVideoIF     = TDA9887_VideoIF_45_75;
	        bAudioIF     = TDA9887_AudioIF_4_5;

        case VIDEOFORMAT_SECAM_D:
            bVideoIF     = TDA9887_VideoIF_38_00;
            bAudioIF     = TDA9887_AudioIF_6_5;
            break;
        case VIDEOFORMAT_SECAM_K:
        case VIDEOFORMAT_SECAM_K1:
            bVideoIF     = TDA9887_VideoIF_38_90;
            bAudioIF     = TDA9887_AudioIF_6_5;
            break;
        case VIDEOFORMAT_SECAM_L:
            bVideoIF     = TDA9887_VideoIF_38_90;
            bAudioIF     = TDA9887_AudioIF_6_5;
            break;
        case VIDEOFORMAT_SECAM_L1:
            bVideoIF     = TDA9887_VideoIF_33_90;
            bAudioIF     = TDA9887_AudioIF_6_5;
            break;

        case VIDEOFORMAT_NTSC_M:
        case VIDEOFORMAT_NTSC_M_Japan:
            bVideoIF     = TDA9887_VideoIF_45_75;
            bAudioIF     = TDA9887_AudioIF_4_5;
	        break;

        case (VIDEOFORMAT_LASTONE+1): // FM Radio
	        if (m_CardId == 3)
            {
		        // NTSC
		        bVideoIF     = TDA9887_RadioIF_45_75;
                bAudioIF     = TDA9887_AudioIF_4_5;
	        }
            else
            {
		        // PAL
		        bVideoIF     = TDA9887_RadioIF_38_90;
                bAudioIF     = TDA9887_AudioIF_5_5;
	        }
	        break;

        case VIDEOFORMAT_NTSC_50:
            ///\todo check
            bVideoIF     = TDA9887_VideoIF_38_90;
	        bAudioIF     = TDA9887_AudioIF_5_5;

        case VIDEOFORMAT_PAL_60:
            ///\todo check
            bVideoIF     = TDA9887_VideoIF_45_75;
            bAudioIF     = TDA9887_AudioIF_5_5;
            break;

        }

        bData[0] =  m_DeviceAddress;
        bData[1] =  0;

        bData[2] =  bVideoTrap                   |  // B0: video trap bypass
		            TDA9887_AutoMuteFmInactive   |  // B1: auto mute
	                bCarrierMode                 |  // B2: InterCarrier for PAL else QSS
	    	        bModulation                  |  // B3 - B4: positive AM TV for SECAM only
	    	        TDA9887_ForcedMuteAudioOFF   |  // B5: forced Audio Mute (off)
		            bOutPort1                    |  // B6: Out Port 1
		            bOutPort2;                      // B7: Out Port 2
        bData[3] =  bTopAdjust                   |  // C0 - C4: Top Adjust 0 == -16dB  31 == 15dB
	    	        bDeEmphasis                  |  // C5: De-emphasis on/off
	    	        bDeEmphVal                   |  // C6: De-emphasis 50/75 microsec
	    	        TDA9887_AudioGain0;             // C7: normal audio gain
        bData[4] =  bAudioIF                     |  // E0 - E1: Sound IF
		            bVideoIF                     |  // E2 - E4: Video IF
		            TDA9887_TunerGainNormal      |  // E5: Tuner gain (normal)
		            TDA9887_Gating_18            |  // E6: Gating (18%)
		            TDA9887_AgcOutOFF;              // E7: VAGC  (off)
    }

    if (bPreSet)
    {
        bData[2] |= TDA9887_OutputPort2Inactive;
    }
    else
    {
        bData[2] &= ~TDA9887_OutputPort2Inactive;
    }

	LOG(2,"TDA9885/6/7 Pinnacle: 0x%02x 0x%02x 0x%02x", bData[2],bData[3],bData[4]);

	m_I2CBus->Write(bData, 5);
}


//////////////////////////////////////////////////////////////////////////
// CTDA9887Ex
//////////////////////////////////////////////////////////////////////////

const CTDA9887Ex::TTDABytes CTDA9887Ex::k_TDAStandardtSettings[TDA9887_FORMAT_LASTONE] =
{
	{
		// TDA9887_FORMAT_PAL_BG
		TDA9887_NegativeFmTV | TDA9887_QSS,
		TDA9887_DeemphasisON | TDA9887_Deemphasis50 | TDA9887_TakeOverPointDefault,
		TDA9887_AudioIF_5_5  | TDA9887_VideoIF_38_90,
	},
	{
		// TDA9887_FORMAT_PAL_I
		TDA9887_NegativeFmTV | TDA9887_QSS,
		TDA9887_DeemphasisON | TDA9887_Deemphasis50 | TDA9887_TakeOverPointDefault,
		TDA9887_AudioIF_6_0  | TDA9887_VideoIF_38_90,
	},
	{
		// TDA9887_FORMAT_PAL_DK
		TDA9887_NegativeFmTV | TDA9887_QSS,
		TDA9887_DeemphasisON | TDA9887_Deemphasis50 | TDA9887_TakeOverPointDefault,
		TDA9887_AudioIF_6_5  | TDA9887_VideoIF_38_00,
	},
	{
		// TDA9887_FORMAT_PAL_MN
		TDA9887_NegativeFmTV | TDA9887_QSS,
		TDA9887_DeemphasisON | TDA9887_Deemphasis75 | TDA9887_TakeOverPointDefault,
		TDA9887_AudioIF_4_5  | TDA9887_VideoIF_45_75,
	},
	{
		// TDA9887_FORMAT_SECAM_L
		TDA9887_PositiveAmTV | TDA9887_QSS,
		TDA9887_TakeOverPointDefault,
		TDA9887_AudioIF_6_5  | TDA9887_VideoIF_38_90,
	},
	{
		// TDA9887_FORMAT_SECAM_DK
		TDA9887_NegativeFmTV | TDA9887_QSS,
		TDA9887_DeemphasisON | TDA9887_Deemphasis50 | TDA9887_TakeOverPointDefault,
		TDA9887_AudioIF_6_5  | TDA9887_VideoIF_38_00,
	},
	{
		// TDA9887_FORMAT_NTSC_M
		TDA9887_NegativeFmTV | TDA9887_QSS,
		TDA9887_DeemphasisON | TDA9887_Deemphasis50  | TDA9887_TakeOverPointDefault,
		TDA9887_AudioIF_4_5  | TDA9887_VideoIF_45_75 | TDA9887_Gating_36,
	},
	{
		// TDA9887_FORMAT_NTSC_JP
		TDA9887_NegativeFmTV | TDA9887_QSS,
		TDA9887_DeemphasisON | TDA9887_Deemphasis50  | TDA9887_TakeOverPointDefault,
		TDA9887_AudioIF_4_5  | TDA9887_VideoIF_58_75 | TDA9887_Gating_36,
	},
	{
		// TDA9887_FORMAT_RADIO
		TDA9887_FmRadio      | TDA9887_QSS,
		TDA9887_DeemphasisON | TDA9887_Deemphasis75 | TDA9887_TakeOverPointDefault,
		TDA9887_AudioIF_5_5  | TDA9887_RadioIF_38_90,
	},
};

CTDA9887Ex::CTDA9887Ex()
{
	memcpy(m_TDASettings, k_TDAStandardtSettings, sizeof(m_TDASettings));
}

CTDA9887Ex::~CTDA9887Ex()
{
}

void CTDA9887Ex::TunerSet(IN bool bPreSet, IN eVideoFormat format)
{
	eTDA9887Format tdaFormat = VideoFormat2TDA9887Format(format);
	if (tdaFormat == TDA9887_FORMAT_NONE)
	{
		LOG(2,"CTDA9887Ex: Unsupported video format: %u", format);
	}
	else
	{
		TunerSet(bPreSet, tdaFormat);
	}
}

void CTDA9887Ex::TunerSet(IN bool bPreSet, IN eTDA9887Format format)
{
	if (!bPreSet || format == TDA9887_FORMAT_NONE)
	{
		// Only setup before tuning (bPreSet).
		return;
	}

	BYTE writeBytes[5] = { m_DeviceAddress, 0x00 };

	writeBytes[2] = m_TDASettings[format].b;
	writeBytes[3] = m_TDASettings[format].c;
	writeBytes[4] = m_TDASettings[format].e;

	LOG(2,"CTDA9887Ex: 0x%02x 0x%02x 0x%02x", writeBytes[2], writeBytes[3], writeBytes[4]);
	m_I2CBus->Write(writeBytes, 5);
}

void CTDA9887Ex::SetModes(IN eTDA9887Format format, IN BYTE mask, IN BYTE bits)
{
	if (format == TDA9887_FORMAT_NONE)
	{
		return;
	}

	// Override demodulation
	if (mask & TDA9887_SM_CARRIER_QSS)
	{
		SetBit(m_TDASettings[format].b,
			TDA9887_QSS, (bits & TDA9887_SM_CARRIER_QSS) != 0);
	}
	// Override OutputPort1
	if (mask & TDA9887_SM_OUTPUTPORT1_INACTIVE)
	{
		SetBit(m_TDASettings[format].b,
			TDA9887_OutputPort1Inactive, (bits & TDA9887_SM_OUTPUTPORT1_INACTIVE) != 0);
	}
	// Override OutputPort2
	if (mask & TDA9887_SM_OUTPUTPORT2_INACTIVE)
	{
		SetBit(m_TDASettings[format].b,
			TDA9887_OutputPort2Inactive, (bits & TDA9887_SM_OUTPUTPORT2_INACTIVE) != 0);
	}
	// Override TakeOverPoint
	if (mask & TDA9887_SM_TAKEOVERPOINT_MASK)
	{
		m_TDASettings[format].c &= ~TDA9887_TakeOverPointMax;
		m_TDASettings[format].c |=
			(bits & TDA9887_SM_TAKEOVERPOINT_MASK) >> TDA9887_SM_TAKEOVERPOINT_OFFSET;
	}
}

void CTDA9887Ex::SetModes(IN eTDA9887Format format, IN TTDA9887Modes* modes)
{
	SetModes(format, modes->mask, modes->bits);
}

void CTDA9887Ex::SetModes(IN TTDA9887FormatModes* modes)
{
	SetModes(modes->format, modes->mask, modes->bits);
}

eTDA9887Format CTDA9887Ex::VideoFormat2TDA9887Format(IN eVideoFormat format)
{
	switch (format)
	{
		case VIDEOFORMAT_PAL_B:
		case VIDEOFORMAT_PAL_G:
			return TDA9887_FORMAT_PAL_BG;
		case VIDEOFORMAT_PAL_I:
			return TDA9887_FORMAT_PAL_I;
		case VIDEOFORMAT_PAL_D:
			return TDA9887_FORMAT_PAL_DK;
		case VIDEOFORMAT_PAL_M:
		case VIDEOFORMAT_PAL_N:
		case VIDEOFORMAT_PAL_N_COMBO:
			return TDA9887_FORMAT_PAL_MN;
		case VIDEOFORMAT_SECAM_L:
		case VIDEOFORMAT_SECAM_L1:
			return TDA9887_FORMAT_SECAM_L;
		case VIDEOFORMAT_SECAM_D:
		case VIDEOFORMAT_SECAM_K:
		case VIDEOFORMAT_SECAM_K1:
			return TDA9887_FORMAT_SECAM_DK;
		case VIDEOFORMAT_NTSC_M:
			return TDA9887_FORMAT_NTSC_M;
		case VIDEOFORMAT_NTSC_50:
		case VIDEOFORMAT_NTSC_M_Japan:
			return TDA9887_FORMAT_NTSC_JP;
		case (VIDEOFORMAT_LASTONE+1):
			return TDA9887_FORMAT_RADIO;
		// I'm not sure about the following.
		case VIDEOFORMAT_PAL_H:
		case VIDEOFORMAT_SECAM_B:
		case VIDEOFORMAT_SECAM_G:
		case VIDEOFORMAT_SECAM_H:
			return TDA9887_FORMAT_PAL_BG;
		case VIDEOFORMAT_PAL_60:
			return TDA9887_FORMAT_NTSC_M;
	}
	// NEVER_GET_HERE;
	return TDA9887_FORMAT_NONE;
}

void CTDA9887Ex::SetBit(IN OUT BYTE& bits, IN BYTE bit, IN bool set)
{
	if (set)
	{
		bits |= bit;
	}
	else
	{
		bits &= ~bit;
	}
}

