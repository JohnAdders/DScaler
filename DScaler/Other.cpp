/////////////////////////////////////////////////////////////////////////////
// $Id: Other.cpp,v 1.20 2001-09-21 20:47:12 laurentg Exp $
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
// This was in turn based on  Linux code by:
//
// BT-Parts
//
// Copyright (C) 1996,97,98 Ralph  Metzler (rjkm@thp.uni-koeln.de)
//                         & Marcus Metzler (mocm@thp.uni-koeln.de)
// msp34XX
//
// Copyright (C) 1997,1998 Gerd Knorr <kraxel@goldbach.in-berlin.de>
//
// Copyright (C) 1996,97,98 Ralph  Metzler (rjkm@thp.uni-koeln.de)
//                         & Marcus Metzler (mocm@thp.uni-koeln.de)
// msp34XX
//
// Copyright (C) 1997,1998 Gerd Knorr <kraxel@goldbach.in-berlin.de>
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           Original Release
//                                     Translated most code from German
//                                     Combined Header files
//                                     Cut out all decoding
//                                     Cut out digital hardware stuff
//
// 06 Jan 2001   John Adcock           Addded extra Info on overlay error boxed
// 
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.19  2001/09/19 10:05:50  adcockj
// Updated feature help
//
// Revision 1.18  2001/09/09 17:41:08  adcockj
// Fixed bug in sharpness code
//
// Revision 1.17  2001/09/05 21:05:29  adcockj
// Bug Fixes for new overlay code
//
// Revision 1.16  2001/09/05 15:07:48  adcockj
// Wrapped overlay calls with critical section
// Updated Loging
//
// Revision 1.15  2001/09/02 14:17:51  adcockj
// Improved teletext code
//
// Revision 1.14  2001/08/13 18:54:55  adcockj
// Tidied up surface blanking code
//
// Revision 1.13  2001/08/11 11:17:57  adcockj
// Fixed problems when using zero backbuffers
//
// Revision 1.12  2001/08/02 16:43:05  adcockj
// Added Debug level to LOG function
//
// Revision 1.11  2001/07/28 13:24:40  adcockj
// Added UI for Overlay Controls and fixed issues with SettingsDlg
//
// Revision 1.10  2001/07/27 12:30:09  adcockj
// Added Overlay Color controls (Thanks to Muljadi Budiman)
//
// Revision 1.9  2001/07/16 18:07:50  adcockj
// Added Optimisation parameter to ini file saving
//
// Revision 1.8  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.7  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Other.h"
#include "DScaler.h"
#include "BT848.h"
#include "OutThreads.h"
#include "VBI_VideoText.h"
#include "ErrorBox.h"
#include "Splash.h"
#include "DebugLog.h"
#include "SettingsDlg.h"
#include "AspectRatio.h"

// cope with older DX header files
#if !defined(DDFLIP_DONOTWAIT)
    #define DDFLIP_DONOTWAIT 0
#endif

LPDIRECTDRAW lpDD = NULL;

// this critical section is used to make sure that we don't mess about with the
// overlay in one thread while the other thread is doing something with it
// it is held while the primary surface DC is held
// and while the overlay is locked
CRITICAL_SECTION hDDCritSect;

LPDIRECTDRAWSURFACE     lpDDSurface = NULL;
// OverLay
LPDIRECTDRAWSURFACE     lpDDOverlay = NULL;
LPDIRECTDRAWSURFACE     lpDDOverlayBack = NULL;
BOOL bCanColorKey=FALSE;
DWORD DestSizeAlign;
DWORD SrcSizeAlign;
COLORREF OverlayColor = RGB(32, 16, 16);
DWORD PhysicalOverlayColor = RGB(32, 16, 16);
long BackBuffers = -1;     // Make new user parm, TRB 10/28/00
BOOL bCanDoBob = FALSE;
BOOL bCanDoColorKey = FALSE;
DDCOLORCONTROL OriginalColorControls;
LPDIRECTDRAWCOLORCONTROL pDDColorControl = NULL;
BOOL bUseOverlayControls = FALSE;

long OverlayBrightness = 75;
long OverlayContrast = 10000;
long OverlayHue = 0;
long OverlaySaturation = 10000;
long OverlayGamma = 1;
long OverlaySharpness = 5;

SETTING OtherSettings[];

//-----------------------------------------------------------------------------
// Tells whether or not video overlay is active
BOOL OverlayActive()
{
    return (lpDDOverlay != NULL);
}

//-----------------------------------------------------------------------------
// Blank out video overlay
void Overlay_Clean()
{
    unsigned int nPixel;
    unsigned int nLine;
    HRESULT ddrval;
    DDSURFACEDESC SurfaceDesc;
    unsigned short* pPixel;

    EnterCriticalSection(&hDDCritSect);
    if (lpDDOverlay != NULL)
    {
        memset(&SurfaceDesc, 0x00, sizeof(SurfaceDesc));
        SurfaceDesc.dwSize = sizeof(SurfaceDesc);

        ddrval = lpDDOverlay->Lock(NULL, &SurfaceDesc, DDLOCK_WAIT, NULL);

        for (nLine = 0; nLine < SurfaceDesc.dwHeight; nLine++)
        {
            pPixel = (unsigned short*)((BYTE*)SurfaceDesc.lpSurface + nLine * SurfaceDesc.lPitch);
            for (nPixel = 0; nPixel < SurfaceDesc.dwWidth; ++nPixel)
            {
                *pPixel = 0x8000;
                pPixel++;
            }
        }
        ddrval = lpDDOverlay->Unlock(SurfaceDesc.lpSurface);
    }
    if (lpDDOverlayBack != NULL)
    {
        memset(&SurfaceDesc, 0x00, sizeof(SurfaceDesc));
        SurfaceDesc.dwSize = sizeof(SurfaceDesc);

        ddrval = lpDDOverlayBack->Lock(NULL, &SurfaceDesc, DDLOCK_WAIT, NULL);

        for (nLine = 0; nLine < SurfaceDesc.dwHeight; nLine++)
        {
            pPixel = (unsigned short*)((BYTE*)SurfaceDesc.lpSurface + nLine * SurfaceDesc.lPitch);
            for (nPixel = 0; nPixel < SurfaceDesc.dwWidth; ++nPixel)
            {
                *pPixel = 0x8000;
                pPixel++;
            }
        }
        ddrval = lpDDOverlayBack->Unlock(SurfaceDesc.lpSurface);
    }
    LeaveCriticalSection(&hDDCritSect);
}

