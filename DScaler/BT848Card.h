/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Card.h,v 1.33 2002-10-11 21:35:11 ittarnavsky Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
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
// $Log: not supported by cvs2svn $
// Revision 1.32  2002/10/11 13:38:13  kooiman
// Added support for VoodooTV IF demodulator. Improved TDA9887. Added interface for GPOE/GPDATA access to make this happen.
//
// Revision 1.31  2002/10/02 10:52:36  kooiman
// Fixed C++ type casting for events.
//
// Revision 1.30  2002/09/27 14:13:27  kooiman
// Improved stereo detection & manual audio standard dialog box.
//
// Revision 1.29  2002/09/16 14:37:35  kooiman
// Added stereo autodetection.
//
// Revision 1.28  2002/09/15 15:57:27  kooiman
// Added Audio standard support.
//
// Revision 1.27  2002/09/12 22:00:57  ittarnavsky
// Changes due to the new AudioDecoder handling
// Changes due to the IAudioControls to CAudioControls transition
//
// Revision 1.26  2002/09/07 20:54:49  kooiman
// Added equalizer, loudness, spatial effects for MSP34xx
//
// Revision 1.25  2002/08/27 22:02:32  kooiman
// Added Get/Set input for video and audio for all sources. Added source input change notification.
//
// Revision 1.24  2002/08/03 17:57:52  kooiman
// Added new cards & tuners. Changed the tuner combobox into a sorted list.
//
// Revision 1.23  2002/07/25 05:21:35  dschmelzer
// Added SDI Silk 200 Support
//
// Revision 1.22  2002/07/23 18:11:48  adcockj
// Tuner autodetect patch from Jeroen Kooiman
//
// Revision 1.21  2002/07/02 20:00:06  adcockj
// New setting for MSP input pin selection
//
// Revision 1.20  2002/06/16 18:54:59  robmuller
// ACPI powersafe support.
//
// Revision 1.19  2002/06/05 20:02:27  adcockj
// Applied old RS BT card patch
//
// Revision 1.18  2002/05/19 01:18:42  dschmelzer
// Added a seventh input
//
// Revision 1.17  2002/04/07 10:37:53  adcockj
// Made audio source work per input
//
// Revision 1.16  2002/02/12 02:27:45  ittarnavsky
// fixed the hardware info dialog
//
// Revision 1.15  2002/02/03 18:14:40  adcockj
// Fixed SDI Silk & Sasem cards
//
// Revision 1.14  2002/01/23 12:20:32  robmuller
// Added member function HandleTimerMessages(int TimerId).
//
// Revision 1.13  2001/12/18 15:30:35  adcockj
// Comment changes
//
// Revision 1.10  2001/12/18 13:12:11  adcockj
// Interim check-in for redesign of card specific settings
//
// Revision 1.9  2001/12/05 21:45:10  ittarnavsky
// added changes for the AudioDecoder and AudioControls support
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __BT848CARD_H___
#define __BT848CARD_H___

#include "PCICard.h"
#include "TVFormats.h"
#include "BT848_Defines.h"

#include "I2CLineInterface.h"
#include "I2CBusForLineInterface.h"
#include "ITuner.h"
#include "AudioDecoder.h"
#include "AudioControls.h"

#define INPUTS_PER_CARD 7

