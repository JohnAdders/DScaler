/////////////////////////////////////////////////////////////////////////////
// $Id: VBI_WSSdecode.cpp,v 1.7 2001-08-02 16:43:05 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 Laurent Garnier.  All rights reserved.
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
// Revision 1.6  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.5  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VBI_WSSdecode.h"
#include "VBI.h"
#include "BT848.h"
#include "DebugLog.h"

#define WSS_MAX_SUCCESSIVE_ERR      6

// Structure of WSS signal for 625-line systems
#define WSS625_RUNIN_CODE_LENGTH    29
#define WSS625_START_CODE_LENGTH    24
#define WSS625_DATA_BIT_LENGTH      6
#define WSS625_NB_DATA_BITS         14
#define WSS625_START_POS_MIN        110
#define WSS625_START_POS_MAX        150
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

// Offsets of each clock pixels (7.09379) in VBI buffer line
static int offsets[] = {   0,   7,  14,  21,  28,  35,  43,  50,  57,  64,
                          71,  78,  85,  92,  99, 106, 114, 121, 128, 135,
                         142, 149, 156, 163, 170, 177, 184, 192, 199, 206,
                         213, 220, 227, 234, 241, 248, 255, 262, 270, 277,
                         284, 291, 298, 305, 312, 319, 326, 333, 341, 348,
                         355, 362, 369, 376, 383, 390, 397, 404, 411, 419,
                         426, 433, 440, 447, 454, 461, 468, 475, 482, 489,
                         497, 504, 511, 518, 525, 532, 539, 546, 553, 560,
                         568, 575, 582, 589, 596, 603, 610, 617, 624, 631,
                         638, 646, 653, 660, 667, 674, 681, 688, 695, 702,
                         709, 716, 724, 731, 738, 745, 752, 759, 766, 773,
                         780, 787, 795, 802, 809, 816, 823, 830, 837, 844,
                         851, 858, 865, 873, 880, 887, 894, 901, 908, 915,
                         922, 929, 936, 943, 951, 958, 965, 972, 979, 986 };

// Sequence values for run-in code
static int WSS625_runin[WSS625_RUNIN_CODE_LENGTH] = { 1,1,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1 };

// Sequence values for start code
static int WSS625_start[WSS625_START_CODE_LENGTH] = { 0,0,0,1,1,1,1,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,1 };

// Sequence values for a data bit = 0
static int WSS625_0[WSS625_DATA_BIT_LENGTH] = { 0,0,0,1,1,1 };

// Sequence values for a data bit = 1
static int WSS625_1[WSS625_DATA_BIT_LENGTH] = { 1,1,1,0,0,0 };

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

