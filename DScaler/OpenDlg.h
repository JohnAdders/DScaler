/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 Torbjörn Jansson.  All rights reserved.
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
 * @file OpenDlg.h header file for COpenDlg
 */

#if !defined(__OPENDLG_H__)
#define __OPENDLG_H__

#include "DSDialog.h"

/**
 * This class is a simple open dialog similar to how windows run command and
 * mediaplayer open dialog works.
 */
class COpenDlg : public CDSDialog
{
// Construction
public:
    COpenDlg();

    const std::string& GetFileName() {return m_FileName;}

private:
    virtual BOOL DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    BOOL OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam);
    void OnBrowse(HWND hDlg);
    void OnOK(HWND hDlg);
    void OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify);
    std::string m_FileName;
};

#endif
