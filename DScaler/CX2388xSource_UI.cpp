/////////////////////////////////////////////////////////////////////////////
// $Id: CX2388xSource_UI.cpp,v 1.18 2002-12-04 15:20:08 adcockj Exp $
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.17  2002/12/04 15:15:24  adcockj
// Checked in test code by accident
//
// Revision 1.16  2002/12/03 16:34:13  adcockj
// Corrected channel settings groupings
//
// Revision 1.15  2002/12/03 13:28:23  adcockj
// Corrected per channel settings code
//
// Revision 1.14  2002/12/03 07:56:31  adcockj
// Fixed some problems with settings not saving
//
// Revision 1.13  2002/11/28 18:06:32  adcockj
// Changed text for progressive mode
//
// Revision 1.12  2002/11/13 10:34:36  adcockj
// Improved pixel width support
//
// Revision 1.11  2002/11/12 15:22:50  adcockj
// Made new flag settings have default setting
// Added pixel width for CX2388x cards
//
// Revision 1.10  2002/11/12 11:33:07  adcockj
// Fixed OSD
//
// Revision 1.9  2002/11/09 20:53:46  laurentg
// New CX2388x settings
//
// Revision 1.8  2002/11/09 00:22:23  laurentg
// New settings for CX2388x chip
//
// Revision 1.7  2002/11/08 10:37:46  adcockj
// Added UI for Holo3d Settings
//
// Revision 1.6  2002/11/06 11:11:23  adcockj
// Added new Settings and applied Laurent's filter setup suggestions
//
// Revision 1.5  2002/11/03 15:54:10  adcockj
// Added cx2388x register tweaker support
//
// Revision 1.4  2002/10/31 14:47:20  adcockj
// Added Sharpness
//
// Revision 1.3  2002/10/31 03:10:55  atnak
// Changed CSource::GetTreeSettingsPage to return CTreeSettingsPage*
//
// Revision 1.2  2002/10/29 22:00:30  adcockj
// Added EatlLinesAtTop setting for SDI on holo3d
//
// Revision 1.1  2002/10/29 11:05:28  adcockj
// Renamed CT2388x to CX2388x
//
// 
// CVS Log while file was called CT2388xSource_UI.cpp
//
// Revision 1.8  2002/10/26 17:51:52  adcockj
// Simplified hide cusror code and removed PreShowDialogOrMenu & PostShowDialogOrMenu
//
// Revision 1.7  2002/10/21 19:08:09  adcockj
// Added support for keyboard h/v delay
//
// Revision 1.6  2002/10/21 07:19:33  adcockj
// Preliminary Support for PixelView XCapture
//
// Revision 1.5  2002/09/29 16:16:21  adcockj
// Holo3d imrprovements
//
// Revision 1.4  2002/09/29 13:56:30  adcockj
// Fixed some cursor hide problems
//
// Revision 1.3  2002/09/25 15:11:12  adcockj
// Preliminary code for format specific support for settings per channel
//
// Revision 1.2  2002/09/22 17:47:04  adcockj
// Fixes for holo3d
//
// Revision 1.1  2002/09/11 18:19:38  adcockj
// Prelimainary support for CX2388x based cards
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "CX2388xSource.h"
#include "CX2388x_Defines.h"
#include "DScaler.h"
#include "OutThreads.h"
#include "AspectRatio.h"
#include "DebugLog.h"
#include "SettingsPerChannel.h"

extern const char *TunerNames[TUNER_LASTONE];
extern long EnableCancelButton;

BOOL APIENTRY CCX2388xSource::SelectCardProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    int i;
    int nIndex;
    char buf[128];
    static long OrigTuner;
    static CCX2388xSource* pThis;

    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CCX2388xSource*)lParam;
        sprintf(buf, "Setup card %s", pThis->IDString());
        SetWindowText(hDlg, buf);
        Button_Enable(GetDlgItem(hDlg, IDCANCEL), EnableCancelButton);
        SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_RESETCONTENT, 0, 0);
        for(i = 0; i < CX2388xCARD_LASTONE; i++)
        {
            int nIndex;
            nIndex = SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_ADDSTRING, 0, (LONG)pThis->m_pCard->GetCardName((eCX2388xCardId)i));
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
            i = pThis->m_pCard->AutoDetectTuner((eCX2388xCardId)i);
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

