/////////////////////////////////////////////////////////////////////////////
// $Id: Crash.cpp,v 1.4 2001-11-23 10:49:16 adcockj Exp $
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
/////////////////////////////////////////////////////////////////////////////
// DO NOT USE stdio.h!  printf() calls malloc()!
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.3  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.2  2001/07/27 16:11:32  adcockj
// Added support for new Crash dialog
//
// Revision 1.1  2001/07/24 12:19:00  adcockj
// Added code and tools for crash logging from VirtualDub
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "DScaler.h"
#include <crtdbg.h>

#include <tlhelp32.h>

#include "crash.h"
#include "disasm.h"
#include "list.h"

///////////////////////////////////////////////////////////////////////////

#define CODE_WINDOW (256)

///////////////////////////////////////////////////////////////////////////

static CCodeDisassemblyWindow *g_pcdw;

///////////////////////////////////////////////////////////////////////////

BOOL APIENTRY CrashDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
static const struct ExceptionLookup 
{
    DWORD   code;
    const char *name;
} 
exceptions[]=
{
    {   EXCEPTION_ACCESS_VIOLATION,         "Access Violation"      },
    {   EXCEPTION_BREAKPOINT,               "Breakpoint"            },
    {   EXCEPTION_FLT_DENORMAL_OPERAND,     "FP Denormal Operand"   },
    {   EXCEPTION_FLT_DIVIDE_BY_ZERO,       "FP Divide-by-Zero"     },
    {   EXCEPTION_FLT_INEXACT_RESULT,       "FP Inexact Result"     },
    {   EXCEPTION_FLT_INVALID_OPERATION,    "FP Invalid Operation"  },
    {   EXCEPTION_FLT_OVERFLOW,             "FP Overflow",          },
    {   EXCEPTION_FLT_STACK_CHECK,          "FP Stack Check",       },
    {   EXCEPTION_FLT_UNDERFLOW,            "FP Underflow",         },
    {   EXCEPTION_INT_DIVIDE_BY_ZERO,       "Integer Divide-by-Zero",   },
    {   EXCEPTION_INT_OVERFLOW,             "Integer Overflow",     },
    {   EXCEPTION_PRIV_INSTRUCTION,         "Privileged Instruction",   },
    {   EXCEPTION_ILLEGAL_INSTRUCTION,      "Illegal instruction"   },
    {   0xe06d7363,                         "Unhandled Microsoft C++ Exception",    },
            // hmm... '_msc'... gee, who would have thought?
    {   NULL    },
};

static void Report(HWND hwndList, HANDLE hFile, const char *format, ...)
{
    char buf[256];
    va_list val;
    int ch;

    va_start(val, format);
    ch = wvsprintf(buf, format, val);
    va_end(val);

    if (hwndList)
    {
        SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)buf);
    }

    if (hFile) 
    {
        DWORD dwActual;

        buf[ch] = '\r';
        buf[ch+1] = '\n';
        WriteFile(hFile, buf, ch+2, &dwActual, NULL);
        FlushFileBuffers(hFile);
    }
}

static void SetWindowTitlef(HWND hwnd, const char *format, ...)
{
    char buf[256];
    va_list val;

    va_start(val, format);
    wvsprintf(buf, format, val);
    va_end(val);
    SetWindowText(hwnd, buf);
}

