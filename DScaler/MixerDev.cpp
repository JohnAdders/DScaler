/////////////////////////////////////////////////////////////////////////////
// $Id: MixerDev.cpp,v 1.44 2003-08-15 09:29:08 atnak Exp $
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
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.43  2003/07/30 03:24:34  atnak
// Added NOHARDWAREMUTE to list of saved settings
//
// Revision 1.42  2003/07/29 13:40:02  atnak
// Hide IDC_MIXER_INPUTNAME_ALL too fix
//
// Revision 1.41  2003/07/29 13:33:07  atnak
// Overhauled mixer code
//
// Revision 1.40  2003/06/02 13:15:32  adcockj
// Fixes for CHARSTRING problems
//
// Revision 1.39  2003/04/26 19:39:10  laurentg
// New character string settings
//
// Revision 1.38  2003/01/15 15:54:22  adcockj
// Fixed some keyboard focus issues
//
// Revision 1.37  2002/12/10 12:15:59  atnak
// Tweaked Mixer_OnInputChange() to what I thought was better.
//
// Revision 1.36  2002/12/09 00:32:14  atnak
// Added new muting stuff
//
// Revision 1.35  2002/10/26 17:51:53  adcockj
// Simplified hide cusror code and removed PreShowDialogOrMenu & PostShowDialogOrMenu
//
// Revision 1.34  2002/10/18 03:33:00  flibuste2
// Added Mixer_IsMuted()
// (need by Audio.Audio_IsMuted())
// and factorized some code
//
// Revision 1.33  2002/10/08 20:17:48  laurentg
// Calls to PreShowDialogOrMenu / PostShowDialogOrMenu added
//
// Revision 1.32  2002/10/07 22:30:31  kooiman
// Fixed some mixer channel mute issues.
//
// Revision 1.31  2002/09/29 13:56:30  adcockj
// Fixed some cursor hide problems
//
// Revision 1.30  2002/09/28 13:31:41  kooiman
// Added sender object to events and added setting flag to treesettingsgeneric.
//
// Revision 1.29  2002/09/26 16:35:20  kooiman
// Volume event support.
//
// Revision 1.28  2002/09/26 11:33:42  kooiman
// Use event collector
//
// Revision 1.27  2002/09/06 15:08:10  kooiman
// Mixer lines now source dependent.
//
// Revision 1.26  2002/08/27 22:05:21  kooiman
// Fixed wave mute. Finished mixer channel change per video input. Use video input names for mixer lines.
//
// Revision 1.25  2002/08/18 13:30:38  tobbej
// fixed problem with "other 2" line always geting the wrong value (0 in ini file)
//
// Revision 1.24  2002/04/13 18:56:23  laurentg
// Checks added to manage case where the current source is not yet defined
//
// Revision 1.23  2001/12/18 13:12:11  adcockj
// Interim check-in for redesign of card specific settings
//
// Revision 1.22  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.21  2001/11/19 11:11:45  temperton
// Store mixer name instead of index in ini.
//
// Revision 1.20  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.19  2001/11/02 16:32:54  adcockj
// Removed conflict tags
//
// Revision 1.18  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.17  2001/10/22 10:32:58  temperton
// bugfix.
//
// Revision 1.16  2001/08/18 17:24:12  adcockj
// Removed free causing crash on exit
//
// Revision 1.15  2001/08/14 11:36:03  adcockj
// Mixer change to allow restore of initial mixer settings
//
// Revision 1.14.2.3  2001/08/21 09:43:01  adcockj
// Brought branch up to date with latest code fixes
//
// Revision 1.14.2.2  2001/08/17 16:35:14  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
// Revision 1.14.2.1  2001/08/14 16:41:37  adcockj
// Renamed driver
// Got to compile with new class based card
//
// Revision 1.14  2001/07/16 18:07:50  adcockj
// Added Optimisation parameter to ini file saving
//
// Revision 1.13  2001/07/15 13:24:05  adcockj
// Fixed crashing after overlay failure with mixer on
//
// Revision 1.12  2001/07/13 18:13:24  adcockj
// Changed Mute to not be persisted and to work properly
//
// Revision 1.11  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.10  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "MixerDev.h"
#include "DScaler.h"
#include "Audio.h"
#include "Providers.h"

#define MAX_SUPPORTED_INPUTS  6                     // Not readily changable


//  MixerDev prototypes and globals

void    _MixerDev_ReadSettingsFromIni();
void    MixerDev_SettingSetSection(CSource* pSource);
void    MixerDev_SettingSetSection(LPCSTR szSource);

static std::string  g_MixerDev_Section;
static BOOL         g_bMixerDevInvalidSection = TRUE;


//  Mixer typedefs

typedef CMixer* (tSyncChangesCallback)(void* pContext);


//  Mixer prototypes

static void     Mixer_SetCurrentMixerFromName();
static void     Mixer_SetCurrentMixer(long nMixerIndex);

static CMixerLineDst*   Mixer_GetCurrentDestination();
static CMixerLineSrc*   Mixer_GetCurrentActiveSource();

static void     Mixer_EventOnChangeNotification(void*, CEventObject*, eEventType, long, long, eEventType*);

static void     Mixer_OnInputChange(long nVideoInput);
static void     Mixer_OnSourceChange(CSource *pSource);

static CMixer*  LoadSourceSettingsCallback(void* pContext);

static void     Mixer_SetupActiveInputs(CSource* pCurrentSource);

static void     Mixer_MuteInputs(long nDestinationIndex, long* pIndexes, long nCount, BOOL bMute);
static void     Mixer_StoreRestoreInputs(long nDestinationIndex, long* pIndexes, long nCount, BOOL bRestore);

static void     Mixer_DoSettingsTransition(tSyncChangesCallback* pSyncfunc, void* pContext);

static long     Mixer_NameToIndex(char* szName);
static CMixer*  Mixer_Allocate(long nMixerIndex);

BOOL APIENTRY MixerSetupProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);


//  Utility prototypes

static long     LongArrayUnique(const long*, long, long*);
static void     LongArraySubstract(long*, long*, const long*, long);
static void     LongArrayDivide(long*, long *, long*, long *);


// Global variables

static CMixer*  g_pCurrentMixer = NULL;
static long     g_nActiveInputsCount = 0;
static long     g_nActiveInput = -1;

static BOOL     g_bUseMixer = FALSE;                        // Saved setting variable
static BOOL     g_bResetOnExit = FALSE;                     // Saved setting variable
static BOOL     g_bNoHardwareMute = FALSE;                  // Saved setting variable

static char*    g_pMixerName = NULL;                        // Saved setting variable
                                                            // - mem-managed by Setting_Funcs

static long     g_nDestinationIndex;                        // Saved setting variable
static long     g_nSourceIndexes[MAX_SUPPORTED_INPUTS];     // Saved setting variable


