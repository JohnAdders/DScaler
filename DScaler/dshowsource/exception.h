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
 * @file exception.h interface for exception classes.
 */

#if !defined(_EXCEPTION_H__)
#define _EXCEPTION_H__

/**
 * Exception baseclass.
 */
class CDShowException : public std::runtime_error
{
public:
    CDShowException(const std::string& Msg,HRESULT hr = S_OK);
    HRESULT getErrNo() {return m_err;};

private:
    HRESULT m_err;            //error code
};

#endif
