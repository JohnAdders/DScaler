////////////////////////////////////////////////////////////////////////////
// $Id: SchedMessageBox.h,v 1.1 2004-08-12 16:27:47 adcockj Exp $
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

#pragma once

class SchedMessageBox : public CWnd
{
	DECLARE_DYNAMIC(SchedMessageBox)

public:
	SchedMessageBox(CWnd* pParent);

	enum MBIcon
	{
		MBICONNONE = 0,
		MBICONSTOP = MB_ICONSTOP,
		MBICONQUESTION = MB_ICONQUESTION,
		MBICONEXCLAMATION = MB_ICONEXCLAMATION,
		MBICONINFORMATION = MB_ICONINFORMATION
	};

	int MessageBox(LPCTSTR lpszText, int count, bool bclose = false, MBIcon icon = MBICONNONE );	

protected:
	int m_count;
	bool m_autoclose;	
	HWND  m_hWndParent;

	CString FormTitle(int num, int num2, int num3);

	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSubclassedInit(WPARAM wParam, LPARAM lParam);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT nIDEvent);

	DECLARE_MESSAGE_MAP()
};
