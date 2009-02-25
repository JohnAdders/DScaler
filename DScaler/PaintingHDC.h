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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.6  2006/12/01 16:07:52  sabeld
// Added support for Direct3D output
//
// Revision 1.5  2003/10/27 10:39:52  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.4  2003/02/27 01:28:03  atnak
// Fixes bug with BeginPaint/EndPaint region
//
// Revision 1.3  2003/01/24 01:55:17  atnak
// OSD + Teletext conflict fix, offscreen buffering for OSD and Teletext,
// got rid of the pink overlay colorkey for Teletext.
//
// Revision 1.2  2001/11/29 17:30:52  adcockj
// Reorgainised bt848 initilization
// More Javadoc-ing
//
// Revision 1.1  2001/11/22 13:19:37  temperton
// Added CPaintingHDC class for double buffering painting
//
//
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