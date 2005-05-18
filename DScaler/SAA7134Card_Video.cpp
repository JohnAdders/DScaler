/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Card_Video.cpp,v 1.13 2005-05-18 12:18:32 robmuller Exp $
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
// Revision 1.12  2004/11/19 23:47:21  atnak
// Changes to get rid of warnings.
//
// Revision 1.11  2003/10/27 10:39:53  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.10  2003/01/07 23:00:00  atnak
// Removed variable upscale devisor and locked in at 0x200 scaling
// for 27Mhz VBI stepping
//
// Revision 1.9  2002/11/10 09:30:57  atnak
// Added Chroma only comb filter mode for SECAM
//
// Revision 1.8  2002/10/26 04:42:50  atnak
// Added AGC config and automatic volume leveling control
//
// Revision 1.7  2002/10/23 17:05:19  atnak
// Added variable VBI sample rate scaling
//
// Revision 1.6  2002/10/20 07:41:04  atnak
// custom audio standard setup + etc
//
// Revision 1.5  2002/10/15 04:34:26  atnak
// increased the amount of VBI samples to get better decoding
//
// Revision 1.4  2002/10/08 20:35:39  atnak
// whitepeak, colorpeak, comb filter UI options
//
// Revision 1.3  2002/10/08 19:35:45  atnak
// various fixes, tweaks, cleanups
//
// Revision 1.2  2002/10/06 11:11:29  atnak
// Added SetVSync recovery
//
// Revision 1.1  2002/10/03 23:42:07  atnak
// SAA7134Common.h SAA7134Common.cpp SAA7134Card_Video.cpp added
//
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file SAA7134Card.cpp CSAA7134Card Implementation (Video)
 */

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


void CSAA7134Card::SetVideoStandard(eVideoStandard VideoStandard, long& VBILines,
                                    long& VideoWidth, long& VideoHeight,
                                    long HDelayShift, long VDelayShift,
                                    long VBIUpscaleDivisor)
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
            ChromaCtrl2 = 0x06;
        }
        else if (IsSECAMVideoStandard(VideoStandard))
        {
            LumaControl = SAA7134_LUMA_CTRL_LUBW | 0x0B;
            ChromaGain  = SAA7134_CHROMA_GAIN_CTRL_ACGC;
        }
        else if (IsNTSCVideoStandard(VideoStandard))
        {
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
        SAA7134_LUMA_CTRL_LDEL);

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

    VBILines = m_VideoStandards[VideoStandard].wVBIStopLine -
        m_VideoStandards[VideoStandard].wVBIStartLine + 1;

    SetVBIGeometry((WORD)VBIUpscaleDivisor);

    VideoHeight = m_VideoStandards[VideoStandard].wFieldHeight * 2;

    SetGeometry((WORD)VideoWidth, (WORD)VideoHeight, HDelayShift, VDelayShift);
}


void CSAA7134Card::SetVBIGeometry(WORD UpscaleDivisor)
{
    if (m_VideoStandard == VIDEOSTANDARD_INVALID)
    {
        return;
    }

    WORD VStart = m_VideoStandards[m_VideoStandard].wVBIStartLine;
    WORD VStop  = m_VideoStandards[m_VideoStandard].wVBIStopLine;

    SetTaskVBIGeometry(TASKID_A, 0, 719, VStart, VStop, UpscaleDivisor);
    SetTaskVBIGeometry(TASKID_B, 0, 719, VStart, VStop, UpscaleDivisor);
}


