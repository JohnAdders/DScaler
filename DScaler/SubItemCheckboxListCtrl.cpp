/////////////////////////////////////////////////////////////////////////////
// $Id: SubItemCheckboxListCtrl.cpp,v 1.2 2004-08-14 13:45:23 adcockj Exp $
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
//  This file is part of the SettingRepository module.  See
//  SettingRepository.h for more information.
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.1  2004/08/12 14:06:32  atnak
// A CListCtrl delivative for having checkbox subitems.
//
//////////////////////////////////////////////////////////////////////////////

/**
* @file SubItemCheckboxListCtrl.cpp Implementation for CSubItemCheckboxListCtrl class
*/

#include "stdafx.h"
#include "SubItemCheckboxListCtrl.h"


//////////////////////////////////////////////////////////////////////////
// CSubItemCheckboxListCtrl
//////////////////////////////////////////////////////////////////////////

CSubItemCheckboxListCtrl::CSubItemCheckboxListCtrl(UINT styles) :
	CListCtrl(),
	m_queryProc(NULL),
	m_styles(styles),
	m_removeOwnerDraw(FALSE),
	m_iHitItem(-1)
{
}


CSubItemCheckboxListCtrl::CSubItemCheckboxListCtrl(PSCBQUERYPROC proc, PVOID context,
												   UINT styles) :
	CListCtrl(),
	m_queryProc(proc),
	m_callbackContext(context),
	m_styles(styles),
	m_removeOwnerDraw(FALSE),
	m_iHitItem(-1)
{
}


BOOL CSubItemCheckboxListCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	// The owner draw style is inserted at creation and remove straight afterwards
	// in WM_CREATE after the default OnCreate() has been called.  It needs to be
	// set for this short period so that WM_MEASUREITEM is used.  The WM_MEASUREITEM
	// message allows for the setting of a custom row height.  LVS_OWNERDRAWFIXED
	// cannot be left set when the control is repainted since no code has been set up
	// to perform this task.  Setting LVS_OWNERDRAWFIXED in WM_CREATE just before the
	// default OnCreate() call doesn't work.
	m_removeOwnerDraw = !(dwStyle & LVS_OWNERDRAWFIXED);
	return CListCtrl::Create(dwStyle|LVS_OWNERDRAWFIXED, rect, pParentWnd, nID);
}


void CSubItemCheckboxListCtrl::DrawSubItem(HDC hdc, int iItem, int iSubItem, BOOL bEraseBkgnd)
{
	UINT style = SendSCBQuery(SCBQ_GET_CHECKBOX, iItem, iSubItem);
	if (style & SCBR_CHECK_SHOW)
	{
		DrawSubItem(hdc, iItem, iSubItem, style, bEraseBkgnd);
	}
}


