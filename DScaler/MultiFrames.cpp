/////////////////////////////////////////////////////////////////////////////
// $Id: MultiFrames.cpp,v 1.3 2003-03-19 23:55:19 laurentg Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 Laurent Garnier.  All rights reserved.
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
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// $Log: not supported by cvs2svn $
// Revision 1.2  2003/03/17 22:34:23  laurentg
// First step for the navigation through channels in preview mode
//
// Revision 1.1  2003/03/16 18:27:46  laurentg
// New multiple frames feature
//
//
/////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "MultiFrames.h"
#include "Other.h"
#include "StillSource.h"
#include "DebugLog.h"
#include "Providers.h"
#include "AspectRatio.h"
#include "ProgramList.h"
#include "DScaler.h"
//#include "OSD.h"


CMultiFrames* pMultiFrames = NULL;


CMultiFrames::CMultiFrames(int iFrames, int iDuration)
{
	bActive = FALSE;
	bSwitchRequested = FALSE;
	lpMemoryBuffer = NULL;
	iNbFrames = iFrames;
	iCurrentFrame = 0;
	iNbRows = (int)sqrt((double)iNbFrames);
	iNbCols = iNbFrames / iNbRows;
	if (iNbFrames % iNbRows)
	{
		iNbCols++;
	}
	iWidth = DSCALER_MAX_WIDTH;
	iHeight = DSCALER_MAX_HEIGHT;
	iDeltaTicksChange = iDuration;
}

CMultiFrames::~CMultiFrames()
{
	FreeMemoryBuffer();
}

int CMultiFrames::GetWidth()
{
	return iWidth;
}

int CMultiFrames::GetHeight()
{
	return iHeight;
}

BOOL CMultiFrames::IsActive()
{
	return bActive;
}

void CMultiFrames::Enable()
{
	AllocateMemoryBuffer();
	bFrameFilled = (int*) malloc(iNbFrames * sizeof(BOOL));
	bActive = (lpMemoryBuffer != NULL);
	if (bActive)
	{
		Reset();
		UpdateSquarePixelsMode(TRUE);
		AspectSettings.InitialTopOverscan = 0;
		AspectSettings.InitialBottomOverscan = 0;
		AspectSettings.InitialLeftOverscan = 0;
		AspectSettings.InitialRightOverscan = 0;
		AspectSettings.bAnalogueBlanking = FALSE;
		WorkoutOverlaySize(TRUE);
	}
}

void CMultiFrames::Disable()
{
	CSource* pSource = Providers_GetCurrentSource();

	FreeMemoryBuffer();
	free(bFrameFilled);
	bActive = FALSE;
    if (pSource)
    {
        UpdateSquarePixelsMode(pSource->HasSquarePixels());
	    pSource->SetAspectRatioData();
		WorkoutOverlaySize(TRUE);
    }
}

BOOL CMultiFrames::IsSwitchRequested()
{
	return bSwitchRequested;
}

void CMultiFrames::RequestSwitch()
{
	bSwitchRequested = TRUE;
}

void CMultiFrames::DoSwitch()
{
	bSwitchRequested = FALSE;
	if (IsActive())
	{
		Disable();
	}
	else
	{
		Enable();
	}
}

void CMultiFrames::Reset()
{
	if (!bActive)
	{
		return;
	}

	iCurrentFrame = 0;
	LastTickCount = 0;
	for (int i=0 ; i < iNbFrames ; i++)
	{
		bFrameFilled[i] = -1;
	}

	// Reset the memory buffer to a black picture
    int iPitch = iWidth * 2;
    BYTE* lpStartBuffer = START_ALIGNED16(lpMemoryBuffer);
	for (int iLine=0 ; iLine < iHeight ; iLine++)
	{
		for (int iPixel=0 ; iPixel < iWidth ; iPixel++)
		{
			lpStartBuffer[iPixel*2] = 16;
			lpStartBuffer[iPixel*2+1] = 128;
		}
		lpStartBuffer += iPitch;
	}
}

