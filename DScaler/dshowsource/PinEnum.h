/////////////////////////////////////////////////////////////////////////////
// $Id: PinEnum.h,v 1.2 2001-12-17 19:36:16 tobbej Exp $
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
// Revision 1.1  2001/12/09 22:01:48  tobbej
// experimental dshow support, doesnt work yet
// define WANT_DSHOW_SUPPORT if you want to try it
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file PinEnum.h interface for the CDShowPinEnum class.
 */

#if !defined(AFX_PINENUM_H__1446F155_3DEC_4476_805E_0B74863B7EC7__INCLUDED_)
#define AFX_PINENUM_H__1446F155_3DEC_4476_805E_0B74863B7EC7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "exception.h"

/**
 * Exception class for CDShowPinEnum
 * @see CDShowException
 * @see CDShowPinEnum
 */
class CDShowPinEnumException :public CDShowException
{
public:
	CDShowPinEnumException(CString msg,HRESULT hr):CDShowException(msg,hr) {};
	CDShowPinEnumException(CString msg):CDShowException(msg) {};
};



/**
 * Class that enumerates a IBaseFilter's pins.
 * It can be locked to a specified direction (input or output pins)
 */
class CDShowPinEnum
{
public:
	/**
	 * Constructs a CPinEnum object that enumerates all pins in specified direction
	 *
	 * @exception CPinEnumException this is thrown if unable to create a enumerator for the filter
	 * @param filter filter whos pins is to be enumerated
	 * @param pinDir direction
	 */
	CDShowPinEnum(CComPtr<IBaseFilter> filter,PIN_DIRECTION pinDir);	//endast viss riktning
	
	/**
	 * Constructs a CPinEnum object that enumerates all pins.
	 *
	 * @exception CPinEnumException this is thrown if unable to create a enumerator for the filter
	 * @param filter filter whos pins is to be enumerated
	 */
	CDShowPinEnum(CComPtr<IBaseFilter> filter);
	
	/** Copy construktor */
	CDShowPinEnum(CDShowPinEnum &pin);
	/** Destruktor */
	virtual ~CDShowPinEnum();

	/**
	 * Gets next pin.
	 * Currently thers no exception thrown, insted a NULL is returned
	 *
	 * @return next pin
	 */
	CComPtr<IPin> next();

	/**
	 * Skips pins
	 * 
	 * @param cPins number of pins to skip
	 * @return true if successfull
	 */
	bool skip(ULONG cPins);
	
	/**
	 * Resets the enumerator to the begining.
	 * @return true if successfull
	 */
	bool reset();
	
	/**
	 * Copys the enumerator.
	 * 
	 * @exception CPinEnumException this is thrown if clone fails
	 */
	CComPtr<IEnumPins> clone();

	/**
	 * Index operator.
	 * Normaly when you use next and the underlying enumerator has changed 
	 * you get a special error code, but since this function uses reset internaly
	 * you wont know if it has changed.
	 *
	 * @param index index of pin to return
	 */
	CComPtr<IPin> operator[](int index);

private:
	CComPtr<IEnumPins> m_pins;									//enumerator
	PIN_DIRECTION m_pinDir;										//direction
	bool m_anydir;
};

#endif // !defined(AFX_PINENUM_H__1446F155_3DEC_4476_805E_0B74863B7EC7__INCLUDED_)
