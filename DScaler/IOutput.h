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

#ifndef __IOUTPUT_H___
#define __IOUTPUT_H___

#include "settings.h"

#define DSCALER_MAX_WIDTH 1024
#define DSCALER_MAX_HEIGHT 768

// Interface for an output to screen (currently overlay, d3d9)
class IOutput
{
public:
    enum OUTPUTTYPES
    {
        OUT_OVERLAY,
        OUT_D3D
    };

    IOutput(void);
    virtual ~IOutput(void);

    // Get Hold of the Other.c file settings

    virtual void SetCurrentMonitor(HWND hWnd)=0;
    virtual void CheckChangeMonitor(HWND hWnd)=0;
    virtual void GetMonitorRect(HWND hWnd, RECT* rect);

    virtual void ExitDD(void)=0;
    virtual BOOL InitDD(HWND hWnd)=0;
    virtual BOOL CanDoOverlayColorControl()=0;
    virtual BOOL OverlayActive()=0;
    virtual BOOL Overlay_Update(LPRECT pSrcRect, LPRECT pDestRect, DWORD dwFlags)=0;
    virtual BOOL Overlay_Create()=0;
    virtual void Overlay_Clean()=0;
    virtual BOOL Overlay_Destroy()=0;
    virtual DWORD Overlay_ColorMatch(LPDIRECTDRAWSURFACE pdds, COLORREF rgb)=0;
    virtual COLORREF Overlay_GetColor()=0;
    virtual COLORREF Overlay_GetCorrectedColor(HDC hDC)=0;
    virtual void Overlay_SetColorControls()=0;

    ///////////////////////////////////////////////////////////////////////////
    // The following 2 pairs of function hold the critical section
    // between calls and so must always be used in pairs
    ///////////////////////////////////////////////////////////////////////////

    // **** WARNING ****
    // This function is paired with Overlay_Unlock_Back_Buffer
    // If this function succeeds then Overlay_Unlock_Back_Buffer must be called
    // on ALL paths
    virtual BOOL Overlay_Lock_Back_Buffer(TDeinterlaceInfo* pInfo, BOOL bUseExtraBuffer)=0;
    virtual BOOL Overlay_Unlock_Back_Buffer(BOOL bUseExtraBuffer)=0;
    // **** WARNING ****
    // This function is paired with Overlay_Unlock
    // If this function succeeds then Overlay_Unlock must be called
    // on ALL paths
    virtual BOOL Overlay_Lock(TDeinterlaceInfo* pInfo)=0;
    virtual BOOL Overlay_Unlock()=0;

    virtual BOOL Overlay_Flip(DWORD FlipFlag, BOOL bUseExtraBuffer, BYTE* lpExternalMemoryBuffer, int ExternalPitch, TDeinterlaceInfo* pInfo)=0;
    virtual void WaitForVerticalBlank() = 0;
    virtual void Overlay_SetRGB(BOOL IsRGB)=0;
    virtual BOOL Overlay_GetRGB()=0;

    virtual OUTPUTTYPES Type()=0;

    virtual void InitOtherSettings()=0;
    SETTING* GetOtherSettings();
    DWORD GetDestSizeAlign() const {return DestSizeAlign;};
    DWORD GetSrcSizeAlign() const {return SrcSizeAlign;};

protected:
    BOOL m_bSettingInitialized;
    SETTING OtherSettings[OTHER_SETTING_LASTONE];
    DWORD DestSizeAlign;
    DWORD SrcSizeAlign;
};

IOutput* GetActiveOutput();
void SetActiveOutput(IOutput::OUTPUTTYPES eType);

#endif
