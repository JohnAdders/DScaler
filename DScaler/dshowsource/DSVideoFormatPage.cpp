/////////////////////////////////////////////////////////////////////////////
// $Id: DSVideoFormatPage.cpp,v 1.4 2002-09-17 19:29:56 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Torbjörn Jansson.  All rights reserved.
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
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.3  2002/09/11 16:41:02  tobbej
// implemented clear button
//
// Revision 1.2  2002/09/07 13:33:35  tobbej
// implemented delete and allow listbox to be reordered via drag and drop
//
// Revision 1.1  2002/09/04 17:08:31  tobbej
// new video format configuration dialog (resolution)
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file DSVideoFormatPage.cpp implementation file
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT

#include "..\dscaler.h"
#include "DSVideoFormatPage.h"
#include "Other.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDSVideoFormatPage dialog


CDSVideoFormatPage::CDSVideoFormatPage(CString name,vector<CDShowGraph::CVideoFormat> &fmts,CSimpleSetting *pResolution)
	: CTreeSettingsPage(name,CDSVideoFormatPage::IDD),
	m_RealVideoFmt(fmts),
	m_bInUpdateControlls(false),
	m_pResolutionSetting(pResolution)
{
	//{{AFX_DATA_INIT(CDSVideoFormatPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_VideoFmt=fmts;
}


void CDSVideoFormatPage::DoDataExchange(CDataExchange* pDX)
{
	CTreeSettingsPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDSVideoFormatPage)
	DDX_Control(pDX, IDC_DSHOW_VIDEOFMTS_FMTLIST, m_ListBox);
	DDX_Control(pDX, IDC_DSHOW_VIDEOFMTS_WIDTH_SPIN,m_SpinWidth);
	DDX_Control(pDX, IDC_DSHOW_VIDEOFMTS_HEIGHT_SPIN,m_SpinHeight);
	DDX_Control(pDX, IDC_DSHOW_VIDEOFMTS_YUY2,m_YUY2Check);
	DDX_Control(pDX, IDC_DSHOW_VIDEOFMTS_FIELDFMT, m_SampleFormat);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDSVideoFormatPage, CTreeSettingsPage)
	//{{AFX_MSG_MAP(CDSVideoFormatPage)
	ON_LBN_SELCHANGE(IDC_DSHOW_VIDEOFMTS_FMTLIST,OnVideoFmtSelChange)
	ON_NOTIFY(UDN_DELTAPOS, IDC_DSHOW_VIDEOFMTS_WIDTH_SPIN, OnDeltaPosWidth)
	ON_NOTIFY(UDN_DELTAPOS, IDC_DSHOW_VIDEOFMTS_HEIGHT_SPIN, OnDeltaPosHeight)
	ON_EN_CHANGE(IDC_DSHOW_VIDEOFMTS_WIDTH,OnChangeWidth)
	ON_EN_CHANGE(IDC_DSHOW_VIDEOFMTS_HEIGHT,OnChangeHeight)
	ON_EN_CHANGE(IDC_DSHOW_VIDEOFMTS_NAME,OnChangeName)
	ON_BN_CLICKED(IDC_DSHOW_VIDEOFMTS_YUY2,OnClickedYUY2)
	ON_CBN_SELENDOK(IDC_DSHOW_VIDEOFMTS_FIELDFMT,OnSelEndOkFieldFmt)
	ON_WM_CONTEXTMENU()
	ON_BN_CLICKED(IDC_DSHOW_VIDEOFMTS_NEW,OnClickedNew)
	ON_BN_CLICKED(IDC_DSHOW_VIDEOFMTS_DELETE,OnClickedDelete)
	ON_BN_CLICKED(IDC_DSHOW_VIDEOFMTS_CLEAR,OnClickedClear)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CDSVideoFormatPage::OnInitDialog() 
{
	TRACE(_T("%s(%d) : CDSVideoFormatPage::OnInitDialog\n"),__FILE__,__LINE__);
	CTreeSettingsPage::OnInitDialog();
	
	for(vector<CDShowGraph::CVideoFormat>::size_type index=0;index<m_VideoFmt.size();index++)
	{
		int pos=m_ListBox.AddString(m_VideoFmt[index].m_Name.c_str());
		if(pos!=LB_ERR)
		{
			int err=m_ListBox.SetItemData(pos,index);
			if(err==LB_ERR)
			{
				//oops can't set item data
			}
		}
	}
	
	int cbindex=m_SampleFormat.AddString(_T("Auto"));
	m_SampleFormat.SetItemData(cbindex,DSREND_FIELD_FORMAT_AUTO);
	cbindex=m_SampleFormat.AddString(_T("Frame"));
	m_SampleFormat.SetItemData(cbindex,DSREND_FIELD_FORMAT_FRAME);
	cbindex=m_SampleFormat.AddString(_T("Field"));
	m_SampleFormat.SetItemData(cbindex,DSREND_FIELD_FORMAT_FIELD);
	
	//select first entry
	m_ListBox.SetCurSel(0);
	UpdateControlls();

	return TRUE;
}

