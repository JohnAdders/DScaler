/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Souce_UI.cpp,v 1.4 2001-11-09 12:42:07 adcockj Exp $
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
// Revision 1.3  2001/11/02 17:03:59  adcockj
// Merge in PAL_NC change again
//
// Revision 1.2  2001/11/02 16:30:07  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.1.2.3  2001/08/22 18:38:31  adcockj
// Fixed Recursive bug
//
// Revision 1.1.2.2  2001/08/21 16:42:16  adcockj
// Per format/input settings and ini file fixes
//
// Revision 1.1.2.1  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BT848Source.h"
#include "BT848_Defines.h"
#include "DScaler.h"
#include "OutThreads.h"

extern const TCardSetup TVCards[TVCARD_LASTONE];
extern const TTunerSetup Tuners[TUNER_LASTONE];

BOOL APIENTRY CBT848Source::AudioSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    static BOOL TSuperBass;
    static int  TVolume;
    static char TBalance;
    static char TSpatial;
    static char TLoudness;
    static char TBass;
    static char TTreble;
    static CBT848Source* pThis;

    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CBT848Source*)lParam;
        if (pThis->m_pBT848Card->HasMSP() == FALSE)
        {
            ErrorBox("No MSP Audio Device found");
            EndDialog(hDlg, 0);
        }

        TVolume = pThis->m_Volume->GetValue();
        TSpatial = pThis->m_Spatial->GetValue();
        TLoudness = pThis->m_Loudness->GetValue();
        TBass = pThis->m_Bass->GetValue();
        TTreble = pThis->m_Treble->GetValue();
        TBalance = pThis->m_Balance->GetValue();
        TSuperBass = pThis->m_SuperBass->GetValue();

        SetDlgItemInt(hDlg, IDC_D1, TVolume, FALSE);
        SetDlgItemInt(hDlg, IDC_D2, TSpatial, TRUE);
        SetDlgItemInt(hDlg, IDC_D3, TLoudness, TRUE);
        SetDlgItemInt(hDlg, IDC_D4, TBass, TRUE);
        SetDlgItemInt(hDlg, IDC_D5, TTreble, TRUE);
        SetDlgItemInt(hDlg, IDC_D6, TBalance, TRUE);

        CheckDlgButton(hDlg, IDC_CHECK1, TSuperBass);

        pThis->m_Volume->SetupControl(GetDlgItem(hDlg, IDC_SLIDER1));
        pThis->m_Spatial->SetupControl(GetDlgItem(hDlg, IDC_SLIDER2));
        pThis->m_Loudness->SetupControl(GetDlgItem(hDlg, IDC_SLIDER3));
        pThis->m_Bass->SetupControl(GetDlgItem(hDlg, IDC_SLIDER4));
        pThis->m_Treble->SetupControl(GetDlgItem(hDlg, IDC_SLIDER5));
        pThis->m_Balance->SetupControl(GetDlgItem(hDlg, IDC_SLIDER6));
        break;

    case WM_VSCROLL:
    case WM_HSCROLL:
        if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER1))
        {
            pThis->m_Volume->SetFromControl((HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D1, pThis->m_Volume->GetValue(), TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER2))
        {
            pThis->m_Spatial->SetFromControl((HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D2, pThis->m_Spatial->GetValue(), TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER3))
        {
            pThis->m_Loudness->SetFromControl((HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D3, pThis->m_Loudness->GetValue(), TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER4))
        {
            pThis->m_Bass->SetFromControl((HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D4, pThis->m_Bass->GetValue(), TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER5))
        {
            pThis->m_Treble->SetFromControl((HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D5, pThis->m_Treble->GetValue(), TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER6))
        {
            pThis->m_Balance->SetFromControl((HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D5, pThis->m_Balance->GetValue(), TRUE);
        }
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_CHECK1:
            pThis->m_SuperBass->SetValue(IsDlgButtonChecked(hDlg, IDC_CHECK1));
            break;

        case IDOK:
			WriteSettingsToIni(TRUE);
            EndDialog(hDlg, TRUE);
            break;
        
        case IDCANCEL:
            pThis->Mute();
            pThis->m_SuperBass->SetValue(TSuperBass);
            pThis->m_Volume->SetValue(TVolume);
            pThis->m_Spatial->SetValue(TSpatial);
            pThis->m_Loudness->SetValue(TLoudness);
            pThis->m_Bass->SetValue(TBass);
            pThis->m_Treble->SetValue(TTreble);
            pThis->m_Balance->SetValue(TBalance);
            pThis->UnMute();
            EndDialog(hDlg, TRUE);
            break;
        
        case IDC_DEFAULT:
            pThis->Mute();
            pThis->m_SuperBass->SetDefault();
            pThis->m_Volume->SetDefault();
            pThis->m_Spatial->SetDefault();
            pThis->m_Loudness->SetDefault();
            pThis->m_Bass->SetDefault();
            pThis->m_Treble->SetDefault();
            pThis->m_Balance->SetDefault();
            Button_SetCheck(GetDlgItem(hDlg, IDC_CHECK1), pThis->m_SuperBass->GetValue()?BST_CHECKED:BST_UNCHECKED);
            SetDlgItemInt(hDlg, IDC_D1, pThis->m_Volume->GetValue(), FALSE);
            SetDlgItemInt(hDlg, IDC_D2, pThis->m_Spatial->GetValue(), TRUE);
            SetDlgItemInt(hDlg, IDC_D3, pThis->m_Loudness->GetValue(), TRUE);
            SetDlgItemInt(hDlg, IDC_D4, pThis->m_Bass->GetValue(), TRUE);
            SetDlgItemInt(hDlg, IDC_D5, pThis->m_Treble->GetValue(), TRUE);
            SetDlgItemInt(hDlg, IDC_D6, pThis->m_Balance->GetValue(), TRUE);

            pThis->m_Volume->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER1));
            pThis->m_Spatial->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER2));
            pThis->m_Loudness->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER3));
            pThis->m_Bass->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER4));
            pThis->m_Treble->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER5));
            pThis->m_Balance->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER6));
            pThis->UnMute();
            break;
        default:
            break;

        }
        break;
    }
    return (FALSE);
}

