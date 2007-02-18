/////////////////////////////////////////////////////////////////////////////
// $Id: Providers.cpp,v 1.77 2007-02-18 21:50:04 robmuller Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
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
// Revision 1.76  2007/02/18 21:32:44  robmuller
// Added option to not compile cx2388x code.
//
// Revision 1.75  2007/02/18 21:15:31  robmuller
// Added option to not compile BT8x8 code.
//
// Revision 1.74  2005/03/23 14:20:59  adcockj
// Test fix for threading issues
//
// Revision 1.73  2004/12/04 00:06:51  atnak
// Got rid of warnings in VC++ .Net 2003.
//
// Revision 1.72  2004/07/10 11:17:31  adcockj
// Fix compile error
//
// Revision 1.71  2004/07/08 08:17:52  adcockj
// Horrible work around for cx2388x issues
//
// Revision 1.70  2003/10/27 10:39:53  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.69  2003/10/10 11:16:37  laurentg
// Bug fixed : access to the audio mixer
//
// Revision 1.68  2003/09/14 09:20:59  adcockj
// Put repeated code in function
//
// Revision 1.67  2003/08/25 04:04:00  atnak
// Moved the initial Mixer_Init() call to the correct place
//
// Revision 1.66  2003/08/16 18:40:43  laurentg
// Disable access to the audio mixer dialog box for the movie file source
// Display the audio mixer dialog box at first setup of a DShow capture source
//
// Revision 1.65  2003/08/16 09:20:57  laurentg
// Disable access to audio mixer dialog box when the current source is a still
//
// Revision 1.64  2003/08/16 08:06:04  laurentg
// New message box to introduce audio mixer dialog box
//
// Revision 1.63  2003/08/15 18:26:56  laurentg
// Display the mixer setup dialog box at first init of a card
//
// Revision 1.62  2003/05/30 12:22:51  laurentg
// Avoid a double saving of the settings
//
// Revision 1.61  2003/03/27 14:10:21  laurentg
// Restore still sources before DShow sources to avoid problems when opening stills
//
// Revision 1.60  2003/03/25 00:02:11  laurentg
// DShow sources moved before the stills sources and "(DShow)" added at end of DirectShow sources
//
// Revision 1.59  2003/02/22 16:45:02  tobbej
// added a new open file dialog that allows entering urls
//
// Revision 1.58  2003/02/05 19:39:49  tobbej
// renamed some functions
//
// Revision 1.57  2003/01/11 12:53:58  adcockj
// Interim Check in of settings changes
//  - bug fixes for overlay settings changes
//  - Bug fixes for new settings changes
//  - disables settings per channel completely
//
// Revision 1.56  2003/01/10 17:38:13  adcockj
// Interrim Check in of Settings rewrite
//  - Removed SETTINGSEX structures and flags
//  - Removed Seperate settings per channel code
//  - Removed Settings flags
//  - Cut away some unused features
//
// Revision 1.55  2002/12/07 15:59:06  adcockj
// Modified mute behaviour
//
// Revision 1.54  2002/11/03 06:00:29  atnak
// Added redrawing the menu bar when it changes
//
// Revision 1.53  2002/11/02 12:40:28  laurentg
// Error message relative to DScaler.d3u updated
//
// Revision 1.52  2002/10/29 11:05:28  adcockj
// Renamed CT2388x to CX2388x
//
// Revision 1.51  2002/10/26 17:51:53  adcockj
// Simplified hide cusror code and removed PreShowDialogOrMenu & PostShowDialogOrMenu
//
// Revision 1.50  2002/10/08 20:39:01  atnak
// enabled saa7134 stuff
//
// Revision 1.49  2002/10/08 20:17:48  laurentg
// Calls to PreShowDialogOrMenu / PostShowDialogOrMenu added
//
// Revision 1.48  2002/10/07 22:30:57  kooiman
// Fixed NewValue=pSource.
//
// Revision 1.47  2002/09/28 13:31:41  kooiman
// Added sender object to events and added setting flag to treesettingsgeneric.
//
// Revision 1.46  2002/09/26 11:33:42  kooiman
// Use event collector
//
// Revision 1.45  2002/09/17 21:38:48  laurentg
// By default, use the first BT8x8/CX2388x/SAA7134 source as initial source
//
// Revision 1.44  2002/09/16 19:34:19  adcockj
// Fix for auto format change
//
// Revision 1.43  2002/09/14 20:18:16  atnak
// Prelimainary support for SAA713x based cards
//
// Revision 1.42  2002/09/11 18:19:43  adcockj
// Prelimainary support for CX2388x based cards
//
// Revision 1.41  2002/08/13 21:16:06  kooiman
// Added source change notification.
//
// Revision 1.40  2002/08/11 12:12:10  laurentg
// Cut BT Card setup and general hardware setup in two different windows
//
// Revision 1.39  2002/07/29 17:43:29  tobbej
// support for opening graphedit saved filter graphs
//
// Revision 1.38  2002/07/26 22:40:55  laurentg
// Menus updates
//
// Revision 1.37  2002/05/01 13:00:18  laurentg
// Support of JPEG files added
//
// Revision 1.36  2002/04/28 16:46:49  laurentg
// Reinit WSS data when switching source
//
// Revision 1.35  2002/04/27 16:02:59  laurentg
// Initial source
//
// Revision 1.34  2002/04/27 11:21:04  tobbej
// fixed crashing, dont use c style malloc to allocate memory for structs with c++ members
//
// Revision 1.33  2002/04/27 00:38:33  laurentg
// New default source (still) used at DScaler startup or when there is no more source accessible
//
// Revision 1.32  2002/04/13 21:52:40  laurentg
// Management of no current source
//
// Revision 1.31  2002/04/13 18:56:23  laurentg
// Checks added to manage case where the current source is not yet defined
//
// Revision 1.30  2002/04/07 14:55:04  tobbej
// added asf and wmv filetypes to file-open dialog
//
// Revision 1.29  2002/04/06 11:46:45  laurentg
// Check that the current source is not NULL to avoid DScaler exits
//
// Revision 1.28  2002/03/02 18:33:56  laurentg
// At startup, mute the audio of all unused cards
//
// Revision 1.27  2002/02/19 16:03:36  tobbej
// removed CurrentX and CurrentY
// added new member in CSource, NotifySizeChange
//
// Revision 1.26  2002/02/18 23:25:01  laurentg
// At startup, go to the first source having a content
// Order of still sources changed (Patterns before Snapshots)
//
// Revision 1.25  2002/02/17 00:35:35  laurentg
// Problem to modify source submenu when menubar is hidden is solved
//
// Revision 1.24  2002/02/12 16:33:40  tobbej
// updated file-open menu with filetype for media files (avi for example)
//
// Revision 1.23  2002/02/11 21:33:13  laurentg
// Patterns as a new source from the Still provider
//
// Revision 1.22  2002/02/09 21:12:28  laurentg
// Old test patterns restored
// Loading of d3u files improved (absolute or relative path)
//
// Revision 1.21  2002/02/09 14:46:04  laurentg
// OSD main screen updated to display the correct input name (or channel)
// OSD main screen updated to display only activated filters
// Menu label for the BT848 providers now displays the name of the card
//
// Revision 1.20  2002/02/09 02:51:38  laurentg
// Grayed the channels when the source has no tuner
//
// Revision 1.19  2002/02/08 00:36:06  laurentg
// Support of a new type of file : DScaler patterns
//
// Revision 1.18  2002/02/02 12:41:44  laurentg
// CurrentX and CurrentY set when changing source and when switching between still files
//
// Revision 1.17  2002/02/02 01:31:18  laurentg
// Access to the files of the playlist added in the menus
// Save Playlist added
// "Video Adjustments ..." restored in the popup menu
//
// Revision 1.16  2002/02/01 00:41:58  laurentg
// Playlist code updated
//
// Revision 1.15  2002/01/24 00:00:13  robmuller
// Added bOptimizeFileAccess flag to WriteToIni from the settings classes.
//
// Revision 1.14  2001/12/09 22:00:42  tobbej
// experimental dshow support, doesnt work yet
// define WANT_DSHOW_SUPPORT if you want to try it
//
// Revision 1.13  2001/12/08 20:00:24  laurentg
// Access control on sources
//
// Revision 1.12  2001/12/08 13:48:40  laurentg
// New StillSource for snapshots done during the DScaler session
//
// Revision 1.11  2001/12/08 12:01:26  laurentg
// Providers_AddSource and Providers_RemoveSource deleted
//
// Revision 1.10  2001/11/28 16:04:50  adcockj
// Major reorganization of STill support
//
// Revision 1.9  2001/11/25 21:29:50  laurentg
// Take still, Open file, Close file callbacks updated
//
// Revision 1.8  2001/11/24 22:54:25  laurentg
// Close file added for still source
//
// Revision 1.7  2001/11/24 17:58:06  laurentg
// Still source
//
// Revision 1.6  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.5  2001/11/21 12:32:11  adcockj
// Renamed CInterlacedSource to CSource in preparation for changes to DEINTERLACE_INFO
//
// Revision 1.4  2001/11/14 11:28:03  adcockj
// Bug fixes
//
// Revision 1.3  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.2  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.1.2.6  2001/08/23 16:04:57  adcockj
// Improvements to dynamic menus to remove requirement that they are not empty
//
// Revision 1.1.2.5  2001/08/21 16:42:16  adcockj
// Per format/input settings and ini file fixes
//
// Revision 1.1.2.4  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.1.2.3  2001/08/18 17:09:30  adcockj
// Got to compile, still lots to do...
//
// Revision 1.1.2.2  2001/08/17 16:35:14  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
// Revision 1.1.2.1  2001/08/15 14:44:05  adcockj
// Starting to put some flesh onto the new structure
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file Providers.cpp Providers functions
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "Events.h"
#include "resource.h"
#include "Providers.h"
#include "StillProvider.h"
#include "BT848Provider.h"
#include "CX2388xProvider.h"
#include "SAA7134Provider.h"
#include "HardwareDriver.h"
#include "OutThreads.h"
#include "DScaler.h"
#include "Audio.h"
#include "VBI_WSSdecode.h"
#include "SettingsMaster.h"
#include "OpenDlg.h"
#include "MixerDev.h"

