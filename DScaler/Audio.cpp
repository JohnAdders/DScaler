/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
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

/**
 * @file Audio.cpp Audio Functions
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "audio.h"
#include "Status.h"
#include "MixerDev.h"
#include "Providers.h"
#include "DebugLog.h"

CRITICAL_SECTION AudioMuteCriticalSection;
DWORD dwTimerProcThreadId = 0;
BYTE AudioMuteStatus = 0;
BOOL bUserMute = FALSE;

VOID CALLBACK AudioUnmuteDelayTimerProc(HWND hwnd, UINT, UINT_PTR idTimer, DWORD);

//  IMPORTANT: Audio_Mute() and Audio_Unmute() are for internal
//  use only.  It should be placed in pairs.  For user prompted
//  muting, use Audio_SetUserMute().


void Initialize_Mute()
{
    // This critical section isn't needed but may
    // be in future if Audio_Mute/Unmute are going
    // to be used asynchronously.
    InitializeCriticalSection(&AudioMuteCriticalSection);

    // Set the timer proc to only be called on this thread
    dwTimerProcThreadId = GetCurrentThreadId();

    Audio_Mute();

    // Again for user mute if it's initially set
    if(bUserMute == TRUE)
    {
        Audio_Mute();
    }
}


void Audio_Mute(DWORD PostMuteDelay)
{
    EnterCriticalSection(&AudioMuteCriticalSection);

    // Multi-state muting and the handling PostMuteDelay
    // and PreMuteDelay within these two simple
    // asynchronous safe functions alleviates the need
    // for complex and messy checks elsewhere in the program.
    if(++AudioMuteStatus == 1)
    {
        if(Mixer_IsEnabled())
        {
            Mixer_Mute();
        }

        if(!Mixer_IsNoHardwareMute() && Providers_GetCurrentSource())
        {
            Providers_GetCurrentSource()->Mute();
        }

        EventCollector->RaiseEvent(NULL, EVENT_MUTE, 0, 1);

        if(PostMuteDelay > 0)
        {
            Sleep(PostMuteDelay);
        }
    }

    LeaveCriticalSection(&AudioMuteCriticalSection);

    LOG(2, _T(" Mute Called Status on Exit %d"), AudioMuteStatus);
}


void Audio_Unmute(DWORD PreUnmuteDelay)
{
    EnterCriticalSection(&AudioMuteCriticalSection);

    if(AudioMuteStatus > 0)
    {
        if(PreUnmuteDelay > 0)
        {
            // This timer, together with the multi-state audio
            // mute structure, automatically finds the longest
            // delay time for us if there are multiple unmutes
            // around the same time.
            if(GetCurrentThreadId() != dwTimerProcThreadId ||
                !SetTimer(NULL, NULL, PreUnmuteDelay, AudioUnmuteDelayTimerProc))
            {
                // Timer creation failed so do the unmute now.
                PreUnmuteDelay = 0;
            }
        }
        if(PreUnmuteDelay == 0)
        {
            if(--AudioMuteStatus == 0)
            {
                // Always unmute the hardware contrary to how it's
                // done for mute, because the hardware needs to be
                // unmuted from its initial mute state.
                if(Providers_GetCurrentSource())
                {
                    Providers_GetCurrentSource()->UnMute();
                }

                if(Mixer_IsEnabled())
                {
                    Mixer_UnMute();
                }

                EventCollector->RaiseEvent(NULL, EVENT_MUTE, 1, 0);
            }
        }
    }

    LeaveCriticalSection(&AudioMuteCriticalSection);
    LOG(2, _T(" UnMute Called Status on Exit %d"), AudioMuteStatus);
}


BOOL Audio_IsMute()
{
    return (AudioMuteStatus > 0);
}


VOID CALLBACK AudioUnmuteDelayTimerProc(HWND hwnd, UINT, UINT_PTR idTimer, DWORD)
{
    KillTimer(hwnd, idTimer);
    Audio_Unmute(0UL);
}


void Audio_SetUserMute(BOOL bMute)
{
    if(bUserMute != bMute)
    {
        bUserMute = bMute;
        if(bMute == TRUE)
        {
            Audio_Mute();
        }
        else
        {
            Audio_Unmute();
        }
    }
}


BOOL Audio_GetUserMute()
{
    return bUserMute;
}


BOOL UserMute_OnChange(long NewValue)
{
    Audio_SetUserMute(NewValue);
    return FALSE;
}


////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING AudioSettings[AUDIO_SETTING_LASTONE] =
{
    {
        "System in Mute", ONOFF, 0, (LONG_PTR*)&bUserMute,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Audio", "Mute", UserMute_OnChange,
    },
};

SETTING* Audio_GetSetting(AUDIO_SETTING Setting)
{
    if(Setting > -1 && Setting < AUDIO_SETTING_LASTONE)
    {
        return &(AudioSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void Audio_SetMenu(HMENU hMenu)
{
    CheckMenuItemBool(hMenu, IDM_MUTE, bUserMute);
}

