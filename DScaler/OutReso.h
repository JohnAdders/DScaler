/////////////////////////////////////////////////////////////////////////////
// $Id: OutReso.h,v 1.1 2003-02-06 00:58:53 laurentg Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 Laurent Garnier  All rights reserved.
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
//
// This module uses code from Kristian Trenskow provided as DScaler patch
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// $Log: not supported by cvs2svn $
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __OUTRESO_H___
#define __OUTRESO_H___

extern int OutputReso;

void OutReso_UpdateMenu(HMENU hMenu);
void OutReso_SetMenu(HMENU hMenu);
BOOL ProcessOutResoSelection(HWND hWnd, WORD wMenuID);
void OutReso_Change(HWND hWnd, BOOL bUseRegistrySettings);

#endif

