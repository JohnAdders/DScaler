/////////////////////////////////////////////////////////////////////////////
// $Id: MultiFrames.cpp,v 1.8 2003-03-23 09:24:27 laurentg Exp $
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
// Revision 1.7  2003/03/22 18:58:40  laurentg
// New key to switch to or from preview mode
// Spped up initial display of channels in preview mode
//
// Revision 1.6  2003/03/22 15:41:58  laurentg
// Half height deinterlace modes correctly handled in previow mode
// Center of the image in its frame with black borders
//
// Revision 1.5  2003/03/21 22:48:06  laurentg
// Preview mode (multiple frames) improved
//
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
#include "AspectRatio.h"
#include "ProgramList.h"
#include "DScaler.h"
#include "Deinterlace.h"


#define TOP_BORDER		2
#define BOTTOM_BORDER	2
#define LEFT_BORDER		2
#define RIGHT_BORDER	2

#define	LUMIN_CURRENT		235
#define	LUMIN_NOT_CURRENT	128
#define	LUMIN_BLACK			16


CMultiFrames* pMultiFrames = NULL;


CMultiFrames::CMultiFrames(eMultiFramesMode eMode, int iNbCols, int iNbRows, CSource* pSource)
{
	m_Mode = eMode;
	m_NbRows = iNbRows;
	m_NbCols = iNbCols;
	m_NbFrames = m_NbRows * m_NbCols;
	// Each frame must have a width multiple of 2
	int iWidth = DSCALER_MAX_WIDTH / iNbCols;
	iWidth &= 0xfffffffe;
	m_Width = iWidth * iNbCols;
	int iHeight = DSCALER_MAX_HEIGHT / iNbRows;
	m_Height = iHeight * iNbRows;
	m_Active = FALSE;
	bSwitchRequested = FALSE;
	m_MemoryBuffer = NULL;
	bFrameFilled = NULL;
	bNavigAllowed = FALSE;
	m_Source = pSource;
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

CSource* CMultiFrames::GetSource()
{
	return m_Source;
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
	FreeMemoryBuffer();
	if (bFrameFilled)
	{
		free(bFrameFilled);
		bFrameFilled = NULL;
	}
	m_Active = FALSE;
	bNavigAllowed = FALSE;
    UpdateSquarePixelsMode(m_Source->HasSquarePixels());
	m_Source->SetAspectRatioData();
	WorkoutOverlaySize(TRUE);
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

	for (int i=0 ; i < m_NbFrames ; i++)
	{
		ResetFrameToBlack(i);
		bFrameFilled[i] = -1;
	}
	m_CurrentFrame = 0;
	DrawBorder(m_CurrentFrame, TRUE, LUMIN_CURRENT, LEFT_BORDER, RIGHT_BORDER, TOP_BORDER, BOTTOM_BORDER);
	bNavigAllowed = FALSE;
}

void CMultiFrames::SelectFrame()
{
	int i;
	int iUnused;
	BOOL bBeforeCurrent;

	if (!m_Active || (bFrameFilled[m_CurrentFrame] == -1))
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
				DrawBorder(m_CurrentFrame, TRUE, LUMIN_NOT_CURRENT, LEFT_BORDER, RIGHT_BORDER, TOP_BORDER, BOTTOM_BORDER);
				m_CurrentFrame += iDeltaNewFrame;
				DrawBorder(m_CurrentFrame, TRUE, LUMIN_CURRENT, LEFT_BORDER, RIGHT_BORDER, TOP_BORDER, BOTTOM_BORDER);
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
				DrawBorder(m_CurrentFrame, TRUE, LUMIN_NOT_CURRENT, LEFT_BORDER, RIGHT_BORDER, TOP_BORDER, BOTTOM_BORDER);
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
		DrawBorder(m_CurrentFrame, TRUE, LUMIN_NOT_CURRENT, LEFT_BORDER, RIGHT_BORDER, TOP_BORDER, BOTTOM_BORDER);
		m_CurrentFrame = iUnused;
		DrawBorder(m_CurrentFrame, TRUE, LUMIN_CURRENT, LEFT_BORDER, RIGHT_BORDER, TOP_BORDER, BOTTOM_BORDER);
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

void CMultiFrames::UpdateFrame(TDeinterlaceInfo* pInfo, BOOL* bUseExtraBuffer, BYTE** lpBuffer, int* Pitch)
{
	BYTE* lpFrameBuffer;
	int iFrameWidth;
	int iFrameHeight;
	int iFramePitch;

	if (!m_Active)
	{
		*lpBuffer = NULL;
		*Pitch = 0;
		return;
	}

	// Retrieve the buffer corresponding to the frame in the global multiple frames buffer (without the borders)
	SelectFrameBuffer(m_CurrentFrame, FALSE, &lpFrameBuffer, &iFramePitch, &iFrameWidth, &iFrameHeight);

	// Copy (with resize) the input picture into its frame
    Overlay_Lock_Back_Buffer(pInfo, *bUseExtraBuffer);
	if (m_Source->HasSquarePixels())
	{
		// Keep the original ratio
		int iUpdWidth;
		int iUpdHeight;
		int iDelta;
		if ((pInfo->FrameWidth - iFrameWidth) > (pInfo->FrameHeight - iFrameHeight))
		{
			iUpdHeight = iFrameWidth * pInfo->FrameHeight / pInfo->FrameWidth;
			if (iUpdHeight > iFrameHeight)
			{
				iUpdHeight = iFrameHeight;
			}
			iDelta = iFrameHeight - iUpdHeight;

			// Add a black border at top and bottom if necessary
			DrawBorder(m_CurrentFrame, FALSE, LUMIN_BLACK, 0, 0, iDelta / 2, iFrameHeight - iUpdHeight - (iDelta / 2));

			// Center the picture in the frame
			lpFrameBuffer += (iDelta / 2) * iFramePitch;

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
			iDelta = iFrameWidth - iUpdWidth;

			// Add a black border at left and right if necessary
			DrawBorder(m_CurrentFrame, FALSE, LUMIN_BLACK, (iDelta / 4) * 2, iFrameWidth - iUpdWidth - ((iDelta / 4) * 2), 0, 0);

			// Center the picture in the frame
			lpFrameBuffer += (iDelta / 4) * 4;

			iFrameWidth = iUpdWidth;
		}
	}
	ResizeFrame(pInfo->Overlay, pInfo->OverlayPitch, pInfo->FrameWidth, InHalfHeightMode() ? pInfo->FieldHeight : pInfo->FrameHeight, lpFrameBuffer, iFramePitch, iFrameWidth, iFrameHeight);
    Overlay_Unlock_Back_Buffer(*bUseExtraBuffer);

	if (m_Mode == PREVIEW_CHANNELS)
	{
		bFrameFilled[m_CurrentFrame] = Setting_GetValue(Channels_GetSetting(CURRENTPROGRAM));
	}
	else if (m_Mode == PREVIEW_STILLS)
	{
		bFrameFilled[m_CurrentFrame] = ((CStillSource*)m_Source)->GetPlaylistPosition();
	}

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

void CMultiFrames::SelectFrameBuffer(int iFrame, BOOL bIncludingBorder, BYTE** lpFrameBuffer, int *iFramePitch, int *iFrameWidth, int *iFrameHeight)
{
	int iRow = iFrame / m_NbCols;
	int iCol = iFrame % m_NbCols;

	*iFramePitch = m_Width * 2;
	*iFrameHeight = m_Height / m_NbRows;
	*iFrameWidth = m_Width / m_NbCols;
	*lpFrameBuffer = START_ALIGNED16(m_MemoryBuffer) + iRow * *iFrameHeight * *iFramePitch + iCol * *iFrameWidth * 2;

	if (!bIncludingBorder)
	{
		*iFrameWidth -= LEFT_BORDER+RIGHT_BORDER;
		*iFrameHeight -= TOP_BORDER+BOTTOM_BORDER;
		*lpFrameBuffer += TOP_BORDER * *iFramePitch + LEFT_BORDER * 2;
	}
}

void CMultiFrames::DrawBorder(int iFrame, BOOL bIncludingExternalBorder, int iLuminLevel, unsigned int iLeftThick, unsigned int iRightThick, unsigned int iTopThick, unsigned int iBottomThick)
{
	BYTE* lpStartLineBuffer;
	BYTE* lpFrameBuffer;
	int iFrameWidth;
	int iFrameHeight;
	int iFramePitch;
	int iLine;
	int iPixel;

	SelectFrameBuffer(iFrame, bIncludingExternalBorder, &lpFrameBuffer, &iFramePitch, &iFrameWidth, &iFrameHeight);
	lpStartLineBuffer = lpFrameBuffer;

	for (iLine=0 ; iLine < iFrameHeight ; iLine++)
	{
		lpStartLineBuffer = lpFrameBuffer + iLine * iFramePitch;
		// Left border
		for (iPixel=0 ; iPixel < iLeftThick ; iPixel++)
		{
			lpStartLineBuffer[iPixel*2] = iLuminLevel;
			lpStartLineBuffer[iPixel*2+1] = 128;
		}
		// Right border
		for (iPixel=(iFrameWidth-1) ; iPixel>=(iFrameWidth-iRightThick) ; iPixel--)
		{
			lpStartLineBuffer[iPixel*2] = iLuminLevel;
			lpStartLineBuffer[iPixel*2+1] = 128;
		}
	}
	for (iPixel=0 ; iPixel < iFrameWidth ; iPixel++)
	{
		// Top border
		for (iLine=0 ; iLine < iTopThick ; iLine++)
		{
			lpStartLineBuffer = lpFrameBuffer + iLine * iFramePitch;
			lpStartLineBuffer[iPixel*2] = iLuminLevel;
			lpStartLineBuffer[iPixel*2+1] = 128;
		}
		// Bottom border
		for (iLine=(iFrameHeight-1) ; iLine>=(iFrameHeight-iBottomThick) ; iLine--)
		{
			lpStartLineBuffer = lpFrameBuffer + iLine * iFramePitch;
			lpStartLineBuffer[iPixel*2] = iLuminLevel;
			lpStartLineBuffer[iPixel*2+1] = 128;
		}
	}
}

// Add a border around each frame - boder size is 2 pixels
// Color is different for the current active frame and other frames
void CMultiFrames::DrawBorders()
{
	for (int i=0 ; i < m_NbFrames ; i++)
	{
		DrawBorder(i, TRUE, (i == m_CurrentFrame) ? LUMIN_CURRENT : LUMIN_NOT_CURRENT, LEFT_BORDER, RIGHT_BORDER, TOP_BORDER, BOTTOM_BORDER);
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

	SelectFrameBuffer(iFrameSrc, FALSE, &lpFrameSrcBuffer, &iFrameSrcPitch, &iFrameSrcWidth, &iFrameSrcHeight);
	SelectFrameBuffer(iFrameDest, FALSE, &lpFrameDestBuffer, &iFrameDestPitch, &iFrameDestWidth, &iFrameDestHeight);

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
	DrawBorder(iFrame, TRUE, LUMIN_BLACK, m_Width / m_NbCols, 0, 0, 0);
}
