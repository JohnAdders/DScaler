/////////////////////////////////////////////////////////////////////////////
// $Id: HardwareSettings.h,v 1.2 2003-10-27 10:39:51 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Laurent Garnier.  All rights reserved.
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.1  2002/08/11 12:14:02  laurentg
// Cut BT Card setup and general hardware setup in two different windows
//
//
//////////////////////////////////////////////////////////////////////////////

/** 
 * @file hardwaresettings.h hardwaresettings Header file
 */
 
#ifndef __HARDWARESETTINGS_H___
#define __HARDWARESETTINGS_H___

#include "Settings.h"

BOOL APIENTRY HardwareSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

#endif