BOOL APIENTRY CBT848Source::AudioSettingProc1(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    static char TEqualizer[5] = { 0, 0, 0, 0, 0};
    static CBT848Source* pThis;

    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CBT848Source*)lParam;
        if (pThis->m_pBT848Card->HasMSP() == FALSE)
        {
            ErrorBox("No MSP Audio Device found");
            EndDialog(hDlg, 0);
        }

        pThis->m_Equalizer1->SetupControl(GetDlgItem(hDlg, IDC_SLIDER1));
        TEqualizer[0] = pThis->m_Equalizer1->GetValue();
        SetDlgItemInt(hDlg, IDC_D1, pThis->m_Equalizer1->GetValue(), TRUE);

        pThis->m_Equalizer2->SetupControl(GetDlgItem(hDlg, IDC_SLIDER2));
        TEqualizer[1] = pThis->m_Equalizer2->GetValue();
        SetDlgItemInt(hDlg, IDC_D2, pThis->m_Equalizer1->GetValue(), TRUE);

        pThis->m_Equalizer3->SetupControl(GetDlgItem(hDlg, IDC_SLIDER3));
        TEqualizer[2] = pThis->m_Equalizer3->GetValue();
        SetDlgItemInt(hDlg, IDC_D3, pThis->m_Equalizer1->GetValue(), TRUE);

        pThis->m_Equalizer4->SetupControl(GetDlgItem(hDlg, IDC_SLIDER4));
        TEqualizer[3] = pThis->m_Equalizer4->GetValue();
        SetDlgItemInt(hDlg, IDC_D4, pThis->m_Equalizer1->GetValue(), TRUE);

        pThis->m_Equalizer5->SetupControl(GetDlgItem(hDlg, IDC_SLIDER5));
        TEqualizer[4] = pThis->m_Equalizer5->GetValue();
        SetDlgItemInt(hDlg, IDC_D5, pThis->m_Equalizer1->GetValue(), TRUE);
        break;
    case WM_VSCROLL:
    case WM_HSCROLL:
        if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER1))
        {
            pThis->m_Equalizer1->SetFromControl((HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D1, pThis->m_Equalizer1->GetValue(), TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER2))
        {
            pThis->m_Equalizer2->SetFromControl((HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D2, pThis->m_Equalizer2->GetValue(), TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER3))
        {
            pThis->m_Equalizer3->SetFromControl((HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D3, pThis->m_Equalizer3->GetValue(), TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER4))
        {
            pThis->m_Equalizer4->SetFromControl((HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D4, pThis->m_Equalizer4->GetValue(), TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER5))
        {
            pThis->m_Equalizer5->SetFromControl((HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D5, pThis->m_Equalizer5->GetValue(), TRUE);
        }
        break;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
			WriteSettingsToIni(TRUE);
            EndDialog(hDlg, TRUE);
            break;

        case IDCANCEL:
            pThis->Mute();
            pThis->m_Equalizer1->SetValue(TEqualizer[0]);
            pThis->m_Equalizer2->SetValue(TEqualizer[1]);
            pThis->m_Equalizer3->SetValue(TEqualizer[2]);
            pThis->m_Equalizer4->SetValue(TEqualizer[3]);
            pThis->m_Equalizer5->SetValue(TEqualizer[4]);
            pThis->UnMute();
            EndDialog(hDlg, TRUE);
            break;

        case IDC_DEFAULT:
            pThis->Mute();
            pThis->m_Equalizer1->SetDefault();
            pThis->m_Equalizer2->SetDefault();
            pThis->m_Equalizer3->SetDefault();
            pThis->m_Equalizer4->SetDefault();
            pThis->m_Equalizer5->SetDefault();

            SetDlgItemInt(hDlg, IDC_D1, pThis->m_Equalizer1->GetValue(), TRUE);
            SetDlgItemInt(hDlg, IDC_D2, pThis->m_Equalizer1->GetValue(), TRUE);
            SetDlgItemInt(hDlg, IDC_D3, pThis->m_Equalizer1->GetValue(), TRUE);
            SetDlgItemInt(hDlg, IDC_D4, pThis->m_Equalizer1->GetValue(), TRUE);
            SetDlgItemInt(hDlg, IDC_D5, pThis->m_Equalizer1->GetValue(), TRUE);

            pThis->m_Equalizer1->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER1));
            pThis->m_Equalizer2->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER2));
            pThis->m_Equalizer3->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER3));
            pThis->m_Equalizer4->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER4));
            pThis->m_Equalizer5->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER5));

            pThis->UnMute();
            break;
        }

        break;
    }

    return (FALSE);
}



