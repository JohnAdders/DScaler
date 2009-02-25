///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
//  This file is subject to the terms of the GNU General Public License as
//  published by the Free Software Foundation.  A copy of this license is
//  included with this software distribution in the file COPYING.  If you
//  do not have a copy, you may obtain a copy by writing to the Free
//  Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details
///////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
///////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.6  2003/10/27 10:39:51  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.5  2001/07/27 16:11:32  adcockj
// Added support for new Crash dialog
//
// Revision 1.4  2001/07/13 16:14:55  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.3  2001/07/12 16:16:39  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file CPU.cpp CPU Detection
 *  This file contains #define directives that control compilation of CPU-specific
 *  code, mostly deinterlacing functions.  Turning these directives on requires
 *  that you have Microsoft's "Processor Pack" patch installed on your build system.
 *  The Processor Pack is available from Microsoft for free:
 * 
 *  http://msdn.microsoft.com/vstudio/downloads/ppack/
 * 
 *  Note that compiling the code to use a processor-specific feature is safe even
 *  if your PC doesn't have the feature in question; dTV detects processor types
 *  at startup and sets flags in the global "CpuFeatureFlags" (see cpu.h for
 *  the list of flags) which the code uses to determine whether or not to use
 *  each feature.
 */

#include <stdafx.h>
#include "cpu.h"

// Symbolic constants for feature flags in CPUID standard feature flags 

#define CPUID_STD_FPU          0x00000001
#define CPUID_STD_VME          0x00000002
#define CPUID_STD_DEBUGEXT     0x00000004
#define CPUID_STD_4MPAGE       0x00000008
#define CPUID_STD_TSC          0x00000010
#define CPUID_STD_MSR          0x00000020
#define CPUID_STD_PAE          0x00000040
#define CPUID_STD_MCHKXCP      0x00000080
#define CPUID_STD_CMPXCHG8B    0x00000100
#define CPUID_STD_APIC         0x00000200
#define CPUID_STD_SYSENTER     0x00000800
#define CPUID_STD_MTRR         0x00001000
#define CPUID_STD_GPE          0x00002000
#define CPUID_STD_MCHKARCH     0x00004000
#define CPUID_STD_CMOV         0x00008000
#define CPUID_STD_PAT          0x00010000
#define CPUID_STD_PSE36        0x00020000
#define CPUID_STD_MMX          0x00800000
#define CPUID_STD_FXSAVE       0x01000000
#define CPUID_STD_SSE          0x02000000
#define CPUID_STD_SSE2         0x04000000

// Symbolic constants for feature flags in CPUID extended feature flags

#define CPUID_EXT_3DNOW        0x80000000
#define CPUID_EXT_AMD_3DNOWEXT 0x40000000
#define CPUID_EXT_AMD_MMXEXT   0x00400000


UINT CpuFeatureFlags = 0;
//---------------------------------------------------------------------------
// Get features of our CPU - modified from sample code from AMD & Intel