#ifdef WANT_DSHOW_SUPPORT
#include "dshowsource\DSProvider.h"
static CDSProvider* DSProvider = NULL;
#endif


typedef struct
{
    std::string Name;
    CSource*    Object;
    BOOL        DisplayInMenu;
} TSource;

typedef vector<TSource*> SOURCELIST;

extern HMENU hMenu;

void Providers_NotifySourceChange(int OldSource);
void Providers_NotifySourcePreChange();

static SOURCELIST Sources;
static CHardwareDriver* HardwareDriver = NULL;

#ifdef WANT_BT8X8_SUPPORT
static CBT848Provider* BT848Provider = NULL;
#endif

#ifdef WANT_CX2388X_SUPPORT
static CCX2388xProvider* CX2388xProvider = NULL;
#endif

#ifdef WANT_SAA713X_SUPPORT
static CSAA7134Provider* SAA7134Provider = NULL;
#endif

static CStillProvider* StillProvider = NULL;
static long CurrentSource = 0;
static long DefSourceIdx = -1;
long InitSourceIdx = -1;

extern char szIniFile[MAX_PATH];

void Providers_MixerSetup()
{
	MessageBox(GetMainWnd(),
		"The following dialog will allow you to configure how "
		"DScaler uses the system mixer.  Configuring DScaler to use the "
		"system mixer allows DScaler to change the volume level and the "
		"mute state of your sound card by using the Windows mixer.  This is a "
		"must for cards that do not support these functions in hardware. "
		"You should specify the line into which your TV card's loopback "
		"sound cable is attached.\n"
		"\n"
		"If you do not wish to let DScaler use the system mixer, leave "
		"the \"Use the system mixer\" option unchecked.  If unsure, it "
		"is recommended you use the system mixer.", "Next...", MB_OK);

    Mixer_SetupDlg(GetMainWnd());
}


