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
// Change Log
//
// Date          Developer             Changes
//
// 11 Aug 2000   John Adcock           Better support for error messages
//
/////////////////////////////////////////////////////////////////////////////

/** 
 * @file errorbox.h errorbox Header file
 */
 
#ifndef __ERRORBOX_H___
#define __ERRORBOX_H___

void _ErrorBox(HWND hwndParent, LPCSTR szFile, int Line, LPCSTR szMessage);
void _RealErrorBox(LPCSTR szFile, int Line, LPCSTR szMessage);

#define RealErrorBox(Message) _RealErrorBox(__FILE__, __LINE__, Message);

#define ErrorBox(Message) _ErrorBox(NULL, __FILE__, __LINE__, Message);
#define ErrorBoxDlg(hWnd,Message) _ErrorBox(hWnd, __FILE__, __LINE__, Message);

#endif
