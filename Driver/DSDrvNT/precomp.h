/////////////////////////////////////////////////////////////////////////////
// $Id: precomp.h,v 1.3 2001-10-04 12:38:17 adcockj Exp $
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
// This software was based on hwiodrv from the FreeTV project Those portions are
// Copyright (C) Mathias Ellinger
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 11 Jun 2001   Torbörn Jansson       moved includes to precomp.h
//
/////////////////////////////////////////////////////////////////////////////

//ddk build enviroment creates precompiled headers from this file

#if defined(__cplusplus)
extern "C" {
#endif

#include <ntddk.h>
#include <basetsd.h>
#include <windef.h>

#if defined (__cplusplus)
}
#endif

#include "DSDrv.h"
#include "ioclass.h"
#include "debugout.h"
