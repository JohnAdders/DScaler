//
// $Id: MSP34xx_revG.cpp,v 1.1 2002-09-26 11:29:52 kooiman Exp $
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
//
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MSP34x0.h"
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


eSupportedSoundChannels CMSP34x0Decoder::DetectSoundChannels34x1G()
{
    int supported = SUPPORTEDSOUNDCHANNEL_MONO;
    int val = GetDEMRegister(DEM_RD_STATUS);
    LOG(2,"MSP34x1G: Stereo detect: status = 0x%04x",val);
    if (val&0x0040)
    {
        supported |= SUPPORTEDSOUNDCHANNEL_STEREO;
    }
    if (val&0x0180)
    {
        supported |= SUPPORTEDSOUNDCHANNEL_LANG1|SUPPORTEDSOUNDCHANNEL_LANG2;
    }

    return (eSupportedSoundChannels)supported;
}


CMSP34x0Decoder::eStandard CMSP34x0Decoder::DetectStandard34x1G()
{
    if(m_MSPVersion != MSPVersionG) 
    {
        return MSP34x0_STANDARD_NONE;
    }
    WORD Result = GetDEMRegister(DEM_RD_STANDARD_RESULT);
    if (Result >= 0x07ff)
    {
        return MSP34x0_STANDARD_AUTODETECTION_IN_PROGRESS;
    }    
    return (eStandard)Result;
}

void CMSP34x0Decoder::SetSoundChannel34x1G(eSoundChannel soundChannel)
{
    LOG(2,"MSP34xx: Set sound channel: %d",soundChannel);
    
    WORD source = 0;
    //WORD nicam = 0;
    
    SetDSPRegister(DSP_WR_FMAM_PRESCALE, 0x3000);
    SetDSPRegister(DSP_WR_NICAM_PRESCALE, 0x5A00);
    SetDSPRegister(DSP_WR_SCART_PRESCALE, 0x1900);
    
    if (m_AudioStandard == MSP34x0_STANDARD_L_NICAM_AM)
    {
        SetDSPRegister(DSP_WR_FMAM_PRESCALE, 0x7c03);
    }
    else
    {
        if(soundChannel == SOUNDCHANNEL_MONO)
        {            
            SetDSPRegister(DSP_WR_FMAM_PRESCALE, 0x3003);
        }
        else
        {
            SetDSPRegister(DSP_WR_FMAM_PRESCALE, 0x2403); //  0x3000);
        }
    }
    
    switch (soundChannel)
    {
    case SOUNDCHANNEL_MONO:
        source = 0x20;
        break;
    case SOUNDCHANNEL_STEREO:
        source = 0x120;
        break;
    case SOUNDCHANNEL_LANGUAGE1:
        // if not bilingual then stereo is automatically enabled if available
        source = 0x320;
        break;
    case SOUNDCHANNEL_LANGUAGE2:
        // if not bilingual then stereo is automatically enabled if available
        source = 0x420;
        break;
    default:
        break;
    }
    
    SetDSPRegister(DSP_WR_LDSPK_SOURCE, source);
    SetDSPRegister(DSP_WR_HEADPH_SOURCE, source);
    SetDSPRegister(DSP_WR_SCART1_SOURCE, source);
    SetDSPRegister(DSP_WR_SCART2_SOURCE, source);
    SetDSPRegister(DSP_WR_I2S_SOURCE, source);
}


void CMSP34x0Decoder::Initialize34x1G()
{
    
    SetDSPRegister(DSP_WR_I2S1_PRESCALE, 0x1000);
    SetDSPRegister(DSP_WR_I2S2_PRESCALE, 0x1000);

    // set up so that we fall back to AM/FM if there is no NICAM
    // required on D series chips
    /*if(m_MSPVersion == MSPVersionD)
    {
        SetDEMRegister(DEM_WR_AUTO_FMAM, 0x0001);
    }*/
    
    // 3. reset volume to 0dB
    /*SetDSPRegister(DSP_WR_LDSPK_VOLUME, 0x7300);
    SetDSPRegister(DSP_WR_HEADPH_VOLUME, 0x7300);    
    SetDSPRegister(DSP_WR_SCART1_VOLUME, 0x4000);
    SetDSPRegister(DSP_WR_SCART2_VOLUME, 0x4000);    
    */
}

