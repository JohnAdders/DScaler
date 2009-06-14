/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Tom Barry.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//    This file is subject to the terms of the GNU General Public License as
//    published by the Free Software Foundation.  A copy of this license is
//    included with this software distribution in the file COPYING.  If you
//    do not have a copy, you may obtain a copy by writing to the Free
//    Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//    This software is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
//
// GreedyH.c is basically just a wrapper for the new Greedy (High Motion) deinterlace
// method. This member handles all, or most of, the dependencies on Windows, or
// the DScaler environment. That includes the User Interface stuff to adjust parms
// or view the diagnostic pulldown trace.
//
// For details of the actual deinterlace algorithm, see member DI_GreedyHM.c.
//
// For details of the pulldown handling, see member DI_GreedyHMPulldown.c.
//
//////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DS_Deinterlace.h"
#include "resource.h"
#include "commctrl.h"
#include "..\..\DScaler\Slider.h"
#include "DI_GreedyHM.h"
#include "..\help\helpids.h"

int GreedyDiag();
BOOL GetCheckDScalerInfo(TDeinterlaceInfo *info);
BOOL DeinterlaceGreedyH_SSE(TDeinterlaceInfo *info);
BOOL DeinterlaceGreedyH_3DNOW(TDeinterlaceInfo *info);
BOOL DeinterlaceGreedyH_MMX(TDeinterlaceInfo *info);

HINSTANCE hInst = NULL;


////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////

SETTING DI_GreedyHSettings[DI_GREEDYH_SETTING_LASTONE] =
{
    {
        "Max Comb", SLIDER, 0,            // szDisplayName, TYPE, orig val
        &GreedyMaxComb, 5, 0,            // *pValue,Default, Min
        255, 1, 1,                        // Max, Step, OSDDivider
        NULL, "Deinterlace",            // **pszList, Ini Section
        "GreedyMaxComb", NULL,            // Ini name, pfnOnChange
    },
    {
        "Motion Threshold", SLIDER, 0,    // szDisplayName, TYPE, orig val
        &GreedyMotionThreshold, 25, 0,    // *pValue,Default, Min
        255, 1, 1,                        // Max, Step, OSDDivider
        NULL, "Deinterlace",            // **pszList, Ini Section
        "GreedyMotionThreshold", NULL,        // Ini name, pfnOnChange
    },
    {
        "Motion Sense", SLIDER, 0,    // szDisplayName, TYPE, orig val
        &GreedyMotionSense, 30, 0, // *pValue,Default, Min
        255, 1, 1,                        // Max, Step, OSDDivider
        NULL, "Deinterlace",            // **pszList, Ini Section
        "GreedyMotionSense", NULL,// Ini name, pfnOnChange
    },
    {
        "Good PullDown Lvl", SLIDER, 0,        // szDisplayName, TYPE, orig val
        &GreedyGoodPullDownLvl, 83, 0,        // *pValue,Default, Min
        255, 1, 1,                        // Max, Step, OSDDivider
        NULL, "Deinterlace",            // **pszList, Ini Section
        "GreedyGoodPullDownLvl", NULL,        // Ini name, pfnOnChange
    },
    {
        "Bad PullDown Lvl", SLIDER, 0,    // szDisplayName, TYPE, orig val
        &GreedyBadPullDownLvl, 88, 0,    // *pValue,Default, Min
        255, 1, 1,                        // Max, Step, OSDDivider
        NULL, "Deinterlace",            // **pszList, Ini Section
        "GreedyBadPullDownLvl", NULL,    // Ini name, pfnOnChange
    },
    {
        "H. Sharpness", SLIDER, 0,        // szDisplayName, TYPE, orig val
        &GreedyHSharpnessAmt, 50, -100,        // *pValue,Default, Min
        100, 1, 1,                        // Max, Step, OSDDivider
        NULL, "Deinterlace",            // **pszList, Ini Section
        "GreedyHSharpnessAmt", NULL,        // Ini name, pfnOnChange
    },
    {
        "V. Sharpness", SLIDER, 0,        // szDisplayName, TYPE, orig val
        &GreedyVSharpnessAmt, 23, -100,        // *pValue,Default, Min
        100, 1, 1,                        // Max, Step, OSDDivider
        NULL, "Deinterlace",            // **pszList, Ini Section
        "GreedyVHSharpnessAmt", NULL,        // Ini name, pfnOnChange
    },

    {
        "Median Filter", SLIDER, 0,        // szDisplayName, TYPE, orig val
        &GreedyMedianFilterAmt, 5, 0,    // *pValue,Default, Min
        255, 1, 1,                        // Max, Step, OSDDivider
        NULL, "Deinterlace",            // **pszList, Ini Section
        "GreedyMedianFilterAmt", NULL,    // Ini name, pfnOnChange
    },
    {
        "High Comb Skip", SLIDER, 0,    // szDisplayName, TYPE, orig val
        &GreedyLowMotionPdLvl, 0, 0,    // *pValue,Default, Min
        100, 1, 1,                        // Max, Step, OSDDivider
        NULL, "Deinterlace",            // **pszList, Ini Section
        "GreedyLowMotionPdLvl", NULL,    // Ini name, pfnOnChange
    },
    {
        "Auto Pull-Down", ONOFF, 0,        // szDisplayName, TYPE, orig val
        &GreedyUsePulldown, FALSE, 0,    // *pValue,Default, Min
        1, 1, 1,                        // Max, Step, OSDDivider
        NULL, "Deinterlace",            // **pszList, Ini Section
        "GreedyUsePulldown", NULL,        // Ini name, pfnOnChange
    },
    {
        "In-Between Frames", ONOFF, 0,    // szDisplayName, TYPE, orig val
        &GreedyUseInBetween, FALSE, 0,    // *pValue,Default, Min
        1, 1, 1,                        // Max, Step, OSDDivider
        NULL, "Deinterlace",            // **pszList, Ini Section
        "GreedyUseInBetween", NULL,        // Ini name, pfnOnChange
    },
    {
        "Median Filter", ONOFF, 0,        // szDisplayName, TYPE, orig val
        &GreedyUseMedianFilter, FALSE, 0, // *pValue,Default, Min
        1, 1, 1,                        // Max, Step, OSDDivider
        NULL, "Deinterlace",            // **pszList, Ini Section
        "GreedyUseMedianFilter", NULL,    // Ini name, pfnOnChange
    },
    {
        "V. Sharpness", ONOFF, 0,    // szDisplayName, TYPE, orig val
        &GreedyUseVSharpness, FALSE, 0,    // *pValue,Default, Min
        1, 1, 1,                        // Max, Step, OSDDivider
        NULL, "Deinterlace",            // **pszList, Ini Section
        "GreedyUseVSharpness", NULL,        // Ini name, pfnOnChange
    },
    {
        "H. Sharpness", ONOFF, 0,        // szDisplayName, TYPE, orig val
        &GreedyUseHSharpness, FALSE, 0,    // *pValue,Default, Min
        1, 1, 1,                        // Max, Step, OSDDivider
        NULL, "Deinterlace",            // **pszList, Ini Section
        "GreedyUseHSharpness", NULL,        // Ini name, pfnOnChange
    },
};

