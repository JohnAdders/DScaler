/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 1998-2001 Avery Lee.  All rights reserved.
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
 * @file Crash.cpp Crash code taken from VirtuaDub
 */


#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "DScaler.h"

#include <crtdbg.h>
#include <tlhelp32.h>
#include "resource.h"
#include "crash.h"
#include "DynamicFunction.h"
#include "PathHelpers.h"
#include "DbgHelp.h"

unsigned long gBuildNum = 0;

#ifdef _DEBUG

void CheckFPUStack(const TCHAR* file, const int line) throw()
{
    static const TCHAR szFPUProblemCaption[]=_T("FPU/MMX internal problem");
    static const TCHAR szFPUProblemMessage[]=_T("The FPU stack wasn't empty!  Tagword = %04x\nFile: %s, line %d");
    static BOOL seenmsg=FALSE;

    char buf[128];
    unsigned short tagword;

    if (seenmsg)
    {
        return;
    }

    __asm fnstenv buf

    tagword = *(unsigned short *)(buf + 8);

    if (tagword != 0xffff)
    {
        TCHAR err[128];
        wsprintf(err, szFPUProblemMessage, tagword, file, line);
        MessageBox(NULL, err, szFPUProblemCaption, MB_OK);
        seenmsg=TRUE;
    }

}

#endif

SEHException::SEHException(const char* what) :
    std::exception(what)
{
}

void SETranslate(unsigned int u, EXCEPTION_POINTERS* pExp)
{
    DynamicFunctionS7<BOOL, HANDLE, DWORD, HANDLE, MINIDUMP_TYPE, CONST PMINIDUMP_EXCEPTION_INFORMATION, CONST PMINIDUMP_USER_STREAM_INFORMATION, CONST PMINIDUMP_CALLBACK_INFORMATION> WriteDump(_T("DbgHelp.dll"), "MiniDumpWriteDump");
    if(WriteDump)
    {
        BOOL bMiniDumpSuccessful;
        HANDLE hDumpFile;
        MINIDUMP_EXCEPTION_INFORMATION ExpParam;

        tstring FileName(GetInstallationPath() + _T("\\minidump.dmp"));

        hDumpFile = CreateFile(FileName.c_str(), GENERIC_READ|GENERIC_WRITE, 
                    FILE_SHARE_WRITE|FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

        ExpParam.ThreadId = GetCurrentThreadId();
        ExpParam.ExceptionPointers = pExp;
        ExpParam.ClientPointers = TRUE;

        bMiniDumpSuccessful = WriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpWithDataSegs, &ExpParam, NULL, NULL);
    }

    throw SEHException("SEH Exception Occured");
}

DScalerThread::DScalerThread(const TCHAR* ThreadName) :
    m_Name(ThreadName)
{
    m_OldTranslator = _set_se_translator(SETranslate);
}

const TCHAR* DScalerThread::GetName() const
{
    return m_Name.c_str();
}


DScalerThread::~DScalerThread()
{
    _set_se_translator(m_OldTranslator);
}

