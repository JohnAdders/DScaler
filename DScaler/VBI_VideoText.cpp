/////////////////////////////////////////////////////////////////////////////
// $Id: VBI_VideoText.cpp,v 1.60 2003-01-05 18:35:45 laurentg Exp $
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
// Color buttons code was taken from GnomeTV. Those portions are
// Copyright (C) 1999-2000 Zolt�n Sinkovics and Szabolcs Sel�f
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
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 02 Jan 2003   Atsushi Nakagawa      Redid VBI_VideoText.cpp to link in
//                                     with the new and updated CVTDecoder and
//                                     CVTDrawer classes.  CVS Log entries
//                                     prior to and including rev 1.52 may
//                                     no longer have direct validity but
//                                     their cumulative changes still remain
//                                     valid.
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.59  2003/01/05 16:09:44  atnak
// Updated TopText for new teletext
//
// Revision 1.58  2003/01/03 14:44:19  robmuller
// Removed wPageSubCode from VT_PerformFlofKey() as suggested by Atsushi.
//
// Revision 1.57  2003/01/02 21:26:33  atnak
// Only redraw the clock if it changed
//
// Revision 1.56  2003/01/02 14:48:50  atnak
// Added Teletext Settings page
//
// Revision 1.55  2003/01/02 11:05:24  atnak
// Added missing InitialTextPage implementation
//
// Revision 1.54  2003/01/01 21:34:11  atnak
// Added missing subtitles force double height filter
//
// Revision 1.53  2003/01/01 20:49:03  atnak
// Updated VBI_VideoText.* files for new videotext structure
//
// Revision 1.52  2002/10/30 13:37:52  atnak
// Added "Single key teletext toggle" option. (Enables mixed mode menu item)
//
// Revision 1.51  2002/10/23 16:57:13  atnak
// Added TOP-Text support
//
// Revision 1.50  2002/10/15 11:53:38  atnak
// Added UI feedback for some videotext stuff
//
// Revision 1.49  2002/10/15 03:36:29  atnak
// removed rounding from last commit - it wasn't necessary
//
// Revision 1.48  2002/10/15 02:02:58  atnak
// Added rounding in VT decoding to improve accuracy
//
// Revision 1.47  2002/10/13 08:36:14  atnak
// fix to portions of page not updated after subtitle/newsflash change
//
// Revision 1.46  2002/10/12 04:33:53  atnak
// flashing conceal teletext fix, header clear on channel change
//
// Revision 1.45  2002/09/07 20:59:45  kooiman
// Small fixes.
//
// Revision 1.44  2002/06/20 20:00:35  robmuller
// Implemented videotext search highlighting.
//
// Revision 1.43  2002/05/30 19:09:06  robmuller
// Clear VisiblePage on channel change.
//
// Revision 1.42  2002/05/30 10:13:43  robmuller
// Don't search the header line.
//
// Revision 1.41  2002/05/29 18:44:52  robmuller
// Added option to disable font anti-aliasing in Teletext.
//
// Revision 1.40  2002/05/28 20:04:26  robmuller
// Addition to patch #561180 by PietOO:
// In auto code page mode the code page options are grayed and the current code page is checked.
//
// Revision 1.39  2002/05/27 20:17:05  robmuller
// Patch #561180  by PietOO:
// Autodetection of teletext code page.
//
// Revision 1.38  2002/05/24 16:49:00  robmuller
// VideoText searching improved.
//
// Revision 1.37  2002/05/23 18:45:03  robmuller
// Patch #559554 by PietOO.
// Teletext: + text search ctrl-F & next F3
//
// Revision 1.36  2002/03/12 23:29:44  robmuller
// Implemented functions VT_GetNextPage() and VT_GetPreviousPage().
//
// Revision 1.35  2002/02/24 08:18:03  temperton
// TIMER_VTFLASHER set only when displayed page contains flashed elements and only in teletext modes.
//
// Revision 1.34  2002/02/07 13:04:54  temperton
// Added Spanish and Polish teletext code pages. Thanks to Jazz (stawiarz).
//
// Revision 1.33  2002/01/19 17:50:41  robmuller
// Clear more data on channel change.
//
// Revision 1.32  2002/01/19 12:53:00  temperton
// Teletext pages updates at correct time.
// Teletext can use variable-width font.
//
// Revision 1.31  2002/01/15 20:25:45  adcockj
// removed old bitmap code
//
// Revision 1.30  2002/01/15 11:16:03  temperton
// New teletext drawing code.
//
// Revision 1.29  2002/01/12 16:56:21  adcockj
// Series of fixes to bring 4.0.0 into line with 3.1.1
//
// Revision 1.28  2001/11/26 12:48:01  temperton
// Teletext corrections
//
// Revision 1.27  2001/11/23 10:45:32  adcockj
// Added Hebrew and Hungarian codepages
//
// Revision 1.26  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.25  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.24  2001/10/22 17:28:40  temperton
// Removed tabs to comply with coding standards
//
// Revision 1.23  2001/10/22 05:55:07  temperton
// Teletext improvements
//
// Revision 1.22  2001/10/06 17:04:26  adcockj
// Fixed teletext crashing problems
//
// Revision 1.21  2001/09/22 11:09:43  adcockj
// Fixed crashing problems with new code with noisy input
//
// Revision 1.20  2001/09/21 16:43:54  adcockj
// Teletext improvements by Mike Temperton
//
// Revision 1.19  2001/09/21 15:39:02  adcockj
// Added Russian and German code pages
// Corrected UK code page
//
// Revision 1.18  2001/09/05 16:22:34  adcockj
// Fix for new teletext painting overwritting other apps
//
// Revision 1.17  2001/09/05 15:08:43  adcockj
// Updated Loging
//
// Revision 1.16  2001/09/05 06:59:13  adcockj
// Teletext fixes
//
// Revision 1.15  2001/09/02 14:17:51  adcockj
// Improved teletext code
//
// Revision 1.14  2001/08/21 09:39:46  adcockj
// Added Greek teletext Codepage
//
// Revision 1.13.2.2  2001/08/23 16:04:57  adcockj
// Improvements to dynamic menus to remove requirement that they are not empty
//
// Revision 1.13.2.1  2001/08/21 09:43:01  adcockj
// Brought branch up to date with latest code fixes
//
// Revision 1.13  2001/08/13 18:07:24  adcockj
// Added Czech code page for teletext
//
// Revision 1.12  2001/08/02 16:43:05  adcockj
// Added Debug level to LOG function
//
// Revision 1.11  2001/07/16 18:07:50  adcockj
// Added Optimisation parameter to ini file saving
//
// Revision 1.10  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.9  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VTCommon.h"
#include "VTDecoder.h"
#include "VTDrawer.h"
#include "VBI_VideoText.h"
#include "VBI.h"
#include "DScaler.h"

#define VT_MAXPAGEHISTORY   64


BOOL VT_SetCodepage(HDC hDC, LPRECT lpRect, eVTCodepage Codepage);

void VT_HistoryReset();
void VT_HistoryPushPage(WORD wPageHex);
WORD VT_HistoryPopLastPage(WORD wCurrentPageHex);

