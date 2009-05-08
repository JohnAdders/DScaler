/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file Calibration.cpp Calibration Classes
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "Calibration.h"
#include "DScaler.h"
#include "Setting.h"
#include "OSD.h"
#include "Providers.h"
#include "DebugLog.h"
#include "AspectRatio.h"
#include "OutThreads.h"
#include "PathHelpers.h"
#include "SettingsMaster.h"

using namespace std;

// Minimum time in milliseconds between two consecutive evaluations
#define    MIN_TIME_BETWEEN_CALC    35
// Number of calculations to do on successive frames before to decide what to adjust
#define NB_CALCULATIONS_LOW     10
#define NB_CALCULATIONS_HIGH    50
// Delta used to stop the search process when the current value for setting implies
// a result that is superior to this delta when compared to the previous found best result
#define DELTA_STOP              5
// Maximum value
#define MAX_VALUE               1000000000


static BOOL ShowRGBDelta = TRUE;
static BOOL ShowYUVDelta = TRUE;


/////////////////////////////////////////////////////////////////////////////
// Class CCalSetting

CCalSetting::CCalSetting(CSliderSetting* pSetting)
{
    m_pSetting = pSetting;
    min = pSetting->GetMin();
    max = pSetting->GetMax();
    mask_nb = (max-min+1) / 32;
    if ((max-min+1) % 32)
    {
        mask_nb++;
    }
    mask_input = (unsigned int*) malloc(mask_nb * sizeof(unsigned int)),
    mask_output = (unsigned int*) malloc(mask_nb * sizeof(unsigned int)),
    current_value = pSetting->GetValue();
    SetFullRange();
    InitResult();
}

CCalSetting::~CCalSetting()
{
    free(mask_input);
    free(mask_output);
}

BOOL CCalSetting::Update()
{
    int new_value = m_pSetting->GetValue();
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
    LOG(2, _T("Automatic Calibration - saved value = %d"), saved_value);
}

void CCalSetting::Restore()
{
    Adjust(saved_value);
    LOG(2, _T("Automatic Calibration - restored value = %d"), saved_value);
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
    for (i=0 ; i<mask_nb ; i++)
    {
        mask_input[i] = 0;
    }
    for (i=min_val ; i<=max_val ; i++)
    {
        j = i - min;
        mask_input[j/32] |= (1 << (j%32));
    }
    LOG(3, _T("Automatic Calibration - range => min = %d max = %d"), min_value, max_value);
}

void CCalSetting::SetRange(int delta)
{
    int min_val, max_val;

    min_val = m_pSetting->GetValue() - delta;
    if (min_val < min)
    {
        min_val = min;
    }
    max_val = m_pSetting->GetValue() + delta;
    if (max_val > max)
    {
        max_val = max;
    }
    SetRange(min_val, max_val);
}

void CCalSetting::SetRange(unsigned int* mask)
{
    int i, nb;

    for (i=0 ; i<mask_nb ; i++)
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

int CCalSetting::GetRange(unsigned int** mask, int* min_val, int* max_val)
{
    int i, nb;

    for (i=0,nb=0 ; i<=(max - min) ; i++)
    {
        if (mask_input[i/32] & (1 << (i%32)))
        {
            nb++;
        }
    }

    if (mask != NULL)
    {
        *mask = (unsigned int*) malloc(mask_nb * sizeof(unsigned int));
        for (i=0 ; i<mask_nb; i++)
        {
            (*mask)[i] = mask_input[i];
        }
    }
    *min_val = min_value;
    *max_val = max_value;

    return nb;
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
    m_pSetting->ChangeValue(RESET_SILENT);
    current_value = m_pSetting->GetValue();
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
    unsigned int* mask;

    nb_min = GetResult(&mask, &best_val_min, &best_val_max);
    free(mask);
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
    LOG(2, _T("Automatic Calibration - finished - %d values between %d and %d => %d"), nb_min, best_val_min, best_val_max, current_value);
}

void CCalSetting::InitResult()
{
    int i;

    min_diff = MAX_VALUE;
    max_diff = 0;
    desc = FALSE;
    for (i=0 ; i<mask_nb ; i++)
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

        for (j=0 ; j<mask_nb ; j++)
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
    LOG(3, _T("Automatic Calibration - value %d => result = %d min = %d"), current_value, diff, min_diff);
    return min_found;
}

int CCalSetting::GetResult(unsigned int** mask, int* min_val, int* max_val)
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

    *mask = (unsigned int*) malloc(mask_nb * sizeof(unsigned int));

    if (nb_min > 0)
    {
        for (i=0 ; i<mask_nb; i++)
        {
            (*mask)[i] = mask_output[i];
        }
        *min_val = best_val_min;
        *max_val = best_val_max;
    }

    return nb_min;
}

void CCalSetting::Adjust(int value)
{
    current_value = value;
    m_pSetting->SetValue(current_value);
}

/////////////////////////////////////////////////////////////////////////////
// Class CCalibration

