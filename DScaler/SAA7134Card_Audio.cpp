/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Card_Audio.cpp,v 1.2 2002-09-09 14:20:33 atnak Exp $
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
// This software was based on BT848Card_Audio.cpp.  Those portions are
// Copyright (c) 2001 John Adcock.
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 09 Sep 2002   Atsushi Nakagawa      Initial Release
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
//
//////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "SAA7134Card.h"
#include "SAA7134_Defines.h"

#include "DebugLog.h"

CSAA7134Card::TAudioStandardDefinition CSAA7134Card::m_AudioStandards[] =
{
    { 
        "[PAL] B/G-Dual FM-Stereo",
        AUDIO_CARRIER_5_5,  
        AUDIO_CARRIER_5_7421875,
        MONO_FM,
        STEREO_FM,
        FIR_BG_DK_DUAL_FM,
    },
    {
        "[PAL] D/K1-Dual FM-Stereo",
        AUDIO_CARRIER_6_5,  
        AUDIO_CARRIER_6_2578125,
        MONO_FM,
        STEREO_FM,
        FIR_BG_DK_DUAL_FM,
    },
    { 
        "[PAL] D/K2-Dual FM-Stereo",
        AUDIO_CARRIER_6_5,  
        AUDIO_CARRIER_6_7421875,  
        MONO_FM,
        STEREO_FM,
        FIR_BG_DK_DUAL_FM,
    },
    { 
        "[PAL]D/K3-Dual FM-Stereo",
        AUDIO_CARRIER_6_5,  
        AUDIO_CARRIER_5_7421875,  
        MONO_FM,
        STEREO_FM,
        FIR_BG_DK_DUAL_FM
    },
    { 
        "[PAL] B/G-NICAM-FM",
        AUDIO_CARRIER_5_5,  
        AUDIO_CARRIER_5_85,       
        MONO_FM,
        STEREO_NICAM,
        FIR_BG_DK_NICAM,
    },
    { 
        "[PAL] I-NICAM-FM",
        AUDIO_CARRIER_6_0,  
        AUDIO_CARRIER_6_552,      
        MONO_FM,
        STEREO_NICAM,
        FIR_I_NICAM,
    },
    { 
        "[PAL] D/K-NICAM-FM",
        AUDIO_CARRIER_6_5,  
        AUDIO_CARRIER_5_85,       
        MONO_FM,
        STEREO_NICAM,
        FIR_BG_DK_NICAM,
    },
    { 
        "[SECAM] L-NICAM-AM",
        AUDIO_CARRIER_6_5,  
        AUDIO_CARRIER_5_85,       
        MONO_AM,
        STEREO_NICAM,
        FIR_L_NICAM,
    },
    {
        "[NTSC] ??? M (Mono)",
        AUDIO_CARRIER_4_5,  
        AUDIO_CARRIER_4_5,       
        MONO_FM,
        STEREO_NONE,
        FIR_M_N_DUAL_FM,
    },
    {
        "[NTSC] A2 FM-Stereo",
        AUDIO_CARRIER_4_5,  
        AUDIO_CARRIER_4_724212,       
        MONO_FM,
        STEREO_FM,
        FIR_M_N_DUAL_FM,
    }
    // A lot of standards snipped
};


void CSAA7134Card::InitAudio()
{
    DWORD Clock = GetCardSetup()->AudioClock;

    WriteByte(SAA7134_AUDIO_CLOCK_0, Clock & 0xff);
    WriteByte(SAA7134_AUDIO_CLOCK_1, (Clock >> 8) & 0xff);
    WriteByte(SAA7134_AUDIO_CLOCK_2, (Clock >> 16) & 0xff);
    WriteByte(SAA7134_AUDIO_PLL_CTRL, 0x01);

    WriteByte(SAA7134_NICAM_ERROR_LOW, 0x00);
    WriteByte(SAA7134_NICAM_ERROR_HIGH, 0x00);
    WriteByte(SAA7134_MONITOR_SELECT, 0x00);
    WriteByte(SAA7134_FM_DEMATRIX, 0x80);

    // mute
    WriteByte(SAA7134_AUDIO_MUTE_CTRL, 0xFF);
    // Line 1 select
    MaskDataByte(SAA7134_ANALOG_IO_SELECT, 0x00, 0x07);


    // Line volume reduction
    MaskDataByte(SAA7134_ANALOG_IO_SELECT, 0x10, 0x10); // Line 1
    MaskDataByte(SAA7134_ANALOG_IO_SELECT, 0x20, 0x20); // Line 2

    MaskDataByte(SAA7134_AUDIO_FORMAT_CTRL, 0xC0, 0xC0);
    MaskDataByte(SAA7134_SIF_SAMPLE_FREQ, 0x00, 0x03);


/*
    // Line 1 = bit off, Line 2 = bit on
    MaskDataByte(SAA7134_ANALOG_IO_SELECT, 0x00, 0x08);
    MaskDataByte(SAA7134_AUDIO_FORMAT_CTRL, 0x80, 0xC0);

    // 32000hz = 0x01, 48000hz = 0x03
    MaskDataByte(SAA7134_SIF_SAMPLE_FREQ, 0x03, 0x03);
    */
}


