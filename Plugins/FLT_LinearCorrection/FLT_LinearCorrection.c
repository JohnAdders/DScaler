/////////////////////////////////////////////////////////////////////////////
// $Id: FLT_LinearCorrection.c,v 1.14 2001-11-21 15:21:41 adcockj Exp $
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.13  2001/09/11 18:07:05  adcockj
// Fix to prevent crashing with extream values
//
// Revision 1.12  2001/09/11 14:33:48  adcockj
// Fixes to allow parameters to be updated as you change them
//
// Revision 1.11  2001/08/30 12:04:37  adcockj
// Replaced numbers with defines in mask mode setting
//
// Revision 1.10  2001/08/30 11:58:10  adcockj
// Changed mode setting to be select fom list
//
// Revision 1.9  2001/07/13 16:13:33  adcockj
// Added CVS tags and removed tabs
//
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DS_Filter.h"
#include "math.h"

#define MAX_HEIGHT  576
#define MAX_WIDTH   768

#define Y_BLACK     16
#define UV_BLACK    128

#define MASK_TRAPEZOID 0
#define MASK_COMPLEX 1
#define MASK_STRETCH 2

LPCSTR ModeList[] =
{
    "Trapeziod",
    "Complex",
    "Non Linear Stretch",
};

FILTER_METHOD LinearCorrMethod;

typedef struct _BlendStruct
{
    long    pixel1Y;    // The first pixel to use to calculate luminance
    long    pixel2Y;    // The second pixel to use to calculate luminance
    long    pixel1UV;   // The first pixel to use to calculate color
    long    pixel2UV;   // The second pixel to use to calculate color
    long    coef1;      // Coef to apply to pixel1
                        // double value between 0 and 1 multiplied by 1000 to use integer
    long    coef2;      // Coef to apply to pixel2
                        // double value between 0 and 1 multiplied by 1000 to use integer
} BlendStruct;

int PictureWidth = -1;
int PictureHeight = -1;
int NbPixelsPerLineTab[MAX_HEIGHT];
BlendStruct LinearFilterTab[MAX_WIDTH+1][MAX_WIDTH];
BYTE TmpBuf[MAX_WIDTH*2];

BOOL DoOnlyMasking = FALSE;
int MaskType = 0;
int MaskParam1 = 0;
int MaskParam2 = 0;
int MaskParam3 = 0;
int MaskParam4 = 0;

