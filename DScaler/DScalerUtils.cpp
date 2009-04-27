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

using namespace std;

BOOL AreEqualInsensitive(const string& String1, const string& String2)
{
    return _stricmp(String1.c_str(), String2.c_str()) == 0;
}

void Trim(string& StringToTrim)
{
}

void ScreenToClient(HWND hWnd, RECT& Rect)
{
    ScreenToClient(hWnd, (LPPOINT)&Rect.left);
    ScreenToClient(hWnd, (LPPOINT)&Rect.right);
}

void ClientToScreen(HWND hWnd, RECT& Rect)
{
    ClientToScreen(hWnd, (LPPOINT)&Rect.left);
    ClientToScreen(hWnd, (LPPOINT)&Rect.right);
}

void MoveWindow(HWND hWnd, RECT& Rect)
{
    MoveWindow(hWnd, Rect.left, Rect.top, Rect.right - Rect.left, Rect.bottom - Rect.top, TRUE);
}

string ReplaceCharWithString(const string& InString, char CharToReplace, const string& StringToReplace)
{
    string Result;
    const char * CurChar = InString.c_str();
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
