// Searches 1 pixel to the left and right, in both the old
//  and new fields, but takes v-half pel averages. These are odd
// pixel addresses. Any chroma match will not be used. (YUY2)
		__asm
		{
        MERGE4PIXavgH(edi-2, edi+ecx-2, esi+ecx+2, esi+2*ecx+2)  // up left, down right
        MERGE4PIXavgH(edi+2, edi+ecx+2, esi+ecx-2, esi+2*ecx-2)   // up right, down left
        MERGE4PIXavgH(edi+2*ecx-2, edi+ecx-2, esi+ecx+2, esi+2)   // down left, up right
        MERGE4PIXavgH(edi+2*ecx+2, edi+ecx+2, esi+ecx-2, esi-2)   // down right, up left   
		}