////////////////////////////////////////////////////////////////////////////
// $Id: SchedMessageBox.cpp,v 1.1 2004-08-12 16:27:47 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
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
// 30 June 2004: Timer MessageBox with just a cancel button. It is 
// for the scheduled reordings. Most of the code is mine but some stuff
// is from someone else (Mustafa Demirhan I think).
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SchedMessageBox.h"
#include "TimeShift.h"
#include <afxpriv.h>

// Local vars for time display.
//
// need to pad these to double digit because MessageBox
// creation sets the dimentions and initializing to
// zero (0) leads to MessageBox title trucation if
// count is say 23H 56M 54S - get the idea - makes the
// MessageBox the right size!
int m_hours   = 88;
int m_minutes = 88;
int m_seconds = 88;

bool FirstTime = true; // heading flag - needed to get rid of padding values

// Sort out the handles
IMPLEMENT_DYNAMIC(SchedMessageBox, CWnd)
SchedMessageBox::SchedMessageBox(CWnd* pParent)
{
	m_hWndParent = pParent->GetSafeHwnd();    // can be NULL
	m_autoclose = NULL;
	m_count = 0;
}

// Message map stuff
BEGIN_MESSAGE_MAP(SchedMessageBox, CWnd)
	ON_WM_TIMER()
	ON_WM_CREATE()
	ON_MESSAGE(WM_INITDIALOG, OnSubclassedInit)
END_MESSAGE_MAP()


// Unhook window creation
int SchedMessageBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	AfxUnhookWindowCreate();
	return CWnd::OnCreate(lpCreateStruct);
}

// Disable OK button and make invisible, center cancel button and start timer
LRESULT SchedMessageBox::OnSubclassedInit(WPARAM wParam, LPARAM lParam)
{
	LRESULT lRet = Default();

	// Disable OK button nad make it disappear 
	CWnd* pOk = GetDlgItem(IDOK);
	pOk->EnableWindow(FALSE);
	pOk->ShowWindow(FALSE);

	// Move Cancel button to center of dialog
    CRect rectBtn;
    GetDlgItem(IDCANCEL)->GetWindowRect (rectBtn);
    ScreenToClient (&rectBtn);
    rectBtn.left = rectBtn.left - 50;   // this is hard coded for this MessageBox
    rectBtn.right = rectBtn.right - 50; // but could do relative stuff if needed
    GetDlgItem(IDCANCEL)->MoveWindow(rectBtn);

	// Start the timer
	//
	// The FirstTime flag is used later to normaize the timer interval. A short
	// timer interval is used at the start to do quick update of MessageBox title 
	// to get rid of padding. Otherwise 88 88 88 is displayed for about 1 second
	// before the proper values appear.

	FirstTime = true; // reset the flag to noramlize timer later
	SetTimer(100,10,NULL); // set timer to short interval		

	return lRet;
}

// Display running countdown and close when finished.
void SchedMessageBox::OnTimer(UINT nIDEvent)
{	
	if (nIDEvent == 100)
	{		
		if (m_count > 0)
		{
			// Do the calcs for H M S
			m_hours   = (m_count / 3600);		
			m_minutes = ((m_count  - (m_hours * 3600)) / 60);	
			m_seconds = (m_count - ((m_hours * 3600) + (m_minutes * 60)));

			SetWindowText(FormTitle(m_hours, m_minutes, m_seconds));

			m_count = m_count - 1; // take a second off the counter

			if (FirstTime)
			{
				FirstTime = false; // Not first time anymore
				KillTimer(100);
				SetTimer(100,1000,NULL); // set timer to normal interval
			}

			if (m_count == 0) // have we finished?
			{			
				KillTimer(100);
				PostMessage(WM_CLOSE,0,0); // close and go
			}
		}
	}
}

// Display message box, hooking it to do stuff
int SchedMessageBox::MessageBox(LPCTSTR lpszText, int count, bool bclose,MBIcon icon)
{	
	m_autoclose = bclose; // Irrelevant here as autoclose is not used
	// Have to make sure to sleep for at least 1 second otherwise it locks up
	// This happens if user puts a scheduled time = actual time
	if (count == 0) count = 1;
	m_count = count;
	AfxHookWindowCreate(this);
	return ::MessageBox(m_hWndParent, "Timer count-down to start of scheduled recording.\n"
				"\n"
				"The current displayed TV channel will be recorded.\n"
				"\n"
				"You can turn off your monitor and powered speakers\n"
				"during the recording (to save energy). This will not\n"
				"affect the recording.\n"				
				"\n"
				"Press 'CANCEL' to stop the timer and to abort the\n"
				"scheduled recording.\n",
				FormTitle(m_hours, m_minutes, m_seconds),  
				MB_OKCANCEL | MB_ICONINFORMATION);
}

// Compose a title for the dialog based on time left (H M S) to destroy it
CString SchedMessageBox::FormTitle(int num, int num2, int num3)
{
	CString s;
	s.Format("Recording Starts in %d Hours, %d Minutes, and %d Seconds. ", num, num2, num3);
	return s;
}

// Pump to check MessageBox for Cancel command
LRESULT SchedMessageBox::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_COMMAND)
	{
		if(HIWORD(wParam) == BN_CLICKED )
		{
			if (LOWORD(wParam) == IDCANCEL)
			{
			KillTimer(100);
			if (m_count > 0) // For unknown reason I need to do this check
			{
				CTimeShift::CancelSchedule(); // Set the cancel flag
			}
			PostMessage(WM_CLOSE,0,0); // close and go
			}
		}
	}
    return CWnd::DefWindowProc(message, wParam, lParam);
}