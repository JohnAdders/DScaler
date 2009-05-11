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

#pragma warning(disable : 4799)

#include "windows.h"
#include "DS_Deinterlace.h"
#include "..\help\helpids.h"

/////////////////////////////////////////////////////////////////////////////
// Copies memory to two locations using MMX registers for speed.
void memcpyBOBMMX(void *Dest1, void *Dest2, void *Src, size_t nBytes)
{
    __asm
    {
        mov     esi, dword ptr[Src]
        mov     edi, dword ptr[Dest1]
        mov     ebx, dword ptr[Dest2]
        mov     ecx, nBytes
        shr     ecx, 6                      // nBytes / 64
align 8
CopyLoop:
        movq    mm0, qword ptr[esi]
        movq    mm1, qword ptr[esi+8*1]
        movq    mm2, qword ptr[esi+8*2]
        movq    mm3, qword ptr[esi+8*3]
        movq    mm4, qword ptr[esi+8*4]
        movq    mm5, qword ptr[esi+8*5]
        movq    mm6, qword ptr[esi+8*6]
        movq    mm7, qword ptr[esi+8*7]
        movq    qword ptr[edi], mm0
        movq    qword ptr[edi+8*1], mm1
        movq    qword ptr[edi+8*2], mm2
        movq    qword ptr[edi+8*3], mm3
        movq    qword ptr[edi+8*4], mm4
        movq    qword ptr[edi+8*5], mm5
        movq    qword ptr[edi+8*6], mm6
        movq    qword ptr[edi+8*7], mm7
        movq    qword ptr[ebx], mm0
        movq    qword ptr[ebx+8*1], mm1
        movq    qword ptr[ebx+8*2], mm2
        movq    qword ptr[ebx+8*3], mm3
        movq    qword ptr[ebx+8*4], mm4
        movq    qword ptr[ebx+8*5], mm5
        movq    qword ptr[ebx+8*6], mm6
        movq    qword ptr[ebx+8*7], mm7
        add     esi, 64
        add     edi, 64
        add     ebx, 64
        dec ecx
        jne near CopyLoop

        mov     ecx, nBytes
        and     ecx, 63
        cmp     ecx, 0
        je EndCopyLoop
align 8
CopyLoop2:
        mov dl, byte ptr[esi]
        mov byte ptr[edi], dl
        mov byte ptr[ebx], dl
        inc esi
        inc edi
        inc ebx
        dec ecx
        jne near CopyLoop2
EndCopyLoop:
    }
}

/////////////////////////////////////////////////////////////////////////////
// Copies memory to two locations using MMX registers for speed.
void memcpyBOBSSE(void *Dest1, void *Dest2, void *Src, size_t nBytes)
{
    __asm
    {
        mov     esi, dword ptr[Src]
        mov     edi, dword ptr[Dest1]
        mov     ebx, dword ptr[Dest2]
        mov     ecx, nBytes
        shr     ecx, 7                      // nBytes / 128
align 8
CopyLoop:
        movaps  xmm0, xmmword ptr[esi]
        movaps  xmm1, xmmword ptr[esi+16*1]
        movaps  xmm2, xmmword ptr[esi+16*2]
        movaps  xmm3, xmmword ptr[esi+16*3]
        movaps  xmm4, xmmword ptr[esi+16*4]
        movaps  xmm5, xmmword ptr[esi+16*5]
        movaps  xmm6, xmmword ptr[esi+16*6]
        movaps  xmm7, xmmword ptr[esi+16*7]
        movntps xmmword ptr[edi], xmm0
        movntps xmmword ptr[edi+16*1], xmm1
        movntps xmmword ptr[edi+16*2], xmm2
        movntps xmmword ptr[edi+16*3], xmm3
        movntps xmmword ptr[edi+16*4], xmm4
        movntps xmmword ptr[edi+16*5], xmm5
        movntps xmmword ptr[edi+16*6], xmm6
        movntps xmmword ptr[edi+16*7], xmm7
        movntps xmmword ptr[ebx], xmm0
        movntps xmmword ptr[ebx+16*1], xmm1
        movntps xmmword ptr[ebx+16*2], xmm2
        movntps xmmword ptr[ebx+16*3], xmm3
        movntps xmmword ptr[ebx+16*4], xmm4
        movntps xmmword ptr[ebx+16*5], xmm5
        movntps xmmword ptr[ebx+16*6], xmm6
        movntps xmmword ptr[ebx+16*7], xmm7
        add     esi, 128
        add     edi, 128
        add     ebx, 128
        dec ecx
        jne near CopyLoop

        mov     ecx, nBytes
        and     ecx, 127
        cmp     ecx, 0
        je EndCopyLoop
align 8
CopyLoop2:
        mov dl, byte ptr[esi]
        mov byte ptr[edi], dl
        mov byte ptr[ebx], dl
        inc esi
        inc edi
        inc ebx
        dec ecx
        jne near CopyLoop2
EndCopyLoop:
    }
}

