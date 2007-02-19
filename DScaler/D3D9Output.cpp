/////////////////////////////////////////////////////////////////////////////
// $Id: D3D9Output.cpp,v 1.9 2007-02-19 14:48:50 adcockj Exp $
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
// Revision 1.8  2007/02/19 10:13:45  adcockj
// Fixes for Critical thread and RECT issuesin D3D9 and overlay code
//
// Revision 1.7  2007/02/18 20:16:12  robmuller
// Applied coding standards.
//
// Revision 1.6  2007/02/18 18:40:08  robmuller
// Wait for vsync.
//
// Revision 1.5  2007/02/18 16:31:53  robmuller
// Added CVS log.
//
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "d3d9output.h"
#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "DScaler.h"
#include "OutThreads.h"
#include "VBI_VideoText.h"
#include "ErrorBox.h"
#include "Splash.h"
#include "DebugLog.h"
#include "AspectRatio.h"
#include "SettingsPerChannel.h"
#include "PaintingHDC.h"
#include <multimon.h>

// the instance of the d3d9 object
CD3D9Output D3D9OutputInstance;

extern CPaintingHDC OffscreenHDC;

// Our custom vertex type
struct CUSTOMVERTEX
{
	FLOAT x, y, z;
	FLOAT rhw;
	DWORD color;
	FLOAT tu, tv;   // The texture coordinates
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)



//-----------------------------------------------------------------------------
void CD3D9Output::SetCurrentMonitor(HWND hWnd)
{
//   hCurrentMon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);    
}

void CD3D9Output::CheckChangeMonitor(HWND hWnd) 
{
	return;

	HMONITOR hMon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);

	if (hCurrentMon == NULL)
	{
		return;
	}

	if (hMon != hCurrentMon)
	{
		hCurrentMon = hMon;
		Overlay_Stop(hWnd);
		Overlay_Destroy();		
		ExitDD();
		if (InitDD(hWnd))
		{
			Overlay_Start(hWnd);
		}
	}
}

BOOL CD3D9Output::CanDoOverlayColorControl()
{
	return false;
}
	
BOOL CD3D9Output::OverlayActive()
{
	return lpDDOverlay!=NULL;
}
	
void CD3D9Output::Overlay_Clean()
{
}

HWND CD3D9Output::GetHWnd()
{
	return m_hWnd;
}
	
BOOL CD3D9Output::Overlay_Update(LPRECT pSrcRect, LPRECT pDestRect, DWORD dwFlags)
{
	if( g_pD3D==NULL || pDevice==NULL || lpDDOverlay==NULL)
	{
		return false;
	}

	EnterCriticalSection(&hDDCritSect);
	if (pSrcRect == NULL)
    {
		// do nothing, we do not need to hide anything, because drawing is done in Overlay_Flip
	}
	else 
	{
        RECT oldSrc(srcrect);
		CopyRect(&srcrect, pSrcRect);
		// scale srcrect
		srcrect.left=(int)((float)BUFFERWIDTH/(float)DSCALER_MAX_WIDTH*srcrect.left);
		srcrect.right=(int)((float)BUFFERWIDTH/(float)DSCALER_MAX_WIDTH*srcrect.right);
		srcrect.top=(int)((float)BUFFERHEIGHT/(float)DSCALER_MAX_HEIGHT*srcrect.top);
		srcrect.bottom=(int)((float)BUFFERHEIGHT/(float)DSCALER_MAX_HEIGHT*srcrect.bottom);

        if(srcrect.right > oldSrc.right || srcrect.bottom > oldSrc.bottom)
        {
            OffscreenHDC.ReleaseD3DBuffer();
        }
		
		CopyRect(&destrect, pDestRect);		
		MapWindowPoints(HWND_DESKTOP, m_hWnd, (LPPOINT)&destrect, 2);			
	}

	VT_SetOverlayColour((COLORREF)0x00101020);

	LeaveCriticalSection(&hDDCritSect);

	return true;
}

void CD3D9Output::Overlay_ResetColorControls()
{
}
	