HWND ghDlg = NULL;

void SetupSlider(SETTING* pSetting, HWND hSlider)
{
    Slider_SetRangeMax(hSlider, pSetting->MaxValue);
    Slider_SetRangeMin(hSlider, pSetting->MinValue);
    Slider_SetPageSize(hSlider, pSetting->StepValue);
    Slider_SetLineSize(hSlider, 1);
    Slider_SetTic(hSlider, pSetting->Default);
    Slider_SetPos(hSlider, *pSetting->pValue);
//    SetControlValue(pSetting, hSlider);
}

void SetGreedyHDisplayControls(HWND hDlg)
{
    // Set the sliders and visible numbers correctly
    SetupSlider(& DI_GreedyHSettings[GR_MAX_COMB],
        GetDlgItem(hDlg, IDC_MAX_COMB));
    SetDlgItemInt(hDlg, IDC_MAX_COMB_V, GreedyMaxComb, TRUE);

    SetupSlider(& DI_GreedyHSettings[GR_MOTION_THRESHOLD],
        GetDlgItem(hDlg, IDC_MOTION_THRESHOLD));
    SetDlgItemInt(hDlg, IDC_MOTION_THRESHOLD_V, GreedyMotionThreshold, TRUE);

    SetupSlider(& DI_GreedyHSettings[GR_MOTION_SENSE],
        GetDlgItem(hDlg, IDC_MOTION_SENSE));
    SetDlgItemInt(hDlg, IDC_MOTION_SENSE_V, GreedyMotionSense, TRUE);

    SetupSlider(& DI_GreedyHSettings[GR_GOOD_PULLDOWN_LVL],
        GetDlgItem(hDlg, IDC_GOOD_PULLDOWN_LVL));
    SetDlgItemInt(hDlg, IDC_GOOD_PULLDOWN_LVL_V, GreedyGoodPullDownLvl, TRUE);

    SetupSlider(& DI_GreedyHSettings[GR_BAD_PULLDOWN_LVL],
        GetDlgItem(hDlg, IDC_BAD_PULLDOWN_LVL));
    SetDlgItemInt(hDlg, IDC_BAD_PULLDOWN_LVL_V, GreedyBadPullDownLvl, TRUE);

    SetupSlider(& DI_GreedyHSettings[GR_H_SHARPNESS],
        GetDlgItem(hDlg, IDC_H_SHARPNESS));
    SetDlgItemInt(hDlg, IDC_H_SHARPNESS_V, GreedyHSharpnessAmt, TRUE);

    SetupSlider(& DI_GreedyHSettings[GR_V_SHARPNESS],
        GetDlgItem(hDlg, IDC_V_SHARPNESS));
    SetDlgItemInt(hDlg, IDC_V_SHARPNESS_V, GreedyVSharpnessAmt, TRUE);

    SetupSlider(& DI_GreedyHSettings[GR_MEDIAN_FILTER],
        GetDlgItem(hDlg, IDC_MEDIAN_FILTER));
    SetDlgItemInt(hDlg, IDC_MEDIAN_FILTER_V,    GreedyMedianFilterAmt, TRUE);

    SetupSlider(& DI_GreedyHSettings[GR_LOW_MOTION_PD_LVL],
        GetDlgItem(hDlg, IDC_LOW_MOTION_PD_LVL));
    SetDlgItemInt(hDlg, IDC_LOW_MOTION_PD_LVL_V, GreedyLowMotionPdLvl, TRUE);


    CheckDlgButton(hDlg, IDC_USE_PULLDOWN, GreedyUsePulldown);
    CheckDlgButton(hDlg, IDC_USE_IN_BETWEEN, GreedyUseInBetween);
    CheckDlgButton(hDlg, IDC_USE_MEDIAN_FILTER, GreedyUseMedianFilter);
    CheckDlgButton(hDlg, IDC_USE_V_SHARPNESS, GreedyUseVSharpness);
    CheckDlgButton(hDlg, IDC_USE_H_SHARPNESS, GreedyUseHSharpness);

}

