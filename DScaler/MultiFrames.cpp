/////////////////////////////////////////////////////////////////////////////
// $Id: MultiFrames.cpp,v 1.5 2003-03-21 22:48:06 laurentg Exp $
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
// Revision 1.4  2003/03/20 23:27:28  laurentg
// Navigation through frames updated - bugs fixed - few comments added
//
// Revision 1.3  2003/03/19 23:55:19  laurentg
// First step to add stills preview mode
// Second step for the navigation through channels in preview mode
//
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


CMultiFrames* pMultiFrames = NULL;


CMultiFrames::CMultiFrames(eMultiFramesMode eMode, int iNbCols, int iNbRows, int iDuration)
{
	m_Mode = eMode;
	m_NbRows = iNbRows;
	m_NbCols = iNbCols;
	m_NbFrames = m_NbRows * m_NbCols;
	m_Width = DSCALER_MAX_WIDTH;
	m_Height = DSCALER_MAX_HEIGHT;
	m_Active = FALSE;
	bSwitchRequested = FALSE;
	m_MemoryBuffer = NULL;
	bFrameFilled = NULL;
	iDeltaTicksChange = iDuration;
	bNavigAllowed = FALSE;
}

CMultiFrames::~CMultiFrames()
{
	FreeMemoryBuffer();
	if (bFrameFilled)
	{
		free(bFrameFilled);
	}
}

eMultiFramesMode CMultiFrames::GetMode()
{
	return m_Mode;
}

int CMultiFrames::GetWidth()
{
	return m_Width;
}

int CMultiFrames::GetHeight()
{
	return m_Height;
}

BOOL CMultiFrames::IsActive()
{
	return m_Active;
}

void CMultiFrames::Enable()
{
	AllocateMemoryBuffer();
	bFrameFilled = (int*) malloc(m_NbFrames * sizeof(BOOL));
	m_Active = m_MemoryBuffer && bFrameFilled;
	if (m_Active)
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
	if (bFrameFilled)
	{
		free(bFrameFilled);
		bFrameFilled = NULL;
	}
	m_Active = FALSE;
	bNavigAllowed = FALSE;
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
	if (!m_Active)
	{
		return;
	}

	m_CurrentFrame = 0;
	LastTickCount = 0;
	for (int i=0 ; i < m_NbFrames ; i++)
	{
		ResetFrameToBlack(i);
		bFrameFilled[i] = -1;
	}
	bNavigAllowed = FALSE;
}

