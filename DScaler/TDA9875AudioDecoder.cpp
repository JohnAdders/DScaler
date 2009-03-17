//
// $Id$
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2004 Robert Milharcic.  All rights reserved.
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
 * @file TDA9875AudioDecoder.cpp TDA9875AudioDecoder Implementation
 */

#include "stdafx.h"
#include "TDA9875.h"
#include "TDA9875AudioDecoder.h"
#include "Crash.h" 
#include "DebugLog.h"
#include "audio.h"

#define TDA9875_CARRIER(freq) ((1<<24)*((double)freq)/12288000.0)

CTDA9875AudioDecoder::TStandardDefinition CTDA9875AudioDecoder::m_TDA9875Standards[] =
{
    //ANALOG 2-CARRIER SYSTEMS
    {
        "M Analog (Mono) FM/NONE Mono",
        TDA9875_STANDARD_M_ANALOG_FM_NONE,
        TDA9875_CARRIER_4_5,
        TDA9875_CARRIER_4_5,
        MONO_FM,
        STEREO_NONE,
        TDA9875_DE_EMPHASIS_75
    },
    {
        "M Analog (A2+) FM/FM Stereo",
        TDA9875_STANDARD_M_ANALOG_FM_FM,
        TDA9875_CARRIER_4_5,
        TDA9875_CARRIER_4_724,
        MONO_FM,
        STEREO_FM,
        TDA9875_DE_EMPHASIS_75
    },
    {
        "B/G Analog (A2) FM/FM Stereo",
        TDA9875_STANDARD_BG_ANALOG_FM_FM,
        TDA9875_CARRIER_5_5,
        TDA9875_CARRIER_5_742,
        MONO_FM,
        STEREO_FM,
        TDA9875_DE_EMPHASIS_50
    },
    {
        "I Analog (Mono) FM/NONE Mono",
        TDA9875_STANDARD_I_ANALOG_FM_NONE,
        TDA9875_CARRIER_6_0,
        TDA9875_CARRIER_6_0,
        MONO_FM,
        STEREO_NONE,
        TDA9875_DE_EMPHASIS_50
    },
    {
        "D/K1 Analog (A2) FM/FM Stereo",
        TDA9875_STANDARD_DK1_ANALOG_FM_FM,
        TDA9875_CARRIER_6_5,
        TDA9875_CARRIER_6_742,
        MONO_FM,
        STEREO_FM,
        TDA9875_DE_EMPHASIS_50
    },
    {
        "D/K2 Analog (A2*) FM/FM Stereo",
        TDA9875_STANDARD_DK2_ANALOG_FM_FM,
        TDA9875_CARRIER_6_5,
        TDA9875_CARRIER_6_26,
        MONO_FM,
        STEREO_FM,
        TDA9875_DE_EMPHASIS_50
    },
    //2-CARRIER SYSTEMS WITH NICAM
    {
        "B/G Digital (NICAM) FM/NICAM Stereo",
        TDA9875_STANDARD_BG_DIGITAL_FM_NICAM,
        TDA9875_CARRIER_5_5,
        TDA9875_CARRIER_5_85,
        MONO_FM,
        STEREO_NICAM,
        TDA9875_DE_EMPHASIS_J17
    },
    {
        "I Digital (NICAM) FM/NICAM Stereo",
        TDA9875_STANDARD_I_DIGITAL_FM_NICAM,
        TDA9875_CARRIER_6_0,
        TDA9875_CARRIER_6_552,
        MONO_FM,
        STEREO_NICAM,
        TDA9875_DE_EMPHASIS_J17
    },
    {
        "D/K Digital (NICAM) FM/NICAM Stereo",
        TDA9875_STANDARD_DK_DIGITAL_FM_NICAM,
        TDA9875_CARRIER_6_5,
        TDA9875_CARRIER_5_85,
        MONO_FM,
        STEREO_NICAM,
        TDA9875_DE_EMPHASIS_J17
    },
    {
        "L Digital (NICAM) AM/NICAM Stereo",
        TDA9875_STANDARD_L_DIGITAL_AM_NICAM,
        TDA9875_CARRIER_6_5,
        TDA9875_CARRIER_5_85,
        MONO_AM,
        STEREO_NICAM,
        TDA9875_DE_EMPHASIS_J17
    },
    //2-CARRIER SATELLITE SYSTEMS
    {
        "SAT (Main) FM/NONE Mono",
        TDA9875_STANDARD_SAT_FM_MONO,
        TDA9875_CARRIER_6_5,
        TDA9875_CARRIER_6_5,
        MONO_FM_SAT,
        STEREO_NONE,
        TDA9875_DE_EMPHASIS_50
    },
    {
        "SAT1 (Sub) FM/FM Stereo",
        TDA9875_STANDARD_SAT1_FM_FM,
        TDA9875_CARRIER_7_02,
        TDA9875_CARRIER_7_20,
        MONO_FM_SAT,
        STEREO_FM,
        TDA9875_DE_EMPHASIS_ADAPTIVE
    },
    {
        "SAT2 (Sub) FM/FM Stereo",
        TDA9875_STANDARD_SAT2_FM_FM,
        TDA9875_CARRIER_7_38,
        TDA9875_CARRIER_7_56,
        MONO_FM_SAT,
        STEREO_FM,
        TDA9875_DE_EMPHASIS_ADAPTIVE
    },
    {
        "SAT3 (Sub) FM/FM Stereo",
        TDA9875_STANDARD_SAT3_FM_FM,
        TDA9875_CARRIER_7_74,
        TDA9875_CARRIER_7_92,
        MONO_FM_SAT,
        STEREO_FM,
        TDA9875_DE_EMPHASIS_ADAPTIVE
    },
    {
        "SAT4 (Sub) FM/FM Stereo",
        TDA9875_STANDARD_SAT4_FM_FM,
        TDA9875_CARRIER_8_10,
        TDA9875_CARRIER_8_28,
        MONO_FM_SAT,
        STEREO_FM,
        TDA9875_DE_EMPHASIS_ADAPTIVE
    },
    {
        NULL,
        TDA9875_STANDARD_NONE,
        TDA9875_NOCARRIER,
        TDA9875_NOCARRIER,
        MONO_FM,
        STEREO_NONE,
        TDA9875_DE_EMPHASIS_OFF
    },
};