//-----------------------------------------------------------------------------
// Update video overlay with new rectangle
BOOL Overlay_Update(LPRECT pSrcRect, LPRECT pDestRect, DWORD dwFlags)
{
    HRESULT     ddrval;
    DDOVERLAYFX DDOverlayFX;

    if ((lpDD == NULL) || (lpDDSurface == NULL) || (lpDDOverlay == NULL))
    {
        return FALSE;
    }

    EnterCriticalSection(&hDDCritSect);

    memset(&DDOverlayFX, 0x00, sizeof(DDOverlayFX));
    DDOverlayFX.dwSize = sizeof(DDOverlayFX);

    if (pSrcRect == NULL)
    {
        ////////////////////////////////
        // we are trying to hide overlay
        ////////////////////////////////
        ddrval = lpDDOverlay->UpdateOverlay(NULL, lpDDSurface, NULL, dwFlags, &DDOverlayFX);
        // if another device has requested exclusive access then we
        // can get the no hardware error, just wait a bit and try again
        while(ddrval == DDERR_NOOVERLAYHW)
        {
            Sleep(100);
            ddrval = lpDDOverlay->UpdateOverlay(NULL, lpDDSurface, NULL, dwFlags, &DDOverlayFX);
        }
        // just return if we get this here
        // all DDERR_SURFACELOST will be handled by
        // the main processing loop
        if(ddrval == DDERR_SURFACELOST)
        {
            LeaveCriticalSection(&hDDCritSect);
            return FALSE;
        }
        if (FAILED(ddrval))
        {
            // 2001-01-06 John Adcock
            // Now show return code
            char szErrorMsg[200];
            sprintf(szErrorMsg, "Error %x calling UpdateOverlay (Hide)", ddrval);
            ErrorBox(szErrorMsg);
            LeaveCriticalSection(&hDDCritSect);
            return FALSE;
        }
    }
    else
    {
        ////////////////////////////////
        // we are trying to show overlay
        ////////////////////////////////
        if(bCanDoColorKey)
        {
            dwFlags |= DDOVER_KEYDESTOVERRIDE;

            // if we are doing teletext the override the
            // background to pink so that we can do transparent
            // with the PAL RGB colours
            if(VTState == VT_OFF)
            {
                PhysicalOverlayColor = Overlay_ColorMatch(lpDDSurface, OverlayColor);
                if (PhysicalOverlayColor == 0)      // sometimes we glitch and can't get the Value
                {
                    LOG(3, "Physical overlay color is zero!  Retrying.");
                    PhysicalOverlayColor = Overlay_ColorMatch(lpDDSurface, OverlayColor);
                    LeaveCriticalSection(&hDDCritSect);
                }
                LOG(3, "Physical overlay color is %x", PhysicalOverlayColor);
            }
            else
            {
                PhysicalOverlayColor = Overlay_ColorMatch(lpDDSurface, RGB(255, 0, 255));
                if (PhysicalOverlayColor == 0)      // sometimes we glitch and can't get the Value
                {
                    LOG(1, "Physical overlay color is zero!  Retrying.");
                    PhysicalOverlayColor = Overlay_ColorMatch(lpDDSurface, RGB(255, 0, 255));
                }
                LOG(3, "Physical overlay color is %x", PhysicalOverlayColor);
            }

            DDOverlayFX.dckDestColorkey.dwColorSpaceHighValue = PhysicalOverlayColor;
            DDOverlayFX.dckDestColorkey.dwColorSpaceLowValue = PhysicalOverlayColor;
        }
        if(bCanDoBob)
        {
            dwFlags |= DDOVER_BOB;
        }

        ddrval = lpDDOverlay->UpdateOverlay(pSrcRect, lpDDSurface, pDestRect, dwFlags, &DDOverlayFX);
        // if another device has requested exclusive access then we
        // can get the no hardware error, just wait a bit and try again
        while(ddrval == DDERR_NOOVERLAYHW)
        {
            Sleep(100);
            ddrval = lpDDOverlay->UpdateOverlay(pSrcRect, lpDDSurface, pDestRect, dwFlags, &DDOverlayFX);
        }
        // just return if we get this here
        // all DDERR_SURFACELOST will be handled by
        // the main processing loop
        if(ddrval == DDERR_SURFACELOST)
        {
            LeaveCriticalSection(&hDDCritSect);
            return FALSE;
        }
        // we get unsupported error here for mpact2 cards
        // so cope with this by not trying to update
        // the color key Value and just hoping it works
        // with the existing one (black used to work)
        if(ddrval == DDERR_UNSUPPORTED)
        {
            DDCOLORKEY ColorKey;

            LOG(1, "Got unsupported error from Overlay Update");
            ddrval = lpDDOverlay->GetColorKey(DDCKEY_DESTOVERLAY, &ColorKey);
            if(SUCCEEDED(ddrval))
            {
                OverlayColor = ColorKey.dwColorSpaceHighValue;
                LOG(1, "Reset overlay color to %x", OverlayColor);
            }
            dwFlags &= ~DDOVER_KEYDESTOVERRIDE;
            memset(&DDOverlayFX, 0x00, sizeof(DDOverlayFX));
            DDOverlayFX.dwSize = sizeof(DDOverlayFX);
            ddrval = lpDDOverlay->UpdateOverlay(pSrcRect, lpDDSurface, pDestRect, dwFlags, &DDOverlayFX);
        }
        if (FAILED(ddrval))
        {
            if ((pDestRect->top < pDestRect->bottom) && (pDestRect->left < pDestRect->right))
            {
                // 2000-10-29 Added by Mark Rejhon
                // Display error message only if rectangle dimensions are positive.
                // Negative rectangle dimensions are frequently caused by the user
                // resizing the window smaller than the video size.
                // 2001-01-06 John Adcock
                // Now show return code
                char szErrorMsg[200];
                sprintf(szErrorMsg, "Error %x in UpdateOverlay", ddrval);
                ErrorBox(szErrorMsg);
            }
            LeaveCriticalSection(&hDDCritSect);
            return FALSE;
        }
        
        // update the controls
        if(bUseOverlayControls)
        {
           Overlay_SetColorControls();
        }
    }
    LeaveCriticalSection(&hDDCritSect);
    return TRUE;
}

