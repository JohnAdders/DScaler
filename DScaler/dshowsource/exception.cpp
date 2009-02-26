/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Torbjörn Jansson.  All rights reserved.
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
 * @file exception.cpp implementation of the CDShowException class.
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT

#include "exception.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CDShowException::CDShowException(CString msg,HRESULT hr)
:m_errMsg(msg),m_hasErrNo(true),m_err(hr)
{

}

CDShowException::CDShowException(CString msg)
:m_errMsg(msg),m_hasErrNo(false),m_err(S_OK)
{

}

CDShowException::~CDShowException()
{

}

CString CDShowException::getErrorText()
{
    CString msg;
    CString tmpstr;
    if(m_hasErrNo)
    {
        DWORD len=AMGetErrorText(m_err,tmpstr.GetBufferSetLength(MAX_ERROR_TEXT_LEN),MAX_ERROR_TEXT_LEN);
        tmpstr.ReleaseBuffer(len);
        msg.Format("%s\nErrorCode: 0x%x\nErrorText: '%s'",(LPCTSTR)m_errMsg,m_err,(LPCTSTR)tmpstr);
        return msg;
    }
    return m_errMsg;
}

#endif