CTDA9875AudioDecoder::TCarrierDetect CTDA9875AudioDecoder::CarrierDetectTable[] =
{
    {
        TDA9875_CARRIER_4_5,
        {
            TDA9875_CARRIER_4_724,
            TDA9875_NOCARRIER

        }
    },
    {
        TDA9875_CARRIER_5_5,
        {
            TDA9875_CARRIER_5_742,
            TDA9875_CARRIER_5_85,
            TDA9875_NOCARRIER
        }
    },
    {
        TDA9875_CARRIER_6_0,
        {
            TDA9875_CARRIER_6_552,
            TDA9875_NOCARRIER
        }
    },
    {
        TDA9875_CARRIER_6_5,
        {
            TDA9875_CARRIER_5_85,
            TDA9875_CARRIER_6_26,
            TDA9875_CARRIER_6_742,
            TDA9875_NOCARRIER
        }
    },
    {
        TDA9875_CARRIER_7_02,
        {
            TDA9875_CARRIER_7_20,
            TDA9875_NOCARRIER
        }
    },
    {
        TDA9875_CARRIER_7_38,
        {
            TDA9875_CARRIER_7_56,
            TDA9875_NOCARRIER
        }
    },
    {
        TDA9875_CARRIER_7_74,
        {
            TDA9875_CARRIER_7_92,
            TDA9875_NOCARRIER
        }
    },
    {
        TDA9875_CARRIER_8_10,
        {
            TDA9875_CARRIER_8_28,
            TDA9875_NOCARRIER
        }
    },
    {
        TDA9875_NOCARRIER,    
        { 
            TDA9875_NOCARRIER 
        }
    }
};

CTDA9875AudioDecoder::CTDA9875AudioDecoder(CTDA9875AudioControls* pAudioControls) : CAudioDecoder(), CTDA9875()
{
    m_IsInitialized = false;
    m_bUseInputPin1 = false;
    m_AutoDetecting = 0;
    m_TDA9875Thread = NULL;
    m_bStopThread = false;
    m_DetectInterval10ms = 20;
    m_SupportedSoundChannels = SUPPORTEDSOUNDCHANNEL_MONO;
    m_ForceAMSound = false;
    m_pAudioControls = pAudioControls;
    InitializeCriticalSection(&TDA9875CriticalSection);
}

CTDA9875AudioDecoder::~CTDA9875AudioDecoder()
{
    StopThread();
    DeleteCriticalSection(&TDA9875CriticalSection);
}

bool CTDA9875AudioDecoder::GetUseInputPin1()
{
    return m_bUseInputPin1;
}

void CTDA9875AudioDecoder::SetUseInputPin1(bool AValue)
{
    m_bUseInputPin1 = AValue;
    if (m_IsInitialized)
        SetAudioStandard(m_AudioStandard, m_VideoFormat);
}

    // Sound Channels
void CTDA9875AudioDecoder::SetSoundChannel(eSoundChannel soundChannel)
{
    
    CAudioDecoder::SetSoundChannel(soundChannel);

    EnterCriticalSection(&TDA9875CriticalSection);
    if (m_AutoDetecting == 1)
    {
        LeaveCriticalSection(&TDA9875CriticalSection);

        //Autodetect sets soundchannel if its done
        return;
    }
    LeaveCriticalSection(&TDA9875CriticalSection);

    m_IsInitialized = true;

    BYTE bMatrix = 0;

    switch(soundChannel)
    {
        case SOUNDCHANNEL_MONO:
        {
            bMatrix = 0x00;
            break;
        }
        case SOUNDCHANNEL_STEREO:
        {
            if (m_AudioStandard == TDA9875_STANDARD_M_ANALOG_FM_FM)
                bMatrix = 0x05;
            else
                bMatrix = 0x04;
            break;
        }
        case SOUNDCHANNEL_LANGUAGE1:
        {
            bMatrix = 0x02;
            break;
        }
        case SOUNDCHANNEL_LANGUAGE2:
        {
            bMatrix = 0x03;
            break;
        }

    }

    WriteToSubAddress(TDA9875_FMAT, bMatrix);
}

eSoundChannel CTDA9875AudioDecoder::GetSoundChannel()
{
    return CAudioDecoder::GetSoundChannel();
}

eSoundChannel CTDA9875AudioDecoder::IsAudioChannelDetected(eSoundChannel desiredAudioChannel)
{ 
    eSoundChannel SoundChannel = SOUNDCHANNEL_MONO;
    switch (desiredAudioChannel)
    {
    case SOUNDCHANNEL_STEREO:
        if (m_SupportedSoundChannels & SUPPORTEDSOUNDCHANNEL_STEREO)
            SoundChannel = desiredAudioChannel;
        break;
    case SOUNDCHANNEL_LANGUAGE1:
        if (m_SupportedSoundChannels & SUPPORTEDSOUNDCHANNEL_LANG1)
            SoundChannel = desiredAudioChannel;
        break;
    case SOUNDCHANNEL_LANGUAGE2:
        if (m_SupportedSoundChannels & SUPPORTEDSOUNDCHANNEL_LANG2)
            SoundChannel = desiredAudioChannel;
        break;
    }
    return SoundChannel;
}

    // Inputs
void CTDA9875AudioDecoder::SetAudioInput(eAudioInput audioInput)
{
    CAudioDecoder::SetAudioInput(audioInput);
    m_IsInitialized = true;


    switch (m_AudioInput)
    {
    case AUDIOINPUT_RADIO:
    case AUDIOINPUT_STEREO:
    case AUDIOINPUT_INTERNAL:
    case AUDIOINPUT_EXTERNAL:
    case AUDIOINPUT_MUTE:
        Audio_SetUserMute(true);
        break;
    default:
        Audio_SetUserMute(false);
        break;
    }


}
eAudioInput CTDA9875AudioDecoder::GetAudioInput()
{
    return CAudioDecoder::GetAudioInput();
}
const char* CTDA9875AudioDecoder::GetAudioInputName(eAudioInput audioInput)
{
    if (audioInput == AUDIOINPUT_RADIO)
        return "Radio not Available";

    return CAudioDecoder::GetAudioInputName(audioInput);
}

    // Standard
