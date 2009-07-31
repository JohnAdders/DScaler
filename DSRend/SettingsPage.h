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
 * @file SettingsPage.h Declaration of the CSettingsPage
 */

#ifndef __SETTINGSPAGE_H_
#define __SETTINGSPAGE_H_

#include "resource.h"       // main symbols

EXTERN_C const CLSID CLSID_SettingsPage;

/////////////////////////////////////////////////////////////////////////////
// CSettingsPage
class ATL_NO_VTABLE CSettingsPage :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSettingsPage, &CLSID_SettingsPage>,
    public IPropertyPageImpl<CSettingsPage>,
    public CDialogImpl<CSettingsPage>
{
public:
    CSettingsPage()
    {
        m_dwTitleID = IDS_TITLESettingsPage;
        m_dwHelpFileID = IDS_HELPFILESettingsPage;
        m_dwDocStringID = IDS_DOCSTRINGSettingsPage;
    }

    enum {IDD = IDD_SETTINGSPAGE};

DECLARE_REGISTRY_RESOURCEID(IDR_SETTINGSPAGE)
DECLARE_NOT_AGGREGATABLE(CSettingsPage)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSettingsPage) 
    COM_INTERFACE_ENTRY(IPropertyPage)
END_COM_MAP()

BEGIN_MSG_MAP(CSettingsPage)
    CHAIN_MSG_MAP(IPropertyPageImpl<CSettingsPage>)
    COMMAND_HANDLER(IDC_SETTINGSPAGE_FIELDFORMAT, CBN_SELENDOK, OnSelEndOkFieldFormat)
    COMMAND_HANDLER(IDC_SETTINGSPAGE_FORCEYUY2, BN_CLICKED, OnClickedForceYUY2)
    COMMAND_HANDLER(IDC_SETTINGSPAGE_SWAPFIELDS,BN_CLICKED,OnClickedSwapFields)
    COMMAND_HANDLER(IDC_SETTINGSPAGE_VERTMIRROR,BN_CLICKED,OnClickedVertMirror)
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

    STDMETHOD(Activate(HWND hWndParent,LPCRECT pRect,BOOL bModal));
    STDMETHOD(Apply)(void);
    STDMETHOD(SetObjects(ULONG nObjects, IUnknown** ppUnk ));

private:
    LRESULT OnSelEndOkFieldFormat(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnClickedForceYUY2(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnClickedSwapFields(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnClickedVertMirror(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    CComPtr<IDSRendSettings> m_pSettings;
    BOOL m_bForceYUY2;
    BOOL m_bSwapFields;
    BOOL m_bVertMirror;
    DSREND_FIELD_FORMAT m_FieldFmt;
};

#endif //__SETTINGSPAGE_H_
