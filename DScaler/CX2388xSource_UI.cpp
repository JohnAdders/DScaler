/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 John Adcock.  All rights reserved.
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
// This code is based on a version of dTV modified by Michael Eskin and
// others at Connexant.  Those parts are probably (c) Connexant 2002
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file CX2388xSource.cpp CCX2388xSource Implementation (UI)
 */

#include "stdafx.h"

#ifdef WANT_CX2388X_SUPPORT

#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "CX2388xSource.h"
#include "CX2388x_Defines.h"
#include "DScaler.h"
#include "Providers.h"
#include "OutThreads.h"
#include "AspectRatio.h"
#include "DebugLog.h"
#include "SettingsPerChannel.h"
#include "SettingsMaster.h"
#include "LibraryCache.h"

using namespace std;

extern const TCHAR* TunerNames[TUNER_LASTONE];

long EnableCxCancelButton = 1;

INT_PTR CALLBACK CCX2388xSource::SelectCardProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    int i;
    int nIndex;
    static CCX2388xSource* pThis;
    CCX2388xCard* pCard = NULL;
    TCHAR szCardId[9] = _T("n/a     ");
    TCHAR szVendorId[9] = _T("n/a ");
    TCHAR szDeviceId[9] = _T("n/a ");
    DWORD dwCardId(0);

    switch (message)
    {
    case WM_INITDIALOG:
        {
            pThis = (CCX2388xSource*)lParam;
            tstring buf(_T("Setup card "));
            buf += pThis->IDString();
            SetWindowText(hDlg, buf.c_str());
            Button_Enable(GetDlgItem(hDlg, IDCANCEL), EnableCxCancelButton);

            SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_RESETCONTENT, 0, 0);
            for(i = 0; i < pThis->m_pCard->GetMaxCards(); i++)
            {
                int nIndex;
                nIndex = SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_ADDSTRING, 0, (LONG)pThis->m_pCard->GetCardName((eCX2388xCardId)i).c_str());
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

            pCard = pThis->GetCard().GetRawPointer();
            SetDlgItemText(hDlg, IDC_BT_CHIP_TYPE, _T("CX2388x"));
            _stprintf(szVendorId,_T("%04X"), pCard->GetVendorId());
            SetDlgItemText(hDlg, IDC_BT_VENDOR_ID, szVendorId);
            _stprintf(szDeviceId,_T("%04X"), pCard->GetDeviceId());
            SetDlgItemText(hDlg, IDC_BT_DEVICE_ID, szDeviceId);

            dwCardId = pCard->GetSubSystemId();
            if(dwCardId != 0 && dwCardId != 0xffffffff)
            {
                _stprintf(szCardId,_T("%8X"), dwCardId);
            }

            SetDlgItemText(hDlg, IDC_AUTODECTECTID, szCardId);

            return TRUE;
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

            // Update the tstring name value to reflect the newly selected card.
            i = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CARDSSELECT), i);
            pThis->m_CardName->SetValue(pThis->GetCard()->GetCardName((eCX2388xCardId) i ).c_str());

            SettingsMaster->SaveAllSettings(TRUE);
            EndDialog(hDlg, TRUE);
            break;
        case IDCANCEL:
            EndDialog(hDlg, TRUE);
            break;
        case IDC_CARDSSELECT:
            i = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_CARDSSELECT));
            i = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CARDSSELECT), i);
            i = pThis->m_pCard->AutoDetectTuner((eCX2388xCardId)i);
            for (nIndex = 0; nIndex < TUNER_LASTONE; nIndex++)
            {
              if (ComboBox_GetItemData(GetDlgItem(hDlg, IDC_TUNERSELECT), nIndex) == i)
              {
                 ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_TUNERSELECT), nIndex);
              }
            }
            break;
        case IDC_AUTODETECT:
            {
                eCX2388xCardId CardId = pThis->m_pCard->AutoDetectCardType();
                eTunerId TunerId = pThis->m_pCard->AutoDetectTuner(CardId);

                SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_RESETCONTENT, 0, 0);
                for(i = 0; i < pThis->m_pCard->GetMaxCards(); i++)
                {
                    int nIndex;
                    nIndex = SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_ADDSTRING, 0, (LONG)pThis->m_pCard->GetCardName((eCX2388xCardId)i).c_str());
                    SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_SETITEMDATA, nIndex, i);
                    if(i == CardId)
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
                SetFocus(hDlg);
                // Update the tuner combobox after the SetFocus
                // because SetFocus modifies this combobox
                for (nIndex = 0; nIndex < TUNER_LASTONE; nIndex++)
                {
                    i = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_TUNERSELECT), nIndex);
                    if (i == TunerId)
                    {
                        SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_SETCURSEL, nIndex, 0);
                    }
                }
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