int Providers_Load(HMENU hMenu)
{
    int i(0);
    TSource* Source;
    CSource* DefaultSource = NULL;
    MENUITEMINFO    MenuItemInfo;
        bool AllowCx2388xDShow = false;

    HMENU hSubMenu = GetSubMenu(hMenu, 0);

    Providers_Unload();
    HardwareDriver = new CHardwareDriver();
    if(HardwareDriver->LoadDriver() == TRUE)
    {
#ifdef WANT_BT8X8_SUPPORT
        BT848Provider = new CBT848Provider(HardwareDriver);
        for(i = 0; i < BT848Provider->GetNumberOfSources(); ++i)
        {
            Source = new TSource;
            if (BT848Provider->GetSource(i)->GetMenuLabel() == NULL)
            {
                Source->Name = "BT848 Card";
            }
            else
            {
                Source->Name = BT848Provider->GetSource(i)->GetMenuLabel();
            }
            Source->Object = BT848Provider->GetSource(i);
            Source->DisplayInMenu = TRUE;
            Sources.push_back(Source);

			// Set CurrentSource for Providers_GetCurrentSource function
			// used in mixer code
            CurrentSource = Sources.size()-1;
			// The first time, setup the audio mixer for the card
			if (((CBT848Source*)(BT848Provider->GetSource(i)))->IsInitialSetup())
			{
                Providers_MixerSetup();
			}

            // Mute the audio of each source
            BT848Provider->GetSource(i)->Mute();
        }
#endif // WANT_BT8X8_SUPPORT

#ifdef WANT_CX2388X_SUPPORT
        CX2388xProvider = new CCX2388xProvider(HardwareDriver);

        // if we have any of these cards warn the user about the driver issues
        // hopefully this will be enought to get a new release out
        if(CX2388xProvider->GetNumberOfSources() > 0)
        {
            int RegFlag = GetPrivateProfileInt("CX2388x", "UseDShow", -1, szIniFile);
            if(RegFlag == -1)
            {
                int Resp = MessageBox(GetMainWnd(), " You have a CX2388x card.  There have been several reported instability "
                    "problems with these cards if we allow the drivers to run.  Beacuse of this you can either run with "
                    "DScaler's own drivers or with the DShow drivers but not both.  Do you want to use DScaler's own driver?",
                    "CX2388x Question", MB_YESNO | MB_ICONQUESTION);
                if(Resp == IDYES)
                {
                    AllowCx2388xDShow = false;
                    WritePrivateProfileInt("CX2388x", "UseDShow", 0, szIniFile);
                }
                else
                {
                    AllowCx2388xDShow = true;
                    WritePrivateProfileInt("CX2388x", "UseDShow", 1, szIniFile);
                }
            }
            else
            {
                AllowCx2388xDShow = (RegFlag == 1);
            }
        }

        for(i = 0; i < CX2388xProvider->GetNumberOfSources(); ++i)
        {
            Source = new TSource;
            if (CX2388xProvider->GetSource(i)->GetMenuLabel() == NULL)
            {
                Source->Name = "CX Card";
            }
            else
            {
                Source->Name = CX2388xProvider->GetSource(i)->GetMenuLabel();
            }
            Source->Object = CX2388xProvider->GetSource(i);
            Source->DisplayInMenu = TRUE;
            Sources.push_back(Source);

			// Set CurrentSource for Providers_GetCurrentSource function
			// used in mixer code
            CurrentSource = Sources.size()-1;
			// The first time, setup the audio mixer for the card
			if (((CCX2388xSource*)(CX2388xProvider->GetSource(i)))->IsInitialSetup())
			{
                Providers_MixerSetup();
            }

            // Mute the audio of each source
            CX2388xProvider->GetSource(i)->Mute();
        }
#endif // WANT_CX2388X_SUPPORT

#ifdef WANT_SAA713X_SUPPORT
        SAA7134Provider = new CSAA7134Provider(HardwareDriver);
        for(i = 0; i < SAA7134Provider->GetNumberOfSources(); ++i)
        {
            Source = new TSource;
            if (SAA7134Provider->GetSource(i)->GetMenuLabel() == NULL)
            {
                Source->Name = "SAA7134 Card";
            }
            else
            {
                Source->Name = SAA7134Provider->GetSource(i)->GetMenuLabel();
            }
            Source->Object = SAA7134Provider->GetSource(i);
            Source->DisplayInMenu = TRUE;
            Sources.push_back(Source);

			// Set CurrentSource for Providers_GetCurrentSource function
			// used in mixer code
            CurrentSource = Sources.size()-1;
			// The first time, setup the audio mixer for the card
			if (((CSAA7134Source*)(SAA7134Provider->GetSource(i)))->IsInitialSetup())
			{
                Providers_MixerSetup();
			}

            // Mute the audio of each source
            SAA7134Provider->GetSource(i)->Mute();
        }
#endif//xxx

		// Use by default the first BT8x8/CX2388x/SAA7134 source as initial source
		// Do that before loading other sources
		if (InitSourceIdx == -1)
		{
            InitSourceIdx = Providers_FindSource();
		}
    }
    else
    {
        ErrorBox("Can't load Hardware Driver, possibly caused by corrupt installation.  Reboot and try again.");
        delete HardwareDriver;
        HardwareDriver = NULL;
    }

    StillProvider = new CStillProvider();
    for(i = 0; i < StillProvider->GetNumberOfSources(); ++i)
    {
        Source = new TSource;
        if (StillProvider->GetSource(i)->GetMenuLabel() == NULL)
        {
            Source->Name = "Still";
        }
        else
        {
            Source->Name = StillProvider->GetSource(i)->GetMenuLabel();
        }
        Source->Object = StillProvider->GetSource(i);
        Source->DisplayInMenu = TRUE;
        Sources.push_back(Source);
    }

#ifdef WANT_DSHOW_SUPPORT
    DSProvider = new CDSProvider();
    for(i = 0; i < DSProvider->GetNumberOfSources(); ++i)
    {
        if(AllowCx2388xDShow || strstr(DSProvider->GetSource(i)->IDString(), "VEN_14F1&DEV_88") == NULL)
        {
            Source = new TSource;
            Source->Name = DSProvider->GetSourceName(i);
            Source->Name += " (DShow)";
            Source->Object = DSProvider->GetSource(i);
            Source->DisplayInMenu = TRUE;
            Sources.push_back(Source);

		    // Set CurrentSource for Providers_GetCurrentSource function
		    // used in mixer code
            CurrentSource = Sources.size()-1;
		    // The first time, setup the audio mixer for the card
		    if (((CDSSourceBase*)(DSProvider->GetSource(i)))->IsInitialSetup())
		    {
                Providers_MixerSetup();
		    }

            DSProvider->GetSource(i)->Mute();
        }
    }
#endif

    DefaultSource = Providers_GetIntroSource();
    if (!DefaultSource || !DefaultSource->IsAccessAllowed())
    {
        ErrorBox("Can't load file DScaler.d3u or file defined in it");

        // We destroy the source if it exists
        DefSourceIdx = Providers_GetSourceIndex(DefaultSource);
        if (DefSourceIdx >= 0 && static_cast<size_t>(DefSourceIdx) < Sources.size())
        {
            Source = *(Sources.begin() + DefSourceIdx);
            Sources.erase(Sources.begin() + DefSourceIdx);
            delete Source;
        }

        DefaultSource = NULL;
    }

    DefSourceIdx = Providers_GetSourceIndex(DefaultSource);

    // The default source is not listed in the menu
    if (DefSourceIdx >= 0 && static_cast<size_t>(DefSourceIdx) < Sources.size())
    {
        Source = *(Sources.begin() + DefSourceIdx);
        Source->DisplayInMenu = FALSE;
    }

    if (InitSourceIdx >= 0 && static_cast<size_t>(InitSourceIdx) < Sources.size() &&
        Sources[InitSourceIdx]->Object->IsAccessAllowed())
    {
        CurrentSource = InitSourceIdx;
    }
    else if (DefSourceIdx >= 0 && static_cast<size_t>(DefSourceIdx) < Sources.size())
    {
        CurrentSource = DefSourceIdx;
    }
    else
    {
        CurrentSource = Providers_FindSource();
    }

    Providers_NotifySourceChange(-1);

    for(size_t n = 0; n < Sources.size() && n < 100 ; ++n)
    {
        if (Sources[n]->DisplayInMenu)
        {
//            AppendMenu(hSubMenu, MF_STRING | MF_ENABLED, IDM_SOURCE_FIRST + n, Sources[n]->Name.c_str());
            MenuItemInfo.cbSize = sizeof (MenuItemInfo);
            MenuItemInfo.fMask = MIIM_TYPE | MIIM_ID;
            MenuItemInfo.fType = MFT_STRING;
            MenuItemInfo.dwTypeData = (LPSTR) Sources[n]->Name.c_str();
            MenuItemInfo.cch = strlen (Sources[n]->Name.c_str());
            MenuItemInfo.wID = IDM_SOURCE_FIRST + n;
            InsertMenuItem(hSubMenu, GetMenuItemCount(hSubMenu) - 4, TRUE, &MenuItemInfo);
        }
    }

    Providers_UpdateMenu(hMenu);

    return Sources.size();
}

