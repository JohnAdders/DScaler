//
// $Id$
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2004 Sven Grothklags.  All rights reserved.
//
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
//
/////////////////////////////////////////////////////////////////////////////
//
// This software was based on TDA9875AudioDecoder.h.  Those portions are
// Copyright (c) 2004 Robert Milharcic.
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file TDA9873AudioDecoder.h: interface for the CTDA9873AudioDecoder class.
 */

#if !defined(__TDA9873AUDIODECODER_H__)
#define __TDA9873AUDIODECODER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AudioDecoder.h"
#include "I2CDevice.h"

class CTDA9873AudioDecoder : public CAudioDecoder, public CI2CDevice
{
public:
    CTDA9873AudioDecoder();
    virtual ~CTDA9873AudioDecoder();

    BOOL Initialize();

    // Sound Channels
    void SetSoundChannel(eSoundChannel SoundChannel);
    eSoundChannel IsAudioChannelDetected(eSoundChannel DesiredAudioChannel);

    // Inputs
    void SetAudioInput(eAudioInput AudioInput);

    // Standard
    void SetAudioStandard(long Standard, eVideoFormat VideoFormat);
    const TCHAR* GetAudioStandardName(long Standard);
    int GetNumAudioStandards();
    long GetAudioStandard(int nIndex);
    long GetAudioStandardFromVideoFormat(eVideoFormat VideoFormat);
    void DetectAudioStandard(long Interval, int SupportedSoundChannels, eSoundChannel TargetChannel);
    int DetectThread();

    eAudioDecoderType GetAudioDecoderType();

private:
    enum eReg
    {
        TDA9873_REG_SWITCH = 0,
        TDA9873_REG_STANDARD,
        TDA9873_REG_PORT,
        TDA9873_REG_LAST_ONE,
        TDA9873_REG_DETECT = 0xfe,
    };

    enum
    {
        TDA9873IN_MONO = 0x00,
        TDA9873IN_STEREO = 0x02,
        TDA9873IN_LANG12 = 0x04,
        TDA9873IN_CHANNELMASK = 0x06,
        TDA9873IN_MAGIC = 0x80,
        TDA9873IN_MAGICMASK = 0xe0,
    };

    enum
    {
        TDA9873DE_MAGIC = 0x80,
    };

    enum
    {
        TDA9873SW_MONO = 0x04,
        TDA9873SW_STEREO = 0x10,
        TDA9873SW_LANG1 = 0x04,
        TDA9873SW_LANG2 = 0x08,
        TDA9873SW_CHANNELMASK = 0x1c,
        TDA9873SW_INTERNAL = 0x00,
        TDA9873SW_EXTERNALSTEREO = 0x02,
        TDA9873SW_EXTERNALMONO = 0x03,
        TDA9873SW_MUTE = 0x40,
        TDA9873SW_INPUTMASK = 0x43,
    };

    enum eStandard
    {
        TDA9873ST_BG = 0x00,
        TDA9873ST_M = 0x10,
        TDA9873ST_DK1 = 0x20,
        TDA9873ST_DK2 = 0x30,
        TDA9873ST_DK3 = 0x40,
        TDA9873ST_I = 0x50,
        TDA9873ST_STANDARDMASK = 0x70,
    };

    typedef struct
    {
        TCHAR*           Name;
        eStandard       Standard;
    } TStandardDefinition;

    static TStandardDefinition m_TDA9873Standards[];

    BYTE m_ShadowReg[TDA9873_REG_LAST_ONE];

    HANDLE m_TDA9873Thread;
    volatile BOOL m_StopThread;
    volatile BOOL m_ThreadWait;
    volatile BOOL m_DetectSupportedSoundChannels;
    volatile eSoundChannel m_TargetSoundChannel;
    volatile long m_DetectInterval10ms;
    volatile int m_AutoDetecting;
    volatile eSupportedSoundChannels m_SupportedSoundChannels;

    BYTE ReadStatus();
    BOOL Write(eReg SubAddress, BYTE Data, BYTE Mask = 0xff);

    BOOL Detect();

    int Standard2Index(long Standard);

    void StartThread();
    void StopThread();

    virtual BYTE GetDefaultAddress() const;
};

#endif // !defined(__TDA9873AUDIODECODER_H__)
