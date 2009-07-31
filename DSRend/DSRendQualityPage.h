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
 * @file DSRendQualityPage.h Declaration of the CDSRendQualityPage
 */

#ifndef __DSRENDQUALITYPAGE_H_
#define __DSRENDQUALITYPAGE_H_

#include "resource.h"       // main symbols

EXTERN_C const CLSID CLSID_DSRendQualityPage;


/**
 * Propertypage for the filter.
 */
class ATL_NO_VTABLE CDSRendQualityPage :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CDSRendQualityPage, &CLSID_DSRendQualityPage>,
    public IPropertyPageImpl<CDSRendQualityPage>,
    public CDialogImpl<CDSRendQualityPage>
{
public:
    CDSRendQualityPage() :
        m_qualDrawnStart(0),
        m_qualDroppedStart(0),
        m_UpStreamDrawnStart(0),
        m_UpStreamDroppedStart(0)
    {
        m_dwTitleID = IDS_TITLEDSRendQualityPage;
        m_dwHelpFileID = IDS_HELPFILEDSRendQualityPage;
        m_dwDocStringID = IDS_DOCSTRINGDSRendQualityPage;
    }
    HRESULT FinalConstruct();
    HRESULT FinalRelease();

    enum {IDD = IDD_DSRENDQUALITYPAGE};

DECLARE_REGISTRY_RESOURCEID(IDR_DSRENDQUALITYPAGE)
DECLARE_NOT_AGGREGATABLE(CDSRendQualityPage)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDSRendQualityPage) 
    COM_INTERFACE_ENTRY(IPropertyPage)
END_COM_MAP()

BEGIN_MSG_MAP(CDSRendQualityPage)
    CHAIN_MSG_MAP(IPropertyPageImpl<CDSRendQualityPage>)
    MESSAGE_HANDLER(WM_TIMER, OnTimer)
    //MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
    //MESSAGE_HANDLER(WM_CTLCOLORSTATIC,OnCtlColorStatic)
    COMMAND_HANDLER(IDC_RESET, BN_CLICKED, OnClickedReset)
    COMMAND_HANDLER(IDC_RESTORE, BN_CLICKED, OnClickedRestore)
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

    STDMETHOD(Activate(HWND hWndParent,LPCRECT pRect,BOOL bModal));
    STDMETHOD(Deactivate());
    STDMETHOD(Apply());
    STDMETHOD(SetObjects(ULONG nObjects, IUnknown** ppUnk ));

private:
    void updateDialog();
    void findUpstreamFilter();
    CComPtr<IQualProp> m_pQuality;
    int m_qualDrawnStart;
    int m_qualDroppedStart;

    CComPtr<IAMDroppedFrames> m_pUpstreamDF;
    long m_UpStreamDrawnStart;
    long m_UpStreamDroppedStart;
    LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //LRESULT OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnClickedReset(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnClickedRestore(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};

#endif //__DSRENDQUALITYPAGE_H_
