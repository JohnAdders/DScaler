// Searches 2 pixel to the left and right, in both the old
//  and new fields, but takes averages. These are even
// pixel addresses. Chroma match will be used. (YUY2)
        __asm
        {
        MERGE4PIXavg(edi-4, esi+2*ecx+4)  // up left, down right
        MERGE4PIXavg(edi+4, esi+2*ecx-4)  // up right, down left
        MERGE4PIXavg(edi+ecx-4, esi+ecx+4) // left, right
        MERGE4PIXavg(edi+ecx+4, esi+ecx-4) // right, left
        MERGE4PIXavg(edi+2*ecx-4, esi+4)   // down left, up right
        MERGE4PIXavg(edi+2*ecx+4, esi-4)   // down right, up left   
        }