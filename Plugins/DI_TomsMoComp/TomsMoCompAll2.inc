
int Search_Effort_0()		// we don't try at all ;-)
{
		//see Search_Effort_Max for comments
#define SKIP_SEARCH
#define USE_STRANGE_BOB
#include "SearchLoopTop.inc"	
#include "SearchLoopBottom.inc"
#undef SKIP_SEARCH
}

int Search_Effort_1()
{
		//see Search_Effort_Max for comments
#include "SearchLoopTop.inc"	
	RESET_CHROMA		// pretend chroma diffs was 255 each
#include "SearchLoop0A.inc"
#include "SearchLoopBottom.inc"
}

int Search_Effort_3()
{
		//see Search_Effort_Max for comments
#include "SearchLoopTop.inc"	
#include "SearchLoopOddA2.inc"
	RESET_CHROMA		// pretend chroma diffs was 255 each
#include "SearchLoop0A.inc"
#include "SearchLoopBottom.inc"
}

int Search_Effort_5()
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
int Search_Effort_9()
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
int Search_Effort_11()
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
int Search_Effort_13()
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
int Search_Effort_15()
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
int Search_Effort_19()
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

int Search_Effort_21()
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
int Search_Effort_Max()
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


int Fieldcopy(void *dest, const void *src, size_t count, 
				int rows, int dst_pitch, int src_pitch)
{
BYTE* pDest = (BYTE*) dest;
BYTE* pSrc = (BYTE*) src;
int i;
	
	for (i=0; i < rows; i++)
	{
		MyMemCopy(pDest, pSrc, count);
		pSrc += src_pitch;
		pDest += dst_pitch;
	}
	return 0;
}



