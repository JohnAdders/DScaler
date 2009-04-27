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
// This software was based on BT848Card_Audio.cpp.  Those portions are
// Copyright (c) 2001 John Adcock.
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file SAA7134Card.cpp CSAA7134Card Implementation (Audio)
 */

#include "stdafx.h"

#ifdef WANT_SAA713X_SUPPORT

#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "SAA7134Card.h"
#include "SAA7134_Defines.h"

#include "DebugLog.h"


void CSAA7134Card::InitAudio()
{
    if (m_DeviceId == 0x7133 || m_DeviceId == 0x7135)
    {
        // SAA7133 and SAA7135 takes a different path.
        InitAudio7133();
        return;
    }

    SetAudioSource(AUDIOINPUTSOURCE_LINE1);

    // Initially mute everything
    SetAudioMute();

    if (m_DeviceId == 0x7130)
    {
        // saa7130 doesn't support most audio stuff
        return;
    }

    // auto gain control enabled
    WriteByte(SAA7134_AGC_GAIN_SELECT,          0x00);

    WriteByte(SAA7134_SIF_SAMPLE_FREQ,          0x40);

    WriteByte(SAA7134_MONITOR_SELECT,           0xA0);
    WriteByte(SAA7134_NICAM_ERROR_LOW,          0x14);
    WriteByte(SAA7134_NICAM_ERROR_HIGH,         0x50);
    WriteByte(SAA7134_NICAM_CONFIG,             0x00);

    WriteByte(SAA7134_DEMODULATOR,              0x00);
    WriteByte(SAA7134_DCXO_IDENT_CTRL,          0x00);
    WriteByte(SAA7134_FM_DEEMPHASIS,            0x22);
    WriteByte(SAA7134_STEREO_DAC_OUTPUT_SELECT, 0x21);

    SetAudioFMDematrix(AUDIOFMDEMATRIX_AUTOSWITCHING);

    WriteByte(SAA7134_DSP_OUTPUT_SELECT,        0x80);

    // input signal is less or equal 2.0Vrms
    OrDataByte(SAA7134_ANALOG_IO_SELECT,        SAA7134_ANALOG_IO_SELECT_VSEL1);
    OrDataByte(SAA7134_ANALOG_IO_SELECT,        SAA7134_ANALOG_IO_SELECT_VSEL2);

    // no audio capture through DMA
    MaskDataDword(SAA7134_NUM_SAMPLES,          0x00, SAA7134_NUM_SAMPLES_MASK);
    WriteByte(SAA7134_AUDIO_FORMAT_CTRL,        0xDD);

    // normal output gain
    MaskDataByte(SAA7134_CHANNEL1_LEVEL,        0x00, 0x1F);
    MaskDataByte(SAA7134_CHANNEL2_LEVEL,        0x00, 0x1F);

    // no I2S output
    WriteByte(SAA7134_I2S_OUTPUT_FORMAT,        0x00);
    WriteByte(SAA7134_I2S_OUTPUT_SELECT,        0x00);
    WriteByte(SAA7134_I2S_OUTPUT_LEVEL,         0x00);
    WriteByte(SAA7134_I2S_AUDIO_OUTPUT,         0x01);

    // Zero is for no plus or minus gain
    SetAudioVolume(0);

    // Setting this FALSE for NICAM doesn't work
    SetAudioLockToVideo(TRUE);
}


void CSAA7134Card::InitAudio7133()
{
    // Out = Line1, In = line2, VSEL = 2.0Vrms for line 1/2.
    // Doing this should effectively mute audio.
    WriteData(SAA7133_ANALOG_IO_SELECT, 0x34);

    // Some of these settings are audio DMA related and are
    // probably not necessary since we don't use DMA.

    // Number of samples = 0x0fff + 1 = 4096. (p125)
    WriteData(SAA7133_NUM_SAMPLES, 0x000fff);
    // Audio-DSP (EPICS) is input source.
    WriteData(SAA7133_AUDIO_SELECT, 0x00);
    // Number for assigned DSP channels 0 to 5.
    WriteData(SAA7133_AUDIO_CHANNEL, 0x543210);
    // 2-channels, 2s comp 32bit, 32bit data for PCI-word. (p126)
    WriteData(SAA7133_AUDIO_FORMAT, 0x23);

    // Auto standard detect allowing B/G/D/K/M,
    // default for bunch of other settings. (p112)
    WriteDSPData7133(SAA7133_A_EASY_PROGRAMMING, 0x00004c);
    // Main matrix = AB[stereo], source = DECODER, same for Dolby (p120)
    WriteDSPData7133(SAA7133_A_DIGITAL_INPUT_XBAR1, 0x0000);
    // Same for AUX1 and AUX2
    WriteDSPData7133(SAA7133_A_DIGITAL_INPUT_XBAR2, 0x0000);
    // DMA1 = MAIN L, DMA2 = MAIN R, silence for others. (p124)
    WriteDSPData7133(SAA7133_A_DIGITAL_OUTPUT_SEL1, 0x00bbbb10);
    // DAC L/R = MAIN L/R, I2S'1 L/R = MAIN L/R, I2S'2 = MAIN L/R
    WriteDSPData7133(SAA7133_A_DIGITAL_OUTPUT_SEL2, 0x00101010);

    // Set all sound feature stuff such as auto volume, incredible
    // stereo, dolby prologic, virtual dolby surround to default.
    WriteDSPData7133(SAA7133_A_SND_FEATURE_CTRL, 0x0000);
    // Default for all dolby prologic options.
    WriteDSPData7133(SAA7133_A_DOLBY_PROLOGIC_CTRL, 0x0000);

    // Main line volume 0 db gain (scale of +24 db to -83 db)
    WriteDSPData7133(SAA7133_A_VOLUME_CTRL1, 0x0000);
    // Aux1/2 line volume 0 db gain (scale of +24 db to -83 db)
    WriteDSPData7133(SAA7133_A_VOLUME_CTRL2, 0x0000);
    // Main line bass and treble 0 db (scale of +15 db to -16 db)
    WriteDSPData7133(SAA7133_A_MAIN_TONE_CTRL, 0x0000);

    // Leave out I2S stuff because I don't know what to do with it.

    // Make all audio video frame locked.
    SetAudioLockToVideo7133(TRUE);
}


BOOL CSAA7134Card::StartDSPAccess7133()
{
    // It shouldn't take more than one sleep for the wait but
    // failure is costly so give extra chances.
    for (int timeout = 2; timeout > 0; timeout--)
    {
        CBitVector ctrl = ReadData(SAA7133_EPICS_ACCESS_STATUS);

        // Reset if the error state is flagged.
        if (ctrl.value(SAA7133_EPICS_ACCESS_STATUS_ERR))
        {
            WriteData(SAA7133_STATUS_RESET, _B(SAA7133_STATUS_RESET_RERR, 1));
        }
        // Reset if the read ready state is flagged.
        if (ctrl.value(SAA7133_EPICS_ACCESS_STATUS_RDR))
        {
            WriteData(SAA7133_STATUS_RESET, _B(SAA7133_STATUS_RESET_RRDR, 1));
        }

        // WRR flagged means there're no current DSP reads or writes.
        if (ctrl.value(SAA7133_EPICS_ACCESS_STATUS_WRR))
        {
            return TRUE;
        }

        // The DSP performs read and write accesses at 32kHz so the
        // period is 31.25 us.  This is the longest time any read or
        // write should take to perform.  However, Windows only provides
        // a 1 ms sleep function so sleep for one millisecond here.
        Sleep(1);
    }

    LOG(0, "SAA7133: Unexpected Error: DSP access WRR flag wait timed out.");
    return FALSE;
}


BOOL CSAA7134Card::WaitDSPAccessState7133(BOOL bRead)
{
    // It shouldn't take more than one sleep for the wait but
    // failure is costly so give extra chances.
    for (int timeout = 2; timeout > 0; timeout--)
    {
        CBitVector ctrl = ReadData(SAA7133_EPICS_ACCESS_STATUS);

        if ((bRead && ctrl.value(SAA7133_EPICS_ACCESS_STATUS_RDR)) ||
            (!bRead && ctrl.value(SAA7133_EPICS_ACCESS_STATUS_WRR)))
        {
            // Read is ready or write is complete.
            return TRUE;
        }

        // The DSP performs read and write accesses at 32kHz so the
        // period is 31.25 us.  This is the longest time any read or
        // write should take to perform.  However, Windows only provides
        // a 1 ms sleep function so sleep for one millisecond here.
        Sleep(1);
    }

    LOG(0, "SAA7133: Unexpected Error: DSP %s flag wait timed out.",
        (bRead ? "read RDR" : "write WRR"));
    return FALSE;
}


