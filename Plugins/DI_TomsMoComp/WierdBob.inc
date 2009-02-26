        __asm
        {
        
        // First, get and save our possible Bob values
        // Assume our pixels are layed out as follows with x the calc'd bob value
        // and the other pixels are from the current field
        //  
        //          j a b c k        current field
        //            x            calculated line
        //        m d e f n        current field
        //
        // we calc the bob value as:
        //        x2 = either avg(a,f), avg(c,d), avg(b,e), avg(j,n), or avg(k,m)
         
        // selected for the    smallest of abs(a,f), abs(c,d), or abs(b,e), etc.

        // a,f
        movq    mm0, qword ptr[ebx-2]        // value a from top left        
        movq    mm1, qword ptr[ebx+ecx+2]    // value f from bottom right            
        movq    mm6, mm0                    
//        pavgb    mm6, mm1                    // avg(a,f), also best so far
        V_PAVGB (mm6, mm1, mm7, ShiftMask)    // avg(a,f), also best so far
        movq    mm7, mm0
        psubusb    mm7, mm1
        psubusb mm1, mm0
        por        mm7, mm1                    // abs diff, also best so far

        // c,d
        movq    mm0, qword ptr[ebx+2]        // value a from top left        
        movq    mm1, qword ptr[ebx+ecx-2]    // value f from bottom right            
        movq    mm2, mm0                        
//        pavgb    mm2, mm1                    // avg(c,d)
        V_PAVGB (mm2, mm1, mm3, ShiftMask)    // avg(c,d)
        movq    mm3, mm0
        psubusb    mm3, mm1
        psubusb mm1, mm0
        por        mm3, mm1                    // abs(c,d)
        movq    mm1, mm3                    // keep copy

        psubusb mm3, mm7            // nonzero where new weights bigger, else 0
        pxor    mm4, mm4
        pcmpeqb mm3, mm4            // now ff where new better, else 00    
        pcmpeqb    mm4, mm3            // here ff where old better, else 00

        pand    mm1, mm3            // keep only better new avg and abs
        pand    mm2, mm3

        pand    mm6, mm4
        pand    mm7, mm4

        por        mm6, mm2            // and merge new & old vals keeping best
        por        mm7, mm1
        por        mm7, UVMask            // but we know chroma is worthless so far
        pand    mm5, YMask            // mask out chroma from here also

        // j,n
        movq    mm0, qword ptr[ebx-4]        // value j from top left        
        movq    mm1, qword ptr[ebx+ecx+4]    // value n from bottom right            
        movq    mm2, mm0                        
//        pavgb    mm2, mm1                    // avg(j,n)
        V_PAVGB (mm2, mm1, mm3, ShiftMask)    // avg(j,n)
        movq    mm3, mm0
        psubusb    mm3, mm1
        psubusb mm1, mm0
        por        mm3, mm1                    // abs(j-n)
        movq    mm1, mm3                    // keep copy

        psubusb mm3, mm7            // nonzero where new weights bigger, else 0
        pxor    mm4, mm4
        pcmpeqb mm3, mm4            // now ff where new better, else 00    
        pcmpeqb    mm4, mm3            // here ff where old better, else 00

        pand    mm1, mm3            // keep only better new avg and abs
        pand    mm3, mm2

        pand    mm6, mm4
        pand    mm7, mm4

        por        mm6, mm3            // and merge new & old vals keeping best
        por        mm7, mm1            // "

        // k, m
        movq    mm0, qword ptr[ebx+4]        // value k from top right        
        movq    mm1, qword ptr[ebx+ecx-4]    // value n from bottom left            
        movq    mm4, mm0                        
//        pavgb    mm4, mm1                    // avg(k,m)
        V_PAVGB (mm4, mm1, mm3, ShiftMask)    // avg(k,m)

        movq    mm3, mm0
        psubusb    mm3, mm1
        psubusb mm1, mm0
        por        mm3, mm1                    // abs(k,m)
        movq    mm1, mm3                    // keep copy

        movq    mm2, mm4            // avg(k,m)

        psubusb mm3, mm7            // nonzero where new weights bigger, else 0
        pxor    mm4, mm4
        pcmpeqb mm3, mm4            // now ff where new better, else 00    
        pcmpeqb    mm4, mm3            // here ff where old better, else 00

        pand    mm1, mm3            // keep only better new avg and abs
        pand    mm3, mm2

        pand    mm6, mm4
        pand    mm7, mm4

        por        mm6, mm3            // and merge new & old vals keeping best
        por        mm7, mm1            // "

        // b,e
        movq    mm0, qword ptr[ebx]        // value b from top         
        movq    mm1, qword ptr[ebx+ecx]    // value e from bottom 

// We will also calc here the max/min values to later limit comb
// so the max excursion will not exceed the Max_Comb constant

#ifdef SKIP_SEARCH        
        movq    mm2, mm0
//        pminub    mm2, mm1
        V_PMINUB (mm2, mm1, mm4)

//        pmaxub    mm6, mm2            // clip our current results so far to be above this
        V_PMAXUB (mm6, mm2)
        movq    mm2, mm0
        V_PMAXUB (mm2, mm1)
//        pminub    mm6, mm2            // clip our current results so far to be below this
        V_PMINUB (mm6, mm2, mm4)

#else
        movq    mm2, mm0
        movq    mm4, qword ptr[eax]
        psubusb mm2, mm4
        psubusb mm4, mm0
        por        mm4, mm2            // abs diff
        
        movq    mm2, mm1
        movq    mm3, qword ptr[eax+ecx]
        psubusb mm2, mm3
        psubusb mm3, mm1
        por        mm3, mm2            // abs diff
//        pmaxub  mm3, mm4            // top or bottom pixel moved most
        V_PMAXUB (mm3, mm4)            // top or bottom pixel moved most
        psubusb mm3, Max_Mov        // moved more than allowed? or goes to 0?
        pxor    mm4, mm4
        pcmpeqb mm3, mm4            // now ff where low motion, else high motion
        
        movq    mm2, mm0
//        pminub    mm2, mm1
        V_PMINUB (mm2, mm1, mm4)

//        pmaxub    mm6, mm2            // clip our current results so far to be above this
        V_PMAXUB (mm6, mm2)

        psubusb mm2, mm3            // maybe decrease it to 0000.. if no surround motion
        movq    Min_Vals, mm2

        movq    mm2, mm0
        V_PMAXUB (mm2, mm1)
//        pminub    mm6, mm2            // clip our current results so far to be below this
        V_PMINUB (mm6, mm2, mm4)
        paddusb mm2, mm3            // maybe increase it to ffffff if no surround motion
        movq    Max_Vals, mm2
#endif
            
        movq    mm2, mm0                        
//        pavgb    mm2, mm1                    // avg(b,e)
        V_PAVGB (mm2, mm1, mm3, ShiftMask)    // avg(b,e)
                
        movq    mm3, mm0
        psubusb    mm3, mm1
        psubusb mm1, mm0
        por        mm3, mm1            // abs(c,d)
        movq    mm1, mm3            // keep copy of diffs

        pxor    mm4, mm4            
        psubusb mm3, mm7            // nonzero where new weights bigger, else 0
        pcmpeqb mm3, mm4            // now ff where new better, else 00    
        pcmpeqb    mm4, mm3            // here ff where old better, else 00

        pand    mm1, mm3
        pand    mm2, mm3

        pand    mm6, mm4
        pand    mm7, mm4

        por        mm6, mm2            // our x2 value
        por        mm7, mm1            // our x2 diffs
        movq    mm4, mm7            // save as bob uncertainty indicator
        }