static void ReportCrashData(HWND hwnd, HWND hwndReason, HANDLE hFile, const EXCEPTION_POINTERS *const pExc)
{
    const EXCEPTION_RECORD *const pRecord = (const EXCEPTION_RECORD *)pExc->ExceptionRecord;
    const CONTEXT *const pContext = (const CONTEXT *)pExc->ContextRecord;
    int i, tos;

    Report(hwnd, hFile, "EAX = %08lx", pContext->Eax);
    Report(hwnd, hFile, "EBX = %08lx", pContext->Ebx);
    Report(hwnd, hFile, "ECX = %08lx", pContext->Ecx);
    Report(hwnd, hFile, "EDX = %08lx", pContext->Edx);
    Report(hwnd, hFile, "EBP = %08lx", pContext->Ebp);
    Report(hwnd, hFile, "DS:ESI = %04x:%08lx", pContext->SegDs, pContext->Esi);
    Report(hwnd, hFile, "ES:EDI = %04x:%08lx", pContext->SegEs, pContext->Edi);
    Report(hwnd, hFile, "SS:ESP = %04x:%08lx", pContext->SegSs, pContext->Esp);
    Report(hwnd, hFile, "CS:EIP = %04x:%08lx", pContext->SegCs, pContext->Eip);
    Report(hwnd, hFile, "FS = %04x", pContext->SegFs);
    Report(hwnd, hFile, "GS = %04x", pContext->SegGs);
    Report(hwnd, hFile, "EFLAGS = %08lx", pContext->EFlags);
    Report(hwnd, hFile, "");

    // extract out MMX registers

    tos = (pContext->FloatSave.StatusWord & 0x3800)>>11;

    for(i=0; i<8; i++) 
    {
        long *pReg = (long *)(pContext->FloatSave.RegisterArea + 10*((i-tos) & 7));
        Report(hwnd, hFile, "MM%c = %08lx%08lx", i+'0', pReg[0], pReg[1]);
    }

    // fill out bomb reason

    const struct ExceptionLookup *pel = exceptions;

    while(pel->code) 
    {
        if (pel->code == pRecord->ExceptionCode)
        {
            break;
        }

        ++pel;
    }

    // Unfortunately, EXCEPTION_ACCESS_VIOLATION doesn't seem to provide
    // us with the read/write flag and virtual address as the docs say...
    // *sigh*

    if (!pel->code) 
    {
        if (hwndReason)
        {
            SetWindowTitlef(hwndReason, "Crash reason: unknown exception 0x%08lx", pRecord->ExceptionCode);
        }

        if (hFile)
        {
            Report(NULL, hFile, "Crash reason: unknown exception 0x%08lx", pRecord->ExceptionCode);
        }
    } 
    else 
    {
        if (hwndReason)
        {
            SetWindowTitlef(hwndReason, "Crash reason: %s", pel->name);
        }

        if (hFile)
        {
            Report(NULL, hFile, "Crash reason: %s", pel->name);
        }
    }

    Report(NULL, hFile, "");

}

static const char *GetNameFromHeap(const char *heap, int idx) 
{
    while(idx--)
    {
        while(*heap++)
        {
            ;
        }
    }

    return heap;
}

static void SpliceProgramPath(char *buf, int bufsiz, const char *fn) 
{
    char tbuf[MAX_PATH];
    char *pszFile;

    GetModuleFileName(NULL, tbuf, sizeof tbuf);
    GetFullPathName(tbuf, bufsiz, buf, &pszFile);
    strcpy(pszFile, fn);
}

//////////////////////////////////////////////////////////////////////////////

