/////////////////////////////////////////////////////////////////////////////
// FLT_LogoKill.c
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
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
// This code is based on the 
// LogoAway VirtualDub filter by Krzysztof Wojdon (c) 2000
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DS_Filter.h"

long Top = 5;
long Left = 5;
long Width = 30;
long Height = 30;

SETTING FLT_LogoKillSettings[FLT_LOGOKILL_SETTING_LASTONE];

BOOL LogoKiller(DEINTERLACE_INFO *info)
{
	return TRUE;
}

BOOL Top_OnChange(long NewValue)
{
    Top = NewValue;
    return TRUE;   
}

BOOL Left_OnChange(long NewValue)
{
    Left = NewValue;
    return TRUE;   
}

BOOL Width_OnChange(long NewValue)
{
    Width = NewValue;
    return TRUE;   
}

BOOL Height_OnChange(long NewValue)
{
    Height = NewValue;
    return TRUE;   
}

void LinearCorrStart(void)
{
    Top_OnChange(Top);
    Left_OnChange(Left);
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING FLT_LogoKillSettings[FLT_LOGOKILL_SETTING_LASTONE] =
{
	{
		"Top", SLIDER, 0, &Top,
		5, 0, 575, 1, 1,
		NULL,
		"LogoKillFilter", "Top", Top_OnChange,
	},
	{
		"Left", SLIDER, 0, &Left,
		5, 0, 755, 1, 1,
		NULL,
		"LogoKillFilter", "Left", Left_OnChange,
	},
	{
		"Width", SLIDER, 0, &Width,
		30, 0, 750, 1, 1,
		NULL,
		"LogoKillFilter", "Width", Width_OnChange,
	},
	{
		"Height", SLIDER, 0, &Height,
		30, 0, 545, 1, 1,
		NULL,
		"LogoKillFilter", "Height", Height_OnChange,
	},
};

FILTER_METHOD LogoKillMethod =
{
	sizeof(FILTER_METHOD),
	FILTER_CURRENT_VERSION,
	"Logo Killer Filter",
	"&Logo Killer (experimental)",
	FALSE,
	FALSE,
	LogoKiller, 
	0,
	TRUE,
	NULL,
	NULL,
	NULL,
	FLT_LOGOKILL_SETTING_LASTONE,
	FLT_LogoKillSettings,
	WM_FLT_LOGOKILL_GETVALUE - WM_USER,
};


__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo(long CpuFeatureFlags)
{
	return &LogoKillMethod;
}

