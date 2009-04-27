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
 * @file DSVideoFormatPage.h header file
 */

#if !defined(_DSVIDEOFORMATPAGE_H__)
#define _DSVIDEOFORMATPAGE_H__

#include "TreeSettingsPage.h"
#include "..\DScalerRes\resource.h"
#include "DSGraph.h"
#include <vector>

/**
 * CDSVideoFormatPage dialog.
 * Makes it possible to customize the resolution submenu.
 * @todo validate settings in OnOK
 * @todo implement "Reset to default" button
 * @todo implement croping, might need changes to dsrend filter
 */
class CDSVideoFormatPage : public CTreeSettingsPage
{
// Construction
public:
    CDSVideoFormatPage(const std::string& name, std::vector<CDShowGraph::CVideoFormat>& fmts,CSliderSetting* pResolution);   // standard constructor
    ~CDSVideoFormatPage() {};

// Implementation
private:
    BOOL ChildDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT OnNotify(HWND hwnd, int id, LPNMHDR nmhdr);
    void OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify);

    BOOL OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam);
    void OnVideoFmtSelChange(HWND hDlg);
    LRESULT OnDeltaPosWidth(HWND hDlg, NMHDR* pNMHDR);
    LRESULT OnDeltaPosHeight(HWND hDlg, NMHDR* pNMHDR);
    void OnChangeWidth(HWND hDlg);
    void OnChangeHeight(HWND hDlg);
    void OnChangeName(HWND hDlg);
    void OnContextMenu(HWND hDlg, HWND hwndContext, UINT xPos, UINT yPos);
    void OnClickedDelete(HWND hDlg);
    void OnClickedNew(HWND hDlg);
    void OnClickedClear(HWND hDlg);

    void OnOK(HWND hDlg);

    void UpdateControlls(HWND hDlg);
    void GenerateName(HWND hDlg, int pos);
    std::vector<CDShowGraph::CVideoFormat> m_VideoFmt;

    /**
     * Reference to the real video format vector that is used by dscaler,
     * this is the one we save the changes to when closing dialog by clicking on ok
     */
    std::vector<CDShowGraph::CVideoFormat> &m_RealVideoFmt;

    ///flag to prevent controlls from messing with the settings when updating controlls
    BOOL m_bInUpdateControlls;
    CSliderSetting *m_pResolutionSetting;
    HWND m_ListBox;
    HWND m_SpinWidth;
    HWND m_SpinHeight;
};

#endif
