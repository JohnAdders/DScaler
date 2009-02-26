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
    #define CHECK_FPU_STACK checkfpustack(__FILE__, __LINE__);
    void checkfpustack(const char *, const int) throw();
#else
    #define CHECK_FPU_STACK
#endif

struct VirtualDubCheckpoint
{
    const char *file;
    int line;

    inline void set(const char *f, int l)
    {
        file=f; line=l;
    }
};

#define CHECKPOINT_COUNT        (16)

struct VirtualDubThreadState
{
    const char              *pszThreadName;
    unsigned long           dwThreadId;
    void *                  hThread;

    VirtualDubCheckpoint    cp[CHECKPOINT_COUNT];
    int                     nNextCP;
};

extern __declspec(thread) VirtualDubThreadState g_PerThreadState;

/**
 * Create a thread checkpoint.
 * This adds an entry to the crashlog containing file and line number.
 * Note that there is a limit of max CHECKPOINT_COUNT number of checkpoints 
 * for each thread.
 * The crashlog will contain a list of all the checkpoints that each thread 
 * has passed.
 */
#define VDCHECKPOINT (g_PerThreadState.cp[g_PerThreadState.nNextCP++&(CHECKPOINT_COUNT-1)].set(__FILE__, __LINE__))

/**
 * Thread init for crashloging.
 * Call this from the ThreadProc of all newly created threads,
 * this will make the crashlog contain a list of all active threads
 */
void DScalerInitializeThread(const char *pszName);

/**
 * Thread deinit for crashloging.
 * Call this when a thread is about to be terminated, if this is not called 
 * the list of threads in the crashlog might contain threads that is not 
 * running.
 */
void DScalerDeinitializeThread();

LONG WINAPI CrashHandler(EXCEPTION_POINTERS *pExc);
extern "C" unsigned long gBuildNum;

#endif
