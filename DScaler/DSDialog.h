/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009 John Adcock.  All rights reserved.
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
 * @file DScalerUtils.h
 */

#ifndef __DSDIALOG_H___
#define __DSDIALOG_H___

/// DScaler Dialog class
/// Replacement for MFC type class.
/// Allows holding of dialog state within an object instance
class CDSDialog
{
public:
    CDSDialog(LPCTSTR ResourceId);
    virtual ~CDSDialog();
    INT_PTR DoModal(HWND hParent);
    HWND Create(HWND hParent);
    void Destroy(HWND hDlg);
    void SetHelpID(int HelpID);
private:
    virtual BOOL DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) = 0;
    static BOOL CALLBACK MasterModalDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    LPCTSTR m_ResourceId;
    int m_HelpID;
};

tstring GetDlgItemString(HWND hDlg, int id);
int GetDlgItemInt(HWND hDlg, int id);
void SetDlgItemInt(HWND hDlg, int id, int Value);

#endif