BOOL CSAA7134Card::WriteDSPData7133(DWORD registerOffset, DWORD registerMask, CBitVector value)
{
#ifdef _DEBUG
    if (registerOffset < 0x400 || registerOffset >= 0x580)
    {
        LOGD("WriteDSPData7133 used for low latency register %lu!", registerOffset);
        WriteData(registerOffset, registerMask, value);
        return TRUE;
    }
#endif

    DWORD previousValue = 0;

    if (value.mask() != registerMask)
    //if ((value.mask() & registerMask) != 0xffffffff)
    {
        CBitVector b;
        if (!ReadDSPData7133(registerOffset, 0xffffffff, b))
        {
            return FALSE;
        }
        previousValue = b.value();
    }

    // Write DSP data according to the high latency write protocol.
    // All DSP registers are DWORD in size and this function will work
    // exclusively with this size.  It's assumed that registerMask
    // encompasses all operable bits in the registerOffset's DWORD space.
    // As long as there're no registers that do otherwise (and I haven't
    // defined any such in SAA7134_Defines.h, this optimized approach
    // should be fine).  Otherwise, the commented out if expression above
    // will need to be used instead (which will cause much more frequent
    // reads).

    if (!StartDSPAccess7133())
    {
        return FALSE;
    }

    // Write to the DSP.  WriteData() cannot be used to write the data
    // because it can cause a read on the same register.
    WriteDword(registerOffset,
        (previousValue & ~(value.mask() & registerMask)) | (value.value() & registerMask));

    // Wait for the write to complete.
    return WaitDSPAccessState7133(FALSE);
}


BOOL CSAA7134Card::ReadDSPData7133(DWORD registerOffset, DWORD registerMask, CBitVector& value)
{
#ifdef _DEBUG
    if (registerOffset < 0x400 || registerOffset >= 0x500)
    {
        LOGD("ReadDSPData7133 used for low latency register %lu!", registerOffset);
        value = ReadData(registerOffset, registerMask);
        return TRUE;
    }
#endif

    // Read DSP data according to the high latency read protocol.
    if (!StartDSPAccess7133())
    {
        return FALSE;
    }

    // First, read to tell the DSP the address we want.
    ReadDword(registerOffset);

    // Wait for the data to be prepped.
    if (!WaitDSPAccessState7133(TRUE))
    {
        return FALSE;
    }

    // Read the actual data.
    value = CBitVector(registerMask, ReadDword(registerOffset));

    // Check the IDA flag to make sure the read data is valid.
    CBitVector ctrl = ReadData(SAA7133_EPICS_ACCESS_STATUS);
    if (!ctrl.value(SAA7133_EPICS_ACCESS_STATUS_IDA))
    {
        LOG(0, "SAA7133: Unexpected Error: IDA was not set after second DSP read.");
        return FALSE;
    }
    return TRUE;
}


void CSAA7134Card::SetAudioStandard(eAudioStandard AudioStandard)
{
    if (m_DeviceId == 0x7130)
    {
        // saa7130 doesn't support this
        return;
    }
    if (m_DeviceId == 0x7133 || m_DeviceId == 0x7135)
    {
        SetAudioStandard7133(AudioStandard);
        return;
    }

    SetAudioCarrier1Freq(m_AudioStandards[AudioStandard].Carrier1);
    SetAudioCarrier2Freq(m_AudioStandards[AudioStandard].Carrier2);

    SetAudioCarrier1Mode(m_AudioStandards[AudioStandard].Carrier1Mode);
    SetAudioCarrier2Mode(m_AudioStandards[AudioStandard].Carrier2Mode);

    SetCh1FMDeemphasis(m_AudioStandards[AudioStandard].Ch1FMDeemphasis);
    SetCh2FMDeemphasis(m_AudioStandards[AudioStandard].Ch2FMDeemphasis);
}


void CSAA7134Card::SetAudioStandard7133(eAudioStandard audioStandard)
{
    // SAA7133 supports three modes of setting audio standards.
    // 1. Automatic standard detection.
    // 2. Static standard selection.
    // 3. Manual setting of standard related attributes. (expert mode)
    //
    // Expert mode is required to handle the "Custom Standard..."
    // dialog in CSAA7134Source which allows for the setting of user
    // chosen frequency, deemphasis mode, etc.
    //
    // Automatic standard detection mode requires the detection
    // process to be restarted every time the SIF input audio standard
    // changes.  (i.e. Change change, video format change, input change)
    // This is too much effort at this point.

    enum
    {
        SAA7133_DDEP_AUTO,
        SAA7133_DDEP_STATIC,
        SAA7133_DDEP_EXPERT
    };

    // Use static standard select mode for now.  (Maybe change to expert
    // mode in future.)
    BYTE mode = SAA7133_DDEP_STATIC;

    if (mode == SAA7133_DDEP_STATIC)
    {
        BYTE stdsel = 0x00;

        // The ones with no match are probably not even used for me to worry
        // about.  Expert mode could be used but expert mode setting functions
        // are untested.
        switch (audioStandard)
        {
        case AUDIOSTANDARD_BG_DUAL_FM:
            stdsel = 0x04;
            break;
        case AUDIOSTANDARD_DK1_DUAL_FM:
        case AUDIOSTANDARD_DK_FM_MONO: // no match
            stdsel = 0x06;
            break;
        case AUDIOSTANDARD_DK2_DUAL_FM:
            stdsel = 0x07;
            break;
        case AUDIOSTANDARD_DK3_DUAL_FM:
            stdsel = 0x08;
            break;
        case AUDIOSTANDARD_M_DUAL_FM: // no match
            stdsel = 0x0c; // M Korea
            break;
        case AUDIOSTANDARD_BG_NICAM_FM:
            stdsel = 0x05;
        case AUDIOSTANDARD_L_NICAM_AM:
            stdsel = 0x0a;
            break;
        case AUDIOSTANDARD_I_NICAM_FM:
            stdsel = 0x0b;
            break;
        case AUDIOSTANDARD_DK_NICAM_FM:
        case AUDIOSTANDARD_DK_NICAM_FM_HDEV2: // no match
        case AUDIOSTANDARD_DK_NICAM_FM_HDEV3: // no match
            stdsel = 0x09;
            break;
        case AUDIOSTANDARD_M_BTSC:
        case AUDIOSTANDARD_M_BTSC_MONO: // no match
            stdsel = 0x0d;
            break;
        case AUDIOSTANDARD_M_EIAJ:
            stdsel = 0x0e;
            break;
        case AUDIOSTANDARD_FM_RADIO:
            stdsel = 0x0f; // 10.7 MHz, 50 us
            break;
        case AUDIOSTANDARD_SAT: // no match
        case AUDIOSTANDARD_SAT_MONO: // no match
        case AUDIOSTANDARD_SAT_ADR: // no match
            break;
        }
        // Static Standard Select
        WriteDSPData7133(SAA7133_A_EASY_PROGRAMMING,
            _B(SAA7133_A_EASY_PROGRAMMING_EPMODE, 0x1)|
            _B(SAA7133_A_EASY_PROGRAMMING_STDSEL, stdsel)|
            _B(SAA7133_A_EASY_PROGRAMMING_REST, 0)|
            _B(SAA7133_A_EASY_PROGRAMMING_OVMADPT, 0)|
            _B(SAA7133_A_EASY_PROGRAMMING_DDMUTE, 0)|
            _B(SAA7133_A_EASY_PROGRAMMING_FILTBW, 0x0)|
            _B(SAA7133_A_EASY_PROGRAMMING_IDMOD, 0x0)|
            _B(SAA7133_A_EASY_PROGRAMMING_SAPDBX, 0)|
            _B(SAA7133_A_EASY_PROGRAMMING_FHPAL, 0)|
            _B(SAA7133_A_EASY_PROGRAMMING_OVMTHR, 0x0));

        // Restart Automatic Standard Detection.
        WriteDSPData7133(SAA7133_A_EASY_PROGRAMMING,
            _B(SAA7133_A_EASY_PROGRAMMING_REST, 1));
    }
    else if (mode == SAA7133_DDEP_EXPERT)
    {
        // DEMDEC expert mode (fully manual).
        WriteDSPData7133(SAA7133_A_EASY_PROGRAMMING,
            _B(SAA7133_A_EASY_PROGRAMMING_EPMODE, 0x3));

        SetAudioCarrier1Freq7133(m_AudioStandards[audioStandard].Carrier1);
        SetAudioCarrier2Freq7133(m_AudioStandards[audioStandard].Carrier2);

        SetAudioCarrier1Mode7133(m_AudioStandards[audioStandard].Carrier1Mode);
        SetAudioCarrier2Mode7133(m_AudioStandards[audioStandard].Carrier2Mode);

        SetFMDeemphasis7133(m_AudioStandards[audioStandard].Ch1FMDeemphasis);

        // Restart Automatic Standard Detection.
        WriteDSPData7133(SAA7133_A_EASY_PROGRAMMING,
            _B(SAA7133_A_EASY_PROGRAMMING_REST, 1));
    }
    else if (mode == SAA7133_DDEP_AUTO)
    {
        // Write Automatic Standard Detection
        WriteDSPData7133(SAA7133_A_EASY_PROGRAMMING,
            _B(SAA7133_A_EASY_PROGRAMMING_EPMODE, 0x0)|
            _B(SAA7133_A_EASY_PROGRAMMING_STDSEL, 0x00)|
            _B(SAA7133_A_EASY_PROGRAMMING_REST, 0)|
            _B(SAA7133_A_EASY_PROGRAMMING_OVMADPT, 0)|
            _B(SAA7133_A_EASY_PROGRAMMING_DDMUTE, 0)|
            _B(SAA7133_A_EASY_PROGRAMMING_FILTBW, 0x0)|
            _B(SAA7133_A_EASY_PROGRAMMING_IDMOD, 0x0)|
            _B(SAA7133_A_EASY_PROGRAMMING_SAPDBX, 0)|
            _B(SAA7133_A_EASY_PROGRAMMING_FHPAL, 0)|
            _B(SAA7133_A_EASY_PROGRAMMING_OVMTHR, 0x0));

        // Decide main norm for Auto standard detection
        Sleep(500);
        BYTE stdsel = 0x00;
        BYTE dcstd;
        dcstd = ReadByte(SAA7134_STATUS_VIDEO) & SAA7134_STATUS_VIDEO_DCSTD;
        // Read DCSTD Bit's (0:1) : 00 = B/W
        //                          01 = NTSC
        //                          10 = PAL
        //                          11 = SECAM
        if (dcstd == 0x01)
        {
            stdsel = 0x10; // M
            LOG(1, "SAA713x: NTSC signal detected for sound.");
        }
        else if  (dcstd == 0x02)
        {
            stdsel = 0x0b; // B/G + D/K + I
            LOG(1, "SAA713x: PAL signal detected for sound.");
        }
        else if  (dcstd == 0x03)
        {
            stdsel = 0x06; // L + D/K
            LOG(1, "SAA713x: SECAM signal detected for sound.");
        }
        else
        {
            stdsel = 0x1f; // Enable detection of all standards
            LOG(1, "SAA713x: BW signal detected for sound.");
        }

        // Write norm and restart Automatic Standard Detection.
        WriteDSPData7133(SAA7133_A_EASY_PROGRAMMING,
            _B(SAA7133_A_EASY_PROGRAMMING_STDSEL, stdsel));
        WriteDSPData7133(SAA7133_A_EASY_PROGRAMMING,
            _B(SAA7133_A_EASY_PROGRAMMING_REST, 1));
    }
}


