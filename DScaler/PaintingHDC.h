/////////////////////////////////////////////////////////////////////////////
// $Id: PaintingHDC.h,v 1.1 2001-11-22 13:19:37 temperton Exp $
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
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __PAINTINGHDC_H__
#define __PAINTINGHDC_H__

class CPaintingHDC
{
public:
    CPaintingHDC();
    ~CPaintingHDC();

    HDC BeginPaint(HDC hDC, LPRECT pRect);
    EndPaint(void);

    void SetEnabled(BOOL bEnabled);
private:
    HDC m_hBufferDC;
    HDC m_hOriginalDC;
    RECT m_Rect;
    HGDIOBJ m_hSave;
    HBITMAP m_hBmp;
    BOOL m_bEnabled;
};


#endif