/////////////////////////////////////////////////////////////////////////////
// $Id: MixerDev.cpp,v 1.34 2002-10-18 03:33:00 flibuste2 Exp $
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
#include "BT848Card.h"
#include "Providers.h"

CSoundSystem* pSoundSystem = NULL;

BOOL bUseMixer = FALSE;
BOOL bResetOnExit = TRUE;
long MixerIndex = 0;
char MixerName[MAXPNAMELEN] = {0};
long DestIndex = 0;
long InputIndexes[6] = {-1, -1, -1, -1, -1, -1,};
std::string MixerDev_Section;

void MixerDev_SettingSetSection(LPCSTR szSource);
void Mixer_OnInputChange(int);
extern SETTING MixerDevSettings[MIXERDEV_SETTING_LASTONE]; //defined below

void Mixer_OnInputChangeNotification(void *pThis, int PreChange, eSourceInputType InputType, int OldInput, int NewInput);

CMixerLineSource::CMixerLineSource(HMIXER hMixer, int DestId, int SourceId)
{
    MIXERLINECONTROLS Controls;
    MIXERCONTROL* MixerControls;

    m_VolumeControl = -1;
    m_MuteControl = -1;
    m_hMixer = hMixer;

    memset(&m_MixerLine, 0, sizeof(MIXERLINE));
    m_MixerLine.cbStruct = sizeof(MIXERLINE);
    m_MixerLine.dwDestination = DestId;
    m_MixerLine.dwSource = SourceId;
    MMRESULT mmresult = mixerGetLineInfo((HMIXEROBJ) hMixer, &m_MixerLine, MIXER_GETLINEINFOF_SOURCE);
    if(mmresult != MMSYSERR_NOERROR || m_MixerLine.cControls == 0)
    {
        strcpy(m_MixerLine.szName, "Error");

        return;
    }
    
    m_ControlsCount = m_MixerLine.cControls;

    MixerControls = (MIXERCONTROL*)calloc(sizeof(MIXERCONTROL), m_ControlsCount);
    memset(&Controls, 0, sizeof(Controls));
    Controls.cbStruct = sizeof(Controls);
    Controls.dwLineID = m_MixerLine.dwLineID;
    Controls.cControls = m_ControlsCount;
    Controls.cbmxctrl = sizeof(MIXERCONTROL);
    Controls.pamxctrl = MixerControls;
    mmresult = mixerGetLineControls((HMIXEROBJ) hMixer, &Controls, MIXER_GETLINECONTROLSF_ALL);

    for(int i = 0; i < m_ControlsCount; ++i)
    {
        if(MixerControls[i].dwControlType == MIXERCONTROL_CONTROLTYPE_VOLUME)
        {
            m_VolumeControl = MixerControls[i].dwControlID;
            m_VolumeMin = MixerControls[i].Bounds.dwMinimum;
            m_VolumeMax = MixerControls[i].Bounds.dwMaximum;
        }
        else if(MixerControls[i].dwControlType == MIXERCONTROL_CONTROLTYPE_MUTE)
        {
            m_MuteControl = MixerControls[i].dwControlID;
        }
    }
    free(MixerControls);
    m_InitialMute = GetMute();
    m_InitialVolume = GetVolume();
}

CMixerLineSource::~CMixerLineSource()
{
}

void CMixerLineSource::SetMute(BOOL Mute)
{
    if(m_MuteControl != -1)
    {
        MIXERCONTROLDETAILS_BOOLEAN Bool[1] = {Mute};
        MIXERCONTROLDETAILS ControlDetails;
        ControlDetails.cbStruct = sizeof(ControlDetails);
        ControlDetails.cChannels = 1;
        ControlDetails.dwControlID = m_MuteControl;
        ControlDetails.cMultipleItems = 0;
        ControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
        ControlDetails.paDetails = &Bool;

        MMRESULT mmr = mixerSetControlDetails((HMIXEROBJ) m_hMixer, &ControlDetails, MIXER_SETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_HMIXER);
        //MMSYSERR_INVALFLAG
    }
}

BOOL CMixerLineSource::GetMute()
{
    if(m_MuteControl != -1)
    {
        MIXERCONTROLDETAILS_BOOLEAN Bool[1] = {FALSE};
        MIXERCONTROLDETAILS ControlDetails;
        ControlDetails.cbStruct = sizeof(ControlDetails);
        ControlDetails.cChannels = 1;
        ControlDetails.dwControlID = m_MuteControl;
        ControlDetails.cMultipleItems = 0;
        ControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
        ControlDetails.paDetails = &Bool;

        MMRESULT mmr = mixerGetControlDetails((HMIXEROBJ) m_hMixer, &ControlDetails, MIXER_SETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_HMIXER);
        return Bool[0].fValue;
    }
    else
    {
        return FALSE;
    }
}

