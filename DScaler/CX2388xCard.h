/////////////////////////////////////////////////////////////////////////////
// $Id: CX2388xCard.h,v 1.18 2003-07-18 09:41:23 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 John Adcock.  All rights reserved.
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
// This code is based on a version of dTV modified by Michael Eskin and
// others at Connexant.  Those parts are probably (c) Connexant 2002
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __CX2388XCARD_H___
#define __CX2388XCARD_H___

#include "PCICard.h"
#include "TVFormats.h"
#include "CX2388x_Defines.h"

#include "I2CLineInterface.h"
#include "I2CBusForLineInterface.h"
#include "ITuner.h"
#include "AudioDecoder.h"
#include "AudioControls.h"
#include "SAA7118.h"

#define CT_INPUTS_PER_CARD 9

class CCX2388xCard : public CPCICard, 
                     public II2CLineInterface
{
public:
    enum eCombFilter
    {
        COMBFILTER_DEFAULT = 0,
        COMBFILTER_OFF,
        COMBFILTER_CHROMA_ONLY,
        COMBFILTER_FULL,
    };

    enum eFlagWithDefault
    {
        FLAG_DEFAULT = 0,
        FLAG_OFF,
        FLAG_ON,
    };
    
    enum eWhiteCrushMajSel
    {
        MAJSEL_3_OVER_4 = 0,
        MAJSEL_1_OVER_2,
        MAJSEL_1_OVER_4,
        MAJSEL_AUTOMATIC
    };

    enum eAudioStandard
    {
        AUDIO_STANDARD_AUTO = 0,
        AUDIO_STANDARD_BTSC,
        AUDIO_STANDARD_EIAJ,
        AUDIO_STANDARD_A2,
        AUDIO_STANDARD_BTSC_SAP,
        AUDIO_STANDARD_NICAM,
        AUDIO_STANDARD_FM,
    };

    enum eStereoType
    {
        STEREOTYPE_AUTO = 0,
        STEREOTYPE_STEREO, 
        STEREOTYPE_MONO, 
        STEREOTYPE_ALT1, 
        STEREOTYPE_ALT2, 
    };

private:
    /// Different types of input currently supported
    enum eInputType
    {
        /// standard composite input
        INPUTTYPE_TUNER,
        /// standard composite input
        INPUTTYPE_COMPOSITE,
        /// standard s-video input
        INPUTTYPE_SVIDEO,
        /// Digital CCIR656 input on the GPIO pins
        INPUTTYPE_CCIR,
        /// Shows Colour Bars for testing
        INPUTTYPE_COLOURBARS,
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
        TInputType Inputs[CT_INPUTS_PER_CARD];

        /// Any card specific initialization - may be NULL
        void (CCX2388xCard::*pInitCardFunction)(void);
        /// Any card specific routine required to stop capture - may be NULL
        void (CCX2388xCard::*pStopCaptureCardFunction)(void);
        /** Function used to switch between sources
            Cannot be NULL
            Default is StandardBT848InputSelect
        */
        void (CCX2388xCard::*pInputSwitchFunction)(int);
        /// Function to set Contrast and Brightness Default SetAnalogContrastBrightness
        void (CCX2388xCard::*pSetContrastBrightness)(BYTE, BYTE);
        /// Function to set Hue Default SetAnalogHue
        void (CCX2388xCard::*pSetHue)(BYTE);
        /// Function to set SaturationU Default SetAnalogSaturationU
        void (CCX2388xCard::*pSetSaturationU)(BYTE);
        /// Function to set SaturationV Default SetAnalogSaturationV
        void (CCX2388xCard::*pSetSaturationV)(BYTE);
        /// Function to set Format Default SetFormat
        void (CCX2388xCard::*pSetFormat)(int, eVideoFormat, BOOL);
        eTunerId TunerId;
        int MenuId;
    } TCardType;

    /// used to store the ID for autodection
    typedef struct
    {
        DWORD ID;
        eCX2388xCardId CardId;
        char* szName;
    } TAutoDectect;

public:
	void HandleTimerMessages(int TimerId);
    CCX2388xCard(CHardwareDriver* pDriver);
	~CCX2388xCard();

    void StartCapture(BOOL bCaptureVBI);
    void StopCapture();

    void SetCardType(int CardType);
    eCX2388xCardId GetCardType();
    
    void SetVideoSource(int nInput);

    void SetContrastBrightness(BYTE Contrast, BYTE Brightness);
    void SetHue(BYTE Hue);
    void SetSaturationU(BYTE SaturationU);
    void SetSaturationV(BYTE SaturationV);
    void SetSharpness(char Sharpness);

    void SetGeoSize(int nInput, eVideoFormat TVFormat, long& CurrentX, long& CurrentY, long& CurrentVBILines, BOOL IsProgressive);

    BOOL IsVideoPresent();
    DWORD GetRISCPos();
    void ResetHardware();

    eCX2388xCardId AutoDetectCardType();

    LPCSTR GetInputName(int nVideoSource);
    LPCSTR GetCardName(eCX2388xCardId CardId);
    int GetNumInputs();
    BOOL IsInputATuner(int nInput);
    LPCSTR GetTunerType();
    eTunerId AutoDetectTuner(eCX2388xCardId CardId);
    BOOL InitTuner(eTunerId tunerId);
    void SetRISCStartAddress(DWORD RiscBasePhysical);
    void SetRISCStartAddressVBI(DWORD RiscBasePhysical);
	void SetFLIFilmDetect(BOOL FLIFilmDetect);
    void SetLumaAGC(BOOL LumaAGC);
    void SetChromaAGC(BOOL ChromaAGC);
    void SetFastSubcarrierLock(BOOL LockFast);
    void SetWhiteCrushEnable(BOOL WhiteCrush);
    void SetWhiteCrushUp(BYTE WhiteCrushUp);
    void SetWhiteCrushDown(BYTE WhiteCrushDown);
    void SetWhiteCrushMajorityPoint(eWhiteCrushMajSel WhiteCrushMajSel);
    void SetWhiteCrushPerFrame(BOOL WhiteCrushPerFrame);
    void SetLowColorRemoval(BOOL LowColorRemoval);
    void SetCombFilter(eCombFilter CombFilter);
    void SetFullLumaRange(BOOL FullLumaRange);
	void SetRemodulation(eFlagWithDefault Remodulation);
	void SetChroma2HComb(eFlagWithDefault Chroma2HComb);
	void SetForceRemodExcessChroma(eFlagWithDefault ForceRemodExcessChroma);
	void SetIFXInterpolation(eFlagWithDefault IFXInterpolation);
	void SetCombRange(int CombRange);
	void SetSecondChromaDemod(eFlagWithDefault SecondChromaDemod);
	void SetThirdChromaDemod(eFlagWithDefault ThirdChromaDemod);
    void SetHDelay(int nInput, eVideoFormat TVFormat, long CurrentX, int HDelayAdj);
    void SetVDelay(int nInput, eVideoFormat TVFormat, long CurrentX, int VDelayAdj);

    static BOOL APIENTRY ChipSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

    // I2C stuff
    void SetSDA(bool value);
    void SetSCL(bool value);
    bool GetSDA();
    bool GetSCL();
    void Sleep();
    ITuner* GetTuner() const;

    void DumpChipStatus(const char* CardName);
    HMENU GetCardSpecificMenu();

    void AudioInit(int nInput, eVideoFormat TVFormat, eAudioStandard Standard, eStereoType StereoType);
    void SetAudioMute();
    void SetAudioUnMute(WORD nVolume);
    void SetAudioVolume(WORD nVolume);
    void SetAudioBalance(WORD nBalance);
    void ShowRegisterSettingsDialog(HINSTANCE hCX2388xResourceInst);

protected:
    void ManageMyState();
    /// Card does support ACPI
    BOOL SupportsACPI() {return TRUE;};
    /// don't know how to reset
    void ResetChip() {;};

private:
    ULONG GetTickCount();
    DWORD m_I2CSleepCycle;
    DWORD m_I2CRegister;
    bool m_I2CInitialized;
    void InitializeI2C();

private:
    BOOL IsCCIRSource(int nInput);
    const TCardType* GetCardSetup();
    /// Sets up Sample rate converter (Freq in MHz)
    void SetSampleRateConverter(double PLLFreq);
    /** Sets up PLL (Freq in MHz)
	    Returns actual value set for use in further calcs
	*/
    double SetPLL(double PLLFreq);

    void StandardInputSelect(int nInput);

    void StandardSetFormat(int nInput, eVideoFormat TVFormat, BOOL IsProgressive);

    void SetAnalogContrastBrightness(BYTE Contrast, BYTE Brightness);
    void SetAnalogHue(BYTE Hue);
    void SetAnalogSaturationU(BYTE SaturationU);
    void SetAnalogSaturationV(BYTE SaturationV);

    void SetVIPBrightness(BYTE Brightness);
    void SetVIPContrast(BYTE Contrast);
    void SetVIPSaturation(BYTE Saturation);

	void InitH3D();
	void H3DInputSelect(int nInput);
    void MSIInputSelect(int nInput);
    void PlayHDInputSelect(int nInput);
    void PlayHDStopCapture();
    void AsusInputSelect(int nInput);

	void H3DSetFormat(int nInput, eVideoFormat TVFormat, BOOL IsProgressive);
    void SetH3DContrastBrightness(BYTE Contrast, BYTE Brightness);
    void SetH3DHue(BYTE Hue);
    void SetH3DSaturationU(BYTE SaturationU);
    void SetH3DSaturationV(BYTE SaturationV);

    void AudioInitDMA();
    void AudioInitBTSC(eStereoType StereoType);
    void AudioInitBTSCSAP(eStereoType StereoType);
    void AudioInitEIAJ(eStereoType StereoType);
    void AudioInitA2(eStereoType StereoType);
    void AudioInitFM(eStereoType StereoType);
    void AudioInitNICAM(eStereoType StereoType);
    static BOOL APIENTRY RegisterEditProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

private:
    eCX2388xCardId m_CardType;

    CI2CBus*        m_I2CBus;
    II2CTuner*      m_Tuner;
	CSAA7118*       m_SAA7118;

    BOOL            m_RISCIsRunning;
    CAudioDecoder*  m_AudioDecoder;
    CAudioControls* m_AudioControls;
    char            m_TunerType[32];
    int             m_CurrentInput;
    DWORD           m_FilterDefault;
    DWORD           m_2HCombDefault;

private:
    static const TCardType m_TVCards[CX2388xCARD_LASTONE];
    static const TAutoDectect m_AutoDectect[];
};


#endif