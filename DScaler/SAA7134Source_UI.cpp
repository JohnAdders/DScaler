/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Source_UI.cpp,v 1.6 2002-09-25 15:11:12 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Atsushi Nakagawa.  All rights reserved.
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
// This software was based on BT848Souce_UI.cpp.  Those portions are
// Copyright (c) 2001 John Adcock.
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 09 Sep 2002   Atsushi Nakagawa      Initial Release
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.5  2002/09/16 17:52:34  atnak
// Support for SAA7134Res.dll dialogs
//
// Revision 1.4  2002/09/14 19:40:48  atnak
// various changes
//
// Revision 1.3  2002/09/10 12:24:03  atnak
// changed some UI stuff
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "..\DScalerResDbg\SAA7134Res\resource.h"
#include "resource.h"
#include "SAA7134Source.h"
#include "SAA7134_Defines.h"
#include "DScaler.h"
#include "OutThreads.h"
#include "AspectRatio.h"
#include "DebugLog.h"
#include "SettingsPerChannel.h"
#include "Slider.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern const char *TunerNames[TUNER_LASTONE];


// SAA7134: This file still needs loads of work


void CSAA7134Source::InitializeUI()
{
    m_hSAA7134ResourceInst = LoadLibrary("SAA7134Res.dll");
}


void CSAA7134Source::CleanupUI()
{
    if (m_hSAA7134ResourceInst != NULL)
    {
        FreeLibrary(m_hSAA7134ResourceInst);
    }
}


BOOL APIENTRY CSAA7134Source::SelectCardProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    int i;
    int nIndex;
    char buf[128];
    static long OrigTuner;
    static CSAA7134Source* pThis;

    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CSAA7134Source*)lParam;
        sprintf(buf, "Setup card %u with chip %s", pThis->GetDeviceIndex() + 1, pThis->GetChipName());
        SetWindowText(hDlg, buf);
        Button_Enable(GetDlgItem(hDlg, IDCANCEL), pThis->m_bSelectCardCancelButton);
        SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_RESETCONTENT, 0, 0);
        for(i = 0; i < pThis->m_pSAA7134Card->GetMaxCards(); i++)
        {
            int nIndex;
            nIndex = SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_ADDSTRING, 0, (LONG)pThis->m_pSAA7134Card->GetCardName((eTVCardId)i));
            SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_SETITEMDATA, nIndex, i);
            if(i == pThis->m_CardType->GetValue())
            {
                SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_SETCURSEL, nIndex, 0);
            }
        }

        SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_RESETCONTENT, 0, 0);
        for(i = 0; i < TUNER_LASTONE; i++)
        {
            nIndex = SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_ADDSTRING, 0, (LONG)TunerNames[i]);
            SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_SETITEMDATA, nIndex, i);
        }

        OrigTuner = pThis->m_TunerType->GetValue();
        SetFocus(hDlg);
        // Update the tuner combobox after the SetFocus
        // because SetFocus modifies this combobox
        for (nIndex = 0; nIndex < TUNER_LASTONE; nIndex++)
        {
          i = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_TUNERSELECT), nIndex);
          if (i == pThis->m_TunerType->GetValue() )
          {          
            SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_SETCURSEL, nIndex, 0);
          }
        }
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            i = SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_GETCURSEL, 0, 0);
            pThis->m_TunerType->SetValue(ComboBox_GetItemData(GetDlgItem(hDlg, IDC_TUNERSELECT), i));

            i =  SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_GETCURSEL, 0, 0);
            pThis->m_CardType->SetValue(ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CARDSSELECT), i));
            if(OrigTuner != pThis->m_TunerType->GetValue())
            {
                pThis->ChangeTVSettingsBasedOnTuner();
            }
            WriteSettingsToIni(TRUE);
            EndDialog(hDlg, TRUE);
            break;
        case IDCANCEL:
            EndDialog(hDlg, TRUE);
            break;
        case IDC_CARDSSELECT:
            i = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_CARDSSELECT));
            i = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CARDSSELECT), i);                        
            i = pThis->m_pSAA7134Card->AutoDetectTuner((eTVCardId)i);
            for (nIndex = 0; nIndex < TUNER_LASTONE; nIndex++)
            {   
              if (ComboBox_GetItemData(GetDlgItem(hDlg, IDC_TUNERSELECT), nIndex) == i)
              {
                 ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_TUNERSELECT), nIndex);
              }
            }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return (FALSE);
}

