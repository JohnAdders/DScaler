/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1998 Timecop.  All rights reserved.
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

/** 
 * @file VBI_ccdecode.h ccdecode Header file
 */
 
#ifndef __CCDECODE_H___
#define __CCDECODE_H___

enum eCCColor
{
    CC_WHITE,
    CC_GREEN,
    CC_BLUE,
    CC_CYAN,
    CC_RED,
    CC_YELLOW,
    CC_MAGENTA,
    CC_BLACK,
};

typedef struct
{
    BOOL bIsActive;
    wchar_t Text;
    eCCColor ForeColor;
    eCCColor BackColor;
    BOOL bUnderline;
    BOOL bFlash;
    BOOL bItalics;
} TCCChar;

enum eCCMode
{
    CCMODE_OFF,
    CCMODE_CC1,
    CCMODE_CC2,
    CCMODE_CC3,
    CCMODE_CC4,
    CCMODE_TEXT1,
    CCMODE_TEXT2,
    CCMODE_TEXT3,
    CCMODE_TEXT4,
};

enum eCaptionMode
{
    TEXT,
    ROLL_UP,
    POP_ON,
    PAINT_ON,
};

#define CC_CHARS_PER_LINE 48

typedef struct
{
    TCCChar ScreenData[15][CC_CHARS_PER_LINE];
} TCCScreen;

void CC_Init_Data(double VBI_Frequency);
void CC_PaintScreen(HWND hWnd, TCCScreen* Screen, HDC hDC, RECT* PaintRect);
int VBI_DecodeLine_CC(BYTE* vbiline, eCCMode CCMode, BOOL IsOdd);


#endif
