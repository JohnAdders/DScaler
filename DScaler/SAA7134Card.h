/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Card.h,v 1.43 2004-11-28 20:46:35 atnak Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Atsushi Nakagawa.  All rights reserved.
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
// This software was based on v4l2 device driver for philips
// saa7134 based TV cards.  Those portions are
// Copyright (c) 2001,02 Gerd Knorr <kraxel@bytesex.org> [SuSE Labs]
//
// This software was based on BT848Card.h.  Those portions are
// Copyright (c) 2001 John Adcock.
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 09 Sep 2002   Atsushi Nakagawa      Initial Release
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.42  2004/11/27 19:11:44  atnak
// Added settings specific tor TDA9887 to the card list.
//
// Revision 1.41  2004/11/20 14:20:09  atnak
// Changed the card list to an ini file.
//
// Revision 1.40  2004/02/24 04:17:58  atnak
// Added a help comment
//
// Revision 1.39  2004/02/18 06:39:47  atnak
// Changed Setup Card / Tuner so that only cards of the same device are
// shown in the card list.
// Added new card Chronos Video Shuttle II (saa7134 version)
//
// Revision 1.38  2004/02/14 04:03:44  atnak
// Put GPIO settings and AutoDetect IDs into the main card definition
// to remove the need for extra tables and custom functions.
// Added card Medion Philips 7134 Chipset
//
// Revision 1.37  2003/10/27 16:22:57  adcockj
// Added preliminary support for PMS PDI Deluxe card
//
// Revision 1.36  2003/10/27 10:39:53  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.35  2003/08/14 08:25:17  atnak
// Fix to mute lining and audio channel clush
//
// Revision 1.34  2003/08/12 06:46:01  atnak
// Fix for initial muting of SAA7130 cards
//
// Revision 1.33  2003/06/27 08:05:41  atnak
// Added AOPEN VA1000 Lite2
//
// Revision 1.32  2003/04/17 09:17:47  atnak
// Added V-Gear MyTV SAP PK
//
// Revision 1.31  2003/04/16 15:12:17  atnak
// Comments for audio clock values
//
// Revision 1.30  2003/02/12 22:09:47  atnak
// Added M-TV002
//
// Revision 1.29  2002/12/24 08:22:14  atnak
// Added Prime 7133 card
//
// Revision 1.28  2002/12/14 00:29:35  atnak
// Added Manli M-TV001 card
//
// Revision 1.27  2002/12/10 11:05:46  atnak
// Fixed FlyVideo 3000 audio for external inputs
//
// Revision 1.26  2002/11/10 09:30:57  atnak
// Added Chroma only comb filter mode for SECAM
//
// Revision 1.25  2002/11/10 05:11:24  atnak
// Added adjustable audio input level
//
// Revision 1.24  2002/11/08 06:15:34  atnak
// Added state saving
//
// Revision 1.23  2002/11/07 20:33:17  adcockj
// Promoted ACPI functions so that state management works properly
//
// Revision 1.22  2002/11/07 18:54:21  atnak
// Redid getting next field -- fixes some issues
//
// Revision 1.21  2002/11/07 13:37:43  adcockj
// Added State restoration code to PCICard
// Functionality disabled prior to testing and not done for SAA7134
//
// Revision 1.20  2002/10/30 04:35:47  atnak
// Added attempt to reduce driver conflict instability
//
// Revision 1.19  2002/10/28 11:10:15  atnak
// Various changes and revamp to settings
//
// Revision 1.18  2002/10/26 15:37:57  adcockj
// Made ITuner more abstract by removing inheritance from CI2CDevice
// New class II2CTuner created for tuners that are controled by I2C
//
// Revision 1.17  2002/10/26 04:41:44  atnak
// Clean up + added auto card detection
//
// Revision 1.16  2002/10/23 17:05:20  atnak
// Added variable VBI sample rate scaling
//
// Revision 1.15  2002/10/20 07:41:04  atnak
// custom audio standard setup + etc
//
// Revision 1.14  2002/10/16 11:40:10  atnak
// Added KWORLD KW-TV713XRF card.  Thanks "b"
//
// Revision 1.13  2002/10/12 20:01:52  atnak
// added some automatic error recovery
//
// Revision 1.12  2002/10/08 20:35:39  atnak
// whitepeak, colorpeak, comb filter UI options
//
// Revision 1.11  2002/10/08 19:35:45  atnak
// various fixes, tweaks, cleanups
//
// Revision 1.10  2002/10/08 12:24:46  atnak
// added various functions to configure carriers
//
// Revision 1.9  2002/10/06 12:14:52  atnak
// cleaned up SetPageTable(...)
//
// Revision 1.8  2002/10/04 23:40:46  atnak
// proper support for audio channels mono,stereo,lang1,lang2 added
//
// Revision 1.7  2002/10/04 13:24:46  atnak
// Audio mux select through GPIO added (for 7130 cards)
//
// Revision 1.6  2002/10/03 23:36:23  atnak
// Various changes (major): VideoStandard, AudioStandard, CSAA7134Common, cleanups, tweaks etc,
//
// Revision 1.5  2002/09/16 17:51:58  atnak
// Added controls for L/R/Nicam audio volume
//
// Revision 1.4  2002/09/14 19:40:48  atnak
// various changes
//
// Revision 1.3  2002/09/10 12:14:35  atnak
// Some changes to eAudioStandard stuff
//
//
//////////////////////////////////////////////////////////////////////////////