void CDSVideoFormatPage::OnOK()
{
	TRACE(_T("%s(%d) : CDSVideoFormatPage::OnOK\n"),__FILE__,__LINE__);
	
	//update the resoltion setting so it points to the right entry in the new vector
	long OldResolution=m_pResolutionSetting->GetValue();
	long NewResolution=-1;
	CDShowGraph::CVideoFormat OldFormat;
	if(OldResolution>=0 && OldResolution<m_RealVideoFmt.size())
	{
		OldFormat=m_RealVideoFmt[OldResolution];
	}
	else
	{
		OldResolution=-1;
	}

	m_RealVideoFmt.erase(m_RealVideoFmt.begin(),m_RealVideoFmt.end());
	for(int i=0;i<m_ListBox.GetCount();i++)
	{
		int pos=m_ListBox.GetItemData(i);
		m_RealVideoFmt.push_back(m_VideoFmt[pos]);
	}

	if(OldResolution!=-1)
	{
		for(int i=0;i<m_RealVideoFmt.size();i++)
		{
			if(m_RealVideoFmt[i]==OldFormat)
			{
				NewResolution=i;
				break;
			}
		}
	}

	m_pResolutionSetting->SetValue(NewResolution);
}

void CDSVideoFormatPage::OnVideoFmtSelChange()
{
	TRACE(_T("%s(%d) : CDSVideoFormatPage::OnVideoFmtSelChange\n"),__FILE__,__LINE__);
	if(m_ListBox.GetCurSel()!=LB_ERR)
	{
		UpdateControlls();
	}
}

void CDSVideoFormatPage::UpdateControlls()
{
	TRACE(_T("%s(%d) : CDSVideoFormatPage::UpdateControlls\n"),__FILE__,__LINE__);
	if(m_bInUpdateControlls)
	{
		return;
	}
	
	m_bInUpdateControlls=true;
	int CurSel=m_ListBox.GetCurSel();
	if(CurSel!=LB_ERR)
	{
		::EnableWindow(::GetDlgItem(m_hWnd,IDC_DSHOW_VIDEOFMTS_NAME),TRUE);
		::EnableWindow(::GetDlgItem(m_hWnd,IDC_DSHOW_VIDEOFMTS_WIDTH),TRUE);
		::EnableWindow(::GetDlgItem(m_hWnd,IDC_DSHOW_VIDEOFMTS_WIDTH_SPIN),TRUE);
		::EnableWindow(::GetDlgItem(m_hWnd,IDC_DSHOW_VIDEOFMTS_HEIGHT_SPIN),TRUE);
		::EnableWindow(::GetDlgItem(m_hWnd,IDC_DSHOW_VIDEOFMTS_HEIGHT),TRUE);
		::EnableWindow(::GetDlgItem(m_hWnd,IDC_DSHOW_VIDEOFMTS_FIELDFMT),TRUE);
		::EnableWindow(::GetDlgItem(m_hWnd,IDC_DSHOW_VIDEOFMTS_YUY2),TRUE);
		
		DWORD pos=m_ListBox.GetItemData(CurSel);
		SetDlgItemText(IDC_DSHOW_VIDEOFMTS_NAME,m_VideoFmt[pos].m_Name.c_str());
		SetDlgItemInt(IDC_DSHOW_VIDEOFMTS_WIDTH,m_VideoFmt[pos].m_Width);
		SetDlgItemInt(IDC_DSHOW_VIDEOFMTS_HEIGHT,m_VideoFmt[pos].m_Height);
		m_YUY2Check.SetCheck(m_VideoFmt[pos].m_bForceYUY2 ? BST_CHECKED : BST_UNCHECKED);
		m_SampleFormat.SetCurSel(m_VideoFmt[pos].m_FieldFmt);

		///@todo update the range of the spinboxes with proper values
		m_SpinWidth.SetRange(16,DSCALER_MAX_WIDTH);
		m_SpinHeight.SetRange(2,DSCALER_MAX_HEIGHT);
	}
	else
	{
		::EnableWindow(::GetDlgItem(m_hWnd,IDC_DSHOW_VIDEOFMTS_NAME),FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd,IDC_DSHOW_VIDEOFMTS_WIDTH),FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd,IDC_DSHOW_VIDEOFMTS_WIDTH_SPIN),FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd,IDC_DSHOW_VIDEOFMTS_HEIGHT_SPIN),FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd,IDC_DSHOW_VIDEOFMTS_HEIGHT),FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd,IDC_DSHOW_VIDEOFMTS_FIELDFMT),FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd,IDC_DSHOW_VIDEOFMTS_YUY2),FALSE);
	}
	m_bInUpdateControlls=false;
}

