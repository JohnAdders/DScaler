/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
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
#include "DS_Deinterlace.h"
#include "..\help\helpids.h"

BOOL DeinterlaceEvenOnly(TDeinterlaceInfo* pInfo)
{
    int nLineTarget;
    BYTE* CurrentLine = pInfo->PictureHistory[0]->pData;

    if (pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_EVEN)
    {
        for (nLineTarget = 0; nLineTarget < pInfo->FieldHeight; nLineTarget++)
        {
            // copy latest field's rows to overlay, resulting in a half-height image.
            pInfo->pMemcpy(pInfo->Overlay + nLineTarget * pInfo->OverlayPitch,
                        CurrentLine,
                        pInfo->LineLength);
            CurrentLine += pInfo->InputPitch;
        }
        // need to clear up MMX registers
        DO_EMMS;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


DEINTERLACE_METHOD EvenOnlyMethod =
{
    sizeof(DEINTERLACE_METHOD),
    DEINTERLACE_CURRENT_VERSION,
    "Even Scanlines Only",
    "Even",
    TRUE,
    FALSE,
    DeinterlaceEvenOnly,
    25,
    30,
    0,
    NULL,
    INDEX_EVEN_ONLY,
    NULL,
    NULL,
    NULL,
    NULL,
    1,
    0,
    0,
    -1,
    NULL,
    0,
    FALSE,
    FALSE,
    IDH_EVEN,
};


__declspec(dllexport) DEINTERLACE_METHOD* GetDeinterlacePluginInfo(long CpuFeatureFlags)
{
    return &EvenOnlyMethod;
}

BOOL WINAPI NoCRTDllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}


