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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.2  2002/02/16 00:24:57  lindsey
// Added CVS tags
//
// Revision 1.1.1.1  2002/02/15  lindsey
// Added Histogram Filter
//
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


/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////

// This might need to be tuned for other processors.
// Then again, it doesn't seem to matter much what I set it to on my Athlon
// so long as it's between 64 and about 300

#define PREFETCH_STRIDE                         128

// Colors used in the histogram

#define Y_DOT_COLOR                             0x7FFF7FFF
#define U_DOT_COLOR                             0xFFFF7FFF
#define V_DOT_COLOR                             0x7FFFFFFF

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
} eMODE;


/////////////////////////////////////////////////////////////////////////////
// Function prototypes
/////////////////////////////////////////////////////////////////////////////

__declspec(dllexport)   FILTER_METHOD* GetFilterPluginInfo( long CpuFeatureFlags );

void                    ShowHistogram( DWORD* pHistogram, TDeinterlaceInfo* pInfo, DWORD Color );
LONG __cdecl            DispatchHistogram( TDeinterlaceInfo* pInfo );

LONG                    GatherHistogram_SSE( TDeinterlaceInfo* pInfo );
LONG                    GatherHistogram_3DNOW( TDeinterlaceInfo* pInfo );
LONG                    GatherHistogram_MMX( TDeinterlaceInfo* pInfo );


/////////////////////////////////////////////////////////////////////////////
// Globals
/////////////////////////////////////////////////////////////////////////////

FILTER_METHOD HistogramMethod;


// The histograms

DWORD   gpYHistogram[256];
DWORD   gpUHistogram[256];
DWORD   gpVHistogram[256];


// Which of the histograms should be displayed?

eMODE   gMode = MODE_Y;

// Should we average two scanlines to reduce color crosstalk artifacts?

BOOLEAN gUseComb = TRUE;


// Store CPU type for use in the dispatcher routine

LONG    gCpuFeatureFlags = 0;


/////////////////////////////////////////////////////////////////////////////
// Settings
/////////////////////////////////////////////////////////////////////////////

LPCSTR ModeList[] =
{
    "Brightness (Y)",
    "U (Red) Color",
    "V (Blue) Color",
    "Colors",
    "All pixel components"
};


SETTING FLT_HistogramSettings[FLT_HISTOGRAM_SETTING_LASTONE] =
{
    {
        "Display Mode", ITEMFROMLIST, 0, (LONG*)&gMode,
        MODE_Y, MODE_Y, MODE_LASTONE -1, 1, 1,
        ModeList,
        "HistogramFilter", "DisplayMode", NULL,
    },
    {
        "Use Histogram Filter", ONOFF, 0, &(HistogramMethod.bActive),
        FALSE, 0, 1, 1, 1,
        NULL,
        "HistogramFilter", "UseHistogramFilter", NULL,
    },
    {
        "Vertical Averaging", ONOFF, 0, (LONG*)&gUseComb,
        TRUE, 0, 1, 1, 1,
        NULL,
        "HistogramFilter", "UseCombFilter", NULL,
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
    WM_FLT_HISTOGRAM_GETVALUE - WM_USER,
    TRUE,
    1,
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


    if( (gMode == MODE_Y) || (gMode == MODE_YUV) )
    {
        ShowHistogram(gpYHistogram, pInfo, Y_DOT_COLOR);
    }
    if( (gMode == MODE_U) || (gMode == MODE_UV) || (gMode == MODE_YUV) )
    {
        ShowHistogram(gpUHistogram, pInfo, U_DOT_COLOR);
    }
    if( (gMode == MODE_V) || (gMode == MODE_UV) || (gMode == MODE_YUV) )
    {
        ShowHistogram(gpVHistogram, pInfo, V_DOT_COLOR);
    }

    return FilterResult;
}


// Show just one of the color component histograms

void ShowHistogram( DWORD* pHistogram, TDeinterlaceInfo* pInfo, DWORD Color )
{
    DWORD       Index = 0;
    DWORD       LeftOffset = 0;

    if( pInfo->FrameWidth > 512 )
    {
        LeftOffset = ((pInfo->FrameWidth - 512)/4);
    }

    for( ; (Index < 256) && (Index * 2 < (DWORD) pInfo->FrameWidth); ++Index )
    {
        DOUBLE      ScaledValue = 0.0;
        DWORD       DWordScaledValue = 0;
        DWORD       BottomLine = pInfo->FieldHeight - MARGIN;
        
        ScaledValue = HISTOGRAM_SCALE*log(1.0 + pHistogram[Index]);
        DWordScaledValue = (DWORD) (ScaledValue + 1.00001);
        if( DWordScaledValue < (DWORD) (pInfo->FieldHeight - MARGIN*2) )
        {
            *(DWORD*)(pInfo->PictureHistory[0]->pData + (BottomLine - DWordScaledValue) * pInfo->InputPitch + (Index+LeftOffset)*4) = Color;
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