/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 All rights reserved.
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

/**
 * @file memalloc.cpp implementation file for aligned malloc.
 */
#include "StdAfx.h"
#include "mem.h"

//aligned malloc/free from tom barry
void *aligned_malloc(size_t size, size_t alignment)
{
    struct B { BYTE* yy; };
    size_t algn = 4;
    size_t mask = 0xfffffffc;
    B* pB;

    BYTE* x;
    BYTE* y;

    while (algn < alignment)        // align to next power of 2
    {
        algn <<= 1;
        mask <<= 1;
    }

    x = (BYTE*)malloc(size+algn);
    y = (BYTE*) (((unsigned int) (x+algn) & mask) - 4);
    pB = (B*) y;
    pB->yy = x;
    return  y+4;
}

void aligned_free(void *x)
{
    struct B { BYTE* yy; };
    B* pB = (B*) ((BYTE*)x-4);
    free(pB->yy);
}