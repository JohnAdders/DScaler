//
// $Id: MSP34xx_revA.cpp,v 1.2 2002-10-11 21:53:56 ittarnavsky Exp $
//
/////////////////////////////////////////////////////////////////////////////
//
// copyleft 2001 itt@myself.com
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
// Revision 1.1  2002/09/26 11:29:52  kooiman
// Split MSP code in 3 parts.
//
//
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MSP34x0.h"
#include "MSP34x0AudioDecoder.h"
#include "DebugLog.h"
#include "Crash.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#define MSP_UNCARRIER(carrier) ((double)(carrier)*18.432/(double)(1<<24))
#define MSP_CARRIER_HZ(freq) ((int)((double)(freq/18432000.0)*(1<<24)))
#define MSP_UNCARRIER_HZ(carrier) ((long)((double)(carrier)*18432000.0/double(1<<24)))


CMSP34x0AudioDecoder::eStandard CMSP34x0AudioDecoder::DetectStandardRevA()
{
    static int m_IndexVal1;
    static int m_IndexVal2;
    static int m_IndexMax1;
    static int m_IndexMax2;
    static int m_MajorIndex;
    static int m_MinorIndex;

    if (m_CarrierDetect_Phase == 0) //Setup
    {                
        if (m_ForceAMSound && IsSECAMVideoFormat(m_VideoFormat))
        {
		   // autodetect doesn't work well with AM ...		   
           return MSP34x0_STANDARD_L_NICAM_AM;
        }

        m_IndexVal1 = m_IndexVal2 = 0;
	    m_IndexMax1 = m_IndexMax2 = -1;

        m_MajorIndex = 0;
        m_MinorIndex = 0;

        SetCarrierRevA(CarrierDetectTable[m_MajorIndex].Major, CarrierDetectTable[m_MajorIndex].Major);

        m_CarrierDetect_Phase = 1;
        return MSP34x0_STANDARD_AUTODETECTION_IN_PROGRESS;
    }

    if (m_CarrierDetect_Phase == 1) //Detect Major
    {
        // Check carrier
        int val = GetDSPRegister(DSP_RD_FM1_DCLVL);
        if (val > 32768) val-= 65536;
        if (m_IndexVal1 < val)
        {
		   m_IndexVal1 = val;
           m_IndexMax1 = m_MajorIndex;
        }

        LOG(2,"MSP3400: Detecting standard. Major carrier = %g Mhz, val=%d",MSP_UNCARRIER(CarrierDetectTable[m_MajorIndex].Major),val);
        
        m_MajorIndex++;
        if (CarrierDetectTable[m_MajorIndex].Major != MSP34x0_NOCARRIER)
        {
            SetCarrierRevA(CarrierDetectTable[m_MajorIndex].Major, CarrierDetectTable[m_MajorIndex].Major);
            return MSP34x0_STANDARD_AUTODETECTION_IN_PROGRESS;
        }
        else
        {                          
            LOG(2,"MSP3400: Detecting standard. Found Major carrier = %g Mhz. Setup Minor.",MSP_UNCARRIER(CarrierDetectTable[m_IndexMax1].Major));
            
            // Found Major carrier.
            m_MajorIndex = m_IndexMax1;

            // Now setup the detection for the minor carrier                        
            m_MinorIndex = 0;
            if (CarrierDetectTable[m_MajorIndex].Minor[m_MinorIndex] == 
                  CarrierDetectTable[m_MajorIndex].Major)
            {
               //Did that already
               m_MinorIndex++;
            }
            
            if (CarrierDetectTable[m_MajorIndex].Minor[m_MinorIndex] == MSP34x0_NOCARRIER)
            {
                m_CarrierDetect_Phase = 3;  //Finished
            }
            else
            {
                SetCarrierRevA(CarrierDetectTable[m_MajorIndex].Minor[m_MinorIndex], CarrierDetectTable[m_MajorIndex].Minor[m_MinorIndex]);
                m_CarrierDetect_Phase = 2;
                
                int n = 0;
                while (CarrierDetectTable[m_MajorIndex].Minor[n] != MSP34x0_NOCARRIER)
                {
                    if (CarrierDetectTable[m_MajorIndex].Minor[n] == CarrierDetectTable[m_MajorIndex].Major)
                    {
                        m_IndexVal2 = m_IndexVal1;
                        m_IndexMax2 = n;

                        LOG(2,"MSP3400: Detecting standard. Minor carrier = %g Mhz, val=%d",MSP_UNCARRIER(CarrierDetectTable[m_MajorIndex].Minor[n]),m_IndexVal2);
                        break;
                    }
                    n++;
                }
                return MSP34x0_STANDARD_AUTODETECTION_IN_PROGRESS;
            }
        }        
    }

    if (m_CarrierDetect_Phase == 2) //Detect minor
    {
        // Check carrier
        int val = GetDSPRegister(DSP_RD_FM1_DCLVL);
        if (val > 32768) val-= 65536;
        if (m_IndexVal2 < val)
        {
		    m_IndexVal2 = val;
            m_IndexMax2 = m_MinorIndex;
        }
        
        LOG(2,"MSP3400: Detecting standard. Minor carrier = %g Mhz, val=%d",MSP_UNCARRIER(CarrierDetectTable[m_MajorIndex].Minor[m_MinorIndex]),val);

        m_MinorIndex++;
        if (CarrierDetectTable[m_MajorIndex].Minor[m_MinorIndex] == 
                  CarrierDetectTable[m_MajorIndex].Major)
        {
            //Did that already
            m_MinorIndex++;
        }

        if (CarrierDetectTable[m_MajorIndex].Minor[m_MinorIndex] != MSP34x0_NOCARRIER)
        {
            SetCarrierRevA(CarrierDetectTable[m_MajorIndex].Minor[m_MinorIndex], CarrierDetectTable[m_MajorIndex].Minor[m_MinorIndex]);
            return MSP34x0_STANDARD_AUTODETECTION_IN_PROGRESS;
        }
        else
        {
            m_CarrierDetect_Phase = 3;        
        }
    }
    if (m_CarrierDetect_Phase == 3) //Found carriers, determine standard
    {
          eCarrier MajorCarrier = CarrierDetectTable[ m_IndexMax1 ].Major;
          eCarrier MinorCarrier = MajorCarrier;
          if (m_IndexMax2 >= 0)
          {
              MinorCarrier = CarrierDetectTable[ m_IndexMax1 ].Minor[ m_IndexMax2 ];
          }

          LOG(2,"MSP3400: Detecting standard. Found Major/Minor : %g / %g Mhz",MSP_UNCARRIER(MajorCarrier),(MSP_UNCARRIER(MinorCarrier)));

          eStandard Standard = MSP34x0_STANDARD_NONE;
          int n = 0;
          while (m_MSPStandards[n].Name != NULL)
          {
              if (    (m_MSPStandards[n].MajorCarrier == MajorCarrier)
                   && (m_MSPStandards[n].MinorCarrier == MinorCarrier)
                 )
              {
                  Standard = m_MSPStandards[n].Standard;                  
                  break;
              }
              n++;
          }

          //Duplicates
          if ((MajorCarrier == MSP34x0_CARRIER_4_5) && (MinorCarrier == MSP34x0_CARRIER_4_5))
          {
              // BTSC or Japan Stereo?

              if (m_VideoFormat == VIDEOFORMAT_NTSC_M_Japan)
              {
                  Standard = MSP34x0_STANDARD_M_EIA_J;
              }
              else
              {
                  Standard = MSP34x0_STANDARD_M_BTSC;
              }        
                
              LOG(2,"MSP3400: Standard can be BTSC or Japan Stereo.");
              LOG(2,"MSP3400: Chose %s based on video format (%d)",(Standard==MSP34x0_STANDARD_M_BTSC)?"BTSC":"Japan Stereo",m_VideoFormat);
          }            
          else 
          if ((MajorCarrier == MSP34x0_CARRIER_6_5) && (MinorCarrier == MSP34x0_CARRIER_5_85))
          {
             Standard = MSP34x0_STANDARD_DK_NICAM_FM;
             if (IsSECAMVideoFormat(m_VideoFormat))
             {
                Standard = MSP34x0_STANDARD_L_NICAM_AM;
             }             
             LOG(2,"MSP3400: Standard can be NICAM AM or FM.");
             LOG(2,"MSP3400: Chose NICAM %s based on video format (%d) (%sSECAM)",(Standard==MSP34x0_STANDARD_L_NICAM_AM)?"AM":"FM",m_VideoFormat,IsSECAMVideoFormat(m_VideoFormat)?"":"No ");
          }
          else
          if ((MajorCarrier == MSP34x0_CARRIER_6_5) && (MinorCarrier == MSP34x0_CARRIER_6_5))
          {
              Standard = MSP34x0_STANDARD_SAT;
              // D/K FM-Mono(HDEV3) or Sat?
              LOG(2,"MSP3400: Standard can be D/K FM-MONO(HDEV3) or SAT.");
              LOG(2,"MSP3400: Chose SAT.");
          }

          m_AudioStandardMajorCarrier = MSP_UNCARRIER_HZ(MajorCarrier);
          m_AudioStandardMinorCarrier = MSP_UNCARRIER_HZ(MinorCarrier);
          if (Standard != MSP34x0_STANDARD_NONE)
          {                            
              LOG(2,"MSP3400: Detected standard: %d with carriers at %g/%g Mhz",Standard,(double)m_AudioStandardMajorCarrier/1000000.0, (double)m_AudioStandardMinorCarrier/1000000.0);              
          }
          else
          {              
              //Just try...
              if (IsSECAMVideoFormat(m_VideoFormat))
              {
                    Standard = MSP34x0_STANDARD_L_NICAM_AM;
              }
              else
              {
                    Standard = MSP34x0_STANDARD_M_DUAL_FM;              
              }
              LOG(2,"MSP3400: Detected standard: Unknown. Set to %d with carriers at %g/%g Mhz",Standard,(double)m_AudioStandardMajorCarrier/1000000.0, (double)m_AudioStandardMinorCarrier/1000000.0);                            
          }
          return Standard;
    }

    return MSP34x0_STANDARD_NONE;    
}