void Overlay_ResetColorControls()
{
    if (pDDColorControl == NULL)
    {
        return;
    }

    if(OriginalColorControls.dwFlags != 0)
    {
        HRESULT ddrval = pDDColorControl->SetColorControls(&OriginalColorControls);
        if (FAILED(ddrval))
        {
            char szErrorMsg[200];
            sprintf(szErrorMsg, "Error %x in SetColorControls()", ddrval);
            ErrorBox(szErrorMsg);
        }
    }
}

void Overlay_SetColorControls()
{
    DDCOLORCONTROL sColorControl;

    if (pDDColorControl == NULL)
    {
        return;
    }

    sColorControl.dwSize = sizeof(DDCOLORCONTROL);
    if (SUCCEEDED(pDDColorControl->GetColorControls(&sColorControl)))
    {
        if(sColorControl.dwFlags != 0)
        {
            // Needed on the nVidia
            if(OriginalColorControls.dwFlags == 0)
            {
                memcpy(&OriginalColorControls, &sColorControl, sizeof(DDCOLORCONTROL));
                if(!(sColorControl.dwFlags & DDCOLOR_BRIGHTNESS))
                {
                    OtherSettings[OVERLAYBRIGHTNESS].szDisplayName = NULL;
                }
                if(!(sColorControl.dwFlags & DDCOLOR_CONTRAST))
                {
                    OtherSettings[OVERLAYCONTRAST].szDisplayName = NULL;
                }
                if(!(sColorControl.dwFlags & DDCOLOR_HUE))
                {
                    OtherSettings[OVERLAYHUE].szDisplayName = NULL;
                }
                if(!(sColorControl.dwFlags & DDCOLOR_SATURATION))
                {
                    OtherSettings[OVERLAYSATURATION].szDisplayName = NULL;
                }
                if(!(sColorControl.dwFlags & DDCOLOR_SHARPNESS))
                {
                    OtherSettings[OVERLAYSHARPNESS].szDisplayName = NULL;
                }
                if(!(sColorControl.dwFlags & DDCOLOR_GAMMA))
                {
                    OtherSettings[OVERLAYGAMMA].szDisplayName = NULL;
                }

            }

            sColorControl.lBrightness = OverlayBrightness * 10;
            sColorControl.lContrast = OverlayContrast * 100;
            sColorControl.lHue = OverlayHue;
            sColorControl.lSaturation = OverlaySaturation * 100;
            sColorControl.lGamma = OverlayGamma;
            sColorControl.lSharpness = OverlaySharpness;

            if(bUseOverlayControls)
            {
                HRESULT ddrval = pDDColorControl->SetColorControls(&sColorControl);
                if (FAILED(ddrval))
                {
                    char szErrorMsg[200];
                    sprintf(szErrorMsg, "Error %x in SetColorControls()", ddrval);
                    ErrorBox(szErrorMsg);
                }
            }
        }
    }
    else
    {
        ErrorBox("Cannot get color control");
    }
}


