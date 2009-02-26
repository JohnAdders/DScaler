// Searches 2 pixel to the left and right, in both the old
//  and new fields, but takes half pel averages. These are even
// pixel addresses. Chroma match will be used. (YUY2)

          __asm
        {
        MERGE4PIXavgH(edi-4, edi+ecx-4, esi+ecx+4, esi+2*ecx+4)  // up left, down right
        MERGE4PIXavgH(edi+4, edi+ecx+4, esi+ecx-4, esi+2*ecx-4)  // up right, down left
        MERGE4PIXavgH(edi+2*ecx-4, edi+ecx-4, esi+ecx+4, esi+4)  // down left, up right
        MERGE4PIXavgH(edi+2*ecx+4, edi+ecx+4, esi+ecx-4, esi-4)  // down right, up left   
        }