BOOL APIENTRY CBT848Source::SelectCardProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    int i;
    static long OrigProcessorSpeed;
    static long OrigTradeOff;
    static long OrigTuner;
    static CBT848Source* pThis;

    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CBT848Source*)lParam;
        SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_RESETCONTENT, 0, 0);
        for(i = 0; i < TVCARD_LASTONE; i++)
        {
            int nIndex;
            nIndex = SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_ADDSTRING, 0, (LONG)TVCards[i].szName);
            SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_SETITEMDATA, nIndex, i);
            if(i == pThis->m_CardType->GetValue())
            {
                SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_SETCURSEL, nIndex, 0);
            }
        }

        SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_RESETCONTENT, 0, 0);
        for(i = 0; i < TUNER_LASTONE; i++)
        {
            SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_ADDSTRING, 0, (LONG)Tuners[i].szName);
        }
        SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_SETCURSEL, pThis->m_TunerType->GetValue(), 0);

        SendMessage(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED), CB_ADDSTRING, 0, (LONG)"Above 500 MHz");
        SendMessage(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED), CB_ADDSTRING, 0, (LONG)"300 - 500 MHz");
        SendMessage(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED), CB_ADDSTRING, 0, (LONG)"Below 300 MHz");
        SendMessage(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED), CB_SETCURSEL, pThis->m_ProcessorSpeed->GetValue(), 0);
        SendMessage(GetDlgItem(hDlg, IDC_TRADEOFF), CB_ADDSTRING, 0, (LONG)"Show all frames - Lowest judder");
        SendMessage(GetDlgItem(hDlg, IDC_TRADEOFF), CB_ADDSTRING, 0, (LONG)"Best picture quality");
        SendMessage(GetDlgItem(hDlg, IDC_TRADEOFF), CB_SETCURSEL, pThis->m_TradeOff->GetValue(), 0);
        OrigProcessorSpeed = pThis->m_ProcessorSpeed->GetValue();
        OrigTradeOff = pThis->m_TradeOff->GetValue();
        OrigTuner = pThis->m_TunerType->GetValue();
        SetFocus(hDlg);
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            pThis->m_TunerType->SetValue(SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_GETCURSEL, 0, 0));
            i =  SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_GETCURSEL, 0, 0);
            pThis->m_CardType->SetValue(ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CARDSSELECT), i));
            pThis->m_ProcessorSpeed->SetValue(ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED)));
            pThis->m_TradeOff->SetValue(ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_TRADEOFF)));
            if(OrigProcessorSpeed != pThis->m_ProcessorSpeed->GetValue() || 
                OrigTradeOff != pThis->m_TradeOff->GetValue())
            {
                pThis->ChangeDefaultsBasedOnHardware();
            }
            if(OrigTuner != pThis->m_TunerType->GetValue())
            {
                pThis->ChangeTVSettingsBasedOnTuner();
            }
			WriteSettingsToIni(FALSE);
            EndDialog(hDlg, TRUE);
            break;
        case IDCANCEL:
            EndDialog(hDlg, TRUE);
            break;
        case IDC_CARDSSELECT:
            i = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_CARDSSELECT));
            i = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CARDSSELECT), i);
            if(TVCards[i].TunerId == TUNER_USER_SETUP)
            {
                ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_TUNERSELECT), TUNER_ABSENT);
            }
            else if(TVCards[i].TunerId == TUNER_AUTODETECT)
            {
                ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_TUNERSELECT), pThis->m_pBT848Card->AutoDetectTuner((eTVCardId)i));
            }
            else
            {
                ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_TUNERSELECT), TVCards[i].TunerId);
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