eSupportedSoundChannels CMSP34x0AudioDecoder::DetectSoundChannelsRevA()
{
    int supported = SUPPORTEDSOUNDCHANNEL_MONO;
    switch (m_AudioStandard)
    {
    case MSP34x0_STANDARD_M_DUAL_FM:
    case MSP34x0_STANDARD_BG_DUAL_FM:
    case MSP34x0_STANDARD_DK1_DUAL_FM:
    case MSP34x0_STANDARD_DK2_DUAL_FM:
    case MSP34x0_STANDARD_DK_FM_MONO:
    case MSP34x0_STANDARD_DK3_DUAL_FM:
    case MSP34x0_STANDARD_M_EIA_J:    	
        {
            int val = GetDSPRegister(DSP_RD_A2_STEREO_DETECT);
            if (val > 32767) { val -= 65536; }

            if (val > 4096)
            {
                supported |= SUPPORTEDSOUNDCHANNEL_STEREO;
            }
            else if (val < -4096)
            {
                supported |= SUPPORTEDSOUNDCHANNEL_LANG1|SUPPORTEDSOUNDCHANNEL_LANG2;
            }
            LOG(2,"MSP3400: Stereo detect: val = %d",val);
        }
        break;
    case MSP34x0_STANDARD_BG_NICAM_FM:
    case MSP34x0_STANDARD_I_NICAM_FM:
    case MSP34x0_STANDARD_DK_NICAM_FM:
    case MSP34x0_STANDARD_DK_NICAM_FM_HDEV2:
    case MSP34x0_STANDARD_DK_NICAM_FM_HDEV3:
    case MSP34x0_STANDARD_L_NICAM_AM:
        {
            int val = GetDEMRegister(DEM_RD_C_AD_BITS);
            LOG(2,"MSP3400: Stereo detect: Nicam sync = %d, val = %d",val&1,(val&0x1e)>>1);
            if (val&1)
            {                
                // nicam synced
                switch ((val & 0x1e) >> 1)  
                {
                case 0:
                case 8:
                    supported |= SUPPORTEDSOUNDCHANNEL_STEREO;
                    break;
                case 1:
                case 9:
                    supported |= SUPPORTEDSOUNDCHANNEL_LANG1;
                    break;
                case 2:
                case 10:
                    supported |= SUPPORTEDSOUNDCHANNEL_LANG1|SUPPORTEDSOUNDCHANNEL_LANG2;
                    break;
                default:
                    supported |= SUPPORTEDSOUNDCHANNEL_MONO;
                    break;
                }
            }
            else
            {
                supported |= SUPPORTEDSOUNDCHANNEL_MONO;
            }
        }
        break;
    case MSP34x0_STANDARD_M_BTSC:
    case MSP34x0_STANDARD_M_BTSC_MONO:
        {
            int val = GetDEMRegister(DEM_RD_STATUS);
            LOG(2,"MSP3400: Stereo detect: BTSC status = 0x%04x",val);
            if (val&0x0040)
            {
                supported |= SUPPORTEDSOUNDCHANNEL_STEREO;
            }
            if (val&0x0180)
            {
                supported |= SUPPORTEDSOUNDCHANNEL_LANG1|SUPPORTEDSOUNDCHANNEL_LANG2;
            }
        }
        break;
    default:
        break;
    }
    return (eSupportedSoundChannels)supported;
}