void CMixerLineSource::ResetToOriginal()
{
    SetMute(m_InitialMute);
    SetVolume(m_InitialVolume);
}


// sets all channels to the same volume
void CMixerLineSource::SetVolume(int PercentageVolume)
{
    if(m_VolumeControl != -1)
    {
        MIXERCONTROLDETAILS_UNSIGNED Vol[1] = {MulDiv(PercentageVolume, (m_VolumeMax - m_VolumeMin), 100)};
        MIXERCONTROLDETAILS ControlDetails;
        ControlDetails.cbStruct = sizeof(ControlDetails);
        ControlDetails.cChannels = 1;
        ControlDetails.dwControlID = m_VolumeControl;
        ControlDetails.cMultipleItems = 0;
        ControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
        ControlDetails.paDetails = &Vol;

        MMRESULT mmr = mixerSetControlDetails((HMIXEROBJ) m_hMixer, &ControlDetails, MIXER_SETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_HMIXER);
    }
}

// get average volume ignores balance
int CMixerLineSource::GetVolume()
{
    if(m_VolumeControl != -1)
    {
        MIXERCONTROLDETAILS_UNSIGNED Vol[1] = {0,};
        MIXERCONTROLDETAILS ControlDetails;
        ControlDetails.cbStruct = sizeof(ControlDetails);
        ControlDetails.cChannels = 1;
        ControlDetails.dwControlID = m_VolumeControl;
        ControlDetails.cMultipleItems = 0;
        ControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
        ControlDetails.paDetails = &Vol;

        MMRESULT mmr = mixerGetControlDetails((HMIXEROBJ) m_hMixer, &ControlDetails, MIXER_SETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_HMIXER);
        if(mmr == MMSYSERR_NOERROR)
        {
            return MulDiv(Vol[0].dwValue, 100, m_VolumeMax - m_VolumeMin);
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

char* CMixerLineSource::GetName()
{
    return m_MixerLine.szName;
}

CMixerLineDest::CMixerLineDest(HMIXER hMixer, int DestId)
{
    memset(&m_MixerLine, 0, sizeof(MIXERLINE));
    m_MixerLine.cbStruct = sizeof(MIXERLINE);
    m_MixerLine.dwDestination = DestId;
    m_SourceCount = 0;
    m_SourceLines = NULL;

    MMRESULT mmresult = mixerGetLineInfo((HMIXEROBJ)hMixer, &m_MixerLine, MIXER_GETLINEINFOF_DESTINATION);
    if(mmresult != MMSYSERR_NOERROR || m_MixerLine.cConnections == 0)
    {
        strcpy(m_MixerLine.szName, "Error");

        return;
    }
    m_SourceCount = m_MixerLine.cConnections;
    m_SourceLines = (CMixerLineSource**)calloc(sizeof(CMixerLineSource*), m_SourceCount);
    for(int i = 0; i < m_SourceCount; ++i)
    {
        m_SourceLines[i] = new CMixerLineSource(hMixer, DestId, i);
    }
}

CMixerLineDest::~CMixerLineDest()
{
    if(m_SourceLines != NULL)
    {
        for(int i = 0; i < m_SourceCount; ++i)
        {
            delete m_SourceLines[i];
        }
        free(m_SourceLines);
    }
}

int CMixerLineDest::GetNumSourceLines()
{
    return m_SourceCount;
}

CMixerLineSource* CMixerLineDest::GetSourceLine(int LineIndex)
{
    if(LineIndex >= 0 && LineIndex <= m_SourceCount)
    {
        return m_SourceLines[LineIndex];
    }
    else
    {
        return NULL;
    }
}

char* CMixerLineDest::GetName()
{
    return m_MixerLine.szName;
}

void CMixerLineDest::ResetToOriginal()
{
    if(m_SourceLines != NULL)
    {
        for(int i = 0; i < m_SourceCount; ++i)
        {
            m_SourceLines[i]->ResetToOriginal();
        }
    }
}

CMixer::CMixer(int MixerId)
{
    MMRESULT mmresult = mixerGetDevCaps(MixerId, &m_MixerDev, sizeof(MIXERCAPS));
    m_hMixer = NULL;
    m_LineCount = 0;
    m_DestLines = NULL;
    if(mmresult != MMSYSERR_NOERROR || m_MixerDev.cDestinations == 0)
    {
        return;
    }
    mmresult = mixerOpen(&m_hMixer, MixerId, NULL, NULL, MIXER_OBJECTF_MIXER);
    if(mmresult != MMSYSERR_NOERROR)
    {
        return;
    }

    m_LineCount = m_MixerDev.cDestinations;
    m_DestLines = (CMixerLineDest**)calloc(sizeof(CMixerLineDest*), m_LineCount);
    for(int i = 0; i < m_LineCount; ++i)
    {
        m_DestLines[i] = new CMixerLineDest(m_hMixer, i);
    }
}

CMixer::~CMixer()
{
    if(m_hMixer != NULL)
    {
        mixerClose(m_hMixer);
    }
    if(m_DestLines != NULL)
    {
        for(int i = 0; i < m_LineCount; ++i)
        {
            delete m_DestLines[i];
        }
        free(m_DestLines);
    }
}

int CMixer::GetNumDestLines()
{
    return m_LineCount;
}

CMixerLineDest* CMixer::GetDestLine(int LineIndex)
{
    if(LineIndex >= 0 && LineIndex < m_LineCount)
    {
        return m_DestLines[LineIndex];
    }
    else
    {
        return NULL;
    }
}

void CMixer::ResetToOriginal()
{
    if(m_DestLines != NULL)
    {
        for(int i = 0; i < m_LineCount; ++i)
        {
            m_DestLines[i]->ResetToOriginal();
        }
    }
}


CSoundSystem::CSoundSystem()
{
    m_DeviceCount = mixerGetNumDevs();
    m_Mixer = NULL;
}

CSoundSystem::~CSoundSystem()
{
    if(m_Mixer != NULL)
    {
        delete m_Mixer;
        m_Mixer = NULL;
    }
}

int CSoundSystem::GetNumMixers()
{
    return m_DeviceCount;
}

char* CSoundSystem::GetMixerName(int MixerIndex)
{
    if(MixerIndex >= 0 && MixerIndex < m_DeviceCount)
    {
        static MIXERCAPS MixerCaps;
        MMRESULT mmresult = mixerGetDevCaps(MixerIndex, &MixerCaps, sizeof(MIXERCAPS));
        if(mmresult == MMSYSERR_NOERROR)
        {
            return MixerCaps.szPname;
        }
        else
        {
            return "Error";
        }
    }
    else
    {
        return "Not Found";
    }
}

char* CSoundSystem::GetMixerName2(int MixerIndex)
{
    static MIXERCAPS MixerCaps;
    MMRESULT mmresult = mixerGetDevCaps(MixerIndex, &MixerCaps, sizeof(MixerCaps));
    if(mmresult == MMSYSERR_NOERROR)
    {
        return MixerCaps.szPname;
    }
    else
    {
        static char szMixerError = NULL;
        return &szMixerError;
    }
}

int CSoundSystem::FindMixer(char* szPname)
{
    int DeviceCount = mixerGetNumDevs();
    for (int i = 0; i < DeviceCount; i++)
    {
        char* pDeviceName = GetMixerName2(i);
        if(*pDeviceName)
        {
            if(!lstrcmp(szPname, pDeviceName))
            {
                return i;
            }
        }
    }

    return -1;
}

void CSoundSystem::SetMixer(int MixerIndex)
{
    if(m_Mixer != NULL)
    {
        delete m_Mixer;
        m_Mixer = NULL;
    }
    if(MixerIndex >= 0 && MixerIndex < m_DeviceCount)
    {
        m_Mixer = new CMixer(MixerIndex);
    }
}

CMixer* CSoundSystem::GetMixer()
{
    return m_Mixer;
}

void ComboBox_AddValueAndSel(HWND hControl, LPCSTR szValue, long ItemData, long SelIndex)
{
    int Index = ComboBox_AddString(hControl, szValue);
    ComboBox_SetItemData(hControl, Index, ItemData);
    if(ItemData == SelIndex)
    {
        ComboBox_SetCurSel(hControl, Index);
    }
}

ComboBox_GetCurSelItemData(HWND hControl)
{
    return ComboBox_GetItemData(hControl, ComboBox_GetCurSel(hControl));
}

void EnableComboBoxes(HWND hDlg, BOOL Enabled)
{
    ComboBox_Enable(GetDlgItem(hDlg, IDC_MIXER), Enabled);
    ComboBox_Enable(GetDlgItem(hDlg, IDC_DEST), Enabled);

    int NumInputs = 0;
    if (Providers_GetCurrentSource()) 
    { 
        NumInputs = Providers_GetCurrentSource()->NumInputs(VIDEOINPUT);
    }

    ComboBox_Enable(GetDlgItem(hDlg, IDC_INPUT1), (NumInputs>=1)?Enabled:FALSE);
    ComboBox_Enable(GetDlgItem(hDlg, IDC_INPUT2), (NumInputs>=2)?Enabled:FALSE);
    ComboBox_Enable(GetDlgItem(hDlg, IDC_INPUT3), (NumInputs>=3)?Enabled:FALSE);
    ComboBox_Enable(GetDlgItem(hDlg, IDC_INPUT4), (NumInputs>=4)?Enabled:FALSE);
    ComboBox_Enable(GetDlgItem(hDlg, IDC_INPUT5), (NumInputs>=5)?Enabled:FALSE);
    ComboBox_Enable(GetDlgItem(hDlg, IDC_INPUT6), (NumInputs>=6)?Enabled:FALSE);
}

void FillLineBox(HWND hDlg, long ControlId, long SelIndex)
{
    HWND hControl = GetDlgItem(hDlg, ControlId);
    int Line = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_DEST));
    ComboBox_ResetContent(hControl);
    ComboBox_AddValueAndSel(hControl, 
                        "None",
                        -1, 
                        SelIndex);
    for(int i(0); i < pSoundSystem->GetMixer()->GetDestLine(Line)->GetNumSourceLines(); ++i)
    {
        ComboBox_AddValueAndSel(hControl, 
                            pSoundSystem->GetMixer()->GetDestLine(Line)->GetSourceLine(i)->GetName(),
                            i, 
                            SelIndex);
    }
}

void FillLineBoxes(HWND hDlg)
{
    FillLineBox(hDlg, IDC_INPUT1, InputIndexes[0]);
    FillLineBox(hDlg, IDC_INPUT2, InputIndexes[1]);
    FillLineBox(hDlg, IDC_INPUT3, InputIndexes[2]);
    FillLineBox(hDlg, IDC_INPUT4, InputIndexes[3]);
    FillLineBox(hDlg, IDC_INPUT5, InputIndexes[4]);
    FillLineBox(hDlg, IDC_INPUT6, InputIndexes[5]);
}

void FillDestBox(HWND hDlg)
{
    pSoundSystem->SetMixer(ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_MIXER)));
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_DEST));
    for(int i(0); i < pSoundSystem->GetMixer()->GetNumDestLines(); ++i)
    {
        ComboBox_AddValueAndSel(GetDlgItem(hDlg, IDC_DEST), 
                            pSoundSystem->GetMixer()->GetDestLine(i)->GetName(),
                            i, 
                            DestIndex);
    }
    FillLineBoxes(hDlg);
}