void VT_DecoderEventProc(BYTE uMsg, DWORD dwParam);
void VT_PerformUpscrollDuplicationFilter();
void VT_PerformDoubleHeightSubtitlesFilter();
void VT_DeleteHilightList(TVTLeftRight** pHilightList);
void VT_UpdateHilightList(BOOL bUpdatedOnly = FALSE);
BYTE VT_UpdateHilightListProc(TVTPage*, WORD, LPWORD, WORD, BYTE, BYTE, LPVOID);

BOOL VT_RegionOnChange(long NewValue);
BOOL VT_ShowSubcodeInOSDOnChange(long NewValue);
BOOL VT_CachingControlOnChange(long NewValue);
BOOL VT_HighGranularityCachingOnChange(long NewValue);
BOOL VT_SubstituteErrorsWithSpacesOnChange(long NewValue);


CVTDecoder          VTDecoder(VT_DecoderEventProc);
CVTDrawer           VTDrawer;

eVTCodepage         VTCodepage = VTCODEPAGE_ENGLISH;
eVTCodepage         VTUserCodepage = VTCODEPAGE_ENGLISH;
eVTRegion           VTRegion = VTREGION_DEFAULT;
BOOL                VTAutoCodepage = TRUE;

eVTState            VTState = VT_OFF;
TVTPage             VTVisiblePage;

WORD                VTSavePageHex = 0x100;
WORD                VTSavePageSubCode = 0xFFFF;

WORD                VTPageHex;
WORD                VTPageSubCode;
DWORD               VTLoadedPageCode = 0UL;

BOOL                VTShowHidden = FALSE;
BOOL                VTShowFlash = FALSE;
BOOL                VTAntialias = TRUE;

BOOL                VTShowSubcodeInOSD = FALSE;

BYTE                VTCachingControl = 0;
BOOL                VTHighGranularityCaching = FALSE;
BOOL                VTSubstituteErrorsWithSpaces = FALSE;

BOOL                VTUpscrollDuplicationFilter = FALSE;
BOOL                VTDoubleHeightSubtitlesFilter = FALSE;

char                VTSearchString[41] = "";
BOOL                VTHilightSearch = TRUE;

UINT                VTFlashTimer = 0;

BOOL                VTShowOSD = FALSE;
char                VTOSDBuffer[4] = "";
char*               VTPageOSD = NULL;
char                VTPageInput[4] = "";

WORD                VTPageHistoryHead;
WORD                VTPageHistory[VT_MAXPAGEHISTORY];

TVTLeftRight**      VTHilightListPtr = NULL;
TVTLeftRight*       VTHilightListArray[25];

BYTE                VTDoubleProfile[25];

WORD                VTCursorRowCol = 0xFFFF;
WORD                VTCursorPageHex = 0;

CRITICAL_SECTION    VTPageChangeMutex;


void VBI_VT_Init()
{
    InitializeCriticalSection(&VTPageChangeMutex);

    VTDrawer.SetAntialias(VTAntialias);

    VTSavePageHex = 0x100;
    VTSavePageSubCode = 0xFFFF;

    VT_HistoryReset();

    for (int i = 0; i < 25; i++)
    {
        VTHilightListArray[i] = NULL;
    }
}


void VBI_VT_Exit()
{
    for (int i = 0; i < 25; i++)
    {
        VT_DeleteHilightList(&VTHilightListArray[i]);
    }

    DeleteCriticalSection(&VTPageChangeMutex);
}


void VT_Init_Data(double VBI_Frequency)
{
}


void VT_SetState(HDC hDC, LPRECT lpRect, eVTState State)
{
    if (State == VTState)
    {
        return;
    }

    VT_ClearInput();

    if (State != VT_OFF && VTState != VT_OFF)
    {
        VTState = State;
        VT_SetPageOSD(NULL, TRUE);
        VT_Redraw(hDC, lpRect);
        SetTimer(::hWnd, TIMER_VTINPUT, 1000, NULL);
        return;
    }

    VTState = State;

    if (State == VT_OFF)
    {
        if ((VTPageHex & 0xF00) == 0x000 ||
            (VTPageHex & 0xF00) >= 0x900)
        {
            VTSavePageHex = 0x100;
            VTSavePageSubCode = 0xFFFF;
        }
        else
        {
            VTSavePageHex = VTPageHex;
            VTSavePageSubCode = VTPageSubCode;
        }

        VTPageHex = 0xFFFF;
        VTPageSubCode = 0xFFFF;

        KillTimer(::hWnd, VTFlashTimer);
        VTFlashTimer = 0;

        KillTimer(::hWnd, TIMER_VTINPUT);
        VTShowOSD = FALSE;

        VTHilightListPtr = NULL;
        VTCursorRowCol = 0xFFFF;
    }
    else
    {
        if (VT_GetAutoCodepage())
        {
            VT_SetCodepage(NULL, NULL, VTCODEPAGE_NONE);
        }
        VT_SetPage(hDC, lpRect, VTSavePageHex, VTSavePageSubCode);
    }
}


eVTState VT_GetState()
{
    return VTState;
}


void VT_ChannelChange()
{
    VTDecoder.ResetDecoder();

    VT_HistoryReset();

    VTHilightListPtr = NULL;
    *VTSearchString = '\0';

    if (VTState != VT_OFF)
    {
        VT_SetPage(NULL, NULL, 0x100);
    }
    else
    {
        VTSavePageHex = 0x100;
        VTSavePageSubCode = 0xFFFF;
    }
}


BOOL VT_SetAutoCodepage(HDC hDC, LPRECT lpRect, BOOL bAuto)
{
    if (bAuto != VTAutoCodepage)
    {
        VTAutoCodepage = bAuto;

        if (bAuto)
        {
            return VT_SetCodepage(hDC, lpRect, VTCODEPAGE_NONE);
        }
        else
        {
            return VT_SetCodepage(hDC, lpRect, VTUserCodepage);
        }
    }
    return FALSE;
}


BOOL VT_GetAutoCodepage()
{
    return VTAutoCodepage;
}


BOOL VT_SetCodepage(HDC hDC, LPRECT lpRect, eVTCodepage Codepage)
{
    if (Codepage == VTCODEPAGE_NONE)
    {
        BYTE SubsetCode = VTDecoder.GetCharacterSubsetCode();
        Codepage = CVTCommon::GetRegionCodepage(VTRegion, SubsetCode, TRUE);
    }
    else
    {
        VTUserCodepage = Codepage;
        VTAutoCodepage = FALSE;
    }

    if (Codepage != VTCodepage)
    {
        VTCodepage = Codepage;
        VT_Redraw(hDC, lpRect);
        return TRUE;
    }

    return FALSE;
}


void VT_SetAntialias(HDC hDC, LPRECT lpRect, BOOL bAntialias)
{
    if (bAntialias != VTAntialias)
    {
        VTAntialias = bAntialias;
        VTDrawer.SetAntialias(bAntialias);
        VT_Redraw(hDC, lpRect);
    }
}


BOOL VT_GetAntialias()
{
    return VTAntialias;
}


void VT_SetShowHidden(HDC hDC, LPRECT lpRect, BOOL bShowHidden)
{
    if (bShowHidden != VTShowHidden)
    {
        VTShowHidden = bShowHidden;
        VT_Redraw(hDC, lpRect, VTDF_HIDDENONLY);
    }
}


BOOL VT_GetShowHidden()
{
    return VTShowHidden;
}