BOOL APIENTRY CSAA7134Source::RegisterEditProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    int i;
    char buf[128];
    static CSAA7134Source* pThis;
    static DWORD dwAddress;
    static BOOL bDisableChanges = TRUE;
    static BOOL bUnsafeRead = TRUE;
    BYTE Data;
    WORD LastChanged = 0;

    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CSAA7134Source*)lParam;
        sprintf(buf, "Edit Register on %s chip", pThis->GetChipName());
        SetWindowText(hDlg, buf);
        SendMessage(GetDlgItem(hDlg, IDC_REGISTERSELECT), CB_RESETCONTENT, 0, 0);
        for(i = 0; i < 0x300; i++)
        {
            int nIndex;
            sprintf(buf, "0x%03X", i);
            nIndex = SendMessage(GetDlgItem(hDlg, IDC_REGISTERSELECT), CB_ADDSTRING, 0, (LONG)buf);
            SendMessage(GetDlgItem(hDlg, IDC_REGISTERSELECT), CB_SETITEMDATA, nIndex, i);
            if (i == 0)
            {   
                SendMessage(GetDlgItem(hDlg, IDC_REGISTERSELECT), CB_SETCURSEL, nIndex, i);
            }
        }
        SetFocus(hDlg);
        RegisterEditProc(hDlg, WM_COMMAND, MAKELONG(IDC_REGISTERSELECT, 0L), lParam);
        break;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_REFRESH:
            if (!bUnsafeRead)
            {
                Data = pThis->m_pSAA7134Card->DirectGetByte(dwAddress);
                sprintf(buf, "%d", Data);
                SetDlgItemText(hDlg, IDC_VALUE_DEC, buf);
                sprintf(buf, "0x%02X", Data);
                SetDlgItemText(hDlg, IDC_VALUE_HEX, buf);
                SetDlgItemText(hDlg, IDC_STATIC_BIT0, Data & (1<<0) ? "1" : "0");
                SetDlgItemText(hDlg, IDC_STATIC_BIT1, Data & (1<<1) ? "1" : "0");
                SetDlgItemText(hDlg, IDC_STATIC_BIT2, Data & (1<<2) ? "1" : "0");
                SetDlgItemText(hDlg, IDC_STATIC_BIT3, Data & (1<<3) ? "1" : "0");
                SetDlgItemText(hDlg, IDC_STATIC_BIT4, Data & (1<<4) ? "1" : "0");
                SetDlgItemText(hDlg, IDC_STATIC_BIT5, Data & (1<<5) ? "1" : "0");
                SetDlgItemText(hDlg, IDC_STATIC_BIT6, Data & (1<<6) ? "1" : "0");
                SetDlgItemText(hDlg, IDC_STATIC_BIT7, Data & (1<<7) ? "1" : "0");
                CheckDlgButton(hDlg, IDC_BIT0, Data & (1<<0));
                CheckDlgButton(hDlg, IDC_BIT1, Data & (1<<1));
                CheckDlgButton(hDlg, IDC_BIT2, Data & (1<<2));
                CheckDlgButton(hDlg, IDC_BIT3, Data & (1<<3));
                CheckDlgButton(hDlg, IDC_BIT4, Data & (1<<4));
                CheckDlgButton(hDlg, IDC_BIT5, Data & (1<<5));
                CheckDlgButton(hDlg, IDC_BIT6, Data & (1<<6));
                CheckDlgButton(hDlg, IDC_BIT7, Data & (1<<7));
            }
            break;

        case IDOK:
        case IDCANCEL:
            EndDialog(hDlg, TRUE);
            break;

        case IDC_REGISTERSELECT:
            i = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_REGISTERSELECT));
            dwAddress = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_REGISTERSELECT), i);
            if (dwAddress >= 0x300)
            {
                bUnsafeRead = TRUE;
                bDisableChanges = TRUE;
            }
            else
            {
                bUnsafeRead = FALSE;
                bDisableChanges = FALSE;
            }
            RegisterEditProc(hDlg, WM_COMMAND, MAKELONG(IDC_REFRESH, 0L), lParam);
            break;

        case IDC_BIT7:
        case IDC_BIT6:
        case IDC_BIT5:
        case IDC_BIT4:
        case IDC_BIT3:
        case IDC_BIT2:
        case IDC_BIT1:
        case IDC_BIT0:
            if (!bDisableChanges)
            {
                i = LOWORD(wParam) - IDC_BIT0;
                pThis->m_pSAA7134Card->DirectSetBit(dwAddress, i,
                    (BST_CHECKED == IsDlgButtonChecked(hDlg, LOWORD(wParam))));
                RegisterEditProc(hDlg, WM_COMMAND, MAKELONG(IDC_REFRESH, 0L), lParam);
            }
            else
            {
                LastChanged = LOWORD(wParam);
            }
        default:
            break;
        }
        break;
    default:
        break;
    }
    return (FALSE);
}

