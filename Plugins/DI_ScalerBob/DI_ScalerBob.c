/////////////////////////////////////////////////////////////////////////////
// $Id: DI_ScalerBob.c,v 1.7 2008-02-08 13:43:20 adcockj Exp $
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
// Change Log
//
// Date          Developer             Changes
//
// 04 Jan 2001   John Adcock           Split into separate module
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.6  2002/06/13 12:10:24  adcockj
// Move to new Setings dialog for filers, video deint and advanced settings
//
// Revision 1.5  2001/11/21 15:21:40  adcockj
// Renamed DEINTERLACE_INFO to TDeinterlaceInfo in line with standards
// Changed TDeinterlaceInfo structure to have history of pictures.
//
// Revision 1.4  2001/07/13 16:13:33  adcockj
// Added CVS tags and removed tabs
//
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DS_Deinterlace.h"
#include "..\help\helpids.h"

BOOL DeinterlaceScalerBob(TDeinterlaceInfo* pInfo)
{
    int nLineTarget;
    BYTE* CurrentLine = pInfo->PictureHistory[0]->pData;

    for (nLineTarget = 0; nLineTarget < pInfo->FieldHeight; nLineTarget++)
    {
        pInfo->pMemcpy(pInfo->Overlay + nLineTarget * pInfo->OverlayPitch,
                    CurrentLine,
                    pInfo->LineLength);
        CurrentLine += pInfo->InputPitch;
    }
    // need to clear up MMX registers
    _asm
    {
        emms
    }
    return TRUE;
}

DEINTERLACE_METHOD ScalerBobMethod =
{
    sizeof(DEINTERLACE_METHOD),
    DEINTERLACE_CURRENT_VERSION,
    "Scaler Bob", 
    NULL,
    TRUE,
    FALSE,
    DeinterlaceScalerBob,
    50, 
    60,
    0,
    NULL,
    INDEX_SCALER_BOB,
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
    IDH_SCALER_BOB,
};


__declspec(dllexport) DEINTERLACE_METHOD* GetDeinterlacePluginInfo(long CpuFeatureFlags)
{
    return &ScalerBobMethod;
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}

