/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Mike Temperton.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
// This header file is free software; you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details
/////////////////////////////////////////////////////////////////////////////

/**
 * @file paintinghdc.h paintinghdc Header file
 */

#ifndef __PAINTINGHDC_H__
#define __PAINTINGHDC_H__

#include <d3d9.h>

/** Class that allows easy buffered GDI painting
*/
class CPaintingHDC
{
public:
    CPaintingHDC();
    ~CPaintingHDC();

    HDC BeginPaint(HDC hDC, LPRECT pRect);
    void EndPaint();

    HDC GetBufferDC();

    BOOL UpdateGeometry(HDC hDC, LPRECT pRect, BOOL bBufferTrim = FALSE);
    void BitBltRects(LPRECT pRectList, LONG nRectCount, HDC hDestDC = NULL);
    void BitBltRectsD3D(LPRECT pRectList, LONG nRectCount, LPDIRECT3DSURFACE9 target, HDC hDestDC = NULL);
    void ReleaseD3DBuffer();
private:

    HDC m_hBufferDC;
    HDC m_hOriginalDC;
    RECT m_Rect;
    RECT m_PaintRect;
    LPDIRECT3DSURFACE9 m_ddsurface;
    LPDIRECT3DSURFACE9 m_alphasurface;
    HGDIOBJ m_hSaveBmp;
    HBITMAP m_hBmp;
    void CreateD3DBuffer();
};


#endif