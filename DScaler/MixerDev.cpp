/////////////////////////////////////////////////////////////////////////////
// $Id: MixerDev.cpp,v 1.11 2001-07-13 16:14:56 adcockj Exp $
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
// Revision 1.10  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "MixerDev.h"
#include "DScaler.h"

CSoundSystem* pSoundSystem = NULL;

BOOL bSystemInMute = FALSE;
BOOL bUseMixer = FALSE;
long MixerIndex = 0;
long DestIndex = 0;
long TunerLineIndex = -1;
long CompositeLineIndex = -1;
long SVideoLineIndex = -1;
long Other1LineIndex = -1;
long Other2LineIndex = -1;

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
    ComboBox_Enable(GetDlgItem(hDlg, IDC_TUNER), Enabled);
    ComboBox_Enable(GetDlgItem(hDlg, IDC_COMPOSITE), Enabled);
    ComboBox_Enable(GetDlgItem(hDlg, IDC_SVIDEO), Enabled);
    ComboBox_Enable(GetDlgItem(hDlg, IDC_OTHER1), Enabled);
    ComboBox_Enable(GetDlgItem(hDlg, IDC_OTHER2), Enabled);
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
    FillLineBox(hDlg, IDC_TUNER, TunerLineIndex);
    FillLineBox(hDlg, IDC_COMPOSITE, CompositeLineIndex);
    FillLineBox(hDlg, IDC_SVIDEO, SVideoLineIndex);
    FillLineBox(hDlg, IDC_OTHER1, Other1LineIndex);
    FillLineBox(hDlg, IDC_OTHER2, Other2LineIndex);
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
    static long Old_TunerLineIndex;
    static long Old_CompositeLineIndex;
    static long Old_SVideoLineIndex;
    static long Old_Other1LineIndex;
    static long Old_Other2LineIndex;

    switch (message)
    {
    case WM_INITDIALOG:
        Old_bUseMixer = bUseMixer;
        Old_MixerIndex = MixerIndex;
        Old_DestIndex = DestIndex;
        Old_TunerLineIndex = TunerLineIndex;
        Old_CompositeLineIndex = CompositeLineIndex;
        Old_SVideoLineIndex = SVideoLineIndex;
        Old_Other1LineIndex = Other1LineIndex;
        Old_Other2LineIndex = Other2LineIndex;
        Button_SetCheck(GetDlgItem(hDlg, IDC_USE_MIXER), bUseMixer);
        if(bUseMixer == TRUE)
        {
            EnableComboBoxes(hDlg, TRUE);
            FillMixersBox(hDlg);
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_USE_MIXER:
            bUseMixer = (Button_GetCheck(GetDlgItem(hDlg, IDC_USE_MIXER)) == BST_CHECKED);
            if(bUseMixer == TRUE)
            {
                pSoundSystem->SetMixer(MixerIndex);
                EnableComboBoxes(hDlg, TRUE);
                FillMixersBox(hDlg);
            }
            else
            {
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
            MixerIndex = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_MIXER));
            DestIndex = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_DEST));
            TunerLineIndex = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_TUNER));
            CompositeLineIndex = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_COMPOSITE));
            SVideoLineIndex = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_SVIDEO));
            Other1LineIndex = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_OTHER1));
            Other2LineIndex = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_OTHER2));
            if(bUseMixer == FALSE)
            {
                pSoundSystem->SetMixer(-1);
            }
            EndDialog(hDlg, 0);
            break;

        case IDCANCEL:
            bUseMixer = Old_bUseMixer;
            MixerIndex = Old_MixerIndex;
            DestIndex = Old_DestIndex;
            TunerLineIndex = Old_TunerLineIndex;
            CompositeLineIndex = Old_CompositeLineIndex;
            SVideoLineIndex = Old_SVideoLineIndex;
            Other1LineIndex = Old_Other1LineIndex;
            Other2LineIndex = Old_Other2LineIndex;
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
    if(pSoundSystem == NULL || pSoundSystem->GetNumMixers() > 0)
    {
        DialogBox(hInst, MAKEINTRESOURCE(IDD_MIXERSETUP), hWndParent, MixerSetupProc);
    }
    else
    {
        MessageBox(hWnd, "No Mixer hardware found", "DScaler Error", MB_OK);
    }
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

CMixerLineSource* Mixer_GetInputLine(long NewType)
{
    CMixerLineDest* DestLine = Mixer_GetDestLine();
    if(DestLine == NULL)
    {
        return NULL;
    }

    if(bUseMixer)
    {
        switch(NewType)
        {
        case SOURCE_TUNER:
            if(TunerLineIndex > -1)
            {
                return DestLine->GetSourceLine(TunerLineIndex);
            }
            break;
        case SOURCE_COMPOSITE:
            if(CompositeLineIndex > -1)
            {
                return DestLine->GetSourceLine(CompositeLineIndex);
            }
            break;
        case SOURCE_SVIDEO:
        case SOURCE_COMPVIASVIDEO:
            if(SVideoLineIndex > -1)
            {
                return DestLine->GetSourceLine(SVideoLineIndex);
            }
            break;
        case SOURCE_OTHER1:
            if(Other1LineIndex > -1)
            {
                return DestLine->GetSourceLine(Other1LineIndex);
            }
            break;
        case SOURCE_OTHER2:
            if(Other2LineIndex > -1)
            {
                return DestLine->GetSourceLine(Other2LineIndex);
            }
            break;
        default:
            break;
        }
    }
    return NULL;
}

void Mixer_Mute()
{
   CMixerLineSource* CurLine = Mixer_GetInputLine(Setting_GetValue(BT848_GetSetting(VIDEOSOURCE)));
   if(CurLine != NULL)
   {
       CurLine->SetMute(TRUE);
   }
}