void CTDA9875AudioDecoder::SetAudioStandard(long Standard, eVideoFormat videoformat)
{
    CAudioDecoder::SetAudioStandard(Standard, videoformat);

    //Only for tuner
    if(m_AudioInput != AUDIOINPUT_TUNER)
    {
        return;
    }

    m_IsInitialized = true;

    int nIndex = 0;
    TStandardDefinition StandardDefinition;

    StandardDefinition.Standard = TDA9875_STANDARD_NONE; 

    // Find default carrier frequencies
    m_AudioStandardMajorCarrier = 0;
    m_AudioStandardMinorCarrier = 0;

    while (m_TDA9875Standards[nIndex].Name != NULL)
    {
        if (m_TDA9875Standards[nIndex].Standard == Standard)
        {
            StandardDefinition = m_TDA9875Standards[nIndex];
            break;
        }
        nIndex++;
    }


    if (Standard == TDA9875_STANDARD_AUTO)
    {
        // Mute
        Audio_SetUserMute(true);
        m_CarrierDetect_Phase = 0;
        m_pAudioControls->m_bKeepItMuted = true;
        return;
    }

    if (Standard == TDA9875_STANDARD_NONE)
    {
        Reset();
        return;
    }

    //De_emphasis
    WriteToSubAddress(TDA9875_DEEM, (BYTE)StandardDefinition.De_emphasis);

    BYTE DCR = 0;

    //Channel 1 mode AM, FM, SAT
    if (StandardDefinition.MonoType == MONO_AM)
        DCR |= 1;
    else if (StandardDefinition.MonoType == MONO_FM_SAT)
    {
        WriteToSubAddress(TDA9875_CFG, 1); //SIF2
        DCR |= 2;
    }
    else
        WriteToSubAddress(TDA9875_CFG, 0); //SIF1

    //Channel 2 mode AM, FM, NICAM
    if (StandardDefinition.StereoType == STEREO_AM)
        DCR |= 4;
    else if (StandardDefinition.StereoType == STEREO_NICAM)
        DCR |= 8;

    else if (StandardDefinition.StereoType == STEREO_NONE)
        SetSoundChannel(SOUNDCHANNEL_MONO);

    //Ident mode
    if (StandardDefinition.Standard == TDA9875_STANDARD_M_ANALOG_FM_FM)
        DCR |= 32;    //M stereo (Korean)

    WriteToSubAddress(TDA9875_DCR, DCR);

    //Main channel select / digital matrix
    if (StandardDefinition.StereoType == STEREO_NICAM)
    {
        WriteToSubAddress(TDA9875_MCS, 1);        //MAIN NICAM, L + R
        WriteToSubAddress(TDA9875_ACS, 1);        //AUX NICAM, L + R
        WriteToSubAddress(TDA9875_DACOS, 1);    //DAC NICAM L + R 

    }
    else
    {
        WriteToSubAddress(TDA9875_MCS, 0);        //MAIN FM, L + R
        WriteToSubAddress(TDA9875_ACS, 0);        //AUX FM, L + R
        WriteToSubAddress(TDA9875_DACOS, 0);    //DAC FM L + R 
    }

    SetAudioStandardCarriers(StandardDefinition.MajorCarrier, StandardDefinition.MinorCarrier);
}
long CTDA9875AudioDecoder::GetAudioStandardCurrent()
{
    return CAudioDecoder::GetAudioStandardCurrent();
}
const char* CTDA9875AudioDecoder::GetAudioStandardName(long Standard)
{
    if (Standard == TDA9875_STANDARD_NONE)
    {
        return "<No standard>";
    }
    if (Standard >= TDA9875_STANDARD_AUTODETECTION_IN_PROGRESS)
    {
        return "Detecting...";
    }
    int nIndex = 0;
    while (m_TDA9875Standards[nIndex].Name != NULL)
    {
        if (m_TDA9875Standards[nIndex].Standard == Standard)
        {
            return m_TDA9875Standards[nIndex].Name;
        }
        nIndex++;
    }
    return NULL;
}
int CTDA9875AudioDecoder::GetNumAudioStandards()
{
    return ((sizeof(m_TDA9875Standards)/sizeof(TStandardDefinition)));
}
long CTDA9875AudioDecoder::GetAudioStandard(int nIndex)
{
    if ((nIndex < 0) || (nIndex >= GetNumAudioStandards()) )
    {
        return 0;
    }
    if (nIndex == 0)
    {
        return TDA9875_STANDARD_NONE;
    }
    return m_TDA9875Standards[nIndex-1].Standard;
}
long CTDA9875AudioDecoder::GetAudioStandardMajorCarrier(long Standard)
{
    if (Standard < 0) //Current
    {
        if (m_AudioStandardMajorCarrier == 0)
        {
            Standard = m_AudioStandard; //default carrier
        }
        else
        {
            return m_AudioStandardMajorCarrier;
        }
    }
    if (Standard == TDA9875_STANDARD_NONE)
    {
        return 0; //No standard
    }
    int nIndex = 0;
    while (m_TDA9875Standards[nIndex].Name != NULL)
    {
        if (m_TDA9875Standards[nIndex].Standard == Standard)
        {
            return m_TDA9875Standards[nIndex].MajorCarrier;
        }
        nIndex++;
    }
    return 0;

}
long CTDA9875AudioDecoder::GetAudioStandardMinorCarrier(long Standard)
{
    if (Standard < 0) //Current
    {
        if (m_AudioStandardMinorCarrier == 0)
        {
            Standard = m_AudioStandard; //default carrier
        }
        else
        {
            return m_AudioStandardMinorCarrier;
        }
    }
    if (Standard == TDA9875_STANDARD_NONE)
    {
        return 0; //No standard
    }
    int nIndex = 0;
    while (m_TDA9875Standards[nIndex].Name != NULL)
    {
        if (m_TDA9875Standards[nIndex].Standard == Standard)
        {
            return m_TDA9875Standards[nIndex].MinorCarrier;
        }
        nIndex++;
    }
    return 0;

}
void CTDA9875AudioDecoder::SetAudioStandardCarriersTDA9875(long MajorCarrier, long MinorCarrier)
{
    DWORD dwC2 = (DWORD)TDA9875_CARRIER(MinorCarrier);
    DWORD dwC1 = (DWORD)TDA9875_CARRIER(MajorCarrier);

    WriteToSubAddress(TDA9875_C1MSB, (BYTE)((dwC1 & 0xff0000) >> 16));
    WriteToSubAddress(TDA9875_C1MIB, (BYTE)((dwC1 & 0x00ff00) >> 8));
    WriteToSubAddress(TDA9875_C1LSB, (BYTE)(dwC1 & 0x0000ff));

    WriteToSubAddress(TDA9875_C2MSB, (BYTE)((dwC2 & 0xff0000) >> 16));
    WriteToSubAddress(TDA9875_C2MIB, (BYTE)((dwC2 & 0x00ff00) >> 8));
    WriteToSubAddress(TDA9875_C2LSB, (BYTE)(dwC2 & 0x0000ff));

}
void CTDA9875AudioDecoder::SetAudioStandardCarriers(long MajorCarrier, long MinorCarrier)
{
    if ((MajorCarrier != 0) && (MinorCarrier!=0) &&
        ((MajorCarrier != m_AudioStandardMajorCarrier) || (MinorCarrier != m_AudioStandardMinorCarrier)))
    {
        SetAudioStandardCarriersTDA9875(MajorCarrier, MinorCarrier);
    }
    CAudioDecoder::SetAudioStandardCarriers(MajorCarrier, MinorCarrier);

}
long CTDA9875AudioDecoder::GetAudioStandardFromVideoFormat(eVideoFormat videoFormat)
{
    /// Guess the correct format
    WORD standard = TDA9875_STANDARD_NONE;
    switch(videoFormat)
    {
    case VIDEOFORMAT_PAL_B:
        standard = TDA9875_STANDARD_BG_ANALOG_FM_FM;
        break;
    case VIDEOFORMAT_PAL_D:
        standard = TDA9875_STANDARD_DK1_ANALOG_FM_FM;
        break;
    case VIDEOFORMAT_PAL_G:
        standard = TDA9875_STANDARD_BG_ANALOG_FM_FM;
        break;
    case VIDEOFORMAT_PAL_H:
        // \todo FIXME
        standard = TDA9875_STANDARD_NONE;
        break;
    case VIDEOFORMAT_PAL_I:
        standard = TDA9875_STANDARD_I_ANALOG_FM_NONE;
        break;
    case VIDEOFORMAT_PAL_M:
        // \todo FIXME
        standard = TDA9875_STANDARD_M_ANALOG_FM_NONE;
        break;
    case VIDEOFORMAT_PAL_N:
        // \todo FIXME
        standard = TDA9875_STANDARD_NONE;
        break;
    case VIDEOFORMAT_PAL_N_COMBO:
        // \todo FIXME
        standard = TDA9875_STANDARD_NONE;
        break;
    case VIDEOFORMAT_SECAM_B:
        standard = TDA9875_STANDARD_BG_ANALOG_FM_FM;
        break;
    case VIDEOFORMAT_SECAM_D:
        standard = TDA9875_STANDARD_DK1_ANALOG_FM_FM;
        break;
    case VIDEOFORMAT_SECAM_G:
        standard = TDA9875_STANDARD_BG_ANALOG_FM_FM;
        break;
    case VIDEOFORMAT_SECAM_H:
        standard = TDA9875_STANDARD_DK1_ANALOG_FM_FM;
        break;
    case VIDEOFORMAT_SECAM_K:
        standard = TDA9875_STANDARD_DK1_ANALOG_FM_FM;
        break;
    case VIDEOFORMAT_SECAM_K1:
        standard = TDA9875_STANDARD_DK2_ANALOG_FM_FM;
        break;
    case VIDEOFORMAT_SECAM_L:
        standard = TDA9875_STANDARD_L_DIGITAL_AM_NICAM;
        break;
    case VIDEOFORMAT_SECAM_L1:
        standard = TDA9875_STANDARD_L_DIGITAL_AM_NICAM;
        break;
    default:
    case VIDEOFORMAT_PAL_60:
    case VIDEOFORMAT_NTSC_50:
        standard = TDA9875_STANDARD_NONE;
        break;
    }
    return standard;

}

