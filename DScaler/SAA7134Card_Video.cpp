/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Card_Video.cpp,v 1.2 2002-10-06 11:11:29 atnak Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Atsushi Nakagawa.  All rights reserved.
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
// This software was based on v4l2 device driver for philips
// saa7134 based TV cards.  Those portions are
// Copyright (c) 2001,02 Gerd Knorr <kraxel@bytesex.org> [SuSE Labs]
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 03 Oct 2002   Atsushi Nakagawa      Moved video setting stuff in here
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.1  2002/10/03 23:42:07  atnak
// SAA7134Common.h SAA7134Common.cpp SAA7134Card_Video.cpp added
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SAA7134Card.h"
#include "SAA7134_Defines.h"
#include "SAA7134Common.h"


void CSAA7134Card::SetBrightness(BYTE Brightness)
{
    WriteByte(SAA7134_DEC_LUMA_BRIGHT, Brightness);
}

BYTE CSAA7134Card::GetBrightness()
{
    return ReadByte(SAA7134_DEC_LUMA_BRIGHT);
}

void CSAA7134Card::SetContrast(BYTE Contrast)
{
    WriteByte(SAA7134_DEC_LUMA_CONTRAST, Contrast);
}

BYTE CSAA7134Card::GetContrast()
{
    return ReadByte(SAA7134_DEC_LUMA_CONTRAST);
}

void CSAA7134Card::SetSaturation(BYTE SaturationU)
{
    WriteByte(SAA7134_DEC_CHROMA_SATURATION, SaturationU);
}

BYTE CSAA7134Card::GetSaturation()
{
    return ReadByte(SAA7134_DEC_CHROMA_SATURATION);
}

void CSAA7134Card::SetHue(BYTE Hue)
{
    WriteByte(SAA7134_DEC_CHROMA_HUE, Hue);
}

BYTE CSAA7134Card::GetHue()
{
    return ReadByte(SAA7134_DEC_CHROMA_HUE);
}

// Find SAA7134 equivalent or remove
void CSAA7134Card::SetWhiteCrushUp(BYTE WhiteCrushUp)
{
}

// Find SAA7134 equivalent or remove
BYTE CSAA7134Card::GetWhiteCrushUp()
{
    return 0x00;
}

// Find SAA7134 equivalent or remove
void CSAA7134Card::SetWhiteCrushDown(BYTE WhiteCrushDown)
{
//    WriteByte(BT848_WC_DOWN, WhiteCrushDown);
}

// Find SAA7134 equivalent or remove
BYTE CSAA7134Card::GetWhiteCrushDown()
{
    return 0x00;
}

// Find SAA7134 equivalent or remove
void CSAA7134Card::SetBDelay(BYTE BDelay)
{
}

// Find SAA7134 equivalent or remove
BYTE CSAA7134Card::GetBDelay()
{
    return 0x00;
}


