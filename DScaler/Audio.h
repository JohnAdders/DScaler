/////////////////////////////////////////////////////////////////////////////
// $Id: Audio.h,v 1.7 2001-07-13 16:14:55 adcockj Exp $
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
// 11 Aug 2000   John Adcock           Moved Audio_ Function in here
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __AUDIO_H___
#define __AUDIO_H___

#include "settings.h"

// Get Hold of the Audio.cpp file settings
SETTING* Audio_GetSetting(AUDIO_SETTING Setting);
void Audio_ReadSettingsFromIni();
void Audio_WriteSettingsToIni();
void Audio_SetMenu(HMENU hMenu);


enum eAudioMuxType
{
    AUDIOMUX_TUNER = 0,
    AUDIOMUX_MSP_RADIO,
    AUDIOMUX_EXTERNAL,
    AUDIOMUX_INTERNAL,
    AUDIOMUX_MUTE,
    AUDIOMUX_STEREO
};

BOOL APIENTRY AudioSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY AudioSettingProc1(HWND hDlg, UINT message, UINT wParam, LONG lParam);

BOOL Audio_SetSource(eAudioMuxType nChannel);
void Audio_Autodetect();
BOOL Audio_WriteMSP(BYTE bSubAddr, int wAddr, int wData);
void Audio_SetToneControl();
int Audio_GetStereoMode(BOOL bHardwareMode);

void Audio_Mute();
void Audio_Unmute();

BOOL Audio_MSP_Init(BYTE DRead, BYTE DWrite);
BOOL Audio_MSP_IsPresent();
const char* Audio_MSP_Status();
const char* Audio_MSP_VersionString();
BOOL Audio_MSP_Reset();
BOOL Audio_MSP_Version();
void Audio_MSP_Set_MajorMinor_Mode(int MajorMode, int MinorMode);
void Audio_MSP_SetStereo(int MajorMode, int MinorMode, int Mode);
void Audio_MSP_Watch_Mode();
void Audio_MSP_Print_Mode();
void Audio_MSP_Mute(void); // MAE 8 Dec 2000
void Audio_MSP_Unmute(void);

BOOL APIENTRY AudioSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

// MSP34x0 definitions
#define MSP_CONTROL 0x00 // Software reset
#define MSP_TEST    0x01 // Internal use
#define MSP_WR_DEM  0x10 // Write demodulator
#define MSP_RD_DEM  0x11 // Read demodulator
#define MSP_WR_DSP  0x12 // Write DSP
#define MSP_RD_DSP  0x13 // Read DSP

#define MSP_MODE_AM_DETECT   0
#define MSP_MODE_FM_RADIO    2
#define MSP_MODE_FM_TERRA    3
#define MSP_MODE_FM_SAT      4
#define MSP_MODE_FM_NICAM1   5
#define MSP_MODE_FM_NICAM2   6

#define VIDEO_SOUND_MONO    1
#define VIDEO_SOUND_STEREO  2
#define VIDEO_SOUND_LANG1   3
#define VIDEO_SOUND_LANG2   4

// This macro is allowed for *constants* only, gcc must calculate it
// at compile time.  Remember -- no floats in kernel Mode
#define MSP_CARRIER(freq) ((int)((float)(freq/18.432)*(1<<24)))

extern eAudioMuxType AudioSource;

#endif
