/////////////////////////////////////////////////////////////////////////////
// $Id: SearchLoop0A.inc,v 1.1 2003-01-02 13:15:01 adcockj Exp $
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

// Searches just the center pixel, in both the old
//  and new fields, but takes averages. This is an even
// pixel address. Any chroma match will be used. (YUY2)
// We best like finding 0 motion so we will bias everything we found previously
// up by a little, and adjust later

		__asm
		{

#ifdef IS_SSE2
		paddusb xmm7, ONES				// bias toward no motion
#else
		paddusb mm7, ONES				// bias toward no motion
#endif

        MERGE4PIXavg(edi+ecx, esi+ecx)  // center, in old and new

		}
