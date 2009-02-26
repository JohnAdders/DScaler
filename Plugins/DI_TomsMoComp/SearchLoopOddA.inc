// Searches 1 pixel to the left and right, in both the old
//  and new fields, but takes averages. These are odd
// pixel addresses. Any chroma match will not be used. (YUY2)
        __asm
        {
        MERGE4PIXavg(edi-2, esi+2*ecx+2)  // up left, down right
        MERGE4PIXavg(edi+2, esi+2*ecx-2)  // up right, down left
        MERGE4PIXavg(edi+2*ecx-2, esi+2)   // down left, up right
        MERGE4PIXavg(edi+2*ecx+2, esi-2)   // down right, up left   
        }
#include "SearchLoopOddA2.inc"