CCalibration::CCalibration()
{
    m_CurTestPat = NULL;
    m_CurSubPat = NULL;
    m_TypeCalibration = CAL_MANUAL;
    m_IsRunning = FALSE;

    m_Brightness   = NULL;
    m_Contrast     = NULL;
    m_Saturation_U = NULL;
    m_Saturation_V = NULL;
    m_Hue          = NULL;

    m_YUVRangePat = NULL;

    last_tick_count = -1;
    LoadTestPatterns();

#ifdef TEST_CONV_COLORSPACE
    // Tests of conversion RGB <=> YCbCr
    unsigned char r, g, b, y, cb, cr;
    CColorBar* bar = m_TestPatterns[0]->m_ColorBars[0];
    r = 0, g = 0; b = 0;
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    LOG(1, _T("RGB(%u, %u, %u) => YCbCr(%u, %u, %u)"), r, g, b, y, cb, cr);
    r = 255, g = 255; b = 255;
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    LOG(1, _T("RGB(%u, %u, %u) => YCbCr(%u, %u, %u)"), r, g, b, y, cb, cr);
    r = 255, g = 0; b = 0;
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    LOG(1, _T("RGB(%u, %u, %u) => YCbCr(%u, %u, %u)"), r, g, b, y, cb, cr);
    r = 0, g = 255; b = 0;
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    LOG(1, _T("RGB(%u, %u, %u) => YCbCr(%u, %u, %u)"), r, g, b, y, cb, cr);
    r = 0, g = 0; b = 255;
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    LOG(1, _T("RGB(%u, %u, %u) => YCbCr(%u, %u, %u)"), r, g, b, y, cb, cr);
    r = 191, g = 0; b = 0;
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    LOG(1, _T("RGB(%u, %u, %u) => YCbCr(%u, %u, %u)"), r, g, b, y, cb, cr);
    r = 0, g = 191; b = 0;
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    LOG(1, _T("RGB(%u, %u, %u) => YCbCr(%u, %u, %u)"), r, g, b, y, cb, cr);
    r = 0, g = 0; b = 191;
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    LOG(1, _T("RGB(%u, %u, %u) => YCbCr(%u, %u, %u)"), r, g, b, y, cb, cr);
    r = 191, g = 191; b = 0;
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    LOG(1, _T("RGB(%u, %u, %u) => YCbCr(%u, %u, %u)"), r, g, b, y, cb, cr);
    r = 191, g = 0; b = 191;
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    LOG(1, _T("RGB(%u, %u, %u) => YCbCr(%u, %u, %u)"), r, g, b, y, cb, cr);
    r = 0, g = 191; b = 191;
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    LOG(1, _T("RGB(%u, %u, %u) => YCbCr(%u, %u, %u)"), r, g, b, y, cb, cr);
    r = 191, g = 191; b = 191;
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    LOG(1, _T("RGB(%u, %u, %u) => YCbCr(%u, %u, %u)"), r, g, b, y, cb, cr);
    y = 16, cb = 128; cr = 128;
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    LOG(1, _T("YCbCr(%u, %u, %u) => RGB(%u, %u, %u)"), y, cb, cr, r, g, b);
    y = 235, cb = 128; cr = 128;
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    LOG(1, _T("YCbCr(%u, %u, %u) => RGB(%u, %u, %u)"), y, cb, cr, r, g, b);
    y = 65, cb = 100; cr = 212;
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    LOG(1, _T("YCbCr(%u, %u, %u) => RGB(%u, %u, %u)"), y, cb, cr, r, g, b);
    y = 112, cb = 72; cr = 58;
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    LOG(1, _T("YCbCr(%u, %u, %u) => RGB(%u, %u, %u)"), y, cb, cr, r, g, b);
    y = 35, cb = 212; cr = 114;
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    LOG(1, _T("YCbCr(%u, %u, %u) => RGB(%u, %u, %u)"), y, cb, cr, r, g, b);
    y = 162, cb = 44; cr = 142;
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    LOG(1, _T("YCbCr(%u, %u, %u) => RGB(%u, %u, %u)"), y, cb, cr, r, g, b);
    y = 84, cb = 184; cr = 198;
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    LOG(1, _T("YCbCr(%u, %u, %u) => RGB(%u, %u, %u)"), y, cb, cr, r, g, b);
    y = 131, cb = 156; cr = 44;
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    LOG(1, _T("YCbCr(%u, %u, %u) => RGB(%u, %u, %u)"), y, cb, cr, r, g, b);
    y = 180, cb = 128; cr = 128;
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    LOG(1, _T("YCbCr(%u, %u, %u) => RGB(%u, %u, %u)"), y, cb, cr, r, g, b);

    r = 0, g = 0; b = 0;
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    LOG(1, _T("RGB(0, 0, 0) => YCbCr(%u, %u, %u) => RGB(%u, %u, %u)"), y, cb, cr, r, g, b);
    r = 255, g = 255; b = 255;
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    LOG(1, _T("RGB(255, 255, 255) => YCbCr(%u, %u, %u) => RGB(%u, %u, %u)"), y, cb, cr, r, g, b);
    r = 255, g = 0; b = 0;
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    LOG(1, _T("RGB(255, 0, 0) => YCbCr(%u, %u, %u) => RGB(%u, %u, %u)"), y, cb, cr, r, g, b);
    r = 0, g = 255; b = 0;
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    LOG(1, _T("RGB(0, 255, 0) => YCbCr(%u, %u, %u) => RGB(%u, %u, %u)"), y, cb, cr, r, g, b);
    r = 0, g = 0; b = 255;
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    LOG(1, _T("RGB(0, 0, 255) => YCbCr(%u, %u, %u) => RGB(%u, %u, %u)"), y, cb, cr, r, g, b);
    r = 191, g = 0; b = 0;
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    LOG(1, _T("RGB(191, 0, 0) => YCbCr(%u, %u, %u) => RGB(%u, %u, %u)"), y, cb, cr, r, g, b);
    r = 0, g = 191; b = 0;
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    LOG(1, _T("RGB(0, 191, 0) => YCbCr(%u, %u, %u) => RGB(%u, %u, %u)"), y, cb, cr, r, g, b);
    r = 0, g = 0; b = 191;
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    LOG(1, _T("RGB(0, 0, 191) => YCbCr(%u, %u, %u) => RGB(%u, %u, %u)"), y, cb, cr, r, g, b);
    r = 191, g = 191; b = 0;
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    LOG(1, _T("RGB(191, 191, 0) => YCbCr(%u, %u, %u) => RGB(%u, %u, %u)"), y, cb, cr, r, g, b);
    r = 191, g = 0; b = 191;
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    LOG(1, _T("RGB(191, 0, 191) => YCbCr(%u, %u, %u) => RGB(%u, %u, %u)"), y, cb, cr, r, g, b);
    r = 0, g = 191; b = 191;
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    LOG(1, _T("RGB(0, 191, 191) => YCbCr(%u, %u, %u) => RGB(%u, %u, %u)"), y, cb, cr, r, g, b);
    r = 191, g = 191; b = 191;
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    LOG(1, _T("RGB(191, 191, 191) => YCbCr(%u, %u, %u) => RGB(%u, %u, %u)"), y, cb, cr, r, g, b);

    y = 16, cb = 128; cr = 128;
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    LOG(1, _T("YCbCr(16, 128, 128) => RGB(%u, %u, %u) => YCbCr(%u, %u, %u)"), r, g, b, y, cb, cr);
    y = 235, cb = 128; cr = 128;
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    LOG(1, _T("YCbCr(235, 128, 128) => RGB(%u, %u, %u) => YCbCr(%u, %u, %u)"), r, g, b, y, cb, cr);
    y = 65, cb = 100; cr = 212;
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    LOG(1, _T("YCbCr(65, 100, 212) => RGB(%u, %u, %u) => YCbCr(%u, %u, %u)"), r, g, b, y, cb, cr);
    y = 112, cb = 72; cr = 58;
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    LOG(1, _T("YCbCr(112, 72, 58) => RGB(%u, %u, %u) => YCbCr(%u, %u, %u)"), r, g, b, y, cb, cr);
    y = 35, cb = 212; cr = 114;
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    LOG(1, _T("YCbCr(35, 212, 114) => RGB(%u, %u, %u) => YCbCr(%u, %u, %u)"), r, g, b, y, cb, cr);
    y = 162, cb = 44; cr = 142;
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    LOG(1, _T("YCbCr(162, 44, 142) => RGB(%u, %u, %u) => YCbCr(%u, %u, %u)"), r, g, b, y, cb, cr);
    y = 84, cb = 184; cr = 198;
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    LOG(1, _T("YCbCr(84, 184, 198) => RGB(%u, %u, %u) => YCbCr(%u, %u, %u)"), r, g, b, y, cb, cr);
    y = 131, cb = 156; cr = 44;
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    LOG(1, _T("YCbCr(131, 156, 44) => RGB(%u, %u, %u) => YCbCr(%u, %u, %u)"), r, g, b, y, cb, cr);
    y = 180, cb = 128; cr = 128;
    bar->YUV2RGB(y, cb, cr, &r, &g, &b);
    bar->RGB2YUV(r, g, b, &y, &cb, &cr);
    LOG(1, _T("YCbCr(180, 128, 128) => RGB(%u, %u, %u) => YCbCr(%u, %u, %u)"), r, g, b, y, cb, cr);
#endif
}