void VT_SetOverlayColour(COLORREF ColorRef)
{
    VTDrawer.SetTransparencyColour(ColorRef);
}


BOOL VT_SetPage(HDC hDC, LPRECT lpRect, WORD wPageHex, WORD wPageSubCode)
{
    if (wPageHex < 0x100 ||
        wPageHex > 0x899 ||
        CVTCommon::IsNonVisiblePage(wPageHex))
    {
        return FALSE;
    }

    EnterCriticalSection(&VTPageChangeMutex);

    VTPageHex = wPageHex;
    VTPageSubCode = wPageSubCode;

    DWORD dwPageCode = MAKELONG(VTPageHex, VTPageSubCode);

    VTLoadedPageCode = VTDecoder.GetDisplayPage(dwPageCode, &VTVisiblePage);

    LeaveCriticalSection(&VTPageChangeMutex);

    VTShowHidden = FALSE;
    VTCursorRowCol = 0xFFFF;

    if (VTLoadedPageCode != 0)
    {
        VTDecoder.GetDisplayHeader(&VTVisiblePage, TRUE);
        VT_UpdateHilightList();
        VT_UpdateFlashTimer();
        VT_SetPageOSD(NULL, TRUE);
        VT_Redraw(hDC, lpRect);
        SetTimer(::hWnd, TIMER_VTINPUT, 1000, NULL);
        VT_HistoryPushPage(VTPageHex);
    }
    else
    {
        VTHilightListPtr = NULL;
        KillTimer(::hWnd, VTFlashTimer);
        VTFlashTimer = 0;
        VTDecoder.GetDisplayHeader(&VTVisiblePage, FALSE);
        VTVisiblePage.wControlBits = VTCONTROL_INHIBITDISP;
        VTVisiblePage.bShowRow24 = FALSE;

        // Get the loading status message
        VTDecoder.GetDisplayComment(dwPageCode, &VTVisiblePage);
        
        VT_SetPageOSD(NULL, FALSE);
        VT_Redraw(hDC, lpRect);
    }

    return TRUE;
}


BOOL VT_PageScroll(HDC hDC, LPRECT lpRect, BOOL bForwards)
{
    DWORD dwPageCode = MAKELONG(VTPageHex, 0xFFFF);

    EnterCriticalSection(&VTPageChangeMutex);

    dwPageCode = VTDecoder.GetNextDisplayPage(dwPageCode, &VTVisiblePage, !bForwards);

    VTPageSubCode = 0xFFFF;

    if (dwPageCode != 0)
    {
        VTPageHex = LOWORD(dwPageCode);
        VTLoadedPageCode = dwPageCode;
    }

    LeaveCriticalSection(&VTPageChangeMutex);

    if (dwPageCode != 0)
    {
        VTShowHidden = FALSE;
        VTCursorRowCol = 0xFFFF;
        VTDecoder.GetDisplayHeader(&VTVisiblePage, TRUE);
        VT_UpdateHilightList();
        VT_UpdateFlashTimer();
        VT_SetPageOSD(NULL, TRUE);
        VT_Redraw(hDC, lpRect);
        SetTimer(::hWnd, TIMER_VTINPUT, 1000, NULL);
        VT_HistoryPushPage(VTPageHex);

        return TRUE;
    }
    else
    {
        VT_ShowHeader(hDC, lpRect);
    }

    return FALSE;
}


BOOL VT_SubPageScroll(HDC hDC, LPRECT lpRect, BOOL bForwards)
{
    if (LOWORD(VTLoadedPageCode) != VTPageHex)
    {
        return FALSE;
    }

    DWORD dwPageCode = VTLoadedPageCode;

    EnterCriticalSection(&VTPageChangeMutex);

    dwPageCode = VTDecoder.GetNextDisplaySubPage(dwPageCode, &VTVisiblePage, !bForwards);

    if (dwPageCode != 0)
    {
        VTPageHex = LOWORD(dwPageCode);
        VTPageSubCode = HIWORD(dwPageCode);
        VTLoadedPageCode = dwPageCode;
    }
    else
    {
        VTPageSubCode = 0xFFFF;
    }

    LeaveCriticalSection(&VTPageChangeMutex);

    if (dwPageCode != 0)
    {
        VTShowHidden = FALSE;
        VTCursorRowCol = 0xFFFF;
        VTDecoder.GetDisplayHeader(&VTVisiblePage, TRUE);
        VT_UpdateHilightList();
        VT_UpdateFlashTimer();
        VT_SetPageOSD(NULL, TRUE);
        VT_Redraw(hDC, lpRect);
        SetTimer(::hWnd, TIMER_VTINPUT, 1000, NULL);

        return TRUE;
    }
    else
    {
        VT_ShowHeader(hDC, lpRect);
    }

    return FALSE;
}


BOOL VT_IsSearchStringValid(BOOL bBeepOnCorrection)
{
    if (*VTSearchString == '\0')
    {
        return FALSE;
    }

    for (int i = 0; VTSearchString[i] != '\0'; i++)
    {
        if (VTSearchString[i] != ' ')
        {
            return TRUE;
        }
    }

    *VTSearchString = '\0';

    if (bBeepOnCorrection)
    {
        MessageBeep(MB_OK);
    }
    return FALSE;
}


BOOL VT_PerformSearch(HDC hDC, LPRECT lpRect, BOOL bInclusive, BOOL bReverse)
{
    if (!VT_IsSearchStringValid(TRUE))
    {
        if (VTHilightListPtr != NULL)
        {
            // Get rid of the existing hilighting
            VT_UpdateHilightList();
            VT_Redraw(hDC, lpRect);
        }
        return FALSE;
    }

    DWORD dwPageCode;

    if (!bInclusive && LOWORD(VTLoadedPageCode) == VTPageHex)
    {
        dwPageCode = VTLoadedPageCode;
    }
    else
    {
        dwPageCode = MAKELONG(VTPageHex, bReverse ? 0xFFFF : 0);
    }

    EnterCriticalSection(&VTPageChangeMutex);

    dwPageCode = VTDecoder.FindInDisplayPage(dwPageCode, bInclusive,
        VTSearchString, &VTVisiblePage, bReverse);

    if (dwPageCode != 0)
    {
        VTPageHex = LOWORD(dwPageCode);
        VTPageSubCode = HIWORD(dwPageCode);
        VTLoadedPageCode = dwPageCode;
    }

    LeaveCriticalSection(&VTPageChangeMutex);

    if (dwPageCode != 0)
    {
        VTShowHidden = FALSE;
        VTCursorRowCol = 0xFFFF;
        VTDecoder.GetDisplayHeader(&VTVisiblePage, TRUE);
        VT_UpdateHilightList();
        VT_UpdateFlashTimer();
        VT_SetPageOSD(NULL, TRUE);
        VT_Redraw(hDC, lpRect);
        SetTimer(::hWnd, TIMER_VTINPUT, 1000, NULL);
        VT_HistoryPushPage(VTPageHex);

        return TRUE;
    }
    else
    {
        // No pages found
        MessageBeep(MB_OK);
    }

    return FALSE;
}


