/////////////////////////////////////////////////////////////////////////////
// $Id$
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
// This software was based on BT848Card.cpp.  Those portions are
// Copyright (c) 2001 John Adcock.
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file SAA7134Card.cpp CSAA7134Card Implementation
 */

#include "stdafx.h"

#ifdef WANT_SAA713X_SUPPORT

#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "SAA7134Card.h"
#include "SAA7134_Defines.h"
#include "SAA7134Common.h"
#include "SAA7134I2CBus.h"
#include "Audio.h"
#include "DebugLog.h"
#include "CPU.h"
/// \todo remove need for this
#include "ProgramList.h"
/// \todo remove need for this
#include "OutThreads.h"

using namespace std;

CSAA7134Card::CSAA7134Card(SmartPtr<CHardwareDriver> pDriver) :
    CPCICard(pDriver),
    m_CardType(SAA7134CARDID_UNKNOWN),
    m_PreparedRegions(0x00),
    m_VideoStandard(VIDEOSTANDARD_INVALID),
    m_bAudioLineReservedForMute(FALSE),
    m_I2CBus(new CSAA7134I2CBus(this)),
    m_bStereoExternalLines(FALSE)
{
    m_LastTriggerError = 0UL;

}


CSAA7134Card::~CSAA7134Card()
{
    WriteDword(SAA7134_IRQ1, 0UL);
    WriteDword(SAA7134_IRQ2, 0UL);
    MaskDataDword(SAA7134_MAIN_CTRL, 0, 0x000000FF);

    ClosePCICard();
}


int CSAA7134Card::RegionID2Channel(eRegionID RegionID)
{
    switch (RegionID)
    {
    case REGIONID_VIDEO_A: return 0;
    case REGIONID_VIDEO_B: return 1;
    case REGIONID_VBI_A: return 2;
    case REGIONID_VBI_B: return 3;
    default:
        // NEVER_GET_HERE;
        break;
    }
    return 0;
}


BYTE CSAA7134Card::TaskID2TaskMask(eTaskID TaskID)
{
    switch (TaskID)
    {
    case TASKID_A: return SAA7134_TASK_A_MASK;
    case TASKID_B: return SAA7134_TASK_B_MASK;
    default:
        // NEVER_GET_HERE;
        break;
    }
    return 0;
}


void CSAA7134Card::PrepareCard()
{
    // Disable all interrupts
    WriteDword(SAA7134_IRQ1, 0UL);
    WriteDword(SAA7134_IRQ2, 0UL);

    // Disable all DMA
    MaskDataDword(SAA7134_MAIN_CTRL, 0UL,
        SAA7134_MAIN_CTRL_TE6 |
        SAA7134_MAIN_CTRL_TE5 |
        SAA7134_MAIN_CTRL_TE4 |
        SAA7134_MAIN_CTRL_TE3 |
        SAA7134_MAIN_CTRL_TE2 |
        SAA7134_MAIN_CTRL_TE1 |
        SAA7134_MAIN_CTRL_TE0);

    // Disable all regions
    WriteByte(SAA7134_REGION_ENABLE, 0x00);
}


