/////////////////////////////////////////////////////////////////////////////
// $Id: FLT_LogoKill.c,v 1.5 2001-07-13 16:13:33 adcockj Exp $
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
// This code is based on the 
// LogoAway VirtualDub filter by Krzysztof Wojdon (c) 2000
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DS_Filter.h"

long Top = 5;
long Left = 5;
long Width = 30;
long Height = 30;
long Max = 128;

typedef enum
{
    MODE_GREY,
    MODE_MAX,
    MODE_DYNAMIC_MAX,
    MODE_LASTONE,
} eMODE;

eMODE Mode = MODE_GREY;

LPCSTR ModeList[] =
{
    "Grey",
    "Limit To Max Value",
    "Dynamic Max",
};

SETTING FLT_LogoKillSettings[FLT_LOGOKILL_SETTING_LASTONE];

long LogoKiller(DEINTERLACE_INFO *info)
{
    BYTE* lpOverlay = info->Overlay + Left * 8;
    const __int64 qwGrey = 0x7f7f7f7f7f7f7f7f;
    long Pitch = info->OverlayPitch;

    // we use some of the integer SSE instructions these are supported
    // either by PIII and above or by Althons and above
    if((info->CpuFeatureFlags & FEATURE_SSE) || (info->CpuFeatureFlags & FEATURE_MMXEXT))
    {
        // we are OK to use everything
    }
    else
    {
        // we'll just fall back to GREY mode on
        // old style CPUs
        Mode = MODE_GREY;
    }

    // check bounds
    if((Top + Height) >= info->FrameHeight ||
        (Left + Width) >= info->FrameWidth / 4)
    {
        return 1000;
    }

    switch(Mode)
    {
    case MODE_DYNAMIC_MAX:
        // here we find the maximum luma from the top and bottom lines
        // then use this value to max out any brighter areas in the middle
        _asm
        {
            // start off with zero as the max luma value
            // for each of the pixels in the mmx 4
            mov eax, 0xff00
            pinsrw mm1, eax, 0
            pinsrw mm1, eax, 1
            pinsrw mm1, eax, 2
            pinsrw mm1, eax, 3
            
            // set edi to be the top right corner
            mov edi, lpOverlay
            mov ebx, Pitch
            mov eax, Top
            imul eax, ebx
            add edi, eax

            // loop through the top line looking for the max
            // luma values in each of the positions
            mov edx, edi
            mov ecx, Width
            LOOP_FINDMAX1:
            movq mm0, qword ptr[edx]
            pmaxub mm1, mm0
            add edx, 8
            dec ecx
            jnz LOOP_FINDMAX1

            // goto bottom line
            mov eax, Height
            sub ecx, 1
            imul eax, ebx
            add edi, eax

            // loop through the bottom line looking for the max
            // luma values in each of the positions
            mov edx, edi
            mov ecx, Width
            LOOP_FINDMAX2:
            movq mm0, qword ptr[edx]
            pmaxub mm1, mm0
            add edx, 8
            dec ecx
            jnz LOOP_FINDMAX2
            
            // go back to the top
            sub edi, eax

            // here we get the maximum
            // luma and have it in the end position
            movq mm2, mm1            
            movq mm3, mm1            
            movq mm4, mm1            
            psrlq mm2, 16
            psrlq mm3, 32
            psrlq mm4, 48
            pmaxub mm1, mm2
            pmaxub mm3, mm4
            pmaxub mm1, mm3
            
            // now fill mm1 with the dynamically found
            // max value
            pextrw eax, mm1, 0
            pinsrw mm1, eax, 1
            pinsrw mm1, eax, 2
            pinsrw mm1, eax, 3

            // loop through and use the max value
            mov eax, Height
            LOOP_DYN_MAX_OUTER:
            mov edx, edi
            mov ecx, Width
            LOOP_DYN_MAX_INNER:
            movq mm0, qword ptr[edx]
            pminub mm0, mm1
            movq qword ptr[edx], mm0
            add edx, 8
            dec ecx
            jnz LOOP_DYN_MAX_INNER
            add edi, ebx
            dec eax
            jnz LOOP_DYN_MAX_OUTER
        }
        break;
    case MODE_MAX:
        _asm
        {
            // set up mm1 as the max value to be used
            mov eax, 0xff00
            add eax, Max
            pinsrw mm1, eax, 0
            pinsrw mm1, eax, 1
            pinsrw mm1, eax, 2
            pinsrw mm1, eax, 3

            // set edi to top left
            mov edi, lpOverlay
            mov ebx, Pitch
            mov eax, Top
            imul eax, ebx
            add edi, eax
            // for each row
            mov eax, Height
            LOOP_MAX_OUTER:
            // for each 4 pixel chunk
            mov edx, edi
            mov ecx, Width
            LOOP_MAX_INNER:
            movq mm0, qword ptr[edx]
            pminub mm0, mm1
            movq qword ptr[edx], mm0
            add edx, 8
            dec ecx
            jnz LOOP_MAX_INNER
            add edi, ebx
            dec eax
            jnz LOOP_MAX_OUTER
        }
        break;
    case MODE_GREY:
    default:
        _asm
        {
            // set up mm0 as mid grey
            movq mm0, qwGrey
            
            // set edi to top left
            mov edi, lpOverlay
            mov ebx, Pitch
            mov eax, Top
            imul eax, ebx
            add edi, eax
            // loop over height
            mov eax, Height
            LOOP_GREY_OUTER:
            // loop over width
            mov edx, edi
            mov ecx, Width
            LOOP_GREY_INNER:
            // set area to grey
            movq qword ptr[edx], mm0
            add edx, 8
            dec ecx
            jnz LOOP_GREY_INNER
            add edi, ebx
            dec eax
            jnz LOOP_GREY_OUTER
        }
        break;
    }
    _asm
    {
        emms
    }
    return 1000;
}

