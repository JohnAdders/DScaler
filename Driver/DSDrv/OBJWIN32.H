/////////////////////////////////////////////////////////////////////////////
// objwin32.h
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
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
// 15 Apr 1995   Mathias Ellinger      initial version
//
// 24 Jul 2000   John Adcock           Original dTV Release
//                                     Added Memory Alloc functions
//
/////////////////////////////////////////////////////////////////////////////

#if ! defined (__OBJWIN32_H)
#define __OBJWIN32_H

#if defined (_AFXDLL) || defined (_AFXEXT) || defined (F_USE_AFX)

#if ! defined (F_USE_AFX)
#define F_USE_AFX
#endif

#include <afxext.h>
#endif


#if defined (CLASSEXPORT)
#define _CLASSEXPORT     __declspec(dllexport)
#else
#define _CLASSEXPORT
#endif

#if ! defined (_WINDOWS_)
#include <windows.h>
#endif



class _CLASSEXPORT CWin32Object
 #if defined (F_USE_AFX)
  : public CObject
 #endif
 {
//  DECLARE_DYNAMIC( CWin32Object )

  public:
   CWin32Object()
    #if defined (F_USE_AFX)
     : CObject()
    #endif
    {
     dwLastError = ERROR_SUCCESS;
    }

           DWORD   getLastError(void);
           LPSTR   getErrorMessage(LPSTR pzsMessageBuffer);


  protected:

   virtual void     setError();
   virtual void     setError(DWORD dwError);

  protected:
   DWORD   dwLastError;
 };


inline DWORD
CWin32Object::getLastError(void)
 {
  return dwLastError;
 }

inline void
CWin32Object::setError(void)
 {
  dwLastError = ::GetLastError();
 }


inline void
CWin32Object::setError(DWORD dwError)
 {
  dwLastError = dwError;
 }


inline
LPSTR
CWin32Object::getErrorMessage(LPSTR pszMessageBuffer)
 {
  ::FormatMessage(
                  FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL,
                  dwLastError,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR) pszMessageBuffer,
                  128,                              // ???
                  NULL
                 );

  return pszMessageBuffer;
 }

#endif