void CDSVideoFormatPage::GenerateName(int pos)
{
	if(m_bInUpdateControlls)
	{
		return;
	}

	TRACE(_T("%s(%d) : CDSVideoFormatPage::GenerateName\n"),__FILE__,__LINE__);
	CString name;
	name.Format("%ldx%ld",m_VideoFmt[pos].m_Width,m_VideoFmt[pos].m_Height);

	if(m_VideoFmt[pos].m_bForceYUY2 || m_VideoFmt[pos].m_FieldFmt!=DSREND_FIELD_FORMAT_AUTO)
	{
		name+=" (";
		if(m_VideoFmt[pos].m_bForceYUY2)
		{
			name+="YUY2";
		}
		
		if(m_VideoFmt[pos].m_bForceYUY2 && m_VideoFmt[pos].m_FieldFmt!=DSREND_FIELD_FORMAT_AUTO)
		{
			name+=",";
		}
		
		if(m_VideoFmt[pos].m_FieldFmt==DSREND_FIELD_FORMAT_FRAME)
		{
			name+="Frame";
		}
		else if(m_VideoFmt[pos].m_FieldFmt==DSREND_FIELD_FORMAT_FIELD)
		{
			name+="Field";
		}
		
		name+=")";
	}

	SetDlgItemText(IDC_DSHOW_VIDEOFMTS_NAME,name);
}

void CDSVideoFormatPage::OnDeltaPosWidth(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TRACE(_T("%s(%d) : CDSVideoFormatPage::OnDeltaPosWidth\n"),__FILE__,__LINE__);
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	
	//spin to nearest multiple of 16
	if(pNMUpDown->iDelta>0)
	{
		//spin up
		pNMUpDown->iDelta=pNMUpDown->iDelta*16-(pNMUpDown->iPos%16);
	}
	else
	{
		//spin down
		pNMUpDown->iDelta=pNMUpDown->iDelta*16+(pNMUpDown->iPos%16!=0 ? 16-(pNMUpDown->iPos%16) : 0);
	}

	*pResult = 0;
}

void CDSVideoFormatPage::OnDeltaPosHeight(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TRACE(_T("%s(%d) : CDSVideoFormatPage::OnDeltaPosHeight\n"),__FILE__,__LINE__);
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	
	//spin to nearest multiple of 2
	if(pNMUpDown->iDelta>0)
	{
		//spin up
		pNMUpDown->iDelta=pNMUpDown->iDelta*2-(pNMUpDown->iPos%2);
	}
	else
	{
		//spin down
		pNMUpDown->iDelta=pNMUpDown->iDelta*2+(pNMUpDown->iPos%2!=0 ? 2-(pNMUpDown->iPos%2) : 0);
	}

	*pResult = 0;
}

void CDSVideoFormatPage::OnChangeWidth()
{
	if(m_bInUpdateControlls)
	{
		return;
	}
	
	TRACE(_T("%s(%d) : CDSVideoFormatPage::OnChangeWidth\n"),__FILE__,__LINE__);
	if(m_hWnd!=NULL && m_ListBox.m_hWnd!=NULL)
	{
		int width=GetDlgItemInt(IDC_DSHOW_VIDEOFMTS_WIDTH);
		///@todo check that the width is valid

		int CurSel=m_ListBox.GetCurSel();
		if(CurSel!=LB_ERR)
		{
			DWORD pos=m_ListBox.GetItemData(CurSel);
			m_VideoFmt[pos].m_Width=width;
			
			GenerateName(pos);
		}
	}
}

void CDSVideoFormatPage::OnChangeHeight()
{
	if(m_bInUpdateControlls)
	{
		return;
	}
	
	TRACE(_T("%s(%d) : CDSVideoFormatPage::OnChangeHeight\n"),__FILE__,__LINE__);
	if(m_hWnd!=NULL && m_ListBox.m_hWnd!=NULL)
	{
		int height=GetDlgItemInt(IDC_DSHOW_VIDEOFMTS_HEIGHT);
		///@todo check that the height is valid

		int CurSel=m_ListBox.GetCurSel();
		if(CurSel!=LB_ERR)
		{
			DWORD pos=m_ListBox.GetItemData(CurSel);
			m_VideoFmt[pos].m_Height=height;
			
			GenerateName(pos);
		}
	}
}

