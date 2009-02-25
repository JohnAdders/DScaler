/////////////////////////////////////////////////////////////////////////////
// $Id$
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
// 29 Dec 2002   Atsushi Nakagawa      Moved VPS stuff out of VBI_VideoText.*
//
/////////////////////////////////////////////////////////////////////////////

/** 
 * @file vbi_vpsdecode.h vbi_vpsdecode Header file
 */
 
#ifndef __VBI_VPSDECODE_H___
#define __VBI_VPSDECODE_H___

enum eVPSAudio
{
    VPSAUDIO_UNKNOWN,
    VPSAUDIO_MONO,
    VPSAUDIO_STEREO,
    VPSAUDIO_DUAL,
};

// VPS data
typedef struct 
{
    BOOL Valid;
    WORD CNI;
    char LabelTemp[9];
    char LabelLast[9];
    char LabelCurr[9];
    int  LabelIndex;
    int  Month;
    int  Day;
    int  Hour;
    int  Minute;
    eVPSAudio Audio;
    BYTE PTY;
} TVPSDataStruct;

void VBI_VPS_Init();
void VBI_VPS_Exit();
void VPS_Init_Data(double VBI_Frequency);
void VPS_Clear_Data();

void VBI_DecodeLine_VPS(BYTE* VBI_Buffer);
void VPS_DecodeLabel(BYTE b);

BYTE VBI_Scan(BYTE* VBI_Buffer, UINT step, UINT* scanPos);
BYTE VPS_Scan(const BYTE* VBI_Buffer, UINT step, UINT* scanPos, BOOL* pError);

void VPS_ChannelChange();
void VPS_GetChannelName(LPSTR lpBuffer, LONG nLength);
void VPS_GetChannelNameFromCNI(LPSTR lpBuffer, LONG nLength);

BYTE ReverseBitOrder( BYTE b );


#endif