//----------------------------------------------------------------------
//  Public functions
//----------------------------------------------------------------------

void Mixer_Init()
{
    eEventType EventList[] = { EVENT_SOURCE_CHANGE,
                               EVENT_VIDEOINPUT_CHANGE,
                               EVENT_NONE };

    EventCollector->Register(Mixer_EventOnChangeNotification, NULL, EventList);

    // This call takes care of all necessary initializations
    Mixer_OnSourceChange(Providers_GetCurrentSource());
}


void Mixer_Exit()
{
    EventCollector->Unregister(Mixer_EventOnChangeNotification, NULL);

    // This call takes care of all necessary changes
    Mixer_OnSourceChange(NULL);
}


BOOL Mixer_IsEnabled()
{
    return g_bUseMixer;
}


BOOL Mixer_IsNoHardwareMute()
{
    return g_bUseMixer && g_bNoHardwareMute;
}


void Mixer_SetMute(BOOL bEnabled)
{
    CMixerLineSrc* pLineSrc = Mixer_GetCurrentActiveSource();

    if (pLineSrc != NULL)
    {
        pLineSrc->SetMute(bEnabled);
    }
}


BOOL Mixer_GetMute(void)
{
    CMixerLineSrc* pLineSrc = Mixer_GetCurrentActiveSource();

    if (pLineSrc != NULL)
    {
        return pLineSrc->GetMute();
    }

    return FALSE;
}


void Mixer_Mute()
{
    Mixer_SetMute(TRUE);
}


void Mixer_UnMute()
{
    Mixer_SetMute(FALSE);
}


void Mixer_SetVolume(long newVolume)
{
    CMixerLineSrc* pLineSrc = Mixer_GetCurrentActiveSource();

    if (pLineSrc != NULL)
    {
        long oldVolume = pLineSrc->GetVolume();

        if (newVolume < 0)
        {
            newVolume = 0;
        }
        else if (newVolume > 100)
        {
            newVolume = 100;
        }

        pLineSrc->SetVolume(newVolume);

		EventCollector->RaiseEvent(NULL, EVENT_MIXERVOLUME, oldVolume, newVolume);
    }
}


long Mixer_GetVolume()
{
    CMixerLineSrc* pLineSrc = Mixer_GetCurrentActiveSource();

    if (pLineSrc != NULL)
    {
        return pLineSrc->GetVolume();
    }

    return -1;
}


void Mixer_AdjustVolume(long delta)
{
    CMixerLineSrc* pLineSrc = Mixer_GetCurrentActiveSource();

    if (pLineSrc != NULL)
    {
        long oldVolume = pLineSrc->GetVolume();
        long newVolume = oldVolume + delta;

        if (newVolume < 0)
        {
            newVolume = 0;
        }
        else if (newVolume > 100)
        {
            newVolume = 100;
        }

        pLineSrc->SetVolume(newVolume);

		EventCollector->RaiseEvent(NULL, EVENT_MIXERVOLUME, oldVolume, newVolume);
    }
}


void Mixer_Volume_Up()
{
    Mixer_AdjustVolume(5);
}


void Mixer_Volume_Down()
{
    Mixer_AdjustVolume(-5);
}


void Mixer_SetupDlg(HWND hWndParent)
{
    CMixerFinder mixerFinder;

    if (mixerFinder.GetMixerCount() > 0)
    {
        DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_MIXERSETUP), hWndParent, MixerSetupProc);
    }
    else
    {
        MessageBox(hWnd, "No mixer hardware found", "DScaler Error", MB_OK);
    }
}


//----------------------------------------------------------------------
//  Internal functions
//----------------------------------------------------------------------

static void Mixer_SetCurrentMixerFromName()
{
    Mixer_SetCurrentMixer(Mixer_NameToIndex(g_pMixerName));
}


static void Mixer_SetCurrentMixer(long nMixerIndex)
{
    if (g_pCurrentMixer != NULL)
    {
        if (g_pCurrentMixer->GetIndex() == nMixerIndex)
        {
            return;
        }

        delete g_pCurrentMixer;
        g_pCurrentMixer = NULL;
    }

    g_pCurrentMixer = Mixer_Allocate(nMixerIndex);
}


static CMixerLineDst* Mixer_GetCurrentDestination()
{
    if (g_bUseMixer && g_pCurrentMixer != NULL && g_nDestinationIndex != -1)
    {
        return g_pCurrentMixer->GetDestinationLine(g_nDestinationIndex);
    }

    return NULL;
}


static CMixerLineSrc* Mixer_GetCurrentActiveSource()
{
    if (g_nActiveInput == -1 || g_nSourceIndexes[g_nActiveInput] == -1)
    {
        return NULL;
    }

    CMixerLineDst* pLineDst = Mixer_GetCurrentDestination();

    if (pLineDst != NULL)
    {
        return pLineDst->GetSourceLine(g_nSourceIndexes[g_nActiveInput]);
    }

    return NULL;
}


static void Mixer_EventOnChangeNotification(void*, CEventObject*, eEventType EventType,
                                            long OldValue, long NewValue, eEventType*)
{
    switch (EventType)
    {
    case EVENT_SOURCE_CHANGE:
        Mixer_OnSourceChange((CSource*)NewValue);
        break;

    case EVENT_VIDEOINPUT_CHANGE:
        Mixer_OnInputChange(NewValue);
        break;

    default:
        break;
    }
}


static void Mixer_OnInputChange(long nVideoInput)
{
    if (nVideoInput < 0 || nVideoInput >= g_nActiveInputsCount)
    {
        g_nActiveInput = -1;
    }
    else
    {
        g_nActiveInput = nVideoInput;
    }

    CMixerLineDst* pLineDst = Mixer_GetCurrentDestination();

    if (pLineDst == NULL)
    {
        return;
    }

    long nActiveSourceIndex = -1;

    if (g_nActiveInput != -1)
    {
        nActiveSourceIndex = g_nSourceIndexes[g_nActiveInput];
    }

    long nUniqueSources[MAX_SUPPORTED_INPUTS];
    long nUniqueSourcesCount;

    nUniqueSourcesCount = LongArrayUnique(g_nSourceIndexes, g_nActiveInputsCount, nUniqueSources);
    LongArraySubstract(nUniqueSources, &nUniqueSourcesCount, &nActiveSourceIndex, 1);

    // Mute mixer lines for inactive lines
    Mixer_MuteInputs(g_nDestinationIndex, nUniqueSources, nUniqueSourcesCount, TRUE);

    if (nActiveSourceIndex != -1)
    {
        CMixerLineSrc* pLineSrc = pLineDst->GetSourceLine(nActiveSourceIndex);

        if (pLineSrc != NULL)
        {
            pLineSrc->SetMute(Audio_IsMute());

            // We do not store volume values
            // pLineSrc->SetVolume(Volume);

            EventCollector->RaiseEvent(NULL, EVENT_MIXERVOLUME, -1, pLineSrc->GetVolume());
        }
    }
}


