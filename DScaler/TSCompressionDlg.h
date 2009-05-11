/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Nick Kochakian.  All rights reserved.
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

#if !defined(_TSCOMPRESSION_H__)
#define _TSCOMPRESSION_H__

// TSCompression.h : header file
//

#include "TSOptionsDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CTSCompressionDlg dialog

class CTSCompressionDlg : public CDSDialog
{
// Construction
public:
    CTSCompressionDlg(TS_OPTIONS* Options);

private:
    tstring m_AudioFormat;
    tstring m_VideoFormat;

    virtual INT_PTR DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    BOOL OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam);
    void OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify);

    void OnConfigVideo(HWND hDlg);
    void OnConfigAudio(HWND hDlg);
    void OnOK(HWND hDlg);

    TS_OPTIONS* m_Options;
    FOURCC     m_fcc;
};

#endif
