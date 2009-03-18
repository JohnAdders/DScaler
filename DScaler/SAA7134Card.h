/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Atsushi Nakagawa.     All rights    reserved.
/////////////////////////////////////////////////////////////////////////////
//
//    This file is subject to    the    terms of the GNU General Public    License    as
//    published by the Free Software Foundation.    A copy of this license is
//    included with this software    distribution in    the    file COPYING.  If you
//    do not have    a copy,    you    may    obtain a copy by writing to    the    Free
//    Software Foundation, 675 Mass Ave, Cambridge, MA 02139,    USA.
//
//    This software is distributed in    the    hope that it will be useful,
//    but    WITHOUT    ANY    WARRANTY; without even the implied warranty    of
//    MERCHANTABILITY    or FITNESS FOR A PARTICULAR    PURPOSE.  See the
//    GNU    General    Public License for more    details
/////////////////////////////////////////////////////////////////////////////
//
// This    software was based on v4l2 device driver for philips
// saa7134 based TV    cards.    Those portions are
// Copyright (c) 2001,02 Gerd Knorr    <kraxel@bytesex.org> [SuSE Labs]
//
// This    software was based on BT848Card.h.    Those portions are
// Copyright (c) 2001 John Adcock.
//
/////////////////////////////////////////////////////////////////////////////

/**    
 * @file saa7134card.h saa7134card Header file
 */
 
#ifdef WANT_SAA713X_SUPPORT

#ifndef    __SAA7134CARD_H___
#define    __SAA7134CARD_H___

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

#define    SA_INPUTS_PER_CARD 7
#define    SA_AUTODETECT_ID_PER_CARD 3


