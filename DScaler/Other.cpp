/////////////////////////////////////////////////////////////////////////////
// other.h
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
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
// 06 Jan 2001   John Adcock           Addded extra info on overlay error boxed
// 
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Other.h"
#include "DScaler.h"
#include "BT848.h"
#include "OutThreads.h"
#include "VBI_VideoText.h"
#include "ErrorBox.h"
#include "Splash.h"
#include "DebugLog.h"

LPDIRECTDRAW lpDD = NULL;

LPDIRECTDRAWSURFACE     lpDDSurface = NULL;
// OverLay
LPDIRECTDRAWSURFACE     lpDDOverlay = NULL;
LPDIRECTDRAWSURFACE     lpDDOverlayBack = NULL;
BOOL Can_ColorKey=FALSE;
DWORD DestSizeAlign;
DWORD SrcSizeAlign;
COLORREF OverlayColor = RGB(32, 16, 16);
DWORD PhysicalOverlayColor = RGB(32, 16, 16);
long Back_Buffers = -1;		// Make new user parm, TRB 10/28/00
BOOL bCanDoBob = FALSE;
BOOL bCanDoColorKey = FALSE;

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
	int nPixel;
	int nLine;
	HRESULT ddrval;
	DDSURFACEDESC ddsd;

	if (lpDDOverlay != NULL)
	{
		memset(&ddsd, 0x00, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);

		ddrval = IDirectDrawSurface_Lock(lpDDOverlay, NULL, &ddsd, DDLOCK_WAIT, NULL);

		for (nLine = 0; nLine < (signed) ddsd.dwHeight; nLine++)
		{
			for (nPixel = 0; nPixel < (signed) ddsd.dwWidth * 2; nPixel += 4)
			{
				*((int *) ddsd.lpSurface + (nLine * ddsd.lPitch + nPixel) / 4) = 0x80008000;
			}
		}
		ddrval = IDirectDrawSurface_Unlock(lpDDOverlay, ddsd.lpSurface);
	}
	if (lpDDOverlayBack != NULL)
	{
		memset(&ddsd, 0x00, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);

		ddrval = IDirectDrawSurface_Lock(lpDDOverlayBack, NULL, &ddsd, DDLOCK_WAIT, NULL);

		for (nLine = 0; nLine < (signed) ddsd.dwHeight; nLine++)
		{
			for (nPixel = 0; nPixel < (signed) ddsd.dwWidth * 2; nPixel += 4)
			{
				*((int *) ddsd.lpSurface + (nLine * ddsd.lPitch + nPixel) / 4) = 0x80008000;
			}
		}
		ddrval = IDirectDrawSurface_Unlock(lpDDOverlayBack, ddsd.lpSurface);
	}
}