void CSAA7134Card::SetTaskVBIGeometry(eTaskID TaskID, WORD HStart, WORD HStop,
                                  WORD VStart, WORD VStop, WORD UpscaleDivisor)
{
    BYTE TaskMask = TaskID2TaskMask(TaskID);

    WORD SampleBytes = (WORD)((double) 0x400 / UpscaleDivisor * (HStop - HStart + 1));

    WriteWord(SAA7134_VBI_H_START(TaskMask), HStart);
    WriteWord(SAA7134_VBI_H_STOP(TaskMask), HStop);
    WriteWord(SAA7134_VBI_V_START(TaskMask), VStart);
    WriteWord(SAA7134_VBI_V_STOP(TaskMask), VStop);

    WriteWord(SAA7134_VBI_H_SCALE_INC(TaskMask), UpscaleDivisor);

    WORD Lines = VStop - VStart + 1;

    eRegionID RegionID = TaskID2VBIRegion(TaskID);
    if (GetDMA(RegionID))
    {
        // Make sure we aren't grabbing more lines than we have memory
        WORD LinesAvailable = CalculateLinesAvailable(RegionID, SampleBytes);

        if (LinesAvailable < Lines)
        {
            Lines = LinesAvailable;
        }
    }

    WriteWord(SAA7134_VBI_V_LEN(TaskMask), Lines);
    WriteWord(SAA7134_VBI_H_LEN(TaskMask), SampleBytes);
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
        HDelay += (WORD)HDelayShift;
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
        VDelay += (WORD)(VDelayShift / 2);
    }

    SetTaskGeometry(TASKID_A, FieldWidth, FieldHeight, HDelay, VDelay,
        ScaleWidth, ScaleHeight / 2);
    SetTaskGeometry(TASKID_B, FieldWidth, FieldHeight, HDelay, VDelay,
        ScaleWidth, ScaleHeight / 2);
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
        {
            Lines = LinesAvailable;
        }
    }

    // The number of lines and pixels to DMA
    WriteWord(SAA7134_VIDEO_LINES(TaskMask), Lines);
    WriteWord(SAA7134_VIDEO_PIXELS(TaskMask), ScaleWidth);
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


