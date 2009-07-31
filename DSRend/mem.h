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

/**
 * @file mem.h header file for optimized memcpy and aligned malloc.
 */

#if !defined(AFX_MEM_H__27CADF33_4B34_4ED3_B3D9_77791C73CBE9__INCLUDED_)
#define AFX_MEM_H__27CADF33_4B34_4ED3_B3D9_77791C73CBE9__INCLUDED_

typedef void (_cdecl MEMCPY_FUNC)(void* pOutput, void* pInput, size_t nSize);

extern "C"
{
    void memcpyMMX(void* Dest, void* Src, size_t nBytes);
    void memcpySSE(void* Dest, void* Src, size_t nBytes);
    void memcpyAMD(void* Dest, void* Src, size_t nBytes);
    void memcpySimple(void* Dest, void* Src, size_t nBytes);
    void *aligned_malloc(size_t size, size_t alignment);
    void aligned_free(void *x);
}

#endif