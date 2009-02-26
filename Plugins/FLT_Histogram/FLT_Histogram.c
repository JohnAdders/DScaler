/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Lindsey Dubb.  All rights reserved.
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

/*
Some are red.  And some are blue.
Some are old.  And some are new.

Some are sad.

And some are glad.

And some are very, very bad.

Why are they sad and glad and bad?
I do not know.  Go ask your dad.

                           -- Dr. Seuss

Histogram filter.  This diagnostic filter counts the frequency of pixels with
a certain Y, U, or V value, and reports the results in an on screen (logarithmic)
graph.

There isn't a whole lot to it.  It could definitely be more efficient,
but I don't consider efficiency to be a very high priority, here.

Probably the most confusing bit is the "comb filter" option.  By default,
the average of two vertical values are used instead of the values from the
original image.  This causes inaccuracy near vertical boundaries, but it
also removes color crosstalk artifacts.  Color crosstalk will otherwise
artifactually increase the range of the distribution. 

*/

#include <math.h>

#include "windows.h"
#include "DS_Filter.h"
#include "..\help\helpids.h"


/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////

// This might need to be tuned for other processors.
// Then again, it doesn't seem to matter much what I set it to on my Athlon
// so long as it's between 64 and about 300

#define PREFETCH_STRIDE                         128

// Colors used in the histogram

#define Y_DOT_COLOR                             0x7FFF7FFF
#define Y_DOT_DIMMED                            0x7F7F7F7F
#define Y_CCIR_601_MIN                          16
#define Y_CCIR_601_MAX                          235

#define U_DOT_COLOR                             0x7FFFFFFF
#define U_DOT_DIMMED                            0x7F7FFF7F
#define V_DOT_COLOR                             0xFFFF7FFF
#define V_DOT_DIMMED                            0xFF7F7F7F
#define UV_CCIR_601_MIN                         16
#define UV_CCIR_601_MAX                         240

// Vertical scale of the histogram

#define HISTOGRAM_SCALE                         16.0

// Vertical space above and below the histogram

#define MARGIN                                  20

// Well, I could have done this with three checkboxes, but this corresponds more closely
// to the way I expect the filter will be used.  Besides, it prevents the silly situation
// of having the filter enabled but nothing displayed.

typedef enum
{
    MODE_Y,
    MODE_U,
    MODE_V,
    MODE_UV,
    MODE_YUV,
    MODE_LASTONE,
} eColorMode;


// Likewise this could have been done with a checkbox, but this is clearer.

typedef enum
{
    COMB_MODE_NTSC,
    COMB_MODE_PAL,
    COMB_MODE_NONE,
    VIDEO_MODE_LASTONE,
} eVideoMode;


/////////////////////////////////////////////////////////////////////////////
// Function prototypes
/////////////////////////////////////////////////////////////////////////////

__declspec(dllexport)   FILTER_METHOD* GetFilterPluginInfo( long CpuFeatureFlags );

void ShowHistogram( DWORD* pHistogram, TDeinterlaceInfo* pInfo, DWORD Color, DWORD EndColor, DWORD LowValue, DWORD HighValue );
LONG __cdecl            DispatchHistogram( TDeinterlaceInfo* pInfo );

LONG                    GatherHistogram_SSE_PREFETCH( TDeinterlaceInfo* pInfo );
LONG                    GatherHistogram_3DNOW_PREFETCH( TDeinterlaceInfo* pInfo );
LONG                    GatherHistogram_SSE( TDeinterlaceInfo* pInfo );
LONG                    GatherHistogram_3DNOW( TDeinterlaceInfo* pInfo );
LONG                    GatherHistogram_MMX( TDeinterlaceInfo* pInfo );


/////////////////////////////////////////////////////////////////////////////
// Globals
/////////////////////////////////////////////////////////////////////////////

FILTER_METHOD HistogramMethod;

long        gUsePrefetching = TRUE;


// The histograms

DWORD       gpYHistogram[256];
DWORD       gpUHistogram[256];
DWORD       gpVHistogram[256];


// Which of the histograms should be displayed?

eColorMode  gColorMode = MODE_Y;

// Should we average two scanlines to reduce color crosstalk artifacts?

LONG        gCombMode = COMB_MODE_NTSC;


// Store CPU type for use in the dispatcher routine

LONG        gCpuFeatureFlags = 0;


/////////////////////////////////////////////////////////////////////////////
// Settings
/////////////////////////////////////////////////////////////////////////////

LPCSTR ModeList[] =
{
    "Brightness (Y)",
    "U (Blue) Color",
    "V (Red) Color",
    "Colors",
    "All pixel components"
};

LPCSTR CombModeList[] =
{
    "Composite NTSC",
    "Composite PAL",
    "Disable"
};


SETTING FLT_HistogramSettings[FLT_HISTOGRAM_SETTING_LASTONE] =
{
    {
        "Display Mode", ITEMFROMLIST, 0, (LONG*)&gColorMode,
        MODE_Y, 0, MODE_LASTONE - 1, 1, 1,
        ModeList,
        "HistogramFilter", "DisplayMode", NULL,
    },
    {
        "Fast Memory Access", ONOFF, 0, &gUsePrefetching,
        TRUE, 0, 1, 1, 1,
        NULL,
        "HistogramFilter", "UsePrefetching", NULL,
    },
    {
        "Histogram Filter", ONOFF, 0, &(HistogramMethod.bActive),
        FALSE, 0, 1, 1, 1,
        NULL,
        "HistogramFilter", "UseHistogramFilter", NULL,
    },
    {
        "Color Correction", ITEMFROMLIST, 0, (LONG*)&gCombMode,
        COMB_MODE_NTSC, 0, VIDEO_MODE_LASTONE - 1, 1, 1,
        CombModeList,
        "HistogramFilter", "CombFilterMode", NULL,
    },
};