void UpdStretchTables(int Width)
{
    int i, j;
    double Start, End;
    double pixel, pixel_before, pixel_after;
    short pixel1Y, pixel2Y, pixel1UV, pixel2UV;
    long Boundaries[3];
    double StretchFactors[3] = {192.0, 144.0, 120.0};
    double x;

    Boundaries[0] = 0;
    Boundaries[1] = (int)((double)Width * StretchFactors[0] / (144.0 * 2.0) / 5.0);
    Boundaries[2] = (int)(((double)Width * StretchFactors[0] / (144.0 * 2.0) / 5.0) +
                        ((double)Width * StretchFactors[1] / 144.0 / 5.0));

    i=Width;
    Start = (double)(Width - i) / 2.0;
    End = Start + i - 1;
    
    for (j=0 ; j<Width ; j++)
    {
        x = (double)(j) / (double)Width;
        pixel = (double)(-2 * x * x * x / 3.0 + x * x + 2.0 / 3.0 * x) * (double)Width;
        pixel_before = floor(pixel);
        pixel_after = ceil(pixel);

        pixel1Y = (short)pixel_before;
        pixel2Y = (short)pixel_after;
        LinearFilterTab[Width][j].pixel1Y = pixel1Y * 2;
        LinearFilterTab[Width][j].pixel2Y = pixel2Y * 2;

        if (pixel1Y == pixel2Y)
        {
            if ((j % 2) == (pixel1Y % 2))
            {
                pixel1UV = pixel1Y;
            }
            else if ((j % 2) == 0)
            {
                pixel1UV = pixel1Y - 1;
            }
            else
            {
                pixel1UV = pixel1Y + 1;
            }
            pixel2UV = pixel1UV;
        }
        else
        {
            if ((j % 2) == (pixel1Y % 2))
            {
                if ((j % 2) == 0)
                {
                    pixel1UV = pixel1Y;
                    pixel2UV = pixel1Y;
                }
                else
                {
                    pixel1UV = pixel1Y;
                    pixel2UV = pixel2Y + 1;
                }
            }
            else if ((j % 2) == (pixel2Y % 2))
            {
                if ((j % 2) == 0)
                {
                    pixel1UV = pixel1Y - 1;
                    pixel2UV = pixel2Y;
                }
                else
                {
                    pixel1UV = pixel2Y;
                    pixel2UV = pixel2Y;
                }
            }
        }
        LinearFilterTab[Width][j].pixel1UV = pixel1UV * 2 + 1;
        LinearFilterTab[Width][j].pixel2UV = pixel2UV * 2 + 1;

        if (pixel_before < pixel_after)
        {
            LinearFilterTab[Width][j].coef1 = (short)ceil((pixel_after - pixel) * 1024.0 - 0.5);
            LinearFilterTab[Width][j].coef2 = (short)ceil((pixel - pixel_before) * 1024.0 - 0.5);
        }
        else
        {
            LinearFilterTab[Width][j].coef1 = 512;  // 0.5 * 1024
            LinearFilterTab[Width][j].coef2 = 512;  // 0.5 * 1024
        }
    }
}

void UpdLinearFilterTables(int Width)
{
    int i, j;
    double Start, End;
    double pixel, pixel_before, pixel_after;
    short pixel1Y, pixel2Y, pixel1UV, pixel2UV;

    if(MaskType == MASK_STRETCH)
    {
        UpdStretchTables(Width);
        return;
    }

    for (i=0 ; i<=Width ; i++)
    {
        Start = (double)(Width - i) / 2.0;
        End = Start + i - 1;
        for (j=0 ; j<Width ; j++)
        {
            if ((i == 0) || ((double)j < Start) || ((double)j > End))
            {
                LinearFilterTab[i][j].pixel1Y = -1;
                LinearFilterTab[i][j].pixel2Y = -1;
                LinearFilterTab[i][j].pixel1UV = -1;
                LinearFilterTab[i][j].pixel2UV = -1;
                LinearFilterTab[i][j].coef1 = 0;
                LinearFilterTab[i][j].coef2 = 0;
            }
            else
            {
                if (i > 1)
                {
                    pixel = ((double)j - Start) / (double)i * (double)Width;
                }
                else
                {
                    pixel = (double)(Width - 1) / 2.0;
                }
                pixel_before = floor(pixel);
                pixel_after = ceil(pixel);

                pixel1Y = (short)pixel_before;
                pixel2Y = (short)pixel_after;
                LinearFilterTab[i][j].pixel1Y = pixel1Y * 2;
                LinearFilterTab[i][j].pixel2Y = pixel2Y * 2;
    
                if (pixel1Y == pixel2Y)
                {
                    if ((j % 2) == (pixel1Y % 2))
                    {
                        pixel1UV = pixel1Y;
                    }
                    else if ((j % 2) == 0)
                    {
                        pixel1UV = pixel1Y - 1;
                    }
                    else
                    {
                        pixel1UV = pixel1Y + 1;
                    }
                    pixel2UV = pixel1UV;
                }
                else
                {
                    if ((j % 2) == (pixel1Y % 2))
                    {
                        if ((j % 2) == 0)
                        {
                            pixel1UV = pixel1Y;
                            pixel2UV = pixel1Y;
                        }
                        else
                        {
                            pixel1UV = pixel1Y;
                            pixel2UV = pixel2Y + 1;
                        }
                    }
                    else if ((j % 2) == (pixel2Y % 2))
                    {
                        if ((j % 2) == 0)
                        {
                            pixel1UV = pixel1Y - 1;
                            pixel2UV = pixel2Y;
                        }
                        else
                        {
                            pixel1UV = pixel2Y;
                            pixel2UV = pixel2Y;
                        }
                    }
                }
                LinearFilterTab[i][j].pixel1UV = pixel1UV * 2 + 1;
                LinearFilterTab[i][j].pixel2UV = pixel2UV * 2 + 1;

                if (pixel_before < pixel_after)
                {
                    LinearFilterTab[i][j].coef1 = (short)ceil((pixel_after - pixel) * 1024.0 - 0.5);
                    LinearFilterTab[i][j].coef2 = (short)ceil((pixel - pixel_before) * 1024.0 - 0.5);
                }
                else
                {
                    LinearFilterTab[i][j].coef1 = 512;  // 0.5 * 1024
                    LinearFilterTab[i][j].coef2 = 512;  // 0.5 * 1024
                }
            }
        }
    }
}

