/////////////////////////////////////////////////////////////////////////////
// $Id: DevEnum.h,v 1.3 2005-03-11 14:54:41 adcockj Exp $
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
// Revision 1.2  2001/12/17 19:36:16  tobbej
// renamed a few classes
//
// Revision 1.1  2001/12/09 22:01:48  tobbej
// experimental dshow support, doesnt work yet
// define WANT_DSHOW_SUPPORT if you want to try it
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file DevEnum.h interface for the CDShowDevEnum class.
 */

#if !defined(AFX_DEVENUM_H__480881D3_9016_422D_8B1C_44D23FED26FE__INCLUDED_)
#define AFX_DEVENUM_H__480881D3_9016_422D_8B1C_44D23FED26FE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "exception.h"

/**
 * Exception class for CDevEnum
 * @see CDShowException
 */
class CDShowDevEnumException: public CDShowException
{
public:
	CDShowDevEnumException(CString msg,HRESULT hr):CDShowException(msg,hr) {};
	CDShowDevEnumException(CString msg):CDShowException(msg) {};
};

/**
 * Class for enumerating dshow devices.
 * It is a class built around the direct show "System device enumerator"
 */
class CDShowDevEnum  
{
public:

	/**
	 * Create a device enumerator for a given device class
	 * @throws CDevEnumException
	 */
	void initEnum(REFCLSID devClass);
	
	/**
	 * Gets next device if any.
	 * @throws CDevEnumException
	 * @return true if successfull
	 */
	bool getNext();

	/**
	 * This function is used to read a property from the current device or filter.
	 * 
	 * @param szName Name of property to get, for example "FriendlyName" for devicename or "CLSID"
	 * @return value of property
	 */
	string getProperty(string szName);

	/**
	 * @return string representation of current device. Can be used to create a device with createDevice member
	 */
    string getDisplayName() {return std::string(m_DisplayName);};

	//name of device
	//string getFriendlyName() {return getProperty("FriendlyName");};

	/**
	 * Creates a device from given string.
	 * @param displayName name of device (not friendlyname)
	 * @param interf interface to return from the device
	 * @param device pointer to device
	 */
	static void createDevice(string displayName,REFIID interf, void *device);
	
	/**
	 * Default constructor.
	 * You must call initEnum memeber befor using the object
	 */
	CDShowDevEnum();

	/**
	 * Constructor that initializes the object to use the specified device class.
	 * @throws CDevEnumException
	 */
	CDShowDevEnum(REFCLSID devClass);
	virtual ~CDShowDevEnum();

private:
	/// @throws CDevEnumException
	void createSysEnum();

	CComPtr<IEnumMoniker> m_pEnumCat;
	CComPtr<ICreateDevEnum> m_pSysDevEnum;
	CComPtr<IPropertyBag> m_pPropBag;

	CString m_DisplayName;
};

#endif // !defined(AFX_DEVENUM_H__480881D3_9016_422D_8B1C_44D23FED26FE__INCLUDED_)
