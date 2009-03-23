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
 * @file vtmosaicgraphics.h vtmosaicgraphics Header file
 */
 
#ifndef __VTMOSAICGRAPHICS_H___
#define __VTMOSAICGRAPHICS_H___

#include "stdafx.h"


class CVTMosaicGraphics
{
public:
    static void DrawG1Mosaic(HDC hDC, LPRECT lpRect, BYTE uChar,
                             HBRUSH hBrush, BOOL bSeparated);

private:
    static BYTE m_G1MosaicSet[64];

    static BYTE m_BitTable[3][2];

    static BYTE m_xmgraf[2][2];
    static BYTE m_ymgraf[3][3];
};
  
   
#endif