void UpdNbPixelsPerLineTable(int Height, int Width)
{
    int x;
    double val1, val2, val3, val4;
    double a, b;
    double val;

    switch (MaskType)
    {
    case MASK_TRAPEZOID: // Trapezoid
        val1 = (double)((100 - 2 * MaskParam1) * Width) / 100.0;
        val2 = (double)((100 - 2 * MaskParam2) * Width) / 100.0;
        b = val1;
        a = (val2 - b) / (double)(Height - 1);
        for (x=0 ; x<Height ; x++)
        {
            val = a * x + b;
            NbPixelsPerLineTab[x] = (int)ceil(val - 0.5);
            if(NbPixelsPerLineTab[x] < 0)
            {
                NbPixelsPerLineTab[x] = 0;
            }
        }
        break;

    case MASK_COMPLEX:
        val1 = (double)((100 - 2 * MaskParam1) * Width) / 100.0;
        val2 = (double)((100 - 2 * MaskParam2) * Width) / 100.0;
        val3 = (double)((100 - 2 * MaskParam3) * Width) / 100.0;
        val4 = ceil((double)((Height - 1) * MaskParam4) / 100.0 - 0.5);
        b = val1;
        a = (val2 - b) / val4;
        for (x=0 ; x<(int)val4 ; x++)
        {
            val = a * x + b;
            NbPixelsPerLineTab[x] = (int)ceil(val - 0.5);
            if(NbPixelsPerLineTab[x] < 0)
            {
                NbPixelsPerLineTab[x] = 0;
            }
        }
        a = (val3 - val2) / ((double)(Height - 1) - val4);
        b = val3 - a * (double)(Height - 1);
        for (x=(int)val4 ; x<Height ; x++)
        {
            val = a * x + b;
            NbPixelsPerLineTab[x] = (int)ceil(val - 0.5);
            if(NbPixelsPerLineTab[x] < 0)
            {
                NbPixelsPerLineTab[x] = 0;
            }
        }
        break;

    default:
        for (x=0 ; x<Height ; x++)
        {
            NbPixelsPerLineTab[x] = Width;
        }
        break;
    }
}