void CSAA7134Card::ManageMyState()
{
    int i;

    ManageByte(SAA7134_INCR_DELAY);
    ManageByte(SAA7134_ANALOG_IN_CTRL1);
    ManageByte(SAA7134_ANALOG_IN_CTRL2);
    ManageByte(SAA7134_ANALOG_IN_CTRL3);
    ManageByte(SAA7134_ANALOG_IN_CTRL4);

    ManageByte(SAA7134_HSYNC_START);
    ManageByte(SAA7134_HSYNC_STOP);
    ManageByte(SAA7134_SYNC_CTRL);
    ManageByte(SAA7134_LUMA_CTRL);
    ManageByte(SAA7134_DEC_LUMA_BRIGHT);
    ManageByte(SAA7134_DEC_LUMA_CONTRAST);
    ManageByte(SAA7134_DEC_CHROMA_SATURATION);
    ManageByte(SAA7134_DEC_CHROMA_HUE);
    ManageByte(SAA7134_CHROMA_CTRL1);
    ManageByte(SAA7134_CHROMA_GAIN_CTRL);
    ManageByte(SAA7134_CHROMA_CTRL2);
    ManageByte(SAA7134_MODE_DELAY_CTRL);
    ManageByte(SAA7134_ANALOG_ADC);
    ManageByte(SAA7134_VGATE_START);
    ManageByte(SAA7134_VGATE_STOP);
    ManageByte(SAA7134_MISC_VGATE_MSB);
    ManageByte(SAA7134_RAW_DATA_GAIN);
    ManageByte(SAA7134_RAW_DATA_OFFSET);
    ManageByte(SAA7134_STATUS_VIDEO);
    ManageByte(SAA7134_STATUS_VIDEO_HIBYTE);

    for (i = 0; i < 16; i++)
    {
        ManageByte(SAA7134_GREEN_PATH(i));
        ManageByte(SAA7134_BLUE_PATH(i));
        ManageByte(SAA7134_RED_PATH(i));
    }

    ManageByte(SAA7134_START_GREEN);
    ManageByte(SAA7134_START_BLUE);
    ManageByte(SAA7134_START_RED);

    for (i = SAA7134_TASK_A_MASK; i < 0x077; i++)
    {
        ManageByte(i);
    }

    for (i = SAA7134_TASK_B_MASK; i < 0x0B7; i++)
    {
        ManageByte(i);
    }

    ManageByte(SAA7134_OFMT_VIDEO_A);
    ManageByte(SAA7134_OFMT_DATA_A);
    ManageByte(SAA7134_OFMT_VIDEO_B);
    ManageByte(SAA7134_OFMT_DATA_B);
    ManageByte(SAA7134_ALPHA_NOCLIP);
    ManageByte(SAA7134_ALPHA_CLIP);
    ManageByte(SAA7134_UV_PIXEL);
    ManageByte(SAA7134_CLIP_RED);
    ManageByte(SAA7134_CLIP_GREEN);
    ManageByte(SAA7134_CLIP_BLUE);

    /* We don't really need to save these
    for (i = 0; i < 16; i++)
    {
        ManageByte(SAA7134_CLIP_H_ACTIVE(i));
        ManageByte(SAA7134_CLIP_H_NOIDEA(i));
        ManageByte(SAA7134_CLIP_H_POS(i));
        ManageByte(SAA7134_CLIP_H_POS_HIBYTE(i));
        ManageByte(SAA7134_CLIP_V_ACTIVE(i));
        ManageByte(SAA7134_CLIP_V_NOIDEA(i));
        ManageByte(SAA7134_CLIP_V_POS(i));
        ManageByte(SAA7134_CLIP_V_POS_HIBYTE(i));
    }
    */

    ManageByte(SAA7134_DCXO_IDENT_CTRL);
    ManageByte(SAA7134_DEMODULATOR);
    ManageByte(SAA7134_AGC_GAIN_SELECT);
    ManageDword(SAA7134_CARRIER1_FREQ);
    ManageDword(SAA7134_CARRIER2_FREQ);
    ManageDword(SAA7134_NUM_SAMPLES);
    ManageByte(SAA7134_AUDIO_FORMAT_CTRL);
    ManageByte(SAA7134_MONITOR_SELECT);
    ManageByte(SAA7134_FM_DEEMPHASIS);
    ManageByte(SAA7134_FM_DEMATRIX);
    ManageByte(SAA7134_CHANNEL1_LEVEL);
    ManageByte(SAA7134_CHANNEL2_LEVEL);
    ManageByte(SAA7134_NICAM_CONFIG);
    ManageByte(SAA7134_NICAM_LEVEL_ADJUST);
    ManageByte(SAA7134_STEREO_DAC_OUTPUT_SELECT);
    ManageByte(SAA7134_I2S_OUTPUT_FORMAT);
    ManageByte(SAA7134_I2S_OUTPUT_SELECT);
    ManageByte(SAA7134_I2S_OUTPUT_LEVEL);
    ManageByte(SAA7134_DSP_OUTPUT_SELECT);

    // This stops unmuting on exit
    // ManageByte(SAA7134_AUDIO_MUTE_CTRL);

    ManageByte(SAA7134_SIF_SAMPLE_FREQ);

    // Managing this causes problems for cards that
    // use audio line for muting
    //ManageByte(SAA7134_ANALOG_IO_SELECT);

    ManageDword(SAA7134_AUDIO_CLOCK);
    ManageByte(SAA7134_AUDIO_PLL_CTRL);
    ManageDword(SAA7134_AUDIO_CLOCKS_PER_FIELD);

    ManageByte(SAA7134_VIDEO_PORT_CTRL0);
    ManageByte(SAA7134_VIDEO_PORT_CTRL1);
    ManageByte(SAA7134_VIDEO_PORT_CTRL2);
    ManageByte(SAA7134_VIDEO_PORT_CTRL3);
    ManageByte(SAA7134_VIDEO_PORT_CTRL4);
    ManageByte(SAA7134_VIDEO_PORT_CTRL5);
    ManageByte(SAA7134_VIDEO_PORT_CTRL6);
    ManageByte(SAA7134_VIDEO_PORT_CTRL7);
    ManageByte(SAA7134_VIDEO_PORT_CTRL8);

    ManageByte(SAA7134_TS_PARALLEL);
    ManageByte(SAA7134_TS_PARALLEL_SERIAL);
    ManageByte(SAA7134_TS_SERIAL0);
    ManageByte(SAA7134_TS_SERIAL1);
    ManageByte(SAA7134_TS_DMA0);
    ManageByte(SAA7134_TS_DMA1);
    ManageByte(SAA7134_TS_DMA2);

    ManageByte(SAA7134_I2S_AUDIO_OUTPUT);
    ManageByte(SAA7134_SPECIAL_MODE);

    if (m_DeviceId == 0x7133 || m_DeviceId == 0x7135)
    {
        ManageData(SAA7133_ANALOG_IO_SELECT);
    }

    // do these ones last
    /* It is probably safer if we leave DMA
       and IRQ stuff zeroed when we're done.
    ManageWord(SAA7134_SOURCE_TIMING);
    ManageByte(SAA7134_REGION_ENABLE);

    for (i = 0; i < 7; i++)
    {
        ManageDword(SAA7134_RS_BA1(i));
        ManageDword(SAA7134_RS_BA2(i));
        ManageDword(SAA7134_RS_PITCH(i));

        ManageByte(SAA7134_RS_CONTROL_0(i));
        ManageByte(SAA7134_RS_CONTROL_1(i));
        ManageByte(SAA7134_RS_CONTROL_2(i));
        ManageByte(SAA7134_RS_CONTROL_3(i));
    }

    ManageDword(SAA7134_FIFO_SIZE);
    ManageDword(SAA7134_THRESHOULD);

    ManageDword(SAA7134_MAIN_CTRL);

    ManageDword(SAA7134_IRQ1);
    ManageDword(SAA7134_IRQ2);
    */
}


void CSAA7134Card::ResetHardware()
{
    WriteByte(SAA7134_REGION_ENABLE, 0x00);
    WriteByte(SAA7134_REGION_ENABLE, SAA7134_REGION_ENABLE_SWRST);
    WriteByte(SAA7134_REGION_ENABLE, 0x00);

    // LOG(0, _T("Initial registery dump"));
    // DumpRegisters();

    WriteWord(SAA7134_SOURCE_TIMING, SAA7134_SOURCE_TIMING_DVED);

    WriteByte(SAA7134_START_GREEN, 0x00);
    WriteByte(SAA7134_START_BLUE, 0x00);
    WriteByte(SAA7134_START_RED, 0x00);

    for (int i = 0; i < 0x0F; i++)
    {
        WriteByte(SAA7134_GREEN_PATH(i), (i+1)<<4);
        WriteByte(SAA7134_BLUE_PATH(i), (i+1)<<4);
        WriteByte(SAA7134_RED_PATH(i), (i+1)<<4);
    }
    WriteByte(SAA7134_GREEN_PATH(0x0F), 0xFF);
    WriteByte(SAA7134_BLUE_PATH(0x0F), 0xFF);
    WriteByte(SAA7134_RED_PATH(0x0F), 0xFF);

    // RAM FIFO config ???
    WriteDword(SAA7134_FIFO_SIZE, 0x08070503);
    WriteDword(SAA7134_THRESHOULD, 0x02020202);

    // Enable audio and video processing
    WriteDword(SAA7134_MAIN_CTRL,
            SAA7134_MAIN_CTRL_VPLLE |
            SAA7134_MAIN_CTRL_APLLE |
            SAA7134_MAIN_CTRL_EXOSC |
            SAA7134_MAIN_CTRL_EVFE1 |
            SAA7134_MAIN_CTRL_EVFE2 |
            SAA7134_MAIN_CTRL_ESFE  |
            SAA7134_MAIN_CTRL_EBADC |
            SAA7134_MAIN_CTRL_EBDAC);

    WriteByte(SAA7134_SPECIAL_MODE, 0x01);

    InitAudio();

    SetTypicalSettings();
    SetupTasks();
}