void CTDA9875AudioDecoder::DetectAudioStandard(long Interval, int SupportedSoundChannels, eSoundChannel TargetChannel)
{
    CAudioDecoder::DetectAudioStandard(Interval, SupportedSoundChannels, TargetChannel);

    if (Interval > 0)
    {                
        m_DetectInterval10ms = (Interval/10);
    }
    else
    {
        //Abort
        StopThread();    
        return;
    }

    if (SupportedSoundChannels)
    {
        m_TargetSoundChannel = TargetChannel;
    }

    EnterCriticalSection(&TDA9875CriticalSection);    
    int AutoDetecting = m_AutoDetecting;       
    LeaveCriticalSection(&TDA9875CriticalSection);

    // Stop thread if detecting audio standard
    // Suspend thread if detecting stereo
    if (AutoDetecting == 1)
    {
        // Mute
        StopThread();
        
    }
    else if (AutoDetecting == 2)
    {
        EnterCriticalSection(&TDA9875CriticalSection);    
        m_ThreadWait = TRUE;
        LeaveCriticalSection(&TDA9875CriticalSection);
        Sleep(10);
        
        EnterCriticalSection(&TDA9875CriticalSection);    
        BOOL bSuspended = (m_AutoDetecting == 0);
        LeaveCriticalSection(&TDA9875CriticalSection);

        if (!bSuspended)
        {            
            Sleep(50);
            EnterCriticalSection(&TDA9875CriticalSection);    
            bSuspended = (m_AutoDetecting == 0);
            LeaveCriticalSection(&TDA9875CriticalSection);
            
            if (!bSuspended)
            {
                StopThread();
            }
        }
        Sleep(1);   
    }

    if (SupportedSoundChannels == 1)
    {
        SetAudioStandard(TDA9875_STANDARD_AUTO, m_VideoFormat);
    }

    EnterCriticalSection(&TDA9875CriticalSection);        
    m_DetectCounter = 1;
    Sleep(10);
    if (SupportedSoundChannels == 2)
    {
        m_AutoDetecting = 2;
    }
    else
    {
        m_AutoDetecting = 1;
    }
    m_DetectSupportedSoundChannels = (SupportedSoundChannels == 1);
    SetSoundChannel(SOUNDCHANNEL_MONO);
    m_ThreadWait = FALSE;
    LeaveCriticalSection(&TDA9875CriticalSection);
    //Start or resume thread
    StartThread();

}

CAudioDecoder::eAudioDecoderType CTDA9875AudioDecoder::GetAudioDecoderType()
{
    return CAudioDecoder::AUDIODECODERTYPE_TDA9875;
}

#define CARRIER_OFFSET_25KHZ (25000)
#define CARRIER_OFFSET_15KHZ (15000)
#define CARRIER_OFFSET_5KHZ (5000)

