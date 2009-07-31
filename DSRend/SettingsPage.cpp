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
 * @file SettingsPage.cpp Implementation of CSettingsPage
 */

#include "stdafx.h"
#include "DSRend.h"
#include "SettingsPage.h"


STDMETHODIMP CSettingsPage::Activate(HWND hWndParent,LPCRECT pRect,BOOL bModal)
{
    ATLTRACE(_T("%s(%d) : CSettingsPage::Activate\n"),__FILE__,__LINE__);
    HRESULT hr=IPropertyPageImpl<CSettingsPage>::Activate(hWndParent,pRect,bModal);
    if(FAILED(hr))
    {
        return hr;
    }

    if(m_hWnd!=NULL && m_pSettings!=NULL)
    {
        hr=m_pSettings->get_FieldFormat(&m_FieldFmt);
        if(FAILED(hr))
        {
            return hr;
        }
        hr=m_pSettings->get_ForceYUY2(&m_bForceYUY2);
        if(FAILED(hr))
        {
            return hr;
        }
        hr=m_pSettings->get_SwapFields(&m_bSwapFields);
        if(FAILED(hr))
        {
            return hr;
        }
        hr=m_pSettings->get_VertMirror(&m_bVertMirror);
        if(FAILED(hr))
        {
            return hr;
        }

        HWND hFieldFormat=GetDlgItem(IDC_SETTINGSPAGE_FIELDFORMAT);
        if(hFieldFormat!=NULL)
        {
            LRESULT index=SendMessage(hFieldFormat,CB_ADDSTRING,0,(LPARAM)_T("Auto"));
            SendMessage(hFieldFormat,CB_SETITEMDATA,index,DSREND_FIELD_FORMAT_AUTO);
            if(m_FieldFmt==DSREND_FIELD_FORMAT_AUTO)
            {
                SendMessage(hFieldFormat,CB_SETCURSEL,index,0);
            }

            index=SendMessage(hFieldFormat,CB_ADDSTRING,0,(LPARAM)_T("Frame"));
            SendMessage(hFieldFormat,CB_SETITEMDATA,index,DSREND_FIELD_FORMAT_FRAME);
            if(m_FieldFmt==DSREND_FIELD_FORMAT_FRAME)
            {
                SendMessage(hFieldFormat,CB_SETCURSEL,index,0);
            }

            index=SendMessage(hFieldFormat,CB_ADDSTRING,0,(LPARAM)_T("Field"));
            SendMessage(hFieldFormat,CB_SETITEMDATA,index,DSREND_FIELD_FORMAT_FIELD);
            if(m_FieldFmt==DSREND_FIELD_FORMAT_FIELD)
            {
                SendMessage(hFieldFormat,CB_SETCURSEL,index,0);
            }
        }

        HWND hCheck=GetDlgItem(IDC_SETTINGSPAGE_FORCEYUY2);
        if(hCheck!=NULL)
        {
            SendMessage(hCheck,BM_SETCHECK,m_bForceYUY2==TRUE ? BST_CHECKED : BST_UNCHECKED,0);
        }
        hCheck=GetDlgItem(IDC_SETTINGSPAGE_SWAPFIELDS);
        if(hCheck!=NULL)
        {
            SendMessage(hCheck,BM_SETCHECK,m_bSwapFields==TRUE ? BST_CHECKED : BST_UNCHECKED,0);
        }
        hCheck=GetDlgItem(IDC_SETTINGSPAGE_VERTMIRROR);
        if(hCheck!=NULL)
        {
            SendMessage(hCheck,BM_SETCHECK,m_bVertMirror==TRUE ? BST_CHECKED : BST_UNCHECKED,0);
        }
        

        DSRendStatus status;
        hr=m_pSettings->get_Status(&status);

        if(hr==VFW_E_NOT_CONNECTED)
        {
            SetDlgItemText(IDC_SETTINGSPAGE_FIELD,_T("Not Connected"));
            SetDlgItemText(IDC_SETTINGSPAGE_COLORCNV,_T("Not Connected"));
            SetDlgItemText(IDC_SETTINGSPAGE_SIZE,_T("Not Connected"));
        }
        else if(FAILED(hr))
        {
            SetDlgItemText(IDC_SETTINGSPAGE_FIELD,_T("Err"));
            SetDlgItemText(IDC_SETTINGSPAGE_COLORCNV,_T("Err"));
            SetDlgItemText(IDC_SETTINGSPAGE_SIZE,_T("Err"));
        }
        else
        {
            SetDlgItemText(IDC_SETTINGSPAGE_FIELD,status.bFieldInput==TRUE ? _T("Field") : _T("Frame"));
            SetDlgItemText(IDC_SETTINGSPAGE_COLORCNV,status.bNeedConv==TRUE ? _T("Yes") : _T("No"));
            TCHAR size[20];
            wsprintf(size,_T("%lix%li"),status.Width,status.Height);
            SetDlgItemText(IDC_SETTINGSPAGE_SIZE,size);

        }

    }

    return S_OK;
}

