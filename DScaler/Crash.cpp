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

unsigned long gBuildNum = 0;

#ifdef _DEBUG

void CheckFPUStack(const char *file, const int line) throw()
{
    static const char szFPUProblemCaption[]="FPU/MMX internal problem";
    static const char szFPUProblemMessage[]="The FPU stack wasn't empty!  Tagword = %04x\nFile: %s, line %d";
    static BOOL seenmsg=FALSE;

    char    buf[128];
    unsigned short tagword;

    if (seenmsg)
    {
        return;
    }

    __asm fnstenv buf

    tagword = *(unsigned short *)(buf + 8);

    if (tagword != 0xffff)
    {
        wsprintf(buf, szFPUProblemMessage, tagword, file, line);
        MessageBox(NULL, buf, szFPUProblemCaption, MB_OK);
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
    throw SEHException("SEH Exception Occured");
}

DScalerThread::DScalerThread(const char* ThreadName) :
    m_Name(ThreadName)
{
    m_OldTranslator = _set_se_translator(SETranslate);
}

const char* DScalerThread::GetName() const
{
    return m_Name.c_str();
}


DScalerThread::~DScalerThread()
{
    _set_se_translator(m_OldTranslator);
}