BOOL VT_ShowTestPage(HDC hDC, LPRECT lpRect)
{
    EnterCriticalSection(&VTPageChangeMutex);

    VTPageHex = 0x900;
    VTPageSubCode = 0x0000;
    VTLoadedPageCode = MAKELONG(VTPageHex, VTPageSubCode);

    LeaveCriticalSection(&VTPageChangeMutex);

    VTDecoder.CreateTestDisplayPage(&VTVisiblePage);

    VTShowHidden = FALSE;
    VTCursorRowCol = 0xFFFF;
    VTHilightListPtr = NULL;
    VTDecoder.GetDisplayHeader(&VTVisiblePage, TRUE);
    VT_UpdateFlashTimer();
    VT_SetPageOSD(NULL, TRUE);
    VT_Redraw(hDC, lpRect);
    SetTimer(::hWnd, TIMER_VTINPUT, 1000, NULL);

    return TRUE;
}


BOOL VT_PerformFlofKey(HDC hDC, LPRECT lpRect, BYTE nFlofKey)
{
    DWORD dwPageCode = 0UL;

    if (!VTVisiblePage.bShowRow24)
    {
        MessageBeep(MB_OK);
        return FALSE;
    }

    dwPageCode = VTVisiblePage.EditorialLink[nFlofKey];

    WORD wPageHex = LOWORD(dwPageCode);

    if (wPageHex == VTPAGE_PREVIOUS)
    {
        wPageHex = VT_HistoryPopLastPage(VTPageHex);
    }

    if (wPageHex == 0 || (wPageHex & 0xFF) == 0xFF)
    {
        MessageBeep(MB_OK);
        return FALSE;
    }

    return VT_SetPage(hDC, lpRect, wPageHex);
}


void VT_SetPageOSD(char OSD[3])
{
    char szOSD[9];

    if (OSD == NULL)
    {
        VTPageOSD = NULL;

        if (VTShowSubcodeInOSD != FALSE &&
            LOWORD(VTLoadedPageCode) == VTPageHex)
        {
            sprintf(szOSD, "%03X/%-4X", VTPageHex, HIWORD(VTLoadedPageCode));
        }
        else
        {
            sprintf(szOSD, "  P%03X \x07", VTPageHex);
        }
    }
    else
    {
        BOOL bSpace = FALSE;

        CopyMemory(VTOSDBuffer, OSD, 3);

        for (BYTE i = 0; i < 3; i++)
        {
            if (bSpace || VTOSDBuffer[i] == '\0')
            {
                VTOSDBuffer[i] = ' ';
                bSpace = TRUE;
            }
        }

        VTPageOSD = VTOSDBuffer;

        sprintf(szOSD, "  P%c%c%c \x07", VTPageOSD[0], VTPageOSD[1], VTPageOSD[2]);
    }

    CopyMemory(VTVisiblePage.Frame[0], szOSD, 8);
    VTVisiblePage.LineState[0] |= CACHESTATE_HASDATA;
}


void VT_SetPageOSD(char OSD[3], BOOL bOSDModeOn)
{
    VT_SetPageOSD(OSD);
    VTShowOSD = bOSDModeOn;
}


void VT_ShowHeader(HDC hDC, LPRECT lpRect, char OSD[3])
{
    VT_SetPageOSD(OSD, TRUE);
    VT_Redraw(hDC, lpRect, VTDF_HEADERONLY);
    SetTimer(::hWnd, TIMER_VTINPUT, 1200, NULL);
}


BOOL VT_IsTransparencyInPage()
{
    if (LOWORD(VTLoadedPageCode) != VTPageHex)
    {
        return FALSE;
    }

    if (VTState == VT_MIXED ||
        (VTVisiblePage.wControlBits & VTCONTROL_NEWSFLASH) ||
        (VTVisiblePage.wControlBits & VTCONTROL_SUBTITLE))
    {
        return TRUE;
    }

    return FALSE;
}


void VT_HistoryReset()
{
    VTPageHistoryHead = 0;
    VTPageHistory[0] = 0;
}


void VT_HistoryPushPage(WORD wPageHex)
{
    if (VTPageHistory[VTPageHistoryHead] != wPageHex)
    {
        VTPageHistoryHead = (VTPageHistoryHead + 1) % VT_MAXPAGEHISTORY;
        VTPageHistory[VTPageHistoryHead] = wPageHex;
    }
}


WORD VT_HistoryPopLastPage(WORD wCurrentPageHex)
{
    WORD wPageHex;

    if (VTPageHistory[VTPageHistoryHead] == wCurrentPageHex)
    {
        WORD wLastHistoryHead = (VT_MAXPAGEHISTORY +
            VTPageHistoryHead - 1) % VT_MAXPAGEHISTORY;

        if (VTPageHistory[wLastHistoryHead] == 0)
        {
            return 0;
        }

        VTPageHistory[VTPageHistoryHead] = 0;
        VTPageHistoryHead = wLastHistoryHead;
    }

    wPageHex = VTPageHistory[VTPageHistoryHead];

    if (VTPageHistory[VTPageHistoryHead] != 0)
    {
        VTPageHistory[VTPageHistoryHead] = 0;
        VTPageHistoryHead = (VT_MAXPAGEHISTORY +
            VTPageHistoryHead - 1) % VT_MAXPAGEHISTORY;
    }

    return wPageHex;
}


BOOL VT_OnInput(HDC hDC, LPRECT lpRect, char cInput)
{
    BYTE nLength = strlen(VTPageInput);

    if (nLength == 3)
    {
        VTPageInput[0] = VTPageInput[1];
        VTPageInput[1] = VTPageInput[2];
        VTPageInput[2] = cInput;
    }
    else
    {
        VTPageInput[nLength] = cInput;
        VTPageInput[++nLength] = '\0';
    }

    if (nLength == 3)
    {
        WORD i = atoi(VTPageInput);

        if (i >= 100 && i < 900)
        {
            WORD wPageHex = 0;

            wPageHex |= i / 100 * 0x100;
            wPageHex |= i % 100 / 10 * 0x10;
            wPageHex |= i % 10;

            return VT_SetPage(hDC, lpRect, wPageHex);
        }
        else
        {
            VT_ShowHeader(hDC, lpRect, VTPageInput);
        }
    }
    else
    {
        VT_ShowHeader(hDC, lpRect, VTPageInput);
    }

    return FALSE;
}


void VT_ClearInput()
{
    *VTPageInput = '\0';
}


void VT_OnInputTimer(HDC hDC, LPRECT lpRect)
{
    KillTimer(::hWnd, TIMER_VTINPUT);
    VT_ClearInput();

    if (VTState == VT_OFF)
    {
        return;
    }

    VT_SetPageOSD(NULL, FALSE);
    VT_Redraw(hDC, lpRect, VTDF_HEADERONLY);
}


void VT_RedrawFlash(HDC hDC, LPRECT lpRect)
{
    VTShowFlash = !VTShowFlash;
    VT_Redraw(hDC, lpRect, VTDF_FLASHONLY);
}


void VT_Redraw(HDC hDC, LPRECT lpRect, WORD uVTDFFlags)
{
    if (VTState == VT_OFF || hDC == NULL || lpRect == NULL)
    {
        return;
    }

    WORD uFlags = uVTDFFlags;

    if (VTShowHidden)
    {
        uFlags |= VTDF_HIDDEN;
    }
    if (VTShowFlash)
    {
        uFlags |= VTDF_FLASH;
    }
    if (VTState == VT_MIXED)
    {
        uFlags |= VTDF_MIXEDMODE;
    }
    if (VTShowOSD)
    {
        uFlags |= VTDF_FORCEHEADER;
    }

    VTDrawer.SetCodepage(VTCodepage);
    VTDrawer.DrawPage(hDC, lpRect, &VTVisiblePage,
        uFlags, VTHilightListPtr, VTDoubleProfile);
}