void FillMixersBox(HWND hDlg)
{
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_MIXER));
    pSoundSystem->SetMixer(MixerIndex);
    for(int i(0); i < pSoundSystem->GetNumMixers(); ++i)
    {
        ComboBox_AddValueAndSel(GetDlgItem(hDlg, IDC_MIXER), 
                            pSoundSystem->GetMixerName(i),
                            i, 
                            MixerIndex);
    }
    FillDestBox(hDlg);
}

BOOL APIENTRY MixerSetupProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    static BOOL Old_bUseMixer;
    static long Old_MixerIndex;
    static long Old_DestIndex;
    static long OldIndexes[6];
    int i;

    switch (message)
    {
    case WM_INITDIALOG:
        {
            int NumInputs = 0;
            if (Providers_GetCurrentSource()) 
            {     
                NumInputs = Providers_GetCurrentSource()->NumInputs(VIDEOINPUT);
            }
            if ( NumInputs > 6 ) 
            {
                // 6 in dialog right now
                NumInputs = 6;
            }
            
            string sInputName;
            for (i = 0; i < NumInputs; i++)
            {
                char *szName = (char*)Providers_GetCurrentSource()->GetInputName(VIDEOINPUT, i);
                if (szName == NULL)
                {
                    sInputName = "Input ";
                    sInputName+=i;
                }
                else
                {
                    sInputName = szName;
                }
                int RcNr = IDC_MIXER_INPUT0NAME;
                
                if (i==0) { RcNr = IDC_MIXER_INPUT0NAME; }
                else if (i==1) { RcNr = IDC_MIXER_INPUT1NAME; }
                else if (i==2) { RcNr = IDC_MIXER_INPUT2NAME; }
                else if (i==3) { RcNr = IDC_MIXER_INPUT3NAME; }
                else if (i==4) { RcNr = IDC_MIXER_INPUT4NAME; }
                else if (i==5) { RcNr = IDC_MIXER_INPUT5NAME; }

                SetDlgItemText(hDlg, RcNr, sInputName.c_str());
            }            
        }

        Old_bUseMixer = bUseMixer;
        Old_MixerIndex = MixerIndex;
        Old_DestIndex = DestIndex;
        for(i = 0; i < 6; ++i)
        {
            OldIndexes[i] = InputIndexes[i];
        }
        Button_SetCheck(GetDlgItem(hDlg, IDC_USE_MIXER), bUseMixer);
        Button_SetCheck(GetDlgItem(hDlg, IDC_RESETONEXIT), bResetOnExit);
        if(bUseMixer == TRUE)
        {
            EnableComboBoxes(hDlg, TRUE);
            FillMixersBox(hDlg);
        }
        
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_RESETONEXIT:
            bResetOnExit = (Button_GetCheck(GetDlgItem(hDlg, IDC_RESETONEXIT)) == BST_CHECKED);
            break;

        case IDC_USE_MIXER:
            bUseMixer = (Button_GetCheck(GetDlgItem(hDlg, IDC_USE_MIXER)) == BST_CHECKED);
            if(bUseMixer == TRUE)
            {
                pSoundSystem->SetMixer(MixerIndex);
                EnableComboBoxes(hDlg, TRUE);
                FillMixersBox(hDlg);
                if (Providers_GetCurrentSource() != NULL)
                {
                    Mixer_OnInputChange(Providers_GetCurrentSource()->GetInput(VIDEOINPUT));
                }
            }
            else
            {
                // Turn off use of mixer
                if (Providers_GetCurrentSource() != NULL)
                {
                    if(pSoundSystem->GetMixer() != NULL && bResetOnExit)
                    {                          
                          pSoundSystem->GetMixer()->ResetToOriginal();
                    }
                    else
                    {
                        ////Mute
                        //bUseMixer = TRUE;                  
                        //Mixer_OnInputChange(-1);
                        //bUseMixer = FALSE;
                    }
                }
                EnableComboBoxes(hDlg, FALSE);
                pSoundSystem->SetMixer(-1);
            }
            break;
        case IDC_MIXER:
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                FillDestBox(hDlg);
            }
            break;
        case IDC_DEST:
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                FillLineBoxes(hDlg);
            }
            break;
        case IDOK:
            if (bUseMixer && (Providers_GetCurrentSource() != NULL))
            {
                Mixer_OnInputChangeNotification(NULL,0,VIDEOINPUT,-1,-1);
            }

            MixerIndex = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_MIXER));
            lstrcpy((char*) &MixerName, pSoundSystem->GetMixerName2(MixerIndex));
            DestIndex = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_DEST));
            InputIndexes[0] = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_INPUT1));
            InputIndexes[1] = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_INPUT2));
            InputIndexes[2] = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_INPUT3));
            InputIndexes[3] = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_INPUT4));
            InputIndexes[4] = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_INPUT5));
            InputIndexes[5] = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_INPUT6));
            if(bUseMixer == FALSE)
            {
                pSoundSystem->SetMixer(-1);
            }
            else
            {
                if (Providers_GetCurrentSource() != NULL)
                {
                    Mixer_OnInputChangeNotification(NULL,0,VIDEOINPUT,-1,Providers_GetCurrentSource()->GetInput(VIDEOINPUT));
                }
            }
			WriteSettingsToIni(TRUE);
            EndDialog(hDlg, 0);
            break;

        case IDCANCEL:
            bUseMixer = Old_bUseMixer;
            MixerIndex = Old_MixerIndex;
            DestIndex = Old_DestIndex;
            for(i = 0; i < 6; ++i)
            {
                InputIndexes[i] = OldIndexes[i];
            }
            if(bUseMixer == FALSE)
            {
                pSoundSystem->SetMixer(-1);
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

void Mixer_SetupDlg(HWND hWndParent)
{
    PreShowDialogOrMenu();
    if(pSoundSystem == NULL || pSoundSystem->GetNumMixers() > 0)
    {
        DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_MIXERSETUP), hWndParent, MixerSetupProc);
    }
    else
    {
        MessageBox(hWnd, "No Mixer hardware found", "DScaler Error", MB_OK);
    }
    PostShowDialogOrMenu();
}

