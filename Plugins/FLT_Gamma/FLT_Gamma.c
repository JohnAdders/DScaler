/////////////////////////////////////////////////////////////////////////////
// $Id: FLT_Gamma.c,v 1.20 2008-03-10 17:41:47 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//  This file is subject to the terms of the GNU General Public License as
//  published by the Free Software Foundation.  A copy of this license is
//  included with this software distribution in the file COPYING.  If you
//  do not have a copy, you may obtain a copy by writing to the Free
//  Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.19  2008/02/08 13:43:21  adcockj
// Changes to support cmake compilation
//
// Revision 1.18  2006/10/06 13:35:32  adcockj
// Added projects for .NET 2005 and fixed most of the warnings and errors
//
// Revision 1.17  2004/04/06 12:20:56  adcockj
// Added .NET 2003 project files and some fixes to support this
//
// Revision 1.16  2003/09/13 08:36:16  adcockj
// Crash patch from Steve Gotthardt (Patch-805174)
//
// Revision 1.15  2003/06/26 11:42:54  adcockj
// Reduced teh size of some of the dlls
//
// Revision 1.14  2002/06/18 19:46:08  adcockj
// Changed appliaction Messages to use WM_APP instead of WM_USER
//
// Revision 1.13  2002/06/13 12:10:25  adcockj
// Move to new Setings dialog for filers, video deint and advanced settings
//
// Revision 1.12  2001/11/28 16:04:50  adcockj
// Major reorganization of STill support
//
// Revision 1.11  2001/11/26 15:27:18  adcockj
// Changed filter structure
//
// Revision 1.10  2001/11/21 15:21:41  adcockj
// Renamed DEINTERLACE_INFO to TDeinterlaceInfo in line with standards
// Changed TDeinterlaceInfo structure to have history of pictures.
//
// Revision 1.9  2001/10/17 11:45:17  adcockj
// Corrected gamma settings and made gamma number similar to industry standard
//
// Revision 1.8  2001/07/13 16:13:33  adcockj
// Added CVS tags and removed tabs
//
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DS_Filter.h"
#include "math.h"
#include "..\help\helpids.h"

unsigned char GammaTable[256] = {0,};

FILTER_METHOD GammaMethod; 

long Gamma = 1300;
BOOL bUseStoredTable = FALSE;
long BlackLevel = 0;
long WhiteLevel = 255;

long __cdecl FilterGamma(TDeinterlaceInfo* pInfo)
{
    BYTE* Pixels = NULL;
    short* Table;
    int y;
    int Cycles;

    if (pInfo->PictureHistory[0] == NULL )
    {
        return 1000;
    }

    Pixels = pInfo->PictureHistory[0]->pData;

    // Need to have the current and next-to-previous fields to do the filtering.
    Cycles = pInfo->LineLength / 4;

    Table = (short*)GammaTable;

    for (y = 0; y < pInfo->FieldHeight; y++)
    {
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
        Pixels += pInfo->InputPitch;
    }
    return 1000;
}

double GetGammaAdjustedValue(double Input, double Gamma)
{
    if(Input <= 0.0)
    {
        return 0.0;
    }
    else if(Input >= 1.0)
    {
        return 1.0;
    }
    else
    {
		// make gamma consistent with
		// industry standard
		// above 1 is brighter
		// below 1 is darker
        return pow(Input, 1.0 / Gamma);
    }
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
            AdjustedValue = 255.0 * GetGammaAdjustedValue((double)(i - BlackLevel) / (double)(WhiteLevel - BlackLevel), (double)Gamma / 1000.0);
            GammaTable[i] = (unsigned char)AdjustedValue;
        }
    }
    return FALSE;
}

BOOL BlackLevel_OnChange(long NewValue)
{
    // Don't let the black level get lighter than white
    if (NewValue >= WhiteLevel)
    {
        NewValue = WhiteLevel - 1;
    }
    BlackLevel = NewValue;
    return Gamma_OnChange(Gamma);
}

BOOL WhiteLevel_OnChange(long NewValue)
{
    // Don't let the white level get darker than black
    if (NewValue <= BlackLevel)
    {
        NewValue = BlackLevel + 1;
    }
    WhiteLevel = NewValue;
    return Gamma_OnChange(Gamma);
}


BOOL UseStoredTable_OnChange(long NewValue)
{
    char szEntry[10];
    int i;
    bUseStoredTable = NewValue;
    if(bUseStoredTable)
    {
        char szIniFile[MAX_PATH];
        GetCurrentDirectory(MAX_PATH, szIniFile);
        strcat(szIniFile, "\\gamma.ini");

        for(i = 0; i < 256; i++)
        {
            wsprintf(szEntry, "%d", i);
            GammaTable[i] = (unsigned char)GetPrivateProfileInt("Gamma", szEntry, i, szIniFile);
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
    {
        "Black Level", SLIDER, 0, &BlackLevel,
        0, 0, 255, 5, 1,
        NULL,
        "GammaFilter", "BlackLevel", BlackLevel_OnChange,
    },
    {
        "White Level", SLIDER, 0, &WhiteLevel,
        255, 0, 255, 5, 1,
        NULL,
        "GammaFilter", "WhiteLevel", WhiteLevel_OnChange,
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
    DEINTERLACE_INFO_CURRENT_VERSION,
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
    WM_FLT_GAMMA_GETVALUE - WM_APP,
    TRUE,
    1,
    IDH_GAMMA,
};


__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo(long CpuFeatureFlags)
{
    return &GammaMethod;
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}