void ApplyLinearFilter(BYTE* pLine, int NewWidth, MEMCPY_FUNC *pCopy)
{
    int i;
    BYTE *t;
    BlendStruct *tab = LinearFilterTab[NewWidth];

    if (DoOnlyMasking)
    {
        t = (BYTE*)pLine;
        for (i=0 ; i<PictureWidth ; i++,t+=2,tab++)
        {
            if (tab->pixel1Y == -1)
            {
                // Color the pixel in black
                t[0] = Y_BLACK;
                t[1] = UV_BLACK;
            }
        }
    }
    else
    {
        DWORD OldSI;
        DWORD OldSP;

        // Build the new line
        t = TmpBuf;

        ///////////////////////////////////////////////////////////////////////////////////////////// 
        // the assember code below implements the following code
        // it doesn't seem to optimize it much
        ///////////////////////////////////////////////////////////////////////////////////////////// 
        // for (i=0 ; i<PictureWidth ; i++,t+=2,tab++)
        // {
        //   if (tab->pixel1Y == -1)
        //   {
        //      // Color the pixel in black
        //      t[0] = Y_BLACK;
        //      t[1] = UV_BLACK;
        //   }
        //   else
        //   {
        //      t[0] = (pLine[tab->pixel1Y] * tab->coef1 + pLine[tab->pixel2Y] * tab->coef2) / 1024;
        //      t[1] = (pLine[tab->pixel1UV] * tab->coef1 + pLine[tab->pixel2UV] * tab->coef2) / 1024;
        //   }
        // }
        // 
        // // Replace the old line by the new line
        // pCopy(pLine, TmpBuf, PictureWidth*2);
        
        _asm
        {
            mov OldSI, esi
            mov OldSP, esp
            mov ecx, PictureWidth
            mov edi, pLine
            mov esi, t
            mov esp, tab
FILTER_LOOP:
            xor eax, eax
            xor ebx, ebx
            mov edx, [esp]
            cmp edx, -1
            je BLACKOUT
            mov bl, byte ptr[edi + edx]
            imul ebx, [esp + 16]
            mov edx, [esp + 4]
            mov al, byte ptr[edi + edx]
            imul eax, dword ptr[esp + 20]
            add eax, ebx
            shr eax, 10
            mov byte ptr[esi], al
            inc esi

            xor eax, eax
            xor ebx, ebx
            mov edx, [esp + 8]
            mov bl, byte ptr[edi + edx]
            imul ebx, [esp + 16]
            mov edx, [esp + 12]
            mov al, byte ptr[edi + edx]
            imul eax, [esp + 20]
            add eax, ebx
            shr eax, 10
            mov byte ptr[esi], al
            inc esi

            add esp, 24
            loop FILTER_LOOP
            jmp ENDPROCESS
BLACKOUT:
            mov byte ptr[esi], Y_BLACK
            inc esi
            mov byte ptr[esi], UV_BLACK
            inc esi
            add esp, 24
            loop FILTER_LOOP
ENDPROCESS:
            mov ecx, PictureWidth
            shl ecx, 1
            mov esi, t
            mov edi, pLine
            rep movs

            mov esi, OldSI
            mov esp, OldSP
        }
    }
}

BOOL LinearCorrection(TDeinterlaceInfo* pInfo)
{
    BYTE* Pixels = pInfo->PictureHistory[0]->pData;
    int i;

    if (Pixels == NULL )
    {
        return 1000;
    }

    // If there is a change concerning the height or the width of the picture
    if ((pInfo->FrameWidth != PictureWidth) || (pInfo->FrameHeight != PictureHeight))
    {
        // Verify that the filter can manage this size of picture
        if ((pInfo->FrameWidth > MAX_WIDTH) || (pInfo->FrameHeight > MAX_HEIGHT))
        {
            return 1000;
        }

        // Update the internal tables of the filter
        UpdNbPixelsPerLineTable(pInfo->FrameHeight, pInfo->FrameWidth);
        UpdLinearFilterTables(pInfo->FrameWidth);
        PictureWidth = pInfo->FrameWidth;
        PictureHeight = pInfo->FrameHeight;
    }

    // Update each line of the picture
    for (i = 1 ; i < PictureHeight ; i += 2)
    {
        if (NbPixelsPerLineTab[i] != PictureWidth || MaskType == MASK_STRETCH)
        {
            ApplyLinearFilter(Pixels, NbPixelsPerLineTab[i], pInfo->pMemcpy);
        }
        Pixels += pInfo->InputPitch;
    }
    if(MaskType == MASK_STRETCH)
    {
        return 1333;
    }
    else
    {
        return 1000;
    }
}

