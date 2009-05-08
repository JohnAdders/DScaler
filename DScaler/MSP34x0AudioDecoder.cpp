//
// $Id$
//
/////////////////////////////////////////////////////////////////////////////
//
// copyleft 2002 itt@myself.com
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
 * @file MSP34x0AudioDecoder.cpp CMSP34x0AudioDecoder Implementation
 */

#include "stdafx.h"

#ifdef WANT_BT8X8_SUPPORT

#include "MSP34x0.h"
#include "MSP34x0AudioDecoder.h"
#include "Providers.h"
#include "DebugLog.h"
#include "Crash.h"

#define MSP_UNCARRIER(carrier) ((double)(carrier)*18.432/(double)(1<<24))
#define MSP_CARRIER_HZ(freq) ((int)((double)(freq/18432000.0)*(1<<24)))
#define MSP_UNCARRIER_HZ(carrier) ((long)((double)(carrier)*18432000.0/double(1<<24)))

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMSP34x0AudioDecoder::CMSP34x0AudioDecoder() : CAudioDecoder(), CMSP34x0()
{
    m_IsInitialized = FALSE;
    m_bHasEqualizer = FALSE;
    m_bHasDolby = FALSE;
    m_bUseInputPin1 = FALSE;
    m_ForceAMSound = FALSE;

    m_MSP34xxThread = NULL;
    m_bStopThread = FALSE;
    m_AutoDetecting = 0;

    m_ForceVersionA = FALSE;
    m_ForceVersionD = FALSE;
    m_ForceVersionG = FALSE;
    m_ForceHasEqualizer = FALSE;

    m_KeepWatchingStereo = FALSE;

    m_SupportedSoundChannels = SUPPORTEDSOUNDCHANNEL_NONE;

    m_DetectInterval10ms = 20; //Detect audio standard in steps of 200 milliseconds

    InitializeCriticalSection(&MSP34xxCriticalSection);
}

CMSP34x0AudioDecoder::~CMSP34x0AudioDecoder()
{
    StopThread();
    DeleteCriticalSection(&MSP34xxCriticalSection);
}

////
BOOL CMSP34x0AudioDecoder::GetUseInputPin1()
{
    return m_bUseInputPin1;
}

void CMSP34x0AudioDecoder::SetUseInputPin1(BOOL AValue)
{
    m_bUseInputPin1 = AValue;
    if(m_IsInitialized)
    {
        SetAudioStandard(m_AudioStandard, m_VideoFormat);
    }
}

