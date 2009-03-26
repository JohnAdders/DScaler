/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 Tom Barry & John Adcock.  All rights reserved.
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
//
//  (From Tom Barry)
//  Also, this program is "Philanthropy-Ware".  That is, if you like it and
//  feel the need to reward or inspire the author then please feel free (but
//  not obligated) to consider joining or donating to the Electronic Frontier
//  Foundation. This will help keep cyber space free of barbed wire and bullsh*t.
//  See www.eff.org for details
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Just gets the values for a simple weave
/////////////////////////////////////////////////////////////////////////////

__asm
{
    // at the start of this function
    // mm0 = 0
    // mm6 = Bob pixels

    movq mm1, qword ptr[edi+ecx]
    movq mm2, qword ptr[esi+ecx]
    movq mm3, mm1            /* another copy of our pixel1 value */
    movq mm4, mm2            /* another copy of our pixel2 value */

    PABS(mm3, mm4, mm5)

    V_PAVGB (mm1, mm2, mm4, ShiftMask) /* avg of 2 pixels */

    // these must be the values as we exit
    // mm0 = 0
    // mm1 = weave pixels
    // mm3 = "movement" in the centre
    // mm6 = Bob pixels
}