// dialog code
BOOL APIENTRY GreedyHProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{

static int TGreedyMaxComb;                    // max comb we allow past clip
static int TGreedyMotionThreshold;            // ignore changes < this
static int TGreedyMotionSense;        // how rapidly to bob when > Threshold
static int TGreedyGoodPullDownLvl;
static int TGreedyBadPullDownLvl;
static int TGreedyHSharpnessAmt;                // % sharpness to add
static int TGreedyVSharpnessAmt;                // % sharpness to add
static int TGreedyMedianFilterAmt;            // Don't filter if > this
static int TGreedyLowMotionPdLvl;        // Skip if comb too large (inverse)

static BOOL TGreedyUsePulldown;
static BOOL TGreedyUseInBetween;
static BOOL TGreedyUseMedianFilter;
static BOOL TGreedyUseVSharpness;
static BOOL TGreedyUseHSharpness;

static BOOL ShowAdvanced = FALSE;
static BOOL ShowDiag = FALSE;

    switch (message)
    {
    case WM_INITDIALOG:

        // Capture the current global values
        if (!GreedySSEBox)
        {
            GreedyUsePulldown = FALSE;
            GreedyUseInBetween = FALSE;
            GreedyUseMedianFilter = FALSE;
            GreedyUseVSharpness = FALSE;
            GreedyUseHSharpness = FALSE;
        }

        TGreedyMaxComb = GreedyMaxComb;
        TGreedyMotionThreshold= GreedyMotionThreshold;
        TGreedyMotionSense = GreedyMotionSense;
        TGreedyGoodPullDownLvl= GreedyGoodPullDownLvl;
        TGreedyBadPullDownLvl = GreedyBadPullDownLvl;
        TGreedyHSharpnessAmt = GreedyHSharpnessAmt;
        TGreedyVSharpnessAmt = GreedyVSharpnessAmt;
        TGreedyMedianFilterAmt = GreedyMedianFilterAmt;
        TGreedyLowMotionPdLvl = GreedyLowMotionPdLvl;

        TGreedyUsePulldown = GreedyUsePulldown;
        TGreedyUseInBetween = GreedyUseInBetween;
        TGreedyUseMedianFilter = GreedyUseMedianFilter;
        TGreedyUseVSharpness = GreedyUseVSharpness;
        TGreedyUseHSharpness = GreedyUseHSharpness;

        if (ShowAdvanced)                   // if already showing then make small
        {
            if (ShowDiag)
            {
                MoveWindow(hDlg, 30, 30, 850, 470, TRUE);  // big window with trace
                GreedyDiag(hDlg);
            }
            else
            {                              // middle siz window with sliders & trace
                MoveWindow(hDlg, 30, 30, 352, 470, TRUE);
            }
        }
        else
        {
            MoveWindow(hDlg, 30, 30, 352, 205, TRUE);
        }


        SetGreedyHDisplayControls(hDlg);
        return TRUE;

        break;

    case WM_DESTROY:
        ghDlg = NULL;
        return TRUE;
        break;

    case WM_VSCROLL:
    case WM_HSCROLL:
        if((HWND)lParam == GetDlgItem(hDlg, IDC_MAX_COMB))
        {
            GreedyMaxComb = Slider_GetPos(GetDlgItem(hDlg, IDC_MAX_COMB));
            SetDlgItemInt(hDlg, IDC_MAX_COMB_V, GreedyMaxComb, TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_MOTION_THRESHOLD))
        {
            GreedyMotionThreshold= Slider_GetPos(GetDlgItem(hDlg, IDC_MOTION_THRESHOLD));
            SetDlgItemInt(hDlg, IDC_MOTION_THRESHOLD_V, GreedyMotionThreshold, TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_MOTION_SENSE))
        {
            GreedyMotionSense = Slider_GetPos(GetDlgItem(hDlg, IDC_MOTION_SENSE));
            SetDlgItemInt(hDlg, IDC_MOTION_SENSE_V, GreedyMotionSense, TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_GOOD_PULLDOWN_LVL))
        {
            GreedyGoodPullDownLvl= Slider_GetPos(GetDlgItem(hDlg, IDC_GOOD_PULLDOWN_LVL));
            SetDlgItemInt(hDlg, IDC_GOOD_PULLDOWN_LVL_V, GreedyGoodPullDownLvl, TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_BAD_PULLDOWN_LVL))
        {
            GreedyBadPullDownLvl = Slider_GetPos(GetDlgItem(hDlg, IDC_BAD_PULLDOWN_LVL));
            SetDlgItemInt(hDlg, IDC_BAD_PULLDOWN_LVL_V, GreedyBadPullDownLvl, TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_H_SHARPNESS))
        {
            GreedyHSharpnessAmt = Slider_GetPos(GetDlgItem(hDlg, IDC_H_SHARPNESS));
            SetDlgItemInt(hDlg, IDC_H_SHARPNESS_V, GreedyHSharpnessAmt, TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_V_SHARPNESS))
        {
            GreedyVSharpnessAmt = Slider_GetPos(GetDlgItem(hDlg, IDC_V_SHARPNESS));
            SetDlgItemInt(hDlg, IDC_V_SHARPNESS_V, GreedyVSharpnessAmt, TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_MEDIAN_FILTER))
        {
            GreedyMedianFilterAmt = Slider_GetPos(GetDlgItem(hDlg, IDC_MEDIAN_FILTER));
            SetDlgItemInt(hDlg, IDC_MEDIAN_FILTER_V, GreedyMedianFilterAmt, TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_LOW_MOTION_PD_LVL))
        {
            GreedyLowMotionPdLvl = Slider_GetPos(GetDlgItem(hDlg, IDC_LOW_MOTION_PD_LVL));
            SetDlgItemInt(hDlg, IDC_LOW_MOTION_PD_LVL_V, GreedyLowMotionPdLvl, TRUE);
        }
        break;

    case WM_MOUSEMOVE:

              return TRUE;

    case WM_COMMAND:

        switch LOWORD(wParam)
        {
        case IDOK:                            // Is Done, use our new parms
            DestroyWindow(hDlg);
            return TRUE;
            break;

        case IDCANCEL:

            // Restore the prev values
            GreedyMaxComb = TGreedyMaxComb;
            GreedyMotionThreshold = TGreedyMotionThreshold;
            GreedyMotionSense = TGreedyMotionSense;
            GreedyGoodPullDownLvl = TGreedyGoodPullDownLvl;
            GreedyBadPullDownLvl = TGreedyBadPullDownLvl;
            GreedyHSharpnessAmt = TGreedyHSharpnessAmt;
            GreedyVSharpnessAmt = TGreedyVSharpnessAmt;
            GreedyMedianFilterAmt = TGreedyMedianFilterAmt;
            GreedyLowMotionPdLvl = TGreedyLowMotionPdLvl;

            GreedyUsePulldown = TGreedyUsePulldown;
            GreedyUseInBetween = TGreedyUseInBetween;
            GreedyUseMedianFilter = TGreedyUseMedianFilter;
            GreedyUseVSharpness = TGreedyUseVSharpness;
            GreedyUseHSharpness = TGreedyUseHSharpness;

            DestroyWindow(hDlg);
            return TRUE;
            break;

        case IDC_USE_PULLDOWN:
            GreedyUsePulldown = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_USE_PULLDOWN));
            return TRUE;
            break;

        case IDC_USE_IN_BETWEEN:
            GreedyUseInBetween = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_USE_IN_BETWEEN));
            return TRUE;
            break;

        case IDC_USE_MEDIAN_FILTER:
            GreedyUseMedianFilter = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_USE_MEDIAN_FILTER));
            return TRUE;
            break;

        case IDC_USE_V_SHARPNESS:
            GreedyUseVSharpness = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_USE_V_SHARPNESS));
            return TRUE;
            break;

        case IDC_USE_H_SHARPNESS:
            GreedyUseHSharpness = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_USE_H_SHARPNESS));
            return TRUE;
            break;

        case IDC_DIAG:                          // expand screen, show trace
            if (ShowDiag)
            {
                ShowDiag = FALSE;
                MoveWindow(ghDlg, 30, 30, 352, 470, TRUE);  // middle siz window with sliders
            }
            else
            {
                ShowDiag = TRUE;
                MoveWindow(ghDlg, 30, 30, 850, 470, TRUE);  // big window with sliders & trace
                GreedyDiag(hDlg);
            }
            GreedyDiag(hDlg);
            break;

        case IDC_REFRESH:                       // refresh trace display
            GreedyDiag(hDlg);
            break;

        case IDC_ADVANCED:                      // show advanced screen
            if (ShowAdvanced)                   // if already showing then make small
            {
                ShowAdvanced = FALSE;
                MoveWindow(ghDlg, 30, 30, 352, 205, TRUE);
            }

            else
            {
                ShowAdvanced = TRUE;
                if (ShowDiag)
                {
                    MoveWindow(ghDlg, 30, 30, 800, 470, TRUE);  // big window with trace
                    GreedyDiag(hDlg);
                }
                else
                {                              // middle siz window with sliders & trace
                    MoveWindow(ghDlg, 30, 30, 352, 470, TRUE);
                }
            }

            break;

        case IDC_DEFAULT:
