/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Card.h,v 1.40 2003-10-27 16:22:56 adcockj Exp $
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
// Revision 1.39  2003/10/27 10:39:50  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.38  2002/11/07 20:33:16  adcockj
// Promoted ACPI functions so that state management works properly
//
// Revision 1.37  2002/11/07 13:37:42  adcockj
// Added State restoration code to PCICard
// Functionality disabled prior to testing and not done for SAA7134
//
// Revision 1.36  2002/10/27 12:33:33  adcockj
// Fixed UseInputPin1 code
//
// Revision 1.35  2002/10/26 15:37:55  adcockj
// Made ITuner more abstract by removing inheritance from CI2CDevice
// New class II2CTuner created for tuners that are controled by I2C
//
// Revision 1.34  2002/10/15 19:16:29  kooiman
// Fixed Spatial effect for Audio decoder & MSP
//
// Revision 1.33  2002/10/11 21:35:11  ittarnavsky
// moved eAudioDecoderType to AudioDecoder.h
// removed GetAudioDecoderType()
// changed GetAudioDecoderID() with GetAudioDecoderType()
// added UseInputPin1 support
//
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

/** 
 * @file bt848card.h  bt848card Header file
 */

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
        void (CBT848Card::*pSetContrastBrightness)(WORD, BYTE);
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
    CBT848Card(CHardwareDriver* pDriver);
    ~CBT848Card();

    void SetCardType(int CardType);
    eTVCardId GetCardType();
    
    void SetVideoSource(int nInput);

    void ResetHardware(DWORD RiscBasePhysical);

    void SetContrastBrightness(WORD Contrast, BYTE Brightness);
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

    HMENU GetCardSpecificMenu();

    void InitAudio(bool UsePin1);

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
    void SetAudioSpatialEffect(int nLevel);
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


protected:
    void ManageMyState();
    BOOL SupportsACPI() {return Is878Family();};
    void ResetChip();

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
    void StandardSetFormat(int nInput, eVideoFormat TVFormat);

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
    II2CTuner*         m_Tuner;
    CAudioDecoder*  m_AudioDecoder;
    CAudioControls* m_AudioControls;

	CSAA7118*       m_SAA7118;
	int             m_CurrentInput;

	void InitPMSDeluxe();
	void PMSDeluxeInputSelect(int nInput);
	void SetPMSDeluxeFormat(int nInput, eVideoFormat TVFormat);
	void SetPMSDeluxeContrastBrightness(WORD Contrast, BYTE Brightness);
	void SetPMSDeluxeSaturationU(WORD SaturationU);
	void SetPMSDeluxeSaturationV(WORD SaturationV);
	void SetPMSDeluxeHue(BYTE Hue);

    void SetAnalogContrastBrightness(WORD Contrast, BYTE Brightness);
    void SetAnalogSaturationU(WORD SaturationU);
    void SetAnalogSaturationV(WORD SaturationV);
    void SetAnalogHue(BYTE Hue);

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