BOOL Top_OnChange(long NewValue)
{
    Top = NewValue;
    return TRUE;   
}

BOOL Left_OnChange(long NewValue)
{
    Left = NewValue;
    return TRUE;   
}

BOOL Width_OnChange(long NewValue)
{
    Width = NewValue;
    return TRUE;   
}

BOOL Height_OnChange(long NewValue)
{
    Height = NewValue;
    return TRUE;   
}

void LinearCorrStart(void)
{
    Top_OnChange(Top);
    Left_OnChange(Left);
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING FLT_LogoKillSettings[FLT_LOGOKILL_SETTING_LASTONE] =
{
    {
        "Top", SLIDER, 0, &Top,
        20, 0, 575, 1, 1,
        NULL,
        "LogoKillFilter", "Top", Top_OnChange,
    },
    {
        "Left", SLIDER, 0, &Left,
        20, 0, 191, 1, 1,
        NULL,
        "LogoKillFilter", "Left", Left_OnChange,
    },
    {
        "Width", SLIDER, 0, &Width,
        20, 0, 191, 1, 1,
        NULL,
        "LogoKillFilter", "Width", Width_OnChange,
    },
    {
        "Height", SLIDER, 0, &Height,
        50, 2, 575, 1, 1,
        NULL,
        "LogoKillFilter", "Height", Height_OnChange,
    },
    {
        "Mode", ITEMFROMLIST, 0, (long*)&Mode,
        MODE_GREY, MODE_GREY, MODE_LASTONE -1, 1, 1,
        ModeList,
        "LogoKillFilter", "Mode", NULL,
    },
    {
        "Max", SLIDER, 0, &Max,
        128, 0, 255, 1, 1,
        NULL,
        "LogoKillFilter", "Max", NULL,
    },
};

FILTER_METHOD LogoKillMethod =
{
    sizeof(FILTER_METHOD),
    FILTER_CURRENT_VERSION,
    "Logo Killer Filter",
    "&Logo Killer (experimental)",
    FALSE,
    FALSE,
    LogoKiller, 
    0,
    TRUE,
    NULL,
    NULL,
    NULL,
    FLT_LOGOKILL_SETTING_LASTONE,
    FLT_LogoKillSettings,
    WM_FLT_LOGOKILL_GETVALUE - WM_USER,
};


__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo(long CpuFeatureFlags)
{
    return &LogoKillMethod;
}

BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}