void CD3D9Output::Overlay_SetColorControls()
{
}
	
BOOL CD3D9Output::Overlay_Create()
{
	char msg[500];

	if (lpDDOverlay)
    {
        return FALSE;
    }

	EnterCriticalSection(&hDDCritSect);

	if(FlipResult==D3DERR_DEVICELOST || FlipResult==D3DERR_DRIVERINTERNALERROR)
	{
		LOG(1, "D3D Device lost .. trying reset");
		// Device lost .. try to reset
        OffscreenHDC.ReleaseD3DBuffer();
		if(pDevice->TestCooperativeLevel()==D3DERR_DEVICENOTRESET)
		{
			// ok to reset
			if(SUCCEEDED(pDevice->Reset(&d3dpp)))
			{                
				Overlay_Update(&srcrect, &destrect, SW_SHOW);
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}		
	}

	if(FAILED(pDevice->CreateOffscreenPlainSurface(DSCALER_MAX_WIDTH, DSCALER_MAX_HEIGHT, (bIsRGB ? D3DFMT_R5G6B5 : D3DFMT_YUY2), D3DPOOL_DEFAULT,
		&lpDDOverlay, NULL))) 
	{
		ErrorBox("CreateOffscreenPlainSurface failed");
		return false;
	}

	

	if(FAILED(pDevice->CreateTexture(BUFFERWIDTH, BUFFERHEIGHT, 1, D3DUSAGE_RENDERTARGET ,D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_lpOsdTexture, NULL))) 
	{
		ErrorBox("Creating OSD texture failed");
		return false;
	}
	
	if(FAILED(m_lpOsdTexture->GetSurfaceLevel(0, &lpDDOSD))) 
	{
		ErrorBox("Getting surface from OSD texture failed");
		return false;
	}

	sprintf(msg, "Using Direct3D output");
    AddSplashTextLine(msg);

	// try to create a memory buffer
    // that we can use if any output filters are switched
    // on.  This is required because reading and writing back to 
    // video memory is very slow
    lpExtraMemoryForFilters = (BYTE*)malloc(DSCALER_MAX_WIDTH * DSCALER_MAX_HEIGHT * 2 + 16);
    if(lpExtraMemoryForFilters == NULL)
    {
       // if we can't do a system memory buffer it's not the end of the
       // world it just means that any output filters will run very slow
       LOG(1, "Couldn't create additional buffer for output filters");
       lpExtraMemoryForFilters = NULL;
    }
	
	LeaveCriticalSection(&hDDCritSect);

	return true;
}
	
DWORD CD3D9Output::Overlay_ColorMatch(LPDIRECTDRAWSURFACE pdds, COLORREF rgb)
{
	return 1;
}
	
BOOL CD3D9Output::Overlay_Destroy()
{
    EnterCriticalSection(&hDDCritSect);

    // Now destroy the Extra Surface
    if(lpExtraMemoryForFilters != NULL)
    {
        free(lpExtraMemoryForFilters);
        lpExtraMemoryForFilters = NULL;
    }

	if(lpDDOSD!=NULL)
	{
		lpDDOSD->Release();
		lpDDOSD = NULL;
	}

	if(m_lpOsdTexture!=NULL) 
	{
		m_lpOsdTexture->Release();
		m_lpOsdTexture = NULL;
	}

	if(lpDDOverlay!=NULL)
	{
		lpDDOverlay->Release();
		lpDDOverlay=NULL;
	}	

	LeaveCriticalSection(&hDDCritSect);
	return true;
}
	
COLORREF CD3D9Output::Overlay_GetColor()
{
	return (COLORREF)0x00101020;
}
	
COLORREF CD3D9Output::Overlay_GetCorrectedColor(HDC hDC)
{
		return (COLORREF)0x00101020;
}
	
BOOL CD3D9Output::Overlay_Lock_Extra_Buffer(TDeinterlaceInfo* pInfo)
{
	if(lpExtraMemoryForFilters == NULL)
    {
        LOG(1, "Extra Buffer has been deleted");
        return FALSE;
    }


    pInfo->OverlayPitch = DSCALER_MAX_WIDTH * 2;
    // get back some memory aligned on 16 byte boundary for SSE
    pInfo->Overlay = lpExtraMemoryForFilters + (16 - ((DWORD)lpExtraMemoryForFilters % 16));
    return TRUE;
}
	
BOOL CD3D9Output::Overlay_Lock_Back_Buffer(TDeinterlaceInfo* pInfo, BOOL bUseExtraBuffer)
{
	HRESULT ddrval;
	
	if(bUseExtraBuffer && lpExtraMemoryForFilters != NULL)
    {
        return Overlay_Lock_Extra_Buffer(pInfo);
    }
	EnterCriticalSection(&hDDCritSect);

	D3DLOCKED_RECT r;
	
	ddrval=lpDDOverlay->LockRect(&r, NULL, D3DLOCK_DISCARD);
	if(FAILED(ddrval))
    {
        LOG(1, "Lock failed %8x", ddrval);
        LeaveCriticalSection(&hDDCritSect);
        return FALSE;
    }

	
	pInfo->OverlayPitch = r.Pitch;       // Set new pitch, may change
	pInfo->Overlay = (BYTE*)r.pBits;

	

	// stay in critical section
	
	return true;
}
	
BOOL CD3D9Output::Overlay_Lock(TDeinterlaceInfo* pInfo)
{
    // \todo this doesn't really work for now (GetFrontBufferData always fails) .. do we need this anyway?????
    if(pDevice==NULL)
    {
        return false;
    }

    LPDIRECT3DSURFACE9 temp;
    if(FAILED(pDevice->CreateOffscreenPlainSurface(
        GetSystemMetrics(SM_CXFULLSCREEN), 
        GetSystemMetrics(SM_CYFULLSCREEN),
        D3DFMT_A8R8G8B8,
        D3DPOOL_SYSTEMMEM,
        &temp,
        NULL)))
    {
        LOG(1, "Could not create Surface in Overlay_Lock");
        return FALSE;
    }

    if(FAILED(pDevice->CreateOffscreenPlainSurface(
        GetSystemMetrics(SM_CXFULLSCREEN), 
        GetSystemMetrics(SM_CYFULLSCREEN),
        (bIsRGB ? D3DFMT_R5G6B5 : D3DFMT_YUY2),
        D3DPOOL_SYSTEMMEM,
        &lpDDFrontBuffer,
        NULL)))
    {
        LOG(1, "Could not create Surface in Overlay_Lock");
        return FALSE;
    }

    EnterCriticalSection(&hDDCritSect);
        
    HRESULT ddrval=pDevice->GetFrontBufferData(0, temp);
    if(FAILED(ddrval))
    {
        LOG(1, "GetFrontBufferData failed %8x", ddrval);
        temp->Release();
        lpDDFrontBuffer->Release();
        lpDDFrontBuffer=NULL;
        LeaveCriticalSection(&hDDCritSect);
        return FALSE;
    }
    
    ddrval=pDevice->StretchRect(temp, NULL, lpDDFrontBuffer, NULL, D3DTEXF_NONE);
    if(FAILED(ddrval))
    {
        LOG(1, "Could not copy front buffer data %8x", ddrval);
        temp->Release();
        lpDDFrontBuffer->Release();
        lpDDFrontBuffer=NULL;
        LeaveCriticalSection(&hDDCritSect);
        return FALSE;
    }

    temp->Release();
  
    D3DLOCKED_RECT lr;
    ddrval=lpDDFrontBuffer->LockRect(&lr, NULL, D3DLOCK_READONLY);
    if(FAILED(ddrval))
    {
        LOG(1, "Lock failed %8x", ddrval);
        LeaveCriticalSection(&hDDCritSect);        
        lpDDFrontBuffer->Release();
        lpDDFrontBuffer=NULL;
        return FALSE;
    }

    pInfo->OverlayPitch = lr.Pitch;         // Set new pitch, may change
    pInfo->Overlay = (BYTE*)lr.pBits;

	
    // stay in critical section
    return TRUE;	
}

BOOL CD3D9Output::Overlay_Unlock() 
{
    // always leave the critical section
    if(lpDDFrontBuffer==NULL)
    {
        LeaveCriticalSection(&hDDCritSect);
        return FALSE;
    }
    if(FAILED(lpDDFrontBuffer->UnlockRect()))
    {
        LOG(1, "Unlock failed");
        LeaveCriticalSection(&hDDCritSect);
        return FALSE;
    }
    lpDDFrontBuffer->Release();
    lpDDFrontBuffer=NULL;
    LeaveCriticalSection(&hDDCritSect);
    return TRUE;
}

	
BOOL CD3D9Output::Overlay_Unlock_Back_Buffer(BOOL bUseExtraBuffer) 
{
	if(bUseExtraBuffer && lpExtraMemoryForFilters != NULL)
    {
        return TRUE;
    }

    // make sure we always release the critical section
    BOOL RetVal = TRUE;
	if(FAILED( lpDDOverlay->UnlockRect()))
	{
		RetVal=false;
	} 
	else
	{
		// ok, now update the backbuffer
		LPDIRECT3DSURFACE9 back;
	
		if(SUCCEEDED(pDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO, &back))) 
        {
			// draw tv picture
			pDevice->StretchRect(lpDDOverlay, NULL, back, NULL, D3DTEXF_NONE);
			
			
			// drawing OSD
			// with DrawPrimitive because of alpha channel
			
			pDevice->BeginScene();

			LPDIRECT3DVERTEXBUFFER9 g_pVB;
			
			if(SUCCEEDED(pDevice->CreateVertexBuffer(6*sizeof(CUSTOMVERTEX), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pVB, NULL ))) 
			{
				VOID* pVertices;
				CUSTOMVERTEX vertexArray[] =
				{
					// x, y, z, rhw, color
					{ 0.0f,               0.0f,                0.0f, 1.0f, 0xffffffff, 0.0, 0.0, },
					{ (FLOAT)BUFFERWIDTH, 0.0f,                0.0f, 1.0f, 0xffffffff, 1.0, 0.0, },
					{ (FLOAT)BUFFERWIDTH, (FLOAT)BUFFERHEIGHT, 0.0f, 1.0f, 0xffffffff, 1.0, 1.0, },

					{ (FLOAT)BUFFERWIDTH, (FLOAT)BUFFERHEIGHT, 0.0f, 1.0f, 0xffffffff, 1.0, 1.0, },
					{ 0.0f,               (FLOAT)BUFFERHEIGHT, 0.0f, 1.0f, 0xffffffff, 0.0, 1.0, },
					{ 0.0f,               0.0f,                0.0f, 1.0f, 0xffffffff, 0.0, 0.0, },
				};


				if(SUCCEEDED(g_pVB->Lock(0, sizeof(vertexArray), (void**)&pVertices, 0 )))
				{
					memcpy(pVertices, vertexArray, sizeof(vertexArray));
					g_pVB->Unlock();
				}

				pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
 				pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
				pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

				pDevice->SetTexture(0, m_lpOsdTexture);
				//pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
				//pDevice->SetTextureStageState(0, D3DTSS_CONSTANT, 0xA0);
				//pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEMP);


				pDevice->SetStreamSource( 0, g_pVB, 0, sizeof(CUSTOMVERTEX) );
				pDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
				
				pDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, 2 );

				pDevice->SetTexture(0,NULL);

				g_pVB->Release();
			}
			pDevice->EndScene();
		
			back->Release();
		}
	}

	LeaveCriticalSection(&hDDCritSect);
	return RetVal;
}
	

