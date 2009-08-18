/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 John Adcock.  All rights reserved.
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
//
// This code is based on a version of dTV modified by Michael Eskin and
// others at Connexant.  Those parts are probably (c) Connexant 2002
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file CX2388xSource.cpp CCX2388xSource Implementation (Audio)
 */

#include "stdafx.h"

#ifdef WANT_CX2388X_SUPPORT

#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "CX2388xSource.h"
#include "CX2388x_Defines.h"
#include "DScaler.h"
#include "OutThreads.h"
#include "AspectRatio.h"
#include "DebugLog.h"
#include "SettingsPerChannel.h"
#include "status.h"
#include "Audio.h"

void CCX2388xSource::Mute()
{
    m_pCard->SetAudioMute();
}

void CCX2388xSource::UnMute()
{
    m_pCard->SetAudioUnMute((WORD)m_Volume->GetValue());
}

void CCX2388xSource::VolumeOnChange(long NewValue, long OldValue)
{
    m_pCard->SetAudioVolume((WORD)NewValue);
    EventCollector->RaiseEvent(this, EVENT_VOLUME, OldValue, NewValue);
}

void CCX2388xSource::BalanceOnChange(long NewValue, long OldValue)
{
    m_pCard->SetAudioBalance((WORD)NewValue);
}

void CCX2388xSource::AudioStandardOnChange(long NewValue, long OldValue)
{
    StopUpdateAudioStatus();
    m_pCard->AudioInit(
                        m_VideoSource->GetValue(),
                        (eVideoFormat)m_VideoFormat->GetValue(),
                        (eCX2388xAudioStandard)NewValue,
                        (eCX2388xStereoType)m_StereoType->GetValue()
                      );

    // needed, AUD_VOL_CTRL's default is 0x14 after writing to AUD_SOFT_RESET
    m_Volume->DoOnChangeEvent();

    StartUpdateAudioStatus();
}

void CCX2388xSource::StereoTypeOnChange(long NewValue, long OldValue)
{
    StopUpdateAudioStatus();
    m_pCard->AudioInit(
                        m_VideoSource->GetValue(),
                        (eVideoFormat)m_VideoFormat->GetValue(),
                        (eCX2388xAudioStandard)m_AudioStandard->GetValue(),
                        (eCX2388xStereoType)NewValue
                      );

    m_Volume->DoOnChangeEvent();

    StartUpdateAudioStatus();
}

void CCX2388xSource::StartUpdateAudioStatus()
{
    m_AutoDetectA2Counter = 0;
    SetTimer(GetMainWnd(), TIMER_CX2388X, TIMER_CX2388X_MS, NULL);
}

void CCX2388xSource::StopUpdateAudioStatus()
{
    KillTimer(GetMainWnd(), TIMER_CX2388X);
}

