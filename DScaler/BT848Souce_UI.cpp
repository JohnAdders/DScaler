/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Souce_UI.cpp,v 1.33 2002-08-08 21:15:08 kooiman Exp $
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
// Revision 1.32  2002/08/08 12:35:51  kooiman
// Better channel settings support for BT848 settings.
//
// Revision 1.31  2002/08/05 12:05:44  kooiman
// Added support for per channel settings.
//
// Revision 1.30  2002/08/03 17:57:52  kooiman
// Added new cards & tuners. Changed the tuner combobox into a sorted list.
//
// Revision 1.29  2002/07/02 20:00:07  adcockj
// New setting for MSP input pin selection
//
// Revision 1.28  2002/06/30 20:02:50  laurentg
// Menus entries grayed because not yet implemented
//
// Revision 1.27  2002/06/18 19:29:14  robmuller
// Reset videotext if video source changed.
//
// Revision 1.26  2002/06/09 23:27:21  robmuller
// Implemented canceling for advanced video settings dialog.
//
// Revision 1.25  2002/06/05 20:53:49  adcockj
// Default changes and settings fixes
//
// Revision 1.24  2002/05/19 01:19:23  dschmelzer
// Added a seventh input
//
// Revision 1.23  2002/05/09 12:47:07  adcockj
// Fixed keyboard shortcut bug on Hdelay
//
// Revision 1.22  2002/04/07 10:37:53  adcockj
// Made audio source work per input
//
// Revision 1.21  2002/02/26 15:48:49  robmuller
// Balance slider no longer changes the treble value.
//
// Revision 1.20  2002/02/17 17:48:03  laurentg
// Bug with tuner combobox corrected
//
// Revision 1.19  2002/02/11 08:23:09  adcockj
// Fixed pixel width menu problem
//
// Revision 1.18  2002/02/10 13:06:27  adcockj
// Various sound bugs found by Laurent fixed
//
// Revision 1.17  2002/02/09 18:39:01  adcockj
// Fixed AGC bug in advanced video settings
//
// Revision 1.16  2002/02/09 02:44:56  laurentg
// Overscan now stored in a setting of the source
//
// Revision 1.15  2002/02/08 08:13:17  adcockj
// Put input type text on menu
//
// Revision 1.14  2002/01/24 00:00:13  robmuller
// Added bOptimizeFileAccess flag to WriteToIni from the settings classes.
//
// Revision 1.13  2002/01/12 16:56:21  adcockj
// Series of fixes to bring 4.0.0 into line with 3.1.1
//
// Revision 1.12  2001/12/19 19:24:45  ittarnavsky
// prepended SOUNDCHANNEL_ to all members of the eSoundChannel enum
//
// Revision 1.11  2001/12/18 17:56:11  adcockj
// Backed out menu modification functions for inputs
//
// Revision 1.10  2001/12/18 13:12:11  adcockj
// Interim check-in for redesign of card specific settings
//
// Revision 1.9  2001/12/05 21:45:10  ittarnavsky
// added changes for the AudioDecoder and AudioControls support
//
// Revision 1.8  2001/12/03 17:27:55  adcockj
// SECAM NICAM patch from Quenotte
//
// Revision 1.7  2001/11/29 17:30:51  adcockj
// Reorgainised bt848 initilization
// More Javadoc-ing
//
// Revision 1.6  2001/11/25 01:58:34  ittarnavsky
// initial checkin of the new I2C code
//
// Revision 1.5  2001/11/23 10:49:16  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.4  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
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
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "BT848Source.h"
#include "BT848_Defines.h"
#include "DScaler.h"
#include "OutThreads.h"
#include "AspectRatio.h"
#include "DebugLog.h"
#include "SettingsPerChannel.h"

extern const char *TunerNames[TUNER_LASTONE];

BOOL APIENTRY CBT848Source::AudioSettingProc1(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    /// \todo FIXME pass control to AudioDecoder's settings dialog
    EndDialog(hDlg, TRUE);
    return FALSE;
}

