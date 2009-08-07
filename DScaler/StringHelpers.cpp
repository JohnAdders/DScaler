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
 * @file DScalerUtils.cpp
 */

#include "stdafx.h"
#include "DScalerUtils.h"
#include "DebugLog.h"

using namespace std;

BOOL AreEqualInsensitive(const tstring& String1, const tstring& String2)
{
    return _tcsicmp(String1.c_str(), String2.c_str()) == 0;
}

void Trim(tstring& StringToTrim)
{
}


std::string UnicodeToMBCS(const std::wstring& InString)
{
    //covert to char
    int n=WideCharToMultiByte(CP_ACP, 0, InString.c_str(), -1, NULL, 0, NULL, NULL);
    vector<char> name(n);
    WideCharToMultiByte(CP_ACP, 0, InString.c_str(), -1, &name[0], n, NULL, NULL);
    return &name[0];
}

std::wstring MBCSToUnicode(const std::string& InString)
{
    //covert to wide char
    int n=MultiByteToWideChar(CP_ACP, 0, InString.c_str(), -1, NULL, 0);
    vector<wchar_t> name(n);
    MultiByteToWideChar(CP_ACP, 0, InString.c_str(), -1, &name[0], n);
    return &name[0];
}


tstring ReplaceCharWithString(const tstring& InString, char CharToReplace, const tstring& StringToReplace)
{
    tstring Result;
    const TCHAR*  CurChar = InString.c_str();
    while (*CurChar != '\0' )
    {
        if (*CurChar == CharToReplace )
        {
            Result += StringToReplace;
        }
        else
        {
            Result += *CurChar;
        }
        ++CurChar;
    }
    return Result;
}

tstring EncodeASCIISafeString(const tstring& InString)
{
    tostringstream oss;
    tstring::const_iterator CurIt(InString.begin());
    tstring::const_iterator LastIt(InString.end());

    while(CurIt != LastIt)
    {
        TCHAR CurChar(*CurIt);

        if(CurChar < 32 || CurChar > 127)
        {
            oss << '%' << (DWORD)CurChar << '%';
        }
        else if(CurChar == '%')
        {
            oss << _T("%%");
        }
        else
        {
            oss << CurChar;
        }
        ++CurIt;
    }
    return oss.str();
}

tstring DecodeASCIISafeString(const tstring& InString)
{
    tostringstream oss;
    tstring::const_iterator CurIt(InString.begin());
    tstring::const_iterator LastIt(InString.end());
    BOOL DoingPercent(FALSE);
    tstring TempString;

    while(CurIt != LastIt)
    {
        TCHAR CurChar(*CurIt);

        if(CurChar == '%')
        {
            if(DoingPercent)
            {
                if(TempString.empty())
                {
                    oss << '%';
                }
                else
                {
                    DWORD CharValue(FromString<DWORD>(TempString));
#ifdef _UNICODE    
                    oss << (TCHAR)CharValue;
#else
                    if(CharValue < 255)
                    {
                        oss << (TCHAR)CharValue;
                    }
                    else
                    {
                        oss << (TCHAR)'?';
                    }
#endif
                }
                DoingPercent = FALSE;
            }
            else
            {
                DoingPercent = TRUE;
                TempString = _T("");
            }
        }
        else
        {
            if(DoingPercent)
            {
                TempString += CurChar;
            }
            else
            {
                oss << CurChar;
            }
        }
        ++CurIt;
    }
    
    if(DoingPercent)
    {
        LOG(1, "Invalid escaping in DecodeASCIISafeString");
        return InString;
    }

    return oss.str();
}