void CSAA7134Card::SetVideoStandard(eVideoStandard VideoStandard, long& VBILines,
                                    long& VideoWidth, long& VideoHeight,
                                    long HDelayShift, long VDelayShift)
{
    BYTE SyncControl    = 0;
    BYTE LumaControl    = 0;
    BYTE ChromaCtrl1    = 0;
    BYTE ChromaGain     = 0;
    BYTE ChromaCtrl2    = 0;
    BYTE MiscVGateMSB   = 0;

    if (VideoStandard == VIDEOSTANDARD_AUTODETECT)
    {
        // Automatic field rate (50Hz/60z) detection
        SyncControl = SAA7134_SYNC_CTRL_AUFD;
        LumaControl = SAA7134_LUMA_CTRL_YCOMB;

        // Automatic colour standard detection
        ChromaCtrl1 = SAA7134_CHROMA_CTRL1_AUTO | SAA7134_CHROMA_CTRL1_DCVF;
    }
    else
    {
        if (m_VideoStandards[VideoStandard].Is25fps)
        {
            MiscVGateMSB = SAA7134_MISC_VGATE_MSB_VGPS;
        }
        else
        {
            SyncControl = SAA7134_SYNC_CTRL_FSEL;
        }

        if (IsPALVideoStandard(VideoStandard))
        {
            LumaControl = SAA7134_LUMA_CTRL_YCOMB;
            ChromaCtrl2 = 0x06;
        }
        else if (IsSECAMVideoStandard(VideoStandard))
        {
            LumaControl = SAA7134_LUMA_CTRL_LUBW | 0x0B;
            ChromaGain  = SAA7134_CHROMA_GAIN_CTRL_ACGC;
        }
        else if (IsNTSCVideoStandard(VideoStandard))
        {
            LumaControl = SAA7134_LUMA_CTRL_YCOMB;
            ChromaCtrl1 = SAA7134_CHROMA_CTRL1_DCVF;
            ChromaCtrl2 = SAA7134_CHROMA_CTRL2_CHBW | 0x06;
        }
        else
        {
            // NEVER_GET_HERE;
        }

        ChromaCtrl1 |= (m_VideoStandards[VideoStandard].CSTDMask << 4);
    }

    MaskDataByte(SAA7134_SYNC_CTRL, SyncControl,
       SAA7134_SYNC_CTRL_AUFD |
       SAA7134_SYNC_CTRL_FSEL |
       SAA7134_SYNC_CTRL_VNOI);

    MaskDataByte(SAA7134_LUMA_CTRL, LumaControl,
        SAA7134_LUMA_CTRL_LUFI |
        SAA7134_LUMA_CTRL_LUBW |
        SAA7134_LUMA_CTRL_LDEL |
        SAA7134_LUMA_CTRL_YCOMB);

    MaskDataByte(SAA7134_CHROMA_CTRL1, ChromaCtrl1,
        SAA7134_CHROMA_CTRL1_CSTD |
        SAA7134_CHROMA_CTRL1_DCVF |
        SAA7134_CHROMA_CTRL1_AUTO);

    MaskDataByte(SAA7134_CHROMA_GAIN_CTRL, ChromaGain,
        SAA7134_CHROMA_GAIN_CTRL_CGAIN |
        SAA7134_CHROMA_GAIN_CTRL_ACGC);

    MaskDataByte(SAA7134_CHROMA_CTRL2, ChromaCtrl2,
        SAA7134_CHROMA_CTRL2_LCBW |
        SAA7134_CHROMA_CTRL2_CHBW);

    MaskDataByte(SAA7134_MISC_VGATE_MSB, MiscVGateMSB,
        SAA7134_MISC_VGATE_MSB_VGPS);

    m_VideoStandard = VideoStandard;

    UpdateAudioClocksPerField(VideoStandard);

    WORD VBIStartLine   = m_VideoStandards[VideoStandard].wVBIStartLine;
    WORD VBIStopLine    = m_VideoStandards[VideoStandard].wVBIStopLine;

    if (VBIStopLine - VBIStartLine + 1 > VBILines)
    {
        VBIStopLine = VBIStartLine + VBILines - 1;
    }
    else
    {
        VBILines = VBIStopLine - VBIStartLine + 1;
    }

    SetVBIGeometry(TASKID_A, 0, 719, VBIStartLine, VBIStopLine);
    SetVBIGeometry(TASKID_B, 0, 719, VBIStartLine, VBIStopLine);

    VideoHeight = m_VideoStandards[VideoStandard].wFieldHeight * 2;

    SetGeometry(VideoWidth, VideoHeight, HDelayShift, VDelayShift);
}


