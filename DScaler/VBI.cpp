/////////////////////////////////////////////////////////////////////////////
// $Id: VBI.cpp,v 1.24 2003-01-10 17:38:40 adcockj Exp $
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
// Revision 1.23  2003/01/07 16:49:09  adcockj
// Changes to allow variable sampling rates for VBI
//
// Revision 1.22  2003/01/05 18:35:45  laurentg
// Init function for VBI added
//
// Revision 1.21  2003/01/05 12:42:52  laurentg
// WSS decoding updated to take into account a VBI frequency of 27.0 MHz
//
// Revision 1.20  2003/01/03 00:54:19  laurentg
// New mode for AR autodetection using only WSS
//
// Revision 1.19  2003/01/01 20:53:11  atnak
// Updates for various VideoText and VPS changes/reorganization
//
// Revision 1.18  2002/10/30 13:37:52  atnak
// Added "Single key teletext toggle" option. (Enables mixed mode menu item)
//
// Revision 1.17  2002/08/15 14:16:18  kooiman
// Cleaner settings per channel implementation
//
// Revision 1.16  2002/08/08 12:57:03  kooiman
// Added VBI settings to settings per channel.
//
// Revision 1.15  2002/08/05 22:33:38  laurentg
// WSS decoding and VBI decoding locked when AR autodetection mode is ON and this mode used is set to use WSS
//
// Revision 1.14  2002/06/20 20:00:37  robmuller
// Implemented videotext search highlighting.
//
// Revision 1.13  2002/05/24 18:04:32  robmuller
// Gray VideoText search items when necessary.
//
// Revision 1.12  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
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
#include "VBI_VPSdecode.h"
#include "Providers.h"
#include "DebugLog.h"
#include "AspectRatio.h"
#include "SettingsPerChannel.h"

BOOL bStopVBI;
HANDLE VBIThread;

BYTE VBI_thresh;
BYTE VBIOffset;

BOOL bCaptureVBI = FALSE;
BOOL DoTeletext = FALSE;
BOOL DoVPS = FALSE;
BOOL DoWSS = FALSE;
BOOL bSearchHighlight = TRUE;

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
	static double VBI_Frequency = -1;
	if (Frequency != VBI_Frequency)
	{
		VBI_Frequency = Frequency;
		VT_Init_Data(VBI_Frequency);
		CC_Init_Data(VBI_Frequency);
		VPS_Init_Data(VBI_Frequency);
		WSS_Init_Data(VBI_Frequency);
	}
}

void VBI_ChannelChange()
{
    VT_ChannelChange();
    VPS_ChannelChange();
}

void VBI_DecodeLine(unsigned char* VBI_Buffer, int line, BOOL IsOdd)
{
    TTVFormat* TVFormat = GetTVFormat(Providers_GetCurrentSource()->GetFormat());

    // set up threshold and offset data
    VBI_AGC(VBI_Buffer, 120, 450, 1);

    // all kinds of data with videotext data format: videotext, intercast, ... 
    if (DoTeletext)
    {
        VBI_DecodeLine_VT(VBI_Buffer);
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
    if (DoVPS && (line == 9))
    {
        VBI_DecodeLine_VPS(VBI_Buffer);
    }

    // WSS information with source aspect ratio. 
    if (DoWSS && !IsOdd && (line == TVFormat->WSS_Line))
    {
//		LOG(1, "WSS VBI_thresh %d VBIOffset %d", VBI_thresh, VBIOffset);
        VBI_DecodeLine_WSS(VBI_Buffer);
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
    {
        "Search Highlight", ONOFF, 0, (long*)&bSearchHighlight,
        TRUE, 0, 1, 1, 1,
        NULL,
        "VBI", "SearchHighLight", VT_HilightSearchOnChange,
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

CSettingsHolderStandAlone VBISettingsHolder;

void VBI_ReadSettingsFromIni()
{
    if(VBISettingsHolder.GetNumSettings() == 0)
    {
        CSettingGroup *pVBIGroup = VBISettingsHolder.GetSettingsGroup("VBI", "VBI", "VBI Decoder");
        CSettingGroup *pCaptureGroup = pVBIGroup->GetGroup("VBICapture","VBI - Capture");
        CSettingGroup *pVBISettingsGroup = pVBIGroup->GetGroup("VBISettings","VBI - Settings");

        VBISettingsHolder.AddSetting(&VBISettings[CAPTURE_VBI], pCaptureGroup);

        VBISettingsHolder.AddSetting(&VBISettings[CLOSEDCAPTIONMODE], pCaptureGroup);
        VBISettingsHolder.AddSetting(&VBISettings[DOTELETEXT], pCaptureGroup);
        VBISettingsHolder.AddSetting(&VBISettings[DOVPS], pCaptureGroup);
        VBISettingsHolder.AddSetting(&VBISettings[DOWSS], pCaptureGroup);
        VBISettingsHolder.AddSetting(&VBISettings[SEARCHHIGHLIGHT], pCaptureGroup);

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
    int i = Setting_GetValue(Aspect_GetSetting(AUTODETECTASPECT));
    BOOL LockWSS = (i == 2) || ( (i == 1) && Setting_GetValue(Aspect_GetSetting(USEWSS)) );

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
        EnableMenuItem(hMenu, IDM_VPS_OUT, MF_GRAYED);
        for(i = CCMODE_OFF; i <= CCMODE_TEXT4; i++)
        {
            EnableMenuItem(hMenu, IDM_CCOFF + i, MF_GRAYED);
        }
    }
}
