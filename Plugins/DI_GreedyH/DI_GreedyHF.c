/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Tom Barry.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//    This file is subject to the terms of the GNU General Public License as
//    published by the Free Software Foundation.  A copy of this license is
//    included with this software distribution in the file COPYING.  If you
//    do not have a copy, you may obtain a copy by writing to the Free
//    Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//    This software is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details
//
/////////////////////////////////////////////////////////////////////////////

// This is the first version of the Greedy High Motion Deinterlace method I wrote (and kept).
// It doesn't have many of the fancier options but I left it in because it's faster. It runs with
// a field delay of 1 in a single pass with no call needed to UpdateFieldStore. It will be called
// if no special options are needed. The logic is somewhat different than the other rtns.  TRB 7/2001
//
// This version has been modified to be compatible with other DScaler functions such as Auto Pulldown.
// It will now automatically be call if none of the UI check boxes are check and also if we are not
// running on an SSE capable machine (Athlon, Duron, P-III, fast Celeron).


#include "windows.h"
#include "DS_Deinterlace.h"
#include "DI_GreedyHM.h"


#define IS_SSE
#define SSE_TYPE SSE
#define FUNCT_NAME DI_GreedyHF_SSE
#include "DI_GreedyHF.asm"
#undef SSE_TYPE
#undef IS_SSE
#undef FUNCT_NAME

#define IS_3DNOW
#define FUNCT_NAME DI_GreedyHF_3DNOW
#define SSE_TYPE 3DNOW
#include "DI_GreedyHF.asm"
#undef SSE_TYPE
#undef IS_3DNOW
#undef FUNCT_NAME

#define IS_3DNOW
#define SSE_TYPE MMX
#define FUNCT_NAME DI_GreedyHF_MMX
#include "DI_GreedyHF.asm"
#undef SSE_TYPE
#undef IS_3DNOW
#undef FUNCT_NAME