BOOL APIENTRY CBT848Source::AudioSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    static int  TVolume;
    static char TBalance;
    static char TBass;
    static char TTreble;
    static CBT848Source* pThis;

    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CBT848Source*)lParam;

        TVolume = pThis->m_Volume->GetValue();
        TBass = pThis->m_Bass->GetValue();
        TTreble = pThis->m_Treble->GetValue();
        TBalance = pThis->m_Balance->GetValue();

        SetDlgItemInt(hDlg, IDC_D1, TVolume, FALSE);
        SetDlgItemInt(hDlg, IDC_D4, TBass, TRUE);
        SetDlgItemInt(hDlg, IDC_D5, TTreble, TRUE);
        SetDlgItemInt(hDlg, IDC_D6, TBalance, TRUE);

        pThis->m_Volume->SetupControl(GetDlgItem(hDlg, IDC_SLIDER1));
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
            SetDlgItemInt(hDlg, IDC_D6, pThis->m_Balance->GetValue(), TRUE);
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
            pThis->m_Volume->SetValue(TVolume);
            pThis->m_Bass->SetValue(TBass);
            pThis->m_Treble->SetValue(TTreble);
            pThis->m_Balance->SetValue(TBalance);
            pThis->UnMute();
            EndDialog(hDlg, TRUE);
            break;
        
        case IDC_DEFAULT:
            pThis->Mute();
            pThis->m_Volume->SetDefault();
            pThis->m_Bass->SetDefault();
            pThis->m_Treble->SetDefault();
            pThis->m_Balance->SetDefault();
            SetDlgItemInt(hDlg, IDC_D1, pThis->m_Volume->GetValue(), FALSE);
            SetDlgItemInt(hDlg, IDC_D4, pThis->m_Bass->GetValue(), TRUE);
            SetDlgItemInt(hDlg, IDC_D5, pThis->m_Treble->GetValue(), TRUE);
            SetDlgItemInt(hDlg, IDC_D6, pThis->m_Balance->GetValue(), TRUE);

            pThis->m_Volume->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER1));
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

BOOL APIENTRY CBT848Source::SelectCardProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    int i;
    int nIndex;
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
            nIndex = SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_ADDSTRING, 0, (LONG)pThis->m_pBT848Card->GetCardName((eTVCardId)i));
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
            i = pThis->m_pBT848Card->AutoDetectTuner((eTVCardId)i);
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

void CBT848Source::SetMenu(HMENU hMenu)
{
    int i;
    MENUITEMINFO MenuItemInfo;
    char Buffer[265];

    // set up the input menu
    for(i = 0;i < m_pBT848Card->GetNumInputs(); ++i)
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
        sprintf(Buffer, "%s\tCtrl+Alt+F%d",m_pBT848Card->GetInputName(i), i + 1);
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


    EnableMenuItemBool(m_hMenu, IDM_AUDIOSETTINGS, m_pBT848Card->HasMSP());
    // Grayed "Equalize" because not yet implemented
//    EnableMenuItemBool(m_hMenu, IDM_AUDIOSETTINGS1, m_pBT848Card->HasMSP());
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

    CheckMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_MONO, (m_AudioChannel->GetValue() == 1));
    CheckMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_STEREO, (m_AudioChannel->GetValue() == 2));
    CheckMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_LANGUAGE1, (m_AudioChannel->GetValue() == 3));
    CheckMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_LANGUAGE2, (m_AudioChannel->GetValue() == 4));

    CheckMenuItemBool(m_hMenu, IDM_AUTOSTEREO, m_AutoStereoSelect->GetValue());

    CheckMenuItemBool(m_hMenu, IDM_USEINPUTPIN1, m_UseInputPin1->GetValue());

    CheckMenuItemBool(m_hMenu, IDM_SAVE_BY_FORMAT, m_bSavePerFormat->GetValue());
    CheckMenuItemBool(m_hMenu, IDM_SAVE_BY_INPUT, m_bSavePerInput->GetValue());
    CheckMenuItemBool(m_hMenu, IDM_SAVE_BY_CHANNEL, m_bSavePerChannel->GetValue());
    
    if (m_bSavePerFormat->GetValue())
    {
        m_bSavePerChannel->SetValue(FALSE);
        CheckMenuItemBool(m_hMenu, IDM_SAVE_BY_CHANNEL, m_bSavePerChannel->GetValue());
    }
    EnableMenuItemBool(m_hMenu, IDM_SAVE_BY_CHANNEL_CLEARSETTINGS, m_bSavePerChannel->GetValue());
}

