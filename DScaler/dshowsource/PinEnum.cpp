/////////////////////////////////////////////////////////////////////////////
// $Id: PinEnum.cpp,v 1.3 2001-12-17 19:36:16 tobbej Exp $
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
// Revision 1.2  2001/12/14 14:11:13  adcockj
// Added #ifdef to allow compilation without SDK
//
// Revision 1.1  2001/12/09 22:01:48  tobbej
// experimental dshow support, doesnt work yet
// define WANT_DSHOW_SUPPORT if you want to try it
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file PinEnum.cpp implementation of the CDShowPinEnum class.
 */


#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT
#include "PinEnum.h"
#include "exception.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDShowPinEnum::CDShowPinEnum(CComPtr<IBaseFilter> filter,PIN_DIRECTION pinDir)
:m_anydir(false)
{
	ASSERT(filter!=NULL);
	ASSERT(pinDir==PINDIR_INPUT || pinDir==PINDIR_OUTPUT);

	HRESULT hr=filter->EnumPins(&m_pins);
	if(FAILED(hr))
		throw CDShowPinEnumException("Failed to create pin enumerator",hr);
	
	ASSERT(m_pins!=NULL);
	m_pinDir=pinDir;
}

CDShowPinEnum::CDShowPinEnum(CComPtr<IBaseFilter> filter)
:m_anydir(true)
{
	ASSERT(filter!=NULL);
	HRESULT hr=filter->EnumPins(&m_pins);
	if(FAILED(hr))
		throw CDShowPinEnumException("Failed to create pin enumerator",hr);
}

CDShowPinEnum::CDShowPinEnum(CDShowPinEnum &pin)
{
	m_pins=pin.clone();
}

CDShowPinEnum::~CDShowPinEnum()
{

}

//////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////

CComPtr<IPin> CDShowPinEnum::next()
{
	CComPtr<IPin> pin;
	HRESULT hr=S_OK;

	//only pins in specified direction?
	if(m_anydir)
	{
		hr=m_pins->Next(1,&pin,NULL);
		if(FAILED(hr) || pin==NULL)
			return NULL;
			//throw CExPinEnum("next() failed",hr);
	}
	else
	{
		//step thru all pins until one with the right direction is found
		while(true)
		{
			hr=m_pins->Next(1,&pin,NULL);
			if(FAILED(hr) || pin==NULL)
				return NULL;
				//throw CExPinEnum("next() failed",hr);
			
			PIN_DIRECTION pinDir;
			hr=pin->QueryDirection(&pinDir);
			if(SUCCEEDED(hr) && pinDir==m_pinDir)
			{
				break;
			}
			else
			{
				//releas the pin becaus it was eighter wrong direction or some error
				//if we dont, there will be an assert trigered in CComPtr<>
				pin.Release();
			}
		}
	}
	return pin;
}

CComPtr<IPin> CDShowPinEnum::operator[](int index)
{
	CComPtr<IPin> pin;
	
	//reset the enumerator
	reset();

	//we cant use skip here since we might be "locked" in a specified pin direction.
	for(int i=0;i<index;i++)
	{
		pin=next();
	}
	return pin;
}

bool CDShowPinEnum::skip(ULONG cPins)
{
	return SUCCEEDED(m_pins->Skip(cPins))==TRUE;
}

bool CDShowPinEnum::reset()
{
	return SUCCEEDED(m_pins->Reset())==TRUE;
}

CComPtr<IEnumPins> CDShowPinEnum::clone()
{
	CComPtr<IEnumPins> pEnum;
	HRESULT hr=m_pins->Clone(&pEnum);
	
	if(FAILED(hr))
	{
		throw CDShowPinEnumException("clone() failed",hr);
	}

	return pEnum;
}

#endif