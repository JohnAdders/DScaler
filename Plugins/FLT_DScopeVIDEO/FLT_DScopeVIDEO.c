/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2003 Michael Joubert   All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
/////////////////////////////////////////////////////////////////////////////

//to do
/*

- add frame counter and flip
- separate window (full)
- separate window (controls)
- use vbi data direct
- absolute line numbering
- write line to file
- file dialog
- try new (smaller) line number font
- add shaded plot background
- move all calcs out of main loop (Update Parameters)
- verify operation for other screen resolutions
- complete and test osd

- test ->IsFirstInSeries, get working correctly.
  current changes look ok, but is sometimes
  missed when 'hurry when late' is active(expected), and other times...

  also when one channel is say 320 pixels width and you change to a channel of
  say 720 pixels width, the filter sometimes misses the update
  (with 'hurry when late' OFF)

-  DShow needs work

- right justify line marker text

*/

#include "windows.h"
#include "..\..\Api\DS_Filter.h"
#include "..\..\Api\DS_Control.h"
#include "..\..\Api\DS_ApiCommon.h"
#include "..\..\DScalerRes\resource.h"
#include "..\help\helpids.h"
#include <math.h>
#include "DSV_DrawString.h"

//----------------------------------------------------------------------------
// Function prototypes

__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo      (long CpuFeatureFlags);
static                int            DisplayScope             (TDeinterlaceInfo* pInfo);
static                int            LineNumber_OnChange      (long NewValue);
static                BOOL           PlotIntensityInvert_OnChange (long NewValue);
static                int            Y1_BaseLine_OnChange     (long NewValue);
static                int            Y_Scale_OnChange         (long NewValue);
static                BOOL           UseGrid_OnChange         (long NewValue);
static                int            GridColor_OnChange       (long NewValue);
static                int            GridIntensity_OnChange   (long NewValue);
static                BOOL           UseLineMarker_OnChange   (long NewValue);
static                BOOL           UsePictureFill_OnChange  (long NewValue);
static                BOOL           UseSolidBackground_OnChange(long NewValue);
static                FILTER_METHOD  DScopeVideoMethod;       //see DS_Filter.h
static                int            DisplayScope             (TDeinterlaceInfo* pInfo);
static                void           DScopeOSDText            (char* strbuff);
static                BOOL           DScopeVideoEnabled_OnChange(long NewValue);
static                void           DScopeVideoStart         (void);
static                void           DScopeVideoStop          (void);
static                void           DrawGrid                 (DWORD* pDestination, int pitch, int height, int width);
static                void           DrawLineMarker           (BYTE* source, int source_pitch, BYTE* pDestination, int dest_pitch, int dest_frame_height, int width);
static                void           DScopeUpdateParameters   (void);

//----------------------------------------------------------------------------
// Filter globals

static                FILTER_METHOD DScopeVideoMethod;       //see DS_Filter.h
static                BOOL          UseDScopeVideo;          //
static                double        X_Scale;                 //
static                double        Y_Scale;                 //
static                int           Y_ScalePercent;          //
static                BOOL          bGrid;                   // make available to other functions
static                int           GridColor;               //
static                BOOL          bLineMarker;             //
static                BOOL          bPictureFill;            //
static                BOOL          bSolidBackground;        //
static                BOOL          bPlotIntensityInvert;    //
static                int           VideoLine;               //
static                int           VideoLineDS;             //
static                int           FieldLineDS;             //   precalc, saves some time
static                int           InputLine_Offset;        //       "      "     "    "
static                DWORD         InputPitch;              //       "      "     "    "
static                int           GridIntensity;           //
static                DWORD         dwGrid_Color;            //
static                BYTE          LineBuffer [2048];       // use buffer, for future experiments
static                BYTE          LineBuffer2[2048];       //
static                BOOL          EVEN;                    //
static                BOOL          ODD;                     // for readability...
static                DWORD         LineMarkerStart;         //
static                DWORD         LineMarkerEnd;           //
static                DWORD         LineMarkerTextStart;     // text start position


static                int           tempGridColor;           //
static                BYTE*         pGrid_Temp;              //
static                DWORD         dwGrid_Color;            //
static                char          strbuff[80];             // for debug text output

static                int           Y1_BaseLine;             //