void CSubItemCheckboxListCtrl::DrawSubItem(HDC hdc, int iItem, int iSubItem, UINT style, BOOL bEraseBkgnd)
{
	CRect boundRect;

	// Get the bound rect for the subitem field.
	GetSubItemRect(iItem, iSubItem, LVIR_BOUNDS, boundRect);

	// Get the standard width and height of a check box.
	int cx = GetSystemMetrics(SM_CXMENUCHECK);
	int cy = GetSystemMetrics(SM_CYMENUCHECK);

	// Create a rect for the check box
	CRect checkRect(CPoint(boundRect.left, boundRect.top), CSize(cx, cy));

	// Center the check box rect within the bound rect.
	if (boundRect.Width() > cx)
	{
		checkRect.OffsetRect((boundRect.Width() - cx) / 2, 0);
	}
	if (boundRect.Height() > cy)
	{
		checkRect.OffsetRect(0, (boundRect.Height() - cy) / 2);
	}

	// Save the DC because clipping will be changed.
	int savedDC = SaveDC(hdc);

	// Erase the background if necessary.
	if (bEraseBkgnd)
	{
		HBRUSH hBrush;
		if (GetFocus() == this && (GetItemState(iItem, LVIS_SELECTED) & LVIS_SELECTED))
		{
			hBrush = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
		}
		else
		{
			// Striping is only supported if the background is white by default.
			// Otherwise it's impossible to tell what colour will work.
			if (m_styles & SCBS_VISUAL_STRIPE &&
				iItem % 2 && GetSysColor(COLOR_WINDOW) == 0x00ffffff)
			{
				hBrush = CreateSolidBrush(0x00eeeeee);
			}
			else
			{
				hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
			}
		}

		// Exclude the rect for the check box when filling the rect
		ExcludeClipRect(hdc, checkRect.left, checkRect.top, checkRect.right, checkRect.bottom);

		// Fill the subitem's background
		FillRect(hdc, boundRect, hBrush);
		DeleteObject(hBrush);
	}

	UINT nState = (style & SCBR_CHECK_INTERMEDIATE) ? DFCS_BUTTON3STATE : DFCS_BUTTONCHECK;

	if (style & SCBR_CHECK_FLAT)
	{
		nState |= DFCS_FLAT;
	}
	if (style & SCBR_CHECK_INACTIVE)
	{
		nState |= DFCS_INACTIVE;
	}
	if (style & (SCBR_CHECK_CHECKED|SCBR_CHECK_INTERMEDIATE))
	{
		nState |= DFCS_CHECKED;
	}
	if (m_overHitItem && iItem == m_iHitItem && iSubItem == m_iHitSubItem)
	{
		nState |= DFCS_PUSHED;
	}

	// Clip the check box rect tightly because DrawFrameControl()
	// can be unpredictable.
	HRGN hrgn = CreateRectRgnIndirect(checkRect);
	SelectClipRgn(hdc, hrgn);
	DeleteObject(hrgn);

	// Paint the check box.
	DrawFrameControl(hdc, checkRect, DFC_BUTTON, nState);

	// Restore clipping information.
	if (savedDC != 0)
	{
		RestoreDC(hdc, savedDC);
	}
}


BOOL CSubItemCheckboxListCtrl::SendSCBQuery(UINT uMsg, int iItem, int iSubItem)
{
	if (m_queryProc != NULL)
	{
		// Send the message to the callback.
		return (m_queryProc)(uMsg, iItem, iSubItem, this, m_callbackContext);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
// CSubItemCheckboxListCtrl AFX Message Maps
//////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CSubItemCheckboxListCtrl, CListCtrl)
	ON_WM_CREATE()
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_WM_MEASUREITEM_REFLECT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////
// CSubItemCheckboxListCtrl AFX Message Handlers
//////////////////////////////////////////////////////////////////////////

int CSubItemCheckboxListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListCtrl::OnCreate(lpCreateStruct) != 0)
	{
		return -1;
	}

	if (m_removeOwnerDraw)
	{
		ModifyStyle(LVS_OWNERDRAWFIXED, 0);
	}

	return 0;
}


LRESULT CSubItemCheckboxListCtrl::OnSetFont(WPARAM wParam, LPARAM lParam)
{
	// Set the font as normal by using the default proc.
	LRESULT result = DefWindowProc(WM_SETFONT, wParam, lParam);

	// Remember if owner draw is initially set.
	m_removeOwnerDraw = !(GetStyle() & LVS_OWNERDRAWFIXED);

	// The font change will cause CListCtrl to re-adjust its own row heights.
	// Force the control resize so that WM_MEASUREITEM will be processed again.
	// LVS_OWNERDRAWFIXED needs to be set for WM_MEASUREITEM to be used, so turn
	// it on just for this part.
	if (m_removeOwnerDraw)
	{
		ModifyStyle(0, LVS_OWNERDRAWFIXED);
	}

	// Get the control's original dimensions.
	CRect rect;
	GetWindowRect(rect);

	// Force the window to resize.
	SetWindowPos(NULL, 0, 0, 0, 0,
		SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOREDRAW|SWP_NOOWNERZORDER|SWP_NOZORDER);
	SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(),
		SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOREDRAW|SWP_NOOWNERZORDER|SWP_NOZORDER);

	// Turn off owner draw if it's not actually used.
	if (m_removeOwnerDraw)
	{
		ModifyStyle(LVS_OWNERDRAWFIXED, 0);
	}
	return result;
}


void CSubItemCheckboxListCtrl::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	// The checkbox height is given by SM_CYMENUCHECK.  The height necessary
	// for encompassing the checkbox should be given by SM_CYMENUSIZE.
	UINT cy = GetSystemMetrics(SM_CYMENUSIZE);

	// Set the height to at least the height for displaying checkboxes.
	if (lpMeasureItemStruct->itemHeight < cy)
	{
		lpMeasureItemStruct->itemHeight = cy;
	}
}