void Providers_Unload()
{
#ifdef WANT_DSHOW_SUPPORT
	if(DSProvider!=NULL)
	{
		delete DSProvider;
		DSProvider=NULL;
	}
#endif
    if(StillProvider != NULL)
    {
        delete StillProvider;
        StillProvider = NULL;
    }
#ifdef WANT_BT8X8_SUPPORT
    if(BT848Provider != NULL)
    {
        delete BT848Provider;
        BT848Provider = NULL;
    }
#endif
#ifdef WANT_CX2388X_SUPPORT
    if(CX2388xProvider != NULL)
    {
        delete CX2388xProvider;
        CX2388xProvider = NULL;
    }
#endif
#ifdef WANT_SAA713X_SUPPORT
    if(SAA7134Provider != NULL)
    {
        delete SAA7134Provider;
        SAA7134Provider = NULL;
    }
#endif
    if(HardwareDriver != NULL)
    {
        HardwareDriver->UnloadDriver();
        delete HardwareDriver;
        HardwareDriver = NULL;
    }
    for(vector<TSource*>::iterator it = Sources.begin(); 
        it != Sources.end(); 
        ++it)
    {
        delete *it;
    }
    Sources.clear();
}

CSource* Providers_GetCurrentSource()
{
    if(CurrentSource >= 0 && static_cast<size_t>(CurrentSource) < Sources.size())
    {
        return Sources[CurrentSource]->Object;
    }
    else
    {
        return NULL;
    }
}

