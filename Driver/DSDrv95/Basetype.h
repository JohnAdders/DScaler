//---------------------------------------------------------------------------
// FileName     : basetype.h
// Created      : 15/04/95
// Project      : Some projects
// Author       : Elli
// Contents     : Some basetype depended on current OS and environment
// Note         : This program is free software; you can redistribute it
//                and/or modify it under the terms of the GNU General Public
//                License as published by the Free Software Foundation;
//                either version 2 of the License, or (at your option) any
//                later version.
//
//                This program is distributed in the hope that it will be useful,
//                but WITHOUT ANY WARRANTY; without even the implied warranty of
//                MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//                GNU General Public License for more details.
//
//                You should have received a copy of the GNU General Public
//                License along with this program; if not, write to the Free
//                Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
// History      :
//                V 1.00 initial version
//---------------------------------------------------------------------------


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







