/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Laurent Garnier.  All rights reserved.
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
// Change Log
//
// Date          Developer             Changes
//
// 23 Jul 2001   Laurent Garnier       File created
//
/////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "Calibration.h"
#include "resource.h"
#include "DScaler.h"
#include "Settings.h"
#include "OSD.h"
#include "DebugLog.h"


// Minimum time in milliseconds between two consecutive evaluations
#define	MIN_TIME_BETWEEN_CALC	50
// Number of calculations to do on successive frames before to decide what to adjust
#define NB_CALCULATIONS         8
// Delta used to stop the search process when the current value for setting implies
// a result that is superior to this delta when compared to the previous found best result
#define DELTA_STOP              5
// Maximum value
#define MAX_VALUE               1000000

// Macro to restrict range to [0,255]
#define LIMIT(x) (((x)<0)?0:((x)>255)?255:(x))

// Macro to return the absolute value
#define ABSOLUTE_VALUE(x) ((x) < 0) ? -(x) : (x)


/////////////////////////////////////////////////////////////////////////////
// Class CColorBar

CColorBar::CColorBar(unsigned short int left, unsigned short int right, unsigned short int top, unsigned short int bottom, BOOL YUV, unsigned char R_Y, unsigned char G_U, unsigned char B_V)
{ 
    left_border = left; 
    right_border = right; 
    top_border = top; 
    bottom_border = bottom;
    if (YUV)
    {
        ref_Y_val = R_Y;
        ref_U_val = G_U;
        ref_V_val = B_V;
        // Calculate RGB values from YUV values
        YUV2RGB(ref_Y_val, ref_U_val, ref_V_val, &ref_R_val, &ref_G_val, &ref_B_val);
    }
    else
    {
        ref_R_val = R_Y;
        ref_G_val = G_U;
        ref_B_val = B_V;
        // Calculate YUV values from RGB values
        RGB2YUV(ref_R_val, ref_G_val, ref_B_val, &ref_Y_val, &ref_U_val, &ref_V_val);
    }

    cpt_Y = cpt_U = cpt_V = cpt_nb = 0;
}

CColorBar::CColorBar(CColorBar *pColorBar)
{
    unsigned char val1, val2, val3;
    unsigned short int left, right, top, bottom;

    pColorBar->GetPosition(&left, &right, &top, &bottom);
    left_border = left; 
    right_border = right; 
    top_border = top; 
    bottom_border = bottom;

    pColorBar->GetRefColor(FALSE, &val1, &val2, &val3);
    ref_R_val = val1;
    ref_G_val = val2;
    ref_B_val = val3;

    pColorBar->GetRefColor(TRUE, &val1, &val2, &val3);
    ref_Y_val = val1;
    ref_U_val = val2;
    ref_V_val = val3;

    cpt_Y = cpt_U = cpt_V = cpt_nb = 0;
}

// This methode returns the position of the color bar
void CColorBar::GetPosition(unsigned short int *left, unsigned short int *right, unsigned short int *top, unsigned short int *bottom)
{
    *left = left_border;
    *right = right_border;
    *top = top_border;
    *bottom = bottom_border;
}

// This methode returns the reference color
// If parameter YUV is TRUE, it returns YUV values else RGB values
void CColorBar::GetRefColor(BOOL YUV, unsigned char *pR_Y, unsigned char *pG_U, unsigned char *pB_V)
{ 
    if (YUV)
    {
        *pR_Y = ref_Y_val;
        *pG_U = ref_U_val;
        *pB_V = ref_V_val;
    }
    else
    {
        *pR_Y = ref_R_val;
        *pG_U = ref_G_val;
        *pB_V = ref_B_val;
    }
}

// This methode returns the calculated average color
// If parameter YUV is TRUE, it returns YUV values else RGB values
void CColorBar::GetCurrentAvgColor(BOOL YUV, unsigned char *pR_Y, unsigned char *pG_U, unsigned char *pB_V)
{ 
    if (YUV)
    {
        *pR_Y = Y_val;
        *pG_U = U_val;
        *pB_V = V_val;
    }
    else
    {
        *pR_Y = R_val;
        *pG_U = G_val;
        *pB_V = B_val;
    }
}

// This methode returns the delta between reference color and calculated average color
// It includes a delta for each color component + the sum of these three absolute deltas.
// If parameter YUV is TRUE, it returns YUV values else RGB values
void CColorBar::GetDeltaColor(BOOL YUV, int *pR_Y, int *pG_U, int *pB_V, int *pTotal)
{
    if (YUV)
    {
        *pR_Y = Y_val - ref_Y_val;
        *pG_U = U_val - ref_U_val;
        *pB_V = V_val - ref_V_val;
    }
    else
    {
        *pR_Y = R_val - ref_R_val;
        *pG_U = G_val - ref_G_val;
        *pB_V = B_val - ref_B_val;
    }

    *pTotal = 0;
    *pTotal += ABSOLUTE_VALUE(*pR_Y);
    *pTotal += ABSOLUTE_VALUE(*pG_U);
    *pTotal += ABSOLUTE_VALUE(*pB_V);
}