void CSAA7134Card::SetGeometry(WORD ScaleWidth, WORD ScaleHeight,
                               long HDelayShift, long VDelayShift)
{
    if (m_VideoStandard == VIDEOSTANDARD_INVALID)
    {
        return;
    }

    WORD FieldWidth     = m_VideoStandards[m_VideoStandard].wFieldWidth;
    WORD FieldHeight    = m_VideoStandards[m_VideoStandard].wFieldHeight;

    WORD HDelay         = m_VideoStandards[m_VideoStandard].wHDelay;
    WORD VDelay         = m_VideoStandards[m_VideoStandard].wVDelay;

    if (HDelayShift != 0)
    {
        if (HDelay + HDelayShift < 0)
        {
            HDelayShift = -HDelay;
        }
        HDelay += HDelayShift;
    }

    if (VDelayShift != 0)
    {
        if (IsVBIActive())
        {
            if (VDelayShift < GetMinimumVDelayWithVBI())
            {
                VDelayShift = GetMinimumVDelayWithVBI();
            }
        }

        if (VDelay + (VDelayShift / 2) < 0)
        {
            VDelayShift = -VDelay * 2;
        }
        VDelay += (VDelayShift / 2);
    }

    SetTaskGeometry(TASKID_A, FieldWidth, FieldHeight, HDelay, VDelay,
        ScaleWidth, ScaleHeight / 2);
    SetTaskGeometry(TASKID_B, FieldWidth, FieldHeight, HDelay, VDelay,
        ScaleWidth, ScaleHeight / 2);
}


void CSAA7134Card::SetVBIGeometry(eTaskID TaskID, WORD HStart, WORD HStop,
                                  WORD VStart, WORD VStop)
{
    BYTE TaskMask = TaskID2TaskMask(TaskID);

    // It should come out less than this amount
    WORD SampleBytes = 1024;

    WriteWord(SAA7134_VBI_H_START(TaskMask), HStart);
    WriteWord(SAA7134_VBI_H_STOP(TaskMask), HStop);
    WriteWord(SAA7134_VBI_V_START(TaskMask), VStart);
    WriteWord(SAA7134_VBI_V_STOP(TaskMask), VStop);

    // SAA7134_VBI_H_SCALE_INC:
    //   0x400 for 100%, 0x200 for 200%

    // (This may be specific to PAL-BG)
    // DScaler wants exactly 0x0186 horizontal scaling but SAA7134
    // can't handle this scaling.  Instead, we scale 0x30C (half of
    // 0x0186) and double the bytes in SAA7134Source.cpp
    // WriteWord(SAA7134_VBI_H_SCALE_INC(TaskMask), 0x0186);
    //
    WriteWord(SAA7134_VBI_H_SCALE_INC(TaskMask), 0x030C);

    WORD Lines = VStop - VStart + 1;

    eRegionID RegionID = TaskID2VBIRegion(TaskID);
    if (GetDMA(RegionID))
    {
        // Make sure we aren't grabbing more lines than we have memory
        WORD LinesAvailable = CalculateLinesAvailable(RegionID, SampleBytes);

        if (LinesAvailable < Lines)
            Lines = LinesAvailable;
    }

    WriteWord(SAA7134_VBI_H_LEN(TaskMask), SampleBytes);
    WriteWord(SAA7134_VBI_V_LEN(TaskMask), Lines);
}


void CSAA7134Card::SetTaskGeometry(eTaskID TaskID,
                                   WORD Width, WORD Height,
                                   WORD HDelay, WORD VDelay,
                                   WORD ScaleWidth, WORD ScaleHeight)
{
    BYTE TaskMask = TaskID2TaskMask(TaskID);

    WriteWord(SAA7134_VIDEO_H_START(TaskMask), HDelay);
    WriteWord(SAA7134_VIDEO_H_STOP(TaskMask), HDelay + Width - 1);
    WriteWord(SAA7134_VIDEO_V_START(TaskMask), VDelay);
    WriteWord(SAA7134_VIDEO_V_STOP(TaskMask), VDelay + Height - 1);

    // settings for no horizontal prescaling
    ResetHPrescale(TaskID);

    // 1024UL = median
    WORD HorizScale = 1024UL * Width / ScaleWidth;
    WriteWord(SAA7134_H_SCALE_INC(TaskMask), HorizScale);

    // settings for vertical scaling
    WORD VertScale = 1024UL * Height / ScaleHeight;
    WriteWord(SAA7134_V_SCALE_RATIO(TaskMask), VertScale);

    WORD Lines = ScaleHeight;

    eRegionID RegionID = TaskID2VideoRegion(TaskID);
    if (GetDMA(RegionID))
    {
        // Make sure we aren't grabbing more lines than we have memory
        // Multiply CurrentX by 2 because YUV is two bytes per pixel
        WORD LinesAvailable = CalculateLinesAvailable(RegionID, ScaleWidth * 2);

        if (LinesAvailable < Lines)
            Lines = LinesAvailable;
    }

    // The number of pixels and lines to DMA
    WriteWord(SAA7134_VIDEO_PIXELS(TaskMask), ScaleWidth);
    WriteWord(SAA7134_VIDEO_LINES(TaskMask), Lines);
}