// Note - actual default values below should maybe be set in DI_GreedyHSETTINGS
            GreedyMaxComb = 5;                    // max comb we allow past clip
            GreedyMotionThreshold = 25;            // ignore changes < this
            GreedyMotionSense = 30;                // how rapidly to bob when > Threshold
            GreedyGoodPullDownLvl= 83;
            GreedyBadPullDownLvl = 88;
            GreedyHSharpnessAmt = 50;                // % sharpness to add
            GreedyVSharpnessAmt = 23;            // % sharpness (more needs more cpu)
            GreedyMedianFilterAmt = 3;            // Don't filter if > this
            GreedyLowMotionPdLvl = 9;            // Do PullDown on if motion < this

            GreedyUsePulldown = FALSE;
            GreedyUseInBetween = FALSE;
            GreedyUseMedianFilter = FALSE;
            GreedyUseVSharpness = FALSE;
            GreedyUseHSharpness = FALSE;

            SetGreedyHDisplayControls(hDlg);
            return TRUE;
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

void __cdecl GreedyHShowUI(HWND hwndMain)
{
    if(ghDlg == NULL)
    {
        ghDlg = CreateDialog(hInst, "GREEDYH", hwndMain, GreedyHProc);
    }
}

static int ct = 0;  // >>> for test only

