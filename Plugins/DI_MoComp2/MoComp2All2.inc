/////////////////////////////////////////////////////////////////////////////
// $Id: MoComp2All2.inc,v 1.1 2003-01-02 13:15:01 adcockj Exp $
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

#define SEARCH_EFFORT_FUNC(n) Search_Effort_##n

int SEARCH_EFFORT_FUNC(0)()		// we don't try at all ;-)
{
		//see Search_Effort_Max for comments
#define SKIP_SEARCH
#include "SearchLoopTop.inc"	
#include "SearchLoopBottom.inc"
#undef SKIP_SEARCH
}

int SEARCH_EFFORT_FUNC(1)()
{
		//see Search_Effort_Max for comments
#include "SearchLoopTop.inc"	
	RESET_CHROMA		// pretend chroma diffs was 255 each
#include "SearchLoop0A.inc"
#include "SearchLoopBottom.inc"
}

int SEARCH_EFFORT_FUNC(3)()
{
		//see Search_Effort_Max for comments
#include "SearchLoopTop.inc"	
#include "SearchLoopOddA2.inc"
	RESET_CHROMA		// pretend chroma diffs was 255 each
#include "SearchLoop0A.inc"
#include "SearchLoopBottom.inc"
}

int SEARCH_EFFORT_FUNC(5)()
{
		//see Search_Effort_Max for comments
#include "SearchLoopTop.inc"	
#include "SearchLoopOddA2.inc"
#include "SearchLoopOddAH2.inc"
	RESET_CHROMA		// pretend chroma diffs was 255 each
#include "SearchLoop0A.inc"
#include "SearchLoopBottom.inc"
}

// 3x3 search
int SEARCH_EFFORT_FUNC(9)()
{
		//see SearchEffortMax() for comments
#include "SearchLoopTop.inc"	
#include "SearchLoopOddA.inc"
	RESET_CHROMA			// pretend chroma diffs was 255 each
#include "SearchLoopVA.inc"
#include "SearchLoop0A.inc"
#include "SearchLoopBottom.inc"
}

// Search 9 with 2 H-half pels added
int SEARCH_EFFORT_FUNC(11)()
{
		//see SearchEffortMax() for comments
#include "SearchLoopTop.inc"	
#include "SearchLoopOddA.inc"
#include "SearchLoopOddAH2.inc"
	RESET_CHROMA			// pretend chroma diffs was 255 each
#include "SearchLoopVA.inc"
#include "SearchLoop0A.inc"
#include "SearchLoopBottom.inc"
}

// Search 11 with 2 V-half pels added
int SEARCH_EFFORT_FUNC(13)()
{
		//see SearchEffortMax() for comments
#include "SearchLoopTop.inc"	
#include "SearchLoopOddA.inc"
#include "SearchLoopOddAH2.inc"
	RESET_CHROMA			// pretend chroma diffs was 255 each
#include "SearchLoopVAH.inc"
#include "SearchLoopVA.inc"
#include "SearchLoop0A.inc"
#include "SearchLoopBottom.inc"
}

// 5x3
int SEARCH_EFFORT_FUNC(15)()
{
		//see SearchEffortMax() for comments
#include "SearchLoopTop.inc"	
#include "SearchLoopOddA.inc"	
	RESET_CHROMA			// pretend chroma diffs was 255 each
#include "SearchLoopEdgeA.inc"	
#include "SearchLoopVA.inc"
#include "SearchLoop0A.inc"
#include "SearchLoopBottom.inc"
}

// 5x3 + 4 half pels
int SEARCH_EFFORT_FUNC(19)()
{
		//see SearchEffortMax() for comments
#include "SearchLoopTop.inc"	
#include "SearchLoopOddA.inc"	
#include "SearchLoopOddAH2.inc"
	RESET_CHROMA			// pretend chroma diffs was 255 each
#include "SearchLoopEdgeA.inc"	
#include "SearchLoopVAH.inc"
#include "SearchLoopVA.inc"
#include "SearchLoop0A.inc"
#include "SearchLoopBottom.inc"
}



// Handle one 4x1 block of pixels
// Search a 7x3 area, no half pels

int SEARCH_EFFORT_FUNC(21)()
{
		//see SearchLoopTop.inc for comments
#include "SearchLoopTop.inc"
		
		// odd addresses -- the pixels at odd address wouldn't generate 
		// good luma values but we will mask those off

#include "SearchLoopOddA6.inc"  // 4 odd v half pels, 3 to left & right
#include "SearchLoopOddA.inc"   // 6 odd pels, 1 to left & right
		
	RESET_CHROMA		// pretend chroma diffs was 255 each

		// even addresses -- use both luma and chroma from these
		// search averages of 2 pixels left and right
#include "SearchLoopEdgeA.inc"
		// search vertical line and averages, -1,0,+1
#include "SearchLoopVA.inc"
		// blend our results and loop
#include "SearchLoop0A.inc"
#include "SearchLoopBottom.inc"
}


// Handle one 4x1 block of pixels
// Search a 9x3 area, no half pels
int SEARCH_EFFORT_FUNC(Max)()
{
		//see SearchLoopTop.inc for comments
#include "SearchLoopTop.inc"
		
		// odd addresses -- the pixels at odd address wouldn't generate 
		// good luma values but we will mask those off

#include "SearchLoopOddA6.inc"  // 4 odd v half pels, 3 to left & right
#include "SearchLoopOddA.inc"   // 6 odd pels, 1 to left & right
		
	RESET_CHROMA		// pretend chroma diffs was 255 each

		// even addresses -- use both luma and chroma from these
		// search averages of 4 pixels left and right
#include "SearchLoopEdgeA8.inc"
		// search averages of 2 pixels left and right
#include "SearchLoopEdgeA.inc"
		// search vertical line and averages, -1,0,+1
#include "SearchLoopVA.inc"
		// blend our results and loop
#include "SearchLoop0A.inc"
#include "SearchLoopBottom.inc"
}

#undef SEARCH_EFFORT_FUNC