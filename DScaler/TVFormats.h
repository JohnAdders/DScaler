/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
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
// $Log: not supported by cvs2svn $
// Revision 1.9  2006/12/28 14:18:36  adcockj
// Added patch for Curtiss-Wright cards from Bill Randle
//
// Revision 1.8  2003/10/27 10:39:54  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.7  2003/05/30 10:06:25  adcockj
// Fix for format names in ini file
//
// Revision 1.6  2003/01/07 16:49:08  adcockj
// Changes to allow variable sampling rates for VBI
//
// Revision 1.5  2002/10/18 11:41:47  adcockj
// Updated Fsc values
//
// Revision 1.4  2001/12/05 21:45:11  ittarnavsky
// added changes for the AudioDecoder and AudioControls support
//
/////////////////////////////////////////////////////////////////////////////


/** 
 * @file tvformats.h tvformats Header file
 */
 
#ifndef __TVFORMATS_H___
#define __TVFORMATS_H___

enum eVideoFormat
{
    VIDEOFORMAT_PAL_B = 0,
    VIDEOFORMAT_PAL_D,
    VIDEOFORMAT_PAL_G,
    VIDEOFORMAT_PAL_H,
    VIDEOFORMAT_PAL_I,
    VIDEOFORMAT_PAL_M,
    VIDEOFORMAT_PAL_N,
    VIDEOFORMAT_PAL_60,
    VIDEOFORMAT_PAL_N_COMBO,

    VIDEOFORMAT_SECAM_B,
    VIDEOFORMAT_SECAM_D,
    VIDEOFORMAT_SECAM_G,
    VIDEOFORMAT_SECAM_H,
    VIDEOFORMAT_SECAM_K,
    VIDEOFORMAT_SECAM_K1,
    VIDEOFORMAT_SECAM_L,
    VIDEOFORMAT_SECAM_L1,

    VIDEOFORMAT_NTSC_M,
    VIDEOFORMAT_NTSC_M_Japan,
    VIDEOFORMAT_NTSC_50,

    VIDEOFORMAT_LAST_TV,

    VIDEOFORMAT_RGB_640X480_60 = VIDEOFORMAT_LAST_TV,
    VIDEOFORMAT_RGB_800X600_60,
    VIDEOFORMAT_RGB_1024X768_60,
    VIDEOFORMAT_RGB_640X480_75,
    VIDEOFORMAT_RGB_800X600_75,
    VIDEOFORMAT_RGB_1024X768_75,

    VIDEOFORMAT_LASTONE
};

typedef struct
{
    WORD wCropHeight;
    WORD wTotalWidth;
    BYTE bDelayA;
    BYTE bDelayB;
    BYTE bIForm;
    WORD wHDelayx1;
    WORD wHActivex1;
    WORD wVDelay;
    WORD wCropOffset;
    BOOL Is25fps;
    WORD VBIPacketSize;
    WORD VBILines;
    /** Colour subcarrier frequency in MHz
        For SECAM this is the Db frequency
        for others it is Fsc
    */
    double Fsc;
    /** Frequency at which the bt848 will sample VBI
        Also used by the cx2388x in some modes
    */
    double Bt848VBISamplingFrequency;
    BOOL NeedsPLL;

    WORD CC_Line;
    WORD WSS_Line;
} TTVFormat;

TTVFormat* GetTVFormat(eVideoFormat Format);
bool IsPALVideoFormat(eVideoFormat Format);
bool IsNTSCVideoFormat(eVideoFormat Format);
bool IsSECAMVideoFormat(eVideoFormat Format);
bool IsRGBVideoFormat(eVideoFormat Format);

extern const char* VideoFormatNames[];
extern const char* VideoFormatSaveNames[];


#endif