////
CMSP34x0AudioDecoder::TStandardDefinition CMSP34x0AudioDecoder::m_MSPStandards[] =
{
    {
        _T("M-Dual FM-Stereo"),
        MSP34x0_STANDARD_M_DUAL_FM,
        MSP34x0_CARRIER_4_5,
        MSP34x0_CARRIER_4_724212,
        MONO_FM,
        STEREO_FM,
        FIR_BG_DK_DUAL_FM,
    },
    {
        _T("B/G-Dual FM-Stereo"),
        MSP34x0_STANDARD_BG_DUAL_FM,
        MSP34x0_CARRIER_5_5,
        MSP34x0_CARRIER_5_7421875,
        MONO_FM,
        STEREO_FM,
        FIR_BG_DK_DUAL_FM,
    },
    {
        _T("D/K1-Dual FM-Stereo"),
        MSP34x0_STANDARD_DK1_DUAL_FM,
        MSP34x0_CARRIER_6_5,
        MSP34x0_CARRIER_6_2578125,
        MONO_FM,
        STEREO_FM,
        FIR_BG_DK_DUAL_FM,
    },
    {
        _T("D/K2-Dual FM-Stereo"),
        MSP34x0_STANDARD_DK2_DUAL_FM,
        MSP34x0_CARRIER_6_5,
        MSP34x0_CARRIER_6_7421875,
        MONO_FM,
        STEREO_FM,
        FIR_BG_DK_DUAL_FM,
    },
    {
        _T("D/K-FM-Mono with HDEV3"),
        MSP34x0_STANDARD_DK_FM_MONO,
        MSP34x0_CARRIER_6_5,
        MSP34x0_CARRIER_6_5,
        MONO_FM,
        STEREO_NONE,
        FIR_BG_DK_DUAL_FM,
    },
    {
        _T("D/K3-Dual FM-Stereo"),
        MSP34x0_STANDARD_DK3_DUAL_FM,
        MSP34x0_CARRIER_6_5,
        MSP34x0_CARRIER_5_7421875,
        MONO_FM,
        STEREO_FM,
        FIR_BG_DK_DUAL_FM
    },
    {
        _T("B/G-NICAM-FM"),
        MSP34x0_STANDARD_BG_NICAM_FM,
        MSP34x0_CARRIER_5_5,
        MSP34x0_CARRIER_5_85,
        MONO_FM,
        STEREO_NICAM,
        FIR_BG_DK_NICAM,
    },
    {
        _T("L-NICAM-AM"),
        MSP34x0_STANDARD_L_NICAM_AM,
        MSP34x0_CARRIER_6_5,
        MSP34x0_CARRIER_5_85,
        MONO_AM,
        STEREO_NICAM,
        FIR_L_NICAM,
    },
    {
        _T("I-NICAM-FM"),
        MSP34x0_STANDARD_I_NICAM_FM,
        MSP34x0_CARRIER_6_0,
        MSP34x0_CARRIER_6_552,
        MONO_FM,
        STEREO_NICAM,
        FIR_I_NICAM,
    },
    {
        _T("D/K-NICAM-FM"),
        MSP34x0_STANDARD_DK_NICAM_FM,
        MSP34x0_CARRIER_6_5,
        MSP34x0_CARRIER_5_85,
        MONO_FM,
        STEREO_NICAM,
        FIR_BG_DK_NICAM,
    },
    {
        _T("D/K-NICAM-FM with HDEV2"),
        MSP34x0_STANDARD_DK_NICAM_FM_HDEV2,
        MSP34x0_CARRIER_6_5,
        MSP34x0_CARRIER_5_85,
        MONO_FM,
        STEREO_NICAM,
        FIR_BG_DK_NICAM,
    },
    {
        _T("D/K-NICAM-FM with HDEV3"),
        MSP34x0_STANDARD_DK_NICAM_FM_HDEV3,
        MSP34x0_CARRIER_6_5,
        MSP34x0_CARRIER_5_85,
        MONO_FM,
        STEREO_NICAM,
        FIR_BG_DK_NICAM,
    },
    {
        _T("M-BTSC-Stereo"),
        MSP34x0_STANDARD_M_BTSC,
        MSP34x0_CARRIER_4_5,
        MSP34x0_CARRIER_4_5,
        MONO_FM,
        STEREO_BTSC,
        FIR_BG_DK_DUAL_FM,
    },
    {
        _T("M-BTSC-Mono + SAP"),
        MSP34x0_STANDARD_M_BTSC_MONO,
        MSP34x0_CARRIER_4_5,
        MSP34x0_CARRIER_4_5,
        MONO_FM,
        STEREO_MONO_SAP,
        FIR_BG_DK_DUAL_FM,
    },
    {
        _T("M-EIA-J Japan Stereo"),
        MSP34x0_STANDARD_M_EIA_J,
        MSP34x0_CARRIER_4_5,
        MSP34x0_CARRIER_4_5,
        MONO_FM,
        STEREO_FM,
        FIR_BG_DK_DUAL_FM,
    },
    {
        _T("FM-Stereo Radio"),
        MSP34x0_STANDARD_FM_RADIO,
        MSP34x0_CARRIER_10_7,
        MSP34x0_CARRIER_10_7,
        MONO_FM,
        STEREO_FM,
        FIR_FM_RADIO,
    },
    {
        _T("SAT-Mono"),
        MSP34x0_STANDARD_SAT_MONO,
        MSP34x0_CARRIER_6_5,
        MSP34x0_CARRIER_6_5,
        MONO_FM,
        STEREO_NONE,
        FIR_SAT,
    },
    {
        _T("SAT-Stereo"),
        MSP34x0_STANDARD_SAT,
        MSP34x0_CARRIER_7_02,
        MSP34x0_CARRIER_7_20,
        MONO_FM,
        STEREO_FM,
        FIR_SAT,
    },
    {
        _T("SAT ASTRA Digital Radio"),
        MSP34x0_STANDARD_SAT_ADR,
        MSP34x0_CARRIER_6_12,
        MSP34x0_CARRIER_6_12,
        MONO_FM,
        STEREO_ADR,
        FIR_BG_DK_DUAL_FM, //??
    },
    {
        NULL,
        MSP34x0_STANDARD_NONE,
        MSP34x0_CARRIER_6_5,
        MSP34x0_CARRIER_6_5,
        MONO_FM,
        STEREO_NONE,
        FIR_BG_DK_DUAL_FM,
    },
};

CMSP34x0AudioDecoder::TFIRType CMSP34x0AudioDecoder::m_FIRTypes[] =
{
    {
        { -2, -8, -10, 10, 50, 86 },        //FIR_BG_DK_NICAM
        {  3, 18, 27, 48, 66, 72 },
    },
    {
        {  2, 4, -6, -4, 40, 94 },          //FIR_I_NICAM
        {  3, 18, 27, 48, 66, 72 },
    },
    {
        {  -2, -8, -10, 10, 50, 86 },       //FIR_L_NICAM
        {  -4, -12, -9, 23, 79, 126 },
    },
    {
        {  3, 18, 27, 48, 66, 72 },         //FIR_BG_DK_DUAL_FM
        {  3, 18, 27, 48, 66, 72 },
    },
    {
        { 75, 19, 36, 35, 39, 40 },         //FIR_AM_DETECT
        { 75, 19, 36, 35, 39, 40 },
    },
    {
        {  1,  9, 14, 24, 33, 37 },         //FIR_SAT
        {  3, 18, 27, 48, 66, 72 },
    },
    {
        { -8, -8, 4, 6, 78, 107 },          //FIR_FM_RADIO
        { -8, -8, 4, 6, 78, 107 },
    },
};