static                double        temp;                    //
static                int           a,b,c;                   //
static                int           OSD_SIZE;                //
static                HWND          hPrevWindow;             //
static                ATOM          FLT_DSCOPE_VIDEO;        //

static                BYTE*         pOverlay;                //
static                DWORD*        pFrameWidth;             // Number of pixels in each scanline.
static                DWORD*        pLineLength;             //
static                DWORD*        pInputPitch;             //
static                DWORD*        pOverlayPitch;           //

static                LONG          Source_left;              // RECT info
static                LONG          Source_top;               //
static                DWORD*        pSource_right;            //
static                LONG          Source_bottom;            //

static                BOOL          bHasRunOnce = FALSE;      //

static    BOOL test1;

//----------------------------------------------------------------------------

FILTER_METHOD DScopeVideoMethod;

long FilterDScopeVIDEO(TDeinterlaceInfo* pInfo)
{

    // make sure we have data
    if ( pInfo->PictureHistory[0] == NULL )
    {
        return (1000);
    }

    DisplayScope(pInfo);
    return 1000;
}

//----------------------------------------------------------------------------
// Start of Settings related code

SETTING FLT_DScopeVideo_SETTINGS[FLT_DSV_SETTING_LASTONE] =
{
    {
        "Enable",                       // char* szDisplayName;
        ONOFF,                          // SETTING_TYPE Type;
        0,                              // long  LastSavedValue;
        &(DScopeVideoMethod.bActive),   // long* pValue;
        FALSE,                          // long  Default;
        0,                              // long  MinValue;
        1,                              // long  MaxValue;
        1,                              // long  StepValue;
        1,                              // long  OSDDivider;
        NULL,                           // const char** pszList;
        "DScope Video",                 // char* szIniSection;
        "USEDSVIDEO",                   // char* szIniEntry;
        DScopeVideoEnabled_OnChange,    // SETTING_ONCHANGE* pfnOnChange;
    },
    {
        "Line",                         // char* szDisplayName;
        SLIDER,                         // SETTING_TYPE Type;
        0,                              // long  LastSavedValue;
        &VideoLine,                     // long* pValue;
        10,                             // long  Default;
        1,                              // long  MinValue;
        576,                            // long  MaxValue;
        1,                              // long  StepValue;
        1,                              // long  OSDDivider;
        NULL,                           // const char** pszList;
        "DScope Video",                 // char* szIniSection;
        "VideoLine",                    // char* szIniEntry;
        LineNumber_OnChange,            // SETTING_ONCHANGE* pfnOnChange;
    },
    {
        "Plot Intensity Invert",        // char* szDisplayName;
        ONOFF,                          // SETTING_TYPE Type;
        0,                              // long  LastSavedValue;
        &bPlotIntensityInvert,          // long* pValue;
        0,                              // long  Default;
        0,                              // long  MinValue;
        1,                              // long  MaxValue;
        1,                              // long  StepValue;
        1,                              // long  OSDDivider;
        NULL,                           // const char** pszList;
        "DScope Video",                 // char* szIniSection;
        "PlotIntensityInvert",          // char* szIniEntry;
        PlotIntensityInvert_OnChange,   // SETTING_ONCHANGE* pfnOnChange;
    },
    {
        "Plot Y Scale",                 // char* szDisplayName;
        SLIDER,                         // SETTING_TYPE Type;
        0,                              // long  LastSavedValue;
        &Y_ScalePercent,                // long* pValue;
        30,                             // long  Default;
        5,                              // long  MinValue;
        100,                            // long  MaxValue;
        1,                              // long  StepValue;
        1,                              // long  OSDDivider;
        NULL,                           // const char** pszList;
        "DScope Video",                 // char* szIniSection;
        "Y_Scale",                      // char* szIniEntry;
        Y_Scale_OnChange,               // SETTING_ONCHANGE* pfnOnChange;
    },
    {
        "Plot Y Position",              // char* szDisplayName;
        SLIDER,                         // SETTING_TYPE Type;
        0,                              // long  LastSavedValue;
        &Y1_BaseLine,                   // long* pValue;
        150,                            // long  Default;
        50,                             // long  MinValue;
        576,                            // long  MaxValue;
        1,                              // long  StepValue;
        1,                              // long  OSDDivider;
        NULL,                           // const char** pszList;
        "DScope Video",                 // char* szIniSection;
        "Y1_BaseLine",                  // char* szIniEntry;
        Y1_BaseLine_OnChange,           // SETTING_ONCHANGE* pfnOnChange;
    },
    {
        "Grid",                         // char* szDisplayName;
        ONOFF,                          // SETTING_TYPE Type;
        0,                              // long  LastSavedValue;
        &bGrid,                         // long* pValue;
        FALSE,                          // long  Default;
        0,                              // long  MinValue;
        1,                              // long  MaxValue;
        1,                              // long  StepValue;
        1,                              // long  OSDDivider;
        NULL,                           // const char** pszList;
        "DScope Video",                 // char* szIniSection;
        "UseGrid",                      // char* szIniEntry;
        UseGrid_OnChange,               // SETTING_ONCHANGE* pfnOnChange;
    },
    {
        "Grid Colour",                  // char* szDisplayName;
        SLIDER,                         // SETTING_TYPE Type;
        0,                              // long  LastSavedValue;
        &GridColor,                     // long* pValue;
        30,                             // long  Default;
        0,                              // long  MinValue;
        0xFF,                           // long  MaxValue;
        1,                              // long  StepValue;
        1,                              // long  OSDDivider;
        NULL,                           // const char** pszList;
        "DScope Video",                 // char* szIniSection;
        "GridColor",                    // char* szIniEntry;
        GridColor_OnChange,             // SETTING_ONCHANGE* pfnOnChange;
    },
    {
        "Grid Intensity",               // char* szDisplayName;
        SLIDER,                         // SETTING_TYPE Type;
        0,                              // long  LastSavedValue;
        &GridIntensity,                 // long* pValue;
        50,                             // long  Default;
        0,                              // long  MinValue;
        0xFF,                           // long  MaxValue;
        1,                              // long  StepValue;
        1,                              // long  OSDDivider;
        NULL,                           // const char** pszList;
        "DScope Video",                 // char* szIniSection;
        "GridIntensity",                // char* szIniEntry;
        GridIntensity_OnChange,         // SETTING_ONCHANGE* pfnOnChange;
    },
    {
        "Solid Background",             // char* szDisplayName;
        ONOFF,                          // SETTING_TYPE Type;
        0,                              // long  LastSavedValue;
        &bSolidBackground,              // long* pValue;
        FALSE,                          // long  Default;
        0,                              // long  MinValue;
        1,                              // long  MaxValue;
        1,                              // long  StepValue;
        1,                              // long  OSDDivider;
        NULL,                           // const char** pszList;
        "DScope Video",                 // char* szIniSection;
        "UseSolidBackground",           // char* szIniEntry;
        UseSolidBackground_OnChange,    // SETTING_ONCHANGE* pfnOnChange;
    },
    {
        "Line Marker",                  // char* szDisplayName;
        ONOFF,                          // SETTING_TYPE Type;
        0,                              // long  LastSavedValue;
        &bLineMarker,                   // long* pValue;
        FALSE,                          // long  Default;
        0,                              // long  MinValue;
        1,                              // long  MaxValue;
        1,                              // long  StepValue;
        1,                              // long  OSDDivider;
        NULL,                           // const char** pszList;
        "DScope Video",                 // char* szIniSection;
        "UseLineMarker",                // char* szIniEntry;
        UseLineMarker_OnChange,         // SETTING_ONCHANGE* pfnOnChange;
    },
    {
        "Fill Picture from line (view VBI, VTI, use with v-delay)", // char* szDisplayName;
        ONOFF,                          // SETTING_TYPE Type;
        0,                              // long  LastSavedValue;
        &bPictureFill,                  // long* pValue;
        FALSE,                          // long  Default;
        0,                              // long  MinValue;
        1,                              // long  MaxValue;
        1,                              // long  StepValue;
        1,                              // long  OSDDivider;
        NULL,                           // const char** pszList;
        "DScope Video",                 // char* szIniSection;
        "UsePictureFill",               // char* szIniEntry;
        UsePictureFill_OnChange,        // SETTING_ONCHANGE* pfnOnChange;
    }

};

