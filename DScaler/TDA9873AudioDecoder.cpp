/////////////////////////////////////////////////////////////////////////////
//
// $Id$
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2004 Sven Grothklags.  All rights reserved.
//
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
//
/////////////////////////////////////////////////////////////////////////////
//
// This software was based on TDA9875AudioDecoder.cpp.  Those portions are
// Copyright (c) 2004 Robert Milharcic.
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: not supported by cvs2svn $
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file TDA9873AudioDecoder.cpp: implementation of the CTDA9873AudioDecoder class.
 */

#include "stdafx.h"
#include "TDA9873AudioDecoder.h"
#include "Crash.h" 
#include "DebugLog.h"

CTDA9873AudioDecoder::TStandardDefinition CTDA9873AudioDecoder::m_TDA9873Standards[] =
{
    {
        "B/G",
        TDA9873ST_BG,
    },
    {
        "M",
        TDA9873ST_M,
    },
    {
        "D/K (1)",
        TDA9873ST_DK1,
    },
    {
        "D/K (2)",
        TDA9873ST_DK2,
    },
    {
        "D/K (3)",
        TDA9873ST_DK3,
    },
    {
        "I",
        TDA9873ST_I,
    },
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTDA9873AudioDecoder::CTDA9873AudioDecoder()
{
    m_ShadowReg[TDA9873_REG_SWITCH] = 0;
    m_ShadowReg[TDA9873_REG_STANDARD] = 0;
    m_ShadowReg[TDA9873_REG_PORT] = 0;

    m_AudioStandard = m_ShadowReg[TDA9873_REG_STANDARD] & TDA9873ST_STANDARDMASK;

    m_AutoDetecting = 0;
    m_TDA9873Thread = NULL;
    m_SupportedSoundChannels = SUPPORTEDSOUNDCHANNEL_MONO;
}

CTDA9873AudioDecoder::~CTDA9873AudioDecoder()
{
    StopThread();
}

CAudioDecoder::eAudioDecoderType CTDA9873AudioDecoder::GetAudioDecoderType()
{
    return AUDIODECODERTYPE_TDA9873;
}

BYTE CTDA9873AudioDecoder::GetDefaultAddress() const
{
    return 0xb6 >> 1;
}

BYTE CTDA9873AudioDecoder::ReadStatus()
{
    BYTE result = m_DeviceAddress << 1;
    if(!m_I2CBus->Read(&result, 1, &result, 1))
    {
        result = -1;
    }

    return result;
}

bool CTDA9873AudioDecoder::Write(eReg SubAddress, BYTE Data, BYTE Mask)
{
    ASSERT(SubAddress >= TDA9873_REG_SWITCH && SubAddress < TDA9873_REG_LAST_ONE);

    BYTE NewValue = (m_ShadowReg[SubAddress]&~Mask) | (Data&Mask);

    if(NewValue == m_ShadowReg[SubAddress])
        return true;

    m_ShadowReg[SubAddress] = NewValue;
    return WriteToSubAddress(SubAddress, &NewValue, 1);
}

bool CTDA9873AudioDecoder::Detect()
{
    if((ReadStatus()&TDA9873IN_MAGICMASK) != TDA9873IN_MAGIC)
    {
        return false;
    }

    BYTE result;
    if(!ReadFromSubAddress(TDA9873_REG_DETECT, &result, 1) || result != TDA9873DE_MAGIC)
    {
        return false;
    }

    return true;
}

bool CTDA9873AudioDecoder::Initialize()
{
    if(!Detect())
    {
        // Try alternative address
        m_DeviceAddress ^= 1;
        if(!Detect()) return false;
    }

    return Write(TDA9873_REG_SWITCH, 0xe4) &&
           Write(TDA9873_REG_STANDARD, 0x06) &&
           Write(TDA9873_REG_PORT, 0x03);
}

void CTDA9873AudioDecoder::SetSoundChannel(eSoundChannel SoundChannel)
{
    CAudioDecoder::SetSoundChannel(SoundChannel);

    BYTE ch;
    switch(SoundChannel)
    {
    default:
        ch = TDA9873SW_MONO;
        break;
    case SOUNDCHANNEL_STEREO:
        ch = TDA9873SW_STEREO;
        break;
    case SOUNDCHANNEL_LANGUAGE1:
        ch = TDA9873SW_LANG1;
        break;
    case SOUNDCHANNEL_LANGUAGE2:
        ch = TDA9873SW_LANG2;
        break;
    }

    Write(TDA9873_REG_SWITCH, ch, TDA9873SW_CHANNELMASK);
}

eSoundChannel CTDA9873AudioDecoder::IsAudioChannelDetected(eSoundChannel DesiredAudioChannel)
{
    eSoundChannel SoundChannel = SOUNDCHANNEL_MONO;
#if 1
    switch (DesiredAudioChannel)
    {
    case SOUNDCHANNEL_STEREO:
        if (m_SupportedSoundChannels & SUPPORTEDSOUNDCHANNEL_STEREO)
            SoundChannel = DesiredAudioChannel;
        break;
    case SOUNDCHANNEL_LANGUAGE1:
        if (m_SupportedSoundChannels & SUPPORTEDSOUNDCHANNEL_LANG1)
            SoundChannel = DesiredAudioChannel;
        break;
    case SOUNDCHANNEL_LANGUAGE2:
        if (m_SupportedSoundChannels & SUPPORTEDSOUNDCHANNEL_LANG2)
            SoundChannel = DesiredAudioChannel;
        break;
    }
#else
    switch (DesiredAudioChannel)
    {
    case SOUNDCHANNEL_STEREO:
        if ((ReadStatus()&TDA9873IN_CHANNELMASK) == TDA9873IN_STEREO)
        {
            SoundChannel = DesiredAudioChannel;
        }
        break;
    case SOUNDCHANNEL_LANGUAGE1:
    case SOUNDCHANNEL_LANGUAGE2:
        if ((ReadStatus()&TDA9873IN_CHANNELMASK) == TDA9873IN_LANG12)
        {
            SoundChannel = DesiredAudioChannel;
        }
        break;
    default:
        // Nothing
        break;
    }
#endif
    return SoundChannel;
}

void CTDA9873AudioDecoder::SetAudioInput(eAudioInput AudioInput)
{
    CAudioDecoder::SetAudioInput(AudioInput);

    BYTE in;
    switch(AudioInput)
    {
    default:
        in = TDA9873SW_INTERNAL;
        break;
    case AUDIOINPUT_MUTE:
        in = TDA9873SW_MUTE;
        break;
    case AUDIOINPUT_RADIO:
        in = TDA9873SW_EXTERNALSTEREO;
        break;
    }

    Write(TDA9873_REG_SWITCH, in, TDA9873SW_INPUTMASK);
}

int CTDA9873AudioDecoder::Standard2Index(long Standard)
{
    for(int i = 0; i < GetNumAudioStandards(); ++i)
    {
        if(m_TDA9873Standards[i].Standard == Standard)
        {
            return i;
        }
    }

    return -1;
}

void CTDA9873AudioDecoder::SetAudioStandard(long Standard, eVideoFormat VideoFormat)
{
    CAudioDecoder::SetAudioStandard(Standard, VideoFormat);

    int index = Standard2Index(Standard);

    if(index < 0)
    {
        return;
    }

    Write(TDA9873_REG_STANDARD, m_TDA9873Standards[index].Standard, TDA9873ST_STANDARDMASK);
}

const char* CTDA9873AudioDecoder::GetAudioStandardName(long Standard)
{
    int index = Standard2Index(Standard);

    if(index < 0)
    {
        return NULL;
    }

    return m_TDA9873Standards[index].Name;
}

int CTDA9873AudioDecoder::GetNumAudioStandards()
{
    return sizeof(m_TDA9873Standards)/sizeof(m_TDA9873Standards[0]);
}

long CTDA9873AudioDecoder::GetAudioStandard(int nIndex)
{
    if ((nIndex < 0) || (nIndex >= GetNumAudioStandards()) )
    {
        return 0;
    }

    return m_TDA9873Standards[nIndex].Standard;
}

long CTDA9873AudioDecoder::GetAudioStandardFromVideoFormat(eVideoFormat VideoFormat)
{
    // ToDo: Check, add, ...

    /// Guess the correct format
    WORD Standard;
    switch(VideoFormat)
    {
    case VIDEOFORMAT_PAL_B:
    case VIDEOFORMAT_PAL_G:
    case VIDEOFORMAT_SECAM_B:
    case VIDEOFORMAT_SECAM_G:
        Standard = TDA9873ST_BG;
        break;
    case VIDEOFORMAT_PAL_D:
    case VIDEOFORMAT_SECAM_D:
    case VIDEOFORMAT_SECAM_K:
        Standard = TDA9873ST_DK1;
        break;
    case VIDEOFORMAT_PAL_I:
        Standard = TDA9873ST_I;
        break;
    case VIDEOFORMAT_PAL_M:
    case VIDEOFORMAT_PAL_N:
    case VIDEOFORMAT_NTSC_M:
    case VIDEOFORMAT_NTSC_M_Japan:
        Standard = TDA9873ST_M;
        break;
    case VIDEOFORMAT_SECAM_K1:
        Standard = TDA9873ST_DK2;
        break;
    default:
        Standard = TDA9873ST_BG;
        break;
    }
    return Standard;
}

void CTDA9873AudioDecoder::DetectAudioStandard(long Interval, int SupportedSoundChannels, eSoundChannel TargetChannel)
{
    CAudioDecoder::DetectAudioStandard(Interval, SupportedSoundChannels, TargetChannel);

    if (Interval > 0)
    {
        m_DetectInterval10ms = (Interval+9)/10;
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

    // Suspend thread if detecting
    if (m_AutoDetecting)
    {
        LOGD("Abort1 TDA9873 detect loop");
        m_ThreadWait = true;
        Sleep(10);
        
        if (m_AutoDetecting)
        {            
            LOGD("Abort2 TDA9873 detect loop");
            Sleep(50);

            if (m_AutoDetecting)
            {
                LOGD("Abort3 TDA9873 detect loop");
                StopThread();
            }
        }
    }

    if (SupportedSoundChannels == 2)
    {
        m_AutoDetecting = 2;
    }
    else
    {
        m_AutoDetecting = 1;
        m_DetectSupportedSoundChannels = (SupportedSoundChannels == 1);
    }

    //Start or resume thread
    StartThread();
}

DWORD WINAPI TDA9873ThreadProc(LPVOID lpThreadParameter)
{
    int Result = 1;

    DScalerInitializeThread("TDA9873DetectThread");
    if (lpThreadParameter != NULL)
    {
        __try
        {
            Result = ((CTDA9873AudioDecoder*)lpThreadParameter)->DetectThread();
        }
        __except (CrashHandler((EXCEPTION_POINTERS*)_exception_info()))
        {
            LOG(1, "Crash in TDA9873 detect loop");
            DScalerDeinitializeThread();
            return 1;
        }
    }
    DScalerDeinitializeThread();

    return Result;
}

void CTDA9873AudioDecoder::StartThread()
{
    DWORD LinkThreadID;

    m_ThreadWait = false;

    if (m_TDA9873Thread != NULL)
    {
        // Already started, resume
        if(ResumeThread(m_TDA9873Thread) == 0)
        {
            LOG(1, "TDA9873 detect loop not waiting(1)");
            Sleep(10);
            if(ResumeThread(m_TDA9873Thread) == 0)
            {
                LOG(1, "TDA9873 detect loop not waiting(2)");
            }
        }
        return;
    }

    m_StopThread = false;

    m_TDA9873Thread = CreateThread((LPSECURITY_ATTRIBUTES) NULL,  // No security.
                             (DWORD) 0,                     // Same stack size.
                             TDA9873ThreadProc,                  // Thread procedure.
                             (LPVOID)this,                          // Parameter.
                             (DWORD) 0,                     // Start immediatly.
                             (LPDWORD) &LinkThreadID);     // Thread ID.
}

void CTDA9873AudioDecoder::StopThread()
{
    if (m_TDA9873Thread != NULL)
    {
        // Signal the stop
        m_StopThread = TRUE;
        ResumeThread(m_TDA9873Thread);

        // Wait one second for the thread to exit gracefully
        DWORD dwResult = WaitForSingleObject(m_TDA9873Thread, 1000);

        if (dwResult != WAIT_OBJECT_0)
        {
            LOG(1,"TDA9873 detect loop did not terminate gracefully: terminating!");
            TerminateThread(m_TDA9873Thread, 0);
        }

        CloseHandle(m_TDA9873Thread);
        m_TDA9873Thread = NULL;
    }
}

int CTDA9873AudioDecoder::DetectThread()
{
    int DetectCounter = 1;
    m_SupportedSoundChannels = SUPPORTEDSOUNDCHANNEL_MONO;

    while (!m_StopThread)
    {
        if (m_ThreadWait)
        {
            LOGD("Waiting TDA9873 detect loop");
            m_AutoDetecting = 0;
            SuspendThread(m_TDA9873Thread);
            LOGD("Resuming TDA9873 detect loop");
            DetectCounter = 1;
            m_SupportedSoundChannels = SUPPORTEDSOUNDCHANNEL_MONO;
        }

        if (m_AutoDetecting==1)         //Detect standard
        {
            // No autodetection of standard possible!!?
            EventCollector->RaiseEvent(this, EVENT_AUDIOSTANDARD_DETECTED, 0, m_AudioStandard);

            if (m_DetectSupportedSoundChannels)
            {
                m_AutoDetecting = 2;    //Try to find stereo
            }
            else
            {
                m_ThreadWait = true;    //Finished
            }

            DetectCounter = 1;            
        }

        if (m_AutoDetecting==2)         //Detect mono/stereo/lang1/lang2
        {
            if ((DetectCounter >= 100) && (DetectCounter%m_DetectInterval10ms) == 0)
            {
                eSupportedSoundChannels Supported = SUPPORTEDSOUNDCHANNEL_MONO;
                int iSupported = (int)Supported;

                BYTE status = ReadStatus();

                if ((status & TDA9873IN_CHANNELMASK) == TDA9873IN_STEREO)
                {
                    iSupported |= SUPPORTEDSOUNDCHANNEL_STEREO;
                }
                if ((status & TDA9873IN_CHANNELMASK) == TDA9873IN_LANG12)
                {
                    iSupported |= SUPPORTEDSOUNDCHANNEL_LANG1|SUPPORTEDSOUNDCHANNEL_LANG2;
                }

                Supported = (eSupportedSoundChannels)iSupported;

                if ((Supported != m_SupportedSoundChannels) || (DetectCounter >= 200))
                {
                    m_SupportedSoundChannels = Supported;

                    LOG(2,"TDA9873: Detect stereo: Supported: %s%s%s%s",
                        (Supported&SUPPORTEDSOUNDCHANNEL_MONO)?"Mono ":"",
                        (Supported&SUPPORTEDSOUNDCHANNEL_STEREO)?"Stereo ":"",
                        (Supported&SUPPORTEDSOUNDCHANNEL_LANG1)?"Lang1 ":"",
                        (Supported&SUPPORTEDSOUNDCHANNEL_LANG2)?"Lang2 ":"");

                    SetSoundChannel(IsAudioChannelDetected(m_TargetSoundChannel));

                    EventCollector->RaiseEvent(this, EVENT_AUDIOCHANNELSUPPORT_DETECTED, 0, Supported);

                    if (DetectCounter >= 200)
                    {
                        m_ThreadWait = true;    //Finished
                    }
                }
            }
        }

        if (!m_StopThread && !m_ThreadWait)
        {
            Sleep(10);
            ++DetectCounter;
        } 
    }

    return 0;
}