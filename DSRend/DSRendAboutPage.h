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
 * @file DSRendAboutPage.h Declaration of the CDSRendAboutPage
 */

#ifndef __DSRENDABOUTPAGE_H_
#define __DSRENDABOUTPAGE_H_

#include "resource.h"       // main symbols

EXTERN_C const CLSID CLSID_DSRendAboutPage;

/////////////////////////////////////////////////////////////////////////////
// CDSRendAboutPage
class ATL_NO_VTABLE CDSRendAboutPage :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CDSRendAboutPage, &CLSID_DSRendAboutPage>,
    public IPropertyPageImpl<CDSRendAboutPage>,
    public CDialogImpl<CDSRendAboutPage>
{
public:
    CDSRendAboutPage() 
    {
        m_dwTitleID = IDS_TITLEDSRendAboutPage;
        m_dwHelpFileID = IDS_HELPFILEDSRendAboutPage;
        m_dwDocStringID = IDS_DOCSTRINGDSRendAboutPage;
    }

    enum {IDD = IDD_DSRENDABOUTPAGE};

DECLARE_REGISTRY_RESOURCEID(IDR_DSRENDABOUTPAGE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDSRendAboutPage) 
    COM_INTERFACE_ENTRY(IPropertyPage)
END_COM_MAP()

BEGIN_MSG_MAP(CDSRendAboutPage)
    CHAIN_MSG_MAP(IPropertyPageImpl<CDSRendAboutPage>)
    //MESSAGE_HANDLER(WM_SIZE, OnSize)
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

    STDMETHOD(Activate(HWND hWndParent,LPCRECT pRect,BOOL bModal));
    STDMETHOD(Apply());
    //LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

#endif //__DSRENDABOUTPAGE_H_