/////////////////////////////////////////////////////////////////////////////
// Simple Bob.  Copies the most recent field to the overlay, with each scanline
// copied twice.
/////////////////////////////////////////////////////////////////////////////
BOOL DeinterlaceBob(TDeinterlaceInfo* pInfo)
{
    int i;
    BYTE* lpOverlay = pInfo->Overlay;
    BYTE* CurrentLine = pInfo->PictureHistory[0]->pData;
    DWORD Pitch = pInfo->InputPitch;

    // No recent data?  We can't do anything.
    if (CurrentLine == NULL)
    {
        return FALSE;
    }

    // If field is odd we will offset it down 1 line to avoid jitter  TRB 1/21/01
    if (pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD)
    {
        if (pInfo->CpuFeatureFlags & FEATURE_SSE)
        {
            pInfo->pMemcpy(lpOverlay, CurrentLine, pInfo->LineLength);   // extra copy of first line
            lpOverlay += pInfo->OverlayPitch;                            // and offset out output ptr
            for (i = 0; i < pInfo->FieldHeight - 1; i++)
            {
                memcpyBOBSSE(lpOverlay, lpOverlay + pInfo->OverlayPitch,
                    CurrentLine, pInfo->LineLength);
                lpOverlay += 2 * pInfo->OverlayPitch;
                CurrentLine += Pitch;
            }
            pInfo->pMemcpy(lpOverlay, CurrentLine, pInfo->LineLength);   // only 1 copy of last line
        }
        else
        {
            pInfo->pMemcpy(lpOverlay, CurrentLine, pInfo->LineLength);   // extra copy of first line
            lpOverlay += pInfo->OverlayPitch;                    // and offset out output ptr
            for (i = 0; i < pInfo->FieldHeight - 1; i++)
            {
                memcpyBOBMMX(lpOverlay, lpOverlay + pInfo->OverlayPitch,
                    CurrentLine, pInfo->LineLength);
                lpOverlay += 2 * pInfo->OverlayPitch;
                CurrentLine += Pitch;
            }
            pInfo->pMemcpy(lpOverlay, CurrentLine, pInfo->LineLength);   // only 1 copy of last line
        }
    }
    else
    {
        if (pInfo->CpuFeatureFlags & FEATURE_SSE)
        {
            for (i = 0; i < pInfo->FieldHeight; i++)
            {
                memcpyBOBSSE(lpOverlay, lpOverlay + pInfo->OverlayPitch,
                    CurrentLine, pInfo->LineLength);
                lpOverlay += 2 * pInfo->OverlayPitch;
                CurrentLine += Pitch;
            }
        }
        else
        {
            for (i = 0; i < pInfo->FieldHeight; i++)
            {
                memcpyBOBMMX(lpOverlay, lpOverlay + pInfo->OverlayPitch,
                    CurrentLine, pInfo->LineLength);
                lpOverlay += 2 * pInfo->OverlayPitch;
                CurrentLine += Pitch;
            }
        }
    }
    // need to clear up MMX registers
    DO_EMMS;
    return TRUE;
}

DEINTERLACE_METHOD BobMethod =
{
    sizeof(DEINTERLACE_METHOD),
    DEINTERLACE_CURRENT_VERSION,
    "Simple Bob",
    NULL,
    FALSE,
    FALSE,
    DeinterlaceBob,
    50,
    60,
    0,
    NULL,
    INDEX_BOB,
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
    IDH_BOB,
};


__declspec(dllexport) DEINTERLACE_METHOD* GetDeinterlacePluginInfo(long CpuFeatureFlags)
{
    return &BobMethod;
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}