// This method analyzed the overlay buffer to calculate average color
// in the zone defined by the color bar
BOOL CColorBar::CalcAvgColor(BOOL reinit, unsigned int nb_calc_needed, short **Lines, int height, int width)
{ 
    int left, right, top, bottom, i, j;
    unsigned int Y, U, V, nb_Y, nb_U, nb_V;
    BYTE *buf;

    if (reinit)
    {
        cpt_Y = 0;
        cpt_U = 0;
        cpt_V = 0;
        cpt_nb = 0;
    }

    // Calculate the exact coordinates of rectangular zone in the buffer
    left = width * left_border / 10000;
    right = width * right_border / 10000;
    top = height * top_border / 10000;
    bottom = height * bottom_border / 10000;

    // Sum separately Y, U and V in this rectangular zone
    // Each line is like this : YUYVYUYV...
    Y = 0; nb_Y = 0;
    U = 0; nb_U = 0;
    V = 0; nb_V = 0;
    for (i = top ; i <= bottom ; i++)
    {
        for (j = left ; j <= right ; j++)
        {
            buf = (BYTE *)Lines[i];
            Y += buf[j*2];
            nb_Y++;
            if (j % 2)
            {
                V += buf[j*2+1];
                nb_V++;
            }
            else
            {
                U += buf[j*2+1];
                nb_U++;
            }
        }
    }

    // Calculate the average for Y, U and V
//    if (nb_Y > 0)
//    {
//        cpt_Y += (Y + (nb_Y / 2)) / nb_Y;
//    }
//    if (nb_U > 0)
//    {
//        cpt_U += (U + (nb_U / 2)) / nb_U;
//    }
//    if (nb_V > 0)
//    {
//        cpt_V += (V + (nb_V / 2)) / nb_V;
//    }
    cpt_Y += Y;
    cpt_U += U;
    cpt_V += V;
    cpt_nb++;

    if (cpt_nb >= nb_calc_needed)
    {
//        Y_val = (cpt_Y + (cpt_nb / 2)) / cpt_nb;
//        U_val = (cpt_U + (cpt_nb / 2)) / cpt_nb;
//        V_val = (cpt_V + (cpt_nb / 2)) / cpt_nb;
        if (nb_Y > 0)
        {
            Y_val = (cpt_Y + (cpt_nb * nb_Y / 2)) / (cpt_nb * nb_Y);
        }
        else
        {
            Y_val = 0;
        }
        if (nb_Y > 0)
        {
            U_val = (cpt_U + (cpt_nb * nb_U / 2)) / (cpt_nb * nb_U);
        }
        else
        {
            U_val = 0;
        }
        if (nb_Y > 0)
        {
            V_val = (cpt_V + (cpt_nb * nb_V / 2)) / (cpt_nb * nb_V);
        }
        else
        {
            V_val = 0;
        }
        V_val = (cpt_V + (cpt_nb * nb_V / 2)) / (cpt_nb * nb_V);

        // Save corresponding RGB values too
        YUV2RGB(Y_val, U_val, V_val, &R_val, &G_val, &B_val);

        LOG(5, "CalcAvgColor YUV %d %d %d %d %d %d %d %d %d", Y_val, U_val, V_val, left, right, top, bottom, height, width);
        LOG(5, "CalcAvgColor RGB %d %d %d %d %d %d %d %d %d", R_val, G_val, B_val, left, right, top, bottom, height, width);

        cpt_Y = 0;
        cpt_U = 0;
        cpt_V = 0;
        cpt_nb = 0;

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

// Convert RGB to YUV
void CColorBar::RGB2YUV(unsigned char R, unsigned char G, unsigned char B, unsigned char *pY, unsigned char *pU, unsigned char *pV)
{
    unsigned int y, cr, cb;

//    y  = ( 16840*R + 33058*G +  6405*B + 1048576)>>16;
//    cr = ( 28781*R - 24110*G -  4671*B + 8388608)>>16;
//    cb = ( -9713*R - 19068*G + 28781*B + 8388608)>>16;
    y  = ( 16843*R + 33030*G +  6423*B + 1048576)>>16;
    cr = ( 28770*R - 24117*G -  4653*B + 8388608)>>16;
    cb = ( -9699*R - 19071*G + 28770*B + 8388608)>>16;

    *pY = LIMIT(y);
    *pU = LIMIT(cb);
    *pV = LIMIT(cr);
}

// Convert YUV to RGB
void CColorBar::YUV2RGB(unsigned char Y, unsigned char U, unsigned char V, unsigned char *pR, unsigned char *pG, unsigned char *pB)
{
    int y, cr, cb, r, g, b;

    cb = U - 128;
    cr = V - 128;
    y = Y - 16;

    r = ( 76284*y + 104595*cr             )>>16;
    g = ( 76284*y -  53281*cr -  25625*cb )>>16;
    b = ( 76284*y             + 132252*cb )>>16;

    *pR = LIMIT(r);
    *pG = LIMIT(g);
    *pB = LIMIT(b);
}


/////////////////////////////////////////////////////////////////////////////
// Class CSubPattern

CSubPattern::CSubPattern(eTypeAdjust type)
{
    type_adjust = type;
    nb_color_bars = 0;
    for (int i = 0 ; i < MAX_COLOR_BARS ; i++)
    {
        color_bars[i] = NULL;
    }
    idx_color_bar = -1;
}

CSubPattern::~CSubPattern()
{
    for (int i = 0 ; i < MAX_COLOR_BARS ; i++)
    {
        if (color_bars[i] != NULL)
        {
            delete color_bars[i];
        }
    }
}

// This method returns the type of settings that can be adjusted with this sub-pattern
eTypeAdjust CSubPattern::GetTypeAdjust()
{
    return type_adjust;
}

// This method allows to add a new color bar in the sub-pattern
// Returns 0 if the color bar is correctly added
int CSubPattern::AddColorBar(CColorBar *color_bar)
{
    int i;

    for (i = 0 ; i < MAX_COLOR_BARS ; i++)
    {
        if (color_bars[i] == NULL)
        {
            break;
        }
    }
    if (i < MAX_COLOR_BARS)
    {
        color_bars[i] = color_bar;
        nb_color_bars++;
        return 0;
    }
    else
    {
        return -1;
    }
}

// This method returns the first bar of the test pattern
// The first is the first added
// Returns NULL pointer if no color bar is defined for the sub-pattern
CColorBar *CSubPattern::GetFirstColorBar()
{
    for (idx_color_bar = 0 ; idx_color_bar < MAX_COLOR_BARS ; idx_color_bar++)
    {
        if (color_bars[idx_color_bar] != NULL)
        {
            return color_bars[idx_color_bar];
        }
    }
    idx_color_bar = -1;
    return NULL;
}

// This method returns the next color bar of the sub-pattern
// GetFirstColorBar must be called at least one time before calling GetNextColorBar
// Returns NULL pointer if we were already on the last color bar
CColorBar *CSubPattern::GetNextColorBar()
{
    for (idx_color_bar++; idx_color_bar < MAX_COLOR_BARS ; idx_color_bar++)
    {
        if (color_bars[idx_color_bar] != NULL)
        {
            return color_bars[idx_color_bar];
        }
    }
    idx_color_bar = -1;
    return NULL;
}

// This function searches in the sub-pattern if it exists color
// bars with same features as the one in argument
// Returns one of the found color bars or NULL if no found
CColorBar *CSubPattern::FindSameCoclorBar(CColorBar *pColorBar)
{
    CColorBar *pCurBar;
    unsigned short int left, right, top, bottom;
    unsigned char Y, U, V;
    unsigned short int left2, right2, top2, bottom2;
    unsigned char Y2, U2, V2;

    pColorBar->GetPosition(&left, &right, &top, &bottom);
    pColorBar->GetRefColor(TRUE, &Y, &U, &V);

    pCurBar = GetFirstColorBar();
    while (pCurBar != NULL)
    {
        pCurBar->GetPosition(&left2, &right2, &top2, &bottom2);
        pCurBar->GetRefColor(TRUE, &Y2, &U2, &V2);
        if ( (left == left2)
          && (right == right2)
          && (top == top2)
          && (bottom == bottom2)
          && (Y == Y2)
          && (U == U2)
          && (V == V2) )
        {
            break;
        }
        pCurBar = GetNextColorBar();
    }
    return pCurBar;
}

// This method analyzes the current overlay buffer
BOOL CSubPattern::CalcCurrentSubPattern(BOOL reinit, unsigned int nb_calc_needed, short **Lines, int height, int width)
{
    BOOL result_avail;

    // Do the job for each defined color bar
    for (int i = 0 ; i < MAX_COLOR_BARS ; i++)
    {
        if (color_bars[i] != NULL)
        {
            result_avail = color_bars[i]->CalcAvgColor(reinit, nb_calc_needed, Lines, height, width);
        }
    }

    return result_avail;
}

// This methode returns the sum of absolute delta between reference color
// and calculated average color through all the color bars
void CSubPattern::GetSumDeltaColor(BOOL YUV, int *pR_Y, int *pG_U, int *pB_V, int *pTotal)
{
    int i, j;
    int delta[4];
    int sum_delta[4];

    // Set the sums to 0
    for (j = 0 ; j <= 3 ; j++)
    {
        sum_delta[j] = 0;
    }
    // Go through all the color bars
    for (i = 0 ; i < MAX_COLOR_BARS ; i++)
    {
        if (color_bars[i] != NULL)
        {
            color_bars[i]->GetDeltaColor(YUV, &delta[0], &delta[1], &delta[2], &delta[3]);
            for (j = 0 ; j <= 3 ; j++)
            {
                sum_delta[j] += ABSOLUTE_VALUE(delta[j]);
            }
        }
    }

    *pR_Y = sum_delta[0];
    *pG_U = sum_delta[1];
    *pB_V = sum_delta[2];
    *pTotal = sum_delta[3];
}
	
/////////////////////////////////////////////////////////////////////////////
// Class CTestPattern

CTestPattern::CTestPattern(char* name, eVideoFormat format)
{
    strcpy(pattern_name, name);
    video_format = format;
    nb_sub_patterns = 0;
    for (int i = 0 ; i < MAX_SUB_PATTERNS ; i++)
    {
        sub_patterns[i] = NULL;
    }
    idx_sub_pattern = -1;
}

CTestPattern::~CTestPattern()
{
    for (int i = 0 ; i < MAX_SUB_PATTERNS ; i++)
    {
        if (sub_patterns[i] != NULL)
        {
            delete sub_patterns[i];
        }
    }
}

// This method returns the name of the test pattern
char *CTestPattern::GetName()
{
    return pattern_name;
}

// This method returns the video format of the test pattern
eVideoFormat CTestPattern::GetVideoFormat()
{
    return video_format;
}

// This method allows to add a new sub-pattern to the test pattern
// Returns 0 if the sub-pattern is correctly added
int CTestPattern::AddSubPattern(CSubPattern *sub_pattern)
{
    int i;

    for (i = 0 ; i < MAX_SUB_PATTERNS ; i++)
    {
        if (sub_patterns[i] == NULL)
        {
            break;
        }
    }
    if (i < MAX_SUB_PATTERNS)
    {
        sub_patterns[i] = sub_pattern;
        nb_sub_patterns++;
        return 0;
    }
    else
    {
        return -1;
    }
}

// This method allows to create a new sub-pattern to the test pattern
// which is a merge of all the others sub-patterns
// Returns 0 if the sub-pattern is correctly created
int CTestPattern::CreateGlobalSubPattern()
{
    int i;
    CSubPattern *sub_pattern;
    CColorBar *pCurBar;
    CColorBar *pNewBar;

    // Create the new sub-pattern
    sub_pattern = new CSubPattern(ADJ_MANUAL);

    for (i = 0 ; i < MAX_SUB_PATTERNS ; i++)
    {
        if (sub_patterns[i] == NULL)
        {
            break;
        }
        else
        {
            // Add all the color bars from the current sub-pattern
            // to the new sub-pattern
            pCurBar = sub_patterns[i]->GetFirstColorBar();
            while (pCurBar != NULL)
            {
                if (sub_pattern->FindSameCoclorBar(pCurBar) == NULL)
                {
                    pNewBar = new CColorBar(pCurBar);
                    sub_pattern->AddColorBar(pNewBar);
                }
                pCurBar = sub_patterns[i]->GetNextColorBar();
            }
        }
    }
    if (i < MAX_SUB_PATTERNS)
    {
        sub_patterns[i] = sub_pattern;
        nb_sub_patterns++;
        return 0;
    }
    else
    {
        delete sub_pattern;
        return -1;
    }
}

// This method determines the type of content in the test pattern
// going all over the sub-patterns
eTypeContentPattern CTestPattern::DetermineTypeContent()
{
    if ( (GetSubPattern(ADJ_BRIGHTNESS) != NULL)
      && (GetSubPattern(ADJ_CONTRAST) != NULL)
      && (GetSubPattern(ADJ_SATURATION_U) != NULL)
      && (GetSubPattern(ADJ_SATURATION_V) != NULL)
      && (GetSubPattern(ADJ_HUE) != NULL) )
        return PAT_GRAY_AND_COLOR;

    if ( (GetSubPattern(ADJ_BRIGHTNESS) != NULL)
      && (GetSubPattern(ADJ_CONTRAST) != NULL) )
        return PAT_RANGE_OF_GRAY;

    if ( (GetSubPattern(ADJ_SATURATION_U) != NULL)
      && (GetSubPattern(ADJ_SATURATION_V) != NULL)
      && (GetSubPattern(ADJ_HUE) != NULL) )
        return PAT_COLOR;

    return PAT_UNKNOWN;
}

// This method returns the (first) sub-pattern allowing to adjust particular settings
// Returns NULL pointer if no sub-pattern allows to do this type of adjustments
CSubPattern *CTestPattern::GetSubPattern(eTypeAdjust type_adjust)
{
    CSubPattern *sub_pattern;

    sub_pattern = GetFirstSubPattern();
    while (sub_pattern != NULL)
    {
        if (sub_pattern->GetTypeAdjust() == type_adjust)
        {
            break;
        }
        sub_pattern = GetNextSubPattern();
    }
    return sub_pattern;
}

// This method returns the first sub-pattern of the test pattern
// The first is the first added
// Returns NULL pointer if no sub-pattern is defined for the test pattern
CSubPattern *CTestPattern::GetFirstSubPattern()
{
    for (idx_sub_pattern = 0 ; idx_sub_pattern < MAX_SUB_PATTERNS ; idx_sub_pattern++)
    {
        if (sub_patterns[idx_sub_pattern] != NULL)
        {
            return sub_patterns[idx_sub_pattern];
        }
    }
    idx_sub_pattern = -1;
    return NULL;
}

// This method returns the next sub-pattern of the test pattern
// GetFirstSubPattern must be called at least one time before calling GetNextSubPattern
// Returns NULL pointer if we were already on the last sub-pattern
CSubPattern *CTestPattern::GetNextSubPattern()
{
    for (idx_sub_pattern++; idx_sub_pattern < MAX_SUB_PATTERNS ; idx_sub_pattern++)
    {
        if (sub_patterns[idx_sub_pattern] != NULL)
        {
            return sub_patterns[idx_sub_pattern];
        }
    }
    idx_sub_pattern = -1;
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// Class CCalSetting

CCalSetting::CCalSetting(BT848_SETTING setting)
{
    type_setting = setting;
    min = BT848_GetSetting(type_setting)->MinValue;
    max = BT848_GetSetting(type_setting)->MaxValue;
    current_value = Setting_GetValue(BT848_GetSetting(type_setting));
    SetFullRange();
    InitResult();
}

BOOL CCalSetting::Update()
{
    int new_value = Setting_GetValue(BT848_GetSetting(type_setting));
    if (new_value != current_value)
    {
        current_value = new_value;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void CCalSetting::Save()
{
    saved_value = current_value;
    LOG(2, "Automatic Calibration - %s saved value = %d", BT848_GetSetting(type_setting)->szDisplayName, saved_value);
}

void CCalSetting::Restore()
{
    Setting_SetValue(BT848_GetSetting(type_setting), saved_value);
    LOG(2, "Automatic Calibration - %s restored value = %d", BT848_GetSetting(type_setting)->szDisplayName, saved_value);
}

void CCalSetting::SetFullRange()
{
    SetRange(min, max);
}

void CCalSetting::SetRange(int min_val, int max_val)
{
    int i, j;

    min_value = min_val;
    max_value = max_val;
    for (i=0 ; i<16 ; i++)
    {
        mask_input[i] = 0;
    }
    for (i=min_val ; i<=max_val ; i++)
    {
        j = i - min;
        mask_input[j/32] |= (1 << (j%32));
    }

    LOG(3, "Automatic Calibration - %s range => min = %d max = %d", BT848_GetSetting(type_setting)->szDisplayName, min_value, max_value);
}

void CCalSetting::SetRange(int delta)
{
    int min_val, max_val;

    min_val = Setting_GetValue(BT848_GetSetting(type_setting)) - delta;
    if (min_val < min)
    {
        min_val = min;
    }
    max_val = Setting_GetValue(BT848_GetSetting(type_setting)) + delta;
    if (max_val > max)
    {
        max_val = max;
    }
    SetRange(min_val, max_val);
}

void CCalSetting::SetRange(int *mask)
{
    int i, nb;

    for (i=0 ; i<16 ; i++)
    {
        mask_input[i] = mask[i];
    }
    for (i=0,nb=0 ; i<=(max - min) ; i++)
    {
        if (mask[i/32] & (1 << (i%32)))
        {
            nb++;
            if (nb == 1)
            {
                min_value = i + min;
                max_value = i + min;
            }
            else
            {
                max_value = i + min;
            }
        }
    }
}

void CCalSetting::GetRange(int *mask, int *min_val, int *max_val)
{
    for (int i=0 ; i<16; i++)
    {
        mask[i] = mask_input[i];
    }
    *min_val = min_value;
    *max_val = max_value;
}

void CCalSetting::AdjustMin()
{
    Adjust(min_value);
}

void CCalSetting::AdjustMax()
{
    Adjust(max_value);
}

void CCalSetting::AdjustDefault()
{
    Adjust(BT848_GetSetting(type_setting)->Default);
}

BOOL CCalSetting::AdjustNext()
{
    int i, j;

    if (end)
    {
        return FALSE;
    }

    for (i=(current_value+1) ; i<=max_value ; i++)
    {
        j = i - min;
        if (mask_input[j/32] & (1 << (j%32)))
        {
            Adjust(i);
            return TRUE;
        }
    }

    return FALSE;
}

void CCalSetting::AdjustBest()
{
    int nb_min;
    int best_val_min;
    int best_val_max;
    int mask[16];

    nb_min = GetResult(&mask[0], &best_val_min, &best_val_max);
    if (nb_min > 0)
    {
        // Set the setting to one of the best found values
        Adjust(best_val_max);
    }
    else
    {
        // Set the setting to its default value
        AdjustDefault();
    }

    LOG(2, "Automatic Calibration - %s finished - %d values between %d and %d => %d", BT848_GetSetting(type_setting)->szDisplayName, nb_min, best_val_min, best_val_max, current_value);
}

void CCalSetting::InitResult()
{
    int i;

    min_diff = MAX_VALUE;
    max_diff = 0;
    desc = FALSE;
    for (i=0 ; i<16 ; i++)
    {
        mask_output[i] = 0;
    }
    end = FALSE;
}

BOOL CCalSetting::UpdateResult(int diff, int threshold, BOOL only_one)
{
    int i, j;
    BOOL min_found = FALSE;

    if (diff > max_diff)
    {
        max_diff = diff;
    }
    i = current_value - min;
    if (diff < min_diff)
    {
        min_diff = diff;

        for (j=0 ; j<16 ; j++)
        {
            mask_output[j] = 0;
        }
        mask_output[i/32] = (1 << (i%32));

        if ((threshold >= 0) && ((max_diff - min_diff) > threshold))
        {
            desc = TRUE;
        }
    }
    else if ((diff == min_diff) && !only_one)
    {
        mask_output[i/32] |= (1 << (i%32));
    }
    else if ((threshold >= 0) && ((diff - min_diff) > threshold))
    {
        end = TRUE;
        min_found = desc;
    }
    LOG(3, "Automatic Calibration - %s value %d => result = %d min = %d", BT848_GetSetting(type_setting)->szDisplayName, current_value, diff, min_diff);
    return min_found;
}

int CCalSetting::GetResult(int *mask, int *min_val, int *max_val)
{
    int i;
    int nb_min;
    int best_val_min;
    int best_val_max;

    for (i=0,nb_min=0 ; i<=(max - min) ; i++)
    {
        if (mask_output[i/32] & (1 << (i%32)))
        {
            nb_min++;
            if (nb_min == 1)
            {
                best_val_min = i + min;
                best_val_max = i + min;
            }
            else
            {
                best_val_max = i + min;
            }
        }
    }

    if (nb_min > 0)
    {
        for (i=0 ; i<16; i++)
        {
            mask[i] = mask_output[i];
        }
        *min_val = best_val_min;
        *max_val = best_val_max;
    }

    return nb_min;
}

void CCalSetting::Adjust(int value)
{
    current_value = value;
    Setting_SetValue(BT848_GetSetting(type_setting), current_value);
}

/////////////////////////////////////////////////////////////////////////////
// Class CCalibration

CCalibration::CCalibration()
{
    nb_test_patterns = 0;
    for (int i = 0 ; i < MAX_TEST_PATTERNS ; i++)
    {
        test_patterns[i] = NULL;
    }
    current_test_pattern = NULL;
    current_sub_pattern = NULL;
    type_calibration = CAL_MANUAL;
    running = FALSE;
    brightness   = new CCalSetting(BRIGHTNESS);
    contrast     = new CCalSetting(CONTRAST);
    saturation_U = new CCalSetting(SATURATIONU);
    saturation_V = new CCalSetting(SATURATIONV);
    hue          = new CCalSetting(HUE);
    last_tick_count = -1;
    LoadTestPatterns();
}

CCalibration::~CCalibration()
{
    UnloadTestPatterns();

    delete brightness;
    delete contrast;
    delete saturation_U;
    delete saturation_V;
    delete hue;
}

// This method loads all the predefined test patterns
void CCalibration::LoadTestPatterns()
{
    CColorBar *color_bar;
    CSubPattern *sub_pattern;

    test_patterns[nb_test_patterns] = new CTestPattern("THX Optimode - Monitor Performance", FORMAT_NTSC);

    sub_pattern = new CSubPattern(ADJ_BRIGHTNESS_CONTRAST);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar( 347,  764, 6875, 7500, FALSE,   0,   0,   0);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(1181, 1597, 6875, 7500, FALSE,  22,  24,  23);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(2014, 2431, 6875, 7500, FALSE,  48,  50,  49);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(7083, 7500, 6875, 7500, FALSE, 201, 203, 202);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(7917, 8333, 6875, 7500, FALSE, 227, 228, 227);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(8750, 9167, 6875, 7500, FALSE, 252, 253, 252);
    sub_pattern->AddColorBar(color_bar);

    sub_pattern = new CSubPattern(ADJ_BRIGHTNESS);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar(1181, 1597, 6875, 7500, FALSE,  22,  24,  23);
    sub_pattern->AddColorBar(color_bar);

    sub_pattern = new CSubPattern(ADJ_CONTRAST);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar(8750, 9167, 6875, 7500, FALSE, 252, 253, 252);
    sub_pattern->AddColorBar(color_bar);

    sub_pattern = new CSubPattern(ADJ_COLOR);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar( 278, 1042, 2396, 4167, FALSE, 251, 252, 251);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(1458, 2222, 2396, 4167, FALSE, 188, 190,   0);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(2708, 3472, 2396, 4167, FALSE,   0, 188, 185);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(3889, 4653, 2396, 4167, FALSE,   0, 188,   0);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(5139, 5903, 2396, 4167, FALSE, 187,   0, 187);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(6319, 7083, 2396, 4167, FALSE, 186,   0,   0);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(7569, 8333, 2396, 4167, FALSE,   0,   0, 187);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(8750, 9514, 2396, 4167, FALSE,   0,   0,   0);
    sub_pattern->AddColorBar(color_bar);
    
    sub_pattern = new CSubPattern(ADJ_SATURATION_U);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar(7569, 8333, 2396, 4167, FALSE,   0,   0, 187);
    sub_pattern->AddColorBar(color_bar);

    sub_pattern = new CSubPattern(ADJ_SATURATION_V);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar(6319, 7083, 2396, 4167, FALSE, 186,   0,   0);
    sub_pattern->AddColorBar(color_bar);

    sub_pattern = new CSubPattern(ADJ_HUE);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar(2708, 3472, 2396, 4167, FALSE,   0, 188, 185);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(5139, 5903, 2396, 4167, FALSE, 187,   0, 187);
    sub_pattern->AddColorBar(color_bar);

    test_patterns[nb_test_patterns]->CreateGlobalSubPattern();

    nb_test_patterns++;

    ///////////////

    test_patterns[nb_test_patterns] = new CTestPattern("THX Optimode - tint color", FORMAT_NTSC);

    sub_pattern = new CSubPattern(ADJ_COLOR);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar( 417, 1111, 2500, 5625, FALSE, 188, 189, 188);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(1806, 2500, 2500, 5625, FALSE, 189, 190,   0);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(3194, 3889, 2500, 5625, FALSE,   0, 190, 188);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(4583, 5278, 2500, 5625, FALSE,   0, 189,   0);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(6042, 6736, 2500, 5625, FALSE, 188,   0, 190);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(7431, 8125, 2500, 5625, FALSE, 189,   0,   0);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(8889, 9583, 2500, 5625, FALSE,   0,   0, 189);
    sub_pattern->AddColorBar(color_bar);
    
    sub_pattern = new CSubPattern(ADJ_SATURATION_U);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar(8889, 9583, 2500, 5625, FALSE,   0,   0, 189);
    sub_pattern->AddColorBar(color_bar);

    sub_pattern = new CSubPattern(ADJ_SATURATION_V);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar(7431, 8125, 2500, 5625, FALSE, 189,   0,   0);
    sub_pattern->AddColorBar(color_bar);

    sub_pattern = new CSubPattern(ADJ_HUE);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar(3194, 3889, 2500, 5625, FALSE,   0, 190, 188);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(6042, 6736, 2500, 5625, FALSE, 188,   0, 190);
    sub_pattern->AddColorBar(color_bar);

    test_patterns[nb_test_patterns]->CreateGlobalSubPattern();

    nb_test_patterns++;

    ///////////////

    test_patterns[nb_test_patterns] = new CTestPattern("VE - T 18 C 2 - range of gray", FORMAT_NTSC);
    
    sub_pattern = new CSubPattern(ADJ_BRIGHTNESS_CONTRAST);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar( 417,  972, 2500, 7500, FALSE,   0,   0,   0);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(1458, 2014, 2500, 7500, FALSE,  30,  30,  30);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(2361, 2917, 2500, 7500, FALSE,  60,  60,  60);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(3333, 3889, 2500, 7500, FALSE,  90,  90,  90);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(4236, 4792, 2500, 7500, FALSE, 117, 119, 119);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(5208, 5764, 2500, 7500, FALSE, 150, 150, 150);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(6181, 6736, 2500, 7500, FALSE, 177, 179, 179);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(7153, 7708, 2500, 7500, FALSE, 206, 209, 205);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(8125, 8681, 2500, 7500, FALSE, 236, 236, 236);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(9097, 9653, 2500, 7500, FALSE, 254, 254, 254);
    sub_pattern->AddColorBar(color_bar);
    
    sub_pattern = new CSubPattern(ADJ_BRIGHTNESS);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar(1458, 2014, 2500, 7500, FALSE,  30,  30,  30);
    sub_pattern->AddColorBar(color_bar);

    sub_pattern = new CSubPattern(ADJ_CONTRAST);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar(9097, 9653, 2500, 7500, FALSE, 254, 254, 254);
    sub_pattern->AddColorBar(color_bar);

    test_patterns[nb_test_patterns]->CreateGlobalSubPattern();
    
    nb_test_patterns++;

    ///////////////

    test_patterns[nb_test_patterns] = new CTestPattern("VE - T 18 C 10 - color bars", FORMAT_NTSC);
    
    sub_pattern = new CSubPattern(ADJ_COLOR);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar( 417, 1111, 2500, 5208, FALSE, 190, 190, 190);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(1806, 2500, 2500, 5208, FALSE, 198, 199,   0);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(3194, 3889, 2500, 5208, FALSE,   0, 190, 189);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(4583, 5278, 2500, 5208, FALSE,   0, 190,   0);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(6042, 6736, 2500, 5208, FALSE, 190,   0, 189);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(7431, 8125, 2500, 5208, FALSE, 188,   0,   0);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(8889, 9583, 2500, 5208, FALSE,   0,   0, 187);
    sub_pattern->AddColorBar(color_bar);
    
    sub_pattern = new CSubPattern(ADJ_SATURATION_U);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar(8889, 9583, 2500, 5208, FALSE,   0,   0, 187);
    sub_pattern->AddColorBar(color_bar);

    sub_pattern = new CSubPattern(ADJ_SATURATION_V);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar(7431, 8125, 2500, 5208, FALSE, 188,   0,   0);
    sub_pattern->AddColorBar(color_bar);

    sub_pattern = new CSubPattern(ADJ_HUE);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar(3194, 3889, 2500, 5208, FALSE,   0, 190, 189);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(6042, 6736, 2500, 5208, FALSE, 190,   0, 189);
    sub_pattern->AddColorBar(color_bar);

    test_patterns[nb_test_patterns]->CreateGlobalSubPattern();
    
    nb_test_patterns++;

    ///////////////

    test_patterns[nb_test_patterns] = new CTestPattern("AVIA - T 1 C 7 - range of gray", FORMAT_NTSC);
    
    sub_pattern = new CSubPattern(ADJ_BRIGHTNESS_CONTRAST);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar( 208,  764, 2500, 7500, FALSE, 254, 251, 255);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(1111, 1667, 2500, 7500, FALSE, 226, 223, 227);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(2014, 2569, 2500, 7500, FALSE, 200, 197, 201);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(2917, 3472, 2500, 7500, FALSE, 172, 169, 173);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(3819, 4375, 2500, 7500, FALSE, 145, 142, 146);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(4722, 5278, 2500, 7500, FALSE, 117, 114, 118);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(5625, 6181, 2500, 7500, FALSE,  90,  87,  91);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(6528, 7083, 2500, 7500, FALSE,  62,  59,  63);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(7431, 7986, 2500, 7500, FALSE,  34,  31,  35);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(8333, 8889, 2500, 7500, FALSE,   7,   4,   8);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(9236, 9792, 2500, 7500, FALSE,   1,   0,   2);
    sub_pattern->AddColorBar(color_bar);
    
    sub_pattern = new CSubPattern(ADJ_BRIGHTNESS);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar(8333, 8889, 2500, 7500, FALSE,   7,   4,   8);
    sub_pattern->AddColorBar(color_bar);

    sub_pattern = new CSubPattern(ADJ_CONTRAST);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar( 208,  764, 2500, 7500, FALSE, 254, 251, 255);
    sub_pattern->AddColorBar(color_bar);

    test_patterns[nb_test_patterns]->CreateGlobalSubPattern();
    
    nb_test_patterns++;

    ///////////////

    test_patterns[nb_test_patterns] = new CTestPattern("AVIA - T 4 C 4 - color bars", FORMAT_NTSC);
    
    sub_pattern = new CSubPattern(ADJ_COLOR);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar( 417, 1111, 2083, 4167, FALSE, 190, 190, 190);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(1806, 2500, 2083, 4167, FALSE, 190, 189,   0);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(3194, 3889, 2083, 4167, FALSE,   0, 189, 188);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(4583, 5278, 2083, 4167, FALSE,   0, 189,   0);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(6042, 6736, 2083, 4167, FALSE, 190,   0, 191);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(7431, 8125, 2083, 4167, FALSE, 190,   0,   0);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(8889, 9583, 2083, 4167, FALSE,   0,   0, 190);
    sub_pattern->AddColorBar(color_bar);
    
    sub_pattern = new CSubPattern(ADJ_SATURATION_U);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar(8889, 9583, 2083, 4167, FALSE,   0,   0, 190);
    sub_pattern->AddColorBar(color_bar);

    sub_pattern = new CSubPattern(ADJ_SATURATION_V);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar(7431, 8125, 2083, 4167, FALSE, 190,   0,   0);
    sub_pattern->AddColorBar(color_bar);

    sub_pattern = new CSubPattern(ADJ_HUE);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar(3194, 3889, 2083, 4167, FALSE,   0, 189, 188);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(6042, 6736, 2083, 4167, FALSE, 190,   0, 191);
    sub_pattern->AddColorBar(color_bar);

    test_patterns[nb_test_patterns]->CreateGlobalSubPattern();
    
    nb_test_patterns++;

    ///////////////

    test_patterns[nb_test_patterns] = new CTestPattern("THX Optimode - Screen Performance", FORMAT_PAL_BDGHI);

    sub_pattern = new CSubPattern(ADJ_BRIGHTNESS_CONTRAST);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar( 347,  764, 3472, 4167, FALSE,   0,   0,   1);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(1181, 1597, 3472, 4167, FALSE,  23,  24,  24);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(2014, 2431, 3472, 4167, FALSE,  49,  50,  50);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(7083, 7500, 3472, 4167, FALSE, 202, 203, 203);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(7917, 8333, 3472, 4167, FALSE, 228, 228, 229);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(8750, 9167, 3472, 4167, FALSE, 253, 253, 253);
    sub_pattern->AddColorBar(color_bar);

    sub_pattern = new CSubPattern(ADJ_BRIGHTNESS);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar(1181, 1597, 3472, 4167, FALSE,  23,  24,  24);
    sub_pattern->AddColorBar(color_bar);

    sub_pattern = new CSubPattern(ADJ_CONTRAST);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar(8750, 9167, 3472, 4167, FALSE, 253, 253, 253);
    sub_pattern->AddColorBar(color_bar);

    sub_pattern = new CSubPattern(ADJ_COLOR);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar( 278, 1042, 2344, 3038, FALSE, 251, 253, 251);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(1458, 2222, 2344, 3038, FALSE, 189, 190,   0);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(2708, 3472, 2344, 3038, FALSE,   0, 190, 187);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(3889, 4653, 2344, 3038, FALSE,   0, 189,   0);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(5139, 5903, 2344, 3038, FALSE, 189,   0, 190);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(6319, 7083, 2344, 3038, FALSE, 188,   0,   0);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(7569, 8333, 2344, 3038, FALSE,  0,    0, 188);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(8750, 9514, 2344, 3038, FALSE,  0,    0,   0);
    sub_pattern->AddColorBar(color_bar);
    
    sub_pattern = new CSubPattern(ADJ_SATURATION_U);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar(7569, 8333, 2344, 3038, FALSE,  0,    0, 188);
    sub_pattern->AddColorBar(color_bar);

    sub_pattern = new CSubPattern(ADJ_SATURATION_V);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar(6319, 7083, 2344, 3038, FALSE, 188,   0,   0);
    sub_pattern->AddColorBar(color_bar);

    sub_pattern = new CSubPattern(ADJ_HUE);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar(2708, 3472, 2344, 3038, FALSE,   0, 190, 187);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(5139, 5903, 2344, 3038, FALSE, 189,   0, 190);
    sub_pattern->AddColorBar(color_bar);

    test_patterns[nb_test_patterns]->CreateGlobalSubPattern();

    nb_test_patterns++;

    ///////////////

    test_patterns[nb_test_patterns] = new CTestPattern("Collector LAL 2000 - T 36 C 1 - range of gray", FORMAT_PAL_BDGHI);
    
    sub_pattern = new CSubPattern(ADJ_BRIGHTNESS_CONTRAST);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar( 278,  833, 2500, 7500, FALSE,   0,   0,   0);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(1181, 1736, 2500, 7500, FALSE,  20,  23,  20);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(2014, 2569, 2500, 7500, FALSE,  46,  49,  45);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(2917, 3472, 2500, 7500, FALSE,  72,  75,  71);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(3819, 4375, 2500, 7500, FALSE,  98, 101,  97);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(4722, 5278, 2500, 7500, FALSE, 123, 125, 122);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(5556, 6111, 2500, 7500, FALSE, 149, 151, 148);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(6458, 7014, 2500, 7500, FALSE, 174, 177, 173);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(7361, 7917, 2500, 7500, FALSE, 201, 203, 200);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(8264, 8819, 2500, 7500, FALSE, 227, 229, 226);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(9167, 9722, 2500, 7500, FALSE, 253, 252, 252);
    sub_pattern->AddColorBar(color_bar);
    
    sub_pattern = new CSubPattern(ADJ_BRIGHTNESS);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar(1181, 1736, 2500, 7500, FALSE,  20,  23,  20);
    sub_pattern->AddColorBar(color_bar);

    sub_pattern = new CSubPattern(ADJ_CONTRAST);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar(9167, 9722, 2500, 7500, FALSE, 253, 252, 252);
    sub_pattern->AddColorBar(color_bar);

    test_patterns[nb_test_patterns]->CreateGlobalSubPattern();
    
    nb_test_patterns++;

    ///////////////

    test_patterns[nb_test_patterns] = new CTestPattern("Collector LAL 2000 - T 31 C 1 - color bars", FORMAT_PAL_BDGHI);
    
    sub_pattern = new CSubPattern(ADJ_COLOR);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar( 556, 1111, 2500, 7500, FALSE, 253, 253, 253);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(1736, 2292, 2500, 7500, FALSE, 189, 191,   0);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(2917, 3472, 2500, 7500, FALSE,   0, 189, 188);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(4097, 4653, 2500, 7500, FALSE,   0, 190,   0);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(5278, 5833, 2500, 7500, FALSE, 189,   0, 189);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(6458, 7014, 2500, 7500, FALSE, 189,   0,   0);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(7708, 8264, 2500, 7500, FALSE,   0,   0, 189);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(8889, 9444, 2500, 7500, FALSE,   0,   0,   0);
    sub_pattern->AddColorBar(color_bar);
    
    sub_pattern = new CSubPattern(ADJ_SATURATION_U);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar(7708, 8264, 2500, 7500, FALSE,   0,   0, 189);
    sub_pattern->AddColorBar(color_bar);

    sub_pattern = new CSubPattern(ADJ_SATURATION_V);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar(6458, 7014, 2500, 7500, FALSE, 189,   0,   0);
    sub_pattern->AddColorBar(color_bar);

    sub_pattern = new CSubPattern(ADJ_HUE);
    test_patterns[nb_test_patterns]->AddSubPattern(sub_pattern);
    color_bar = new CColorBar(2917, 3472, 2500, 7500, FALSE,   0, 189, 188);
    sub_pattern->AddColorBar(color_bar);
    color_bar = new CColorBar(5278, 5833, 2500, 7500, FALSE, 189,   0, 189);
    sub_pattern->AddColorBar(color_bar);

    test_patterns[nb_test_patterns]->CreateGlobalSubPattern();

    nb_test_patterns++;
}

// This method unloads all the predefined test patterns
void CCalibration::UnloadTestPatterns()
{
    // Destroy all test patterns
    for (int i = 0 ; i < MAX_TEST_PATTERNS ; i++)
    {
        if (test_patterns[i] != NULL)
        {
            delete test_patterns[i];
            test_patterns[i] = NULL;
        }
    }
    nb_test_patterns = 0;
    current_test_pattern = NULL;
    current_sub_pattern = NULL;
}

void CCalibration::UpdateMenu(HMENU hMenu)
{
    HMENU           hMenuPatterns;
    MENUITEMINFO    MenuItemInfo;
    int             i;
	char			*name;

    hMenuPatterns = GetPatternsSubmenu();
    if (hMenuPatterns == NULL) return;

    i = GetMenuItemCount(hMenuPatterns);
    while (i)
    {
        i--;
        RemoveMenu(hMenuPatterns, i, MF_BYPOSITION);
    }

	for (i=0; i < MAX_TEST_PATTERNS ; i++)
	{
		if (test_patterns[i] != NULL)
        {
			name = test_patterns[i]->GetName();
			if (strlen (name) > 0)
			{
	            MenuItemInfo.cbSize = sizeof (MenuItemInfo);
		        MenuItemInfo.fType = MFT_STRING;
			    MenuItemInfo.dwTypeData = name;
				MenuItemInfo.cch = strlen (name);

	            MenuItemInfo.fMask = MIIM_TYPE | MIIM_ID;
		        MenuItemInfo.wID = IDM_PATTERN_SELECT + i + 1;
			    InsertMenuItem(hMenuPatterns, i, TRUE, &MenuItemInfo);
			}
        }
    }
}

void CCalibration::SetMenu(HMENU hMenu)
{
    HMENU   hMenuPatterns;
    int     i;
	char	*name;
    eTypeContentPattern type_content;

    if ((current_test_pattern != NULL) && (current_test_pattern->GetVideoFormat() != Setting_GetValue(BT848_GetSetting(TVFORMAT))))
    {
        current_test_pattern = NULL;
    }

    if (current_test_pattern != NULL)
    {
        type_content = current_test_pattern->DetermineTypeContent();
    }
    else
    {
        type_content = PAT_UNKNOWN;
    }

    hMenuPatterns = GetPatternsSubmenu();
    if (hMenuPatterns == NULL) return;

    for (i=0; i < MAX_TEST_PATTERNS ; i++)
    {
		if (test_patterns[i] != NULL)
        {
			name = test_patterns[i]->GetName();
			if (strlen (name) > 0)
			{
				EnableMenuItem(hMenuPatterns, i, running ? MF_BYPOSITION | MF_GRAYED : MF_BYPOSITION | MF_ENABLED);
				EnableMenuItem(hMenuPatterns, i, (running || (test_patterns[i]->GetVideoFormat() != Setting_GetValue(BT848_GetSetting(TVFORMAT)))) ? MF_BYPOSITION | MF_GRAYED : MF_BYPOSITION | MF_ENABLED);
				CheckMenuItem(hMenuPatterns, i, (current_test_pattern == test_patterns[i]) ? MF_BYPOSITION | MF_CHECKED : MF_BYPOSITION | MF_UNCHECKED);
			}
        }
    }
	
	EnableMenuItem(hMenu, IDM_START_MANUAL_CALIBRATION, (running || (current_test_pattern == NULL)) ? MF_GRAYED : MF_ENABLED);
	EnableMenuItem(hMenu, IDM_START_AUTO_CALIBRATION, (running || (current_test_pattern == NULL) || (type_content != PAT_GRAY_AND_COLOR)) ? MF_GRAYED : MF_ENABLED);
	EnableMenuItem(hMenu, IDM_START_AUTO_CALIBRATION2, (running || (current_test_pattern == NULL) || ((type_content != PAT_GRAY_AND_COLOR) && (type_content != PAT_RANGE_OF_GRAY))) ? MF_GRAYED : MF_ENABLED);
	EnableMenuItem(hMenu, IDM_START_AUTO_CALIBRATION3, (running || (current_test_pattern == NULL) || ((type_content != PAT_GRAY_AND_COLOR) && (type_content != PAT_COLOR))) ? MF_GRAYED : MF_ENABLED);
	EnableMenuItem(hMenu, IDM_STOP_CALIBRATION, (!running || (current_test_pattern == NULL)) ? MF_GRAYED : MF_ENABLED);
}

void CCalibration::SelectTestPattern(int num)
{
	if (test_patterns[num] != NULL)
	{
		current_test_pattern = test_patterns[num];
	}
	else
	{
		current_test_pattern = NULL;
	}
    current_sub_pattern = NULL;
}

CTestPattern *CCalibration::GetCurrentTestPattern()
{
	return current_test_pattern;
}

CSubPattern *CCalibration::GetSubPattern(eTypeAdjust type_adjust)
{
    CSubPattern *sub_pattern = NULL;

    if (current_test_pattern != NULL)
    {
        sub_pattern = current_test_pattern->GetSubPattern(type_adjust);
    }

    return sub_pattern;
}

CSubPattern *CCalibration::GetCurrentSubPattern()
{
	return current_sub_pattern;
}

void CCalibration::Start(eTypeCalibration type)
{
    if (current_test_pattern == NULL)
        return;

	type_calibration = type;

    brightness->Update();
    contrast->Update();
    saturation_U->Update();
    saturation_V->Update();
    hue->Update();
    brightness->Save();
    contrast->Save();
    saturation_U->Save();
    saturation_V->Save();
    hue->Save();

    switch (type_calibration)
    {
    case CAL_AUTO_BRIGHT_CONTRAST:
        initial_step = 1;
        nb_steps = 6;
        brightness->AdjustDefault();
        contrast->AdjustDefault();
        break;
    case CAL_AUTO_COLOR:
        initial_step = 7;
        nb_steps = 8;
        saturation_U->AdjustDefault();
        saturation_V->AdjustDefault();
        hue->AdjustDefault();
        break;
    case CAL_AUTO_FULL:
        initial_step = 1;
        nb_steps = 14;
        brightness->AdjustDefault();
        contrast->AdjustDefault();
        saturation_U->AdjustDefault();
        saturation_V->AdjustDefault();
        hue->AdjustDefault();
        break;
    case CAL_MANUAL:
    default:
        initial_step = 0;
        nb_steps = 1;
        break;
    }
    current_step = initial_step;
    full_range = FALSE;
    nb_tries = 0;
    first_calc = TRUE;

    // Display the specific OSD screen
    OSD_ShowInfosScreen(hWnd, 4, 0);

    running = TRUE;
}

void CCalibration::Stop()
{
    if (type_calibration != CAL_MANUAL)
    {
        if ( (current_step != -1)
          || (MessageBox(hWnd, "Do you want to keep the current settings ?", "DScaler Question", MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1) == IDNO) )
        {
            brightness->Restore();
            contrast->Restore();
            saturation_U->Restore();
            saturation_V->Restore();
            hue->Restore();
        }
    }

    // Erase the OSD screen
    OSD_Clear(hWnd);

	running = FALSE;
}

BOOL CCalibration::IsRunning()
{
	return running;
}

BOOL CCalibration::GetCurrentStep()
{
	return current_step;
}

eTypeCalibration CCalibration::GetType()
{
	return type_calibration;
}

void CCalibration::Make(short **Lines, int height, int width, int tick_count)
{
    int nb;
    int min, max;
    int mask[16];
    BOOL new_settings;
    BOOL found;

	if (!running
	 || (current_test_pattern == NULL)
	 || ((last_tick_count != -1) && ((tick_count - last_tick_count) < MIN_TIME_BETWEEN_CALC)))
		return;

	last_tick_count = tick_count;

    switch (current_step)
    {
    case -1:    // Automatic calibration finished 
        Stop();
        break;

    case 0:     // Manual calibration
        current_sub_pattern = GetSubPattern(ADJ_MANUAL);
        if (current_sub_pattern == NULL)
        {
            break;
        }

        new_settings = FALSE;
        new_settings |= brightness->Update();
        new_settings |= contrast->Update();
        new_settings |= saturation_U->Update();
        new_settings |= saturation_V->Update();
        new_settings |= hue->Update();

        // Calculations with current setitngs
        if ( current_sub_pattern->CalcCurrentSubPattern(first_calc || new_settings, NB_CALCULATIONS, Lines, height, width)
          && (type_calibration != CAL_MANUAL) )
        {
            current_step = -1;
            last_tick_count = tick_count + 500;
        }
        first_calc = FALSE;
        break;

    case 1:
        if (full_range)
        {
            brightness->SetFullRange();
        }
        else
        {
            brightness->SetRange((nb_tries == 0) ? 75 : 25);
        }
        if (step_init(ADJ_BRIGHTNESS, brightness, (CCalSetting *)NULL, (CCalSetting *)NULL))
        {
            current_step++;
        }
        else
        {
            // We stop calibration
            current_step = initial_step + nb_steps;
        }
        break;

    case 2:     // Step to find a short range for brightness setting
        if (step_process(Lines, height, width, 4, TRUE, FALSE, &found))
        {
            if (found || full_range)
            {
                full_range = FALSE;
                current_step++;
            }
            else
            {
                full_range = TRUE;
                current_step--;
            }
        }
        break;

    case 3:
        if (full_range)
        {
            contrast->SetFullRange();
        }
        else
        {
            contrast->SetRange((nb_tries == 0) ? 50 : 25);
        }
        if (step_init(ADJ_CONTRAST, contrast, (CCalSetting *)NULL, (CCalSetting *)NULL))
        {
            current_step++;
        }
        else
        {
            // We stop calibration
            current_step = initial_step + nb_steps;
        }
        break;

    case 4:     // Step to find a short range for contrast setting
        if (step_process(Lines, height, width, 4, TRUE, FALSE, &found))
        {
            if (found || full_range)
            {
                full_range = FALSE;
                nb_tries++;
                if (nb_tries < 2)
                {
                    current_step -= 3;
                }
                else
                {
                    nb_tries = 0;
                    current_step++;
                }
            }
            else
            {
                full_range = TRUE;
                current_step--;
            }
        }
        break;

    case 5:
        nb = brightness->GetResult(mask, &min, &max);
        if (nb == 0)
        {
            current_step += 2;
            break;
        }
        nb = max - min;
        if (nb <= 6)
        {
            min -= (6 - nb) / 2;
            max += (6 - nb) / 2;
            brightness->SetRange(min, max);
        }
        nb = contrast->GetResult(mask, &min, &max);
        if (nb == 0)
        {
            current_step += 2;
            break;
        }
        nb = max - min;
        if (nb <= 6)
        {
            min -= (6 - nb) / 2;
            max += (6 - nb) / 2;
            contrast->SetRange(min, max);
        }
        if (step_init(ADJ_BRIGHTNESS_CONTRAST, brightness, contrast, (CCalSetting *)NULL))
        {
            current_step++;
        }
        else
        {
            current_step += 2;
        }
        break;

    case 6:     // Step to adjust fine brightness + contradt
        if (step_process(Lines, height, width, 4, FALSE, TRUE, &found))
        {
            current_step++;
        }
        break;

    case 7:
        if (full_range)
        {
            saturation_U->SetFullRange();
        }
        else
        {
            saturation_U->SetRange(75);
        }
        if (step_init(ADJ_SATURATION_U, saturation_U, (CCalSetting *)NULL, (CCalSetting *)NULL))
        {
            current_step++;
        }
        else
        {
            // We stop calibration
            current_step = initial_step + nb_steps;
        }
        break;

    case 8:     // Step to find a short range for saturation U setting
        if (step_process(Lines, height, width, 2, TRUE, FALSE, &found))
        {
            if (found || full_range)
            {
                full_range = FALSE;
                current_step++;
            }
            else
            {
                full_range = TRUE;
                current_step--;
            }
        }
        break;

    case 9:
        if (full_range)
        {
            saturation_V->SetFullRange();
        }
        else
        {
            saturation_V->SetRange(75);
        }
        if (step_init(ADJ_SATURATION_V, saturation_V, (CCalSetting *)NULL, (CCalSetting *)NULL))
        {
            current_step++;
        }
        else
        {
            // We stop calibration
            current_step = initial_step + nb_steps;
        }
        break;

    case 10:    // Step to find a short range for saturation V setting
        if (step_process(Lines, height, width, 3, TRUE, FALSE, &found))
        {
            if (found || full_range)
            {
                full_range = FALSE;
                current_step += 3;
            }
            else
            {
                full_range = TRUE;
                current_step--;
            }
        }
        break;

    case 11:
        hue->SetRange(20);
        if (step_init(ADJ_HUE, hue, (CCalSetting *)NULL, (CCalSetting *)NULL))
        {
            current_step++;
        }
        else
        {
            // We stop calibration
            current_step = initial_step + nb_steps;
        }
        break;

    case 12:    // Step to find a short range for hue setting
        if (step_process(Lines, height, width, 2, TRUE, FALSE, &found))
        {
            full_range = FALSE;
            nb_tries++;
            if (nb_tries < 2)
            {
                current_step -= 5;
            }
            else
            {
                nb_tries = 0;
                current_step++;
            }
        }
        break;

    case 13:
        nb = saturation_U->GetResult(mask, &min, &max);
        if (nb == 0)
        {
            current_step += 2;
            break;
        }
        nb = max - min;
        if (nb <= 8)
        {
            min -= (8 - nb) / 2;
            max += (8 - nb) / 2;
            saturation_U->SetRange(min, max);
        }
        nb = saturation_V->GetResult(mask, &min, &max);
        if (nb == 0)
        {
            current_step += 2;
            break;
        }
        nb = max - min;
        if (nb <= 8)
        {
            min -= (8 - nb) / 2;
            max += (8 - nb) / 2;
            saturation_V->SetRange(min, max);
        }
        if (step_init(ADJ_COLOR, saturation_U, saturation_V, (CCalSetting *)NULL))
        {
            current_step++;
        }
        else
        {
            current_step += 2;
        }
        break;

    case 14:    // Step to adjust fine color saturation
        if (step_process(Lines, height, width, 4, FALSE, TRUE, &found))
        {
            current_step++;
        }
        break;

    default:
        break;
    }

    // Test to check if all steps are already done
    if ((current_step > 0) && ((current_step - initial_step) >= nb_steps))
    {
        current_step = 0;
        first_calc = TRUE;
    }
}

BOOL CCalibration::step_init(eTypeAdjust type_adjust, CCalSetting *_setting1, CCalSetting *_setting2, CCalSetting *_setting3)
{
    // Get the bar to use for this step
    current_sub_pattern = GetSubPattern(type_adjust);
    if (current_sub_pattern == NULL)
    {
        setting1 = (CCalSetting *)NULL;
        setting2 = (CCalSetting *)NULL;
        setting3 = (CCalSetting *)NULL;
        return FALSE;
    }
    else
    {
        // Initialize
        setting1 = _setting1;
        if (setting1 != (CCalSetting *)NULL)
        {
            // Set the settings to their minimum
            setting1->AdjustMin();
            setting1->InitResult();
        }
        setting2 = _setting2;
        if (setting2 != (CCalSetting *)NULL)
        {
            // Set the settings to their minimum
            setting2->AdjustMin();
            setting2->InitResult();
        }
        setting3 = _setting3;
        if (setting3 != (CCalSetting *)NULL)
        {
            // Set the settings to their minimum
            setting3->AdjustMin();
            setting3->InitResult();
        }

//        first_calc = TRUE;
        nb_calcul = 0;
        total_dif = 0;

        return TRUE;
    }
}

BOOL CCalibration::step_process(short **Lines, int height, int width, unsigned int sig_component, BOOL stop_when_found, BOOL only_one, BOOL *best_found)
{
    int val[4];
    BOOL YUV;
    int idx;
//    int dif;

    // Calculations with current settings
    current_sub_pattern->CalcCurrentSubPattern(TRUE, 1, Lines, height, width);

    // See how good is the red result
    if ((sig_component >= 1) && (sig_component <= 4))
    {
        YUV = TRUE;
        idx = sig_component - 1;
    }
    else if ((sig_component >= 5) && (sig_component <= 8))
    {
        YUV = FALSE;
        idx = sig_component - 5;
    }
    current_sub_pattern->GetSumDeltaColor(YUV, &val[0], &val[1], &val[2], &val[3]);
//    dif = val[idx];
    total_dif += val[idx];
    nb_calcul++;

    // Waiting at least 5 calculations
    if (nb_calcul < NB_CALCULATIONS)
    {
        last_tick_count = -1;
        return FALSE;
    }

    if (setting1 != (CCalSetting *)NULL)
    {
        *best_found = setting1->UpdateResult(total_dif, stop_when_found ? DELTA_STOP*NB_CALCULATIONS : -1, only_one);
//        *best_found = setting1->UpdateResult(dif, stop_when_found ? DELTA_STOP : -1, only_one);
    }
    if (setting2 != (CCalSetting *)NULL)
    {
        *best_found = setting2->UpdateResult(total_dif, stop_when_found ? DELTA_STOP*NB_CALCULATIONS : -1, only_one);
//        *best_found = setting2->UpdateResult(dif, stop_when_found ? DELTA_STOP : -1, only_one);
    }
    if (setting3 != (CCalSetting *)NULL)
    {
        *best_found = setting3->UpdateResult(total_dif, stop_when_found ? DELTA_STOP*NB_CALCULATIONS : -1, only_one);
//        *best_found = setting3->UpdateResult(dif, stop_when_found ? DELTA_STOP : -1, only_one);
    }

    nb_calcul = 0;
    total_dif = 0;

    // Increase the third setting
    if ((setting3 != (CCalSetting *)NULL) && setting3->AdjustNext())
    {
        return FALSE;
    }
    // Increase the second setting
    else if ((setting2 != (CCalSetting *)NULL) && setting2->AdjustNext())
    {
        if (setting3 != (CCalSetting *)NULL)
        {
            // Set the third setting to its minimum
            setting3->AdjustMin();
        }
        return FALSE;
    }
    // Increase the first setting
    else if ((setting1 != (CCalSetting *)NULL) && setting1->AdjustNext())
    {
        if (setting2 != (CCalSetting *)NULL)
        {
            // Set the second setting to its minimum
            setting2->AdjustMin();
        }
        if (setting3 != (CCalSetting *)NULL)
        {
            // Set the third setting to its minimum
            setting3->AdjustMin();
        }
        return FALSE;
    }
    else
    {
        // Set the settings to the best values found
        if (setting1 != (CCalSetting *)NULL)
        {
            setting1->AdjustBest();
        }
        if (setting2 != (CCalSetting *)NULL)
        {
            setting2->AdjustBest();
        }
        if (setting3 != (CCalSetting *)NULL)
        {
            setting3->AdjustBest();
        }
        return TRUE;
    }
}

/////////////////////////////////////////////////////////////////////////////

CCalibration *pCalibration = NULL;