CMixerLineDest* Mixer_GetDestLine()
{
    // check that can go down pointers
    if(pSoundSystem == NULL)
    {
        return NULL;
    }
    if(pSoundSystem->GetMixer() == NULL)
    {
        return NULL;
    }

    return pSoundSystem->GetMixer()->GetDestLine(DestIndex);
}

CMixerLineSource* Mixer_GetInputLine(long nInput)
{
    CMixerLineDest* DestLine = Mixer_GetDestLine();
    if(DestLine == NULL)
    {
        return NULL;
    }

    if(bUseMixer)
    {
        if(InputIndexes[nInput] > -1)
        {
            return DestLine->GetSourceLine(InputIndexes[nInput]);
        }
    }
    return NULL;
}


CMixerLineSource* GetInputLine()
{
    CMixerLineSource* CurLine = NULL;
    int InputNr = -1;
    if (Providers_GetCurrentSource()) 
    { 
        InputNr = Providers_GetCurrentSource()->GetInput(VIDEOINPUT); 
    }    
    if ((InputNr >= 0) && (InputNr < 6))
    { 
        CurLine = Mixer_GetInputLine(InputNr); 
    }
    return CurLine;
}

void Mixer_Mute()
{    
    CMixerLineSource* CurLine = GetInputLine();
    if(CurLine != NULL)
    {
       CurLine->SetMute(TRUE);
    }
}