//-----------------------------------------------------------------------------
// Update video overlay with new rectangle
BOOL Overlay_Update(LPRECT pSrcRect, LPRECT pDestRect, DWORD dwFlags, BOOL ColorKey)
{
	HRESULT		ddrval;
	DDOVERLAYFX DDOverlayFX;

	if ((lpDD == NULL) || (lpDDSurface == NULL) || (lpDDOverlay == NULL))
	{
		return FALSE;
	}

	memset(&DDOverlayFX, 0x00, sizeof(DDOverlayFX));
	DDOverlayFX.dwSize = sizeof(DDOverlayFX);

	if (pSrcRect == NULL)
	{
		////////////////////////////////
		// we are trying to hide overlay
		////////////////////////////////
		ddrval = IDirectDrawSurface_UpdateOverlay(lpDDOverlay, NULL, lpDDSurface, NULL, dwFlags, &DDOverlayFX);
		// if another device has requested exclusive access then we
		// can get the no hardware error, just wait a bit and try again
		while(ddrval == DDERR_NOOVERLAYHW)
		{
			Sleep(100);
			ddrval = IDirectDrawSurface_UpdateOverlay(lpDDOverlay, NULL, lpDDSurface, NULL, dwFlags, &DDOverlayFX);
		}
		// just return if we get this here
		// all DDERR_SURFACELOST will be handled by
		// the main processing loop
		if(ddrval == DDERR_SURFACELOST)
		{
			return FALSE;
		}
		if (FAILED(ddrval))
		{
			// 2001-01-06 John Adcock
			// Now show return code
			char szErrorMsg[200];
			sprintf(szErrorMsg, "Error %x calling UpdateOverlay (Hide)", ddrval);
			ErrorBox(szErrorMsg);
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

			PhysicalOverlayColor = Overlay_ColorMatch(lpDDSurface, OverlayColor);
			if (PhysicalOverlayColor == 0)		// sometimes we glitch and can't get the value
			{
				LOG(" Physical overlay color is zero!  Retrying.");
				PhysicalOverlayColor = Overlay_ColorMatch(lpDDSurface, OverlayColor);
			}
			LOG(" Physical overlay color is %x", PhysicalOverlayColor);

			DDOverlayFX.dckDestColorkey.dwColorSpaceHighValue = PhysicalOverlayColor;
			DDOverlayFX.dckDestColorkey.dwColorSpaceLowValue = PhysicalOverlayColor;
		}
		if(bCanDoBob)
		{
			dwFlags |= DDOVER_BOB;
		}

		ddrval = IDirectDrawSurface_UpdateOverlay(lpDDOverlay, pSrcRect, lpDDSurface, pDestRect, dwFlags, &DDOverlayFX);
		// if another device has requested exclusive access then we
		// can get the no hardware error, just wait a bit and try again
		while(ddrval == DDERR_NOOVERLAYHW)
		{
			Sleep(100);
			ddrval = IDirectDrawSurface_UpdateOverlay(lpDDOverlay, pSrcRect, lpDDSurface, pDestRect, dwFlags, &DDOverlayFX);
		}
		// just return if we get this here
		// all DDERR_SURFACELOST will be handled by
		// the main processing loop
		if(ddrval == DDERR_SURFACELOST)
		{
			return FALSE;
		}
		// we get unsupported error here for mpact2 cards
		// so cope with this by not trying to update
		// the color key value and just hoping it works
		// with the existing one (black used to work)
		if(ddrval == DDERR_UNSUPPORTED)
		{
			DDCOLORKEY ColorKey;

			LOG(" Got unsupported error from Overlay Update");
			ddrval = IDirectDrawSurface_GetColorKey(lpDDOverlay, DDCKEY_DESTOVERLAY, &ColorKey);
			if(SUCCEEDED(ddrval))
			{
				OverlayColor = ColorKey.dwColorSpaceHighValue;
				LOG(" Reset overlay color to %x", OverlayColor);
			}
			dwFlags &= ~DDOVER_KEYDESTOVERRIDE;
			memset(&DDOverlayFX, 0x00, sizeof(DDOverlayFX));
			DDOverlayFX.dwSize = sizeof(DDOverlayFX);
			ddrval = IDirectDrawSurface_UpdateOverlay(lpDDOverlay, pSrcRect, lpDDSurface, pDestRect, dwFlags, &DDOverlayFX);
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
			return FALSE;
		}
	}
	return TRUE;
}