BOOL APIENTRY CSAA7134Source::OtherEditProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    static CSAA7134Source* pThis;

    static char LeftGain = 0;
    static char RightGain = 0;
    static char NicamGain = 0;
    static BOOL bLinked = TRUE;

    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CSAA7134Source*)lParam;

        LeftGain = pThis->m_pSAA7134Card->GetAudioLeftVolume();
        RightGain = pThis->m_pSAA7134Card->GetAudioRightVolume();
        NicamGain = pThis->m_pSAA7134Card->GetAudioNicamVolume();

        Slider_ClearTicks(GetDlgItem(hDlg, IDC_LEFT_SLIDER), TRUE);
        Slider_SetRangeMax(GetDlgItem(hDlg, IDC_LEFT_SLIDER), 15);
        Slider_SetRangeMin(GetDlgItem(hDlg, IDC_LEFT_SLIDER), -16);
        Slider_SetPageSize(GetDlgItem(hDlg, IDC_LEFT_SLIDER), 1);
        Slider_SetLineSize(GetDlgItem(hDlg, IDC_LEFT_SLIDER), 1);
        Slider_SetTic(GetDlgItem(hDlg, IDC_LEFT_SLIDER), 0);
        Slider_SetPos(GetDlgItem(hDlg, IDC_LEFT_SLIDER), LeftGain);
        SetDlgItemInt(hDlg, IDC_LEFT_EDIT, LeftGain, TRUE);

        Slider_ClearTicks(GetDlgItem(hDlg, IDC_RIGHT_SLIDER), TRUE);
        Slider_SetRangeMax(GetDlgItem(hDlg, IDC_RIGHT_SLIDER), 15);
        Slider_SetRangeMin(GetDlgItem(hDlg, IDC_RIGHT_SLIDER), -16);
        Slider_SetPageSize(GetDlgItem(hDlg, IDC_RIGHT_SLIDER), 1);
        Slider_SetLineSize(GetDlgItem(hDlg, IDC_RIGHT_SLIDER), 1);
        Slider_SetTic(GetDlgItem(hDlg, IDC_RIGHT_SLIDER), 0);
        Slider_SetPos(GetDlgItem(hDlg, IDC_RIGHT_SLIDER), RightGain);
        SetDlgItemInt(hDlg, IDC_RIGHT_EDIT, RightGain, TRUE);

        Slider_ClearTicks(GetDlgItem(hDlg, IDC_NICAM_SLIDER), TRUE);
        Slider_SetRangeMax(GetDlgItem(hDlg, IDC_NICAM_SLIDER), 15);
        Slider_SetRangeMin(GetDlgItem(hDlg, IDC_NICAM_SLIDER), -16);
        Slider_SetPageSize(GetDlgItem(hDlg, IDC_NICAM_SLIDER), 1);
        Slider_SetLineSize(GetDlgItem(hDlg, IDC_NICAM_SLIDER), 1);
        Slider_SetTic(GetDlgItem(hDlg, IDC_NICAM_SLIDER), 0);
        Slider_SetPos(GetDlgItem(hDlg, IDC_NICAM_SLIDER), NicamGain);
        SetDlgItemInt(hDlg, IDC_NICAM_EDIT, NicamGain, TRUE);

        CheckDlgButton(hDlg, IDC_LINKED_CHECK, bLinked);

        SetFocus(hDlg);
        break;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_LINKED_CHECK:
            bLinked = (BST_CHECKED == IsDlgButtonChecked(hDlg, LOWORD(wParam)));
            break;

        case IDOK:
        case IDCANCEL:
            EndDialog(hDlg, TRUE);
            break;

        default:
            break;
        }
        break;

    case WM_VSCROLL:
    case WM_HSCROLL:
        if((HWND)lParam == GetDlgItem(hDlg, IDC_LEFT_SLIDER))
        {
            LeftGain = Slider_GetPos(GetDlgItem(hDlg, IDC_LEFT_SLIDER));
            SetDlgItemInt(hDlg, IDC_LEFT_EDIT, LeftGain, TRUE);
            pThis->m_pSAA7134Card->SetAudioLeftVolume(LeftGain);

            if (bLinked)
            {
                RightGain = LeftGain;
                Slider_SetPos(GetDlgItem(hDlg, IDC_RIGHT_SLIDER), RightGain);
                SetDlgItemInt(hDlg, IDC_RIGHT_EDIT, RightGain, TRUE);
                pThis->m_pSAA7134Card->SetAudioRightVolume(RightGain);
            }
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_RIGHT_SLIDER))
        {
            RightGain = Slider_GetPos(GetDlgItem(hDlg, IDC_RIGHT_SLIDER));
            SetDlgItemInt(hDlg, IDC_RIGHT_EDIT, RightGain, TRUE);
            pThis->m_pSAA7134Card->SetAudioRightVolume(RightGain);

            if (bLinked)
            {
                LeftGain = RightGain;
                Slider_SetPos(GetDlgItem(hDlg, IDC_LEFT_SLIDER), LeftGain);
                SetDlgItemInt(hDlg, IDC_LEFT_EDIT, LeftGain, TRUE);
                pThis->m_pSAA7134Card->SetAudioLeftVolume(LeftGain);
            }
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_NICAM_SLIDER))
        {
            NicamGain = Slider_GetPos(GetDlgItem(hDlg, IDC_NICAM_SLIDER));
            SetDlgItemInt(hDlg, IDC_NICAM_EDIT, NicamGain, TRUE);
            pThis->m_pSAA7134Card->SetAudioNicamVolume(NicamGain);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_BLUR_SLIDER))
        {
        }
        break;

    default:
        break;
    }
    return (FALSE);
}