void __cdecl GreedyHStartUp(long NumPlugIns, struct _DEINTERLACE_METHOD** OtherPlugins, DEINTERLACEPLUGINSETSTATUS* SetStatus)
{
        ++ct;

    if (lpFieldStore == NULL)
    {
 //       lpFieldStore = (__int64 *) calloc( FSSIZE, sizeof( __int64 ) );
    }
}



static int ct2 = 0;        // >>> for test only

void __cdecl GreedyHExit(void)
{
    ++ct2;
    if (ghDlg != NULL)
    {
        DestroyWindow(ghDlg);
    }

    if (lpFieldStore != NULL)
    {
//        free( lpFieldStore );   // >>> causes link error ???
    }

}


DEINTERLACE_METHOD GreedyHMethod =
{
    sizeof(DEINTERLACE_METHOD),        // size of this struct
    DEINTERLACE_CURRENT_VERSION,    // curr version compiled
    "Video (Greedy, High Motion)",    // What to display when selected
    "GreedyH",                        // Short name
    FALSE,                            // Is 1/2 height?
    FALSE,                            // Is film mode?
    DeinterlaceGreedyH_MMX,            // Pointer to Algorithm function (cannot be NULL)
    50,                                // flip frequency in 50Hz mode
    60,                                // flip frequency in 60Hz mode
    DI_GREEDYH_SETTING_LASTONE,        // number of settings
    DI_GreedyHSettings,                // ptr to start of Settings[nSettings]
    INDEX_VIDEO_GREEDYH,            // Index Number (position in menu) should map
    NULL,                            // to old enum value and d should be unique
    GreedyHStartUp,                    // call this if plugin needs to do anything before use
    GreedyHShowUI,                    // call this to display any UI, if NULL no UI is supoprted
    GreedyHExit,                     // call this if plugin needs to deallocate anything
    3,                                // how many fields are required to run this plug-in
    0,                                // Track number of mode Changes
    0,                                // Track Time in mode
    WM_DI_GREEDYH_GETVALUE - WM_APP, // the offset used by the external settings API
    NULL,                            // Dll module so that we can unload the dll cleanly at the end
    0,                                // Menu Id used for this plug-in, 0 to auto allocate one
    FALSE,                            // do we need FieldDiff filled in in info
    FALSE,                            // do we need CombFactor filled in in info
    IDH_GREEDYHM,
};