/** 
 * @file saa7134card.h saa7134card Header file
 */
 
#ifndef __SAA7134CARD_H___
#define __SAA7134CARD_H___

#include "PCICard.h"
#include "TVFormats.h"
#include "SAA7134_Defines.h"
#include "SAA7134Common.h"
#include "SAA7134I2CInterface.h"
#include "TDA9887.h"
#include "HierarchicalConfigParser.h"
#include "ParsingCommon.h"

#include "ITuner.h"
//#include "AudioDecoder.h"
#include "IAudioControls.h"

#define SA_INPUTS_PER_CARD 7
#define SA_AUTODETECT_ID_PER_CARD 3


/** A Generic saa7134 based capture card
    The card can cope with the standard inputs,
    a tuner and one of the supported sound chips.
    Control is given over most of the supported chip 
    functions.
*/
class CSAA7134Card : public CPCICard,
                     public ISAA7134I2CInterface,
                     public CSAA7134Common
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
        /// When the card doesn't have internal mute
        INPUTTYPE_MUTE,
        /// Stores the state the cards should be put into at the end
        INPUTTYPE_FINAL,
    };

    /// SAA7134's video input pins
    /// RegSpy: SAA7134_ANALOG_IN_CTRL1
    /// RegSpy: 0000nnnn
    /// RegSpy: nnnn: 5 = reserved
    /// RegSpy: nnnn: 6 or 8 = 0 + s-video
    /// RegSpy: nnnn: 7 or 9 = 1 + s-video
    enum eVideoInputSource
    {
        VIDEOINPUTSOURCE_NONE = -1,     // reserved for radio
        VIDEOINPUTSOURCE_PIN0 = 0,
        VIDEOINPUTSOURCE_PIN1,
        VIDEOINPUTSOURCE_PIN2,
        VIDEOINPUTSOURCE_PIN3,
        VIDEOINPUTSOURCE_PIN4,
    };

    /// Possible clock crystals a card could have
    enum eAudioCrystal
    {
        AUDIOCRYSTAL_NONE = 0,          // only on saa7130
        AUDIOCRYSTAL_32110Hz,           // 0x187DE7
        AUDIOCRYSTAL_24576Hz,           // 0x200000
    };

    /// Defines each input on a card
    typedef struct
    {
        /// Name of the input
        char szName[64];
        /// Type of the input
        eInputType InputType;
        /// Which video pin on the card is to be used
        eVideoInputSource VideoInputPin;
        /// Which line on the card is to be default
        eAudioInputSource AudioLineSelect;
        DWORD dwGPIOStatusMask;
        DWORD dwGPIOStatusBits;
    } TInputType;

    /// Defines the specific settings for a given card
    typedef struct
    {
        char szName[128];
        WORD DeviceId;
        int NumInputs;
        TInputType Inputs[SA_INPUTS_PER_CARD];
        eTunerId TunerId;
        /// The type of clock crystal the card has
        eAudioCrystal AudioCrystal;
        DWORD dwGPIOMode;
        DWORD AutoDetectId[SA_AUTODETECT_ID_PER_CARD];
        BOOL bUseTDA9887;
    } TCardType;

    /// Same as TCardType but required to store dynamic values.
    class CCardTypeEx : public TCardType
    {
    public:
        /// Card specific TDA9887 modes for various video formats.
        std::vector<TTDA9887FormatModes> tda9887Modes;

        CCardTypeEx() { };
        /// TCardType to CCardTypeEx implicit conversion constructor.
        CCardTypeEx(const TCardType& card) : TCardType(card) { };
    };

    /// used to store the ID for autodetection
    typedef struct
    {
        WORD DeviceId;
        WORD SubSystemVendorId;
        WORD SubSystemId;
        eSAA7134CardId CardId;
    } TAutoDetectSAA7134;

    /// used to store temporary information used while parsing
    /// the SAA713x card list
    typedef struct
    {
        // List of all cards parsed and last item for currently parsing.
        std::vector<CCardTypeEx>*   pCardList;
        // Pointer to the last item in pCardList when a card is being parsed.
        CCardTypeEx*                pCurrentCard;
        // Number of cards successfully parsed so far.
        size_t                      nGoodCards;
        // Pointer to the HCParser instance doing the parsing.
        HCParser::CHCParser*        pHCParser;
        // Used by ParsingCommon's tuner parser for temporary data.
        TParseTunerInfo             tunerInfo;
        // Used by ParsingCommon's useTDA9887 parser for temporary data.
        TParseUseTDA9887Info        useTDA9887Info;
    } TParseCardInfo;