void CMultiFrames::SelectFrame()
{
	int i;
	int iUnused;
	BOOL bBeforeCurrent;

	if (!m_Active)
	{
		return;
	}

	DWORD CurrentTickCount = GetTickCount();
	if (LastTickCount == 0)
	{
		LastTickCount = CurrentTickCount;
	}

	if (bFrameFilled[m_CurrentFrame] == -1)
	{
		bNavigAllowed = FALSE;
		return;
	}

	iUnused = -1;
	for (i=(m_CurrentFrame-1) ; i>=0 ; i--)
	{
		if (bFrameFilled[i] == -1)
		{
			iUnused = i;
			bBeforeCurrent = TRUE;
			bNavigAllowed = FALSE;
			break;
		}
	}
	if (iUnused == -1)
	{
		for (i=(m_CurrentFrame+1) ; i<m_NbFrames ; i++)
		{
			if (bFrameFilled[i] == -1)
			{
				iUnused = i;
				bBeforeCurrent = FALSE;
				bNavigAllowed = FALSE;
				break;
			}
		}
	}
	if (iUnused == -1)
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
			if ( ((m_CurrentFrame + iDeltaNewFrame) >= 0)
			  && ((m_CurrentFrame + iDeltaNewFrame) < m_NbFrames) )
			{
				m_CurrentFrame += iDeltaNewFrame;
				if (m_Mode == PREVIEW_CHANNELS)
				{
					SendMessage(hWnd, WM_COMMAND, IDM_CHANNEL_INDEX, bFrameFilled[m_CurrentFrame]);
				}
				else if (m_Mode == PREVIEW_STILLS)
				{
					SendMessage(hWnd, WM_COMMAND, IDM_PLAYLIST_INDEX, bFrameFilled[m_CurrentFrame]);
				}
				bFrameFilled[m_CurrentFrame] = -1;
				bNavigAllowed = FALSE;
			}
			else
			{
				ShiftFrames(iDeltaNewFrame);
				if (m_Mode == PREVIEW_CHANNELS)
				{
					SendMessage(hWnd, WM_COMMAND, IDM_CHANNEL_INDEX, bFrameFilled[m_CurrentFrame]);
				}
				else if (m_Mode == PREVIEW_STILLS)
				{
					SendMessage(hWnd, WM_COMMAND, IDM_PLAYLIST_INDEX, bFrameFilled[m_CurrentFrame]);
				}
				bNavigAllowed = FALSE;
			}
		}
	}
	else
	{
		if ((CurrentTickCount - LastTickCount) >= iDeltaTicksChange)
		{
			LastTickCount = CurrentTickCount;
			m_CurrentFrame = iUnused;
			if (bBeforeCurrent)
			{
				if (m_Mode == PREVIEW_CHANNELS)
				{
					SendMessage(hWnd, WM_COMMAND, IDM_CHANNELMINUS, 0);
				}
				else if (m_Mode == PREVIEW_STILLS)
				{
					SendMessage(hWnd, WM_COMMAND, IDM_PLAYLIST_PREVIOUS_CIRC, 0);
				}
			}
			else
			{
				if (m_Mode == PREVIEW_CHANNELS)
				{
					SendMessage(hWnd, WM_COMMAND, IDM_CHANNELPLUS, 0);
				}
				else if (m_Mode == PREVIEW_STILLS)
				{
					SendMessage(hWnd, WM_COMMAND, IDM_PLAYLIST_NEXT_CIRC, 0);
				}
			}
		}
	}
}

void CMultiFrames::UpdateFrame(TDeinterlaceInfo* pInfo, BOOL* bUseExtraBuffer, BYTE** lpBuffer, int* Pitch)
{
	CSource* pSource = Providers_GetCurrentSource();
	BYTE* lpFrameBuffer;
	int iFrameWidth;
	int iFrameHeight;
	int iFramePitch;

	if (!m_Active || !pSource)
	{
		*lpBuffer = NULL;
		*Pitch = 0;
		return;
	}

	// Retrieve the buffer corresponding to the frame in the global multiple frames buffer
	SelectFrameBuffer(m_CurrentFrame, &lpFrameBuffer, &iFramePitch, &iFrameWidth, &iFrameHeight);

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
			iUpdWidth &= 0xfffffffe;
			if (iUpdWidth > iFrameWidth)
			{
				iUpdWidth = iFrameWidth;
			}
			lpFrameBuffer += ( (iFrameWidth - iUpdWidth) / 4 ) * 4;
			iFrameWidth = iUpdWidth;
		}
	}
	ResizeFrame(pInfo->Overlay, pInfo->OverlayPitch, pInfo->FrameWidth, pInfo->FrameHeight, lpFrameBuffer, iFramePitch, iFrameWidth, iFrameHeight);
    Overlay_Unlock_Back_Buffer(*bUseExtraBuffer);

	if (m_Mode == PREVIEW_CHANNELS)
	{
		bFrameFilled[m_CurrentFrame] = Setting_GetValue(Channels_GetSetting(CURRENTPROGRAM));
	}
	else if (m_Mode == PREVIEW_STILLS)
	{
		bFrameFilled[m_CurrentFrame] = ((CStillSource*)pSource)->GetPlaylistPosition();
	}

	DrawBorders();

	// The input picture is replaced by the full multiple frames picture
	*bUseExtraBuffer = TRUE;
	*lpBuffer = m_MemoryBuffer;
	*Pitch = m_Width * 2;
	pInfo->FieldHeight = m_Height;
	pInfo->FrameHeight = m_Height;
	pInfo->FrameWidth = m_Width;
	pInfo->LineLength = m_Width * 2;
}

