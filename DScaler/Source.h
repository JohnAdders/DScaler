/////////////////////////////////////////////////////////////////////////////
// $Id: Source.h,v 1.3 2001-11-25 21:19:40 laurentg Exp $
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

#ifndef __SOURCE_H___
#define __SOURCE_H___

#include "DS_ApiCommon.h"
#include "Setting.h"
#include "TVFormats.h"

class CSource : public CSettingsHolder
{
public:
    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual void Reset() = 0;
    virtual void GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming) = 0;
    virtual BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam) = 0;
    virtual LPCSTR GetStatus() = 0;
    double GetFieldFrequency();
    virtual CSetting* GetVolume() = 0;
    virtual CSetting* GetBalance() = 0;
    virtual void Mute() = 0;
    virtual void UnMute() = 0;
    virtual CSetting* GetBrightness() = 0;
    virtual CSetting* GetContrast() = 0;
    virtual CSetting* GetHue() = 0;
    virtual CSetting* GetSaturation() = 0;
    virtual CSetting* GetSaturationU() = 0;
    virtual CSetting* GetSaturationV() = 0;
    virtual eVideoFormat GetFormat() = 0;
    virtual BOOL IsInTunerMode() = 0;
    virtual int GetWidth() = 0;
    virtual int GetHeight() = 0;
    virtual BOOL HasTuner() = 0;
    virtual void SetMenu(HMENU hMenu) = 0;
    virtual void HandleTimerMessages(int TimerId) = 0;
    virtual BOOL SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat) = 0;
    virtual BOOL IsVideoPresent() = 0;
    virtual void DecodeVBI(TDeinterlaceInfo* pInfo) = 0;
    virtual LPCSTR GetMenuLabel() = 0;

    HMENU GetSourceMenu();
protected:
    CSource(long SetMessage, long MenuId);
    ~CSource();
    double m_FieldFrequency;
    HMENU m_hMenu;
};



#endif