static BOOL decode_sequence(BYTE* vbiline, int* DecodedVals, int NbVal, int Threshold, int *Offsets)
{
    int i;

    for (i = 0 ; i < NbVal ; i++)
    {
        if (decodebit (vbiline + Offsets[i] - Offsets[0], Threshold, 7) != DecodedVals[i])
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

    if (Threshold < WSS625_MIN_THRESHOLD)
    {
//      LOG(3, "WSS signal threshold too low (%x)", Threshold);
        return FALSE;
    }

    for (i = WSS625_START_POS_MIN ; i <= WSS625_START_POS_MAX ; i++)
    {
//      if ( (vbiline[i+3] <= Threshold)
//        || (vbiline[i+2] <= Threshold)
//        || (vbiline[i+4] <= Threshold)
//        || (vbiline[i+1] <= Threshold)
//        || (vbiline[i+5] <= Threshold) )
        if (vbiline[i] < Threshold)
            continue;

        StartPos = i;
//      LOG(3, "WSS decoding at start position = %d", StartPos);

        // run-in code decoding
        k = 0;
        if (decode_sequence (vbiline + i + offsets[k], WSS625_runin, WSS625_RUNIN_CODE_LENGTH, Threshold, &offsets[k]))
        {
//          LOG(3, "WSS run-in code detected (start pos = %d, Threshold = %x)", i, Threshold);
            k += WSS625_RUNIN_CODE_LENGTH;

            // Start code decoding
            if (decode_sequence (vbiline + i + offsets[k], WSS625_start, WSS625_START_CODE_LENGTH, Threshold, &offsets[k]))
            {
//              LOG(3, "WSS start code detected");
                k += WSS625_START_CODE_LENGTH;

                // Data bits decoding
                nb = 0;
                for (j = 0 ; j < WSS625_NB_DATA_BITS ; j++)
                {
                    if (decode_sequence (vbiline + i + offsets[k], WSS625_0, WSS625_DATA_BIT_LENGTH, Threshold, &offsets[k]))
                    {
                        bits[j] = 0;
                        nb++;
//                      LOG(3, "WSS b%d = 0 (start pos = %d, Threshold = %x)", j, i, Threshold);
                    }
                    else if (decode_sequence (vbiline + i + offsets[k], WSS625_1, WSS625_DATA_BIT_LENGTH, Threshold, &offsets[k]))
                    {
                        bits[j] = 1;
                        nb++;
//                      LOG(3, "WSS b%d = 1 (start pos = %d, Threshold = %x)", j, i, Threshold);
                    }
                    else
                    {
                        bits[j] = -1;
//                      LOG(3, "WSS b%d = ?", j);
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
//              int pos = StartPos + offsets[k];
//              LOG(3, "WSS bit b%d => %x %x %x %x %x %x %x", i, vbiline[pos], vbiline[pos+1], vbiline[pos+2], vbiline[pos+3], vbiline[pos+4], vbiline[pos+5], vbiline[pos+6]);
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
//  else
//  {
//      LOG(3, "WSS decode ERROR Threshold = %x", Threshold);
//      StartPos = 0;
//      for (j = 0 ; j < 250 ; j++)
//      {
//          k = StartPos + j * 7;
//          LOG(3, "WSS pos = %d : %x %x %x %x %x %x %x", k, vbiline[k], vbiline[k + 1], vbiline[k + 2], vbiline[k + 3], vbiline[k + 4], vbiline[k + 5], vbiline[k + 6]);
//      }
//  }

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

int WSS_DecodeLine(BYTE* vbiline)
{
//  int     PrevAspectMode = WSS_Data.AspectMode;
//  int     PrevAspectRatio = WSS_Data.AspectRatio;
//  int     PrevDecodeStatus = WSS_CtrlData.DecodeStatus;
    BOOL    bResuDecoding;
//  int     NewAspectMode;
//  int     NewAspectRatio;
//  BOOL    bSwitch = FALSE;
//  char    szInfo[32];

    switch (BT848_GetTVFormat()->wCropHeight)
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
//          bSwitch = TRUE;
//          NewAspectMode = WSS_CtrlData.AspectModeWhenErr;
//          NewAspectRatio = WSS_CtrlData.AspectRatioWhenErr;
        }
    }
    else
    {
//      if (! PrevDecodeOk && (WSS_CtrlData.NbSuccessiveErr >= WSS_MAX_SUCCESSIVE_ERR))
//      {
//          WSS_CtrlData.AspectModeWhenErr = AspectSettings.AspectMode;
//          WSS_CtrlData.AspectRatioWhenErr = AspectSettings.SourceAspect;
//      }

        WSS_CtrlData.DecodeStatus = WSS_STATUS_OK;
        WSS_CtrlData.NbDecodeOk++;
        WSS_CtrlData.NbSuccessiveErr = 0;

//      // Manage WSS ratio information
//      if ( (WSS_Data.AspectMode != -1)
//        && (WSS_Data.AspectRatio != -1) )
//      {
//          if ( (WSS_Data.AspectMode != PrevAspectMode)
//            || (WSS_Data.AspectRatio != PrevAspectRatio) )
//          {
//              bSwitch = TRUE;
//              NewAspectMode = WSS_Data.AspectMode;
//              NewAspectRatio = WSS_Data.AspectRatio;
//          }
//          else
//          {
//              NewAspectMode = WSS_Data.AspectMode;
//              if (WSS_Data.AspectMode != AspectSettings.AspectMode)
//              {
//                  bSwitch = TRUE;
//              }
//              if (WSS_Data.AspectRatio > AspectSettings.SourceAspect)
//              {
//                  bSwitch = TRUE;
//                  NewAspectRatio = WSS_Data.AspectRatio;
//              }
//              else
//              {
//                  NewAspectRatio = AspectSettings.SourceAspect;
//              }
//          }
//      }
    }

//  if (bSwitch
//   && (NewAspectMode != -1)
//   && (NewAspectRatio != -1)
//   && ( (NewAspectMode != AspectSettings.AspectMode)
//     || (NewAspectRatio != AspectSettings.SourceAspect) ) )
//  {
//      SwitchToRatio (NewAspectMode, NewAspectRatio);
//
//      // OSD message
//      sprintf(szInfo, "%.2f:1", (double)Setting_GetValue(Aspect_GetSetting(SOURCE_ASPECT)) / 1000.0);
//      if ( (Setting_GetValue(Aspect_GetSetting(ASPECT_MODE)) == 1)
//        && (Setting_GetValue(Aspect_GetSetting(SOURCE_ASPECT)) != 1333) )
//      {
//          strcat(szInfo, " Letterbox");
//      }
//      else if (Setting_GetValue(Aspect_GetSetting(ASPECT_MODE)) == 2)
//      {
//          strcat(szInfo, " Anamorphic");
//      }
//      strcat(szInfo, " Signal");
//      OSD_ShowText(hWnd, szInfo, 0);
//  }

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