/////////////////////////////////////////////////////////////////////////////
// $Id: stdafx.h,v 1.20 2004-08-14 13:45:23 adcockj Exp $
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.19  2003/10/27 10:39:54  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.18  2003/03/05 21:45:23  tobbej
// new include for stringstream
//
// Revision 1.17  2002/11/06 20:49:00  adcockj
// Changes for DSShow compiliation
//
// Revision 1.16  2002/06/13 08:21:30  adcockj
// Changed to compile without DShow by default
//
// Revision 1.15  2002/03/26 19:48:32  adcockj
// Changed default to compile with DShow
//
// Revision 1.14  2002/02/03 10:33:41  tobbej
// fixed spelling error
//
// Revision 1.13  2001/12/17 19:54:20  tobbej
// added a comment about the dshow define
//
// Revision 1.12  2001/12/14 13:59:34  adcockj
// Fix to remove dependancy on dshow.h if not required
//
// Revision 1.11  2001/12/09 22:00:42  tobbej
// experimental dshow support, doesnt work yet
// define WANT_DSHOW_SUPPORT if you want to try it
//
// Revision 1.10  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.9  2001/11/17 18:15:57  adcockj
// Bugfixes (including very silly performance bug)
//
// Revision 1.8  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.7  2001/07/23 20:52:07  ericschmidt
// Added TimeShift class.  Original Release.  Got record and playback code working.
//
// Revision 1.6  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.5  2001/07/12 16:26:59  adcockj
// Added CVS id and log
//
//////////////////////////////////////////////////////////////////////////////

/** 
 * @file stdafx.h Precompiled Header file
 */
 

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define VC_EXTRALEAN
#include <afxwin.h>
#include <afxext.h>
#include <afxcmn.h>

#include <atlbase.h>

//uncomment the folowing line if you want to try the experimental direct show support
#define WANT_DSHOW_SUPPORT 0
#ifdef WANT_DSHOW_SUPPORT
    #include <dshow.h>
#endif

//#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <io.h>
#include <fcntl.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ddraw.h>
#include <process.h>
#include <math.h>
#include <mmsystem.h>
#include <vfw.h>
#include <winioctl.h>
#include "ErrorBox.h"
#include "DSDrv.h"
#include "HtmlHelp.H"
#include <vector>
#include <string>
#include <sstream>
// fix for including external header with IDC_STATIC defined
#ifdef IDC_STATIC
#undef IDC_STATIC
#endif

using namespace std;
