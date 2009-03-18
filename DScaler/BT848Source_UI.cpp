/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file BT848Source.cpp CBT848Source Implementation (UI)
 */

#include "stdafx.h"

#ifdef WANT_BT8X8_SUPPORT

#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "BT848Source.h"
#include "BT848_Defines.h"
#include "DScaler.h"
#include "Providers.h"
#include "OutThreads.h"
#include "AspectRatio.h"
#include "DebugLog.h"
#include "SettingsPerChannel.h"
#include "Slider.h"
#include "OSD.h"
#include "LibraryCache.h"

using namespace std;

extern const char *TunerNames[TUNER_LASTONE];
long EnableCancelButton = 1;



BOOL APIENTRY CBT848Source::AudioStandardManualProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    static int TAudioStandardManual;
    static long TAudioStandardMajorCarrier;
    static long TAudioStandardMinorCarrier;
    static CBT848Source* pThis;
    static vector<long> vCarriers;
    static int WasDetecting;

    switch (message)
    {
    case WM_INITDIALOG:
        {
            pThis = (CBT848Source*)lParam;

            TAudioStandardManual = pThis->m_AudioStandardManual->GetValue();
            TAudioStandardMajorCarrier = pThis->m_AudioStandardMajorCarrier->GetValue();
            TAudioStandardMinorCarrier = pThis->m_AudioStandardMinorCarrier->GetValue();

            WasDetecting = 0;

            // Fill standard list box
            int Num = pThis->m_pBT848Card->GetNumAudioStandards();
            long CurrentStandard = pThis->m_pBT848Card->GetAudioStandardCurrent();

            SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_LIST), LB_RESETCONTENT, 0, 0);
            
            vCarriers.clear();

            int i;
            int nIndex;
            int Select = -1;
            for (i = 0; i < Num; i++) 
            { 
                long Standard = pThis->m_pBT848Card->GetAudioStandard(i);
                if (pThis->m_pBT848Card->GetAudioStandardName(Standard) != NULL)
                {
                    nIndex = SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_LIST), LB_ADDSTRING, 0, 
                                (LPARAM) pThis->m_pBT848Card->GetAudioStandardName(Standard)); 
                    SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_LIST), LB_SETITEMDATA, nIndex, (LPARAM) Standard); 

                    if (Standard == CurrentStandard)
                    {
                        Select = nIndex;
                    }

                    long Carrier;                    
                    Carrier = pThis->m_pBT848Card->GetAudioStandardMajorCarrier(Standard);
                    if (Carrier != 0)
                    {
                        BOOL bAdd = TRUE;
                        for (vector<long>::iterator it = vCarriers.begin(); it != vCarriers.end(); ++it)
                        {
                            if  ((*it) == Carrier) { bAdd = FALSE; break; }
                        }
                        if (bAdd) { vCarriers.push_back(Carrier); }
                    }

                    Carrier = pThis->m_pBT848Card->GetAudioStandardMinorCarrier(Standard);
                    if (Carrier != 0)
                    {
                        BOOL bAdd = TRUE;
                        for (vector<long>::iterator it = vCarriers.begin(); it != vCarriers.end(); ++it)
                        {
                            if  ((*it) == Carrier) { bAdd = FALSE; break; }
                        }
                        if (bAdd) { vCarriers.push_back(Carrier); }
                    }

                }
            } 
            if (Select>=0)
            {
                SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_LIST), LB_SETCURSEL, Select, 0);
            }

            SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), CB_RESETCONTENT, 0, 0);
            SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_RESETCONTENT, 0, 0);
            char buf[20];
            BOOL bMajorInList = FALSE;
            BOOL bMinorInList = FALSE;

            for (i = 0; i < vCarriers.size(); i++) 
            { 
                sprintf(buf, "%g", (double)vCarriers[i] / 1000000.0);
                nIndex = SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), CB_ADDSTRING, 0, (LPARAM)buf);
                SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), CB_SETITEMDATA, nIndex, (LPARAM)vCarriers[i]); 

                if (vCarriers[i] == pThis->m_pBT848Card->GetAudioStandardMajorCarrier(-1))
                {
                    SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), CB_SETCURSEL, nIndex, 0);
                    bMajorInList = TRUE;
                }

                nIndex = SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_ADDSTRING, 0, (LPARAM)buf);
                SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_SETITEMDATA, nIndex, (LPARAM)vCarriers[i]); 
                
                if (vCarriers[i] == pThis->m_pBT848Card->GetAudioStandardMinorCarrier(-1))
                {
                    SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_SETCURSEL, nIndex, 0);
                    bMinorInList = TRUE;
                }
            }

            if (!bMajorInList)
            {
                int i = vCarriers.size();
                char buf[30];
                vCarriers.push_back(pThis->m_pBT848Card->GetAudioStandardMajorCarrier(-1));
                sprintf(buf, "%g", (double)vCarriers[i] / 1000000.0);
                int nIndex = SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), CB_ADDSTRING, 0, (LPARAM)buf);
                SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), CB_SETITEMDATA, nIndex, (LPARAM)vCarriers[i]); 
                SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), CB_SETCURSEL, nIndex, 0);                
            }
            if (!bMinorInList)
            {
                int i = vCarriers.size();
                char buf[30];
                vCarriers.push_back(pThis->m_pBT848Card->GetAudioStandardMinorCarrier(-1));
                sprintf(buf, "%g", (double)vCarriers[i] / 1000000.0);
                int nIndex = SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_ADDSTRING, 0, (LPARAM)buf);
                SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_SETITEMDATA, nIndex, (LPARAM)vCarriers[i]); 
                SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_SETCURSEL, nIndex, 0);                
            }

            
            CheckDlgButton(hDlg, IDC_AUDIOSTANDARD_MANUAL_MONO, (pThis->m_AudioChannel->GetValue() == SOUNDCHANNEL_MONO));
            CheckDlgButton(hDlg, IDC_AUDIOSTANDARD_MANUAL_STEREO, (pThis->m_AudioChannel->GetValue() == SOUNDCHANNEL_STEREO));
            CheckDlgButton(hDlg, IDC_AUDIOSTANDARD_MANUAL_LANG1, (pThis->m_AudioChannel->GetValue() == SOUNDCHANNEL_LANGUAGE1));
            CheckDlgButton(hDlg, IDC_AUDIOSTANDARD_MANUAL_LANG2, (pThis->m_AudioChannel->GetValue() == SOUNDCHANNEL_LANGUAGE2));
            
            CheckDlgButton(hDlg, IDC_AUDIOSTANDARD_MANUAL_FORCEAM, ((pThis->m_MSP34xxFlags->GetValue()&0x08)!=0));

            SetTimer(hDlg, IDC_AUDIOSTANDARD_MANUAL_DETECTNOW, 200, NULL);
            
            SetFocus(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_LIST)); 
        }
        return TRUE;
        break;
    case WM_TIMER:
        if (wParam == IDC_AUDIOSTANDARD_MANUAL_DETECTNOW)
        {
            if (pThis->m_DetectingAudioStandard != WasDetecting)
            {
                WasDetecting = pThis->m_DetectingAudioStandard;
                if (WasDetecting)
                {
                    SetDlgItemText(hDlg, IDC_AUDIOSTANDARD_MANUAL_DETECTNOW, "Detecting...");
                    //disable
                    EnableWindow(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MANUAL_DETECTNOW), FALSE);
                    ListBox_SetCurSel(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_LIST), -1);
                    EnableWindow(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_LIST), FALSE);
                    EnableWindow(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), FALSE);
                    EnableWindow(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), FALSE);
                }
                else
                {
                    //enable
                    EnableWindow(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MANUAL_DETECTNOW), TRUE);
                    SetDlgItemText(hDlg, IDC_AUDIOSTANDARD_MANUAL_DETECTNOW, "Detect");
                    EnableWindow(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_LIST), TRUE);
                    int Num = pThis->m_pBT848Card->GetNumAudioStandards();
                    int i;
                    for (i = 0; i < Num; i++)
                    {
                        if (ListBox_GetItemData(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_LIST), i) == pThis->m_AudioStandardManual->GetValue())
                        {
                            ListBox_SetCurSel(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_LIST), i);
                            break;
                        }
                    }
                    EnableWindow(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), TRUE);
                    EnableWindow(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), TRUE);
                    BOOL bMajorInList = FALSE;
                    BOOL bMinorInList = FALSE;                        
                    for (i = 0; i < vCarriers.size(); i++) 
                    {
                        if (ComboBox_GetItemData(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER),i) == 
                             pThis->m_pBT848Card->GetAudioStandardMajorCarrier(-1)
                           )
                        {
                            SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), CB_SETCURSEL, i, 0);   
                            bMajorInList = TRUE;
                        }

                        if (ComboBox_GetItemData(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER),i) == 
                             pThis->m_pBT848Card->GetAudioStandardMinorCarrier(-1)
                           )
                        {
                            SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_SETCURSEL, i, 0);   
                            bMinorInList = TRUE;
                        }
                    }
                    if (!bMajorInList)
                    {
                        int i = vCarriers.size();
                        char buf[30];
                        vCarriers.push_back(pThis->m_pBT848Card->GetAudioStandardMajorCarrier(-1));
                        sprintf(buf, "%g", (double)vCarriers[i] / 1000000.0);
                        int nIndex = SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), CB_ADDSTRING, 0, (LPARAM)buf);
                        SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), CB_SETITEMDATA, nIndex, (LPARAM)vCarriers[i]); 
                        SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), CB_SETCURSEL, nIndex, 0);                
                    }
                    if (!bMinorInList)
                    {
                        int i = vCarriers.size();
                        char buf[30];
                        vCarriers.push_back(pThis->m_pBT848Card->GetAudioStandardMinorCarrier(-1));
                        sprintf(buf, "%g", (double)vCarriers[i] / 1000000.0);
                        int nIndex = SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_ADDSTRING, 0, (LPARAM)buf);
                        SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_SETITEMDATA, nIndex, (LPARAM)vCarriers[i]); 
                        SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_SETCURSEL, nIndex, 0);                
                    }
                }                
            }            
            
            CheckDlgButton(hDlg, IDC_AUDIOSTANDARD_MANUAL_MONO, (pThis->m_AudioChannel->GetValue() == SOUNDCHANNEL_MONO));
            CheckDlgButton(hDlg, IDC_AUDIOSTANDARD_MANUAL_STEREO, (pThis->m_AudioChannel->GetValue() == SOUNDCHANNEL_STEREO));
            CheckDlgButton(hDlg, IDC_AUDIOSTANDARD_MANUAL_LANG1, (pThis->m_AudioChannel->GetValue() == SOUNDCHANNEL_LANGUAGE1));
            CheckDlgButton(hDlg, IDC_AUDIOSTANDARD_MANUAL_LANG2, (pThis->m_AudioChannel->GetValue() == SOUNDCHANNEL_LANGUAGE2));            
        }
        break;
    case WM_COMMAND:                
        switch(LOWORD(wParam))
        {
            case IDC_AUDIOSTANDARD_LIST: 
                switch (HIWORD(wParam)) 
                { 
                    case LBN_SELCHANGE: 
                    {
                        int nItem = SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_LIST), LB_GETCURSEL, 0, 0); 
                        long Standard = SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_LIST), LB_GETITEMDATA, nItem, 0); 

                        //pThis->m_AudioChannel->SetValue(SOUNDCHANNEL_MONO);
                        pThis->m_AudioStandardManual->SetValue(Standard);                        
                        
                        for (int i = 0; i < vCarriers.size(); i++) 
                        {
                            if (ComboBox_GetItemData(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER),i) == 
                                 pThis->m_pBT848Card->GetAudioStandardMajorCarrier(Standard)
                               )
                            {
                                SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), CB_SETCURSEL, i, 0);                                   
                            }
                            if (ComboBox_GetItemData(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER),i) == 
                                 pThis->m_pBT848Card->GetAudioStandardMinorCarrier(Standard)
                               )
                            {
                                SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_SETCURSEL, i, 0);                               
                            }
                        }                        
                    }                    
                    break;
                }
                break;
            case IDC_AUDIOSTANDARD_MINORCARRIER:
                {
                    int i = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER));
                    i = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), i);                                            
                    pThis->m_AudioStandardMinorCarrier->SetValue(i);
                }
                break;
            case IDC_AUDIOSTANDARD_MAJORCARRIER:
                {
                    int i = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER));
                    i = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), i);                                            
                    pThis->m_AudioStandardMajorCarrier->SetValue(i);
                }
                break;
            case IDC_AUDIOSTANDARD_MANUAL_MONO:
                pThis->m_AudioChannel->SetValue(SOUNDCHANNEL_MONO);
                break;
            case IDC_AUDIOSTANDARD_MANUAL_STEREO:
                pThis->m_AudioChannel->SetValue(SOUNDCHANNEL_STEREO);
                break;
            case IDC_AUDIOSTANDARD_MANUAL_LANG1:
                pThis->m_AudioChannel->SetValue(SOUNDCHANNEL_LANGUAGE1);
                break;
            case IDC_AUDIOSTANDARD_MANUAL_LANG2:
                pThis->m_AudioChannel->SetValue(SOUNDCHANNEL_LANGUAGE2);
                break;
            case IDC_AUDIOSTANDARD_MANUAL_FORCEAM:
                {
                    long bFlags = pThis->m_MSP34xxFlags->GetValue();
                    pThis->m_MSP34xxFlags->SetValue( (bFlags & ~0x08) | ((bFlags&0x08)?0x00:0x08) );
                }
                break;

            case IDC_AUDIOSTANDARD_MANUAL_DETECTSTEREO:
                pThis->m_pBT848Card->DetectAudioStandard(pThis->m_AudioStandardDetectInterval->GetValue(), 2, SOUNDCHANNEL_STEREO);
                SendMessage(hDlg, WM_TIMER, IDC_AUDIOSTANDARD_MANUAL_DETECTNOW,0);
                break;
            case IDC_AUDIOSTANDARD_MANUAL_DETECTNOW:
                pThis->m_DetectingAudioStandard = 1;
                if (pThis->m_AutoStereoSelect->GetValue())
                {
                    pThis->m_pBT848Card->DetectAudioStandard(pThis->m_AudioStandardDetectInterval->GetValue(), 1, SOUNDCHANNEL_STEREO);
                }
                else
                {
                    pThis->m_pBT848Card->DetectAudioStandard(pThis->m_AudioStandardDetectInterval->GetValue(), 1, (eSoundChannel)pThis->m_AudioChannel->GetValue());
                }
                SendMessage(hDlg, WM_TIMER, IDC_AUDIOSTANDARD_MANUAL_DETECTNOW,0);
                break;

            case IDOK:
                WriteSettingsToIni(TRUE);
                EndDialog(hDlg, TRUE);
                break;
            case IDCANCEL:
                if (pThis->m_DetectingAudioStandard)
                {
            
                }
                else
                {
                    pThis->m_AudioStandardManual->SetValue(TAudioStandardManual);
                    pThis->m_AudioStandardMajorCarrier->SetValue(TAudioStandardMajorCarrier);
                    pThis->m_AudioStandardMinorCarrier->SetValue(TAudioStandardMinorCarrier);
                }

                EndDialog(hDlg, TRUE);
                break;
        }
        break;
    default:
            break;
    }
    return (FALSE);
}