static void Mixer_OnSourceChange(CSource *pSource)
{
    Mixer_DoSettingsTransition(LoadSourceSettingsCallback, pSource);
}


static CMixer* LoadSourceSettingsCallback(void* pContext)
{
    CSource* pSource = (CSource*)pContext;

    // DoSettingsTranstion wants everything changed to the new
    // settings, except g_pCurrentMixer.  Instead of changing
    // g_pCurrentMixer, the new mixer should be returned.

    // Load the new source's settings
    MixerDev_SettingSetSection(pSource);
    _MixerDev_ReadSettingsFromIni();

    Mixer_SetupActiveInputs(pSource);

    if (g_bUseMixer)
    {
        long nMixerIndex = Mixer_NameToIndex(g_pMixerName);

        if (g_pCurrentMixer != NULL &&
            g_pCurrentMixer->GetIndex() == nMixerIndex)
        {
            return g_pCurrentMixer;
        }

        return Mixer_Allocate(nMixerIndex);
    }

    return NULL;
}


static void Mixer_SetupActiveInputs(CSource* pCurrentSource)
{
    if (pCurrentSource != NULL)
    {
        g_nActiveInputsCount = pCurrentSource->NumInputs(VIDEOINPUT);
        g_nActiveInput = pCurrentSource->GetInput(VIDEOINPUT);

        if (g_nActiveInputsCount > MAX_SUPPORTED_INPUTS)
        {
            g_nActiveInputsCount = MAX_SUPPORTED_INPUTS;
        }

        if (g_nActiveInput < -1 || g_nActiveInput >= g_nActiveInputsCount)
        {
            g_nActiveInput = -1;
        }
    }
    else
    {
        g_nActiveInputsCount = 0;
        g_nActiveInput = -1;
    }
}


static void Mixer_MuteInputs(long nDestinationIndex, long* pIndexes, long nCount, BOOL bMute)
{
    CMixerLineDst* pLineDst;
    CMixerLineSrc* pLineSrc;

    if (g_pCurrentMixer == NULL || nDestinationIndex == -1 || nCount == 0)
    {
        return;
    }

    if ((pLineDst = g_pCurrentMixer->GetDestinationLine(nDestinationIndex)) == NULL)
    {
        return;
    }

    for (int i = 0; i < nCount; i++)
    {
        if (pIndexes[i] == -1)
        {
            continue;
        }

        if ((pLineSrc = pLineDst->GetSourceLine(pIndexes[i])) != NULL)
        {
            pLineSrc->SetMute(bMute);
        }
    }
}


static void Mixer_StoreRestoreInputs(long nDestinationIndex, long* pIndexes, long nCount, BOOL bRestore)
{
    CMixerLineDst* pLineDst;
    CMixerLineSrc* pLineSrc;

    if (g_pCurrentMixer == NULL || nDestinationIndex == -1 || nCount == 0)
    {
        return;
    }

    if ((pLineDst = g_pCurrentMixer->GetDestinationLine(nDestinationIndex)) == NULL)
    {
        return;
    }

    for (int i = 0; i < nCount; i++)
    {
        if (pIndexes[i] == -1)
        {
            continue;
        }

        if ((pLineSrc = pLineDst->GetSourceLine(pIndexes[i])) != NULL)
        {
            if (bRestore)
            {
                pLineSrc->RestoreState();
            }
            else
            {
                pLineSrc->StoreState();
            }
        }
    }
}


static void Mixer_DoSettingsTransition(tSyncChangesCallback* pSyncfunc, void* pContext)
{
    long nOldDestination = -1;
    long nOldSources[MAX_SUPPORTED_INPUTS];
    long nOldSourcesCount = 0;
    long nNewSources[MAX_SUPPORTED_INPUTS];
    long nNewSourcesCount = 0;

    // Hold onto the old mixer info
    if (g_bUseMixer && g_pCurrentMixer != NULL && g_nDestinationIndex != -1)
    {
        nOldDestination = g_nDestinationIndex;
        nOldSourcesCount = LongArrayUnique(g_nSourceIndexes, g_nActiveInputsCount, nOldSources);
    }

    // Synchronize all new settings except g_pCurrentMixer
    CMixer* pNewMixer = (pSyncfunc)(pContext);

    // Check the new mixer info
    if (g_bUseMixer && pNewMixer != NULL && g_nDestinationIndex != -1)
    {
        nNewSourcesCount = LongArrayUnique(g_nSourceIndexes, g_nActiveInputsCount, nNewSources);

        // Compare the old with the new mixer info
        if (pNewMixer == g_pCurrentMixer && g_nDestinationIndex == nOldDestination)
        {
            LongArrayDivide(nOldSources, &nOldSourcesCount, nNewSources, &nNewSourcesCount);
        }
    }

    // Reset the no longer used inputs
    if (g_bResetOnExit)
    {
        Mixer_StoreRestoreInputs(nOldDestination, nOldSources, nOldSourcesCount, TRUE);
    }
    else
    {
        Mixer_MuteInputs(nOldDestination, nOldSources, nOldSourcesCount, TRUE);
    }

    // Synchronize g_pCurrentMixer
    if (g_pCurrentMixer != pNewMixer)
    {
        if (g_pCurrentMixer != NULL)
        {
            delete g_pCurrentMixer;
        }

        g_pCurrentMixer = pNewMixer;
    }

    // Setup the new inputs
    if (g_bUseMixer)
    {
        Mixer_StoreRestoreInputs(g_nDestinationIndex, nNewSources, nNewSourcesCount, FALSE);
        Mixer_OnInputChange(g_nActiveInput);
    }
}


static long Mixer_NameToIndex(char* szName)
{
    long nMixerIndex = -1;

    if (szName != NULL)
    {
        CMixerFinder mixerFinder;
        nMixerIndex = mixerFinder.FindMixer(szName);
    }

    return nMixerIndex;
}


static CMixer* Mixer_Allocate(long nMixerIndex)
{
    CMixer* pMixer = NULL;

    if (nMixerIndex != -1)
    {
        try
        {
            pMixer = new CMixer(nMixerIndex);
        }
        catch (...)
        {
            // do nothing
        }
    }

    return pMixer;
}


//----------------------------------------------------------------------
//  APIENTRY MixerSetupProc
//----------------------------------------------------------------------

static void InitAndNameActiveInputs(HWND hDlg);

static void AdjustSameForAllInputsBox(HWND hDlg);
static void SwitchSameForAllInputsMode(HWND hDlg, int iOneSameZeroNot);
static void EnableDisableMixerControls(HWND hDlg, BOOL bEnable);

static void RefillMixerDeviceBox(HWND hDlg, long nSelectIndex);
static void RefillDestinationBox(HWND hDlg, long nSelectIndex);
static void RefillSourceBox(HWND hDlg, long nSourceControlId, long nSelectIndex);