void CMSP34x0AudioDecoder::SetSoundChannelRevA(eSoundChannel SoundChannel)
{       
    WORD nicam = 0;  // channel source: FM/AM or nicam
	WORD source = 0;

    if (m_AudioInput == AUDIOINPUT_EXTERNAL)
    {
	    nicam = 0x0200;
	}
    else
    {    
        // switch demodulator
        switch (m_AudioStandard)
        {
        case MSP34x0_STANDARD_BG_DUAL_FM:
            if (SoundChannel == SOUNDCHANNEL_STEREO)
            {
                SetDSPRegister(DSP_WR_FMAM_PRESCALE, 0x3001);
            }
            else
            {
                SetDSPRegister(DSP_WR_FMAM_PRESCALE, 0x3000);
            }                 
            break;
        case MSP34x0_STANDARD_M_DUAL_FM:
            if (SoundChannel == SOUNDCHANNEL_STEREO)
            {
                SetDSPRegister(DSP_WR_FMAM_PRESCALE, 0x3002);
            }
            else
            {
                SetDSPRegister(DSP_WR_FMAM_PRESCALE, 0x3000);
            }                 
            break;
        case MSP34x0_STANDARD_DK1_DUAL_FM:
        case MSP34x0_STANDARD_DK2_DUAL_FM:
        case MSP34x0_STANDARD_DK_FM_MONO:
        case MSP34x0_STANDARD_DK3_DUAL_FM:
        case MSP34x0_STANDARD_M_EIA_J:    	
            break;
        case MSP34x0_STANDARD_SAT_MONO:          
        case MSP34x0_STANDARD_SAT:        
            if ( (SoundChannel == SOUNDCHANNEL_LANGUAGE1) || (SoundChannel == SOUNDCHANNEL_LANGUAGE2) )
            {
                SetCarrierRevA(MSP34x0_CARRIER_7_02, MSP34x0_CARRIER_7_38);
            } 
            else if (SoundChannel == SOUNDCHANNEL_STEREO)
            {			
                SetCarrierRevA(MSP34x0_CARRIER_7_02, MSP34x0_CARRIER_7_20);
            } 
            else 
            {
                SetCarrierRevA(MSP34x0_CARRIER_6_5, MSP34x0_CARRIER_6_5);              
            }
            break;
        case MSP34x0_STANDARD_SAT_ADR:
            break;
        case MSP34x0_STANDARD_BG_NICAM_FM:
        case MSP34x0_STANDARD_I_NICAM_FM:
        case MSP34x0_STANDARD_DK_NICAM_FM:
        case MSP34x0_STANDARD_DK_NICAM_FM_HDEV2:
        case MSP34x0_STANDARD_DK_NICAM_FM_HDEV3:
        case MSP34x0_STANDARD_L_NICAM_AM:
            //SetCarrierRevA(m_MSPStandards[m_AudioStandard].MajorCarrier, m_MSPStandards[m_AudioStandard].MinorCarrier);
            //if (m_NicamOn)
            //{
            nicam=0x0100;
            //}
            break;
        case MSP34x0_STANDARD_M_BTSC:
        case MSP34x0_STANDARD_M_BTSC_MONO:
            nicam = 0x0300;
            break;
        case MSP34x0_STANDARD_FM_RADIO:
            //SetSCARTxbar(MSP34x0_SCARTOUTPUT_DSP_INPUT, MSP34x0_SCARTINPUT_SCART_2);
            //SetDSPRegister(DSP_WR_SCART_PRESCALE, 0x1900);                 
            nicam = 0x0200;
            break;
        default:
            return;
        }
  }

	
    if (SoundChannel == SOUNDCHANNEL_LANGUAGE1) 
    {
	      source = 0x0000 | nicam;
	} 
    else if (SoundChannel == SOUNDCHANNEL_LANGUAGE2) 
    {
		  source = 0x0010 | nicam;		
	} 
    else if (SoundChannel == SOUNDCHANNEL_STEREO) 
    {
		  source = 0x0020 | nicam;
	} 
    else 
    {
        // Mono		
		source = 0x0000 | nicam;

        if (m_AudioStandard == MSP34x0_STANDARD_L_NICAM_AM)
        {
            source = 0x200;
            /// AM mono decoding is handled by tuner, not MSP chip
            SetSCARTxbar(MSP34x0_SCARTOUTPUT_DSP_INPUT, MSP34x0_SCARTINPUT_MONO);            
            SetDSPRegister(DSP_WR_SCART_PRESCALE, 0x1900);
		 }
	}
	
    SetDSPRegister(DSP_WR_LDSPK_SOURCE, source);
    SetDSPRegister(DSP_WR_HEADPH_SOURCE, source);
    SetDSPRegister(DSP_WR_SCART1_SOURCE, source);
    SetDSPRegister(DSP_WR_SCART2_SOURCE, source);
    SetDSPRegister(DSP_WR_I2S_SOURCE, source);    	
}


