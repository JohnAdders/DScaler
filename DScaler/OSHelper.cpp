/////////////////////////////////////////////////////////////////////////////
// $Id: OSHelper.cpp,v 1.1 2001-07-24 12:19:00 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1998-2001 Avery Lee.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
/////////////////////////////////////////////////////////////////////////////
// This file was taken from VirtualDub
// VirtualDub - Video processing and capture application
// Copyright (C) 1998-2001 Avery Lee.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "oshelper.h"

void Draw3DRect(HDC hDC, LONG x, LONG y, LONG dx, LONG dy, BOOL inverted) {
	HPEN hPenOld;

	hPenOld = (HPEN)SelectObject(hDC, GetStockObject(inverted ? WHITE_PEN : BLACK_PEN));
	MoveToEx(hDC, x, y+dy-1, NULL);
	LineTo(hDC, x+dx-1, y+dy-1);
	LineTo(hDC, x+dx-1, y);
	DeleteObject(SelectObject(hDC, GetStockObject(inverted ? BLACK_PEN : WHITE_PEN)));
	MoveToEx(hDC, x, y+dy-1, NULL);
	LineTo(hDC, x, y);
	LineTo(hDC, x+dx-1, y);
	DeleteObject(SelectObject(hDC, hPenOld));
}

// We follow MAME32's lead and put our keys in:
//
//	HKEY_CURRENT_USER\Software\Freeware\DScaler\
//

HKEY OpenConfigKey(const char *szKeyName) {
	char temp[MAX_PATH]="Software\\Freeware\\DScaler";
	HKEY hkey;

	if (szKeyName) {
		strcat(temp, "\\");
		strcat(temp, szKeyName);
	}

	return RegOpenKeyEx(HKEY_CURRENT_USER, temp, 0, KEY_ALL_ACCESS, &hkey)==ERROR_SUCCESS
			? hkey
			: NULL;
}

HKEY CreateConfigKey(const char *szKeyName) {
	char temp[MAX_PATH]="Software\\Freeware\\DScaler";
	HKEY hkey;
	DWORD dwDisposition;

	if (szKeyName) {
		strcat(temp, "\\");
		strcat(temp, szKeyName);
	}

	return RegCreateKeyEx(HKEY_CURRENT_USER, temp, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisposition)==ERROR_SUCCESS
			? hkey
			: NULL;
}

BOOL DeleteConfigValue(const char *szKeyName, const char *szValueName) {
	HKEY hkey;
	BOOL success;

	if (!(hkey = OpenConfigKey(szKeyName)))
		return FALSE;

	success = (RegDeleteValue(hkey, szValueName) == ERROR_SUCCESS);

	RegCloseKey(hkey);

	return success;
}

BOOL QueryConfigString(const char *szKeyName, const char *szValueName, char *lpBuffer, int cbBuffer) {
	HKEY hkey;
	BOOL success;
	DWORD type;

	if (!(hkey = OpenConfigKey(szKeyName)))
		return FALSE;

	success = (ERROR_SUCCESS == RegQueryValueEx(hkey, szValueName, 0, &type, (LPBYTE)lpBuffer, (LPDWORD)&cbBuffer));

	RegCloseKey(hkey);

	return success;
}

DWORD QueryConfigBinary(const char *szKeyName, const char *szValueName, char *lpBuffer, int cbBuffer) {
	HKEY hkey;
	BOOL success;
	DWORD type;
	DWORD size = cbBuffer;

	if (!(hkey = OpenConfigKey(szKeyName)))
		return 0;

	success = (ERROR_SUCCESS == RegQueryValueEx(hkey, szValueName, 0, &type, (LPBYTE)lpBuffer, (LPDWORD)&size));

	RegCloseKey(hkey);

	return success ? size : 0;
}

BOOL QueryConfigDword(const char *szKeyName, const char *szValueName, DWORD *lpdwData) {
	HKEY hkey;
	BOOL success;
	DWORD type;
	DWORD size = sizeof(DWORD);

	if (!(hkey = OpenConfigKey(szKeyName)))
		return 0;

	success = (ERROR_SUCCESS == RegQueryValueEx(hkey, szValueName, 0, &type, (LPBYTE)lpdwData, (LPDWORD)&size));

	RegCloseKey(hkey);

	return success;
}

BOOL SetConfigString(const char *szKeyName, const char *szValueName, const char *lpBuffer) {
	HKEY hkey;
	BOOL success;

	if (!(hkey = CreateConfigKey(szKeyName)))
		return FALSE;

	success = (ERROR_SUCCESS == RegSetValueEx(hkey, szValueName, 0, REG_SZ, (LPBYTE)lpBuffer, strlen(lpBuffer)+1));

	RegCloseKey(hkey);

	return success;
}

BOOL SetConfigBinary(const char *szKeyName, const char *szValueName, const char *lpBuffer, int cbBuffer) {
	HKEY hkey;
	BOOL success;

	if (!(hkey = CreateConfigKey(szKeyName)))
		return FALSE;

	success = (ERROR_SUCCESS == RegSetValueEx(hkey, szValueName, 0, REG_BINARY, (LPBYTE)lpBuffer, cbBuffer));

	RegCloseKey(hkey);

	return success;
}

BOOL SetConfigDword(const char *szKeyName, const char *szValueName, DWORD dwData) {
	HKEY hkey;
	BOOL success;

	if (!(hkey = CreateConfigKey(szKeyName)))
		return FALSE;

	success = (ERROR_SUCCESS == RegSetValueEx(hkey, szValueName, 0, REG_DWORD, (LPBYTE)&dwData, sizeof(DWORD)));

	RegCloseKey(hkey);

	return success;
}

///////////////////////////////////////////////////////////////////////////
//
//	help support
//
///////////////////////////////////////////////////////////////////////////

static char g_szHelpPath[MAX_PATH]="VirtualD.hlp";

void HelpSetPath() {
	char szPath[MAX_PATH];
	char *lpFilePart;
	char *ext = NULL;

	if (GetModuleFileName(NULL, szPath, sizeof szPath))
		if (GetFullPathName(szPath, sizeof g_szHelpPath, g_szHelpPath, &lpFilePart))
			strcpy(lpFilePart,"VirtualD.hlp");
}

const char *HelpGetPath() {
	return g_szHelpPath;
}

void HelpShowHelp(HWND hwnd) {
	WinHelp(hwnd, g_szHelpPath, HELP_FINDER, 0);
}

void HelpContext(HWND hwnd, DWORD helpID) {
	WinHelp(hwnd, g_szHelpPath, HELP_CONTEXT, helpID);
}

void HelpPopup(HWND hwnd, DWORD helpID) {
	WinHelp(hwnd, g_szHelpPath, HELP_CONTEXTPOPUP, helpID);
}

void HelpPopupByID(HWND hwnd, DWORD ctrlID, const DWORD *lookup) {
	while(lookup[0]) {
		if (lookup[0] == ctrlID)
			HelpPopup(hwnd, lookup[1]);

		lookup+=2;
	}
}


///////////////////////////////////////////////////////////////////////////
//
//	disk free space
//
///////////////////////////////////////////////////////////////////////////

static HINSTANCE g_hInstKernel32 = NULL;
static BOOL (__stdcall *g_fpGetDiskFreeSpaceEx)(LPCTSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER) = NULL;
static bool g_triedGetDiskFreeSpaceEx = false;

__int64 MyGetDiskFreeSpace(const char *lpszRoot) {
	__int64 client_free, total_space, free_space;
	DWORD dwSectorsPerCluster, dwBytesPerSector, dwFreeClusters, dwTotalClusters;
	char tmp[MAX_PATH];

	if (!g_hInstKernel32 && !(g_hInstKernel32 = LoadLibrary("kernel32.dll")))
		return -1;

	if (!g_triedGetDiskFreeSpaceEx) {
		g_fpGetDiskFreeSpaceEx = (BOOL (__stdcall *)(LPCTSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER))GetProcAddress(g_hInstKernel32, "GetDiskFreeSpaceExA");
		g_triedGetDiskFreeSpaceEx = true;
	}

	lpszRoot = SplitPathRoot(tmp, lpszRoot);

	if (g_fpGetDiskFreeSpaceEx) {
		return g_fpGetDiskFreeSpaceEx(lpszRoot, (PULARGE_INTEGER)&client_free, (PULARGE_INTEGER)&total_space, (PULARGE_INTEGER)&free_space)
			? free_space
			: -1;
	} else {
		return GetDiskFreeSpace(lpszRoot, &dwSectorsPerCluster, &dwBytesPerSector, &dwFreeClusters, &dwTotalClusters)
			? dwFreeClusters * dwSectorsPerCluster * dwBytesPerSector
			: -1;
	}
}


const char *SplitPathName(const char *path) {
	const char *s = path;

	while(*s) ++s;

	while(s>path && s[-1]!='\\' && s[-1]!=':')
		--s;

	return s;
}

const char *SplitPathExt(const char *path) {
	const char *s = path;
	const char *tail;

	while(*s) ++s;

	tail = s;

	while(s>path && s[-1]!='\\' && s[-1]!=':') {
		if (s[-1]=='.')
			return s-1;

		--s;
	}

	return tail;
}

char *MergePath(char *path, const char *fn) {
	char *slash=NULL, *colon=NULL;
	char *s = path;

	if (!*s) {
		strcpy(path, fn);
		return path;
	}

	while(*s)
		++s;

	if (s[-1]!='\\' && s[-1]!=':')
		*s++ = '\\';

	strcpy(s, fn);

	return path;
}

char *SplitPathRoot(char *dst, const char *path) {

	if (!path)
		return NULL;

	// C:

	if (isalpha(path[0]) && path[1]==':') {
		dst[0] = path[0];
		dst[1] = ':';
		dst[2] = '\\';
		dst[3] = 0;

		return dst;
	}

	// UNC path?

	if (path[0] == '\\' && path[1] == '\\') {
		const char *s = path+2;
		char *t = dst;

		*t++ = '\\';
		*t++ = '\\';

		while(*s && *s != '\\')
			*t++ = *s++;

		if (*s)
			*t++ = *s++;

		while(*s && *s != '\\')
			*t++ = *s++;

		*t++ = '\\';
		*t = 0;

		return dst;
	}

	return NULL;
}

bool IsFilenameOnFATVolume(const char *pszFilename) {
	char szFileRoot[MAX_PATH];
	DWORD dwMaxComponentLength;
	DWORD dwFSFlags;
	char szFilesystem[MAX_PATH];

	if (!GetVolumeInformation(SplitPathRoot(szFileRoot, pszFilename),
			NULL, 0,		// Volume name buffer
			NULL,			// Serial number buffer
			&dwMaxComponentLength,
			&dwFSFlags,
			szFilesystem,
			sizeof szFilesystem))
		return false;

	return !strnicmp(szFilesystem, "FAT", 3);
}

///////////////////////////////////////////////////////////////////////////

void LaunchURL(const char *pURL) {
	ShellExecute(NULL, "open", pURL, NULL, NULL, SW_SHOWNORMAL);
}

///////////////////////////////////////////////////////////////////////////

bool EnableCPUTracking() {
	HKEY hOpen;
	DWORD cbData;
	DWORD dwType;
	LPBYTE pByte;
	DWORD rc;

	bool fSuccess = true;

    if ( (rc = RegOpenKeyEx(HKEY_DYN_DATA,"PerfStats\\StartStat", 0,
					KEY_READ, &hOpen)) == ERROR_SUCCESS) {

		// query to get data size
		if ( (rc = RegQueryValueEx(hOpen,"KERNEL\\CPUUsage",NULL,&dwType,
				NULL, &cbData )) == ERROR_SUCCESS) {

			pByte = (LPBYTE)malloc(cbData);

			rc = RegQueryValueEx(hOpen,"KERNEL\\CPUUsage",NULL,&dwType, pByte,
                              &cbData );

			free(pByte);
		} else
			fSuccess = false;

		RegCloseKey(hOpen);
	} else
		fSuccess = false;

	return fSuccess;
}

bool DisableCPUTracking() {
	HKEY hOpen;
	DWORD cbData;
	DWORD dwType;
	LPBYTE pByte;
	DWORD rc;

	bool fSuccess = true;

    if ( (rc = RegOpenKeyEx(HKEY_DYN_DATA,"PerfStats\\StopStat", 0,
					KEY_READ, &hOpen)) == ERROR_SUCCESS) {

		// query to get data size
		if ( (rc = RegQueryValueEx(hOpen,"KERNEL\\CPUUsage",NULL,&dwType,
				NULL, &cbData )) == ERROR_SUCCESS) {

			pByte = (LPBYTE)malloc(cbData);

			rc = RegQueryValueEx(hOpen,"KERNEL\\CPUUsage",NULL,&dwType, pByte,
                              &cbData );

			free(pByte);
		} else
			fSuccess = false;

		RegCloseKey(hOpen);
	} else
		fSuccess = false;

	return fSuccess;
}

CPUUsageReader::CPUUsageReader() {
	FILETIME ftCreate, ftExit;

	hkeyKernelCPU = NULL;
	fNTMethod = false;

	if (GetProcessTimes(GetCurrentProcess(), &ftCreate, &ftExit, (FILETIME *)&kt_last, (FILETIME *)&ut_last)) {

		// Using Windows NT/2000 method
		GetSystemTimeAsFileTime((FILETIME *)&st_last);

		fNTMethod = true;

	} else {

		// Using Windows 95/98 method

		HKEY hkey;

		if (EnableCPUTracking()) {

			if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_DYN_DATA, "PerfStats\\StatData", 0, KEY_READ, &hkey)) {
				hkeyKernelCPU = hkey;
			} else
				DisableCPUTracking();
		}
	}
}

CPUUsageReader::~CPUUsageReader() {
	if (hkeyKernelCPU) {
		RegCloseKey(hkeyKernelCPU);
		DisableCPUTracking();
	}
}

int CPUUsageReader::read() {

	if (hkeyKernelCPU) {
		DWORD type;
		DWORD dwUsage;
		DWORD size = sizeof dwUsage;

		if (ERROR_SUCCESS == RegQueryValueEx(hkeyKernelCPU, "KERNEL\\CPUUsage", 0, &type, (LPBYTE)&dwUsage, (LPDWORD)&size))
			return (int)dwUsage;
		
		return -1;
	} else if (fNTMethod) {
		FILETIME ftCreate, ftExit;
		unsigned __int64 kt, st, ut;
		int cpu;

		GetProcessTimes(GetCurrentProcess(), &ftCreate, &ftExit, (FILETIME *)&kt, (FILETIME *)&ut);
		GetSystemTimeAsFileTime((FILETIME *)&st);

		if (st == st_last)
			return 100;
		else
			cpu = (int)((100 * (kt + ut - kt_last - ut_last) + (st - st_last)/2) / (st - st_last));

		kt_last = kt;
		ut_last = ut;
		st_last = st;

		return cpu;
	}

	return -1;
}
