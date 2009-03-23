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

#ifndef _DYNAMIC_FUNCTION_H
#define _DYNAMIC_FUNCTION_H

#include "LibraryCache.h"

template<typename FUNCTYPE>
class DynamicFunctionParent
{
public:
    DynamicFunctionParent(const char* LibraryName, const char* FunctionName) :
        m_pFunc(0)
    {
        HMODULE LibraryModule = LibraryCache::GetLibraryHandle(LibraryName);
        if(LibraryModule)
        {
            m_pFunc = (FUNCTYPE)GetProcAddress(LibraryModule,FunctionName);
        }
    }

    operator BOOL() const
	{
		return m_pFunc != 0;
	}
protected:
    FUNCTYPE m_pFunc;
};

// Class for __cdecl functions taking one parameter
template<typename RETVAL, typename PARAM1>
class DynamicFunctionC1 : public DynamicFunctionParent< RETVAL (__cdecl *)(PARAM1) >
{
public:
    DynamicFunctionC1(const char* LibraryName, const char* FunctionName) :
        DynamicFunctionParent(LibraryName, FunctionName)
    {
    }
    
    RETVAL operator()(PARAM1 Param1) const
    {
        return m_pFunc(Param1);
    }
};


// Class for __stdcall functions taking no parameters
template<typename RETVAL>
class DynamicFunctionS0 : public DynamicFunctionParent< RETVAL (__stdcall *)() >
{
public:
    DynamicFunctionS0(const char* LibraryName, const char* FunctionName) :
        DynamicFunctionParent(LibraryName, FunctionName)
    {
    }
    
    RETVAL operator()() const
    {
        return m_pFunc();
    }
};

// Class for __stdcall functions taking one parameter
template<typename RETVAL, typename PARAM1>
class DynamicFunctionS1 : public DynamicFunctionParent< RETVAL (__stdcall *)(PARAM1) >
{
public:
    DynamicFunctionS1(const char* LibraryName, const char* FunctionName) :
        DynamicFunctionParent(LibraryName, FunctionName)
    {
    }
    
    RETVAL operator()(PARAM1 Param1) const
    {
        return m_pFunc(Param1);
    }
};

// Class for __stdcall functions taking two parameter
template<typename RETVAL, typename PARAM1, typename PARAM2>
class DynamicFunctionS2 : public DynamicFunctionParent< RETVAL (__stdcall *)(PARAM1, PARAM2) >
{
public:
    DynamicFunctionS2(const char* LibraryName, const char* FunctionName) :
        DynamicFunctionParent(LibraryName, FunctionName)
    {
    }
    
    RETVAL operator()(PARAM1 Param1, PARAM2 Param2) const
    {
        return m_pFunc(Param1, Param2);
    }
};

// Class for __stdcall functions taking three parameter
template<typename RETVAL, typename PARAM1, typename PARAM2, typename PARAM3>
class DynamicFunctionS3 : public DynamicFunctionParent< RETVAL (__stdcall *)(PARAM1, PARAM2, PARAM3) >
{
public:
    DynamicFunctionS3(const char* LibraryName, const char* FunctionName) :
        DynamicFunctionParent(LibraryName, FunctionName)
    {
    }
    
    RETVAL operator()(PARAM1 Param1, PARAM2 Param2, PARAM3 Param3) const
    {
        return m_pFunc(Param1, Param2, Param3);
    }
};

// Class for __stdcall functions taking four parameter
template<typename RETVAL, typename PARAM1, typename PARAM2, typename PARAM3, typename PARAM4>
class DynamicFunctionS4 : public DynamicFunctionParent< RETVAL (__stdcall *)(PARAM1, PARAM2, PARAM3, PARAM4) >
{
public:
    DynamicFunctionS4(const char* LibraryName, const char* FunctionName) :
        DynamicFunctionParent(LibraryName, FunctionName)
    {
    }
    
    RETVAL operator()(PARAM1 Param1, PARAM2 Param2, PARAM3 Param3, PARAM4 Param4) const
    {
        return m_pFunc(Param1, Param2, Param3, Param4);
    }
};

// Class for __stdcall functions taking five parameter
template<typename RETVAL, typename PARAM1, typename PARAM2, typename PARAM3, typename PARAM4, typename PARAM5>
class DynamicFunctionS5 : public DynamicFunctionParent< RETVAL (__stdcall *)(PARAM1, PARAM2, PARAM3, PARAM4, PARAM5) >
{
public:
    DynamicFunctionS5(const char* LibraryName, const char* FunctionName) :
        DynamicFunctionParent(LibraryName, FunctionName)
    {
    }
    
    RETVAL operator()(PARAM1 Param1, PARAM2 Param2, PARAM3 Param3, PARAM4 Param4, PARAM5 Param5) const
    {
        return m_pFunc?m_pFunc(Param1, Param2, Param3, Param4, Param5):0;
    }
};

// Class for __stdcall functions taking six parameter
template<typename RETVAL, typename PARAM1, typename PARAM2, typename PARAM3, typename PARAM4, typename PARAM5, typename PARAM6>
class DynamicFunctionS6 : public DynamicFunctionParent< RETVAL (__stdcall *)(PARAM1, PARAM2, PARAM3, PARAM4, PARAM5, PARAM6) >
{
public:
    DynamicFunctionS6(const char* LibraryName, const char* FunctionName) :
        DynamicFunctionParent(LibraryName, FunctionName)
    {
    }
    
    RETVAL operator()(PARAM1 Param1, PARAM2 Param2, PARAM3 Param3, PARAM4 Param4, PARAM5 Param5, PARAM6 Param6) const
    {
        return m_pFunc?m_pFunc(Param1, Param2, Param3, Param4, Param5, Param6):0;
    }
};

#endif