public:
    // Reads SAA713x cards out of an INI file for all instances of
    // CSAA7134Card to use.  If this is not called, CSAA7134 will
    // have no cards to work with.  A return value of FALSE indicates
    // a parsing error occured and the user chose to EXIT the program.
    static BOOL InitializeSAA713xCardList();
    // This function makes sure there is at least the "unknown" card
    // in the card list.  It is called by InitializeSAA713xCardList()
    // and other functions so does not need to be called directly.
    static void InitializeSAA713xUnknownCard();


public:
    CSAA7134Card(CHardwareDriver* pDriver);
    ~CSAA7134Card();
    

    /** General card setup
     */
    int     GetMaxCards();
    int     GetCardByName(LPCSTR cardName);
    LPCSTR  GetCardName(eSAA7134CardId CardId);
    WORD    GetCardDeviceId(eSAA7134CardId CardId);

    void            SetCardType(int CardType);
    eSAA7134CardId  GetCardType();

    eTunerId        AutoDetectTuner(eSAA7134CardId CardId);
    eSAA7134CardId  AutoDetectCardType();

    LPCSTR  GetChipType();
    LPCSTR  GetTunerType();
    ITuner* GetTuner() const;

    int     GetNumInputs();
    LPCSTR  GetInputName(int nVideoSource);
    BOOL    IsInputATuner(int nInput);

    int     GetFinalInputNumber();

    /** Tuner
     */
    BOOL InitTuner(eTunerId tunerId);


    /** General capture setup
     */
    void StopCapture();
    void StartCapture(BOOL bCaptureVBI);

    void PrepareCard();
    void ResetHardware();

    void SetDMA(eRegionID RegionID, BOOL bState);
    BOOL GetDMA(eRegionID RegionID);

    void SetPageTable(eRegionID RegionID, DWORD pPhysical, DWORD nPages);
    void SetBaseOffsets(eRegionID RegionID, DWORD dwEvenOffset, DWORD dwOddOffset, DWORD dwPitch);
    void SetBSwapAndWSwap(eRegionID RegionID, BOOL bBSwap, BOOL bWSwap);

    BOOL GetProcessingRegion(eRegionID& RegionID, BOOL& bIsFieldOdd);
    BOOL GetProcessingFieldID(TFieldID* pFieldID);
    BOOL GetIRQEventRegion(eRegionID& RegionID, BOOL& bIsFieldOdd);


    /** Video
     */
    void SetVideoSource(int nInput);

    void SetVideoStandard(eVideoStandard VideoStandard, long& VBILines, long& VideoWidth, long& VideoHeight, long HDelayShift, long VDelayShift, long VBIUpscaleDivisor);
    void SetGeometry(WORD ScaleWidth, WORD ScaleHeight, long HDelayShift, long VDelayShift);
    void SetVBIGeometry(WORD UpscaleDivisor);

    void SetBrightness(BYTE Brightness);
    BYTE GetBrightness();
    void SetHue(BYTE Hue);
    BYTE GetHue();
    void SetContrast(BYTE Contrast);
    BYTE GetContrast();
    void SetSaturation(BYTE Saturation);
    BYTE GetSaturation();

    void SetWhitePeak(BOOL WhitePeak);
    BOOL GetWhitePeak();
    void SetColorPeak(BOOL ColorPeak);
    BOOL GetColorPeak();

    void SetCombFilter(eCombFilter CombFilter);
    void SetVideoMirror(BOOL bMirror);

    void SetAutomaticGainControl(BOOL bAGC);
    void SetGainControl(WORD GainControl);

    /* Video sync
     */
    void SetHPLLMode(eHPLLMode HPLLMode);
    void SetVSyncRecovery(eVSyncRecovery VSyncRecovery);

    /** Miscellaneous video
     */
    long GetMinimumVDelay();
    long GetMinimumVDelayWithVBI();

    BOOL IsVideoPresent();
    BOOL Is25fpsSignalDetected();
    BOOL IsInterlacedSignalDetected();

    /** Audio
     */
    void InitAudio();

    void SetAudioSource(eAudioInputSource InputSource);
    void SetAudioStandard(eAudioStandard AudioStandard);
    void SetAudioChannel(eAudioChannel AudioChannel);

    void _SetIOSelectOCS(eAudioInputSource InputSource, BOOL bStereoExternal);

    int GetInputAudioLine(int nInput);
    LPCSTR GetAudioStandardName(eAudioStandard AudioStandard);
    eAudioChannel GetAudioChannel();

    void SetAudioMute();
    void SetAudioUnMute();
    void SetAudioVolume(BYTE nVolume);
    void SetAudioBalance(WORD nBalance);
    void SetAudioBass(WORD nBass);
    void SetAudioTreble(WORD nTreble);

    void SetAudioCarrier1Freq(DWORD Carrier);
    void SetAudioCarrier2Freq(DWORD Carrier);
    void SetAudioCarrier1Mode(eAudioCarrierMode Mode);
    void SetAudioCarrier2Mode(eAudioCarrierMode Mode);

    void SetCh1FMDeemphasis(eAudioFMDeemphasis FMDeemphasis);
    void SetCh2FMDeemphasis(eAudioFMDeemphasis FMDeemphasis);

    void SetAudioFMDematrix(eAudioFMDematrix FMDematrix);
    void SetFilterBandwidth(eAudioFilterBandwidth FilterBandwidth);

    void SetAudioSampleRate(eAudioSampleRate SampleRate);
    void SetAutomaticVolume(eAutomaticVolume AVL);

    void SetAudioLine1Voltage(eAudioLineVoltage LineVoltage);
    void SetAudioLine2Voltage(eAudioLineVoltage LineVoltage);

    /** Miscellaneous audio
     */
    void SetAudioLockToVideo(BOOL bLockAudio);
    BOOL IsAudioChannelDetected(eAudioChannel AudioChannel);
    void GetAudioDecoderStatus(char* pBuffer, WORD nBufferSize);


    /** I2C
     */
    BYTE GetI2CStatus();
    void SetI2CStatus(BYTE Status);
    void SetI2CCommand(BYTE Command);
    void SetI2CData(BYTE Data);
    BYTE GetI2CData();


    /** Miscellaneous hardware
     */
    void StatGPIO();
    void EnableI2SAudioOutput(WORD wRate);
    void EnableCCIR656VideoOut();


    /** Windows messages
     */
    void HandleTimerMessages(int TimerId);
    static BOOL APIENTRY ChipSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);


    /** Card list parsing
     */
    static void ReadCardInputInfoProc(int, const HCParser::ParseTag*, unsigned char, const char*, void*);
    static void ReadCardInputProc(int, const HCParser::ParseTag*, unsigned char, const char*, void*);
    static void ReadCardUseTDA9887Proc(int, const HCParser::ParseTag*, unsigned char, const char*, void*);
    static void ReadCardDefaultTunerProc(int, const HCParser::ParseTag*, unsigned char, const char*, void*);
    static void ReadCardInfoProc(int, const HCParser::ParseTag*, unsigned char, const char*, void*);
    static void ReadCardAutoDetectIDProc(int, const HCParser::ParseTag*, unsigned char, const char*, void*);
    static void ReadCardProc(int, const HCParser::ParseTag*, unsigned char, const char*, void*);
    static BOOL APIENTRY ParseErrorProc(HWND hDlg, UINT message, UINT wParam, LPARAM lParam);


    /** DEBUG
     */
    void CheckRegisters(DWORD* AOdd, DWORD* AEven, DWORD* BOdd, DWORD* BEven);
    void DumpRegisters();

    BYTE DirectGetByte(DWORD dwAddress);
    void DirectSetBit(DWORD dwAddress, int nBit, BOOL bSet);