void CSAA7134Card::SetAudioCarrier1Freq(DWORD Carrier)
{
    if (m_DeviceId == 0x7130)
    {
        // saa7130 doesn't support this
        return;
    }
    if (m_DeviceId == 0x7133 || m_DeviceId == 0x7135)
    {
        SetAudioCarrier1Freq7133(Carrier);
        return;
    }

    WriteDword(SAA7134_CARRIER1_FREQ, Carrier);
}


void CSAA7134Card::SetAudioCarrier1Freq7133(DWORD Carrier)
{
    // Only in DDEP mode in expert mode
    WriteDSPData7133(SAA7133_A_DEMDEC_CARRIER1, Carrier);
}


void CSAA7134Card::SetAudioCarrier2Freq(DWORD Carrier)
{
    if (m_DeviceId == 0x7130)
    {
        // saa7130 doesn't support this
        return;
    }
    if (m_DeviceId == 0x7133 || m_DeviceId == 0x7135)
    {
        SetAudioCarrier2Freq7133(Carrier);
        return;
    }

    WriteDword(SAA7134_CARRIER2_FREQ, Carrier);
}


void CSAA7134Card::SetAudioCarrier2Freq7133(DWORD Carrier)
{
    // Only in DDEP mode in expert mode
    WriteDSPData7133(SAA7133_A_DEMDEC_CARRIER2, Carrier);
}


void CSAA7134Card::SetAudioCarrier1Mode(eAudioCarrierMode mode)
{
    if (m_DeviceId == 0x7130)
    {
        // saa7130 doesn't support this
        return;
    }
    if (m_DeviceId == 0x7133 || m_DeviceId == 0x7135)
    {
        SetAudioCarrier1Mode7133(mode);
        return;
    }

    if (mode == AUDIOCHANNELMODE_AM)
    {
        OrDataByte(SAA7134_DEMODULATOR, SAA7134_DEMODULATOR_CH1MODE);
    }
    else
    {
        AndDataByte(SAA7134_DEMODULATOR, ~SAA7134_DEMODULATOR_CH1MODE);
    }
}


void CSAA7134Card::SetAudioCarrier1Mode7133(eAudioCarrierMode mode)
{
    // Only in DDEP mode in expert mode
    WriteDSPData7133(SAA7133_A_DEMDEC_CFG,
        _B(SAA7133_A_DEMDEC_CFG_CH1MOD, mode == AUDIOCHANNELMODE_AM ? 1 : 0));
}


void CSAA7134Card::SetAudioCarrier2Mode(eAudioCarrierMode mode)
{
    if (m_DeviceId == 0x7130)
    {
        // saa7130 doesn't support this
        return;
    }
    if (m_DeviceId == 0x7133 || m_DeviceId == 0x7135)
    {
        SetAudioCarrier2Mode7133(mode);
        return;
    }

    BYTE IdentCtrl      = 0x00;
    BYTE Demodulator    = 0x00;
    BYTE AudioPLLCtrl   = 0x00;

    switch (mode)
    {
    case AUDIOCHANNELMODE_FM:
        Demodulator = 0x00;
        break;

    case AUDIOCHANNELMODE_FM_KOREA:
        IdentCtrl |= SAA7134_DCXO_IDENT_CTRL_IDAREA;
        Demodulator = 0x00;
        break;

    case AUDIOCHANNELMODE_AM:
        Demodulator = 0x01;
        break;

    case AUDIOCHANNELMODE_NICAM:
        AudioPLLCtrl |= SAA7134_AUDIO_PLL_CTRL_SWLOOP;
        Demodulator = 0x10;
        break;

    case AUDIOCHANNELMODE_BTSC:
    case AUDIOCHANNELMODE_EIAJ:
        // not supported by saa7134
        break;
    }

    MaskDataByte(SAA7134_DEMODULATOR, Demodulator,
        SAA7134_DEMODULATOR_CH2MOD0 |
        SAA7134_DEMODULATOR_CH2MOD1);

    MaskDataByte(SAA7134_DCXO_IDENT_CTRL, IdentCtrl,
        SAA7134_DCXO_IDENT_CTRL_IDAREA);

    MaskDataByte(SAA7134_AUDIO_PLL_CTRL, AudioPLLCtrl,
        SAA7134_AUDIO_PLL_CTRL_SWLOOP);
}


void CSAA7134Card::SetAudioCarrier2Mode7133(eAudioCarrierMode mode)
{
    CBitVector demdecCfg;

    // Only in DDEP mode in expert mode

    // FM A2, FM-ident region = Europe (there's also a "Japan" in 7133/5)
    demdecCfg = _B(SAA7133_A_DEMDEC_CFG_DECPATH, 0)|_B(SAA7133_A_DEMDEC_CFG_IDAREA, 0);

    switch (mode)
    {
    case AUDIOCHANNELMODE_FM:
        break;

    case AUDIOCHANNELMODE_FM_KOREA:
        // 1 = FM-ident region = Korea
        demdecCfg += _B(SAA7133_A_DEMDEC_CFG_IDAREA, 1);
        break;

    case AUDIOCHANNELMODE_AM:
        // Explanation for this value is omitted in doc but 7130_4 doc
        // says 0 = FM, 1 = AM, 2 = NICAM.
        demdecCfg += _B(SAA7133_A_DEMDEC_CFG_CH2MOD, 1);
        break;

    case AUDIOCHANNELMODE_NICAM:
        // 2 = Unverfied: NICAM
        demdecCfg += _B(SAA7133_A_DEMDEC_CFG_CH2MOD, 2);
        // 3 = NICAM + analog mono (not sure if above is necessary)
        demdecCfg += _B(SAA7133_A_DEMDEC_CFG_DECPATH, 3);
        break;

    case AUDIOCHANNELMODE_BTSC:
        // 4 = BTSC stereo + SAP with 150 ms deemphasis
        demdecCfg += _B(SAA7133_A_DEMDEC_CFG_DECPATH, 4);
        break;
    case AUDIOCHANNELMODE_EIAJ:
        // 6 = EIAJ stereo
        demdecCfg += _B(SAA7133_A_DEMDEC_CFG_DECPATH, 6);
        break;
    }

    WriteDSPData7133(SAA7133_A_DEMDEC_CFG, demdecCfg);
}