__declspec(dllexport) DEINTERLACE_METHOD* GetDeinterlacePluginInfo(long CpuFeatureFlags)
{
    GreedyFeatureFlags = CpuFeatureFlags;
    if (CpuFeatureFlags & (FEATURE_SSE | FEATURE_MMXEXT) )   // Pentium 3,4, or Athlon?
    {
        GreedySSEBox = TRUE;
        GreedyHMethod.pfnAlgorithm = DeinterlaceGreedyH_SSE;
    }
    else if (CpuFeatureFlags & FEATURE_3DNOW)
    {
        GreedySSEBox = FALSE;
        GreedyHMethod.pfnAlgorithm = DeinterlaceGreedyH_3DNOW;
    }
    else
  {
        GreedySSEBox = FALSE;
        GreedyHMethod.pfnAlgorithm = DeinterlaceGreedyH_MMX;
    }
    return &GreedyHMethod;
}

BOOL WINAPI NoCRTDllMain(HANDLE hInstance, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    hInst = hInstance;
    return TRUE;
}

int ShowPdHistFlags(HWND hDlg, int CombN, int Kontrast,  int MotionN, int RatN,
                        int CombK, int Flags, GR_PULLDOWN_INFO Hist)
{
    char OFlags[9] = "0       ";
    SetDlgItemInt(hDlg, CombN, Hist.Comb , TRUE);
    SetDlgItemInt(hDlg, Kontrast, Hist.Kontrast , TRUE);
    if (Hist.Kontrast == 0 || Hist.Motion == 0)
    {
        SetDlgItemInt(hDlg, MotionN, 0 , TRUE);
    }
    else
    {

        SetDlgItemInt(hDlg, Kontrast,  (100*Hist.CombChoice - 70* Hist.Kontrast )/ Hist.Motion  , TRUE);
        SetDlgItemInt(hDlg, MotionN, Hist.Motion / 10 , TRUE);
        SetDlgItemInt(hDlg, CombK, 100 * Hist.CombChoice / Hist.Kontrast , TRUE);
        SetDlgItemInt(hDlg, RatN,  1000 * Hist.Avg / (Hist.Kontrast * PDAVGLEN) , TRUE);
    }
    if (Hist.Flags & 1)
    {
        OFlags[0] = '1';
    }

    if (Hist.Flags2 & PD_VIDEO)
    {
        OFlags[1] = 'V';
    }

    if (Hist.Flags2 & PD_32_PULLDOWN)
    {
        OFlags[2] = '3';
    }

    else if (Hist.Flags2 & PD_22_PULLDOWN)
    {
        OFlags[2] = '2';
    }

    else if (Hist.Flags2 & PD_LOW_MOTION)
    {
        OFlags[2] = 'L';
    }

    if (Hist.Flags2 & PD_PULLDOWN)
    {
        OFlags[3] = 'P';
    }

    if (Hist.Flags2 & PD_BAD)
    {
        OFlags[4] = 'B';
    }

    if (Hist.Flags2 & PD_MERGED)
    {
        OFlags[5] = 'M';
    }

    if (!(Hist.Flags2 & PD_ODD))
    {
        OFlags[6] = 'e';
    }
    if (Hist.Flags2 == 0)
    {
        OFlags[7] = '?';
    }
    SetDlgItemText(hDlg, Flags, OFlags);

    return 0;
}