protected:
    void SetTypicalSettings();
    void SetupTasks();

    void SetTaskGeometry(eTaskID TaskID, WORD Width, WORD Height, WORD HDelay, WORD VDelay, WORD ScaleWidth, WORD ScaleHeight);
    void SetTaskVBIGeometry(eTaskID TaskID, WORD HStart, WORD HStop, WORD VStart, WORD VStop, WORD UpscaleDivisor);

    void ResetHPrescale(eTaskID TaskID);
    void SetHPrescale(eTaskID TaskID, WORD wSourceSize, WORD wScaleSize);

    BOOL IsVBIActive();
    void CheckVBIAndVideoOverlap(eTaskID TaskID);

    void VerifyMemorySize(eRegionID RegionID);
    WORD CalculateLinesAvailable(eRegionID RegionID, WORD wBytePerLine);

    void UpdateAudioClocksPerField(eVideoStandard VideoStandard);
    void CheckScalerError(BOOL bErrorOccurred, WORD ScalerStatus);

    int RegionID2Channel(eRegionID RegionID);
    BYTE TaskID2TaskMask(eTaskID TaskID);

    void ManageMyState();
    /// Card does support ACPI
    BOOL SupportsACPI() {return TRUE;};
    /// don't know how to reset
    void ResetChip() {;};


