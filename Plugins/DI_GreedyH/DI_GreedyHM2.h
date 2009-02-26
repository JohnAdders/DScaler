/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Tom Barry.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//    This file is subject to the terms of the GNU General Public License as
//    published by the Free Software Foundation.  A copy of this license is
//    included with this software distribution in the file COPYING.  If you
//    do not have a copy, you may obtain a copy by writing to the Free
//    Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//    This software is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////

// in tight loop some vars are accessed faster in local storage
const __int64 YMask        = 0x00ff00ff00ff00ff;    // to keep only luma
const __int64 UVMask    = 0xff00ff00ff00ff00;    // to keep only chroma
const __int64 ShiftMask = 0xfefffefffefffeff;    // to avoid shifting chroma to luma
const __int64 Ones        = 0xffffffffffffffff;        // constant foxes, no packed not instr
const __int64 EdgeSenseMax = 0x00aa00aa00aa00aa; // 4 170's, 170/256 = 2/3
const __int64 QW256        = 0x0100010001000100;   // 4 256's

__int64    MaxCombW = MaxComb;
__int64 EdgeThresholdW = EdgeThreshold;    
__int64 YMaskW = YMask;
__int64 EdgeSenseW = EdgeSense;
__int64 EdgeSenseMaxW = EdgeSenseMax;
__int64 MotionThresholdW = MotionThreshold;
__int64 MotionSenseW = MotionSense;