/** A Generic bt848 based capture card
    The card can cope with the standard inputs,
    a tuner and one of the supported sound chips.
    Control is given over most of the supported chip 
    functions.
*/
class CBT848Card : public CPCICard, 
                   public II2CLineInterface
{
private:
    /// Different types of input currently supported
    enum eInputType
    {
        /// standard composite input
        INPUTTYPE_COMPOSITE,
        /// standard s-video input
        INPUTTYPE_SVIDEO,
        /// standard analogue tuner input composite
        INPUTTYPE_TUNER,
        /// Digital CCIR656 input on the GPIO pins
        INPUTTYPE_CCIR,
        /// Radio input so no video
        INPUTTYPE_RADIO,
    };

    /// Sounds chips we expect to find on a card
    enum eSoundChip
    {
        SOUNDCHIP_NONE,
        SOUNDCHIP_MSP,
    };

    /// Does the card have a PLL generator - used for PAL & SECAM
    enum ePLLFreq
    {
        PLL_NONE = 0,
        PLL_28,
        PLL_35,
    };

    /// Defines each input on a card
    typedef struct
    {
        /// Name of the input
        LPCSTR szName;
        /// Type of the input
        eInputType InputType;
        /// Which mux on the card is to be used
        BYTE MuxSelect;
    } TInputType;

    /// Defines the specific settings for a given card
    typedef struct
    {
        LPCSTR szName;
        int NumInputs;
        TInputType Inputs[INPUTS_PER_CARD];
        ePLLFreq PLLFreq;
        eTunerId TunerId;
        eSoundChip SoundChip;
        /// Any card specific initialization - may be NULL
        void (CBT848Card::*pInitCardFunction)(void);
        /** Function used to switch between sources
            Cannot be NULL
            Default is StandardBT848InputSelect
        */
        void (CBT848Card::*pInputSwitchFunction)(int);
        /// Any card specific method used to select stereo - may be NULL
        CAudioDecoder::eAudioDecoderType AudioDecoderType;
        /// Bit Mask for audio GPIO operations
        DWORD GPIOMask;
        /** GPIO Flags for the various inputs
            We seem to use 
            AUDIOINPUT_TUNER to be selected when tuner is selected and no MSP
            AUDIOINPUT_RADIO to be selected when tuner is selected and ther is a MSP
            AUDIOINPUT_EXTERNAL to be selected when anything but the tuner is required
            AUDIOINPUT_MUTE for muting
        */
        DWORD AudioMuxSelect[6];
    } TCardType;

    /// used to store the ID for autodection
    typedef struct
    {
        DWORD ID;
        eTVCardId CardId;
        char* szName;
    } TAutoDectect878;
    
public:
    BOOL Is878Family();
    void SetACPIStatus(int ACPIStatus);
    int GetACPIStatus();
    void HandleTimerMessages(int TimerId);
    CBT848Card(CHardwareDriver* pDriver);
    ~CBT848Card();

    //BOOL FindCard(WORD VendorID, WORD DeviceID, int CardIndex);
    void CloseCard();

    void SetCardType(int CardType);
    eTVCardId GetCardType();
    
    void SetVideoSource(int nInput);

    void ResetHardware(DWORD RiscBasePhysical);

    void SetBrightness(BYTE Brightness);
    BYTE GetBrightness();
    void SetWhiteCrushUp(BYTE WhiteCrushUp);
    BYTE GetWhiteCrushUp();
    void SetWhiteCrushDown(BYTE WhiteCrushDown);
    BYTE GetWhiteCrushDown();
    void SetHue(BYTE Hue);
    BYTE GetHue();
    void SetContrast(WORD Contrast);
    WORD GetContrast(WORD Contrast);
    void SetSaturationU(WORD SaturationU);
    WORD GetSaturationU(WORD SaturationU);
    void SetSaturationV(WORD SaturationV);
    WORD GetSaturationV(WORD SaturationV);
    void SetBDelay(BYTE BDelay);
    BYTE GetBDelay();
    void SetEvenLumaDec(BOOL EvenLumaDec);
    BOOL GetEvenLumaDec();
    void SetOddLumaDec(BOOL OddLumaDec);
    BOOL GetOddLumaDec();
    void SetEvenChromaAGC(BOOL EvenChromaAGC);
    BOOL GetEvenChromaAGC();
    void SetOddChromaAGC(BOOL OddChromaAGC);
    BOOL GetOddChromaAGC();
    void SetEvenLumaPeak(BOOL EvenLumaPeak);
    BOOL GetEvenLumaPeak();
    void SetOddLumaPeak(BOOL OddLumaPeak);
    BOOL GetOddLumaPeak();
    void SetColorKill(BOOL ColorKill);
    BOOL GetColorKill();
    void SetHorFilter(BOOL HorFilter);
    BOOL GetHorFilter();
    void SetVertFilter(BOOL VertFilter);
    BOOL GetVertFilter();
    void SetFullLumaRange(BOOL FullLumaRange);
    BOOL GetFullLumaRange();
    void SetCoring(BOOL Coring);
    BOOL GetCoring();
    void SetEvenComb(BOOL EvenComb);
    BOOL GetEvenComb();
    void SetOddComb(BOOL OddComb);
    BOOL GetOddComb();
    void SetAgcDisable(BOOL AgcDisable);
    BOOL GetAgcDisable();
    void SetCrush(BOOL Crush);
    BOOL GetCrush();
    void SetColorBars(BOOL ColorBars);
    BOOL GetColorBars();
    void SetGammaCorrection(BOOL GammaCorrection);
    BOOL GetGammaCorrection();

    LPCSTR GetChipType();
    LPCSTR GetTunerType();

    void RestartRISCCode(DWORD RiscBasePhysical);
    void SetGeoSize(int nInput, eVideoFormat TVFormat, long& CurrentX, long& CurrentY, long& CurrentVBILines, int VDelay, int HDelay);

    BOOL IsVideoPresent();
    void SetRISCStartAddress(DWORD RiscBasePhysical);
    DWORD GetRISCPos();
    void SetDMA(BOOL bState);
    void StopCapture();
    void StartCapture(BOOL bCaptureVBI);

    void InitAudio();

    // AudioControls facade
    void SetAudioMute();
    void SetAudioUnMute(WORD nVolume, eAudioInput Input);
    bool HasAudioVolume();
    void SetAudioVolume(WORD nVolume);
    bool HasAudioBalance();
    void SetAudioBalance(WORD nBalance);
    bool HasAudioBass();
    void SetAudioBass(WORD nBass);
    bool HasAudioTreble();
    void SetAudioTreble(WORD nTreble);
    bool HasAudioEqualizers();
    void SetAudioEqualizerLevel(WORD nIndex, WORD nLevel);
    bool HasAudioBassBoost();
    void SetAudioBassBoost(bool bBoost);
    bool HasAudioLoudness();
    void SetAudioLoudness(WORD nLevel);
    bool HasAudioSpatialEffect();
    void SetAudioSpatialEffect(WORD nLevel);
    bool HasAudioDolby();
    void SetAudioDolby(WORD nMode, WORD nNoise, WORD nSpatial, WORD nPan, WORD nPanorama);
    bool HasAudioAutoVolumeCorrection();
    void SetAudioAutoVolumeCorrection(long milliSeconds);

    // AudioDecoder facade    
    CAudioDecoder::eAudioDecoderType GetAudioDecoderType();
    int SetAudioDecoderValue(int What, long Val);
    long GetAudioDecoderValue(int What);   
    void SetAudioSource(eAudioInput audioInput);
    void SetAudioChannel(eSoundChannel audioChannel);
    eSoundChannel IsAudioChannelDetected(eSoundChannel desiredAudioChannel);
    void SetAudioStandard(long Standard, eVideoFormat videoformat);
    long GetAudioStandardCurrent();    
    const char* GetAudioStandardName(long Standard);
    int GetNumAudioStandards();
    long GetAudioStandard(int nIndex);
    long GetAudioStandardMajorCarrier(long Standard);
    long GetAudioStandardMinorCarrier(long Standard);
    void SetAudioStandardCarriers(long MajorCarrier, long MinorCarrier);
    long GetAudioStandardFromVideoFormat(eVideoFormat videoFormat);
    void DetectAudioStandard(long Interval, int SupportedSoundChannels, eSoundChannel TargetChannel);

    eTunerId AutoDetectTuner(eTVCardId CardId);
    eTVCardId AutoDetectCardType();
    bool AutoDetectMSP3400();

    BOOL InitTuner(eTunerId tunerId);
    LPCSTR GetInputName(int nVideoSource);
    LPCSTR GetCardName(eTVCardId CardId);
    int GetNumInputs();
    BOOL IsInputATuner(int nInput);
    eAudioInput GetAudioInput();
    LPCSTR GetAudioInputName(eAudioInput nInput);
    int GetNumAudioInputs();

    bool GetHasUseInputPin1();
    bool GetUseInputPin1();
    void SetUseInputPin1(bool AValue);
    
    BOOL IsMyAudioDecoder(CAudioDecoder* pAudioDecoder);

    static BOOL APIENTRY ChipSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

    // I2C stuff
    void SetSDA(bool value);
    void SetSCL(bool value);
    bool GetSDA();
    bool GetSCL();
    void Sleep();
    ITuner* GetTuner() const;

    // GPIO stuff
    void SetGPOE(ULONG val);
    ULONG GetGPOE();
    void SetGPDATA(ULONG val);
    ULONG GetGPDATA();
private:
    ULONG GetTickCount();
    DWORD m_I2CSleepCycle;
    DWORD m_I2CRegister;
    bool m_I2CInitialized;
    void InitializeI2C();

    void SetGeometryEvenOdd(BOOL bOdd, int wHScale, int wVScale, int wHActive, int wVActive, int wHDelay, int wVDelay, BYTE bCrop);
    void SetPLL(ePLLFreq PLL);
    BOOL IsCCIRSource(int nInput);
    const TCardType* GetCardSetup();

    void StandardBT848InputSelect(int nInput);
    void Sasem4ChannelInputSelect(int nInput);
    void RSBTCardInputSelect(int nInput);
    void Silk200InputSelect(int nInput);

    void InitPXC200();
    void InitHauppauge();
    void InitVoodoo();
    void InitSasem();
    void InitRSBT();

    void BootMSP34xx(int pin);
    void CtrlTDA8540(BYTE SLV, BYTE SUB, BYTE SW1, BYTE GCO, BYTE OEN);
    void CtrlSilkSDISwitch(BYTE SLV, BYTE IEN);

    char m_AudioDecoderType[32];
    char m_TunerType[32];

    eTVCardId m_CardType;

    CI2CBus*        m_I2CBus;
    ITuner*         m_Tuner;
    CAudioDecoder*  m_AudioDecoder;
    CAudioControls* m_AudioControls;

    static const TCardType m_TVCards[TVCARD_LASTONE];
    static const TAutoDectect878 m_AutoDectect878[];
    static const eTunerId m_Tuners_miro[];
    static const bool     m_Tuners_miro_fm[];
    static const eTunerId m_Tuners_hauppauge[];
    static const eTunerId m_Tuners_avermedia_0[];
    static const eTunerId m_Tuners_avermedia_0_fm[];
    static const eTunerId m_Tuners_avermedia_1[];

};


#endif