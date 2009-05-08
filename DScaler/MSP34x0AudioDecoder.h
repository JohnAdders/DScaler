//
// $Id$
//
/////////////////////////////////////////////////////////////////////////////
//
// copyleft 2002 itt@myself.com
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
 * @file msp34x0audiodecoder.h msp34x0audiodecoder Header
 */

#ifdef WANT_BT8X8_SUPPORT

#if !defined(__MSP34X0AUDIODECODER_H__)
#define __MSP34X0AUDIODECODER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MSP34x0.h"
#include "AudioDecoder.h"

class CMSP34x0AudioDecoder :
    public CMSP34x0,
    public CAudioDecoder
{
public:
    CMSP34x0AudioDecoder();
    virtual ~CMSP34x0AudioDecoder();

    CAudioDecoder::eAudioDecoderType GetAudioDecoderType();

    BOOL GetUseInputPin1();
    void SetUseInputPin1(BOOL AValue);

    // from CAudioDecoder the default Getters are used
    void SetSoundChannel(eSoundChannel soundChannel);
    void SetAudioInput(eAudioInput audioInput);
    eSoundChannel IsAudioChannelDetected(eSoundChannel desiredAudioChannel);

    // Standard
    void SetAudioStandard(long Standard, eVideoFormat videoformat);
    long GetAudioStandardCurrent();
    const TCHAR* GetAudioStandardName(long Standard);
    int GetNumAudioStandards();
    long GetAudioStandard(int nIndex);
    long GetAudioStandardMajorCarrier(long Standard);
    long GetAudioStandardMinorCarrier(long Standard);
    void SetAudioStandardCarriers(long MajorCarrier, long MinorCarrier);
    long GetAudioStandardFromVideoFormat(eVideoFormat videoFormat);
    void DetectAudioStandard(long Interval, int SupportedSoundChannels, eSoundChannel TargetChannel);

    int SetAudioDecoderValue(int What, long Val);
    long GetAudioDecoderValue(int What);

    BOOL HasEqualizer() { return m_bHasEqualizer; }
    BOOL HasDolby() { return m_bHasDolby; }
private:

    enum eStandard
    {
        MSP34x0_STANDARD_NONE = 0x0000,
        MSP34x0_STANDARD_AUTO = 0x0001,
        MSP34x0_STANDARD_M_DUAL_FM = 0x0002,
        MSP34x0_STANDARD_BG_DUAL_FM =  0x0003,
        MSP34x0_STANDARD_DK1_DUAL_FM = 0x0004,
        MSP34x0_STANDARD_DK2_DUAL_FM = 0x0005,
        MSP34x0_STANDARD_DK_FM_MONO = 0x0006,
        MSP34x0_STANDARD_DK3_DUAL_FM = 0x0007,
        MSP34x0_STANDARD_BG_NICAM_FM = 0x0008,
        MSP34x0_STANDARD_L_NICAM_AM = 0x0009,
        MSP34x0_STANDARD_I_NICAM_FM = 0x000a,
        MSP34x0_STANDARD_DK_NICAM_FM = 0x000b,
        MSP34x0_STANDARD_DK_NICAM_FM_HDEV2 = 0x000c,
        MSP34x0_STANDARD_DK_NICAM_FM_HDEV3 = 0x000d,
        MSP34x0_STANDARD_M_BTSC = 0x0020,
        MSP34x0_STANDARD_M_BTSC_MONO = 0x0021,
        MSP34x0_STANDARD_M_EIA_J = 0x0030,
        MSP34x0_STANDARD_FM_RADIO = 0x0040,
        MSP34x0_STANDARD_SAT_MONO = 0x0050,
        MSP34x0_STANDARD_SAT = 0x0051,
        MSP34x0_STANDARD_SAT_ADR = 0x0060,
        MSP34x0_STANDARD_AUTODETECTION_IN_PROGRESS = 0x07FF,
    };

    enum eMonoType
    {
        MONO_FM,
        MONO_AM,
    };

    enum eStereoType
    {
        STEREO_NONE,
        STEREO_FM,
        STEREO_NICAM,
        STEREO_BTSC,
        STEREO_MONO_SAP,
        STEREO_SAT,
        STEREO_ADR,
    };

    enum eFIRType
    {
        FIR_BG_DK_NICAM,
        FIR_I_NICAM,
        FIR_L_NICAM,
        FIR_BG_DK_DUAL_FM,
        FIR_AM_DETECT,
        FIR_SAT,
        FIR_FM_RADIO,
    };

#define MSP_CARRIER(freq) ((int)((float)(freq/18.432)*(1<<24)))
    enum eCarrier
    {
        MSP34x0_NOCARRIER           = 0,
        MSP34x0_CARRIER_4_5         = MSP_CARRIER(4.5),
        MSP34x0_CARRIER_4_724212    = MSP_CARRIER(4.724212),
        MSP34x0_CARRIER_5_5         = MSP_CARRIER(5.5),
        MSP34x0_CARRIER_5_7421875   = MSP_CARRIER(5.7421875),
        MSP34x0_CARRIER_5_85        = MSP_CARRIER(5.85),
        MSP34x0_CARRIER_6_0         = MSP_CARRIER(6.0),
        MSP34x0_CARRIER_6_12        = MSP_CARRIER(6.12),
        MSP34x0_CARRIER_6_2578125   = MSP_CARRIER(6.2578125),
        MSP34x0_CARRIER_6_5         = MSP_CARRIER(6.5),
        MSP34x0_CARRIER_6_552       = MSP_CARRIER(6.552),
        MSP34x0_CARRIER_6_7421875   = MSP_CARRIER(6.7421875),
        MSP34x0_CARRIER_7_02        = MSP_CARRIER(7.02),
        MSP34x0_CARRIER_7_20        = MSP_CARRIER(7.20),
        MSP34x0_CARRIER_7_38        = MSP_CARRIER(7.38),
        MSP34x0_CARRIER_10_7        = MSP_CARRIER(10.7),
    };
#undef MSP_CARRIER

    typedef struct
    {
        TCHAR*           Name;
        eStandard       Standard;
        eCarrier        MajorCarrier;
        eCarrier        MinorCarrier;
        eMonoType       MonoType;
        eStereoType     StereoType;
        eFIRType        FIRType;
    } TStandardDefinition;

    typedef struct
    {
        BYTE       FIR1[6];
        BYTE       FIR2[6];
    } TFIRType;

    enum eScartOutput
    {
        MSP34x0_SCARTOUTPUT_DSP_INPUT = 0,
        MSP34x0_SCARTOUTPUT_SCART_1,
        MSP34x0_SCARTOUTPUT_SCART_2,
        MSP34x0_SCARTOUTPUT_LASTONE
    };
    enum eScartInput
    {
        MSP34x0_SCARTINPUT_MUTE = 0,
        MSP34x0_SCARTINPUT_MONO,
        MSP34x0_SCARTINPUT_SCART_1,
        MSP34x0_SCARTINPUT_SCART_1_DA,
        MSP34x0_SCARTINPUT_SCART_2,
        MSP34x0_SCARTINPUT_SCART_2_DA,
        MSP34x0_SCARTINPUT_SCART_3,
        MSP34x0_SCARTINPUT_SCART_4,
        MSP34x0_SCARTINPUT_LASTONE
    };

    enum eMSPVersion
    {
        MSPVersionA,
        MSPVersionD,
        MSPVersionG,
    };

    typedef struct
    {
      eCarrier Major;
      eCarrier Minor[10];
    } TCarrierDetect;


    void SetSCARTxbar(eScartOutput output, eScartInput input);

public:
    int DetectThread();

private:
    int m_CarrierDetect_Phase;

    void InitializeRevA();
    eStandard DetectStandardRevA();
    eSupportedSoundChannels DetectSoundChannelsRevA();
    void SetStandardRevA(eStandard standard, eVideoFormat videoformat, BOOL bCurrentCarriers, eSoundChannel soundChannel);
    void SetSoundChannelRevA(eSoundChannel soundChannel);
    void SetCarrierRevA(eCarrier MajorCarrier, eCarrier MinorCarrier);

    void InitializeRevG();
    eStandard DetectStandardRevG();
    eSupportedSoundChannels DetectSoundChannelsRevG();
    void SetStandardRevG(eStandard standard, eVideoFormat videoformat, BOOL bWasDetected = FALSE);
    void SetSoundChannelRevG(eSoundChannel soundChannel);
    void SetCarrierRevG(eCarrier MajorCarrier, eCarrier MinorCarrier);

    // Detect thread
    HANDLE m_MSP34xxThread;
    BOOL m_bStopThread;
    void StartThread();
    void StopThread();
    CRITICAL_SECTION MSP34xxCriticalSection;
    BOOL m_ThreadWait;

    int  m_AutoDetecting;
    long m_DetectCounter;
    long m_DetectInterval10ms;
    //

private:
    void Initialize();
    void SetModus();
    BOOL m_IsInitialized;
    eMSPVersion m_MSPVersion;
    BOOL m_bHasEqualizer;
    BOOL m_bHasDolby;
    BOOL m_ForceAMSound;
    BOOL m_ForceVersionA;
    BOOL m_ForceVersionD;
    BOOL m_ForceVersionG;
    BOOL m_ForceHasEqualizer;
    BOOL m_KeepWatchingStereo;
    BOOL m_DetectSupportedSoundChannels;
    eSupportedSoundChannels m_SupportedSoundChannels;
    eSoundChannel m_TargetSoundChannel;
    BOOL m_bUseInputPin1;

    static TStandardDefinition m_MSPStandards[];
    static TFIRType            m_FIRTypes[];
    static WORD m_ScartMasks[MSP34x0_SCARTOUTPUT_LASTONE][MSP34x0_SCARTINPUT_LASTONE + 1];
    static TCarrierDetect CarrierDetectTable[];
};

#endif // !defined(__MSP34X0AUDIODECODER_H__)

#endif // WANT_BT8X8_SUPPORT