void CSAA7134Source::SetMenu(HMENU hMenu)
{
    int i;
    MENUITEMINFO MenuItemInfo;
    char Buffer[265];

    // set up the input menu
    for(i = 0;i < m_pSAA7134Card->GetNumInputs(); ++i)
    {
        // reset the menu info structure
        memset(&MenuItemInfo, 0, sizeof(MenuItemInfo));
        MenuItemInfo.cbSize = sizeof(MenuItemInfo);
        MenuItemInfo.fMask = MIIM_TYPE;

        // get the size of the string
        GetMenuItemInfo(m_hMenu, IDM_SOURCE_INPUT1 + i, FALSE, &MenuItemInfo);
        // set the buffer and get the current string
        MenuItemInfo.dwTypeData = Buffer;
        GetMenuItemInfo(m_hMenu, IDM_SOURCE_INPUT1 + i, FALSE, &MenuItemInfo);
        // create the new string and correct the menu
        sprintf(Buffer, "%s\tCtrl+Alt+F%d",m_pSAA7134Card->GetInputName(i), i + 1);
        MenuItemInfo.cch = strlen(Buffer);
        SetMenuItemInfo(m_hMenu, IDM_SOURCE_INPUT1 + i, FALSE, &MenuItemInfo);
        
        // enable the menu and check it appropriately
        EnableMenuItem(m_hMenu, IDM_SOURCE_INPUT1 + i, MF_ENABLED);
        CheckMenuItemBool(m_hMenu, IDM_SOURCE_INPUT1 + i, (m_VideoSource->GetValue() == i));
    }
    
    while(i < INPUTS_PER_CARD)
    {
        EnableMenuItem(m_hMenu, IDM_SOURCE_INPUT1 + i, MF_GRAYED);
        ++i;
    }

    BOOL DoneWidth = FALSE;

    EnableMenuItemBool(m_hMenu, IDM_AUDIOSETTINGS1, FALSE);

    EnableMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_768, GetTVFormat((eVideoFormat)m_VideoFormat->GetValue())->wHActivex1 >= 768);

    CheckMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_768, (m_PixelWidth->GetValue() == 768));
    DoneWidth |= (m_PixelWidth->GetValue() == 768);
    CheckMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_754, (m_PixelWidth->GetValue() == 754));
    DoneWidth |= (m_PixelWidth->GetValue() == 754);
    CheckMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_720, (m_PixelWidth->GetValue() == 720));
    DoneWidth |= (m_PixelWidth->GetValue() == 720);
    CheckMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_640, (m_PixelWidth->GetValue() == 640));
    DoneWidth |= (m_PixelWidth->GetValue() == 640);
    CheckMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_480, (m_PixelWidth->GetValue() == 480));
    DoneWidth |= (m_PixelWidth->GetValue() == 480);
    CheckMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_384, (m_PixelWidth->GetValue() == 384));
    DoneWidth |= (m_PixelWidth->GetValue() == 384);
    CheckMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_320, (m_PixelWidth->GetValue() == 320));
    DoneWidth |= (m_PixelWidth->GetValue() == 320);
    CheckMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_CUSTOM, !DoneWidth);

    eVideoFormat videoFormat = (eVideoFormat)m_VideoFormat->GetValue();
    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_0, (IsPALVideoFormat(videoFormat) 
                                                    && videoFormat != VIDEOFORMAT_PAL_M
                                                    && videoFormat != VIDEOFORMAT_PAL_N
                                                    && videoFormat != VIDEOFORMAT_PAL_60
                                                    && videoFormat != VIDEOFORMAT_PAL_N_COMBO
                                                    ));
    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_1, (videoFormat == VIDEOFORMAT_NTSC_M));
    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_2, (IsSECAMVideoFormat(videoFormat)));
    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_3, (videoFormat == VIDEOFORMAT_PAL_M));
    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_4, (videoFormat == VIDEOFORMAT_PAL_N));
    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_5, (videoFormat == VIDEOFORMAT_NTSC_M_Japan));
    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_6, (videoFormat == VIDEOFORMAT_PAL_60));
    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_7, (videoFormat == VIDEOFORMAT_NTSC_50));
    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_8, (videoFormat == VIDEOFORMAT_PAL_N_COMBO));

    CheckMenuItemBool(m_hMenu, IDM_AUDIO_0, (GetCurrentAudioSetting()->GetValue() == 0));
    CheckMenuItemBool(m_hMenu, IDM_AUDIO_1, (GetCurrentAudioSetting()->GetValue() == 1));
    CheckMenuItemBool(m_hMenu, IDM_AUDIO_2, (GetCurrentAudioSetting()->GetValue() == 2));
    CheckMenuItemBool(m_hMenu, IDM_AUDIO_3, (GetCurrentAudioSetting()->GetValue() == 3));
    CheckMenuItemBool(m_hMenu, IDM_AUDIO_4, (GetCurrentAudioSetting()->GetValue() == 4));
    CheckMenuItemBool(m_hMenu, IDM_AUDIO_5, (GetCurrentAudioSetting()->GetValue() == 5));

    CheckMenuItemBool(m_hMenu, IDM_AUDIOSTANDARD_1, (m_AudioStandard->GetValue() == 0));
    CheckMenuItemBool(m_hMenu, IDM_AUDIOSTANDARD_2, (m_AudioStandard->GetValue() == 1));
    CheckMenuItemBool(m_hMenu, IDM_AUDIOSTANDARD_3, (m_AudioStandard->GetValue() == 2));
    CheckMenuItemBool(m_hMenu, IDM_AUDIOSTANDARD_4, (m_AudioStandard->GetValue() == 3));
    CheckMenuItemBool(m_hMenu, IDM_AUDIOSTANDARD_5, (m_AudioStandard->GetValue() == 4));
    CheckMenuItemBool(m_hMenu, IDM_AUDIOSTANDARD_6, (m_AudioStandard->GetValue() == 5));
    CheckMenuItemBool(m_hMenu, IDM_AUDIOSTANDARD_7, (m_AudioStandard->GetValue() == 6));
    CheckMenuItemBool(m_hMenu, IDM_AUDIOSTANDARD_8, (m_AudioStandard->GetValue() == 7));
    CheckMenuItemBool(m_hMenu, IDM_AUDIOSTANDARD_9, (m_AudioStandard->GetValue() == 8));
    CheckMenuItemBool(m_hMenu, IDM_AUDIOSTANDARD_10, (m_AudioStandard->GetValue() == 9));

    EnableMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_MONO, m_pSAA7134Card->IsAudioChannelAvailable(SOUNDCHANNEL_MONO));
    EnableMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_STEREO, m_pSAA7134Card->IsAudioChannelAvailable(SOUNDCHANNEL_STEREO));
    EnableMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_LANGUAGE1, m_pSAA7134Card->IsAudioChannelAvailable(SOUNDCHANNEL_LANGUAGE1));
    EnableMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_LANGUAGE2, m_pSAA7134Card->IsAudioChannelAvailable(SOUNDCHANNEL_LANGUAGE2));

