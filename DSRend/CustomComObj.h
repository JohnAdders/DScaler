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
 * @file CustomComObj.h Declaration of the CCustomComObject
 */

/**
 * Custom CComObject class.
 * This class is similar to the ATL class CComObject, its used on the input pin to delegate 
 * addref/release to the filter itself.
 * It is also used to send a pointer to the filter to the input pin
 */
template<typename base,typename paramType>
class CCustomComObject : public base
{
public:
    CCustomComObject(paramType pFilter)
        :base(pFilter)
    {
        _Module.Lock();
    }

    ~CCustomComObject()
    {
        FinalRelease();
#ifdef _ATL_DEBUG_INTERFACES
        _Module.DeleteNonAddRefThunk(_GetRawUnknown());
#endif
        _Module.Unlock();
    }
    
    ULONG STDMETHODCALLTYPE AddRef()
    {
        ULONG ref=InternalAddRef();
        //ATLTRACE(_T("AddRef count=%d\n"),ref);
        return ref;
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        ULONG ref=InternalRelease();
        //ATLTRACE(_T("Release count=%d\n"),ref);
        return ref;
    }
    
    STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject)
    {
        return _InternalQueryInterface(iid, ppvObject);
    }
};