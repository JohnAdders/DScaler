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