void VT_DecoderEventProc(BYTE uMsg, DWORD dwParam)
{
    switch (uMsg)
    {
    case DECODEREVENT_HEADERUPDATE:
        PostMessage(hWnd, WM_VIDEOTEXT, VTM_VTHEADERUPDATE, NULL);
        break;

    case DECODEREVENT_COMMENTUPDATE:
        PostMessage(hWnd, WM_VIDEOTEXT, VTM_VTCOMMENTUPDATE, dwParam);
        break;

    case DECODEREVENT_PAGEUPDATE:
        /* // TryEnterCriticalSection is not defined in my API
        if (TryEnterCriticalSection(&VTPageChangeMutex))
        {
            if (LOWORD(dwParam) == VTPageHex)
            {
                if (VTPageSubCode == 0xFFFF || HIWORD(dwParam) == VTPageSubCode)
                {
                    PostMessage(hWnd, WM_VIDEOTEXT, VTM_VTPAGEUPDATE, dwParam);
                }
            }
            LeaveCriticalSection(&VTPageChangeMutex);
        }
        else*/
        {
            PostMessage(hWnd, WM_VIDEOTEXT, VTM_VTPAGEUPDATE, dwParam);
        }
        break;

    case DECODEREVENT_PAGEREFRESH:
        /* // TryEnterCriticalSection is not defined in my API
        if (TryEnterCriticalSection(&VTPageChangeMutex))
        {
            if (LOWORD(dwParam) == VTPageHex)
            {
                if (VTPageSubCode == 0xFFFF)
                {
                    PostMessage(hWnd, WM_VIDEOTEXT, VTM_VTPAGEREFRESH, dwParam);
                }
            }
            LeaveCriticalSection(&VTPageChangeMutex);
        }
        else*/
        {
            PostMessage(hWnd, WM_VIDEOTEXT, VTM_VTPAGEREFRESH, dwParam);
        }
        break;
    }
}


void VT_ProcessHeaderUpdate(HDC hDC, LPRECT lpRect)
{
    if (VTState == VT_OFF)
    {
        return;
    }

    if (VT_GetAutoCodepage())
    {
        if (VT_SetCodepage(hDC, lpRect, VTCODEPAGE_NONE))
        {
            return;
        }
    }

    if (LOWORD(VTLoadedPageCode) != VTPageHex)
    {
        VTDecoder.GetDisplayHeader(&VTVisiblePage);
        VT_Redraw(hDC, lpRect, VTDF_HEADERONLY);
    }
    else
    {
        char szOldClock[8];

        CopyMemory(szOldClock, &VTVisiblePage.Frame[0][32], 8);
        VTDecoder.GetDisplayHeader(&VTVisiblePage, TRUE);

        // Only redraw the clock if it changed
        if (memcmp(&VTVisiblePage.Frame[0][32], szOldClock, 8) != 0)
        {
            VT_Redraw(hDC, lpRect, VTDF_CLOCKONLY);
        }
    }
}


BOOL VT_ProcessCommentUpdate(HDC hDC, LPRECT lpRect, DWORD dwPageCode)
{
    if (VTState == VT_OFF)
    {
        return FALSE;
    }

    if (LOWORD(dwPageCode) != VTPageHex ||
        HIWORD(dwPageCode) != VTPageSubCode)
    {
        return FALSE;
    }

    if (LOWORD(VTLoadedPageCode) != VTPageHex)
    {    
        if (VTDecoder.GetDisplayComment(dwPageCode, &VTVisiblePage))
        {
            if (VTVisiblePage.LineState[24] & CACHESTATE_UPDATED)
            {
                if (LOBYTE(VTCursorRowCol) == 24)
                {
                    VTCursorRowCol = 0xFFFF;
                }

                VT_UpdateFlashTimer();
                VT_Redraw(hDC, lpRect, VTDF_ROW24ONLY);
                return TRUE;
            }
        }
    }
    return FALSE;
}


BOOL VT_ProcessPageUpdate(HDC hDC, LPRECT lpRect, DWORD dwPageCode)
{
    if (VTState == VT_OFF)
    {
        return FALSE;
    }

    if (LOWORD(dwPageCode) == VTPageHex)
    {
        if (VTPageSubCode == 0xFFFF || HIWORD(dwPageCode) == VTPageSubCode)
        {
            dwPageCode = VTDecoder.GetDisplayPage(dwPageCode, &VTVisiblePage);
            VTDecoder.GetDisplayHeader(&VTVisiblePage, TRUE);

            if (VTUpscrollDuplicationFilter != FALSE)
            {
                VT_PerformUpscrollDuplicationFilter();
            }

            if (VTDoubleHeightSubtitlesFilter != FALSE)
            {
                VT_PerformDoubleHeightSubtitlesFilter();
            }

            VTCursorRowCol = 0xFFFF;

            if (dwPageCode != VTLoadedPageCode)
            {
                VTLoadedPageCode = dwPageCode;
                // Reburn the OSD into the reloaded page
                VT_SetPageOSD(VTPageOSD);
                VT_UpdateHilightList();
                VT_UpdateFlashTimer();
                VT_Redraw(hDC, lpRect);
                VT_HistoryPushPage(VTPageHex);
            }
            else
            {
                for (int i = 0; i < 23; i++)
                {
                    // If an updated line was previously drawn as
                    // double height, we need to force an update
                    // on the line proceeding incase the new line
                    // is no longer double height.

                    if ((VTVisiblePage.LineState[i] & CACHESTATE_UPDATED) != 0)
                    {
                        if (VTDoubleProfile[i] != FALSE)
                        {
                            VTVisiblePage.LineState[i + 1] |= CACHESTATE_UPDATED;
                        }
                    }
                }

                // Reburn the OSD into the reloaded page
                VT_SetPageOSD(VTPageOSD);
                VT_UpdateHilightList(TRUE);
                VT_UpdateFlashTimer();
                VT_Redraw(hDC, lpRect, VTDF_UPDATEDONLY);
            }
            return TRUE;
        }
    }
    return FALSE;
}


BOOL VT_ProcessPageRefresh(HDC hDC, LPRECT lpRect, DWORD dwPageCode)
{
    if (VTState == VT_OFF)
    {
        return FALSE;
    }

    if (LOWORD(dwPageCode) == VTPageHex)
    {
        if (VTPageSubCode == 0xFFFF && dwPageCode != VTLoadedPageCode)
        {
            VTLoadedPageCode = VTDecoder.GetDisplayPage(dwPageCode, &VTVisiblePage);
            VTDecoder.GetDisplayHeader(&VTVisiblePage, TRUE);

            // Reburn the OSD into the reloaded page
            VT_SetPageOSD(VTPageOSD);

            VTCursorRowCol = 0xFFFF;
            VT_UpdateHilightList();
            VT_UpdateFlashTimer();
            VT_Redraw(hDC, lpRect);

            return TRUE;
        }
    }

    return FALSE;
}