void CSAA7134Card::SetTypicalSettings()
{
    // Typical settings for video decoder (according to saa7134 manual(v0.1))

    WriteByte(SAA7134_INCR_DELAY,               0x08);
    WriteByte(SAA7134_ANALOG_IN_CTRL1,          0xC0);
    WriteByte(SAA7134_ANALOG_IN_CTRL2,          0x00);
    WriteByte(SAA7134_ANALOG_IN_CTRL3,          0x90);
    WriteByte(SAA7134_ANALOG_IN_CTRL4,          0x90);
    WriteByte(SAA7134_HSYNC_START,              0xEB);
    WriteByte(SAA7134_HSYNC_STOP,               0xE0);

    WriteByte(SAA7134_SYNC_CTRL,                0xB8);  // standard adjust
    WriteByte(SAA7134_LUMA_CTRL,                0x40);  // standard adjust
    WriteByte(SAA7134_DEC_LUMA_BRIGHT,          0x80);  // bright 128
    WriteByte(SAA7134_DEC_LUMA_CONTRAST,        0x44);  // contrast 68
    // Saturation: Auto=64, PAL=61, SECAM=64, NTSC=67
    WriteByte(SAA7134_DEC_CHROMA_SATURATION,    0x40);  // saturation 64
    WriteByte(SAA7134_DEC_CHROMA_HUE,           0x00);  // hue 0
    WriteByte(SAA7134_CHROMA_CTRL1,             0x8B);  // standard adjust
    WriteByte(SAA7134_CHROMA_GAIN_CTRL,         0x00);  // standard adjust
    WriteByte(SAA7134_CHROMA_CTRL2,             0x00);  // standard adjust
    WriteByte(SAA7134_MODE_DELAY_CTRL,          0x00);
    WriteByte(SAA7134_ANALOG_ADC,               0x41);
    WriteByte(SAA7134_VGATE_START,              0x11);
    WriteByte(SAA7134_VGATE_STOP,               0xFE);
    WriteByte(SAA7134_MISC_VGATE_MSB,           0x18);  // standard adjust
    WriteByte(SAA7134_RAW_DATA_GAIN,            0x40);
    WriteByte(SAA7134_RAW_DATA_OFFSET,          0x80);
}


void CSAA7134Card::SetupTasks()
{
    // 0x00 = YUV2
    WriteByte(SAA7134_OFMT_VIDEO_A, 0x00);
    WriteByte(SAA7134_OFMT_VIDEO_B, 0x00);
    // 0x06 = raw VBI
    WriteByte(SAA7134_OFMT_DATA_A, 0x06);
    WriteByte(SAA7134_OFMT_DATA_B, 0x06);

    // Let Task A get Odd then Even field, follow by Task B
    // getting Odd then Even field. (Odd is DScaler's Even)

    WriteByte(SAA7134_TASK_CONDITIONS(SAA7134_TASK_A_MASK), 0x0D);
    WriteByte(SAA7134_TASK_CONDITIONS(SAA7134_TASK_B_MASK), 0x0D);

    // handle two fields per task
    WriteByte(SAA7134_FIELD_HANDLING(SAA7134_TASK_A_MASK), 0x02);
    WriteByte(SAA7134_FIELD_HANDLING(SAA7134_TASK_B_MASK), 0x02);

    WriteByte(SAA7134_V_FILTER(SAA7134_TASK_A_MASK), 0x00);
    WriteByte(SAA7134_V_FILTER(SAA7134_TASK_B_MASK), 0x00);

    WriteByte(SAA7134_LUMA_CONTRAST(SAA7134_TASK_A_MASK), 0x40);
    WriteByte(SAA7134_LUMA_CONTRAST(SAA7134_TASK_B_MASK), 0x40);

    WriteByte(SAA7134_CHROMA_SATURATION(SAA7134_TASK_A_MASK), 0x40);
    WriteByte(SAA7134_CHROMA_SATURATION(SAA7134_TASK_B_MASK), 0x40);

    WriteByte(SAA7134_LUMA_BRIGHT(SAA7134_TASK_A_MASK), 0x80);
    WriteByte(SAA7134_LUMA_BRIGHT(SAA7134_TASK_B_MASK), 0x80);

    WriteWord(SAA7134_H_PHASE_OFF_LUMA(SAA7134_TASK_A_MASK), 0x00);
    WriteWord(SAA7134_H_PHASE_OFF_LUMA(SAA7134_TASK_B_MASK), 0x00);
    WriteWord(SAA7134_H_PHASE_OFF_CHROMA(SAA7134_TASK_A_MASK), 0x00);
    WriteWord(SAA7134_H_PHASE_OFF_CHROMA(SAA7134_TASK_B_MASK), 0x00);

    // 0x88 seems to clean up colour crosstalk
    WriteByte(SAA7134_FIR_PREFILTER_CTRL(SAA7134_TASK_A_MASK), 0x88);
    WriteByte(SAA7134_FIR_PREFILTER_CTRL(SAA7134_TASK_B_MASK), 0x88);

    WriteByte(SAA7134_VBI_PHASE_OFFSET_LUMA(SAA7134_TASK_A_MASK), 0x00);
    WriteByte(SAA7134_VBI_PHASE_OFFSET_LUMA(SAA7134_TASK_B_MASK), 0x00);
    WriteByte(SAA7134_VBI_PHASE_OFFSET_CHROMA(SAA7134_TASK_A_MASK), 0x00);
    WriteByte(SAA7134_VBI_PHASE_OFFSET_CHROMA(SAA7134_TASK_B_MASK), 0x00);

    WriteByte(SAA7134_DATA_PATH(SAA7134_TASK_A_MASK), 0x03);
    WriteByte(SAA7134_DATA_PATH(SAA7134_TASK_B_MASK), 0x03);

    // deinterlace y offsets ?? no idea what these are
    // Odds: 0x00 default
    // Evens: 0x00 default + yscale(1024) / 0x20;
    // ---
    // We can tweak these to change the top line to even (by giving
    // even an offset of 0x20) but it doesn't change VBI
    WriteByte(SAA7134_V_PHASE_OFFSET0(SAA7134_TASK_A_MASK), 0x00); // Odd
    WriteByte(SAA7134_V_PHASE_OFFSET1(SAA7134_TASK_A_MASK), 0x00); // Even
    WriteByte(SAA7134_V_PHASE_OFFSET2(SAA7134_TASK_A_MASK), 0x00); // Odd
    WriteByte(SAA7134_V_PHASE_OFFSET3(SAA7134_TASK_A_MASK), 0x00); // Even

    WriteByte(SAA7134_V_PHASE_OFFSET0(SAA7134_TASK_B_MASK), 0x00); // Odd
    WriteByte(SAA7134_V_PHASE_OFFSET1(SAA7134_TASK_B_MASK), 0x00); // Even
    WriteByte(SAA7134_V_PHASE_OFFSET2(SAA7134_TASK_B_MASK), 0x00); // Odd
    WriteByte(SAA7134_V_PHASE_OFFSET3(SAA7134_TASK_B_MASK), 0x00); // Even
}


