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

/**
 * @file errorbox.h errorbox Header file
 */

#ifndef __ERRORBOX_H___
#define __ERRORBOX_H___

void _ErrorBox(HWND hwndParent, LPCTSTR szFile, int Line, const tstring& Message);
#ifdef _UNICODE
void _ErrorBox(HWND hwndParent, LPCTSTR szFile, int Line, const std::string& Message);
#endif
void _RealErrorBox(LPCTSTR szFile, int Line, const tstring& szMessage);

#define RealErrorBox(Message) _RealErrorBox(_T(__FILE__), __LINE__, Message);

#define ErrorBox(Message) _ErrorBox(NULL, _T(__FILE__), __LINE__, Message);
#define ErrorBoxDlg(hWnd,Message) _ErrorBox(hWnd, _T(__FILE__), __LINE__, Message);

#endif
