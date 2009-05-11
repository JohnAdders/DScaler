/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file cx2388xcard.h  cx2388xcard Header file
 */

#ifdef WANT_CX2388X_SUPPORT

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
#include "SoundChannel.h"

#include "TDA9887.h"
#include "HierarchicalConfigParser.h"
#include "ParsingCommon.h"


#define CX_INPUTS_PER_CARD        9
#define CX_AUTODETECT_ID_PER_CARD 3

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

private:

    typedef struct
    {
        DWORD dwRegister;
        DWORD dwValue;
    } TAudioRegList;

    enum eInputType         // Different types of input currently supported
    {
        INPUTTYPE_TUNER,      // standard composite input
        INPUTTYPE_COMPOSITE,  // standard composite input
        INPUTTYPE_SVIDEO,     // standard s-video input
        INPUTTYPE_CCIR,       // Digital CCIR656 input on the GPIO pins
        INPUTTYPE_COLOURBARS, // Shows Colour Bars for testing
        INPUTTYPE_FINAL,      // Stores the state the cards should be put into at the end
    };

    typedef struct
    {
        DWORD GPIO_0;
        DWORD GPIO_1;
        DWORD GPIO_2;
        DWORD GPIO_3;
    } TGPIOSet;

    typedef struct             // Defines each input on a card
    {
        TCHAR      szName[64]; // Name of the input
        eInputType InputType;  // Type of the input
        BYTE       MuxSelect;  // Which mux on the card is to be used
        TGPIOSet   GPIOSet;    // Which GPIO's on the card is to be used
    } TInputType;

    enum eCardMode
    {
        MODE_STANDARD = 0,
        MODE_H3D,
    };

    typedef struct            // Defines the specific settings for a given card
    {
        TCHAR       szName[128];
        eCardMode   CardMode;
        int         NumInputs;
        TInputType  Inputs[CX_INPUTS_PER_CARD];
        eTunerId    TunerId;
        DWORD       AutoDetectId[CX_AUTODETECT_ID_PER_CARD];
        BOOL        bUseTDA9887;
    } TCardType;


    class CCardTypeEx :    public TCardType                          // Same as TCardType but required to store dynamic values.
    {
    public:
        std::vector<TTDA9887FormatModes> tda9887Modes;            // Card specific TDA9887 modes for various video formats.
        CCardTypeEx() {    };
        CCardTypeEx(const TCardType& card) : TCardType(card) { }; // TCardType to CCardTypeEx implicit conversion constructor.
    };

    typedef struct
    {
        std::vector<CCardTypeEx>*   pCardList;
        CCardTypeEx*                pCurrentCard;
        size_t                      nGoodCards;
        HCParser::CHCParser*        pHCParser;
        TParseTunerInfo             tunerInfo;
        TParseUseTDA9887Info        useTDA9887Info;
    } TParseCardInfo;

public:
    CCX2388xCard(SmartPtr<CHardwareDriver> pDriver);
    ~CCX2388xCard();

    void SetCardType(int CardType);
    eCX2388xCardId GetCardType();
    eCX2388xCardId AutoDetectCardType();
    int    GetMaxCards();
    tstring GetCardName(eCX2388xCardId CardId);
    int    GetCardByName(LPCTSTR cardName);

    void HandleTimerMessages(int TimerId);

    void StartCapture(BOOL bCaptureVBI);
    void StopCapture();

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
    /// Turn on the card and set state to off
    void ResetChip();

    tstring   GetInputName(int nVideoSource);
    tstring   GetTunerType();

    int      GetNumInputs();
    int      GetFinalInputNumber();
    BOOL     IsInputATuner(int nInput);
    eTunerId AutoDetectTuner(eCX2388xCardId CardId);
    BOOL     InitTuner(eTunerId tunerId);
    BOOL     IsThisCardH3D(eCX2388xCardId CardId);

    void SetRISCStartAddress(DWORD RiscBasePhysical);
    void SetRISCStartAddressVBI(DWORD RiscBasePhysical);
    void SetFLIFilmDetect(BOOL FLIFilmDetect);
    void SetLumaAGC(BOOL LumaAGC);
    void SetChromaAGC(BOOL ChromaAGC);
    void SetFastSubcarrierLock(BOOL LockFast);
    void SetWhiteCrushEnable(BOOL WhiteCrush);
    void SetVerticalSyncDetection(BOOL SyncDetection);
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

    static BOOL APIENTRY ChipSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    // I2C stuff
    void SetSDA(BOOL value);
    void SetSCL(BOOL value);
    BOOL GetSDA();
    BOOL GetSCL();
    void Sleep();
    void I2CLock();
    void I2CUnlock();
    SmartPtr<ITuner> GetTuner() const;

    // Audio
    void AudioInit(int nInput, eVideoFormat TVFormat, eCX2388xAudioStandard Standard, eCX2388xStereoType StereoType);
    void SetAudioMute();
    void SetAudioUnMute(WORD nVolume);
    void SetAudioVolume(WORD nVolume);
    void SetAudioBalance(WORD nBalance);
    eCX2388xAudioStandard GetCurrentAudioStandard();
    eCX2388xStereoType GetCurrentStereoType();

    void ShowRegisterSettingsDialog(HINSTANCE hCX2388xResourceInst);
    void DumpChipStatus(const TCHAR* CardName);
    HMENU GetCardSpecificMenu();

protected:
    void ManageMyState();
    /// Card does support ACPI
    BOOL SupportsACPI() {return TRUE;};

    // I2C stuff