RECT CD3D9Output::Overlay_GetCurrentDestRect() 
{
	return destrect;
}

RECT CD3D9Output::Overlay_GetCurrentSrcRect() 
{
	return srcrect;
}
	
void CD3D9Output::Overlay_Copy_External(BYTE* lpExternalMemoryBuffer, int ExternalPitch, TDeinterlaceInfo* pInfo)
{
	BYTE* FromPtr = lpExternalMemoryBuffer + (16 - ((DWORD)lpExternalMemoryBuffer % 16));
    long FromPitch = ExternalPitch;
    // careful as we need to ensure this is always unlocked
    if(Overlay_Lock_Back_Buffer(pInfo, FALSE))
    {
        BYTE* ToPtr = pInfo->Overlay;

        for(int i(0) ; i < pInfo->FrameHeight; ++i)
        {
            pInfo->pMemcpy(ToPtr, FromPtr, pInfo->LineLength);
            FromPtr += FromPitch;
            ToPtr += pInfo->OverlayPitch;
        }
        _asm
        {
            emms
        }

        Overlay_Unlock_Back_Buffer(FALSE);
    }
}

void CD3D9Output::Overlay_Copy_Extra(TDeinterlaceInfo* pInfo) 
{
	Overlay_Lock_Extra_Buffer(pInfo);
    BYTE* FromPtr = pInfo->Overlay;
    long FromPitch = pInfo->OverlayPitch;
    // careful as we need to ensure this is always unlocked
    if(Overlay_Lock_Back_Buffer(pInfo, FALSE))
    {
        BYTE* ToPtr = pInfo->Overlay;

        for(int i(0) ; i < pInfo->FrameHeight; ++i)
        {
            pInfo->pMemcpy(ToPtr, FromPtr, pInfo->LineLength);
            FromPtr += FromPitch;
            ToPtr += pInfo->OverlayPitch;
        }
        _asm
        {
            emms
        }

        Overlay_Unlock_Back_Buffer(FALSE);
    }
}

	
BOOL CD3D9Output::Overlay_Flip(DWORD FlipFlag, BOOL bUseExtraBuffer, BYTE* lpExternalMemoryBuffer, int ExternalPitch, TDeinterlaceInfo* pInfo)
{
	if(lpDDOverlay == NULL)
    {
        LOG(1, "D3DDevice has been deleted - trying to reset");
		return FALSE;		
    }

    if(bUseExtraBuffer && lpExternalMemoryBuffer != NULL)
    {
        Overlay_Copy_External(lpExternalMemoryBuffer, ExternalPitch, pInfo);
    }

    // if we have been using the extra surface then we need to copy
    // the picture onto the overlay
    else if(bUseExtraBuffer && lpExtraMemoryForFilters != NULL)
    {
        Overlay_Copy_Extra(pInfo);
    }
	
    EnterCriticalSection(&hDDCritSect);

    BOOL RetVal = TRUE;
	FlipResult = pDevice->Present(&srcrect, &destrect, m_hWnd, NULL);
	
    
    if(FlipResult==D3DERR_DEVICELOST || FlipResult==D3DERR_DRIVERINTERNALERROR)
	{
		// Device lost .. 
		LeaveCriticalSection(&hDDCritSect);
		return FALSE;		
	}
	else
	{
		m_FramesPresented++;
	}
    
    LeaveCriticalSection(&hDDCritSect);
	
    return RetVal;	
}

