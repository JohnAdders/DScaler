/////////////////////////////////////////////////////////////////////////////
// $Id: DScalerApp.cpp,v 1.15 2002-07-09 07:20:31 adcockj Exp $
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
// Change Log
//
// Date          Developer             Changes
//
// 22 Jun 2001   Torbjörn Jansson      Initial release
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.14  2002/06/13 12:10:21  adcockj
// Move to new Setings dialog for filers, video deint and advanced settings
//
// Revision 1.13  2002/05/24 23:04:55  robmuller
// Patch #553392 by Anonymous.
// Fix for VS.NET.
//
// Revision 1.12  2002/04/24 19:10:38  tobbej
// test of new tree based setting dialog
//
// Revision 1.11  2001/12/09 22:00:42  tobbej
// experimental dshow support, doesnt work yet
// define WANT_DSHOW_SUPPORT if you want to try it
//
// Revision 1.10  2001/12/03 19:33:59  adcockj
// Bug fixes for settings and memory
//
// Revision 1.9  2001/11/23 10:49:16  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.8  2001/11/19 14:02:48  adcockj
// Apply patches from Sandu Turcan
//
// Revision 1.7  2001/11/14 11:28:03  adcockj
// Bug fixes
//
// Revision 1.6  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.5  2001/11/04 14:44:58  adcockj
// Bug fixes
//
// Revision 1.4  2001/07/13 16:14:55  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.3  2001/07/12 16:16:39  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "DScalerApp.h"
#include "DScaler.h"
#include <afxpriv.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDScalerApp

BEGIN_MESSAGE_MAP(CDScalerApp, CWinApp)
    //{{AFX_MSG_MAP(CDScalerApp)
        // NOTE - the ClassWizard will add and remove mapping macros here.
        //    DO NOT EDIT what you see in these blocks of generated code!
    //}}AFX_MSG
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDScalerApp construction

CDScalerApp::CDScalerApp()
{

}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDScalerApp object

CDScalerApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CDScalerApp initialization

//the old winmain
int APIENTRY WinMainOld(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);

BOOL CDScalerApp::InitInstance()
{

#ifdef _DEBUG
    // Uncomment line below for full memory leak debuging.
	// By default you get a single check at the end.
    // afxMemDF=allocMemDF|checkAlwaysMemDF|delayFreeMemDF;
#endif

#ifdef WANT_DSHOW_SUPPORT
	//COM init
	CoInitializeEx(NULL,COINIT_MULTITHREADED);
#endif

// changes to allow copmilation under VC 7.0 added by IDLSOFT
#if _MFC_VER<0x0700
#ifdef _AFXDLL
    Enable3dControls();         // Call this when using MFC in a shared DLL
#else
    Enable3dControlsStatic();   // Call this when linking to MFC statically
#endif
#endif

    hResourceInst = LoadLibrary("DScalerRes.dll");
    if(hResourceInst == NULL)
    {
        MessageBox(NULL, "DScaler can't find the resource library DScalerRes.dll", "Installation Error", MB_OK | MB_ICONSTOP);
        return FALSE;
    }
    
    AfxSetResourceHandle(hResourceInst);

    // If we change WritePrivateProfileInt/WritePrivateProfileString to AfxGetApp()->WriteProfileInt/string
    // we coud easily change DScaler to write to eighter registry or ini file.
    //
    // Use registry for saving settings.
    // without this (if i remember right) AfxGetApp()->WriteProfileInt/string goes to ini file
    SetRegistryKey(_T("DScaler"));

    // call the real winmain

// changes to allow copmilation under VC 7.0 added by IDLSOFT
#if _MFC_VER<0x0700
	WinMainOld(m_hInstance,m_hPrevInstance,m_lpCmdLine,m_nCmdShow);
#else
	WinMainOld(m_hInstance,NULL,m_lpCmdLine,m_nCmdShow);
#endif

    AfxSetResourceHandle(m_hInstance);
    FreeLibrary(hResourceInst);

    // return false so message loop doesn't start
    return FALSE;
}

void CDScalerApp::WinHelp(DWORD dwData, UINT nCmd) 
{
	//compensate for differences between winhelp and htmlhelp
	//(it will fail to open context help if nCmd is not changed)
	if(nCmd==HELP_CONTEXT)
	{
		nCmd=HH_HELP_CONTEXT;
        //the default help id is HID_BASE_RESOURCE+dialog template id
        //but we cant use that for empty pages and the generic property page
        //so set a new help id to use insted.
        if(dwData >= HID_BASE_RESOURCE)
        {
            dwData -= HID_BASE_RESOURCE;
        }
	}
	
	//try to open the help
    if(::HtmlHelp(hWnd, "DScaler.chm", nCmd, dwData)==NULL)
	{
		//didnt work, maybe wrong help id? try to open just the toc
        if(::HtmlHelp(hWnd, "DScaler.chm", HH_DISPLAY_TOC, NULL)==NULL)
		{
			AfxMessageBox(_T("Failed to open help"),MB_OK|MB_ICONASTERISK);
		}
	}
}
