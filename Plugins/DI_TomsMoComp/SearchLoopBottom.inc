        __asm
        {


#ifdef IS_SSE2
//sse2 code deleted for now
#else

// Version for non-SSE2

#ifdef SKIP_SEARCH
        movq    mm0, mm6            // just use the results of our wierd bob
#else


// JA 9/Dec/2002
// failed experiment
// but leave in placeholder for me to play about
#ifdef DONT_USE_STRANGE_BOB
        // Use the best weave if diffs less than 10 as that
        // means the image is still or moving cleanly
        // if there is motion we will clip which will catch anything
        psubusb mm7, FOURS          // sets bits to zero if weave diff < 4
        pxor    mm0, mm0
        pcmpeqb mm7, mm0            // all ff where weave better, else 00
        pcmpeqb mm0, mm7            // all ff where bob better, else 00
        pand    mm0, mm6            // use bob for these pixel values
        pand    mm7, mm5            // use weave for these
        por     mm0, mm7            // combine both
#else
        // Use the better of bob or weave
//      pminub  mm4, TENS           // the most we care about
        V_PMINUB (mm4, TENS, mm0)   // the most we care about

        psubusb mm7, mm4            // foregive that much from weave est?
        psubusb mm7, FOURS          // bias it a bit toward weave
        pxor    mm0, mm0
        pcmpeqb mm7, mm0            // all ff where weave better, else 00
        pcmpeqb mm0, mm7            // all ff where bob better, else 00
        pand    mm0, mm6            // use bob for these pixel values
        pand    mm7, mm5            // use weave for these
        por     mm0, mm7            // combine both
#endif


//      pminub  mm0, Max_Vals       // but clip to catch the stray error
        V_PMINUB (mm0, Max_Vals, mm1) // but clip to catch the stray error
//      pmaxub  mm0, Min_Vals
        V_PMAXUB (mm0, Min_Vals)

#endif


        mov     eax, pDest

#ifdef USE_VERTICAL_FILTER
        movq    mm1, mm0
//      pavgb   mm0, qword ptr[ebx]
        V_PAVGB (mm0, qword ptr[ebx], mm2, ShiftMask)
//      movntq  qword ptr[eax+edx], mm0
        V_MOVNTQ (qword ptr[eax+edx], mm0)
//      pavgb   mm1, qword ptr[ebx+ecx]
        V_PAVGB (mm1, qword ptr[ebx+ecx], mm2, ShiftMask)
        add     eax, dst_pitchw
//      movntq  qword ptr[eax+edx], mm1
        V_MOVNTQ (qword ptr[eax+edx], mm1)
#else

//      movntq  qword ptr[eax+edx], mm0
        V_MOVNTQ (qword ptr[eax+edx], mm0)
#endif

        lea     edx, [edx+8]        // bump offset pointer
        cmp     edx, Last8          // done with line?
        jb      LoopQ               // y
#endif
        
        }   

        // adjust for next line
        pSrc  += src_pitch2;
        pSrcP += src_pitch2;
        pDest += dst_pitch2;
        pBob += src_pitch2;
    }
    
    return 0;
