/////////////////////////////////////////////////////////////////////////////
// $Id: exception.cpp,v 1.1 2001-12-09 22:01:48 tobbej Exp $
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
// Change Log
//
// Date          Developer             Changes
//
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file exception.cpp implementation of the CDSException class.
 */


#include "stdafx.h"

#include "exception.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CDSException::CDSException(CString msg,HRESULT hr)
:m_errMsg(msg),m_hasErrNo(true),m_err(hr)
{

}

CDSException::CDSException(CString msg)
:m_errMsg(msg),m_hasErrNo(false),m_err(S_OK)
{

}

CDSException::~CDSException()
{

}

CString CDSException::getErrorText()
{
	CString msg;
	CString tmpstr;
	if(m_hasErrNo)
	{
		DWORD len=AMGetErrorText(m_err,tmpstr.GetBufferSetLength(MAX_ERROR_TEXT_LEN),MAX_ERROR_TEXT_LEN);
		tmpstr.ReleaseBuffer(len);
		msg.Format("%s\nErrorCode: 0x%x\nErrorText: %s",(LPCTSTR)m_errMsg,m_err,(LPCTSTR)tmpstr);
		return msg;
	}
	return m_errMsg;
}