CCalibration::~CCalibration()
{
    UnloadTestPatterns();

    if (m_Brightness != NULL)
        delete m_Brightness;
    if (m_Contrast != NULL)
        delete m_Contrast;
    if (m_Saturation_U != NULL)
        delete m_Saturation_U;
    if (m_Saturation_V != NULL)
        delete m_Saturation_V;
    if (m_Hue != NULL)
        delete m_Hue;
}

// This method loads all the predefined test patterns
void CCalibration::LoadTestPatterns()
{
    CTestPattern* pattern;
    TCHAR BufferLine[512];
    TCHAR* Buffer;
    struct _stat st;
    FILE* File;
    tstring PaternPath(GetInstallationPath() + _T("\\patterns\\"));
    tstring FullPath;
    CColorBar* color_bar;

    FullPath = PaternPath + _T("card_calibr.d3u");
    File = _tfopen(FullPath.c_str(), _T("r"));
    if(File != NULL)
    {
        while(!feof(File))
        {
            if(_fgetts(BufferLine, 512, File))
            {
                BufferLine[511] = '\0';
                Buffer = BufferLine;
                while(_tcslen(Buffer) > 0 && *Buffer <= ' ')
                {
                    Buffer++;
                }
                if(_tcslen(Buffer) == 0 || *Buffer == '#' || *Buffer == ';')
                {
                    continue;
                }
                // take care of stuff that is at end of the line
                while(_tcslen(Buffer) > 0 && Buffer[_tcslen(Buffer) - 1] <= ' ')
                {
                    Buffer[_tcslen(Buffer) - 1] = '\0';
                }
                if (_tcslen(Buffer) == 0 || _tcslen(Buffer) > 118)
                {
                    continue;
                }
                if (!_tcsncmp(&Buffer[1], _T(":\\"), 2) || (Buffer[0] == '\\'))
                {
                    if (!_tstat(Buffer, &st))
                    {
                        pattern = new CTestPattern(Buffer);
                        if ((pattern->GetWidth() * pattern->GetHeight()) > 0)
                        {
                            m_TestPatterns.push_back(pattern);
                        }
                    }
                }
                else
                {
                    FullPath = PaternPath + Buffer;
                    if (!_tstat(FullPath.c_str(), &st))
                    {
                        pattern = new CTestPattern(FullPath);
                        if ((pattern->GetWidth() * pattern->GetHeight()) > 0)
                        {
                            m_TestPatterns.push_back(pattern);
                        }
                    }
                }
            }
        }
        fclose(File);
    }

    m_YUVRangePat = new CTestPattern(_T("Check YUV Range"), 720, 480);
    color_bar = new CColorBar(0, 10000, 0, 10000, DRAW_NO, 0, 0, FALSE, 0, 0, 0, 0, 0, 0);
    m_YUVRangePat->m_ColorBars.push_back(color_bar);
    m_YUVRangePat->CreateGlobalSubPattern();
}