FILTER_METHOD DScopeVideoMethod =
{
    sizeof(FILTER_METHOD),              // sizeof(FILTER_METHOD)
    FILTER_CURRENT_VERSION,             // set to FILTER_CURRENT_VERSION
    DEINTERLACE_INFO_CURRENT_VERSION,   // Set to DEINTERLACE_INFO_CURRENT_VERSION
    "DScope Video",                     // What to display when selected
    "&DScope Video",                    // What to put in the Menu (NULL to use szName)
    FALSE,                              // Are we active Initially
    FALSE, //TRUE,                      // Do we get called on Input. !*!*! when true causes crashes!!!
    FilterDScopeVIDEO,                  // Pointer to Algorithm function (cannot be NULL)
    0,                                  // id of menu to display status
    FALSE,//TRUE,                          // Always run - do we run if there has been an overrun
    DScopeVideoStart,                   // call this if plugin needs to do anything before it is used
    DScopeVideoStop,                    // call this if plugin needs to deallocate anything
    NULL,                               // Used to save the module Handle
    FLT_DSV_SETTING_LASTONE,            // number of settings
    FLT_DScopeVideo_SETTINGS,           // pointer to start of Settings[nSettings]
    WM_FLT_DSV_GETVALUE - WM_APP,       // the offset used by the external settings API
    TRUE,                               // can we cope with interlaced material, only checked for input filters, output filters are assumed not to care
    2,                                  // How many pictures of history do we need to run
    IDH_DSCOPE_VIDEO,                   // Help ID, needs to be in \help\helpids.h
};

