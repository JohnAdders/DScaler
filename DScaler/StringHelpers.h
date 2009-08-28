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

/**
 * @file StringHelpers.h
 */

#ifndef __STRING_HELPERS_H___
#define __STRING_HELPERS_H___

#include <string>
#include <sstream>

#define DS_MAKE_CHAR_PTR(__TEXT__) #__TEXT__

std::string UnicodeToMBCS(const std::wstring& InString);
std::wstring MBCSToUnicode(const std::string& InString);
__inline std::string MBCSToMBCS(const std::string& InString) {return InString;};
__inline std::wstring UnicodeToUnicode(const std::wstring& InString) {return InString;};


#ifdef _UNICODE
    typedef std::wstring tstring;
    typedef std::wstringstream tstringstream;
    typedef std::wostringstream tostringstream;
    typedef std::wistringstream tistringstream;
    #define ADD_API_LETTER(_NAME_) DS_MAKE_CHAR_PTR(_NAME_##W)
    #define TStringToMBCS UnicodeToMBCS
    #define TStringToUnicode UnicodeToUnicode
    #define MBCSToTString MBCSToUnicode
    #define UnicodeToTString UnicodeToUnicode
#else
    typedef std::string tstring;
    typedef std::stringstream tstringstream;
    typedef std::ostringstream tostringstream;
    typedef std::istringstream tistringstream;
    #define ADD_API_LETTER(_NAME_) DS_MAKE_CHAR_PTR(_NAME_##A)
    #define TStringToMBCS MBCSToMBCS
    #define TStringToUnicode MBCSToUnicode
    #define MBCSToTString MBCSToMBCS
    #define UnicodeToTString UnicodeToMBCS
#endif

tstring EncodeASCIISafeString(const tstring& InString);
tstring DecodeASCIISafeString(const tstring& InString);


class MakeString
{
public:
   template <typename T>
   MakeString& operator<<(T const & Input)
   {
      m_Buffer << Input;
      return *this;
   }
   operator tstring () const
   {
      return m_Buffer.str();
   }
private:
   tostringstream m_Buffer;
};

template<typename T>
inline tstring ToString(const T& Input)
{
    tostringstream oss;
    oss << Input;
    return oss.str();
}

template<typename T>
inline std::string ToMBCSString(const T& Input)
{
    std::ostringstream oss;
    oss << Input;
    return oss.str();
}

template<typename T>
inline T FromString(const tstring& Input)
{
    T RetVal;
    tistringstream iss(Input);
    iss >> RetVal;
    return RetVal;
}

BOOL AreEqualInsensitive(const tstring& String1, const tstring& String2);
void Trim(tstring& StringToTrim);

tstring ReplaceCharWithString(const tstring& InString, char CharToReplace, const tstring& StringToReplace);

#endif
