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
//  Copyright (C) 2004 Mika Lane.  All rights reserved.
//  Copyright (C) 2000-2005 Quenotte  All rights reserved.
//  Copyright (C) 2000 - 2002 by Eduardo José Tagle.
/////////////////////////////////////////////////////////////////////////////

#if !defined(__TDA9874AUDIODECODER_H__)
#define __TDA9874AUDIODECODER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AudioDecoder.h"
#include "I2CDevice.h"
#include "TDA9874.h"
#include "TVFormats.h"


class CTDA9874AudioDecoder : public CTDA9874, public CAudioDecoder
{
public:
    CTDA9874AudioDecoder();
    virtual ~CTDA9874AudioDecoder();

    bool Initialize();

    // Sound Channels
    void SetSoundChannel(eSoundChannel SoundChannel);
    eSoundChannel IsAudioChannelDetected(eSoundChannel DesiredAudioChannel);

    // Inputs
    void SetAudioInput(eAudioInput AudioInput);
	
    // Standard
    void SetAudioStandard(long Standard, eVideoFormat VideoFormat);
    const char* GetAudioStandardName(long Standard);
    int GetNumAudioStandards();
    long GetAudioStandard(int nIndex);
    long GetAudioStandardFromVideoFormat(eVideoFormat VideoFormat);
    void DetectAudioStandard(long Interval, int SupportedSoundChannels, eSoundChannel TargetChannel);
    long GetAudioStandardMajorCarrier(long Standard);
	long GetAudioStandardMinorCarrier(long Standard);	
	int DetectThread();

	

private:	
	enum eStandard
    {
		TDA9874_STANDARD_NONE = 0x0000,
        TDA9874_STANDARD_AUTO = 0x0001,

		//ANALOG
        TDA9874_STANDARD_M_ANALOG_FM_NONE = 0x0010,
        TDA9874_STANDARD_M_ANALOG_FM_FM = 0x0011,
        TDA9874_STANDARD_BG_ANALOG_FM_FM = 0x0012,
        TDA9874_STANDARD_I_ANALOG_FM_NONE = 0x0013,
        TDA9874_STANDARD_DK1_ANALOG_FM_FM = 0x0014,
        TDA9874_STANDARD_DK2_ANALOG_FM_FM = 0x0015,
		TDA9874_STANDARD_DK3_ANALOG_FM_FM = 0x0016,

		//NICAM
        TDA9874_STANDARD_BG_DIGITAL_FM_NICAM = 0x0030,
        TDA9874_STANDARD_I_DIGITAL_FM_NICAM = 0x0031,
        TDA9874_STANDARD_DK_DIGITAL_FM_NICAM = 0x0032,
        TDA9874_STANDARD_L_DIGITAL_AM_NICAM = 0x0033,
		
		//AUTODETECT
        TDA9874_STANDARD_AUTODETECTION_IN_PROGRESS  = 0x07ff
	};

	typedef struct
	{
		char* Name;		// Videonorms name
		DWORD Standard;	// Videonorms that use this
		DWORD c1,c2;	// Carrier 1 and Carrier 2 frequencies
		BYTE  DCR;		// Demodulator control register
		BYTE  FMER;		// FM deemphasis	(only for FM, NICAM ignores it)
		BYTE  FMMR;		// FM dematrix		(only for FM, NICAM ignores it). User can override it
		BOOL  Nicam;	// Is it is NICAM mode
		BOOL  AGCOff;	// If AGC must be turned off
	} ANInfo;

	eSoundChannel m_desiredam;
	int m_SIF;
    eAudioDecoderType GetAudioDecoderType();

	static ANInfo m_TDA9874Standards[];	
      
    HANDLE m_TDA9874Thread;
    volatile bool m_StopThread;
    volatile bool m_ThreadWait;
    volatile bool m_DetectSupportedSoundChannels;
    volatile eSoundChannel m_TargetSoundChannel;
    volatile long m_DetectInterval10ms;
    volatile int m_AutoDetecting;
    volatile eSupportedSoundChannels m_SupportedSoundChannels;    	
    

    void SetChipStandard(bool FastCheck);
    void SetChipMode(eSoundChannel channel);

    int Standard2Index(long Standard);

    void StartThread();
    void StopThread();
};

#endif // !defined(__TDA9874AUDIODECODER_H__)