void CMSP34x0AudioDecoder::InitializeRevA()
{   
    //Everything is set up in SetStandard3400 and SetSoundChannel3400
}

void CMSP34x0AudioDecoder::SetStandardRevA(eStandard standard, eVideoFormat videoformat, BOOL bCurrentCarriers, eSoundChannel soundChannel)
{
    TStandardDefinition StandardDefinition;

    if (standard == MSP34x0_STANDARD_AUTO)
    {
        StandardDefinition.Standard = MSP34x0_STANDARD_AUTO;
        StandardDefinition.FIRType = FIR_AM_DETECT;
        StandardDefinition.StereoType = STEREO_FM;
        StandardDefinition.MajorCarrier = MSP34x0_CARRIER_4_5;
        StandardDefinition.MinorCarrier = MSP34x0_CARRIER_4_5;

        m_CarrierDetect_Phase = 0;
        // Mute
        SetDSPRegister(DSP_WR_LDSPK_VOLUME, 0x0000);
        SetDSPRegister(DSP_WR_HEADPH_VOLUME, 0x0000);
        SetDSPRegister(DSP_WR_SCART1_VOLUME, 0x0000);
        SetDSPRegister(DSP_WR_SCART2_VOLUME, 0x0000);
    }
    else
    {        
        StandardDefinition.Standard = MSP34x0_STANDARD_NONE;

        int standardIndex = 0;        
        while (m_MSPStandards[standardIndex].Name != NULL)
        {
            if (m_MSPStandards[standardIndex].Standard == standard)
            {
                StandardDefinition = m_MSPStandards[standardIndex];
                break;
            }
            standardIndex++;
        }
    }

    if (StandardDefinition.Standard == MSP34x0_STANDARD_NONE)
    {
        //No standard, mute
        Reset();
        return;
    }

    /// Step 0: AD_CV
    if(StandardDefinition.StereoType == STEREO_SAT)
    {
        // set up for AGC bit 7
        // and bits 6..1  100011 for SAT
        SetDEMRegister(DEM_WR_AD_CV, 0xC6);
    }
    else if(StandardDefinition.StereoType == STEREO_ADR)
    {
        // set up for AGC bit 7
        // and bits 6..1  010100 for SAT ADR
        SetDEMRegister(DEM_WR_AD_CV, 0xA8);
    }
    else if(StandardDefinition.StereoType == STEREO_NICAM && 
                StandardDefinition.MonoType == MONO_AM)
    {
        // set up for AGC bit 7
        // and bits 6..1  100011 AM and NICAM
        SetDEMRegister(DEM_WR_AD_CV, 0xC6);
    }
    else
    {
        // set up for AGC bit 7
        // and bits 6..1  101000 FM and NICAM
        // or Dual FM
        SetDEMRegister(DEM_WR_AD_CV, 0xD0);
    }
    
    // I have no idea what this is supposed to do.
    /// Step 1: AUDIO_PLL
    // FIXME what should this be may need to be 0 for NICAM
    // SetDEMRegister(DEM_WR_AUDIO_PLL, 1);

    /// Step 2: FAWCT_SOLL
    if(StandardDefinition.StereoType == STEREO_NICAM)
    {
        SetDEMRegister(DEM_WR_FAWCT_SOLL, 12);
    }
    
    /// Step 3: FAW_ER_TOL
    if(StandardDefinition.StereoType == STEREO_NICAM)
    {
        SetDEMRegister(DEM_WR_FAW_ER_TOL, 2);
    }

	
    // FIR data

    int FIRType = StandardDefinition.FIRType;
    int i;
    for (i = 5; i >= 0; i--)
    {
        SetDEMRegister(DEM_WR_FIR1, m_FIRTypes[FIRType].FIR1[i]);
    }

    SetDEMRegister(DEM_WR_FIR2, 0x0004);
    SetDEMRegister(DEM_WR_FIR2, 0x0040);
    SetDEMRegister(DEM_WR_FIR2, 0x0000);
    
    for (i = 5; i >= 0; i--)
    {
        SetDEMRegister(DEM_WR_FIR2, m_FIRTypes[FIRType].FIR2[i]);
    }

    // Mode reg
    /// Step 6: MODE_REG
    WORD ModeReg = 1 << 10;// bit 10 must be set according to documentation
    if(StandardDefinition.StereoType == STEREO_NICAM)
    {
        // set NICAM mode
        ModeReg |= 1 << 6;
        if(StandardDefinition.MonoType == MONO_AM)
        {
            // set MSP 1/2 to AM
            ModeReg |= 1 << 8;
        }
    }
    else
    {
        if(StandardDefinition.StereoType != STEREO_NONE)
        {
            // set Two carrier FM mode
            ModeReg |= 1 << 7;
        }
        if(StandardDefinition.MonoType == MONO_FM)
        {
            // set MSP channel1 to FM
            ModeReg |= 1 << 7;
        }
        else
        {
            // set MSP 1/2 to AM
            ModeReg |= 1 << 8;
        }
    }
    if (StandardDefinition.StereoType == STEREO_ADR)
    {
        // Set Mode of ADR Interface to ADR Mode
        //ModeReg |= 1<<14;
    }

    switch(standard)
    {
    case MSP34x0_STANDARD_BG_DUAL_FM:
    case MSP34x0_STANDARD_DK1_DUAL_FM:
        ModeReg |= 1 << 13;
        break;
    case MSP34x0_STANDARD_AUTO:
        ModeReg = 0x500;
        break;
    case MSP34x0_STANDARD_L_NICAM_AM:
        ModeReg = 0x140;
    default:
        break;
    }

    SetDEMRegister(DEM_WR_MODE_REG, ModeReg);

    
    // Set Carrier
    if (bCurrentCarriers && (m_AudioStandardMajorCarrier!=0) && (m_AudioStandardMinorCarrier!=0) )
    {
        SetCarrierRevA((eCarrier)MSP_CARRIER_HZ(m_AudioStandardMajorCarrier), (eCarrier)MSP_CARRIER_HZ(m_AudioStandardMinorCarrier));
    }
    else
    {
        SetCarrierRevA(StandardDefinition.MajorCarrier, StandardDefinition.MinorCarrier);
    }

    /// Step 12 NICAM_START
    if(m_MSPStandards[m_AudioStandard].StereoType == STEREO_NICAM)
    {
        SetDEMRegister(DEM_WR_SEARCH_NICAM, 0);
    }
    
    // FM and NICAM prescale
    WORD fmprescale = 0x3000;
    switch(standard)
    {
    case MSP34x0_STANDARD_BG_DUAL_FM:
        fmprescale |= 1;    //B/G stereo 
        break;
    case MSP34x0_STANDARD_M_DUAL_FM:
        fmprescale |= 2;    //M stereo (Korean)
        break;
    case MSP34x0_STANDARD_L_NICAM_AM:
        fmprescale = 0x7c03;
    case MSP34x0_STANDARD_FM_RADIO:
        fmprescale = 0x2403;
    default:
        break;
    }
    
    SetDSPRegister(DSP_WR_FMAM_PRESCALE, fmprescale);
    SetDSPRegister(DSP_WR_NICAM_PRESCALE, 0x5A00);

    // Source
    SetSoundChannelRevA(soundChannel);
    
    
    // reset the ident filter
    SetDSPRegister(DSP_WR_IDENT_MODE, 0x3f);   
    Sleep(1);        
    if (standard == MSP34x0_STANDARD_M_DUAL_FM)
    {       
        SetDSPRegister(DSP_WR_IDENT_MODE, 0x01);   
    }
    else if (standard == MSP34x0_STANDARD_BG_DUAL_FM)
    {
        SetDSPRegister(DSP_WR_IDENT_MODE, 0x00);
    }
    
    /*SetDSPRegister(DSP_WR_LDSPK_VOLUME, 0x7300);
    SetDSPRegister(DSP_WR_HEADPH_VOLUME, 0x7300);

    SetDSPRegister(DSP_WR_SCART1_VOLUME, 0x4000);
    SetDSPRegister(DSP_WR_SCART2_VOLUME, 0x4000);
    */    
}

void CMSP34x0AudioDecoder::SetCarrierRevA(eCarrier MajorCarrier, eCarrier MinorCarrier)
{
   /// Step 7, 8, 9 and 10: DCO1_LO, DCO1_HI, DCO2_LO and DCO2_HI    
    SetDEMRegister(DEM_WR_DCO1_LO, MinorCarrier & 0xfff);
    SetDEMRegister(DEM_WR_DCO1_HI, MinorCarrier >> 12);
    SetDEMRegister(DEM_WR_DCO2_LO, MajorCarrier & 0xfff);
    SetDEMRegister(DEM_WR_DCO2_HI, MajorCarrier >> 12);

    SetDEMRegister(DEM_WR_LOAD_REG_12, 0); 
}