/**    A Generic saa7134 based    capture    card
    The    card can cope with the standard    inputs,
    a tuner    and    one    of the supported sound chips.
    Control    is given over most of the supported    chip 
    functions.
*/
class CSAA7134Card : public    CPCICard,
                     public    ISAA7134I2CInterface,
                     public    CSAA7134Common
{

private:
    
    ///    Different types    of input currently supported
    enum eInputType
    {
        ///    standard composite input
        INPUTTYPE_COMPOSITE,
        ///    standard s-video input
        INPUTTYPE_SVIDEO,
        ///    standard analogue tuner    input composite
        INPUTTYPE_TUNER,
        ///    Digital    CCIR656    input on the GPIO pins
        INPUTTYPE_CCIR,
        ///    Radio input    so no video
        INPUTTYPE_RADIO,
        ///    When the card doesn't have internal    mute
        INPUTTYPE_MUTE,
        ///    Stores the state the cards should be put into at the end
        INPUTTYPE_FINAL,
    };

    ///    SAA7134's video    input pins
    ///    RegSpy:    SAA7134_ANALOG_IN_CTRL1
    ///    RegSpy:    0000nnnn
    ///    RegSpy:    nnnn: 5    = reserved
    ///    RegSpy:    nnnn: 6    or 8 = 0 + s-video
    ///    RegSpy:    nnnn: 7    or 9 = 1 + s-video
    enum eVideoInputSource
    {
        VIDEOINPUTSOURCE_NONE =    -1,        // reserved    for    radio
        VIDEOINPUTSOURCE_PIN0 =    0,
        VIDEOINPUTSOURCE_PIN1,
        VIDEOINPUTSOURCE_PIN2,
        VIDEOINPUTSOURCE_PIN3,
        VIDEOINPUTSOURCE_PIN4,
    };

    ///    Possible clock crystals    a card could have
    enum eAudioCrystal
    {
        AUDIOCRYSTAL_NONE =    0,            // only    on saa7130
        AUDIOCRYSTAL_32110kHz,            // 0x187DE7
        AUDIOCRYSTAL_24576kHz,            // 0x200000
    };

    ///    Defines    each input on a    card
    typedef    struct
    {
        ///    Name of    the    input
        char szName[64];
        ///    Type of    the    input
        eInputType InputType;
        ///    Which video    pin    on the card    is to be used
        eVideoInputSource VideoInputPin;
        ///    Which line on the card is to be    default
        eAudioInputSource AudioLineSelect;
        DWORD dwGPIOStatusMask;
        DWORD dwGPIOStatusBits;
    } TInputType;

    ///    Defines    the    specific settings for a    given card
    typedef    struct
    {
        char szName[128];
        WORD DeviceId;
        int    NumInputs;
        TInputType Inputs[SA_INPUTS_PER_CARD];
        eTunerId TunerId;
        ///    The    type of    clock crystal the card has
        eAudioCrystal AudioCrystal;
        DWORD dwGPIOMode;
        DWORD AutoDetectId[SA_AUTODETECT_ID_PER_CARD];
        BOOL bUseTDA9887;
    } TCardType;

    ///    Same as    TCardType but required to store    dynamic    values.
    class CCardTypeEx :    public TCardType
    {
    public:
        ///    Card specific TDA9887 modes    for    various    video formats.
        std::vector<TTDA9887FormatModes> tda9887Modes;

        CCardTypeEx() {    };
        ///    TCardType to CCardTypeEx implicit conversion constructor.
        CCardTypeEx(const TCardType& card) : TCardType(card) { };
    };

    ///    used to    store the ID for autodetection
    typedef    struct
    {
        WORD DeviceId;
        WORD SubSystemVendorId;
        WORD SubSystemId;
        eSAA7134CardId CardId;
    } TAutoDetectSAA7134;

    ///    used to    store temporary    information    used while parsing
    ///    the    SAA713x    card list
    typedef    struct
    {
        // List    of all cards parsed    and    last item for currently    parsing.
        std::vector<CCardTypeEx>*    pCardList;
        // Pointer to the last item    in pCardList when a    card is    being parsed.
        CCardTypeEx*                pCurrentCard;
        // Number of cards successfully    parsed so far.
        size_t                        nGoodCards;
        // Pointer to the HCParser instance    doing the parsing.
        HCParser::CHCParser*        pHCParser;
        // Used    by ParsingCommon's tuner parser    for    temporary data.
        TParseTunerInfo                tunerInfo;
        // Used    by ParsingCommon's useTDA9887 parser for temporary data.
        TParseUseTDA9887Info        useTDA9887Info;
    } TParseCardInfo;


public:
    // Reads SAA713x cards out of an INI file for all instances    of
    // CSAA7134Card    to use.     If    this is    not    called,    CSAA7134 will
    // have    no cards to    work with.    A return value of FALSE    indicates
    // a parsing error occured and the user    chose to EXIT the program.
    static BOOL    InitializeSAA713xCardList();
    // This    function makes sure    there is at    least the "unknown"    card
    // in the card list.  It is    called by InitializeSAA713xCardList()
    // and other functions so does not need    to be called directly.
    static void    InitializeSAA713xUnknownCard();


public:
    CSAA7134Card(CHardwareDriver* pDriver);
    ~CSAA7134Card();
    

    /**    General    card setup
     */
    int        GetMaxCards();
    int        GetCardByName(LPCSTR cardName);
    std::string GetCardName(eSAA7134CardId CardId);
    WORD    GetCardDeviceId(eSAA7134CardId CardId);

    void            SetCardType(int    CardType);
    eSAA7134CardId    GetCardType();

    eTunerId        AutoDetectTuner(eSAA7134CardId CardId);
    eSAA7134CardId    AutoDetectCardType();

    std::string    GetChipType();
    std::string    GetTunerType();
    ITuner*        GetTuner() const;

    int         GetNumInputs();
    std::string GetInputName(int nVideoSource);
    BOOL    IsInputATuner(int nInput);

    int        GetFinalInputNumber();

    /**    Tuner
     */
    BOOL InitTuner(eTunerId    tunerId);


    /**    General    capture    setup
     */
    void StopCapture();
    void StartCapture(BOOL bCaptureVBI);

    void PrepareCard();
    void ResetHardware();

    void SetDMA(eRegionID RegionID,    BOOL bState);
    BOOL GetDMA(eRegionID RegionID);

    void SetPageTable(eRegionID    RegionID, DWORD    pPhysical, DWORD nPages);
    void SetBaseOffsets(eRegionID RegionID,    DWORD dwEvenOffset,    DWORD dwOddOffset, DWORD dwPitch);
    void SetBSwapAndWSwap(eRegionID    RegionID, BOOL bBSwap, BOOL    bWSwap);

    BOOL GetProcessingRegion(eRegionID&    RegionID, BOOL&    bIsFieldOdd);
    BOOL GetProcessingFieldID(TFieldID*    pFieldID);
    BOOL GetIRQEventRegion(eRegionID& RegionID,    BOOL& bIsFieldOdd);


    /**    Video
     */
    void SetVideoSource(int    nInput);

    void SetVideoStandard(eVideoStandard VideoStandard,    long& VBILines,    long& VideoWidth, long&    VideoHeight, long HDelayShift, long    VDelayShift, long VBIUpscaleDivisor);
    void SetGeometry(WORD ScaleWidth, WORD ScaleHeight,    long HDelayShift, long VDelayShift);
    void SetVBIGeometry(WORD UpscaleDivisor);

    void SetBrightness(BYTE    Brightness);
    BYTE GetBrightness();
    void SetHue(BYTE Hue);
    BYTE GetHue();
    void SetContrast(BYTE Contrast);
    BYTE GetContrast();
    void SetSaturation(BYTE    Saturation);
    BYTE GetSaturation();

    void SetWhitePeak(BOOL WhitePeak);
    BOOL GetWhitePeak();
    void SetColorPeak(BOOL ColorPeak);
    BOOL GetColorPeak();

    void SetCombFilter(eCombFilter CombFilter);
    void SetVideoMirror(BOOL bMirror);

    void SetAutomaticGainControl(BOOL bAGC);
    void SetGainControl(WORD GainControl);

    void SetGammaControl(BOOL bGammaControl);
    void SetGammaLevel(WORD bGammaLevel);

    /* Video sync
     */
    void SetHPLLMode(eHPLLMode HPLLMode);
    void SetVSyncRecovery(eVSyncRecovery VSyncRecovery);

    /**    Miscellaneous video
     */
    long GetMinimumVDelay();
    long GetMinimumVDelayWithVBI();

    BOOL IsVideoPresent();
    BOOL Is25fpsSignalDetected();
    BOOL IsInterlacedSignalDetected();

    /**    Audio
     */
    void InitAudio();

    void SetAudioSource(eAudioInputSource InputSource);
    void SetAudioStandard(eAudioStandard AudioStandard);
    void SetAudioChannel(eAudioChannel AudioChannel);

    void _SetIOSelectOCS(eAudioInputSource InputSource,    BOOL bStereoExternal);

    int    GetInputAudioLine(int nInput);
    LPCSTR GetAudioStandardName(eAudioStandard AudioStandard);
    eAudioChannel GetAudioChannel();

    void SetAudioMute();
    void SetAudioUnMute();
    void SetAudioVolume(BYTE nVolume);
    void SetAudioBalance(WORD nBalance);
    void SetAudioBass(WORD nBass);
    void SetAudioTreble(WORD nTreble);

    void SetAudioCarrier1Freq(DWORD    Carrier);
    void SetAudioCarrier2Freq(DWORD    Carrier);
    void SetAudioCarrier1Mode(eAudioCarrierMode    Mode);
    void SetAudioCarrier2Mode(eAudioCarrierMode    Mode);

    void SetCh1FMDeemphasis(eAudioFMDeemphasis FMDeemphasis);
    void SetCh2FMDeemphasis(eAudioFMDeemphasis FMDeemphasis);

    void SetAudioFMDematrix(eAudioFMDematrix FMDematrix);
    void SetFilterBandwidth(eAudioFilterBandwidth FilterBandwidth);

    void SetAudioSampleRate(eAudioSampleRate SampleRate);
    void SetAutomaticVolume(eAutomaticVolume AVL);

    void SetAudioLine1Voltage(eAudioLineVoltage    LineVoltage);
    void SetAudioLine2Voltage(eAudioLineVoltage    LineVoltage);

    /**    SAA7133    audio
     */
    void InitAudio7133();
    BOOL StartDSPAccess7133();
    BOOL WaitDSPAccessState7133(BOOL bRead);
    BOOL WriteDSPData7133(DWORD    registerOffset,    DWORD registerSize,    CBitVector value);
    BOOL ReadDSPData7133(DWORD registerOffset, DWORD registerSize, CBitVector& value);
    void SetAudioStandard7133(eAudioStandard audioStandard);
    void SetAudioCarrier1Freq7133(DWORD    Carrier);
    void SetAudioCarrier2Freq7133(DWORD    Carrier);
    void SetAudioCarrier1Mode7133(eAudioCarrierMode    mode);
    void SetAudioCarrier2Mode7133(eAudioCarrierMode    mode);
    void SetAudioLockToVideo7133(BOOL bLockAudio);
    void UpdateAudioClocksPerField7133(eVideoStandard videoStandard);
    void SetFMDeemphasis7133(eAudioFMDeemphasis    fmDeemphasis);
    void SetAudioFMDematrix7133(eAudioFMDematrix fmDematrix);
    void SetAudioSampleRate7133(eAudioSampleRate sampleRate);
    void SetAudioSource7133(eAudioInputSource inputSource);
    void _SetIOSelectOCS7133(eAudioInputSource inputSource,    BOOL bStereoExternal);
    BOOL _IsAudioChannelDetected7133(eAudioChannel audioChannel);
    void _SetAudioChannel7133(eAudioChannel    audioChannel);
    eAudioChannel GetAudioChannel7133();
    void SetAutomaticVolume7133(eAutomaticVolume avl);


    /**    Miscellaneous audio
     */
    void SetAudioLockToVideo(BOOL bLockAudio);
    BOOL IsAudioChannelDetected(eAudioChannel AudioChannel);
    void GetAudioDecoderStatus(char* pBuffer, WORD nBufferSize);


    /**    I2C
     */
    BYTE GetI2CStatus();
    void SetI2CStatus(BYTE Status);
    void SetI2CCommand(BYTE    Command);
    void SetI2CData(BYTE Data);
    BYTE GetI2CData();
    void I2CLock();
    void I2CUnlock();


    /**    Miscellaneous hardware
     */
    void StatGPIO();
    void EnableI2SAudioOutput(WORD wRate);
    void EnableCCIR656VideoOut();


    /**    Windows    messages
     */
    void HandleTimerMessages(int TimerId);
    static BOOL    APIENTRY ChipSettingProc(HWND hDlg,    UINT message, UINT wParam, LONG    lParam);


    /**    Card list parsing
     */
    static void    ReadCardInputInfoProc(int, const HCParser::CParseTag*, unsigned    char, const    HCParser::CParseValue*,    void*);
    static void    ReadCardInputProc(int, const HCParser::CParseTag*, unsigned    char, const    HCParser::CParseValue*,    void*);
    static void    ReadCardUseTDA9887Proc(int,    const HCParser::CParseTag*,    unsigned char, const HCParser::CParseValue*, void*);
    static void    ReadCardDefaultTunerProc(int, const    HCParser::CParseTag*, unsigned char, const HCParser::CParseValue*, void*);
    static void    ReadCardInfoProc(int, const    HCParser::CParseTag*, unsigned char, const HCParser::CParseValue*, void*);
    static void    ReadCardAutoDetectIDProc(int, const    HCParser::CParseTag*, unsigned char, const HCParser::CParseValue*, void*);
    static void    ReadCardProc(int, const    HCParser::CParseTag*, unsigned char, const HCParser::CParseValue*, void*);
    static BOOL    APIENTRY ParseErrorProc(HWND hDlg, UINT    message, UINT wParam, LPARAM lParam);


    /**    DEBUG
     */
    void CheckRegisters(DWORD* AOdd, DWORD*    AEven, DWORD* BOdd,    DWORD* BEven);
    void DumpRegisters();

    BYTE DirectGetByte(DWORD dwAddress);
    void DirectSetBit(DWORD    dwAddress, int nBit, BOOL bSet);


protected:
    void SetTypicalSettings();
    void SetupTasks();

    void SetTaskGeometry(eTaskID TaskID, WORD Width, WORD Height, WORD HDelay, WORD    VDelay,    WORD ScaleWidth, WORD ScaleHeight);
    void SetTaskVBIGeometry(eTaskID    TaskID,    WORD HStart, WORD HStop, WORD VStart, WORD VStop, WORD UpscaleDivisor);

    void ResetHPrescale(eTaskID    TaskID);
    void SetHPrescale(eTaskID TaskID, WORD wSourceSize,    WORD wScaleSize);

    BOOL IsVBIActive();
    void CheckVBIAndVideoOverlap(eTaskID TaskID);

    void VerifyMemorySize(eRegionID    RegionID);
    WORD CalculateLinesAvailable(eRegionID RegionID, WORD wBytePerLine);

    void UpdateAudioClocksPerField(eVideoStandard VideoStandard);
    void CheckScalerError(BOOL bErrorOccurred, WORD    ScalerStatus);

    int    RegionID2Channel(eRegionID RegionID);
    BYTE TaskID2TaskMask(eTaskID TaskID);

    void ManageMyState();
    ///    Card does support ACPI
    BOOL SupportsACPI()    {return    TRUE;};
    ///    don't know how to reset
    void ResetChip() {;};


private:
    BOOL IsCCIRSource(int nInput);
    const TCardType* GetCardSetup();

    void StandardSAA7134InputSelect(int    nInput);


private:
    ///    Holds the list of all cards
    static const TCardType            m_SAA7134UnknownCard;
    static std::vector<CCardTypeEx>    m_SAA713xCards;

    static const HCParser::CParseConstant k_parseAudioPinConstants[];
    static const HCParser::CParseConstant k_parseInputTypeConstants[];
    static const HCParser::CParseConstant k_parseAudioCrystalConstants[];

    static const HCParser::CParseTag k_parseInputGPIOSet[];
    static const HCParser::CParseTag k_parseCardInput[];
    static const HCParser::CParseTag k_parseAutoDetectID[];
    static const HCParser::CParseTag k_parseCard[];
    static const HCParser::CParseTag k_parseCardList[];

    eSAA7134CardId        m_CardType;
    std::string           m_TunerType;

    CI2CBus*            m_I2CBus;
    II2CTuner*            m_Tuner;

    ///    Stores amount of memory    assigned to    DMA
    DWORD                m_DMAChannelMemorySize[7];
    ///    Stores regions prepared    for    DMA
    BYTE                m_PreparedRegions;

    DWORD                m_LastTriggerError;

    eVideoStandard        m_VideoStandard;

    eAudioInputSource    m_AudioInputSource;
    BOOL                m_bAudioLineReservedForMute;
    BOOL                m_bStereoExternalLines;
};

#endif

#endif//xxx

