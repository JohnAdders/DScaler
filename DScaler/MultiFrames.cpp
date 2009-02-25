/////////////////////////////////////////////////////////////////////////////
// $Id$
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
// Revision 1.15  2007/02/19 10:13:45  adcockj
// Fixes for Critical thread and RECT issuesin D3D9 and overlay code
//
// Revision 1.14  2006/12/20 07:45:07  adcockj
// added DirectX code from Daniel Sabel
//
// Revision 1.13  2005/03/23 14:20:57  adcockj
// Test fix for threading issues
//
// Revision 1.12  2003/10/27 10:39:52  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.11  2003/08/02 12:48:25  laurentg
// Ctrl+Up and Ctrl+Down to go to the previous and next pages when in preview mode
//
// Revision 1.10  2003/06/15 08:26:36  laurentg
// Update all the frames having the same content
//
// Revision 1.9  2003/06/14 19:35:56  laurentg
// Preview mode improved
//
// Revision 1.8  2003/03/23 09:24:27  laurentg
// Automatic leave preview mode when necessary
//
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


/**
 * @file MultiFrames.cpp MultiFrames Functions
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "MultiFrames.h"
#include "IOutput.h"
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
	m_SwitchRequested = FALSE;
	m_MemoryBuffer = NULL;
	m_FrameFilled = NULL;
	m_NavigAllowed = FALSE;
	m_Source = pSource;
}

CMultiFrames::~CMultiFrames()
{
	FreeMemoryBuffer();
	if (m_FrameFilled)
	{
		free(m_FrameFilled);
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

// Inform if the multi frames output is engaged
BOOL CMultiFrames::IsActive()
{
	return m_Active;
}

// Enable the multi frames output
void CMultiFrames::Enable()
{
	AllocateMemoryBuffer();
	if (m_FrameFilled == NULL)
	{
		m_FrameFilled = (int*) malloc(m_NbFrames * sizeof(int));
		if (m_FrameFilled == NULL)
		{
		   LOG(1, "Couldn't allocate memory for multi frames output");
		}
	}
	m_Active = m_MemoryBuffer && m_FrameFilled;
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

// Disable the multi frames output
void CMultiFrames::Disable()
{
	FreeMemoryBuffer();
	if (m_FrameFilled)
	{
		free(m_FrameFilled);
		m_FrameFilled = NULL;
	}
	m_Active = FALSE;
	m_NavigAllowed = FALSE;
    UpdateSquarePixelsMode(m_Source->HasSquarePixels());
	m_Source->SetAspectRatioData();
	WorkoutOverlaySize(TRUE);
}

BOOL CMultiFrames::IsSwitchRequested()
{
	return m_SwitchRequested;
}

void CMultiFrames::RequestSwitch()
{
	m_SwitchRequested = TRUE;
}

void CMultiFrames::HandleSwitch()
{
	m_SwitchRequested = FALSE;
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
		m_FrameFilled[i] = -1;
	}
	m_CurrentFrame = 0;
	DrawBorder(m_CurrentFrame, TRUE, LUMIN_CURRENT, LEFT_BORDER, RIGHT_BORDER, TOP_BORDER, BOTTOM_BORDER);
	m_NavigAllowed = FALSE;
	m_ContentChanged = TRUE;
}

void CMultiFrames::SelectFrame()
{
	int i;
	int iUnused;
	BOOL bBeforeCurrent;

	if (!m_Active || (m_FrameFilled[m_CurrentFrame] == -1))
	{
		m_NavigAllowed = FALSE;
		return;
	}

	// Here, weknow that the current frame is filled
	// We must search if another one must be filled

	iUnused = -1;

	// Check if there is a frame to fill before the current one
	for (i=(m_CurrentFrame-1) ; i>=0 ; i--)
	{
		if (m_FrameFilled[i] == -1)
		{
			iUnused = i;
			bBeforeCurrent = TRUE;
			m_NavigAllowed = FALSE;
			break;
		}
	}
	if (iUnused == -1)
	{
		// Check if there is a frame to fill after the current one
		for (i=(m_CurrentFrame+1) ; i<m_NbFrames ; i++)
		{
			if (m_FrameFilled[i] == -1)
			{
				iUnused = i;
				bBeforeCurrent = FALSE;
				m_NavigAllowed = FALSE;
				break;
			}
		}
	}

	if (iUnused == -1)
	{
		// All the frames are filled, so we must enable the user navigation
		if (!m_NavigAllowed)
		{
			m_DeltaNewFrame = 0;
			m_PreviousPage = FALSE;
			m_NextPage = FALSE;
		}
		m_NavigAllowed = TRUE;
	}

	if (m_NavigAllowed)
	{
		//
		// There is no frame to fill
		// So check if user asked for move to another frame
		// or to fill the screen with other channels
		//

		if (m_PreviousPage == TRUE)
		{
			m_NavigAllowed = FALSE;
			m_ContentChanged = FALSE;
			for (int i=0 ; i < m_NbFrames ; i++)
			{
				m_FrameFilled[i] = -1;
			}
			DrawBorder(m_CurrentFrame, TRUE, LUMIN_NOT_CURRENT, LEFT_BORDER, RIGHT_BORDER, TOP_BORDER, BOTTOM_BORDER);
			m_CurrentFrame = m_NbFrames-1;
			DrawBorder(m_CurrentFrame, TRUE, LUMIN_CURRENT, LEFT_BORDER, RIGHT_BORDER, TOP_BORDER, BOTTOM_BORDER);
			if (m_Mode == PREVIEW_CHANNELS)
			{
				SendMessage(GetMainWnd(), WM_COMMAND, IDM_CHANNELMINUS, 0);
			}
			else if (m_Mode == PREVIEW_STILLS)
			{
				SendMessage(GetMainWnd(), WM_COMMAND, IDM_PLAYLIST_PREVIOUS_CIRC, 0);
			}
		}
		else if (m_NextPage == TRUE)
		{
			m_NavigAllowed = FALSE;
			m_ContentChanged = FALSE;
			for (int i=0 ; i < m_NbFrames ; i++)
			{
				m_FrameFilled[i] = -1;
			}
			DrawBorder(m_CurrentFrame, TRUE, LUMIN_NOT_CURRENT, LEFT_BORDER, RIGHT_BORDER, TOP_BORDER, BOTTOM_BORDER);
			m_CurrentFrame = 0;
			DrawBorder(m_CurrentFrame, TRUE, LUMIN_CURRENT, LEFT_BORDER, RIGHT_BORDER, TOP_BORDER, BOTTOM_BORDER);
			if (m_Mode == PREVIEW_CHANNELS)
			{
				SendMessage(GetMainWnd(), WM_COMMAND, IDM_CHANNELPLUS, 0);
			}
			else if (m_Mode == PREVIEW_STILLS)
			{
				SendMessage(GetMainWnd(), WM_COMMAND, IDM_PLAYLIST_NEXT_CIRC, 0);
			}
		}
		else if (m_DeltaNewFrame != 0)
		{
			m_NavigAllowed = FALSE;
			m_ContentChanged = FALSE;

			if ( ((m_CurrentFrame + m_DeltaNewFrame) >= 0)
			  && ((m_CurrentFrame + m_DeltaNewFrame) < m_NbFrames) )
			{
				// There is no shift of frames to do

				DrawBorder(m_CurrentFrame, TRUE, LUMIN_NOT_CURRENT, LEFT_BORDER, RIGHT_BORDER, TOP_BORDER, BOTTOM_BORDER);
				m_CurrentFrame += m_DeltaNewFrame;
				DrawBorder(m_CurrentFrame, TRUE, LUMIN_CURRENT, LEFT_BORDER, RIGHT_BORDER, TOP_BORDER, BOTTOM_BORDER);
				if (m_Mode == PREVIEW_CHANNELS)
				{
					SendMessage(GetMainWnd(), WM_COMMAND, IDM_CHANNEL_INDEX, m_FrameFilled[m_CurrentFrame]);
				}
				else if (m_Mode == PREVIEW_STILLS)
				{
					SendMessage(GetMainWnd(), WM_COMMAND, IDM_PLAYLIST_INDEX, m_FrameFilled[m_CurrentFrame]);
				}
				m_FrameFilled[m_CurrentFrame] = -1;
			}
			else
			{
				// There is a shift of frames to do

				DrawBorder(m_CurrentFrame, TRUE, LUMIN_NOT_CURRENT, LEFT_BORDER, RIGHT_BORDER, TOP_BORDER, BOTTOM_BORDER);
				ShiftFrames(m_DeltaNewFrame);
				DrawBorder(m_CurrentFrame, TRUE, LUMIN_CURRENT, LEFT_BORDER, RIGHT_BORDER, TOP_BORDER, BOTTOM_BORDER);
				if (m_Mode == PREVIEW_CHANNELS)
				{
					SendMessage(GetMainWnd(), WM_COMMAND, IDM_CHANNEL_INDEX, m_FrameFilled[m_CurrentFrame]);
				}
				else if (m_Mode == PREVIEW_STILLS)
				{
					SendMessage(GetMainWnd(), WM_COMMAND, IDM_PLAYLIST_INDEX, m_FrameFilled[m_CurrentFrame]);
				}
			}
		}
	}
	else
	{
		//
		// There is one frame to fill
		// iUnused is the index of the frame to fill
		//

		m_ContentChanged = FALSE;

		DrawBorder(m_CurrentFrame, TRUE, LUMIN_NOT_CURRENT, LEFT_BORDER, RIGHT_BORDER, TOP_BORDER, BOTTOM_BORDER);
		m_CurrentFrame = iUnused;
		DrawBorder(m_CurrentFrame, TRUE, LUMIN_CURRENT, LEFT_BORDER, RIGHT_BORDER, TOP_BORDER, BOTTOM_BORDER);

		if (bBeforeCurrent)
		{
			if (m_Mode == PREVIEW_CHANNELS)
			{
				SendMessage(GetMainWnd(), WM_COMMAND, IDM_CHANNELMINUS, 0);
			}
			else if (m_Mode == PREVIEW_STILLS)
			{
				SendMessage(GetMainWnd(), WM_COMMAND, IDM_PLAYLIST_PREVIOUS_CIRC, 0);
			}
		}
		else
		{
			if (m_Mode == PREVIEW_CHANNELS)
			{
				SendMessage(GetMainWnd(), WM_COMMAND, IDM_CHANNELPLUS, 0);
			}
			else if (m_Mode == PREVIEW_STILLS)
			{
				SendMessage(GetMainWnd(), WM_COMMAND, IDM_PLAYLIST_NEXT_CIRC, 0);
			}
		}
	}
}

void CMultiFrames::UpdateFrame(TDeinterlaceInfo* pInfo, BOOL* bUseExtraBuffer, BYTE** lpBuffer, int* Pitch)
{
	BYTE* lpFrameBuffer;
	int iFrameWidth;
	int iFrameHeight;
	int iFrameLinePitch;

	if (!m_Active)
	{
		*lpBuffer = NULL;
		*Pitch = 0;
		return;
	}

	// Retrieve the buffer corresponding to the frame in the global multiple frames buffer (without the borders)
	SelectFrameBuffer(m_CurrentFrame, FALSE, &lpFrameBuffer, &iFrameLinePitch, &iFrameWidth, &iFrameHeight);

	// Copy (with resize) the input picture into its frame
    if(!GetActiveOutput()->Overlay_Lock_Back_Buffer(pInfo, *bUseExtraBuffer))
    {
        return;
    }

    // we need to ensure that the back buffer is always unlocked

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
			lpFrameBuffer += (iDelta / 2) * iFrameLinePitch;

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
	ResizeFrame(pInfo->Overlay, pInfo->OverlayPitch, pInfo->FrameWidth, InHalfHeightMode() ? pInfo->FieldHeight : pInfo->FrameHeight, lpFrameBuffer, iFrameLinePitch, iFrameWidth, iFrameHeight);
    
    // there were no exit paths between the unlock and the lock
    // so we hsould be OK
    GetActiveOutput()->Overlay_Unlock_Back_Buffer(*bUseExtraBuffer);

	if (m_ContentChanged == TRUE)
	{
		if (m_Mode == PREVIEW_CHANNELS)
		{
			m_FrameFilled[m_CurrentFrame] = Setting_GetValue(Channels_GetSetting(CURRENTPROGRAM));
		}
		else if (m_Mode == PREVIEW_STILLS)
		{
			m_FrameFilled[m_CurrentFrame] = ((CStillSource*)m_Source)->GetPlaylistPosition();
		}
	}

	if (m_FrameFilled[m_CurrentFrame] != -1)
	{
		// Update the other frames having the same content
		for (int i=0 ; i < m_NbFrames ; i++)
		{
			if ( (i != m_CurrentFrame)
			  && (m_FrameFilled[i] == m_FrameFilled[m_CurrentFrame]) )
			{
				CopyFrame(m_CurrentFrame, i);
			}
		}
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

// User commands handling
BOOL CMultiFrames::HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam)
{
    switch(LOWORD(wParam))
    {
    case IDM_PREVIEW_PAGE_PREV:
		// Ctrl+Up key
		if (m_NavigAllowed)
		{
			m_PreviousPage = TRUE;
			return TRUE;
		}
		break;
    case IDM_PREVIEW_PAGE_NEXT:
		// Ctrl+Down key
		if (m_NavigAllowed)
		{
			m_NextPage = TRUE;
			return TRUE;
		}
		break;
    case IDM_VT_PAGE_UP:
		// Up key
		if (m_NavigAllowed)
		{
			m_DeltaNewFrame -= m_NbCols;
			return TRUE;
		}
		break;
    case IDM_VT_PAGE_DOWN:
		// Down key
		if (m_NavigAllowed)
		{
			m_DeltaNewFrame += m_NbCols;
			return TRUE;
		}
		break;
    case IDM_VT_PAGE_MINUS:
		// Left key
		if (m_NavigAllowed)
		{
			m_DeltaNewFrame--;
			return TRUE;
		}
		break;
    case IDM_VT_PAGE_PLUS:
		// Right key
		if (m_NavigAllowed)
		{
			m_DeltaNewFrame++;
			return TRUE;
		}
		break;
    default:
        break;
    }
    return FALSE;
}

void CMultiFrames::AckContentChange()
{
	m_ContentChanged = TRUE;
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

// Get the information relative to a frame including :
// - the pointer to the corresponding memory buffer
// - its width and height
// - its line pitch
void CMultiFrames::SelectFrameBuffer(int iFrame, BOOL bIncludingBorder, BYTE** lpFrameBuffer, int *iFrameLinePitch, int *iFrameWidth, int *iFrameHeight)
{
	int iRow = iFrame / m_NbCols;
	int iCol = iFrame % m_NbCols;

	*iFrameLinePitch = m_Width * 2;
	*iFrameHeight = m_Height / m_NbRows;
	*iFrameWidth = m_Width / m_NbCols;
	*lpFrameBuffer = START_ALIGNED16(m_MemoryBuffer) + iRow * *iFrameHeight * *iFrameLinePitch + iCol * *iFrameWidth * 2;

	if (!bIncludingBorder)
	{
		*iFrameWidth -= LEFT_BORDER+RIGHT_BORDER;
		*iFrameHeight -= TOP_BORDER+BOTTOM_BORDER;
		*lpFrameBuffer += TOP_BORDER * *iFrameLinePitch + LEFT_BORDER * 2;
	}
}

// Draw a border around one frame
// Color and size of each border are gien as parameters
void CMultiFrames::DrawBorder(int iFrame, BOOL bIncludingExternalBorder, int iLuminLevel, unsigned int iLeftThick, unsigned int iRightThick, unsigned int iTopThick, unsigned int iBottomThick)
{
	BYTE* lpStartLineBuffer;
	BYTE* lpFrameBuffer;
	int iFrameWidth;
	int iFrameHeight;
	int iFrameLinePitch;
	int iLine;
	int iPixel;

	SelectFrameBuffer(iFrame, bIncludingExternalBorder, &lpFrameBuffer, &iFrameLinePitch, &iFrameWidth, &iFrameHeight);

	for (iLine=0 ; iLine < iFrameHeight ; iLine++)
	{
		lpStartLineBuffer = lpFrameBuffer + iLine * iFrameLinePitch;
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
			lpStartLineBuffer = lpFrameBuffer + iLine * iFrameLinePitch;
			lpStartLineBuffer[iPixel*2] = iLuminLevel;
			lpStartLineBuffer[iPixel*2+1] = 128;
		}
		// Bottom border
		for (iLine=(iFrameHeight-1) ; iLine>=(iFrameHeight-iBottomThick) ; iLine--)
		{
			lpStartLineBuffer = lpFrameBuffer + iLine * iFrameLinePitch;
			lpStartLineBuffer[iPixel*2] = iLuminLevel;
			lpStartLineBuffer[iPixel*2+1] = 128;
		}
	}
}

// Draw a border around each frame - boder size is 2 pixels
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
			CopyFrame(i+iDeltaFrames, i);
			m_FrameFilled[i] = m_FrameFilled[i+iDeltaFrames];
		}
		for (i=(m_NbFrames-iDeltaFrames) ; i<m_NbFrames ; i++)
		{
			ResetFrameToBlack(i);
			m_FrameFilled[i] = -1;
		}
		m_CurrentFrame = m_NbFrames - 1 - iDeltaFrames;
	}
	else if ((iDeltaFrames < 0) && (-iDeltaFrames < m_NbFrames))
	{
		iDeltaFrames *= -1;
		for (i=(m_NbFrames-1) ; i>=iDeltaFrames ; i--)
		{
			CopyFrame(i-iDeltaFrames, i);
			m_FrameFilled[i] = m_FrameFilled[i-iDeltaFrames];
		}
		for (i=(iDeltaFrames-1) ; i>=0 ; i--)
		{
			ResetFrameToBlack(i);
			m_FrameFilled[i] = -1;
		}
		m_CurrentFrame = iDeltaFrames;
	}
}

// Copy the content of one frame to another one
void CMultiFrames::CopyFrame(int iFrameSrc, int iFrameDest)
{
	BYTE* lpFrameSrcBuffer;
	int iFrameSrcWidth;
	int iFrameSrcHeight;
	int iFrameSrcLinePitch;
	BYTE* lpFrameDestBuffer;
	int iFrameDestWidth;
	int iFrameDestHeight;
	int iFrameDestLinePitch;

	SelectFrameBuffer(iFrameSrc, FALSE, &lpFrameSrcBuffer, &iFrameSrcLinePitch, &iFrameSrcWidth, &iFrameSrcHeight);
	SelectFrameBuffer(iFrameDest, FALSE, &lpFrameDestBuffer, &iFrameDestLinePitch, &iFrameDestWidth, &iFrameDestHeight);

	// The source and destination frames must have the same size
	if (iFrameSrcWidth == iFrameDestWidth && iFrameSrcHeight == iFrameDestHeight)
	{
		for (int iLine=0 ; iLine < iFrameSrcHeight ; iLine++)
		{
			memcpy(lpFrameDestBuffer, lpFrameSrcBuffer, iFrameSrcWidth*2);
			lpFrameSrcBuffer += iFrameSrcLinePitch;
			lpFrameDestBuffer += iFrameDestLinePitch;
		}
	}
}

// Paint in black the content of a frame
void CMultiFrames::ResetFrameToBlack(int iFrame)
{
	DrawBorder(iFrame, TRUE, LUMIN_BLACK, m_Width / m_NbCols, 0, 0, 0);
}