int GreedyDiag(HWND hDlg)
{
    GR_PULLDOWN_INFO Hist[15];
    GetHistData(&Hist[0], 15);

    if (Hist[0].AvgChoice > 0)
    {
        ShowPdHistFlags(hDlg, IDC_COMB0, IDC_KONTRAST0, IDC_MOTION0, IDC_RAT0, IDC_COMBK0, IDC_FLAG0, Hist[0]);
        ShowPdHistFlags(hDlg, IDC_COMB1, IDC_KONTRAST1, IDC_MOTION1, IDC_RAT1, IDC_COMBK1, IDC_FLAG1, Hist[1]);
        ShowPdHistFlags(hDlg, IDC_COMB2, IDC_KONTRAST2, IDC_MOTION2, IDC_RAT2, IDC_COMBK2, IDC_FLAG2, Hist[2]);
        ShowPdHistFlags(hDlg, IDC_COMB3, IDC_KONTRAST3, IDC_MOTION3, IDC_RAT3, IDC_COMBK3, IDC_FLAG3, Hist[3]);
        ShowPdHistFlags(hDlg, IDC_COMB4, IDC_KONTRAST4, IDC_MOTION4, IDC_RAT4, IDC_COMBK4, IDC_FLAG4, Hist[4]);
        ShowPdHistFlags(hDlg, IDC_COMB5, IDC_KONTRAST5, IDC_MOTION5, IDC_RAT5, IDC_COMBK5, IDC_FLAG5, Hist[5]);
        ShowPdHistFlags(hDlg, IDC_COMB6, IDC_KONTRAST6, IDC_MOTION6, IDC_RAT6, IDC_COMBK6, IDC_FLAG6, Hist[6]);
        ShowPdHistFlags(hDlg, IDC_COMB7, IDC_KONTRAST7, IDC_MOTION7, IDC_RAT7, IDC_COMBK7, IDC_FLAG7, Hist[7]);
        ShowPdHistFlags(hDlg, IDC_COMB8, IDC_KONTRAST8, IDC_MOTION8, IDC_RAT8, IDC_COMBK8, IDC_FLAG8, Hist[8]);
        ShowPdHistFlags(hDlg, IDC_COMB9, IDC_KONTRAST9, IDC_MOTION9, IDC_RAT9, IDC_COMBK9, IDC_FLAG9, Hist[9]);
        ShowPdHistFlags(hDlg, IDC_COMB10, IDC_KONTRAST10, IDC_MOTION10, IDC_RAT10, IDC_COMBK10, IDC_FLAG10, Hist[10]);
        ShowPdHistFlags(hDlg, IDC_COMB11, IDC_KONTRAST11, IDC_MOTION11, IDC_RAT11, IDC_COMBK11, IDC_FLAG11, Hist[11]);
        ShowPdHistFlags(hDlg, IDC_COMB12, IDC_KONTRAST12, IDC_MOTION12, IDC_RAT12, IDC_COMBK12, IDC_FLAG12, Hist[12]);
        ShowPdHistFlags(hDlg, IDC_COMB13, IDC_KONTRAST13, IDC_MOTION13, IDC_RAT13, IDC_COMBK13, IDC_FLAG13, Hist[13]);
        ShowPdHistFlags(hDlg, IDC_COMB14, IDC_KONTRAST14, IDC_MOTION14, IDC_RAT14, IDC_COMBK14, IDC_FLAG14, Hist[14]);
    }

    return 0;
}


