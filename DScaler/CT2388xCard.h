/////////////////////////////////////////////////////////////////////////////
// $Id: CT2388xCard.h,v 1.1 2002-09-11 18:19:37 adcockj Exp $
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
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

#ifndef __CT2388XCARD_H___
#define __CT2388XCARD_H___

#include "PCICard.h"
#include "TVFormats.h"
#include "CT2388X_Defines.h"

#include "I2CLineInterface.h"
#include "I2CBusForLineInterface.h"
#include "ITuner.h"
#include "AudioDecoder.h"
#include "IAudioControls.h"
#include "NoAudioControls.h"
#include "SAA7118.h"

#define CT_INPUTS_PER_CARD 8

class CCT2388xCard : public CPCICard, 
                     public II2CLineInterface
{
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
        void (CCT2388xCard::*pInitCardFunction)(void);
        /** Function used to switch between sources
            Cannot be NULL
            Default is StandardBT848InputSelect
        */
        void (CCT2388xCard::*pInputSwitchFunction)(int);
        /// Function to set Brighness Default SetAnalogBrightness
        void (CCT2388xCard::*pSetBrightness)(BYTE);
        /// Function to set Contrast Default SetAnalogContrast
        void (CCT2388xCard::*pSetContrast)(BYTE);
        /// Function to set Hue Default SetAnalogHue
        void (CCT2388xCard::*pSetHue)(BYTE);
        /// Function to set SaturationU Default SetAnalogSaturationU
        void (CCT2388xCard::*pSetSaturationU)(BYTE);
        /// Function to set SaturationV Default SetAnalogSaturationV
        void (CCT2388xCard::*pSetSaturationV)(BYTE);
        /// Function to set Format Default SetFormat
        void (CCT2388xCard::*pSetFormat)(int, eVideoFormat, BOOL);
        eTunerId TunerId;
    } TCardType;

    /// used to store the ID for autodection
    typedef struct
    {
        DWORD ID;
        eCT2388xCardId CardId;
        char* szName;
    } TAutoDectect;

public:
	void HandleTimerMessages(int TimerId);
    CCT2388xCard(CHardwareDriver* pDriver);
	~CCT2388xCard();

	void CloseCard();

    void StartCapture(DWORD RiscBasePhysical);
    void StopCapture();

    void SetCardType(int CardType);
    eCT2388xCardId GetCardType();
    
    void SetVideoSource(int nInput);

    void SetBrightness(BYTE Brightness);
    void SetHue(BYTE Hue);
    void SetContrast(BYTE Contrast);
    void SetSaturationU(BYTE SaturationU);
    void SetSaturationV(BYTE SaturationV);

    void SetGeoSize(int nInput, eVideoFormat TVFormat, long& CurrentX, long& CurrentY, int VDelay, int HDelay, BOOL IsProgressive);

    BOOL IsVideoPresent();
    DWORD GetRISCPos();
    void ResetHardware();

    eCT2388xCardId AutoDetectCardType();

    LPCSTR GetInputName(int nVideoSource);
    LPCSTR GetCardName(eCT2388xCardId CardId);
    int GetNumInputs();
    BOOL IsInputATuner(int nInput);
    LPCSTR GetTunerType();
    eTunerId AutoDetectTuner(eCT2388xCardId CardId);
    BOOL InitTuner(eTunerId tunerId);
    void SetRISCStartAddress(DWORD RiscBasePhysical);

    static BOOL APIENTRY ChipSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

    // I2C stuff
    void SetSDA(bool value);
    void SetSCL(bool value);
    bool GetSDA();
    bool GetSCL();
    void Sleep();
    ITuner* GetTuner() const;

    void DumpChipStatus();

private:
    ULONG GetTickCount();
    DWORD m_I2CSleepCycle;
    DWORD m_I2CRegister;
    bool m_I2CInitialized;
    void InitializeI2C();

private:
    BOOL IsCCIRSource(int nInput);
    const TCardType* GetCardSetup();
    void SetPLL(double output_freq, int pre_scaler, BOOL integer_only);
    void SetSampleRateConverter(double PLLFreq);

    void StandardInputSelect(int nInput);

    void StandardSetFormat(int nInput, eVideoFormat TVFormat, BOOL IsProgressive);

    void SetAnalogBrightness(BYTE Brightness);
    void SetAnalogHue(BYTE Hue);
    void SetAnalogContrast(BYTE Contrast);
    void SetAnalogSaturationU(BYTE SaturationU);
    void SetAnalogSaturationV(BYTE SaturationV);

    void SetVIPBrightness(BYTE Brightness);
    void SetVIPContrast(BYTE Contrast);
    void SetVIPSaturation(BYTE Saturation);

	void InitH3D();
	void H3DInputSelect(int nInput);
	void H3DSetFormat(int nInput, eVideoFormat TVFormat, BOOL IsProgressive);
    void SetH3DBrightness(BYTE Brightness);
    void SetH3DHue(BYTE Hue);
    void SetH3DContrast(BYTE Contrast);
    void SetH3DSaturationU(BYTE SaturationU);
    void SetH3DSaturationV(BYTE SaturationV);

private:
    eCT2388xCardId m_CardType;

    CI2CBus*        m_I2CBus;
    ITuner*         m_Tuner;
	CSAA7118*       m_SAA7118;

    BOOL            m_RiscInSram;
    BOOL            m_RISCIsRunning;
    DWORD           m_RiscBasePhysical;
    CAudioDecoder*  m_AudioDecoder;
    IAudioControls* m_AudioControls;
    char            m_TunerType[32];
    int             m_CurrentInput;

private:
    static const TCardType m_TVCards[CT2388xCARD_LASTONE];
    static const TAutoDectect m_AutoDectect[];
};


#endif