// This method unloads all the predefined test patterns
void CCalibration::UnloadTestPatterns()
{
    // Destroy all test patterns
    for(vector<CTestPattern*>::iterator it = m_TestPatterns.begin();
        it != m_TestPatterns.end();
        ++it)
    {
        delete *it;
    }
    m_TestPatterns.clear();
    delete m_YUVRangePat;
    m_YUVRangePat = NULL;
    m_CurTestPat = NULL;
    m_CurSubPat = NULL;
}

BOOL CCalibration::ProcessSelection(HWND hWnd, WORD wMenuId)
{
    int i = 0;
    for(vector<CTestPattern*>::iterator it = m_TestPatterns.begin();
        it != m_TestPatterns.end();
        ++it, ++i)
    {
        if (wMenuId == IDM_PATTERN_SELECT + i + 1)
        {
            pCalibration->SelectTestPattern(i);
            return TRUE;
        }
    }
    return FALSE;
}


void CCalibration::UpdateMenu(HMENU hMenu)
{
    HMENU           hMenuPatterns;
    MENUITEMINFO    MenuItemInfo;
    int             i;
    tstring          name;

    hMenuPatterns = GetPatternsSubmenu();
    if (hMenuPatterns == NULL) return;

    i = 0;
    for(vector<CTestPattern*>::iterator it = m_TestPatterns.begin();
        it != m_TestPatterns.end();
        ++it, ++i)
    {
        name = (*it)->GetName();

        MenuItemInfo.cbSize = sizeof (MenuItemInfo);
        MenuItemInfo.fType = MFT_STRING;
        MenuItemInfo.dwTypeData = &name[0];
        MenuItemInfo.cch = name.length();

        MenuItemInfo.fMask = MIIM_TYPE | MIIM_ID;
        MenuItemInfo.wID = IDM_PATTERN_SELECT + i + 1;
        InsertMenuItem(hMenuPatterns, i, TRUE, &MenuItemInfo);
    }
}

void CCalibration::SetMenu(HMENU hMenu)
{
    HMENU   hMenuPatterns;
    int     i;
    eTypeContentPattern type_content;
    int     SourceHeight;
    CSource* pSource = Providers_GetCurrentSource();
    BOOL    OkBK, OkC, OkH;

    if (pSource != NULL)
    {
        SourceHeight = pSource->GetHeight();
        OkBK = (pSource->GetBrightness() != NULL && pSource->GetContrast() != NULL);
        OkC = ( (pSource->GetSaturationU() != NULL && pSource->GetSaturationV() != NULL)
             || (pSource->GetSaturationU() == NULL && pSource->GetSaturationV() == NULL && pSource->GetSaturation() != NULL) );
        OkH = (pSource->GetHue() != NULL);
    }
    else
    {
        SourceHeight = -1;
        OkBK = FALSE;
        OkC = FALSE;
        OkH = FALSE;
    }

    if ((m_CurTestPat != NULL) && (m_CurTestPat->GetHeight() != SourceHeight))
    {
        m_CurTestPat = NULL;
    }

    if (m_CurTestPat != NULL)
    {
        type_content = m_CurTestPat->DetermineTypeContent();
    }
    else
    {
        type_content = PAT_UNKNOWN;
    }

    hMenuPatterns = GetPatternsSubmenu();
    if (hMenuPatterns == NULL) return;

    i = 0;
    for(vector<CTestPattern*>::iterator it = m_TestPatterns.begin();
        it != m_TestPatterns.end();
        ++it, ++i)
    {
        EnableMenuItem(hMenuPatterns, i, (m_IsRunning || ((*it)->GetHeight() != SourceHeight)) ? MF_BYPOSITION | MF_GRAYED : MF_BYPOSITION | MF_ENABLED);
        CheckMenuItem(hMenuPatterns, i, (m_CurTestPat == (*it)) ? MF_BYPOSITION | MF_CHECKED : MF_BYPOSITION | MF_UNCHECKED);
    }

    EnableMenuItem(hMenu, IDM_START_MANUAL_CALIBRATION, (m_IsRunning || (m_CurTestPat == NULL)) ? MF_GRAYED : MF_ENABLED);
    EnableMenuItem(hMenu, IDM_START_YUV_RANGE, m_IsRunning ? MF_GRAYED : MF_ENABLED);
    if (OkBK && OkC && OkH)
    {
        EnableMenuItem(hMenu, IDM_START_AUTO_CALIBRATION, (m_IsRunning || (m_CurTestPat == NULL) || (type_content != PAT_GRAY_AND_COLOR)) ? MF_GRAYED : MF_ENABLED);
    }
    else
    {
        EnableMenuItem(hMenu, IDM_START_AUTO_CALIBRATION, MF_GRAYED);
    }
    if (OkC && OkH)
    {
        EnableMenuItem(hMenu, IDM_START_AUTO_CALIBRATION3, (m_IsRunning || (m_CurTestPat == NULL) || ((type_content != PAT_GRAY_AND_COLOR) && (type_content != PAT_COLOR))) ? MF_GRAYED : MF_ENABLED);
    }
    else
    {
        EnableMenuItem(hMenu, IDM_START_AUTO_CALIBRATION3, MF_GRAYED);
    }
    if (OkBK)
    {
        EnableMenuItem(hMenu, IDM_START_AUTO_CALIBRATION2, (m_IsRunning || (m_CurTestPat == NULL) || ((type_content != PAT_GRAY_AND_COLOR) && (type_content != PAT_RANGE_OF_GRAY))) ? MF_GRAYED : MF_ENABLED);
    }
    else
    {
        EnableMenuItem(hMenu, IDM_START_AUTO_CALIBRATION2, MF_GRAYED);
    }
    EnableMenuItem(hMenu, IDM_STOP_CALIBRATION, (!m_IsRunning || (m_CurTestPat == NULL)) ? MF_GRAYED : MF_ENABLED);
}

