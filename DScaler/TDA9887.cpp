//
// $Id: TDA9887.cpp,v 1.4 2004-05-16 19:45:08 to_see Exp $
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



/// TDA9885 (PAL, NTSC)
/// TDA9886 (PAL, SECAM, NTSC)
/// TDA9887 (PAL, SECAM, NTSC, FM Radio)

CTDA9887::CTDA9887()
{
}

CTDA9887::~CTDA9887()
{    
}

bool CTDA9887::Detect()
{
    BYTE tda9887set[] = {m_DeviceAddress, 0x00, 0x54, 0x70, 0x44};
	if (m_I2CBus->Write(tda9887set,5))
	{
		return true;
	}
    return false;
}

void CTDA9887::Init(bool bPreInit, eVideoFormat videoFormat)
{
    TunerSet(bPreInit, videoFormat);
}

void CTDA9887::TunerSet(bool bPreSet, eVideoFormat VideoFormat)
{
   static BYTE tda9887set_pal_bg[] =   {m_DeviceAddress, 0x00, 0x96, 0x70, 0x49};
   static BYTE tda9887set_pal_i[] =    {m_DeviceAddress, 0x00, 0x96, 0x70, 0x4a};
   static BYTE tda9887set_pal_dk[] =   {m_DeviceAddress, 0x00, 0x96, 0x70, 0x4b};
   static BYTE tda9887set_pal_l[] =    {m_DeviceAddress, 0x00, 0x86, 0x50, 0x4b};
   static BYTE tda9887set_ntsc[] =     {m_DeviceAddress, 0x00, 0x96, 0x70, 0x44};
   static BYTE tda9887set_ntsc_jp[] =  {m_DeviceAddress, 0x00, 0x96, 0x70, 0x40};
   static BYTE tda9887set_fm_radio[] = {m_DeviceAddress, 0x00, 0x8e, 0x0d, 0x77};
   
   if (!bPreSet)
   {
       // only setup before tuning
       return;
   }
   
   BYTE *tda9887set = tda9887set_pal_bg;

   switch (VideoFormat)
   {
	case VIDEOFORMAT_PAL_B:    
    case VIDEOFORMAT_PAL_G:
    case VIDEOFORMAT_PAL_H:        
    case VIDEOFORMAT_PAL_N:
	case VIDEOFORMAT_SECAM_B:
    case VIDEOFORMAT_SECAM_G:
    case VIDEOFORMAT_SECAM_H:
		tda9887set = tda9887set_pal_bg;
		break;

	case VIDEOFORMAT_PAL_I:
		tda9887set = tda9887set_pal_i;
		break;

	case VIDEOFORMAT_PAL_D:
	case VIDEOFORMAT_SECAM_D:	
    case VIDEOFORMAT_SECAM_K:
    case VIDEOFORMAT_SECAM_K1:
		tda9887set = tda9887set_pal_dk;
		break;
	
	case VIDEOFORMAT_SECAM_L:
    case VIDEOFORMAT_SECAM_L1:
		tda9887set = tda9887set_pal_l;
		break;

    case VIDEOFORMAT_PAL_60:    
		///\todo Video bandwidth of PAL-60?
		break;

	case VIDEOFORMAT_PAL_M:
	case VIDEOFORMAT_PAL_N_COMBO:
	case VIDEOFORMAT_NTSC_M:        
		tda9887set = tda9887set_ntsc;
		break;

    case VIDEOFORMAT_NTSC_50:
    case VIDEOFORMAT_NTSC_M_Japan:
        tda9887set = tda9887set_ntsc_jp;
		break;

	case (VIDEOFORMAT_LASTONE+1):
		//radio
		tda9887set = tda9887set_fm_radio;
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
///////////////////////////////////////////////////////////////////////////////////

//
// TDA defines
//

//// first reg
#define TDA9887_VideoTrapBypassOFF     0x00    // bit b0
#define TDA9887_VideoTrapBypassON      0x01    // bit b0

#define TDA9887_AutoMuteFmInactive     0x00    // bit b1
#define TDA9887_AutoMuteFmActive       0x02    // bit b1

#define TDA9887_Intercarrier           0x00    // bit b2
#define TDA9887_QSS                    0x04    // bit b2

#define TDA9887_PositiveAmTV           0x00    // bit b3:4
#define TDA9887_FmRadio                0x08    // bit b3:4
#define TDA9887_NegativeFmTV           0x10    // bit b3:4

#define TDA9887_ForcedMuteAudioON      0x20    // bit b5
#define TDA9887_ForcedMuteAudioOFF     0x00    // bit b5

#define TDA9887_OutputPort1Active      0x00    // bit b6
#define TDA9887_OutputPort1Inactive    0x40    // bit b6
#define TDA9887_OutputPort2Active      0x00    // bit b7
#define TDA9887_OutputPort2Inactive    0x80    // bit b7


//// second reg
#define TDA9887_DeemphasisOFF          0x00    // bit c5
#define TDA9887_DeemphasisON           0x20    // bit c5
#define TDA9887_Deemphasis75           0x00    // bit c6
#define TDA9887_Deemphasis50           0x40    // bit c6
#define TDA9887_AudioGain0             0x00    // bit c7
#define TDA9887_AudioGain6             0x80    // bit c7


//// third reg
#define TDA9887_AudioIF_4_5             0x00    // bit e0:1
#define TDA9887_AudioIF_5_5             0x01    // bit e0:1
#define TDA9887_AudioIF_6_0             0x02    // bit e0:1
#define TDA9887_AudioIF_6_5             0x03    // bit e0:1


#define TDA9887_VideoIF_58_75           0x00    // bit e2:4
#define TDA9887_VideoIF_45_75           0x04    // bit e2:4
#define TDA9887_VideoIF_38_90           0x08    // bit e2:4
#define TDA9887_VideoIF_38_00           0x0C    // bit e2:4
#define TDA9887_VideoIF_33_90           0x10    // bit e2:4
#define TDA9887_VideoIF_33_40           0x14    // bit e2:4
#define TDA9887_RadioIF_45_75           0x18    // bit e2:4
#define TDA9887_RadioIF_38_90           0x1C    // bit e2:4


#define TDA9887_TunerGainNormal         0x00    // bit e5
#define TDA9887_TunerGainLow            0x20    // bit e5

#define TDA9887_Gating_18               0x00    // bit e6
#define TDA9887_Gating_36               0x40    // bit e6

#define TDA9887_AgcOutON                0x80    // bit e7
#define TDA9887_AgcOutOFF               0x00    // bit e7



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

CTDA9887MsiMaster::CTDA9887MsiMaster()
{
}

CTDA9887MsiMaster::~CTDA9887MsiMaster()
{
}

void CTDA9887MsiMaster::TunerSet(bool bPreSet, eVideoFormat VideoFormat)
{
   static BYTE tda9887set_pal_bg[] =   {m_DeviceAddress, 0x00, 0x56, 0x70, 0x49}; // working
   static BYTE tda9887set_pal_i[] =    {m_DeviceAddress, 0x00, 0x56, 0x70, 0x4a}; // not tested
   static BYTE tda9887set_pal_dk[] =   {m_DeviceAddress, 0x00, 0x56, 0x70, 0x4b}; // working
   static BYTE tda9887set_pal_l[] =    {m_DeviceAddress, 0x00, 0x86, 0x50, 0x4b}; // ?
   static BYTE tda9887set_ntsc[] =     {m_DeviceAddress, 0x00, 0x92, 0x30, 0x04}; // working
   static BYTE tda9887set_ntsc_jp[] =  {m_DeviceAddress, 0x00, 0x96, 0x70, 0x40}; // ?
   static BYTE tda9887set_fm_radio[] = {m_DeviceAddress, 0x00, 0x8e, 0x0d, 0x77}; // ?
   
   if (!bPreSet)
   {
       // only setup before tuning
       return;
   }
   
   BYTE *tda9887set = tda9887set_pal_bg;

   switch (VideoFormat)
   {
	case VIDEOFORMAT_PAL_B:    
    case VIDEOFORMAT_PAL_G:
    case VIDEOFORMAT_PAL_H:        
    case VIDEOFORMAT_PAL_N:
	case VIDEOFORMAT_SECAM_B:
    case VIDEOFORMAT_SECAM_G:
    case VIDEOFORMAT_SECAM_H:
		tda9887set = tda9887set_pal_bg;
		break;

	case VIDEOFORMAT_PAL_I:
		tda9887set = tda9887set_pal_i;
		break;

	case VIDEOFORMAT_PAL_D:
	case VIDEOFORMAT_SECAM_D:	
    case VIDEOFORMAT_SECAM_K:
    case VIDEOFORMAT_SECAM_K1:
		tda9887set = tda9887set_pal_dk;
		break;
	
	case VIDEOFORMAT_SECAM_L:
    case VIDEOFORMAT_SECAM_L1:
		tda9887set = tda9887set_pal_l;
		break;

    case VIDEOFORMAT_PAL_60:    
		///\todo Video bandwidth of PAL-60?
		break;

	case VIDEOFORMAT_PAL_M:
	case VIDEOFORMAT_PAL_N_COMBO:
	case VIDEOFORMAT_NTSC_M:        
		tda9887set = tda9887set_ntsc;
		break;

    case VIDEOFORMAT_NTSC_50:
    case VIDEOFORMAT_NTSC_M_Japan:
        tda9887set = tda9887set_ntsc_jp;
		break;

	case (VIDEOFORMAT_LASTONE+1):
		//radio
		tda9887set = tda9887set_fm_radio;
		break;
   }
      
   if (tda9887set != NULL)
   {   
      LOG(2,"TDA9885/6/7 Msi Master: 0x%02x 0x%02x 0x%02x", tda9887set[2],tda9887set[3],tda9887set[4]);
      m_I2CBus->Write(tda9887set, 5);   
   }
}
