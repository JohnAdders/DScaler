/////////////////////////////////////////////////////////////////////////////
// $Id: MultiFrames.h,v 1.2 2003-03-17 22:34:29 laurentg Exp $
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
// Revision 1.1  2003/03/16 18:27:46  laurentg
// New multiple frames feature
//
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __MULTIFRAMES_H___
#define __MULTIFRAMES_H___


#include "DS_ApiCommon.h"


class CMultiFrames
{
public:
    CMultiFrames(int iFrames, int iDuration);
    ~CMultiFrames();

	int GetWidth();
	int GetHeight();
	BOOL IsActive();
	void Enable();
	void Disable();
	BOOL IsSwitchRequested();
	void RequestSwitch();
	void DoSwitch();
	void Reset();
	void SelectFrame();
	void UpdateFrame(TDeinterlaceInfo* pInfo, BOOL* bUseExtraBuffer, BYTE** lpBuffer, int* Pitch);
	BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam);

protected:

private:
	void AllocateMemoryBuffer();
	void FreeMemoryBuffer();
	void SelectFrameBuffer(int iFrame);
	void DrawBorders();
	void ShiftFrames(int iDeltaFrames);
	int iWidth;
	int iHeight;
	BOOL bActive;
	BOOL bSwitchRequested;
	BYTE* lpMemoryBuffer;
	int iNbFrames;
	int iCurrentFrame;
	int iNbCols;
	int iNbRows;
	BYTE* lpFrameBuffer;
	int iFrameWidth;
	int iFrameHeight;
	int iFramePitch;
	int *bFrameFilled;
	DWORD LastTickCount;
	int iDeltaTicksChange;
	BOOL bNavigAllowed;
	int iDeltaNewFrame;
};


/// Global instance of the MultiFrames object
extern CMultiFrames* pMultiFrames;


#endif
