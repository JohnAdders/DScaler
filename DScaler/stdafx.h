/////////////////////////////////////////////////////////////////////////////
// $Id: stdafx.h,v 1.32 2008-03-10 17:41:46 adcockj Exp $
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
// Revision 1.31  2008/02/08 13:43:19  adcockj
// Changes to support cmake compilation
//
// Revision 1.30  2007/02/18 21:50:04  robmuller
// Added option to not compile saa713x code.
//
// Revision 1.29  2007/02/18 21:32:44  robmuller
// Added option to not compile cx2388x code.
//
// Revision 1.28  2007/02/18 21:15:31  robmuller
// Added option to not compile BT8x8 code.
//
// Revision 1.27  2006/12/20 10:06:54  adcockj
// Added new files to vs2005 projects and added new configurations for express
//
// Revision 1.26  2006/12/12 23:43:10  robmuller
// Fix compile errors with Visual Studio 2005 Express.
//
// Revision 1.25  2006/10/06 13:35:28  adcockj
// Added projects for .NET 2005 and fixed most of the warnings and errors
//
// Revision 1.24  2005/05/12 08:33:19  adcockj
// hopefully fixed win98 comapatability
//
// Revision 1.23  2005/03/20 14:06:54  adcockj
// Added defines for new SDK
//
// Revision 1.22  2005/03/11 14:54:41  adcockj
// Get rid of a load of compilation warnings in vs.net
//
// Revision 1.21  2005/03/07 21:44:13  adcockj
// More prep for new release
//
// Revision 1.20  2004/08/14 13:45:23  adcockj
// Fixes to get new settings code working under VS6
//
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
#ifdef WANT_DSHOW_SUPPORT
	// JA 7-Mar-2005 added to get compiled
    // want to remove the need for this
	// but will require changing a lot of string functions
	#define NO_DSHOW_STRSAFE
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
#include <multimon.h>

// fix for including external header with IDC_STATIC defined
#ifdef IDC_STATIC
#undef IDC_STATIC
#endif

// defien stuff that we try and use if they are available
#define WC_NO_BEST_FIT_CHARS      0x00000400
#define IDC_HAND MAKEINTRESOURCE(32649)


using namespace std;

#pragma warning (disable : 4018)