// FIX: this doesn't work too well.. need to figure out proper
// registers.
// TODO2: check&reconsider user interface for this
void CSAA7134Card::SetAudioStandard(eAudioStandard audioStandard)
{

    switch (m_AudioStandards[audioStandard].FIRType)
    {
    case FIR_BG_DK_DUAL_FM:
    case FIR_M_N_DUAL_FM:
        WriteByte(SAA7134_MONITOR_SELECT,           0x00);
        WriteByte(SAA7134_FM_DEMATRIX,              0x80);
        WriteByte(SAA7134_AUDIO_CLOCKS_PER_FIELD0,  0x00);
        WriteByte(SAA7134_AUDIO_CLOCKS_PER_FIELD1,  0xe0);
        WriteByte(SAA7134_AUDIO_CLOCKS_PER_FIELD2,  0x01);
        WriteByte(SAA7134_AUDIO_PLL_CTRL,           0x00);
        break;
    case FIR_I_NICAM:
    case FIR_BG_DK_NICAM:
    case FIR_L_NICAM:
        WriteByte(SAA7134_MONITOR_SELECT,           0xa0);
        WriteByte(SAA7134_FM_DEMATRIX,              0x00);
        WriteByte(SAA7134_AUDIO_CLOCKS_PER_FIELD0,  0x00);
        WriteByte(SAA7134_AUDIO_CLOCKS_PER_FIELD1,  0x00);
        WriteByte(SAA7134_AUDIO_CLOCKS_PER_FIELD2,  0x00);
        WriteByte(SAA7134_AUDIO_PLL_CTRL,           0x01);
        break;
    }

    WriteDword(SAA7134_CARRIER1_FREQ, m_AudioStandards[audioStandard].MajorCarrier);
    WriteDword(SAA7134_CARRIER2_FREQ, m_AudioStandards[audioStandard].MinorCarrier);
/*
    WriteByte(SAA7134_DEMODULATOR,              0x00);
    WriteByte(SAA7134_DCXO_IDENT_CTRL,          0x00);
    WriteByte(SAA7134_FM_DEEMPHASIS,            0x00);
    WriteByte(SAA7134_STEREO_DAC_OUTPUT_SELECT, 0x80);
*/
    switch (m_AudioStandards[audioStandard].FIRType) {
    case FIR_BG_DK_DUAL_FM:
        WriteByte(SAA7134_DEMODULATOR,              0x00);
        WriteByte(SAA7134_DCXO_IDENT_CTRL,          0x00);
        WriteByte(SAA7134_FM_DEEMPHASIS,            0x22);
        WriteByte(SAA7134_STEREO_DAC_OUTPUT_SELECT, 0xa0);
        break;
    case FIR_M_N_DUAL_FM:
        WriteByte(SAA7134_DEMODULATOR,              0x00);
        WriteByte(SAA7134_DCXO_IDENT_CTRL,          0x01);
        WriteByte(SAA7134_FM_DEEMPHASIS,            0x22);
        WriteByte(SAA7134_STEREO_DAC_OUTPUT_SELECT, 0xa0);
        break;
    case FIR_I_NICAM:
    case FIR_BG_DK_NICAM:
        WriteByte(SAA7134_DEMODULATOR,              0x10);
        WriteByte(SAA7134_DCXO_IDENT_CTRL,          0x00);
        WriteByte(SAA7134_FM_DEEMPHASIS,            0x44);
        WriteByte(SAA7134_STEREO_DAC_OUTPUT_SELECT, 0xa1);
        break;
    case FIR_L_NICAM:
        WriteByte(SAA7134_DEMODULATOR,              0x12);
        WriteByte(SAA7134_DCXO_IDENT_CTRL,          0x00);
        WriteByte(SAA7134_FM_DEEMPHASIS,            0x44);
        WriteByte(SAA7134_STEREO_DAC_OUTPUT_SELECT, 0xa1);
        break;
    case FIR_SAT_DUAL_FM:
        break;
    }
    CheckStereo();
}