void VT_PerformUpscrollDuplicationFilter()
{
    if ((VTVisiblePage.wControlBits & VTCONTROL_SUBTITLE) == 0)
    {
        return;
    }

    for (int i = 1; i <= 21; i++)
    {
        if ((VTVisiblePage.LineState[i] & CACHESTATE_HASDATA) &&
            (VTVisiblePage.LineState[i] & CACHESTATE_UPDATED) &&
            (VTVisiblePage.LineState[i] & CACHESTATE_HASERROR) == 0)
        {
            if ((VTVisiblePage.LineState[i + 2] & CACHESTATE_HASDATA) &&
                (VTVisiblePage.LineState[i + 2] & CACHESTATE_UPDATED) == 0 &&
                (VTVisiblePage.LineState[i + 2] & CACHESTATE_HASERROR) == 0)
            {
                if (memcmp(VTVisiblePage.Frame[i + 2], VTVisiblePage.Frame[i], 40) == 0)
                {
                    for (int j = i + 2; j <= 23; j += 2)
                    {
                        if ((VTVisiblePage.LineState[j + 2] & CACHESTATE_HASDATA) == 0 ||
                            (VTVisiblePage.LineState[j + 2] & CACHESTATE_UPDATED))
                        {
                            break;
                        }
                        memcpy(VTVisiblePage.Frame[j], VTVisiblePage.Frame[j + 2], 40);
                        VTVisiblePage.LineState[j] |= CACHESTATE_UPDATED;
                    }

                    memset(&VTVisiblePage.Frame[j][0], 0x0d, 1);
                    memset(&VTVisiblePage.Frame[j][1], 0x20, 39);
                    VTVisiblePage.LineState[j] |= CACHESTATE_UPDATED;
                }
            }
        }
    }
}


void VT_PerformDoubleHeightSubtitlesFilter()
{
    if ((VTVisiblePage.wControlBits & VTCONTROL_SUBTITLE) == 0)
    {
        return;
    }

    for (int i = 1; i <= 22; i++)
    {
        if (VTVisiblePage.LineState[i] & CACHESTATE_HASDATA)
        {
            for (int j = 0; j < 40; j++)
            {
                if (VTVisiblePage.Frame[i][j] == 0x20)
                {
                    // Replace the first occurance of a space
                    // to a double height control character
                    VTVisiblePage.Frame[i][j] = 0x0D;
                    break;
                }
                else if (VTVisiblePage.Frame[i][j] == 0x0D)
                {
                    break;
                }
            }
        }
    }
}


void VT_UpdateFlashTimer()
{
    if (CVTCommon::HasFlashElement(&VTVisiblePage))
    {
        if (VTFlashTimer == 0)
        {
            VTFlashTimer = SetTimer(::hWnd, TIMER_VTFLASHER, TIMER_VTFLASHER_MS, NULL);
        }
    }
    else
    {
        if (VTFlashTimer != 0)
        {
            if (KillTimer(::hWnd, VTFlashTimer))
            {
                VTFlashTimer = 0;
            }
        }
    }
}


BOOL VT_ClickAtPosition(HDC hDC, LPRECT lpRect, WORD xPos, WORD yPos)
{
    if (VT_IsPageNumberAtPosition(lpRect, xPos, yPos))
    {
        if (VTCursorPageHex <= VTPAGE_FLOFBLUE &&
            VTCursorPageHex >= VTPAGE_FLOFRED)
        {
            BYTE nFlofKey = VTCursorPageHex - VTPAGE_FLOFRED;
            return VT_PerformFlofKey(hDC, lpRect, nFlofKey);
        }

        return VT_SetPage(hDC, lpRect, VTCursorPageHex);
    }

    return FALSE;
}


BOOL VT_IsPageNumberAtPosition(LPRECT lpRect, LPPOINT lpPoint)
{
    if (LOWORD(VTLoadedPageCode) != VTPageHex)
    {
        return FALSE;
    }

    WORD wRowCol = VTDrawer.GetRowColAtPoint(lpRect, lpPoint);

    if (wRowCol != VTCursorRowCol)
    {
        VTCursorRowCol = wRowCol;
        VTCursorPageHex =
            VTDrawer.FindPageNumberAtRowCol(&VTVisiblePage, VTCursorRowCol);        
    }

    return VTCursorPageHex != 0;
}


BOOL VT_IsPageNumberAtPosition(LPRECT lpRect, WORD xPos, WORD yPos)
{
    POINT Point = { xPos, yPos };
    return VT_IsPageNumberAtPosition(lpRect, &Point);
}


void VT_DeleteHilightList(TVTLeftRight** pHilightList)
{
    if (*pHilightList != NULL)
    {
        VT_DeleteHilightList(&(*pHilightList)->Next);
        free(*pHilightList);
        *pHilightList = NULL;
    }
}


void VT_UpdateHilightList(BOOL bUpdatedOnly)
{
    if (*VTSearchString == '\0' || !VTHilightSearch)
    {
        VTHilightListPtr = NULL;
        return;
    }

    VTHilightListPtr = VTHilightListArray;

    WORD wFlags = bUpdatedOnly ? PARSE_UPDATEDONLY : 0;

    CVTCommon::ParsePageElements(&VTVisiblePage, &wFlags,
        VT_UpdateHilightListProc, NULL);
}


BYTE VT_UpdateHilightListProc(TVTPage*, WORD wPoint, LPWORD lpFlags,
                              WORD, BYTE uChar, BYTE uMode, LPVOID lpParam)
{
    static BYTE           nIndex;
    static TVTLeftRight** hHilightListTail;

    BYTE nRow               = LOBYTE(wPoint);
    BYTE nCol               = HIBYTE(wPoint);

    // Don't search the header or row 24
    if (nRow == 0 || nRow == 24)
    {
        return PARSE_CONTINUE;
    }

    if (nCol == 0)
    {
        nIndex = 0;
        VT_DeleteHilightList(&VTHilightListArray[nRow]);
        hHilightListTail = &VTHilightListArray[nRow];
    }

    if (*lpFlags & PARSE_EMPTYLINE)
    {
        return PARSE_STOPLINE;
    }

    if ((uMode & VTMODE_GRAPHICS) != 0 && (uChar & 0x20))
    {
        uChar = 0x00;
    }

    if (_toupper(uChar) == _toupper(VTSearchString[nIndex]))
    {
        // Check if the last character was matched
        if (VTSearchString[++nIndex] == '\0')
        {
            // Add the hilight and continue
            *hHilightListTail = (TVTLeftRight*)malloc(sizeof(TVTLeftRight));
            (*hHilightListTail)->Left = nCol + 1 - nIndex;
            (*hHilightListTail)->Right = nCol;
            *(hHilightListTail = &(*hHilightListTail)->Next) = NULL;
            nIndex = 0;
            return PARSE_CONTINUE;
        }
    }
    else
    {
        BYTE nLength = strlen(&VTSearchString[nIndex]);

        // If the string has repetitive substrings, it is
        // necessary check if the shifting the starting point
        // to the start of the substrings will find a match.
        for (int j = 1; j < nIndex; j++)
        {
            if ((nLength + j) > (40 - nCol))
            {
                // These is not enough chars left in the
                // buffer to complete the match
                return PARSE_STOPLINE;
            }

            if (_strnicmp(VTSearchString, &VTSearchString[j], nIndex - j) == 0)
            {
                // We found a possible substring, try to match
                // the failed character again.
                if (_toupper(uChar) == _toupper(VTSearchString[nIndex - j]))
                {
                    nIndex -= j;
                    if (VTSearchString[++nIndex] == '\0')
                    {
                        // Add the highlight and continue
                        *hHilightListTail = (TVTLeftRight*)malloc(sizeof(TVTLeftRight));
                        (*hHilightListTail)->Left = nCol + 1 - nIndex;
                        (*hHilightListTail)->Right = nCol;
                        *(hHilightListTail = &(*hHilightListTail)->Next) = NULL;
                        nIndex = 0;
                        return PARSE_CONTINUE;
                    }
                    break;
                }
            }
        }

        // No luck finding a valid substring.  Start
        // the matching from the first char again.
        if (j >= nIndex)
        {
            if ((nLength + nIndex) > (40 - nCol))
            {
                // These is not enough chars left in the
                // buffer to complete the match
                return PARSE_STOPLINE;
            }

            if (nIndex != 0)
            {
                // Start again
                nIndex = 0;

                // We have restarted the matching, try to match
                // the failed character one last time.
                if (_toupper(uChar) == _toupper(VTSearchString[nIndex]))
                {
                    nIndex++;
                }
            }
        }
    }
    return PARSE_CONTINUE;
}


