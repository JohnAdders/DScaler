/////////////////////////////////////////////////////////////////////////////
// $Id: VBI.cpp,v 1.12 2001-11-23 10:49:17 adcockj Exp $
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
//
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           Original Release
//                                     Translated most code from German
//                                     Combined Header files
//                                     Cut out all decoding
//                                     Cut out digital hardware stuff
//
// 02 Jan 2001   John Adcock           Started VBI Clean Up
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 11 Mar 2001   Laurent Garnier       Added WSS decoding
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.11  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.10  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.9.2.2  2001/08/21 09:43:01  adcockj
// Brought branch up to date with latest code fixes
//
// Revision 1.9.2.1  2001/08/18 17:09:30  adcockj
// Got to compile, still lots to do...
//
// Revision 1.9  2001/07/16 18:07:50  adcockj
// Added Optimisation parameter to ini file saving
//
// Revision 1.8  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.7  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "VBI.h"
#include "VBI_VideoText.h"
#include "VBI_CCdecode.h"
#include "VBI_WSSdecode.h"
#include "Providers.h"
#include "DebugLog.h"

BOOL bStopVBI;
HANDLE VBIThread;

BYTE VBI_thresh;
BYTE VBIOffset;
int VTStep;
int VPSStep;

BOOL bCaptureVBI = FALSE;
BOOL DoTeletext = FALSE;
BOOL DoVPS = FALSE;
BOOL DoWSS = FALSE;
eCCMode CCMode = CCMODE_OFF;

HWND ShowVPSInfo=NULL;

void VBI_Init()
{
    VBI_VT_Init();
}

void VBI_Exit()
{
    VBI_VT_Exit();
}

void VBI_DecodeLine(unsigned char* VBI_Buffer, int line, BOOL IsOdd)
{
    TTVFormat* TVFormat = GetTVFormat(Providers_GetCurrentSource()->GetFormat());
    VTStep = (int) ((35.468950 / 6.9375) * FPFAC + 0.5);
    VPSStep = 2 * (int) ((35.468950 / 5.0) * FPFAC + 0.5);

    // set up threshold and offset data
    VBI_AGC(VBI_Buffer, 120, 450, 1);

    // all kinds of data with videotext data format: videotext, intercast, ... 
    if (DoTeletext)
    {
        VT_DecodeLine(VBI_Buffer, line, IsOdd);
    }

    // Closed caption information appears on line 21 (line == 11) for NTSC
    // it also appears on PAL videos at line 22
    // see http://www.wgbh.org/wgbh/pages/captioncenter/cctechfacts4.html
    // for more infomation
    if ((CCMode != CCMODE_OFF) && line == TVFormat->CC_Line) 
    {
        CC_DecodeLine(VBI_Buffer, CCMode, IsOdd);
    }

    // VPS information with channel name, time, VCR programming Info, etc. 
    if (DoVPS && (line == 9))
    {
        VTS_DecodeLine(VBI_Buffer);
    }

    // WSS information with source aspect ratio. 
    if (DoWSS && !IsOdd && (line == TVFormat->WSS_Line))
    {
        WSS_DecodeLine(VBI_Buffer);
    }
}

void VBI_AGC(BYTE* Buffer, int start, int stop, int step)
{
    int i, min = 255, max = 0;

    for (i = start; i < stop; i += step)
    {
        if (Buffer[i] < min)
        {
            min = Buffer[i];
        }
        else if (Buffer[i] > max)
        {
            max = Buffer[i];
        }
    }
    VBI_thresh = (max + min) / 2;
    VBIOffset = 128 - VBI_thresh;
}


////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
BOOL Capture_VBI_OnChange(long NewValue)
{
    bCaptureVBI = (BOOL)NewValue;
    WSS_init();
    return FALSE;
}

BOOL DoWSS_OnChange(long NewValue)
{
    DoWSS = (BOOL)NewValue;
    WSS_init();
    return FALSE;
}

SETTING VBISettings[VBI_SETTING_LASTONE] =
{
    {
        "Capture VBI", ONOFF, 0, (long*)&bCaptureVBI,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Show", "CaptureVBI", Capture_VBI_OnChange,
    },
    {
        "CC Mode", SLIDER, 0, (long*)&CCMode,
        CCMODE_OFF, CCMODE_OFF, CCMODE_TEXT4, 1, 1,
        NULL,
        "VBI", "CCMode", NULL,
    },
    {
        "Teletext", ONOFF, 0, (long*)&DoTeletext,
        FALSE, 0, 1, 1, 1,
        NULL,
        "VBI", "DoTeletext", NULL,
    },
    {
        "VPS", ONOFF, 0, (long*)&DoVPS,
        FALSE, 0, 1, 1, 1,
        NULL,
        "VBI", "DoVPS", NULL,
    },
    {
        "WSS", ONOFF, 0, (long*)&DoWSS,
        FALSE, 0, 1, 1, 1,
        NULL,
        "VBI", "DoWSS", DoWSS_OnChange,
    },
};

SETTING* VBI_GetSetting(VBI_SETTING Setting)
{
    if(Setting > -1 && Setting < VBI_SETTING_LASTONE)
    {
        return &(VBISettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void VBI_ReadSettingsFromIni()
{
    int i;
    for(i = 0; i < VBI_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(VBISettings[i]));
    }
}

void VBI_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < VBI_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(VBISettings[i]), bOptimizeFileAccess);
    }
}

void VBI_SetMenu(HMENU hMenu)
{
    int i;
    EnableMenuItem(hMenu, IDM_PDC_OUT, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_VT_OUT, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_VPS_OUT, MF_GRAYED);

    CheckMenuItemBool(hMenu, IDM_VBI, bCaptureVBI);
    if (bCaptureVBI == TRUE)
    {
        // set vt dialog menu items up
        EnableMenuItem(hMenu, IDM_CALL_VIDEOTEXT, (DoTeletext)?MF_ENABLED:MF_GRAYED);
        EnableMenuItem(hMenu, IDM_VT_RESET, (DoTeletext)?MF_ENABLED:MF_GRAYED);
        EnableMenuItem(hMenu, IDM_VT_OUT, (DoTeletext)?MF_ENABLED:MF_GRAYED);
        EnableMenuItem(hMenu, IDM_VPS_OUT, (DoVPS)?MF_ENABLED:MF_GRAYED);
        EnableMenuItem(hMenu, IDM_VBI_VT, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_VBI_VPS, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_VBI_WSS, MF_ENABLED);
        CheckMenuItemBool(hMenu, IDM_VBI_VT, DoTeletext);
        CheckMenuItemBool(hMenu, IDM_VBI_VPS, DoVPS);
        CheckMenuItemBool(hMenu, IDM_VBI_WSS, DoWSS);
        for(i = CCMODE_OFF; i <= CCMODE_TEXT4; i++)
        {
            EnableMenuItem(hMenu, IDM_CCOFF + i, MF_ENABLED);
            CheckMenuItemBool(hMenu, IDM_CCOFF + i, (CCMode == i));
        }
    }
    else
    {
        EnableMenuItem(hMenu, IDM_VBI_VT, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_VBI_VPS, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_VBI_WSS, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_CALL_VIDEOTEXT, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_VT_RESET, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_VT_OUT, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_VPS_OUT, MF_GRAYED);
        for(i = CCMODE_OFF; i <= CCMODE_TEXT4; i++)
        {
            EnableMenuItem(hMenu, IDM_CCOFF + i, MF_GRAYED);
        }
    }
}
