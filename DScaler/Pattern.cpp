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
 * @file Pattern.cpp Pattern classes
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "Pattern.h"
#include "DebugLog.h"

using namespace std;

// Macro to restrict range to [0,255]
#define LIMIT_RGB(x)    (((x)<0)?0:((x)>255)?255:(x))
#define LIMIT_Y(x)      (((x)<16)?16:((x)>235)?235:(x))
#define LIMIT_CbCr(x)   (((x)<16)?16:((x)>240)?240:(x))

// Macro to return the absolute value
#define ABSOLUTE_VALUE(x) ((x) < 0) ? -(x) : (x)

// Macro to return the average value with rounding
#define AVERAGE_VALUE(sum,nb)   ((sum) + ((nb) / 2)) / (nb)


long LeftCropping = 8;
long RightCropping = 16;


/////////////////////////////////////////////////////////////////////////////
// Class CColorBar

CColorBar::CColorBar(unsigned short int left, unsigned short int right, unsigned short int top, unsigned short int bottom, eTypeDraw type_draw, int param1_draw, int param2_draw, BOOL YUV, unsigned char R_Y, unsigned char G_U, unsigned char B_V, unsigned char R_Y_2, unsigned char G_U_2, unsigned char B_V_2)
{
    m_LeftBorder = left;
    m_RightBorder = right;
    m_TopBorder = top;
    m_BottomBorder = bottom;
    m_TypeDraw = type_draw;
    m_Param1Draw = param1_draw;
    m_Param2Draw = param2_draw;
    if (YUV)
    {
        ref_Y_val = R_Y;
        ref_U_val = G_U;
        ref_V_val = B_V;
        // Calculate RGB values from YUV values
        YUV2RGB(ref_Y_val, ref_U_val, ref_V_val, &ref_R_val, &ref_G_val, &ref_B_val);
        ref_Y_val2 = R_Y_2;
        ref_U_val2 = G_U_2;
        ref_V_val2 = B_V_2;
        // Calculate RGB values from YUV values
        YUV2RGB(ref_Y_val2, ref_U_val2, ref_V_val2, &ref_R_val2, &ref_G_val2, &ref_B_val2);
    }
    else
    {
        ref_R_val = R_Y;
        ref_G_val = G_U;
        ref_B_val = B_V;
        // Calculate YUV values from RGB values
        RGB2YUV(ref_R_val, ref_G_val, ref_B_val, &ref_Y_val, &ref_U_val, &ref_V_val);
        ref_R_val2 = R_Y_2;
        ref_G_val2 = G_U_2;
        ref_B_val2 = B_V_2;
        // Calculate YUV values from RGB values
        RGB2YUV(ref_R_val2, ref_G_val2, ref_B_val2, &ref_Y_val2, &ref_U_val2, &ref_V_val2);
    }

    YUV_val_available = RGB_val_available = FALSE;

    for (int i = 0 ; i <= 2; i++)
    {
        component_cpt[i] = 0;
        component_min[i] = 255;
        component_max[i] = 0;
    }
    cpt_nb = 0;
    min_available = FALSE;
    max_available = FALSE;
}

CColorBar::CColorBar(CColorBar* pColorBar)
{
    unsigned char val1, val2, val3;
    unsigned short int left, right, top, bottom;

    pColorBar->GetPosition(&left, &right, &top, &bottom);
    m_LeftBorder = left;
    m_RightBorder = right;
    m_TopBorder = top;
    m_BottomBorder = bottom;

    m_TypeDraw = pColorBar->GetTypeDraw(&m_Param1Draw, &m_Param1Draw);

    pColorBar->GetRefColor(FALSE, &val1, &val2, &val3);
    ref_R_val = val1;
    ref_G_val = val2;
    ref_B_val = val3;

    pColorBar->GetRefColor(TRUE, &val1, &val2, &val3);
    ref_Y_val = val1;
    ref_U_val = val2;
    ref_V_val = val3;

    pColorBar->GetRefColor2(FALSE, &val1, &val2, &val3);
    ref_R_val2 = val1;
    ref_G_val2 = val2;
    ref_B_val2 = val3;

    pColorBar->GetRefColor2(TRUE, &val1, &val2, &val3);
    ref_Y_val2 = val1;
    ref_U_val2 = val2;
    ref_V_val2 = val3;

    YUV_val_available = RGB_val_available = FALSE;

    for (int i = 0 ; i <= 2; i++)
    {
        component_cpt[i] = 0;
        component_min[i] = 255;
        component_max[i] = 0;
    }
    cpt_nb = 0;
    min_available = FALSE;
    max_available = FALSE;
}

