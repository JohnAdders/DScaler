/////////////////////////////////////////////////////////////////////////////
// $Id: VBI_WSSdecode.cpp,v 1.16 2003-10-27 10:39:54 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000-2003 Laurent Garnier.  All rights reserved.
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
//
//  GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
//
//  This is the WideScreen Signaling DECODER.
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 11 Mar 2001   Laurent Garnier       New file
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.15  2003/01/05 18:59:50  laurentg
// WSS decoding working for each VBI frequency
//
// Revision 1.14  2003/01/05 18:35:45  laurentg
// Init function for VBI added
//
// Revision 1.13  2003/01/05 12:42:52  laurentg
// WSS decoding updated to take into account a VBI frequency of 27.0 MHz
//
// Revision 1.12  2003/01/04 20:13:32  laurentg
// Update range for start position
//
// Revision 1.11  2003/01/01 20:32:39  atnak
// Renamed DecodeLine function
//
// Revision 1.10  2002/04/28 16:45:56  laurentg
// Unused code suppressed
//
// Revision 1.9  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.8  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.7.2.2  2001/08/21 09:43:01  adcockj
// Brought branch up to date with latest code fixes
//
// Revision 1.7.2.1  2001/08/18 17:09:30  adcockj
// Got to compile, still lots to do...
//
// Revision 1.7  2001/08/02 16:43:05  adcockj
// Added Debug level to LOG function
//
// Revision 1.6  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.5  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file VBI_WSSdecode.cpp VBI WSS functions
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "VBI_WSSdecode.h"
#include "VBI.h"
#include "Providers.h"
#include "DebugLog.h"

#define WSS_MAX_SUCCESSIVE_ERR      6

// Structure of WSS signal for 625-line systems
#define WSS625_RUNIN_CODE_LENGTH    29
#define WSS625_START_CODE_LENGTH    24
#define WSS625_DATA_BIT_LENGTH      6
#define WSS625_NB_DATA_BITS         14
#define WSS625_START_POS_MIN        64
#define WSS625_START_POS_MAX        200
#define WSS625_MIN_THRESHOLD        0x55

// Possible ratio values for 625-line systems
#define WSS625_RATIO_133                    0x08
#define WSS625_RATIO_155                    0x0e
#define WSS625_RATIO_177_ANAMORPHIC         0x07
#define WSS625_RATIO_155_LETTERBOX_CENTER   0x01
#define WSS625_RATIO_155_LETTERBOX_TOP      0x02
#define WSS625_RATIO_177_LETTERBOX_CENTER   0x0b
#define WSS625_RATIO_177_LETTERBOX_TOP      0x04
#define WSS625_RATIO_BIG_LETTERBOX_CENTER   0x0d

// Possible ratio values for 525-line systems
#define WSS525_RATIO_133                    0x00
#define WSS525_RATIO_177_ANAMORPHIC         0x01
#define WSS525_RATIO_133_LETTERBOX          0x02

#define AR_NONANAMORPHIC 1
#define AR_ANAMORPHIC    2

extern int decodebit(unsigned char* data, int threshold, int NumPixels);

// WSS decoded data
TWSSDataStruct WSS_Data = { -1,-1,FALSE,FALSE,FALSE,FALSE,WSS625_SUBTITLE_NO,FALSE,FALSE,FALSE };

// WSS control data
TWSSCtrlDataStruct WSS_CtrlData = { FALSE,0,0,WSS_MAX_SUCCESSIVE_ERR,WSS625_START_POS_MAX,WSS625_START_POS_MIN,0,0,-1,-1};

// Number of pixels to represent one bit of data
static int	BitLength;

// Offsets of the starting for each bit of data
static int	BitOffsets[140];

// Sequence values for run-in code
static int WSS625_runin[WSS625_RUNIN_CODE_LENGTH] = { 1,1,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1 };

// Sequence values for start code
static int WSS625_start[WSS625_START_CODE_LENGTH] = { 0,0,0,1,1,1,1,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,1 };

// Sequence values for a data bit = 0
static int WSS625_0[WSS625_DATA_BIT_LENGTH] = { 0,0,0,1,1,1 };

// Sequence values for a data bit = 1
static int WSS625_1[WSS625_DATA_BIT_LENGTH] = { 1,1,1,0,0,0 };