BOOL CD3D9Output::InitDD(HWND hWnd) 
{
	if( NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION))) 
    {
		ErrorBox("Direct3DCreate9 failed");
        return false;
	}

	m_hWnd=hWnd;
    

	// find display adapter
	HMONITOR hmon=MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
	D3DADAPTER_IDENTIFIER9 adapter;	
	int adapter_id=D3DADAPTER_DEFAULT;
	for(int i=0;i<g_pD3D->GetAdapterCount();i++)
	{
		if(g_pD3D->GetAdapterMonitor(i)==hmon)
		{
			// this is ours we want to use			
			if(SUCCEEDED(g_pD3D->GetAdapterIdentifier(i, 0, &adapter)))
			{
				adapter_id=i;
				break;
			}
		}
	}
	

	ZeroMemory( &d3dpp, sizeof(d3dpp) );
	d3dpp.Windowed   = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.BackBufferWidth=BUFFERWIDTH;
	d3dpp.BackBufferHeight=BUFFERHEIGHT;
	d3dpp.Flags=D3DPRESENTFLAG_VIDEO | D3DPRESENTFLAG_DEVICECLIP;
	

	if( FAILED( g_pD3D->CreateDevice( adapter_id, D3DDEVTYPE_HAL, hWnd,
                                  D3DCREATE_SOFTWARE_VERTEXPROCESSING|D3DCREATE_MULTITHREADED,
								  &d3dpp, &pDevice ) ) )
	{
		ErrorBox("D3D CreateDevice failed");
		return false;
	}


	return true;
}