void CPU_SetupFeatureFlag(void)
{
   UINT signature = 0;
    char vendor[13]        = "UnknownVendr";  // Needs to be exactly 12 chars 

   // Define known vendor strings here 

   char vendorAMD[13]     = "AuthenticAMD";  // Needs to be exactly 12 chars 
   char vendorIntel[13]   = "GenuineIntel";  // Needs to be exactly 12 chars 

   // Step 1: Check if processor has CPUID support. Processor faults
   // with an illegal instruction exception if the instruction is not
   // supported. This step catches the exception and immediately returns
   // with feature string bits with all 0s, if the exception occurs.
    __try 
    {
        __asm xor    eax, eax
        __asm xor    ebx, ebx
        __asm xor    ecx, ecx
        __asm xor    edx, edx
        __asm cpuid
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return;
    }
    
    CpuFeatureFlags |= FEATURE_CPUID;

    _asm 
    {
         // Step 2: Check if CPUID supports function 1 (signature/std features)
         xor     eax, eax                      // CPUID function #0
         cpuid                                 // largest std func/vendor string
         mov     dword ptr [vendor], ebx       // save 
         mov     dword ptr [vendor+4], edx     //  vendor
         mov     dword ptr [vendor+8], ecx     //   string
         test    eax, eax                      // largest standard function==0?
         jz      $no_standard_features         // yes, no standard features func
         or      [CpuFeatureFlags], FEATURE_STD_FEATURES// does have standard features

         // Step 3: Get standard feature flags and signature
         mov     eax, 1                        // CPUID function #1 
         cpuid                                 // get signature/std feature flgs
         mov     [signature], eax              // save processor signature

         // Step 4: Extract desired features from standard feature flags
         // Check for time stamp counter support
         mov     ecx, CPUID_STD_TSC            // bit 4 indicates TSC support
         and     ecx, edx                      // supports TSC ? CPUID_STD_TSC:0
         neg     ecx                           // supports TSC ? CY : NC
         sbb     ecx, ecx                      // supports TSC ? 0xffffffff:0
         and     ecx, FEATURE_TSC              // supports TSC ? FEATURE_TSC:0
         or      [CpuFeatureFlags], ecx                 // merge into feature flags

         // Check for MMX support
         mov     ecx, CPUID_STD_MMX            // bit 23 indicates MMX support
         and     ecx, edx                      // supports MMX ? CPUID_STD_MMX:0
         neg     ecx                           // supports MMX ? CY : NC
         sbb     ecx, ecx                      // supports MMX ? 0xffffffff:0  
         and     ecx, FEATURE_MMX              // supports MMX ? FEATURE_MMX:0
         or      [CpuFeatureFlags], ecx                 // merge into feature flags

         // Check for CMOV support
         mov     ecx, CPUID_STD_CMOV           // bit 15 indicates CMOV support
         and     ecx, edx                      // supports CMOV?CPUID_STD_CMOV:0
         neg     ecx                           // supports CMOV ? CY : NC
         sbb     ecx, ecx                      // supports CMOV ? 0xffffffff:0
         and     ecx, FEATURE_CMOV             // supports CMOV ? FEATURE_CMOV:0
         or      [CpuFeatureFlags], ecx                 // merge into feature flags
         
         // Check support for P6-style MTRRs
         mov     ecx, CPUID_STD_MTRR           // bit 12 indicates MTRR support
         and     ecx, edx                      // supports MTRR?CPUID_STD_MTRR:0
         neg     ecx                           // supports MTRR ? CY : NC
         sbb     ecx, ecx                      // supports MTRR ? 0xffffffff:0
         and     ecx, FEATURE_P6_MTRR          // supports MTRR ? FEATURE_MTRR:0
         or      [CpuFeatureFlags], ecx                 // merge into feature flags

         // Check for initial SSE support. There can still be partial SSE
         // support. Step 9 will check for partial support.
         mov     ecx, CPUID_STD_SSE            // bit 25 indicates SSE support
         and     ecx, edx                      // supports SSE ? CPUID_STD_SSE:0
         neg     ecx                           // supports SSE ? CY : NC
         sbb     ecx, ecx                      // supports SSE ? 0xffffffff:0
         and     ecx, (FEATURE_MMXEXT+FEATURE_SSEFP+FEATURE_SSE) // supports SSE ? 
                                               // FEATURE_MMXEXT+FEATURE_SSEFP:0
         or      [CpuFeatureFlags], ecx                 // merge into feature flags

         // Check for SSE2 support. (TRB - Was not part of sample code)
         mov     ecx, CPUID_STD_SSE2            // bit 26 indicates SSE2 support
         and     ecx, edx                      // supports SSE2 ? CPUID_STD_SSE2:0
         neg     ecx                           // supports SSE2 ? CY : NC
         sbb     ecx, ecx                      // supports SSE2 ? 0xffffffff:0
         and     ecx, (FEATURE_SSE2)           // supports SSE2 ? 
                                               // FEATURE_SSE2:0
         or      [CpuFeatureFlags], ecx                 // merge into feature flags

         // Step 5: Check for CPUID extended functions
         mov     eax, 0x80000000               // extended function 0x80000000
         cpuid                                 // largest extended function
         cmp     eax, 0x80000000               // no function > 0x80000000 ?
         jbe     $no_extended_features         // yes, no extended feature flags
         or      [CpuFeatureFlags], FEATURE_EXT_FEATURES// does have ext. feature flags

         // Step 6: Get extended feature flags 
         mov     eax, 0x80000001               // CPUID ext. function 0x80000001
         cpuid                                 // EDX = extended feature flags
         
         // Step 7: Extract vendor independent features from extended flags 
         // Check for 3DNow! support (vendor independent)
         mov     ecx, CPUID_EXT_3DNOW          // bit 31 indicates 3DNow! supprt
         and     ecx, edx                      // supp 3DNow! ?CPUID_EXT_3DNOW:0
         neg     ecx                           // supports 3DNow! ? CY : NC
         sbb     ecx, ecx                      // supports 3DNow! ? 0xffffffff:0
         and     ecx, FEATURE_3DNOW            // support 3DNow!?FEATURE_3DNOW:0
         or      [CpuFeatureFlags], ecx                 // merge into feature flags

         // Step 8: Determine CPU vendor
         lea     esi, vendorAMD                // AMD's vendor string
         lea     edi, vendor                   // this CPU's vendor string
         mov     ecx, 12                       // strings are 12 characters
         cld                                   // compare lowest to highest
         repe    cmpsb                         // current vendor str == AMD's ?
         jnz     $not_AMD                      // no, CPU vendor is not AMD

         // Step 9: Check AMD specific extended features
         mov     ecx, CPUID_EXT_AMD_3DNOWEXT   // bit 30 indicates 3DNow! ext.
         and     ecx, edx                      // 3DNow! ext? 
         neg     ecx                           // 3DNow! ext ? CY : NC
         sbb     ecx, ecx                      // 3DNow! ext ? 0xffffffff : 0
         and     ecx, FEATURE_3DNOWEXT         // 3DNow! ext?FEATURE_3DNOWEXT:0
         or      [CpuFeatureFlags], ecx                 // merge into feature flags

         test    [CpuFeatureFlags], FEATURE_MMXEXT      // determined SSE MMX support?
         jnz     $has_mmxext                   // yes, don't need to check again

         // Check support for AMD's multimedia instruction set additions 

         mov     ecx, CPUID_EXT_AMD_MMXEXT     // bit 22 indicates MMX extension
         and     ecx, edx                      // MMX ext?CPUID_EXT_AMD_MMXEXT:0
         neg     ecx                           // MMX ext? CY : NC
         sbb     ecx, ecx                      // MMX ext? 0xffffffff : 0
         and     ecx, FEATURE_MMXEXT           // MMX ext ? FEATURE_MMXEXT:0
         or      [CpuFeatureFlags], ecx                 // merge into feature flags
        
$has_mmxext:

         // Step 10: Check AMD-specific features not reported by CPUID
         // Check support for AMD-K6 processor-style MTRRs          
         mov     eax, [signature]   // get processor signature
         and     eax, 0xFFF         // extract family/model/stepping
         cmp     eax, 0x588         // CPU < AMD-K6-2/CXT ? CY : NC
         sbb     edx, edx       // CPU < AMD-K6-2/CXT ? 0xffffffff:0
         not     edx            // CPU < AMD-K6-2/CXT ? 0:0xffffffff
         cmp     eax, 0x600         // CPU < AMD Athlon ? CY : NC
         sbb     ecx, ecx       // CPU < AMD-K6 ? 0xffffffff:0
         and     ecx, edx       // (CPU>=AMD-K6-2/CXT)&& 
                    // (CPU<AMD Athlon) ? 0xffffffff:0
         and     ecx, FEATURE_K6_MTRR   // (CPU>=AMD-K6-2/CXT)&& 
                    // (CPU<AMD Athlon) ? FEATURE_K6_MTRR:0
         or      [CpuFeatureFlags], ecx         // merge into feature flags

         jmp     $all_done      // desired features determined

$not_AMD:
         // Extract features specific to non AMD CPUs (except SSE2 already done)
$no_extended_features:
$no_standard_features:
$all_done:
   }
}