void WSS_Init_Data(double VBI_Frequency)
{
	int i;

	BitLength = (int)(VBI_Frequency / 5.0);
	for (i = 0 ; i < 140 ; i++)
	{
		BitOffsets[i] = (int)(VBI_Frequency / 5.0 * i + 0.5);
	}
}

// Clear WSS decoded data
static void WSS_clear_data ()
{
    WSS_Data.AspectRatio = -1;
    WSS_Data.AspectMode = -1;
    WSS_Data.FilmMode = FALSE;
    WSS_Data.ColorPlus = FALSE;
    WSS_Data.HelperSignals = FALSE;
    WSS_Data.TeletextSubtitle = FALSE;
    WSS_Data.OpenSubtitles = WSS625_SUBTITLE_NO;
    WSS_Data.SurroundSound = FALSE;
    WSS_Data.CopyrightAsserted = FALSE;
    WSS_Data.CopyProtection = FALSE;
}

// Clear WSS decoded data and WSS control data
void WSS_init ()
{
    // Clear WSS control data
    WSS_CtrlData.DecodeStatus = WSS_STATUS_ERROR;
    WSS_CtrlData.NbDecodeErr = 0;
    WSS_CtrlData.NbDecodeOk = 0;
    WSS_CtrlData.NbSuccessiveErr = WSS_MAX_SUCCESSIVE_ERR;
    WSS_CtrlData.MinPos = WSS625_START_POS_MAX;
    WSS_CtrlData.MaxPos = WSS625_START_POS_MIN;
    WSS_CtrlData.TotalPos = 0;
    WSS_CtrlData.NbErrPos = 0;
    WSS_CtrlData.AspectRatioWhenErr = -1;
    WSS_CtrlData.AspectModeWhenErr = -1;

    // Clear WSS decoded data
    WSS_clear_data ();
}

static BOOL decode_sequence(BYTE* vbiline, int* DecodedVals, int NbVal, int Threshold, int *Offsets, int BitLength)
{
    int i;

    for (i = 0 ; i < NbVal ; i++)
    {
        if (decodebit (vbiline + Offsets[i] - Offsets[0], Threshold, BitLength) != DecodedVals[i])
            break;
    }
    return ( (i == NbVal) ? TRUE : FALSE );
}

