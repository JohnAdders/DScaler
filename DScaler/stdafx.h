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
 * @file stdafx.h Precompiled Header file
 */


#ifndef WINVER
    #define WINVER 0x0400
    #define _WIN32_WINNT 0x0400
#endif

// need to try and remove need for this
// shoudl be using STL or secure function
#define _CRT_SECURE_NO_WARNINGS

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN
//#include <afxwin.h>
//#include <afxext.h>
//#include <afxcmn.h>

#include <atlbase.h>
#include <atltime.h>

//uncomment the folowing line if you want to try the experimental direct show support
#ifdef WANT_DSHOW_SUPPORT
    // JA 7-Mar-2005 added to get compiled
    // want to remove the need for this
    // but will require changing a lot of string functions
    #define NO_DSHOW_STRSAFE
    #include <dshow.h>
#endif

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include <ddraw.h>
#include <process.h>
#include <mmsystem.h>
#include <vfw.h>
#include <winioctl.h>
#include <sys\stat.h>
#include <sys\timeb.h>
#include <vector>
#include <map>
#include <list>
#include <string>
#include <sstream>
#include <iomanip>
#include <functional>
#include <algorithm>
#include <math.h>
#include <multimon.h>
#include <time.h>


#include "ErrorBox.h"
#include "DSDrv.h"
#include "HtmlHelp.H"
#include "SmartPtr.h"
#include "SmartHandle.h"
#include "DScalerUtils.h"
#include "SettingsMaster.h"

// fix for including external header with IDC_STATIC defined
#ifdef IDC_STATIC
#undef IDC_STATIC
#endif


// defien stuff that we try and use if they are available
#define WC_NO_BEST_FIT_CHARS      0x00000400
#define IDC_HAND MAKEINTRESOURCE(32649)

#pragma warning (disable : 4018)