//-----------------------------------------------------------------------------
// Create new video overlay
BOOL Overlay_Create()
{
    DDSURFACEDESC SurfaceDesc;
    DDPIXELFORMAT PixelFormat;
    HRESULT ddrval;
    DDSCAPS caps;
    int minBuffers, maxBuffers, numBuffers;
    char msg[500];

    if (lpDDOverlay)
    {
        return FALSE;
    }

    EnterCriticalSection(&hDDCritSect);

    // 2000-10-31 Moved by Mark Rejhon
    // Attempt to create primary surface before overlay, in this module,
    // because we may have destroyed the primary surface during a computer 
    // resolution change.
    memset(&SurfaceDesc, 0x00, sizeof(SurfaceDesc));
    SurfaceDesc.dwSize = sizeof(SurfaceDesc);
    SurfaceDesc.dwFlags = DDSD_CAPS;
    SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    ddrval = lpDD->CreateSurface(&SurfaceDesc, &lpDDSurface, NULL);
    if (FAILED(ddrval))
    {
        sprintf(msg, "Error creating primary surface: %x", ddrval);
        RealErrorBox(msg);
        LeaveCriticalSection(&hDDCritSect);
        return (FALSE);
    }

    ddrval = lpDDSurface->Lock(NULL, &SurfaceDesc, DDLOCK_WAIT, NULL);
    if (FAILED(ddrval))
    {
        sprintf(msg, "Error locking primary surface: %x", ddrval);
        RealErrorBox(msg);
        LeaveCriticalSection(&hDDCritSect);
        return (FALSE);
    }
    ddrval = lpDDSurface->Unlock(SurfaceDesc.lpSurface);
    if (FAILED(ddrval))
    {
        sprintf(msg, "Error unlocking primary surface: %x", ddrval);
        RealErrorBox(msg);
        LeaveCriticalSection(&hDDCritSect);
        return (FALSE);
    }

    memset(&PixelFormat, 0x00, sizeof(PixelFormat));
    PixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    PixelFormat.dwFlags = DDPF_FOURCC;
    PixelFormat.dwFourCC = MAKEFOURCC('Y', 'U', 'Y', '2');;
    PixelFormat.dwYUVBitCount = 16;

    memset(&SurfaceDesc, 0x00, sizeof(SurfaceDesc));
    SurfaceDesc.dwSize = sizeof(SurfaceDesc);
    SurfaceDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_BACKBUFFERCOUNT;
    SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_VIDEOMEMORY | DDSCAPS_FLIP | DDSCAPS_COMPLEX;

    // create a surface big enough to hold the largest resolution supported
    // this ensures that we can always have enough space to allow
    // Mode changes without recreating the overlay
    SurfaceDesc.dwWidth = DSCALER_MAX_WIDTH;
    SurfaceDesc.dwHeight = DSCALER_MAX_HEIGHT;
    SurfaceDesc.ddpfPixelFormat = PixelFormat;

    // If the user specified a particular back buffer Count, use it.  Otherwise
    // try triple buffering and drop down to double buffering, then single
    // buffering, if the card doesn't have enough memory.
    minBuffers = BackBuffers >= 0 ? BackBuffers : 0;
    maxBuffers = BackBuffers >= 0 ? BackBuffers : 2;

    for (numBuffers = maxBuffers; numBuffers >= minBuffers; numBuffers--)
    {
        SurfaceDesc.dwBackBufferCount = numBuffers;
        // if we get down to zero back buffers then the flags are different
        if(numBuffers == 0)
        {
            SurfaceDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
            SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_VIDEOMEMORY;
        }
        ddrval = lpDD->CreateSurface(&SurfaceDesc, &lpDDOverlay, NULL);

        if (SUCCEEDED(ddrval) || ddrval != DDERR_OUTOFVIDEOMEMORY)
            break;
    }

    if (FAILED(ddrval))
    {
        lpDDOverlay = NULL;
    }

    if (numBuffers < minBuffers)
    {
        // Not enough video memory.  Display different messages depending
        // on what we tried to allocate, since it may mean different things.

        if (minBuffers == 0)
        {
            // We tried single-buffering and it didn't work.
            RealErrorBox("Your video card doesn't have enough overlay\n"
                     "memory for a TV picture.  Try lowering your\n"
                     "color depth or screen resolution.  Rebooting\n"
                     "may help in some cases if memory is being\n"
                     "used by something else on your system.");
            LeaveCriticalSection(&hDDCritSect);
            return (FALSE);
        }
        else
        {
            // We didn't get down to single-buffering, meaning the user
            // specified a back buffer Count.
            sprintf(msg, "Your video card doesn't have enough overlay\n"
                         "memory for %d back buffers.  If you've used\n"
                         "that many back buffers before, you may need\n"
                         "to reboot.  Otherwise try lowering your screen\n"
                         "resolution or color depth, or try setting\n"
                         "BackBuffers=-1 in DScaler.ini to allow DScaler to\n"
                         "decide how many back buffers it can allocate.",
                    BackBuffers);
            RealErrorBox(msg);
            LeaveCriticalSection(&hDDCritSect);
            return (FALSE);
        }
    }

    if (FAILED(ddrval))
    {
        switch (ddrval) 
        {
        case DDERR_NOOVERLAYHW:
            RealErrorBox("Your video card doesn't appear to support\n"
                     "overlays, which DScaler requires.");
            LeaveCriticalSection(&hDDCritSect);
            return (FALSE);

            // Any other interesting error codes?
        }
        
        sprintf(msg, "Can't create overlay surface: %x", ddrval);
        RealErrorBox(msg);
        LeaveCriticalSection(&hDDCritSect);
        return FALSE;
    }

    sprintf(msg, "%d Back Buffers", numBuffers);
    AddSplashTextLine(msg);
    LOG(1, msg);

    ddrval = lpDDOverlay->Lock(NULL, &SurfaceDesc, DDLOCK_WAIT, NULL);
    // sometimes in win98 we get weird error messages here
    // so we need to loop until it's OK or we get a surface lost message
    while(ddrval == DDERR_NOOVERLAYHW || ddrval == DDERR_SURFACEBUSY)
    {
        Sleep(100);
        ddrval = lpDDOverlay->Lock(NULL, &SurfaceDesc, DDLOCK_WAIT, NULL);
    }
    if(ddrval == DDERR_SURFACELOST)
    {
        ddrval = lpDD->CreateSurface(&SurfaceDesc, &lpDDOverlay, NULL);
        if (FAILED(ddrval))
        {
            sprintf(msg, "Lost overlay surface and can't recreate it: %x", ddrval);
            RealErrorBox(msg);
            lpDDOverlay = NULL;
            LeaveCriticalSection(&hDDCritSect);
            return FALSE;
        }
        ddrval = lpDDOverlay->Lock(NULL, &SurfaceDesc, DDLOCK_WAIT, NULL);
    }
    if (FAILED(ddrval))
    {
        char szErrorMsg[200];
        sprintf(szErrorMsg, "Error %x in Lock Surface", ddrval);
        RealErrorBox(szErrorMsg);
        LeaveCriticalSection(&hDDCritSect);
        return (FALSE);
    }

    ddrval = lpDDOverlay->Unlock(SurfaceDesc.lpSurface);
    if (FAILED(ddrval))
    {
        RealErrorBox("Can't Unlock Surface");
        LeaveCriticalSection(&hDDCritSect);
        return (FALSE);
    }

    if(numBuffers != 0)
    {
        memset(&caps, 0, sizeof(caps));
        caps.dwCaps = DDSCAPS_BACKBUFFER;
        ddrval = lpDDOverlay->GetAttachedSurface(&caps, &lpDDOverlayBack);
        if (FAILED(ddrval))
        {
            RealErrorBox("Can't create Overlay Back Surface");
            lpDDOverlayBack = NULL;
            LeaveCriticalSection(&hDDCritSect);
            return (FALSE);
        }
        else
        {
            ddrval = lpDDOverlayBack->Lock(NULL, &SurfaceDesc, DDLOCK_WAIT, NULL);
            if (FAILED(ddrval))
            {
                RealErrorBox("Can't Lock Back Surface");
                LeaveCriticalSection(&hDDCritSect);
                return (FALSE);
            }
            ddrval = DDERR_WASSTILLDRAWING;

            ddrval = lpDDOverlayBack->Unlock(SurfaceDesc.lpSurface);
            if (FAILED(ddrval))
            {
                RealErrorBox("Can't Unlock Back Surface");
                LeaveCriticalSection(&hDDCritSect);
                return (FALSE);
            }
        }
    }
    else
    {
        lpDDOverlayBack = lpDDOverlay;
        lpDDOverlayBack->AddRef();
    }

    // New Code - Getting the DD color control
    ddrval = lpDDOverlay->QueryInterface(IID_IDirectDrawColorControl, (void **) &pDDColorControl);
    if(SUCCEEDED(ddrval))
    {
        ddrval = pDDColorControl->GetColorControls(&OriginalColorControls);
        if(SUCCEEDED(ddrval))
        {
            if(bUseOverlayControls)
            {
               Overlay_SetColorControls();
            }
        }
        else
        {
            pDDColorControl->Release();
        }
    }
    else
    {
       pDDColorControl = NULL;
    }

    LeaveCriticalSection(&hDDCritSect);

    // overlay clean is wrapped in a critcal section already
    Overlay_Clean();
    
    return (TRUE);
}