// This methode returns the position of the color bar
void CColorBar::GetPosition(unsigned short int* left, unsigned short int* right, unsigned short int* top, unsigned short int* bottom)
{
    *left = m_LeftBorder;
    *right = m_RightBorder;
    *top = m_TopBorder;
    *bottom = m_BottomBorder;
}

// This method returns the type of draw for the color bar
eTypeDraw CColorBar::GetTypeDraw(int* pParam1Draw, int* pParam2Draw)
{
    *pParam1Draw = m_Param1Draw;
    *pParam2Draw = m_Param2Draw;
    return m_TypeDraw;
}

// This methode returns the reference color
// If parameter YUV is TRUE, it returns YUV values else RGB values
void CColorBar::GetRefColor(BOOL YUV, BYTE* pR_Y, BYTE* pG_U, BYTE* pB_V)
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

// This methode returns the second reference color
// If parameter YUV is TRUE, it returns YUV values else RGB values
void CColorBar::GetRefColor2(BOOL YUV, BYTE* pR_Y, BYTE* pG_U, BYTE* pB_V)
{
    if (YUV)
    {
        *pR_Y = ref_Y_val2;
        *pG_U = ref_U_val2;
        *pB_V = ref_V_val2;
    }
    else
    {
        *pR_Y = ref_R_val2;
        *pG_U = ref_G_val2;
        *pB_V = ref_B_val2;
    }
}

