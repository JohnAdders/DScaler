/////////////////////////////////////////////////////////////////////////////
// $Id: FD_CommonFunctions.h,v 1.7 2001-11-22 13:32:03 adcockj Exp $
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
//
// Change Log
//
// Date          Developer             Changes
//
// 01 Jun 2001   John Adcock           Split into new file
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __FD_COMMON_FUNCTIONS_H___
#define __FD_COMMON_FUNCTIONS_H___

extern "C"
{
    WORD CalcCombFactorLine(BYTE* YVal1, BYTE* YVal2, BYTE* YVal3, long BytesToProcess);
    WORD CalcCombFactorLineChroma(BYTE* YVal1, BYTE* YVal2, BYTE* YVal3, long BytesToProcess);
    DWORD CalcDiffFactorLine(BYTE* YVal1, BYTE* YVal2, long BytesToProcess);
    DWORD CalcDiffFactorLineChroma(BYTE* YVal1, BYTE* YVal2, long BytesToProcess);
    void memcpyBOBMMX(void* Dest1, void* Dest2, void* Src, size_t nBytes);
    void memcpyBOBSSE(void* Dest1, void* Dest2, void* Src, size_t nBytes);
    void memcpyMMX(void* Dest, void* Src, size_t nBytes);
    void memcpySSE(void* Dest, void* Src, size_t nBytes);
    void memcpyAMD(void* Dest, void* Src, size_t nBytes);
    void memcpySimple(void* Dest, void* Src, size_t nBytes);
}

#endif