//-----------------------------------------------------------------------------
// Create new video overlay
BOOL Overlay_Create()
{
	DDSURFACEDESC ddsd;
	DDPIXELFORMAT PixelFormat;
	HRESULT ddrval;
	DDSCAPS caps;
	int minBuffers, maxBuffers, numBuffers;
	char msg[500];

	if (lpDDOverlay)
	{
		return FALSE;
	}

	// 2000-10-31 Moved by Mark Rejhon
	// Attempt to create primary surface before overlay, in this module,
	// because we may have destroyed the primary surface during a computer 
	// resolution change.
	memset(&ddsd, 0x00, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	ddrval = IDirectDraw_CreateSurface(lpDD, &ddsd, &lpDDSurface, NULL);
	if (FAILED(ddrval))
	{
		sprintf(msg, "Error creating primary surface: %x", ddrval);
		RealErrorBox(msg);
		return (FALSE);
	}

	ddrval = IDirectDrawSurface_Lock(lpDDSurface, NULL, &ddsd, DDLOCK_WAIT, NULL);
	if (FAILED(ddrval))
	{
		sprintf(msg, "Error locking primary surface: %x", ddrval);
		RealErrorBox(msg);
		return (FALSE);
	}
	ddrval = IDirectDrawSurface_Unlock(lpDDSurface, ddsd.lpSurface);
	if (FAILED(ddrval))
	{
		sprintf(msg, "Error unlocking primary surface: %x", ddrval);
		RealErrorBox(msg);
		return (FALSE);
	}

	memset(&PixelFormat, 0x00, sizeof(PixelFormat));
	PixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	PixelFormat.dwFlags = DDPF_FOURCC;
	PixelFormat.dwFourCC = MAKEFOURCC('Y', 'U', 'Y', '2');;
	PixelFormat.dwYUVBitCount = 16;

	memset(&ddsd, 0x00, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_VIDEOMEMORY | DDSCAPS_FLIP | DDSCAPS_COMPLEX;

	// create a surface big enough to hold the largest resolution supported
	// this ensures that we can always have enough space to allow
	// mode changes without recreating the overlay
	ddsd.dwWidth = DTV_MAX_WIDTH;
	ddsd.dwHeight = DTV_MAX_HEIGHT;
	ddsd.ddpfPixelFormat = PixelFormat;

	// If the user specified a particular back buffer count, use it.  Otherwise
	// try triple buffering and drop down to double buffering, then single
	// buffering, if the card doesn't have enough memory.
	minBuffers = Back_Buffers >= 0 ? Back_Buffers : 0;
	maxBuffers = Back_Buffers >= 0 ? Back_Buffers : 2;

	for (numBuffers = maxBuffers; numBuffers >= minBuffers; numBuffers--)
	{
		ddsd.dwBackBufferCount = numBuffers;
		ddrval = IDirectDraw_CreateSurface(lpDD, &ddsd, &lpDDOverlay, NULL);

		if (SUCCEEDED(ddrval) || ddrval != DDERR_OUTOFVIDEOMEMORY)
			break;
	}

	if (FAILED(ddrval))
		lpDDOverlay = NULL;

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
			return (FALSE);
		}
		else
		{
			// We didn't get down to single-buffering, meaning the user
			// specified a back buffer count.
			sprintf(msg, "Your video card doesn't have enough overlay\n"
						 "memory for %d back buffers.  If you've used\n"
						 "that many back buffers before, you may need\n"
						 "to reboot.  Otherwise try lowering your screen\n"
						 "resolution or color depth, or try setting\n"
						 "Back_Buffers=-1 in DScaler.ini to allow DScaler to\n"
						 "decide how many back buffers it can allocate.",
					Back_Buffers);
			RealErrorBox(msg);
			return (FALSE);
		}
	}

	if (FAILED(ddrval))
	{
		switch (ddrval) {
		case DDERR_NOOVERLAYHW:
			RealErrorBox("Your video card doesn't appear to support\n"
				     "overlays, which DScaler requires.");
			return (FALSE);

			// Any other interesting error codes?
		}
		
		sprintf(msg, "Can't create overlay surface: %x", ddrval);
		RealErrorBox(msg);
		return FALSE;
	}

	sprintf(msg, "%d Back Buffers", numBuffers);
	AddSplashTextLine(msg);
	LOG(msg);

	ddrval = IDirectDrawSurface_Lock(lpDDOverlay, NULL, &ddsd, DDLOCK_WAIT, NULL);
	// sometimes in win98 we get weird error messages here
	// so we need to loop until it's OK or we get a surface lost message
	while(ddrval == DDERR_NOOVERLAYHW || ddrval == DDERR_SURFACEBUSY)
	{
		Sleep(100);
		ddrval = IDirectDrawSurface_Lock(lpDDOverlay, NULL, &ddsd, DDLOCK_WAIT, NULL);
	}
	if(ddrval == DDERR_SURFACELOST)
	{
		ddrval = IDirectDraw_CreateSurface(lpDD, &ddsd, &lpDDOverlay, NULL);
		if (FAILED(ddrval))
		{
			sprintf(msg, "Lost overlay surface and can't recreate it: %x", ddrval);
			RealErrorBox(msg);
			lpDDOverlay = NULL;
			return FALSE;
		}
		ddrval = IDirectDrawSurface_Lock(lpDDOverlay, NULL, &ddsd, DDLOCK_WAIT, NULL);
	}
	if (FAILED(ddrval))
	{
		char szErrorMsg[200];
		sprintf(szErrorMsg, "Error %x in Lock Surface", ddrval);
		RealErrorBox(szErrorMsg);
		return (FALSE);
	}

	ddrval = IDirectDrawSurface_Unlock(lpDDOverlay, ddsd.lpSurface);
	if (FAILED(ddrval))
	{
		RealErrorBox("Can't Unlock Surface");
		return (FALSE);
	}

	memset(&caps, 0, sizeof(caps));
	caps.dwCaps = DDSCAPS_BACKBUFFER;
	ddrval = IDirectDrawSurface_GetAttachedSurface(lpDDOverlay, &caps, &lpDDOverlayBack);
	if (FAILED(ddrval))
	{
		RealErrorBox("Can't create Overlay Back Surface");
		lpDDOverlayBack = NULL;
		return (FALSE);
	}
	else
	{
		ddrval = IDirectDrawSurface_Lock(lpDDOverlayBack, NULL, &ddsd, DDLOCK_WAIT, NULL);
		if (FAILED(ddrval))
		{
			RealErrorBox("Can't Lock Back Surface");
			return (FALSE);
		}
		ddrval = DDERR_WASSTILLDRAWING;

		ddrval = IDirectDrawSurface_Unlock(lpDDOverlayBack, ddsd.lpSurface);
		if (FAILED(ddrval))
		{
			RealErrorBox("Can't Unlock Back Surface");
			return (FALSE);
		}
	}

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
    DDSURFACEDESC ddsd;
    HRESULT hres;

    //
    //  Use GDI SetPixel to color match for us
    //
	hres = IDirectDrawSurface_GetDC(pdds, &hdc);
    if (SUCCEEDED(hres))
    {
        rgbT = GetPixel(hdc, 0, 0);     // Save current pixel value
        SetPixel(hdc, 0, 0, rgb);       // Set our value
		IDirectDrawSurface_ReleaseDC(pdds, hdc);
    }
    //
    // Now lock the surface so we can read back the converted color
    //
    ddsd.dwSize = sizeof(ddsd);
    hres = IDirectDrawSurface_Lock(pdds, NULL, &ddsd, DDLOCK_WAIT, NULL);
    if (SUCCEEDED(hres))
    {
        dw = *(DWORD *) ddsd.lpSurface;                 // Get DWORD
        if (ddsd.ddpfPixelFormat.dwRGBBitCount < 32)
		{
            dw &= (1 << ddsd.ddpfPixelFormat.dwRGBBitCount) - 1;  // Mask it to bpp
		}
        IDirectDrawSurface_Unlock(pdds, NULL);
    }
    //
    //  Now put the color that was there back.
    //
	hres = IDirectDrawSurface_GetDC(pdds, &hdc);
    if (SUCCEEDED(hres))
    {
        SetPixel(hdc, 0, 0, rgbT);
        IDirectDrawSurface_ReleaseDC(pdds, hdc);
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
	// Now destroy the Back Overlay
	if (lpDDOverlayBack != NULL)
	{
		IDirectDrawSurface_Release(lpDDOverlayBack);
		lpDDOverlayBack = NULL;
	}

	// Now destroy the main Overlay
	if (lpDDOverlay != NULL)
	{
		// Destroy the video overlays
		IDirectDrawSurface_Release(lpDDOverlay);
		lpDDOverlay = NULL;
	}

	// Now destroy the primary surface
	if (lpDDSurface != NULL) 
	{
		IDirectDrawSurface_Release(lpDDSurface);
		lpDDSurface = NULL;
	}
	return TRUE;
}

COLORREF Overlay_GetColor()
{
	return OverlayColor;
}

void Overlay_WaitForVerticalBlank()
{
	HRESULT ddrval = -1;

	while(ddrval != DD_OK)
	{
		ddrval = IDirectDraw_WaitForVerticalBlank(lpDD, DDWAITVB_BLOCKEND, NULL);
	}
}


//-----------------------------------------------------------------------------
// Initialize DirectDraw
BOOL InitDD(HWND hWnd)
{
	HRESULT ddrval;
	DDCAPS DriverCaps;

	if (FAILED(DirectDrawCreate(NULL, &lpDD, NULL)))
	{
		ErrorBox("DirectDrawCreate failed");
		return (FALSE);
	}

	// can we use Overlay ??
	memset(&DriverCaps, 0x00, sizeof(DriverCaps));
	DriverCaps.dwSize = sizeof(DriverCaps);
	ddrval = IDirectDraw_GetCaps(lpDD, &DriverCaps, NULL);

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

	ddrval = IDirectDraw_SetCooperativeLevel(lpDD, hWnd, DDSCL_NORMAL);

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
		IDirectDraw_Release(lpDD);
		lpDD = NULL;
	}
}