// This methode returns the calculated average color
// If parameter YUV is TRUE, it returns YUV values else RGB values
BOOL CColorBar::GetCurrentAvgColor(BOOL YUV, BYTE* pR_Y, BYTE* pG_U, BYTE* pB_V)
{
    if (YUV && !YUV_val_available && RGB_val_available)
    {
        RGB2YUV(R_val, G_val, B_val, &Y_val, &U_val, &V_val);
        YUV_val_available = TRUE;
    }
    else if (!YUV && !RGB_val_available && YUV_val_available)
    {
        YUV2RGB(Y_val, U_val, V_val, &R_val, &G_val, &B_val);
        RGB_val_available = TRUE;
    }

    if (YUV && YUV_val_available)
    {
        *pR_Y = Y_val;
        *pG_U = U_val;
        *pB_V = V_val;
        return TRUE;
    }
    else if (!YUV && RGB_val_available)
    {
        *pR_Y = R_val;
        *pG_U = G_val;
        *pB_V = B_val;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

// This methode returns the delta between reference color and calculated average color
// It includes a delta for each color component + the sum of these three absolute deltas.
// If parameter YUV is TRUE, it returns YUV values else RGB values
BOOL CColorBar::GetDeltaColor(BOOL YUV, int* pR_Y, int* pG_U, int* pB_V, int* pTotal)
{
    if (YUV && !YUV_val_available && RGB_val_available)
    {
        RGB2YUV(R_val, G_val, B_val, &Y_val, &U_val, &V_val);
        YUV_val_available = TRUE;
    }
    else if (!YUV && !RGB_val_available && YUV_val_available)
    {
        YUV2RGB(Y_val, U_val, V_val, &R_val, &G_val, &B_val);
        RGB_val_available = TRUE;
    }

    if (YUV && YUV_val_available)
    {
        *pR_Y = Y_val - ref_Y_val;
        *pG_U = U_val - ref_U_val;
        *pB_V = V_val - ref_V_val;
    }
    else if (!YUV && RGB_val_available)
    {
        *pR_Y = R_val - ref_R_val;
        *pG_U = G_val - ref_G_val;
        *pB_V = B_val - ref_B_val;
    }
    else
    {
        return FALSE;
    }

    *pTotal = 0;
    *pTotal += ABSOLUTE_VALUE(*pR_Y);
    *pTotal += ABSOLUTE_VALUE(*pG_U);
    *pTotal += ABSOLUTE_VALUE(*pB_V);
    return TRUE;
}

// This method analyzed the overlay buffer to calculate average color
// in the zone defined by the color bar
BOOL CColorBar::CalcAvgColor(BOOL reinit, unsigned int nb_calc_needed, TDeinterlaceInfo* pInfo)
{
    int left, right, top, bottom, i, j, k;
    BYTE* pComponentVal[3];
    unsigned int nb_val[3];
    unsigned char min[3];
    unsigned char max[3];
    BYTE* buf;
    int overscan;
    int left_crop, total_crop;
    int width = pInfo->FrameWidth;
    int height = pInfo->FieldHeight;

    // Calculate the exact coordinates of rectangular zone in the buffer
//    overscan = SourceOverscan * width / (height * 2);
    overscan = 0;
    left_crop = ((LeftCropping * width) + 500) / 1000;
    total_crop = (((LeftCropping + RightCropping) * width) + 500) / 1000;
    left = (width + total_crop - 2 * overscan) * m_LeftBorder / 10000 - left_crop + overscan;
    if (left < 0)
    {
        left = 0;
    }
    else if (left >= width)
    {
        left = width - 1;
    }
    right = (width + total_crop - 2 * overscan) * m_RightBorder / 10000 - left_crop + overscan;
    if (right < 0)
    {
        right = 0;
    }
    else if (right >= width)
    {
        right = width - 1;
    }
//    overscan = SourceOverscan;
    overscan = 0;
    top = (height - overscan) * m_TopBorder / 10000 + overscan / 2;
    if (top < 0)
    {
        top = 0;
    }
    else if (top >= height)
    {
        top = height - 1;
    }
    bottom = (height - overscan) * m_BottomBorder / 10000 + overscan / 2;
    if (bottom < 0)
    {
        bottom = 0;
    }
    else if (bottom >= height)
    {
        bottom = height - 1;
    }

    LOG(5, _T("CalcAvgColor Zone %d %d %d %d"), left, right, top, bottom);

    // Sum separately Y, U and V in this rectangular zone
    // Each line is like this : YUYVYUYV...
    if (reinit)
    {
        cpt_nb = 0;
    }
    for (i = 0 ; i <= 2; i++)
    {
        if (reinit)
        {
            component_cpt[i] = 0;
        }
        min[i] = 255;
        max[i] = 0;
        nb_val[i] = 0;
    }
    for (i = top ; i <= bottom ; i++)
    {
        buf = pInfo->PictureHistory[0]->pData + (i * pInfo->InputPitch);
        for (j = (left*2) ; j <= (right*2) ; j+=2)
        {
            // Component Y
            k = 2;
            component_cpt[k] += buf[j];
            (nb_val[k])++;
            if (buf[j] < min[k])
            {
                min[k] = buf[j];
            }
            if (buf[j] > max[k])
            {
                max[k] = buf[j];
            }
            // Compnent U or V
            k = (j/2) % 2;
            component_cpt[k] += buf[j+1];
            (nb_val[k])++;
            if (buf[j+1] < min[k])
            {
                min[k] = buf[j+1];
            }
            if (buf[j+1] > max[k])
            {
                max[k] = buf[j+1];
            }
        }
    }
    cpt_nb++;
    for (i = 0 ; i <= 2; i++)
    {
        component_min[i] = min[i];
        component_max[i] = max[i];
    }
    min_available = TRUE;
    max_available = TRUE;

    LOG(5, _T("CalcAvgColor Min Y %d U %d V %d"), component_min[2], component_min[0], component_min[1]);
    LOG(5, _T("CalcAvgColor Max Y %d U %d V %d"), component_max[2], component_max[0], component_max[1]);

    if (cpt_nb >= nb_calc_needed)
    {
        YUV_val_available = FALSE;
        RGB_val_available = FALSE;

        pComponentVal[0] = &U_val;
        pComponentVal[1] = &V_val;
        pComponentVal[2] = &Y_val;

        for (i = 0 ; i <= 2 ; i++)
        {
            if (nb_val[i] > 0)
            {
                *(pComponentVal[i]) = AVERAGE_VALUE(component_cpt[i], cpt_nb * nb_val[i]);
            }
            else
            {
                *(pComponentVal[i]) = 0;
            }
            component_cpt[i] = 0;
        }
        cpt_nb = 0;

        YUV_val_available = TRUE;

        LOG(5, _T("CalcAvgColor YUV %d %d %d %d %d %d %d %d %d"), Y_val, U_val, V_val, left, right, top, bottom, height, width);

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL CColorBar::GetMinColor(BYTE* pY, BYTE* pU, BYTE* pV)
{
    if (min_available)
    {
        *pY = component_min[2];
        *pU = component_min[0];
        *pV = component_min[1];
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL CColorBar::GetMaxColor(BYTE* pY, BYTE* pU, BYTE* pV)
{
    if (max_available)
    {
        *pY = component_max[2];
        *pU = component_max[0];
        *pV = component_max[1];
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void CColorBar::Draw(BYTE* Buffer, int Pitch, int Height, int Width, int Overscan, int LCrop, int RCrop)
{
    int left, right, top, bottom;
    int overscan, left_crop, total_crop;
    BYTE* buf;
    int i, j, k, l;

    if (m_TypeDraw == DRAW_NO)
        return;

    overscan = Overscan * Width / (Height * 2);
    left_crop = ((LCrop * Width) + 500) / 1000;
    total_crop = (((LCrop + RCrop) * Width) + 500) / 1000;

    // Calculate the exact coordinates of rectangular zone in the buffer
    left = (Width + total_crop - 2 * overscan) * m_LeftBorder / 10000 - left_crop + overscan;
    if (left < 0)
    {
        left = 0;
    }
    else if (left >= Width)
    {
        left = Width - 1;
    }
    right = (Width + total_crop - 2 * overscan) * m_RightBorder / 10000 - left_crop + overscan;
    if (right < 0)
    {
        right = 0;
    }
    else if (right >= Width)
    {
        right = Width - 1;
    }
    overscan = Overscan;
    top = (Height - overscan) * m_TopBorder / 10000 + overscan / 2;
    if (top < 0)
    {
        top = 0;
    }
    else if (top >= Height)
    {
        top = Height - 1;
    }
    bottom = (Height - overscan) * m_BottomBorder / 10000 + overscan / 2;
    if (bottom < 0)
    {
        bottom = 0;
    }
    else if (bottom >= Height)
    {
        bottom = Height - 1;
    }

    switch (m_TypeDraw)
    {
    case DRAW_BORDER:
        for (i = top ; i <= bottom ; i++)
        {
            buf = Buffer + (i * Pitch);
            for (j = left; j <= right && j < (left + m_Param1Draw) ; j++)
            {
                buf[j*2  ] = ref_Y_val2;
                buf[j*2+1] = (j%2) ? ref_V_val2 : ref_U_val2;
            }
            for (j = right; j >= left && j > (right - m_Param1Draw) ; j--)
            {
                buf[j*2  ] = ref_Y_val2;
                buf[j*2+1] = (j%2) ? ref_V_val2 : ref_U_val2;
            }
        }
        for (i = left ; i <= right ; i++)
        {
            for (j = top; j <= bottom && j < (top + m_Param1Draw) ; j++)
            {
                buf = Buffer + (j * Pitch);
                buf[i*2  ] = ref_Y_val2;
                buf[i*2+1] = (i%2) ? ref_V_val2 : ref_U_val2;
            }
            for (j = bottom; j >= top && j > (bottom - m_Param1Draw) ; j--)
            {
                buf = Buffer + (j * Pitch);
                buf[i*2  ] = ref_Y_val2;
                buf[i*2+1] = (i%2) ? ref_V_val2 : ref_U_val2;
            }
        }
        break;

    case DRAW_LINEH:
        for (i = top ; i <= bottom ; i++)
        {
            k = (i - top) / m_Param1Draw;
            buf = Buffer + (i * Pitch);
            for (j = left ; j <= right ; j++)
            {
                if (k%2)
                {
                    buf[j*2  ] = ref_Y_val2;
                    buf[j*2+1] = (j%2) ? ref_V_val2 : ref_U_val2;
                }
                else
                {
                    buf[j*2  ] = ref_Y_val;
                    buf[j*2+1] = (j%2) ? ref_V_val : ref_U_val;
                }
            }
        }
        break;

    case DRAW_LINEV:
        for (i = top ; i <= bottom ; i++)
        {
            buf = Buffer + (i * Pitch);
            for (j = left ; j <= right ; j++)
            {
                k = (j - left) / m_Param1Draw;
                if (k%2)
                {
                    buf[j*2  ] = ref_Y_val2;
                    buf[j*2+1] = (j%2) ? ref_V_val2 : ref_U_val2;
                }
                else
                {
                    buf[j*2  ] = ref_Y_val;
                    buf[j*2+1] = (j%2) ? ref_V_val : ref_U_val;
                }
            }
        }
        break;

    case DRAW_LINEX:
        for (i = top ; i <= bottom ; i++)
        {
            buf = Buffer + (i * Pitch);
            for (j = left ; j <= right ; j++)
            {
                k = (i - top) / m_Param1Draw;
                l = (j - left) / m_Param1Draw;
                if (k%2 != l%2)
                {
                    buf[j*2  ] = ref_Y_val2;
                    buf[j*2+1] = (j%2) ? ref_V_val2 : ref_U_val2;
                }
                else
                {
                    buf[j*2  ] = ref_Y_val;
                    buf[j*2+1] = (j%2) ? ref_V_val : ref_U_val;
                }
            }
        }
        break;

    case DRAW_FILLED:
        for (i = top ; i <= bottom ; i++)
        {
            buf = Buffer + (i * Pitch);
            for (j = left ; j <= right ; j++)
            {
                buf[j*2  ] = ref_Y_val;
                buf[j*2+1] = (j % 2) ? ref_V_val : ref_U_val;
            }
        }
        break;

    case DRAW_GRADATIONH:
        for (i = top ; i <= bottom ; i++)
        {
            buf = Buffer + (i * Pitch);
            for (j = left ; j <= right ; j++)
            {
                buf[j*2  ] = (m_Param2Draw - m_Param1Draw) * (j - left) / (right - left) + m_Param1Draw;
                buf[j*2+1] = (j % 2) ? ref_V_val : ref_U_val;
            }
        }
        break;

    case DRAW_GRADATIONV:
        for (i = top ; i <= bottom ; i++)
        {
            buf = Buffer + (i * Pitch);
            for (j = left ; j <= right ; j++)
            {
                buf[j*2  ] = (m_Param2Draw - m_Param1Draw) * (i - top) / (bottom - top) + m_Param1Draw;
                buf[j*2+1] = (j % 2) ? ref_V_val : ref_U_val;
            }
        }
        break;

    default:
        break;
    }
}

// Convert RGB to YUV
void CColorBar::RGB2YUV(unsigned char R, unsigned char G, unsigned char B, BYTE* pY, BYTE* pU, BYTE* pV)
{
    unsigned int y, cr, cb;

    // Kr = 0.299
    // Kb = 0.114
    // L = Kr * R + Kb * B + (1 – Kr – Kb) * G
    // Y  = round(219 *  L / 255) + 16
    // Cb = round(224 * 0.5 * (B - L) / ((1 - Kb) * 255)) + 128
    // Cr = round(224 * 0.5 * (R - L) / ((1 - Kr) * 255)) + 128

    y  = ( ( 16829*R + 33039*G +  6416*B + 32768 ) >> 16 ) + 16;
    cb = ( ( -9714*R - 19071*G + 28784*B + 32768 ) >> 16 ) + 128;
    cr = ( ( 28784*R - 24103*G -  4681*B + 32768 ) >> 16 ) + 128;

    *pY = LIMIT_Y(y);
    *pU = LIMIT_CbCr(cb);
    *pV = LIMIT_CbCr(cr);
}

// Convert YUV to RGB
void CColorBar::YUV2RGB(unsigned char Y, unsigned char U, unsigned char V, BYTE* pR, BYTE* pG, BYTE* pB)
{
    int y, cr, cb, r, g, b;

    // R = clip( round( 1.164383 * (Y - 16)                        + 1.596027 * (V - 128) ) )
    // G = clip( round( 1.164383 * (Y - 16) - 0.391762 * (U - 128) - 0.812968 * (V - 128) ) )
    // B = clip( round( 1.164383 * (Y - 16) + 2.017232 * (U - 128)                        ) )

    y = Y - 16;
    cb = U - 128;
    cr = V - 128;

    r = ( 76309*y             + 104597*cr + 32768 ) >> 16;
    g = ( 76309*y -  25675*cb -  53279*cr + 32768 ) >> 16;
    b = ( 76309*y + 132201*cb             + 32768 ) >> 16;

    *pR = LIMIT_RGB(r);
    *pG = LIMIT_RGB(g);
    *pB = LIMIT_RGB(b);
}


/////////////////////////////////////////////////////////////////////////////
// Class CSubPattern

CSubPattern::CSubPattern(eTypeAdjust type)
{
    m_TypeAdjust = type;
}

CSubPattern::~CSubPattern()
{
    m_ColorBars.clear();
}

// This method returns the type of settings that can be adjusted with this sub-pattern
eTypeAdjust CSubPattern::GetTypeAdjust()
{
    return m_TypeAdjust;
}

// This method analyzes the current overlay buffer
BOOL CSubPattern::CalcCurrentSubPattern(BOOL reinit, unsigned int nb_calc_needed, TDeinterlaceInfo* pInfo)
{
    BOOL result_avail;

    // Do the job for each defined color bar
    for(vector<CColorBar*>::iterator it = m_ColorBars.begin();
        it != m_ColorBars.end();
        ++it)
    {
        result_avail = (*it)->CalcAvgColor(reinit, nb_calc_needed, pInfo);
    }

    return result_avail;
}

// This methode returns the sum of absolute delta between reference color
// and calculated average color through all the color bars
void CSubPattern::GetSumDeltaColor(BOOL YUV, int* pR_Y, int* pG_U, int* pB_V, int* pTotal)
{
    int delta[4];
    int sum_delta[4];

    // Set the sums to 0
    int j;
    for (j = 0 ; j <= 3 ; j++)
    {
        sum_delta[j] = 0;
    }
    // Go through all the color bars
    for(vector<CColorBar*>::iterator it = m_ColorBars.begin();
        it != m_ColorBars.end();
        ++it)
    {
        if ((*it)->GetDeltaColor(YUV, &delta[0], &delta[1], &delta[2], &delta[3]) == TRUE)
        {
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

// This method draws in the video signal rectangles around each color bar of the sub-pattern
void CSubPattern::Draw(TDeinterlaceInfo* pInfo)
{
    // Do the job for each defined color bar
    for(vector<CColorBar*>::iterator it = m_ColorBars.begin();
        it != m_ColorBars.end();
        ++it)
    {
        (*it)->Draw(pInfo->PictureHistory[0]->pData, pInfo->InputPitch, pInfo->FieldHeight, pInfo->FrameWidth, 0, LeftCropping, RightCropping);
    }
}

/////////////////////////////////////////////////////////////////////////////
// Class CTestPattern

CTestPattern::CTestPattern(const tstring& name, int width, int height)
{
    m_PatternName = name;
    m_Width = width;
    m_Height = height;
}

CTestPattern::CTestPattern(const tstring& FileName)
{
    FILE* FilePat;
    CColorBar* color_bar;
    CSubPattern* sub_pattern;
    TCHAR BufferLine[512];
    TCHAR* Buffer;
    int i_val[16];
    TCHAR s_val[64];
    TCHAR s_val2[64];
    int n;
    eTypeAdjust TypeAdjust;
    BOOL YUV;
    eTypeDraw TypeDraw;

    FilePat = _tfopen(FileName.c_str(), _T("r"));
    if (!FilePat)
    {
        m_Width = 0;
        m_Height = 0;
        return;
    }

    m_Width = Setting_GetValue(WM_STILL_GETVALUE, PATTERNWIDTH);
    // The width must be even
    if (m_Width%2)
    {
        m_Width--;
    }
    m_Height = Setting_GetValue(WM_STILL_GETVALUE, PATTERNHEIGTH);

    while(!feof(FilePat))
    {
        if(_fgetts(BufferLine, 512, FilePat))
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
            while(_tcslen(Buffer) > 0 && Buffer[_tcslen(Buffer) - 1] <= ' ')
            {
                Buffer[_tcslen(Buffer) - 1] = '\0';
            }
            if (_tcslen(Buffer) == 0)
            {
                continue;
            }
            if (_stscanf(Buffer, _T("PAT %s"), s_val) == 1)
            {
                m_PatternName = _tcsstr(&Buffer[4], s_val);
                LOG(5,_T("PAT %s"), m_PatternName.c_str());
            }
            else if (_stscanf(Buffer, _T("SIZE %d %d"), &i_val[0], &i_val[1]) == 2)
            {
                m_Width = i_val[0];
                m_Height = i_val[1];
                LOG(5,_T("SIZE %d %d"), m_Width, m_Height);
            }
            else if ((n = _stscanf(Buffer, _T("RECT %d %d %d %d %s %d %d %d %d %d %d %s %d %d"), &i_val[0], &i_val[1], &i_val[2], &i_val[3], s_val, &i_val[4], &i_val[5], &i_val[6], &i_val[7], &i_val[8], &i_val[9], s_val2, &i_val[10])) >= 12)
            {
                if (!_tcscmp(s_val, _T("RGB")))
                {
                    YUV = FALSE;
                }
                else if (!_tcscmp(s_val, _T("YUV")))
                {
                    YUV = TRUE;
                }
                else
                {
                    continue;
                }
                if (!_tcscmp(s_val2, _T("BORDER")))
                {
                    TypeDraw = DRAW_BORDER;
                }
                else if (!_tcscmp(s_val2, _T("LINEH")))
                {
                    TypeDraw = DRAW_LINEH;
                }
                else if (!_tcscmp(s_val2, _T("LINEV")))
                {
                    TypeDraw = DRAW_LINEV;
                }
                else if (!_tcscmp(s_val2, _T("LINEX")))
                {
                    TypeDraw = DRAW_LINEX;
                }
                else
                {
                    continue;
                }
                if (n == 12)
                {
                    i_val[10] = 1;
                }
                LOG(5,_T("RECT %s (%d) %s %d %d %d %d %d %d %d %d %d %d"), s_val2, i_val[10], YUV?_T("YUV"):_T("RGB"), i_val[0], i_val[1], i_val[2], i_val[3], i_val[4], i_val[5], i_val[6], i_val[7], i_val[8], i_val[9]);
                color_bar = new CColorBar(i_val[0], i_val[1], i_val[2], i_val[3], TypeDraw, i_val[10], 0, YUV, i_val[4], i_val[5], i_val[6], i_val[7], i_val[8], i_val[9]);
                m_ColorBars.push_back(color_bar);
            }
            else if ((n = _stscanf(Buffer, _T("RECT %d %d %d %d %s %d %d %d %s %d %d"), &i_val[0], &i_val[1], &i_val[2], &i_val[3], s_val, &i_val[4], &i_val[5], &i_val[6], s_val2, &i_val[7], &i_val[8])) >= 9)
            {
                if (!_tcscmp(s_val, _T("RGB")))
                {
                    YUV = FALSE;
                }
                else if (!_tcscmp(s_val, _T("YUV")))
                {
                    YUV = TRUE;
                }
                else
                {
                    continue;
                }
                if (!_tcscmp(s_val2, _T("FILLED")) && n == 9)
                {
                    TypeDraw = DRAW_FILLED;
                    i_val[7] = 0;
                    i_val[8] = 0;
                }
                else if (!_tcscmp(s_val2, _T("BORDER")) && n == 10)
                {
                    TypeDraw = DRAW_BORDER;
                    i_val[8] = 0;
                }
                else if (!_tcscmp(s_val2, _T("BORDER")) && n == 9)
                {
                    TypeDraw = DRAW_BORDER;
                    i_val[7] = 1;
                    i_val[8] = 0;
                }
                else if (!_tcscmp(s_val2, _T("GRADH")) && n == 11)
                {
                    TypeDraw = DRAW_GRADATIONH;
                }
                else if (!_tcscmp(s_val2, _T("GRADV")) && n == 11)
                {
                    TypeDraw = DRAW_GRADATIONV;
                }
                else
                {
                    continue;
                }
                LOG(5,_T("RECT %s (%d) %s %d %d %d %d %d %d %d"), s_val2, i_val[7], YUV?_T("YUV"):_T("RGB"), i_val[0], i_val[1], i_val[2], i_val[3], i_val[4], i_val[5], i_val[6]);
                color_bar = new CColorBar(i_val[0], i_val[1], i_val[2], i_val[3], TypeDraw, i_val[7], i_val[8], YUV, i_val[4], i_val[5], i_val[6], i_val[4], i_val[5], i_val[6]);
                m_ColorBars.push_back(color_bar);
            }
            else if ((n = _stscanf(Buffer, _T("GRP %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d"), s_val, &i_val[0], &i_val[1], &i_val[2], &i_val[3], &i_val[4], &i_val[5], &i_val[6], &i_val[7], &i_val[8], &i_val[9], &i_val[10], &i_val[11], &i_val[12], &i_val[13], &i_val[14], &i_val[15])) >= 2)
            {
                LOG(5,_T("GRP %s"), s_val);
                if (!_tcscmp(s_val, _T("BRIGHTNESS_CONTRAST")))
                {
                    TypeAdjust = ADJ_BRIGHTNESS_CONTRAST;
                }
                else if (!_tcscmp(s_val, _T("BRIGHTNESS")))
                {
                    TypeAdjust = ADJ_BRIGHTNESS;
                }
                else if (!_tcscmp(s_val, _T("CONTRAST")))
                {
                    TypeAdjust = ADJ_CONTRAST;
                }
                else if (!_tcscmp(s_val, _T("COLOR")))
                {
                    TypeAdjust = ADJ_COLOR;
                }
                else if (!_tcscmp(s_val, _T("SATURATION_U")))
                {
                    TypeAdjust = ADJ_SATURATION_U;
                }
                else if (!_tcscmp(s_val, _T("SATURATION_V")))
                {
                    TypeAdjust = ADJ_SATURATION_V;
                }
                else if (!_tcscmp(s_val, _T("HUE")))
                {
                    TypeAdjust = ADJ_HUE;
                }
                else
                {
                    TypeAdjust = ADJ_MANUAL   ;
                }
                if (TypeAdjust != ADJ_MANUAL)
                {
                    sub_pattern = new CSubPattern(TypeAdjust);
                    m_SubPatterns.push_back(sub_pattern);
                    for (int i(0) ; i < (n-1) ; i++)
                    {
                        if (i_val[i] > 0 && i_val[i] <= m_ColorBars.size())
                        {
                            LOG(5,_T("GRP %d"), i_val[i]);
                            sub_pattern->m_ColorBars.push_back(m_ColorBars[i_val[i]-1]);
                        }
                    }
                }
            }
        }
    }

    fclose(FilePat);

    CreateGlobalSubPattern();

    Log();
}

CTestPattern::~CTestPattern()
{
    for(vector<CSubPattern*>::iterator it = m_SubPatterns.begin();
        it != m_SubPatterns.end();
        ++it)
    {
        delete *it;
    }
    m_SubPatterns.clear();
    for(vector<CColorBar*>::iterator it2 = m_ColorBars.begin();
        it2 != m_ColorBars.end();
        ++it2)
    {
        delete *it2;
    }
    m_ColorBars.clear();
}

// This method returns the name of the test pattern
tstring CTestPattern::GetName()
{
    return m_PatternName;
}

// This method returns the width of the test pattern
int CTestPattern::GetWidth()
{
    return m_Width;
}

// This method returns the height (number of lines) of the test pattern
int CTestPattern::GetHeight()
{
    return m_Height;
}

void CTestPattern::SetSize(int width, int height)
{
    m_Width = width;
    m_Height = height;
}

// This method allows to create a new sub-pattern to the test pattern
// which is a merge of all the others sub-patterns
// Returns 0 if the sub-pattern is correctly created
void CTestPattern::CreateGlobalSubPattern()
{
    CSubPattern* sub_pattern;

    // Create the new sub-pattern
    sub_pattern = new CSubPattern(ADJ_MANUAL);

    for(vector<CColorBar*>::iterator it = m_ColorBars.begin();
        it != m_ColorBars.end();
        ++it)
    {
        sub_pattern->m_ColorBars.push_back(*it);
    }
    m_SubPatterns.push_back(sub_pattern);
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
CSubPattern* CTestPattern::GetSubPattern(eTypeAdjust type_adjust)
{
    for(vector<CSubPattern*>::iterator it = m_SubPatterns.begin();
        it != m_SubPatterns.end();
        ++it)
    {
        if ((*it)->GetTypeAdjust() == type_adjust)
        {
            return *it;
        }
    }
    return NULL;
}

void CTestPattern::Draw(BYTE* Buffer, int Pitch)
{
    // Do the job for each defined color bar
    for(vector<CColorBar*>::iterator it = m_ColorBars.begin();
        it != m_ColorBars.end();
        ++it)
    {
        (*it)->Draw(Buffer, Pitch, m_Height, m_Width, 0, 0, 0);
    }
}

void CTestPattern::Log()
{
    unsigned char R, G, B, Y, U, V;
    unsigned short int left, right, top, bottom;

    LOG(3, _T("Pattern %s %dx%d"), m_PatternName.c_str(), m_Width, m_Height);
    for(vector<CColorBar*>::iterator it2 = m_ColorBars.begin();
        it2 != m_ColorBars.end();
        ++it2)
    {
        (*it2)->GetPosition(&left, &right, &top, &bottom);
        (*it2)->GetRefColor(FALSE, &R, &G, &B);
        (*it2)->GetRefColor(TRUE, &Y, &U, &V);
        LOG(3, _T("   T %4d B %4d L %4d R %4d - RGB %3d %3d %3d YUV %3d %3d %3d"), top, bottom, left, right, R, G, B, Y, U, V);
    }
    for(vector<CSubPattern*>::iterator it = m_SubPatterns.begin();
        it != m_SubPatterns.end();
        ++it)
    {
        LOG(3, _T("   Sub-pattern %d"), (*it)->GetTypeAdjust());
        for(vector<CColorBar*>::iterator it2 = (*it)->m_ColorBars.begin();
            it2 != (*it)->m_ColorBars.end();
            ++it2)
        {
            (*it2)->GetPosition(&left, &right, &top, &bottom);
            LOG(3, _T("      T %4d B %4d L %4d R %4d"), top, bottom, left, right);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

CPatternHelper::CPatternHelper(CStillSource* pParent) :
    CStillSourceHelper(pParent)
{
}

BOOL CPatternHelper::OpenMediaFile(const tstring& FileName)
{
    CTestPattern pattern(FileName);
    BYTE* pFrameBuf;
    BYTE* pStartFrame;
    int LinePitch;

    if ((pattern.GetWidth() * pattern.GetHeight()) == 0)
    {
        return FALSE;
    }

    // Allocate memory buffer to store the YUYV values
    LinePitch = (pattern.GetWidth() * 2 * sizeof(BYTE) + 15) & 0xfffffff0;
    pFrameBuf = (BYTE*)malloc(LinePitch * pattern.GetHeight() + 16);
    if (pFrameBuf == NULL)
    {
        return FALSE;
    }
    pStartFrame = START_ALIGNED16(pFrameBuf);

    // Set the background of the pattern to black
    for (int i=0 ; i<pattern.GetHeight() ; i++)
    {
        for (int j=0 ; j<pattern.GetWidth() ; j++)
        {
            *(pStartFrame + i * LinePitch + j * 2    ) = 16;
            *(pStartFrame + i * LinePitch + j * 2 + 1) = 128;
        }
    }

    pattern.Draw(pStartFrame, LinePitch);

    m_pParent->FreeOriginalFrameBuffer();
    m_pParent->m_OriginalFrameBuffer = pFrameBuf;
    m_pParent->m_OriginalFrame.pData = pStartFrame;
    m_pParent->m_LinePitch = LinePitch;
    m_pParent->m_InitialHeight = pattern.GetHeight();
    m_pParent->m_InitialWidth = pattern.GetWidth();
    m_pParent->m_Height = pattern.GetHeight();
    m_pParent->m_Width = pattern.GetWidth();
    m_pParent->m_SquarePixels = FALSE;

    return TRUE;
}

void CPatternHelper::SaveSnapshot(const tstring& FilePath, int Height, int Width, BYTE* pOverlay, LONG OverlayPitch, const tstring& Context)
{
    return;
}