static CMixer* SynchronizeDlgChangesCallback(void* pContext);
static int ComboBox_GetCurSelItemData(HWND hControl);

static CMixer*  g_pDlgActiveMixer = NULL;


BOOL APIENTRY MixerSetupProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    int i;

    switch (message)
    {
    case WM_INITDIALOG:
        InitAndNameActiveInputs(hDlg);

        // Adjust the check boxes
        Button_SetCheck(GetDlgItem(hDlg, IDC_USE_MIXER), g_bUseMixer);
        Button_SetCheck(GetDlgItem(hDlg, IDC_DISABLE_HW_MUTE), g_bNoHardwareMute);
        Button_SetCheck(GetDlgItem(hDlg, IDC_RESETONEXIT), g_bResetOnExit);

        if (!g_bUseMixer)
        {
            // Create a temporally mixer as per user settings
            Mixer_SetCurrentMixerFromName();
        }

        if (g_pCurrentMixer != NULL)
        {
            RefillMixerDeviceBox(hDlg, g_pCurrentMixer->GetIndex());
        }
        else
        {
            RefillMixerDeviceBox(hDlg, -1);
        }

        MixerSetupProc(hDlg, WM_COMMAND, MAKEWPARAM(IDC_MIXER, CBN_SELCHANGE), 0L);

        AdjustSameForAllInputsBox(hDlg);

        // Enable all the fields that should be enabled
        EnableDisableMixerControls(hDlg, g_bUseMixer);

        SetFocus(GetDlgItem(hDlg, IDC_USE_MIXER));

        return FALSE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_USE_MIXER:
            if (Button_GetCheck(GetDlgItem(hDlg, IDC_USE_MIXER)) == BST_CHECKED)
            {
                EnableDisableMixerControls(hDlg, TRUE);
            }
            else
            {
                EnableDisableMixerControls(hDlg, FALSE);
            }
            break;

        case IDC_MIXER:
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                int nIndex = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_MIXER));

                // Delete of the old mixer
                if (g_pDlgActiveMixer != NULL)
                {
                    if (g_pDlgActiveMixer->GetIndex() == nIndex)
                    {
                        // Nothing to do
                        break;
                    }

                    // Make sure it's our own
                    if (g_pDlgActiveMixer != g_pCurrentMixer)
                    {
                        delete g_pDlgActiveMixer;
                    }

                    g_pDlgActiveMixer = NULL;
                }

                // Create the new mixer
                if (nIndex != -1)
                {
                    if (g_pCurrentMixer != NULL && g_pCurrentMixer->GetIndex() == nIndex)
                    {
                        // Just reference the current mixer
                        g_pDlgActiveMixer = g_pCurrentMixer;
                    }
                    else
                    {
                        try
                        {
                            g_pDlgActiveMixer = new CMixer(nIndex);
                        }
                        catch (...)
                        {
                            nIndex = -1;
                        }
                    }
                }

                // Update the destination box
                if (nIndex != -1)
                {
                    if (g_pDlgActiveMixer == g_pCurrentMixer)
                    {
                        RefillDestinationBox(hDlg, g_nDestinationIndex);
                    }
                    else
                    {
                        RefillDestinationBox(hDlg, -1);
                    }
                }
                else
                {
                    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_DEST));
                }

                MixerSetupProc(hDlg, WM_COMMAND, MAKEWPARAM(IDC_DEST, CBN_SELCHANGE), 0);
            }
            break;

        case IDC_DEST:
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                int nIndex = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_DEST));

                // Update the source boxes
                if (nIndex != -1 && g_pDlgActiveMixer != NULL)
                {
                    if (g_pDlgActiveMixer == g_pCurrentMixer && nIndex == g_nDestinationIndex)
                    {
                        for (i = 0; i < g_nActiveInputsCount; i++)
                        {
                            RefillSourceBox(hDlg, IDC_MIXER_INPUT0+i, g_nSourceIndexes[i]);
                        }
                    }
                    else
                    {
                        for (i = 0; i < g_nActiveInputsCount; i++)
                        {
                            RefillSourceBox(hDlg, IDC_MIXER_INPUT0+i, -1);
                        }
                    }
                }
                else
                {
                    for (i = 0; i < g_nActiveInputsCount; i++)
                    {
                        ComboBox_ResetContent(GetDlgItem(hDlg, IDC_MIXER_INPUT0+i));
                    }
                }
            }
            break;

        case IDC_MIXER_INPUT_ALL:
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                int index = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_MIXER_INPUT_ALL));

                for (i = 0; i < g_nActiveInputsCount; i++)
                {
                    RefillSourceBox(hDlg, IDC_MIXER_INPUT0+i, index);
                }
            }
            break;

        case IDC_SEPARATE_INPUT:
            SwitchSameForAllInputsMode(hDlg,
                Button_GetCheck(GetDlgItem(hDlg, IDC_SEPARATE_INPUT)) != BST_CHECKED);
            break;

        case IDOK:
            // Use the new g_bResetOnExit setting in Mixer_DoSettingsTransition()
            g_bResetOnExit =
                Button_GetCheck(GetDlgItem(hDlg, IDC_RESETONEXIT)) == BST_CHECKED;

            Mixer_DoSettingsTransition(SynchronizeDlgChangesCallback, hDlg);
            WriteSettingsToIni(TRUE);

            // FALLTHROUGH

        case IDCANCEL:
            if (g_pDlgActiveMixer != NULL)
            {
                if (g_pDlgActiveMixer != g_pCurrentMixer)
                {
                    delete g_pDlgActiveMixer;
                }
                g_pDlgActiveMixer = NULL;
            }

            if (!g_bUseMixer)
            {
                Mixer_SetCurrentMixer(-1);
            }

            EndDialog(hDlg, 0);
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }

    return FALSE;
}


