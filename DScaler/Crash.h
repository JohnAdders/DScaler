/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1998-2001 Avery Lee.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
/////////////////////////////////////////////////////////////////////////////
// This file was taken from VirtualDub
// VirtualDub - Video processing and capture application
// Copyright (C) 1998-2001 Avery Lee.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////

/**
 * @file crash.h crash Header file
 */

#ifndef __CRASH_H__
#define __CRASH_H__

#ifdef _DEBUG
    #define CHECK_FPU_STACK CheckFPUStack(__FILE__, __LINE__)
    void CheckFPUStack(const char *, const int);
#else
    #define CHECK_FPU_STACK
#endif


// class that SEH errors get translated to
class SEHException : public std::exception
{
public:
    SEHException(const char* what);
};

// class that handles all thread initialisation
// an intance of this should be created at the start of each thread proc
// as early as possible
class DScalerThread
{
public:
    DScalerThread(const char* ThreadName);
    ~DScalerThread();
    const char* GetName() const;
private:
    std::string m_Name;
    _se_translator_function m_OldTranslator;

};

#endif
