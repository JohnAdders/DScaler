/////////////////////////////////////////////////////////////////////////////
// $Id: VBI_CCdecode.h,v 1.4 2001-07-12 16:16:40 adcockj Exp $
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
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           Added Header file
//                                     removed xwindows calls
//                                     put definitions in header
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __CCDECODE_H___
#define __CCDECODE_H___

typedef enum
{
    CC_WHITE,
    CC_GREEN,
    CC_BLUE,
    CC_CYAN,
    CC_RED,
    CC_YELLOW,
    CC_MAGENTA,
    CC_BLACK,
} CC_Color;

typedef struct
{
    BOOL bIsActive;
    char Text;
    CC_Color ForeColor;
    CC_Color BackColor;
    BOOL bUnderline;
    BOOL bFlash;
    BOOL bItalics;
} TCC_Char;

typedef enum
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
} CCMODE;

typedef enum
{
    TEXT,
    ROLL_UP,
    POP_ON,
    PAINT_ON,
} CAPTIONMODE;

#define CC_CHARS_PER_LINE 48

typedef struct
{
    TCC_Char ScreenData[15][CC_CHARS_PER_LINE];
} TCC_Screen;

int CC_DecodeLine(BYTE* vbiline, CCMODE CCMode, BOOL IsOdd);
void CC_PaintScreen(HWND hWnd, TCC_Screen* Screen, HDC hDC, RECT* PaintRect);


#endif