void CBT848Source::SetMenu(HMENU hMenu)
{
    int NumExtraInputs;

    // need to keep track of how many other inputs there are
    // there is always one composite input
    NumExtraInputs = TVCards[m_CardType->GetValue()].nVideoInputs  - 1;

    if(TVCards[m_CardType->GetValue()].SVideoInput == -1)
    {
        EnableMenuItem(m_hMenu, IDM_SOURCE_SVIDEO, MF_GRAYED);
        EnableMenuItem(m_hMenu, IDM_SOURCE_COMPVIASVIDEO, MF_GRAYED);
    }
    else
    {
        EnableMenuItem(m_hMenu, IDM_SOURCE_SVIDEO, MF_ENABLED);
        EnableMenuItem(m_hMenu, IDM_SOURCE_COMPVIASVIDEO, MF_ENABLED);
        // we've used up one more input
        --NumExtraInputs;
    }

    if(HasTuner())
    {
        EnableMenuItem(m_hMenu, IDM_SOURCE_TUNER, MF_ENABLED);
        // we've used up one more input
        --NumExtraInputs;
    }
    else
    {
        EnableMenuItem(m_hMenu, IDM_SOURCE_TUNER, MF_GRAYED);
    }

    EnableMenuItem(m_hMenu, IDM_SOURCE_OTHER1, (NumExtraInputs > 0)?MF_ENABLED:MF_GRAYED);
    EnableMenuItem(m_hMenu, IDM_SOURCE_OTHER2, (NumExtraInputs > 1)?MF_ENABLED:MF_GRAYED);

    EnableMenuItem(m_hMenu, IDM_SOURCE_CCIR656_1, (m_CardType->GetValue() == TVCARD_RS_BT)?MF_ENABLED:MF_GRAYED);
    EnableMenuItem(m_hMenu, IDM_SOURCE_CCIR656_2, (m_CardType->GetValue() == TVCARD_RS_BT)?MF_ENABLED:MF_GRAYED);
    EnableMenuItem(m_hMenu, IDM_SOURCE_CCIR656_3, (m_CardType->GetValue() == TVCARD_RS_BT)?MF_ENABLED:MF_GRAYED);
    EnableMenuItem(m_hMenu, IDM_SOURCE_CCIR656_4, (m_CardType->GetValue() == TVCARD_RS_BT)?MF_ENABLED:MF_GRAYED);

    BOOL DoneWidth = FALSE;
    CheckMenuItemBool(m_hMenu, IDM_SOURCE_TUNER, (m_VideoSource->GetValue() == 0));
    CheckMenuItemBool(m_hMenu, IDM_SOURCE_COMPOSITE, (m_VideoSource->GetValue() == 1));
    CheckMenuItemBool(m_hMenu, IDM_SOURCE_SVIDEO, (m_VideoSource->GetValue() == 2));
    CheckMenuItemBool(m_hMenu, IDM_SOURCE_OTHER1, (m_VideoSource->GetValue() == 3));
    CheckMenuItemBool(m_hMenu, IDM_SOURCE_OTHER2, (m_VideoSource->GetValue() == 4));
    CheckMenuItemBool(m_hMenu, IDM_SOURCE_COMPVIASVIDEO, (m_VideoSource->GetValue() == 5));
    CheckMenuItemBool(m_hMenu, IDM_SOURCE_CCIR656_1, (m_VideoSource->GetValue() == 6));
    CheckMenuItemBool(m_hMenu, IDM_SOURCE_CCIR656_2, (m_VideoSource->GetValue() == 7));
    CheckMenuItemBool(m_hMenu, IDM_SOURCE_CCIR656_3, (m_VideoSource->GetValue() == 8));
    CheckMenuItemBool(m_hMenu, IDM_SOURCE_CCIR656_4, (m_VideoSource->GetValue() == 9));

    if(GetTVFormat((eVideoFormat)m_VideoFormat->GetValue())->wHActivex1 < 768)
    {
        EnableMenuItem(m_hMenu, IDM_SETTINGS_PIXELWIDTH_768, MF_GRAYED);
    }
    else
    {
        EnableMenuItem(m_hMenu, IDM_SETTINGS_PIXELWIDTH_768, MF_ENABLED);
    }
    CheckMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_768, (m_PixelWidth->GetValue() == 768));
    DoneWidth |= (m_PixelWidth->GetValue() == 768);
    CheckMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_754, (m_PixelWidth->GetValue() == 754));
    DoneWidth |= (m_PixelWidth->GetValue() == 754);
    CheckMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_720, (m_PixelWidth->GetValue() == 720));
    DoneWidth |= (m_PixelWidth->GetValue() == 720);
    CheckMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_640, (m_PixelWidth->GetValue() == 640));
    DoneWidth |= (m_PixelWidth->GetValue() == 640);
    CheckMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_384, (m_PixelWidth->GetValue() == 384));
    DoneWidth |= (m_PixelWidth->GetValue() == 384);
    CheckMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_320, (m_PixelWidth->GetValue() == 320));
    DoneWidth |= (m_PixelWidth->GetValue() == 320);
    CheckMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_CUSTOM, !DoneWidth);

    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_0, (m_VideoFormat->GetValue() == 0));
    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_1, (m_VideoFormat->GetValue() == 1));
    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_2, (m_VideoFormat->GetValue() == 2));
    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_3, (m_VideoFormat->GetValue() == 3));
    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_4, (m_VideoFormat->GetValue() == 4));
    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_5, (m_VideoFormat->GetValue() == 5));
    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_6, (m_VideoFormat->GetValue() == 6));
    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_7, (m_VideoFormat->GetValue() == 7));
    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_8, (m_VideoFormat->GetValue() == 8));
    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_9, (m_VideoFormat->GetValue() == 9));

    CheckMenuItemBool(m_hMenu, IDM_AUDIO_0, (m_AudioSource->GetValue() == 0));
    CheckMenuItemBool(m_hMenu, IDM_AUDIO_1, (m_AudioSource->GetValue() == 1));
    CheckMenuItemBool(m_hMenu, IDM_AUDIO_2, (m_AudioSource->GetValue() == 2));
    CheckMenuItemBool(m_hMenu, IDM_AUDIO_3, (m_AudioSource->GetValue() == 3));
    CheckMenuItemBool(m_hMenu, IDM_AUDIO_4, (m_AudioSource->GetValue() == 4));
    CheckMenuItemBool(m_hMenu, IDM_AUDIO_5, (m_AudioSource->GetValue() == 5));

    CheckMenuItemBool(m_hMenu, IDM_MSPMODE_2, (m_MSPMode->GetValue() == 2));
    CheckMenuItemBool(m_hMenu, IDM_MSPMODE_3, (m_MSPMode->GetValue() == 3));
    CheckMenuItemBool(m_hMenu, IDM_MSPMODE_4, (m_MSPMode->GetValue() == 4));
    CheckMenuItemBool(m_hMenu, IDM_MSPMODE_5, (m_MSPMode->GetValue() == 5));
    CheckMenuItemBool(m_hMenu, IDM_MSPMODE_6, (m_MSPMode->GetValue() == 6));

    CheckMenuItemBool(m_hMenu, IDM_MSPSTEREO_1, (m_MSPStereo->GetValue() == 1));
    CheckMenuItemBool(m_hMenu, IDM_MSPSTEREO_2, (m_MSPStereo->GetValue() == 2));
    CheckMenuItemBool(m_hMenu, IDM_MSPSTEREO_3, (m_MSPStereo->GetValue() == 3));
    CheckMenuItemBool(m_hMenu, IDM_MSPSTEREO_4, (m_MSPStereo->GetValue() == 4));

    CheckMenuItemBool(m_hMenu, IDM_MAJOR_CARRIER_0, (m_MSPMajorMode->GetValue() == 0));
    CheckMenuItemBool(m_hMenu, IDM_MAJOR_CARRIER_1, (m_MSPMajorMode->GetValue() == 1));
    CheckMenuItemBool(m_hMenu, IDM_MAJOR_CARRIER_2, (m_MSPMajorMode->GetValue() == 2));
    CheckMenuItemBool(m_hMenu, IDM_MAJOR_CARRIER_3, (m_MSPMajorMode->GetValue() == 3));

    CheckMenuItemBool(m_hMenu, IDM_MINOR_CARRIER_0, (m_MSPMinorMode->GetValue() == 0));
    CheckMenuItemBool(m_hMenu, IDM_MINOR_CARRIER_1, (m_MSPMinorMode->GetValue() == 1));
    CheckMenuItemBool(m_hMenu, IDM_MINOR_CARRIER_2, (m_MSPMinorMode->GetValue() == 2));
    CheckMenuItemBool(m_hMenu, IDM_MINOR_CARRIER_3, (m_MSPMinorMode->GetValue() == 3));
    CheckMenuItemBool(m_hMenu, IDM_MINOR_CARRIER_4, (m_MSPMinorMode->GetValue() == 4));
    CheckMenuItemBool(m_hMenu, IDM_MINOR_CARRIER_5, (m_MSPMinorMode->GetValue() == 5));
    CheckMenuItemBool(m_hMenu, IDM_MINOR_CARRIER_6, (m_MSPMinorMode->GetValue() == 6));
    CheckMenuItemBool(m_hMenu, IDM_MINOR_CARRIER_7, (m_MSPMinorMode->GetValue() == 7));

    CheckMenuItemBool(m_hMenu, IDM_AUTOSTEREO, m_AutoStereoSelect->GetValue());

    CheckMenuItemBool(m_hMenu, IDM_SAVE_BY_FORMAT, m_bSavePerFormat->GetValue());
    CheckMenuItemBool(m_hMenu, IDM_SAVE_BY_INPUT, m_bSavePerInput->GetValue());
    CheckMenuItemBool(m_hMenu, IDM_SAVE_BY_CHANNEL, m_bSavePerChannel->GetValue());
}

