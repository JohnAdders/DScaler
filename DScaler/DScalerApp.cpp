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
 * @file DScalerApp.cpp MFC wrapper for normal WinMain
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "DScalerApp.h"
#include "DScaler.h"
#include <afxpriv.h>
#include <afxdisp.h>
#include "crash.h"
#include "LibraryCache.h"

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

    DScalerInitializeThread("Main thread");

#ifdef WANT_DSHOW_SUPPORT
    //COM init.
    //Use a multi threaded apartment so we don't have to marshal every
    //com interface pointer between this thread and the output thread.
    //Marshaling can only be left out if both threads are in the same
    //apartment and this can only happen if both is in a multithreaded
    //apartment
    //CoInitializeEx(NULL,COINIT_MULTITHREADED);

    AfxOleInit();
#endif

// changes to allow copmilation under VC 7.0 added by IDLSOFT
#if _MFC_VER<0x0700
#ifdef _AFXDLL
    Enable3dControls();         // Call this when using MFC in a shared DLL
#else
    Enable3dControlsStatic();   // Call this when linking to MFC statically
#endif
#endif

    hResourceInst = LibraryCache::GetLibraryHandle("DScalerRes.dll");
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

//#ifdef WANT_DSHOW_SUPPORT
//    CoUninitialize();
//#endif
    DScalerDeinitializeThread();

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
    if(::HtmlHelp(::GetMainWnd(), "DScaler.chm", nCmd, dwData)==NULL)
    {
        //didnt work, maybe wrong help id? try to open just the toc
        if(::HtmlHelp(::GetMainWnd(), "DScaler.chm", HH_DISPLAY_TOC, NULL)==NULL)
        {
            AfxMessageBox(_T("Failed to open help"),MB_OK|MB_ICONASTERISK);
        }
    }
}
