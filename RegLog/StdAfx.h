// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#define VC_EXTRALEAN
#include <afxwin.h>
#include <afxext.h>
#include <afxcmn.h>

#include <atlbase.h>

// Windows Header Files:
#include <windowsx.h>

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <io.h>
#include <fcntl.h>
#include <commctrl.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ddraw.h>
#include <process.h>
#include <math.h>
#include <mmsystem.h>
#include <vfw.h>
#include <winioctl.h>
#include "DSDrv.h"
#include "HtmlHelp.H"
#include <vector>
#include <string>

// fix for including external header with IDC_STATIC defined
#ifdef IDC_STATIC
#undef IDC_STATIC
#endif

using namespace std;


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
