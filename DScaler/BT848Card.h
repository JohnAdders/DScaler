/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Card.h,v 1.15 2002-02-03 18:14:40 adcockj Exp $
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
#include "IAudioControls.h"
#include "NoAudioControls.h"

#define INPUTS_PER_CARD 6

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
        SOUNDCHIP_MT2032,
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
        void (CBT848Card::*pSoundChannelFunction)(eSoundChannel);
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
	void HandleTimerMessages(int TimerId);
    CBT848Card(CHardwareDriver* pDriver);
	~CBT848Card();

	BOOL FindCard(WORD VendorID, WORD DeviceID, int CardIndex);
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

    void RestartRISCCode(DWORD RiscBasePhysical);
    void SetGeoSize(int nInput, eVideoFormat TVFormat, long& CurrentX, long& CurrentY, long& CurrentVBILines, int VDelay, int HDelay);

    BOOL IsVideoPresent();
    void SetRISCStartAddress(DWORD RiscBasePhysical);
    DWORD GetRISCPos();
    void SetDMA(BOOL bState);
    void StopCapture();
    void StartCapture(BOOL bCaptureVBI);

    BOOL HasMSP();

    void InitAudio();

    void SetAudioMute();
    void SetAudioUnMute(long nVolume);
    void SetAudioVolume(WORD nVolume);
    void SetAudioBalance(WORD nBalance);
    void SetAudioBass(WORD nBass);
    void SetAudioTreble(WORD nTreble);

    void SetAudioStandard(eVideoFormat videoFormat);
    void SetAudioSource(eAudioInput nChannel);
    void SetAudioChannel(eSoundChannel audioChannel);
    void GetMSPPrintMode(LPSTR Text);
    eSoundChannel IsAudioChannelDetected(eSoundChannel desiredAudioChannel);
    
    eTunerId AutoDetectTuner(eTVCardId CardId);
    eTVCardId AutoDetectCardType();

    BOOL InitTuner(eTunerId tunerId);
    LPCSTR GetInputName(int nVideoSource);
    LPCSTR GetCardName(eTVCardId CardId);
    int GetNumInputs();
    BOOL IsInputATuner(int nInput);

    static BOOL APIENTRY ChipSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

    // I2C stuff
    void SetSDA(bool value);
    void SetSCL(bool value);
    bool GetSDA();
    bool GetSCL();
    void Sleep();
    ITuner* GetTuner() const;

private:
    ULONG GetTickCount();
    DWORD m_I2CSleepCycle;
    DWORD m_I2CRegister;
    bool m_I2CInitialized;
    void InitializeI2C();

private:
    void SetGeometryEvenOdd(BOOL bOdd, int wHScale, int wVScale, int wHActive, int wVActive, int wHDelay, int wVDelay, BYTE bCrop);
    void SetPLL(ePLLFreq PLL);
    BOOL IsCCIRSource(int nInput);
    const TCardType* GetCardSetup();

    void StandardBT848InputSelect(int nInput);
    void Sasem4ChannelInputSelect(int nInput);

    void InitPXC200();
    void InitHauppauge();
    void InitVoodoo();
    void InitSasem();

    void SetAudioGVBCTV3PCI(eSoundChannel soundChannel);
    void SetAudioLT9415(eSoundChannel soundChannel);
    void SetAudioTERRATV(eSoundChannel soundChannel);
    void SetAudioAVER_TVPHONE(eSoundChannel soundChannel);
    void SetAudioWINFAST2000(eSoundChannel soundChannel);

    void BootMSP34xx(int pin);
    void CtrlTDA8540(BYTE SLV, BYTE SUB, BYTE SW1, BYTE GCO, BYTE OEN);

    char m_MSPVersion[16];
    char m_TunerStatus[32];

private:
    eTVCardId m_CardType;
    bool m_bHasMSP;

    CI2CBus*        m_I2CBus;
    ITuner*         m_Tuner;
    CAudioDecoder*  m_AudioDecoder;
    IAudioControls* m_AudioControls;
    eAudioInput     m_LastAudioSource;

private:
    static const TCardType m_TVCards[TVCARD_LASTONE];
    static const TAutoDectect878 m_AutoDectect878[];
};


#endif