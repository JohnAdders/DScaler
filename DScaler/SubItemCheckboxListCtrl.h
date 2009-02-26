/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004 Atsushi Nakagawa.  All rights reserved.
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
* @file SubItemCheckboxListCtrl.h Header file for CSubItemCheckboxListCtrl class
*/

#ifndef __SUBITEMCHECKBOXLISTCTRL_H__
#define __SUBITEMCHECKBOXLISTCTRL_H__

//
// Description:
//
// CSubItemCheckboxListCtrl is a derivative of CListCtrl that allows
// checkboxes to be used for subitems.
//


enum eSubItemCheckboxStyle
{
    SCBS_NO_SELECT            = 1 << 0,
    SCBS_VISUAL_STRIPE        = 1 << 1,
};

enum eSubItemCheckboxQueryGetReponse
{
    // The checkbox shown.
    SCBR_CHECK_SHOW            = 1 << 0,
    // The checkbox is drawn in the flat style.
    SCBR_CHECK_FLAT            = 1 << 1,
    // The checkbox is inactive.
    SCBR_CHECK_INACTIVE        = 1 << 2,
    // The checkbox is checked.
    SCBR_CHECK_CHECKED        = 1 << 3,
    // The checkbox is checked in the intermediate state.
    SCBR_CHECK_INTERMEDIATE    = 1 << 4,
};

enum eSubItemCheckboxQuery
{
    // Return combinations of the eSubItemCheckboxQueryGetReponse flags.
    SCBQ_GET_CHECKBOX        = 1,
    // The owner should do what ever is necessary to toggle the checkbox.  The
    // checkbox is repainted when the callback returns.  Return value is ignored.
    SCBQ_TOGGLE_CHECKBOX    = 2,
};


typedef class CSubItemCheckboxListCtrl *PSCBLISTCTRL;

// Callback proc for receiving CSubItemCheckboxListCtrl's messages.
typedef UINT (*PSCBQUERYPROC)(UINT query, int iItem, int iSubItem,
                              PSCBLISTCTRL pList, PVOID context);


//////////////////////////////////////////////////////////////////////////
// CSubItemCheckboxListCtrl
//////////////////////////////////////////////////////////////////////////
class CSubItemCheckboxListCtrl : public CListCtrl
{
public:
    CSubItemCheckboxListCtrl(UINT styles = 0);
    CSubItemCheckboxListCtrl(PSCBQUERYPROC proc, PVOID context, UINT styles = 0);
    virtual ~CSubItemCheckboxListCtrl() { };

    virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

    // Draws the subitem to the given device context.
    virtual void DrawSubItem(HDC hdc, int iItem, int iSubItem, BOOL bEraseBkgnd = TRUE);

protected:
    //////////////////////////////////////////////////////////////////////////
    // AFX Message Maps
    //////////////////////////////////////////////////////////////////////////
    // Called when the control is created.
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    // Used for forcing WM_MEASUREITEM when the font changes.
    afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM lParam);
    // Use to set a custom row height for the list control.
    afx_msg void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
    // Called on the NM_CUSTOMDRAW notify reflection.
    afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
    // Called when the left mouse button is pressed in the control.
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    // Called the second time the left mouse button is pressed.
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    // Called when the left mouse button is released.
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    // Called when the mouse is moved over the control.
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);

    // Define some AFX stuff that are necessary for maps.
    DECLARE_MESSAGE_MAP()

protected:
    // Draws the subitem to the given device context.
    virtual void DrawSubItem(HDC hdc, int iItem, int iSubItem, UINT style, BOOL bEraseBkgnd = TRUE);
    // Sends the message to the callback associated with the class.
    virtual BOOL SendSCBQuery(UINT query, int iItem, int iSubItem);

protected:
    // Callback to call with SendSCBMessage().
    PSCBQUERYPROC        m_queryProc;
    PVOID                m_callbackContext;

    UINT                m_styles;
    BOOL                m_removeOwnerDraw;

    // Values used when tracking mouse clicks.
    int                    m_iHitItem;
    int                    m_iHitSubItem;
    CRect                m_hitItemRect;
    BOOL                m_overHitItem;
};


#endif