WORD CMSP34x0AudioDecoder::m_ScartMasks[MSP34x0_SCARTOUTPUT_LASTONE][MSP34x0_SCARTINPUT_LASTONE + 1] =
{
  /* MUTE    MONO    IN1     IN1_DA  IN2     IN2_DA  IN3     IN4     MASK*/
  {  0x0320, 0x0100, 0x0000, -1,     0x0200, -1,     0x0300, 0x0020, 0x0320, },
  {  0x0c40, 0x0800, 0x0440, 0x0c00, 0x0400, 0x0040, 0x0000, 0x0840, 0x0c40, },
  {  0x3000, 0x2000, 0x1000, 0x0000, 0x1080, 0x0080, 0x2080, 0x3080, 0x3080, },
};

CMSP34x0AudioDecoder::TCarrierDetect CMSP34x0AudioDecoder::CarrierDetectTable[] =
{
  {
    MSP34x0_CARRIER_4_5,  {
                              MSP34x0_CARRIER_4_5,            //BTSC, Japan Stereo
                              MSP34x0_CARRIER_4_724212,       //M Dual FM Stereo
                              MSP34x0_NOCARRIER
                          }
  },
  {
    MSP34x0_CARRIER_5_5,  {
                              MSP34x0_CARRIER_5_7421875,      //B/G Dual FM-Stereo
                              MSP34x0_CARRIER_5_85,           //B/G NICAM FM
                              MSP34x0_NOCARRIER
                          }
  },
  {
    MSP34x0_CARRIER_6_0,  (
                              MSP34x0_CARRIER_6_552,            //I NICAM FM
                              MSP34x0_NOCARRIER
                          )
  },
  {
    MSP34x0_CARRIER_6_12, (
                              MSP34x0_CARRIER_6_12,             //SAT ADR
                              MSP34x0_NOCARRIER
                          )

  },
  {
    MSP34x0_CARRIER_6_5,  (
                              MSP34x0_CARRIER_5_7421875,        //D/K3 Dual FM-Stereo
                              MSP34x0_CARRIER_5_85,             //D/K NICAM FM, L NICAM AM
                              MSP34x0_CARRIER_6_2578125,        //D/L1 Dual FM-Stereo
                              MSP34x0_CARRIER_6_5,              //D/K FM-MONO, SAT-MONO
                              MSP34x0_CARRIER_6_7421875,        //D/K2 Dual FM-Stereo
                              MSP34x0_NOCARRIER
                          )
  },
  {
    MSP34x0_CARRIER_7_02, (
                              MSP34x0_CARRIER_7_20,             //SAT Stereo
                              MSP34x0_CARRIER_7_38,             //SAT Lang1/2
                              MSP34x0_NOCARRIER
                          )
  },
  {
    MSP34x0_NOCARRIER,    ( MSP34x0_NOCARRIER )
  }
};


void CMSP34x0AudioDecoder::SetSCARTxbar(eScartOutput output, eScartInput input)
{
    if (-1 == m_ScartMasks[output][input])
        return;
    WORD acb = GetDSPRegister(DSP_RD_ACB);
    acb &= ~m_ScartMasks[output][MSP34x0_SCARTINPUT_LASTONE];
    acb |=  m_ScartMasks[output][input];
    SetDSPRegister(DSP_WR_ACB, acb);
}

void CMSP34x0AudioDecoder::Initialize()
{
    Reset();

    m_bHasEqualizer = FALSE;
    m_bHasDolby = FALSE;

    if((GetVersion() & 0xFF) >= 0x07)
    {
        m_MSPVersion = MSPVersionG;
        m_bHasDolby = TRUE;
    }
    else if((GetVersion() & 0xFF) >= 0x04)
    {
        m_MSPVersion = MSPVersionD;
    }
    else
    {
        m_MSPVersion = MSPVersionA;
    }

    if (m_ForceVersionA)
    {
        m_MSPVersion = MSPVersionA;
    }
    else if (m_ForceVersionD)
    {
        m_MSPVersion = MSPVersionD;
    }
    else if (m_ForceVersionG)
    {
        m_MSPVersion = MSPVersionG;
    }

    if ((GetVersion() & 0xFF) >= 0x03)
    {
        // Equalizer supported by revisions C and higher
        m_bHasEqualizer = TRUE;
    }

    if (m_ForceHasEqualizer)
    {
        m_bHasEqualizer = TRUE;
    }

    if(m_MSPVersion == MSPVersionG)
    {
        InitializeRevG();
    }
    else
    {
        InitializeRevA();
    }
    m_IsInitialized = TRUE;
}