private:
    BOOL IsCCIRSource(int nInput);
    const TCardType* GetCardSetup();

    void StandardSAA7134InputSelect(int nInput);


private:
    /// Holds the list of all cards
    static const TCardType          m_SAA7134UnknownCard;
    static std::vector<CCardTypeEx> m_SAA713xCards;

    static const HCParser::ParseConstant k_parseSAA713xInputAudioPinConstants[];
    static const HCParser::ParseConstant k_parseSAA713xInputTypeConstants[];
    static const HCParser::ParseConstant k_parseSAA713xAudioCrystalConstants[];
    static const HCParser::ParseConstant k_parseSAA713xDefaultTunerConstants[];

    static const HCParser::ParseTag k_parseSAA713xCardInputGPIOSet[];
    static const HCParser::ParseTag k_parseSAA713xCardInput[];
    static const HCParser::ParseTag k_parseSAA713xCardAutoDetectID[];
    static const HCParser::ParseTag k_parseSAA713xCard[];
    static const HCParser::ParseTag k_parseSAA713xCardList[];

    eSAA7134CardId      m_CardType;
    char                m_TunerType[32];

    CI2CBus*            m_I2CBus;
    II2CTuner*          m_Tuner;

    /// Stores amount of memory assigned to DMA
    DWORD               m_DMAChannelMemorySize[7];
    /// Stores regions prepared for DMA
    BYTE                m_PreparedRegions;

    DWORD               m_LastTriggerError;

    eVideoStandard      m_VideoStandard;

    eAudioInputSource   m_AudioInputSource;
    BOOL                m_bAudioLineReservedForMute;
    BOOL                m_bStereoExternalLines;
};


#endif

