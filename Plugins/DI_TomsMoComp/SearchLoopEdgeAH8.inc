// Searches 4pixel to the left and right, in both the old
// and new fields, but takes half pel averages. These are even
// pixel addresses. Chroma match will be used. (YUY2)
		__asm
		{
        MERGE4PIXavgH(edi-8, edi+ecx-8, esi+ecx+8, esi+2*ecx+8)  // up left, down right
        MERGE4PIXavgH(edi+8, edi+ecx+8, esi+ecx-8, esi+2*ecx-8)  // up right, down left
        MERGE4PIXavgH(edi+2*ecx-8, edi+ecx-8, esi+ecx+8, esi+8)  // down left, up right
        MERGE4PIXavgH(edi+2*ecx+8, edi+ecx+8, esi+ecx-8, esi-8)  // down right, up left   
		}
