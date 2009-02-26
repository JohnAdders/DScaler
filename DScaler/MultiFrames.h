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

/** 
 * @file multiframes.h multiframes Header file
 */
 
#ifndef __MULTIFRAMES_H___
#define __MULTIFRAMES_H___


#include "Source.h"


// Different possible modes
enum eMultiFramesMode {
    PREVIEW_CHANNELS = 0,
    PREVIEW_STILLS,
};


#include "DS_ApiCommon.h"


class CMultiFrames
{
public:
    CMultiFrames(eMultiFramesMode eMode, int iNbCols, int iNbRows, CSource* pSource);
    ~CMultiFrames();

    eMultiFramesMode GetMode();
    CSource* GetSource();
    int GetWidth();
    int GetHeight();

    // Inform if the multi frames output is engaged
    BOOL IsActive();

    // Enable the multi frames output
    void Enable();

    // Disable the multi frames output
    void Disable();

    BOOL IsSwitchRequested();
    void RequestSwitch();
    void HandleSwitch();

    void Reset();

    void SelectFrame();
    void UpdateFrame(TDeinterlaceInfo* pInfo, BOOL* bUseExtraBuffer, BYTE** lpBuffer, int* Pitch);

    // User commands handling
    BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam);

    void AckContentChange();

protected:

private:
    // Allocate memory buffer to store the picture containing all the frames
    void AllocateMemoryBuffer();

    // Free previously allocated memory buffer to store the picture containing all the frames
    void FreeMemoryBuffer();

    // Get the information relative to a frame including :
    // - the pointer to the corresponding memory buffer
    // - its width and height
    // - its line pitch
    void SelectFrameBuffer(int iFrame, BOOL bIncludingBorder, BYTE** lpFrameBuffer, int *iFrameLinePitch, int *iFrameWidth, int *iFrameHeight);

    // Draw a border around one frame
    // Color and size of each border are gien as parameters
    void DrawBorder(int iFrame, BOOL bIncludingExternalBorder, int iLuminLevel, unsigned int iLeftThick, unsigned int iRightThick, unsigned int iTopThick, unsigned int iBottomThick);

    // Draw a border around each frame - boder size is 2 pixels
    // Color is different for the current active frame and other frames
    void DrawBorders();

    // Shift all frames in the grid
    //
    // A positive value for iDeltaFrames means that some frames will
    // disappear at top left and new ones will appeared at bottom right
    // A negative value for iDeltaFrames means that some frames will
    // disappear at bottom right and new ones will appeared at top left
    void ShiftFrames(int iDeltaFrames);

    // Copy the content of one frame to another one
    void CopyFrame(int iFrameSrc, int iFrameDest);

    // Paint in black the content of a frame
    void ResetFrameToBlack(int iFrame);

    // Kind of mode : channels or stills
    eMultiFramesMode m_Mode;

    CSource* m_Source;

    // The screen is cut in m_NbCols columns and m_NbRows rows,
    // that is a total of m_NbFrames frames
    int m_NbFrames;
    int m_NbCols;
    int m_NbRows;

    // Width and height in pixels of the picture containing all the frames
    int m_Width;
    int m_Height;

    // Memory buffer containing all the frames
    BYTE* m_MemoryBuffer;

    // Table giving the loading status for each frame
    int* m_FrameFilled;

    // The active frame in the grid
    int m_CurrentFrame;

    // Indicate if multiple frames mode is ON or OFF
    BOOL m_Active;

    // Set when a switch of mode is requested
    BOOL m_SwitchRequested;

    // Indicate if the user commands to navigate through the frames is enabled or not
    BOOL m_NavigAllowed;

    // Number of frames to shift
    // Positive value to shift to left
    // Negative value to shift to right
    int m_DeltaNewFrame;

    // Set to TRUE when the user requests to fill the screen with new channels
    BOOL m_NextPage;
    BOOL m_PreviousPage;

    BOOL m_ContentChanged;
};


/// Global instance of the MultiFrames object
extern CMultiFrames* pMultiFrames;


#endif