void CCalibration::SelectTestPattern(int num)
{
    if ( (num >= 0) && (num < m_TestPatterns.size()) )
    {
        m_CurTestPat = m_TestPatterns[num];
    }
    else
    {
        m_CurTestPat = NULL;
    }
}

CTestPattern* CCalibration::GetCurrentTestPattern()
{
    return m_CurTestPat;
}

CSubPattern* CCalibration::GetSubPattern(eTypeAdjust type_adjust)
{
    CSubPattern* sub_pattern = NULL;

    if (m_CurTestPat != NULL)
    {
        sub_pattern = m_CurTestPat->GetSubPattern(type_adjust);
    }

    return sub_pattern;
}

CSubPattern* CCalibration::GetCurrentSubPattern()
{
    return m_CurSubPat;
}

void CCalibration::Start(eTypeCalibration type)
{
    BOOL OkToStart = FALSE;

    CSource* pSource = Providers_GetCurrentSource();
    if (pSource == NULL)
        return;

    if (type == CAL_CHECK_YUV_RANGE)
    {
        if (m_YUVRangePat != NULL)
        {
            m_YUVRangePat->SetSize(pSource->GetWidth(), pSource->GetHeight());
        }
        m_CurTestPat = m_YUVRangePat;
    }

    if (m_CurTestPat == NULL)
        return;

    if (m_Brightness != NULL)
    {
        delete m_Brightness;
        m_Brightness = NULL;
    }
    if (m_Contrast != NULL)
    {
        delete m_Contrast;
        m_Contrast = NULL;
    }
    if (m_Saturation_U != NULL)
    {
        delete m_Saturation_U;
        m_Saturation_U = NULL;
    }
    if (m_Saturation_V != NULL)
    {
        delete m_Saturation_V;
        m_Saturation_V = NULL;
    }
    if (m_Hue != NULL)
    {
        delete m_Hue;
        m_Hue = NULL;
    }

    /// \todo this is bad coding sort this out
    if (pSource->GetBrightness() != NULL)
    {
        m_Brightness = new CCalSetting(pSource->GetBrightness());
        m_Brightness->Update();
        m_Brightness->Save();
    }
    if (pSource->GetContrast() != NULL)
    {
        m_Contrast = new CCalSetting(pSource->GetContrast());
        m_Contrast->Update();
        m_Contrast->Save();
    }
    if (pSource->GetSaturationU() != NULL)
    {
        m_Saturation_U = new CCalSetting(pSource->GetSaturationU());
        m_Saturation_U->Update();
        m_Saturation_U->Save();
    }
    if (pSource->GetSaturationV() != NULL)
    {
        m_Saturation_V = new CCalSetting(pSource->GetSaturationV());
        m_Saturation_V->Update();
        m_Saturation_V->Save();
    }
    // This case if for cards which have only one unique color setting
    if (pSource->GetSaturationU() == NULL && pSource->GetSaturationV() == NULL && pSource->GetSaturation() != NULL)
    {
        m_Saturation_U = new CCalSetting(pSource->GetSaturation());
        m_Saturation_U->Update();
        m_Saturation_U->Save();
    }
    if (pSource->GetHue() != NULL)
    {
        m_Hue = new CCalSetting(pSource->GetHue());
        m_Hue->Update();
        m_Hue->Save();
    }

    m_TypeCalibration = type;

    switch (m_TypeCalibration)
    {
    case CAL_AUTO_BRIGHT_CONTRAST:
        initial_step = 1;
        nb_steps = 11;
        if (m_Brightness != NULL && m_Contrast != NULL)
        {
            OkToStart = TRUE;
            m_Brightness->AdjustDefault();
            m_Contrast->AdjustDefault();
        }
        break;
    case CAL_AUTO_COLOR:
        initial_step = 12;
        nb_steps = 12;
        if (m_Saturation_U != NULL)
        {
            OkToStart = TRUE;
            m_Saturation_U->AdjustDefault();
            if (m_Saturation_V != NULL)
            {
                m_Saturation_V->AdjustDefault();
            }
            if (m_Hue != NULL)
            {
                m_Hue->AdjustDefault();
            }
        }
        break;
    case CAL_AUTO_FULL:
        initial_step = 1;
        nb_steps = 23;
        if (m_Brightness != NULL && m_Contrast != NULL && m_Saturation_U != NULL)
        {
            OkToStart = TRUE;
            m_Brightness->AdjustDefault();
            m_Contrast->AdjustDefault();
            m_Saturation_U->AdjustDefault();
            if (m_Saturation_V != NULL)
            {
                m_Saturation_V->AdjustDefault();
            }
            if (m_Hue != NULL)
            {
                m_Hue->AdjustDefault();
            }
        }
        break;
    case CAL_MANUAL:
    case CAL_CHECK_YUV_RANGE:
        initial_step = 0;
        nb_steps = 1;
        OkToStart = TRUE;
        break;
    default:
        break;
    }
    if (OkToStart)
    {
        current_step = initial_step;
        full_range = FALSE;
        nb_tries = 0;
        first_calc = TRUE;

        // Display the specific OSD screen
        OSD_ShowInfosScreen(0, 0);

        m_IsRunning = TRUE;
    }
}

void CCalibration::Stop()
{
    if ( (m_TypeCalibration != CAL_MANUAL)
      && (m_TypeCalibration != CAL_CHECK_YUV_RANGE) )
    {
        OSD_ShowInfosScreen(0, 0);
        if ( (current_step != -1)
          || (MessageBox(GetMainWnd(), _T("Do you want to keep the current settings ?"), _T("DScaler Question"), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1) == IDNO) )
        {
            if (m_Brightness != NULL)
                m_Brightness->Restore();
            if (m_Contrast != NULL)
                m_Contrast->Restore();
            if (m_Saturation_U != NULL)
                m_Saturation_U->Restore();
            if (m_Saturation_V != NULL)
                m_Saturation_V->Restore();
            if (m_Hue != NULL)
                m_Hue->Restore();
        }
    }

    // Erase the OSD screen
    OSD_Clear();

    if (m_TypeCalibration == CAL_CHECK_YUV_RANGE)
    {
        m_CurTestPat = NULL;
        m_CurSubPat = NULL;
    }

    m_IsRunning = FALSE;
}

