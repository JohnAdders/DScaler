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
//
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////

/** 
 * @file mixerdev.h mixerdev Header file
 */
 
#ifndef __MIXERDEV_H___
#define __MIXERDEV_H___

#include "settings.h"


//  Mixer functions

void    Mixer_Init();
void    Mixer_Exit();

BOOL    Mixer_IsEnabled();
BOOL    Mixer_IsNoHardwareMute();

void    Mixer_SetMute(BOOL bEnabled);
BOOL    Mixer_GetMute(void);

void    Mixer_SetVolume(long newVolume);
long    Mixer_GetVolume();
void    Mixer_AdjustVolume(long delta);

void    Mixer_Mute();
void    Mixer_UnMute();

void    Mixer_Volume_Up();
void    Mixer_Volume_Down();

void    Mixer_SetupDlg(HWND hWndParent);


//  Settings functions

SETTING*    MixerDev_GetSetting(MIXERDEV_SETTING nSetting);
void        MixerDev_ReadSettingsFromIni();
void        MixerDev_WriteSettingsToIni(BOOL bOptimizeFileAccess);
void        MixerDev_FreeSettings();
void        MixerDev_SetMenu(HMENU hMenu);




//  Internal classes

/**  Individual lines of a destination (e.g. MIDI, Line-In, CD-Audio)
*/
class CMixerLineSrc
{
public:
    CMixerLineSrc(HMIXER hMixer, DWORD nDstIndex, DWORD nSrcIndex);
    virtual ~CMixerLineSrc();

    const char* GetName();

    void        SetMute(BOOL bEnable);
    BOOL        GetMute();

    void        SetVolume(int volumePercentage);
    int         GetVolume();

    void        StoreState();
    void        RestoreState();

private:
    BOOL        MixerControlDetailsSet(DWORD dwControlID, DWORD dwValue);
    BOOL        MixerControlDetailsGet(DWORD dwControlID, LPDWORD lpdwValue);

private:
    HMIXER      m_hMixer;
    MIXERLINE   m_mxl;

    DWORD       m_VolumeControlID;
    DWORD       m_VolumeMinimum;
    DWORD       m_VolumeMaximum;

    DWORD       m_MuteControlID;

    DWORD       m_StoredVolume;
    DWORD        m_StoredMute;
};


/**  System mixer destination lines
*/
class CMixerLineDst
{
public:
    CMixerLineDst(HMIXER hMixer, DWORD nDstIndex);
    virtual ~CMixerLineDst();

    const char*     GetName();

    long            GetSourceCount();
    CMixerLineSrc*  GetSourceLine(DWORD nIndex);

    void            StoreState();
    void            RestoreState();

    BOOL            IsTypicalSpeakerLine();
    BOOL            IsTypicalRecordingLine();

private:
    MIXERLINE       m_mxl;

    DWORD           m_nSourceCount;
    CMixerLineSrc** m_pSourceLines;
};


/**  System mixer control
*/
class CMixer
{
public:
    CMixer(DWORD nMixerIndex);
    virtual ~CMixer();

    const char*     GetName();
    DWORD           GetIndex();

    long            GetDestinationCount();
    CMixerLineDst*  GetDestinationLine(DWORD nIndex);

    void            StoreState();
    void            RestoreState();

private:
    HMIXER          m_hMixer;
    MIXERCAPS       m_mxcaps;

    DWORD           m_nMixerIndex;

    DWORD           m_nDestinationCount;
    CMixerLineDst** m_pDestinationLines;
};


/**  Provides functions for finding mixers
*/
class CMixerFinder
{
public:
    CMixerFinder();
    virtual ~CMixerFinder();

    long        GetMixerCount();
    BOOL        GetMixerName(long nMixerIndex, char szName[MAXPNAMELEN]);

    long        FindMixer(const char* szName);

private:
    UINT        m_nMixerCount;
};


#endif