STDMETHODIMP CSettingsPage::Apply()
{
    ATLTRACE(_T("%s(%d) : CSettingsPage::Apply\n"),__FILE__,__LINE__);
    if(m_bDirty==TRUE)
    {
        HRESULT hr=m_pSettings->put_ForceYUY2(m_bForceYUY2);
        if(FAILED(hr))
        {
            return hr;
        }
        hr=m_pSettings->put_FieldFormat(m_FieldFmt);
        if(FAILED(hr))
        {
            return hr;
        }
        hr=m_pSettings->put_SwapFields(m_bSwapFields);
        if(FAILED(hr))
        {
            return hr;
        }
        hr=m_pSettings->put_VertMirror(m_bVertMirror);
        if(FAILED(hr))
        {
            return hr;
        }
        m_bDirty = FALSE;
        return S_OK;
    }
    else
    {
        return E_UNEXPECTED;
    }
}

STDMETHODIMP CSettingsPage::SetObjects( ULONG nObjects, IUnknown** ppUnk )
{
    ATLTRACE(_T("%s(%d) : CSettingsPage::SetObjects\n"),__FILE__,__LINE__);
    if(nObjects==0)
    {
        if(m_pSettings==NULL)
        {
            return E_UNEXPECTED;
        }
        else
        {
            m_pSettings=NULL;
            return S_OK;
        }
    }
    else
    {
        if(ppUnk==NULL)
            return E_POINTER;
        if(nObjects!=1)
            return E_FAIL;

        HRESULT hr=(*ppUnk)->QueryInterface(IID_IDSRendSettings,(void **)&m_pSettings);
        if(FAILED(hr))
        {
            return E_NOINTERFACE;
        }
        return IPropertyPageImpl<CSettingsPage>::SetObjects(nObjects,ppUnk);
    }
}

LRESULT CSettingsPage::OnSelEndOkFieldFormat(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    ATLTRACE(_T("%s(%d) : CSettingsPage::OnSelEndOkFieldFormat\n"),__FILE__,__LINE__);
    HWND hCombo=GetDlgItem(IDC_SETTINGSPAGE_FIELDFORMAT);
    if(hCombo!=NULL)
    {
        LRESULT index=SendMessage(hCombo,CB_GETCURSEL,0,0);
        if(index!=CB_ERR)
        {
            LRESULT data=SendMessage(hCombo,CB_GETITEMDATA,index,0);
            if(data!=CB_ERR)
            {
                m_FieldFmt=(DSREND_FIELD_FORMAT)data;
                SetDirty(TRUE);
            }
        }
    }
    return 0;
}

LRESULT CSettingsPage::OnClickedForceYUY2(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    ATLTRACE(_T("%s(%d) : CSettingsPage::OnClickedForceYUY2\n"),__FILE__,__LINE__);
    HWND hCheck=GetDlgItem(IDC_SETTINGSPAGE_FORCEYUY2);
    if(hCheck!=NULL)
    {
        LRESULT check=SendMessage(hCheck,BM_GETCHECK,0,0);
        m_bForceYUY2= check==BST_CHECKED ? TRUE : FALSE;
        SetDirty(TRUE);
    }
    return 0;
}

LRESULT CSettingsPage::OnClickedSwapFields(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    ATLTRACE(_T("%s(%d) : CSettingsPage::OnClickedSwapFields\n"),__FILE__,__LINE__);
    HWND hCheck=GetDlgItem(IDC_SETTINGSPAGE_SWAPFIELDS);
    if(hCheck!=NULL)
    {
        LRESULT check=SendMessage(hCheck,BM_GETCHECK,0,0);
        m_bSwapFields= check==BST_CHECKED ? TRUE : FALSE;
        SetDirty(TRUE);
    }
    return 0;
}

LRESULT CSettingsPage::OnClickedVertMirror(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    ATLTRACE(_T("%s(%d) : CSettingsPage::OnClickedVertMirror\n"),__FILE__,__LINE__);
    HWND hCheck=GetDlgItem(IDC_SETTINGSPAGE_VERTMIRROR);
    if(hCheck!=NULL)
    {
        LRESULT check=SendMessage(hCheck,BM_GETCHECK,0,0);
        m_bVertMirror= check==BST_CHECKED ? TRUE : FALSE;
        SetDirty(TRUE);
    }
    return 0;
}