//  Unimplemented
//  CheckMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_MONO, (m_AudioChannel->GetValue() == 1));
//  CheckMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_STEREO, (m_AudioChannel->GetValue() == 2));
//  CheckMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_LANGUAGE1, (m_AudioChannel->GetValue() == 3));
//  CheckMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_LANGUAGE2, (m_AudioChannel->GetValue() == 4));

    CheckMenuItemBool(m_hMenu, IDM_SAA7134CARD_NONSTANDARDSIGNAL, m_NonstandardSignal->GetValue());

    CheckMenuItemBool(m_hMenu, IDM_SAVE_BY_FORMAT, m_bSavePerFormat->GetValue());
    CheckMenuItemBool(m_hMenu, IDM_SAVE_BY_INPUT, m_bSavePerInput->GetValue());
}


BOOL CSAA7134Source::HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam)
{
    switch(LOWORD(wParam))
    {
        case IDM_SETUPCARD:
            Stop_Capture();
            {
                long OrigCard = m_CardType->GetValue();
                long OrigTuner = m_TunerType->GetValue();
                DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_SELECTCARD), hWnd, (DLGPROC) SelectCardProc, (LPARAM)this);
                if (m_CardType->GetValue() != OrigCard)
                {
                    m_pSAA7134Card->SetCardType(m_CardType->GetValue());
                    m_pSAA7134Card->InitAudio();
                    ChangeTVSettingsBasedOnCard();
                }
                if (m_TunerType->GetValue() != OrigTuner)
                {
                    m_pSAA7134Card->InitTuner((eTunerId)m_TunerType->GetValue());
                }   
            }
            Start_Capture();
            break;

        case IDM_BDELAY_PLUS:
            ShowText(hWnd, "BDelay Plus - Unsupported");
            break;

        case IDM_BDELAY_MINUS:
            ShowText(hWnd, "BDelay Minus - Unsupported");
            break;

        case IDM_BDELAY_CURRENT:
            ShowText(hWnd, "BDelay Current - Unsupported");
            break;

        case IDM_HDELAY_PLUS:
            m_HDelay->ChangeValue(ADJUSTUP);
            SendMessage(hWnd, WM_COMMAND, IDM_HDELAY_CURRENT, 0);
            break;

        case IDM_HDELAY_MINUS:
            m_HDelay->ChangeValue(ADJUSTDOWN);
            SendMessage(hWnd, WM_COMMAND, IDM_HDELAY_CURRENT, 0);
            break;

        case IDM_HDELAY_CURRENT:
            m_HDelay->OSDShow();
            break;

        case IDM_VDELAY_PLUS:
            m_VDelay->ChangeValue(ADJUSTUP);
            SendMessage(hWnd, WM_COMMAND, IDM_VDELAY_CURRENT, 0);
            break;

        case IDM_VDELAY_MINUS:
            m_VDelay->ChangeValue(ADJUSTDOWN);
            SendMessage(hWnd, WM_COMMAND, IDM_VDELAY_CURRENT, 0);
            break;

        case IDM_VDELAY_CURRENT:
            m_VDelay->OSDShow();
            break;

        case IDM_PIXELWIDTH_PLUS:
            m_PixelWidth->ChangeValue(ADJUSTUP);
            SendMessage(hWnd, WM_COMMAND, IDM_PIXELWIDTH_CURRENT, 0);
            break;

        case IDM_PIXELWIDTH_MINUS:
            m_PixelWidth->ChangeValue(ADJUSTDOWN);
            SendMessage(hWnd, WM_COMMAND, IDM_PIXELWIDTH_CURRENT, 0);
            break;

        case IDM_PIXELWIDTH_CURRENT:
            m_PixelWidth->OSDShow();
            break;

        case IDM_SOUNDCHANNEL_MONO:
            ShowText(hWnd, "Mono - Unimplemented");
            // m_AudioChannel->SetValue(SOUNDCHANNEL_MONO);
            break;
        case IDM_SOUNDCHANNEL_STEREO:
            ShowText(hWnd, "Stereo - Unimplemented");
            // m_AudioChannel->SetValue(SOUNDCHANNEL_STEREO);
            break;
        case IDM_SOUNDCHANNEL_LANGUAGE1:
            ShowText(hWnd, "Language 1 - Unimplemented");
            // m_AudioChannel->SetValue(SOUNDCHANNEL_LANGUAGE1);
            break;
        case IDM_SOUNDCHANNEL_LANGUAGE2:
            ShowText(hWnd, "Language 2 - Unimplemented");
            // m_AudioChannel->SetValue(SOUNDCHANNEL_LANGUAGE2);
            break;

        case IDM_SAA7134CARD_NONSTANDARDSIGNAL:
            m_NonstandardSignal->SetValue(!m_NonstandardSignal->GetValue());
            m_NonstandardSignal->OSDShow();
            break;

        case IDM_AUDIO_0:
        case IDM_AUDIO_1:
        case IDM_AUDIO_2:
            GetCurrentAudioSetting()->SetValue((LOWORD(wParam) - IDM_AUDIO_0));
            switch (GetCurrentAudioSetting()->GetValue())
            {
            case AUDIOINPUTLINE_TUNER:     
                ShowText(hWnd, "Audio Input - Tuner");
                break;
            case AUDIOINPUTLINE_LINE1: 
                ShowText(hWnd, "Audio Input - Line 1");
                break;
            case AUDIOINPUTLINE_LINE2:  
                ShowText(hWnd, "Audio Input - Line 2");
                break;
            }
            break;

        case IDM_AUDIOSTANDARD_1:
        case IDM_AUDIOSTANDARD_2:
        case IDM_AUDIOSTANDARD_3:
        case IDM_AUDIOSTANDARD_4:
        case IDM_AUDIOSTANDARD_5:
        case IDM_AUDIOSTANDARD_6:
        case IDM_AUDIOSTANDARD_7:
        case IDM_AUDIOSTANDARD_8:
        case IDM_AUDIOSTANDARD_9:
        case IDM_AUDIOSTANDARD_10:
            {
                int nValue = LOWORD(wParam) - IDM_AUDIOSTANDARD_1;
                ShowText(hWnd, m_pSAA7134Card->GetAudioStandardName((eAudioStandard)nValue));
                m_AudioStandard->SetValue(nValue);
            }
            break;

        case IDM_SOURCE_INPUT1:
        case IDM_SOURCE_INPUT2:
        case IDM_SOURCE_INPUT3:
        case IDM_SOURCE_INPUT4:
        case IDM_SOURCE_INPUT5:
        case IDM_SOURCE_INPUT6:
        case IDM_SOURCE_INPUT7:
            {
                int nValue = LOWORD(wParam) - IDM_SOURCE_INPUT1;
                ShowText(hWnd, m_pSAA7134Card->GetInputName(nValue));
                m_VideoSource->SetValue(nValue);
                SendMessage(hWnd, WM_COMMAND, IDM_VT_RESET, 0);
            }
            break;
            
        case IDM_HWINFO:
            DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_HWINFO), hWnd, CSAA7134Card::ChipSettingProc, (LPARAM)m_pSAA7134Card);
            break;

        case IDM_ADV_VIDEOSETTINGS:
            if (m_hSAA7134ResourceInst != NULL)
            {
                DialogBoxParam(m_hSAA7134ResourceInst, "REGISTEREDIT", hWnd, RegisterEditProc, (LPARAM)this);
            }
            else
            {
                ShowText(hWnd, "SAA7134Res.dll not loaded");
            }
            break;

        case IDM_AUDIOSETTINGS:
            if (m_hSAA7134ResourceInst != NULL)
            {
                DialogBoxParam(m_hSAA7134ResourceInst, "OTHEREDIT", hWnd, OtherEditProc, (LPARAM)this);
            }
            else
            {
                ShowText(hWnd, "SAA7134Res.dll not loaded");
            }
            break;

        // Video format (NTSC, PAL, etc)
        case IDM_TYPEFORMAT_0:
            m_VideoFormat->SetValue(VIDEOFORMAT_PAL_B);
            ShowText(hWnd, GetStatus());
            break;
        case IDM_TYPEFORMAT_1:
            m_VideoFormat->SetValue(VIDEOFORMAT_NTSC_M);
            ShowText(hWnd, GetStatus());
            break;
        case IDM_TYPEFORMAT_2:
            m_VideoFormat->SetValue(VIDEOFORMAT_SECAM_B);
            ShowText(hWnd, GetStatus());
            break;
        case IDM_TYPEFORMAT_3:
            m_VideoFormat->SetValue(VIDEOFORMAT_PAL_M);
            ShowText(hWnd, GetStatus());
            break;
        case IDM_TYPEFORMAT_4:
            m_VideoFormat->SetValue(VIDEOFORMAT_PAL_N);
            ShowText(hWnd, GetStatus());
            break;
        case IDM_TYPEFORMAT_5:
            m_VideoFormat->SetValue(VIDEOFORMAT_NTSC_M_Japan);
            ShowText(hWnd, GetStatus());
            break;
        case IDM_TYPEFORMAT_6:
            m_VideoFormat->SetValue(VIDEOFORMAT_PAL_60);
            ShowText(hWnd, GetStatus());
            break;
        case IDM_TYPEFORMAT_7:
            m_VideoFormat->SetValue(VIDEOFORMAT_NTSC_50);
            ShowText(hWnd, GetStatus());
            break;
        case IDM_TYPEFORMAT_8:
            m_VideoFormat->SetValue(VIDEOFORMAT_PAL_N_COMBO);
            ShowText(hWnd, GetStatus());
            break;

        case IDM_SETTINGS_PIXELWIDTH_768:
            m_PixelWidth->SetValue(768);
            break;

        case IDM_SETTINGS_PIXELWIDTH_754:
            m_PixelWidth->SetValue(754);
            break;

        case IDM_SETTINGS_PIXELWIDTH_720:
            m_PixelWidth->SetValue(720);
            break;
    
        case IDM_SETTINGS_PIXELWIDTH_640:
            m_PixelWidth->SetValue(640);
            break;
    
        case IDM_SETTINGS_PIXELWIDTH_480:
            m_PixelWidth->SetValue(480);
            break;
    
        case IDM_SETTINGS_PIXELWIDTH_384:
            m_PixelWidth->SetValue(384);
            break;
    
        case IDM_SETTINGS_PIXELWIDTH_320:
            m_PixelWidth->SetValue(320);
            break;
    
        case IDM_SETTINGS_PIXELWIDTH_CUSTOM:
            m_PixelWidth->SetValue(m_CustomPixelWidth->GetValue());
            break;

        case IDM_SAVE_BY_FORMAT:
            SaveInputSettings(TRUE);
            m_bSavePerFormat->SetValue(!m_bSavePerFormat->GetValue());
            LoadInputSettings();
            break;

        case IDM_SAVE_BY_INPUT:
            SaveInputSettings(TRUE);
            m_bSavePerInput->SetValue(!m_bSavePerInput->GetValue());
            LoadInputSettings();
            break;

        case IDM_SAVE_BY_CHANNEL:
            // Gone           
            break;        
        default:
            return FALSE;
            break;
    }
    return TRUE;
}

