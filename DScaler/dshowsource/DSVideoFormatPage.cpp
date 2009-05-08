/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file DSVideoFormatPage.cpp implementation file
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT

#include "..\dscaler.h"
#include "DSVideoFormatPage.h"
#include "ioutput.h"
#include "DebugLog.h"
#include "DScaler.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CDSVideoFormatPage dialog


CDSVideoFormatPage::CDSVideoFormatPage(const tstring& name,vector<CDShowGraph::CVideoFormat>& fmts, CSliderSetting* pResolution) :
    CTreeSettingsPage(name, IDD_DSHOW_VIDEOFMTS),
    m_RealVideoFmt(fmts),
    m_bInUpdateControlls(FALSE),
    m_pResolutionSetting(pResolution),
    m_ListBox(NULL),
    m_SpinWidth(NULL),
    m_SpinHeight(NULL),
    m_VideoFmt(fmts)
{
}


BOOL CDSVideoFormatPage::ChildDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
    HANDLE_MSG(hDlg, WM_INITDIALOG, OnInitDialog);
    HANDLE_MSG(hDlg, WM_COMMAND, OnCommand);
    HANDLE_MSG(hDlg, WM_CONTEXTMENU, OnContextMenu);
    HANDLE_MSG(hDlg, WM_NOTIFY, OnNotify);
    default:
        return FALSE;
    }
}

LRESULT CDSVideoFormatPage::OnNotify(HWND hwnd, int id, LPNMHDR nmhdr)
{
    switch(id)
    {
    case IDC_DSHOW_VIDEOFMTS_WIDTH_SPIN:
        if(nmhdr->code == UDN_DELTAPOS)
        {
            return OnDeltaPosWidth(hwnd, nmhdr);
        }
        break;
    case IDC_DSHOW_VIDEOFMTS_HEIGHT_SPIN:
        if(nmhdr->code == UDN_DELTAPOS)
        {
            return OnDeltaPosHeight(hwnd, nmhdr);
        }
        break;
    default:
        break;
    }
    return FALSE;
}

void CDSVideoFormatPage::OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
    switch(id)
    {
    case IDC_DSHOW_VIDEOFMTS_FMTLIST:
        if(LBN_SELCHANGE == codeNotify)
        {
            OnVideoFmtSelChange(hDlg);
        }
        break;
    case IDC_DSHOW_VIDEOFMTS_WIDTH:
        if(EN_CHANGE == codeNotify)
        {
            OnChangeWidth(hDlg);
        }
        break;
    case IDC_DSHOW_VIDEOFMTS_HEIGHT:
        if(EN_CHANGE == codeNotify)
        {
            OnChangeHeight(hDlg);
        }
        break;
    case IDC_DSHOW_VIDEOFMTS_NAME:
        if(EN_CHANGE == codeNotify)
        {
            OnChangeName(hDlg);
        }
        break;
    case IDC_DSHOW_VIDEOFMTS_NEW:
        if(BN_CLICKED == codeNotify)
        {
            OnClickedNew(hDlg);
        }
        break;
    case IDC_DSHOW_VIDEOFMTS_DELETE:
        if(BN_CLICKED == codeNotify)
        {
            OnClickedDelete(hDlg);
        }
        break;
    case IDC_DSHOW_VIDEOFMTS_CLEAR:
        if(BN_CLICKED == codeNotify)
        {
            OnClickedClear(hDlg);
        }
        break;
    }
}

BOOL CDSVideoFormatPage::OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
    LOGD(_T("%s(%d) : CDSVideoFormatPage::OnInitDialog\n"),_T(__FILE__),__LINE__);

    m_ListBox = GetDlgItem(hDlg, IDC_DSHOW_VIDEOFMTS_FMTLIST);
    m_SpinWidth = GetDlgItem(hDlg, IDC_DSHOW_VIDEOFMTS_WIDTH_SPIN);
    m_SpinHeight = GetDlgItem(hDlg, IDC_DSHOW_VIDEOFMTS_HEIGHT_SPIN);
    
    for(vector<CDShowGraph::CVideoFormat>::size_type index=0;index<m_VideoFmt.size();index++)
    {
        int pos=ListBox_AddString(m_ListBox, m_VideoFmt[index].m_Name.c_str());
        if(pos!=LB_ERR)
        {
            int err=ListBox_SetItemData(m_ListBox, pos, index);
            if(err==LB_ERR)
            {
                //oops can't set item data
            }
        }
    }

    //select first entry
    ListBox_SetCurSel(m_ListBox, 0);
    UpdateControlls(hDlg);

    return TRUE;
}

