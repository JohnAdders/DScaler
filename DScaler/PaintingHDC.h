/////////////////////////////////////////////////////////////////////////////
// $Id: PaintingHDC.h,v 1.3 2003-01-24 01:55:17 atnak Exp $
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
// Revision 1.2  2001/11/29 17:30:52  adcockj
// Reorgainised bt848 initilization
// More Javadoc-ing
//
// Revision 1.1  2001/11/22 13:19:37  temperton
// Added CPaintingHDC class for double buffering painting
//
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __PAINTINGHDC_H__
#define __PAINTINGHDC_H__


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

private:
    HDC m_hBufferDC;
    HDC m_hOriginalDC;
    RECT m_Rect;
    HGDIOBJ m_hSaveBmp;
    HBITMAP m_hBmp;
};


#endif