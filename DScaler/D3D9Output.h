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
	LPRECT srcrect, destrect;	
	HWND m_hWnd;
	LPDIRECT3DTEXTURE9 m_lpOsdTexture;
	D3DPRESENT_PARAMETERS d3dpp; 
	__int64 m_FramesPresented;
	int BUFFERWIDTH, BUFFERHEIGHT;
	HRESULT FlipResult;
	HMONITOR hCurrentMon;
public:
	LPDIRECT3DDEVICE9 pDevice;
	LPDIRECT3DSURFACE9 lpDDOSD;

	CD3D9Output(void);
	~CD3D9Output(void);

	void LoadDynamicFunctions();
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

	RECT Overlay_GetCurrentDestRect();
	RECT Overlay_GetCurrentSrcRect();
	HWND GetHWnd();


    void InitOtherSettings();

	CTreeSettingsGeneric* Other_GetTreeSettingsPage();

	OUTPUTTYPES Type();
};
