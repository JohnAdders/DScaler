/////////////////////////////////////////////////////////////////////////////
// DI_EvenOnly.c
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
// 04 Jan 2001   John Adcock           Split into separate module
//
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "dTV_Deinterlace.h"


BOOL DeinterlaceEvenOnly(DEINTERLACE_INFO *info)
{
	int nLineTarget;

	if (!info->IsOdd)
	{
		for (nLineTarget = 0; nLineTarget < info->FieldHeight; nLineTarget++)
		{
			// copy latest field's rows to overlay, resulting in a half-height image.
			info->pMemcpy(info->Overlay + nLineTarget * info->OverlayPitch,
						info->EvenLines[0][nLineTarget],
						info->LineLength);
		}
		// need to clear up MMX registers
		_asm
		{
			emms
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


DEINTERLACE_METHOD EvenOnlyMethod =
{
	sizeof(DEINTERLACE_METHOD),
	DEINTERLACE_CURRENT_VERSION,
	"Even Scanlines Only", 
	"Even",
	TRUE, 
	FALSE, 
	DeinterlaceEvenOnly, 
	25, 
	30,
	0,
	NULL,
	INDEX_EVEN_ONLY,
	NULL,
	NULL,
	NULL,
	1,
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
	return &EvenOnlyMethod;
}

BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}


