/////////////////////////////////////////////////////////////////////////////
// $Id: StillSource.h,v 1.7 2001-11-25 10:41:26 laurentg Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
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

#ifndef __STILLSOURCE_H___
#define __STILLSOURCE_H___

#include "Source.h"

class CStillSource : public CSource
{
public:
    CStillSource(LPCSTR FilePath);
    CStillSource(LPCSTR FilePath, int FrameHeight, int FrameWidth, BYTE* pOverlay, LONG OverlayPitch);
    ~CStillSource();
    void CreateSettings(LPCSTR IniSection);
    void Start();
    void Stop();
    void Reset();
    void GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming);
    BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam);
    LPCSTR GetStatus();
    CSetting* GetVolume() {return NULL;};
    CSetting* GetBalance() {return NULL;};
    void Mute() {;};
    void UnMute() {;};
    CSetting* GetBrightness() {return NULL;};
    CSetting* GetContrast() {return NULL;};
    CSetting* GetHue() {return NULL;};
    CSetting* GetSaturation() {return NULL;};
    CSetting* GetSaturationU() {return NULL;};
    CSetting* GetSaturationV() {return NULL;};
    eVideoFormat GetFormat();
    BOOL IsInTunerMode() {return FALSE;};
    int GetWidth();
    int GetHeight();
    BOOL HasTuner() {return FALSE;};
    void SetMenu(HMENU hMenu);
    void HandleTimerMessages(int TimerId);
    BOOL SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat) {return FALSE;};
    BOOL IsVideoPresent() {return TRUE;};
    void DecodeVBI(TDeinterlaceInfo* pInfo) {;};

protected:
    int         m_Width;
    int         m_Height;
    TPicture    m_StillFrame;
    char        m_FilePath[256];
    BOOL        m_AlreadyTryToRead;

private:
    virtual BOOL    ReadNextFrameInFile() = 0;

    DWORD       m_LastTickCount;
    double      m_FrameDuration;
};

#endif