void CMSP34x0AudioDecoder::SetSoundChannel(eSoundChannel soundChannel)
{
    CAudioDecoder::SetSoundChannel(soundChannel);

    EnterCriticalSection(&MSP34xxCriticalSection);
    if (m_AutoDetecting == 1)
    {
        LeaveCriticalSection(&MSP34xxCriticalSection);

        //Autodetect sets soundchannel if its done
        return;
    }
    LeaveCriticalSection(&MSP34xxCriticalSection);

    if(!m_IsInitialized)
    {
        Initialize();
    }

    if(m_MSPVersion == MSPVersionG)
    {
        SetSoundChannelRevG(soundChannel);
    }
    else
    {
        SetSoundChannelRevA(soundChannel);
    }
}


void CMSP34x0AudioDecoder::SetAudioInput(eAudioInput audioInput)
{
    CAudioDecoder::SetAudioInput(audioInput);

    if(!m_IsInitialized)
    {
        Initialize();
    }

    switch (m_AudioInput)
    {
    case AUDIOINPUT_RADIO:
        SetSCARTxbar(MSP34x0_SCARTOUTPUT_DSP_INPUT, MSP34x0_SCARTINPUT_SCART_4);
        break;
    case AUDIOINPUT_STEREO:
        SetSCARTxbar(MSP34x0_SCARTOUTPUT_DSP_INPUT, MSP34x0_SCARTINPUT_SCART_3);
        break;
    case AUDIOINPUT_INTERNAL:
        SetSCARTxbar(MSP34x0_SCARTOUTPUT_DSP_INPUT, MSP34x0_SCARTINPUT_SCART_2);
        break;
    case AUDIOINPUT_EXTERNAL:
        SetSCARTxbar(MSP34x0_SCARTOUTPUT_DSP_INPUT, MSP34x0_SCARTINPUT_SCART_1);
        break;
    case AUDIOINPUT_MUTE:
        SetSCARTxbar(MSP34x0_SCARTOUTPUT_DSP_INPUT, MSP34x0_SCARTINPUT_MUTE);
        break;
    default:
        break;
    }

    SetDSPRegister(DSP_WR_SCART_PRESCALE, 0x1900);
}


eSoundChannel CMSP34x0AudioDecoder::IsAudioChannelDetected(eSoundChannel desiredAudioChannel)
{
    switch (desiredAudioChannel)
    {
    case SOUNDCHANNEL_STEREO:
        if (m_SupportedSoundChannels & SUPPORTEDSOUNDCHANNEL_STEREO)
        {
            return desiredAudioChannel;
        }
        else
        {
            return SOUNDCHANNEL_MONO;
        }
        break;
    case SOUNDCHANNEL_LANGUAGE1:
        if (m_SupportedSoundChannels & SUPPORTEDSOUNDCHANNEL_LANG1)
        {
            return desiredAudioChannel;
        }
        else
        {
            return SOUNDCHANNEL_MONO;
        }
        break;
    case SOUNDCHANNEL_LANGUAGE2:
        if (m_SupportedSoundChannels & SUPPORTEDSOUNDCHANNEL_LANG2)
        {
            return desiredAudioChannel;
        }
        else
        {
            return SOUNDCHANNEL_MONO;
        }
        break;
    default:
        return SOUNDCHANNEL_MONO;
    }
}


long CMSP34x0AudioDecoder::GetAudioStandardFromVideoFormat(eVideoFormat videoFormat)
{
    /// Guess the correct format
    WORD standard = MSP34x0_STANDARD_NONE;
    switch(videoFormat)
    {
    case VIDEOFORMAT_PAL_B:
        standard = MSP34x0_STANDARD_BG_DUAL_FM;
        break;
    case VIDEOFORMAT_PAL_D:
        standard = MSP34x0_STANDARD_DK_NICAM_FM;
        break;
    case VIDEOFORMAT_PAL_G:
        standard = MSP34x0_STANDARD_BG_NICAM_FM;
        break;
    case VIDEOFORMAT_PAL_H:
        // \todo FIXME
        standard = MSP34x0_STANDARD_NONE;
        break;
    case VIDEOFORMAT_PAL_I:
        standard = MSP34x0_STANDARD_I_NICAM_FM;
        break;
    case VIDEOFORMAT_PAL_M:
        // \todo FIXME
        standard = MSP34x0_STANDARD_NONE;
        break;
    case VIDEOFORMAT_PAL_N:
        // \todo FIXME
        standard = MSP34x0_STANDARD_NONE;
        break;
    case VIDEOFORMAT_PAL_N_COMBO:
        // \todo FIXME
        standard = MSP34x0_STANDARD_NONE;
        break;
    case VIDEOFORMAT_SECAM_B:
        standard = MSP34x0_STANDARD_BG_DUAL_FM;
        break;
    case VIDEOFORMAT_SECAM_D:
        standard = MSP34x0_STANDARD_DK_NICAM_FM;
        break;
    case VIDEOFORMAT_SECAM_G:
        standard = MSP34x0_STANDARD_BG_NICAM_FM;
        break;
    case VIDEOFORMAT_SECAM_H:
        standard = MSP34x0_STANDARD_DK_NICAM_FM;
        break;
    case VIDEOFORMAT_SECAM_K:
        standard = MSP34x0_STANDARD_DK_NICAM_FM;
        break;
    case VIDEOFORMAT_SECAM_K1:
        standard = MSP34x0_STANDARD_DK1_DUAL_FM;
        break;
    case VIDEOFORMAT_SECAM_L:
        standard = MSP34x0_STANDARD_L_NICAM_AM;
        break;
    case VIDEOFORMAT_SECAM_L1:
        standard = MSP34x0_STANDARD_L_NICAM_AM;
        break;
    case VIDEOFORMAT_NTSC_M:
        standard = MSP34x0_STANDARD_M_BTSC;
        break;
    case VIDEOFORMAT_NTSC_M_Japan:
        standard = MSP34x0_STANDARD_M_EIA_J;
        break;
    default:
    case VIDEOFORMAT_PAL_60:
    case VIDEOFORMAT_NTSC_50:
        standard = MSP34x0_STANDARD_NONE;
        break;
    }
    return standard;
}