void CCX2388xSource::SetMenu(HMENU hMenu)
{
    int i;
    MENUITEMINFO MenuItemInfo;

    // set up the input menu
    for(i = 0;i < m_pCard->GetNumInputs(); ++i)
    {
        // reset the menu info structure
        memset(&MenuItemInfo, 0, sizeof(MenuItemInfo));
        MenuItemInfo.cbSize = sizeof(MenuItemInfo);
        MenuItemInfo.fMask = MIIM_TYPE;

        // get the size of the tstring
        GetMenuItemInfo(m_hMenu, IDM_SOURCE_INPUT1 + i, FALSE, &MenuItemInfo);
        GetMenuItemInfo(m_hMenu, IDM_SOURCE_INPUT1 + i, FALSE, &MenuItemInfo);
        // create the new tstring and correct the menu
        tstring Buffer(MakeString() << m_pCard->GetInputName(i) << _T("\tCtrl+Alt+F") << i + 1);
        MenuItemInfo.cch = Buffer.length();
        MenuItemInfo.dwTypeData = &Buffer[0];
        SetMenuItemInfo(m_hMenu, IDM_SOURCE_INPUT1 + i, FALSE, &MenuItemInfo);

        // enable the menu and check it appropriately
        //EnableMenuItem(m_hMenu, IDM_SOURCE_INPUT1 + i, MF_ENABLED);
        EnableMenuItem(m_hMenu, IDM_SOURCE_INPUT1 + i, (m_TunerType->GetValue() == TUNER_ABSENT && m_pCard->IsInputATuner(i)) ? MF_GRAYED : MF_ENABLED);
        CheckMenuItemBool(m_hMenu, IDM_SOURCE_INPUT1 + i, (m_VideoSource->GetValue() == i));
    }

    while(i < CX_INPUTS_PER_CARD)
    {
        EnableMenuItem(m_hMenu, IDM_SOURCE_INPUT1 + i, MF_GRAYED);
        ++i;
    }

    BOOL DoneWidth = FALSE;

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

    // grey out formats if in Tuner mode as the format is saved
    // in the channel settings
    EnableMenuItemBool(m_hMenu, IDM_TYPEFORMAT_0, !IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_TYPEFORMAT_1, !IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_TYPEFORMAT_2, !IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_TYPEFORMAT_3, !IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_TYPEFORMAT_4, !IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_TYPEFORMAT_5, !IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_TYPEFORMAT_6, !IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_TYPEFORMAT_7, !IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_TYPEFORMAT_8, !IsInTunerMode());

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

    if(m_pCard->IsThisCardH3D((eCX2388xCardId)m_CardType->GetValue()))
    {
        CheckMenuItemBool(m_hMenu, IDM_PROGRESSIVE, m_IsVideoProgressive->GetValue());
        CheckMenuItemBool(m_hMenu, IDM_FLI_FILMDETECT, m_FLIFilmDetect->GetValue());
        EnableMenuItem(m_hMenu, IDM_PROGRESSIVE, MF_ENABLED);
        EnableMenuItem(m_hMenu, IDM_FLI_FILMDETECT, MF_ENABLED);
    }
    else
    {
        EnableMenuItem(m_hMenu, IDM_PROGRESSIVE, MF_GRAYED);
        EnableMenuItem(m_hMenu, IDM_FLI_FILMDETECT, MF_GRAYED);
    }

    CheckMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_MONO,            (m_StereoType->GetValue() == STEREOTYPE_MONO  ));
    CheckMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_STEREO,            (m_StereoType->GetValue() == STEREOTYPE_STEREO));
    CheckMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_LANGUAGE1,        (m_StereoType->GetValue() == STEREOTYPE_ALT1  ));
    CheckMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_LANGUAGE2,        (m_StereoType->GetValue() == STEREOTYPE_ALT2  ));
    CheckMenuItemBool(m_hMenu, IDM_AUTOSTEREO,                    (m_StereoType->GetValue() == STEREOTYPE_AUTO  ));
    EnableMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_MONO,            IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_STEREO,        IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_LANGUAGE1,        IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_LANGUAGE2,        IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_AUTOSTEREO,                    IsInTunerMode());

    CheckMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_AUTO,        (m_AudioStandard->GetValue() == AUDIO_STANDARD_AUTO            ));
    CheckMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_A2,        (m_AudioStandard->GetValue() == AUDIO_STANDARD_A2            ));
    CheckMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_BTSC,        (m_AudioStandard->GetValue() == AUDIO_STANDARD_BTSC            ));
    CheckMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_BTSC_SAP,    (m_AudioStandard->GetValue() == AUDIO_STANDARD_BTSC_SAP        ));
    CheckMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_EIAJ,        (m_AudioStandard->GetValue() == AUDIO_STANDARD_EIAJ            ));
    CheckMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_FM,        (m_AudioStandard->GetValue() == AUDIO_STANDARD_FM            ));
    CheckMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_NICAM,        (m_AudioStandard->GetValue() == AUDIO_STANDARD_NICAM        ));
    EnableMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_AUTO,        IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_A2,        IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_BTSC,        IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_BTSC_SAP,    IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_EIAJ,        IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_FM,        IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_NICAM,    IsInTunerMode());
}