void LinearCorrStart(void)
{
    // Update the internal tables of the filter
    UpdNbPixelsPerLineTable(576, 720);
    UpdLinearFilterTables(720);
    PictureWidth = 720;
    PictureHeight = 576;
}

BOOL MaskParam1_OnChange(long NewValue)
{
    MaskParam1 = NewValue;
    UpdNbPixelsPerLineTable(PictureHeight, PictureWidth);
    UpdLinearFilterTables(PictureWidth);
    return FALSE;
}

BOOL MaskParam2_OnChange(long NewValue)
{
    MaskParam2 = NewValue;
    UpdNbPixelsPerLineTable(PictureHeight, PictureWidth);
    UpdLinearFilterTables(PictureWidth);
    return FALSE;
}

BOOL MaskParam3_OnChange(long NewValue)
{
    MaskParam3 = NewValue;
    UpdNbPixelsPerLineTable(PictureHeight, PictureWidth);
    UpdLinearFilterTables(PictureWidth);
    return FALSE;
}

BOOL MaskParam4_OnChange(long NewValue)
{
    MaskParam4 = NewValue;
    UpdNbPixelsPerLineTable(PictureHeight, PictureWidth);
    UpdLinearFilterTables(PictureWidth);
    return FALSE;
}

BOOL MaskType_OnChange(long NewValue)
{
    MaskType = NewValue;
    UpdNbPixelsPerLineTable(PictureHeight, PictureWidth);
    UpdLinearFilterTables(PictureWidth);
    return FALSE;
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING FLT_LinearCorrSettings[FLT_LINEAR_CORR_SETTING_LASTONE] =
{
    {
        "Linear Correction Filter", ONOFF, 0, &(LinearCorrMethod.bActive),
        FALSE, 0, 1, 1, 1,
        NULL,
        "LinearCorrectionFilter", "UseLinearCorrFilter", NULL,
    },
    {
        "Do Only Masking", ONOFF, 0, &DoOnlyMasking,
        FALSE, 0, 1, 1, 1,
        NULL,
        "LinearCorrectionFilter", "DoOnlyMasking", NULL,
    },
    {
        "Mask Type", ITEMFROMLIST, 0, &MaskType,
        MASK_TRAPEZOID, MASK_TRAPEZOID, MASK_STRETCH, 1, 1,
        ModeList,
        "LinearCorrectionFilter", "MaskType", MaskType_OnChange,
    },
    {
        "Mask Parameter 1", SLIDER, 0, &MaskParam1,
        0, 0, 100, 1, 1,
        NULL,
        "LinearCorrectionFilter", "MaskParam1", MaskParam1_OnChange,
    },
    {
        "Mask Parameter 2", SLIDER, 0, &MaskParam2,
        0, 0, 100, 1, 1,
        NULL,
        "LinearCorrectionFilter", "MaskParam2", MaskParam2_OnChange,
    },
    {
        "Mask Parameter 3", SLIDER, 0, &MaskParam3,
        0, 0, 100, 1, 1,
        NULL,
        "LinearCorrectionFilter", "MaskParam3", MaskParam3_OnChange,
    },
    {
        "Mask Parameter 4", SLIDER, 0, &MaskParam4,
        0, 0, 100, 1, 1,
        NULL,
        "LinearCorrectionFilter", "MaskParam4", MaskParam4_OnChange,
    },
};

FILTER_METHOD LinearCorrMethod =
{
    sizeof(FILTER_METHOD),
    FILTER_CURRENT_VERSION,
    "Linear Correction Filter",
    "&Linear Correction (experimental)",
    FALSE,
    TRUE,
    LinearCorrection, 
    0,
    TRUE,
    LinearCorrStart,
    NULL,
    NULL,
    FLT_LINEAR_CORR_SETTING_LASTONE,
    FLT_LinearCorrSettings,
    WM_FLT_LINEAR_CORR_GETVALUE - WM_USER,
};


__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo(long CpuFeatureFlags)
{
    return &LinearCorrMethod;
}