// DEBUG: debugging purposes
//   Code can be used for graying out channel menu
void CSAA7134Card::CheckStereo()
{
    BYTE Status;

    Status = (ReadByte(SAA7134_IDENT_SIF) & 0xE0) >> 5;
    LOG(0, "Dual-FM Stereo: %s%s%s",
        Status & (1 << 0) ? "MONO ": "",
        Status & (1 << 1) ? "LANG2 ": "",
        Status & (1 << 2) ? "STEREO ": ""
        );

    Status = ReadByte(SAA7134_NICAM_STATUS);
    LOG(0, "NICAM Stereo: %s%s%s",
        Status & (1 << 3) ? "MONO ": "",
        Status & (1 << 0) ? "LANG2 ": "",
        Status & (1 << 1) ? "STEREO ": "",
        Status & (1 << 2) ? "BIT2 ": ""
        );
}

void CSAA7134Card::SetAudioMute()
{
    // if SAA7134
    WriteByte(SAA7134_AUDIO_MUTE_CTRL, 0xff);

    // if SAA7130 select MUTE line
}

void CSAA7134Card::SetAudioUnMute(long nVolume, eAudioInputLine Input)
{
    // if SAA7134
    WriteByte(SAA7134_AUDIO_MUTE_CTRL, 0xbb);

    // SetAudioSource(Input);
}

// Unlatched. has some problems with distorted sound
void CSAA7134Card::SetAudioVolume(BYTE nGain)
{
    // nGain = -15..0..15, 0 = normal

    WriteByte(SAA7134_CHANNEL1_LEVEL, nGain & 0x1F);
    WriteByte(SAA7134_CHANNEL2_LEVEL, nGain & 0x1F);
    WriteByte(SAA7134_NICAM_LEVEL_ADJUST, nGain & 0x1F);
}

void CSAA7134Card::SetAudioBalance(WORD nBalance)
{
    // TODO: Need to implement
}

void CSAA7134Card::SetAudioBass(WORD nBass)
{
    // TODO: Need to implement
    // SAA7134 doesn't have Bass but SAA7133 & SAA7135 does
}

void CSAA7134Card::SetAudioTreble(WORD nTreble)
{
    // TODO: Need to implement
    // SAA7134 doesn't have Treble but SAA7133 & SAA7135 does
}

// TODO: check this out
void CSAA7134Card::SetAudioChannel(eSoundChannel soundChannel)
{
    if(m_TVCards[m_CardType].pSoundChannelFunction != NULL)
    {
        // call correct function
        // this funny syntax is the only one that works
        // if you want help understanding what is going on
        // I suggest you read http://www.newty.de/
        (*this.*m_TVCards[m_CardType].pSoundChannelFunction)(soundChannel);
    }
}

void CSAA7134Card::SetAudioStandard(eVideoFormat videoFormat)
{
    // TODO: Need to implement - check usability
}

// TODO: make like the comments
// Do not call this function before changing the video source, it is checking to see if a video
// signal is present. Audio is muted if no video signal is detected. 
// This might not be the best place to do this check.
void CSAA7134Card::SetAudioSource(eAudioInputLine nLine)
{
    BYTE MuxSelect;

    switch (nLine)
    {
    case AUDIOINPUTLINE_TUNER: MuxSelect = 0x02; break;
    case AUDIOINPUTLINE_LINE1: MuxSelect = 0x00; break;
    case AUDIOINPUTLINE_LINE2: MuxSelect = 0x01; break;
    }

    MaskDataByte(SAA7134_ANALOG_IO_SELECT, MuxSelect, 0x07);

    // TODO: some cards need GPIO for audio selecting
    /*
    BYTE GpioMask = 0x0;  // FlyVideo 3000 
    BYTE InputGpio = 0x0; // FlyVideo 3000

    if (!GpioMask)
        return;

    MaskDataDword(SAA7134_GPIO_GPMODE0,   GpioMask, GpioMask);
    MaskDataDword(SAA7134_GPIO_GPSTATUS0, InputGpio, GpioMask);
    StatGPIO();
    */
}


int CSAA7134Card::GetInputAudioLine(int nInput)
{
    if(nInput < m_TVCards[m_CardType].NumInputs && nInput >= 0)
    {
        return m_TVCards[m_CardType].Inputs[nInput].AudioLineSelect;
    }
    return 0;
}

LPCSTR CSAA7134Card::GetAudioStandardName(eAudioStandard audioStandard)
{
    return m_AudioStandards[audioStandard].Name;
}