static void InitAndNameActiveInputs(HWND hDlg)
{
    int     nInputCount = 0;
    string  sInputName;
    CSource* pSource;
    int     i;

    pSource = Providers_GetCurrentSource();

    if (pSource != NULL)
    {
        nInputCount = g_nActiveInputsCount;
    }

    // Set the source line names
    for (i = 0; i < nInputCount; i++)
    {
        const char* pName = pSource->GetInputName(VIDEOINPUT, i);
        if (pName == NULL)
        {
            sInputName = "Input ";
            sInputName += i;
        }
        else
        {
            sInputName = pName;
        }

        SetDlgItemText(hDlg, IDC_MIXER_INPUT0NAME+i, sInputName.c_str());

        ShowWindow(GetDlgItem(hDlg, IDC_MIXER_INPUT0NAME+i), SW_SHOW);
        ShowWindow(GetDlgItem(hDlg, IDC_MIXER_INPUT0+i), SW_SHOW);

        LONG lStyle = GetWindowLong(GetDlgItem(hDlg, IDC_MIXER_INPUT0NAME+i), GWL_STYLE);

        // There are better ways of testing if the
        // text wraps but this way is probably okay
        if (sInputName.length() > 16)
        {
            // Unset center vertically so the text
            // wraps properly
            lStyle &= ~SS_CENTERIMAGE;
        }
        else
        {
            lStyle |= SS_CENTERIMAGE;
        }

        SetWindowLong(GetDlgItem(hDlg, IDC_MIXER_INPUT0NAME+i), GWL_STYLE, lStyle);
    }

    // Hide the unused source lines
    for ( ; i < MAX_SUPPORTED_INPUTS; i++)
    {
        ShowWindow(GetDlgItem(hDlg, IDC_MIXER_INPUT0NAME+i), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_MIXER_INPUT0+i), SW_HIDE);
    }

    if (nInputCount == 0)
    {
        ShowWindow(GetDlgItem(hDlg, IDC_MIXER_INPUTNAME_ALL), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_MIXER_INPUT_ALL), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_SEPARATE_INPUT), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_SOURCE_NO_INPUT), SW_SHOW);
    }
    else
    {
        ShowWindow(GetDlgItem(hDlg, IDC_MIXER_INPUTNAME_ALL), SW_SHOW);
        ShowWindow(GetDlgItem(hDlg, IDC_MIXER_INPUT_ALL), SW_SHOW);
        ShowWindow(GetDlgItem(hDlg, IDC_SEPARATE_INPUT), SW_SHOW);
        ShowWindow(GetDlgItem(hDlg, IDC_SOURCE_NO_INPUT), SW_HIDE);
    }
}


static void AdjustSameForAllInputsBox(HWND hDlg)
{
    int nFirstIndex;
    int nIndex;
    int i;

    if (g_nActiveInputsCount == 0)
    {
        return;
    }

    nFirstIndex = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_MIXER_INPUT0));

    for (i = 1; i < g_nActiveInputsCount; i++)
    {
        nIndex = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_MIXER_INPUT0+i));

        if (nIndex != nFirstIndex)
        {
            break;
        }
    }

    if (i == g_nActiveInputsCount)
    {
        if (nFirstIndex == -1)
        {
            nFirstIndex = -1;
        }

        RefillSourceBox(hDlg, IDC_MIXER_INPUT_ALL, nFirstIndex);
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEPARATE_INPUT), FALSE);
    }
    else
    {
        RefillSourceBox(hDlg, IDC_MIXER_INPUT_ALL, -1);
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEPARATE_INPUT), TRUE);
    }
}


static void SwitchSameForAllInputsMode(HWND hDlg, int iOneSameZeroNot)
{
    EnableWindow(GetDlgItem(hDlg, IDC_MIXER_INPUT_ALL), iOneSameZeroNot == 1);
    EnableWindow(GetDlgItem(hDlg, IDC_MIXER_INPUTNAME_ALL), iOneSameZeroNot == 1);

    for (int i = 0; i < g_nActiveInputsCount; i++)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_MIXER_INPUT0+i), iOneSameZeroNot == 0);
        EnableWindow(GetDlgItem(hDlg, IDC_MIXER_INPUT0NAME+i), iOneSameZeroNot == 0);
    }
}


static void EnableDisableMixerControls(HWND hDlg, BOOL bEnable)
{
    EnableWindow(GetDlgItem(hDlg, IDC_MIXER), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_DEST), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_DEST_STATIC), bEnable);

    EnableWindow(GetDlgItem(hDlg, IDC_SEPARATE_INPUT), bEnable);

    SwitchSameForAllInputsMode(hDlg, bEnable == FALSE ? -1 :
        Button_GetCheck(GetDlgItem(hDlg, IDC_SEPARATE_INPUT)) != BST_CHECKED);

    EnableWindow(GetDlgItem(hDlg, IDC_DISABLE_HW_MUTE), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_RESETONEXIT), bEnable);
}


static void RefillMixerDeviceBox(HWND hDlg, long nSelectIndex)
{
    CMixerFinder mixerFinder;
    char buffer[MAXPNAMELEN];

    HWND hMixerControl = GetDlgItem(hDlg, IDC_MIXER);

    ComboBox_ResetContent(hMixerControl);

    int mixerCount = mixerFinder.GetMixerCount();
    BOOL bSelected = FALSE;
    int index;

    for (int i = 0; i < mixerCount; i++)
    {
        mixerFinder.GetMixerName(i, buffer);

        index = ComboBox_AddString(hMixerControl, buffer);
        ComboBox_SetItemData(hMixerControl, index, i);

        if (i == nSelectIndex)
        {
            ComboBox_SetCurSel(hMixerControl, index);
            bSelected = TRUE;
        }
    }

    if (!bSelected)
    {
        ComboBox_SetCurSel(hMixerControl, 0);
    }
}


static void RefillDestinationBox(HWND hDlg, long nSelectIndex)
{
    HWND hDestinationControl = GetDlgItem(hDlg, IDC_DEST);

    ComboBox_ResetContent(hDestinationControl);

    if (g_pDlgActiveMixer == NULL)
    {
        return;
    }

    int destinationCount = g_pDlgActiveMixer->GetDestinationCount();
    int recordingLines = 0;

    const char* pValidName = "";
    int bSelected = FALSE;
    int index;

    for (int i = 0; i < destinationCount; i++)
    {
        CMixerLineDst* pLineDst = g_pDlgActiveMixer->GetDestinationLine(i);

        if (pLineDst->IsTypicalRecordingLine() &&
            destinationCount - recordingLines > 1)
        {
            recordingLines++;
        }
        else
        {
            pValidName = pLineDst->GetName();

            index = ComboBox_AddString(hDestinationControl, pValidName);
            ComboBox_SetItemData(hDestinationControl, index, i);

            if (i == nSelectIndex)
            {
                ComboBox_SetCurSel(hDestinationControl, index);
                bSelected = TRUE;
            }
        }
    }

    if (!bSelected)
    {
        ComboBox_SetCurSel(hDestinationControl, 0);
    }

    // Hide the destination field if there is only one destination
    if(destinationCount - recordingLines == 1)
    {
        SetDlgItemText(hDlg, IDC_DEST_STATIC, pValidName);
        ShowWindow(hDestinationControl, SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_DEST_STATIC), SW_SHOW);
    }
    else
    {
        ShowWindow(GetDlgItem(hDlg, IDC_DEST_STATIC), SW_HIDE);
        ShowWindow(hDestinationControl, SW_SHOW);
    }
}