BOOL CCX2388xSource::HandleWindowsCommands(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    switch(LOWORD(wParam))
    {
        case IDM_SETUPCARD:
            Stop_Capture();
            DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_SELECTCARD), hWnd, (DLGPROC) SelectCardProc, (LPARAM)this);
            m_pCard->SetCardType(m_CardType->GetValue());
            m_pCard->InitTuner((eTunerId)m_TunerType->GetValue());
            Start_Capture();
            break;

        case IDM_HWINFO:
            DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_HWINFO), hWnd, CCX2388xCard::ChipSettingProc, (LPARAM)(CCX2388xCard*)m_pCard.GetRawPointer());
            break;

        case IDM_SOURCE_INPUT1:
        case IDM_SOURCE_INPUT2:
        case IDM_SOURCE_INPUT3:
        case IDM_SOURCE_INPUT4:
        case IDM_SOURCE_INPUT5:
        case IDM_SOURCE_INPUT6:
        case IDM_SOURCE_INPUT7:
        case IDM_SOURCE_INPUT8:
        case IDM_SOURCE_INPUT9:
            {
                int nValue = LOWORD(wParam) - IDM_SOURCE_INPUT1;
                if (nValue < m_pCard->GetNumInputs())
                {
                    if (m_TunerType->GetValue() != TUNER_ABSENT || !m_pCard->IsInputATuner(nValue))
                    {
                        ShowText(hWnd, m_pCard->GetInputName(nValue));
                        SetTrayTip(m_pCard->GetInputName(nValue));
                        m_VideoSource->SetValue(nValue);
                        SendMessage(hWnd, WM_COMMAND, IDM_VT_RESET, 0);
                    }
                }
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

        case IDM_PROGRESSIVE:
            m_IsVideoProgressive->SetValue(!m_IsVideoProgressive->GetValue());
            if(m_IsVideoProgressive->GetValue())
            {
                ShowText(hWnd, _T("Using Faroudja Deinterlacing"));
            }
            else
            {
                ShowText(hWnd, _T("Using DScaler Deinterlacing"));
            }
            break;

        case IDM_FLI_FILMDETECT:
            m_FLIFilmDetect->SetValue(!m_FLIFilmDetect->GetValue());
            if(m_FLIFilmDetect->GetValue())
            {
                ShowText(hWnd, _T("FLI2200 Film Detection - On"));
            }
            else
            {
                ShowText(hWnd, _T("FLI2200 Film Detection - Off"));
            }
            break;

        case IDM_HDELAY_PLUS:
            m_HDelay->ChangeValue(ADJUSTUP_SILENT);
            SendMessage(hWnd, WM_COMMAND, IDM_HDELAY_CURRENT, 0);
            break;

        case IDM_HDELAY_MINUS:
            m_HDelay->ChangeValue(ADJUSTDOWN_SILENT);
            SendMessage(hWnd, WM_COMMAND, IDM_HDELAY_CURRENT, 0);
            break;

        case IDM_HDELAY_CURRENT:
            m_HDelay->OSDShow();
            break;

        case IDM_VDELAY_PLUS:
            m_VDelay->ChangeValue(ADJUSTUP_SILENT);
            SendMessage(hWnd, WM_COMMAND, IDM_VDELAY_CURRENT, 0);
            break;

        case IDM_VDELAY_MINUS:
            m_VDelay->ChangeValue(ADJUSTDOWN_SILENT);
            SendMessage(hWnd, WM_COMMAND, IDM_VDELAY_CURRENT, 0);
            break;

        case IDM_VDELAY_CURRENT:
            m_VDelay->OSDShow();
            break;

        case IDM_DSVIDEO_STANDARD_0:
            // _T("Custom Settings ...") menu
            if (m_hCX2388xResourceInst != NULL)
            {
                m_pCard->ShowRegisterSettingsDialog(m_hCX2388xResourceInst);
            }
            else
            {
                ShowText(hWnd, _T("CX2388xRes.dll not loaded"));
            }
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

        case IDM_PIXELWIDTH_PLUS:
            m_PixelWidth->ChangeValue(ADJUSTUP_SILENT);
            SendMessage(hWnd, WM_COMMAND, IDM_PIXELWIDTH_CURRENT, 0);
            break;

        case IDM_PIXELWIDTH_MINUS:
            m_PixelWidth->ChangeValue(ADJUSTDOWN_SILENT);
            SendMessage(hWnd, WM_COMMAND, IDM_PIXELWIDTH_CURRENT, 0);
            break;

        case IDM_PIXELWIDTH_CURRENT:
            m_PixelWidth->OSDShow();
            break;

        case IDM_SOUNDCHANNEL_MONO:
            m_StereoType->SetValue((eCX2388xStereoType)STEREOTYPE_MONO);
            break;

        case IDM_SOUNDCHANNEL_STEREO:
            m_StereoType->SetValue((eCX2388xStereoType)STEREOTYPE_STEREO);
            break;

        case IDM_SOUNDCHANNEL_LANGUAGE1:
            m_StereoType->SetValue((eCX2388xStereoType)STEREOTYPE_ALT1);
            break;

        case IDM_SOUNDCHANNEL_LANGUAGE2:
            m_StereoType->SetValue((eCX2388xStereoType)STEREOTYPE_ALT2);
            break;

        case IDM_AUTOSTEREO:
            m_StereoType->SetValue((eCX2388xStereoType)STEREOTYPE_AUTO);
            break;

        case IDM_CX2388X_AUDIO_STD_AUTO:
            m_AudioStandard->SetValue((eCX2388xAudioStandard)AUDIO_STANDARD_AUTO);
            break;

        case IDM_CX2388X_AUDIO_STD_A2:
            m_AudioStandard->SetValue((eCX2388xAudioStandard)AUDIO_STANDARD_A2);
            break;

        case IDM_CX2388X_AUDIO_STD_BTSC:
            m_AudioStandard->SetValue((eCX2388xAudioStandard)AUDIO_STANDARD_BTSC);
            break;

        case IDM_CX2388X_AUDIO_STD_BTSC_SAP:
            m_AudioStandard->SetValue((eCX2388xAudioStandard)AUDIO_STANDARD_BTSC_SAP);
            break;

        case IDM_CX2388X_AUDIO_STD_EIAJ:
            m_AudioStandard->SetValue((eCX2388xAudioStandard)AUDIO_STANDARD_EIAJ);
            break;

        case IDM_CX2388X_AUDIO_STD_FM:
            m_AudioStandard->SetValue((eCX2388xAudioStandard)AUDIO_STANDARD_FM);
            break;

        case IDM_CX2388X_AUDIO_STD_NICAM:
            m_AudioStandard->SetValue((eCX2388xAudioStandard)AUDIO_STANDARD_NICAM);
            break;

        default:
            return FALSE;
            break;
    }
    return TRUE;
}