FILTER_METHOD HistogramMethod =
{
    sizeof(FILTER_METHOD),
    FILTER_CURRENT_VERSION,
    DEINTERLACE_INFO_CURRENT_VERSION,
    "Histogram Filter",
    "Histogram",
    FALSE,
    TRUE,
    DispatchHistogram, 
    0,
    TRUE,
    NULL,
    NULL,
    NULL,
    FLT_HISTOGRAM_SETTING_LASTONE,
    FLT_HistogramSettings,
    WM_FLT_HISTOGRAM_GETVALUE - WM_APP,
    TRUE,
    1,
    IDH_HISTOGRAM,
};


/////////////////////////////////////////////////////////////////////////////
// Main code
/////////////////////////////////////////////////////////////////////////////

// The histogram gathering routine is included from FLT_Histogram.asm to allow
// some processor specific optimizations

#define IS_SSE
#include "FLT_Histogram.asm"
#undef IS_SSE

#define IS_3DNOW
#include "FLT_Histogram.asm"
#undef IS_3DNOW

#define IS_MMX
#include "FLT_Histogram.asm"
#undef IS_MMX


// Save the CPU type for use in the dispatcher routine

__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo( long CpuFeatureFlags )
{
    gCpuFeatureFlags = CpuFeatureFlags;
    return &HistogramMethod;
}


BOOL WINAPI DllMain( HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved )
{
    return TRUE;
}


// Traffic organizer -- Run the appropriate version of the data
// gatherer, then call the appropriate display routines.

LONG __cdecl DispatchHistogram( TDeinterlaceInfo* pInfo )
{
    DWORD       Index = 0;
    LONG        FilterResult = 0;
    

    if( pInfo->PictureHistory[0] == NULL )
    {
        return 1000;
    }

    // Clear histograms
    for( Index = 0; Index < 256; ++Index )
    {
        gpYHistogram[Index] = 0;
        gpUHistogram[Index] = 0;
        gpVHistogram[Index] = 0;
    }

    if( gUsePrefetching )
    {
        if( (gCpuFeatureFlags & FEATURE_SSE) || (gCpuFeatureFlags & FEATURE_MMXEXT) )
        {
            FilterResult = GatherHistogram_SSE(pInfo);
        }
        else if( gCpuFeatureFlags & FEATURE_3DNOW )
        {
            FilterResult = GatherHistogram_3DNOW(pInfo);
        }
        else  // MMX version
        {
            FilterResult = GatherHistogram_MMX(pInfo);
        }
    }
    else
    {
        // 3DNOW and SSE are just used for prefetching, so switch to the MMX
        // version if prefetching is disabled.

        FilterResult = GatherHistogram_MMX(pInfo);
    }


    if( (gColorMode == MODE_Y) || (gColorMode == MODE_YUV) )
    {
        ShowHistogram(gpYHistogram, pInfo, Y_DOT_COLOR, Y_DOT_DIMMED, Y_CCIR_601_MIN, Y_CCIR_601_MAX);
    }
    if( (gColorMode == MODE_U) || (gColorMode == MODE_UV) || (gColorMode == MODE_YUV) )
    {
        ShowHistogram(gpUHistogram, pInfo, U_DOT_COLOR, U_DOT_DIMMED, UV_CCIR_601_MIN,UV_CCIR_601_MAX);
    }
    if( (gColorMode == MODE_V) || (gColorMode == MODE_UV) || (gColorMode == MODE_YUV) )
    {
        ShowHistogram(gpVHistogram, pInfo, V_DOT_COLOR, V_DOT_DIMMED, UV_CCIR_601_MIN, UV_CCIR_601_MAX);
    }

    return FilterResult;
}


// Show just one of the color component histograms

void ShowHistogram( DWORD* pHistogram, TDeinterlaceInfo* pInfo, DWORD Color, DWORD EndColor, DWORD LowValue, DWORD HighValue )
{
    DWORD       Index = 0;
    DWORD       LeftOffset = 0;
    DWORD       ThisColor = 0;

    if( pInfo->FrameWidth > 512 )
    {
        LeftOffset = ((pInfo->FrameWidth - 512)/4);
    }

    for( ; (Index < 256) && (Index * 2 < (DWORD) pInfo->FrameWidth); ++Index )
    {
        DOUBLE          ScaledValue = 0.0;
        DWORD           DWordScaledValue = 0;
        const DWORD     BottomLine = pInfo->FieldHeight - MARGIN;
        
        ScaledValue = HISTOGRAM_SCALE*log(1.0 + pHistogram[Index]);
        DWordScaledValue = (DWORD) (ScaledValue + 1.00001);
        if( (Index < LowValue) || (Index > HighValue) )
        {
            ThisColor = EndColor;
        }
        else
        {
            ThisColor = Color;
        }
        if( DWordScaledValue < (DWORD) (pInfo->FieldHeight - MARGIN*2) )
        {
            *(DWORD*)(pInfo->PictureHistory[0]->pData + (BottomLine - DWordScaledValue) * pInfo->InputPitch + (Index+LeftOffset)*4) = ThisColor;
        }
        else
        {
            *(DWORD*)(pInfo->PictureHistory[0]->pData + (MARGIN * pInfo->InputPitch) + (Index+LeftOffset)*4) = Color;
        }

        // Show dots every other pixel to make measurement a little easier.
        // Yeah, this step is done three times if you're showing all three color components
        *(WORD*)(pInfo->PictureHistory[0]->pData + (BottomLine * pInfo->InputPitch) + (Index+LeftOffset)*4) = 0x00FF;
    }

    return;
}