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
#include "shlobj.h"
#include "PathHelpers.h"
#include "DynamicFunction.h"

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
    return StripFile(GetMainExeName());
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

// gets just the the path from a file name
tstring StripFile(const tstring& FileName)
{
    size_t LastSlash(FileName.rfind('\\'));
    if(LastSlash != tstring::npos)
    {
        return FileName.substr(0, LastSlash);
    }
    else
    {
        return _T(".\\");
    }
}

BOOL IsXPOrUp()
{
    OSVERSIONINFO ov;

    ov.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&ov);
    if(ov.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
    {
        return FALSE;
    }

    return (ov.dwMajorVersion >= 5);
}

tstring GetUserFolder(int clsid)
{
    DynamicFunctionS5<HRESULT, HWND, int, HANDLE, DWORD, PTSTR>  GetFolderPath(_T("Shell32.dll"), ADD_API_LETTER(SHGetFolderPath));
    vector<TCHAR> Buffer(MAX_PATH);

    if(IsXPOrUp() && GetFolderPath)
    {
        if(FAILED(GetFolderPath(NULL, clsid | CSIDL_FLAG_CREATE, NULL, 0, &Buffer[0])))
        {
            return GetInstallationPath();
        }

        return &Buffer[0];
    }
    else
    {
        return GetInstallationPath();
    }
}

tstring GetUserFilePath()
{
    DynamicFunctionS6<HRESULT, HWND, int, HANDLE, DWORD, LPCTSTR, PTSTR>  GetFolderPathAndSubDir(_T("Shell32.dll"), ADD_API_LETTER(SHGetFolderPathAndSubDir));
    vector<TCHAR> Buffer(MAX_PATH);

    if(IsXPOrUp() && GetFolderPathAndSubDir)
    {
        if(FAILED(GetFolderPathAndSubDir(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, _T("DScaler4\\"), &Buffer[0])))
        {
            return GetInstallationPath();
        }

        return &Buffer[0];
    }
    else
    {
        return GetInstallationPath();
    }
}

tstring GetUserPicturePath()
{
    return GetUserFolder(CSIDL_MYPICTURES);
}

tstring GetUserVideoPath()
{
    return GetUserFolder(CSIDL_MYVIDEO);
}

tstring AddFileToPath(const tstring& Path, const tstring& FileName)
{
    vector<TCHAR> RetVal(Path.begin(), Path.end());
    RetVal.resize(MAX_PATH);
    if(PathAppend(&RetVal[0], FileName.c_str()) == TRUE)
    {
        return &RetVal[0];
    }
    else
    {
        // return something half sensible
        return FileName;
    }
}

