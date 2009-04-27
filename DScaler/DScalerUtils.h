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
 * @file DScalerUtils.h
 */

#ifndef __DSCALER_UTILS_H___
#define __DSCALER_UTILS_H___

#define NUMBEROF(arr) (sizeof(arr) /sizeof(array[0]))

class MakeString
{
public:
   template <typename T>
   MakeString& operator<<(T const & Input)
   {
      m_Buffer << Input;
      return *this;
   }
   operator std::string () const
   {
      return m_Buffer.str();
   }
private:
   std::ostringstream m_Buffer;
};

template<typename T>
inline std::string ToString(const T& Input)
{
    std::ostringstream oss;
    oss << Input;
    return oss.str();
}

template<typename T>
inline T FromString(const std::string& Input)
{
    T RetVal;
    std::istringstream oss(Input);
    oss >> RetVal;
    return RetVal;
}

namespace DScalerPrivate
{
    class _StringBuffer
    {
        std::string &m_str;
        std::vector<char> m_buffer;

    public:
        _StringBuffer(std::string &str, size_t nSize) :
                    m_str(str),
                    m_buffer(nSize + 1)
        {
            get()[nSize] = 0;
        }

        ~_StringBuffer()
         {
             commit();
         }

         char* get()
         {
             return &(m_buffer[0]);
         }

         operator char*()
         {
             return get();
         }

        void commit()
        {
            if (m_buffer.size() != 0)
            {
                size_t l = std::char_traits<char>::length(get());
                m_str.assign(get(), l);
                m_buffer.resize(0);
            }
        }
    };

    struct NoCasePredicate : public std::binary_function<char, char, BOOL>
    {
        BOOL operator()(char x, char y) const
        {
            return toupper(x) < toupper(y);
        }
    };
}

inline DScalerPrivate::_StringBuffer StringBuffer(std::string &str, size_t nSize)
{
    return DScalerPrivate::_StringBuffer(str, nSize);
}

BOOL AreEqualInsensitive(const std::string& String1, const std::string& String2);
void Trim(std::string& StringToTrim);
void ScreenToClient(HWND hWnd, RECT& Rect);
void ClientToScreen(HWND hWnd, RECT& Rect);
void MoveWindow(HWND hWnd, RECT& Rect);

std::string ReplaceCharWithString(const std::string& InString, char CharToReplace, const std::string& StringToReplace);

#endif