void CSAA7134Card::SetAudioLockToVideo(BOOL bLockAudio)
{
    if (m_DeviceId == 0x7130)
    {
        // saa7130 doesn't support this
        return;
    }
    if (m_DeviceId == 0x7133 || m_DeviceId == 0x7135)
    {
        SetAudioLockToVideo7133(bLockAudio);
        return;
    }

    if (bLockAudio)
    {
        AndDataByte(SAA7134_AUDIO_PLL_CTRL, ~SAA7134_AUDIO_PLL_CTRL_APLL);
    }
    else
    {
        OrDataByte(SAA7134_AUDIO_PLL_CTRL, SAA7134_AUDIO_PLL_CTRL_APLL);
    }
}


void CSAA7134Card::SetAudioLockToVideo7133(BOOL bLockAudio)
{
    // This doesn't affect NICAM (for 7133/5 at least) as NICAM
    // is always video frame locked.
    WriteData(SAA7133_PLL_CONTROL, _B(SAA7133_PLL_CONTROL_APLL, bLockAudio ? 0 : 1));
}


void CSAA7134Card::UpdateAudioClocksPerField(eVideoStandard VideoStandard)
{
    if (m_DeviceId == 0x7130)
    {
        // saa7130 doesn't support this
        return;
    }
    if (m_DeviceId == 0x7133 || m_DeviceId == 0x7135)
    {
        UpdateAudioClocksPerField7133(VideoStandard);
        return;
    }

    DWORD   AudioClock = 0x000000;
    DWORD   AudioClocksPerField = 0x00000;
    BYTE    SampleFreq;

    SampleFreq = ReadByte(SAA7134_SIF_SAMPLE_FREQ) & SAA7134_SIF_SAMPLE_FREQ_SFS;

    // If the sample frequency isn't 44.1KHz
    if (SampleFreq != 0x02)
    {
        switch (GetCardSetup()->AudioCrystal)
        {
        case AUDIOCRYSTAL_32110kHz:
            AudioClock = 0x187DE7;
            break;

        case AUDIOCRYSTAL_24576kHz:
            AudioClock = 0x200000;
            break;
        }

        if ((VideoStandard != VIDEOSTANDARD_AUTODETECT &&
            m_VideoStandards[VideoStandard].Is25fps) ||
            Is25fpsSignalDetected())
        {
            AudioClocksPerField = 0x1E000;
        }
        else
        {
            AudioClocksPerField = 0x19066;
        }
    }
    else
    {
        switch (GetCardSetup()->AudioCrystal)
        {
        case AUDIOCRYSTAL_32110kHz:
            AudioClock = 0x1C2097;
            break;

        case AUDIOCRYSTAL_24576kHz:
            AudioClock = 0x24C000;
            break;
        }

        if (m_VideoStandards[VideoStandard].Is25fps)
        {
            AudioClocksPerField = 0x22740;
        }
        else
        {
            AudioClocksPerField = 0x1CBD5;
        }
    }

    if (AudioClock != 0x000000)
    {
        MaskDataDword(SAA7134_AUDIO_CLOCK, AudioClock,
            SAA7134_AUDIO_CLOCK_MASK);
    }

    MaskDataDword(SAA7134_AUDIO_CLOCKS_PER_FIELD, AudioClocksPerField,
        SAA7134_AUDIO_CLOCKS_PER_FIELD_MASK);
}


void CSAA7134Card::UpdateAudioClocksPerField7133(eVideoStandard videoStandard)
{
    BYTE acs = static_cast<BYTE>(ReadData(SAA7133_PLL_CONTROL).value(SAA7133_PLL_CONTROL_ACS));

    // Six master clocks by ACNI 24/32 MHz, ACPF 50/59.94 Hz
    static const DWORD s_acnipf[6][4] =
    {
        // 24 MHz   32 MHz    50 Hz    59.94 Hz
        { 0x000000, 0x000000, 0x16800, 0x12C4D },   // 147.456 MHz
        { 0x200000, 0x187DE7, 0x1E000, 0x19066 },   // 196.608 MHz
        { 0x211333, 0x195088, 0x1F020, 0x19DD9 },   // 203.213 MHz
        { 0x240000, 0x1B8DA4, 0x21C00, 0x1C273 },   // 221.184 MHz
        { 0x2C1995, 0x21C0B2, 0x29580, 0x227CD },   // 270.950 MHz
        { 0x300000, 0x24BCDA, 0x2D000, 0x2589A }    // 294.912 MHz
    };

    int row = -1;

    // 196.608 MHz (master clock)
    if ((acs & 0x9) == 0x0)
    {
        row = 1;
    }
    // 147.456 MHz
    else if ((acs & 0x9) == 0x1)
    {
        row = 0;
    }
    // 270.950 MHz
    else if ((acs & 0xD) == 0x8)
    {
        row = 4;
    }
    // 203.213 MHz
    else if ((acs & 0xD) == 0x9)
    {
        row = 2;
    }
    // 294.912 MHz
    else if ((acs & 0xD) == 0xC)
    {
        row = 5;
    }
    // 221.184 MHz
    else if ((acs & 0xD) == 0xD)
    {
        row = 3;
    }

    DWORD   acni = 0x000000;    // Audio Clock
    DWORD   acpf = 0x00000;     // Audio Clocks Per Field

    switch (GetCardSetup()->AudioCrystal)
    {
    case AUDIOCRYSTAL_24576kHz: acni = s_acnipf[row][0]; break;
    case AUDIOCRYSTAL_32110kHz: acni = s_acnipf[row][1]; break;
    }

    if ((videoStandard == VIDEOSTANDARD_AUTODETECT && Is25fpsSignalDetected()) ||
        m_VideoStandards[videoStandard].Is25fps)
    {
        acpf = s_acnipf[row][2];
    }
    else
    {
        acpf = s_acnipf[row][3];
    }

    // Ensure the correct transfer of the ACNI register value to PLL.
    WriteData(SAA7133_PLL_CONTROL, _B(SAA7133_PLL_CONTROL_SWLOOP, 1));

    if (acni != 0x000000)
    {
        // Write the audio clock.
        WriteData(SAA7133_AUDIO_CLOCK_NOMINAL, acni);
    }

    // Write the audio clocks per field.
    WriteData(SAA7133_AUDIO_CLOCKS_PER_FIELD, acpf);
}


void CSAA7134Card::SetCh1FMDeemphasis(eAudioFMDeemphasis FMDeemphasis)
{
    if (m_DeviceId == 0x7130)
    {
        // saa7130 doesn't support this
        return;
    }
    if (m_DeviceId == 0x7133 || m_DeviceId == 0x7135)
    {
        // SAA7133 doesn't support separate per channel.
        SetFMDeemphasis7133(FMDeemphasis);
        return;
    }

    BYTE Ch1FMDeemphasis = 0x00;

    switch (FMDeemphasis)
    {
    case AUDIOFMDEEMPHASIS_OFF:
        Ch1FMDeemphasis = 0x04;
        break;

    case AUDIOFMDEEMPHASIS_50_MICROS:
        Ch1FMDeemphasis = 0x00;
        break;

    case AUDIOFMDEEMPHASIS_60_MICROS:
        Ch1FMDeemphasis = 0x01;
        break;

    case AUDIOFMDEEMPHASIS_75_MICROS:
        Ch1FMDeemphasis = 0x02;
        break;

    case AUDIOFMDEEMPHASIS_J17:
        Ch1FMDeemphasis = 0x03;
        break;

    case AUDIOFMDEEMPHASIS_ADAPTIVE:
        Ch1FMDeemphasis = 0x0C;
        break;
    }

    MaskDataByte(SAA7134_FM_DEEMPHASIS, Ch1FMDeemphasis, 0x0F);
}


void CSAA7134Card::SetFMDeemphasis7133(eAudioFMDeemphasis fmDeemphasis)
{
    BYTE fmdeem = 0x0;

    // Only in DDEP mode in expert mode

    switch (fmDeemphasis)
    {
    case AUDIOFMDEEMPHASIS_OFF:
        fmdeem = 0x4;
        break;

    case AUDIOFMDEEMPHASIS_50_MICROS:
        fmdeem = 0x0;
        break;

    case AUDIOFMDEEMPHASIS_60_MICROS:
        fmdeem = 0x1;
        break;

    case AUDIOFMDEEMPHASIS_75_MICROS:
        fmdeem = 0x2;
        break;

    case AUDIOFMDEEMPHASIS_J17:
        fmdeem = 0x3;
        break;

    case AUDIOFMDEEMPHASIS_ADAPTIVE:
        // Not supported.  (fall through)
    default:
        return;
    }

    WriteDSPData7133(SAA7133_A_DEMDEC_CFG_FMDEEM, fmdeem);
}