CAudioDecoder::eAudioDecoderType CMSP34x0AudioDecoder::GetAudioDecoderType()
{
    return CAudioDecoder::AUDIODECODERTYPE_MSP34x0;
}

int CMSP34x0AudioDecoder::SetAudioDecoderValue(int What, long Val)
{
    if (What==0)
    {
        long Flags = Val;
        if (Flags&0x01)
        {
            if (!m_ForceVersionA)
            {
                m_ForceVersionA = TRUE;
                Initialize();
            }
        }
        else if (Flags&0x02)
        {
            if (!m_ForceVersionD)
            {
                m_ForceVersionD = TRUE;
                Initialize();
            }
        }
        else if (Flags&0x04)
        {
            if (!m_ForceVersionG)
            {
                m_ForceVersionG = TRUE;
                Initialize();
            }
        }

        m_ForceAMSound = ((Flags & 0x08)!=0);
        m_ForceHasEqualizer = ((Flags & 0x10)!=0);
        return 1;
    }
    return 0;
}

long CMSP34x0AudioDecoder::GetAudioDecoderValue(int What)
{
    if (What==0)
    {
        long Flags = 0;
        if (m_ForceVersionA)
        {
            Flags |= 0x01;
        }
        else if (m_ForceVersionD)
        {
            Flags |= 0x02;
        }
        else if (m_ForceVersionG)
        {
            Flags |= 0x04;
        }
        if (m_ForceAMSound)
        {
            Flags |= 0x08;
        }
        if (m_ForceHasEqualizer)
        {
             Flags |= 0x10;
        }
        return Flags;
    }
    return 0;
}


long CMSP34x0AudioDecoder::GetAudioStandardCurrent()
{
    return CAudioDecoder::GetAudioStandardCurrent();
}

void CMSP34x0AudioDecoder::SetAudioStandard(long Standard, eVideoFormat VideoFormat)
{
    CAudioDecoder::SetAudioStandard(Standard, VideoFormat);

    //Only for tuner & radio
    if(m_AudioInput != AUDIOINPUT_RADIO && m_AudioInput != AUDIOINPUT_TUNER)
    {
        return;
    }

    if(!m_IsInitialized)
    {
        Initialize();
    }

    // Find default carrier frequencies
    if (Standard == MSP34x0_STANDARD_NONE)
    {
        m_AudioStandardMajorCarrier = 0;
        m_AudioStandardMinorCarrier = 0;
    }
    else
    {
        m_AudioStandardMajorCarrier = 0;
        m_AudioStandardMinorCarrier = 0;

        int nIndex = 0;
        while (m_MSPStandards[nIndex].Name != NULL)
        {
            if (m_MSPStandards[nIndex].Standard == Standard)
            {
                m_AudioStandardMajorCarrier = MSP_UNCARRIER_HZ(m_MSPStandards[nIndex].MajorCarrier);
                m_AudioStandardMinorCarrier = MSP_UNCARRIER_HZ(m_MSPStandards[nIndex].MinorCarrier);
            }
            nIndex++;
        }
    }

    if (m_MSPVersion == MSPVersionG)
    {
        SetStandardRevG((eStandard)Standard, VideoFormat);
        SetSoundChannel(m_SoundChannel);
    }
    else
    {
        SetStandardRevA((eStandard)Standard, VideoFormat, TRUE, m_SoundChannel);
    }
}