CTDA9875AudioDecoder::eStandard CTDA9875AudioDecoder::DetectStandardTDA9875()
{

    static int nMajorIndex;
    static int nMinorIndex;
    static int a;
    static int c;
    static int d;


    short int val;
    double k0, k1, u0;
    bool condition1, condition2;
    BYTE result[2] = {0, 0};
    eStandard Standard = TDA9875_STANDARD_NONE;

    if (m_CarrierDetect_Phase == 0) //Setup
    {
        nMajorIndex = 0;
        nMinorIndex = 0;

        if (m_ForceAMSound && IsSECAMVideoFormat(m_VideoFormat))
            return TDA9875_STANDARD_L_DIGITAL_AM_NICAM;

        WriteToSubAddress(TDA9875_MSR, 8); //channel 1
        m_CarrierDetect_Phase = 1;
    }
    //Major carrier
    if (m_CarrierDetect_Phase == 1) //Major carrier detection
    {
        if (CarrierDetectTable[nMajorIndex].Major == TDA9875_NOCARRIER)
            return TDA9875_STANDARD_NONE;

        //Left
        //25Khz left to the carrier
        SetAudioStandardCarriersTDA9875(
            CarrierDetectTable[nMajorIndex].Major - CARRIER_OFFSET_25KHZ, 
            CarrierDetectTable[nMajorIndex].Major - CARRIER_OFFSET_25KHZ
            );

        m_CarrierDetect_Phase = 2;
        return TDA9875_STANDARD_AUTODETECTION_IN_PROGRESS;
    }
    if (m_CarrierDetect_Phase == 2) //Major carrier detection
    {
        ReadFromSubAddress(TDA9875_L1MSB, result, sizeof(result));
        val = ((short int)result[0]) << 8 | result[1];
        a = val;
        c = (-CARRIER_OFFSET_25KHZ)*val;
        d = CARRIER_OFFSET_25KHZ*CARRIER_OFFSET_25KHZ;
        LOG(1,"TDA9875: Major carrier = %d Hz, 25KHz left = %d",CarrierDetectTable[nMajorIndex].Major,val);

        //15Khz left to the carrier
        SetAudioStandardCarriersTDA9875(
            CarrierDetectTable[nMajorIndex].Major - CARRIER_OFFSET_15KHZ, 
            CarrierDetectTable[nMajorIndex].Major - CARRIER_OFFSET_15KHZ
            );

        m_CarrierDetect_Phase = 3;
        return TDA9875_STANDARD_AUTODETECTION_IN_PROGRESS;
    }
    if (m_CarrierDetect_Phase == 3) //Major carrier detection
    {
        ReadFromSubAddress(TDA9875_L1MSB, result, sizeof(result));
        val = ((short int)result[0]) << 8 | result[1];
        a += val;
        c += (-CARRIER_OFFSET_15KHZ)*val;
        d += CARRIER_OFFSET_15KHZ*CARRIER_OFFSET_15KHZ;
        LOG(1,"TDA9875: Major carrier = %d Hz, 15KHz left = %d",CarrierDetectTable[nMajorIndex].Major,val);

        //5Khz left to the carrier
        SetAudioStandardCarriersTDA9875(
            CarrierDetectTable[nMajorIndex].Major - CARRIER_OFFSET_5KHZ, 
            CarrierDetectTable[nMajorIndex].Major - CARRIER_OFFSET_5KHZ
            );

        m_CarrierDetect_Phase = 4;
        return TDA9875_STANDARD_AUTODETECTION_IN_PROGRESS;
    }
    if (m_CarrierDetect_Phase == 4) //Major carrier detection
    {
        ReadFromSubAddress(TDA9875_L1MSB, result, sizeof(result));
        val = ((short int)result[0]) << 8 | result[1];
        a += val;
        c += (-CARRIER_OFFSET_5KHZ)*val;
        d += CARRIER_OFFSET_5KHZ*CARRIER_OFFSET_5KHZ;
        LOG(1,"TDA9875: Major carrier = %d Hz, 5KHz left = %d",CarrierDetectTable[nMajorIndex].Major,val);

        //5Khz right from the carrier
        SetAudioStandardCarriersTDA9875(
            CarrierDetectTable[nMajorIndex].Major + CARRIER_OFFSET_5KHZ, 
            CarrierDetectTable[nMajorIndex].Major + CARRIER_OFFSET_5KHZ
            );

        m_CarrierDetect_Phase = 5;
        return TDA9875_STANDARD_AUTODETECTION_IN_PROGRESS;
    }
    if (m_CarrierDetect_Phase == 5) //Major carrier detection
    {
        ReadFromSubAddress(TDA9875_L1MSB, result, sizeof(result));
        val = ((short int)result[0]) << 8 | result[1];
        a += val;
        c += CARRIER_OFFSET_5KHZ*val;
        d += CARRIER_OFFSET_5KHZ*CARRIER_OFFSET_5KHZ;
        LOG(1,"TDA9875: Major carrier = %d Hz, 5KHz right = %d",CarrierDetectTable[nMajorIndex].Major,val);

        //15Khz right from the carrier
        SetAudioStandardCarriersTDA9875(
            CarrierDetectTable[nMajorIndex].Major + CARRIER_OFFSET_15KHZ, 
            CarrierDetectTable[nMajorIndex].Major + CARRIER_OFFSET_15KHZ
            );

        m_CarrierDetect_Phase = 6;
        return TDA9875_STANDARD_AUTODETECTION_IN_PROGRESS;
    }
    if (m_CarrierDetect_Phase == 6) //Major carrier detection
    {
        ReadFromSubAddress(TDA9875_L1MSB, result, sizeof(result));
        val = ((short int)result[0]) << 8 | result[1];
        a += val;
        c += CARRIER_OFFSET_15KHZ*val;
        d += CARRIER_OFFSET_15KHZ*CARRIER_OFFSET_15KHZ;
        LOG(1,"TDA9875: Major carrier = %d Hz, 15KHz right = %d",CarrierDetectTable[nMajorIndex].Major,val);

        //25Khz right from the carrier
        SetAudioStandardCarriersTDA9875(
            CarrierDetectTable[nMajorIndex].Major + CARRIER_OFFSET_25KHZ, 
            CarrierDetectTable[nMajorIndex].Major + CARRIER_OFFSET_25KHZ
            );

        m_CarrierDetect_Phase = 7;
        return TDA9875_STANDARD_AUTODETECTION_IN_PROGRESS;
    }
    if (m_CarrierDetect_Phase == 7) //Major carrier detection
    {
        ReadFromSubAddress(TDA9875_L1MSB, result, sizeof(result));
        val = ((short int)result[0]) << 8 | result[1];
        a += val;
        c += CARRIER_OFFSET_25KHZ*val;
        d += CARRIER_OFFSET_25KHZ*CARRIER_OFFSET_25KHZ;
        LOG(1,"TDA9875: Major carrier = %d Hz, 25KHz right = %d",CarrierDetectTable[nMajorIndex].Major,val);
        
        k0 = a/6.0;
        k1 = ((double)c)/d;
        if (k1 != 0)
            u0 = - k0/k1;
        else
            u0 = CARRIER_OFFSET_25KHZ + 1; //out of range
        
        condition1 = ((u0 > -CARRIER_OFFSET_25KHZ) && (u0 < CARRIER_OFFSET_25KHZ));
        condition2 = (k1 > 0.05);

        if (condition1 && condition2) //found
        {
            //Setup
            LOG(1,"TDA9875: Found Major carrier = %d Hz",CarrierDetectTable[nMajorIndex].Major);
            m_CarrierDetect_Phase = 8; // Detect minor carrier
            
            //Minor Carrier
            WriteToSubAddress(TDA9875_MSR, 16); // channel 2
        }
        else
        {
            m_CarrierDetect_Phase = 1;
            nMajorIndex++;
            return TDA9875_STANDARD_AUTODETECTION_IN_PROGRESS;
        }
    }
    //Minor carrier
    if (m_CarrierDetect_Phase == 8) //Minor carrier detection
    {

        if (CarrierDetectTable[nMajorIndex].Minor[nMinorIndex] == TDA9875_NOCARRIER)
        {
            m_CarrierDetect_Phase = 15; //Find standard
        }
        else
        {
            //Left
            //25Khz left to the carrier
            SetAudioStandardCarriersTDA9875(
                CarrierDetectTable[nMajorIndex].Minor[nMinorIndex] - CARRIER_OFFSET_25KHZ, 
                CarrierDetectTable[nMajorIndex].Minor[nMinorIndex] - CARRIER_OFFSET_25KHZ
                );

            m_CarrierDetect_Phase = 9;
            return TDA9875_STANDARD_AUTODETECTION_IN_PROGRESS;
        }
    }
    if (m_CarrierDetect_Phase == 9) //Minor carrier detection
    {
        ReadFromSubAddress(TDA9875_L1MSB, result, sizeof(result));
        val = ((short int)result[0]) << 8 | result[1];
        a = val;
        c = (-CARRIER_OFFSET_25KHZ)*val;
        d = CARRIER_OFFSET_25KHZ*CARRIER_OFFSET_25KHZ;
        LOG(1,"TDA9875: Minor carrier = %d Hz, 25KHz left = %d",CarrierDetectTable[nMajorIndex].Minor[nMinorIndex],val);

        //15Khz left to the carrier
        SetAudioStandardCarriersTDA9875(
            CarrierDetectTable[nMajorIndex].Minor[nMinorIndex] - CARRIER_OFFSET_15KHZ, 
            CarrierDetectTable[nMajorIndex].Minor[nMinorIndex] - CARRIER_OFFSET_15KHZ
            );

        m_CarrierDetect_Phase = 10;
        return TDA9875_STANDARD_AUTODETECTION_IN_PROGRESS;
    }
    if (m_CarrierDetect_Phase == 10) //Minor carrier detection
    {
        ReadFromSubAddress(TDA9875_L1MSB, result, sizeof(result));
        val = ((short int)result[0]) << 8 | result[1];
        a += val;
        c += (-CARRIER_OFFSET_15KHZ)*val;
        d += CARRIER_OFFSET_15KHZ*CARRIER_OFFSET_15KHZ;
        LOG(1,"TDA9875: Minor carrier = %d Hz, 15KHz left = %d",CarrierDetectTable[nMajorIndex].Minor[nMinorIndex],val);

        //5Khz left to the carrier
        SetAudioStandardCarriersTDA9875(
            CarrierDetectTable[nMajorIndex].Minor[nMinorIndex] - CARRIER_OFFSET_5KHZ, 
            CarrierDetectTable[nMajorIndex].Minor[nMinorIndex] - CARRIER_OFFSET_5KHZ
            );

        m_CarrierDetect_Phase = 11;
        return TDA9875_STANDARD_AUTODETECTION_IN_PROGRESS;
    }
    if (m_CarrierDetect_Phase == 11) //Minor carrier detection
    {
        ReadFromSubAddress(TDA9875_L1MSB, result, sizeof(result));
        val = ((short int)result[0]) << 8 | result[1];
        a += val;
        c += (-CARRIER_OFFSET_5KHZ)*val;
        d += CARRIER_OFFSET_5KHZ*CARRIER_OFFSET_5KHZ;
        LOG(1,"TDA9875: Minor carrier = %d Hz, 5KHz left = %d",CarrierDetectTable[nMajorIndex].Minor[nMinorIndex],val);

        //Right
        //5Khz right from the carrier
        SetAudioStandardCarriersTDA9875(
            CarrierDetectTable[nMajorIndex].Minor[nMinorIndex] + CARRIER_OFFSET_5KHZ, 
            CarrierDetectTable[nMajorIndex].Minor[nMinorIndex] + CARRIER_OFFSET_5KHZ
            );
        m_CarrierDetect_Phase = 12;
        return TDA9875_STANDARD_AUTODETECTION_IN_PROGRESS;
    }
    if (m_CarrierDetect_Phase == 12) //Minor carrier detection
    {
        ReadFromSubAddress(TDA9875_L1MSB, result, sizeof(result));
        val = ((short int)result[0]) << 8 | result[1];
        a += val;
        c += CARRIER_OFFSET_5KHZ*val;
        d += CARRIER_OFFSET_5KHZ*CARRIER_OFFSET_5KHZ;
        LOG(1,"TDA9875: Minor carrier = %d Hz, 5KHz right = %d",CarrierDetectTable[nMajorIndex].Minor[nMinorIndex],val);

        //15Khz right from the carrier
        SetAudioStandardCarriersTDA9875(
            CarrierDetectTable[nMajorIndex].Minor[nMinorIndex] + CARRIER_OFFSET_15KHZ, 
            CarrierDetectTable[nMajorIndex].Minor[nMinorIndex] + CARRIER_OFFSET_15KHZ
            );

        m_CarrierDetect_Phase = 13;
        return TDA9875_STANDARD_AUTODETECTION_IN_PROGRESS;
    }
    if (m_CarrierDetect_Phase == 13) //Minor carrier detection
    {
        ReadFromSubAddress(TDA9875_L1MSB, result, sizeof(result));
        val = ((short int)result[0]) << 8 | result[1];
        a += val;
        c += CARRIER_OFFSET_15KHZ*val;
        d += CARRIER_OFFSET_15KHZ*CARRIER_OFFSET_15KHZ;
        LOG(1,"TDA9875: Minor carrier = %d Hz, 15KHz right = %d",CarrierDetectTable[nMajorIndex].Minor[nMinorIndex],val);
        
        //25Khz right from the carrier
        SetAudioStandardCarriersTDA9875(
            CarrierDetectTable[nMajorIndex].Minor[nMinorIndex] + CARRIER_OFFSET_25KHZ, 
            CarrierDetectTable[nMajorIndex].Minor[nMinorIndex] + CARRIER_OFFSET_25KHZ
            );

        m_CarrierDetect_Phase = 14;
        return TDA9875_STANDARD_AUTODETECTION_IN_PROGRESS;
    }
    if (m_CarrierDetect_Phase == 14) //Minor carrier detection
    {
        ReadFromSubAddress(TDA9875_L1MSB, result, sizeof(result));
        val = ((short int)result[0]) << 8 | result[1];
        a += val;
        c += CARRIER_OFFSET_25KHZ*val;
        d += CARRIER_OFFSET_25KHZ*CARRIER_OFFSET_25KHZ;
        LOG(1,"TDA9875: Minor carrier = %d Hz, 25KHz right = %d",CarrierDetectTable[nMajorIndex].Minor[nMinorIndex],val);

        k0 = a/6.0;
        k1 = ((double)c)/d;
        if (k1 != 0)
            u0 = - k0/k1;
        else
            u0 = CARRIER_OFFSET_25KHZ + 1; //out of range
        
        condition1 = ((u0 > -CARRIER_OFFSET_25KHZ) && (u0 < CARRIER_OFFSET_25KHZ));
        condition2 = (k1 > 0.05);

        if (condition1 && condition2) //found
        {
            LOG(1,"TDA9875: Found Minor carrier = %d Hz",CarrierDetectTable[nMajorIndex].Minor[nMinorIndex]);
            m_CarrierDetect_Phase = 15; // Find standard
        }
        else
        {
            m_CarrierDetect_Phase = 8;
            nMinorIndex++;
            return TDA9875_STANDARD_AUTODETECTION_IN_PROGRESS;
        }
    }

    //Find standard
    if (m_CarrierDetect_Phase == 15)
    {
        eCarrier MajorCarrier;
        eCarrier MinorCarrier;

        MajorCarrier = MinorCarrier = CarrierDetectTable[nMajorIndex].Major;
        if (CarrierDetectTable[nMajorIndex].Minor[nMinorIndex] != TDA9875_NOCARRIER)
            MinorCarrier = CarrierDetectTable[nMajorIndex].Minor[nMinorIndex];

        int n = 0;
        while (m_TDA9875Standards[n].Name != NULL)
        {
            if (m_TDA9875Standards[n].MajorCarrier == MajorCarrier)
            {
                Standard = m_TDA9875Standards[n].Standard;
                if (MajorCarrier == MinorCarrier)
                    break;

                if (m_TDA9875Standards[n].MinorCarrier == MinorCarrier)
                {
                    Standard = m_TDA9875Standards[n].Standard;
                    break;
                }
            }

            n++;
        }

        if ((MajorCarrier == TDA9875_CARRIER_4_5) && (MinorCarrier == TDA9875_CARRIER_4_5))
            Standard = TDA9875_STANDARD_M_ANALOG_FM_NONE;
        //Solve DK_FM_NICAM vs L_AM_NICAM ambiguity
        else if ((MajorCarrier == TDA9875_CARRIER_6_5) && (MinorCarrier == TDA9875_CARRIER_5_85))
        {
            Standard = TDA9875_STANDARD_DK_DIGITAL_FM_NICAM;
            if (IsSECAMVideoFormat(m_VideoFormat))
                Standard = TDA9875_STANDARD_L_DIGITAL_AM_NICAM;
        }
        else if ((MajorCarrier == TDA9875_CARRIER_6_5) && (MinorCarrier == TDA9875_CARRIER_6_5))
            Standard = TDA9875_STANDARD_SAT_FM_MONO;

        return Standard;
    }
    
    return Standard;
}