void CDSVideoFormatPage::OnOK(HWND hDlg)
{
    LOGD(_T("%s(%d) : CDSVideoFormatPage::OnOK\n"),_T(__FILE__),__LINE__);

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
    for(int i=0;i<ListBox_GetCount(m_ListBox);i++)
    {
        int pos=ListBox_GetItemData(m_ListBox, i);
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

void CDSVideoFormatPage::OnVideoFmtSelChange(HWND hDlg)
{
    LOGD(_T("%s(%d) : CDSVideoFormatPage::OnVideoFmtSelChange\n"),__FILE__,__LINE__);
    if(ListBox_GetCurSel(m_ListBox)!=LB_ERR)
    {
        UpdateControlls(hDlg);
    }
}

void CDSVideoFormatPage::UpdateControlls(HWND hDlg)
{
    LOGD(_T("%s(%d) : CDSVideoFormatPage::UpdateControlls\n"),__FILE__,__LINE__);
    if(m_bInUpdateControlls)
    {
        return;
    }

    m_bInUpdateControlls=TRUE;
    int CurSel=ListBox_GetCurSel(m_ListBox);
    if(CurSel!=LB_ERR)
    {
        EnableWindow(GetDlgItem(hDlg,IDC_DSHOW_VIDEOFMTS_NAME),TRUE);
        EnableWindow(GetDlgItem(hDlg,IDC_DSHOW_VIDEOFMTS_WIDTH),TRUE);
        EnableWindow(GetDlgItem(hDlg,IDC_DSHOW_VIDEOFMTS_WIDTH_SPIN),TRUE);
        EnableWindow(GetDlgItem(hDlg,IDC_DSHOW_VIDEOFMTS_HEIGHT_SPIN),TRUE);
        EnableWindow(GetDlgItem(hDlg,IDC_DSHOW_VIDEOFMTS_HEIGHT),TRUE);
        EnableWindow(GetDlgItem(hDlg,IDC_DSHOW_VIDEOFMTS_FIELDFMT),TRUE);
        EnableWindow(GetDlgItem(hDlg,IDC_DSHOW_VIDEOFMTS_YUY2),TRUE);

        DWORD pos=ListBox_GetItemData(m_ListBox, CurSel);
        SetDlgItemText(hDlg, IDC_DSHOW_VIDEOFMTS_NAME,m_VideoFmt[pos].m_Name.c_str());
        SetDlgItemInt(hDlg, IDC_DSHOW_VIDEOFMTS_WIDTH,m_VideoFmt[pos].m_Width);
        SetDlgItemInt(hDlg, IDC_DSHOW_VIDEOFMTS_HEIGHT,m_VideoFmt[pos].m_Height);

        ///@todo update the range of the spinboxes with proper values
        SendMessage(m_SpinWidth, UDM_SETRANGE, 0, MAKELPARAM(DSCALER_MAX_WIDTH, 16));
        SendMessage(m_SpinHeight, UDM_SETRANGE, 0, MAKELPARAM(DSCALER_MAX_HEIGHT, 2));
    }
    else
    {
        EnableWindow(GetDlgItem(hDlg,IDC_DSHOW_VIDEOFMTS_NAME),FALSE);
        EnableWindow(GetDlgItem(hDlg,IDC_DSHOW_VIDEOFMTS_WIDTH),FALSE);
        EnableWindow(GetDlgItem(hDlg,IDC_DSHOW_VIDEOFMTS_WIDTH_SPIN),FALSE);
        EnableWindow(GetDlgItem(hDlg,IDC_DSHOW_VIDEOFMTS_HEIGHT_SPIN),FALSE);
        EnableWindow(GetDlgItem(hDlg,IDC_DSHOW_VIDEOFMTS_HEIGHT),FALSE);
        EnableWindow(GetDlgItem(hDlg,IDC_DSHOW_VIDEOFMTS_FIELDFMT),FALSE);
        EnableWindow(GetDlgItem(hDlg,IDC_DSHOW_VIDEOFMTS_YUY2),FALSE);
    }
    m_bInUpdateControlls=FALSE;
}

void CDSVideoFormatPage::GenerateName(HWND hDlg, int pos)
{
    if(m_bInUpdateControlls)
    {
        return;
    }

    LOGD(_T("%s(%d) : CDSVideoFormatPage::GenerateName\n"),__FILE__,__LINE__);
    
    tstring name(MakeString() <<  m_VideoFmt[pos].m_Width << _T("x") << m_VideoFmt[pos].m_Height);

    if(m_VideoFmt[pos].m_bForceYUY2 || m_VideoFmt[pos].m_FieldFmt!=DSREND_FIELD_FORMAT_AUTO)
    {
        name+=_T(" (");
        if(m_VideoFmt[pos].m_bForceYUY2)
        {
            name+=_T("YUY2");
        }

        if(m_VideoFmt[pos].m_bForceYUY2 && m_VideoFmt[pos].m_FieldFmt!=DSREND_FIELD_FORMAT_AUTO)
        {
            name+=_T(",");
        }

        if(m_VideoFmt[pos].m_FieldFmt==DSREND_FIELD_FORMAT_FRAME)
        {
            name+=_T("Frame");
        }
        else if(m_VideoFmt[pos].m_FieldFmt==DSREND_FIELD_FORMAT_FIELD)
        {
            name+=_T("Field");
        }

        name+=_T(")");
    }

    SetDlgItemText(hDlg, IDC_DSHOW_VIDEOFMTS_NAME, name.c_str());
}

LRESULT CDSVideoFormatPage::OnDeltaPosWidth(HWND hDlg, NMHDR* pNMHDR)
{
    LOGD(_T("%s(%d) : CDSVideoFormatPage::OnDeltaPosWidth\n"),__FILE__,__LINE__);
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

    return 0;
}

LRESULT CDSVideoFormatPage::OnDeltaPosHeight(HWND hDlg, NMHDR* pNMHDR)
{
    LOGD(_T("%s(%d) : CDSVideoFormatPage::OnDeltaPosHeight\n"),__FILE__,__LINE__);
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

    return 0;
}

void CDSVideoFormatPage::OnChangeWidth(HWND hDlg)
{
    if(m_bInUpdateControlls)
    {
        return;
    }

    LOGD(_T("%s(%d) : CDSVideoFormatPage::OnChangeWidth\n"),__FILE__,__LINE__);
    if(hDlg != NULL && m_ListBox != NULL)
    {
        int width=GetDlgItemInt(hDlg, IDC_DSHOW_VIDEOFMTS_WIDTH);
        ///@todo check that the width is valid

        int CurSel=ListBox_GetCurSel(m_ListBox);
        if(CurSel!=LB_ERR)
        {
            DWORD pos=ListBox_GetItemData(m_ListBox, CurSel);
            m_VideoFmt[pos].m_Width=width;

            GenerateName(hDlg, pos);
        }
    }
}

void CDSVideoFormatPage::OnChangeHeight(HWND hDlg)
{
    if(m_bInUpdateControlls)
    {
        return;
    }

    LOGD(_T("%s(%d) : CDSVideoFormatPage::OnChangeHeight\n"),__FILE__,__LINE__);
    if(hDlg!=NULL && m_ListBox!=NULL)
    {
        int height=GetDlgItemInt(hDlg, IDC_DSHOW_VIDEOFMTS_HEIGHT);
        ///@todo check that the height is valid

        int CurSel=ListBox_GetCurSel(m_ListBox);
        if(CurSel!=LB_ERR)
        {
            DWORD pos=ListBox_GetItemData(m_ListBox, CurSel);
            m_VideoFmt[pos].m_Height=height;

            GenerateName(hDlg, pos);
        }
    }
}

void CDSVideoFormatPage::OnChangeName(HWND hDlg)
{
    if(m_bInUpdateControlls)
    {
        return;
    }

    LOGD(_T("%s(%d) : CDSVideoFormatPage::OnChangeName\n"),__FILE__,__LINE__);
    int CurSel=ListBox_GetCurSel(m_ListBox);
    if(CurSel!=LB_ERR)
    {
        DWORD pos=ListBox_GetItemData(m_ListBox, CurSel);
        tstring name = GetDlgItemString(hDlg, IDC_DSHOW_VIDEOFMTS_NAME);

        //make sure control characters can't be used
        name = ReplaceCharWithString(name, '&',_T(""));
        name = ReplaceCharWithString(name, '#',_T(""));
        SetDlgItemText(hDlg, IDC_DSHOW_VIDEOFMTS_NAME, name.c_str());

        m_VideoFmt[pos].m_Name=name;

        //replace item in the listbox with the new name
        ListBox_DeleteString(m_ListBox, CurSel);
        CurSel=ListBox_InsertString(m_ListBox, CurSel, name.c_str());
        ListBox_SetItemData(m_ListBox, CurSel,pos);
        ListBox_SetCurSel(m_ListBox, CurSel);
    }
}

void CDSVideoFormatPage::OnContextMenu(HWND hDlg, HWND hwndContext, UINT xPos, UINT yPos)
{
    LOGD(_T("%s(%d) : CDSVideoFormatPage::OnContextMenu\n"),__FILE__,__LINE__);
    POINT p;
    GetCursorPos(&p);
    ScreenToClient(m_ListBox, &p);
    RECT rect;
    GetClientRect(m_ListBox, &rect);

    //is the click inside the listbox?
    if(PtInRect(&rect, p)==TRUE)
    {
        HMENU popup = LoadMenu(hResourceInst, MAKEINTRESOURCE(IDC_DSHOW_VIDEOFMTS_CONTEXTMENU));

        //enable/disable delete menu entry
        DWORD result=SendMessage(m_ListBox, LB_ITEMFROMPOINT, 0, MAKELPARAM(p.x, p.y));
        if(!(HIWORD(result)))
        {
            EnableMenuItem(popup, IDC_DSHOW_VIDEOFMTS_DELETE,MF_BYCOMMAND|MF_ENABLED);
            ListBox_SetCurSel(m_ListBox, LOWORD(result));
        }
        else
        {
            EnableMenuItem(popup, IDC_DSHOW_VIDEOFMTS_DELETE,MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
        }
        SetForegroundWindow(hDlg);
        GetCursorPos(&p);
        TrackPopupMenu(GetSubMenu(popup, 0), TPM_LEFTALIGN, p.x, p.y, 0, hDlg, 0);
    }
}

void CDSVideoFormatPage::OnClickedDelete(HWND hDlg)
{
    LOGD(_T("%s(%d) : CDSVideoFormatPage::OnClickedDelete\n"),__FILE__,__LINE__);

    int CurSel=ListBox_GetCurSel(m_ListBox);
    if(CurSel!=LB_ERR)
    {
        DWORD pos=ListBox_GetItemData(m_ListBox, CurSel);

        //delete item from listbox and select a new item
        ListBox_DeleteString(m_ListBox, CurSel);
        if(CurSel >= ListBox_GetCount(m_ListBox))
        {
            ListBox_SetCurSel(m_ListBox, ListBox_GetCount(m_ListBox)-1);
        }
        else
        {
            ListBox_SetCurSel(m_ListBox, CurSel);
        }
        UpdateControlls(hDlg);
    }
}

void CDSVideoFormatPage::OnClickedNew(HWND hDlg)
{
    LOGD(_T("%s(%d) : CDSVideoFormatPage::OnClickedNew\n"),__FILE__,__LINE__);

    CDShowGraph::CVideoFormat NewFormat;
    NewFormat.m_Name=_T("<New Item>");
    m_VideoFmt.push_back(NewFormat);
    int pos=ListBox_AddString(m_ListBox, NewFormat.m_Name.c_str());
    ListBox_SetItemData(m_ListBox, pos, m_VideoFmt.size()-1);
}

void CDSVideoFormatPage::OnClickedClear(HWND hDlg)
{
    LOGD(_T("%s(%d) : CDSVideoFormatPage::OnClickedClear\n"),__FILE__,__LINE__);

    //remove everything
    ListBox_ResetContent(m_ListBox);
    m_VideoFmt.erase(m_VideoFmt.begin(),m_VideoFmt.end());
    UpdateControlls(hDlg);
}

#endif
