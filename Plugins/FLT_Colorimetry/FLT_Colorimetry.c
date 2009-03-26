/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 John Adcock.  All rights reserved.
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

#include "windows.h"
#include "DS_Filter.h"
#include "math.h"
#include "..\help\helpids.h"


FILTER_METHOD ColorimetryMethod;

enum eColorimetryType
{
    BT709_BT601,
    BT601_BT709,
    BLANK,
    COLORIMETRY_LASTONE
};

LPCSTR ModeList[] =
{
    "BT709 -> BT601",
    "BT601 -> BT709",
    "None",
};


enum eColorimetryType ColorimetryType = BT709_BT601;

__int64 qwLumaMask = 0x00FF00FF00FF00FF;
__int64 qwChromaMask = 0xFF00FF00FF00FF00;
__int64 qwChromaBase = 0x0080008000800080;
__int64 qwLumaBase =   0x0010001000100010;
__int64 qwVChromaMask = 0x0000FF000000FF00;
__int64 qwZeros = 0x0000000000000000;

__int64 qwConvertMatrix[5];

long __cdecl FilterColorimetry(TDeinterlaceInfo* pInfo)
{
    BYTE* Pixels = NULL;
    int y;
    int Cycles;
    DWORD Table = (DWORD)qwConvertMatrix;

    if (pInfo->PictureHistory[0] == NULL )
    {
        return 1000;
    }

    Pixels = pInfo->PictureHistory[0]->pData;

    Cycles = pInfo->LineLength / 8;


    for (y = 0; y < pInfo->FieldHeight; y++)
    {
        _asm
        {
            mov ecx, Cycles
            mov edx, dword ptr[Pixels]
            mov eax, Table
            movq mm5, qwLumaMask
            movq mm6, qwLumaBase
            movq mm7, qwChromaBase
LOOP_LABEL:
            movq mm0,qword ptr[edx]
            movq mm1, mm0
            pand mm0, mm5
            psrlw mm1, 8
            psubusw mm0, mm6
            psubw mm1, mm7
            // should swap U and V round
            pshufw mm2, mm1, 0xb1
            // at this point we have
            // mm0 = 00YY00YY00YY00YY
            // mm1 = 00VV00UU00VV00UU
            // mm2 = 00UU00VV00UU00VV
            movq mm3, mm1
            movq mm4, mm2
            pmullw mm0, qword ptr[eax]
            add eax, 8
            pmullw mm3, qword ptr[eax]
            add eax, 8
            pmullw mm4, qword ptr[eax]
            add eax, 8
            paddsw mm0, mm3
            paddsw mm0, mm4
            psraw mm0, 6
            paddsb mm0, mm6
            pminsw mm0, mm5
            pmaxsw mm0, qwZeros

            pmullw mm1, qword ptr[eax]
            add eax, 8
            pmullw mm2, qword ptr[eax]
            paddsw mm1, mm2
            psraw mm1, 6
            paddw mm1, mm7
            pminsw mm0, mm5
            pmaxsw mm1, qwZeros
            pslld mm1, 8

            por mm0, mm1

            movq qword ptr[edx], mm0

            sub eax, 32

            add edx, 8
            loop LOOP_LABEL
        }
        Pixels += pInfo->InputPitch;
    }
    _asm emms;
    return 1000;
}

BOOL Colorimetry_OnChange(long NewValue)
{
    ColorimetryType = NewValue;
    switch(ColorimetryType)
    {
    case BT709_BT601:
        // 1     0.09931166    0.19169955
        // 0     0.98985381   -0.11065251
        // 0    -0.07245296    0.98339782
        qwConvertMatrix[0] = 0x0040004000400040;
        qwConvertMatrix[1] = 0x000C0006000C0006;
        qwConvertMatrix[2] = 0x0006000C0006000C;
        qwConvertMatrix[3] = 0x003E003F003E003F;
        qwConvertMatrix[4] = 0xFFF9FFFCFFF9FFFC;
        break;
    case BT601_BT709:
        // 1   -0.11554975   -0.20793764
        // 0    1.01863972    0.11461795
        // 0    0.07504945    1.02532707
        qwConvertMatrix[0] = 0x0040004000400040;
        qwConvertMatrix[1] = 0xFFF3FFF9FFF3FFF9;
        qwConvertMatrix[2] = 0xFFF9FFF3FFF9FFF3;
        qwConvertMatrix[3] = 0x0042004100420041;
        qwConvertMatrix[4] = 0x0005000700050007;
        break;
    default:
        // if we get anything else
        // just use identity matrix
        qwConvertMatrix[0] = 0x0040004000400040;
        qwConvertMatrix[1] = 0x0000000000000000;
        qwConvertMatrix[2] = 0x0000000000000000;
        qwConvertMatrix[3] = 0x0040004000400040;
        qwConvertMatrix[4] = 0x0000000000000000;
        break;
    }
    return FALSE;
}


////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING FLT_ColorimetrySettings[FLT_COLORIMETRY_SETTING_LASTONE] =
{
    {
        "Colorimetry Filter", ONOFF, 0, &(ColorimetryMethod.bActive),
        FALSE, 0, 1, 1, 1,
        NULL,
        "ColorimetryFilter", "UseColorimetryFilter", NULL,
    },
    {
        "Colorimetry Type", ITEMFROMLIST, 0, (long*)&ColorimetryType,
        0, 0, COLORIMETRY_LASTONE - 1, 1, 1,
        ModeList,
        "ColorimetryFilter", "Colorimetry", Colorimetry_OnChange,
    },
};

void __cdecl FilterStartColorimetry(void)
{
    // this will reset the table correctly
    Colorimetry_OnChange(ColorimetryType);
}

FILTER_METHOD ColorimetryMethod =
{
    sizeof(FILTER_METHOD),
    FILTER_CURRENT_VERSION,
    DEINTERLACE_INFO_CURRENT_VERSION,
    "Colorimetry Filter",
    "&Colorimetry",
    FALSE,
    TRUE,
    FilterColorimetry,
    0,
    TRUE,
    FilterStartColorimetry,
    NULL,
    NULL,
    FLT_COLORIMETRY_SETTING_LASTONE,
    FLT_ColorimetrySettings,
    WM_FLT_COLORIMETRY_GETVALUE - WM_APP,
    TRUE,
    1,
    IDH_FILTERS,
};


__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo(long CpuFeatureFlags)
{
    if(CpuFeatureFlags & (FEATURE_SSE))
    {
        return &ColorimetryMethod;
    }
    else
    {
        return NULL;
    }
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}