long CTDA9875AudioDecoder::GetAudioDecoderValue(int What)
{
    if (What==0)
    {
        long Flags = 0;
        if (m_ForceAMSound)
        {
            Flags |= 0x08;
        }
        return Flags;
    }
    return 0;
}

int CTDA9875AudioDecoder::SetAudioDecoderValue(int What, long Val)
{
    if (What==0)
    {
        long Flags = Val;
        m_IsInitialized = true;
        m_ForceAMSound = ((Flags & 0x08)!=0);
        return 1;
    }
    return 0;

}

DWORD WINAPI TDA9875ThreadProc(LPVOID lpThreadParameter)
{
    DScalerThread("TDA9875DetectThread");
    if (lpThreadParameter != NULL)
    {
        int Result;
        try
        {
            Result = ((CTDA9875AudioDecoder*)lpThreadParameter)->DetectThread();
        }
        catch(...)
        {
            return 1;
        }
        return Result;
    }
    return 1;
}

void CTDA9875AudioDecoder::StartThread()
{
    DWORD LinkThreadID;

    if (m_TDA9875Thread != NULL)
    {
        // Already started, resume
        ResumeThread(m_TDA9875Thread);
        return;
    }

    m_bStopThread = false;

    m_TDA9875Thread = CreateThread((LPSECURITY_ATTRIBUTES) NULL,  // No security.
                             (DWORD) 0,                     // Same stack size.
                             TDA9875ThreadProc,                  // Thread procedure.
                             (LPVOID)this,                          // Parameter.
                             (DWORD) 0,                     // Start immediatly.
                             (LPDWORD) & LinkThreadID);     // Thread ID.
}

