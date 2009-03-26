/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Mike Temperton.  All rights reserved.
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
//  GNU Library General Public License for more details
/////////////////////////////////////////////////////////////////////////////

/**
* @file PaintingHDC.cpp CPaintingHDC Implementation
*/

#include "stdafx.h"
#include "PaintingHDC.h"
#include "IOutput.h"
#include "D3D9Output.h"
#include "VBI_VideoText.h"


CPaintingHDC::CPaintingHDC()
{
    m_hBufferDC = NULL;
    m_hOriginalDC = NULL;
    m_hSaveBmp = NULL;
    m_hBmp = NULL;
    m_ddsurface = NULL;
    m_alphasurface = NULL;
}


CPaintingHDC::~CPaintingHDC()
{
    ReleaseD3DBuffer();
    if (m_hBufferDC != NULL)
    {
        DeleteObject(SelectObject(m_hBufferDC, m_hSaveBmp));
        DeleteDC(m_hBufferDC);
    }

}

void CPaintingHDC::ReleaseD3DBuffer()
{
    if(m_ddsurface!=NULL)
    {
        m_ddsurface->Release();
        m_ddsurface=NULL;
    }
    if(m_alphasurface!=NULL)
    {
        m_alphasurface->Release();
        m_alphasurface=NULL;
    }
}

void CPaintingHDC::CreateD3DBuffer()
{
    if(m_ddsurface==NULL && GetActiveOutput()->Type() == IOutput::OUT_D3D && ((CD3D9Output *)GetActiveOutput())->pDevice != NULL)
    {
        RECT src=((CD3D9Output *)GetActiveOutput())->Overlay_GetCurrentSrcRect();
        ((CD3D9Output *)GetActiveOutput())->pDevice->CreateOffscreenPlainSurface(src.right,src.bottom,
                 D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &m_ddsurface, NULL);
        ((CD3D9Output *)GetActiveOutput())->pDevice->CreateOffscreenPlainSurface(src.right,src.bottom,
                 D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &m_alphasurface, NULL);
    }
}

HDC CPaintingHDC::BeginPaint(HDC hDC, LPRECT pRect)
{
    UpdateGeometry(hDC, pRect);
    CopyRect(&m_PaintRect, pRect);
    return m_hBufferDC;
}


void CPaintingHDC::EndPaint()
{
    if(GetActiveOutput()->Type() == IOutput::OUT_OVERLAY)
    {
        BitBltRects(&m_PaintRect, 1);
        m_hOriginalDC = NULL;
    }
    else
    {
        BitBltRectsD3D(&m_PaintRect, 1, ((CD3D9Output *)GetActiveOutput())->lpDDOSD);
    }
}


HDC CPaintingHDC::GetBufferDC()
{
    return m_hBufferDC;
}


BOOL CPaintingHDC::UpdateGeometry(HDC hDC, LPRECT pRect, BOOL bBufferTrim)
{
    BOOL bRecreated = FALSE;

    if (m_hBufferDC != NULL)
    {
        if ((bBufferTrim != FALSE && (m_Rect.right != pRect->right ||
            m_Rect.bottom != pRect->bottom)) || (bBufferTrim == FALSE &&
            (m_Rect.right < pRect->right || m_Rect.bottom < pRect->bottom)))
        {
            DeleteObject(SelectObject(m_hBufferDC, m_hSaveBmp));
            DeleteDC(m_hBufferDC);
            m_hBufferDC = NULL;

            ReleaseD3DBuffer();
        }
    }

    if (m_hBufferDC == NULL)
    {
        m_hBufferDC = CreateCompatibleDC(hDC);

        if (m_hBufferDC != NULL)
        {
            m_hBmp = CreateCompatibleBitmap(hDC, pRect->right, pRect->bottom);

            if (m_hBmp != NULL)
            {
                m_hSaveBmp = SelectObject(m_hBufferDC, m_hBmp);

                if (m_hSaveBmp == NULL)
                {
                    DeleteObject(m_hBmp);
                    m_hBmp = NULL;
                }
            }

            if (m_hBmp == NULL)
            {
                DeleteDC(m_hBufferDC);
                m_hBufferDC = NULL;
            }
            else
            {
                m_Rect.right = pRect->right;
                m_Rect.bottom = pRect->bottom;
                bRecreated = TRUE;
            }

            CreateD3DBuffer();
        }
    }

    if (m_hBufferDC != NULL)
    {
        m_hOriginalDC = hDC;

        m_Rect.left = pRect->left;
        m_Rect.top = pRect->top;
    }


    return bRecreated;
}