void CSAA7134Card::SetCh2FMDeemphasis(eAudioFMDeemphasis FMDeemphasis)
{
    if (m_DeviceId == 0x7130)
    {
        // saa7130 doesn't support this
        return;
    }
    if (m_DeviceId == 0x7133 || m_DeviceId == 0x7135)
    {
        return;
    }

    BYTE Ch2FMDeemphasis = 0x00;

    switch (FMDeemphasis)
    {
    case AUDIOFMDEEMPHASIS_OFF:
        Ch2FMDeemphasis = 0x40;
        break;

    case AUDIOFMDEEMPHASIS_50_MICROS:
        Ch2FMDeemphasis = 0x00;
        break;

    case AUDIOFMDEEMPHASIS_60_MICROS:
        Ch2FMDeemphasis = 0x10;
        break;

    case AUDIOFMDEEMPHASIS_75_MICROS:
        Ch2FMDeemphasis = 0x20;
        break;

    case AUDIOFMDEEMPHASIS_J17:
        Ch2FMDeemphasis = 0x30;
        break;

    case AUDIOFMDEEMPHASIS_ADAPTIVE:
        Ch2FMDeemphasis = 0xC0;
        break;
    }

    MaskDataByte(SAA7134_FM_DEEMPHASIS, Ch2FMDeemphasis, 0xF0);
}


void CSAA7134Card::SetAudioFMDematrix(eAudioFMDematrix FMDematrix)
{
    BYTE FMDematrixSelect = 0x00;

    if (m_DeviceId == 0x7130)
    {
        // saa7130 doesn't support this
        return;
    }
    if (m_DeviceId == 0x7133 || m_DeviceId == 0x7135)
    {
        SetAudioFMDematrix7133(FMDematrix);
        return;
    }

    switch (FMDematrix)
    {
    case AUDIOFMDEMATRIX_AUTOSWITCHING:
        FMDematrixSelect = 0x80;
        break;

    case AUDIOFMDEMATRIX_MONO1:
        FMDematrixSelect = 0x00;
        break;

    case AUDIOFMDEMATRIX_MONO2:
        FMDematrixSelect = 0x01;
        break;

    case AUDIOFMDEMATRIX_DUAL:
        FMDematrixSelect = 0x02;
        break;

    case AUDIOFMDEMATRIX_DUAL_SWAPPED:
        FMDematrixSelect = 0x03;
        break;

    case AUDIOFMDEMATRIX_STEREO_EUROPE:
        FMDematrixSelect = 0x04;
        break;

    case AUDIOFMDEMATRIX_STEREO_KOREA__6DB:
        FMDematrixSelect = 0x05;
        break;

    case AUDIOFMDEMATRIX_STEREO_KOREA:
        FMDematrixSelect = 0x06;
        break;

    }

    WriteByte(SAA7134_FM_DEMATRIX, FMDematrixSelect);
}


void CSAA7134Card::SetAudioFMDematrix7133(eAudioFMDematrix fmDematrix)
{
    BYTE fmdemat = 0x0;

    // Only in DDEP mode in expert mode

    switch (fmDematrix)
    {
    case AUDIOFMDEMATRIX_AUTOSWITCHING:
        // SAA7133 doesn't seem to have auto switching.
        break;
    case AUDIOFMDEMATRIX_MONO1:
        fmdemat = 0x0;
        break;
    case AUDIOFMDEMATRIX_MONO2:
        fmdemat = 0x1;
        break;
    case AUDIOFMDEMATRIX_DUAL:
        fmdemat = 0x2;
        break;
    case AUDIOFMDEMATRIX_DUAL_SWAPPED:
        fmdemat = 0x3;
        break;
    case AUDIOFMDEMATRIX_STEREO_EUROPE:
        fmdemat = 0x4;
        break;
    case AUDIOFMDEMATRIX_STEREO_KOREA__6DB:
    case AUDIOFMDEMATRIX_STEREO_KOREA:
        fmdemat = 0x05;
        break;
    }

    WriteDSPData7133(SAA7133_A_DEMDEC_OUT_CFG,
        _B(SAA7133_A_DEMDEC_OUT_CFG_FMDEMAT, fmdemat));
}


void CSAA7134Card::SetFilterBandwidth(eAudioFilterBandwidth FilterBandwidth)
{
    // Sound carrier filter bandwidth

    if (m_DeviceId == 0x7130)
    {
        // saa7130 doesn't support this
        return;
    }
    if (m_DeviceId == 0x7133 || m_DeviceId == 0x7135)
    {
        // Only in DDEP mode in expert mode
        // saa7133 supports similar but this function isn't used anyhow.
        return;
    }

    switch (FilterBandwidth)
    {
    case AUDIOFILTERBANDWIDTH_NARROW_NARROW:
        break;

    case AUDIOFILTERBANDWIDTH_XWIDE_NARROW:
        break;

    case AUDIOFILTERBANDWIDTH_MEDIUM_MEDIUM:
        break;

    case AUDIOFILTERBANDWIDTH_WIDE_WIDE:
        break;

    default:
        break;
    }
}


void CSAA7134Card::SetAudioSampleRate(eAudioSampleRate SampleRate)
{
    if (m_DeviceId == 0x7130)
    {
        // saa7130 doesn't support this
        return;
    }
    if (m_DeviceId == 0x7133 || m_DeviceId == 0x7135)
    {
        SetAudioSampleRate7133(SampleRate);
        return;
    }

    BYTE SampleFrequency = 0;

    if (m_AudioInputSource == AUDIOINPUTSOURCE_DAC)
    {
        // SampleRate is always AUDIOSAMPLERATE_32000Hz;
        SampleFrequency = 0x40;
    }
    else
    {
        switch (SampleRate)
        {
        case AUDIOSAMPLERATE_32000Hz: SampleFrequency = 0x01; break;
        case AUDIOSAMPLERATE_44100Hz: SampleFrequency = 0x02; break;
        case AUDIOSAMPLERATE_48000Hz: SampleFrequency = 0x03; break;
        }
    }

    MaskDataByte(SAA7134_SIF_SAMPLE_FREQ, SampleFrequency,
        SAA7134_SIF_SAMPLE_FREQ_SFS);

    UpdateAudioClocksPerField(m_VideoStandard);
}


void CSAA7134Card::SetAudioSampleRate7133(eAudioSampleRate sampleRate)
{
    BYTE acs = 0;

    if (m_AudioInputSource == AUDIOINPUTSOURCE_DAC)
    {
        // 0000: 32 kHz w/ DEMDEC, 196.608 MHz, /3 = DSP
        acs = 0x0;
    }
    else
    {
        switch (sampleRate)
        {
            // 0100: 32 kHz baseband, 196.608 MHz, /3 = DSP
        case AUDIOSAMPLERATE_32000Hz: acs = 0x4; break;
            // 1000: 44.1 kHz baseband, 270.950 MHz, /3 = DSP
        case AUDIOSAMPLERATE_44100Hz: acs = 0x8; break;
            // 1100: 48 kHz baseband, 294.912 MHz, /3 = DSP
        case AUDIOSAMPLERATE_48000Hz: acs = 0xC; break;
        }
    }

    WriteData(SAA7133_PLL_CONTROL, _B(SAA7133_PLL_CONTROL_ACS, acs));

    // Update the audio clock and audio clocks per field.
    UpdateAudioClocksPerField7133(m_VideoStandard);
}


void CSAA7134Card::SetAudioSource(eAudioInputSource InputSource)
{
    if (m_DeviceId != 0x7133 && m_DeviceId != 0x7135)
    {
        if (InputSource == AUDIOINPUTSOURCE_DAC_LINE1)
        {
            InputSource = AUDIOINPUTSOURCE_LINE1;
        }
        else if (InputSource == AUDIOINPUTSOURCE_DAC_LINE2)
        {
            InputSource = AUDIOINPUTSOURCE_LINE2;
        }
    }

    m_AudioInputSource = InputSource;

    if (m_bAudioLineReservedForMute)
    {
        // This means that the card's line selection is tied up to a specific
        // line in order perform mute.  (SAA7130 for instance, cannot mute
        // any other way.)  The line recorded above is set at unmute.
        return;
    }

    if (m_DeviceId == 0x7133 || m_DeviceId == 0x7135)
    {
        SetAudioSource7133(InputSource);
        return;
    }

    _SetIOSelectOCS(InputSource, m_bStereoExternalLines);

    if (InputSource == AUDIOINPUTSOURCE_DAC)
    {
        SetAudioSampleRate(AUDIOSAMPLERATE_32000Hz);
    }
    else if (InputSource == AUDIOINPUTSOURCE_LINE1 || InputSource == AUDIOINPUTSOURCE_DAC_LINE1)
    {
        AndDataByte(SAA7134_ANALOG_IO_SELECT, ~SAA7134_ANALOG_IO_SELECT_ICS);
    }
    else if (InputSource == AUDIOINPUTSOURCE_LINE2 || InputSource == AUDIOINPUTSOURCE_DAC_LINE2)
    {
        OrDataByte(SAA7134_ANALOG_IO_SELECT, SAA7134_ANALOG_IO_SELECT_ICS);
    }
}


