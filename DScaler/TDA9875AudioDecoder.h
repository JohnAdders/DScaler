/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 John Adcock.  All rights reserved.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file TDA9875AudioDecoder.h CTDA9875AudioDecoder Header
 */

#if !defined(__TDA9875AUDIODECODER_H__)
#define __TDA9875AUDIODECODER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TDA9875.h"
#include "AudioDecoder.h"
#include "TDA9875AudioControls.h"

class CTDA9875AudioDecoder :
    public CTDA9875,
        public CAudioDecoder
{
private:

    int m_CarrierDetect_Phase;
    CTDA9875AudioControls* m_pAudioControls;
    BOOL m_ForceAMSound;
    BOOL m_IsInitialized;
    BOOL m_bUseInputPin1;
    int m_AutoDetecting;
    HANDLE m_TDA9875Thread;
    BOOL m_bStopThread;
    CRITICAL_SECTION TDA9875CriticalSection;
    long m_DetectCounter;
    long m_DetectInterval10ms;
    BOOL m_ThreadWait;
    BOOL m_DetectSupportedSoundChannels;
    eSupportedSoundChannels m_SupportedSoundChannels;
    eSoundChannel m_TargetSoundChannel;


    enum eDe_emphasis
    {
        TDA9875_DE_EMPHASIS_50    = 0,
        TDA9875_DE_EMPHASIS_60    = 17,
        TDA9875_DE_EMPHASIS_75    = 34,
        TDA9875_DE_EMPHASIS_J17    = 51,
        TDA9875_DE_EMPHASIS_OFF = 68,
        TDA9875_DE_EMPHASIS_ADAPTIVE = 170
    };

#define CARRIER_HZ(freq) (int)(((float)(freq))*1000000)

    enum eCarrier
    {
        TDA9875_NOCARRIER        = 0,
        TDA9875_CARRIER_4_5        = CARRIER_HZ(4.5),
        TDA9875_CARRIER_4_724    = CARRIER_HZ(4.724),
        TDA9875_CARRIER_5_5        = CARRIER_HZ(5.5),
        TDA9875_CARRIER_5_742    = CARRIER_HZ(5.742),
        TDA9875_CARRIER_5_85    = CARRIER_HZ(5.85),
        TDA9875_CARRIER_6_0        = CARRIER_HZ(6.0),
        TDA9875_CARRIER_6_26    = CARRIER_HZ(6.26),
        TDA9875_CARRIER_6_5        = CARRIER_HZ(6.5),
        TDA9875_CARRIER_6_552    = CARRIER_HZ(6.552),
        TDA9875_CARRIER_6_742    = CARRIER_HZ(6.742),
        TDA9875_CARRIER_7_02    = CARRIER_HZ(7.02),
        TDA9875_CARRIER_7_20    = CARRIER_HZ(7.20),
        TDA9875_CARRIER_7_38    = CARRIER_HZ(7.38),
        TDA9875_CARRIER_7_56    = CARRIER_HZ(7.56),
        TDA9875_CARRIER_7_74    = CARRIER_HZ(7.74),
        TDA9875_CARRIER_7_92    = CARRIER_HZ(7.92),
        TDA9875_CARRIER_8_10    = CARRIER_HZ(8.10),
        TDA9875_CARRIER_8_28    = CARRIER_HZ(8.28)
    };
#undef TDA9875_CARRIER

    enum eStandard
    {
        TDA9875_STANDARD_NONE = 0x0000,
        TDA9875_STANDARD_AUTO = 0x0001,
        //ANALOG
        TDA9875_STANDARD_M_ANALOG_FM_NONE = 0x0010,
        TDA9875_STANDARD_M_ANALOG_FM_FM = 0x0011,
        TDA9875_STANDARD_BG_ANALOG_FM_FM = 0x0012,
        TDA9875_STANDARD_I_ANALOG_FM_NONE = 0x0013,
        TDA9875_STANDARD_DK1_ANALOG_FM_FM = 0x0014,
        TDA9875_STANDARD_DK2_ANALOG_FM_FM = 0x0015,
        //NICAM
        TDA9875_STANDARD_BG_DIGITAL_FM_NICAM = 0x0030,
        TDA9875_STANDARD_I_DIGITAL_FM_NICAM = 0x0031,
        TDA9875_STANDARD_DK_DIGITAL_FM_NICAM = 0x0032,
        TDA9875_STANDARD_L_DIGITAL_AM_NICAM = 0x0033,
        //SATELLITE
        TDA9875_STANDARD_SAT_FM_MONO = 0x0050,
        TDA9875_STANDARD_SAT1_FM_FM = 0x0051,
        TDA9875_STANDARD_SAT2_FM_FM = 0x0052,
        TDA9875_STANDARD_SAT3_FM_FM = 0x0053,
        TDA9875_STANDARD_SAT4_FM_FM = 0x0054,
        //AUTODETECT
        TDA9875_STANDARD_AUTODETECTION_IN_PROGRESS  = 0x07ff
    };

    enum eMonoType
    {
        MONO_FM,
        MONO_AM,
        MONO_FM_SAT
    };

    enum eStereoType
    {
        STEREO_NONE,
        STEREO_FM,
        STEREO_NICAM,
        STEREO_AM,
    };

    typedef struct
    {
        char*           Name;
        eStandard       Standard;
        eCarrier        MajorCarrier;
        eCarrier        MinorCarrier;
        eMonoType       MonoType;
        eStereoType     StereoType;
        eDe_emphasis    De_emphasis;
    } TStandardDefinition;

    typedef struct
    {
      eCarrier Major;
      eCarrier Minor[10];
    } TCarrierDetect;

    static TStandardDefinition m_TDA9875Standards[];
    static TCarrierDetect CarrierDetectTable[];

    void StartThread();
    void StopThread();

    CTDA9875AudioDecoder::eStandard DetectStandardTDA9875();
    void SetAudioStandardCarriersTDA9875(long MajorCarrier, long MinorCarrier);


public:

    CTDA9875AudioDecoder(CTDA9875AudioControls* pAudioControls);
    virtual ~CTDA9875AudioDecoder();

    int DetectThread();

    // Sound Channels
    virtual void SetSoundChannel(eSoundChannel soundChannel);
    virtual eSoundChannel GetSoundChannel();
    virtual eSoundChannel IsAudioChannelDetected(eSoundChannel desiredAudioChannel);

    // Inputs
    virtual void SetAudioInput(eAudioInput audioInput);
    virtual eAudioInput GetAudioInput();
    virtual const char* GetAudioInputName(eAudioInput audioInput);

    BOOL GetUseInputPin1();
    void SetUseInputPin1(BOOL AValue);


    // Standard
    virtual void SetAudioStandard(long Standard, eVideoFormat videoformat);
    virtual long GetAudioStandardCurrent();
    virtual const char* GetAudioStandardName(long Standard);
    virtual int GetNumAudioStandards();
    virtual long GetAudioStandard(int nIndex);
    virtual long GetAudioStandardMajorCarrier(long Standard);
    virtual long GetAudioStandardMinorCarrier(long Standard);
    virtual void SetAudioStandardCarriers(long MajorCarrier, long MinorCarrier);
    virtual long GetAudioStandardFromVideoFormat(eVideoFormat videoFormat);
    // Interval in milliseconds. <=0 for abort.
    // SupportedSoundChannels == 1: detect after standard
    // SupportedSoundChannels == 2: detect now, no standard detect
    virtual void DetectAudioStandard(long Interval, int SupportedSoundChannels, eSoundChannel TargetChannel);

    virtual eAudioDecoderType GetAudioDecoderType();

    // \todo: eliminate this
    virtual int SetAudioDecoderValue(int What, long Val);
    virtual long GetAudioDecoderValue(int What);

};

#endif // !defined(__TDA9875AUDIODECODER_H__)