/////////////////////////////////////////////////////////////////////////////
// $Id: MixerDev.h,v 1.11 2001-07-16 18:07:50 adcockj Exp $
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
//
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           Original Release
//                                     Translated most code from German
//                                     Combined Header files
//                                     Cut out all decoding
//                                     Cut out digital hardware stuff
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __MIXERDEV_H___
#define __MIXERDEV_H___

#include "settings.h"
#include "bt848.h"

SETTING* MixerDev_GetSetting(MIXERDEV_SETTING Setting);
void MixerDev_ReadSettingsFromIni();
void MixerDev_WriteSettingsToIni(BOOL bOptimizeFileAccess);
void MixerDev_SetMenu(HMENU hMenu);


class CMixerLineSource
{
public:
    CMixerLineSource(HMIXER hMixer, int DestId, int SourceId);
    ~CMixerLineSource();
    char* GetName();
    void SetMute(BOOL Mute);
    BOOL GetMute();
    void SetVolume(int PercentageVolume);
    int GetVolume();
private:
    int m_ControlsCount;
    MIXERLINE m_MixerLine;
    int m_VolumeControl;
    DWORD m_VolumeMin;
    DWORD m_VolumeMax;
    int m_MuteControl;
    HMIXER m_hMixer;
};

class CMixerLineDest
{
public:
    CMixerLineDest(HMIXER hMixer, int DestId);
    ~CMixerLineDest();
    int GetNumSourceLines();
    CMixerLineSource* GetSourceLine(int LineIndex);
    char* GetName();
private:
    int m_SourceCount;
    MIXERLINE m_MixerLine;
    CMixerLineSource** m_SourceLines;
};

class CMixer
{
public:
    CMixer(int MixerId);
    ~CMixer();
    int GetNumDestLines();
    CMixerLineDest* GetDestLine(int LineIndex);
private:
    int m_LineCount;
    MIXERCAPS m_MixerDev;
    CMixerLineDest** m_DestLines;
    HMIXER m_hMixer;
};

class CSoundSystem
{
public:
    CSoundSystem();
    ~CSoundSystem();
    int GetNumMixers();
    char* GetMixerName(int MixerIndex);
    void SetMixer(int MixerIndex);
    CMixer* GetMixer();
private:
    int m_DeviceCount;
    CMixer* m_Mixer;
};

BOOL APIENTRY MixerSetupProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
void Mixer_SetupDlg(HWND hWndParent);

extern BOOL bUseMixer;

void Mixer_Mute();
void Mixer_UnMute();
void Mixer_Volume_Up();
void Mixer_Volume_Down();
void Mixer_OnInputChange(eVideoSourceType NewType);
long Mixer_GetVolume();
void Mixer_Init();
void Mixer_Exit();

#endif