void CSAA7134Card::SetAudioSource7133(eAudioInputSource inputSource)
{
    _SetIOSelectOCS7133(inputSource, m_bStereoExternalLines);

    if (inputSource == AUDIOINPUTSOURCE_DAC)
    {
        SetAudioSampleRate7133(AUDIOSAMPLERATE_32000Hz);
    }
    else if (inputSource == AUDIOINPUTSOURCE_LINE1 || inputSource == AUDIOINPUTSOURCE_DAC_LINE1)
    {
        // Input crossbar select line 1.  Sample rate and audio clock adjustments
        // for the external two lines should be made by CSAA7134Source calling
        // SetAudioSampleRate() straight after.  CSAA7134Card doesn't record the
        // sample rate state to be able to make the adjustment itself.
        WriteData(SAA7133_ANALOG_IO_SELECT, _B(SAA7133_ANALOG_IO_SELECT_ICS, 0));
    }
    else if (inputSource == AUDIOINPUTSOURCE_LINE2 || inputSource == AUDIOINPUTSOURCE_DAC_LINE2)
    {
        // Input crossbar select line 2.
        WriteData(SAA7133_ANALOG_IO_SELECT, _B(SAA7133_ANALOG_IO_SELECT_ICS, 1));
    }
}


void CSAA7134Card::_SetIOSelectOCS(eAudioInputSource InputSource, BOOL bStereoExternal)
{
    if (m_DeviceId == 0x7133 || m_DeviceId == 0x7135)
    {
        _SetIOSelectOCS7133(InputSource, bStereoExternal);
        return;
    }

    BYTE LineSelect;

    switch (InputSource)
    {
    case AUDIOINPUTSOURCE_DAC: LineSelect = 0x02; break;
    case AUDIOINPUTSOURCE_DAC_LINE1:
    case AUDIOINPUTSOURCE_LINE1: LineSelect = bStereoExternal ? 0x00 : 0x03; break;
    case AUDIOINPUTSOURCE_DAC_LINE2:
    case AUDIOINPUTSOURCE_LINE2: LineSelect = bStereoExternal ? 0x01 : 0x05; break;
    }

    MaskDataByte(SAA7134_ANALOG_IO_SELECT, LineSelect,
        SAA7134_ANALOG_IO_SELECT_OCS);
}


void CSAA7134Card::_SetIOSelectOCS7133(eAudioInputSource inputSource, BOOL bStereoExternal)
{
    // In SAA7133, 0111 is "no output enabled".
    BYTE ocs = 0x7;

    switch (inputSource)
    {
        // DAC L/DAC R
    case AUDIOINPUTSOURCE_DAC: ocs = 0x2;
        // DACL = MAIN left, DACR = MAIN right.
        WriteDSPData7133(SAA7133_A_DIGITAL_OUTPUT_SEL2,
            _B(SAA7133_A_DIGITAL_OUTPUT_SEL2_ASDACL, 0)|
            _B(SAA7133_A_DIGITAL_OUTPUT_SEL2_ASDACR, 1));
        break;
        // EXTIL1/EXTIR1 or EXTIL1/EXTIL1
    case AUDIOINPUTSOURCE_LINE1: ocs = (bStereoExternal ? 0x0 : 0x3); break;
        // EXTIL2/EXTIR2 or EXTIL2/EXTIL2
    case AUDIOINPUTSOURCE_LINE2: ocs = (bStereoExternal ? 0x1 : 0x5); break;
        // Via DAC auxiliary lines.
    case AUDIOINPUTSOURCE_DAC_LINE1:
        // DACL = AUX1 left, DACR = AUX1 right.
        WriteDSPData7133(SAA7133_A_DIGITAL_OUTPUT_SEL2,
            _B(SAA7133_A_DIGITAL_OUTPUT_SEL2_ASDACL, 2)|
            _B(SAA7133_A_DIGITAL_OUTPUT_SEL2_ASDACR, 3));
        break;
    case AUDIOINPUTSOURCE_DAC_LINE2:
        // DACL = AUX2 left, DACR = AUX2 right.
        WriteDSPData7133(SAA7133_A_DIGITAL_OUTPUT_SEL2,
            _B(SAA7133_A_DIGITAL_OUTPUT_SEL2_ASDACL, 4)|
            _B(SAA7133_A_DIGITAL_OUTPUT_SEL2_ASDACR, 5));
        break;
    }

    WriteData(SAA7133_ANALOG_IO_SELECT, _B(SAA7133_ANALOG_IO_SELECT_OCS, ocs));
}


BOOL CSAA7134Card::IsAudioChannelDetected(eAudioChannel AudioChannel)
{
    if (m_AudioInputSource == AUDIOINPUTSOURCE_LINE1 ||
        m_AudioInputSource == AUDIOINPUTSOURCE_LINE2 ||
        m_AudioInputSource == AUDIOINPUTSOURCE_DAC_LINE1 ||
        m_AudioInputSource == AUDIOINPUTSOURCE_DAC_LINE2)
    {
        // external lines are only stereo
        if (AudioChannel == AUDIOCHANNEL_MONO ||
            AudioChannel == AUDIOCHANNEL_STEREO)
        {
            return TRUE;
        }
        return FALSE;
    }

    if (m_DeviceId == 0x7133 || m_DeviceId == 0x7135)
    {
        return _IsAudioChannelDetected7133(AudioChannel);
    }

    BYTE Status;

    // Read carrier 2 mode to see if it's NICAM
    if (ReadByte(SAA7134_DEMODULATOR) & SAA7134_DEMODULATOR_CH2MOD1)
    {
        Status = (ReadByte(SAA7134_NICAM_STATUS) & SAA7134_NICAM_STATUS_SIN);

        switch (AudioChannel)
        {
        case AUDIOCHANNEL_MONO:
            return Status == 0x00 || Status == 0x02;
        case AUDIOCHANNEL_STEREO:
            return Status == 0x02;
        case AUDIOCHANNEL_LANGUAGE1:
        case AUDIOCHANNEL_LANGUAGE2:
            return Status == 0x01;
        }
    }
    else
    {
        Status = (ReadByte(SAA7134_IDENT_SIF) & SAA7134_IDENT_SIF_IDP) >> 5;

        switch (AudioChannel)
        {
        case AUDIOCHANNEL_MONO:
            return (Status & (1 << 1)) == 0;
        case AUDIOCHANNEL_STEREO:
            return (Status & (1 << 2)) != 0;
        case AUDIOCHANNEL_LANGUAGE1:
        case AUDIOCHANNEL_LANGUAGE2:
            return (Status & (1 << 1)) != 0;
        }
    }

    // NEVER_GET_HERE;
    return FALSE;
}


BOOL CSAA7134Card::_IsAudioChannelDetected7133(eAudioChannel audioChannel)
{
    CBitVector b;
    ReadDSPData7133(SAA7133_A_MAIN_STATUS, b);

    // Checking general stereo (GST) and general dual (GDU) is unverified.
    if (b.value(SAA7133_A_MAIN_STATUS_GST))
    {
        return audioChannel == AUDIOCHANNEL_STEREO;
    }
    if (b.value(SAA7133_A_MAIN_STATUS_GDU))
    {
        return audioChannel == AUDIOCHANNEL_LANGUAGE1 ||
            audioChannel == AUDIOCHANNEL_LANGUAGE2;
    }
    return audioChannel == AUDIOCHANNEL_MONO;
}


void CSAA7134Card::SetAudioChannel(eAudioChannel AudioChannel)
{
    if (m_AudioInputSource == AUDIOINPUTSOURCE_LINE1 ||
        m_AudioInputSource == AUDIOINPUTSOURCE_LINE2 ||
        m_AudioInputSource == AUDIOINPUTSOURCE_DAC_LINE1 ||
        m_AudioInputSource == AUDIOINPUTSOURCE_DAC_LINE2)
    {
        m_bStereoExternalLines = (AudioChannel == AUDIOCHANNEL_STEREO);

        if (m_bAudioLineReservedForMute != FALSE)
        {
            return;
        }

        _SetIOSelectOCS(m_AudioInputSource, m_bStereoExternalLines);
        return;
    }

    if (m_DeviceId == 0x7133 || m_DeviceId == 0x7135)
    {
        _SetAudioChannel7133(AudioChannel);
        return;
    }

    if (AudioChannel == AUDIOCHANNEL_MONO)
    {
        // Select Left/Left
        MaskDataByte(SAA7134_DSP_OUTPUT_SELECT, 0x10,
            SAA7134_DSP_OUTPUT_SELECT_CSM);

        // Disable automatic stereo adjustment
        AndDataByte(SAA7134_DSP_OUTPUT_SELECT, ~SAA7134_DSP_OUTPUT_SELECT_AASDMA);

        // Selecting 00 will force FM/AM.  This means selecting mono
        // when on NICAM will revert to ch1 FM
        MaskDataByte(SAA7134_STEREO_DAC_OUTPUT_SELECT, 0x00,
            SAA7134_STEREO_DAC_OUTPUT_SELECT_SDOS);
    }
    else
    {
        // When AASDMA is on, SDOS 01 will select NICAM over
        // FM only if it is available
        MaskDataByte(SAA7134_STEREO_DAC_OUTPUT_SELECT, 0x01,
            SAA7134_STEREO_DAC_OUTPUT_SELECT_SDOS);

        // Enable automatic stereo adjustment and the card
        // will sort itself out
        OrDataByte(SAA7134_DSP_OUTPUT_SELECT, SAA7134_DSP_OUTPUT_SELECT_AASDMA);

        if (AudioChannel == AUDIOCHANNEL_LANGUAGE1)
        {
            MaskDataByte(SAA7134_DSP_OUTPUT_SELECT, 0x10,
                SAA7134_DSP_OUTPUT_SELECT_CSM);
        }
        else if (AudioChannel == AUDIOCHANNEL_LANGUAGE2)
        {
            MaskDataByte(SAA7134_DSP_OUTPUT_SELECT, 0x20,
                SAA7134_DSP_OUTPUT_SELECT_CSM);
        }
    }
}