void CCX2388xSource::ChangeDefaultsForVideoInput(BOOL bDontSetValue)
{
    if(m_pCard->IsThisCardH3D((eCX2388xCardId)m_CardType->GetValue()))
    {
        m_Brightness->ChangeDefault(128, bDontSetValue);
        m_Contrast->ChangeDefault(128, bDontSetValue);
        m_Hue->ChangeDefault(128, bDontSetValue);
        m_Saturation->ChangeDefault(128, bDontSetValue);
        m_SaturationU->ChangeDefault(128, bDontSetValue);
        m_SaturationV->ChangeDefault(128, bDontSetValue);
        m_IsVideoProgressive->ChangeDefault(TRUE, bDontSetValue);
    }
    else
    {
        m_Brightness->ChangeDefault(128, bDontSetValue);
        m_Contrast->ChangeDefault(0x39, bDontSetValue);
        m_Hue->ChangeDefault(128, bDontSetValue);
        m_Saturation->ChangeDefault((0x7f + 0x5A) / 2, bDontSetValue);
        m_SaturationU->ChangeDefault(0x7f, bDontSetValue);
        m_SaturationV->ChangeDefault(0x5A, bDontSetValue);
        m_IsVideoProgressive->ChangeDefault(FALSE, bDontSetValue);
    }
}