void Mixer_UnMute()
{
   CMixerLineSource* CurLine = Mixer_GetInputLine(Setting_GetValue(BT848_GetSetting(VIDEOSOURCE)));
   if(CurLine != NULL)
   {
       CurLine->SetMute(FALSE);
   }
}

void Mixer_Volume_Up()
{
   CMixerLineSource* CurLine = Mixer_GetInputLine(Setting_GetValue(BT848_GetSetting(VIDEOSOURCE)));
   if(CurLine != NULL)
   {
       long Vol = CurLine->GetVolume();
       if(Vol <= 95)
       {
           CurLine->SetVolume(Vol+5);
       }
       else
       {
           CurLine->SetVolume(100);
       }
   }
}

void Mixer_Volume_Down()
{
   CMixerLineSource* CurLine = Mixer_GetInputLine(Setting_GetValue(BT848_GetSetting(VIDEOSOURCE)));
   if(CurLine != NULL)
   {
       long Vol = CurLine->GetVolume();
       if(Vol >= 5)
       {
           CurLine->SetVolume(Vol-5);
       }
       else
       {
           CurLine->SetVolume(0);
       }
   }
}

long Mixer_GetVolume()
{
   CMixerLineSource* CurLine = Mixer_GetInputLine(Setting_GetValue(BT848_GetSetting(VIDEOSOURCE)));
   if(CurLine != NULL)
   {
       return CurLine->GetVolume();
   }
   else
   {
       return 0;
   }
}

// here we go down and mute all input execpt the one we want
void Mixer_OnInputChange(eVideoSourceType NewType)
{
    CMixerLineDest* DestLine = Mixer_GetDestLine();
    if(DestLine == NULL)
    {
        return;
    }

    if(bUseMixer)
    {
        if(TunerLineIndex > -1)
        {
            if(DestLine->GetSourceLine(TunerLineIndex) != NULL)
            {
                DestLine->GetSourceLine(TunerLineIndex)->SetMute(NewType != SOURCE_TUNER);
            }
        }
        if(CompositeLineIndex > -1)
        {
            if(DestLine->GetSourceLine(CompositeLineIndex) != NULL)
            {
                DestLine->GetSourceLine(CompositeLineIndex)->SetMute(NewType != SOURCE_COMPOSITE);
            }
        }
        if(SVideoLineIndex > -1)
        {
            if(DestLine->GetSourceLine(SVideoLineIndex) != NULL)
            {
                DestLine->GetSourceLine(SVideoLineIndex)->SetMute((NewType != SOURCE_SVIDEO) && (NewType != SOURCE_COMPVIASVIDEO));
            }
        }
        if(Other1LineIndex > -1)
        {
            if(DestLine->GetSourceLine(Other1LineIndex) != NULL)
            {
                DestLine->GetSourceLine(Other1LineIndex)->SetMute(NewType != SOURCE_OTHER1);
            }
        }
        if(Other2LineIndex > -1)
        {
            if(DestLine->GetSourceLine(Other2LineIndex) != NULL)
            {
                DestLine->GetSourceLine(Other2LineIndex)->SetMute(NewType != SOURCE_OTHER2);
            }
        }
    }
}

void Mixer_Exit()
{
    if(bUseMixer)
    {
        // Mute all inputs
        Mixer_OnInputChange(SOURCE_CCIR656_4);
        pSoundSystem->SetMixer(-1);
    }
    if(pSoundSystem != NULL)
    {
        delete pSoundSystem;
        pSoundSystem = NULL;
    }
}

void Mixer_Init()
{
    pSoundSystem = new CSoundSystem();
    if(bUseMixer)
    {
        pSoundSystem->SetMixer(MixerIndex);
        if(pSoundSystem->GetMixer() != NULL)
        {
            Mixer_OnInputChange((eVideoSourceType)Setting_GetValue(BT848_GetSetting(VIDEOSOURCE)));
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
        "System in Mute", ONOFF, 0, (long*)&bSystemInMute,
        FALSE, 0, 1, 1, 1, 
        NULL,
        "Sound", "System_In_Mute", NULL,
    },
    {
        "MixerIndex", SLIDER, 0, (long*)&MixerIndex,
        0, 0, 255, 1, 1,
        NULL,
        "Mixer", "MixerIndex", NULL,
    },
    {
        "DestIndex", SLIDER, 0, (long*)&DestIndex,
        0, 0, 255, 1, 1,
        NULL,
        "Mixer", "DestIndex", NULL,
    },
    {
        "TunerLineIndex", SLIDER, 0, (long*)&TunerLineIndex,
        -1, -1, 255, 1, 1,
        NULL,
        "Mixer", "TunerLineIndex", NULL,
    },
    {
        "CompositeLineIndex", SLIDER, 0, (long*)&CompositeLineIndex,
        -1, -1, 255, 1, 1,
        NULL,
        "Mixer", "CompositeLineIndex", NULL,
    },
    {
        "SVideoLineIndex", SLIDER, 0, (long*)&SVideoLineIndex,
        -1, -1, 255, 1, 1,
        NULL,
        "Mixer", "SVideoLineIndex", NULL,
    },
    {
        "Other1LineIndex", SLIDER, 0, (long*)&Other1LineIndex,
        -1, -1, 255, 1, 1,
        NULL,
        "Mixer", "Other1LineIndex", NULL,
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
    for(i = 0; i < MIXERDEV_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(MixerDevSettings[i]));
    }
}

void MixerDev_WriteSettingsToIni()
{
    int i;
    for(i = 0; i < MIXERDEV_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(MixerDevSettings[i]));
    }
}


void MixerDev_SetMenu(HMENU hMenu)
{
    CheckMenuItemBool(hMenu, IDM_MUTE, bSystemInMute);
}
