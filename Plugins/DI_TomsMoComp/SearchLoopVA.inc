// Searches the center vertical line above center and below, in both the old
// and new fields, but takes averages.  These are even pixel addresses.
        __asm
        {
        MERGE4PIXavg(edi+2*ecx, esi)    // down, up
        MERGE4PIXavg(edi, esi+2*ecx)    // up, down
        }