//
// $Id$
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

/**
 * @file MSP34x0AudioDecoder.cpp CMSP34x0AudioDecoder Implementation (Rev D)
 */

#include "stdafx.h"

#ifdef WANT_BT8X8_SUPPORT

#include "MSP34x0.h"
#include "MSP34x0AudioDecoder.h"
#include "DebugLog.h"
#include "Crash.h"


#define MSP_UNCARRIER_HZ(carrier) ((long)((double)(carrier)*18432000.0/double(1<<24)))


eSupportedSoundChannels CMSP34x0AudioDecoder::DetectSoundChannelsRevG()
{
    LOG(2,_T("MSPrevG: DetectSoundChannelsRevG() enter"));

    int supported = SUPPORTEDSOUNDCHANNEL_MONO;
    int val = GetDEMRegister(DEM_RD_STATUS);
    if (val & 0x0040)
    {
        supported |= SUPPORTEDSOUNDCHANNEL_STEREO;
    }
    if (val & 0x0180)
    {
        supported |= SUPPORTEDSOUNDCHANNEL_LANG1 | SUPPORTEDSOUNDCHANNEL_LANG2;
    }

    LOG(2,_T("MSPrevG: DetectSoundChannelsRevG()=%d exit"), supported);
    return (eSupportedSoundChannels)supported;
}


CMSP34x0AudioDecoder::eStandard CMSP34x0AudioDecoder::DetectStandardRevG()
{
    LOG(2,_T("MSPrevG: DetectStandardRevG() enter"));

    if(m_MSPVersion != MSPVersionG)
    {
        return MSP34x0_STANDARD_NONE;
    }
    WORD Result = GetDEMRegister(DEM_RD_STANDARD_RESULT);
    if (Result >= 0x07ff)
    {
        return MSP34x0_STANDARD_AUTODETECTION_IN_PROGRESS;
    }

    LOG(2,_T("MSPrevG: DetectStandardRevG()=%d exit"), Result);
    return (eStandard)Result;
}

void CMSP34x0AudioDecoder::SetSoundChannelRevG(eSoundChannel soundChannel)
{
    LOG(2,_T("MSPrevG: SetSoundChannelRevG(%d) enter"), soundChannel);

    //SetDSPRegister(DSP_WR_FMAM_PRESCALE, 0x3000);
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

    WORD source = 0;
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

    LOG(2,_T("MSPrevG: SetSoundChannelRevG(%d) exit"), soundChannel);
}


void CMSP34x0AudioDecoder::InitializeRevG()
{
    LOG(2,_T("MSPrevG: InitializeRevG() enter"));
    SetDSPRegister(DSP_WR_I2S1_PRESCALE, 0x1000);
    SetDSPRegister(DSP_WR_I2S2_PRESCALE, 0x1000);
    LOG(2,_T("MSPrevG: InitializeRevG() exit"));
}

void CMSP34x0AudioDecoder::SetStandardRevG(eStandard standard, eVideoFormat videoformat, BOOL bWasDetected)
{
    LOG(2,_T("MSPrevG: SetStandardRevG(%d, %d, %d)"), standard, videoformat, bWasDetected);

    WORD modus = 0;

    if (standard == MSP34x0_STANDARD_AUTO)
    {
        // Set modus register
        modus = 0x3003;

        if (m_AudioInput == AUDIOINPUT_RADIO)
        {
            modus = 0x0003; //autodetect
        }
        else
        {
            switch(videoformat)
            {
            case VIDEOFORMAT_SECAM_L:
            case VIDEOFORMAT_SECAM_L1:
                modus = 0x6003;
                break;

            case VIDEOFORMAT_PAL_M:
            case VIDEOFORMAT_PAL_60:
            case VIDEOFORMAT_NTSC_50:
                modus = 0x3003;
                break;
            case VIDEOFORMAT_PAL_B:
            case VIDEOFORMAT_PAL_D:
            case VIDEOFORMAT_PAL_G:
            case VIDEOFORMAT_PAL_H:
            case VIDEOFORMAT_PAL_I:
            case VIDEOFORMAT_PAL_N:
            case VIDEOFORMAT_PAL_N_COMBO:
                modus = 0x7003;
                break;
            case VIDEOFORMAT_NTSC_M_Japan:
                modus = 0x5003;
                break;
            case VIDEOFORMAT_NTSC_M:
                modus = 0x3003;
                break;
            case VIDEOFORMAT_SECAM_B:
            case VIDEOFORMAT_SECAM_D:
            case VIDEOFORMAT_SECAM_G:
            case VIDEOFORMAT_SECAM_H:
            case VIDEOFORMAT_SECAM_K:
            case VIDEOFORMAT_SECAM_K1:
                modus = 0x7003;
                break;
            }
        }
    }
    else
    {
        /*
        if(m_SoundChannel == SOUNDCHANNEL_MONO)
        {
            SetDEMRegister(DEM_WR_MODUS, 0x2002);  //turn of auto sound select
        } else {
            SetDEMRegister(DEM_WR_MODUS, 0x2003);
        }
        */
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
        modus |= 0x2000;
    }
    SetDEMRegister(DEM_WR_MODUS, modus);

    if (!bWasDetected)
    {
        // Set standard
        SetDEMRegister(DEM_WR_STANDARD_SELECT, (int)standard);
    }


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

    LOG(2,_T("MSPrevG: SetStandardRevG(%d, %d, %d) exit"), standard, videoformat, bWasDetected);
}

void CMSP34x0AudioDecoder::SetCarrierRevG(eCarrier MajorCarrier, eCarrier MinorCarrier)
{
    // do nothing
}

#endif // WANT_BT8X8_SUPPORT