void CDSVideoFormatPage::OnChangeName()
{
	if(m_bInUpdateControlls)
	{
		return;
	}

	TRACE(_T("%s(%d) : CDSVideoFormatPage::OnChangeName\n"),__FILE__,__LINE__);
	int CurSel=m_ListBox.GetCurSel();
	if(CurSel!=LB_ERR)
	{
		DWORD pos=m_ListBox.GetItemData(CurSel);
		CString name;
		GetDlgItemText(IDC_DSHOW_VIDEOFMTS_NAME,name);
		
		//make sure control characters can't be used
		if(name.Replace("&","")!=0 || name.Replace("#","")!=0)
		{
			SetDlgItemText(IDC_DSHOW_VIDEOFMTS_NAME,name);
		}

		m_VideoFmt[pos].m_Name=name;
		
		//replace item in the listbox with the new name
		m_ListBox.DeleteString(CurSel);
		CurSel=m_ListBox.InsertString(CurSel,name);
		m_ListBox.SetItemData(CurSel,pos);
		m_ListBox.SetCurSel(CurSel);
	}
}

void CDSVideoFormatPage::OnClickedYUY2()
{
	if(m_bInUpdateControlls)
	{
		return;
	}

	int CurSel=m_ListBox.GetCurSel();
	if(CurSel!=LB_ERR)
	{
		DWORD pos=m_ListBox.GetItemData(CurSel);
		m_VideoFmt[pos].m_bForceYUY2=m_YUY2Check.GetCheck()==BST_CHECKED;
		GenerateName(pos);
	}
}

void CDSVideoFormatPage::OnSelEndOkFieldFmt()
{
	if(m_bInUpdateControlls)
	{
		return;
	}

	int CurSel=m_ListBox.GetCurSel();
	if(CurSel!=LB_ERR)
	{
		DWORD pos=m_ListBox.GetItemData(CurSel);
		m_VideoFmt[pos].m_FieldFmt=(DSREND_FIELD_FORMAT)m_SampleFormat.GetItemData(m_SampleFormat.GetCurSel());
		GenerateName(pos);
	}
}

void CDSVideoFormatPage::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	TRACE(_T("%s(%d) : CDSVideoFormatPage::OnContextMenu\n"),__FILE__,__LINE__);
	CPoint p;
	GetCursorPos(&p);
	m_ListBox.ScreenToClient(&p);
	CRect rect;
	m_ListBox.GetClientRect(&rect);

	//is the click inside the listbox?
	if(rect.PtInRect(p)==TRUE)
	{
		CMenu popup;
		popup.LoadMenu(IDC_DSHOW_VIDEOFMTS_CONTEXTMENU);

		//enable/disable delete menu entry
		BOOL bOutside;
		UINT item=m_ListBox.ItemFromPoint(p,bOutside);
		if(bOutside==FALSE)
		{
			popup.EnableMenuItem(IDC_DSHOW_VIDEOFMTS_DELETE,MF_BYCOMMAND|MF_ENABLED);
			m_ListBox.SetCurSel(item);
		}
		else
		{
			popup.EnableMenuItem(IDC_DSHOW_VIDEOFMTS_DELETE,MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
		}
		SetForegroundWindow();
		GetCursorPos(&p);
		popup.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN,p.x, p.y,this);
	}
}

void CDSVideoFormatPage::OnClickedDelete()
{
	TRACE(_T("%s(%d) : CDSVideoFormatPage::OnClickedDelete\n"),__FILE__,__LINE__);
	
	int CurSel=m_ListBox.GetCurSel();
	if(CurSel!=LB_ERR)
	{
		DWORD pos=m_ListBox.GetItemData(CurSel);
		
		//delete item from listbox and select a new item
		m_ListBox.DeleteString(CurSel);
		if(CurSel>=m_ListBox.GetCount())
		{
			m_ListBox.SetCurSel(m_ListBox.GetCount()-1);
		}
		else
		{
			m_ListBox.SetCurSel(CurSel);
		}
		UpdateControlls();
	}
}

void CDSVideoFormatPage::OnClickedNew()
{
	TRACE(_T("%s(%d) : CDSVideoFormatPage::OnClickedNew\n"),__FILE__,__LINE__);

	CDShowGraph::CVideoFormat NewFormat;
	NewFormat.m_Name="<New Item>";
	m_VideoFmt.push_back(NewFormat);
	int pos=m_ListBox.AddString(NewFormat.m_Name.c_str());
	m_ListBox.SetItemData(pos,m_VideoFmt.size()-1);
}

void CDSVideoFormatPage::OnClickedClear()
{
	TRACE(_T("%s(%d) : CDSVideoFormatPage::OnClickedClear\n"),__FILE__,__LINE__);

	//remove everything
	m_ListBox.ResetContent();
	m_VideoFmt.erase(m_VideoFmt.begin(),m_VideoFmt.end());
	UpdateControlls();
}

#endif