static BOOL WSS625_DecodeLine(BYTE* vbiline)
{
    int     i, j, k;
    BOOL    DecodeOk = FALSE;
    int     Threshold = 0;
    int     StartPos;
    int     bits[WSS625_NB_DATA_BITS];
    int     packedbits;
    int     nb;

    Threshold = VBI_thresh;
//  LOG(1, "threshold %x", Threshold);

    if (Threshold < WSS625_MIN_THRESHOLD)
    {
//		LOG(1, "WSS signal threshold too low (%x)", Threshold);
        return FALSE;
    }

    for (i = WSS625_START_POS_MIN ; i <= WSS625_START_POS_MAX ; i++)
    {
        if (vbiline[i] < Threshold)
            continue;

        StartPos = i;
//		LOG(1, "WSS decoding at start position = %d", StartPos);

        // run-in code decoding
        k = 0;
        if (decode_sequence (vbiline + i + BitOffsets[k], WSS625_runin, WSS625_RUNIN_CODE_LENGTH, Threshold, &BitOffsets[k], BitLength))
        {
//			LOG(1, "WSS run-in code detected (start pos = %d, Threshold = %x)", i, Threshold);
            k += WSS625_RUNIN_CODE_LENGTH;

            // Start code decoding
            if (decode_sequence (vbiline + i + BitOffsets[k], WSS625_start, WSS625_START_CODE_LENGTH, Threshold, &BitOffsets[k], BitLength))
            {
//				LOG(1, "WSS start code detected");
                k += WSS625_START_CODE_LENGTH;

                // Data bits decoding
                nb = 0;
                for (j = 0 ; j < WSS625_NB_DATA_BITS ; j++)
                {
                    if (decode_sequence (vbiline + i + BitOffsets[k], WSS625_0, WSS625_DATA_BIT_LENGTH, Threshold, &BitOffsets[k], BitLength))
                    {
                        bits[j] = 0;
                        nb++;
//						LOG(1, "WSS b%d = 0 (start pos = %d, Threshold = %x)", j, i, Threshold);
                    }
                    else if (decode_sequence (vbiline + i + BitOffsets[k], WSS625_1, WSS625_DATA_BIT_LENGTH, Threshold, &BitOffsets[k], BitLength))
                    {
                        bits[j] = 1;
                        nb++;
//						LOG(1, "WSS b%d = 1 (start pos = %d, Threshold = %x)", j, i, Threshold);
                    }
                    else
                    {
                        bits[j] = -1;
//						LOG(1, "WSS b%d = ?", j);
                    }
                    k += WSS625_DATA_BIT_LENGTH;
                }
                if (nb == WSS625_NB_DATA_BITS)
                {
                    DecodeOk = TRUE;
                    break;
                }
            }
        }
        WSS_CtrlData.NbErrPos++;
    }

    if (DecodeOk)
    {
//      LOG(3, "WSS decode OK start pos = %d, Threshold = %x", StartPos, Threshold);
//      k = WSS625_RUNIN_CODE_LENGTH + WSS625_START_CODE_LENGTH;
//      for (i = 0 ; i < WSS625_NB_DATA_BITS ; i++)
//      {
//          for (j = 0 ; j < WSS625_DATA_BIT_LENGTH ; j++)
//          {
//              int pos = StartPos + BitOffsets[k];
//				if (BitLength == 5)
//					LOG(3, "WSS bit b%d => %x %x %x %x %x", i, vbiline[pos], vbiline[pos+1], vbiline[pos+2], vbiline[pos+3], vbiline[pos+4]);
//				else if (BitLength == 7)
//					LOG(3, "WSS bit b%d => %x %x %x %x %x %x %x", i, vbiline[pos], vbiline[pos+1], vbiline[pos+2], vbiline[pos+3], vbiline[pos+4], vbiline[pos+5], vbiline[pos+6]);
//              k++;
//          }
//          LOG(3, "WSS bit b%d = %d", i, bits[i]);
//      }

        // Decoding statistics
        WSS_CtrlData.TotalPos += StartPos;
        if (StartPos < WSS_CtrlData.MinPos)
            WSS_CtrlData.MinPos = StartPos;
        if (StartPos > WSS_CtrlData.MaxPos)
            WSS_CtrlData.MaxPos = StartPos;

        packedbits = 0;
        for (j = 0 ; j < WSS625_NB_DATA_BITS ; j++)
        {
            packedbits |= bits[j]<<j;
        }
        switch (packedbits & 0x000f)
        {
        case WSS625_RATIO_133:
            WSS_Data.AspectMode = AR_NONANAMORPHIC;
            WSS_Data.AspectRatio = 1333;
            break;
        case WSS625_RATIO_177_ANAMORPHIC:
            WSS_Data.AspectMode = AR_ANAMORPHIC;
            WSS_Data.AspectRatio = 1778;
            break;
        case WSS625_RATIO_155:
        case WSS625_RATIO_155_LETTERBOX_CENTER:
        case WSS625_RATIO_155_LETTERBOX_TOP:
            WSS_Data.AspectMode = AR_NONANAMORPHIC;
            WSS_Data.AspectRatio = 1555;
            break;
        case WSS625_RATIO_177_LETTERBOX_CENTER:
        case WSS625_RATIO_177_LETTERBOX_TOP:
            WSS_Data.AspectMode = AR_NONANAMORPHIC;
            WSS_Data.AspectRatio = 1778;
            break;
        case WSS625_RATIO_BIG_LETTERBOX_CENTER:
            WSS_Data.AspectMode = AR_NONANAMORPHIC;
            WSS_Data.AspectRatio = -1;
            break;
        default:
            WSS_Data.AspectMode = -1;
            WSS_Data.AspectRatio = -1;
            break;
        }
        WSS_Data.FilmMode = (packedbits & 0x0010) ? TRUE : FALSE;
        WSS_Data.ColorPlus = (packedbits & 0x0020) ? TRUE : FALSE;
        WSS_Data.HelperSignals = (packedbits & 0x0040) ? TRUE : FALSE;
        WSS_Data.TeletextSubtitle = (packedbits & 0x0100) ? TRUE : FALSE;
        WSS_Data.OpenSubtitles = packedbits & 0x0600;
        WSS_Data.SurroundSound = (packedbits & 0x0800) ? TRUE : FALSE;
        WSS_Data.CopyrightAsserted = (packedbits & 0x1000) ? TRUE : FALSE;
        WSS_Data.CopyProtection = (packedbits & 0x2000) ? TRUE : FALSE;
    }
//	else
//	{
//		LOG(1, "WSS decode ERROR Threshold = %x", Threshold);
//		StartPos = 0;
//		while (vbiline[StartPos] < Threshold && StartPos <= WSS625_START_POS_MAX)
			StartPos++;
//		for (j = 0 ; j < 100 ; j++)
//		{
//			k = StartPos + j * BitLength;
//			if (BitLength == 5)
//				LOG(1, "WSS pos = %d : %x %x %x %x %x", k, vbiline[k], vbiline[k + 1], vbiline[k + 2], vbiline[k + 3], vbiline[k + 4]);
//			else if (BitLength == 7)
//				LOG(1, "WSS pos = %d : %x %x %x %x %x %x %x", k, vbiline[k], vbiline[k + 1], vbiline[k + 2], vbiline[k + 3], vbiline[k + 4], vbiline[k + 5], vbiline[k + 6]);
//		}
//		for (j = 0 ; j < 100 ; j++)
//		{
//			k = StartPos + BitOffsets[j];
//			if (BitLength == 5)
//				LOG(1, "WSS pos = %d : %x %x %x %x %x", k, vbiline[k], vbiline[k + 1], vbiline[k + 2], vbiline[k + 3], vbiline[k + 4]);
//			else if (BitLength == 7)
//				LOG(1, "WSS pos = %d : %x %x %x %x %x %x %x", k, vbiline[k], vbiline[k + 1], vbiline[k + 2], vbiline[k + 3], vbiline[k + 4], vbiline[k + 5], vbiline[k + 6]);
//		}
//	}

    return DecodeOk;
}