//----------------------------------------------------------------------------
int LineNumber_OnChange(long NewValue)
{
    VideoLine = NewValue;
    DScopeUpdateParameters();
    return FALSE;
}
//----------------------------------------------------------------------------
int Y_Scale_OnChange(long NewValue)
{
    Y_ScalePercent = (int) NewValue;
    DScopeUpdateParameters();
    return FALSE;
}
//----------------------------------------------------------------------------
int Y1_BaseLine_OnChange(long NewValue)
{
    Y1_BaseLine = NewValue;
    return FALSE;
}
//----------------------------------------------------------------------------
BOOL PlotIntensityInvert_OnChange(long NewValue)
{
    bPlotIntensityInvert = NewValue;
    return FALSE;
}
//----------------------------------------------------------------------------
BOOL UseLineMarker_OnChange(long NewValue)
{
    bLineMarker = NewValue;
    return FALSE;
}
//----------------------------------------------------------------------------
BOOL UseGrid_OnChange(long NewValue)
{
    bGrid = NewValue;
    return FALSE;
}
//----------------------------------------------------------------------------
int GridColor_OnChange(long NewValue)      // crude colour change..
{
    GridColor   = NewValue;
    pGrid_Temp  = (BYTE*) &dwGrid_Color;   // mj fix here, create startup defaults
    pGrid_Temp  = pGrid_Temp + 1;
    *pGrid_Temp = (BYTE) GridColor;
    pGrid_Temp  = pGrid_Temp + 2;
    *pGrid_Temp = (BYTE) GridColor;
    return FALSE;
}
//----------------------------------------------------------------------------
int GridIntensity_OnChange(long NewValue)
{
    GridIntensity = NewValue;
    pGrid_Temp  = (BYTE*) &dwGrid_Color;    // mj fix here, create startup defaults
    *pGrid_Temp = (BYTE) GridIntensity;
    pGrid_Temp  = pGrid_Temp + 2;
    *pGrid_Temp = (BYTE) GridIntensity;
    return FALSE;
}
//----------------------------------------------------------------------------
BOOL UsePictureFill_OnChange(long NewValue)
{
    bPictureFill = NewValue;
    return FALSE;
}
//----------------------------------------------------------------------------
BOOL UseSolidBackground_OnChange(long NewValue)
{
    bSolidBackground = NewValue;
    return FALSE;
}

/*----------------------------------------------------------------------------

    Display Scope main routine

*/