/*
 *  SetPageTable
 *
 *  If page tables aren't being used, pPhysical should be 0 and
 *  nPages should contain the size of the memory block that will be
 *  used with this channel.
 *
 *  RegionID is one of: REGIONID_VIDEO_A, REGIONID_VIDEO_B
 *                      REGIONID_VBI_A, REGIONID_VBI_B
 */
void CSAA7134Card::SetPageTable(eRegionID RegionID, DWORD pPhysical, DWORD nPages)
{
    int Channel = RegionID2Channel(RegionID);

    if (pPhysical != 0)
    {
        DWORD Page = pPhysical >> 12;

        // ME must be enabled for page tables to work
        WriteByte(SAA7134_RS_CONTROL_0(Channel), (BYTE)(Page & 0xFF));
        WriteByte(SAA7134_RS_CONTROL_1(Channel), (BYTE)(Page >> 8 & 0xFF));
        WriteByte(SAA7134_RS_CONTROL_2(Channel), (BYTE)(Page >> 16 |
            SAA7134_RS_CONTROL_2_ME | SAA7134_RS_CONTROL_2_BURST_MAX));

        m_DMAChannelMemorySize[Channel] = nPages * 4096;
    }
    else
    {
        WriteByte(SAA7134_RS_CONTROL_0(Channel), 0x00);
        WriteByte(SAA7134_RS_CONTROL_1(Channel), 0x00);
        WriteByte(SAA7134_RS_CONTROL_2(Channel), SAA7134_RS_CONTROL_2_BURST_MAX);

        m_DMAChannelMemorySize[Channel] = nPages;
    }
}


void CSAA7134Card::SetBaseOffsets(eRegionID RegionID,
                                      DWORD dwEvenOffset,
                                      DWORD dwOddOffset,
                                      DWORD dwPitch
                                  )
{
    int Channel = RegionID2Channel(RegionID);

    // WARNING!!  SAA7134 thinks the top line is odd but DScaler expects
    //            it to be even.  Do the conversion in here in SAA7134Card.cpp
    //            so we don't confuse ourselves... (I hope)  Make sure everything
    //            we give back to SAA7134Source.cpp has already been converted.
    //            GetProcessingRegion() and GetIRQEventRegion() and should have
    //            the conversion back.
    //
    // SAA7134 writes the upper field to BA1 the lower field to B2, but
    // for some reason, B2 (lower) gets written to processed before B1.
    //
    // Number bytes to offset into every page
    // Give the even offset as odd and odd offset as even
    WriteDword(SAA7134_RS_BA2(Channel), dwOddOffset);
    WriteDword(SAA7134_RS_BA1(Channel), dwEvenOffset);

    // Number of bytes to spend per line
    WriteDword(SAA7134_RS_PITCH(Channel), dwPitch);
}


void CSAA7134Card::SetBSwapAndWSwap(eRegionID RegionID, BOOL bBSwap, BOOL bWSwap)
{
    int Channel = RegionID2Channel(RegionID);

    WriteByte(SAA7134_RS_CONTROL_3(Channel),
            (bBSwap ? SAA7134_RS_CONTROL_3_BSWAP : 0x00) |
            (bWSwap ? SAA7134_RS_CONTROL_3_WSWAP : 0x00)
        );
}


void CSAA7134Card::VerifyMemorySize(eRegionID RegionID)
{
    WORD LinesAvailable;
    WORD BytesPerLine;

    // Only Video and VBI memory size checking is implemented
    if (!IsRegionIDVideo(RegionID) && !IsRegionIDVBI(RegionID))
    {
        return;
    }

    BYTE TaskMask = TaskID2TaskMask(RegionID2TaskID(RegionID));

    if (IsRegionIDVBI(RegionID))
    {
        BytesPerLine = ReadWord(SAA7134_VBI_H_LEN(TaskMask));
    }
    else
    {
        // YUV is 2 bytes per pixel
        BytesPerLine = ReadWord(SAA7134_VIDEO_PIXELS(TaskMask)) * 2;
    }

    LinesAvailable = CalculateLinesAvailable(RegionID, BytesPerLine);

    if (IsRegionIDVBI(RegionID))
    {
        if (LinesAvailable < ReadWord(SAA7134_VBI_V_LEN(TaskMask)))
        {
            WriteWord(SAA7134_VBI_H_LEN(TaskMask), LinesAvailable);
        }
    }
    else
    {
        if (LinesAvailable < ReadWord(SAA7134_VIDEO_LINES(TaskMask)))
        {
            WriteWord(SAA7134_VIDEO_LINES(TaskMask), LinesAvailable);
        }
    }
}


WORD CSAA7134Card::CalculateLinesAvailable(eRegionID RegionID, WORD wBytesPerLine)
{
    int Channel = RegionID2Channel(RegionID);

    DWORD Offset1     = ReadDword(SAA7134_RS_BA1(Channel));
    DWORD Offset2     = ReadDword(SAA7134_RS_BA2(Channel));
    DWORD Pitch         = ReadDword(SAA7134_RS_PITCH(Channel));

    DWORD MaxBaseOffset = (Offset2 > Offset1) ? Offset2 : Offset1;
    DWORD MinimumBytesAvailable;

    MinimumBytesAvailable = m_DMAChannelMemorySize[Channel];

    if (MinimumBytesAvailable < MaxBaseOffset)
    {
        return 0;
    }

    MinimumBytesAvailable -= MaxBaseOffset;
    if (MinimumBytesAvailable < wBytesPerLine)
    {
        return 0;
    }

    return (WORD)((MinimumBytesAvailable - wBytesPerLine) / Pitch + 1);
}