void CD3D9Output::Overlay_SetRGB(BOOL IsRGB)
{
    bIsRGB = IsRGB;
}

BOOL CD3D9Output::Overlay_GetRGB()
{
    return bIsRGB;
}

void CD3D9Output::ExitDD(void) 
{
    EnterCriticalSection(&hDDCritSect);
	if(pDevice!=NULL)
	{
		pDevice->Release();
		pDevice=NULL;
	}

	if(g_pD3D!=NULL)
	{
		g_pD3D->Release();	
		g_pD3D=NULL;
	}

    OffscreenHDC.ReleaseD3DBuffer();
    
    LeaveCriticalSection(&hDDCritSect);
}

	
IOutput::OUTPUTTYPES CD3D9Output::Type()
{
	return OUT_D3D;
}

void CD3D9Output::InitOtherSettings()
{    
}


CD3D9Output::CD3D9Output(void)
{
	g_pD3D=NULL;
	pDevice=NULL;
	lpDDOverlay=NULL;
	SrcSizeAlign = 1;
	DestSizeAlign = 1;
	memset(&srcrect, 0, sizeof(RECT));
	memset(&destrect, 0, sizeof(RECT));
	m_FramesPresented=0;
	FlipResult=S_OK;
	hCurrentMon =NULL;
    lpDDFrontBuffer=NULL;
	bIsRGB = FALSE;

	BUFFERWIDTH=GetSystemMetrics(SM_CXFULLSCREEN);
	BUFFERHEIGHT=GetSystemMetrics(SM_CYFULLSCREEN);

    InitializeCriticalSection(&hDDCritSect);
}