int DisplayScope(TDeinterlaceInfo* pInfo)
{
    BYTE* pLineSource;
    BYTE* pLineDestination;
    BYTE  Pixel_Y;
    DWORD LineSegmentStart = 0;
    DWORD LineSegmentEnd = 2048;
    DWORD LineSegmentLength = LineSegmentEnd - LineSegmentStart;
    DWORD i;
    int   Y1_PlotValue;
    int   X1_PlotValue;

    BYTE* CurrentOddLine;
    BYTE* CurrentEvenLine;


    if(!bHasRunOnce)  // seems to be triggered when station is changed (and filter is enabled)
    {
        // get local copy of pointers for use globally, within filter
        pOverlay      = pInfo->Overlay;
        pInputPitch   = &pInfo->InputPitch;
        pFrameWidth   = &pInfo->FrameWidth;
        pLineLength   = &pInfo->LineLength;
        pOverlayPitch = &pInfo->OverlayPitch;
        pSource_right = &pInfo->SourceRect.right;

        DScopeUpdateParameters();
        bHasRunOnce = TRUE;
    }


    //if(Info.PictureHistory[HistoryRequired - 1]->IsFirstInSeries == TRUE)  (ref:J.A.)
    // still some work needed here...
    if(pInfo->PictureHistory[1]->IsFirstInSeries == TRUE)
    {
        pOverlay      = pInfo->Overlay;
        pInputPitch   = &pInfo->InputPitch;
        pFrameWidth   = &pInfo->FrameWidth;
        pLineLength   = &pInfo->LineLength;
        pOverlayPitch = &pInfo->OverlayPitch;
        pSource_right = &pInfo->SourceRect.right;

        DScopeUpdateParameters();

#ifdef _DEBUG
        OutputDebugString   ( "IsFirstInSeries = TRUE (in DScope filter)" );
#endif
    }



    if (pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD)  // ref:DI_Weave.c
    {
        CurrentOddLine  = pInfo->PictureHistory[0]->pData;
        CurrentEvenLine = pInfo->PictureHistory[1]->pData;
    }
    else
    {
        CurrentOddLine  = pInfo->PictureHistory[1]->pData;
        CurrentEvenLine = pInfo->PictureHistory[0]->pData;
    }

    // TV to DScaler format = subtract 1, then divide by 2 (discard remainder)... mj fix to match absolute...

    if(VideoLine & 1)   // get correct source, from latest history...
    {
        pLineSource = CurrentOddLine  + InputLine_Offset;
    }
    else
    {
        pLineSource = CurrentEvenLine + InputLine_Offset;
    }

    pInfo->pMemcpy(&LineBuffer[0], pLineSource, pInfo->LineLength);

    if(bPictureFill)
    {
        for (i = 0; i < (DWORD) pInfo->FrameHeight; i = i++)
        {
            pLineDestination = pInfo->Overlay + (i * pInfo->OverlayPitch);
            pInfo->pMemcpy(pLineDestination, &LineBuffer[0], pInfo->LineLength);// copy line
        }
    }
    _asm
    {
        emms
    }

    // draw Y grid-lines
    if (bGrid)               // check start position zero mj ************
    {
        DrawGrid((DWORD*) pInfo->Overlay, pInfo->OverlayPitch, pInfo->FrameHeight, pInfo->FrameWidth);
    }


    // draw plot data
    for (i = LineSegmentStart; i < pInfo->LineLength; i = i + 2) // do every second byte, 'Y' only
    {

        // scale X value to fit screen width
        X1_PlotValue = (int)((i-LineSegmentStart) * X_Scale);

        // align 'Y' (YUYV) values, skipping color bytes
        X1_PlotValue = (X1_PlotValue * 2);

        // prevent line over-run...
        if (X1_PlotValue > ((int)pInfo->LineLength))
            continue;

        // calc and scale data values to y axis
        Y1_PlotValue = Y1_BaseLine - (int) (LineBuffer[i] * Y_Scale);

        // use inverted luminance ('Y') to draw plot?
        if (bPlotIntensityInvert)
        {
            // invert 'Y'
            Pixel_Y = 255 - *(BYTE*) (pInfo->Overlay + (Y1_PlotValue * pInfo->OverlayPitch) + X1_PlotValue);

            if(Pixel_Y > 127)
            {
                Pixel_Y = 255;          // could be improved..
            }
            else
            {
                Pixel_Y = 0;
            }
        }
        else
        {
            Pixel_Y = 255;
        }

        // tidy here MJ @@
        if ((pInfo->Overlay + (Y1_PlotValue * pInfo->OverlayPitch) + X1_PlotValue) > pInfo->Overlay )  // check for writing before overlay area
        {  // the above check can be improved !! MJ **
            //*(BYTE*)( pInfo->PictureHistory[0]->pData + (Y1_PlotValue * pInfo->InputPitch) + X1_PlotValue) = Pixel_Y;  // set Y, finally!
            *(BYTE*)( pOverlay + (Y1_PlotValue * *pOverlayPitch) + X1_PlotValue) = Pixel_Y;  // set Y, finally!
        }

    }


    // draw line marker
    if (bLineMarker)
    {
        DrawLineMarker( pInfo->Overlay,             //
                        pInfo->OverlayPitch,        //
                        pInfo->Overlay,             //
                        pInfo->OverlayPitch,        //
                        pInfo->FrameHeight,         //
                        pInfo->FrameWidth );        //
    }

    return 0;
}


