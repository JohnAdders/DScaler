// Searches 4 pixel to the left and right, in both the old
//  and new fields, but takes averages. These are even
// pixel addresses. Chroma match will be used. (YUY2)

		__asm
		{
        MERGE4PIXavg(edi-8, esi+2*ecx+8)  // up left, down right
        MERGE4PIXavg(edi+8, esi+2*ecx-8)  // up right, down left
        MERGE4PIXavg(edi+ecx-8, esi+ecx+8) // left, right
        MERGE4PIXavg(edi+ecx+8, esi+ecx-8) // right, left
        MERGE4PIXavg(edi+2*ecx-8, esi+8)   // down left, up right
        MERGE4PIXavg(edi+2*ecx+8, esi-8)   // down right, up left   
		}