BOOL APIENTRY CBT848Source::AudioSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    static int  TVolume;
    static int TBalance;
    static int TBass;
    static int TTreble;
    static BOOL TSuperBass;
    static int  TLoudness;
    static int  TSpatialEffect;
    static BOOL TUseEqualizer;
    static int TEqualizerVal[6];
    static CBT848Source* pThis;

    // \todo: enable/disable controls based on the AudioControls capabilities

    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CBT848Source*)lParam;

        TVolume = pThis->m_Volume->GetValue();
        TBass = pThis->m_Bass->GetValue();
        TTreble = pThis->m_Treble->GetValue();
        TBalance = pThis->m_Balance->GetValue();
        TSuperBass = pThis->m_AudioSuperbass->GetValue();;
        TLoudness = pThis->m_AudioLoudness->GetValue();
        TSpatialEffect = pThis->m_AudioSpatialEffect->GetValue();;

        TUseEqualizer = pThis->m_UseEqualizer->GetValue();

        TEqualizerVal[1] = pThis->m_EqualizerBand1->GetValue();
        TEqualizerVal[2] = pThis->m_EqualizerBand2->GetValue();
        TEqualizerVal[3] = pThis->m_EqualizerBand3->GetValue();
        TEqualizerVal[4] = pThis->m_EqualizerBand4->GetValue();
        TEqualizerVal[5] = pThis->m_EqualizerBand5->GetValue();

        // \todo: check if the differenct controls are supported

        SetDlgItemInt(hDlg, IDC_VOLUME_VAL, TVolume, FALSE);        
        SetDlgItemInt(hDlg, IDC_BALANCE_VAL, TBalance, TRUE);

        //SetDlgItemInt(hDlg, IDC_D4, TBass, TRUE);
        //SetDlgItemInt(hDlg, IDC_D5, TTreble, TRUE);

        pThis->m_Volume->SetupControl(GetDlgItem(hDlg, IDC_VOLUME_SLIDER));       
        pThis->m_Balance->SetupControl(GetDlgItem(hDlg, IDC_BALANCE_SLIDER));

        if (!pThis->m_pBT848Card->HasAudioBassBoost())
        {
            SendMessage(GetDlgItem(hDlg, IDC_SUPERBASS), WM_ENABLE, FALSE, 0);                
        }
        else
        {
            CheckDlgButton(hDlg, IDC_SUPERBASS, TSuperBass);
        }

        pThis->m_AudioLoudness->SetControlValue(GetDlgItem(hDlg, IDC_LOUDNESS_SLIDER));
        pThis->m_AudioSpatialEffect->SetupControl(GetDlgItem(hDlg, IDC_SPATIALEFFECT_SLIDER));

        if (!pThis->m_pBT848Card->HasAudioEqualizers())
        {
            SendMessage(GetDlgItem(hDlg,IDC_USEEQUALIZER),WM_ENABLE,FALSE,0);                
            TUseEqualizer = FALSE;
        }
        else
        {
            CheckDlgButton(hDlg, IDC_USEEQUALIZER, TUseEqualizer);
        }
        SendMessage(hDlg, WM_COMMAND, IDC_USEEQUALIZER, 0);
        return TRUE;
        break;

    case WM_VSCROLL:
    case WM_HSCROLL:
        if((HWND)lParam == GetDlgItem(hDlg, IDC_VOLUME_SLIDER))
        {
            pThis->m_Volume->SetFromControl((HWND)lParam);
            SetDlgItemInt(hDlg, IDC_VOLUME_VAL, pThis->m_Volume->GetValue(), TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_BALANCE_SLIDER))
        {
            pThis->m_Balance->SetFromControl((HWND)lParam);
            SetDlgItemInt(hDlg, IDC_BALANCE_VAL, pThis->m_Balance->GetValue(), TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_BAND1_SLIDER))
        {
            if (pThis->m_UseEqualizer->GetValue())
            {
                pThis->m_EqualizerBand1->SetFromControl((HWND)lParam);
                SetDlgItemInt(hDlg, IDC_BAND1_VAL, pThis->m_EqualizerBand1->GetValue(), TRUE);
            }
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_BAND2_SLIDER))
        {
            if (pThis->m_UseEqualizer->GetValue())
            {
                pThis->m_EqualizerBand2->SetFromControl((HWND)lParam);
                SetDlgItemInt(hDlg, IDC_BAND2_VAL, pThis->m_EqualizerBand2->GetValue(), TRUE);
            }        
            else
            {
                pThis->m_Bass->SetFromControl((HWND)lParam);
                SetDlgItemInt(hDlg, IDC_BAND2_VAL, pThis->m_Bass->GetValue(), TRUE);
            }
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_BAND3_SLIDER))
        {
            if (pThis->m_UseEqualizer->GetValue())
            {
                pThis->m_EqualizerBand3->SetFromControl((HWND)lParam);
                SetDlgItemInt(hDlg, IDC_BAND3_VAL, pThis->m_EqualizerBand3->GetValue(), TRUE);
            }
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_BAND4_SLIDER))
        {
            if (pThis->m_UseEqualizer->GetValue())
            {
                pThis->m_EqualizerBand4->SetFromControl((HWND)lParam);
                SetDlgItemInt(hDlg, IDC_BAND4_VAL, pThis->m_EqualizerBand4->GetValue(), TRUE);
            }        
            else
            {
                pThis->m_Treble->SetFromControl((HWND)lParam);
                SetDlgItemInt(hDlg, IDC_BAND4_VAL, pThis->m_Treble->GetValue(), TRUE);
            }
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_BAND5_SLIDER))
        {
            if (pThis->m_UseEqualizer->GetValue())
            {
                pThis->m_EqualizerBand5->SetFromControl((HWND)lParam);
                SetDlgItemInt(hDlg, IDC_BAND5_VAL, pThis->m_EqualizerBand5->GetValue(), TRUE);
            }
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_LOUDNESS_SLIDER))
        {
            pThis->m_AudioLoudness->SetFromControl((HWND)lParam);            
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SPATIALEFFECT_SLIDER))
        {
            pThis->m_AudioSpatialEffect->SetFromControl((HWND)lParam);            
        }
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_USEEQUALIZER:
            {
                BOOL bEnabled = (IsDlgButtonChecked(hDlg, IDC_USEEQUALIZER) == BST_CHECKED);
                pThis->m_UseEqualizer->SetValue(  bEnabled );            
                if (bEnabled)
                {
                    ShowWindow(GetDlgItem(hDlg, IDC_BAND1_SLIDER),SW_SHOW);
                    ShowWindow(GetDlgItem(hDlg, IDC_BAND3_SLIDER),SW_SHOW);
                    ShowWindow(GetDlgItem(hDlg, IDC_BAND5_SLIDER),SW_SHOW);
                    
                    ShowWindow(GetDlgItem(hDlg, IDC_BAND1_TEXT),SW_SHOW);
                    ShowWindow(GetDlgItem(hDlg, IDC_BAND3_TEXT),SW_SHOW);
                    ShowWindow(GetDlgItem(hDlg, IDC_BAND5_TEXT),SW_SHOW);

                    ShowWindow(GetDlgItem(hDlg, IDC_BAND1_VAL),SW_SHOW);
                    ShowWindow(GetDlgItem(hDlg, IDC_BAND3_VAL),SW_SHOW);
                    ShowWindow(GetDlgItem(hDlg, IDC_BAND5_VAL),SW_SHOW);

                    SetDlgItemText(hDlg, IDC_BAND2_TEXT,"500 Hz");
                    SetDlgItemText(hDlg, IDC_BAND4_TEXT,"5000 Hz");                    

                    pThis->m_EqualizerBand1->SetupControl(GetDlgItem(hDlg, IDC_BAND1_SLIDER));
                    pThis->m_EqualizerBand2->SetupControl(GetDlgItem(hDlg, IDC_BAND2_SLIDER));
                    pThis->m_EqualizerBand3->SetupControl(GetDlgItem(hDlg, IDC_BAND3_SLIDER));
                    pThis->m_EqualizerBand4->SetupControl(GetDlgItem(hDlg, IDC_BAND4_SLIDER));
                    pThis->m_EqualizerBand5->SetupControl(GetDlgItem(hDlg, IDC_BAND5_SLIDER));
                    
                    SetDlgItemInt(hDlg, IDC_BAND1_VAL, pThis->m_EqualizerBand1->GetValue(), TRUE);
                    SetDlgItemInt(hDlg, IDC_BAND2_VAL, pThis->m_EqualizerBand2->GetValue(), TRUE);
                    SetDlgItemInt(hDlg, IDC_BAND3_VAL, pThis->m_EqualizerBand3->GetValue(), TRUE);
                    SetDlgItemInt(hDlg, IDC_BAND4_VAL, pThis->m_EqualizerBand4->GetValue(), TRUE);
                    SetDlgItemInt(hDlg, IDC_BAND5_VAL, pThis->m_EqualizerBand5->GetValue(), TRUE);
                }
                else
                {
                    ShowWindow(GetDlgItem(hDlg, IDC_BAND1_SLIDER),SW_HIDE);
                    ShowWindow(GetDlgItem(hDlg, IDC_BAND3_SLIDER),SW_HIDE);
                    ShowWindow(GetDlgItem(hDlg, IDC_BAND5_SLIDER),SW_HIDE);
                    ShowWindow(GetDlgItem(hDlg, IDC_BAND1_TEXT),SW_HIDE);
                    ShowWindow(GetDlgItem(hDlg, IDC_BAND3_TEXT),SW_HIDE);
                    ShowWindow(GetDlgItem(hDlg, IDC_BAND5_TEXT),SW_HIDE);
                    ShowWindow(GetDlgItem(hDlg, IDC_BAND1_VAL),SW_HIDE);
                    ShowWindow(GetDlgItem(hDlg, IDC_BAND3_VAL),SW_HIDE);
                    ShowWindow(GetDlgItem(hDlg, IDC_BAND5_VAL),SW_HIDE);

                    SetDlgItemText(hDlg, IDC_BAND2_TEXT,"Bass");
                    SetDlgItemText(hDlg, IDC_BAND4_TEXT,"Treble");
                    
                    pThis->m_Bass->SetupControl(GetDlgItem(hDlg, IDC_BAND2_SLIDER));
                    pThis->m_Treble->SetupControl(GetDlgItem(hDlg, IDC_BAND4_SLIDER));
                    
                    SetDlgItemInt(hDlg, IDC_BAND2_VAL, pThis->m_Bass->GetValue(), TRUE);
                    SetDlgItemInt(hDlg, IDC_BAND4_VAL, pThis->m_Treble->GetValue(), TRUE);
                }
            }
            break;
        case IDC_SUPERBASS:
            {
                BOOL bEnabled = (IsDlgButtonChecked(hDlg, IDC_SUPERBASS) == BST_CHECKED);
                pThis->m_AudioSuperbass->SetValue(  bEnabled );            
            }
            break;
        case IDOK:
            WriteSettingsToIni(TRUE);
            EndDialog(hDlg, TRUE);
            break;
        
        case IDCANCEL:
            pThis->Mute();
            pThis->m_Volume->SetValue(TVolume);            
            pThis->m_Balance->SetValue(TBalance);
            pThis->m_AudioSuperbass->SetValue(TSuperBass);
            pThis->m_AudioLoudness->SetValue(TLoudness);
            pThis->m_AudioSpatialEffect->SetValue(TSpatialEffect);            
            
            pThis->m_EqualizerBand1->SetValue(TEqualizerVal[1]);
            pThis->m_EqualizerBand2->SetValue(TEqualizerVal[2]);
            pThis->m_EqualizerBand3->SetValue(TEqualizerVal[3]);
            pThis->m_EqualizerBand4->SetValue(TEqualizerVal[4]);
            pThis->m_EqualizerBand5->SetValue(TEqualizerVal[5]);
                    
            pThis->m_Bass->SetValue(TBass);
            pThis->m_Treble->SetValue(TTreble);
            
            pThis->UnMute();
            EndDialog(hDlg, TRUE);
            break;
        
        case IDC_DEFAULT:
            pThis->Mute();
            pThis->m_Volume->SetDefault();
            pThis->m_Balance->SetDefault();
            pThis->m_AudioSuperbass->SetDefault();
            pThis->m_AudioLoudness->SetDefault();
            pThis->m_AudioSpatialEffect->SetDefault();

            if (pThis->m_UseEqualizer->GetValue())
            {
                pThis->m_EqualizerBand1->SetDefault();
                pThis->m_EqualizerBand2->SetDefault();
                pThis->m_EqualizerBand3->SetDefault();
                pThis->m_EqualizerBand4->SetDefault();
                pThis->m_EqualizerBand5->SetDefault();
            } 
            else
            {        
                pThis->m_Bass->SetDefault();
                pThis->m_Treble->SetDefault();            
            }
            SetDlgItemInt(hDlg, IDC_VOLUME_VAL, pThis->m_Volume->GetValue(), FALSE);
            SetDlgItemInt(hDlg, IDC_BALANCE_VAL, pThis->m_Balance->GetValue(), TRUE);

            pThis->m_Volume->SetControlValue(GetDlgItem(hDlg, IDC_VOLUME_SLIDER));
            pThis->m_Balance->SetControlValue(GetDlgItem(hDlg, IDC_BALANCE_SLIDER));

            CheckDlgButton(hDlg, IDC_SUPERBASS, pThis->m_AudioSuperbass->GetValue());

            pThis->m_AudioLoudness->SetControlValue(GetDlgItem(hDlg, IDC_LOUDNESS_SLIDER));
            pThis->m_AudioSpatialEffect->SetupControl(GetDlgItem(hDlg, IDC_SPATIALEFFECT_SLIDER));

            pThis->UnMute();
            
            SendMessage(hDlg, WM_COMMAND, IDC_USEEQUALIZER, 0);
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
    char buf[128];
    static CBT848Source* pThis;
    CBT848Card* pCard = NULL;
    char szCardId[9] = "n/a     ";
    char szVendorId[9] = "n/a ";
    char szDeviceId[9] = "n/a ";
    DWORD dwCardId(0);

    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CBT848Source*)lParam;
        sprintf(buf, "Setup card %u with chip %s", pThis->GetDeviceIndex() + 1, pThis->GetChipName());
        SetWindowText(hDlg, buf);
        Button_Enable(GetDlgItem(hDlg, IDCANCEL), EnableCancelButton);
        SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_RESETCONTENT, 0, 0);
        for(i = 0; i < TVCARD_LASTONE; i++)
        {
            int nIndex;
            nIndex = SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_ADDSTRING, 0, (LONG)pThis->m_pBT848Card->GetCardName((eTVCardId)i).c_str());
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

        pCard = pThis->GetBT848Card();
        SetDlgItemText(hDlg, IDC_BT_CHIP_TYPE, pCard->GetChipType().c_str());
        sprintf(szVendorId,"%04X", pCard->GetVendorId());
        SetDlgItemText(hDlg, IDC_BT_VENDOR_ID, szVendorId);
        sprintf(szDeviceId,"%04X", pCard->GetDeviceId());
        SetDlgItemText(hDlg, IDC_BT_DEVICE_ID, szDeviceId);
        dwCardId = pCard->GetSubSystemId();
        if(dwCardId != 0 && dwCardId != 0xffffffff)
        {
            sprintf(szCardId,"%8X", dwCardId);
        }
        SetDlgItemText(hDlg, IDC_AUTODECTECTID, szCardId);

        return TRUE;
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            i = SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_GETCURSEL, 0, 0);
            pThis->m_TunerType->SetValue(ComboBox_GetItemData(GetDlgItem(hDlg, IDC_TUNERSELECT), i));

            i =  SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_GETCURSEL, 0, 0);
            pThis->m_CardType->SetValue(ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CARDSSELECT), i));
            WriteSettingsToIni(TRUE);
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
        case IDC_AUTODETECT:
            {
                eTVCardId CardId = pThis->m_pBT848Card->AutoDetectCardType();
                eTunerId TunerId = pThis->m_pBT848Card->AutoDetectTuner(CardId);
                
                SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_RESETCONTENT, 0, 0);
                for(i = 0; i < TVCARD_LASTONE; i++)
                {
                    int nIndex;
                    nIndex = SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_ADDSTRING, 0, (LONG)pThis->m_pBT848Card->GetCardName((eTVCardId)i).c_str());
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
        GetMenuItemInfo(m_hMenu, IDM_SOURCE_INPUT1 + i, FALSE, &MenuItemInfo);
        // create the new string and correct the menu
        ostringstream oss;
        oss << m_pBT848Card->GetInputName(i);
        oss << "\tCtrl+Alt+F" << i + 1;
        string Buffer(oss.str());
        MenuItemInfo.cch = Buffer.length();
        MenuItemInfo.dwTypeData = const_cast<char*>(Buffer.c_str());
        SetMenuItemInfo(m_hMenu, IDM_SOURCE_INPUT1 + i, FALSE, &MenuItemInfo);
        
        // enable the menu and check it appropriately
        //EnableMenuItem(m_hMenu, IDM_SOURCE_INPUT1 + i, MF_ENABLED);
        EnableMenuItem(m_hMenu, IDM_SOURCE_INPUT1 + i, (m_TunerType->GetValue() == TUNER_ABSENT && m_pBT848Card->IsInputATuner(i)) ? MF_GRAYED : MF_ENABLED);
        CheckMenuItemBool(m_hMenu, IDM_SOURCE_INPUT1 + i, (m_VideoSource->GetValue() == i));
    }
    
    while(i < BT_INPUTS_PER_CARD)
    {
        EnableMenuItem(m_hMenu, IDM_SOURCE_INPUT1 + i, MF_GRAYED);
        ++i;
    }

    BOOL DoneWidth = FALSE;


    // \todo: check for additional controls

    EnableMenuItemBool(m_hMenu, IDM_AUDIOSETTINGS, 
        m_pBT848Card->HasAudioBalance() 
        || m_pBT848Card->HasAudioBass()
        || m_pBT848Card->HasAudioBassBoost()
        || m_pBT848Card->HasAudioEqualizers() 
        || m_pBT848Card->HasAudioLoudness()
        || m_pBT848Card->HasAudioTreble()
        || m_pBT848Card->HasAudioVolume()
        );

    EnableMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_768, GetTVFormat((eVideoFormat)m_VideoFormat->GetValue())->wHActivex1 >= 768);

    if(m_CardType->GetValue() == TVCARD_CWCEC_ATLAS)
    {
        CheckMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_1024, (m_PixelWidth->GetValue() == 1024));
        DoneWidth |= (m_PixelWidth->GetValue() == 1024);
        CheckMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_800, (m_PixelWidth->GetValue() == 800));
        DoneWidth |= (m_PixelWidth->GetValue() == 800);
    }
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
    if(m_CardType->GetValue() == TVCARD_CWCEC_ATLAS)
    {
        CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_10, (videoFormat == VIDEOFORMAT_RGB_640X480_60));
        CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_11, (videoFormat == VIDEOFORMAT_RGB_800X600_60));
        CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_12, (videoFormat == VIDEOFORMAT_RGB_1024X768_60));
        CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_13, (videoFormat == VIDEOFORMAT_RGB_640X480_75));
        CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_14, (videoFormat == VIDEOFORMAT_RGB_800X600_75));
        CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_15, (videoFormat == VIDEOFORMAT_RGB_1024X768_75));
    }

    CheckMenuItemBool(m_hMenu, IDM_AUDIO_0, (GetCurrentAudioSetting()->GetValue() == 0));
    CheckMenuItemBool(m_hMenu, IDM_AUDIO_1, (GetCurrentAudioSetting()->GetValue() == 1));
    CheckMenuItemBool(m_hMenu, IDM_AUDIO_2, (GetCurrentAudioSetting()->GetValue() == 2));
    CheckMenuItemBool(m_hMenu, IDM_AUDIO_3, (GetCurrentAudioSetting()->GetValue() == 3));
    CheckMenuItemBool(m_hMenu, IDM_AUDIO_4, (GetCurrentAudioSetting()->GetValue() == 4));
    CheckMenuItemBool(m_hMenu, IDM_AUDIO_5, (GetCurrentAudioSetting()->GetValue() == 5));

    // Rename sound channels
    // Add [] around the name of unsuppored channels
    int RealAudioChannel = m_pBT848Card->IsAudioChannelDetected((eSoundChannel)m_AudioChannel->GetValue());

    // Find submenu
    HMENU hBTMenu = GetSubMenu(m_hMenu, 0);
    HMENU hSoundChannelMenu = NULL;
    for (i = 0; i < GetMenuItemCount(hBTMenu); i++)
    {
        if (GetMenuItemID(GetSubMenu(hBTMenu, i), 0) == IDM_SOUNDCHANNEL_MONO)
        {
           hSoundChannelMenu = GetSubMenu(hBTMenu, i);
           SetMenuDefaultItem(hSoundChannelMenu, -1, TRUE);
           break;
        }
    }
    
    UINT MenuID = IDM_SOUNDCHANNEL_MONO;
    int AudioChannelNr = 1;
    do {
        // reset the menu info structure
        memset(&MenuItemInfo, 0, sizeof(MenuItemInfo));
        MenuItemInfo.cbSize = sizeof(MenuItemInfo);
        MenuItemInfo.fMask = MIIM_TYPE;

        // get the size of the string
        GetMenuItemInfo(m_hMenu, MenuID, FALSE, &MenuItemInfo);
        ++MenuItemInfo.cch;
        string Buffer;
        Buffer.resize(MenuItemInfo.cch);

        // get string into buffer
        MenuItemInfo.dwTypeData = const_cast<char*>(Buffer.c_str());
        GetMenuItemInfo(m_hMenu, MenuID, FALSE, &MenuItemInfo);

        char Buffer2[256];
        if (Buffer[0]=='[') 
        {            
            strcpy(Buffer2, &Buffer[1]);
        }
        else
        {
            strcpy(Buffer2, &Buffer[0]);
        }   
        int Len = strlen(Buffer2);
        if (Buffer2[Len-1]==']') { Buffer2[Len-1]=0; }        
        
        
        if (m_AutoStereoSelect->GetValue() && (AudioChannelNr!=m_pBT848Card->IsAudioChannelDetected((eSoundChannel)AudioChannelNr)))
        {
            Buffer = "[";
            Buffer += Buffer2;
            Buffer += "]";
            MenuItemInfo.dwTypeData = &Buffer[0];
            MenuItemInfo.cch = Buffer.length();
        }    
        else
        {
            MenuItemInfo.dwTypeData = Buffer2;
            MenuItemInfo.cch = strlen(Buffer2);
        }                
        SetMenuItemInfo(m_hMenu, MenuID, FALSE, &MenuItemInfo);
        
        CheckMenuItemBool(m_hMenu, MenuID, (m_AudioChannel->GetValue() == AudioChannelNr));
        
        if (m_AutoStereoSelect->GetValue() && (hSoundChannelMenu != NULL) && (AudioChannelNr == RealAudioChannel))
        {
            //Highlight real audio channel
            //SetMenuDefaultItem(hSoundChannelMenu, AudioChannelNr-1, TRUE); //MenuID, FALSE);
        }        

        AudioChannelNr++;
        if (AudioChannelNr==2)
        {
            MenuID = IDM_SOUNDCHANNEL_STEREO;
        }
        else if (AudioChannelNr==3)
        {
            MenuID = IDM_SOUNDCHANNEL_LANGUAGE1;
        }
        if (AudioChannelNr==4)
        {
            MenuID = IDM_SOUNDCHANNEL_LANGUAGE2;
        }
    } while (AudioChannelNr<=4);
    

    EnableMenuItemBool(m_hMenu, IDM_AUTOSTEREO, TRUE);
    CheckMenuItemBool(m_hMenu, IDM_AUTOSTEREO, m_AutoStereoSelect->GetValue());

    if (m_pBT848Card->GetAudioDecoderType() == CAudioDecoder::AUDIODECODERTYPE_MSP34x0 ||
        m_pBT848Card->GetAudioDecoderType() == CAudioDecoder::AUDIODECODERTYPE_TDA9875)
    {
        EnableMenuItemBool(m_hMenu, IDM_USEINPUTPIN1, TRUE);
        CheckMenuItemBool(m_hMenu, IDM_USEINPUTPIN1, m_UseInputPin1->GetValue());
    }
    else
    {
        EnableMenuItemBool(m_hMenu, IDM_USEINPUTPIN1, FALSE);
    }

    // Why does a pop-up menu item don't have an own ID?
    hBTMenu = GetSubMenu(m_hMenu, 0);
    for (i = 0; i < GetMenuItemCount(hBTMenu); i++)
    {
        if (GetMenuItemID(GetSubMenu(hBTMenu, i), 0) == IDM_AUDIOSTANDARD_VIDEOFORMATDEFAULT)
        {            
            EnableMenuItem(hBTMenu, i, (m_pBT848Card->GetNumAudioStandards()>0) ? (MF_BYPOSITION|MF_ENABLED) : (MF_BYPOSITION|MF_GRAYED|MF_DISABLED));
        }
    }
    CheckMenuItemBool(m_hMenu, IDM_AUDIOSTANDARD_VIDEOFORMATDEFAULT, (m_AudioStandardDetect->GetValue() == 0));
    CheckMenuItemBool(m_hMenu, IDM_AUDIOSTANDARD_AUTODETECTPERVIDEOFORMAT, (m_AudioStandardDetect->GetValue() == 2));
    CheckMenuItemBool(m_hMenu, IDM_AUDIOSTANDARD_AUTODETECTPERCHANNEL, (m_AudioStandardDetect->GetValue() == 3));
    CheckMenuItemBool(m_hMenu, IDM_AUDIOSTANDARD_MANUAL, (m_AudioStandardDetect->GetValue() == 4));
    
    string Buffer("<No standard>");
    if (m_pBT848Card->GetNumAudioStandards()>0)
    {
        const char *szName = m_pBT848Card->GetAudioStandardName(m_pBT848Card->GetAudioStandardCurrent());
        if (szName != NULL)
        {
            Buffer = szName;
        }
    }
    memset(&MenuItemInfo, 0, sizeof(MenuItemInfo));
    MenuItemInfo.cbSize = sizeof(MenuItemInfo);
    MenuItemInfo.fMask = MIIM_TYPE;
    MenuItemInfo.dwTypeData = &Buffer[0];
    MenuItemInfo.cch = Buffer.length();
    SetMenuItemInfo(m_hMenu, IDM_AUDIOSTANDARD_STANDARD, FALSE, &MenuItemInfo);
    EnableMenuItem(m_hMenu, IDM_AUDIOSTANDARD_STANDARD, MF_GRAYED|MF_DISABLED);
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
            InitAudio();
            Start_Capture();
            break;

        case IDM_BDELAY_PLUS:
            m_BDelay->ChangeValue(ADJUSTUP_SILENT);
            SendMessage(hWnd, WM_COMMAND, IDM_BDELAY_CURRENT, 0);
            break;

        case IDM_BDELAY_MINUS:
            m_BDelay->ChangeValue(ADJUSTDOWN_SILENT);
            SendMessage(hWnd, WM_COMMAND, IDM_BDELAY_CURRENT, 0);
            break;

        case IDM_BDELAY_CURRENT:
            m_BDelay->OSDShow();
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
        case IDM_SOURCE_INPUT8:
        case IDM_SOURCE_INPUT9:
        case IDM_SOURCE_INPUT10:
        case IDM_SOURCE_INPUT11:
        case IDM_SOURCE_INPUT12:
            {
                int nValue = LOWORD(wParam) - IDM_SOURCE_INPUT1;
                if (nValue < m_pBT848Card->GetNumInputs())
                {
                    if (m_TunerType->GetValue() != TUNER_ABSENT || !m_pBT848Card->IsInputATuner(nValue))
                    {
                        ShowText(hWnd, m_pBT848Card->GetInputName(nValue));
                        SetTrayTip(m_pBT848Card->GetInputName(nValue));
                        m_VideoSource->SetValue(nValue);
                        SendMessage(hWnd, WM_COMMAND, IDM_VT_RESET, 0);
                    }
                }
            }
            break;
            
        case IDM_HWINFO:
            DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_HWINFO), hWnd, CBT848Card::ChipSettingProc, (LPARAM)m_pBT848Card);
            break;

        case IDM_AUDIOSETTINGS:
            DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_AUDIOSETTINGS), hWnd, AudioSettingProc, (LPARAM)this);
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
        case IDM_TYPEFORMAT_10:
            m_VideoFormat->SetValue(VIDEOFORMAT_RGB_640X480_60);
            ShowText(hWnd, GetStatus());
            break;
        case IDM_TYPEFORMAT_11:
            m_VideoFormat->SetValue(VIDEOFORMAT_RGB_800X600_60);
            ShowText(hWnd, GetStatus());
            break;
        case IDM_TYPEFORMAT_12:
            m_VideoFormat->SetValue(VIDEOFORMAT_RGB_1024X768_60);
            ShowText(hWnd, GetStatus());
            break;
        case IDM_TYPEFORMAT_13:
            m_VideoFormat->SetValue(VIDEOFORMAT_RGB_640X480_75);
            ShowText(hWnd, GetStatus());
            break;
        case IDM_TYPEFORMAT_14:
            m_VideoFormat->SetValue(VIDEOFORMAT_RGB_800X600_75);
            ShowText(hWnd, GetStatus());
            break;
        case IDM_TYPEFORMAT_15:
            m_VideoFormat->SetValue(VIDEOFORMAT_RGB_1024X768_75);
            ShowText(hWnd, GetStatus());
            break;

        case IDM_DSVIDEO_STANDARD_0:
            // "Custom Settings ..." menu
            if (m_hBT8x8ResourceInst != NULL)
            {
                m_pBT848Card->ShowRegisterSettingsDialog(m_hBT8x8ResourceInst);
            }
            else
            {
                ShowText(hWnd, "BT8x8Res.dll not loaded");
            }
            break;

        case IDM_SETTINGS_PIXELWIDTH_1024:
            m_PixelWidth->SetValue(1024);
            break;

        case IDM_SETTINGS_PIXELWIDTH_800:
            m_PixelWidth->SetValue(800);
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

        case IDM_AUDIOSTANDARD_VIDEOFORMATDEFAULT:
            m_AudioStandardDetect->SetValue(0);
            break;

        case IDM_AUDIOSTANDARD_AUTODETECTPERVIDEOFORMAT:
            m_AudioStandardDetect->SetValue(2);
            break;
        case IDM_AUDIOSTANDARD_AUTODETECTPERCHANNEL:
            m_AudioStandardDetect->SetValue(3);
            break;
        case IDM_AUDIOSTANDARD_MANUAL:
            m_AudioStandardDetect->SetValue(4, TRUE);
            DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_AUDIOSTANDARD_MANUAL), hWnd, AudioStandardManualProc, (LPARAM)this);
            break;
        case IDM_AUDIOSTANDARD_DETECTNOW:
            {                
                m_DetectingAudioStandard = 1;
                if (m_AutoStereoSelect->GetValue())
                {
                    m_pBT848Card->DetectAudioStandard(m_AudioStandardDetectInterval->GetValue(), 1, SOUNDCHANNEL_STEREO);
                }
                else
                {
                    m_pBT848Card->DetectAudioStandard(m_AudioStandardDetectInterval->GetValue(), 1, (eSoundChannel)m_AudioChannel->GetValue());
                }
                OSD_ShowText("Detect Audio Standard", 0);
            }
            break;

        case IDC_TOOLBAR_VOLUME_CHANNEL:
            {
                eSoundChannel SoundChannel = (eSoundChannel)lParam;
                m_AudioChannel->SetValue(SoundChannel);
            }
            break;

        default:
            return FALSE;
            break;
    }
    return TRUE;
}

