// Searches the center vertical line above center and below, in both the old
// and new fields, but takes averages.  These are even pixel addresses.
        __asm
        {
        MERGE4PIXavgH(edi+2*ecx, edi+ecx, esi+ecx, esi)    // down, up
        MERGE4PIXavgH(edi, edi+ecx, esi+ecx, esi+2*ecx)    // up, down
        }