void CSAA7134Card::CheckVBIAndVideoOverlap(eTaskID TaskID)
{
    BYTE TaskMask = TaskID2TaskMask(TaskID);

    WORD VBIStopLine = ReadWord(SAA7134_VBI_V_STOP(TaskMask));
    WORD VIDStartLine = ReadWord(SAA7134_VIDEO_V_START(TaskMask));

    int Delta;

    if ((Delta = VIDStartLine - VBIStopLine) < 1)
    {
        WriteWord(SAA7134_VIDEO_V_START(TaskMask), VIDStartLine - Delta + 1);

        WORD VIDStopLine = ReadWord(SAA7134_VIDEO_V_STOP(TaskMask));
        WriteWord(SAA7134_VIDEO_V_STOP(TaskMask), VIDStopLine - Delta + 1);
    }
}


BOOL CSAA7134Card::IsVBIActive()
{
    BYTE Region = SAA7134_REGION_ENABLE_VBI_ENA | SAA7134_REGION_ENABLE_VBI_ENB;

    return (ReadByte(SAA7134_REGION_ENABLE) & Region) != 0;
}


long CSAA7134Card::GetMinimumVDelay()
{
    // x2 because field lines x 2 = frame lines
    return -(m_VideoStandards[m_VideoStandard].wVDelay) * 2;
}


long CSAA7134Card::GetMinimumVDelayWithVBI()
{
    WORD VDelay         = m_VideoStandards[m_VideoStandard].wVDelay;
    WORD VBIStopLine    = m_VideoStandards[m_VideoStandard].wVBIStopLine;

    return -(VDelay - VBIStopLine - 1) * 2;
}


void CSAA7134Card::SetHPLLMode(eHPLLMode HPLLMode)
{
    if (HPLLMode == HPLLMODE_TV)
    {
        MaskDataByte(SAA7134_SYNC_CTRL, 0x00, SAA7134_SYNC_CTRL_HTD);
    }
    else if (HPLLMode == HPLLMODE_VCR)
    {
        MaskDataByte(SAA7134_SYNC_CTRL, 0x08, SAA7134_SYNC_CTRL_HTD);
    }
    else
    {
        MaskDataByte(SAA7134_SYNC_CTRL, 0x18, SAA7134_SYNC_CTRL_HTD);
    }
}


void CSAA7134Card::SetVSyncRecovery(eVSyncRecovery VSyncRecovery)
{
    BYTE VNOI = 0x00;

    switch (VSyncRecovery)
    {
    case VSYNCRECOVERY_NORMAL:
        VNOI = 0x00;
        break;
    case VSYNCRECOVERY_FAST_TRACKING:
        if (ReadByte(SAA7134_SYNC_CTRL) & SAA7134_SYNC_CTRL_AUFD)
        {
            VNOI = 0x00;
        }
        else
        {
            VNOI = 0x01;
        }
        break;
    case VSYNCRECOVERY_FREE_RUNNING:
        VNOI = 0x02;
        break;
    case VSYNCRECOVERY_IMMEDIATE_MODE:
        VNOI = 0x03;
        break;
    }
    MaskDataByte(SAA7134_SYNC_CTRL, VNOI, SAA7134_SYNC_CTRL_VNOI);
}