long Providers_GetSourceIndex(CSource* Src)
{
    for (size_t i(0) ; i < Sources.size() ; i++)
    {
        if (Sources[i]->Object == Src)
        {
            return static_cast<long>(i);
        }
    }
    return -1;
}

CSource* Providers_GetStillsSource()
{
    return StillProvider->GetSource(0);
}

CSource* Providers_GetSnapshotsSource()
{
    return StillProvider->GetSource(2);
}

CSource* Providers_GetPatternsSource()
{
    return StillProvider->GetSource(1);
}

CSource* Providers_GetIntroSource()
{
    return StillProvider->GetSource(3);
}

BOOL Providers_IsStillSource(CSource* source)
{
	if (StillProvider != NULL)
	{
		for (int i=0 ; i<StillProvider->GetNumberOfSources() ; i++)
		{
			if (StillProvider->GetSource(i) == source)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

int Providers_FindSource()
{
    size_t SourceIdx = 0;
    while ((SourceIdx < Sources.size()) && !Sources[SourceIdx]->Object->IsAccessAllowed())
    {
        SourceIdx++;
    }
    if(SourceIdx >= 0 && SourceIdx < Sources.size())
    {
        return static_cast<int>(SourceIdx);
    }
    else
    {
        return -1;
    }
}

void Providers_SetMenu(HMENU hMenu)
{
    for(size_t i(0); i < Sources.size(); ++i)
    {
        if (Sources[i]->DisplayInMenu)
        {
            CheckMenuItemBool(hMenu, IDM_SOURCE_FIRST + i, (CurrentSource == i));
            EnableMenuItem(hMenu, IDM_SOURCE_FIRST + i, Sources[i]->Object->IsAccessAllowed() ? MF_ENABLED : MF_GRAYED);
        }
    }

    if(CurrentSource >= 0 && static_cast<size_t>(CurrentSource) < Sources.size())
    {
        Sources[CurrentSource]->Object->SetMenu(hMenu);
    }
}

void Providers_UpdateMenu(HMENU hMenu)
{
    // first we need to detach the old menu if there is one
    // if we don't do this our menu gets deleted
    RemoveMenu(hMenu, 1, MF_BYPOSITION);

    if(CurrentSource >= 0 && static_cast<size_t>(CurrentSource) < Sources.size())
    {
        // get The name of our menu
        char Text[256];
        HMENU hSubMenu = Sources[CurrentSource]->Object->GetSourceMenu();
        GetMenuString(hSubMenu, 0, Text, 256, MF_BYPOSITION);
        // Add the new menu
        InsertMenu(hMenu, 1, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT)GetSubMenu(hSubMenu, 0), Text);

        // Update our menu
        Sources[CurrentSource]->Object->UpdateMenu();
    }
    else
    {
        // Add an empty new menu
        InsertMenu(hMenu, 1, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT)CreatePopupMenu(), "No source");
    }

    RedrawMenuBar(hMenu);
}

BOOL Providers_HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam)
{
    if(LOWORD(wParam) >= IDM_SOURCE_FIRST && LOWORD(wParam) <= IDM_SOURCE_LAST)
    {
        int NewSource(LOWORD(wParam) - IDM_SOURCE_FIRST);
        if(NewSource >= 0 && static_cast<size_t>(NewSource) < Sources.size())
        {
            Providers_NotifySourcePreChange();
            Stop_Capture();
            WSS_init();
            int OldSource = CurrentSource;
            CurrentSource = NewSource;
            Providers_NotifySourceChange(OldSource);
            Providers_UpdateMenu(hMenu);
            Start_Capture();
            return TRUE;
        }
    }
    else if (LOWORD(wParam) == IDM_SOURCE_INITIAL)
    {
        InitSourceIdx = CurrentSource;
        return TRUE;
    }
    else if (LOWORD(wParam) == IDM_OPEN_FILE)
    {
        CString file;
		if(COpenDlg::ShowOpenDialog(hWnd,file))
        {
            Stop_Capture();
            for(size_t i = 0; i < Sources.size(); ++i)
            {
                if(Sources[i]->Object->OpenMediaFile(file, FALSE))
                {                    
                    Providers_NotifySourcePreChange();
                    
                    int OldSource = CurrentSource;
                    CurrentSource = static_cast<long>(i);
                    WSS_init();
                    Providers_UpdateMenu(hMenu);
                    Start_Capture();
                    Providers_NotifySourceChange(OldSource);
                    return TRUE;
                }
            }
            Start_Capture();
            MessageBox(hWnd, "Unsupported File Type", "DScaler Warning", MB_OK);
            return TRUE;
        }
    }
    else if (LOWORD(wParam) == IDM_SWITCH_SOURCE)
    {
        Providers_NotifySourcePreChange();            
        
        Stop_Capture();
        WSS_init();
        int OldSource = CurrentSource;
        if (DefSourceIdx >= 0 && static_cast<size_t>(DefSourceIdx) < Sources.size())
        {
            CurrentSource = DefSourceIdx;
        }
        else
        {
            CurrentSource = Providers_FindSource();
        }
        Providers_UpdateMenu(hMenu);
        Start_Capture();
        
        Providers_NotifySourceChange(OldSource);
        return TRUE;
    }
    if(CurrentSource >= 0 && static_cast<size_t>(CurrentSource) < Sources.size())
    {
        return Sources[CurrentSource]->Object->HandleWindowsCommands(hWnd, wParam, lParam);
    }
    return FALSE;
}