void CSAA7134Card::_SetAudioChannel7133(eAudioChannel audioChannel)
{
    BYTE maindm = 0;

    // Only the DAC line should see this path.  SetAudioChannel()
    // exits early for external lines.

    // Unlike what was done for SAA7134, audio channels here are set
    // up at the digital input crossbar.  "Mono" in this case is
    // (Left + Right)/2 rather than the Left/Left that was done for
    // SAA7134.  SAA7133 also offers channel selections for aux1,
    // aux2 (external lines) and dolby pro but they're ignored.
    // There doesn't seem to be automatic stereo detect in SAA7133
    // so that's one less thing this function needs to worry about.
    switch (audioChannel)
    {
    case AUDIOCHANNEL_MONO:
        maindm = 1;
        break;
    case AUDIOCHANNEL_STEREO:
        maindm = 0;
        break;
    case AUDIOCHANNEL_LANGUAGE1:
        maindm = 2;
        break;
    case AUDIOCHANNEL_LANGUAGE2:
        maindm = 3;
        break;
    }

    WriteDSPData7133(SAA7133_A_DIGITAL_INPUT_XBAR1,
        _B(SAA7133_A_DIGITAL_INPUT_XBAR1_MAINDM, maindm));
}


CSAA7134Card::eAudioChannel CSAA7134Card::GetAudioChannel()
{
    if (m_DeviceId == 0x7133 || m_DeviceId == 0x7135)
    {
        return GetAudioChannel7133();
    }

    if (m_AudioInputSource == AUDIOINPUTSOURCE_LINE1 ||
        m_AudioInputSource == AUDIOINPUTSOURCE_LINE2)
    {
        if (m_bAudioLineReservedForMute)
        {
            if (m_bStereoExternalLines)
            {
                // Although the card is configured to accept
                // stereo, the actual format is dependant
                // on the external source.  For this reason, we
                // return ``AUDIOCHANNEL_EXTERNAL'', to mean
                // "depends on the external source".

                return AUDIOCHANNEL_EXTERNAL;
            }

            return AUDIOCHANNEL_MONO;
        }

        BYTE AudioLine;

        AudioLine = ReadByte(SAA7134_ANALOG_IO_SELECT) &
            SAA7134_ANALOG_IO_SELECT_OCS;

        switch (AudioLine)
        {
        case 0x00:
        case 0x01:
            return AUDIOCHANNEL_EXTERNAL;

        case 0x03:
        case 0x05:
            return AUDIOCHANNEL_MONO;

        default:
            // NEVER_GET_HERE;
            break;
        }
        return AUDIOCHANNEL_MONO;
    }

    // If automatic select is enabled, things work a bit different
    if (ReadByte(SAA7134_DSP_OUTPUT_SELECT) & SAA7134_DSP_OUTPUT_SELECT_AASDMA)
    {
        if (IsAudioChannelDetected(AUDIOCHANNEL_STEREO))
        {
            return AUDIOCHANNEL_STEREO;
        }
        else if (IsAudioChannelDetected(AUDIOCHANNEL_LANGUAGE2))
        {
            if (ReadByte(SAA7134_DSP_OUTPUT_SELECT) & 0x20)
            {
                return AUDIOCHANNEL_LANGUAGE2;
            }
            return AUDIOCHANNEL_LANGUAGE1;
        }
        return AUDIOCHANNEL_MONO;
    }

    switch (ReadByte(SAA7134_DSP_OUTPUT_SELECT) &
        SAA7134_DSP_OUTPUT_SELECT_CSM)
    {
    case 0x00:  // L / R
    case 0x30:  // R / L
    case 0x40:  // (L+R)/2 / (L+R)/2
        if (IsAudioChannelDetected(AUDIOCHANNEL_STEREO))
        {
            return AUDIOCHANNEL_STEREO;
        }
        break;

    case 0x10:
        if (IsAudioChannelDetected(AUDIOCHANNEL_LANGUAGE1))
        {
            return AUDIOCHANNEL_LANGUAGE1;
        }
        break;

    case 0x20:
        if (IsAudioChannelDetected(AUDIOCHANNEL_LANGUAGE2))
        {
            return AUDIOCHANNEL_LANGUAGE2;
        }
        break;

    default:
        // NEVER_GET_HERE;
        break;
    }
    return AUDIOCHANNEL_MONO;
}


CSAA7134Card::eAudioChannel CSAA7134Card::GetAudioChannel7133()
{
    if (m_AudioInputSource == AUDIOINPUTSOURCE_LINE1 ||
        m_AudioInputSource == AUDIOINPUTSOURCE_LINE2 ||
        m_AudioInputSource == AUDIOINPUTSOURCE_DAC_LINE1 ||
        m_AudioInputSource == AUDIOINPUTSOURCE_DAC_LINE2)
    {
        if (m_bAudioLineReservedForMute)
        {
            if (m_bStereoExternalLines)
            {
                return AUDIOCHANNEL_EXTERNAL;
            }
            return AUDIOCHANNEL_MONO;
        }

        //todo fix: This won't work for via DAC lines.
        CBitVector b = ReadData(SAA7133_ANALOG_IO_SELECT);
        switch (b.value(SAA7133_ANALOG_IO_SELECT_OCS))
        {
        case 0x00:
        case 0x01:
            return AUDIOCHANNEL_EXTERNAL;
        case 0x03:
        case 0x05:
            return AUDIOCHANNEL_MONO;
        default:
            // NEVER_GET_HERE;
            break;
        }
        return AUDIOCHANNEL_MONO;
    }

    // Auto stereo isn't used for SAA7133 so that's one less thing this
    // needs to worry about.

    CBitVector b;

    ReadDSPData7133(SAA7133_A_DIGITAL_INPUT_XBAR1, b);
    switch (b.value(SAA7133_A_DIGITAL_INPUT_XBAR1_MAINDM))
    {
    case 0: // LR
    case 4: // RL
        if (IsAudioChannelDetected(AUDIOCHANNEL_STEREO))
        {
            return AUDIOCHANNEL_STEREO;
        }
        break;
    case 2: // LL (automatrix off)
    case 6: // language A (automatrix on)
        if (IsAudioChannelDetected(AUDIOCHANNEL_LANGUAGE1))
        {
            return AUDIOCHANNEL_LANGUAGE1;
        }
        break;
    case 3: // RR (automatrix off)
    case 7: // language B (automatrix on)
        if (IsAudioChannelDetected(AUDIOCHANNEL_LANGUAGE2))
        {
            return AUDIOCHANNEL_LANGUAGE2;
        }
        break;
    case 1: // (L + R)/2
        break;
    }
    return AUDIOCHANNEL_MONO;
}