void Mixer_UnMute()
{   
    CMixerLineSource* CurLine = GetInputLine();
        
    if(CurLine != NULL)
    {
       CurLine->SetMute(FALSE);       
    }
}

BOOL Mixer_IsMuted()
{
    BOOL muted = FALSE;    
    CMixerLineSource* CurLine = GetInputLine();
    if(CurLine != NULL)
    {
       muted = CurLine->GetMute();       
    }
    return muted;
}

void Mixer_Volume_Up()
{   
    CMixerLineSource* CurLine = GetInputLine();
    
    if(CurLine != NULL)
    {
        long Vol = CurLine->GetVolume();
		long NewVol;
        if(Vol <= 95)
        {
           NewVol = Vol + 5;
        }
        else
        {
           NewVol = 100;
        }
		CurLine->SetVolume(NewVol);
		EventCollector->RaiseEvent(NULL, EVENT_MIXERVOLUME,Vol,NewVol);
    }
}

void Mixer_Volume_Down()
{
    CMixerLineSource* CurLine = GetInputLine();
    
    if(CurLine != NULL)
    {        
		long Vol = CurLine->GetVolume();
		long NewVol;
        if(Vol >= 5)
        {
           NewVol = Vol - 5;
        }
        else
        {
           NewVol = 0;
        }
		CurLine->SetVolume(NewVol);
		EventCollector->RaiseEvent(NULL, EVENT_MIXERVOLUME,Vol,NewVol);
    }
}

