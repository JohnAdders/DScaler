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
 * @file TreeSettingsPage.h header file for CTreeSettingsPage
 */

#if !defined(_TREESETTINGSPAGE_H__)
#define _TREESETTINGSPAGE_H__

#include "DSDialog.h"

/**
 * Class for one page in the settings dialog.
 * The dialog box template used must have WS_CHILD and DS_CONTROL atributes
 * set and no borders, the DS_CONTROL atribut makes tab work properly.
 * This class can be used directly to create for example an empty page or a
 * page with only text (maybe some help text or a description of the subpages)
 * or it can be inherited from.
 *
 * @see CTreeSettingsDlg
 */
class CTreeSettingsPage : public CDSDialog
{
// Construction
public:
    /**
     * Constructor.
     * @param name name used in the tree
     * @param nIDTemplate dialog template id
     */
    CTreeSettingsPage(const tstring& name,UINT nIDTemplate);   // standard constructor
    CTreeSettingsPage(const tstring& TreeName,const tstring& HeaderName,UINT nIDTemplate);
    virtual ~CTreeSettingsPage();

    virtual BOOL OnSetActive(HWND hDlg);
    virtual BOOL OnKillActive(HWND hDlg);

    virtual void OnOK(HWND hDlg);
    virtual void OnCancel(HWND hDlg);

    /**
     * This function is used by CTreeSettingsDlg to get the minimum allowed
     * size of the page.
     * The default implementation of this function gets the windows size at
     * first call and then uses that for width and height.
     * @see CTreeSettingsDlg
     * @param width minimum allowed width of the page
     * @param height minimum allowed heith of the page
     */
    void GetMinSize(HWND hWnd, int& width, int& height);

    /// @return name of this page
    const tstring& GetName() {return m_name;};
    const tstring& GetHeaderName() {return m_HeaderName;};

protected:
    tstring m_name;
    tstring m_HeaderName;
    UINT m_dlgID;

    int m_minWidth;
    int m_minHeight;
    BOOL m_bInitMinSize;
private:
    virtual INT_PTR DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    virtual INT_PTR ChildDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif
