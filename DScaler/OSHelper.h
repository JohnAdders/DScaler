/////////////////////////////////////////////////////////////////////////////
// $Id: OSHelper.h,v 1.1 2001-07-24 12:19:00 adcockj Exp $
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

#ifndef f_OSHELPER_H
#define f_OSHELPER_H

#include <windows.h>

void Draw3DRect(HDC hDC, LONG x, LONG y, LONG dx, LONG dy, BOOL inverted);

HKEY OpenConfigKey(const char *szKeyName);
HKEY CreateConfigKey(const char *szKeyName);
BOOL DeleteConfigValue(const char *szKeyName, const char *szValueName);
BOOL QueryConfigString(const char *szKeyName, const char *szValueName, char *lpBuffer, int cbBuffer);
DWORD QueryConfigBinary(const char *szKeyName, const char *szValueName, char *lpBuffer, int cbBuffer);
BOOL QueryConfigDword(const char *szKeyName, const char *szValueName, DWORD *lpdwData);
BOOL SetConfigString(const char *szKeyName, const char *szValueName, const char *lpBuffer);
BOOL SetConfigBinary(const char *szKeyName, const char *szValueName, const char *lpBuffer, int cbBuffer);
BOOL SetConfigDword(const char *szKeyName, const char *szValueName, DWORD dwData);

void HelpSetPath();
const char *HelpGetPath();
void HelpShowHelp(HWND hwnd);
void HelpContext(HWND hwnd, DWORD helpID);
void HelpPopup(HWND hwnd, DWORD helpID);
void HelpPopupByID(HWND hwnd, DWORD ctrlID, const DWORD *lookup);

__int64 MyGetDiskFreeSpace(const char *lpRoot);

const char *SplitPathName(const char *path);
inline char *SplitPathName(char *path) { return const_cast<char *>(SplitPathName(const_cast<const char *>(path))); }

const char *SplitPathExt(const char *path);
inline char *SplitPathExt(char *path) { return const_cast<char *>(SplitPathExt(const_cast<const char *>(path))); }

char *SplitPathRoot(char *dst, const char *path);
char *MergePath(char *path, const char *fn);
bool IsFilenameOnFATVolume(const char *pszFilename);

void LaunchURL(const char *pURL);

class CPUUsageReader {
public:
	CPUUsageReader();
	~CPUUsageReader();
	int read();

private:
	bool fNTMethod;
	HKEY hkeyKernelCPU;

	unsigned __int64 kt_last;
	unsigned __int64 ut_last;
	unsigned __int64 st_last;
};

#endif