void CSubItemCheckboxListCtrl::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLVCUSTOMDRAW lpNMCustomDraw = (LPNMLVCUSTOMDRAW)pNMHDR;

	int iItem = lpNMCustomDraw->nmcd.dwItemSpec;
	int iSubItem = lpNMCustomDraw->iSubItem;

	switch (lpNMCustomDraw->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;
	case CDDS_ITEMPREPAINT:
		if (m_styles & SCBS_VISUAL_STRIPE &&
			iItem % 2 && GetSysColor(COLOR_WINDOW) == 0x00ffffff)
		{
			lpNMCustomDraw->clrTextBk = 0x00eeeeee;
		}
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
		break;
	case CDDS_ITEMPREPAINT|CDDS_SUBITEM:
		if (iSubItem != 0)
		{
			UINT style = SendSCBQuery(SCBQ_GET_CHECKBOX, iItem, iSubItem);
			if (!(style & SCBR_CHECK_SHOW))
			{
				*pResult = CDRF_DODEFAULT;
				break;
			}
			DrawSubItem(lpNMCustomDraw->nmcd.hdc, iItem, iSubItem, style);
			*pResult = CDRF_SKIPDEFAULT;
		}
		break;
	default:
		*pResult = CDRF_DODEFAULT;
		break;
	}
}


void CSubItemCheckboxListCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();

	LVHITTESTINFO hitTestInfo;
    hitTestInfo.pt = (POINT)point;
    hitTestInfo.flags = 0;
    hitTestInfo.iSubItem = 0;
    hitTestInfo.iItem = 0;
	if (SubItemHitTest(&hitTestInfo) != -1)
	{
		if (hitTestInfo.flags & LVHT_ONITEM && hitTestInfo.iSubItem != 0)
		{
			UINT style = SendSCBQuery(SCBQ_GET_CHECKBOX, hitTestInfo.iItem, hitTestInfo.iSubItem);
			if (style & SCBR_CHECK_SHOW && !(style & SCBR_CHECK_INACTIVE))
			{
				m_iHitItem = hitTestInfo.iItem;
				m_iHitSubItem = hitTestInfo.iSubItem;
				GetSubItemRect(m_iHitItem, m_iHitSubItem, LVIR_BOUNDS, m_hitItemRect);
				m_overHitItem = TRUE;

				CDC* pDC = GetDC();
				DrawSubItem(pDC->m_hDC, m_iHitItem, m_iHitSubItem, style, FALSE);
				ReleaseDC(pDC);

				SetCapture();
			}
		}
	}

	// Remove all selections and focused items.
	SetItemState(-1, 0, LVIS_SELECTED);
	SetItemState(-1, 0, LVIS_FOCUSED);

	if (!(m_styles & SCBS_NO_SELECT) && hitTestInfo.iItem != -1)
	{
		// Select the clicked item.
		SetItemState(hitTestInfo.iItem, LVIS_SELECTED, LVIS_SELECTED);
		SetItemState(hitTestInfo.iItem, LVIS_FOCUSED, LVIS_FOCUSED);
	}
}


void CSubItemCheckboxListCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	OnLButtonDown(nFlags, point);
}


void CSubItemCheckboxListCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_iHitItem != -1)
	{
		BOOL ptInRect = m_hitItemRect.PtInRect(point);

		if (ptInRect != m_overHitItem)
		{
			m_overHitItem = ptInRect;

			CDC* pDC = GetDC();
			DrawSubItem(pDC->m_hDC, m_iHitItem, m_iHitSubItem, FALSE);
			ReleaseDC(pDC);
		}
	}
	else
	{
		CListCtrl::OnMouseMove(nFlags, point);
	}
}


void CSubItemCheckboxListCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_iHitItem != -1)
	{
		ReleaseCapture();
		m_overHitItem = FALSE;

		if (m_hitItemRect.PtInRect(point))
		{
			SendSCBQuery(SCBQ_TOGGLE_CHECKBOX, m_iHitItem, m_iHitSubItem);
		}

		CDC* pDC = GetDC();
		DrawSubItem(pDC->m_hDC, m_iHitItem, m_iHitSubItem, FALSE);
		ReleaseDC(pDC);
		m_iHitItem = -1;
	}
}