//-----------------------------------------------------------------------------
// Name: DDColorMatch()
// Desc: Convert a RGB color to a pysical color.
//       We do this by leting GDI SetPixel() do the color matching
//       then we lock the memory and see what it got mapped to.
//-----------------------------------------------------------------------------
DWORD Overlay_ColorMatch(LPDIRECTDRAWSURFACE pdds, COLORREF rgb)
{
    COLORREF rgbT;
    HDC hdc;
    DWORD dw = CLR_INVALID;
    DDSURFACEDESC SurfaceDesc;
    HRESULT hres;

    //
    //  Use GDI SetPixel to color match for us
    //
    hres = pdds->GetDC(&hdc);
    if (SUCCEEDED(hres))
    {
        rgbT = GetPixel(hdc, 0, 0);     // Save current pixel Value
        SetPixel(hdc, 0, 0, rgb);       // Set our Value
        pdds->ReleaseDC(hdc);
    }
    //
    // Now lock the surface so we can read back the converted color
    //
    SurfaceDesc.dwSize = sizeof(SurfaceDesc);
    hres = pdds->Lock(NULL, &SurfaceDesc, DDLOCK_WAIT, NULL);
    if (SUCCEEDED(hres))
    {
        dw = *(DWORD*)SurfaceDesc.lpSurface;                 // Get DWORD
        if (SurfaceDesc.ddpfPixelFormat.dwRGBBitCount < 32)
        {
            dw &= (1 << SurfaceDesc.ddpfPixelFormat.dwRGBBitCount) - 1;  // Mask it to bpp
        }
        pdds->Unlock(NULL);
    }
    //
    //  Now put the color that was there back.
    //
    hres = pdds->GetDC(&hdc);
    if (SUCCEEDED(hres))
    {
        SetPixel(hdc, 0, 0, rgbT);
        pdds->ReleaseDC(hdc);
    }
    return dw;
}

//-----------------------------------------------------------------------------
// Deinitialize video overlay
//
// 2000-10-31 Added by Mark Rejhon
// Provide a way to destroy the video overlay and primary, which should
// be done right before a computer resolution change.
// 
BOOL Overlay_Destroy()
{
    EnterCriticalSection(&hDDCritSect);

    // reset overlay control to previous settings so that
    // DVD's will look OK
    if(pDDColorControl != NULL)
    {
        Overlay_ResetColorControls();
        pDDColorControl->Release();
        pDDColorControl = NULL;

    }
    // Now destroy the Back Overlay
    if (lpDDOverlayBack != NULL)
    {
        lpDDOverlayBack->Release();
        lpDDOverlayBack = NULL;
    }

    // Now destroy the main Overlay
    if (lpDDOverlay != NULL)
    {
        // Destroy the video overlays
        lpDDOverlay->Release();
        lpDDOverlay = NULL;
    }

    // Now destroy the primary surface
    if (lpDDSurface != NULL) 
    {
        lpDDSurface->Release();
        lpDDSurface = NULL;
    }
    LeaveCriticalSection(&hDDCritSect);
    return TRUE;
}

COLORREF Overlay_GetColor()
{
    return OverlayColor;
}


static HRESULT FlipResult = 0;             // Need to try again for flip?

//
// Add a function to Lock the overlay surface and update some Info from it.
// We always lock and write to the back buffer.
// Flipping takes care of the proper buffer addresses.
// Some of this Info can change each time.  
// We also check to see if we still need to Flip because the
// non-waiting last flip failed.  If so, try it one more time,
// then give up.  Tom Barry 10/26/00
//
BOOL Overlay_Lock(DEINTERLACE_INFO* pInfo)
{
    if(lpDDOverlay == NULL || lpDDOverlayBack == NULL)
    {
        LOG(1, "Overlay has been deleted");
        return FALSE;
    }

    EnterCriticalSection(&hDDCritSect);

    HRESULT         ddrval;
    static DWORD    dwFlags = DDLOCK_WAIT | DDLOCK_NOSYSLOCK;
    DDSURFACEDESC   SurfaceDesc;

    if (FlipResult == DDERR_WASSTILLDRAWING)             // prev flip was busy?
    {
        ddrval = lpDDOverlay->Flip(NULL, DDFLIP_DONOTWAIT);  
        if(ddrval == DDERR_SURFACELOST)
        {
            LOG(1, "Flip before lock failed");
            LeaveCriticalSection(&hDDCritSect);
            return FALSE;
        }
        FlipResult = 0;                 // but no time to try any more
    }

    memset(&SurfaceDesc, 0x00, sizeof(SurfaceDesc));
    SurfaceDesc.dwSize = sizeof(SurfaceDesc);
    ddrval = lpDDOverlayBack->Lock(NULL, &SurfaceDesc, dwFlags, NULL);

    // fix suggested by christoph for NT 4.0 sp6
    if(ddrval == E_INVALIDARG && (dwFlags & DDLOCK_NOSYSLOCK))
    {
        //remove flag
        ddrval = lpDDOverlayBack->Lock(NULL, &SurfaceDesc, DDLOCK_WAIT, NULL);
        if( SUCCEEDED(ddrval) )
        {
            //remember for next time
            dwFlags = DDLOCK_WAIT;
            LOG(1, "Switched to not using NOSYSLOCK");
        }
    }

    if(FAILED(ddrval))
    {
        LOG(1, "Lock failed %8x", ddrval);
        LeaveCriticalSection(&hDDCritSect);
        return FALSE;
    }

    pInfo->OverlayPitch = SurfaceDesc.lPitch;         // Set new pitch, may change
    pInfo->Overlay = (BYTE*)SurfaceDesc.lpSurface;
    // stay in critical section
    return TRUE;
}

BOOL Overlay_Unlock()
{
    if(lpDDOverlayBack == NULL)
    {
        LOG(1, "Overlay has been deleted");
        LeaveCriticalSection(&hDDCritSect);
        return FALSE;
    }

    // we are already in critical section
    HRESULT ddrval = lpDDOverlayBack->Unlock(NULL);
    BOOL RetVal = TRUE;
    if(FAILED(ddrval))
    {
        if(ddrval != DDERR_SURFACELOST)
        {
            LOG(1, "Unexpected failure in Unlock %8x", ddrval);
        }
        RetVal = FALSE;
    }
    LeaveCriticalSection(&hDDCritSect);
    return RetVal;
}