static void RefillSourceBox(HWND hDlg, long nSourceControlId, long nSelectIndex)
{
    HWND hSourceControl = GetDlgItem(hDlg, nSourceControlId);

    ComboBox_ResetContent(hSourceControl);

    if (g_pDlgActiveMixer == NULL)
    {
        return;
    }

    int nDestinationIndex = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_DEST));

    if (nDestinationIndex != -1)
    {
        int sourceCount = 0;
        int index;

        index = ComboBox_AddString(hSourceControl, "<None>");
        ComboBox_SetItemData(hSourceControl, index, -1);

        ComboBox_SetCurSel(hSourceControl, index);

        CMixerLineDst* pLineDst = g_pDlgActiveMixer->GetDestinationLine(nDestinationIndex);

        if (pLineDst != NULL)
        {
            sourceCount = pLineDst->GetSourceCount();
        }

        for (int i = 0; i < sourceCount; i++)
        {
            const char* pName = pLineDst->GetSourceLine(i)->GetName();

            if (pName == NULL || strcmp(pName, "Error") == 0)
            {
                continue;
            }

            index = ComboBox_AddString(hSourceControl, pName);
            ComboBox_SetItemData(hSourceControl, index, i);

            if (i == nSelectIndex)
            {
                ComboBox_SetCurSel(hSourceControl, index);
            }
        }
    }
}


static CMixer* SynchronizeDlgChangesCallback(void* pContext)
{
    HWND hDlg = (HWND)pContext;

    // DoSettingsTranstion wants everything changed to the new
    // settings, except g_pCurrentMixer.  Instead of changing
    // g_pCurrentMixer, the new mixer should be returned.

    extern SETTING MixerDevSettings[MIXERDEV_SETTING_LASTONE];

    if (g_pDlgActiveMixer != NULL)
    {
        Setting_SetValue(&MixerDevSettings[MIXERNAME], (long)g_pDlgActiveMixer->GetName(), FALSE);
        g_nDestinationIndex = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_DEST));
    }
    else
    {
        Setting_SetValue(&MixerDevSettings[MIXERNAME], (long)(const char*)"", FALSE);
        g_nDestinationIndex = -1;
    }

    int i = 0;

    // Read the new settings
    if (g_nDestinationIndex != -1)
    {
        if (Button_GetCheck(GetDlgItem(hDlg, IDC_SEPARATE_INPUT)) != BST_CHECKED)
        {
            int nIndex = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_MIXER_INPUT_ALL));

            for ( ; i < g_nActiveInputsCount; i++)
            {
                g_nSourceIndexes[i] = nIndex;
            }
        }
        else
        {
            for ( ; i < g_nActiveInputsCount; i++)
            {
                g_nSourceIndexes[i] = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_MIXER_INPUT0+i));
            }
        }
    }

    for ( ; i < MAX_SUPPORTED_INPUTS; i++)
    {
        g_nSourceIndexes[i] = -1;
    }

    g_bUseMixer = (Button_GetCheck(GetDlgItem(hDlg, IDC_USE_MIXER)) == BST_CHECKED);
    g_bNoHardwareMute = (Button_GetCheck(GetDlgItem(hDlg, IDC_DISABLE_HW_MUTE)) == BST_CHECKED);

    return g_pDlgActiveMixer;
}


static int ComboBox_GetCurSelItemData(HWND hControl)
{
    int nIndex = ComboBox_GetCurSel(hControl);

    if (nIndex != CB_ERR)
    {
        return ComboBox_GetItemData(hControl, nIndex);
    }

    return -1;
}


//----------------------------------------------------------------------
//  Long array utilities
//----------------------------------------------------------------------

static long LongArrayUnique(const long* source, long size, long* dest)
{
    long resultCount = 0;
    int i, j;

    if (size > 0)
    {
        dest[resultCount++] = source[0];

        for (i = 1; i < size; i++)
        {
            for (j = 0; j < resultCount; j++)
            {
                if (dest[j] == source[i])
                {
                    break;
                }
            }

            if (j == resultCount)
            {
                dest[resultCount++] = source[i];
            }
        }
    }

    return resultCount;
}


static void LongArraySubstract(long* values, long* valuesSize,
                               const long* subtract, long substractSize)
{
    int i, j;

    for (i = 0; i < substractSize; i++)
    {
        for (j = 0; j < *valuesSize; j++)
        {
            if (values[j] == subtract[i])
            {
                break;
            }
        }

        if (j != *valuesSize)
        {
            if (--(*valuesSize) != j)
            {
                values[j] = values[*valuesSize];
            }
        }
    }
}


static void LongArrayDivide(long* dividend, long *dividendSize,
                            long* divisor, long *divisorSize)
{
    long resultCount = 0;
    int i, j;

    for (i = 0; i < *dividendSize; i++)
    {
        for (j = 0; j < *divisorSize; j++)
        {
            if (divisor[j] == dividend[i])
            {
                break;
            }
        }

        if (j == *divisorSize)
        {
            dividend[resultCount++] = dividend[i];
        }
        else
        {
            if (--(*divisorSize) != j)
            {
                divisor[j] = divisor[*divisorSize];
            }
        }
    }

    *dividendSize = resultCount;
}


//----------------------------------------------------------------------
//  CMixerLineSrc
//----------------------------------------------------------------------

CMixerLineSrc::CMixerLineSrc(HMIXER hMixer, DWORD nDstIndex, DWORD nSrcIndex)
{
    MMRESULT mmresult;

    m_hMixer = hMixer;
    m_VolumeControlID = 0xFFFFFFFF;
    m_MuteControlID = 0xFFFFFFFF;

    ZeroMemory(&m_mxl, sizeof(MIXERLINE));

    m_mxl.cbStruct      = sizeof(MIXERLINE);
    m_mxl.dwDestination = nDstIndex;
    m_mxl.dwSource      = nSrcIndex;

    mmresult = mixerGetLineInfo((HMIXEROBJ)hMixer, &m_mxl, MIXER_GETLINEINFOF_SOURCE);
    if (mmresult != MMSYSERR_NOERROR || m_mxl.cControls == 0)
    {
        strcpy(m_mxl.szName, "Error");
        return;
    }

    MIXERLINECONTROLS mxlc;
    LPMIXERCONTROL pmxctrl;

    pmxctrl = (MIXERCONTROL*)malloc(sizeof(MIXERCONTROL) * m_mxl.cControls);

    mxlc.cbStruct   = sizeof(MIXERLINECONTROLS);
    mxlc.dwLineID   = m_mxl.dwLineID;
    mxlc.cControls  = m_mxl.cControls;
    mxlc.cbmxctrl   = sizeof(MIXERCONTROL);
    mxlc.pamxctrl   = pmxctrl;

    mmresult = mixerGetLineControls((HMIXEROBJ)hMixer, &mxlc, MIXER_GETLINECONTROLSF_ALL);

    for (int i = 0; i < m_mxl.cControls; i++)
    {
        if (pmxctrl[i].dwControlType == MIXERCONTROL_CONTROLTYPE_VOLUME)
        {
            m_VolumeControlID = pmxctrl[i].dwControlID;
            m_VolumeMinimum = pmxctrl[i].Bounds.dwMinimum;
            m_VolumeMaximum = pmxctrl[i].Bounds.dwMaximum;

        }
        else if (pmxctrl[i].dwControlType == MIXERCONTROL_CONTROLTYPE_MUTE)
        {
            m_MuteControlID = pmxctrl[i].dwControlID;
        }
    }

    free(pmxctrl);
}


