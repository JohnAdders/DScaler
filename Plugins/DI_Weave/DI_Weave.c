/////////////////////////////////////////////////////////////////////////////
// DI_Weave.c
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 30 Dec 2000   Mark Rejhon           Split into separate module
//
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DS_Deinterlace.h"

///////////////////////////////////////////////////////////////////////////////
// Simple Weave.  Copies alternating scanlines from the most recent fields.
BOOL DeinterlaceWeave(DEINTERLACE_INFO *info)
{
	int i;
	BYTE *lpOverlay = info->Overlay;

	if (info->EvenLines[0] == NULL || info->OddLines[0] == NULL)
		return FALSE;

	for (i = 0; i < info->FieldHeight; i++)
	{
		info->pMemcpy(lpOverlay, info->EvenLines[0][i], info->LineLength);
		lpOverlay += info->OverlayPitch;

		info->pMemcpy(lpOverlay, info->OddLines[0][i], info->LineLength);
		lpOverlay += info->OverlayPitch;
	}
	_asm
	{
		emms
	}
	return TRUE;
}

DEINTERLACE_METHOD WeaveMethod =
{
	sizeof(DEINTERLACE_METHOD),
	DEINTERLACE_CURRENT_VERSION,
	"Simple Weave", 
	NULL,
	FALSE, 
	FALSE, 
	DeinterlaceWeave, 
	50, 
	60,
	0,
	NULL,
	INDEX_WEAVE,
	NULL,
	NULL,
	NULL,
	2,
	0,
	0,
	-1,
	NULL,
	0,
	FALSE,
	FALSE,
};


__declspec(dllexport) DEINTERLACE_METHOD* GetDeinterlacePluginInfo(long CpuFeatureFlags)
{
	return &WeaveMethod;
}

BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}