BOOL Overlay_Flip(DWORD FlipFlag)
{
    if(lpDDOverlay == NULL)
    {
        LOG(1, "Overlay has been deleted");
        return FALSE;
    }

    EnterCriticalSection(&hDDCritSect);
    BOOL RetVal = TRUE;
    FlipResult = lpDDOverlay->Flip(NULL, FlipFlag); 
    if(FAILED(FlipResult))
    {
        if(FlipResult != DDERR_WASSTILLDRAWING && 
            FlipResult != DDERR_SURFACELOST &&
            FlipResult != DDERR_NOTFLIPPABLE)
        {
            LOG(1, "Surface Flip failed %8x", FlipResult);
        }
        // return OK if we get DDERR_WASSTILLDRAWING
        // we'll do the flip next time
        // also if we get DDERR_NOTFLIPPABLE
        // as this probably means the overlay has been hiden
        RetVal = (FlipResult == DDERR_WASSTILLDRAWING) || (FlipResult == DDERR_NOTFLIPPABLE);
    }
    LeaveCriticalSection(&hDDCritSect);
    return RetVal;
}

HDC Overlay_GetDC()
{
    if(lpDDSurface == NULL)
    {
        LOG(1, "Surface has been deleted");
        return NULL;
    }

    HDC hDC;
    EnterCriticalSection(&hDDCritSect);
    HRESULT ddrval = lpDDSurface->GetDC(&hDC);
    if(FAILED(ddrval))
    {
        LOG(1, "Surface GetDC failed %8x", ddrval);        
        hDC = NULL;
        LeaveCriticalSection(&hDDCritSect);
    }
    // stay in critical section if successful
    return hDC;
}

void Overlay_ReleaseDC(HDC hDC)
{
    if(hDC == NULL || lpDDSurface == NULL)
    {
        LOG(1, "Overlay_ReleaseDC called with invalid params");
        return;
    }
    // we are already in critical section
    HRESULT ddrval = lpDDSurface->ReleaseDC(hDC);
    if(FAILED(ddrval))
    {
        LOG(1, "Surface ReleaseDC failed %8x", ddrval);        
    }
    LeaveCriticalSection(&hDDCritSect);
}

