/////////////////////////////////////////////////////////////////////////////
// $Id: FLT_Sharpness.c,v 1.5 2001-11-26 15:27:19 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Tom Barry.  All rights reserved.
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
// Revision 1.4  2001/11/21 15:21:41  adcockj
// Renamed DEINTERLACE_INFO to TDeinterlaceInfo in line with standards
// Changed TDeinterlaceInfo structure to have history of pictures.
//
// Revision 1.3  2001/08/09 21:34:59  adcockj
// Fixed bugs raise by Timo and Keld
//
// Revision 1.2  2001/08/03 14:24:06  adcockj
// fixed settings
//
// Revision 1.1  2001/08/03 12:27:17  adcockj
// Added Sharpness filter (copy of code from GreedyH)
//
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DS_Filter.h"

long Sharpness = 128;

long FilterSharpness(TDeinterlaceInfo* pInfo)
{
    BYTE* Pixels = pInfo->PictureHistory[0]->pData;
    int y;
    __int64 qwYMask = 0x00ff00ff00ff00ff;
    __int64 qwSharpness;
    int Cycles;

    // Need to have the current field to do the filtering.
    // and if we have nothing to do just return
    if (Pixels == NULL || 
        Sharpness == 0)
    {
        return 1000;
    }

    qwSharpness = Sharpness;
    qwSharpness |= qwSharpness << 48 | qwSharpness << 32 | qwSharpness << 16;
    Cycles = pInfo->LineLength / 8 - 2;

    for (y = 0; y < pInfo->FieldHeight; y++)
    {
        _asm
        {
            mov eax, Pixels
            movq mm1, [eax]
            add eax, 8
            movq mm2, [eax]
            mov ecx, Cycles
            movq mm5, qwYMask
            movq mm4, qwSharpness
LOOP_LABEL:
            movq mm0, mm1
            movq mm1, mm2
            movq mm2, [eax + 8]

            // do edge enhancement. 
		    movq	mm7, mm1				// work copy of curr pixel val
		    psrlq   mm0, 48					// right justify 1 pixel from qword to left
		    psllq   mm7, 16                 // left justify 3 pixels
		    por     mm0, mm7				// and combine
		    
		    movq	mm6, mm2				// copy of right qword pixel val
		    movq	mm7, mm1                // another copy of L2N current
		    psllq	mm6, 48					// left just 1 pixel from qword to right
		    psrlq   mm7, 16					// right just 3 pixels
		    por		mm6, mm7				// combine
		    pavgb	mm0, mm6				// avg of forward and prev by 1 pixel

            // we handle the possible plus and minus sharpness adjustments separately
		    movq    mm7, mm1				// another copy of L2N
		    psubusb mm7, mm0				// curr - surround
		    psubusb mm0, mm1                // surround - curr
		    pand	mm7, mm5                // YMask
		    pand	mm0, mm5                // YMask
		    pmullw  mm7, mm4                // mult by sharpness factor
		    pmullw  mm0, mm4                // mult by sharpness factor
		    psrlw   mm7, 8					// now have diff*EdgeEnhAmt/256 ratio			
		    psrlw   mm0, 8					// now have diff*EdgeEnhAmt/256 ratio			

		    paddusb mm7, mm1				// edge enhancement up
		    psubusb mm7, mm0                // edge enhancement down, mm7 now our sharpened value

            movq [eax], mm7
            add eax, 8
            dec ecx
            jne near LOOP_LABEL
        }
        Pixels += pInfo->InputPitch;
    }
    _asm 
    {
        emms
    }
    return 1000;
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
FILTER_METHOD SharpnessMethod;

SETTING FLT_SharpnessSettings[FLT_SHARPNESS_SETTING_LASTONE] =
{
    {
        "Sharpness", SLIDER, 0, &Sharpness,
        128, 0, 255, 1, 1,
        NULL,
        "SharpnessFilter", "Sharpness", NULL,
    },
    {
        "Sharpness Filter", ONOFF, 0, &(SharpnessMethod.bActive),
        FALSE, 0, 1, 1, 1,
        NULL,
        "SharpnessFilter", "UseSharpnessFilter", NULL,
    },
};

FILTER_METHOD SharpnessMethod =
{
    sizeof(FILTER_METHOD),
    FILTER_CURRENT_VERSION,
    DEINTERLACE_INFO_CURRENT_VERSION,
    "Sharpness Filter",
    "&Sharpness",
    FALSE,
    TRUE,
    FilterSharpness, 
    0,
    TRUE,
    NULL,
    NULL,
    NULL,
    FLT_SHARPNESS_SETTING_LASTONE,
    FLT_SharpnessSettings,
    WM_FLT_SHARPNESS_GETVALUE - WM_USER,
    TRUE,
    1,
};


__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo(long CpuFeatureFlags)
{
    // this plugin need as decent processor
    // as were going to need an acurate average
    if(CpuFeatureFlags & FEATURE_MMXEXT || CpuFeatureFlags & FEATURE_SSE)
    {
        return &SharpnessMethod;
    }
    else
    {
        return NULL;
    }
}

BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}

