// Searches 1 pixel to the left and right, in both the old
//  and new fields, but takes horizontal half pel averages. These are odd
// pixel addresses. Any chroma match will not be used. (YUY2)
		__asm
		{
        MERGE4PIXavgH(edi+ecx-2, edi+ecx, esi+ecx, esi+ecx+2) // left, right
        MERGE4PIXavgH(edi+ecx+2, edi+ecx, esi+ecx, esi+ecx-2) // right, left
		}
#include "SearchLoopOddA2.inc"