static bool IsValidCall(char *buf, int len) 
{
    // Permissible CALL sequences that we care about:
    //
    //  E8 xx xx xx xx          CALL near relative
    //  FF (group 2)            CALL near absolute indirect
    //
    // Minimum sequence is 2 bytes (call eax).
    // Maximum sequence is 7 bytes (call dword ptr [eax+disp32]).
    if (len >= 5 && buf[-5] == (char)0xE8)
    {
        return true;
    }

    // FF 14 xx                 CALL [reg32+reg32*scale]
    if (len >= 3 && buf[-3] == (char)0xFF && buf[-2]==0x14)
    {
        return true;
    }

    // FF 15 xx xx xx xx        CALL disp32
    if (len >= 6 && buf[-6] == (char)0xFF && buf[-5]==0x15)
    {
        return true;
    }

    // FF 00-3F(!14/15)         CALL [reg32]
    if (len >= 2 && buf[-2] == (char)0xFF && (unsigned char)buf[-1] < 0x40)
    {
        return true;
    }

    // FF D0-D7                 CALL reg32
    if (len >= 2 && buf[-2] == (char)0xFF && (buf[-1]&0xF8) == 0xD0)
    {
        return true;
    }

    // FF 50-57 xx              CALL [reg32+reg32*scale+disp8]
    if (len >= 3 && buf[-3] == (char)0xFF && (buf[-2]&0xF8) == 0x50)
    {
        return true;
    }

    // FF 90-97 xx xx xx xx xx  CALL [reg32+reg32*scale+disp32]
    if (len >= 7 && buf[-7] == (char)0xFF && (buf[-6]&0xF8) == 0x90)
    {
        return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////

struct ModuleInfo 
{
    const char *name;
    unsigned long base, size;
};

// ARRGH.  Where's psapi.h?!?

struct Win32ModuleInfo 
{
    DWORD base, size, entry;
};

typedef BOOL (__stdcall *PENUMPROCESSMODULES)(HANDLE, HMODULE *, DWORD, LPDWORD);
typedef DWORD (__stdcall *PGETMODULEBASENAME)(HANDLE, HMODULE, LPTSTR, DWORD);
typedef BOOL (__stdcall *PGETMODULEINFORMATION)(HANDLE, HMODULE, Win32ModuleInfo *, DWORD);

typedef HANDLE (__stdcall *PCREATETOOLHELP32SNAPSHOT)(DWORD, DWORD);
typedef BOOL (WINAPI *PMODULE32FIRST)(HANDLE, LPMODULEENTRY32);
typedef BOOL (WINAPI *PMODULE32NEXT)(HANDLE, LPMODULEENTRY32);

static ModuleInfo *CrashGetModules(void *&ptr)
{
    void *pMem = VirtualAlloc(NULL, 65536, MEM_COMMIT, PAGE_READWRITE);

    if (!pMem) 
    {
        ptr = NULL;
        return NULL;
    }

    // This sucks.  If we're running under Windows 9x, we must use
    // TOOLHELP.DLL to get the module list.  Under Windows NT, we must
    // use PSAPI.DLL.  With Windows 2000, we can use both (but prefer
    // PSAPI.DLL).

    HMODULE hmodPSAPI = LoadLibrary("psapi.dll");

    if (hmodPSAPI) 
    {
        // Using PSAPI.DLL.  Call EnumProcessModules(), then GetModuleFileNameEx()
        // and GetModuleInformation().

        PENUMPROCESSMODULES pEnumProcessModules = (PENUMPROCESSMODULES)GetProcAddress(hmodPSAPI, "EnumProcessModules");
        PGETMODULEBASENAME pGetModuleBaseName = (PGETMODULEBASENAME)GetProcAddress(hmodPSAPI, "GetModuleBaseNameA");
        PGETMODULEINFORMATION pGetModuleInformation = (PGETMODULEINFORMATION)GetProcAddress(hmodPSAPI, "GetModuleInformation");
        HMODULE* pModules;
        HMODULE* pModules0 = (HMODULE *)((char *)pMem + 0xF000);
        DWORD cbNeeded;

        if (pEnumProcessModules && pGetModuleBaseName && pGetModuleInformation
            && pEnumProcessModules(GetCurrentProcess(), pModules0, 0x1000, &cbNeeded)) 
        {

            ModuleInfo *pMod, *pMod0;
            char *pszHeap = (char *)pMem, *pszHeapLimit;

            if (cbNeeded > 0x1000)
            {
                cbNeeded = 0x1000;
            }

            pModules = (HMODULE *)((char *)pMem + 0x10000 - cbNeeded);
            memmove(pModules, pModules0, cbNeeded);

            pMod = pMod0 = (ModuleInfo *)((char *)pMem + 0x10000 - sizeof(ModuleInfo) * (cbNeeded / sizeof(HMODULE) + 1));
            pszHeapLimit = (char *)pMod;

            do 
            {
                HMODULE hCurMod = *pModules++;
                Win32ModuleInfo mi;

                if (pGetModuleBaseName(GetCurrentProcess(), hCurMod, pszHeap, pszHeapLimit - pszHeap)
                    && pGetModuleInformation(GetCurrentProcess(), hCurMod, &mi, sizeof mi)) 
                {

                    char *period = NULL;

                    pMod->name = pszHeap;

                    while(*pszHeap++)
                    {
                        if (pszHeap[-1] == '.')
                        {
                            period = pszHeap-1;
                        }
                    }

                    if (period) 
                    {
                        *period = 0;
                        pszHeap = period+1;
                    }

                    pMod->base = mi.base;
                    pMod->size = mi.size;
                    ++pMod;
                }
            } 
            while((cbNeeded -= sizeof(HMODULE *)) > 0);

            pMod->name = NULL;

            FreeLibrary(hmodPSAPI);
            ptr = pMem;
            return pMod0;
        }

        FreeLibrary(hmodPSAPI);
    }
    else 
    {
        // No PSAPI.  Use the ToolHelp functions in KERNEL.

        HMODULE hmodKERNEL32 = LoadLibrary("kernel32.dll");

        PCREATETOOLHELP32SNAPSHOT pCreateToolhelp32Snapshot = (PCREATETOOLHELP32SNAPSHOT)GetProcAddress(hmodKERNEL32, "CreateToolhelp32Snapshot");
        PMODULE32FIRST pModule32First = (PMODULE32FIRST)GetProcAddress(hmodKERNEL32, "Module32First");
        PMODULE32NEXT pModule32Next = (PMODULE32NEXT)GetProcAddress(hmodKERNEL32, "Module32Next");
        HANDLE hSnap;

        if (pCreateToolhelp32Snapshot && pModule32First && pModule32Next) 
        {
            if ((HANDLE)-1 != (hSnap = pCreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0))) 
            {
                ModuleInfo *pModInfo = (ModuleInfo *)((char *)pMem + 65536);
                char *pszHeap = (char *)pMem;
                MODULEENTRY32 me;

                --pModInfo;
                pModInfo->name = NULL;

                me.dwSize = sizeof(MODULEENTRY32);

                if (pModule32First(hSnap, &me))
                {
                    do 
                    {
                        if (pszHeap+strlen(me.szModule) >= (char *)(pModInfo - 1))
                        {
                            break;
                        }

                        strcpy(pszHeap, me.szModule);

                        --pModInfo;
                        pModInfo->name = pszHeap;

                        char *period = NULL;

                        while(*pszHeap++);
                        {
                            if (pszHeap[-1]=='.')
                            {
                                period = pszHeap-1;
                            }
                        }

                        if (period) 
                        {
                            *period = 0;
                            pszHeap = period+1;
                        }

                        pModInfo->base = (unsigned long)me.modBaseAddr;
                        pModInfo->size = me.modBaseSize;

                    } 
                    while(pModule32Next(hSnap, &me));
                }

                CloseHandle(hSnap);

                FreeLibrary(hmodKERNEL32);

                ptr = pMem;
                return pModInfo;
            }
        }

        FreeLibrary(hmodKERNEL32);
    }

    VirtualFree(pMem, 0, MEM_RELEASE);

    ptr = NULL;
    return NULL;
}

///////////////////////////////////////////////////////////////////////////
//
//  info from Portable Executable/Common Object File Format (PE/COFF) spec

typedef unsigned short ushort;
typedef unsigned long ulong;

struct PEHeader 
{
    ulong       signature;
    ushort      machine;
    ushort      sections;
    ulong       timestamp;
    ulong       symbol_table;
    ulong       symbols;
    ushort      opthdr_size;
    ushort      characteristics;
};

struct PESectionHeader 
{
    char        name[8];
    ulong       virtsize;
    ulong       virtaddr;
    ulong       rawsize;
    ulong       rawptr;
    ulong       relocptr;
    ulong       linenoptr;
    ushort      reloc_cnt;
    ushort      lineno_cnt;
    ulong       characteristics;
};

struct PEExportDirectory 
{
    ulong       flags;
    ulong       timestamp;
    ushort      major;
    ushort      minor;
    ulong       nameptr;
    ulong       ordbase;
    ulong       addrtbl_cnt;
    ulong       nametbl_cnt;
    ulong       addrtbl_ptr;
    ulong       nametbl_ptr;
    ulong       ordtbl_ptr;
};

struct PE32OptionalHeader 
{
    ushort      magic;                  // 0
    char        major_linker_ver;       // 2
    char        minor_linker_ver;       // 3
    ulong       codesize;               // 4
    ulong       idatasize;              // 8
    ulong       udatasize;              // 12
    ulong       entrypoint;             // 16
    ulong       codebase;               // 20
    ulong       database;               // 24
    ulong       imagebase;              // 28
    ulong       section_align;          // 32
    ulong       file_align;             // 36
    ushort      majoros;                // 40
    ushort      minoros;                // 42
    ushort      majorimage;             // 44
    ushort      minorimage;             // 46
    ushort      majorsubsys;            // 48
    ushort      minorsubsys;            // 50
    ulong       reserved;               // 52
    ulong       imagesize;              // 56
    ulong       hdrsize;                // 60
    ulong       checksum;               // 64
    ushort      subsystem;              // 68
    ushort      characteristics;        // 70
    ulong       stackreserve;           // 72
    ulong       stackcommit;            // 76
    ulong       heapreserve;            // 80
    ulong       heapcommit;             // 84
    ulong       loaderflags;            // 88
    ulong       dictentries;            // 92

    // Not part of header, but it's convienent here

    ulong       export_RVA;             // 96
    ulong       export_size;            // 100
};

struct PE32PlusOptionalHeader 
{
    ushort      magic;                  // 0
    char        major_linker_ver;       // 2
    char        minor_linker_ver;       // 3
    ulong       codesize;               // 4
    ulong       idatasize;              // 8
    ulong       udatasize;              // 12
    ulong       entrypoint;             // 16
    ulong       codebase;               // 20
    __int64     imagebase;              // 24
    ulong       section_align;          // 32
    ulong       file_align;             // 36
    ushort      majoros;                // 40
    ushort      minoros;                // 42
    ushort      majorimage;             // 44
    ushort      minorimage;             // 46
    ushort      majorsubsys;            // 48
    ushort      minorsubsys;            // 50
    ulong       reserved;               // 52
    ulong       imagesize;              // 56
    ulong       hdrsize;                // 60
    ulong       checksum;               // 64
    ushort      subsystem;              // 68
    ushort      characteristics;        // 70
    __int64     stackreserve;           // 72
    __int64     stackcommit;            // 80
    __int64     heapreserve;            // 88
    __int64     heapcommit;             // 96
    ulong       loaderflags;            // 104
    ulong       dictentries;            // 108

    // Not part of header, but it's convienent here

    ulong       export_RVA;             // 112
    ulong       export_size;            // 116
};

static const char *CrashLookupExport(HMODULE hmod, unsigned long addr, unsigned long &fnbase) 
{
    char *pBase = (char *)hmod;

    // The PEheader offset is at hmod+0x3c.  Add the size of the optional header
    // to step to the section headers.

    PEHeader *pHeader = (PEHeader *)(pBase + ((long *)hmod)[15]);

    if (pHeader->signature != 'EP')
    {
        return NULL;
    }

    // Verify the optional structure.

    PEExportDirectory *pExportDir;

    if (pHeader->opthdr_size < 104)
    {
        return NULL;
    }

    switch(*(short *)((char *)pHeader + sizeof(PEHeader))) 
    {
    case 0x10b:     // PE32
        {
            PE32OptionalHeader *pOpt = (PE32OptionalHeader *)((char *)pHeader + sizeof(PEHeader));

            if (pOpt->dictentries < 1)
            {
                return NULL;
            }

            pExportDir = (PEExportDirectory *)(pBase + pOpt->export_RVA);
        }
        break;
    case 0x20b:     // PE32+
        {
            PE32PlusOptionalHeader *pOpt = (PE32PlusOptionalHeader *)((char *)pHeader + sizeof(PEHeader));

            if (pOpt->dictentries < 1)
            {
                return NULL;
            }

            pExportDir = (PEExportDirectory *)(pBase + pOpt->export_RVA);
        }
        break;

    default:
        return NULL;
    }

    // Hmmm... no exports?
    if ((char *)pExportDir == pBase)
    {
        return NULL;
    }

    // Find the location of the export information.
    ulong *pNameTbl = (ulong *)(pBase + pExportDir->nametbl_ptr);
    ulong *pAddrTbl = (ulong *)(pBase + pExportDir->addrtbl_ptr);
    ushort *pOrdTbl = (ushort *)(pBase + pExportDir->ordtbl_ptr);

    // Scan exports.
    const char *pszName = NULL;
    ulong bestdelta = 0xFFFFFFFF;
    int i;

    addr -= (ulong)pBase;

    for(i=0; i<pExportDir->nametbl_cnt; i++) 
    {
        ulong fnaddr;
        int idx;

        idx = pOrdTbl[i];
        fnaddr = pAddrTbl[idx];

        if (addr >= fnaddr) 
        {
            ulong delta = addr - fnaddr;

            if (delta < bestdelta) 
            {
                bestdelta = delta;
                fnbase = fnaddr;

                if (pNameTbl[i])
                {
                    pszName = pBase + pNameTbl[i];
                }
                else 
                {
                    static char buf[8];

                    wsprintf(buf, "ord%d", pOrdTbl[i]);
                    pszName = buf;
                }

            }
        }
    }

    return pszName;
}

///////////////////////////////////////////////////////////////////////////

static bool IsExecutableProtection(DWORD dwProtect) 
{
    MEMORY_BASIC_INFORMATION meminfo;

    // Windows NT/2000 allows Execute permissions, but Win9x seems to
    // rip it off.  So we query the permissions on our own code block,
    // and use it to determine if READONLY/READWRITE should be
    // considered 'executable.'

    VirtualQuery(IsExecutableProtection, &meminfo, sizeof meminfo);

    switch((unsigned char)dwProtect) 
    {
    case PAGE_READONLY:             // *sigh* Win9x...
    case PAGE_READWRITE:            // *sigh*
        return (meminfo.Protect==PAGE_READONLY || meminfo.Protect==PAGE_READWRITE);

    case PAGE_EXECUTE:
    case PAGE_EXECUTE_READ:
    case PAGE_EXECUTE_READWRITE:
    case PAGE_EXECUTE_WRITECOPY:
        return true;
    }
    return false;
}

static const char *CrashGetModuleBaseName(HMODULE hmod, char *pszBaseName) 
{
    char szPath1[MAX_PATH];
    char szPath2[MAX_PATH];

    __try 
    {
        DWORD dw;
        char *pszFile, *period = NULL;

        if (!GetModuleFileName(hmod, szPath1, sizeof szPath1))
        {
            return NULL;
        }

        dw = GetFullPathName(szPath1, sizeof szPath2, szPath2, &pszFile);

        if (!dw || dw>sizeof szPath2)
        {
            return NULL;
        }

        strcpy(pszBaseName, pszFile);

        pszFile = pszBaseName;

        while(*pszFile++)
        {
            if (pszFile[-1]=='.')
            {
                period = pszFile-1;
            }
        }

        if (period)
        {
            *period = 0;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) 
    {
        return NULL;
    }

    return pszBaseName;
}

static bool ReportCrashCallStack(HWND hwnd, HANDLE hFile, const EXCEPTION_POINTERS *const pExc, bool fExtra)
{
    const CONTEXT *const pContext = (const CONTEXT *)pExc->ContextRecord;
    HANDLE hprMe = GetCurrentProcess();
    char *lpAddr = (char *)pContext->Esp;
    int limit = 100;
    unsigned long data, first_rva;
    const char *debug_data = NULL;
    const char *fnname_heap, *classname_heap, *rva_heap;
    const unsigned long (*seg_heap)[2];
    int seg_cnt;

    // Attempt to read debug file.

    char szPath[MAX_PATH];

    SpliceProgramPath(szPath, sizeof szPath, "DScaler.dbg");

    HANDLE hFile2;
    bool fSuccessful = false;
    LONG lFileSize;
    DWORD dwActual;

    do 
    {
        hFile2 = CreateFile(szPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (INVALID_HANDLE_VALUE == hFile2) 
        {
            SpliceProgramPath(szPath, sizeof szPath, "DScaler.dbg");
            hFile2 = CreateFile(szPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        }

        if (INVALID_HANDLE_VALUE == hFile2)
        {
            break;
        }

        lFileSize = GetFileSize(hFile2, NULL);

        if (0xFFFFFFFF == lFileSize)
        {
            break;
        }

        debug_data = (const char *)VirtualAlloc(NULL, lFileSize, MEM_COMMIT, PAGE_READWRITE);
        if (!debug_data)
        {
            break;
        }

        if (!ReadFile(hFile2, (void *)debug_data, lFileSize, &dwActual, NULL) || dwActual!=lFileSize)
        {
            break;
        }

        fSuccessful = true;
    } 
    while(0);

    if (hFile2 != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile2);
    }

    if (!fSuccessful) 
    {
        if (debug_data)
        {
            VirtualFree((void *)debug_data, 0, MEM_RELEASE);
        }

        Report(hwnd, hFile, "Could not open debug resource file.");
        return false;
    }

    rva_heap = debug_data + 20;
    classname_heap = rva_heap + ((long *)debug_data)[1];
    fnname_heap = classname_heap + ((long *)debug_data)[2];
    seg_heap = (unsigned long (*)[2])(fnname_heap + ((long *)debug_data)[3]);
    seg_cnt = ((long *)debug_data)[4];

    first_rva = *(long *)rva_heap;
    rva_heap += 4;

    // Get some module names.

    void *pModuleMem;
    ModuleInfo *pModules = CrashGetModules(pModuleMem);

    // Walk up the stack.  Hopefully it wasn't fscked.

    if (*(long *)debug_data != gBuildNum) 
    {
        Report(hwnd, hFile, "Wrong DScaler.DBG file (build %d)", *(long *)debug_data);
    } 
    else 
    {
        data = pContext->Eip;
        do 
        {
            int i;

            bool fValid = true;
            char buf[7];
            int len;
            MEMORY_BASIC_INFORMATION meminfo;
            long parm1=0, parm2=0;

            VirtualQuery((void *)data, &meminfo, sizeof meminfo);
            
            if (!IsExecutableProtection(meminfo.Protect) || meminfo.State!=MEM_COMMIT) 
            {
                fValid = false;
            }

            if (data != pContext->Eip) 
            {
                len = 7;

                while(len > 0 && !ReadProcessMemory(GetCurrentProcess(), (void *)(data-len), buf+7-len, len, NULL))
                {
                    --len;
                }

                fValid &= IsValidCall(buf+7, len);

                if (fValid) 
                {
                    ReadProcessMemory(GetCurrentProcess(), (void *)(lpAddr+0), &parm1, 4, NULL);
                    ReadProcessMemory(GetCurrentProcess(), (void *)(lpAddr+4), &parm2, 4, NULL);
                }
            }
            
            if (fValid) 
            {
                for(i=0; i<seg_cnt; i++)
                {
                    if (data >= seg_heap[i][0] && data < seg_heap[i][0] + seg_heap[i][1])
                    {
                        break;
                    }
                }

                if (i>=seg_cnt) 
                {
                    ModuleInfo *pMods = pModules;
                    ModuleInfo mi;
                    char szName[MAX_PATH];

                    mi.name = NULL;

                    if (pMods) 
                    {
                        while(pMods->name) 
                        {
                            if (data >= pMods->base && (data - pMods->base) < pMods->size)
                            {
                                break;
                            }

                            ++pMods;
                        }

                        mi = *pMods;
                    } 
                    else
                    {
                        // Well, something failed, or we didn't have either PSAPI.DLL or ToolHelp
                        // to play with.  So we'll use a nastier method instead.
                        mi.base = (unsigned long)meminfo.AllocationBase;
                        mi.name = CrashGetModuleBaseName((HMODULE)mi.base, szName);
                    }

                    if (mi.name) 
                    {
                        unsigned long fnbase;
                        const char *pExportName = CrashLookupExport((HMODULE)mi.base, data, fnbase);

                        if (pExportName)
                        {
                            Report(hwnd, hFile, "%08lx: %s!%s(%lx, %lx) [%08lx+%lx+%lx]", data, mi.name, pExportName, parm1, parm2, mi.base, fnbase, (data-mi.base-fnbase));
                        }
                        else
                        {
                            Report(hwnd, hFile, "%08lx: %s!%08lx(%lx, %lx)", data, mi.name, data - mi.base, parm1, parm2);
                        }
                    } 
                    else
                    {
                        Report(hwnd, hFile, "%08lx: %08lx(%lx, %lx)", data, data, parm1, parm2);
                    }

                    --limit;
                }
                else 
                {
                    int idx = -1;
                    const char *pp = rva_heap;
                    long rva = data;
                    long diff = 0;

                    // scan down the RVAs

                    rva -= first_rva;

                    while(rva >= 0 && pp<classname_heap) 
                    {
                        char c;

                        diff = 0;
                        do 
                        {
                            c = *pp++;

                            diff = (diff<<7) | (c & 0x7f);
                        }
                        while(c & 0x80);

                        rva -= diff;
                        ++idx;
                    }

                    if (pp<classname_heap && idx>=0) 
                    {
                        const char *fn_name = GetNameFromHeap(fnname_heap, idx);
                        const char *class_name = NULL;
                        const char *prefix = "";

                        if (*fn_name < 32) 
                        {
                            int class_idx;

                            class_idx = ((unsigned)fn_name[0] - 1)*128 + ((unsigned)fn_name[1] - 1);
                            class_name = GetNameFromHeap(classname_heap, class_idx);

                            fn_name += 2;

                            if (*fn_name == 1) 
                            {
                                fn_name = class_name;
                            } 
                            else if (*fn_name == 2) 
                            {
                                fn_name = class_name;
                                prefix = "~";
                            } 
                            else if (*fn_name < 32)
                            {
                                fn_name = "(special)";
                            }
                        }

                        Report(hwnd, hFile, "%08lx: %s%s%s%s(%lx, %lx)", data, class_name?class_name:"", class_name?"::":"", prefix, fn_name, parm1, parm2);
                        --limit;
                    } 
                    else
                    { 
                        fValid = false;
                    }
                }
            }

            if (fValid && fExtra) 
            {
                char c;
                char buf[80];
                char *dst = (char *)parm1;
                int j;

                // xxxxxxxx: xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx ................
                for(j=0; j<2; j++) 
                {
                    wsprintf(buf, "%08lx:%65c", dst, ' ');

                    for(i=0; i<16; i++)
                    {
                        if (ReadProcessMemory(GetCurrentProcess(), dst+i, &c, 1, NULL)) 
                        {
                            wsprintf(buf+10+3*i, "%02x", (int)(unsigned char)c);
                            buf[12+3*i]=' ';
                            buf[58+i]=isprint(c)?c:'.';
                        } 
                        else 
                        {
                            buf[10+3*i]='?';
                            buf[11+3*i]='?';
                            buf[58+i]='?';
                        }
                    }

                    Report(hwnd, hFile, "\t%s", buf);

                    dst = (char *)parm2;
                }
                Report(hwnd, hFile, "");

            }

            lpAddr += 4;
        } 
        while(limit > 0 && ReadProcessMemory(hprMe, lpAddr-4, &data, 4, NULL));
    }

    if (pModuleMem)
    {
        VirtualFree(pModuleMem, 0, MEM_RELEASE);
    }

    VirtualFree((void *)debug_data, 0, MEM_RELEASE);

    return true;
}

static void DoSave(const EXCEPTION_POINTERS *pExc, bool fSaveExtra) 
{
    HANDLE hFile;
    char szModName2[MAX_PATH];
    char tbuf[256];
    long idx;

    SpliceProgramPath(szModName2, sizeof szModName2, "crashinfo.txt");

    hFile = CreateFile(szModName2, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (INVALID_HANDLE_VALUE == hFile)
    {
        return;
    }

    Report(NULL, hFile,
            "DScaler crash report -- build %d\r\n"
            "--------------------------------------\r\n"
            "\r\n"
            "Disassembly:", gBuildNum);

    idx = 0;

    while(idx = g_pcdw->getInstruction(tbuf, idx)) 
    {
        Report(NULL, hFile, "%s", tbuf);
    }

    Report(NULL, hFile, "");

    // Detect operating system.

    OSVERSIONINFO ovi;
    ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (GetVersionEx(&ovi)) 
    {
        Report(NULL, hFile, "Windows %d.%d (Win%s build %d) [%s]"
            ,ovi.dwMajorVersion
            ,ovi.dwMinorVersion
            ,ovi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS
            ? (ovi.dwMinorVersion>0 ? "98" : "95")
                : ovi.dwPlatformId == VER_PLATFORM_WIN32_NT
                    ? (ovi.dwMajorVersion >= 5 ? "2000" : "NT")
                    : "?"
            ,ovi.dwBuildNumber & 0xffff
            ,ovi.szCSDVersion);
    }

    Report(NULL, hFile, "");

    ReportCrashData(NULL, NULL, hFile, pExc);

    Report(NULL, hFile, "");

    ReportCrashCallStack(NULL, hFile, pExc, fSaveExtra);

    Report(NULL, hFile, "\r\n-- End of report");

    // try and make sure this gets written to disk
    // as we may crash the machine very soon
    FlushFileBuffers(hFile);

    CloseHandle(hFile);
}

BOOL APIENTRY CrashDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
    static const EXCEPTION_POINTERS *s_pExc;
    static bool s_bHaveCallstack;

    switch(msg) 
    {
    case WM_INITDIALOG:
        {
            HWND hwndList1 = GetDlgItem(hDlg, IDC_ASMBOX);
            HWND hwndList2 = GetDlgItem(hDlg, IDC_REGDUMP);
            HWND hwndList3 = GetDlgItem(hDlg, IDC_CALL_STACK);
            HWND hwndReason = GetDlgItem(hDlg, IDC_STATIC_BOMBREASON);
            const EXCEPTION_POINTERS *const pExc = (const EXCEPTION_POINTERS *)lParam;
            const EXCEPTION_RECORD *const pRecord = (const EXCEPTION_RECORD *)pExc->ExceptionRecord;
            const CONTEXT *const pContext = (const CONTEXT *)pExc->ContextRecord;

            s_pExc = pExc;

            g_pcdw->DoInitListbox(hwndList1);

            SendMessage(hwndList2, WM_SETFONT, SendMessage(hwndList1, WM_GETFONT, 0, 0), MAKELPARAM(TRUE, 0));
            SendMessage(hwndList3, WM_SETFONT, SendMessage(hwndList1, WM_GETFONT, 0, 0), MAKELPARAM(TRUE, 0));

            ReportCrashData(hwndList2, hwndReason, NULL, pExc);
            s_bHaveCallstack = ReportCrashCallStack(hwndList3, NULL, pExc, false);

        }
        return TRUE;

    case WM_COMMAND:
        switch(LOWORD(wParam)) 
        {
        case IDCANCEL: 
        case IDOK:
            EndDialog(hDlg, FALSE);
            return TRUE;
        case IDC_SAVEPLUS:
        case IDC_SAVE2:
            if (!s_bHaveCallstack)
            {
                if (IDOK != MessageBox(hDlg,
                    "DScaler cannot load its crash resource file, and thus the crash dump will be "
                    "missing the most important part, the call stack. Crash dumps are much less useful "
                    "to the author without the call stack.",
                    "DScaler warning", MB_OK|MB_ICONEXCLAMATION))
                {
                    return TRUE;
                }
            }
            DoSave(s_pExc, LOWORD(wParam)==IDC_SAVEPLUS);
            return TRUE;
        }
        break;

    case WM_MEASUREITEM:
        return g_pcdw->DoMeasureItem(lParam);

    case WM_DRAWITEM:
        return g_pcdw->DoDrawItem(lParam);
    }

    return FALSE;
}

LONG WINAPI CrashHandler(EXCEPTION_POINTERS *pExc) 
{
    LPTOP_LEVEL_EXCEPTION_FILTER OldHandler = SetUnhandledExceptionFilter(NULL);

    static char buf[CODE_WINDOW+16];
    HANDLE hprMe = GetCurrentProcess();
    void *lpBaseAddress = pExc->ExceptionRecord->ExceptionAddress;
    char *lpAddr = (char *)((long)lpBaseAddress & -32);

    memset(buf, 0, sizeof buf);

    if ((unsigned long)lpAddr > CODE_WINDOW/2)
    {
        lpAddr -= CODE_WINDOW/2;
    }
    else
    {
        lpAddr = NULL;
    }

    if (!ReadProcessMemory(hprMe, lpAddr, buf, CODE_WINDOW, NULL))
    {
        int i;

        for(i=0; i<CODE_WINDOW; i+=32)
        {
            if (!ReadProcessMemory(hprMe, lpAddr+i, buf+i, 32, NULL))
            {
                memset(buf+i, 0, 32);
            }
        }
    }

    CCodeDisassemblyWindow cdw(buf, CODE_WINDOW, (char *)(buf-lpAddr), lpAddr);

    g_pcdw = &cdw;

    cdw.setFaultAddress(lpBaseAddress);

    if(bShowCrashDialog)
    {
        DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_DISASM_CRASH), NULL, CrashDlgProc, (LPARAM)pExc);
    }
    else
    {
        DoSave(pExc, TRUE);
    }

    SetUnhandledExceptionFilter(OldHandler);

    return EXCEPTION_EXECUTE_HANDLER;
}

LONG WINAPI UnexpectedCrashHandler(EXCEPTION_POINTERS *pExc)  
{
    CrashHandler(pExc);
    return EXCEPTION_CONTINUE_SEARCH;
}