void CSAA7134Source::ChangeSectionNamesForInput()
{
    int Input = -1;
    int Format = -1;

    if(m_bSavePerInput->GetValue())
    {
        Input = m_VideoSource->GetValue();
    }

    if(m_bSavePerFormat->GetValue())
    {
        Format = m_VideoFormat->GetValue();
    }

    if(Input == -1 && Format == -1)
    {
        m_Brightness->SetSection(m_Section.c_str());
        m_Contrast->SetSection(m_Section.c_str());
        m_Hue->SetSection(m_Section.c_str());
        m_Saturation->SetSection(m_Section.c_str());
        m_Overscan->SetSection(m_Section.c_str());
    }
    else
    {
        char szSection[128];
        sprintf(szSection, "%s_%d_%d", m_Section.c_str(), Input, Format);
        m_Brightness->SetSection(szSection);
        m_Contrast->SetSection(szSection);
        m_Hue->SetSection(szSection);
        m_Saturation->SetSection(szSection);
        m_Overscan->SetSection(szSection);
    }

    ChangeDefaultsForInput();
}

void CSAA7134Source::ChangeDefaultsForInput()
{
    eVideoFormat format = GetFormat();
    if(IsNTSCVideoFormat(format))
    {
        m_Overscan->ChangeDefault(DEFAULT_OVERSCAN_NTSC);
    }
    else
    {
        m_Overscan->ChangeDefault(DEFAULT_OVERSCAN_PAL);
    }
}