long Providers_GetNumber()
{
    return Sources.size();
}

CSource*  Providers_GetByIndex(long Index)
{
    return Sources[Index]->Object;
}


void Provider_HandleTimerMessages(int TimerId)
{
    if(CurrentSource >= 0 && static_cast<size_t>(CurrentSource) < Sources.size())
    {
        Sources[CurrentSource]->Object->HandleTimerMessages(TimerId);
    }
}

void Providers_ReadFromIni()
{
    if(CurrentSource >= 0 && static_cast<size_t>(CurrentSource) < Sources.size())
    {
        Sources[CurrentSource]->Object->ReadFromIni();
    }
}

void Providers_WriteToIni(BOOL bOptimizeFileAccess)
{
    if(CurrentSource >= 0 && static_cast<size_t>(CurrentSource) < Sources.size())
    {
        Sources[CurrentSource]->Object->WriteToIni(bOptimizeFileAccess);
    }
}

void Providers_ChangeSettingsBasedOnHW(int ProcessorSpeed, int TradeOff)
{
    for (size_t i(0) ; i < Sources.size() ; i++)
    {
        Sources[i]->Object->ChangeSettingsBasedOnHW(ProcessorSpeed, TradeOff);
    }
}


void Providers_NotifySourcePreChange()
{
    if (EventCollector != NULL)
    {
        EventCollector->RaiseEvent(NULL, EVENT_SOURCE_PRECHANGE, (long)Providers_GetCurrentSource(), (long)Providers_GetCurrentSource(), NULL);
    }
    if(Providers_GetCurrentSource() != NULL)
    {
        Providers_GetCurrentSource()->UnsetSourceAsCurrent();
    }
	// Laurent 30/05/2003 Not necessary because it is done two lines below
	// when calling UnsetSourceAsCurrent
    // good time to save the current settings
    // SettingsMaster->SaveSettings();
}

void Providers_NotifySourceChange(int OldSource)
{
    CSource* pOldSource = NULL;

    if(OldSource >= 0 && static_cast<size_t>(OldSource) < Sources.size())
    {
        pOldSource = Sources[OldSource]->Object;
    }
    if (EventCollector != NULL)
    {
        EventCollector->RaiseEvent(NULL, EVENT_SOURCE_CHANGE, (long)pOldSource, (long)Providers_GetCurrentSource(), NULL);
    }

    if(Providers_GetCurrentSource() != NULL)
    {
        Providers_GetCurrentSource()->SetSourceAsCurrent();
    }
}
