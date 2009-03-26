/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file bt848card.h  bt848card Header file
 */

#ifdef WANT_BT8X8_SUPPORT

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

#include "SAA7118.h"
#include "AD9882.h"

#define BT_INPUTS_PER_CARD 12

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
        /// Synchronous Pixel Interface input on the GPIO pins
        INPUTTYPE_SPI,
    };

    /// Sounds chips we expect to find on a card
    enum eSoundChip
    {
        SOUNDCHIP_NONE,
        SOUNDCHIP_MSP,
        SOUNDCHIP_TDA9875,
        SOUNDCHIP_TDA9874
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
        TInputType Inputs[BT_INPUTS_PER_CARD];
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
        /// Function to set Contrast and Brightness Default SetAnalogContrastBrightness
        void (CBT848Card::*pSetContrastBrightness)(WORD, WORD);
        /// Function to set SaturationU Default SetAnalogSaturationU
        void (CBT848Card::*pSetSaturationU)(WORD);
        /// Function to set SaturationV Default SetAnalogSaturationV
        void (CBT848Card::*pSetSaturationV)(WORD);
        /// Function to set Hue Default SetAnalogHue
        void (CBT848Card::*pSetHue)(BYTE);
        /// Function to set Format Default SetFormat
        void (CBT848Card::*pSetFormat)(int, eVideoFormat);
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
    void HandleTimerMessages(int TimerId);
    CBT848Card(SmartPtr<CHardwareDriver> pDriver);
    ~CBT848Card();

    void SetCardType(int CardType);
    eTVCardId GetCardType();

    void SetVideoSource(int nInput);

    void ResetHardware(DWORD RiscBasePhysical);

    void SetContrastBrightness(WORD Contrast, WORD Brightness);
    void SetWhiteCrushUp(BYTE WhiteCrushUp);
    void SetWhiteCrushDown(BYTE WhiteCrushDown);
    void SetHue(BYTE Hue);
    void SetSaturationU(WORD SaturationU);
    void SetSaturationV(WORD SaturationV);
    void SetBDelay(BYTE BDelay);
    void SetEvenLumaDec(BOOL EvenLumaDec);
    void SetOddLumaDec(BOOL OddLumaDec);
    void SetEvenChromaAGC(BOOL EvenChromaAGC);
    void SetOddChromaAGC(BOOL OddChromaAGC);
    void SetEvenLumaPeak(BOOL EvenLumaPeak);
    void SetOddLumaPeak(BOOL OddLumaPeak);
    void SetColorKill(BOOL ColorKill);
    void SetHorFilter(BOOL HorFilter);
    void SetVertFilter(BOOL VertFilter);
    void SetFullLumaRange(BOOL FullLumaRange);
    void SetCoring(BOOL Coring);
    void SetEvenComb(BOOL EvenComb);
    void SetOddComb(BOOL OddComb);
    void SetAgcDisable(BOOL AgcDisable);
    void SetCrush(BOOL Crush);
    void SetColorBars(BOOL ColorBars);
    void SetGammaCorrection(BOOL GammaCorrection);

    std::string GetChipType();
    std::string GetTunerType();

    void RestartRISCCode(DWORD RiscBasePhysical);
    void SetGeoSize(int nInput, eVideoFormat TVFormat, long& CurrentX, long& CurrentY, long& CurrentVBILines, int VDelay, int HDelay);

    BOOL IsVideoPresent();
    void SetRISCStartAddress(DWORD RiscBasePhysical);
    DWORD GetRISCPos();
    void SetDMA(BOOL bState);
    void StopCapture();
    void StartCapture(BOOL bCaptureVBI);

    HMENU GetCardSpecificMenu();

    void InitAudio(BOOL UsePin1);

    // AudioControls facade
    void SetAudioMute();
    void SetAudioUnMute(WORD nVolume, eAudioInput Input);
    BOOL HasAudioVolume();
    void SetAudioVolume(WORD nVolume);
    BOOL HasAudioBalance();
    void SetAudioBalance(WORD nBalance);
    BOOL HasAudioBass();
    void SetAudioBass(WORD nBass);
    BOOL HasAudioTreble();
    void SetAudioTreble(WORD nTreble);
    BOOL HasAudioEqualizers();
    void SetAudioEqualizerLevel(WORD nIndex, WORD nLevel);
    BOOL HasAudioBassBoost();
    void SetAudioBassBoost(BOOL bBoost);
    BOOL HasAudioLoudness();
    void SetAudioLoudness(WORD nLevel);
    BOOL HasAudioSpatialEffect();
    void SetAudioSpatialEffect(int nLevel);
    BOOL HasAudioDolby();
    void SetAudioDolby(WORD nMode, WORD nNoise, WORD nSpatial, WORD nPan, WORD nPanorama);
    BOOL HasAudioAutoVolumeCorrection();
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
    BOOL AutoDetectMSP3400();

    BOOL InitTuner(eTunerId tunerId);
    std::string GetInputName(int nVideoSource);
    std::string GetCardName(eTVCardId CardId);
    int GetNumInputs();
    BOOL IsInputATuner(int nInput);
    eAudioInput GetAudioInput();
    LPCSTR GetAudioInputName(eAudioInput nInput);
    int GetNumAudioInputs();

    BOOL GetHasUseInputPin1();
    BOOL GetUseInputPin1();
    void SetUseInputPin1(BOOL AValue);

    void SetPMSChannelGain(int ChannelNum, WORD Gain);

    BOOL IsMyAudioDecoder(SmartPtr<CAudioDecoder> pAudioDecoder);

    static BOOL APIENTRY ChipSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

    BOOL IsSPISource(int nInput);
    SmartPtr<CAD9882> GetAD9882() { return m_AD9882; }

    // I2C stuff
    void SetSDA(BOOL value);
    void SetSCL(BOOL value);
    BOOL GetSDA();
    BOOL GetSCL();
    void Sleep();
    void I2CLock();
    void I2CUnlock();
    SmartPtr<ITuner> GetTuner() const;

    // GPIO stuff
    void SetGPOE(ULONG val);
    ULONG GetGPOE();
    void SetGPDATA(ULONG val);
    ULONG GetGPDATA();
    void ShowRegisterSettingsDialog(HINSTANCE hBT8x8xResourceInst);


protected:
    void ManageMyState();
    BOOL SupportsACPI() {return Is878Family();};
    void ResetChip();

private:
    ULONG GetTickCount();
    DWORD m_I2CSleepCycle;
    DWORD m_I2CRegister;
    BOOL m_I2CInitialized;
    void InitializeI2C();

    void SetGeometryEvenOdd(BOOL bOdd, int wHScale, int wVScale, int wHActive, int wVActive, int wHDelay, int wVDelay, BYTE bCrop);
    void SetPLL(ePLLFreq PLL);
    BOOL IsCCIRSource(int nInput);
    SmartPtr<const TCardType> GetCardSetup();

    void StandardBT848InputSelect(int nInput);
    void Sasem4ChannelInputSelect(int nInput);
    void RSBTCardInputSelect(int nInput);
    void Silk200InputSelect(int nInput);
    void StandardSetFormat(int nInput, eVideoFormat TVFormat);

    void InitPXC200();
    void InitHauppauge();
    void InitVoodoo();
    void InitSasem();
    void InitRSBT();

    void BootMSP34xx(int pin);
    void CtrlTDA8540(BYTE SLV, BYTE SUB, BYTE SW1, BYTE GCO, BYTE OEN);
    void CtrlSilkSDISwitch(BYTE SLV, BYTE IEN);

    static BOOL APIENTRY RegisterEditProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

    std::string m_AudioDecoderType;
    std::string m_TunerType;

    eTVCardId m_CardType;

    SmartPtr<CI2CBus>        m_I2CBus;
    SmartPtr<II2CTuner>      m_Tuner;
    SmartPtr<CAudioDecoder>  m_AudioDecoder;
    SmartPtr<CAudioControls> m_AudioControls;

    SmartPtr<CSAA7118>       m_SAA7118;
    int                      m_CurrentInput;
    SmartPtr<CAD9882>        m_AD9882;
    DWORD                    m_PixelWidth;

    void InitPMSDeluxe();
    void PMSDeluxeInputSelect(int nInput);
    void SetPMSDeluxeFormat(int nInput, eVideoFormat TVFormat);
    void SetPMSDeluxeContrastBrightness(WORD Contrast, WORD Brightness);
    void SetPMSDeluxeSaturationU(WORD SaturationU);
    void SetPMSDeluxeSaturationV(WORD SaturationV);
    void SetPMSDeluxeHue(BYTE Hue);

    void InitAtlas();
    void AtlasInputSelect(int nInput);
    void SetAtlasFormat(int nInput, eVideoFormat TVFormat);
    void SetAtlasContrastBrightness(WORD Contrast, WORD Brightness);
    void SetAtlasSaturationU(WORD SaturationU);
    void SetAtlasSaturationV(WORD SaturationV);
    void SetAtlasHue(BYTE Hue);

    void SetAnalogContrastBrightness(WORD Contrast, WORD Brightness);
    void SetAnalogSaturationU(WORD SaturationU);
    void SetAnalogSaturationV(WORD SaturationV);
    void SetAnalogHue(BYTE Hue);

    static const TCardType m_TVCards[TVCARD_LASTONE];
    static const TAutoDectect878 m_AutoDectect878[];
    static const eTunerId m_Tuners_miro[];
    static const BOOL     m_Tuners_miro_fm[];
    static const eTunerId m_Tuners_avermedia_0[];
    static const eTunerId m_Tuners_avermedia_0_fm[];
    static const eTunerId m_Tuners_avermedia_1[];

};


#endif

#endif // WANT_BT8X8_SUPPORT