/////////////////////////////////////////////////////////////////////////////
// $Id: TVFormats.h,v 1.3 2001-11-02 17:03:59 adcockj Exp $
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

#ifndef __TVFORMATS_H___
#define __TVFORMATS_H___

enum eVideoFormat
{
    FORMAT_PAL_BDGHI = 0,
    FORMAT_NTSC,
    FORMAT_SECAM,
    FORMAT_PAL_M,
    FORMAT_PAL_N,
    FORMAT_NTSC_J,
    FORMAT_PAL60,
    FORMAT_NTSC50,
    FORMAT_PAL_NC,
    FORMAT_LASTONE,
};

typedef struct
{
    LPCSTR szDesc;
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
    double Fsc;
    BOOL NeedsPLL;
    WORD CC_Clock;
    WORD CC_Gap;
    WORD CC_Line;
    WORD WSS_Line;
} TTVFormat;

TTVFormat* GetTVFormat(eVideoFormat Format);
extern const char* FormatList[FORMAT_LASTONE];


#endif