BOOL APIENTRY CBT848Source::AdvVideoSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    static CBT848Source* pThis = NULL;
    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CBT848Source*) lParam;
        // Luma AGC, 0 says AGC enabled
        CheckDlgButton(hDlg, IDC_AGC, !(pThis->m_BtAgcDisable));
        // Adaptive AGC, 0 says Crush disabled
        CheckDlgButton(hDlg, IDC_CRUSH, pThis->m_BtCrush->GetValue());
        // Even CAGC, 0 says CAGC disable
        CheckDlgButton(hDlg, IDC_E_CAGC, pThis->m_BtEvenChromaAGC->GetValue()); 
        // Odd CAGC
        CheckDlgButton(hDlg, IDC_O_CAGC, pThis->m_BtOddChromaAGC->GetValue());
        // Even Peak, 0 says normal, not Luma peak
        CheckDlgButton(hDlg, IDC_E_LUMA_PEAK, pThis->m_BtEvenLumaPeak->GetValue());
        // Odd Peak
        CheckDlgButton(hDlg, IDC_O_LUMA_PEAK, pThis->m_BtOddLumaPeak->GetValue());
        // Luma Output Range, 0 says Luma Normal, 1=Full    
        CheckDlgButton(hDlg, IDC_LUMA_RANGE, pThis->m_BtFullLumaRange->GetValue());
        // Even Luma decimation,  0 says disable
        CheckDlgButton(hDlg, IDC_E_LUMA_DEC, pThis->m_BtEvenLumaDec->GetValue());
        // Odd Luma decimation
        CheckDlgButton(hDlg, IDC_O_LUMA_DEC, pThis->m_BtOddLumaDec->GetValue());
        // Even COMB, 0 = disable
        CheckDlgButton(hDlg, IDC_E_COMB, pThis->m_BtEvenComb->GetValue());
        // Odd COMB
        CheckDlgButton(hDlg, IDC_O_COMB, pThis->m_BtOddComb->GetValue());
        // Color Bars, 0 = disable
        CheckDlgButton(hDlg, IDC_COLOR_BARS, pThis->m_BtColorBars->GetValue());
        // Gamma correction removal, 0=enabled
        CheckDlgButton(hDlg, IDC_GAMMA_CORR, !pThis->m_BtGammaCorrection->GetValue());
        // More Vertical Filter, 0=no, 4=yes, other values no good at our res
        // (Z filter)   TRB 12/19/00
        CheckDlgButton(hDlg, IDC_VERT_FILTER, pThis->m_BtVertFilter->GetValue());
        // More Horizontal Filter, 0=no, else max full res filter TRB 12/19/00
        CheckDlgButton(hDlg, IDC_HOR_FILTER, pThis->m_BtHorFilter->GetValue());
        break;

    case WM_MOUSEMOVE:
        return (FALSE);

    case WM_COMMAND:

        switch LOWORD(wParam)
        {
        case IDOK:                          // Is Done
			WriteSettingsToIni(TRUE);
            EndDialog(hDlg, TRUE);
            break;

        case IDC_AGC:                       // Changed AGC
            pThis->m_BtAgcDisable->SetValue(BST_CHECKED != IsDlgButtonChecked(hDlg, IDC_AGC));
            break;  

        case IDC_CRUSH:                     // Changed Adaptive AGC
            pThis->m_BtCrush->SetValue(BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CRUSH));
            break;  

        case IDC_E_CAGC:                    // Changed Even CAGC
            pThis->m_BtEvenChromaAGC->SetValue(BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_E_CAGC));
            break;

        case IDC_O_CAGC:                    // Changed Odd CAGC
            pThis->m_BtOddChromaAGC->SetValue(BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_O_CAGC));
            break;

        case IDC_E_LUMA_PEAK:               // Changed Even Peak
            pThis->m_BtEvenLumaPeak->SetValue(BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_E_LUMA_PEAK));
            break;

        case IDC_O_LUMA_PEAK:               // Changed Odd Peak
            pThis->m_BtOddLumaPeak->SetValue(BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_O_LUMA_PEAK));
            break;

        case IDC_LUMA_RANGE:                // Luma Output Range
            pThis->m_BtFullLumaRange->SetValue(BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_LUMA_RANGE));
            break;

        case IDC_E_LUMA_DEC:                // Changed Even L.decimation
            pThis->m_BtEvenLumaDec->SetValue(BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_E_LUMA_DEC));
            break;

        case IDC_O_LUMA_DEC:                // Changed Odd L.decimation
            pThis->m_BtOddLumaDec->SetValue(BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_O_LUMA_DEC));
            break;

        case IDC_E_COMB:                    // Changed Even COMB
            pThis->m_BtEvenComb->SetValue(BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_E_COMB));
            break;

        case IDC_O_COMB:                    // Changed Odd COMB
            pThis->m_BtOddComb->SetValue(BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_O_COMB));
           break;

        case IDC_COLOR_BARS:                // Color Bars
            pThis->m_BtColorBars->SetValue(BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_COLOR_BARS));
            break;

        case IDC_GAMMA_CORR:                // Gamma correction removal
            pThis->m_BtGammaCorrection->SetValue(BST_CHECKED != IsDlgButtonChecked(hDlg, IDC_GAMMA_CORR));
            break;

        case IDC_VERT_FILTER:               // Use vertical z-filter
            pThis->m_BtVertFilter->SetValue(BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_VERT_FILTER));
            break;

        case IDC_HOR_FILTER:                // Use Hor peaking filter
            pThis->m_BtHorFilter->SetValue(BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_HOR_FILTER));
            break;

        }
        break;
    }
    return (FALSE);
}