BOOL APIENTRY CBT848Source::AdvVideoSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    static CBT848Source* pThis = NULL;

    static BOOL OldAgcDisable;
    static BOOL OldCrush;
    static BOOL OldEvenChromaAGC;
    static BOOL OldOddChromaAGC;
    static BOOL OldEvenLumaPeak;
    static BOOL OldOddLumaPeak;
    static BOOL OldFullLumaRange;
    static BOOL OldEvenLumaDec;
    static BOOL OldOddLumaDec;
    static BOOL OldEvenComb;
    static BOOL OldOddComb;
    static BOOL OldColorBars;
    static BOOL OldGammaCorrection;
    static BOOL OldVertFilter;
    static BOOL OldHorFilter;

    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CBT848Source*) lParam;

        OldAgcDisable = pThis->m_BtAgcDisable->GetValue();
        OldCrush = pThis->m_BtCrush->GetValue();
        OldEvenChromaAGC = pThis->m_BtEvenChromaAGC->GetValue();
        OldOddChromaAGC = pThis->m_BtOddChromaAGC->GetValue();
        OldEvenLumaPeak = pThis->m_BtEvenLumaPeak->GetValue();
        OldOddLumaPeak = pThis->m_BtOddLumaPeak->GetValue();
        OldFullLumaRange = pThis->m_BtFullLumaRange->GetValue();
        OldEvenLumaDec = pThis->m_BtEvenLumaDec->GetValue();
        OldOddLumaDec = pThis->m_BtOddLumaDec->GetValue();
        OldEvenComb = pThis->m_BtEvenComb->GetValue();
        OldOddComb = pThis->m_BtOddComb->GetValue();
        OldColorBars = pThis->m_BtColorBars->GetValue();
        OldGammaCorrection = pThis->m_BtGammaCorrection->GetValue();
        OldVertFilter = pThis->m_BtVertFilter->GetValue();
        OldHorFilter = pThis->m_BtHorFilter->GetValue();

        // Luma AGC, 0 says AGC enabled
        CheckDlgButton(hDlg, IDC_AGC, !OldAgcDisable);
        // Adaptive AGC, 0 says Crush disabled
        CheckDlgButton(hDlg, IDC_CRUSH, OldCrush);
        // Even CAGC, 0 says CAGC disable
        CheckDlgButton(hDlg, IDC_E_CAGC, OldEvenChromaAGC); 
        // Odd CAGC
        CheckDlgButton(hDlg, IDC_O_CAGC, OldOddChromaAGC);
        // Even Peak, 0 says normal, not Luma peak
        CheckDlgButton(hDlg, IDC_E_LUMA_PEAK, OldEvenLumaPeak);
        // Odd Peak
        CheckDlgButton(hDlg, IDC_O_LUMA_PEAK, OldOddLumaPeak);
        // Luma Output Range, 0 says Luma Normal, 1=Full    
        CheckDlgButton(hDlg, IDC_LUMA_RANGE, OldFullLumaRange);
        // Even Luma decimation,  0 says disable
        CheckDlgButton(hDlg, IDC_E_LUMA_DEC, OldEvenLumaDec);
        // Odd Luma decimation
        CheckDlgButton(hDlg, IDC_O_LUMA_DEC, OldOddLumaDec);
        // Even COMB, 0 = disable
        CheckDlgButton(hDlg, IDC_E_COMB, OldEvenComb);
        // Odd COMB
        CheckDlgButton(hDlg, IDC_O_COMB, OldOddComb);
        // Color Bars, 0 = disable
        CheckDlgButton(hDlg, IDC_COLOR_BARS, OldColorBars);
        // Gamma correction removal, 0=enabled
        CheckDlgButton(hDlg, IDC_GAMMA_CORR, !OldGammaCorrection);
        // More Vertical Filter, 0=no, 4=yes, other values no good at our res
        // (Z filter)   TRB 12/19/00
        CheckDlgButton(hDlg, IDC_VERT_FILTER, OldVertFilter);
        // More Horizontal Filter, 0=no, else max full res filter TRB 12/19/00
        CheckDlgButton(hDlg, IDC_HOR_FILTER, OldHorFilter);
        break;

    case WM_MOUSEMOVE:
        return (FALSE);

    case WM_COMMAND:

        switch LOWORD(wParam)
        {
        case IDCANCEL:                      //  Restore settings
            pThis->m_BtAgcDisable->SetValue(OldAgcDisable);
            pThis->m_BtCrush->SetValue(OldCrush);
            pThis->m_BtEvenChromaAGC->SetValue(OldEvenChromaAGC);
            pThis->m_BtOddChromaAGC->SetValue(OldOddChromaAGC);
            pThis->m_BtEvenLumaPeak->SetValue(OldEvenLumaPeak);
            pThis->m_BtOddLumaPeak->SetValue(OldOddLumaPeak);
            pThis->m_BtFullLumaRange->SetValue(OldFullLumaRange);
            pThis->m_BtEvenLumaDec->SetValue(OldEvenLumaDec);
            pThis->m_BtOddLumaDec->SetValue(OldOddLumaDec);
            pThis->m_BtEvenComb->SetValue(OldEvenComb);
            pThis->m_BtOddComb->SetValue(OldOddComb);
            pThis->m_BtColorBars->SetValue(OldColorBars);
            pThis->m_BtGammaCorrection->SetValue(OldGammaCorrection);
            pThis->m_BtVertFilter->SetValue(OldVertFilter);
            pThis->m_BtHorFilter->SetValue(OldHorFilter);

            EndDialog(hDlg, TRUE);
            break;

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
            m_pBT848Card->SetCardType(m_CardType->GetValue());
            m_pBT848Card->InitTuner((eTunerId)m_TunerType->GetValue());
            m_pBT848Card->InitAudio();
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
            m_AudioChannel->SetValue(SOUNDCHANNEL_MONO);
            break;
        case IDM_SOUNDCHANNEL_STEREO:
            m_AudioChannel->SetValue(SOUNDCHANNEL_STEREO);
            break;
        case IDM_SOUNDCHANNEL_LANGUAGE1:
            m_AudioChannel->SetValue(SOUNDCHANNEL_LANGUAGE1);
            break;
        case IDM_SOUNDCHANNEL_LANGUAGE2:
            m_AudioChannel->SetValue(SOUNDCHANNEL_LANGUAGE2);
            break;

        case IDM_AUTOSTEREO:
            m_AutoStereoSelect->SetValue(!m_AutoStereoSelect->GetValue());
            break;

        case IDM_USEINPUTPIN1:
            m_UseInputPin1->SetValue(!m_UseInputPin1->GetValue());
            break;

        case IDM_AUDIO_0:
        case IDM_AUDIO_1:
        case IDM_AUDIO_2:
        case IDM_AUDIO_3:
        case IDM_AUDIO_4:
        case IDM_AUDIO_5:
            GetCurrentAudioSetting()->SetValue((LOWORD(wParam) - IDM_AUDIO_0));
            switch (GetCurrentAudioSetting()->GetValue())
            {
            case AUDIOINPUT_TUNER:     
                ShowText(hWnd, "Audio Input - Tuner");     
                break;
            case AUDIOINPUT_RADIO: 
                ShowText(hWnd, "Audio Input - MSP/Radio"); 
                break;
            case AUDIOINPUT_EXTERNAL:  
                ShowText(hWnd, "Audio Input - External");  
                break;
            case AUDIOINPUT_INTERNAL:  
                ShowText(hWnd, "Audio Input - Internal");  
                break;
            case AUDIOINPUT_MUTE:      
                ShowText(hWnd, "Audio Input - Disabled");  
                break;
            case AUDIOINPUT_STEREO:    
                ShowText(hWnd, "Audio Input - Stereo");    
                break;
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
                ShowText(hWnd, m_pBT848Card->GetInputName(nValue));
                m_VideoSource->SetValue(nValue);
                SendMessage(hWnd, WM_COMMAND, IDM_VT_RESET, 0);
            }
            break;
            
        case IDM_HWINFO:
            DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_HWINFO), hWnd, CBT848Card::ChipSettingProc, (LPARAM)m_pBT848Card);
            break;

        case IDM_AUDIOSETTINGS:
            if (m_pBT848Card->HasMSP() == TRUE)
            {
               DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_AUDIOSETTINGS), hWnd, AudioSettingProc, (LPARAM)this);
            }
            break;

        case IDM_AUDIOSETTINGS1:
            if (m_pBT848Card->HasMSP() == TRUE)
            {
                // do'nt do anything until we get this working
                //DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_AUDIOEQUALIZER), hWnd, AudioSettingProc1, (LPARAM)this);
            }
            break;

        case IDM_ADV_VIDEOSETTINGS:
            DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_ADV_VIDEOSETTINGS), hWnd, AdvVideoSettingProc, (LPARAM)this);
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
        /// \todo save per channel settings as well
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
        m_Overscan->SetSection(m_Section.c_str());
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
        m_Overscan->SetSection(szSection);
        m_BDelay->SetSection(szSection);
    }

    ChangeDefaultsForInput();
}

