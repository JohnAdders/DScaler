/////////////////////////////////////////////////////////////////////////////
// $Id: SearchLoopOddA6.inc,v 1.1 2003-01-02 13:15:01 adcockj Exp $
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
// CVS Log
//
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

// Searches 3 pixels to the left and right, in both the old
//  and new fields, but takes averages. These are odd
// pixel addresses. Any chroma match will not be used. (YUY2)
		__asm
		{
        MERGE4PIXavg(edi-6, esi+2*ecx+6)  // up left, down right
        MERGE4PIXavg(edi+6, esi+2*ecx-6)  // up right, down left
        MERGE4PIXavg(edi+ecx-6, esi+ecx+6) // left, right
        MERGE4PIXavg(edi+ecx+6, esi+ecx-6) // right, left
        MERGE4PIXavg(edi+2*ecx-6, esi+6)   // down left, up right
        MERGE4PIXavg(edi+2*ecx+6, esi-6)   // down right, up left   
		}