void CCX2388xSource::ChangeDefaultsForVideoFormat(BOOL bDontSetValue)
{
    eVideoFormat format = GetFormat();
    if(IsNTSCVideoFormat(format))
    {
        m_TopOverscan->ChangeDefault(DEFAULT_OVERSCAN_NTSC, bDontSetValue);
        m_BottomOverscan->ChangeDefault(DEFAULT_OVERSCAN_NTSC, bDontSetValue);
        m_LeftOverscan->ChangeDefault(DEFAULT_OVERSCAN_NTSC, bDontSetValue);
        m_RightOverscan->ChangeDefault(DEFAULT_OVERSCAN_NTSC, bDontSetValue);
    }
    else if(IsSECAMVideoFormat(format))
    {
        // Suggested colour values (PixelView XCapture)
        m_Saturation->ChangeDefault(57, bDontSetValue);
        m_SaturationU->ChangeDefault(64, bDontSetValue);
        m_SaturationV->ChangeDefault(51, bDontSetValue);

        m_TopOverscan->ChangeDefault(DEFAULT_OVERSCAN_PAL, bDontSetValue);
        m_BottomOverscan->ChangeDefault(DEFAULT_OVERSCAN_PAL, bDontSetValue);
        m_LeftOverscan->ChangeDefault(DEFAULT_OVERSCAN_PAL, bDontSetValue);
        m_RightOverscan->ChangeDefault(DEFAULT_OVERSCAN_PAL, bDontSetValue);
    }
    else
    {
        // Laurent's suggested colour values
        m_Saturation->ChangeDefault(133, bDontSetValue);
        m_SaturationU->ChangeDefault(154, bDontSetValue);
        m_SaturationV->ChangeDefault(112, bDontSetValue);
        // \todo add in correct default video values for PAL

        m_TopOverscan->ChangeDefault(DEFAULT_OVERSCAN_PAL, bDontSetValue);
        m_BottomOverscan->ChangeDefault(DEFAULT_OVERSCAN_PAL, bDontSetValue);
        m_LeftOverscan->ChangeDefault(DEFAULT_OVERSCAN_PAL, bDontSetValue);
        m_RightOverscan->ChangeDefault(DEFAULT_OVERSCAN_PAL, bDontSetValue);
    }
}

