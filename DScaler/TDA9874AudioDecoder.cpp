/////////////////////////////////////////////////////////////////////////////
// $Id$
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
//  Copyright (C) 2004 Mika Laine.  All rights reserved.
//  Copyright (C) 2000-2005 Quenotte  All rights reserved.
//  Copyright (C) 2000 - 2002 by Eduardo José Tagle.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TDA9874AudioDecoder.h"
#include "Crash.h"
#include "DebugLog.h"
//#include "I2C.h"
#include "SoundChannel.h"
//#include "BT848Card.h"

#define CFREQ(x) DWORD(((x) * 16777216.0)/12.288)

CTDA9874AudioDecoder::ANInfo CTDA9874AudioDecoder::m_TDA9874Standards[] =
{
    // First NICAM, because autodetection works better
    {_T("NICAM I"),        TDA9874_STANDARD_I_DIGITAL_FM_NICAM,    CFREQ(6.0),CFREQ(6.552),0x08,0x00,0x00,TRUE ,FALSE}, // Dematrix is Mono so NICAM reverts to Mono if BER is high
    {_T("NICAM BG"),    TDA9874_STANDARD_BG_DIGITAL_FM_NICAM,    CFREQ(5.5),CFREQ(5.85 ),0x08,0x00,0x00,TRUE ,FALSE},
    {_T("NICAM DK"),    TDA9874_STANDARD_DK_DIGITAL_FM_NICAM,    CFREQ(6.5),CFREQ(5.85 ),0x08,0x00,0x00,TRUE ,FALSE},
    {_T("A2 M (Korea)"),TDA9874_STANDARD_M_ANALOG_FM_FM,        CFREQ(4.5),CFREQ(4.724),0x20,0x22,0x05,FALSE,FALSE},
    {_T("A2 BG"),        TDA9874_STANDARD_BG_ANALOG_FM_FM,        CFREQ(5.5),CFREQ(5.742),0x00,0x00,0x04,FALSE,FALSE},
    {_T("A2 DK (1)"),    TDA9874_STANDARD_DK1_ANALOG_FM_FM,        CFREQ(6.5),CFREQ(6.258),0x00,0x00,0x04,FALSE,FALSE},
    {_T("A2 DK (2)"),    TDA9874_STANDARD_DK2_ANALOG_FM_FM,        CFREQ(6.5),CFREQ(6.742),0x00,0x00,0x04,FALSE,FALSE},
    {_T("A2 DK (3)"),    TDA9874_STANDARD_DK3_ANALOG_FM_FM,        CFREQ(6.5),CFREQ(5.742),0x00,0x00,0x04,FALSE,FALSE},
    {_T("NICAM AM"),    TDA9874_STANDARD_L_DIGITAL_AM_NICAM,    CFREQ(6.5),CFREQ(5.85 ),0x09,0x00,0x00,TRUE ,TRUE },
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTDA9874AudioDecoder::CTDA9874AudioDecoder()
{
    m_AudioStandard = 1;
    m_SIF=0;
    m_AutoDetecting = 0;
    m_TDA9874Thread = NULL;
    m_SupportedSoundChannels = SUPPORTEDSOUNDCHANNEL_LANG2;
}

CTDA9874AudioDecoder::~CTDA9874AudioDecoder()
{
    StopThread();
}

BOOL CTDA9874AudioDecoder::Initialize()
{
    WriteToSubAddress(TDA9874A_GCONR, 0x10);    // Reset
    WriteToSubAddress(TDA9874A_AGCGR, 0x00);    // 0 dB

    // Muting
    WriteToSubAddress(TDA9874A_AMCONR, 0xF9);    // mute enable ALL outs

    // Channel Gain settings
    WriteToSubAddress(TDA9874A_C1OLAR, 0x00);    // 0 dB
    WriteToSubAddress(TDA9874A_C2OLAR, 0x00);    // 0 dB

    // NICAM settings
    WriteToSubAddress(TDA9874A_NCONR, 0x00);    // Enable NICAM automute on BER (switchs to mono), and NICAM deemphasis. Should be selected by user based on needs
    WriteToSubAddress(TDA9874A_NOLAR, 0x0a);    // 10 dB
    WriteToSubAddress(TDA9874A_NLELR, 0x14);    // NICAM lower error limit
    WriteToSubAddress(TDA9874A_NUELR, 0x50);    // NICAM upper error limit

    // Start with something!
    SetChipMode(SOUNDCHANNEL_STEREO);

    // Fix Me
    //SetAudioConnector(AudioConTuner);
    SetChipStandard(FALSE);
    // Reinit autodetection
    return TRUE;
}

// Function to set Chip Standard
void CTDA9874AudioDecoder::SetChipStandard(BOOL FastCheck)
{
    WriteToSubAddress(TDA9874A_GCONR, 0xc0 | m_SIF | ((m_TDA9874Standards[m_AudioStandard].AGCOff) ? 0x02 : 0x00) ); // Select the proper SIF and turn AGC on/off if needed to
    WriteToSubAddress(TDA9874A_MSR, (m_TDA9874Standards[m_AudioStandard].Nicam) ? 0x03 : 0x02 ); // We monitor channel (1+2)/2 (not used by now)

    // Demodulator settings
    // Now program the needed Carrier frequencies...
    BYTE Buffer[3];
    Buffer[0] = BYTE((m_TDA9874Standards[m_AudioStandard].c1 >> 16) & 0xFF);
    Buffer[1] = BYTE((m_TDA9874Standards[m_AudioStandard].c1 >> 8 ) & 0xFF);
    Buffer[2] = BYTE((m_TDA9874Standards[m_AudioStandard].c1      ) & 0xFF);
    CI2CDevice::WriteToSubAddress(TDA9874A_C1FRA, Buffer, sizeof(Buffer));

    Buffer[0] = BYTE((m_TDA9874Standards[m_AudioStandard].c2 >> 16) & 0xFF);
    Buffer[1] = BYTE((m_TDA9874Standards[m_AudioStandard].c2 >> 8 ) & 0xFF);
    Buffer[2] = BYTE((m_TDA9874Standards[m_AudioStandard].c2      ) & 0xFF);
    CI2CDevice::WriteToSubAddress(TDA9874A_C2FRA, Buffer, sizeof(Buffer));

    WriteToSubAddress(TDA9874A_DCR,
        m_TDA9874Standards[m_AudioStandard].DCR | ((FastCheck) ? 0x80 : 0x00) ); // Fast Detect if asked to

    // FM Settings
    WriteToSubAddress(TDA9874A_FMER, m_TDA9874Standards[m_AudioStandard].FMER);

    // Set the signal source...
    WriteToSubAddress(TDA9874A_SDACOSR,    (m_TDA9874Standards[m_AudioStandard].Nicam) ? 0x01:0x00); /* Signal source */
}

// Function to change audio mode
void CTDA9874AudioDecoder::SetChipMode(eSoundChannel audio)
{
    LOG(1, _T("SetChipMode audio=%d"), (int)audio);

    // FM settings
    WriteToSubAddress(TDA9874A_FMMR,
        (  audio == SOUNDCHANNEL_STEREO ) ? 0x00 :
        (( audio == SOUNDCHANNEL_LANGUAGE1 || audio == SOUNDCHANNEL_LANGUAGE2) ? 0x02 :
        m_TDA9874Standards[m_AudioStandard].FMMR) );

    // Select the proper outs.
    WriteToSubAddress(TDA9874A_AOSR,
        (  audio == SOUNDCHANNEL_MONO) ? ((m_AudioStandard==8)?0x43:0x40) :
        (( audio == SOUNDCHANNEL_STEREO ) ? 0x00 :
        (( audio == SOUNDCHANNEL_LANGUAGE1 ) ? 0x10 :
        (( audio == SOUNDCHANNEL_LANGUAGE2 ) ? 0x20 : 0x00 ))) );

    // Fix Me
    // Si mode mono => On passe en AM/FM
    //WriteToSubAddress(TDA9874A_SDACOSR, (( (m_TDA9874Standards[m_AudioStandard].Nicam) && (desiredam!=SOUNDCHANNEL_MONO) )?0x01:0x00));
}

CAudioDecoder::eAudioDecoderType CTDA9874AudioDecoder::GetAudioDecoderType()
{
    return AUDIODECODERTYPE_TDA9874;
}

void CTDA9874AudioDecoder::SetSoundChannel(eSoundChannel SoundChannel)
{
    CAudioDecoder::SetSoundChannel(SoundChannel);
    int audio = SoundChannel;
    LOG(1, _T("TDA9874: SetSoundChannel:%d"), SoundChannel);

    // FM settings
    WriteToSubAddress(TDA9874A_FMMR,
        (  audio == SOUNDCHANNEL_MONO ) ? 0x00 :
        (( audio == SOUNDCHANNEL_LANGUAGE1 || audio == SOUNDCHANNEL_LANGUAGE2 ) ? 0x02 :
        m_TDA9874Standards[m_AudioStandard].FMMR) );

    // Select the proper outs.
    WriteToSubAddress(TDA9874A_AOSR,
        (  audio == SOUNDCHANNEL_MONO )        ? 0x40 :
        (( audio == SOUNDCHANNEL_STEREO  )    ? 0x00 :
        (( audio == SOUNDCHANNEL_LANGUAGE1  ) ? 0x10 :
        (( audio == SOUNDCHANNEL_LANGUAGE2  ) ? 0x20 : 0x00 ))) );
}

eSoundChannel CTDA9874AudioDecoder::IsAudioChannelDetected(eSoundChannel DesiredAudioChannel)
{
    LOG(1,_T("TDA9874: IsAudioChannelDetected:%d"),DesiredAudioChannel);
    eSoundChannel SoundChannel = SOUNDCHANNEL_MONO;
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
    return SoundChannel;
}

void CTDA9874AudioDecoder::SetAudioInput(eAudioInput AudioInput)
{
    CAudioDecoder::SetAudioInput(AudioInput);
    LOG(1, _T("TDA9874: SetAudioInput:%d"), AudioInput);

    switch(AudioInput)
    {
    case AUDIOINPUT_RADIO:
    case AUDIOINPUT_STEREO:
    case AUDIOINPUT_INTERNAL:
    case AUDIOINPUT_EXTERNAL:
    case AUDIOINPUT_MUTE:
        WriteToSubAddress(TDA9874A_AMCONR, 0xFF); // mute enable ALL outs
        break;
    default:
        WriteToSubAddress(TDA9874A_AMCONR, 0xF9); // unmute enable ALL outs
        //Audio_SetUserMute(FALSE);
        break;
    }
}

long CTDA9874AudioDecoder::GetAudioStandardMajorCarrier(long Standard)
{
    LOG(1,_T("getMajorCarrier, standard=%d"),Standard);
    return m_TDA9874Standards[Standard].c1;
}

long CTDA9874AudioDecoder::GetAudioStandardMinorCarrier(long Standard)
{
    LOG(1,_T("getMinorCarrier, standard=%d"),Standard);
    return m_TDA9874Standards[Standard].c2;
}

int CTDA9874AudioDecoder::Standard2Index(long Standard)
{
    LOG(1, _T("TDA9874: standard2index"), Standard);
    for(int i = 0; i < GetNumAudioStandards(); ++i)
    {
        if(m_TDA9874Standards[i].Standard == Standard)
        {
            return i;
        }
    }

    return -1;
}

void CTDA9874AudioDecoder::SetAudioStandard(long Standard, eVideoFormat VideoFormat)
{
    LOG(1, _T("TDA9874: SetAudioStandard"), Standard);
    CAudioDecoder::SetAudioStandard(Standard, VideoFormat);

    int index = Standard2Index(Standard);

    if(index < 0)
    {
        return;
    }

    m_AudioStandard = index;

    // Set the signal source...
    WriteToSubAddress(TDA9874A_SDACOSR,
        (m_TDA9874Standards[m_AudioStandard].Nicam) ? 0x01:0x00); // Signal source

    WriteToSubAddress( TDA9874A_AMCONR, 0x00 ); // unmute

    // Now program the needed Carrier frequencies...
    BYTE Buffer[3];
    Buffer[0] = BYTE((m_TDA9874Standards[index].c1 >> 16) & 0xFF);
    Buffer[1] = BYTE((m_TDA9874Standards[index].c1 >> 8 ) & 0xFF);
    Buffer[2] = BYTE((m_TDA9874Standards[index].c1      ) & 0xFF);
    CI2CDevice::WriteToSubAddress(TDA9874A_C1FRA, Buffer, sizeof(Buffer));

    Buffer[0] = BYTE((m_TDA9874Standards[index].c2 >> 16) & 0xFF);
    Buffer[1] = BYTE((m_TDA9874Standards[index].c2 >> 8 ) & 0xFF);
    Buffer[2] = BYTE((m_TDA9874Standards[index].c2      ) & 0xFF);
    CI2CDevice::WriteToSubAddress(TDA9874A_C2FRA, Buffer, sizeof(Buffer));
}

const TCHAR* CTDA9874AudioDecoder::GetAudioStandardName(long Standard)
{
    int index = Standard2Index(Standard);

    if(index < 0)
    {
        return NULL;
    }

    return m_TDA9874Standards[index].Name;
}

int CTDA9874AudioDecoder::GetNumAudioStandards()
{
    return sizeof(m_TDA9874Standards) / sizeof(m_TDA9874Standards[0]);
}

long CTDA9874AudioDecoder::GetAudioStandard(int nIndex)
{
    if ((nIndex < 0) || (nIndex >= GetNumAudioStandards()) )
    {
        return 0;
    }

    return m_TDA9874Standards[nIndex].Standard;
}

long CTDA9874AudioDecoder::GetAudioStandardFromVideoFormat(eVideoFormat videoFormat)
{
    // ToDo: Check, add, ...
    // Guess the correct format
    WORD standard;
    switch(videoFormat)
    {
    case VIDEOFORMAT_PAL_B:
        standard = TDA9874_STANDARD_BG_DIGITAL_FM_NICAM;
        break;
    case VIDEOFORMAT_PAL_D:
        standard = TDA9874_STANDARD_DK1_ANALOG_FM_FM;
        break;
    case VIDEOFORMAT_PAL_G:
        standard = TDA9874_STANDARD_BG_DIGITAL_FM_NICAM;
        break;
    case VIDEOFORMAT_PAL_H:
        // \todo FIXME
        standard = TDA9874_STANDARD_NONE;
        break;
    case VIDEOFORMAT_PAL_I:
        standard = TDA9874_STANDARD_I_ANALOG_FM_NONE;
        break;
    case VIDEOFORMAT_PAL_M:
        // \todo FIXME
        standard = TDA9874_STANDARD_M_ANALOG_FM_NONE;
        break;
    case VIDEOFORMAT_PAL_N:
        // \todo FIXME
        standard = TDA9874_STANDARD_NONE;
        break;
    case VIDEOFORMAT_PAL_N_COMBO:
        // \todo FIXME
        standard = TDA9874_STANDARD_NONE;
        break;
    case VIDEOFORMAT_SECAM_B:
        standard = TDA9874_STANDARD_BG_ANALOG_FM_FM;
        break;
    case VIDEOFORMAT_SECAM_D:
        standard = TDA9874_STANDARD_DK1_ANALOG_FM_FM;
        break;
    case VIDEOFORMAT_SECAM_G:
        standard = TDA9874_STANDARD_BG_ANALOG_FM_FM;
        break;
    case VIDEOFORMAT_SECAM_H:
        standard = TDA9874_STANDARD_DK1_ANALOG_FM_FM;
        break;
    case VIDEOFORMAT_SECAM_K:
        standard = TDA9874_STANDARD_DK1_ANALOG_FM_FM;
        break;
    case VIDEOFORMAT_SECAM_K1:
        standard = TDA9874_STANDARD_DK2_ANALOG_FM_FM;
        break;
    case VIDEOFORMAT_SECAM_L:
        standard = TDA9874_STANDARD_L_DIGITAL_AM_NICAM;
        break;
    case VIDEOFORMAT_SECAM_L1:
        standard = TDA9874_STANDARD_L_DIGITAL_AM_NICAM;
        break;
    default:
    case VIDEOFORMAT_PAL_60:
    case VIDEOFORMAT_NTSC_50:
        standard = TDA9874_STANDARD_NONE;
        break;
    }
    return standard;
}

void CTDA9874AudioDecoder::DetectAudioStandard(long Interval, int SupportedSoundChannels, eSoundChannel TargetChannel)
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
        LOGD(_T("Abort1 TDA9874 detect loop"));
        m_ThreadWait = TRUE;
        Sleep(10);

        if (m_AutoDetecting)
        {
            LOGD(_T("Abort2 TDA9874 detect loop"));
            Sleep(50);

            if (m_AutoDetecting)
            {
                LOGD(_T("Abort3 TDA9874 detect loop"));
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

DWORD WINAPI TDA9874ThreadProc(LPVOID lpThreadParameter)
{
    int Result = 1;

    DScalerThread thisThread(_T("TDA9874DetectThread"));
    if (lpThreadParameter != NULL)
    {
        try
        {
            Result = ((CTDA9874AudioDecoder*)lpThreadParameter)->DetectThread();
        }
        catch(...)
        {
            LOG(1, _T("Crash in TDA9874 detect loop"));
            return 1;
        }
    }
    return Result;
}

void CTDA9874AudioDecoder::StartThread()
{
    DWORD LinkThreadID;

    m_ThreadWait = FALSE;

    if (m_TDA9874Thread != NULL)
    {
        // Already started, resume
        if(ResumeThread(m_TDA9874Thread) == 0)
        {
            LOG(1, _T("TDA9874 detect loop not waiting(1)"));
            Sleep(10);
            if(ResumeThread(m_TDA9874Thread) == 0)
            {
                LOG(1, _T("TDA9874 detect loop not waiting(2)"));
            }
        }
        return;
    }

    m_StopThread = FALSE;

    m_TDA9874Thread = CreateThread((LPSECURITY_ATTRIBUTES) NULL,    // No security.
                             (DWORD) 0,                                // Same stack size.
                             TDA9874ThreadProc,                        // Thread procedure.
                             (LPVOID)this,                          // Parameter.
                             (DWORD) 0,                                // Start immediatly.
                             (LPDWORD) &LinkThreadID);                // Thread ID.
}

void CTDA9874AudioDecoder::StopThread()
{
    if (m_TDA9874Thread != NULL)
    {
        // Signal the stop
        m_StopThread = TRUE;
        ResumeThread(m_TDA9874Thread);

        // Wait one second for the thread to exit gracefully
        DWORD dwResult = WaitForSingleObject(m_TDA9874Thread, 1000);

        if (dwResult != WAIT_OBJECT_0)
        {
            LOG(1,_T("TDA9874 detect loop did not terminate gracefully: terminating!"));
            TerminateThread(m_TDA9874Thread, 0);
        }

        CloseHandle(m_TDA9874Thread);
        m_TDA9874Thread = NULL;
    }
}

int CTDA9874AudioDecoder::DetectThread()
{
    int DetectCounter = 1;
    m_SupportedSoundChannels = SUPPORTEDSOUNDCHANNEL_MONO;

    while (!m_StopThread)
    {
        if (m_ThreadWait)
        {
            LOGD(_T("Waiting TDA9874 detect loop"));
            m_AutoDetecting = 0;
            SuspendThread(m_TDA9874Thread);
            LOGD(_T("Resuming TDA9874 detect loop"));
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
                m_ThreadWait = TRUE;    //Finished
            }

            DetectCounter = 1;
        }

        if (m_AutoDetecting==2)         //Detect mono/stereo/lang1/lang2
        {
            if ((DetectCounter >= 100) && (DetectCounter % m_DetectInterval10ms) == 0)
            {
                eSupportedSoundChannels Supported = SUPPORTEDSOUNDCHANNEL_LANG2;
                int iSupported = (int)Supported;

                // Fix Me

                /*
                BYTE status = ReadStatus();

                if ((status & TDA9874IN_CHANNELMASK) == TDA9874IN_STEREO)
                {
                    iSupported |= SUPPORTEDSOUNDCHANNEL_STEREO;
                }
                if ((status & TDA9874IN_CHANNELMASK) == TDA9874IN_LANG12)
                {
                    iSupported |= SUPPORTEDSOUNDCHANNEL_LANG1|SUPPORTEDSOUNDCHANNEL_LANG2;
                }

                Supported = (eSupportedSoundChannels)iSupported;

                if ((Supported != m_SupportedSoundChannels) || (DetectCounter >= 200))
                {
                    m_SupportedSoundChannels = Supported;

                    LOG(1,_T("TDA9874: Detect stereo: Supported: %s%s%s%s"),
                        (Supported&SUPPORTEDSOUNDCHANNEL_MONO)?_T("Mono "):_T(""),
                        (Supported&SUPPORTEDSOUNDCHANNEL_STEREO)?_T("Stereo "):_T(""),
                        (Supported&SUPPORTEDSOUNDCHANNEL_LANG1)?_T("Lang1 "):_T(""),
                        (Supported&SUPPORTEDSOUNDCHANNEL_LANG2)?_T("Lang2 "):_T(""));

                    SetSoundChannel(IsAudioChannelDetected(m_TargetSoundChannel));

                    EventCollector->RaiseEvent(this, EVENT_AUDIOCHANNELSUPPORT_DETECTED, 0, Supported);

                    if (DetectCounter >= 200)
                    {
                        m_ThreadWait = TRUE;    //Finished
                    }
                }
                */
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