//-----------------------------------------------------------------------------
// Initialize DirectDraw
BOOL InitDD(HWND hWnd)
{
    HRESULT ddrval;
    DDCAPS DriverCaps;

    InitializeCriticalSection(&hDDCritSect);

    if (FAILED(DirectDrawCreate(NULL, &lpDD, NULL)))
    {
        ErrorBox("DirectDrawCreate failed");
        return (FALSE);
    }

    // can we use Overlay ??
    memset(&DriverCaps, 0x00, sizeof(DriverCaps));
    DriverCaps.dwSize = sizeof(DriverCaps);
    ddrval = lpDD->GetCaps(&DriverCaps, NULL);

    if (SUCCEEDED(ddrval))
    {
        if (DriverCaps.dwCaps & DDCAPS_OVERLAY)
        {
            if (!(DriverCaps.dwCaps & DDCAPS_OVERLAYSTRETCH))
            {
                ErrorBox("Can't Strech Overlay");
                return FALSE;
            }

            bCanDoColorKey = ((DriverCaps.dwCKeyCaps & DDCKEYCAPS_DESTOVERLAY) > 0);

            if (DriverCaps.dwCaps & DDCAPS_ALIGNSIZESRC)
            {
                SrcSizeAlign = DriverCaps.dwAlignSizeSrc;
            }
            else
            {
                SrcSizeAlign = 1;
            }

            if (DriverCaps.dwCaps & DDCAPS_ALIGNSIZEDEST)
            {
                DestSizeAlign = DriverCaps.dwAlignSizeDest;
            }
            else
            {
                DestSizeAlign = 1;
            }
            bCanDoBob = ((DriverCaps.dwCaps2 & DDCAPS2_CANFLIPODDEVEN) != 0);
        }
        else
        {
            ErrorBox("Can't Use Overlay");
            return (FALSE);
        }
    }

    ddrval = lpDD->SetCooperativeLevel(hWnd, DDSCL_NORMAL);

    if (FAILED(ddrval))
    {
        ErrorBox("SetCooperativeLevel failed");
        return (FALSE);
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
// Deinitialize DirectDraw
void ExitDD(void)
{
    if (lpDD != NULL)
    {
        Overlay_Destroy();
        lpDD->Release();
        lpDD = NULL;
    }
    DeleteCriticalSection(&hDDCritSect);
}

#define LIMIT(x) (((x)<0)?0:((x)>255)?255:(x))
#pragma pack(1)

// A TIFF image-file directory entry.  There are a bunch of
// these in a TIFF file.
typedef struct 
{
    WORD Tag;       // Entry Type
    WORD Type;      // 1=byte, 2=C string, 3=word, 4=dword (we always use dword)
    DWORD Count;    // Number of units (of Type specified by "Type") in Value
    DWORD Value;
} TTiffDirEntry;

// Field data types.
enum eTiffDataType
{
    Byte = 1,
    String = 2,
    Short = 3,
    Long = 4
};

// A TIFF header with some hardwired fields.
typedef struct 
{
    char byteOrder[2];
    WORD version;
    DWORD firstDirOffset;

    // TIFF files contain a bunch of extra information, each of which is a
    // tagged "directory" entry.  The entries must be in ascending numerical
    // order.

    WORD numDirEntries;
    TTiffDirEntry fileType;      // What kind of file this is (Tag 254)
    TTiffDirEntry width;         // Width of image (Tag 256)
    TTiffDirEntry height;            // Height of image (Tag 257)
    TTiffDirEntry bitsPerSample; // Number of bits per channel per pixel (Tag 258)
    TTiffDirEntry compression;   // Compression settings (Tag 259)
    TTiffDirEntry photometricInterpretation; // What kind of pixel data this is (Tag 262)
    TTiffDirEntry description;   // Image description (Tag 270)
    TTiffDirEntry make;          // "Scanner" maker, aka DScaler's URL (Tag 271)
    TTiffDirEntry model;         // "Scanner" model, aka DScaler version (Tag 272)
    TTiffDirEntry stripOffset;   // Offset to image data (Tag 273)
    TTiffDirEntry samplesPerPixel; // Number of color channels (Tag 277)
    TTiffDirEntry rowsPerStrip;  // Number of rows in a strip (Tag 278)
    TTiffDirEntry stripByteCounts; // Number of bytes per strip (Tag 279)
    TTiffDirEntry planarConfiguration; // Are channels interleaved? (Tag 284)
    DWORD nextDirOffset;

    // We store a few strings in the file; include them in the structure so
    // it's easy to compute their offsets.  Yeah, this wastes a bit of disk
    // space, but an insignificant percentage of the overall file size.
    char descriptionText[80];
    char makeText[40];
    char modelText[16];
    WORD bitCounts[3];
} TTiffHeader;

#define STRUCT_OFFSET(s,f)  ((int)(((BYTE*)&(s)->f) - (BYTE*)(s)))

//-----------------------------------------------------------------------------
// Fill a TIFF directory entry with information.
static void FillTiffDirEntry(TTiffDirEntry* entry, WORD Tag, DWORD Value, eTiffDataType Type)
{
    BYTE bValue;
    WORD wValue;

    entry->Tag = Tag;
    entry->Count = 1;
    entry->Type = (int) Type;

    switch (Type) {
    case Byte:
        bValue = (BYTE) Value;
        memcpy(&entry->Value, &bValue, 1);
        break;

    case Short:
        wValue = (WORD) Value;
        memcpy(&entry->Value, &wValue, 2);
        break;

    case String:    // in which case it's a file offset
    case Long:
        entry->Value = Value;
        break;
    }
}


//-----------------------------------------------------------------------------
// Fill a TIFF header with information about the current image.
static void FillTiffHeader(TTiffHeader* head, char* description, char* make, char* model)
{
    memset(head, 0, sizeof(TTiffHeader));

    strcpy(head->byteOrder, "II");      // Intel byte order
    head->version = 42;                 // We're TIFF 5.0 compliant, but the version field is unused
    head->firstDirOffset = STRUCT_OFFSET(head, numDirEntries);
    head->numDirEntries = 14;
    head->nextDirOffset = 0;            // No additional directories

    strcpy(head->descriptionText, description);
    strcpy(head->makeText, make);
    strcpy(head->modelText, model);
    head->bitCounts[0] = head->bitCounts[1] = head->bitCounts[2] = 8;

    head->description.Tag = 270;
    head->description.Type = 2;
    head->description.Count = strlen(description) + 1;
    head->description.Value = STRUCT_OFFSET(head, descriptionText);

    head->make.Tag = 271;
    head->make.Type = 2;
    head->make.Count = strlen(make) + 1;
    head->make.Value = STRUCT_OFFSET(head, makeText);

    head->model.Tag = 272;
    head->model.Type = 2;
    head->model.Count = strlen(model) + 1;
    head->model.Value = STRUCT_OFFSET(head, modelText);
    
    head->bitsPerSample.Tag = 258;
    head->bitsPerSample.Type = Short;
    head->bitsPerSample.Count = 3;
    head->bitsPerSample.Value = STRUCT_OFFSET(head, bitCounts);

    FillTiffDirEntry(&head->fileType, 254, 0, Long);                        // Just the image, no thumbnails
    FillTiffDirEntry(&head->width, 256, CurrentX, Short);
    FillTiffDirEntry(&head->height, 257, CurrentY, Short);
    FillTiffDirEntry(&head->compression, 259, 1, Short);                    // No compression
    FillTiffDirEntry(&head->photometricInterpretation, 262, 2, Short);      // RGB image data
    FillTiffDirEntry(&head->stripOffset, 273, sizeof(TTiffHeader), Long);    // Image comes after header
    FillTiffDirEntry(&head->samplesPerPixel, 277, 3, Short);                // RGB = 3 channels/pixel
    FillTiffDirEntry(&head->rowsPerStrip, 278, CurrentY, Short);            // Whole image is one strip
    FillTiffDirEntry(&head->stripByteCounts, 279, CurrentX * CurrentY * 3, Long);   // Size of image data
    FillTiffDirEntry(&head->planarConfiguration, 284, 1, Short);            // RGB bytes are interleaved
}

//-----------------------------------------------------------------------------
// Save still image snapshot as TIFF format to disk
// FileName is an output parameter : the file generated
BOOL SaveStill(char *FileName)
{
    int y, cr, cb, r, g, b, i, j, n = 0;
    FILE* file;
    BYTE rgb[3];
    BYTE* buf;
    char name[13];
    struct stat st;
    TTiffHeader head;
    DDSURFACEDESC SurfaceDesc;
    HRESULT ddrval;
    char description[80];

    *FileName = '\0';

    if (lpDDOverlay == NULL)
    {
        return FALSE;
    }

    memset(&SurfaceDesc, 0x00, sizeof(SurfaceDesc));
    SurfaceDesc.dwSize = sizeof(SurfaceDesc);

    ddrval = IDirectDrawSurface_Lock(lpDDOverlay, NULL, &SurfaceDesc, DDLOCK_WAIT, NULL);
    if (FAILED(ddrval))
    {
        ErrorBox("Error Locking Overlay");
        return FALSE;
    }

    while (n < 100)
    {
        sprintf(name,"tv%06d.tif",++n) ;
        if (stat(name, &st))
            break;
    }
    if(n == 100)
    {
        ErrorBox("Could not create a file.  You may have too many captures already.");
        ddrval = IDirectDrawSurface_Unlock(lpDDOverlay, SurfaceDesc.lpSurface);
        if (FAILED(ddrval))
        {
            ErrorBox("Error Unlocking Overlay");
        }
        return FALSE;
    }

    file = fopen(name,"wb");
    if (!file)
    {
        ErrorBox("Could not open file in SaveStill");
        ddrval = IDirectDrawSurface_Unlock(lpDDOverlay, SurfaceDesc.lpSurface);
        if (FAILED(ddrval))
        {
            ErrorBox("Error Unlocking Overlay");
        }
        return FALSE;
    }

    strcpy(FileName, name);

    sprintf(description, "DScaler image, deinterlace Mode %s", GetDeinterlaceModeName());
    // How do we figure out our version number?!?!
    FillTiffHeader(&head, description, "http://deinterlace.sourceforge.net/", "DScaler version 2.x");
    fwrite(&head, sizeof(head), 1, file);

    for (i = 0; i < CurrentY; i++ )
    {
        buf = (BYTE*)SurfaceDesc.lpSurface + i * SurfaceDesc.lPitch;
        for (j = 0; j < CurrentX ; j+=2)
        {
            cb = buf[1] - 128;
            cr = buf[3] - 128;
            y = buf[0] - 16;

            r = ( 76284*y + 104595*cr             )>>16;
            g = ( 76284*y -  53281*cr -  25624*cb )>>16;
            b = ( 76284*y             + 132252*cb )>>16;
            rgb[0] = LIMIT(r);
            rgb[1] = LIMIT(g);
            rgb[2] = LIMIT(b);

            fwrite(rgb,3,1,file) ;

            y = buf[2] - 16;
            r = ( 76284*y + 104595*cr             )>>16;
            g = ( 76284*y -  53281*cr -  25624*cb )>>16;
            b = ( 76284*y             + 132252*cb )>>16;
            rgb[0] = LIMIT(r);
            rgb[1] = LIMIT(g);
            rgb[2] = LIMIT(b);
            fwrite(rgb,3,1,file);

            buf += 4;
        }
    }
    fclose(file);
    ddrval = IDirectDrawSurface_Unlock(lpDDOverlay, SurfaceDesc.lpSurface);
    if (FAILED(ddrval))
    {
        ErrorBox("Error Unlocking Overlay");
        return FALSE;
    }
    return TRUE;
}

BOOL Overlay_ColorKey_OnChange(long NewValue)
{
    OverlayColor = (COLORREF)NewValue;
    WorkoutOverlaySize();
    return FALSE;
}

BOOL Overlay_Brightness_OnChange(long NewValue)
{
   OverlayBrightness = NewValue;
   Overlay_SetColorControls();
   return FALSE;
}

BOOL Overlay_Contrast_OnChange(long NewValue)
{
   OverlayContrast = NewValue;
   Overlay_SetColorControls();
   return FALSE;
}

BOOL Overlay_Hue_OnChange(long NewValue)
{
   OverlayHue = NewValue;
   Overlay_SetColorControls();
   return FALSE;
}

BOOL Overlay_Saturation_OnChange(long NewValue)
{
   OverlaySaturation = NewValue;
   Overlay_SetColorControls();
   return FALSE;
}

BOOL Overlay_Gamma_OnChange(long NewValue)
{
   OverlayGamma = NewValue;
   Overlay_SetColorControls();
   return FALSE;
}

BOOL Overlay_Sharpness_OnChange(long NewValue)
{
   OverlaySharpness = NewValue;
   Overlay_SetColorControls();
   return FALSE;
}

BOOL Overlay_UseControls_OnChange(long NewValue)
{
    bUseOverlayControls = NewValue;
    if(bUseOverlayControls)
    {
       Overlay_SetColorControls();
    }
    else
    {
       Overlay_ResetColorControls();
    }
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////

SETTING OtherSettings[OTHER_SETTING_LASTONE] =
{
    {
        "Back Buffers", SLIDER, 0, (long*)&BackBuffers,
        -1, -1, 2, 1, 1,
        NULL,
        "Overlay", "BackBuffers", NULL,
    },
    {
        "Overlay Colorkey", SLIDER, 0, (long*)&OverlayColor,
        RGB(32,16,16), 0, RGB(255,255,255), 1, 1,
        NULL,
        "Overlay", "OverlayColor", Overlay_ColorKey_OnChange,
    },
    {
        "Use Overlay Controls", ONOFF, 0, (long*)&bUseOverlayControls,
         FALSE, 0, 1, 1, 1,
         NULL,
        "Overlay", "UseOverlayControls", Overlay_UseControls_OnChange,
    },
    // DirectX docs say brightness value should be 0 to 10,000
    // which would mean a range of 0 to 1000.
    // however nVidia cards seem to have a range of -200 to 200
    // so the curreent range allows for this and leaves a lage margin
    // of error
    {
        "Overlay Brightness", SLIDER, 0, (long*)&OverlayBrightness,
        75, -1000, 1000, 5, 10,
        NULL,
        "Overlay", "OverlayBrightness", Overlay_Brightness_OnChange,
    },
    {
        "Overlay Contrast", SLIDER, 0, (long*)&OverlayContrast,
        100, 0, 200, 1, 1,
        NULL,
        "Overlay", "OverlayContrast", Overlay_Contrast_OnChange,
    },
    {
        "Overlay Hue", SLIDER, 0, (long*)&OverlayHue,
        0, -180, 180, 1, 1,
        NULL,
        "Overlay", "OverlayHue", Overlay_Hue_OnChange,
    },
    {
        "Overlay Saturation", SLIDER, 0, (long*)&OverlaySaturation,
        100, 0, 200, 1, 1,
        NULL,
        "Overlay", "OverlaySaturation", Overlay_Saturation_OnChange,
    },
    {
        "Overlay Gamma", SLIDER, 0, (long*)&OverlayGamma,
        1, 1, 500, 1, 1,
        NULL,
        "Overlay", "OverlayGamma", Overlay_Gamma_OnChange,
    },
    {
        "Overlay Sharpness", SLIDER, 0, (long*)&OverlaySharpness,
        5, 
        0, 10, 1, 1,
        NULL,
        "Overlay", "OverlaySharpness", Overlay_Sharpness_OnChange,
    },
};


SETTING* Other_GetSetting(OTHER_SETTING Setting)
{
    if(Setting > -1 && Setting < OTHER_SETTING_LASTONE)
    {
        return &(OtherSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void Other_ReadSettingsFromIni()
{
    int i;
    for(i = 0; i < OTHER_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(OtherSettings[i]));
    }
}

void Other_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < OTHER_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(OtherSettings[i]), bOptimizeFileAccess);
    }
}

void Other_ShowUI()
{
    CSettingsDlg::ShowSettingsDlg("Overlay Settings",OtherSettings, OTHER_SETTING_LASTONE);
}