BOOL CBT848Source::HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam)
{
    switch(LOWORD(wParam))
    {
        case IDM_SETUPCARD:
            Stop_Capture();
            DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_SELECTCARD), hWnd, (DLGPROC) SelectCardProc, (LPARAM)this);
            Start_Capture();
            break;

        case IDM_BDELAY_PLUS:
            m_BDelay->ChangeValue(ADJUSTUP);
            SendMessage(hWnd, WM_COMMAND, IDM_BDELAY_CURRENT, 0);
            break;

        case IDM_BDELAY_MINUS:
            m_BDelay->ChangeValue(ADJUSTDOWN);
            SendMessage(hWnd, WM_COMMAND, IDM_BDELAY_CURRENT, 0);
            break;

        case IDM_BDELAY_CURRENT:
            m_BDelay->OSDShow();
            break;

        case IDM_HDELAY_PLUS:
            m_HDelay->ChangeValue(ADJUSTUP);
            SendMessage(hWnd, WM_COMMAND, IDM_HDELAY_CURRENT, 0);
            break;

        case IDM_HDELAY_MINUS:
            m_BDelay->ChangeValue(ADJUSTDOWN);
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
            m_HDelay->OSDShow();
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

        case IDM_MSPMODE_3:
        case IDM_MSPMODE_2:
        case IDM_MSPMODE_4:
        case IDM_MSPMODE_5:
        case IDM_MSPMODE_6:
            m_MSPMode->SetValue(LOWORD(wParam) - (IDM_MSPMODE_2 - 2));
            break;

        case IDM_MAJOR_CARRIER_0:
        case IDM_MAJOR_CARRIER_1:
        case IDM_MAJOR_CARRIER_2:
        case IDM_MAJOR_CARRIER_3:
            m_MSPMajorMode->SetValue(LOWORD(wParam) - IDM_MAJOR_CARRIER_0);
            break;

        case IDM_MINOR_CARRIER_0:
        case IDM_MINOR_CARRIER_1:
        case IDM_MINOR_CARRIER_2:
        case IDM_MINOR_CARRIER_3:
        case IDM_MINOR_CARRIER_4:
        case IDM_MINOR_CARRIER_5:
        case IDM_MINOR_CARRIER_6:
        case IDM_MINOR_CARRIER_7:
            m_MSPMinorMode->SetValue(LOWORD(wParam) - IDM_MINOR_CARRIER_0);
            break;

        case IDM_MSPSTEREO_1:
        case IDM_MSPSTEREO_2:
        case IDM_MSPSTEREO_3:
        case IDM_MSPSTEREO_4:
            m_MSPStereo->SetValue(LOWORD(wParam) - (IDM_MSPSTEREO_1 - 1));
            break;

        case IDM_AUTOSTEREO:
            m_AutoStereoSelect->SetValue(!m_AutoStereoSelect->GetValue());
            break;

        case IDM_AUDIO_0:
        case IDM_AUDIO_1:
        case IDM_AUDIO_2:
        case IDM_AUDIO_3:
        case IDM_AUDIO_4:
        case IDM_AUDIO_5:
            m_AudioSource->SetValue((LOWORD(wParam) - IDM_AUDIO_0));
            switch (m_AudioSource->GetValue())
            {
            case CBT848Card::AUDIOMUX_TUNER:     
                ShowText(hWnd, "Audio Input - Tuner");     
                break;
            case CBT848Card::AUDIOMUX_MSP_RADIO: 
                ShowText(hWnd, "Audio Input - MSP/Radio"); 
                break;
            case CBT848Card::AUDIOMUX_EXTERNAL:  
                ShowText(hWnd, "Audio Input - External");  
                break;
            case CBT848Card::AUDIOMUX_INTERNAL:  
                ShowText(hWnd, "Audio Input - Internal");  
                break;
            case CBT848Card::AUDIOMUX_MUTE:      
                ShowText(hWnd, "Audio Input - Disabled");  
                break;
            case CBT848Card::AUDIOMUX_STEREO:    
                ShowText(hWnd, "Audio Input - Stereo");    
                break;
            }
            break;

        case IDM_SOURCE_TUNER:
        case IDM_SOURCE_COMPOSITE:
        case IDM_SOURCE_SVIDEO:
        case IDM_SOURCE_OTHER1:
        case IDM_SOURCE_OTHER2:
        case IDM_SOURCE_COMPVIASVIDEO:
        case IDM_SOURCE_CCIR656_1:
        case IDM_SOURCE_CCIR656_2:
        case IDM_SOURCE_CCIR656_3:
        case IDM_SOURCE_CCIR656_4:
            {
                CBT848Card::eVideoSourceType nValue = (CBT848Card::eVideoSourceType)(LOWORD(wParam) - IDM_SOURCE_TUNER);
                ShowText(hWnd, m_pBT848Card->GetSourceName(nValue));
                m_VideoSource->SetValue(nValue);
            }
            break;
            
        case IDM_HWINFO:
            DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_HWINFO), hWnd, CBT848Card::ChipSettingProc, (LPARAM)m_pBT848Card);
            break;

        case IDM_AUDIOSETTINGS:
            DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_AUDIOSETTINGS), hWnd, AudioSettingProc);
            break;

        case IDM_AUDIOSETTINGS1:
            DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_AUDIOEQUALIZER), hWnd, AudioSettingProc1);
            break;

        case IDM_ADV_VIDEOSETTINGS:
            DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_ADV_VIDEOSETTINGS), hWnd, AdvVideoSettingProc, (LPARAM)this);
            break;

        case IDM_TYPEFORMAT_0:
        case IDM_TYPEFORMAT_1:
        case IDM_TYPEFORMAT_2:
        case IDM_TYPEFORMAT_3:
        case IDM_TYPEFORMAT_4:
        case IDM_TYPEFORMAT_5:
        case IDM_TYPEFORMAT_6:
        case IDM_TYPEFORMAT_7:
            // Video format (NTSC, PAL, etc)
            m_VideoFormat->SetValue(LOWORD(wParam) - IDM_TYPEFORMAT_0);
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
            SaveInputSettings();
            m_bSavePerFormat->SetValue(!m_bSavePerFormat->GetValue());
            LoadInputSettings();
            break;

        case IDM_SAVE_BY_INPUT:
            SaveInputSettings();
            m_bSavePerInput->SetValue(!m_bSavePerInput->GetValue());
            LoadInputSettings();
            break;

        case IDM_SAVE_BY_CHANNEL:
            SaveInputSettings();
            m_bSavePerChannel->SetValue(!m_bSavePerChannel->GetValue());
            LoadInputSettings();
            break;

        default:
            return FALSE;
            break;
    }
    return TRUE;
}