const TCHAR* CMSP34x0AudioDecoder::GetAudioStandardName(long Standard)
{
    if (Standard == MSP34x0_STANDARD_NONE)
    {
        return _T("<No standard>");
    }
    if (Standard >= MSP34x0_STANDARD_AUTODETECTION_IN_PROGRESS)
    {
        return _T("Detecting...");
    }
    int nIndex = 0;
    while (m_MSPStandards[nIndex].Name != NULL)
    {
        if (m_MSPStandards[nIndex].Standard == Standard)
        {
            return m_MSPStandards[nIndex].Name;
        }
        nIndex++;
    }
    return NULL;
}

int CMSP34x0AudioDecoder::GetNumAudioStandards()
{
    return ((sizeof(m_MSPStandards)/sizeof(TStandardDefinition)));
}

long CMSP34x0AudioDecoder::GetAudioStandard(int nIndex)
{
    if ((nIndex < 0) || (nIndex >= GetNumAudioStandards()) )
    {
        return 0;
    }
    if (nIndex == 0)
    {
        return MSP34x0_STANDARD_NONE;
    }
    return m_MSPStandards[nIndex-1].Standard;
}

long CMSP34x0AudioDecoder::GetAudioStandardMajorCarrier(long Standard)
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
    if (Standard == MSP34x0_STANDARD_NONE)
    {
        return 0; //No standard
    }
    int nIndex = 0;
    while (m_MSPStandards[nIndex].Name != NULL)
    {
        if (m_MSPStandards[nIndex].Standard == Standard)
        {
            return MSP_UNCARRIER_HZ(m_MSPStandards[nIndex].MajorCarrier);
        }
        nIndex++;
    }
    return 0;
}

long CMSP34x0AudioDecoder::GetAudioStandardMinorCarrier(long Standard)
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
    if (Standard == MSP34x0_STANDARD_NONE)
    {
        return 0; //No standard
    }
    int nIndex = 0;
    while (m_MSPStandards[nIndex].Name != NULL)
    {
        if (m_MSPStandards[nIndex].Standard == Standard)
        {
            return MSP_UNCARRIER_HZ(m_MSPStandards[nIndex].MinorCarrier);
        }
        nIndex++;
    }
    return 0;
}


void CMSP34x0AudioDecoder::SetAudioStandardCarriers(long MajorCarrier, long MinorCarrier)
{
    CAudioDecoder::SetAudioStandardCarriers(MajorCarrier, MinorCarrier);

    if ((MajorCarrier != 0) && (MinorCarrier!=0) &&
        ((MajorCarrier != m_AudioStandardMajorCarrier) || (MinorCarrier != m_AudioStandardMinorCarrier)))
    {
        if (m_MSPVersion == MSPVersionG)
        {
            SetCarrierRevG((eCarrier)MSP_CARRIER_HZ(MajorCarrier), (eCarrier)MSP_CARRIER_HZ(MinorCarrier));
        }
        else
        {
            SetCarrierRevA((eCarrier)MSP_CARRIER_HZ(MajorCarrier), (eCarrier)MSP_CARRIER_HZ(MinorCarrier));
        }
    }
}


void CMSP34x0AudioDecoder::DetectAudioStandard(long Interval, int SupportedSoundChannels, eSoundChannel TargetChannel)
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

    LOGD(_T("MSP34xx: Detect %d\n"),SupportedSoundChannels);

    EnterCriticalSection(&MSP34xxCriticalSection);
    int AutoDetecting = m_AutoDetecting;
    LeaveCriticalSection(&MSP34xxCriticalSection);

    // Stop thread if detecting audio standard
    // Suspend thread if detecting stereo
    if (AutoDetecting == 1)
    {
        StopThread();
    }
    else if (AutoDetecting == 2)
    {
        EnterCriticalSection(&MSP34xxCriticalSection);
        m_ThreadWait = TRUE;
        LeaveCriticalSection(&MSP34xxCriticalSection);
        Sleep(10);

        EnterCriticalSection(&MSP34xxCriticalSection);
        BOOL bSuspended = (m_AutoDetecting == 0);
        LeaveCriticalSection(&MSP34xxCriticalSection);

        if (!bSuspended)
        {
            Sleep(50);
            EnterCriticalSection(&MSP34xxCriticalSection);
            bSuspended = (m_AutoDetecting == 0);
            LeaveCriticalSection(&MSP34xxCriticalSection);

            if (!bSuspended)
            {
                StopThread();
            }
        }
        Sleep(1);
    }

    //Setup
    if (SupportedSoundChannels == 2)
    {
        if (TargetChannel != SOUNDCHANNEL_MONO)
        {
            if(m_MSPVersion == MSPVersionG)
            {
                SetDEMRegister(DEM_WR_MODUS, 0x2003);
            }
        }
    }
    else
    {
        if(m_MSPVersion == MSPVersionG)
        {
            SetStandardRevG(MSP34x0_STANDARD_AUTO, m_VideoFormat);
        }
        else
        {
            SetStandardRevA(MSP34x0_STANDARD_AUTO, m_VideoFormat, FALSE, SOUNDCHANNEL_MONO);
        }
    }

    //Set new state
    EnterCriticalSection(&MSP34xxCriticalSection);
    m_DetectCounter = 0;

    if (SupportedSoundChannels == 2)
    {
        m_AutoDetecting = 2;
    }
    else
    {
        m_AutoDetecting = 1;
        m_DetectSupportedSoundChannels = (SupportedSoundChannels == 1);
    }
    m_SupportedSoundChannels = SUPPORTEDSOUNDCHANNEL_MONO;
    m_ThreadWait = FALSE;
    LeaveCriticalSection(&MSP34xxCriticalSection);

    //Start or resume thread
    StartThread();
}


