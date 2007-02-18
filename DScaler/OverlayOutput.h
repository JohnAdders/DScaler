/////////////////////////////////////////////////////////////////////////////
// $Id: OverlayOutput.h,v 1.4 2007-02-18 15:02:16 robmuller Exp $
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
// CVS Log
//
// $Log: not supported by cvs2svn $
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "ioutput.h"

typedef struct 
{
	HMONITOR hMon;
	LPDIRECTDRAW lpDD;
} TMonitor;
#define MAX_MONITORS	4
static TMonitor Monitors[MAX_MONITORS];
static int NbMonitors=0;
static HMONITOR hCurrentMon=0;


class COverlayOutput :
	public IOutput
{
public:
	COverlayOutput(void);
	~COverlayOutput(void);

	void SetCurrentMonitor(HWND hWnd);
	void CheckChangeMonitor(HWND hWnd);
	BOOL CanDoOverlayColorControl();
	BOOL OverlayActive();
	void Overlay_Clean();
	BOOL Overlay_Update(LPRECT pSrcRect, LPRECT pDestRect, DWORD dwFlags);
	void Overlay_ResetColorControls();
	void Overlay_SetColorControls();
	BOOL Overlay_Create();
	DWORD Overlay_ColorMatch(LPDIRECTDRAWSURFACE pdds, COLORREF rgb);
	BOOL Overlay_Destroy();
	COLORREF Overlay_GetColor();
	COLORREF Overlay_GetCorrectedColor(HDC hDC);
	BOOL Overlay_Lock_Extra_Buffer(TDeinterlaceInfo* pInfo);
	BOOL Overlay_Lock_Back_Buffer(TDeinterlaceInfo* pInfo, BOOL bUseExtraBuffer);
	BOOL Overlay_Lock(TDeinterlaceInfo* pInfo);
	BOOL Overlay_Unlock_Back_Buffer(BOOL bUseExtraBuffer);
	BOOL Overlay_Unlock();
	void Overlay_Copy_External(BYTE* lpExternalMemoryBuffer, int ExternalPitch, TDeinterlaceInfo* pInfo);
	void Overlay_Copy_Extra(TDeinterlaceInfo* pInfo);
	BOOL Overlay_Flip(DWORD FlipFlag, BOOL bUseExtraBuffer, BYTE* lpExternalMemoryBuffer, int ExternalPitch, TDeinterlaceInfo* pInfo);
	HDC Overlay_GetDC();
	void Overlay_ReleaseDC(HDC hDC);
	BOOL InitDD(HWND hWnd);
	void ExitDD(void);
    void WaitForVerticalBlank();
	void Overlay_SetRGB(BOOL IsRGB);
	BOOL Overlay_GetRGB();

    void InitOtherSettings();
	
	CTreeSettingsGeneric* Other_GetTreeSettingsPage();

	OUTPUTTYPES Type();

private:
	void LoadDynamicFunctions();

	// this critical section is used to make sure that we don't mess about with the
	// overlay in one thread while the other thread is doing something with it
	// it is held while the primary surface DC is held
	// and while the overlay is locked
	CRITICAL_SECTION hDDCritSect;

	LPDIRECTDRAWSURFACE     lpDDSurface;
	// OverLay
	LPDIRECTDRAWSURFACE     lpDDOverlay;
	LPDIRECTDRAWSURFACE     lpDDOverlayBack;
	BYTE*                   lpExtraMemoryForFilters;
	BOOL bCanColorKey;
	
	COLORREF g_OverlayColor;
	long BackBuffers;     // Make new user parm, TRB 10/28/00
	BOOL bAllowBobMode;
	BOOL bCanDoBob;
	BOOL bCanDoFlipInterval;
	BOOL bCanDoColorKey;
	DDCOLORCONTROL OriginalColorControls;
	LPDIRECTDRAWCOLORCONTROL pDDColorControl;
	BOOL bUseOverlayControls;
	ULONG OutputTicksPerFrame;

	long OverlayBrightness;
	long OverlayContrast;
	long OverlayHue;
	long OverlaySaturation;
	long OverlayGamma;
	long OverlaySharpness;
	HRESULT FlipResult;             // Need to try again for flip?
	BOOL bIsRGB;

	
	
	static BOOL WINAPI DDEnumCallbackEx(GUID* pGuid, LPTSTR pszDesc, LPTSTR pszDriverName,
							 VOID* pContext, HMONITOR hMonitor );
	static BOOL ListMonitors(HWND hWnd);

	LPDIRECTDRAW GetCurrentDD(HWND hWnd);

	static BOOL Overlay_ColorKey_OnChange(long NewValue);
	static BOOL Overlay_Brightness_OnChange(long NewValue);
	static BOOL Overlay_Contrast_OnChange(long NewValue);
	static BOOL Overlay_Hue_OnChange(long NewValue);
	static BOOL Overlay_Saturation_OnChange(long NewValue);
	static BOOL Overlay_Gamma_OnChange(long NewValue);
	static BOOL Overlay_Sharpness_OnChange(long NewValue);
	static BOOL Overlay_UseControls_OnChange(long NewValue);
    static BOOL Overlay_BackBuffers_OnChange(long NewValue);
    static BOOL Overlay_AllowBobMode_OnChange(long NewValue);
    
    LPDIRECTDRAW lpDD; 
};

extern COverlayOutput ConfigOutput;