void CSAA7134Card::GetAudioDecoderStatus(char* pBuffer, WORD nBufferSize)
{
    *pBuffer = '\0';

    if (m_DeviceId == 0x7130)
    {
        // saa7130 doesn't support this
        return;
    }

    DWORD avStatus = ReadDword(SAA7134_AV_STATUS);
    char buffer[256] = "";

    if (avStatus & SAA7134_AV_STATUS_PILOT)
    {
        strcat(buffer, "FM ");
    }
    if (avStatus & SAA7134_AV_STATUS_VDSP)
    {
        strcat(buffer, "NICAM ");
    }
    if (avStatus & SAA7134_AV_STATUS_DUAL)
    {
        strcat(buffer, "FM_DUAL ");
    }
    if (avStatus & SAA7134_AV_STATUS_STEREO)
    {
        strcat(buffer, "FM_STEREO ");
    }
    if (avStatus & SAA7134_AV_STATUS_DMB)
    {
        strcat(buffer, "NICAM_DUAL ");
    }
    if (avStatus & SAA7134_AV_STATUS_SMB)
    {
        strcat(buffer, "NICAM_STEREO ");
    }

    // SAA7133 has extra status.
    if (m_DeviceId == 0x7133 || m_DeviceId == 0x7135)
    {
        CBitVector b;

        ReadDSPData7133(SAA7133_A_MAIN_STATUS, b);
        if (b.value(SAA7133_A_MAIN_STATUS_SAPDET))
        {
            strcat(pBuffer, "SAP ");
        }
        if (b.value(SAA7133_A_MAIN_STATUS_BPILOT))
        {
            strcat(pBuffer, "BTSC/FM ");
        }
        if (b.value(SAA7133_A_MAIN_STATUS_APILOT))
        {
            strcat(pBuffer, "A2/EIAJ ");
        }
        if (b.value(SAA7133_A_MAIN_STATUS_AST))
        {
            strcat(pBuffer, "A2/EIAJ_STEREO ");
        }
        if (b.value(SAA7133_A_MAIN_STATUS_ADU))
        {
            strcat(pBuffer, "A2/EIAJ_DUAL ");
        }
        if (b.value(SAA7133_A_MAIN_STATUS_GDU))
        {
            strcat(pBuffer, "DUAL ");
        }
        if (b.value(SAA7133_A_MAIN_STATUS_GST))
        {
            strcat(pBuffer, "STEREO ");
        }
    }

    if (nBufferSize < sizeof(buffer))
    {
        buffer[nBufferSize-1] = '\0';
    }

    strcpy(pBuffer, buffer);
}


void CSAA7134Card::SetAutomaticVolume(eAutomaticVolume AVL)
{
    if (m_DeviceId == 0x7130)
    {
        // saa7130 doesn't support this
        return;
    }
    if (m_DeviceId == 0x7133 || m_DeviceId == 0x7135)
    {
        SetAutomaticVolume7133(AVL);
        return;
    }

    BYTE DACOutputSelect = 0x00;

    switch (AVL)
    {
    case AUTOMATICVOLUME_SHORTDECAY:
        DACOutputSelect = 0x10;
        break;

    case AUTOMATICVOLUME_MEDIUMDECAY:
        DACOutputSelect = 0x20;
        break;

    case AUTOMATICVOLUME_LONGDECAY:
        DACOutputSelect = 0x30;
        break;

    default:
        break;
    }

    MaskDataByte(SAA7134_STEREO_DAC_OUTPUT_SELECT, DACOutputSelect,
        SAA7134_STEREO_DAC_OUTPUT_SELECT_AVL);
}


void CSAA7134Card::SetAutomaticVolume7133(eAutomaticVolume avl)
{
    BYTE avlmod;

    // SAA7133 offers "very short decay", "very long decay" and other
    // sound processing features but the existing SAA7134 framework
    // only expects these.

    switch (avl)
    {
    case AUTOMATICVOLUME_SHORTDECAY:
        avlmod = 2;
        break;
    case AUTOMATICVOLUME_MEDIUMDECAY:
        avlmod = 3;
        break;
    case AUTOMATICVOLUME_LONGDECAY:
        avlmod = 4;
        break;
    case AUTOMATICVOLUME_OFF:
    default:
        avlmod = 0;
        break;
    }

    WriteData(SAA7133_A_SND_FEATURE_CTRL, _B(SAA7133_A_SND_FEATURE_CTRL_AVLMOD, avlmod));
}


void CSAA7134Card::SetAudioLine1Voltage(eAudioLineVoltage lineVoltage)
{
    if (m_DeviceId == 0x7130 || m_DeviceId == 0x7134)
    {
        switch (lineVoltage)
        {
        case AUDIOLINEVOLTAGE_1VRMS:
            AndDataByte(SAA7134_ANALOG_IO_SELECT, ~SAA7134_ANALOG_IO_SELECT_VSEL1);
            break;

        case AUDIOLINEVOLTAGE_2VRMS:
            OrDataByte(SAA7134_ANALOG_IO_SELECT, SAA7134_ANALOG_IO_SELECT_VSEL1);
            break;
        }
    }
    else if (m_DeviceId == 0x7133 || m_DeviceId == 0x7135)
    {
        WriteData(SAA7133_ANALOG_IO_SELECT, _B(SAA7133_ANALOG_IO_SELECT_VSEL1,
            (lineVoltage == AUDIOLINEVOLTAGE_1VRMS) ? 0 : 1));
    }
}


void CSAA7134Card::SetAudioLine2Voltage(eAudioLineVoltage lineVoltage)
{
    if (m_DeviceId == 0x7130 || m_DeviceId == 0x7134)
    {
        switch (lineVoltage)
        {
        case AUDIOLINEVOLTAGE_1VRMS:
            AndDataByte(SAA7134_ANALOG_IO_SELECT, ~SAA7134_ANALOG_IO_SELECT_VSEL2);
            break;

        case AUDIOLINEVOLTAGE_2VRMS:
            OrDataByte(SAA7134_ANALOG_IO_SELECT, SAA7134_ANALOG_IO_SELECT_VSEL2);
            break;
        }
    }
    else if (m_DeviceId == 0x7133 || m_DeviceId == 0x7135)
    {
        WriteData(SAA7133_ANALOG_IO_SELECT, _B(SAA7133_ANALOG_IO_SELECT_VSEL2,
            (lineVoltage == AUDIOLINEVOLTAGE_1VRMS) ? 0 : 1));
    }
}


void CSAA7134Card::SetAudioMute()
{
    if (m_DeviceId == 0x7130)
    {
        // Mute by setting a different input and output crossbar.
        // Input crossbar line 2.
        OrDataByte(SAA7134_ANALOG_IO_SELECT, SAA7134_ANALOG_IO_SELECT_ICS);
        // Output crossbar line 1.
        _SetIOSelectOCS(AUDIOINPUTSOURCE_LINE1, m_bStereoExternalLines);

        // Set this so we know the audio line is reserved
        m_bAudioLineReservedForMute = TRUE;
    }
    else if (m_DeviceId == 0x7134)
    {
        WriteByte(SAA7134_AUDIO_MUTE_CTRL, 0xFF);
    }
    else if (m_DeviceId == 0x7133 || m_DeviceId == 0x7135)
    {
        // Select "no output enabled" for the output crossbar select.  There're
        // other ways to mute the SAA7133 but this seems to be the most straight
        // forward.  (Other ways offer selective muting of different lines.)
        WriteData(SAA7133_ANALOG_IO_SELECT, _B(SAA7133_ANALOG_IO_SELECT_OCS, 0x7));

        // Set this so we know the audio line is reserved
        m_bAudioLineReservedForMute = TRUE;
    }
}


void CSAA7134Card::SetAudioUnMute()
{
    if (m_DeviceId == 0x7130)
    {
        m_bAudioLineReservedForMute = FALSE;
        SetAudioSource(m_AudioInputSource);
    }
    else if (m_DeviceId == 0x7134)
    {
        MaskDataByte(SAA7134_AUDIO_MUTE_CTRL, 0x00, SAA7134_AUDIO_MUTE_CTRL_MUTSOUT);
    }
    else if (m_DeviceId == 0x7133 || m_DeviceId == 0x7135)
    {
        // Restore the selected audio line to ummute.
        m_bAudioLineReservedForMute = FALSE;
        SetAudioSource(m_AudioInputSource);
        // Unmute soft-mute control so it doesn't interfere.
        WriteDSPData7133(SAA7133_A_SOFTMUTE_CTRL, 0x00);
    }
}


// Unused
void CSAA7134Card::SetAudioVolume(BYTE nGain)
{
    if (m_DeviceId == 0x7130)
    {
        // saa7130 doesn't support this
        return;
    }
    if (m_DeviceId == 0x7133 || m_DeviceId == 0x7135)
    {
        // SAA7133 has similar but this function isn't used anyhow.
        return;
    }

    // nGain = -15..0..15, 0 = normal

    // Dual FM Level adjust
    WriteByte(SAA7134_CHANNEL1_LEVEL, nGain & 0x1F);
    WriteByte(SAA7134_CHANNEL2_LEVEL, nGain & 0x1F);
    // NICAM Level adjust
    WriteByte(SAA7134_NICAM_LEVEL_ADJUST, nGain & 0x1F);
}

void CSAA7134Card::SetAudioBalance(WORD nBalance)
{
    // Unsupported
}

void CSAA7134Card::SetAudioBass(WORD nBass)
{
    // Unsupported
}

void CSAA7134Card::SetAudioTreble(WORD nTreble)
{
    // Unsupported
}

int CSAA7134Card::GetInputAudioLine(int nInput)
{
    if (nInput < m_SAA713xCards[m_CardType].NumInputs && nInput >= 0)
    {
        return m_SAA713xCards[m_CardType].Inputs[nInput].AudioLineSelect;
    }
    return 0;
}

LPCSTR CSAA7134Card::GetAudioStandardName(eAudioStandard audioStandard)
{
    return m_AudioStandards[audioStandard].Name;
}

#endif//xxx