void CSAA7134Card::SetDMA(eRegionID RegionID, BOOL bState)
{
    BYTE    Region = 0;
    DWORD   IRQs = 0;
    DWORD   Control = 0;

    switch (RegionID)
    {
    case REGIONID_VIDEO_A:
        Region = SAA7134_REGION_ENABLE_VID_ENA;
        Control = SAA7134_MAIN_CTRL_TE0;
        IRQs = SAA7134_IRQ1_INTE_RA0_0 | SAA7134_IRQ1_INTE_RA0_1;
        break;
    case REGIONID_VIDEO_B:
        Region = SAA7134_REGION_ENABLE_VID_ENB;
        Control = SAA7134_MAIN_CTRL_TE1;
        IRQs = SAA7134_IRQ1_INTE_RA0_2 | SAA7134_IRQ1_INTE_RA0_3;
        break;
    case REGIONID_VBI_A:
        Region = SAA7134_REGION_ENABLE_VBI_ENA;
        Control = SAA7134_MAIN_CTRL_TE2;
        IRQs = SAA7134_IRQ1_INTE_RA0_4 | SAA7134_IRQ1_INTE_RA0_5;
        break;

    case REGIONID_VBI_B:
        Region = SAA7134_REGION_ENABLE_VBI_ENB;
        Control = SAA7134_MAIN_CTRL_TE3;
        IRQs = SAA7134_IRQ1_INTE_RA0_6 | SAA7134_IRQ1_INTE_RA0_7;
        break;
    }

    if (bState) {
        m_PreparedRegions |= Region;
        // Don't turn on interrupts because we don't have an ISR!
        // MaskDataDword(SAA7134_IRQ1, IRQs, IRQs);
        MaskDataDword(SAA7134_MAIN_CTRL, Control, Control);
    }
    else {
        m_PreparedRegions &= ~Region;
        AndDataByte(SAA7134_REGION_ENABLE, ~Region);
        MaskDataDword(SAA7134_IRQ1, 0x00, IRQs);
        MaskDataDword(SAA7134_MAIN_CTRL, 0x00, Control);
    }
}


BOOL CSAA7134Card::GetDMA(eRegionID RegionID)
{
    DWORD Mask = 0;

    switch (RegionID) {
    case REGIONID_VIDEO_A: Mask = SAA7134_MAIN_CTRL_TE0; break;
    case REGIONID_VIDEO_B: Mask = SAA7134_MAIN_CTRL_TE1; break;
    case REGIONID_VBI_A: Mask = SAA7134_MAIN_CTRL_TE2; break;
    case REGIONID_VBI_B: Mask = SAA7134_MAIN_CTRL_TE3; break;
    }

    return (ReadDword(SAA7134_MAIN_CTRL) & Mask) > 0;
}


void CSAA7134Card::StartCapture(BOOL bCaptureVBI)
{
    BYTE Region;

    Region = SAA7134_REGION_ENABLE_VID_ENA | SAA7134_REGION_ENABLE_VID_ENB;

    VerifyMemorySize(REGIONID_VIDEO_A);
    VerifyMemorySize(REGIONID_VIDEO_B);

    if (bCaptureVBI)
    {
        CheckVBIAndVideoOverlap(TASKID_A);
        CheckVBIAndVideoOverlap(TASKID_B);

        VerifyMemorySize(REGIONID_VBI_A);
        VerifyMemorySize(REGIONID_VBI_B);

        Region |= SAA7134_REGION_ENABLE_VBI_ENA | SAA7134_REGION_ENABLE_VBI_ENB;
    }

    MaskDataByte(SAA7134_REGION_ENABLE, Region, m_PreparedRegions);
}


void CSAA7134Card::StopCapture()
{
    WriteByte(SAA7134_REGION_ENABLE, 0x00);
}


BOOL CSAA7134Card::IsVideoPresent()
{
    WORD CheckMask = SAA7134_STATUS_VIDEO_HLCK;

    if ((ReadWord(SAA7134_STATUS_VIDEO) & CheckMask) == 0)
    {
        return TRUE;
    }

    return FALSE;
}


/*
 * data comes in:
 *       FIDSCI and TASK set
 * data written out:
 *       VID_A / VID_B and FIDSCO set
 *       (DMA status DONE cleared, status buffer toggled)
 * data out finished:
 *       (IRQ fired & DMA status DONE set)
 */

// Gets the field the card is currently processing
BOOL CSAA7134Card::GetProcessingRegion(eRegionID& RegionID, BOOL& bIsFieldOdd)
{
    WORD Status;

    Status  = ReadWord(SAA7134_SCALER_STATUS);

    if (Status & (SAA7134_SCALER_STATUS_TRERR |
        SAA7134_SCALER_STATUS_CFERR |
        SAA7134_SCALER_STATUS_LDERR |
        SAA7134_SCALER_STATUS_WASRST))
    {
        CheckScalerError(TRUE, Status);
        return FALSE;
    }

    CheckScalerError(FALSE, Status);

    // FIDSCI XOR FIDSCO should be 0
    if (Status & SAA7134_SCALER_STATUS_D6_D5)
    {
        return FALSE;
    }

    if (Status & SAA7134_SCALER_STATUS_VID_A)
    {
        RegionID = REGIONID_VIDEO_A;
    }
    else if (Status & SAA7134_SCALER_STATUS_VID_B)
    {
        RegionID = REGIONID_VIDEO_B;
    }
    else
    {
        return FALSE;
    }

    if (Status & SAA7134_SCALER_STATUS_FIDSCO)
    {
        bIsFieldOdd = FALSE;
    }
    else
    {
        bIsFieldOdd = TRUE;
    }

    // Everything above is SAA7134 style, here we convert evens
    // to odd and odds to even so SAA7134Source can take even
    // has to top line --instead of odd which SAA7134 uses.
    //   ala. CSAACard::SetBaseOffsets()
    bIsFieldOdd = !bIsFieldOdd;

    return TRUE;
}


BOOL CSAA7134Card::GetProcessingFieldID(TFieldID* pFieldID)
{
    WORD Status;
    BYTE FieldID;

    Status  = ReadWord(SAA7134_SCALER_STATUS);

    if (Status & (SAA7134_SCALER_STATUS_TRERR |
        SAA7134_SCALER_STATUS_CFERR |
        SAA7134_SCALER_STATUS_LDERR |
        SAA7134_SCALER_STATUS_WASRST))
    {
        CheckScalerError(TRUE, Status);
        return FALSE;
    }

    CheckScalerError(FALSE, Status);

    // FIDSCI XOR FIDSCO should be 0
    if (Status & SAA7134_SCALER_STATUS_D6_D5)
    {
        return FALSE;
    }

    if (Status & SAA7134_SCALER_STATUS_VID_A)
    {
        FieldID = 0;
    }
    else if (Status & SAA7134_SCALER_STATUS_VID_B)
    {
        FieldID = 1;
    }
    else
    {
        return FALSE;
    }

    FieldID <<= FIELDID_FRAMESHIFT;

    if ((Status & SAA7134_SCALER_STATUS_FIDSCO) == 0)
    {
        FieldID |= FIELDID_SECONDFIELD;
    }

    *pFieldID = FieldID;

    return TRUE;
}


/*
 * CheckScalerError
 *
 * Tries to recover from card errors by resetting various
 * parts of the card.
 */
