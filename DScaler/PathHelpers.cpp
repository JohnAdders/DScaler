////////////////////////////////////////////////////////////////////////////
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

/**
 * @file PathHelpers.cpp
 */

#include "stdafx.h"
#include "PathHelpers.h"

using namespace std;

tstring GetMainExeName()
{
    vector<TCHAR> RetVal(MAX_PATH);
    if(GetModuleFileName (NULL, &RetVal[0], MAX_PATH) == 0)
    {
        throw std::logic_error("GetModuleFileName failed");
    }
    return &RetVal[0];
}

tstring GetInstallationPath()
{
    tstring ExePath(GetMainExeName());
    size_t LastSlash(ExePath.rfind('\\'));
    if(LastSlash != tstring::npos)
    {
        return ExePath.substr(0, LastSlash);
    }
    else
    {
        return _T("");
    }
}

tstring GetExtension(const tstring& FileName)
{
    size_t LastDot(FileName.rfind('.'));
    if(LastDot != tstring::npos)
    {
        return FileName.substr(LastDot + 1);
    }
    else
    {
        return _T("");
    }
}

tstring StripPath(const tstring& FileName)
{
    tstring RetVal;
    size_t LastSlash(FileName.rfind('\\'));
    if(LastSlash != tstring::npos)
    {
        return FileName.substr(LastSlash + 1);
    }
    else
    {
        return FileName;
    }
}


