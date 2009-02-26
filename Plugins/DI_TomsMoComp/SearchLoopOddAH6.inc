// Searches 3 pixels to the left and right, in both the old
//  and new fields, but takes v-half pel averages. These are odd
// pixel addresses. Any chroma match will not be used. (YUY2)
        __asm
        {
        MERGE4PIXavgH(edi-6, edi+ecx-6, esi+ecx+6, esi+2*ecx+6)  // up left, down right
        MERGE4PIXavgH(edi+6, edi+ecx+6, esi+ecx-6, esi+2*ecx-6)   // up right, down left
        MERGE4PIXavgH(edi+2*ecx-6, edi+ecx-6, esi+ecx+6, esi+6)   // down left, up right
        MERGE4PIXavgH(edi+2*ecx+6, edi+ecx+6, esi+ecx-6, esi-6)   // down right, up left   
        }