void CSAA7134Card::CheckScalerError(BOOL bErrorOccurred, WORD ScalerStatus)
{
    if (!bErrorOccurred)
    {
        m_LastTriggerError = 0UL;
        return;
    }

    if (ScalerStatus & SAA7134_SCALER_STATUS_TRERR)
    {
        // - Wait 1 second from the first time this occurred
        // and if it's still a problem, reset the task
        // conditions.  Hopefully, the card will sort itself out.
        //
        // - If the last sighting is older than 2 seconds prior,
        // CheckScalerError() wasn't called consistently enough
        // for the value to be useful.

        DWORD CurrentTick = GetTickCount();

        if (CurrentTick < m_LastTriggerError ||
            CurrentTick > m_LastTriggerError + 2000)
        {
            m_LastTriggerError = CurrentTick;
        }
        else if (CurrentTick > m_LastTriggerError + 1000)
        {
            LOG(2, _T("SAA7134: Trying Trigger Error recovery"));
            WriteByte(SAA7134_TASK_CONDITIONS(SAA7134_TASK_A_MASK), 0x00);
            WriteByte(SAA7134_TASK_CONDITIONS(SAA7134_TASK_B_MASK), 0x00);
            WriteByte(SAA7134_TASK_CONDITIONS(SAA7134_TASK_A_MASK), 0x0D);
            WriteByte(SAA7134_TASK_CONDITIONS(SAA7134_TASK_B_MASK), 0x0D);

            m_LastTriggerError = CurrentTick;
        }
    }
    else if (ScalerStatus & SAA7134_SCALER_STATUS_CFERR)
    {
        // This one sorts itself out.
    }
    else
    {
        // do nothing
    }

    /*
    LOG(0, _T("Scaler Status: %s%s%s%s%s%s%s%s%s%s%s%s"),
        (ScalerStatus & SAA7134_SCALER_STATUS_VID_A) ? _T("VID_A ") : _T(""),
        (ScalerStatus & SAA7134_SCALER_STATUS_VBI_A) ? _T("VBI_A ") : _T(""),
        (ScalerStatus & SAA7134_SCALER_STATUS_VID_B) ? _T("VID_B ") : _T(""),
        (ScalerStatus & SAA7134_SCALER_STATUS_VBI_B) ? _T("VBI_B ") : _T(""),
        (ScalerStatus & SAA7134_SCALER_STATUS_TRERR) ? _T("TRERR ") : _T(""),
        (ScalerStatus & SAA7134_SCALER_STATUS_CFERR) ? _T("CFERR ") : _T(""),
        (ScalerStatus & SAA7134_SCALER_STATUS_LDERR) ? _T("LDERR ") : _T(""),
        (ScalerStatus & SAA7134_SCALER_STATUS_WASRST) ? _T("WASRST ") : _T(""),
        (ScalerStatus & SAA7134_SCALER_STATUS_FIDSCI) ? _T("FIDSCI ") : _T(""),
        (ScalerStatus & SAA7134_SCALER_STATUS_FIDSCO) ? _T("FIDSCO ") : _T(""),
        (ScalerStatus & SAA7134_SCALER_STATUS_D6_D5) ? _T("D6_D5 ") : _T(""),
        (ScalerStatus & SAA7134_SCALER_STATUS_TASK) ? _T("TASK ") : _T("")
    );
    */
}


/*
 *  I2C Stuff
 */

BYTE CSAA7134Card::GetI2CStatus()
{
    return ReadByte(SAA7134_I2C_ATTR_STATUS) & 0x0F;
}


void CSAA7134Card::SetI2CStatus(BYTE Status)
{
    MaskDataByte(SAA7134_I2C_ATTR_STATUS, Status, 0x0F);
}


void CSAA7134Card::SetI2CCommand(BYTE Command)
{
    MaskDataByte(SAA7134_I2C_ATTR_STATUS, Command, 0xC0);
}


void CSAA7134Card::SetI2CData(BYTE Data)
{
    WriteByte(SAA7134_I2C_DATA, Data);
}


BYTE CSAA7134Card::GetI2CData()
{
    return ReadByte(SAA7134_I2C_DATA);
}

void CSAA7134Card::I2CLock()
{
    LockCard();
}

void CSAA7134Card::I2CUnlock()
{
    UnlockCard();
}


BOOL APIENTRY CSAA7134Card::ChipSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    CSAA7134Card* pThis = NULL;
    TCHAR szCardId[9] = _T("none");
    TCHAR szVendorId[9] = _T("none");
    TCHAR szDeviceId[9] = _T("none");
    DWORD dwCardId(0);

    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CSAA7134Card*)lParam;

        SetDlgItemText(hDlg, IDC_BT_CHIP_TYPE, pThis->GetChipType().c_str());

        _stprintf(szVendorId,_T("%04X"), pThis->GetVendorId());
        SetDlgItemText(hDlg, IDC_BT_VENDOR_ID, szVendorId);

        _stprintf(szDeviceId,_T("%04X"), pThis->GetDeviceId());
        SetDlgItemText(hDlg, IDC_BT_DEVICE_ID, szDeviceId);

        SetDlgItemText(hDlg, IDC_TUNER_TYPE, pThis->GetTunerType().c_str());

        if ((pThis->m_DeviceId == 0x7134) || pThis->m_DeviceId == 0x7133)
        {
            SetDlgItemText(hDlg, IDC_AUDIO_DECODER_TYPE, _T("SAA713x OnChip"));
        }
        else
        {
            SetDlgItemText(hDlg, IDC_AUDIO_DECODER_TYPE, _T("n/a"));
        }

        dwCardId = pThis->GetSubSystemId();
        if (dwCardId != 0x00001131)
        {
            _stprintf(szCardId, _T("%08X"), dwCardId);
        }
        SetDlgItemText(hDlg, IDC_AUTODECTECTID, szCardId);
        return TRUE;
        break;

    case WM_COMMAND:
        if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
        {
            EndDialog(hDlg, TRUE);
        }
        break;
    }

    return (FALSE);
}


tstring CSAA7134Card::GetChipType()
{
    switch (m_DeviceId)
    {
    case 0x7130:
        return _T("SAA7130");
    case 0x7133:
        return _T("SAA7133");
    case 0x7134:
        return _T("SAA7134");
    }
    return _T("n/a");
}


/*
 *  UNUSED STUFF
 */

