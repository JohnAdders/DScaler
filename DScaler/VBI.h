/////////////////////////////////////////////////////////////////////////////
// $Id: VBI.h,v 1.9 2003-01-05 18:35:45 laurentg Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
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
// 24 Jul 2000   John Adcock           Original Release
//                                     Translated most code from German
//                                     Combined Header files
//                                     Cut out all decoding
//                                     Cut out digital hardware stuff
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __VBI_H___
#define __VBI_H___

#include "settings.h"

// Get Hold of the FD_50Hz.c file settings
SETTING* VBI_GetSetting(VBI_SETTING Setting);
void VBI_ReadSettingsFromIni();
void VBI_WriteSettingsToIni(BOOL bOptimizeFileAccess);
void VBI_SetMenu(HMENU hMenu);

#define VBI_VT  1
#define VBI_VPS 2
#define VBI_CC  32

#define FPSHIFT 16
#define FPFAC (1<<FPSHIFT)

void VBI_Init();
void VBI_Exit();
void VBI_Init_data(double Frequency);
void VBI_DecodeLine(unsigned char* VBI_Buffer, int line, BOOL IsOdd);
void VBI_AGC(BYTE* Buffer, int start, int stop, int step);

extern BYTE VBI_thresh;
extern BYTE VBIOffset;
extern BOOL bCaptureVBI;
extern int VTStep;
extern int VPSStep;
extern BOOL bSearchHighlight;
#endif