long Mixer_GetVolume()
{   
    CMixerLineSource* CurLine = GetInputLine();
    
    if(CurLine != NULL)
    {
        return CurLine->GetVolume();
    }
    else
    {
        return 0;
    }
}


void Mixer_SetVolume(long Volume)
{  
    CMixerLineSource* CurLine = GetInputLine();
    
    if(CurLine != NULL)
    {
        long OldVol = CurLine->GetVolume();
		CurLine->SetVolume(Volume);		
		EventCollector->RaiseEvent(NULL, EVENT_MIXERVOLUME,OldVol,Volume);
    }
    else
    {
        return;
    }
}


void Mixer_OnInputChange(int NewVideoInputNr)
{
    if (!bUseMixer)
    {
        return;
    }
    
    CMixerLineDest* DestLine = Mixer_GetDestLine();
    if(DestLine == NULL)
    {
        return;
    }

    if(Providers_GetCurrentSource()!=NULL )
    {
        int NewInputIndex = -1;
        
        if (   (NewVideoInputNr >=0 ) 
            && (NewVideoInputNr < Providers_GetCurrentSource()->NumInputs(VIDEOINPUT)) 
            && (NewVideoInputNr < 6) 
           )
        {
            NewInputIndex = InputIndexes[NewVideoInputNr];
        }
        
        // Mute mixer source lines for inactive video inputs
        for(int i(0) ; i < Providers_GetCurrentSource()->NumInputs(VIDEOINPUT); ++i)
        {
            if(InputIndexes[i] != -1)
            {
                if( (NewInputIndex != InputIndexes[i]) && (DestLine->GetSourceLine(InputIndexes[i]) != NULL))
                {
                    DestLine->GetSourceLine(InputIndexes[i])->SetMute(TRUE);
                }
            }
        }

        // Enable mixer source line for the new video input
        if( (NewInputIndex != -1) && (DestLine->GetSourceLine(NewInputIndex) != NULL) )
        {
           DestLine->GetSourceLine(NewInputIndex)->SetMute(FALSE);
		   
		   EventCollector->RaiseEvent(NULL, EVENT_MIXERVOLUME,-1,DestLine->GetSourceLine(NewInputIndex)->GetVolume());
        }
    }
}

void Mixer_OnInputChangeNotification(void *pThis, int PreChange, eSourceInputType InputType, int OldInput, int NewInput)
{
    if ((!PreChange) && (InputType == VIDEOINPUT))
    {
        if (bUseMixer)
        {            
            Mixer_OnInputChange(NewInput);
        }
    }
}