// \CURRENTLY UNUSED
// Gets the field the card just finished processing (if IRQs work)
BOOL CSAA7134Card::GetIRQEventRegion(eRegionID& RegionID, BOOL& bIsFieldOdd)
{
    DWORD Report;
    DWORD Status;

    Report = ReadDword(SAA7134_IRQ_REPORT);
    Status = ReadDword(SAA7134_IRQ_STATUS);
    WriteDword(SAA7134_IRQ_REPORT, Report);

    if (Report == 0)
    {
        return FALSE;
    }

    BOOL bIsTaskA = (Status & 0x20);

    if (Status & 0x40)
    {
        RegionID = bIsTaskA ? REGIONID_VBI_A : REGIONID_VBI_B;
    }
    else
    {
        RegionID = bIsTaskA ? REGIONID_VIDEO_A : REGIONID_VIDEO_B;
    }

    bIsFieldOdd = (Status & 0x10);

    // Everything above is SAA7134 style, here we convert evens
    // to odd and odds to even so SAA7137Source can take even
    // has to top line --instead of odd which SAA7137 uses.
    //   ala. CSAACard::SetBaseOffsets()
    bIsFieldOdd = !bIsFieldOdd;

    return TRUE;
}


// \CURRENTLY UNUSED
// Unused - v4l2 uses this if i2s_rate exists on the card
void CSAA7134Card::EnableI2SAudioOutput(WORD wRate)
{
    // set rate
    MaskDataByte(SAA7134_SIF_SAMPLE_FREQ, wRate == 32000 ? 0x01 : 0x03, 0x03);

    // enable I2S output -- no idea
    WriteByte(SAA7134_DSP_OUTPUT_SELECT,    0x80);
    WriteByte(SAA7134_I2S_OUTPUT_SELECT,    0x80);
    WriteByte(SAA7134_I2S_OUTPUT_FORMAT,    0x01);
    WriteByte(SAA7134_I2S_OUTPUT_LEVEL,     0x00);
    WriteByte(SAA7134_I2S_AUDIO_OUTPUT,     0x01);
}


// \CURRENTLY UNUSED
// Unused, (v4l2 checks video_out existance in saa7134-cards.c)
void CSAA7134Card::EnableCCIR656VideoOut()
{
    // enable video output for CCIR656
    WriteByte(SAA7134_VIDEO_PORT_CTRL0, 0x00);
    WriteByte(SAA7134_VIDEO_PORT_CTRL1, 0xB1);
    WriteByte(SAA7134_VIDEO_PORT_CTRL2, 0x00);
    WriteByte(SAA7134_VIDEO_PORT_CTRL3, 0xA1);
    WriteByte(SAA7134_VIDEO_PORT_CTRL4, 0x00);
    WriteByte(SAA7134_VIDEO_PORT_CTRL5, 0x04);
    WriteByte(SAA7134_VIDEO_PORT_CTRL6, 0x06);
    WriteByte(SAA7134_VIDEO_PORT_CTRL7, 0x00);
    WriteByte(SAA7134_VIDEO_PORT_CTRL8, 0x00);
}



/*
 *  TEMPORALLY DEBUGGING STUFF
 */

// \TEMP DEBUG
void CSAA7134Card::DumpRegisters()
{
    DWORD   Data;
    TCHAR   HexString[64];

    for (int i = 0x000; i < 0x400; i += 16)
    {
        *HexString = '\0';

        Data = ReadDword(i);
        _stprintf(HexString, _T("%02x%02x %02x%02x"),
            Data & 0xFF, (Data >> 8) & 0xFF, (Data >> 16) & 0xFF, (Data >> 24) & 0xFF);

        Data = ReadDword(i + 4);
        _stprintf(HexString, _T("%s %02x%02x %02x%02x"), HexString,
            Data & 0xFF, (Data >> 8) & 0xFF, (Data >> 16) & 0xFF, (Data >> 24) & 0xFF);

        Data = ReadDword(i + 8);
        _stprintf(HexString, _T("%s|%02x%02x %02x%02x"), HexString,
            Data & 0xFF, (Data >> 8) & 0xFF, (Data >> 16) & 0xFF, (Data >> 24) & 0xFF);

        Data = ReadDword(i + 12);
        _stprintf(HexString, _T("%s %02x%02x %02x%02x"), HexString,
            Data & 0xFF, (Data >> 8) & 0xFF, (Data >> 16) & 0xFF, (Data >> 24) & 0xFF);

        LOG(0, _T("%03lX: %s"), i, HexString);
    }
}


// \TEMP DEBUG
// Don't know what this is for, came from v4l2 saa7134 code
void CSAA7134Card::StatGPIO()
{
    MaskDataDword(SAA7134_GPIO_GPMODE, 0UL, SAA7134_GPIO_GPMODE_GPRESCN);
    MaskDataDword(SAA7134_GPIO_GPMODE, SAA7134_GPIO_GPMODE_GPRESCN,
        SAA7134_GPIO_GPMODE_GPRESCN);

    DWORD Mode = ReadDword(SAA7134_GPIO_GPMODE) & 0x0EFFFFFF;
    DWORD Status = ReadDword(SAA7134_GPIO_GPSTATUS) & 0x0EFFFFFF;
    LOG(0, _T("debug: gpio: mode=0x%07lx in=0x%07lx out=0x%07lx\n"), Mode,
            (~Mode) & Status, Mode & Status);
}