#define LIMIT(x) (((x)<0)?0:((x)>255)?255:(x))
#pragma pack(1)

// A TIFF image-file directory entry.  There are a bunch of
// these in a TIFF file.
struct TiffDirEntry {
	WORD tag;		// Entry type
	WORD type;		// 1=byte, 2=C string, 3=word, 4=dword (we always use dword)
	DWORD count;	// Number of units (of type specified by "type") in value
	DWORD value;
};

// Field data types.
enum TiffDataType {
	Byte = 1,
	String = 2,
	Short = 3,
	Long = 4
};

// A TIFF header with some hardwired fields.
struct TiffHeader {
	char byteOrder[2];
	WORD version;
	DWORD firstDirOffset;

	// TIFF files contain a bunch of extra information, each of which is a
	// tagged "directory" entry.  The entries must be in ascending numerical
	// order.

	WORD numDirEntries;
	struct TiffDirEntry fileType;		// What kind of file this is (tag 254)
	struct TiffDirEntry width;			// Width of image (tag 256)
	struct TiffDirEntry height;			// Height of image (tag 257)
	struct TiffDirEntry bitsPerSample;	// Number of bits per channel per pixel (tag 258)
	struct TiffDirEntry compression;	// Compression settings (tag 259)
	struct TiffDirEntry photometricInterpretation; // What kind of pixel data this is (tag 262)
	struct TiffDirEntry description;	// Image description (tag 270)
	struct TiffDirEntry make;			// "Scanner" maker, aka dTV's URL (tag 271)
	struct TiffDirEntry model;			// "Scanner" model, aka dTV version (tag 272)
	struct TiffDirEntry stripOffset;	// Offset to image data (tag 273)
	struct TiffDirEntry samplesPerPixel; // Number of color channels (tag 277)
	struct TiffDirEntry rowsPerStrip;	// Number of rows in a strip (tag 278)
	struct TiffDirEntry stripByteCounts; // Number of bytes per strip (tag 279)
	struct TiffDirEntry planarConfiguration; // Are channels interleaved? (tag 284)
	DWORD nextDirOffset;

