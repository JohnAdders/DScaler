/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Eric Schmidt.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//    This file is subject to the terms of the GNU General Public License as
//    published by the Free Software Foundation.  A copy of this license is
//    included with this software distribution in the file COPYING.  If you
//    do not have a copy, you may obtain a copy by writing to the Free
//    Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//    This software is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(_TSOPTIONSDLG_H__)
#define _TSOPTIONSDLG_H__

#include "TimeShift.h"
#include "DSDialog.h"

/** Holds all of the configuration variables for the TimeShift options */
typedef struct
{
    int        recHeight;  /**< Equal to a TS_* height constant */
    tsFormat_t format;     /**< A FORMAT_* constant */
    DWORD      sizeLimit;  /**< The maximum size of each recorded file (0 = no limit) */
    TCHAR      path[MAX_PATH + 1];
    FOURCC     fcc;        /**< The FOURCC of the video codec to use */
} TS_OPTIONS;

/** TimeShift options dialog */
class CTSOptionsDlg : public CDSDialog
{

public:
    CTSOptionsDlg();

// Implementation
private:
    virtual INT_PTR DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    BOOL UpdateData(HWND hDlg, BOOL bSaveAndValidate);
    BOOL OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam);
    void OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify);

    void OnButtonCompression(HWND hDlg);
    void OnCompressionhelp(HWND hDlg);
    void OnWavehelp(HWND hDlg);
    void OnHeighthelp(HWND hDlg);
    void OnButtonMixer(HWND hDlg);
    void OnTSPathSelect(HWND hDlg);
    void OnUpdateTSSize(HWND hDlg);
    void OnKillfocusTSSize(HWND hDlg);
    void OnTSSizeNoLimit(HWND hDlg);
    void OnOK(HWND hDlg);

    BOOL IsChecked(HWND hDlg, int id);
    void SetChecked(HWND hDlg, int id, BOOL checked);

private:
    TS_OPTIONS options;

    /** Keeps track of the last usable size (> 0) so it can be restored if the
     * "no limit" check box is unchecked
     */
    DWORD m_lastSize;
};

#endif
