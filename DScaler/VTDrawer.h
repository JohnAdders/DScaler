/////////////////////////////////////////////////////////////////////////////
// $Id: VTDrawer.h,v 1.1 2002-01-15 11:16:03 temperton Exp $
/////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2002 Mike Temperton.  All rights reserved.
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
//  GNU Library General Public License for more details
/////////////////////////////////////////////////////////////////////////////
//
//  Portions Copyright (C) 2000 John Adcock
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __VTDRAWER_H__
#define __VTDRAWER_H__

#include "stdafx.h"
#include "VBI_VideoText.h"

typedef char TVTHeaderLine[40], *LPTVTHeaderLine, *PTVTHeaderLine;

#define VTDF_HIDDEN          0x0001
#define VTDF_HIDDENONLY      0x0002
#define VTDF_FLASH           0x0004
#define VTDF_FLASHONLY       0x0008
#define VTDF_MIXMODE         0x0010
#define VTDF_CLOCKONLY       0x0020
#define VTDF_HEADERONLY      0x0040
#define VTDF_FORCEDRAW       0x0080
#define VTDF_CLEARFLASH      0x0100

#define VTDF_HIDDENMASK      0x0003
#define VTDF_FLASHMASK       0x000C

class
    CVTDrawer
{
public:
    CVTDrawer();
    ~CVTDrawer();

    void SetBounds(HDC hDC, RECT* Rect);
    bool Draw(TVTPage* pPage, TVTHeaderLine* pHeader, HDC hDC, LPPOINT pOrigin, unsigned long ulFlags, eVTCodePage VTCodePage);

    int GetAvgWidth();
    int GetAvgHeight();
private:
    HBRUSH m_hBrushes[9];
    RECT m_Rect;
    HFONT m_hFont;
    HFONT m_hDoubleFont;
    int m_AvgWidth, m_AvgHeight;
    double m_dAvgWidth, m_dAvgHeight;
};

#endif