void CSAA7134Card::SetCombFilter(eCombFilter CombFilter)
{
    switch (CombFilter)
    {
    case COMBFILTER_OFF:
        AndDataByte(SAA7134_LUMA_CTRL, ~SAA7134_LUMA_CTRL_YCOMB);
        AndDataByte(SAA7134_CHROMA_CTRL1, ~SAA7134_CHROMA_CTRL1_CCOMB);
        break;

    case COMBFILTER_CHROMA_ONLY:
        AndDataByte(SAA7134_LUMA_CTRL, ~SAA7134_LUMA_CTRL_YCOMB);
        OrDataByte(SAA7134_CHROMA_CTRL1, SAA7134_CHROMA_CTRL1_CCOMB);
        break;

    case COMBFILTER_FULL:
        OrDataByte(SAA7134_LUMA_CTRL, SAA7134_LUMA_CTRL_YCOMB);
        OrDataByte(SAA7134_CHROMA_CTRL1, SAA7134_CHROMA_CTRL1_CCOMB);
        break;

    default:
        break;
    }
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


void CSAA7134Card::SetColorPeak(BOOL ColorPeak)
{
    if (ColorPeak)
    {
        AndDataByte(SAA7134_ANALOG_IN_CTRL2, ~SAA7134_ANALOG_IN_CTRL2_CPOFF);
    }
    else
    {
        OrDataByte(SAA7134_ANALOG_IN_CTRL2, SAA7134_ANALOG_IN_CTRL2_CPOFF);
    }
}


BOOL CSAA7134Card::GetColorPeak()
{
    return (ReadByte(SAA7134_ANALOG_IN_CTRL2) &
        SAA7134_ANALOG_IN_CTRL2_CPOFF) == 0;
}


void CSAA7134Card::SetAutomaticGainControl(BOOL bAGC)
{
    if (bAGC)
    {
        AndDataByte(SAA7134_ANALOG_IN_CTRL2, ~SAA7134_ANALOG_IN_CTRL2_GAFIX);
    }
    else
    {
        OrDataByte(SAA7134_ANALOG_IN_CTRL2, SAA7134_ANALOG_IN_CTRL2_GAFIX);
    }
}


void CSAA7134Card::SetGainControl(WORD GainControl)
{
    // Only 9 bits of GainControl is used

    WriteByte(SAA7134_ANALOG_IN_CTRL3, GainControl & 0xFF);
    WriteByte(SAA7134_ANALOG_IN_CTRL4, GainControl & 0xFF);

    BYTE AnalogInputCtrl2 = 0x00;

    if (GainControl & 0x0100)
    {
        AnalogInputCtrl2 = 0x03;
    }

    MaskDataByte(SAA7134_ANALOG_IN_CTRL2, AnalogInputCtrl2,
        SAA7134_ANALOG_IN_CTRL2_GAI18 |
        SAA7134_ANALOG_IN_CTRL2_GAI18);
}

void CSAA7134Card::SetGammaControl(BOOL bGammaControl)
{
	// bit 0: ??? (this one is not explained in the documentation. Set to bypass RGB to YUV matrix?)
	// bit 1: set to bypass YUV to RGB matrix 
	// bit 2: set to enable transformation via LUT curve
	
	if(bGammaControl)
	{
		AndDataByte(SAA7134_DATA_PATH(SAA7134_TASK_A_MASK), ~3);
		OrDataByte(SAA7134_DATA_PATH(SAA7134_TASK_A_MASK), 4);

		AndDataByte(SAA7134_DATA_PATH(SAA7134_TASK_B_MASK), ~3);
		OrDataByte(SAA7134_DATA_PATH(SAA7134_TASK_B_MASK), 4);

	}
	else
	{
		AndDataByte(SAA7134_DATA_PATH(SAA7134_TASK_A_MASK), ~4);
		OrDataByte(SAA7134_DATA_PATH(SAA7134_TASK_A_MASK), 3);

		AndDataByte(SAA7134_DATA_PATH(SAA7134_TASK_B_MASK), ~4);
		OrDataByte(SAA7134_DATA_PATH(SAA7134_TASK_B_MASK), 3);
	}
}

void CSAA7134Card::SetGammaLevel(WORD bGammaLevel)
{
    WriteByte(SAA7134_START_GREEN, 0x00);
    WriteByte(SAA7134_START_BLUE, 0x00);
    WriteByte(SAA7134_START_RED, 0x00);

    for (int i = 0; i < 0x0F; i++)
    {
		BYTE AdjustedValue = (BYTE)(255.0 * pow(((i+1)<<4)/255.0, 1000/(double)bGammaLevel));

        WriteByte(SAA7134_GREEN_PATH(i), AdjustedValue);
        WriteByte(SAA7134_BLUE_PATH(i), AdjustedValue);
        WriteByte(SAA7134_RED_PATH(i), AdjustedValue);
    }

    WriteByte(SAA7134_GREEN_PATH(0x0F), 0xFF);
    WriteByte(SAA7134_BLUE_PATH(0x0F), 0xFF);
    WriteByte(SAA7134_RED_PATH(0x0F), 0xFF);
}


void CSAA7134Card::SetVideoMirror(BOOL bMirror)
{
    if (bMirror)
    {
        OrDataByte(SAA7134_V_FILTER(SAA7134_TASK_A_MASK), SAA7134_V_FILTER_YMIR);
        OrDataByte(SAA7134_V_FILTER(SAA7134_TASK_B_MASK), SAA7134_V_FILTER_YMIR);
    }
    else
    {
        AndDataByte(SAA7134_V_FILTER(SAA7134_TASK_A_MASK), ~SAA7134_V_FILTER_YMIR);
        AndDataByte(SAA7134_V_FILTER(SAA7134_TASK_B_MASK), ~SAA7134_V_FILTER_YMIR);
    }
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
    MaskDataByte(SAA7134_FIR_PREFILTER_CTRL(TaskMask), 0x08, 0x0F); /* VPFY ?? */
}
