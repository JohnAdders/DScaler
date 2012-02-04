/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Torbjörn Jansson.  All rights reserved.
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
 * @file DSRend.cpp Implementation of DLL Exports.
 */


// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL, 
//      run nmake -f DSRendps.mk in the project directory.

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "DSRend.h"

#include "DSRend_i.c"
#include "DSRendFilter.h"
#include "DSRendQualityPage.h"
#include "Cpu.h"
#include "SettingsPage.h"
#include "DSRendAboutPage.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_DSRendFilter, CDSRendFilter)
OBJECT_ENTRY(CLSID_DSRendQualityPage, CDSRendQualityPage)
OBJECT_ENTRY(CLSID_SettingsPage, CSettingsPage)
OBJECT_ENTRY(CLSID_DSRendAboutPage, CDSRendAboutPage)
END_OBJECT_MAP()

/// Filter name
static const WCHAR g_wszName[] = L"DScaler Renderer";

/// DLL Entry Point
extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_DSRENDLib);
        DisableThreadLibraryCalls(hInstance);
        CPU_SetupFeatureFlag();
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
    return TRUE;    // ok
}


/// Used to determine whether the DLL can be unloaded by OLE
STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

/// Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

/// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
    REGPINTYPES regPinTypes =
    {
        &MEDIATYPE_Video,
        &MEDIASUBTYPE_NULL    
    };
    REGFILTERPINS2 regFilterPins2 =
    {
        REG_PINFLAG_B_RENDERER,
        1,
        1,
        &regPinTypes,
        0,
        NULL,
        &CLSID_NULL
    };
    REGFILTER2 regFilter2;
    regFilter2.dwVersion=2;
    regFilter2.dwMerit=MERIT_DO_NOT_USE;
    regFilter2.cPins2=1;
    regFilter2.rgPins2=&regFilterPins2;
    
    
    CComPtr<IFilterMapper2> pMapper;
    HRESULT hr;

    hr=pMapper.CoCreateInstance(CLSID_FilterMapper2);
    if(FAILED(hr))
        return hr;

    // registers object, typelib and all interfaces in typelib
    hr=_Module.RegisterServer(TRUE);
    if(FAILED(hr))
        return hr;
    
    // no need to register as a filter
    // register filter as a directshow filter
    //hr=pMapper->RegisterFilter(CLSID_DSRendFilter,g_wszName,NULL,&CLSID_LegacyAmFilterCategory,NULL,&regFilter2);
    return hr;
}

/// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
    HRESULT hr;
    CComPtr<IFilterMapper2> pMapper;
    hr=pMapper.CoCreateInstance(CLSID_FilterMapper2);
    if(FAILED(hr))
        return hr;

    hr=pMapper->UnregisterFilter(&CLSID_LegacyAmFilterCategory, NULL, CLSID_DSRendFilter);
    if(FAILED(hr))
        return hr;

    return _Module.UnregisterServer(TRUE);
}

// DllInstall - Adds/Removes entries to the system registry per user
//              per machine.	
STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
    HRESULT hr = E_FAIL;
    static const wchar_t szUserSwitch[] = _T("user");

    if (pszCmdLine != NULL)
    {
    	if (_wcsnicmp(pszCmdLine, szUserSwitch, _countof(szUserSwitch)) == 0)
    	{
    		AtlSetPerUserRegistration(true);
    	}
    }

    if (bInstall)
    {	
    	hr = DllRegisterServer();
    	if (FAILED(hr))
    	{	
    		DllUnregisterServer();
    	}
    }
    else
    {
    	hr = DllUnregisterServer();
    }

    return hr;
}