void Mixer_OnSourceChange(void *pThis, int Flags, CSource *pSource)
{
    if (pSource == NULL) return;

    if (Flags&1)
    {
        //pSource->Unregister_InputChangeNotification(NULL,Mixer_OnInputChangeNotification);
        
        // Save settings        
        int Num = pSource->NumInputs(VIDEOINPUT);

        MixerDev_SettingSetSection(pSource->IDString());
        if (bUseMixer)
        {
            if (Num>=1) { Setting_WriteToIni(&(MixerDevSettings[INPUT1INDEX]), FALSE); }
            if (Num>=2) { Setting_WriteToIni(&(MixerDevSettings[INPUT2INDEX]), FALSE); }
            if (Num>=3) { Setting_WriteToIni(&(MixerDevSettings[INPUT3INDEX]), FALSE); }
            if (Num>=4) { Setting_WriteToIni(&(MixerDevSettings[INPUT4INDEX]), FALSE); }
            if (Num>=5) { Setting_WriteToIni(&(MixerDevSettings[INPUT5INDEX]), FALSE); }
            if (Num>=6) { Setting_WriteToIni(&(MixerDevSettings[INPUT6INDEX]), FALSE); }
        
            // Mute
            Mixer_OnInputChangeNotification(NULL,0,VIDEOINPUT,-1,-1);
        }
    } 
    else 
    {
        // Read settings
        MixerDev_SettingSetSection(pSource->IDString());
        Setting_ReadFromIni(&(MixerDevSettings[INPUT1INDEX]));
        Setting_ReadFromIni(&(MixerDevSettings[INPUT2INDEX]));
        Setting_ReadFromIni(&(MixerDevSettings[INPUT3INDEX]));
        Setting_ReadFromIni(&(MixerDevSettings[INPUT4INDEX]));
        Setting_ReadFromIni(&(MixerDevSettings[INPUT5INDEX]));
        Setting_ReadFromIni(&(MixerDevSettings[INPUT6INDEX]));
        
        //pSource->Register_InputChangeNotification(NULL,Mixer_OnInputChangeNotification);        
        //EventCollector->Register(Mixer_EventOnChangeNotification,NULL, (EVENT_SOURCE_PRECHANGE,EVENT_SOURCE_CHANGE,EVENT_VIDEOINPUT_PRECHANGE,EVENT_VIDEOINPUT_CHANGE,0));
        Mixer_OnInputChangeNotification(NULL,0,VIDEOINPUT,-1,pSource->GetInput(VIDEOINPUT));        
    }
}

void Mixer_EventOnChangeNotification(void *pThis, CEventObject *pEventObject, eEventType EventType, long OldValue, long NewValue, eEventType *ComingUp)
{
    if (EventType == EVENT_SOURCE_PRECHANGE) 
    { 
        Mixer_OnSourceChange(pThis, 1, (CSource*)OldValue); 
    }
    else if (EventType == EVENT_SOURCE_CHANGE) 
    { 
        Mixer_OnSourceChange(pThis, 0, (CSource*)NewValue); 
    }
    else 
	{
		if (pEventObject == (CEventObject*)Providers_GetCurrentSource())
		{
			if (EventType == EVENT_VIDEOINPUT_PRECHANGE) 
            { 
                Mixer_OnInputChangeNotification(pThis, 1, VIDEOINPUT, OldValue, NewValue); 
            }
			else if (EventType == EVENT_VIDEOINPUT_CHANGE) 
            { 
                Mixer_OnInputChangeNotification(pThis, 0, VIDEOINPUT, OldValue, NewValue); 
            }    
		}
	}
}


void Mixer_Exit()
{
    if(pSoundSystem != NULL)
    {
        // set the chip to mute
        if (Providers_GetCurrentSource() != NULL)
        {
            Providers_GetCurrentSource()->Mute();
            Mixer_OnSourceChange(NULL,1,Providers_GetCurrentSource());                        
        }        
        //Providers_Unregister_SourceChangeNotification(NULL,Mixer_OnSourceChange);
        EventCollector->Unregister(Mixer_EventOnChangeNotification,NULL);

		if(bUseMixer)
		{
            if(pSoundSystem->GetMixer() != NULL && bResetOnExit)
            {
                pSoundSystem->GetMixer()->ResetToOriginal();
            }
		}
        delete pSoundSystem;
        pSoundSystem = NULL;
    }
}


void Mixer_Init()
{
    pSoundSystem = new CSoundSystem();

    //Providers_Register_SourceChangeNotification(NULL,Mixer_OnSourceChange);                        
    eEventType EventList[] = {EVENT_SOURCE_PRECHANGE,EVENT_SOURCE_CHANGE,EVENT_VIDEOINPUT_PRECHANGE,EVENT_VIDEOINPUT_CHANGE,EVENT_NONE};
    EventCollector->Register(Mixer_EventOnChangeNotification,NULL, EventList);
    if(bUseMixer)
    {
        pSoundSystem->SetMixer(MixerIndex);
        if(pSoundSystem->GetMixer() != NULL)
        {
            //
        }
        else
        {
            bUseMixer = FALSE;
        }        
    }
    else
    {
        pSoundSystem->SetMixer(-1);
    }

    if (Providers_GetCurrentSource())
    {
        Mixer_OnSourceChange(NULL,2,Providers_GetCurrentSource());                                
    }    
}