	// We store a few strings in the file; include them in the structure so
	// it's easy to compute their offsets.  Yeah, this wastes a bit of disk
	// space, but an insignificant percentage of the overall file size.
	char descriptionText[80];
	char makeText[40];
	char modelText[16];
	WORD bitCounts[3];
};

#define STRUCT_OFFSET(s,f)  ((int)(((BYTE *) &(s)->f) - (BYTE *)(s)))


//-----------------------------------------------------------------------------
// Fill a TIFF directory entry with information.
static void FillTiffDirEntry(struct TiffDirEntry *entry, WORD tag, DWORD value, enum TiffDataType type)
{
	BYTE bValue;
	WORD wValue;

	entry->tag = tag;
	entry->count = 1;
	entry->type = (int) type;

	switch (type) {
	case Byte:
		bValue = (BYTE) value;
		memcpy(&entry->value, &bValue, 1);
		break;

	case Short:
		wValue = (WORD) value;
		memcpy(&entry->value, &wValue, 2);
		break;

	case String:	// in which case it's a file offset
	case Long:
		entry->value = value;
		break;
	}
}


//-----------------------------------------------------------------------------
// Fill a TIFF header with information about the current image.
static void FillTiffHeader(struct TiffHeader *head, char *description, char *make, char *model)
{
	memset(head, 0, sizeof(struct TiffHeader));

	strcpy(head->byteOrder, "II");		// Intel byte order
	head->version = 42;					// We're TIFF 5.0 compliant, but the version field is unused
	head->firstDirOffset = STRUCT_OFFSET(head, numDirEntries);
	head->numDirEntries = 14;
	head->nextDirOffset = 0;			// No additional directories

	strcpy(head->descriptionText, description);
	strcpy(head->makeText, make);
	strcpy(head->modelText, model);
	head->bitCounts[0] = head->bitCounts[1] = head->bitCounts[2] = 8;

	head->description.tag = 270;
	head->description.type = 2;
	head->description.count = strlen(description) + 1;
	head->description.value = STRUCT_OFFSET(head, descriptionText);

	head->make.tag = 271;
	head->make.type = 2;
	head->make.count = strlen(make) + 1;
	head->make.value = STRUCT_OFFSET(head, makeText);

	head->model.tag = 272;
	head->model.type = 2;
	head->model.count = strlen(model) + 1;
	head->model.value = STRUCT_OFFSET(head, modelText);
	
	head->bitsPerSample.tag = 258;
	head->bitsPerSample.type = Short;
	head->bitsPerSample.count = 3;
	head->bitsPerSample.value = STRUCT_OFFSET(head, bitCounts);

	FillTiffDirEntry(&head->fileType, 254, 0, Long);						// Just the image, no thumbnails
	FillTiffDirEntry(&head->width, 256, CurrentX, Short);
	FillTiffDirEntry(&head->height, 257, CurrentY, Short);
	FillTiffDirEntry(&head->compression, 259, 1, Short);					// No compression
	FillTiffDirEntry(&head->photometricInterpretation, 262, 2, Short);		// RGB image data
	FillTiffDirEntry(&head->stripOffset, 273, sizeof(struct TiffHeader), Long);	// Image comes after header
	FillTiffDirEntry(&head->samplesPerPixel, 277, 3, Short);				// RGB = 3 channels/pixel
	FillTiffDirEntry(&head->rowsPerStrip, 278, CurrentY, Short);			// Whole image is one strip
	FillTiffDirEntry(&head->stripByteCounts, 279, CurrentX * CurrentY * 3, Long);	// Size of image data
	FillTiffDirEntry(&head->planarConfiguration, 284, 1, Short);			// RGB bytes are interleaved
}