static BOOL WSS525_DecodeLine(BYTE* vbiline)
{
    BOOL    DecodeOk = FALSE;
    int     packedbits;

    // !!!!!!!!!!!!!!!!!!!!
    // !!! Code missing !!!
    // !!!!!!!!!!!!!!!!!!!!

    if (DecodeOk)
    {
        packedbits = 0;
        switch (packedbits & 0x0003)
        {
        case WSS525_RATIO_133:
        case WSS525_RATIO_133_LETTERBOX:
            WSS_Data.AspectMode = AR_NONANAMORPHIC;
            WSS_Data.AspectRatio = 1333;
            break;
        case WSS525_RATIO_177_ANAMORPHIC:
            WSS_Data.AspectMode = AR_ANAMORPHIC;
            WSS_Data.AspectRatio = 1778;
            break;
        default:
            WSS_Data.AspectMode = -1;
            WSS_Data.AspectRatio = -1;
            break;
        }
    }

    return DecodeOk;
}

int VBI_DecodeLine_WSS(BYTE* vbiline)
{
    BOOL    bResuDecoding;
    TTVFormat* TVFormat = GetTVFormat(Providers_GetCurrentSource()->GetFormat());
    
    switch (TVFormat->wCropHeight)
    {
    // 625-line systems
    case 576:
        bResuDecoding = WSS625_DecodeLine(vbiline);
        break;

    // 525-line systems
    case 400:
        bResuDecoding = WSS525_DecodeLine(vbiline);
        break;

    default:
        bResuDecoding = FALSE;
        break;
    }

    if (! bResuDecoding)
    {
        WSS_CtrlData.NbDecodeErr++;
        WSS_CtrlData.NbSuccessiveErr++;
        // Clear WSS decoded data
        // after two many successive decoding errors
        if (WSS_CtrlData.NbSuccessiveErr >= WSS_MAX_SUCCESSIVE_ERR)
            WSS_CtrlData.DecodeStatus = WSS_STATUS_ERROR;
        else
            WSS_CtrlData.DecodeStatus = WSS_STATUS_PONCTUAL_ERROR;
        if (WSS_CtrlData.NbSuccessiveErr == WSS_MAX_SUCCESSIVE_ERR)
        {
            WSS_clear_data();
        }
    }
    else
    {
        WSS_CtrlData.DecodeStatus = WSS_STATUS_OK;
        WSS_CtrlData.NbDecodeOk++;
        WSS_CtrlData.NbSuccessiveErr = 0;
    }

    return ((WSS_CtrlData.DecodeStatus == WSS_STATUS_OK) ? 0 : -1);
}

BOOL WSS_GetRecommendedAR (int* pMode, int* pRatio)
{
    if ((WSS_CtrlData.DecodeStatus == WSS_STATUS_ERROR)
        || (WSS_Data.AspectMode == -1) )
        return FALSE;
    else
    {
        *pMode = WSS_Data.AspectMode;
        *pRatio = WSS_Data.AspectRatio;
        return TRUE;
    }
}