void VT_GetStation(LPSTR lpBuffer, LONG nLength)
{
    VTDecoder.GetStatusDisplay(lpBuffer, nLength);
}


BOOL APIENTRY VTSearchProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        SetDlgItemText(hDlg, IDC_COMBO1, VTSearchString);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            GetDlgItemText(hDlg, IDC_COMBO1, VTSearchString, 40);
            EndDialog(hDlg, TRUE);
            break;

        case IDCANCEL:
            EndDialog(hDlg, FALSE);
            break;

        default:
            ; // do nothing
            break;

        }
        break;
    }

    return (FALSE);
}


BOOL APIENTRY VTInfoProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    DWORD dwPageCode;
    char buffer[8];

    switch (message)
    {
    case WM_INITDIALOG:
        SetTimer(hDlg, 0, 200, NULL);
        // no break

    case WM_TIMER:
        SetDlgItemInt(hDlg, IDC_TEXT1, VTDecoder.GetReceivedPagesCount(), FALSE);

        if ((dwPageCode = VTDecoder.GetProcessingPageCode()) != 0)
        {
            sprintf(buffer, "%03x", LOWORD(dwPageCode));
            SetDlgItemText(hDlg, IDC_TEXT2, buffer);

            sprintf(buffer, "%X", HIWORD(dwPageCode));
            SetDlgItemText(hDlg, IDC_TEXT3, buffer);
        }
        else
        {
            SetDlgItemText(hDlg, IDC_TEXT2, "null");
            SetDlgItemText(hDlg, IDC_TEXT3, "null");
        }
        break;

    case WM_COMMAND:
        if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
        {
            KillTimer(hDlg, 0);
            EndDialog(hDlg, TRUE);
        }
        break;
    }

    return FALSE;
}


static const char* VTCodepageName[VTCODEPAGE_LASTONE] =
{
    "English",
    "French",
    "Swedish/Finnish/Hungarian",
    "Czesh/Slovak",
    "German",
    "Portuguese/Spanish",
    "Italian",
    "Polish",
    "Turkish",
    "Serbian/Croation/Slovenian",
    "Rumanian",
    "Serbian/Croatian",
    "Russian/Bulgarian",
    "Estonian",
    "Ukrainian",
    "Lettish/Lithuanian",
    "Greek",
    NULL,
    NULL,
    "Arabic",
    "Hebrew",
};


static const char* VTRegionName[VTREGION_LASTONE] = 
{
    "0: Default",
    "1: Central Europe",
    "2: Southern Europe",
    "3: Balkans",
    "4: Eastern Europe",
    NULL,
    "5: Mediterranian",
    NULL,
    "6: North Africa",
    NULL,
    "7: Middle East",
};


static const char* VTCachingControlName[3] =
{
    "0: Normal Error Correction",
    "1: Second Chance Error Correction",
    "2: Always Update",
};


void VT_UpdateMenu(HMENU hMenu)
{
    hMenu = GetVTCodepageSubmenu();

    for (int i = 0; i < VTCODEPAGE_LASTONE; i++)
    {
        if (VTCodepageName[i] != NULL)
        {
            AppendMenu(hMenu, MF_STRING,
                IDM_VT_CODEPAGE_FIRST + i, VTCodepageName[i]);
        }
    }
}


void VT_SetMenu(HMENU hMenu)
{
    for (int i = 0; i < VTCODEPAGE_LASTONE; i++)
    {
        if (VTCodepageName[i] != NULL)
        {
            EnableMenuItem(hMenu, IDM_VT_CODEPAGE_FIRST + i,
                VTAutoCodepage ? MF_GRAYED : MF_ENABLED);
            CheckMenuItemBool(hMenu, IDM_VT_CODEPAGE_FIRST + i,
                i == VTCodepage);
        }
    }
}


BOOL ProcessVTCodepageSelection(HWND hWnd, WORD wMenuID)
{
    if (wMenuID >= IDM_VT_CODEPAGE_FIRST &&
        wMenuID < (IDM_VT_CODEPAGE_FIRST + VTCODEPAGE_LASTONE))
    {
        int i = wMenuID - IDM_VT_CODEPAGE_FIRST;

        if (VTCodepageName[i] != NULL)
        {
            VTUserCodepage = (eVTCodepage)i;

            if (!VT_GetAutoCodepage())
            {
                if (VTUserCodepage != VTCodepage)
                {
                    VTCodepage = VTUserCodepage;

                    if (VT_GetState() != VT_OFF)
                    {
                        InvalidateDisplayAreaRect(hWnd, NULL, FALSE);
                    }
                }
            }
        }
        return TRUE;
    }
    return FALSE;
}


SETTING VTSettings[VT_SETTING_LASTONE] =
{
    {
        "Code Page", SLIDER, 0, (long*)&VTUserCodepage,
        VTCODEPAGE_ENGLISH, 0, VTCODEPAGE_LASTONE - 1, 1, 1,
        NULL,
        "VT", "CodePage", NULL,
    },
    {
        "Language Region", ITEMFROMLIST, 0, (long*)&VTRegion,
        VTREGION_DEFAULT, 0, VTREGION_LASTONE - 1, 1, 1,
        VTRegionName,
        "VT", "LanguageRegion", VT_RegionOnChange,
    },
    {
        "Show Page Subcode in OSD", ONOFF, 0, (long*)&VTShowSubcodeInOSD,
        FALSE, 0, 1, 1, 1,
        NULL,
        "VT", "ShowPageSubcodeInOSD", VT_ShowSubcodeInOSDOnChange,
    },
    {
        "Page Lines Caching Control", ITEMFROMLIST, 0, (long*)&VTCachingControl,
        0, 0, 2, 1, 1,
        VTCachingControlName,
        "VT", "PageLinesCachingControl", VT_CachingControlOnChange,
    },
    {
        "High Granularity Caching", ONOFF, 0, (long*)&VTHighGranularityCaching,
        FALSE, 0, 1, 1, 1,
        NULL,
        "VT", "HighGranularityCaching", VT_HighGranularityCachingOnChange,
    },
    {
        "Substitute Errors with Spaces", ONOFF, 0, (long*)&VTSubstituteErrorsWithSpaces,
        FALSE, 0, 1, 1, 1,
        NULL,
        "VT", "SubstituteErrorsWithSpaces", VT_SubstituteErrorsWithSpacesOnChange,
    },
    {
        "Filter: Upscroll Subtitle Duplication Remover", ONOFF, 0,
        (long*)&VTUpscrollDuplicationFilter,
        FALSE, 0, 1, 1, 1,
        NULL,
        "VT", "UpscrollSubtitleDuplicationFilter", NULL,
    },
    {
        "Filter: Force Double Height Subtitles", ONOFF, 0,
        (long*)&VTDoubleHeightSubtitlesFilter,
        FALSE, 0, 1, 1, 1,
        NULL,
        "VT", "ForceDoubleHeightSubtitlesFilter", NULL,
    },
};