/////////////////////////////////////////////////////////////////////////////////////////
/// Auto detect/monitoring thread ///////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
DWORD WINAPI MSP34xxThreadProc(LPVOID lpThreadParameter)
{
    DScalerThread thisThread(_T("MSP34xxDetectThread"));

    if (lpThreadParameter != NULL)
    {
        int Result = 1;
        try
        {
            Result = ((CMSP34x0AudioDecoder*)lpThreadParameter)->DetectThread();
        }
        catch(...)
        {
            LOG(1, _T("Crash in MSP34xx detect loop"));
        }
        return Result;
    }
    return 1;
}

void CMSP34x0AudioDecoder::StartThread()
{
    DWORD LinkThreadID;

    if (m_MSP34xxThread != NULL)
    {
        // Already started, resume
        LOGD(_T("MSP34xx: StartThread: Resume thread\n"));
        ResumeThread(m_MSP34xxThread);
        return;
    }
    LOGD(_T("MSP34xx: StartThread: Create thread\n"));

    m_bStopThread = FALSE;

    m_MSP34xxThread = CreateThread((LPSECURITY_ATTRIBUTES) NULL,  // No security.
                             (DWORD) 0,                     // Same stack size.
                             MSP34xxThreadProc,                  // Thread procedure.
                             (LPVOID)this,                          // Parameter.
                             (DWORD) 0,                     // Start immediatly.
                             (LPDWORD) & LinkThreadID);     // Thread ID.
}

void CMSP34x0AudioDecoder::StopThread()
{
    DWORD ExitCode;
    int i;
    BOOL Thread_Stopped = FALSE;

    LOGD(_T("MSP34xx: StopThread: Try stop\n"));
    if (m_MSP34xxThread != NULL)
    {
        i = 10;
        m_bStopThread = TRUE;
        ResumeThread(m_MSP34xxThread);
        while(i-- > 0 && !Thread_Stopped)
        {
            if (GetExitCodeThread(m_MSP34xxThread, &ExitCode) == TRUE)
            {
                if (ExitCode != STILL_ACTIVE)
                {
                    Thread_Stopped = TRUE;
                }
                else
                {
                    Sleep(50);
                }
            }
            else
            {
                Thread_Stopped = TRUE;
            }
        }

        if (Thread_Stopped == FALSE)
        {
            LOGD(_T("MSP34xx: StopThread: Terminate thread\n"));
            TerminateThread(m_MSP34xxThread, 0);
            Sleep(50);
        }
        CloseHandle(m_MSP34xxThread);
        m_MSP34xxThread = NULL;
        LOGD(_T("MSP34xx: StopThread: Thread stopped\n"));
    }
}

