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
 * @file TreeSettingsDlg.h header file for tree settings dialog
 */

#if !defined(_TREESETTINGSDLG_H__)
#define _TREESETTINGSDLG_H__

// TreeSettingsDlg.h : header file
//

#include <vector>
#include "Setting.h"
#include "DSDialog.h"
#include "TreeSettingsPage.h"
#include "..\DScalerRes\resource.h"

#define FILTER_SETTINGS_MASK        0x01
#define DEINTERLACE_SETTINGS_MASK   0x02
#define ADVANCED_SETTINGS_MASK      0x04
#define ALL_SETTINGS_MASK            0x08

/**
 * Tree based settings dialog.
 * @see CTreeSettingsPage
 */
class CTreeSettingsDlg : public CDSDialog
{
// Construction
public:
    CTreeSettingsDlg(const tstring& Caption);   // standard constructor
    virtual ~CTreeSettingsDlg();

    /**
     * Adds a new page to the tree.
     *
     * @param pPage pointer to the page
     * @param parent parent node in the tree.
     * note that the parent must already have been added with a call to AddPage
     * @return integer used when adding new pages as a child of this one
     */
    int AddPage(SmartPtr<CTreeSettingsPage> pPage,int parent=-1);
    int AddPages(CComPtr<ISpecifyPropertyPages> SpecifyPages, int Parent = -1);

    BOOL ShowPage(HWND hDlg, int iPage);

    /**
     * @param iStartPage page that will be shown initialy
     */
    void SetStartPage(int iStartPage) {m_iStartPage=iStartPage;};
    static void ShowTreeSettingsDlg(int iSettingsMask);

// Implementation
protected:

    void OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify);
    LRESULT OnNotify(HWND hwnd, int id, LPNMHDR nmhdr);
    void OnOK(HWND hDlg);
    void OnCancel(HWND hDlg);
    LRESULT OnSelchangingTree(HWND hDlg, NMHDR* pNMHDR);
    BOOL OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam);
    void OnHelpBtn(HWND hDlg);
    void OnSize(HWND hDlg, UINT nType, int cx, int cy);
    void OnPaint(HWND hDlg);
    void OnGetMinMaxInfo(HWND hDlg, MINMAXINFO FAR* lpMMI);

private:
    virtual BOOL DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    ///internal class. used for storing information about one page
    class CPageInfo
    {
    public:
        CPageInfo();
        ~CPageInfo();
        SmartPtr<CTreeSettingsPage> m_pPage;
        int m_parent;
        HTREEITEM m_hTreeItem;
        HWND m_hWnd;
    };
    ///vector of all pages
    std::vector<CPageInfo> m_pages;
    tstring m_settingsDlgCaption;
    ///current page
    int m_iCurrentPage;
    ///start page
    int m_iStartPage;
    //image list
    //CImageList m_ImageList;

    HFONT m_StaticFont;
    RECT m_GripperRect;
    POINT m_MinMaxInfo;
};

#endif