SmartPtr<CSettingsHolder> CCX2388xSource::GetSettingsPage()
{
    SmartPtr<CSettingsHolder> Holder(new CSettingsHolder);

    Holder->AddSetting(m_HDelay.DynamicCast<CSimpleSetting>());
    Holder->AddSetting(m_VDelay.DynamicCast<CSimpleSetting>());
    Holder->AddSetting(m_AnalogueBlanking.DynamicCast<CSimpleSetting>());
    Holder->AddSetting(m_ConexantStopDriver.DynamicCast<CSimpleSetting>());

    if(m_pCard->IsThisCardH3D((eCX2388xCardId)m_CardType->GetValue()))
    {
        Holder->AddSetting(m_EatLinesAtTop.DynamicCast<CSimpleSetting>());
        Holder->AddSetting(m_Sharpness.DynamicCast<CSimpleSetting>());
    }
    else
    {
        Holder->AddSetting(m_LumaAGC.DynamicCast<CSimpleSetting>());
        Holder->AddSetting(m_ChromaAGC.DynamicCast<CSimpleSetting>());
        Holder->AddSetting(m_FastSubcarrierLock.DynamicCast<CSimpleSetting>());
        Holder->AddSetting(m_WhiteCrush.DynamicCast<CSimpleSetting>());
        Holder->AddSetting(m_LowColorRemoval.DynamicCast<CSimpleSetting>());
        Holder->AddSetting(m_CombFilter.DynamicCast<CSimpleSetting>());
        Holder->AddSetting(m_FullLumaRange.DynamicCast<CSimpleSetting>());
        Holder->AddSetting(m_Remodulation.DynamicCast<CSimpleSetting>());
        Holder->AddSetting(m_Chroma2HComb.DynamicCast<CSimpleSetting>());
        Holder->AddSetting(m_ForceRemodExcessChroma.DynamicCast<CSimpleSetting>());
        Holder->AddSetting(m_IFXInterpolation.DynamicCast<CSimpleSetting>());
        Holder->AddSetting(m_CombRange.DynamicCast<CSimpleSetting>());
        Holder->AddSetting(m_SecondChromaDemod.DynamicCast<CSimpleSetting>());
        Holder->AddSetting(m_ThirdChromaDemod.DynamicCast<CSimpleSetting>());
        Holder->AddSetting(m_WhiteCrushUp.DynamicCast<CSimpleSetting>());
        Holder->AddSetting(m_WhiteCrushDown.DynamicCast<CSimpleSetting>());
        Holder->AddSetting(m_WhiteCrushMajorityPoint.DynamicCast<CSimpleSetting>());
        Holder->AddSetting(m_WhiteCrushPerFrame.DynamicCast<CSimpleSetting>());
        Holder->AddSetting(m_Volume.DynamicCast<CSimpleSetting>());
        Holder->AddSetting(m_Balance.DynamicCast<CSimpleSetting>());
        Holder->AddSetting(m_AudioStandard.DynamicCast<CSimpleSetting>());
        Holder->AddSetting(m_StereoType.DynamicCast<CSimpleSetting>());
        Holder->AddSetting(m_AutoMute.DynamicCast<CSimpleSetting>());
        Holder->AddSetting(m_VerticalSyncDetection.DynamicCast<CSimpleSetting>());
    }

    return Holder;
}

void CCX2388xSource::InitializeUI()
{
    MENUITEMINFO    MenuItemInfo;
    HMENU           hSubMenu;
    LPTSTR           pMenuName;

    m_hCX2388xResourceInst = LibraryCache::GetLibraryHandle(_T("CX2388xRes.dll"));

    if(m_hCX2388xResourceInst != NULL)
    {
        hSubMenu = GetSubMenu(m_hMenu, 0);

        // Set up two separators with the Custom Settings ...
        // menu in between before listing the standards.
        MenuItemInfo.cbSize = sizeof(MenuItemInfo);
        MenuItemInfo.fMask = MIIM_TYPE;
        MenuItemInfo.fType = MFT_SEPARATOR;

        pMenuName = _T("Custom Settings ...");
        MenuItemInfo.fMask = MIIM_TYPE | MIIM_ID;
        MenuItemInfo.fType = MFT_STRING;
        MenuItemInfo.dwTypeData = pMenuName;
        MenuItemInfo.cch = _tcslen(pMenuName);
        MenuItemInfo.wID = IDM_DSVIDEO_STANDARD_0;
        InsertMenuItem(hSubMenu, 5, TRUE, &MenuItemInfo);
    }
}

#endif // WANT_CX2388X_SUPPORT
