/////////////////////////////////////////////////////////////////////////////
// $Id: VTMosaicGraphics.h,v 1.1 2002-01-15 11:16:03 temperton Exp $
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
// CVS Log
//
// $Log: not supported by cvs2svn $
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __VTMosaicGraphicsH__
#define __VTMosaicGraphicsH__

#include "stdafx.h"

void DrawGraphChar(HDC hDC, HBRUSH hBrush, int Char, BOOL Separated, int left, int top, int width, int height);
     
#endif
