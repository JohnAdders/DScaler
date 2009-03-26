/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file VBI.cpp VBI functions
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "VBI.h"
#include "VBI_VideoText.h"
#include "VBI_CCdecode.h"
#include "VBI_WSSdecode.h"
#include "VBI_VPSdecode.h"
#include "Providers.h"
#include "DebugLog.h"
#include "AspectRatio.h"
#include "SettingsPerChannel.h"
#include "SettingsMaster.h"

BOOL bStopVBI;
HANDLE VBIThread;

BYTE VBI_thresh;
BYTE VBIOffset;

BOOL bCaptureVBI = FALSE;
BOOL DoTeletext = FALSE;
BOOL DoVPS = FALSE;
BOOL DoWSS = FALSE;
BOOL bSearchHighlight = TRUE;
BOOL bOldCCMethod = FALSE;

eCCMode CCMode = CCMODE_OFF;

void VBI_SavePerChannelSetup(void *pThis, int Start);

void VBI_Init()
{
    VBI_VT_Init();
    VBI_VPS_Init();
}

void VBI_Exit()
{
    VBI_VPS_Exit();
    VBI_VT_Exit();
}

void VBI_Init_data(double Frequency)
{
    VT_Init_Data(Frequency);
    CC_Init_Data(Frequency);
    VPS_Init_Data(Frequency);
    WSS_Init_Data(Frequency);
}

void VBI_ChannelChange()
{
    VT_ChannelChange();
    VPS_ChannelChange();
}