/*----------------------------------------------------------------------------



*/
__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo(long CpuFeatureFlags)
{
    return &DScopeVideoMethod;
}


/*----------------------------------------------------------------------------



*/
void DScopeOSDText (char* strbuff)
{
    // SendMessage:
    // Messages sent between threads are processed only when the receiving thread executes
    // message retrieval code. The sending thread is blocked until the receiving thread
    // processes the message.

    if ((FLT_DSCOPE_VIDEO = GlobalAddAtom( strbuff )) != 0)
    {
        OSD_SIZE = SendMessage(hPrevWindow, WM_OSD_GETVALUE, OSD_PERCENTAGESIZE, OSD_SIZE );
        SendMessage(hPrevWindow, WM_OSD_SETVALUE, OSD_ANTIALIAS, TRUE );
        SendMessage(hPrevWindow, WM_OSD_SETVALUE, OSD_PERCENTAGESIZE, 4 );
        SendMessage(hPrevWindow, WM_COMMAND, IDM_SET_OSD_TEXT, (LPARAM) FLT_DSCOPE_VIDEO );

        // error here mj, check *********
        SendMessage(hPrevWindow, WM_OSD_SETVALUE, OSD_PERCENTAGESIZE, OSD_SIZE ); // restore setting
        GlobalDeleteAtom( FLT_DSCOPE_VIDEO );
    }
    return;
}

/*----------------------------------------------------------------------------



*/
BOOL DScopeVideoEnabled_OnChange( long NewValue ) // gets called when filter state changes
{                                            // ie. when going from channel without filter to
                                             // channel with filter. Is not called if going from
                                             // channel to channel if filter state has no change.
    UseDScopeVideo = (BOOL) NewValue;

    // MJ DEBUGGING
#ifdef _DEBUG
    OutputDebugString   ( "DScopeVideoEnabled_OnChange called" );
#endif

    if ( UseDScopeVideo )
    {
        DScopeUpdateParameters ();          // quick fix only, until pInfo->IsFirstInSeries is working...

        if (!bLineMarker)
        {
            //DScopeOSDText ( "DScope Video" );     // mj debug here, only allow this once !!
        }
    }

    return TRUE;
}

/*----------------------------------------------------------------------------

    DScope start-up code

*/
void DScopeVideoStart( void )
{
#ifdef _DEBUG
    OutputDebugString ( "DSCOPE_VIDEO - START" );
#endif

    hPrevWindow = FindWindow((LPCTSTR) "DScaler", NULL);

#ifdef _DEBUG
    if (hPrevWindow == NULL)
    {
        OutputDebugString ( "Could not find DScaler !!" );
    }
#endif

    // ## problem here ## DScaler does not provide valid data that would be
    // useful at this point, such as pointers to source or overlay, pInfo data
    // that could be useful for filter setup, and doing calcs that could be
    // outside main filter loop..


    // following line is of no use, see above...
    // DScopeUpdateParameters ();

    // bHasRunOnce is used as a work-around..

    return;
}

/*----------------------------------------------------------------------------

    DScope stop code

*/
void DScopeVideoStop( void )
{
#ifdef _DEBUG
    OutputDebugString ( "DSCOPE_VIDEO - STOP" );
#endif
    return;
}

