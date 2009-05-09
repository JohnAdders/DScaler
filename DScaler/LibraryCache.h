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

#ifndef _LIBRARY_CACHE_H
#define _LIBRARY_CACHE_H

#include <map>
#include <string>

class LibraryCache
{
public:
    ~LibraryCache();
    static HMODULE GetLibraryHandle(const TCHAR* LibaryName);
private:
    typedef std::map<tstring, HMODULE> LibraryMap;
    static LibraryCache* getInstance();
    static void Unload(LibraryMap::value_type& MapValueToUnload);
    LibraryCache();
    HMODULE GetLibraryHandleInternal(const TCHAR* LibraryName);
    LibraryMap m_LibraryMap;
};

#endif