void Mixer_UpdateIndex()
{
    MixerIndex = CSoundSystem::FindMixer((char*) &MixerName);
    if(MixerIndex < 0)
    {
        MixerIndex = NULL;
    }
}

SETTING MixerDevSettings[MIXERDEV_SETTING_LASTONE] =
{
    {
        "Use Mixer", ONOFF, 0, (long*)&bUseMixer,
        FALSE, 0, 1, 1, 1, 
        NULL,
        "Mixer", "UseMixer", NULL,
    },
    {
        "DestIndex", SLIDER, 0, (long*)&DestIndex,
        0, 0, 255, 1, 1,
        NULL,
        "Mixer", "DestIndex", NULL,
    },
    {
        "Input 1 Index", SLIDER, 0, (long*)&InputIndexes[0],
        -1, -1, 255, 1, 1,
        NULL,
        "Mixer", "Input1Index", NULL,
    },
    {
        "Input 2 Index", SLIDER, 0, (long*)&InputIndexes[1],
        -1, -1, 255, 1, 1,
        NULL,
        "Mixer", "Input2Index", NULL,
    },
    {
        "Input 3 Index", SLIDER, 0, (long*)&InputIndexes[2],
        -1, -1, 255, 1, 1,
        NULL,
        "Mixer", "Input3Index", NULL,
    },
    {
        "Input 4 Index", SLIDER, 0, (long*)&InputIndexes[3],
        -1, -1, 255, 1, 1,
        NULL,
        "Mixer", "Input4Index", NULL,
    },
    {
        "Reset Mixer on Exit", ONOFF, 0, (long*)&bResetOnExit,
        TRUE, 0, 1, 1, 1, 
        NULL,
        "Mixer", "ResetOnExit", NULL,
    },
    {
        "Input 5 Index", SLIDER, 0, (long*)&InputIndexes[4],
        -1, -1, 255, 1, 1,
        NULL,
        "Mixer", "Input5Index", NULL,
    },
    {
        "Input 6 Index", SLIDER, 0, (long*)&InputIndexes[5],
        -1, -1, 255, 1, 1,
        NULL,
        "Mixer", "Input6Index", NULL,
    },
};


SETTING* MixerDev_GetSetting(MIXERDEV_SETTING Setting)
{
    if(Setting > -1 && Setting < MIXERDEV_SETTING_LASTONE)
    {
        return &(MixerDevSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void MixerDev_ReadSettingsFromIni()
{
    int i;
    if (Providers_GetCurrentSource() != NULL)
    {
        MixerDev_SettingSetSection(Providers_GetCurrentSource()->IDString());
    }
    for(i = 0; i < MIXERDEV_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(MixerDevSettings[i]));
    }
    
    extern char szIniFile[MAX_PATH];
    GetPrivateProfileString("Mixer", "MixerName", "", (char*) &MixerName, sizeof(MixerName), szIniFile);
    MixerIndex = CSoundSystem::FindMixer((char*) MixerName);
    if(MixerIndex < 0)
    {
        MixerIndex = NULL;
    }
}

void MixerDev_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < MIXERDEV_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(MixerDevSettings[i]), bOptimizeFileAccess);
    }

    extern char szIniFile[MAX_PATH];
    WritePrivateProfileString("Mixer", "MixerName", (char*) &MixerName, szIniFile);
}


void MixerDev_SetMenu(HMENU hMenu)
{
}

void MixerDev_SettingSetSection(LPCSTR szSource)
{
    MixerDev_Section = string("MixerInput_") + szSource;
    Setting_SetSection(&MixerDevSettings[INPUT1INDEX], (char*)MixerDev_Section.c_str());
    Setting_SetSection(&MixerDevSettings[INPUT2INDEX], (char*)MixerDev_Section.c_str());
    Setting_SetSection(&MixerDevSettings[INPUT3INDEX], (char*)MixerDev_Section.c_str());
    Setting_SetSection(&MixerDevSettings[INPUT4INDEX], (char*)MixerDev_Section.c_str());
    Setting_SetSection(&MixerDevSettings[INPUT5INDEX], (char*)MixerDev_Section.c_str());
    Setting_SetSection(&MixerDevSettings[INPUT6INDEX], (char*)MixerDev_Section.c_str());    
}