//-----------------------------------------------------------------------------
// Save still image snapshot as TIFF format to disk
void SaveStill()
{
	int y, cr, cb, r, g, b, i, j, n = 0;
	FILE *file;
	BYTE rgb[3];
	BYTE* buf;
	char name[13];
	struct stat st;
	struct TiffHeader head;
	DDSURFACEDESC ddsd;
	HRESULT ddrval;
	char description[80];

	if (lpDDOverlay != NULL)
	{
		memset(&ddsd, 0x00, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);

		ddrval = IDirectDrawSurface_Lock(lpDDOverlay, NULL, &ddsd, DDLOCK_WAIT, NULL);
		if (FAILED(ddrval))
		{
			ErrorBox("Error Locking Overlay");
			return;
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
			ddrval = IDirectDrawSurface_Unlock(lpDDOverlay, ddsd.lpSurface);
			if (FAILED(ddrval))
			{
				ErrorBox("Error Unlocking Overlay");
				return;
			}
			return;
		}

		file = fopen(name,"wb");
		if (!file)
		{
			ErrorBox("Could not open file in SaveStill");
			ddrval = IDirectDrawSurface_Unlock(lpDDOverlay, ddsd.lpSurface);
			if (FAILED(ddrval))
			{
				ErrorBox("Error Unlocking Overlay");
				return;
			}
			return;
		}

		sprintf(description, "DScaler image, deinterlace mode %s", GetDeinterlaceModeName());
		// How do we figure out our version number?!?!
		FillTiffHeader(&head, description, "http://deinterlace.sourceforge.net/", "DScaler version 2.x");
		fwrite(&head, sizeof(head), 1, file);

		for (i = 0; i < CurrentY; i++ )
		{
			buf = (BYTE*)ddsd.lpSurface + i * ddsd.lPitch;
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
		ddrval = IDirectDrawSurface_Unlock(lpDDOverlay, ddsd.lpSurface);
		if (FAILED(ddrval))
		{
			ErrorBox("Error Unlocking Overlay");
			return;
		}
	}
	return;
}

/////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////

SETTING OtherSettings[OTHER_SETTING_LASTONE] =
{
	{
		"Back Buffers", NUMBER, 0, (long*)&Back_Buffers,
		-1, -1, 2, 1, 1,
		NULL,
		"Overlay", "Back_Buffers", NULL,
	},
	{
		"Overlay Color", NUMBER, 0, (long*)&OverlayColor,
		RGB(32,16,16), 0, RGB(255,255,255), 1, 1,
		NULL,
		"Overlay", "OverlayColor", NULL,
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

void Other_WriteSettingsToIni()
{
	int i;
	for(i = 0; i < OTHER_SETTING_LASTONE; i++)
	{
		Setting_WriteToIni(&(OtherSettings[i]));
	}
}