void CBT848Source::ChangeSectionNamesForInput()
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

    if(IsInTunerMode() && m_bSavePerChannel->GetValue())
    {
        // TODO: save per channel settings as well
        //Format = m_VideoFormat->GetValue()));
    }

    if(Input == -1 && Format == -1)
    {
        m_Brightness->SetSection(m_Section.c_str());
        m_Contrast->SetSection(m_Section.c_str());
        m_Hue->SetSection(m_Section.c_str());
        m_Saturation->SetSection(m_Section.c_str());
        m_SaturationU->SetSection(m_Section.c_str());
        m_SaturationV->SetSection(m_Section.c_str());
        m_BDelay->SetSection(m_Section.c_str());
    }
    else
    {
        char szSection[128];
        sprintf(szSection, "%s_%d_%d", m_Section.c_str(), Input, Format);
        m_Brightness->SetSection(szSection);
        m_Contrast->SetSection(szSection);
        m_Hue->SetSection(szSection);
        m_Saturation->SetSection(szSection);
        m_SaturationU->SetSection(szSection);
        m_SaturationV->SetSection(szSection);
        m_BDelay->SetSection(szSection);
    }

    ChangeDefaultsForInput();
}

void CBT848Source::ChangeDefaultsForInput()
{
    switch(GetFormat())
    {
    case FORMAT_NTSC:
    case FORMAT_NTSC_J:
    case FORMAT_NTSC50:
        m_Brightness->ChangeDefault(DEFAULT_BRIGHTNESS_NTSC);
        m_Contrast->ChangeDefault(DEFAULT_CONTRAST_NTSC);
        m_Hue->ChangeDefault(DEFAULT_HUE_NTSC);
        m_Saturation->ChangeDefault((DEFAULT_SAT_U_NTSC + DEFAULT_SAT_V_NTSC) / 2);
        m_SaturationU->ChangeDefault(DEFAULT_SAT_U_NTSC);
        m_SaturationV->ChangeDefault(DEFAULT_SAT_V_NTSC);
        //Aspect_GetSetting(OVERSCAN)->Default = DEFAULT_OVERSCAN_NTSC;
        break;
    case FORMAT_PAL_BDGHI:
    case FORMAT_PAL_N:
    case FORMAT_PAL60:
    case FORMAT_SECAM:
    case FORMAT_PAL_M:
    case FORMAT_PAL_NC:
        m_Brightness->ChangeDefault(DEFAULT_BRIGHTNESS_PAL);
        m_Contrast->ChangeDefault(DEFAULT_CONTRAST_PAL);
        m_Hue->ChangeDefault(DEFAULT_HUE_PAL);
        m_Saturation->ChangeDefault((DEFAULT_SAT_U_PAL + DEFAULT_SAT_V_PAL) / 2);
        m_SaturationU->ChangeDefault(DEFAULT_SAT_U_PAL);
        m_SaturationV->ChangeDefault(DEFAULT_SAT_V_PAL);
        //Aspect_GetSetting(OVERSCAN)->Default = DEFAULT_OVERSCAN_PAL;
        break;
    }

    // set up defaults fro position parameters
    if(m_bSavePerFormat->GetValue())
    {
        m_BDelay->ChangeDefault(GetTVFormat((eVideoFormat)m_VideoFormat->GetValue())->bDelayB);
    }
    else
    {
        m_BDelay->ChangeDefault(0);
    }

    switch(m_VideoSource->GetValue())
    {
    case CBT848Card::SOURCE_CCIR656_1:
    case CBT848Card::SOURCE_CCIR656_2:
    case CBT848Card::SOURCE_CCIR656_3:
    case CBT848Card::SOURCE_CCIR656_4:
        m_Brightness->ChangeDefault(0);
        m_Contrast->ChangeDefault(0x80);
        m_Hue->ChangeDefault(0x00);
        m_Saturation->ChangeDefault(0x80);
        m_SaturationU->ChangeDefault(0x80);
        m_SaturationV->ChangeDefault(0x80);
        break;
    default:
        break;
    }

}


void CBT848Source::LoadInputSettings()
{
    ChangeDefaultsForInput();
    ChangeSectionNamesForInput();
    m_Brightness->ReadFromIni();
    m_Contrast->ReadFromIni();
    m_Hue->ReadFromIni();
    m_Saturation->ReadFromIni();
    m_SaturationU->ReadFromIni();
    m_SaturationV->ReadFromIni();
    m_BDelay->ReadFromIni();
}

void CBT848Source::SaveInputSettings()
{
    m_Brightness->WriteToIni();
    m_Contrast->WriteToIni();
    m_Hue->WriteToIni();
    m_Saturation->WriteToIni();
    m_SaturationU->WriteToIni();
    m_SaturationV->WriteToIni();
    m_BDelay->WriteToIni();
}