void CCX2388xSource::SetMenu(HMENU hMenu)
{
    int i;
    MENUITEMINFO MenuItemInfo;
    char Buffer[265];

    // set up the input menu
    for(i = 0;i < m_pCard->GetNumInputs(); ++i)
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
        sprintf(Buffer, "%s\tCtrl+Alt+F%d",m_pCard->GetInputName(i), i + 1);
        MenuItemInfo.cch = strlen(Buffer);
        SetMenuItemInfo(m_hMenu, IDM_SOURCE_INPUT1 + i, FALSE, &MenuItemInfo);
        
        // enable the menu and check it appropriately
        EnableMenuItem(m_hMenu, IDM_SOURCE_INPUT1 + i, MF_ENABLED);
        CheckMenuItemBool(m_hMenu, IDM_SOURCE_INPUT1 + i, (m_VideoSource->GetValue() == i));
	}
    
    while(i < CT_INPUTS_PER_CARD)
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

    CheckMenuItemBool(m_hMenu, IDM_SAVE_BY_INPUT, m_bSavePerInput->GetValue());
    CheckMenuItemBool(m_hMenu, IDM_SAVE_BY_FORMAT, m_bSavePerFormat->GetValue());
	if(m_CardType->GetValue() == CX2388xCARD_HOLO3D)
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
}

BOOL CCX2388xSource::HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam)
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

        case IDM_SOURCE_INPUT1:
        case IDM_SOURCE_INPUT2:
        case IDM_SOURCE_INPUT3:
        case IDM_SOURCE_INPUT4:
        case IDM_SOURCE_INPUT5:
        case IDM_SOURCE_INPUT6:
        case IDM_SOURCE_INPUT7:
        case IDM_SOURCE_INPUT8:
            {
                int nValue = LOWORD(wParam) - IDM_SOURCE_INPUT1;
                ShowText(hWnd, m_pCard->GetInputName(nValue));
                m_VideoSource->SetValue(nValue);
                SendMessage(hWnd, WM_COMMAND, IDM_VT_RESET, 0);
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

        case IDM_PROGRESSIVE:
            m_IsVideoProgressive->SetValue(!m_IsVideoProgressive->GetValue());
            if(m_IsVideoProgressive->GetValue())
            {
                ShowText(hWnd, "Using Faroudja Deinterlacing");
            }
            else
            {
                ShowText(hWnd, "Using DScaler Deinterlacing");
            }
            break;

		case IDM_FLI_FILMDETECT:
            m_FLIFilmDetect->SetValue(!m_FLIFilmDetect->GetValue());
            if(m_FLIFilmDetect->GetValue())
            {
                ShowText(hWnd, "FLI2200 Film Detection - On");
            }
            else
            {
                ShowText(hWnd, "FLI2200 Film Detection - Off");
            }
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

        case IDM_DSVIDEO_STANDARD_0:
            // "Custom Settings ..." menu
            if (m_hCX2388xResourceInst != NULL)
            {
                m_pCard->ShowRegisterSettingsDialog(m_hCX2388xResourceInst);
            }
            else
            {
                ShowText(hWnd, "CX2388xRes.dll not loaded");
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

        default:
            return FALSE;
            break;
    }
    return TRUE;
}

void CCX2388xSource::ChangeSectionNamesForInput()
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
    }

    ChangeDefaultsForInput(TRUE);
}

void CCX2388xSource::ChangeDefaultsForCard(BOOL bDontSetValue)
{
    if(m_CardType->GetValue() != CX2388xCARD_HOLO3D)
    {
        m_Brightness->ChangeDefault(128, bDontSetValue);
        m_Contrast->ChangeDefault(0x39, bDontSetValue);
        m_Hue->ChangeDefault(128, bDontSetValue);
        m_Saturation->ChangeDefault((0x7f + 0x5A) / 2, bDontSetValue);
        m_SaturationU->ChangeDefault(0x7f, bDontSetValue);
        m_SaturationV->ChangeDefault(0x5A, bDontSetValue);
        m_IsVideoProgressive->ChangeDefault(FALSE, bDontSetValue);
    }
    else
    {
        m_Brightness->ChangeDefault(128, bDontSetValue);
        m_Contrast->ChangeDefault(128, bDontSetValue);
        m_Hue->ChangeDefault(128, bDontSetValue);
        m_Saturation->ChangeDefault(128, bDontSetValue);
        m_SaturationU->ChangeDefault(128, bDontSetValue);
        m_SaturationV->ChangeDefault(128, bDontSetValue);
        m_IsVideoProgressive->ChangeDefault(TRUE, bDontSetValue);
    }
}