void CSAA7134Source::LoadInputSettings()
{
    ChangeDefaultsForInput();
    
    if (!SettingsPerChannel())
    {
        ChangeSectionNamesForInput();
        m_Brightness->ReadFromIni();
        m_Contrast->ReadFromIni();
        m_Hue->ReadFromIni();
        m_Saturation->ReadFromIni();
        m_Overscan->ReadFromIni();
    }

    ChangeChannelSectionNames();
}

void CSAA7134Source::SaveInputSettings(BOOL bOptimizeFileAccess)
{
    if (!SettingsPerChannel())
    {        
        m_Brightness->WriteToIni(bOptimizeFileAccess);
        m_Contrast->WriteToIni(bOptimizeFileAccess);
        m_Hue->WriteToIni(bOptimizeFileAccess);
        m_Saturation->WriteToIni(bOptimizeFileAccess);
        m_Overscan->WriteToIni(bOptimizeFileAccess);
    }
}


void CSAA7134Source::ChangeChannelSectionNames()
{    
      if (!m_SettingsByChannelStarted)
    {
        return;
    }

    std::string sOldSection = m_ChannelSubSection;
    
    int Input = -1;
    int Format = -1;
    
    if(m_bSavePerInput->GetValue())
    {
        Input = m_VideoSource->GetValue();
    }
    if(m_bSavePerFormat->GetValue())
    {
        Format = m_VideoFormat->GetValue();
    }

    if(Input != -1 || Format != -1)
    {
        char szSection[100];
        sprintf(szSection, "%s_%d_%d", m_Section.c_str(), Input, Format);
        m_ChannelSubSection = szSection;     
    } 
    else
    {
        m_ChannelSubSection = m_Section;
    }
    if (sOldSection != m_ChannelSubSection)
    {
        if (sOldSection.size() > 0)
        {            
            if (m_CurrentChannel >=0)
            {            
                SettingsPerChannel_SaveChannelSettings(sOldSection.c_str(), m_VideoSource->GetValue(), m_CurrentChannel, GetFormat());
            }        
            SettingsPerChannel_UnregisterSection(sOldSection.c_str());
        }
    
        SettingsPerChannel_RegisterSetSection(m_ChannelSubSection.c_str());
        SettingsPerChannel_RegisterSetting("Brightness", "SAA713x - Brightness",TRUE, m_Brightness);
        SettingsPerChannel_RegisterSetting("Hue", "SAA713x - Hue", TRUE, m_Hue);            
        SettingsPerChannel_RegisterSetting("Contrast", "SAA713x - Contrast", TRUE, m_Contrast);
        SettingsPerChannel_RegisterSetting("Saturation","SAA713x - Saturation",TRUE, m_Saturation);
        SettingsPerChannel_RegisterSetting("Overscan", "SAA713x - Overscan", TRUE, m_Overscan);
        
        SettingsPerChannel_RegisterSetting("AudioChannel", "SAA713x - Audio Channel", TRUE, m_AudioChannel);
        SettingsPerChannel_RegisterSetting("NonstandardSignal", "SAA713x - Miscellaneous", TRUE, m_NonstandardSignal);
        
        // SettingsPerChannel_RegisterSetting("Volume","SAA713x - Volume",TRUE, m_Volume);           
        // SettingsPerChannel_RegisterSetting("Balance","SAA713x - Balance",TRUE, m_Balance);
     
        SettingsPerChannel_RegisterSetting("Delays", "SAA713x - H/V Delay", FALSE, m_HDelay);
        SettingsPerChannel_RegisterSetting("Delays", "SAA713x - H/V Delay", FALSE, m_VDelay);
    }
}