void CMultiFrames::SelectFrame()
{
	CSource* pSource = Providers_GetCurrentSource();
	int i;

	if (!bActive || !pSource)
	{
		bNavigAllowed = FALSE;
		return;
	}

	DWORD CurrentTickCount = GetTickCount();
	if (LastTickCount == 0)
	{
		LastTickCount = CurrentTickCount;
	}

	if (bFrameFilled[iCurrentFrame] == -1)
	{
		bNavigAllowed = FALSE;
		return;
	}

	for (i=0; i < iNbFrames ; i++)
	{
		if (bFrameFilled[(iCurrentFrame+i+1) % iNbFrames] == -1)
		{
			bNavigAllowed = FALSE;
			break;
		}
	}
	if (i == iNbFrames)
	{
		if (!bNavigAllowed)
		{
			iDeltaNewFrame = 0;
		}
		bNavigAllowed = TRUE;
	}
	if (bNavigAllowed)
	{
		if (iDeltaNewFrame != 0)
		{
			if ( ((iCurrentFrame + iDeltaNewFrame) >= 0)
			  && ((iCurrentFrame + iDeltaNewFrame) < iNbFrames) )
			{
				iCurrentFrame += iDeltaNewFrame;
				if (pSource->IsInTunerMode())
				{
					SendMessage(hWnd, WM_COMMAND, IDM_CHANNEL_INDEX, bFrameFilled[iCurrentFrame]);
				}
				else if ( (pSource == Providers_GetStillsSource())
				       || (pSource == Providers_GetSnapshotsSource())
				       || (pSource == Providers_GetPatternsSource())
				       || (pSource == Providers_GetIntroSource()) )
				{
					SendMessage(hWnd, WM_COMMAND, IDM_PLAYLIST_INDEX, bFrameFilled[iCurrentFrame]);
				}
				bFrameFilled[iCurrentFrame] = -1;
				bNavigAllowed = FALSE;
			}
			else
			{
				ShiftFrames(iDeltaNewFrame);
				if (pSource->IsInTunerMode())
				{
					SendMessage(hWnd, WM_COMMAND, IDM_CHANNEL_INDEX, bFrameFilled[iCurrentFrame]);
				}
				else if ( (pSource == Providers_GetStillsSource())
				       || (pSource == Providers_GetSnapshotsSource())
				       || (pSource == Providers_GetPatternsSource())
				       || (pSource == Providers_GetIntroSource()) )
				{
					SendMessage(hWnd, WM_COMMAND, IDM_PLAYLIST_INDEX, bFrameFilled[iCurrentFrame]);
				}
				bFrameFilled[iCurrentFrame] = -1;
				bNavigAllowed = FALSE;
			}
		}
	}
	else
	{
		if ((CurrentTickCount - LastTickCount) >= iDeltaTicksChange)
		{
			iCurrentFrame = (iCurrentFrame+i+1) % iNbFrames;
			LastTickCount = CurrentTickCount;
			if (pSource->IsInTunerMode())
			{
				SendMessage(hWnd, WM_COMMAND, IDM_CHANNELPLUS, 0);
			}
			else if ( (pSource == Providers_GetStillsSource())
				   || (pSource == Providers_GetSnapshotsSource())
				   || (pSource == Providers_GetPatternsSource())
				   || (pSource == Providers_GetIntroSource()) )
			{
				SendMessage(hWnd, WM_COMMAND, IDM_PLAYLIST_NEXT_CIRC, 0);
			}
		}
	}
}