void CMSP34x0Decoder::SetStandard34x1G(eStandard standard, eVideoFormat videoformat)
{
    if(m_MSPVersion != MSPVersionG) return;

    LOG(2,"MSP34xx: Set standard: %d",standard);
    
/*
    // the following is only valid for RevG since it uses Automatic Sound Select
    WORD modus = 0;

    // if mono output is forced, Automatic Sound Select must be disabled, otherwise enabled.
    if(m_SoundChannel == SOUNDCHANNEL_MONO)
    {
        modus = 0;
    }
    else
    {
        modus = 1;
    }

    // choose sound IF2 input pin if needed.
    // todo: Maybe some cards are using IF1?
    switch(m_AudioInput)
    {
    case AUDIOINPUT_RADIO:
    case AUDIOINPUT_TUNER:
        if(!m_bUseInputPin1)
        {
            modus |= 0x100;
        }
        break;
    default:
        break;
    }

    SetDEMRegister(DEM_WR_MODUS, modus);
*/
    
    if (standard == MSP34x0_STANDARD_AUTO)
    {    
        // Set mode    
        WORD mode = 0x3003;

        if (m_AudioInput == AUDIOINPUT_RADIO)
        {        
            mode = 0x0003; //autodetect        
        }
        else
        {   
            switch(videoformat)
            {
                case VIDEOFORMAT_SECAM_L:
                case VIDEOFORMAT_SECAM_L1:
                    mode = 0x6003;
                    break;
            
                case VIDEOFORMAT_PAL_M:
                case VIDEOFORMAT_PAL_60:
                case VIDEOFORMAT_NTSC_50:
                    mode = 0x3003;
                    break;
                case VIDEOFORMAT_PAL_B:
                case VIDEOFORMAT_PAL_D:
                case VIDEOFORMAT_PAL_G:
                case VIDEOFORMAT_PAL_H:
                case VIDEOFORMAT_PAL_I:
                case VIDEOFORMAT_PAL_N:
                case VIDEOFORMAT_PAL_N_COMBO:
                    mode = 0x7003;
                    break;
                case VIDEOFORMAT_NTSC_M_Japan:
                    mode = 0x5003;
                    break;
                case VIDEOFORMAT_NTSC_M:
                    mode = 0x3003;
                    break;
                case VIDEOFORMAT_SECAM_B:        
                case VIDEOFORMAT_SECAM_D:
                case VIDEOFORMAT_SECAM_G:
                case VIDEOFORMAT_SECAM_H:
                case VIDEOFORMAT_SECAM_K:
                case VIDEOFORMAT_SECAM_K1:
                    mode = 0x7003;
                    break;
            }
        }
        SetDEMRegister(DEM_WR_MODUS, mode);
    }
        
    // Set standard
    SetDEMRegister(DEM_WR_STANDARD_SELECT, (int)standard);    
        
    m_AudioStandardMajorCarrier = 0;
    m_AudioStandardMinorCarrier = 0;
    if (standard != MSP34x0_STANDARD_AUTO)
    {
        int nIndex = 0;
        while (m_MSPStandards[nIndex].Name != NULL)
        {
            if (m_MSPStandards[nIndex].Standard == standard)
            {
                m_AudioStandardMajorCarrier = MSP_UNCARRIER_HZ(m_MSPStandards[nIndex].MajorCarrier);
                m_AudioStandardMinorCarrier = MSP_UNCARRIER_HZ(m_MSPStandards[nIndex].MinorCarrier);
            }
            nIndex++;
        }
    }
}

void CMSP34x0Decoder::SetCarrier34x1G(eCarrier MajorCarrier, eCarrier MinorCarrier)
{
    // Not necessary because rev G detects the carrier?
}

