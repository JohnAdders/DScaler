/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 Mathias Ellinger.  All rights reserved.
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
// 20 Jun 1994   Mathias Ellinger      initial version
//
// 24 Jul 2000   John Adcock           Original dTV Release
//                                     Added Memory Alloc functions
//
/////////////////////////////////////////////////////////////////////////////

#if ! defined (__BASETYPE_H)
#define __BASETYPE_H

#define IN
#define OUT
#define BOTH

#if defined (WIN32) && ! defined (_WINDOWS_)

#include <windows.h>

#elif defined (_NTKERNEL_)

#if ! defined (_NTDDK_)
extern "C" {
#include <ntddk.h>
}
#endif

#define WINAPI
typedef char * PCHAR ;
typedef ULONG DWORD, * LPDWORD;



#elif defined (WIN95)

#define WINAPI

#include "basedef.h"
#include <vmm.h>
#include <vwin32.h>
#include <winerror.h>


#define METHOD_BUFFERED                 0
#define FILE_ANY_ACCESS                 0
#define NTSTATUS                        ULONG
#define STATUS_SUCCESS                  ERROR_SUCCESS
#define STATUS_INSUFFICIENT_RESOURCES   ERROR_NOT_ENOUGH_MEMORY
#define STATUS_ACCESS_DENIED            ERROR_ACCESS_DENIED

#endif


#endif