void CMultiFrames::UpdateFrame(TDeinterlaceInfo* pInfo, BOOL* bUseExtraBuffer, BYTE** lpBuffer, int* Pitch)
{
	CSource* pSource = Providers_GetCurrentSource();

	if (!bActive || !pSource)
	{
		*lpBuffer = NULL;
		*Pitch = 0;
		return;
	}

	// Retrieve the buffer corresponding to the frame in the global multiple frames buffer
	SelectFrameBuffer(iCurrentFrame);

	// Shift to bottom for two lines and to right for two pixels
	// to leave place for the borders
	iFrameWidth -= 4;
	iFrameHeight -= 4;
	lpFrameBuffer += 2 * iFramePitch + 4;

	// Copy (with resize) the input picture into its frame
    Overlay_Lock_Back_Buffer(pInfo, *bUseExtraBuffer);
	if (pSource->HasSquarePixels())
	{
		// Keep the original ratio
		int iUpdWidth;
		int iUpdHeight;
		if ((pInfo->FrameWidth - iFrameWidth) > (pInfo->FrameHeight - iFrameHeight))
		{
			iUpdHeight = iFrameWidth * pInfo->FrameHeight / pInfo->FrameWidth;
			if (iUpdHeight > iFrameHeight)
			{
				iUpdHeight = iFrameHeight;
			}
			lpFrameBuffer += ( (iFrameHeight - iUpdHeight) / 2 ) * iFramePitch;
			iFrameHeight = iUpdHeight;
		}
		else
		{
			iUpdWidth = iFrameHeight * pInfo->FrameWidth / pInfo->FrameHeight;
			if (iUpdWidth > iFrameWidth)
			{
				iUpdWidth = iFrameWidth;
			}
			lpFrameBuffer += ( (iFrameWidth - iUpdWidth) / 2 ) * 2;
			iFrameWidth = iUpdWidth;
		}
	}
	ResizeFrame(pInfo->Overlay, pInfo->OverlayPitch, pInfo->FrameWidth, pInfo->FrameHeight, lpFrameBuffer, iFramePitch, iFrameWidth, iFrameHeight);
    Overlay_Unlock_Back_Buffer(*bUseExtraBuffer);

	if (pSource->IsInTunerMode())
	{
		bFrameFilled[iCurrentFrame] = Setting_GetValue(Channels_GetSetting(CURRENTPROGRAM));
	}
	else if ( (pSource == Providers_GetStillsSource())
		   || (pSource == Providers_GetSnapshotsSource())
		   || (pSource == Providers_GetPatternsSource())
		   || (pSource == Providers_GetIntroSource()) )
	{
		bFrameFilled[iCurrentFrame] = ((CStillSource*)pSource)->GetPlaylistPosition();
	}

	DrawBorders();

	// The input picture is replaced by the full multiple frames picture
	*bUseExtraBuffer = TRUE;
	*lpBuffer = lpMemoryBuffer;
	*Pitch = iWidth * 2;
	pInfo->FieldHeight = iHeight;
	pInfo->FrameHeight = iHeight;
	pInfo->FrameWidth = iWidth;
	pInfo->LineLength = iWidth * 2;
}

BOOL CMultiFrames::HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam)
{
    switch(LOWORD(wParam))
    {
    case IDM_VT_PAGE_UP:
		// Up key
		if (bNavigAllowed)
		{
			iDeltaNewFrame -= iNbCols;
			return TRUE;
		}
		break;
    case IDM_VT_PAGE_DOWN:
		// Down key
		if (bNavigAllowed)
		{
			iDeltaNewFrame += iNbCols;
			return TRUE;
		}
		break;
    case IDM_VT_PAGE_MINUS:
		// Left key
		if (bNavigAllowed)
		{
			iDeltaNewFrame--;
			return TRUE;
		}
		break;
    case IDM_VT_PAGE_PLUS:
		// Right key
		if (bNavigAllowed)
		{
			iDeltaNewFrame++;
			return TRUE;
		}
		break;
    case IDM_SHOW_OSD:
		// Return key
		RequestSwitch();
		return TRUE;
		break;
    default:
        break;
    }
    return FALSE;
}

void CMultiFrames::AllocateMemoryBuffer()
{
	if (lpMemoryBuffer == NULL)
	{
		lpMemoryBuffer = (BYTE*)malloc(iWidth * iHeight * 2 + 16);
		if (lpMemoryBuffer == NULL)
		{
		   LOG(1, "Couldn't create additional buffer for multiple frames output");
		}
	}
}