CMixerLineSrc::~CMixerLineSrc()
{
}


const char* CMixerLineSrc::GetName()
{
    return m_mxl.szName;
}


void CMixerLineSrc::SetMute(BOOL bEnable)
{
    if (m_MuteControlID != 0xFFFFFFFF)
    {
        MixerControlDetailsSet(m_MuteControlID, bEnable);
    }
}


BOOL CMixerLineSrc::GetMute()
{
    DWORD dwEnabled = FALSE;

    if (m_MuteControlID != 0xFFFFFFFF)
    {
        MixerControlDetailsGet(m_MuteControlID, &dwEnabled);
    }

    return dwEnabled;
}


void CMixerLineSrc::SetVolume(int volumePercentage)
{
    if (m_VolumeControlID != 0xFFFFFFFF)
    {
        // sets all channels to the same volume
        MixerControlDetailsSet(m_VolumeControlID,
            MulDiv(volumePercentage, (m_VolumeMaximum - m_VolumeMinimum), 100));
    }
}


int CMixerLineSrc::GetVolume()
{
    DWORD nVolume = 0;

    if (m_VolumeControlID != 0xFFFFFFFF)
    {
        // get average volume, ignores balance
        if (MixerControlDetailsGet(m_VolumeControlID, &nVolume))
        {
            return MulDiv(nVolume, 100, m_VolumeMaximum - m_VolumeMinimum);
        }
    }

    return nVolume;
}


void CMixerLineSrc::StoreState()
{
    MixerControlDetailsGet(m_MuteControlID, &m_StoredMute);
    MixerControlDetailsGet(m_VolumeControlID, &m_StoredVolume);
}


void CMixerLineSrc::RestoreState()
{
    MixerControlDetailsSet(m_MuteControlID, m_StoredMute);
    MixerControlDetailsSet(m_VolumeControlID, m_StoredVolume);
}


BOOL CMixerLineSrc::MixerControlDetailsSet(DWORD dwControlID, DWORD dwValue)
{
    MIXERCONTROLDETAILS mxcd;

    mxcd.cbStruct       = sizeof(MIXERCONTROLDETAILS);
    mxcd.cChannels      = 1;
    mxcd.dwControlID    = dwControlID;
    mxcd.cMultipleItems = 0;
    mxcd.cbDetails      = sizeof(DWORD);
    mxcd.paDetails      = &dwValue;

    return mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mxcd,
        MIXER_SETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_HMIXER) == MMSYSERR_NOERROR;
}


BOOL CMixerLineSrc::MixerControlDetailsGet(DWORD dwControlID, LPDWORD lpdwValue)
{
    MIXERCONTROLDETAILS mxcd;

    mxcd.cbStruct       = sizeof(MIXERCONTROLDETAILS);
    mxcd.cChannels      = 1;
    mxcd.dwControlID    = dwControlID;
    mxcd.cMultipleItems = 0;
    mxcd.cbDetails      = sizeof(DWORD);
    mxcd.paDetails      = lpdwValue;

    return mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mxcd,
        MIXER_SETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_HMIXER) == MMSYSERR_NOERROR;
}


//----------------------------------------------------------------------
//  CMixerLineDst
//----------------------------------------------------------------------

CMixerLineDst::CMixerLineDst(HMIXER hMixer, DWORD nDstIndex)
{
    ZeroMemory(&m_mxl, sizeof(MIXERLINE));

    m_mxl.cbStruct = sizeof(MIXERLINE);
    m_mxl.dwDestination = nDstIndex;

    m_nSourceCount = 0;
    m_pSourceLines = NULL;

    MMRESULT mmresult = mixerGetLineInfo((HMIXEROBJ)hMixer, &m_mxl,
        MIXER_GETLINEINFOF_DESTINATION);

    if (mmresult != MMSYSERR_NOERROR || m_mxl.cConnections == 0)
    {
        strcpy(m_mxl.szName, "Error");
        return;
    }

    m_nSourceCount = m_mxl.cConnections;
    m_pSourceLines = new CMixerLineSrc*[m_nSourceCount];

    for (int i = 0; i < m_nSourceCount; i++)
    {
        m_pSourceLines[i] = new CMixerLineSrc(hMixer, nDstIndex, i);
    }
}


CMixerLineDst::~CMixerLineDst()
{
    if (m_pSourceLines != NULL)
    {
        for (int i = 0; i < m_nSourceCount; i++)
        {
            delete m_pSourceLines[i];
        }
        delete [] m_pSourceLines;
    }
}


const char* CMixerLineDst::GetName()
{
    return m_mxl.szName;
}


long CMixerLineDst::GetSourceCount()
{
    return m_nSourceCount;
}


CMixerLineSrc* CMixerLineDst::GetSourceLine(DWORD nIndex)
{
    if (nIndex >= m_nSourceCount)
    {
        return NULL;
    }

    return m_pSourceLines[nIndex];
}


void CMixerLineDst::StoreState()
{
    for (int i = 0; i < m_nSourceCount; i++)
    {
        m_pSourceLines[i]->StoreState();
    }
}


void CMixerLineDst::RestoreState()
{
    for (int i = 0; i < m_nSourceCount; i++)
    {
        m_pSourceLines[i]->RestoreState();
    }
}


BOOL CMixerLineDst::IsTypicalSpeakerLine()
{
    return (m_mxl.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_SPEAKERS);
}


BOOL CMixerLineDst::IsTypicalRecordingLine()
{
    return (m_mxl.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_WAVEIN);
}


//----------------------------------------------------------------------
//  CMixer
//----------------------------------------------------------------------

CMixer::CMixer(DWORD nMixerIndex)
{
    MMRESULT mmresult;

    m_hMixer = NULL;
    m_nDestinationCount = 0;
    m_pDestinationLines = NULL;

    m_nMixerIndex = nMixerIndex;

    mmresult = mixerGetDevCaps(m_nMixerIndex, &m_mxcaps, sizeof(MIXERCAPS));
    if (mmresult != MMSYSERR_NOERROR || m_mxcaps.cDestinations == 0)
    {
        delete this;
        throw NULL;
    }

    mmresult = mixerOpen(&m_hMixer, m_nMixerIndex, NULL, NULL, MIXER_OBJECTF_MIXER);
    if (mmresult != MMSYSERR_NOERROR)
    {
        delete this;
        throw NULL;
    }

    m_nDestinationCount = m_mxcaps.cDestinations;
    m_pDestinationLines = new CMixerLineDst*[m_nDestinationCount];

    for (int i = 0; i < m_nDestinationCount; i++)
    {
        m_pDestinationLines[i] = new CMixerLineDst(m_hMixer, i);
    }
}