// Entered here on deinterlace call for SSE
BOOL DeinterlaceGreedyH_SSE(TDeinterlaceInfo *info)
{
    if ( !GetCheckDScalerInfo(info) || !(GreedyFeatureFlags & (FEATURE_SSE | FEATURE_MMXEXT)) )
    {
        return FALSE;
    }

    else if (GreedyUseMedianFilter || GreedyUsePulldown
        || GreedyUseVSharpness || GreedyUseHSharpness)
    {
        return DI_GreedyHM();           // only SSE version for this one
    }
    else
    {
        return DI_GreedyHF_SSE(info);  // faster more compatible way
    }
}

// Entered here on deinterlace call for 3DNOW
BOOL DeinterlaceGreedyH_3DNOW(TDeinterlaceInfo *info)
{
    if ( !GetCheckDScalerInfo(info) || !(GreedyFeatureFlags & FEATURE_3DNOW) )
    {
        return FALSE;
    }

    return DI_GreedyHF_3DNOW(info);    // faster more compatible way
}

// Entered here on deinterlace call for MMX
BOOL DeinterlaceGreedyH_MMX(TDeinterlaceInfo *info)
{
    if ( !GetCheckDScalerInfo(info) || !(GreedyFeatureFlags & FEATURE_MMX) )
    {
        return FALSE;
    }

    return DI_GreedyHF_MMX(info);  // faster more compatible way
}


// We fill in some GreedyHM external values. These should be in a parm structure, but aren't
BOOL GetCheckDScalerInfo(TDeinterlaceInfo *info)
{
    #include "DI_GreedyHM2.h"

    __int64 i;
//>>>    InfoIsOdd = info->IsOdd;
//>>>    InfoIsOdd = (info->PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD) ? 1 : 0;
    InfoIsOdd = (info->PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD) ? 1 : 0;

    OverlayPitch =info->OverlayPitch;
    InpPitch =  info->InputPitch;
    lpCurOverlay = info->Overlay;
    pLines = info->PictureHistory[0]->pData;

 /*
    pOddLines = info->OddLines[0];
    pEvenLines = info->EvenLines[0];

    pPrevLines = info->IsOdd
        ? info->OddLines[1]
        : info->EvenLines[1];

    pLines = InfoIsOdd
        ? pOddLines
        : pEvenLines;
*/
    FieldHeight = info->FieldHeight;
    FrameHeight = info->FrameHeight;
    LineLength = info->LineLength;
    pMemcpy = info->pMemcpy;

    // Set up our two parms that are actually evaluated for each pixel
    i=GreedyMaxComb;
    MaxComb = i << 56 | i << 48 | i << 40 | i << 32 | i << 24 | i << 16 | i << 8 | i;

    i = GreedyMotionThreshold;        // scale to range of 0-257
    MotionThreshold = i << 48 | i << 32 | i << 16 | i | UVMask;

    i = GreedyMotionSense ;        // scale to range of 0-257
    MotionSense = i << 48 | i << 32 | i << 16 | i;

    i = GreedyGoodPullDownLvl;                    // scale to range of 0-257
    EdgeThreshold = i << 48 | i << 32 | i << 16 | i | UVMask;

    i=GreedyBadPullDownLvl * 128 / 100;
    EdgeSense =  i << 48 | i << 32  | i << 16  | i;

    i=GreedyMedianFilterAmt;
    MedianFilterAmt =  i << 48 | i << 32 | i << 16 | i;

    i=GreedyHSharpnessAmt* 257/100;
    HSharpnessAmt =  i << 48 | i << 32 | i << 16 | i;
    return TRUE;
//>>>>    return (FieldHeight < FSMAXROWS);
}