void CMultiFrames::FreeMemoryBuffer()
{
	if (lpMemoryBuffer != NULL)
	{
		free(lpMemoryBuffer);
		lpMemoryBuffer = NULL;
	}
}

void CMultiFrames::SelectFrameBuffer(int iFrame)
{
	int iRow = iFrame / iNbCols;
	int iCol = iFrame % iNbCols;

	iFramePitch = iWidth * 2;
	iFrameWidth = iWidth / iNbCols;
	iFrameHeight = iHeight / iNbRows;
	lpFrameBuffer = START_ALIGNED16(lpMemoryBuffer) + iRow * iFrameHeight * iFramePitch + iCol * iFrameWidth * 2;
}

void CMultiFrames::DrawBorders()
{
	BYTE* lpStartBuffer;

	for (int i=0 ; i < iNbFrames ; i++)
	{
		SelectFrameBuffer(i);

		for (int iPixel=0 ; iPixel < iFrameWidth ; iPixel++)
		{
			// Top border
			lpStartBuffer = lpFrameBuffer;
			lpStartBuffer[iPixel*2] = (i == iCurrentFrame) ? 235 : 128;
			lpStartBuffer[iPixel*2+1] = 128;
			lpStartBuffer[iFramePitch+iPixel*2] = (i == iCurrentFrame) ? 235 : 128;
			lpStartBuffer[iFramePitch+iPixel*2+1] = 128;

			// Bottom border
			lpStartBuffer = lpFrameBuffer + (iFrameHeight - 2) * iFramePitch;
			lpStartBuffer[iPixel*2] = (i == iCurrentFrame) ? 235 : 128;
			lpStartBuffer[iPixel*2+1] = 128;
			lpStartBuffer[iFramePitch+iPixel*2] = (i == iCurrentFrame) ? 235 : 128;
			lpStartBuffer[iFramePitch+iPixel*2+1] = 128;
		}

		for (int iLine=0 ; iLine < iFrameHeight ; iLine++)
		{
			// Left border
			lpStartBuffer = lpFrameBuffer + iLine * iFramePitch;
			lpStartBuffer[0] = (i == iCurrentFrame) ? 235 : 128;
			lpStartBuffer[1] = 128;
			lpStartBuffer[2] = (i == iCurrentFrame) ? 235 : 128;
			lpStartBuffer[3] = 128;

			// Right borders
			lpStartBuffer = lpFrameBuffer + iLine * iFramePitch + (iFrameWidth - 2) * 2;
			lpStartBuffer[0] = (i == iCurrentFrame) ? 235 : 128;
			lpStartBuffer[1] = 128;
			lpStartBuffer[2] = (i == iCurrentFrame) ? 235 : 128;
			lpStartBuffer[3] = 128;
		}
	}
}

void CMultiFrames::ShiftFrames(int iDeltaFrames)
{
	if ((iDeltaFrames > 0) && (iDeltaFrames < iNbFrames))
	{
		BYTE* lpStartBuffer = START_ALIGNED16(lpMemoryBuffer);
		int i;
		int iRow = iDeltaFrames / iNbCols;
		int iCol = iDeltaFrames % iNbCols;
		int iP = iWidth * 2;
		int iW = iWidth / iNbCols;
		int iH = iHeight / iNbRows;
		int iShift = iRow * iH * iP + iCol * iW * 2;
		memcpy(lpStartBuffer, lpStartBuffer + iShift, iHeight * iWidth * 2 - iShift);
		for (i=0 ; i<(iNbFrames-iDeltaFrames) ; i++)
		{
			bFrameFilled[i] = bFrameFilled[i+iDeltaFrames];
		}
		for (i=(iNbFrames-iDeltaFrames) ; i<iNbFrames ; i++)
		{
			bFrameFilled[i] = -1;
		}
		iCurrentFrame -= iDeltaFrames;
	}
}