int CMSP34x0AudioDecoder::DetectThread()
{
    int AutoDetecting;
    BOOL bWait;

    m_DetectCounter = 0;
    while (!m_bStopThread)
    {
        EnterCriticalSection(&MSP34xxCriticalSection);
        bWait = m_ThreadWait;
        LeaveCriticalSection(&MSP34xxCriticalSection);

        if (bWait)
        {
            LOGD(_T("MSP34xx Thread: Suspended\n"));
            m_AutoDetecting = 0;
            SuspendThread(m_MSP34xxThread);
            LOGD(_T("MSP34xx Thread: Resumed\n"));
        }

        EnterCriticalSection(&MSP34xxCriticalSection);
        AutoDetecting = m_AutoDetecting;
        LeaveCriticalSection(&MSP34xxCriticalSection);

        if ( (AutoDetecting==1) //Detect standard
            && ((m_DetectCounter%m_DetectInterval10ms)==0))
        {
            if((m_MSPVersion == MSPVersionG)) //||(m_MSPVersion == MSPVersionD)
            {
                eStandard standard = DetectStandardRevG();
                LOG(2,_T("MSP34revG: Detected standard=0x%04x"), standard);

                if (standard != MSP34x0_STANDARD_AUTODETECTION_IN_PROGRESS)
                {
                    m_SupportedSoundChannels = SUPPORTEDSOUNDCHANNEL_MONO;

                    if (m_DetectSupportedSoundChannels)
                    {
                        m_SoundChannel = m_TargetSoundChannel;
                    }

                    if (m_ForceAMSound && IsSECAMVideoFormat(m_VideoFormat))
                    {
                        // autodetect doesn't work well with AM ...
                        standard = MSP34x0_STANDARD_L_NICAM_AM;
                        SetStandardRevG(standard, m_VideoFormat);
                    }

                    m_AudioStandard = standard;

                    SetStandardRevG(standard, m_VideoFormat, TRUE);

                    AutoDetecting = 2;  //Detect stereo modes

                    SetSoundChannelRevG(m_SoundChannel);

                     EventCollector->RaiseEvent(this,EVENT_AUDIOSTANDARD_DETECTED, 0, standard);

                    EnterCriticalSection(&MSP34xxCriticalSection);
                    if (!m_DetectSupportedSoundChannels)
                    {
                        AutoDetecting = 0;
                    }
                    LeaveCriticalSection(&MSP34xxCriticalSection);
                 }
            }
            else
            {
                eStandard standard = DetectStandardRevA();
                LOG(2,_T("MSPrevA: Detected standard=0x%04x"),standard);
                if (standard != MSP34x0_STANDARD_AUTODETECTION_IN_PROGRESS)
                {
                    m_SupportedSoundChannels = SUPPORTEDSOUNDCHANNEL_MONO;
                    m_AudioStandard = standard;

                    SetStandardRevA(standard, m_VideoFormat, TRUE, SOUNDCHANNEL_MONO);

                     EventCollector->RaiseEvent(this,EVENT_AUDIOSTANDARD_DETECTED, 0, standard);

                    EnterCriticalSection(&MSP34xxCriticalSection);
                    if (m_DetectSupportedSoundChannels)
                    {
                         LeaveCriticalSection(&MSP34xxCriticalSection);

                         AutoDetecting = 2; //Try to find stereo
                    }
                    else
                    {
                         LeaveCriticalSection(&MSP34xxCriticalSection);

                         AutoDetecting = 0; //Finished
                         SetSoundChannelRevA(m_SoundChannel);
                    }
                }
            }
            EnterCriticalSection(&MSP34xxCriticalSection);
            m_AutoDetecting = AutoDetecting;
            m_DetectCounter = 1;
            LeaveCriticalSection(&MSP34xxCriticalSection);
        }

        EnterCriticalSection(&MSP34xxCriticalSection);
        AutoDetecting = m_AutoDetecting;
        if (m_AutoDetecting == 0)
        {
            m_ThreadWait = TRUE;
        }
        LeaveCriticalSection(&MSP34xxCriticalSection);

        if (AutoDetecting==2) //Detect mono/stereo/lang1/lang2
        {
            if ((m_DetectCounter%m_DetectInterval10ms) == 0)
            {
                eSupportedSoundChannels Supported;
                if(m_MSPVersion == MSPVersionG)
                {
                    Supported = DetectSoundChannelsRevG();
                }
                else
                {
                    Supported = DetectSoundChannelsRevA();
                }

                if ((Supported != m_SupportedSoundChannels) || (m_DetectCounter>=150))
                {
                    m_SupportedSoundChannels = Supported;
                    LOG(2,_T("MSP34xx: Detect stereo: Supported: %s%s%s%s"),
                        (Supported&SUPPORTEDSOUNDCHANNEL_MONO)?_T("Mono "):_T(""),
                        (Supported&SUPPORTEDSOUNDCHANNEL_STEREO)?_T("Stereo "):_T(""),
                        (Supported&SUPPORTEDSOUNDCHANNEL_LANG1)?_T("Lang1 "):_T(""),
                        (Supported&SUPPORTEDSOUNDCHANNEL_LANG2)?_T("Lang2 "):_T(""));

                    m_SoundChannel = IsAudioChannelDetected(m_TargetSoundChannel); //m_SoundChannel);
                    SetSoundChannel(m_SoundChannel);

                    EventCollector->RaiseEvent(this,EVENT_AUDIOCHANNELSUPPORT_DETECTED, 0, m_SupportedSoundChannels);

                    if (!m_KeepWatchingStereo)
                    {
                        //Finished
                        EnterCriticalSection(&MSP34xxCriticalSection);
                        m_AutoDetecting = 0;
                        m_ThreadWait = TRUE;

                        AutoDetecting = 0;
                        LeaveCriticalSection(&MSP34xxCriticalSection);
                    }
                }
            }
        }

        //Check for suspend mode before and after the Sleep.
        EnterCriticalSection(&MSP34xxCriticalSection);
        bWait = m_ThreadWait;
        if (bWait)
        {
            AutoDetecting = 0;
        }
        LeaveCriticalSection(&MSP34xxCriticalSection);

        if (AutoDetecting != 0)
        {
            Sleep(10);
            m_DetectCounter++;
        }
    }
    return 0;
}

#endif // WANT_BT8X8_SUPPORT