void CBT848Source::ChangeDefaultsForInput()
{
    eVideoFormat format = GetFormat();
    if(IsNTSCVideoFormat(format))
    {
        m_Brightness->ChangeDefault(DEFAULT_BRIGHTNESS_NTSC);
        m_Contrast->ChangeDefault(DEFAULT_CONTRAST_NTSC);
        m_Hue->ChangeDefault(DEFAULT_HUE_NTSC);
        m_Saturation->ChangeDefault((DEFAULT_SAT_U_NTSC + DEFAULT_SAT_V_NTSC) / 2);
        m_SaturationU->ChangeDefault(DEFAULT_SAT_U_NTSC);
        m_SaturationV->ChangeDefault(DEFAULT_SAT_V_NTSC);
        m_Overscan->ChangeDefault(DEFAULT_OVERSCAN_NTSC);
    }
    else
    {
        m_Brightness->ChangeDefault(DEFAULT_BRIGHTNESS_PAL);
        m_Contrast->ChangeDefault(DEFAULT_CONTRAST_PAL);
        m_Hue->ChangeDefault(DEFAULT_HUE_PAL);
        m_Saturation->ChangeDefault((DEFAULT_SAT_U_PAL + DEFAULT_SAT_V_PAL) / 2);
        m_SaturationU->ChangeDefault(DEFAULT_SAT_U_PAL);
        m_SaturationV->ChangeDefault(DEFAULT_SAT_V_PAL);
        m_Overscan->ChangeDefault(DEFAULT_OVERSCAN_PAL);
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
}


void CBT848Source::LoadInputSettings()
{
    ChangeDefaultsForInput();
    
    if (!SettingsPerChannel())
    {
        ChangeSectionNamesForInput();
        m_Brightness->ReadFromIni();
        m_Contrast->ReadFromIni();
        m_Hue->ReadFromIni();
        m_Saturation->ReadFromIni();
        m_SaturationU->ReadFromIni();
        m_SaturationV->ReadFromIni();
        m_Overscan->ReadFromIni();
        m_BDelay->ReadFromIni();
    }

    ChangeChannelSectionNames();
}

void CBT848Source::SaveInputSettings(BOOL bOptimizeFileAccess)
{
    if (!SettingsPerChannel())
    {        
        m_Brightness->WriteToIni(bOptimizeFileAccess);
        m_Contrast->WriteToIni(bOptimizeFileAccess);
        m_Hue->WriteToIni(bOptimizeFileAccess);
        m_Saturation->WriteToIni(bOptimizeFileAccess);
        m_SaturationU->WriteToIni(bOptimizeFileAccess);
        m_SaturationV->WriteToIni(bOptimizeFileAccess);
        m_Overscan->WriteToIni(bOptimizeFileAccess);
        m_BDelay->WriteToIni(bOptimizeFileAccess);
    }
}


void CBT848Source::ChangeChannelSectionNames()
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
                SettingsPerChannel_WriteSettings(sOldSection.c_str(),m_CurrentChannel,TRUE);
            }        
            SettingsPerChannel_UnregisterSection(sOldSection.c_str());
        }
    
        SettingsPerChannel_RegisterSetSection(m_ChannelSubSection.c_str());
        SettingsPerChannel_RegisterSetting("Brightness","BT8x8 - Brightness",TRUE, m_Brightness);
        SettingsPerChannel_RegisterSetting("Hue","BT8x8 - Hue",TRUE, m_Hue);            
        SettingsPerChannel_RegisterSetting("Contrast","BT8x8 - Contrast",TRUE, m_Contrast);
        
        SettingsPerChannel_RegisterSetting("Saturation","BT8x8 - Saturation",TRUE, m_Saturation);
        SettingsPerChannel_RegisterSetting("Saturation","BT8x8 - Saturation",TRUE, m_SaturationU);
        SettingsPerChannel_RegisterSetting("Saturation","BT8x8 - Saturation",TRUE, m_SaturationV);
    
        SettingsPerChannel_RegisterSetting("Overscan","BT8x8 - Overscan",TRUE, m_Overscan);
        
        SettingsPerChannel_RegisterSetting("AudioChannel","BT8x8 - Audio Channel",TRUE, m_AudioChannel);
        SettingsPerChannel_RegisterSetting("AutoStereoSelect","BT8x8 - Auto Stereo Select",TRUE, m_AutoStereoSelect);
        if (GetVolume()!=NULL) 
        {
            SettingsPerChannel_RegisterSetting("Volume","BT8x8 - Volume",TRUE, m_Volume);            
            SettingsPerChannel_RegisterSetting("Balance","BT8x8 - Balance",TRUE, m_Balance);
        }
    }
}
