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
 * @file DevEnum.cpp implementation of the CDShowDevEnum class.
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT

#include "DevEnum.h"

#include "exception.h"

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDShowDevEnum::CDShowDevEnum()
:m_pSysDevEnum(NULL),m_pEnumCat(NULL)
{
    createSysEnum();
}

CDShowDevEnum::CDShowDevEnum(REFCLSID devClass)
:m_pSysDevEnum(NULL),m_pEnumCat(NULL)
{
    createSysEnum();
    initEnum(devClass);
}

CDShowDevEnum::~CDShowDevEnum()
{

}

void CDShowDevEnum::initEnum(REFCLSID devClass)
{
    HRESULT hr;
    if(m_pEnumCat!=NULL)
    {
        m_pEnumCat.Release();
        m_pEnumCat=NULL;
    }

    hr=m_pSysDevEnum->CreateClassEnumerator(devClass, &m_pEnumCat, 0);
    if(FAILED(hr))
    {
        throw CDShowDevEnumException("Cant create Class enumerator",hr);
    }
}

BOOL CDShowDevEnum::getNext()
{
    // if there are none in the list we get null
    if(m_pEnumCat==NULL)
    {
        return FALSE;
    }

    CComPtr<IMoniker> pMoniker;
    ULONG cFetched;


    if(m_pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
    {
        m_pPropBag.Release();
        pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&m_pPropBag);

        //get the moniker's DisplayName
        //(used to create this device)
        WCHAR *szDisplayName=NULL;
        if(SUCCEEDED(pMoniker->GetDisplayName(NULL,NULL,&szDisplayName)))
        {

            m_DisplayName = szDisplayName ? UnicodeToTString(szDisplayName) : _T("");
            CoTaskMemFree(szDisplayName);
        }
        else
        {
            //sätt den till en tom sträng om de blev fel
            m_DisplayName=_T("");
        }

        return TRUE;

    }
    else
    {
        return FALSE;
    }
}

void CDShowDevEnum::createSysEnum()
{
    HRESULT hr=m_pSysDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
    if(FAILED(hr))
    {
        //throw an exception
        //this error shoud never hapend unless somting is very wrong (or com is not initialized)
        throw CDShowDevEnumException("Cant create System Device Enumerator",hr);
    }
}

void CDShowDevEnum::createDevice(tstring displayName,REFIID interf, void *device)
{
    //try to create a moniker
    CComPtr<IBindCtx> pBC;
    CComPtr<IMoniker> pmDev;
    HRESULT hr = CreateBindCtx(0, &pBC);
    if(SUCCEEDED(hr))
    {
        DWORD dwEaten;
        hr = MkParseDisplayName(pBC, TStringToUnicode(displayName).c_str(), &dwEaten, &pmDev);

        if(FAILED(hr))
        {
            throw CDShowDevEnumException("createDevice: cant create moniker",hr);
        }
    }
    hr = pmDev->BindToObject(0, 0, interf, (void**)device);
    if(FAILED(hr))
    {
        throw CDShowDevEnumException("createDevice: BindToObject failed",hr);
    }
}

tstring CDShowDevEnum::getProperty(tstring szName)
{
    VARIANT varProperty;
    HRESULT hr;
    varProperty.vt = VT_BSTR;
    hr = m_pPropBag->Read(TStringToUnicode(szName).c_str(), &varProperty, 0);
    if(SUCCEEDED(hr))
    {
        tstring tmp(UnicodeToTString(varProperty.bstrVal));
        SysFreeString(varProperty.bstrVal);
        return tmp;
    }
    else
    {
        //shoud probably use an exception insted
        return _T("");
    }
}

#endif