CMixer::~CMixer()
{
    if (m_pDestinationLines != NULL)
    {
        for (int i = 0; i < m_nDestinationCount; i++)
        {
            delete m_pDestinationLines[i];
        }

        delete [] m_pDestinationLines;
    }

    if (m_hMixer != NULL)
    {
        mixerClose(m_hMixer);
    }
}


const char* CMixer::GetName()
{
    return m_mxcaps.szPname;
}


DWORD CMixer::GetIndex()
{
    return m_nMixerIndex;
}


long CMixer::GetDestinationCount()
{
    return m_nDestinationCount;
}


CMixerLineDst* CMixer::GetDestinationLine(DWORD nIndex)
{
    if (nIndex >= m_nDestinationCount)
    {
        return NULL;
    }

    return m_pDestinationLines[nIndex];
}


void CMixer::StoreState()
{
    for (int i = 0; i < m_nDestinationCount; i++)
    {
        m_pDestinationLines[i]->StoreState();
    }
}


void CMixer::RestoreState()
{
    for (int i = 0; i < m_nDestinationCount; i++)
    {
        m_pDestinationLines[i]->RestoreState();
    }
}


//----------------------------------------------------------------------
//  CMixerFinder
//----------------------------------------------------------------------

CMixerFinder::CMixerFinder()
{
    m_nMixerCount = mixerGetNumDevs();
}


CMixerFinder::~CMixerFinder()
{
}


long CMixerFinder::GetMixerCount()
{
    return m_nMixerCount;
}


BOOL CMixerFinder::GetMixerName(long nMixerIndex, char szName[MAXPNAMELEN])
{
    MIXERCAPS mxcaps;
    MMRESULT mmresult;

    ASSERT(nMixerIndex >= 0 && nMixerIndex < m_nMixerCount);

    mmresult = mixerGetDevCaps(nMixerIndex, &mxcaps, sizeof(MIXERCAPS));
    if (mmresult == MMSYSERR_NOERROR)
    {
        strcpy(szName, mxcaps.szPname);
        return TRUE;
    }
    return FALSE;
}


long CMixerFinder::FindMixer(const char* szName)
{
    char buffer[MAXPNAMELEN];

    for (int i = 0; i < m_nMixerCount; i++)
    {
        if (GetMixerName(i, buffer) && lstrcmp(buffer, szName) == 0)
        {
            return i;
        }
    }
    return -1;
}


//----------------------------------------------------------------------
//  MixerDev stuff
//----------------------------------------------------------------------

SETTING MixerDevSettings[MIXERDEV_SETTING_LASTONE] =
{
    {
        "Use Mixer", ONOFF, 0, (long*)&g_bUseMixer,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Mixer", "UseMixer", NULL,
    },
    {
        "DestIndex", SLIDER, 0, (long*)&g_nDestinationIndex,
        0, 0, 255, 1, 1,
        NULL,
        "Mixer", "DestIndex", NULL,
    },
    {
        "Input 1 Index", SLIDER, 0, (long*)&g_nSourceIndexes[0],
        -1, -1, 255, 1, 1,
        NULL,
        "Mixer", "Input1Index", NULL,
    },
    {
        "Input 2 Index", SLIDER, 0, (long*)&g_nSourceIndexes[1],
        -1, -1, 255, 1, 1,
        NULL,
        "Mixer", "Input2Index", NULL,
    },
    {
        "Input 3 Index", SLIDER, 0, (long*)&g_nSourceIndexes[2],
        -1, -1, 255, 1, 1,
        NULL,
        "Mixer", "Input3Index", NULL,
    },
    {
        "Input 4 Index", SLIDER, 0, (long*)&g_nSourceIndexes[3],
        -1, -1, 255, 1, 1,
        NULL,
        "Mixer", "Input4Index", NULL,
    },
    {
        "Reset Mixer on Exit", ONOFF, 0, (long*)&g_bResetOnExit,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Mixer", "ResetOnExit", NULL,
    },
    {
        "Input 5 Index", SLIDER, 0, (long*)&g_nSourceIndexes[4],
        -1, -1, 255, 1, 1,
        NULL,
        "Mixer", "Input5Index", NULL,
    },
    {
        "Input 6 Index", SLIDER, 0, (long*)&g_nSourceIndexes[5],
        -1, -1, 255, 1, 1,
        NULL,
        "Mixer", "Input6Index", NULL,
    },
    {
        "Mixer Name", CHARSTRING, 0, (long*)&g_pMixerName,
        (long)"", 0, 0, 0, 0,
        NULL,
        "Mixer", "MixerName", NULL,
    },
    {
        "No Hardware Mute", ONOFF, 0, (long*)&g_bNoHardwareMute,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Mixer", "NoHardwareMute", NULL,
    },
};


SETTING* MixerDev_GetSetting(MIXERDEV_SETTING nSetting)
{
    if (nSetting >= 0 && nSetting < MIXERDEV_SETTING_LASTONE)
    {
        return &(MixerDevSettings[nSetting]);
    }
    else
    {
        return NULL;
    }
}


void MixerDev_ReadSettingsFromIni()
{
    MixerDev_SettingSetSection(Providers_GetCurrentSource());
    _MixerDev_ReadSettingsFromIni();
}


void _MixerDev_ReadSettingsFromIni()
{
    if (g_bMixerDevInvalidSection)
    {
        MixerDev_FreeSettings();
        g_bUseMixer = FALSE;
        return;
    }

    for (int i = 0; i < MIXERDEV_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(MixerDevSettings[i]));
    }
}


void MixerDev_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    if (g_bMixerDevInvalidSection)
    {
        return;
    }

    for (int i = 0; i < MIXERDEV_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(MixerDevSettings[i]), bOptimizeFileAccess);
    }
}


void MixerDev_FreeSettings()
{
    for (int i = 0; i < MIXERDEV_SETTING_LASTONE; i++)
    {
        Setting_Free(&(MixerDevSettings[i]));
    }
}


void MixerDev_SettingSetSection(CSource* pSource)
{
    if (pSource == NULL)
    {
        g_bMixerDevInvalidSection = TRUE;
    }
    else
    {
        MixerDev_SettingSetSection(pSource->IDString());
    }
}


void MixerDev_SettingSetSection(LPCSTR szSource)
{
    g_MixerDev_Section = string("MixerInput_") + szSource;

    g_bMixerDevInvalidSection = FALSE;

    for (int i = 0; i < MIXERDEV_SETTING_LASTONE; i++)
    {
        Setting_SetSection(&(MixerDevSettings[i]), (char*)g_MixerDev_Section.c_str());
    }
}


void MixerDev_SetMenu(HMENU hMenu)
{

}