void CBT848Source::ChangeDefaultsForVideoFormat(BOOL bDontSetValue)
{
    eVideoFormat format = GetFormat();
    TTVFormat *tvFormat = GetTVFormat(format);

    if(m_pBT848Card->IsSPISource(m_VideoSource->GetValue()) &&
        (m_CardType->GetValue() == TVCARD_CWCEC_ATLAS))
    {
        int hpixels, vlines, ilace, vfreq;
        int pll_phase, hsync_pw, syncsep, sogthresh;
        int precoast_act, precoast_inact;
        int vco_range, cpump;
        int hoffs, voffs;
        BOOL sog;

        pll_phase = 0;
        syncsep = 32;
        hpixels = tvFormat->wTotalWidth;
        if(IsRGBVideoFormat(format))
        {
            vlines = tvFormat->wCropHeight + tvFormat->wCropOffset;
            hsync_pw = 32;
            sogthresh = 15;
            precoast_act = 0;
            precoast_inact = 0;
            ilace = 0;
            vfreq = 75;
            sog = FALSE;
            hoffs = tvFormat->wHDelayx1;
            voffs = tvFormat->wVDelay;
            if ((format >= VIDEOFORMAT_RGB_640X480_60) &&
                (format <= VIDEOFORMAT_RGB_1024X768_60))
            {
                vfreq = 60;
            }
            if ((format == VIDEOFORMAT_RGB_800X600_60) ||
                (format == VIDEOFORMAT_RGB_800X600_75))
            {
                pll_phase = 16;
            }
        }
        else
        {
            ilace = 1;
            sogthresh = 24;
            hsync_pw = 128;
            sog = TRUE;
            switch (format)
            {
            case VIDEOFORMAT_NTSC_M:
            default:
                vlines = 525;
                precoast_act = 7;
                precoast_inact = 4;
                vfreq = 60;
                hoffs = 114;
                voffs = 15;
                break;
            case VIDEOFORMAT_PAL_B:
            case VIDEOFORMAT_PAL_D:
            case VIDEOFORMAT_PAL_G:
            case VIDEOFORMAT_PAL_H:
            case VIDEOFORMAT_PAL_I:
                vlines = 625;
                precoast_act = 3;
                precoast_inact = 2;
                vfreq = 50;
                hoffs = 156;
                voffs = 19;
                break;
            }
        }
        m_pBT848Card->GetAD9882()->calcAD9882PLL(hpixels, vlines, vfreq,
                ilace, vco_range, cpump);

        m_AD9882PLL->ChangeDefault(hpixels, bDontSetValue);
        m_AD9882VCO->ChangeDefault(vco_range, bDontSetValue);
        m_AD9882Pump->ChangeDefault(cpump, bDontSetValue);
        m_AD9882Phase->ChangeDefault(pll_phase, bDontSetValue);
        m_AD9882PreCoast->ChangeDefault(precoast_act, bDontSetValue);
        m_AD9882PostCoast->ChangeDefault(precoast_inact, bDontSetValue);
        m_AD9882HSync->ChangeDefault(hsync_pw, bDontSetValue);
        m_AD9882SyncSep->ChangeDefault(syncsep, bDontSetValue);
        m_AD9882SOGThresh->ChangeDefault(sogthresh, bDontSetValue);
        m_AD9882SOG->ChangeDefault(sog, bDontSetValue);
        m_HorizOffset->ChangeDefault(hoffs, bDontSetValue);
        m_VertOffset->ChangeDefault(voffs, bDontSetValue);
    }
    else if(IsNTSCVideoFormat(format))
    {
        if(m_CardType->GetValue() != TVCARD_PMSDELUXE && m_CardType->GetValue() != TVCARD_SWEETSPOT)
        {
            m_Brightness->ChangeDefault(DEFAULT_BRIGHTNESS_NTSC, bDontSetValue);
            m_Contrast->ChangeDefault(DEFAULT_CONTRAST_NTSC, bDontSetValue);
            m_Hue->ChangeDefault(DEFAULT_HUE_NTSC, bDontSetValue);
            m_Saturation->ChangeDefault((DEFAULT_SAT_U_NTSC + DEFAULT_SAT_V_NTSC) / 2, bDontSetValue);
            m_SaturationU->ChangeDefault(DEFAULT_SAT_U_NTSC, bDontSetValue);
            m_SaturationV->ChangeDefault(DEFAULT_SAT_V_NTSC, bDontSetValue);
        }
        else
        {
            m_Brightness->ChangeDefault(0, bDontSetValue);
            m_Contrast->ChangeDefault(128, bDontSetValue);
            m_Hue->ChangeDefault(0, bDontSetValue);
            m_Saturation->ChangeDefault(128, bDontSetValue);
            m_SaturationU->ChangeDefault(128, bDontSetValue);
            m_SaturationV->ChangeDefault(128, bDontSetValue);
        }

        m_TopOverscan->ChangeDefault(DEFAULT_OVERSCAN_NTSC, bDontSetValue);
        m_BottomOverscan->ChangeDefault(DEFAULT_OVERSCAN_NTSC, bDontSetValue);
        m_LeftOverscan->ChangeDefault(DEFAULT_OVERSCAN_NTSC, bDontSetValue);
        m_RightOverscan->ChangeDefault(DEFAULT_OVERSCAN_NTSC, bDontSetValue);
    }
    else if(IsSECAMVideoFormat(format))
    {
        if(m_CardType->GetValue() != TVCARD_PMSDELUXE && m_CardType->GetValue() != TVCARD_SWEETSPOT)
        {
            m_Brightness->ChangeDefault(DEFAULT_BRIGHTNESS_SECAM, bDontSetValue);
            m_Contrast->ChangeDefault(DEFAULT_CONTRAST_SECAM, bDontSetValue);
            m_Hue->ChangeDefault(DEFAULT_HUE_SECAM, bDontSetValue);
            m_Saturation->ChangeDefault((DEFAULT_SAT_U_SECAM + DEFAULT_SAT_V_SECAM) / 2, bDontSetValue);
            m_SaturationU->ChangeDefault(DEFAULT_SAT_U_SECAM, bDontSetValue);
            m_SaturationV->ChangeDefault(DEFAULT_SAT_V_SECAM, bDontSetValue);
        }
        else
        {
            m_Brightness->ChangeDefault(0, bDontSetValue);
            m_Contrast->ChangeDefault(128, bDontSetValue);
            m_Hue->ChangeDefault(0, bDontSetValue);
            m_Saturation->ChangeDefault(128, bDontSetValue);
            m_SaturationU->ChangeDefault(128, bDontSetValue);
            m_SaturationV->ChangeDefault(128, bDontSetValue);
        }
        m_TopOverscan->ChangeDefault(DEFAULT_OVERSCAN_PAL, bDontSetValue);
        m_BottomOverscan->ChangeDefault(DEFAULT_OVERSCAN_PAL, bDontSetValue);
        m_LeftOverscan->ChangeDefault(DEFAULT_OVERSCAN_PAL, bDontSetValue);
        m_RightOverscan->ChangeDefault(DEFAULT_OVERSCAN_PAL, bDontSetValue);
    }
    else
    {
        if(m_CardType->GetValue() != TVCARD_PMSDELUXE && m_CardType->GetValue() != TVCARD_SWEETSPOT)
        {
            m_Brightness->ChangeDefault(DEFAULT_BRIGHTNESS_PAL, bDontSetValue);
            m_Contrast->ChangeDefault(DEFAULT_CONTRAST_PAL, bDontSetValue);
            m_Hue->ChangeDefault(DEFAULT_HUE_PAL, bDontSetValue);
            m_Saturation->ChangeDefault((DEFAULT_SAT_U_PAL + DEFAULT_SAT_V_PAL) / 2, bDontSetValue);
            m_SaturationU->ChangeDefault(DEFAULT_SAT_U_PAL, bDontSetValue);
            m_SaturationV->ChangeDefault(DEFAULT_SAT_V_PAL, bDontSetValue);
        }
        else
        {
            m_Brightness->ChangeDefault(0, bDontSetValue);
            m_Contrast->ChangeDefault(128, bDontSetValue);
            m_Hue->ChangeDefault(0, bDontSetValue);
            m_Saturation->ChangeDefault(128, bDontSetValue);
            m_SaturationU->ChangeDefault(128, bDontSetValue);
            m_SaturationV->ChangeDefault(128, bDontSetValue);
        }
        m_TopOverscan->ChangeDefault(DEFAULT_OVERSCAN_PAL, bDontSetValue);
        m_BottomOverscan->ChangeDefault(DEFAULT_OVERSCAN_PAL, bDontSetValue);
        m_LeftOverscan->ChangeDefault(DEFAULT_OVERSCAN_PAL, bDontSetValue);
        m_RightOverscan->ChangeDefault(DEFAULT_OVERSCAN_PAL, bDontSetValue);
    }
}

void CBT848Source::InitializeUI()
{
    MENUITEMINFO    MenuItemInfo;
    HMENU           hSubMenu;
    LPSTR           pMenuName;

    m_hBT8x8ResourceInst = LibraryCache::GetLibraryHandle("BT8x8Res.dll");

    if(m_hBT8x8ResourceInst != NULL)
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
        InsertMenuItem(hSubMenu, 3, TRUE, &MenuItemInfo);
    }
}

#endif // WANT_BT8X8_SUPPORT