/////////////////////////////////////////////////////////////////////////////
// FieldTiming.h
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
//
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 13 Apr 2001   John Adcock           Split timing code out of OutThreads.c
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __FIELDTIMING_H___
#define __FIELDTIMING_H___

#include "dTV_Deinterlace.h"

void Timing_Setup();
void Timing_WaitForNextField(DEINTERLACE_INFO* pInfo);
void Timing_Reset();
void Timing_WaitForTimeToFlip(DEINTERLACE_INFO* pInfo, DEINTERLACE_METHOD* CurrentMethod, BOOL* bStopThread);
int Timing_GetDroppedFields();
void Timing_ResetDroppedFields();
int Timing_GetUsedFields();
void Timing_ResetUsedFields();

#endif
