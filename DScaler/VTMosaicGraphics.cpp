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
 * @file VTMosaicGraphics.cpp CVTMosaicGraphics Implementation
 */

#include "stdafx.h"
#include "VTMosaicGraphics.h"


BYTE CVTMosaicGraphics::m_G1MosaicSet[64] =
{
    0x00, 0x20, 0x10, 0x30, 0x08, 0x28, 0x18, 0x38, 0x04, 0x24, 0x14, 0x34, 0x0C, 0x2C, 0x1C, 0x3C,
    0x02, 0x22, 0x12, 0x32, 0x0A, 0x2A, 0x1A, 0x3A, 0x06, 0x26, 0x16, 0x36, 0x0E, 0x2E, 0x1E, 0x3E,
    0x01, 0x21, 0x11, 0x31, 0x09, 0x29, 0x19, 0x39, 0x05, 0x25, 0x15, 0x35, 0x0D, 0x2D, 0x1D, 0x3D,
    0x03, 0x23, 0x13, 0x33, 0x0B, 0x2B, 0x1B, 0x3B, 0x07, 0x27, 0x17, 0x37, 0x0F, 0x2F, 0x1F, 0x3F,
};


BYTE CVTMosaicGraphics::m_xmgraf[2][2] =
{
    0, 0,
    0, 1,
};


BYTE CVTMosaicGraphics::m_ymgraf[3][3] =
{
    0, 0, 0,
    0, 0, 1,
    0, 1, 1,
};


BYTE CVTMosaicGraphics::m_BitTable[3][2] =
{
    0x20, 0x10,
    0x08, 0x04,
    0x02, 0x01,
};


void CVTMosaicGraphics::DrawG1Mosaic(HDC hDC, LPRECT lpRect, BYTE uChar,
                                      HBRUSH hBrush, BOOL bSeparated)
{
    int x_m = (lpRect->right - lpRect->left) % 2;
    int y_m = (lpRect->bottom - lpRect->top) % 3;
    int x_d = (lpRect->right - lpRect->left) / 2;
    int y_d = (lpRect->bottom - lpRect->top) / 3;
    int w, h;

    uChar = (uChar & 0x1f) | ((uChar & 0x40) >> 1);

    for (int x = 0; x < 2; x++)
    {
        for (int y = 0; y < 3; y++)
        {
            if (m_G1MosaicSet[uChar] & m_BitTable[y][x])
            {
                RECT rect;
                rect.left = lpRect->left + (x ? (x_d * x + m_xmgraf[x_m][x-1]) : 0);
                rect.top = lpRect->top + (y ? (y_d * y + m_ymgraf[y_m][y-1]) : 0);

                w = x_d + m_xmgraf[x_m][x];
                h = y_d + m_ymgraf[y_m][y];

                if (bSeparated)
                {
                    w = w * 3 / 4;
                    h = h * 3 / 4;
                }

                rect.right = rect.left + w;
                rect.bottom = rect.top + h;

                FillRect(hDC, &rect, hBrush);
            }
        }
    }
}