void VBI_DecodeLine(unsigned char* VBI_Buffer, int line, BOOL IsOdd)
{
    static eCCMode PrevCCMode = CCMODE_OFF;

    TTVFormat* TVFormat = GetTVFormat(Providers_GetCurrentSource()->GetFormat());

    // set up threshold and offset data
    VBI_AGC(VBI_Buffer, 120, 450, 1);

    // all kinds of data with videotext data format: videotext, intercast, ...
    if (DoTeletext)
    {
        VBI_DecodeLine_VT(VBI_Buffer);
    }

    // If the user has just changed CC selection, then we erase the CC display
     if (PrevCCMode != CCMode)
    {
        VBI_DecodeLine_CC(VBI_Buffer, CCMODE_OFF, IsOdd);
    }

   // Closed caption information appears on line 21 (line == 11) for NTSC
    // it also appears on PAL videos at line 22
    // see http://www.wgbh.org/wgbh/pages/captioncenter/cctechfacts4.html
    // for more infomation
    if ((CCMode != CCMODE_OFF) && line == TVFormat->CC_Line)
    {
        VBI_DecodeLine_CC(VBI_Buffer, CCMode, IsOdd);
    }

    // VPS information with channel name, time, VCR programming Info, etc.
    if (DoVPS && !IsOdd && (line == 9))
    {
        VBI_DecodeLine_VPS(VBI_Buffer);
    }

    // WSS information with source aspect ratio.
    if (DoWSS && !IsOdd && (line == TVFormat->WSS_Line))
    {
//        LOG(1, "WSS VBI_thresh %d VBIOffset %d", VBI_thresh, VBIOffset);
        VBI_DecodeLine_WSS(VBI_Buffer);
    }

    PrevCCMode = CCMode;
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
    {
        "Search Highlight", ONOFF, 0, (long*)&bSearchHighlight,
        TRUE, 0, 1, 1, 1,
        NULL,
        "VBI", "SearchHighLight", VT_HilightSearchOnChange,
    },
    {
        "Use Original CC Method", ONOFF, 0, (long*)&bOldCCMethod,
        TRUE, 0, 1, 1, 1,
        NULL,
        "VBI", "UseOriginalCCMethod", NULL,
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

CSettingsHolder VBISettingsHolder;

void VBI_ReadSettingsFromIni()
{
    if(VBISettingsHolder.GetNumSettings() == 0)
    {
        CSettingGroup *pCaptureGroup = SettingsMaster->GetGroup("VBI - Capture", SETTING_BY_CHANNEL | SETTING_BY_FORMAT | SETTING_BY_INPUT, FALSE);

        VBISettingsHolder.AddSettings(VBISettings, VBI_SETTING_LASTONE, pCaptureGroup);

#ifdef _DEBUG
        if (VBI_SETTING_LASTONE != VBISettingsHolder.GetNumSettings())
        {
            LOGD("Number of settings in VBI source is not equal to the number of settings in DS_Control.h");
            LOGD("DS_Control.h or VBI.cpp are probably not in sync with each other.");
        }
#endif

    }

    VBISettingsHolder.DisableOnChange();
    VBISettingsHolder.ReadFromIni();
    VBISettingsHolder.EnableOnChange();

    // don't know why this is needed but it might be
    // \todo remove
    VT_HilightSearchOnChange(bSearchHighlight);
}

void VBI_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    VBISettingsHolder.WriteToIni(bOptimizeFileAccess);
}

void VBI_SetMenu(HMENU hMenu)
{
    int i = Setting_GetValue(WM_ASPECT_GETVALUE, AUTODETECTASPECT);
    BOOL LockWSS = (i == 2) || ( (i == 1) && Setting_GetValue(WM_ASPECT_GETVALUE, USEWSS)) ;

    EnableMenuItem(hMenu, IDM_PDC_OUT, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_VT_OUT, MF_GRAYED);
    EnableMenuItem(hMenu, IDM_VPS_OUT, MF_GRAYED);

    EnableMenuItem(hMenu, IDM_VBI, LockWSS ? MF_GRAYED : MF_ENABLED);
    CheckMenuItemBool(hMenu, IDM_VBI, bCaptureVBI);
    if (bCaptureVBI == TRUE)
    {
        // set vt dialog menu items up
        EnableMenuItem(hMenu, IDM_CALL_VIDEOTEXT, (DoTeletext) ? MF_ENABLED : MF_GRAYED);
        EnableMenuItem(hMenu, IDM_VT_MIXEDMODE, (DoTeletext) ? MF_ENABLED : MF_GRAYED);
        EnableMenuItem(hMenu, IDM_VT_RESET, (DoTeletext) ? MF_ENABLED : MF_GRAYED);
        EnableMenuItem(hMenu, IDM_VT_OUT, (DoTeletext) ? MF_ENABLED : MF_GRAYED);
        EnableMenuItem(hMenu, IDM_VT_SEARCH, (DoTeletext && VT_GetState() != VT_OFF) ? MF_ENABLED : MF_GRAYED);
        EnableMenuItem(hMenu, IDM_VT_SEARCHNEXT, (DoTeletext && VT_GetState() != VT_OFF) ? MF_ENABLED : MF_GRAYED);
        EnableMenuItem(hMenu, IDM_VT_SEARCHHIGHLIGHT, (DoTeletext && VT_GetState() != VT_OFF) ? MF_ENABLED : MF_GRAYED);
        EnableMenuItem(hMenu, IDM_VT_ANTIALIAS, DoTeletext ? MF_ENABLED : MF_GRAYED);
        EnableMenuItem(hMenu, IDM_VT_GOTO, (DoTeletext) ? MF_ENABLED : MF_GRAYED);
        EnableMenuItem(hMenu, IDM_VPS_OUT, (DoVPS) ? MF_ENABLED : MF_GRAYED);
        EnableMenuItem(hMenu, IDM_VBI_VT, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_VBI_VPS, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_VBI_WSS, LockWSS ? MF_GRAYED : MF_ENABLED);
        CheckMenuItemBool(hMenu, IDM_VBI_VT, DoTeletext);
        CheckMenuItemBool(hMenu, IDM_VBI_VPS, DoVPS);
        CheckMenuItemBool(hMenu, IDM_VBI_WSS, DoWSS);
        CheckMenuItemBool(hMenu, IDM_VT_SEARCHHIGHLIGHT, bSearchHighlight);
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
        EnableMenuItem(hMenu, IDM_VT_MIXEDMODE, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_VT_RESET, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_VT_OUT, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_VT_SEARCH, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_VT_SEARCHNEXT, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_VT_SEARCHHIGHLIGHT, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_VT_ANTIALIAS, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_VT_GOTO, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_VPS_OUT, MF_GRAYED);
        for(i = CCMODE_OFF; i <= CCMODE_TEXT4; i++)
        {
            EnableMenuItem(hMenu, IDM_CCOFF + i, MF_GRAYED);
        }
    }
}