void CSAA7134Card::SetWhitePeak(BOOL WhitePeak)
{
    if (WhitePeak)
    {
        AndDataByte(SAA7134_INCR_DELAY, ~SAA7134_INCR_DELAY_WPOFF);
    }
    else
    {
        OrDataByte(SAA7134_INCR_DELAY, SAA7134_INCR_DELAY_WPOFF);
    }
}

BOOL CSAA7134Card::GetWhitePeak()
{
    return (ReadByte(SAA7134_INCR_DELAY) & SAA7134_INCR_DELAY_WPOFF) == 0;
}

void CSAA7134Card::SetColourPeak(BOOL ColourPeak)
{
    if (ColourPeak)
    {
        AndDataByte(SAA7134_ANALOG_IN_CTRL2, ~SAA7134_ANALOG_IN_CTRL2_CPOFF);
    }
    else
    {
        OrDataByte(SAA7134_ANALOG_IN_CTRL2, SAA7134_ANALOG_IN_CTRL2_CPOFF);
    }
}

BOOL CSAA7134Card::GetColourPeak()
{
    return (ReadByte(SAA7134_ANALOG_IN_CTRL2) &
        SAA7134_ANALOG_IN_CTRL2_CPOFF) == 0;
}


BOOL CSAA7134Card::Is25fpsSignalDetected()
{
    return (ReadWord(SAA7134_STATUS_VIDEO) &
        SAA7134_STATUS_VIDEO_FIDT) == 0;
}


BOOL CSAA7134Card::IsInterlacedSignalDetected()
{
    return (ReadWord(SAA7134_STATUS_VIDEO) &
        SAA7134_STATUS_VIDEO_INTL) != 0;
}


// Unused, might not need this for DScaler
// SAA7134_H_SCALE_INC does the scaling we need
// - This is "prescaling" as opposed to scaling.. don't know what that means
void CSAA7134Card::SetHPrescale(eTaskID TaskID, WORD wSourceSize, WORD wScaleSize)
{
    BYTE TaskMask = TaskID2TaskMask(TaskID);

    static const struct {
        int xpsc;
        int xacl;
        int xc2_1;
        int xdcg;
        int vpfy;
    } vals[] = {
        /* XPSC XACL XC2_1 XDCG VPFY */
        {    1,   0,    0,    0,   0 },
        {    2,   2,    1,    2,   2 },
        {    3,   4,    1,    3,   2 },
        {    4,   8,    1,    4,   2 },
        {    5,   8,    1,    4,   2 },
        {    6,   8,    1,    4,   3 },
        {    7,   8,    1,    4,   3 },
        {    8,  15,    0,    4,   3 },
        {    9,  15,    0,    4,   3 },
        {   10,  16,    1,    5,   3 },
    };

    int i = 2;

    WriteByte(SAA7134_H_PRESCALE(TaskMask), vals[i].xpsc);
    WriteByte(SAA7134_ACC_LENGTH(TaskMask), vals[i].xacl);
    WriteByte(SAA7134_LEVEL_CTRL(TaskMask), (vals[i].xc2_1 << 3) | (vals[i].xdcg));
    MaskDataByte(SAA7134_FIR_PREFILTER_CTRL(TaskMask),
        (vals[i].vpfy << 2) | vals[i].vpfy, 0x0F);
}


void CSAA7134Card::ResetHPrescale(eTaskID TaskID)
{
    BYTE TaskMask = TaskID2TaskMask(TaskID);

    WriteByte(SAA7134_H_PRESCALE(TaskMask), 0x01);
    WriteByte(SAA7134_ACC_LENGTH(TaskMask), 0x00);

    /* 0x07 mask = XDCG, 0x08 mask = 0xXC2_1 whatever that means */
    WriteByte(SAA7134_LEVEL_CTRL(TaskMask), 0x00);
    MaskDataByte(SAA7134_FIR_PREFILTER_CTRL(TaskMask), 0x00, 0x0F); /* VPFY ?? */
}