/*----------------------------------------------------------------------------

    Draw Y grid-lines or solid background

*/
void DrawGrid( DWORD* pDestination, int pitch, int height, int width )
{
    static DWORD* pGridLine_Lower;
    static DWORD* pGridLine_Upper;
    static DWORD  i;
    static DWORD  ii;  // mj experimenting here
    static DWORD  iii; //
    const  _int32 white = 0x000000FF;
    int temp;



    //pGridLine_Lower   = (DWORD) pDestination + (Y1_BaseLine * pitch);
    _asm
    {

        mov     eax, Y1_BaseLine            // mov dest, src
        mul     pitch                       // result in eax
        add     eax, dword ptr[pDestination]//
        mov     pGridLine_Lower,eax         //
    }

    if (pGridLine_Lower > pDestination + (height * pitch))
    {
        pGridLine_Lower = pDestination + ((height-1) * pitch);

    }

    //pGridLine_Upper = (DWORD) pGridLine_Lower - (((int)255 * Y_Scale) * pitch);
    _asm
    {
        fild    white                       // push (int)pitch on stack
        fmul    Y_Scale                     // * Y_Scale
        frndint                             // round to integer
        fimul   pitch                       // * 255

        // tidy & fix here mj

        //fsubr   dword ptr[pGridLine_Lower]     // (rev)subtract st(0) from pGridLine_Lower not working???

        fistp   temp   //dword ptr[pGridLine_Upper]  // store as integer, and pop fpu stack
        mov    eax, temp
        mov    ecx, dword ptr[pGridLine_Lower]
        sub    ecx, eax
        mov    dword ptr[pGridLine_Upper], ecx
    }

    if (pGridLine_Upper < pDestination )               //
    {
        pGridLine_Upper = pDestination;
    }

    if(bSolidBackground)
    {                             // FIXES NEEDED HERE !! MJ
        for (ii = (DWORD)pGridLine_Upper; ii < (DWORD)pGridLine_Lower  + pitch; ii = ii + pitch)
        {
            for (i = 0; i < (DWORD) width * 2 ; i = i + 4 )     // something strange here!!, DWORD increment not working ?? MJ
            {
                *(DWORD*)(ii + i) = dwGrid_Color;               // draw line
            }
        }
    }
    else
    {
        for (iii = 0; iii < (DWORD) (width / 2) ; iii = iii + 2)// dashed line
        {
            *(DWORD*)(pGridLine_Upper + iii) = dwGrid_Color;    // top grid
            *(DWORD*)(pGridLine_Lower + iii) = dwGrid_Color;    // bottom grid
        }
    }
    return;
}
/*----------------------------------------------------------------------------

    Draw Line Marker

*/
void DrawLineMarker( BYTE* source, int source_pitch, BYTE* pDestination, int dest_pitch, int dest_frame_height, int width )
{
     static DWORD i;
     static BYTE* pLineMarker;

     pLineMarker = pDestination + (VideoLineDS * dest_pitch);

     // draw white indicator
    if (pLineMarker < pDestination + (dest_frame_height * dest_pitch))// prevent over-run,, move this MJ
    {
           for (i = LineMarkerStart; i < LineMarkerEnd; i = i + 2) // do every second byte ('Y')
            {
                *(BYTE*) (pLineMarker + i) = 255;       // set 'Y' full-on
            }

            _ltoa ( VideoLine, strbuff, 10 );
            strcat( strbuff, "   " );

        // draw text
        if (VideoLineDS < (dest_frame_height - 20) )    //  *********** tempory fix only mj
        {
            DrawString (source,                      // source
                     pDestination,                   // pDestination
                     source_pitch,                   // pitch [source]
                     dest_frame_height,              // frame height[destination]
                     LineMarkerTextStart,            // x
                     (int) (VideoLineDS / 20),       // y
                     strbuff);                       // text
        }
    }
    return;
}

/*----------------------------------------------------------------------------

    Update Parameters

    - most calculations to be placed/relocated here, to reduce work-load within loops...

*/
void DScopeUpdateParameters ( void )
{
    #ifdef _DEBUG
        OutputDebugString   ( "DScopeUpdateParameters called" );
    #endif

    LineMarkerStart = (*pSource_right * 2) - 32;
    LineMarkerEnd   = *pLineLength;

    // chars are 10 pixels wide, start text 4 chars from right side
    LineMarkerTextStart = (DWORD)(*pFrameWidth / 10) - 4;

    // X scaling to fit screen width
    X_Scale = ((double) *pFrameWidth / (double) *pLineLength);

    // convert from percentage to fraction
    Y_Scale = ((double) Y_ScalePercent / (double) 100);

    // translate start offset from TV to DScaler format
    VideoLineDS = VideoLine - 1;
    FieldLineDS = (int)(VideoLineDS / 2);
    InputLine_Offset = FieldLineDS * *pInputPitch;

    return;
}

//----------------------------------------------------------------------------