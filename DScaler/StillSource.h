/////////////////////////////////////////////////////////////////////////////
// $Id: StillSource.h,v 1.2 2001-11-02 16:30:08 adcockj Exp $
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

#include "InterlacedSource.h"

class CStillSource : public CInterlacedSource
{
public:
    CStillSource();
    ~CStillSource();
    void CreateSettings(LPCSTR IniSection);
    void Start();
    void Stop();
    void Reset();
    void GetNextField(DEINTERLACE_INFO* pInfo, BOOL AccurateTiming);
    BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam);
    LPCSTR GetStatus();
    CSetting* GetVolume();
    CSetting* GetBalance();
    void Mute();
    void UnMute();
    CSetting* GetBrightness();
    CSetting* GetContrast();
    CSetting* GetHue();
    CSetting* GetSaturation();
    CSetting* GetSaturationU();
    CSetting* GetSaturationV();
    eVideoFormat GetFormat();
    BOOL IsInTunerMode() {return FALSE;}
    int GetWidth() {return 720;};
    int GetHeight() {return 480;};
    BOOL HasTuner();
    void SetMenu(HMENU hMenu);
    void HandleTimerMessages(int TimerId);
    BOOL SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat) {return FALSE;};
    BOOL IsVideoPresent() {return TRUE;};
    void DecodeVBI(DEINTERLACE_INFO* pInfo) {;};
private:
};

#endif