// called every 250ms
void CCX2388xSource::UpdateAudioStatus()
{
    eSoundChannel SoundChannel = SOUNDCHANNEL_MONO;

    if(IsInTunerMode())
    {
        if(IsVideoPresent())
        {
            if(TRUE == (BOOL)m_AutoMute->GetValue())
            {
                if(Audio_IsMute() && !Audio_GetUserMute())
                {
                    Audio_Unmute();
                }
            }

            switch(m_pCard->GetCurrentAudioStandard())
            {
            case AUDIO_STANDARD_A2:
                switch(m_pCard->GetCurrentStereoType())
                {
                case STEREOTYPE_AUTO:
                    switch((eVideoFormat)m_VideoFormat->GetValue())
                    {
                    case VIDEOFORMAT_PAL_I:
                        SoundChannel = AutoDetect_I();
                        break;

                    case VIDEOFORMAT_PAL_B:
                    case VIDEOFORMAT_PAL_G:
                    case VIDEOFORMAT_PAL_D:
                    case VIDEOFORMAT_SECAM_K:
                    case VIDEOFORMAT_SECAM_D:
                        SoundChannel = AutoDetect_BGDK();
                        break;
                    }
                    break;

                case STEREOTYPE_MONO:
                    SoundChannel = SOUNDCHANNEL_MONO;
                    break;

                case STEREOTYPE_STEREO:
                    SoundChannel = SOUNDCHANNEL_STEREO;
                    break;

                case STEREOTYPE_ALT1:
                    SoundChannel = SOUNDCHANNEL_LANGUAGE1;
                    break;

                case STEREOTYPE_ALT2:
                    SoundChannel = SOUNDCHANNEL_LANGUAGE2;
                    break;
                }

                break;

            case AUDIO_STANDARD_NICAM:
                switch(m_pCard->GetCurrentStereoType())
                {
                case STEREOTYPE_AUTO:
                case STEREOTYPE_STEREO:
                    SoundChannel = SOUNDCHANNEL_STEREO;
                    break;

                case STEREOTYPE_MONO:
                    SoundChannel = SOUNDCHANNEL_MONO;
                    break;

                case STEREOTYPE_ALT1:
                    SoundChannel = SOUNDCHANNEL_LANGUAGE1;
                    break;

                case STEREOTYPE_ALT2:
                    SoundChannel = SOUNDCHANNEL_LANGUAGE2;
                    break;
                }

                break;

            case AUDIO_STANDARD_BTSC:
                switch(m_pCard->GetCurrentStereoType())
                {
                case STEREOTYPE_AUTO:
                case STEREOTYPE_STEREO:
                    SoundChannel = SOUNDCHANNEL_STEREO;
                    break;

                case STEREOTYPE_MONO:
                case STEREOTYPE_ALT1:
                case STEREOTYPE_ALT2:
                    SoundChannel = SOUNDCHANNEL_MONO;
                    break;
                }

                break;

            case AUDIO_STANDARD_BTSC_SAP:
                SoundChannel = SOUNDCHANNEL_STEREO;
                break;

            case AUDIO_STANDARD_AUTO:
            case AUDIO_STANDARD_EIAJ:
            case AUDIO_STANDARD_FM:
                break;    // \todo: add more support
            }
        }

        // no Video present
        else
        {
            if(TRUE == (BOOL)m_AutoMute->GetValue())
            {
                if(!Audio_IsMute())
                {
                    Audio_Mute();
                }
            }

            m_AutoDetectA2Counter = 0;
        }
    }

    // if not in tuner mode, show always Stereo
    else
    {
        SoundChannel = SOUNDCHANNEL_STEREO;
    }

    TCHAR szSoundChannel[256] = _T("");

    switch(SoundChannel)
    {
    case SOUNDCHANNEL_MONO:
        _tcscpy(szSoundChannel,_T("Mono"));
        break;

    case SOUNDCHANNEL_STEREO:
        _tcscpy(szSoundChannel,_T("Stereo"));
        break;

    case SOUNDCHANNEL_LANGUAGE1:
        _tcscpy(szSoundChannel,_T("Language 1"));
        break;

    case SOUNDCHANNEL_LANGUAGE2:
        _tcscpy(szSoundChannel,_T("Language 2"));
        break;
    }

    StatusBar_ShowText(STATUS_AUDIO, szSoundChannel);
    EventCollector->RaiseEvent(this, EVENT_SOUNDCHANNEL, -1, SoundChannel);
}

eSoundChannel CCX2388xSource::AutoDetect_BGDK()
{
    static int iA2DetectCnt = 0;

    // Start Auto Detecting...
    if(m_AutoDetectA2Counter == 0)
    {
        // ... with Mono.
        iA2DetectCnt = 0;
        m_pCard->WriteDword(AUD_PHASE_FIX_CTL, 0x00000000);
        m_pCard->AndOrDataDword(AUD_DEEMPH1_SRC_SEL, 0, ~0x00000002);
    }

    // Timeout Auto Detecting after 10 * 250 ms
    if(m_AutoDetectA2Counter == 10)
    {
        if(iA2DetectCnt < 3)
        {
            // Stereo not found; switch to Mono
            m_pCard->OrDataDword(AUD_DEEMPH1_SRC_SEL, 0x00000002);
            m_pCard->WriteDword(AUD_CTL, EN_DAC_ENABLE|EN_FMRADIO_EN_RDS|EN_DMTRX_SUMDIFF|EN_A2_FORCE_MONO1);
        }

        m_AutoDetectA2Counter++;
    }

    else if(m_AutoDetectA2Counter < 10)
    {
        DWORD dwVal = m_pCard->ReadDword(AUD_STATUS) & 0x03;

        if(dwVal == 0)
        {
            // Stereo detected
            iA2DetectCnt++;
            if(iA2DetectCnt == 3)
            {
                // after 3x detected switch to Stereo
                m_pCard->WriteDword(AUD_MODE_CHG_TIMER,        0x000000f0);
                m_pCard->WriteDword(AUD_PHASE_FIX_CTL,        0x00000001);
                m_pCard->OrDataDword(AUD_DEEMPH1_SRC_SEL,    0x00000002);
                m_pCard->WriteDword(AUD_CTL, EN_DAC_ENABLE|EN_FMRADIO_EN_RDS|EN_DMTRX_SUMDIFF|EN_A2_FORCE_STEREO);
            }
        }

        m_AutoDetectA2Counter++;
    }

    eSoundChannel SoundChannel;
    iA2DetectCnt >= 3 ? SoundChannel = SOUNDCHANNEL_STEREO : SoundChannel = SOUNDCHANNEL_MONO;
    return SoundChannel;
}

eSoundChannel CCX2388xSource::AutoDetect_I()
{
    // \Todo: add nicam detection
    eSoundChannel SoundChannel = SOUNDCHANNEL_MONO;
    return SoundChannel;
}

#endif // WANT_CX2388X_SUPPORT
