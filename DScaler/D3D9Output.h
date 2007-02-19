/////////////////////////////////////////////////////////////////////////////
// $Id: D3D9Output.h,v 1.6 2007-02-19 14:48:50 adcockj Exp $
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
// Revision 1.5  2007/02/19 10:13:45  adcockj
// Fixes for Critical thread and RECT issuesin D3D9 and overlay code
//
// Revision 1.4  2007/02/18 16:31:53  robmuller
// Added CVS log.
//
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "ioutput.h"
#include <d3d9.h>

#define D3D_ENABLEEPG 0xFFFFFFFF
#define D3D_DISABLEEPG 0xFFFFFFFE

class CD3D9Output :
	public IOutput
{
private:
	CRITICAL_SECTION hDDCritSect;
	LPDIRECT3D9 g_pD3D;
	LPDIRECT3DSURFACE9 lpDDOverlay, lpDDFrontBuffer;
	
	BYTE* lpExtraMemoryForFilters;

	SETTING OtherSettings[OTHER_SETTING_LASTONE];
	RECT srcrect;
    RECT destrect;	
	HWND m_hWnd;
	LPDIRECT3DTEXTURE9 m_lpOsdTexture;
	D3DPRESENT_PARAMETERS d3dpp; 
	__int64 m_FramesPresented;
	int BUFFERWIDTH, BUFFERHEIGHT;
	HRESULT FlipResult;
	HMONITOR hCurrentMon;
	BOOL bIsRGB;
public:
	LPDIRECT3DDEVICE9 pDevice;
	LPDIRECT3DSURFACE9 lpDDOSD;

	CD3D9Output(void);
	~CD3D9Output(void);

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

    ///////////////////////////////////////////////////////////////////////////
    // The following 2 pairs of function hold the critical section
    // between calls and so must always be used in pairs
    ///////////////////////////////////////////////////////////////////////////

    // **** WARNING ****
    // This function is paired with Overlay_Unlock_Back_Buffer
    // If this function succeeds then Overlay_Unlock_Back_Buffer must be called
    // on ALL paths
	virtual BOOL Overlay_Lock_Back_Buffer(TDeinterlaceInfo* pInfo, BOOL bUseExtraBuffer);
	virtual BOOL Overlay_Unlock_Back_Buffer(BOOL bUseExtraBuffer);
    // **** WARNING ****
    // This function is paired with Overlay_Unlock
    // If this function succeeds then Overlay_Unlock must be called
    // on ALL paths
	virtual BOOL Overlay_Lock(TDeinterlaceInfo* pInfo);
	virtual BOOL Overlay_Unlock();

	void Overlay_Copy_External(BYTE* lpExternalMemoryBuffer, int ExternalPitch, TDeinterlaceInfo* pInfo);
	void Overlay_Copy_Extra(TDeinterlaceInfo* pInfo);
    BOOL Overlay_Flip(DWORD FlipFlag, BOOL bUseExtraBuffer, BYTE* lpExternalMemoryBuffer, int ExternalPitch, TDeinterlaceInfo* pInfo);
	BOOL InitDD(HWND hWnd);
	void ExitDD(void);
    void WaitForVerticalBlank() {};

	RECT Overlay_GetCurrentDestRect();
	RECT Overlay_GetCurrentSrcRect();
	HWND GetHWnd();
	void Overlay_SetRGB(BOOL IsRGB);
	BOOL Overlay_GetRGB();


    void InitOtherSettings();

	OUTPUTTYPES Type();
};

CTreeSettingsGeneric* D3D9_GetTreeSettingsPage();
SETTING* D3D9_GetSetting(OTHER_SETTING Setting);
void D3D9_ReadSettingsFromIni();
void D3D9_WriteSettingsToIni(BOOL bOptimizeFileAccess);