// \TEMP DEBUG
void CSAA7134Card::CheckRegisters(DWORD* AOdd, DWORD* AEven, DWORD* BOdd, DWORD* BEven)
{
    static WORD OldScalerStatus = 0;
    static DWORD OldIRQStatus = 0;
    static DWORD OldDMAStatus = 0;
    static DWORD OldAOdd = 0;
    static DWORD OldBOdd = 0;
    static DWORD OldAEven = 0;
    static DWORD OldBEven = 0;

    WORD ScalerStatus  = ReadWord(SAA7134_SCALER_STATUS);
    DWORD IRQStatus = ReadDword(SAA7134_IRQ_STATUS);
    DWORD DMAStatus = (ReadDword(SAA7134_DMA_STATUS) >> 20) & 0x1F;

    if (*AOdd != OldAOdd)
    {
        OldAOdd = *AOdd;

        LOG(0, _T("A Odd CHANGED"));
    }

    if (*BOdd != OldBOdd)
    {
        OldBOdd = *BOdd;

        LOG(0, _T("B Odd CHANGED"));
    }

    if (*AEven != OldAEven)
    {
        OldAEven = *AEven;

        LOG(0, _T("A Even CHANGED"));
    }

    if (*BEven != OldBEven)
    {
        OldBEven = *BEven;

        LOG(0, _T("B Even CHANGED"));
    }

    if (DMAStatus != OldDMAStatus)
    {
        OldDMAStatus = DMAStatus;

        LOG(0, _T("DMA Status: %02x"), DMAStatus);
        //LOG(0, _T("DMA Status: %x"), (DMAStatus >> 20) & 0x0F);
    }

    if (IRQStatus != OldIRQStatus)
    {
        OldIRQStatus = IRQStatus;

        LOG(0, _T("IRQ Status: %s,%s,%s,%ld"),
               (IRQStatus & 0x40) ? _T("vbi")  : _T("video"),
               (IRQStatus & 0x20) ? _T("b")    : _T("a"),
               (IRQStatus & 0x10) ? _T("odd")  : _T("even"),
               (IRQStatus & 0x0f));
    }

    if (ScalerStatus != OldScalerStatus)
    {
        OldScalerStatus = ScalerStatus;

        LOG(0, _T("Scaler Status: %s%s%s%s%s%s%s%s%s%s%s%s"),
            (ScalerStatus & SAA7134_SCALER_STATUS_VID_A) ? _T("VID_A ") : _T(""),
            (ScalerStatus & SAA7134_SCALER_STATUS_VBI_A) ? _T("VBI_A ") : _T(""),
            (ScalerStatus & SAA7134_SCALER_STATUS_VID_B) ? _T("VID_B ") : _T(""),
            (ScalerStatus & SAA7134_SCALER_STATUS_VBI_B) ? _T("VBI_B ") : _T(""),
            (ScalerStatus & SAA7134_SCALER_STATUS_TRERR) ? _T("TRERR ") : _T(""),
            (ScalerStatus & SAA7134_SCALER_STATUS_CFERR) ? _T("CFERR ") : _T(""),
            (ScalerStatus & SAA7134_SCALER_STATUS_LDERR) ? _T("LDERR ") : _T(""),
            (ScalerStatus & SAA7134_SCALER_STATUS_WASRST) ? _T("WASRST ") : _T(""),
            (ScalerStatus & SAA7134_SCALER_STATUS_FIDSCI) ? _T("FIDSCI ") : _T(""),
            (ScalerStatus & SAA7134_SCALER_STATUS_FIDSCO) ? _T("FIDSCO ") : _T(""),
            (ScalerStatus & SAA7134_SCALER_STATUS_D6_D5) ? _T("D6_D5 ") : _T(""),
            (ScalerStatus & SAA7134_SCALER_STATUS_TASK) ? _T("TASK ") : _T("")
            );
    }
}


// \TEMP DEBUG
BYTE CSAA7134Card::DirectGetByte(DWORD dwAddress)
{
    return ReadByte(dwAddress);
}


// \TEMP DEBUG
void CSAA7134Card::DirectSetBit(DWORD dwAddress, int nBit, BOOL bSet)
{
    if (bSet)
    {
        OrDataByte(dwAddress, (1<<nBit));
        if (dwAddress >= 0x040 || dwAddress < 0x080)
        {
            OrDataByte(dwAddress + 0x040, (1<<nBit));
        }
    }
    else
    {
        AndDataByte(dwAddress, ~(1<<nBit));
        if (dwAddress >= 0x040 || dwAddress < 0x080)
        {
            AndDataByte(dwAddress + 0x040, ~(1<<nBit));
        }
    }
}



/* might need this for future debugging
    static WORD OldStatus = 0;
    Status = ReadWord(SAA7134_STATUS_VIDEO);

    if (Status != OldStatus)
    {
        OldStatus = Status;
        LOG(0, _T("Video Status: %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s"),
            (Status & SAA7134_STATUS_VIDEO_DCSTD0) ? _T("DCSTD0 ") : _T(""),
            (Status & SAA7134_STATUS_VIDEO_DCSCT1) ? _T("DCSCT1 ") : _T(""),
            (Status & SAA7134_STATUS_VIDEO_WIPA) ? _T("WIPA ") : _T(""),
            (Status & SAA7134_STATUS_VIDEO_GLIMB) ? _T("GLIMB ") : _T(""),
            (Status & SAA7134_STATUS_VIDEO_GLIMT) ? _T("GLIMT ") : _T(""),
            (Status & SAA7134_STATUS_VIDEO_SLTCA) ? _T("SLTCA ") : _T(""),
            (Status & SAA7134_STATUS_VIDEO_HLCK) ? _T("HLCK ") : _T(""),
            (Status & SAA7134_STATUS_VIDEO_RDCAP) ? _T("RDCAP ") : _T(""),
            (Status & SAA7134_STATUS_VIDEO_COPRO) ? _T("COPRO ") : _T(""),
            (Status & SAA7134_STATUS_VIDEO_COLSTR) ? _T("COLSTR ") : _T(""),
            (Status & SAA7134_STATUS_VIDEO_TYPE3) ? _T("TYPE3 ") : _T(""),
            (Status & SAA7134_STATUS_VIDEO_FIDT) ? _T("FIDT ") : _T(""),
            (Status & SAA7134_STATUS_VIDEO_HLVLN) ? _T("HLVLN ") : _T(""),
            (Status & SAA7134_STATUS_VIDEO_INTL) ? _T("INTL ") : _T(""),
            (Status & (1<<7)) ? _T("Unknown1 ") : _T(""),
            (Status & (1<<12)) ? _T("Unknown2") : _T("")
            );
    }
*/
/*
    static WORD OldStatus = 0;
    Status = ReadWord(SAA7134_SCALER_STATUS);

    if (Status != OldStatus)
    {
        OldStatus = Status;
        LOG(0, _T("Scaler Status: %s%s%s%s%s%s%s%s%s%s%s%s"),
            (Status & SAA7134_SCALER_STATUS_VID_A) ? _T("VID_A ") : _T(""),
            (Status & SAA7134_SCALER_STATUS_VBI_A) ? _T("VBI_A ") : _T(""),
            (Status & SAA7134_SCALER_STATUS_VID_B) ? _T("VID_B ") : _T(""),
            (Status & SAA7134_SCALER_STATUS_VBI_B) ? _T("VBI_B ") : _T(""),
            (Status & SAA7134_SCALER_STATUS_TRERR) ? _T("TRERR ") : _T(""),
            (Status & SAA7134_SCALER_STATUS_CFERR) ? _T("CFERR ") : _T(""),
            (Status & SAA7134_SCALER_STATUS_LDERR) ? _T("LDERR ") : _T(""),
            (Status & SAA7134_SCALER_STATUS_WASRST) ? _T("WASRST ") : _T(""),
            (Status & SAA7134_SCALER_STATUS_FIDSCI) ? _T("FIDSCI ") : _T(""),
            (Status & SAA7134_SCALER_STATUS_FIDSCO) ? _T("FIDSCO ") : _T(""),
            (Status & SAA7134_SCALER_STATUS_D6_D5) ? _T("D6_D5 ") : _T(""),
            (Status & SAA7134_SCALER_STATUS_TASK) ? _T("TASK ") : _T("")
            );
    }
*/

#endif//xxx