BOOL CMultiFrames::HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam)
{
    switch(LOWORD(wParam))
    {
    case IDM_VT_PAGE_UP:
		// Up key
		if (bNavigAllowed)
		{
			iDeltaNewFrame -= m_NbCols;
			return TRUE;
		}
		break;
    case IDM_VT_PAGE_DOWN:
		// Down key
		if (bNavigAllowed)
		{
			iDeltaNewFrame += m_NbCols;
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

// Allocate memory buffer to store the picture containing all the frames
void CMultiFrames::AllocateMemoryBuffer()
{
	if (m_MemoryBuffer == NULL)
	{
		m_MemoryBuffer = (BYTE*)malloc(m_Width * m_Height * 2 + 16);
		if (m_MemoryBuffer == NULL)
		{
		   LOG(1, "Couldn't create additional buffer for multiple frames output");
		}
	}
}

// Free previously allocated memory buffer to store the picture containing all the frames
void CMultiFrames::FreeMemoryBuffer()
{
	if (m_MemoryBuffer != NULL)
	{
		free(m_MemoryBuffer);
		m_MemoryBuffer = NULL;
	}
}

void CMultiFrames::SelectFrameBuffer(int iFrame, BYTE** lpFrameBuffer, int *iFramePitch, int *iFrameWidth, int *iFrameHeight)
{
	int iRow = iFrame / m_NbCols;
	int iCol = iFrame % m_NbCols;

	*iFramePitch = m_Width * 2;
	*iFrameWidth = m_Width / m_NbCols;
	*iFrameHeight = m_Height / m_NbRows;
	*lpFrameBuffer = START_ALIGNED16(m_MemoryBuffer) + iRow * *iFrameHeight * *iFramePitch + iCol * *iFrameWidth * 2;
}

// Add a border around each frame - boder size is 2 pixels
// Color is different for the current active frame and other frames
void CMultiFrames::DrawBorders()
{
	BYTE* lpStartBuffer;
	BYTE* lpFrameBuffer;
	int iFrameWidth;
	int iFrameHeight;
	int iFramePitch;

	for (int i=0 ; i < m_NbFrames ; i++)
	{
		SelectFrameBuffer(i, &lpFrameBuffer, &iFramePitch, &iFrameWidth, &iFrameHeight);

		for (int iPixel=0 ; iPixel < iFrameWidth ; iPixel++)
		{
			// Top border
			lpStartBuffer = lpFrameBuffer;
			lpStartBuffer[iPixel*2] = (i == m_CurrentFrame) ? 235 : 128;
			lpStartBuffer[iPixel*2+1] = 128;
			lpStartBuffer[iFramePitch+iPixel*2] = (i == m_CurrentFrame) ? 235 : 128;
			lpStartBuffer[iFramePitch+iPixel*2+1] = 128;

			// Bottom border
			lpStartBuffer = lpFrameBuffer + (iFrameHeight - 2) * iFramePitch;
			lpStartBuffer[iPixel*2] = (i == m_CurrentFrame) ? 235 : 128;
			lpStartBuffer[iPixel*2+1] = 128;
			lpStartBuffer[iFramePitch+iPixel*2] = (i == m_CurrentFrame) ? 235 : 128;
			lpStartBuffer[iFramePitch+iPixel*2+1] = 128;
		}

		for (int iLine=0 ; iLine < iFrameHeight ; iLine++)
		{
			// Left border
			lpStartBuffer = lpFrameBuffer + iLine * iFramePitch;
			lpStartBuffer[0] = (i == m_CurrentFrame) ? 235 : 128;
			lpStartBuffer[1] = 128;
			lpStartBuffer[2] = (i == m_CurrentFrame) ? 235 : 128;
			lpStartBuffer[3] = 128;

			// Right borders
			lpStartBuffer = lpFrameBuffer + iLine * iFramePitch + (iFrameWidth - 2) * 2;
			lpStartBuffer[0] = (i == m_CurrentFrame) ? 235 : 128;
			lpStartBuffer[1] = 128;
			lpStartBuffer[2] = (i == m_CurrentFrame) ? 235 : 128;
			lpStartBuffer[3] = 128;
		}
	}
}

// Shift all frames in the grid
//
// A positive value for iDeltaFrames means that some frames will
// disappear at top left and new ones will appeared at bottom right
// A negative value for iDeltaFrames means that some frames will
// disappear at bottom right and new ones will appeared at top left
void CMultiFrames::ShiftFrames(int iDeltaFrames)
{
	int i;

	if ((iDeltaFrames > 0) && (iDeltaFrames < m_NbFrames))
	{
		for (i=0 ; i<(m_NbFrames-iDeltaFrames) ; i++)
		{
			MoveFrame(i+iDeltaFrames, i);
			bFrameFilled[i] = bFrameFilled[i+iDeltaFrames];
		}
		for (i=(m_NbFrames-iDeltaFrames) ; i<m_NbFrames ; i++)
		{
			ResetFrameToBlack(i);
			bFrameFilled[i] = -1;
		}
		m_CurrentFrame = m_NbFrames - 1 - iDeltaFrames;
	}
	else if ((iDeltaFrames < 0) && (-iDeltaFrames < m_NbFrames))
	{
		iDeltaFrames *= -1;
		for (i=(m_NbFrames-1) ; i>=iDeltaFrames ; i--)
		{
			MoveFrame(i-iDeltaFrames, i);
			bFrameFilled[i] = bFrameFilled[i-iDeltaFrames];
		}
		for (i=(iDeltaFrames-1) ; i>=0 ; i--)
		{
			ResetFrameToBlack(i);
			bFrameFilled[i] = -1;
		}
		m_CurrentFrame = iDeltaFrames;
	}
}

// Move the content of one frame from one position to another one
void CMultiFrames::MoveFrame(int iFrameSrc, int iFrameDest)
{
	BYTE* lpFrameSrcBuffer;
	int iFrameSrcWidth;
	int iFrameSrcHeight;
	int iFrameSrcPitch;
	BYTE* lpFrameDestBuffer;
	int iFrameDestWidth;
	int iFrameDestHeight;
	int iFrameDestPitch;

	SelectFrameBuffer(iFrameSrc, &lpFrameSrcBuffer, &iFrameSrcPitch, &iFrameSrcWidth, &iFrameSrcHeight);
	SelectFrameBuffer(iFrameDest, &lpFrameDestBuffer, &iFrameDestPitch, &iFrameDestWidth, &iFrameDestHeight);

	// The source and destination frames must have the same size
	if (iFrameSrcWidth == iFrameDestWidth && iFrameSrcHeight == iFrameDestHeight)
	{
		for (int iLine=0 ; iLine < iFrameSrcHeight ; iLine++)
		{
			for (int iPixel=0 ; iPixel < iFrameSrcWidth ; iPixel++)
			{
				lpFrameDestBuffer[iPixel*2] = lpFrameSrcBuffer[iPixel*2];
				lpFrameDestBuffer[iPixel*2+1] = lpFrameSrcBuffer[iPixel*2+1];
			}
			lpFrameSrcBuffer += iFrameSrcPitch;
			lpFrameDestBuffer += iFrameDestPitch;
		}
	}
}

// Paint in black the content of a frame
void CMultiFrames::ResetFrameToBlack(int iFrame)
{
	BYTE* lpFrameBuffer;
	int iFrameWidth;
	int iFrameHeight;
	int iFramePitch;

	SelectFrameBuffer(iFrame, &lpFrameBuffer, &iFramePitch, &iFrameWidth, &iFrameHeight);

	for (int iLine=0 ; iLine < iFrameHeight ; iLine++)
	{
		for (int iPixel=0 ; iPixel < iFrameWidth ; iPixel++)
		{
			lpFrameBuffer[iPixel*2] = 16;
			lpFrameBuffer[iPixel*2+1] = 128;
		}
		lpFrameBuffer += iFramePitch;
	}
}
