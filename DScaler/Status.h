/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
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
//
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 11 Aug 2000   John Adcock           Reorganised code
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.8  2003/01/26 10:34:57  tobbej
// changed statusbar updates from output thread to be thread safe (PostMessage instead of SendMessage)
//
//////////////////////////////////////////////////////////////////////////////

/** 
 * @file status.h status Header file
 */
 
#ifndef __STATUS_H___
#define __STATUS_H___

enum eStatusBarBox
{
    STATUS_TEXT = 0,
    STATUS_MODE,
    STATUS_AUDIO,
    STATUS_FPS,
    STATUS_LASTONE,
};

BOOL StatusBar_Init();
BOOL StatusBar_IsVisible();
BOOL StatusBar_Adjust(HWND hWnd);
void StatusBar_Destroy();
void StatusBar_ShowText(eStatusBarBox Field, LPCSTR  szText);
void StatusBar_ShowWindow(BOOL bShow);
void StatusBar_Repaint();
int StatusBar_Height();


#endif