void CTDA9875AudioDecoder::StopThread()
{
    DWORD ExitCode;
    int i;
    bool Thread_Stopped = false;

    if (m_TDA9875Thread != NULL)
    {
        i = 10;
        EnterCriticalSection(&TDA9875CriticalSection); 
        m_bStopThread = true;
        LeaveCriticalSection(&TDA9875CriticalSection); 
        ResumeThread(m_TDA9875Thread);
        while(i-- > 0 && !Thread_Stopped)
        {
            if (GetExitCodeThread(m_TDA9875Thread, &ExitCode) == TRUE)
            {
                if (ExitCode != STILL_ACTIVE)
                {                    
                    Thread_Stopped = true;
                }
                else
                {                    
                    Sleep(50);
                }
            }
            else
            {
                Thread_Stopped = true;
            }
        }

        if (Thread_Stopped == false)
        {
            TerminateThread(m_TDA9875Thread, 0);
            Sleep(50);
        }
        CloseHandle(m_TDA9875Thread);
        m_TDA9875Thread = NULL;
    }
}

int CTDA9875AudioDecoder::DetectThread()
{
    int AutoDetecting;
    BOOL bWait;

    EnterCriticalSection(&TDA9875CriticalSection); 
    m_DetectCounter = 1;
    bool bStopThread = m_bStopThread;
    LeaveCriticalSection(&TDA9875CriticalSection); 
    while (!bStopThread)
    {
        EnterCriticalSection(&TDA9875CriticalSection);
        bWait = m_ThreadWait;
        LeaveCriticalSection(&TDA9875CriticalSection);
        if (bWait)
        {
            EnterCriticalSection(&TDA9875CriticalSection);
            m_AutoDetecting = 0;
            LeaveCriticalSection(&TDA9875CriticalSection);
            LOG(1, "TDA9875: Suspending thread");
            SuspendThread(m_TDA9875Thread);
        }

        EnterCriticalSection(&TDA9875CriticalSection);
        AutoDetecting = m_AutoDetecting; 
        long DetectCounter = m_DetectCounter;
        LeaveCriticalSection(&TDA9875CriticalSection);

        // Override detect interval, always use 100 ms interval
        if ((AutoDetecting==1) &&
            ((DetectCounter%10)==0)) //Detect standard
        {

            EnterCriticalSection(&TDA9875CriticalSection);
            m_AudioStandard = TDA9875_STANDARD_AUTODETECTION_IN_PROGRESS;
            LeaveCriticalSection(&TDA9875CriticalSection);

            eStandard standard = DetectStandardTDA9875();
            if (standard != TDA9875_STANDARD_AUTODETECTION_IN_PROGRESS)
            {
                EventCollector->RaiseEvent(this,EVENT_AUDIOSTANDARD_DETECTED, 0, standard);

                EnterCriticalSection(&TDA9875CriticalSection);
                m_SupportedSoundChannels = SUPPORTEDSOUNDCHANNEL_MONO;
                m_AudioStandard = standard;
                SetAudioStandard(standard, m_VideoFormat);
                LeaveCriticalSection(&TDA9875CriticalSection);

                m_pAudioControls->m_bKeepItMuted = false;
                Audio_SetUserMute(false);

                EnterCriticalSection(&TDA9875CriticalSection);
                if (m_DetectSupportedSoundChannels)
                {
                    m_AutoDetecting = AutoDetecting = 2; //Try to find stereo
                    LeaveCriticalSection(&TDA9875CriticalSection);
                }
                else
                {
                    m_AutoDetecting = AutoDetecting = 0; //Finished
                    SetSoundChannel(m_SoundChannel);
                    LeaveCriticalSection(&TDA9875CriticalSection);
                }
            }
            EnterCriticalSection(&TDA9875CriticalSection);
            DetectCounter = m_DetectCounter = 1;            
            LeaveCriticalSection(&TDA9875CriticalSection);

        }

        EnterCriticalSection(&TDA9875CriticalSection);
        AutoDetecting = m_AutoDetecting;
        if (AutoDetecting == 0)
            m_ThreadWait = TRUE;
        LeaveCriticalSection(&TDA9875CriticalSection);


        if (AutoDetecting==2) //Detect mono/stereo/lang1/lang2
        {
            EnterCriticalSection(&TDA9875CriticalSection);
            DetectCounter = m_DetectCounter;
            LeaveCriticalSection(&TDA9875CriticalSection);

            if ((DetectCounter%m_DetectInterval10ms) == 0)
            {

                eSupportedSoundChannels Supported = SUPPORTEDSOUNDCHANNEL_MONO;
                int iSupported = (int)Supported;

                BYTE result[1];

                EnterCriticalSection(&TDA9875CriticalSection);
                ReadFromSubAddress(TDA9875_STA, 0, 0, result, sizeof(result));
                LeaveCriticalSection(&TDA9875CriticalSection);

                if ((result[0] & 0x02) == 0x02)
                    iSupported |= SUPPORTEDSOUNDCHANNEL_STEREO;
                if ((result[0] & 0x04) == 0x04) 
                    iSupported |= SUPPORTEDSOUNDCHANNEL_LANG1;
                if ((result[0] & 0x06) == 0x06) 
                    iSupported |= SUPPORTEDSOUNDCHANNEL_LANG1 | SUPPORTEDSOUNDCHANNEL_LANG2;

                Supported = (eSupportedSoundChannels)iSupported;

                EnterCriticalSection(&TDA9875CriticalSection);
                eSupportedSoundChannels SupportedSoundChannels = m_SupportedSoundChannels;
                DetectCounter = m_DetectCounter;
                LeaveCriticalSection(&TDA9875CriticalSection);
                if ((Supported != SupportedSoundChannels) || (DetectCounter >= 200))
                {
                    EnterCriticalSection(&TDA9875CriticalSection);
                    m_SupportedSoundChannels = Supported;
                    LeaveCriticalSection(&TDA9875CriticalSection);

                    EventCollector->RaiseEvent(this, EVENT_AUDIOCHANNELSUPPORT_DETECTED, 0, Supported);

                    EnterCriticalSection(&TDA9875CriticalSection);
                    m_SoundChannel = IsAudioChannelDetected(m_TargetSoundChannel);
                    SetSoundChannel(m_SoundChannel);
                    LeaveCriticalSection(&TDA9875CriticalSection);
                    

                    //Finished
                    if (DetectCounter >= 200)
                    {
                        EnterCriticalSection(&TDA9875CriticalSection);
                        m_AutoDetecting = 0;                        
                        m_ThreadWait = TRUE;
                        LeaveCriticalSection(&TDA9875CriticalSection);
                        AutoDetecting = 0;
                    }
                }
            }
        }

        EnterCriticalSection(&TDA9875CriticalSection);
        bWait = m_ThreadWait;
        if (bWait)
        {
            AutoDetecting = 0;
        }
        LeaveCriticalSection(&TDA9875CriticalSection);

        if (AutoDetecting != 0)
        {
            Sleep(10);
            EnterCriticalSection(&TDA9875CriticalSection); 
            m_DetectCounter++;
            LeaveCriticalSection(&TDA9875CriticalSection); 
        } 
        EnterCriticalSection(&TDA9875CriticalSection); 
        bStopThread = m_bStopThread;
        LeaveCriticalSection(&TDA9875CriticalSection); 
    }
    return 0;
}