void CPaintingHDC::BitBltRects(LPRECT pRectList, LONG nRectCount, HDC hDstDC)
{
    if (m_hBufferDC == NULL)
    {
        return;
    }

    if (hDstDC == NULL)
    {
        hDstDC = m_hOriginalDC;
    }

    if (hDstDC == NULL)
    {
        return;
    }

    for (int i = 0; i < nRectCount; i++)
    {
        BitBlt(hDstDC, pRectList[i].left, pRectList[i].top,
            pRectList[i].right - pRectList[i].left,
            pRectList[i].bottom - pRectList[i].top,
            m_hBufferDC, pRectList[i].left, pRectList[i].top, SRCCOPY);
    }
}



void CPaintingHDC::BitBltRectsD3D(LPRECT pRectList, LONG nRectCount, LPDIRECT3DSURFACE9 target, HDC hDstDC)
{
    if (m_hBufferDC == NULL || target == NULL)
    {
        return;
    }

    if(m_ddsurface==NULL || m_alphasurface==NULL)
    {
        CreateD3DBuffer();
        if(m_ddsurface==NULL || m_alphasurface==NULL)
        {
            return;
        }
    }

    if (hDstDC == NULL)
    {
        hDstDC = m_hOriginalDC;
    }

    if (hDstDC == NULL)
    {
        return;
    }

    // get rect we output video to
    RECT outRect = ((CD3D9Output *)GetActiveOutput())->Overlay_GetCurrentDestRect();


    // VT_BLACK gets painted directly
    RECT r;
    for (int i = 0; i < nRectCount; i++)
    {
        if(VT_GetState()!=VT_BLACK && IntersectRect(&r, &pRectList[i], &outRect))
        {

            HDC outDC;
            // GetDC doesn't work for ARGB surfaces prior to Vista so the m_alphasurface is necessary :( -> slow as hell
            if(SUCCEEDED(m_ddsurface->GetDC(&outDC)))
            {
                // further processing necessary
                // first do the things around output
                if(pRectList[i].left<outRect.left)
                {
                    // have to paint left from output
                    r.left=pRectList[i].left;
                    r.right=outRect.left;
                    r.top=pRectList[i].top;
                    r.bottom=pRectList[i].bottom;
                    BitBlt(hDstDC, r.left, r.top,
                        r.right - r.left,
                        r.bottom - r.top,
                        m_hBufferDC, r.left, r.top, SRCCOPY);
                }
                if(pRectList[i].right>outRect.right)
                {
                    // have to paint right from output
                    r.left=outRect.right;
                    r.right=pRectList[i].right;
                    r.top=pRectList[i].top;
                    r.bottom=pRectList[i].bottom;
                    BitBlt(hDstDC, r.left, r.top,
                        r.right - r.left,
                        r.bottom - r.top,
                        m_hBufferDC, r.left, r.top, SRCCOPY);
                }
                if(pRectList[i].top<outRect.top)
                {
                    // top
                    r.left=pRectList[i].left; // maybe drawing 2x ... really necessary? however don't care
                    r.right=pRectList[i].right;
                    r.top=pRectList[i].top;
                    r.bottom=outRect.top;
                    BitBlt(hDstDC, r.left, r.top,
                        r.right - r.left,
                        r.bottom - r.top,
                        m_hBufferDC, r.left, r.top, SRCCOPY);
                }
                if(pRectList[i].bottom>outRect.bottom)
                {
                    // bottom
                    r.left=pRectList[i].left; // maybe drawing 2x ... really necessary? however don't care
                    r.right=pRectList[i].right;
                    r.top=outRect.bottom;
                    r.bottom=pRectList[i].bottom;
                    BitBlt(hDstDC, r.left, r.top,
                        r.right - r.left,
                        r.bottom - r.top,
                        m_hBufferDC, r.left, r.top, SRCCOPY);
                }

                // TODO: this could be optimized somehow -- any ideas???


                RECT src=((CD3D9Output *)GetActiveOutput())->Overlay_GetCurrentSrcRect();

                // some scaling around to get proper coordinates for stretchblt

                if(pRectList[i].top<outRect.top) pRectList[i].top=outRect.top;
                if(pRectList[i].bottom>outRect.bottom) pRectList[i].bottom=outRect.bottom;
                if(pRectList[i].left<outRect.left) pRectList[i].left=outRect.left;
                if(pRectList[i].right>outRect.right) pRectList[i].right=outRect.right;

                RECT or;
                float scaleY=(float)(src.bottom-src.top)/(float)(outRect.bottom-outRect.top);
                float scaleX=(float)(src.right-src.left)/(float)(outRect.right-outRect.left);

                or.left=(LONG)max(src.left, scaleX*(pRectList[i].left-outRect.left)+src.left);
                or.top=(LONG)max(src.top, scaleY*(pRectList[i].top-outRect.top)+src.top);
                or.bottom=(LONG)min(src.bottom, scaleY*(pRectList[i].bottom-outRect.top)+src.top);
                or.right=(LONG)min(src.right, scaleX*(pRectList[i].right-outRect.left)+src.left);

                StretchBlt(outDC, or.left, or.top,or.right-or.left, or.bottom-or.top,
                    m_hBufferDC, pRectList[i].left, pRectList[i].top,
                    pRectList[i].right-pRectList[i].left,
                    pRectList[i].bottom-pRectList[i].top, SRCCOPY);



                m_ddsurface->ReleaseDC(outDC);


                // Set alpha channel according to color key - we can't use Direct3D here, at least i found no way
                // read memory access seems to be slow .. so asm isn't much faster here

                D3DLOCKED_RECT lr, ddlr;
                if(SUCCEEDED(m_ddsurface->LockRect(&ddlr, &or, D3DLOCK_READONLY )))
                {
                    if(SUCCEEDED(m_alphasurface->LockRect(&lr, &or, 0)))
                    {
                        COLORREF colorkey=GetActiveOutput()->Overlay_GetColor();
                        colorkey=((colorkey&0xFF)<<16)|(colorkey&0xFF00)|((colorkey&0xFF0000)>>16);

                        for (int nLine = 0; nLine < or.bottom-or.top; nLine++)
                        {
                            DWORD *pPixel = (DWORD*)((BYTE*)lr.pBits + nLine * lr.Pitch );
                            DWORD *ddPixel = (DWORD*)((BYTE*)ddlr.pBits + nLine * ddlr.Pitch );
                            for (int nPixel = 0; nPixel < or.right-or.left; ++nPixel)
                            {
                                if(*ddPixel==colorkey)
                                    *pPixel=0;
                                else
                                    *pPixel=0xFF000000|*ddPixel;
                                pPixel++;
                                ddPixel++;
                            }
                        }

                        m_alphasurface->UnlockRect();
                    }
                    m_ddsurface->UnlockRect();
                }

            }

        }
        else
        {
            // outside or black vt output, just blit
            BitBlt(hDstDC, pRectList[i].left, pRectList[i].top,
                pRectList[i].right - pRectList[i].left,
                pRectList[i].bottom - pRectList[i].top,
                m_hBufferDC, pRectList[i].left, pRectList[i].top, SRCCOPY);

        }
    }

    if(VT_GetState()!=VT_BLACK)
    {
        ((CD3D9Output *)GetActiveOutput())->pDevice->UpdateSurface(m_alphasurface, NULL, target, NULL);
    }
}