CD3D9Output::~CD3D9Output(void)
{
	Overlay_Destroy();
    DeleteCriticalSection(&hDDCritSect);
}

SETTING* D3D9_GetSetting(OTHER_SETTING Setting)
{
    if(Setting > -1 && Setting < OTHER_SETTING_LASTONE)
    {
        return &(D3D9OutputInstance.GetOtherSettings()[Setting]);
    }
    else
    {
        return NULL;
    }
}

CSettingsHolderStandAlone D3D9SettingsHolder;

void D3D9_ReadSettingsFromIni()
{
    CSettingGroup *pD3D9Group = D3D9SettingsHolder.GetSettingsGroup("D3D9", SETTING_BY_CHANNEL | SETTING_BY_FORMAT | SETTING_BY_INPUT, FALSE);

#ifdef _DEBUG
    if (D3D9_SETTING_LASTONE != D3D9SettingsHolder.GetNumSettings())
    {
        LOGD("Number of settings in D3D9 source is not equal to the number of settings in DS_Control.h");
        LOGD("DS_Control.h or D3D9Output.cpp are probably not in sync with each other.");
    }
#endif

    D3D9SettingsHolder.DisableOnChange();
    D3D9SettingsHolder.ReadFromIni();
    D3D9SettingsHolder.EnableOnChange();
}

void D3D9_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    D3D9SettingsHolder.WriteToIni(bOptimizeFileAccess);
}

CTreeSettingsGeneric* D3D9_GetTreeSettingsPage()
{	
	return new CTreeSettingsGeneric("Direct3D Settings", D3D9OutputInstance.GetOtherSettings(), 0);
}