SETTING* VT_GetSetting(VT_SETTING Setting)
{
    if (Setting > -1 && Setting < VT_SETTING_LASTONE)
    {
        return &VTSettings[Setting];
    }
    else
    {
        return NULL;
    }
}


void VT_ReadSettingsFromIni()
{
    for (int i = 0; i < VT_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&VTSettings[i]);
    }

    if (!VT_GetAutoCodepage())
    {
        VT_SetCodepage(NULL, NULL, VTUserCodepage);
    }

    VTDecoder.SetCachingControl(VTCachingControl);
    VTDecoder.SetHighGranularityCaching(VTHighGranularityCaching);
    VTDecoder.SetSubstituteSpacesForError(VTSubstituteErrorsWithSpaces);
}


void VT_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    for (int i = 0; i < VT_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&VTSettings[i], bOptimizeFileAccess);
    }
}


CTreeSettingsGeneric* VideoText_GetTreeSettingsPage()
{
    // Teletext Settings
    SETTING* VideoTextSettings[] =
    {
        &VTSettings[VT_LANGUAGE_REGION              ],
        &VTSettings[VT_SHOW_SUBCODE_OSD             ],
        &VTSettings[VT_LINES_CACHINGCONTROL         ],
        &VTSettings[VT_HIGH_GRANULARITY_CACHING     ],
        &VTSettings[VT_SUBSTITUTE_ERROR_SPACES      ],
        &VTSettings[VT_SUBTITLE_DUPLICATION_FILTER  ],
        &VTSettings[VT_DOUBLEHEIGHT_SUBTITLES_FILTER]
    };

    WORD nCount = sizeof(VideoTextSettings)/sizeof(SETTING*);
    return new CTreeSettingsGeneric("Teletext Settings", VideoTextSettings, nCount);
}


BOOL VT_RegionOnChange(long NewValue)
{
    VTRegion = (eVTRegion)NewValue;

    if (VT_GetState() != VT_OFF)
    {
        if (VT_GetAutoCodepage())
        {
            if (VT_SetCodepage(NULL, NULL, VTCODEPAGE_NONE))
            {
                InvalidateDisplayAreaRect(::hWnd, NULL, FALSE);
            }
        }
    }
    return FALSE;
}


BOOL VT_ShowSubcodeInOSDOnChange(long NewValue)
{
    VTShowSubcodeInOSD = NewValue;
    VT_SetPageOSD(VTPageOSD);

    if (VT_GetState() != VT_OFF)
    {
        InvalidateDisplayAreaRect(::hWnd, NULL, FALSE);
    }

    return FALSE;
}


BOOL VT_CachingControlOnChange(long NewValue)
{
    VTCachingControl = NewValue;
    VTDecoder.SetCachingControl(VTCachingControl);
    return FALSE;
}


BOOL VT_HighGranularityCachingOnChange(long NewValue)
{
    VTHighGranularityCaching = NewValue;
    VTDecoder.SetHighGranularityCaching(NewValue);
    return FALSE;
}


BOOL VT_SubstituteErrorsWithSpacesOnChange(long NewValue)
{
    VTSubstituteErrorsWithSpaces = NewValue;
    VTDecoder.SetSubstituteSpacesForError(NewValue);
    return FALSE;
}


BOOL VT_HilightSearchOnChange(long NewValue)
{
    bSearchHighlight = (BOOL)NewValue;
    VTHilightSearch = (BOOL)NewValue;
    VT_UpdateHilightList();
    return FALSE;
}


//////////////////////////////////////////////////////////////////////
void VBI_DecodeLine_VT(BYTE* VBI_Buffer)
{
    unsigned char data[45];
    unsigned char min, max;
    int dt[256], hi[6], lo[6];
    int i, n, sync, thr;

    // remove DC. edge-detector
    for (i = 40; i < 240; ++i)
    {
        dt[i] = VBI_Buffer[i + VTStep / FPFAC] - VBI_Buffer[i]; // amplifies the edges best.
    }

    // set barrier
    for (i = 240; i < 256; i += 2)
    {
        dt[i] = 100;
        dt[i+1] = -100;
    }

    // find 6 rising and falling edges
    for (i = 40, n = 0; n < 6; ++n)
    {
        while (dt[i] < 32)
        {
            i++;
        }
        hi[n] = i;
        while (dt[i] > -32)
        {
            i++;
        }
        lo[n] = i;
    }
    if (i >= 240)
    {
        return; // not enough periods found
    }

    i = hi[5] - hi[1];  // length of 4 periods (8 bits), normally 40.9
    if (i < 39 || i > 42)
    {
        return; // bad frequency
    }

    // AGC and sync-reference
    min = 255;
    max = 0;
    sync = 0;

    for (i = hi[4]; i < hi[5]; ++i)
    {
        if (VBI_Buffer[i] > max)
        {
            max = VBI_Buffer[i];
            sync = i;
        }
    }
    for (i = lo[4]; i < lo[5]; ++i)
    {
        if (VBI_Buffer[i] < min)
        {
            min = VBI_Buffer[i];
        }
    }

    thr = (min + max) / 2;

    // search start-byte 11100100
    //for (i = 4 * VTStep + vbi->pll_adj*VTStep/10; i < 16*VTStep; i += VTStep)
    for (i = 4 * VTStep; i < (int)(16*VTStep); i += VTStep)
    {
        if (VBI_Buffer[sync + i/FPFAC] > thr && VBI_Buffer[sync + (i+VTStep)/FPFAC] > thr) // two ones is enough...
        {
            // got it...
            memset(data, 0, sizeof(data));
            data[0] = 0x55;
            data[1] = 0x55;
            for (n = 0; n < 43*8; ++n, i += VTStep)
            {
                if (VBI_Buffer[sync + i/FPFAC] +
                    VBI_Buffer[sync + i/FPFAC - 1] +
                    VBI_Buffer[sync + i/FPFAC + 1] > 3 * thr)
                {
                    data[2 + n/8] |= 1 << (n % 8);
                }
            }

            if (data[2] != 0x27)    // really 11100100? (rev order!)
            {
                return;
            }

            VTDecoder.DecodeLine(data);
        }
    }
    return;
}