void CCX2388xSource::ChangeDefaultsForInput(BOOL bDontSetValue)
{
    eVideoFormat format = GetFormat();
    if(IsNTSCVideoFormat(format))
    {
        m_Overscan->ChangeDefault(DEFAULT_OVERSCAN_NTSC, bDontSetValue);
    }
    else
    {
        m_Overscan->ChangeDefault(DEFAULT_OVERSCAN_PAL, bDontSetValue);
    }
}

void CCX2388xSource::LoadInputSettings()
{
    ChangeDefaultsForInput(TRUE);
    
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
    }

    ChangeChannelSectionNames();
}

void CCX2388xSource::SaveInputSettings(BOOL bOptimizeFileAccess)
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
    }
}


void CCX2388xSource::ChangeChannelSectionNames()
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
        SettingsPerChannel_RegisterSetting("Brightness","CX2388x - Brightness",TRUE, m_Brightness);
        SettingsPerChannel_RegisterSetting("Hue","CX2388x - Hue",TRUE, m_Hue);            
        SettingsPerChannel_RegisterSetting("Contrast","CX2388x - Contrast",TRUE, m_Contrast);
        
        SettingsPerChannel_RegisterSetting("Saturation","CX2388x - Saturation", TRUE);
        SettingsPerChannel_RegisterSetting("Saturation","CX2388x - Saturation", TRUE, m_Saturation);
        SettingsPerChannel_RegisterSetting("Saturation","CX2388x - Saturation", TRUE, m_SaturationU);
        SettingsPerChannel_RegisterSetting("Saturation","CX2388x - Saturation", TRUE, m_SaturationV);
    
        SettingsPerChannel_RegisterSetting("Overscan","CX2388x - Overscan",TRUE, m_Overscan);

        if(m_CardType->GetValue() == CX2388xCARD_HOLO3D)
        {
            SettingsPerChannel_RegisterSetting("FLIFilmDetect", "CX2388x - FLIFilmDetect" , TRUE, m_FLIFilmDetect);
            SettingsPerChannel_RegisterSetting("Sharpness", "CX2388x - Sharpness", TRUE, m_Sharpness);
            SettingsPerChannel_RegisterSetting("UseFLIDeinterlacing", "CX2388x - UseFLIDeinterlacing", FALSE, m_IsVideoProgressive);
        }
        else
        {
            SettingsPerChannel_RegisterSetting("CX2388xAdvancedSettings", "CX2388x - Advanced Settings", FALSE);
            SettingsPerChannel_RegisterSetting("CX2388xAdvancedSettings", "CX2388x - Advanced Settings", FALSE, m_LumaAGC);
            SettingsPerChannel_RegisterSetting("CX2388xAdvancedSettings", "CX2388x - Advanced Settings", FALSE, m_ChromaAGC);
            SettingsPerChannel_RegisterSetting("CX2388xAdvancedSettings", "CX2388x - Advanced Settings", FALSE, m_FastSubcarrierLock);
            SettingsPerChannel_RegisterSetting("CX2388xAdvancedSettings", "CX2388x - Advanced Settings", FALSE, m_WhiteCrush);
            SettingsPerChannel_RegisterSetting("CX2388xAdvancedSettings", "CX2388x - Advanced Settings", FALSE, m_LowColorRemoval);
            SettingsPerChannel_RegisterSetting("CX2388xAdvancedSettings", "CX2388x - Advanced Settings", FALSE, m_CombFilter);
            SettingsPerChannel_RegisterSetting("CX2388xAdvancedSettings", "CX2388x - Advanced Settings", FALSE, m_FullLumaRange);
            SettingsPerChannel_RegisterSetting("CX2388xAdvancedSettings", "CX2388x - Advanced Settings", FALSE, m_Remodulation);
            SettingsPerChannel_RegisterSetting("CX2388xAdvancedSettings", "CX2388x - Advanced Settings", FALSE, m_Chroma2HComb);
            SettingsPerChannel_RegisterSetting("CX2388xAdvancedSettings", "CX2388x - Advanced Settings", FALSE, m_ForceRemodExcessChroma);
            SettingsPerChannel_RegisterSetting("CX2388xAdvancedSettings", "CX2388x - Advanced Settings", FALSE, m_IFXInterpolation);
            SettingsPerChannel_RegisterSetting("CX2388xAdvancedSettings", "CX2388x - Advanced Settings", FALSE, m_CombRange);
            SettingsPerChannel_RegisterSetting("CX2388xAdvancedSettings", "CX2388x - Advanced Settings", FALSE, m_SecondChromaDemod);
            SettingsPerChannel_RegisterSetting("CX2388xAdvancedSettings", "CX2388x - Advanced Settings", FALSE, m_ThirdChromaDemod);
            SettingsPerChannel_RegisterSetting("CX2388xAdvancedSettings", "CX2388x - Advanced Settings", FALSE, m_WhiteCrushUp);
            SettingsPerChannel_RegisterSetting("CX2388xAdvancedSettings", "CX2388x - Advanced Settings", FALSE, m_WhiteCrushDown);
            SettingsPerChannel_RegisterSetting("CX2388xAdvancedSettings", "CX2388x - Advanced Settings", FALSE, m_WhiteCrushMajorityPoint);
            SettingsPerChannel_RegisterSetting("CX2388xAdvancedSettings", "CX2388x - Advanced Settings", FALSE, m_WhiteCrushPerFrame);
        }
    }
}