private:
    ULONG GetTickCount();
    DWORD m_I2CSleepCycle;
    DWORD m_I2CRegister;
    BOOL  m_I2CInitialized;
    void  InitializeI2C();

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

    // H3D
    void InitH3D();
    void H3DSetFormat(int nInput, eVideoFormat TVFormat, BOOL IsProgressive);
    void H3DInputSelect(int nInput);
    void SetH3DContrastBrightness(BYTE Contrast, BYTE Brightness);
    void SetH3DHue(BYTE Hue);
    void SetH3DSaturationU(BYTE SaturationU);
    void SetH3DSaturationV(BYTE SaturationV);
    BOOL IsCurCardH3D();

    // Audio
    void AudioInitDMA();
    void AudioInitBTSC(eVideoFormat TVFormat, eCX2388xStereoType StereoType);
    void AudioInitBTSCSAP(eVideoFormat TVFormat, eCX2388xStereoType StereoType);
    void AudioInitEIAJ(eVideoFormat TVFormat, eCX2388xStereoType StereoType);
    void AudioInitA2(eVideoFormat TVFormat, eCX2388xStereoType StereoType);
    void AudioInitFM(eVideoFormat TVFormat, eCX2388xStereoType StereoType);
    void AudioInitNICAM(eVideoFormat TVFormat, eCX2388xStereoType StereoType);
    void SetAudioRegisters(const TAudioRegList* pAudioList);

    static BOOL APIENTRY RegisterEditProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
    eCX2388xCardId  m_CardType;

    SmartPtr<CI2CBus>        m_I2CBus;
    SmartPtr<II2CTuner>      m_Tuner;
    SmartPtr<CSAA7118>       m_SAA7118;

    BOOL            m_RISCIsRunning;
    SmartPtr<CAudioDecoder>  m_AudioDecoder;
    SmartPtr<CAudioControls> m_AudioControls;
    tstring     m_TunerType;
    int             m_CurrentInput;
    DWORD           m_FilterDefault;
    DWORD           m_2HCombDefault;

    // Audio
    eCX2388xAudioStandard m_CurrentAudioStandard;
    eCX2388xStereoType    m_CurrentStereoType;

private:
    // Audio
    static const TAudioRegList m_RegList_BTSC[];
    static const TAudioRegList m_RegList_BTSC_SAP[];
    static const TAudioRegList m_RegList_EIAJ[];
    static const TAudioRegList m_RegList_Nicam_Common[];
    static const TAudioRegList m_RegList_Nicam_Deemph1[];
    static const TAudioRegList m_RegList_Nicam_Deemph2[];
    static const TAudioRegList m_RegList_AM_L[];
    static const TAudioRegList m_RegList_A1_I[];
    static const TAudioRegList m_RegList_A2_BGDKM_Common[];
    static const TAudioRegList m_RegList_A2_BG[];
    static const TAudioRegList m_RegList_A2_DK[];
    static const TAudioRegList m_RegList_A2_M[];
    static const TAudioRegList m_RegList_TV_Deemph50[];
    static const TAudioRegList m_RegList_TV_Deemph75[];
    static const TAudioRegList m_RegList_FM_Deemph50[];
    static const TAudioRegList m_RegList_FM_Deemph75[];

// Parsing
public:

    // Reads CX2388 cards out of an INI file for all instances    of
    // CCX2388xCard    to use.     If    this is    not    called,    CCX2388x will
    // have    no cards to    work with.    A return value of FALSE    indicates
    // a parsing error occured and the user    chose to EXIT the program.
    static BOOL    InitializeCX2388xCardList();

    // This    function makes sure    there is at    least the "unknown"    card
    // in the card list.  It is    called by InitializeCX2388xCardList()
    // and other functions so does not need    to be called directly.
    static void    InitializeCX2388xUnknownCard();

    static void ReadCardInputInfoProc(int, const HCParser::CParseTag*, unsigned char, const HCParser::CParseValue*, void*);
    static void ReadCardInputProc(int, const HCParser::CParseTag*, unsigned char, const HCParser::CParseValue*, void*);
    static void ReadCardUseTDA9887Proc(int, const HCParser::CParseTag*, unsigned char, const HCParser::CParseValue*, void*);
    static void ReadCardDefaultTunerProc(int, const HCParser::CParseTag*, unsigned char, const HCParser::CParseValue*, void*);
    static void ReadCardInfoProc(int, const HCParser::CParseTag*, unsigned char, const HCParser::CParseValue*, void*);
    static void ReadCardAutoDetectIDProc(int, const HCParser::CParseTag*, unsigned char, const HCParser::CParseValue*, void*);
    static void ReadCardProc(int, const HCParser::CParseTag*, unsigned char, const HCParser::CParseValue*, void*);
    static BOOL    APIENTRY ParseErrorProc(HWND hDlg, UINT    message, WPARAM wParam, LPARAM lParam);

private:
    ///    Holds the list of all cards
    static const TCardType            m_CX2388xUnknownCard;
    static std::vector<CCardTypeEx>    m_CX2388xCards;

    static const HCParser::CParseConstant k_parseInputTypeConstants[];
    static const HCParser::CParseConstant k_parseCardModeConstants[];

    static const HCParser::CParseTag k_parseCardGPIOSet[];
    static const HCParser::CParseTag k_parseCardInput[];
    static const HCParser::CParseTag k_parseCardAutoDetectID[];
    static const HCParser::CParseTag k_parseCard[];
    static const HCParser::CParseTag k_parseCardList[];
};

#endif

#endif // WANT_CX2388X_SUPPORT