BOOL CCalibration::IsRunning()
{
    return m_IsRunning;
}

BOOL CCalibration::GetCurrentStep()
{
    return current_step;
}

eTypeCalibration CCalibration::GetType()
{
    return m_TypeCalibration;
}

void CCalibration::Make(TDeinterlaceInfo* pInfo, int tick_count)
{
    int nb1, nb2, nb3;
    int min, max;
    unsigned int* mask;
    BOOL new_settings;
    BOOL found;

    if (!m_IsRunning
     || (m_CurTestPat == NULL))
        return;

    if ((last_tick_count != -1) && ((tick_count - last_tick_count) < MIN_TIME_BETWEEN_CALC))
    {
        if (m_CurSubPat != NULL)
        {
            m_CurSubPat->Draw(pInfo);
        }
        return;
    }

    last_tick_count = tick_count;

    switch (current_step)
    {
    case -1:    // Automatic calibration finished
        Stop();
        break;

    case 0:     // Manual calibration
        m_CurSubPat = GetSubPattern(ADJ_MANUAL);
        if (m_CurSubPat == NULL)
        {
            break;
        }

        new_settings = FALSE;
        if (m_Brightness != NULL)
            new_settings |= m_Brightness->Update();
        if (m_Contrast != NULL)
            new_settings |= m_Contrast->Update();
        if (m_Saturation_U != NULL)
            new_settings |= m_Saturation_U->Update();
        if (m_Saturation_V != NULL)
            new_settings |= m_Saturation_V->Update();
        if (m_Hue != NULL)
            new_settings |= m_Hue->Update();

        // Calculations with current setitngs
        if ( m_CurSubPat->CalcCurrentSubPattern(first_calc || new_settings, NB_CALCULATIONS_LOW, pInfo)
          && (m_TypeCalibration != CAL_MANUAL)
          && (m_TypeCalibration != CAL_CHECK_YUV_RANGE) )
        {
            current_step = -1;
            last_tick_count = tick_count + 500;
        }
        first_calc = FALSE;
        break;

    case 1:
        m_Brightness->SetRange((nb_tries == 0) ? 75 : 25);
        if (step_init(ADJ_BRIGHTNESS, m_Brightness, (CCalSetting* )NULL, (CCalSetting* )NULL))
        {
            LOG(2, _T("Automatic Calibration - brightness - reduced range - try %d"), nb_tries+1);
            current_step++;
        }
        else
        {
            // We stop calibration
            current_step = initial_step + nb_steps;
        }
        break;

    case 2:     // Step to find a short range for brightness setting
        if (step_process(pInfo, 1, NB_CALCULATIONS_LOW, TRUE, FALSE, &found))
        {
            current_step += (found ? 3 : 1);
        }
        break;

    case 3:
        m_Brightness->SetFullRange();
        if (step_init(ADJ_BRIGHTNESS, m_Brightness, (CCalSetting* )NULL, (CCalSetting* )NULL))
        {
            LOG(2, _T("Automatic Calibration - brightness - full range - try %d"), nb_tries+1);
            current_step++;
        }
        else
        {
            // We stop calibration
            current_step = initial_step + nb_steps;
        }
        break;

    case 4:     // Step to find a short range for brightness setting
        if (step_process(pInfo, 1, NB_CALCULATIONS_LOW, TRUE, FALSE, &found))
        {
            current_step++;
        }
        break;

    case 5:
        m_Contrast->SetRange((nb_tries == 0) ? 50 : 25);
        if (step_init(ADJ_CONTRAST, m_Contrast, (CCalSetting* )NULL, (CCalSetting* )NULL))
        {
            LOG(2, _T("Automatic Calibration - contrast - reduced range - try %d"), nb_tries+1);
            current_step++;
        }
        else
        {
            // We stop calibration
            current_step = initial_step + nb_steps;
        }
        break;

    case 6:     // Step to find a short range for contrast setting
        if (step_process(pInfo, 1, NB_CALCULATIONS_LOW, TRUE, FALSE, &found))
        {
            current_step += (found ? 3 : 1);
        }
        break;

    case 7:
        m_Contrast->SetFullRange();
        if (step_init(ADJ_CONTRAST, m_Contrast, (CCalSetting* )NULL, (CCalSetting* )NULL))
        {
            LOG(2, _T("Automatic Calibration - contrast - full range - try %d"), nb_tries+1);
            current_step++;
        }
        else
        {
            // We stop calibration
            current_step = initial_step + nb_steps;
        }
        break;

    case 8:     // Step to find a short range for contrast setting
        if (step_process(pInfo, 1, NB_CALCULATIONS_LOW, TRUE, FALSE, &found))
        {
            current_step++;
        }
        break;

    case 9:
        nb_tries++;
        if (nb_tries < 2)
        {
            current_step -= 8;
        }
        else
        {
            nb_tries = 0;
            current_step++;
        }
        break;

    case 10:
        if (m_Brightness->GetResult(&mask, &min, &max) > 0)
        {
            m_Brightness->SetRange(mask);
        }
        else
        {
            m_Brightness->SetRange(0);
        }
        free(mask);
        nb1 = m_Brightness->GetRange(NULL, &min, &max);
        if (m_Contrast->GetResult(&mask, &min, &max) > 0)
        {
            m_Contrast->SetRange(mask);
        }
        else
        {
            m_Contrast->SetRange(0);
        }
        free(mask);
        nb2 = m_Contrast->GetRange(NULL, &min, &max);
        if ((nb1 == 1) && (nb2 == 1))
        {
            current_step += 2;
            break;
        }
        if (step_init(ADJ_BRIGHTNESS_CONTRAST, m_Brightness, m_Contrast, (CCalSetting* )NULL))
        {
            LOG(2, _T("Automatic Calibration - brightness + contrast - %d %d"), nb1, nb2);
            current_step++;
        }
        else
        {
            current_step += 2;
        }
        break;

    case 11:     // Step to adjust fine brightness + contradt
        if (step_process(pInfo, 1, NB_CALCULATIONS_HIGH, FALSE, TRUE, &found))
        {
            current_step++;
        }
        break;

    case 12:
        m_Saturation_U->SetRange(75);
        if (step_init(ADJ_SATURATION_U, m_Saturation_U, (CCalSetting* )NULL, (CCalSetting* )NULL))
        {
            LOG(2, _T("Automatic Calibration - saturation U - reduced range - try %d"), nb_tries+1);
            current_step++;
        }
        else
        {
            // We stop calibration
            current_step = initial_step + nb_steps;
        }
        break;

    case 13:     // Step to find a short range for saturation U setting
        if (step_process(pInfo, 2, NB_CALCULATIONS_LOW, TRUE, FALSE, &found))
        {
            current_step += (found ? 3 : 1);
        }
        break;

    case 14:
        m_Saturation_U->SetFullRange();
        if (step_init(ADJ_SATURATION_U, m_Saturation_U, (CCalSetting* )NULL, (CCalSetting* )NULL))
        {
            LOG(2, _T("Automatic Calibration - saturation U - full range - try %d"), nb_tries+1);
            current_step++;
        }
        else
        {
            // We stop calibration
            current_step = initial_step + nb_steps;
        }
        break;

    case 15:     // Step to find a short range for saturation U setting
        if (step_process(pInfo, 2, NB_CALCULATIONS_LOW, TRUE, FALSE, &found))
        {
            current_step++;
        }
        break;

    case 16:
        if (m_Saturation_V == NULL)
        {
            current_step += 4;
            break;
        }
        m_Saturation_V->SetRange(75);
        if (step_init(ADJ_SATURATION_V, m_Saturation_V, (CCalSetting* )NULL, (CCalSetting* )NULL))
        {
            LOG(2, _T("Automatic Calibration - saturation V - reduced range - try %d"), nb_tries+1);
            current_step++;
        }
        else
        {
            // We stop calibration
            current_step = initial_step + nb_steps;
        }
        break;

    case 17:    // Step to find a short range for saturation V setting
        if (step_process(pInfo, 3, NB_CALCULATIONS_LOW, TRUE, FALSE, &found))
        {
            current_step += (found ? 3 : 1);
        }
        break;

    case 18:
        if (m_Saturation_V == NULL)
        {
            current_step += 2;
            break;
        }
        m_Saturation_V->SetFullRange();
        if (step_init(ADJ_SATURATION_V, m_Saturation_V, (CCalSetting* )NULL, (CCalSetting* )NULL))
        {
            LOG(2, _T("Automatic Calibration - saturation V - full range - try %d"), nb_tries+1);
            current_step++;
        }
        else
        {
            // We stop calibration
            current_step = initial_step + nb_steps;
        }
        break;

    case 19:    // Step to find a short range for saturation V setting
        if (step_process(pInfo, 3, NB_CALCULATIONS_LOW, TRUE, FALSE, &found))
        {
            current_step++;
        }
        break;

    case 20:
        if (m_Hue == NULL)
        {
            current_step += 2;
            break;
        }
        m_Hue->SetRange(30);
        if (step_init(ADJ_HUE, m_Hue, (CCalSetting* )NULL, (CCalSetting* )NULL))
        {
            LOG(2, _T("Automatic Calibration - hue - reduced range - try %d"), nb_tries+1);
            current_step++;
        }
        else
        {
            // We stop calibration
            current_step = initial_step + nb_steps;
        }
        break;

    case 21:    // Step to find a short range for hue setting
        if (step_process(pInfo, 2, NB_CALCULATIONS_LOW, TRUE, FALSE, &found))
        {
            current_step++;
        }
        break;

    case 22:
        if (m_Saturation_U->GetResult(&mask, &min, &max) > 0)
        {
            m_Saturation_U->SetRange(mask);
        }
        else
        {
            m_Saturation_U->SetRange(0);
        }
        free(mask);
        nb1 = m_Saturation_U->GetRange(NULL, &min, &max);
        if (m_Saturation_V != NULL)
        {
            if (m_Saturation_V->GetResult(&mask, &min, &max) > 0)
            {
                m_Saturation_V->SetRange(mask);
            }
            else
            {
                m_Saturation_V->SetRange(0);
            }
            free(mask);
            nb2 = m_Saturation_V->GetRange(NULL, &min, &max);
        }
        else
        {
            nb2 = 1;
        }
        if (m_Hue != NULL)
        {
            if (m_Hue->GetResult(&mask, &min, &max) > 0)
            {
                m_Hue->SetRange(mask);
            }
            else
            {
                m_Hue->SetRange(0);
            }
            free(mask);
            nb3 = m_Hue->GetRange(NULL, &min, &max);
        }
        else
        {
            nb3 = 1;
        }
        if ((nb1 == 1) && (nb2 == 1) && (nb3 == 1))
        {
            current_step += 2;
            break;
        }
        if (step_init(ADJ_COLOR, m_Saturation_U, m_Saturation_V, m_Hue))
        {
            LOG(2, _T("Automatic Calibration - saturation U + saturation V + hue - %d %d %d"), nb1, nb2, nb3);
            current_step++;
        }
        else
        {
            current_step += 2;
        }
        break;

    case 23:    // Step to adjust fine color saturation and hue
        if (step_process(pInfo, 4, NB_CALCULATIONS_HIGH, FALSE, TRUE, &found))
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

    if (m_CurSubPat != NULL)
    {
        m_CurSubPat->Draw(pInfo);
    }
}

BOOL CCalibration::step_init(eTypeAdjust type_adjust, CCalSetting* _setting1, CCalSetting* _setting2, CCalSetting* _setting3)
{
    // Get the bar to use for this step
    m_CurSubPat = GetSubPattern(type_adjust);
    if (m_CurSubPat == NULL)
    {
        setting1 = (CCalSetting* )NULL;
        setting2 = (CCalSetting* )NULL;
        setting3 = (CCalSetting* )NULL;
        return FALSE;
    }
    else
    {
        // Initialize
        setting1 = _setting1;
        if (setting1 != (CCalSetting* )NULL)
        {
            // Set the settings to their minimum
            setting1->AdjustMin();
            setting1->InitResult();
        }
        setting2 = _setting2;
        if (setting2 != (CCalSetting* )NULL)
        {
            // Set the settings to their minimum
            setting2->AdjustMin();
            setting2->InitResult();
        }
        setting3 = _setting3;
        if (setting3 != (CCalSetting* )NULL)
        {
            // Set the settings to their minimum
            setting3->AdjustMin();
            setting3->InitResult();
        }

        nb_calcul = 0;
        total_dif = 0;

        return TRUE;
    }
}

BOOL CCalibration::step_process(TDeinterlaceInfo* pInfo, unsigned int sig_component, unsigned int nb_calc, BOOL stop_when_found, BOOL only_one, BOOL* best_found)
{
    int val[4];
    BOOL YUV;
    int idx;
//    int dif;

    // Calculations with current settings
    m_CurSubPat->CalcCurrentSubPattern(TRUE, 1, pInfo);

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
    m_CurSubPat->GetSumDeltaColor(YUV, &val[0], &val[1], &val[2], &val[3]);
//    dif = val[idx];
    total_dif += val[idx];
    nb_calcul++;

    // Waiting at least 5 calculations
    if (nb_calcul < nb_calc)
    {
        last_tick_count = -1;
        return FALSE;
    }

    if (setting1 != (CCalSetting* )NULL)
    {
       * best_found = setting1->UpdateResult(total_dif, stop_when_found ? DELTA_STOP*nb_calc : -1, only_one);
//       * best_found = setting1->UpdateResult(dif, stop_when_found ? DELTA_STOP : -1, only_one);
    }
    if (setting2 != (CCalSetting* )NULL)
    {
       * best_found = setting2->UpdateResult(total_dif, stop_when_found ? DELTA_STOP*nb_calc : -1, only_one);
//       * best_found = setting2->UpdateResult(dif, stop_when_found ? DELTA_STOP : -1, only_one);
    }
    if (setting3 != (CCalSetting* )NULL)
    {
       * best_found = setting3->UpdateResult(total_dif, stop_when_found ? DELTA_STOP*nb_calc : -1, only_one);
//       * best_found = setting3->UpdateResult(dif, stop_when_found ? DELTA_STOP : -1, only_one);
    }

    nb_calcul = 0;
    total_dif = 0;

    // Increase the third setting
    if ((setting3 != (CCalSetting* )NULL) && setting3->AdjustNext())
    {
        return FALSE;
    }
    // Increase the second setting
    else if ((setting2 != (CCalSetting* )NULL) && setting2->AdjustNext())
    {
        if (setting3 != (CCalSetting* )NULL)
        {
            // Set the third setting to its minimum
            setting3->AdjustMin();
        }
        return FALSE;
    }
    // Increase the first setting
    else if ((setting1 != (CCalSetting* )NULL) && setting1->AdjustNext())
    {
        if (setting2 != (CCalSetting* )NULL)
        {
            // Set the second setting to its minimum
            setting2->AdjustMin();
        }
        if (setting3 != (CCalSetting* )NULL)
        {
            // Set the third setting to its minimum
            setting3->AdjustMin();
        }
        return FALSE;
    }
    else
    {
        // Set the settings to the best values found
        if (setting1 != (CCalSetting* )NULL)
        {
            setting1->AdjustBest();
        }
        if (setting2 != (CCalSetting* )NULL)
        {
            setting2->AdjustBest();
        }
        if (setting3 != (CCalSetting* )NULL)
        {
            setting3->AdjustBest();
        }
        return TRUE;
    }
}

/////////////////////////////////////////////////////////////////////////////

SmartPtr<CCalibration> pCalibration;

/////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////

SETTING CalibrSettings[CALIBR_SETTING_LASTONE] =
{
    {
        "Left player cropping", SLIDER, 0, (long*)&LeftCropping,
         8, 0, 50, 1, 1,
         NULL,
        "Calibration", "LeftPlayerCropping", NULL,
    },
    {
        "Right player cropping", SLIDER, 0, (long*)&RightCropping,
         16, 0, 50, 1, 1,
         NULL,
        "Calibration", "RightPlayerCropping", NULL,
    },
    {
        "Show RGB delta in OSD", ONOFF, 0, (long*)&ShowRGBDelta,
         TRUE, 0, 1, 1, 1,
         NULL,
        "Calibration", "ShowRGBDelta", NULL,
    },
    {
        "Show YUV delta in OSD", ONOFF, 0, (long*)&ShowYUVDelta,
         TRUE, 0, 1, 1, 1,
         NULL,
        "Calibration", "ShowYUVDelta", NULL,
    },
};


SETTING* Calibr_GetSetting(CALIBR_SETTING Setting)
{
    if(Setting > -1 && Setting < CALIBR_SETTING_LASTONE)
    {
        return &(CalibrSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}