CTreeSettingsPage* CCX2388xSource::GetTreeSettingsPage()
{
    vector <CSimpleSetting*>vSettingsList;

    vSettingsList.push_back(m_HDelay);
    vSettingsList.push_back(m_VDelay);
    if(m_CardType->GetValue() == CX2388xCARD_HOLO3D)
    {
        vSettingsList.push_back(m_EatLinesAtTop);
        vSettingsList.push_back(m_Sharpness);
    }
    else
    {
        vSettingsList.push_back(m_LumaAGC);
        vSettingsList.push_back(m_ChromaAGC);
        vSettingsList.push_back(m_FastSubcarrierLock);
        vSettingsList.push_back(m_WhiteCrush);
        vSettingsList.push_back(m_LowColorRemoval);
        vSettingsList.push_back(m_CombFilter);
        vSettingsList.push_back(m_FullLumaRange);
        vSettingsList.push_back(m_Remodulation);
        vSettingsList.push_back(m_Chroma2HComb);
        vSettingsList.push_back(m_ForceRemodExcessChroma);
        vSettingsList.push_back(m_IFXInterpolation);
        vSettingsList.push_back(m_CombRange);
        vSettingsList.push_back(m_SecondChromaDemod);
        vSettingsList.push_back(m_ThirdChromaDemod);
    }

    return new CTreeSettingsGeneric("CX2388x Advanced",vSettingsList);
}

void CCX2388xSource::InitializeUI()
{
    MENUITEMINFO    MenuItemInfo;
    HMENU           hSubMenu;
    LPSTR           pMenuName;

    m_hCX2388xResourceInst = LoadLibrary("CX2388xRes.dll");

    if(m_hCX2388xResourceInst != NULL)
    {
        hSubMenu = GetSubMenu(m_hMenu, 0);

        // Set up two separators with the Custom Settings ...
        // menu in between before listing the standards.
        MenuItemInfo.cbSize = sizeof(MenuItemInfo);
        MenuItemInfo.fMask = MIIM_TYPE;
        MenuItemInfo.fType = MFT_SEPARATOR;

        pMenuName = "Custom Settings ...";
        MenuItemInfo.fMask = MIIM_TYPE | MIIM_ID;
        MenuItemInfo.fType = MFT_STRING;
        MenuItemInfo.dwTypeData = pMenuName;
        MenuItemInfo.cch = strlen(pMenuName);
        MenuItemInfo.wID = IDM_DSVIDEO_STANDARD_0;
        InsertMenuItem(hSubMenu, 5, TRUE, &MenuItemInfo);
    }
}
