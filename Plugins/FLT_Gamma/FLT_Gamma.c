/////////////////////////////////////////////////////////////////////////////
// FLT_TNoise.c
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

#include "windows.h"
#include "DS_Filter.h"
#include "math.h"

unsigned char GammaTable[256];

FILTER_METHOD GammaMethod;

long Gamma = 1300;
BOOL bUseStoredTable = FALSE;

BOOL FilterGamma(DEINTERLACE_INFO *info)
{
	short *Pixels;
	short *Table;
	int y;
	int Cycles;

	// Need to have the current and next-to-previous fields to do the filtering.
	if ((info->IsOdd && info->OddLines[0] == NULL) ||
		(! info->IsOdd && info->EvenLines[0] == NULL))
	{
		return FALSE;
	}

	Cycles = info->LineLength / 4;

	Table = (short*)GammaTable;

	for (y = 0; y < info->FieldHeight; y++)
	{
		if (info->IsOdd)
		{
			Pixels = info->OddLines[0][y];
		}
		else
		{
			Pixels = info->EvenLines[0][y];
		}

		_asm
		{
			mov ecx, Cycles
			mov edx, dword ptr[Pixels]
			mov ebx, dword ptr[Table]
LOOP_LABEL:
			mov al,byte ptr[edx]
			xlatb
			mov byte ptr[edx], al
			add edx, 2			
			mov al,byte ptr[edx]
			xlatb
			mov byte ptr[edx], al
			add edx, 2			
			loop LOOP_LABEL
		}

	}
	return TRUE;
}

double GetGammaAdjustedValue(double Input, double Gamma)
{
	return pow(Input, Gamma);
}

BOOL Gamma_OnChange(long NewValue)
{
	int i;
	double AdjustedValue;

	Gamma = NewValue;
	if(!bUseStoredTable)
	{
		for (i = 0;  i < 256; i++)
		{
			AdjustedValue = 255.0 * GetGammaAdjustedValue((double)(i) / 255.0, (double)Gamma / 1000.0);
			GammaTable[i] = (unsigned char)AdjustedValue;
		}
	}
	return FALSE;
}

BOOL UseStoredTable_OnChange(long NewValue)
{
	char szEntry[10];
	int i;
	bUseStoredTable = NewValue;
	if(bUseStoredTable)
	{
		for(i = 0; i < 256; i++)
		{
			wsprintf(szEntry, "%d", i);
			GammaTable[i] = (unsigned char)GetPrivateProfileInt("Gamma", szEntry, i, "Gamma.ini");
		}
		return FALSE;
	}
	else
	{
		return Gamma_OnChange(Gamma);
	}
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING FLT_GammaSettings[FLT_GAMMA_SETTING_LASTONE] =
{
	{
		"Gamma", SLIDER, 0, &Gamma,
		1300, 0, 3000, 10, 1000,
		NULL,
		"GammaFilter", "Gamma", Gamma_OnChange,
	},
	{
		"Use Stored Gamma Table", YESNO, 0, &bUseStoredTable,
		FALSE, 0, 1, 1, 1,
		NULL,
		"GammaFilter", "bUseStoredTable", UseStoredTable_OnChange,
	},
	{
		"Gamma Filter", ONOFF, 0, &(GammaMethod.bActive),
		FALSE, 0, 1, 1, 1,
		NULL,
		"GammaFilter", "UseGammaFilter", NULL,
	},
};

void __cdecl FilterStartGamma(void)
{
	// this will reset the table correctly
	UseStoredTable_OnChange(bUseStoredTable);
}

FILTER_METHOD GammaMethod =
{
	sizeof(FILTER_METHOD),
	FILTER_CURRENT_VERSION,
	"Gamma Filter",
	"&Gamma",
	FALSE,
	TRUE,
	FilterGamma, 
	0,
	TRUE,
	FilterStartGamma,
	NULL,
	NULL,
	FLT_GAMMA_SETTING_LASTONE,
	FLT_GammaSettings,
	WM_FLT_GAMMA_GETVALUE - WM_USER,
};


__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo(long CpuFeatureFlags)
{
	return &GammaMethod;
}

