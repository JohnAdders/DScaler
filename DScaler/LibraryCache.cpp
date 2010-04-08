/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009 John Adcock.  All rights reserved.
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

#include "StdAfx.h"
#include <algorithm>
#include "LibraryCache.h"

void LibraryCache::Unload(LibraryMap::value_type& MapValueToUnload)
{
    if(MapValueToUnload.second != NULL)
    {
        FreeLibrary(MapValueToUnload.second);
    }
}

HMODULE LibraryCache::GetLibraryHandle(const TCHAR* LibaryName)
{
    return getInstance()->GetLibraryHandleInternal(LibaryName);
}
LibraryCache* LibraryCache::getInstance()
{
    static LibraryCache theInstance;
    return &theInstance;
}

LibraryCache::LibraryCache()
{
}

LibraryCache::~LibraryCache()
{
    std::for_each(m_LibraryMap.begin(), m_LibraryMap.end(), Unload);
}

HMODULE LibraryCache::GetLibraryHandleInternal(const TCHAR* LibraryName)
{
    LibraryMap::iterator it = m_LibraryMap.find(LibraryName);
    if(it == m_LibraryMap.end())
    {
        HMODULE RetVal = LoadLibrary(LibraryName);
        m_LibraryMap[LibraryName] = RetVal;
        return RetVal;
    }
    else
    {
        return it->second;
    }
}


