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


///////////////////////////////////////////////////////////////////////////////
// This file contains #define directives that control compilation of CPU-specific
// code, mostly deinterlacing functions.  Turning these directives on requires
// that you have Microsoft's "Processor Pack" patch installed on your build system.
// The Processor Pack is available from Microsoft for free:
//
// http://msdn.microsoft.com/vstudio/downloads/ppack/
//
// Note that compiling the code to use a processor-specific feature is safe even
// if your PC doesn't have the feature in question; dTV detects processor types
// at startup and sets flags in the global "CpuFeatureFlags" (see cpu.h for
// the list of flags) which the code uses to determine whether or not to use
// each feature.
///////////////////////////////////////////////////////////////////////////////

/**
 * @file cpu.h cpu Header file
 */

#ifndef __CPU_H__
#define __CPU_H__ 1

#include "DS_ApiCommon.h"

void  CPU_SetupFeatureFlag(void);
extern UINT CpuFeatureFlags;        // TRB 12/20/00 Processor capability flags

#endif
