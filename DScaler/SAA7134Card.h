/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Card.h,v 1.9 2002-10-06 12:14:52 atnak Exp $
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

#ifndef __SAA7134CARD_H___
#define __SAA7134CARD_H___

#include "PCICard.h"
#include "TVFormats.h"
#include "SAA7134_Defines.h"
#include "SAA7134Common.h"
#include "SAA7134I2CInterface.h"

#include "ITuner.h"
//#include "AudioDecoder.h"
#include "IAudioControls.h"

#define INPUTS_PER_CARD 7


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
    };

    enum eAudioCrystal
    {
        AUDIOCRYSTAL_32110Hz = 0,
        AUDIOCRYSTAL_24576Hz,
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
        /// Which line on the card is to be default
        eAudioInputSource AudioLineSelect;
    } TInputType;

    /// Defines the specific settings for a given card
    typedef struct
    {
        LPCSTR szName;
        int NumInputs;
        TInputType Inputs[INPUTS_PER_CARD];
        eTunerId TunerId;
        /// The type of clock crystal the card has
        eAudioCrystal AudioCrystal;
        /// Any card specific initialization - may be NULL
        void (CSAA7134Card::*pInitCardFunction)(void);
        /** Function used to switch between sources
            Cannot be NULL
            Default is StandardBT848InputSelect
        */
        void (CSAA7134Card::*pInputSwitchFunction)(int);
    } TCardType;

    /// used to store the ID for autodection
    typedef struct
    {
        DWORD ID;
        eTVCardId CardId;
        char* szName;
    } TAutoDectect878;

public:

    void SetACPIStatus(int ACPIStatus);
    int GetACPIStatus();
    void HandleTimerMessages(int TimerId);
    CSAA7134Card(CHardwareDriver* pDriver);
    ~CSAA7134Card();

    //BOOL FindCard(WORD VendorID, WORD DeviceID, int CardIndex);
    
    void SetCardType(int CardType);
    eTVCardId GetCardType();
    
    void SetVideoSource(int nInput);

    void ResetHardware();

    void SetBrightness(BYTE Brightness);
    BYTE GetBrightness();
    void SetWhiteCrushUp(BYTE WhiteCrushUp);
    BYTE GetWhiteCrushUp();
    void SetWhiteCrushDown(BYTE WhiteCrushDown);
    BYTE GetWhiteCrushDown();
    void SetHue(BYTE Hue);
    BYTE GetHue();
    void SetContrast(BYTE Contrast);
    BYTE GetContrast();
    void SetSaturation(BYTE Saturation);
    BYTE GetSaturation();
    void SetBDelay(BYTE BDelay);
    BYTE GetBDelay();
    void SetEvenLumaDec(BOOL EvenLumaDec);
    void SetOddLumaDec(BOOL OddLumaDec);
    BOOL GetEvenLumaDec();
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

    void SetWhitePeak(BOOL WhitePeak);
    BOOL GetWhitePeak();
    void SetColourPeak(BOOL ColourPeak);
    BOOL GetColourPeak();

    void SetHPLLMode(eHPLLMode HPLLMode);
    void SetVSyncRecovery(eVSyncRecovery VSyncRecovery);

    LPCSTR GetChipType();
    LPCSTR GetTunerType();

    void SetVideoStandard(eVideoStandard VideoStandard, long& VBILines, long& VideoWidth, long& VideoHeight, long HDelayShift, long VDelayShift);
    void SetGeometry(WORD ScaleWidth, WORD ScaleHeight, long HDelayShift, long VDelayShift);
    
    void SetTaskGeometry(eTaskID TaskID, WORD Width, WORD Height, WORD HDelay, WORD VDelay, WORD ScaleWidth, WORD ScaleHeight);
    void SetVBIGeometry(eTaskID TaskID, WORD HStart, WORD HStop, WORD VStart, WORD VStop);

    long GetMinimumVDelay();
    long GetMinimumVDelayWithVBI();

    BOOL IsVideoPresent();
    void SetDMA(eRegionID RegionID, BOOL bState);
    BOOL GetDMA(eRegionID RegionID);
    void StopCapture();
    void StartCapture(BOOL bCaptureVBI);

    BOOL Is25fpsSignalDetected();
    BOOL IsInterlacedSignalDetected();

    void InitAudio();

    void SetAudioLockToVideo(BOOL bLockAudio);

    void SetAudioMute();
    void SetAudioUnMute(long nVolume, eAudioInputSource InputSource);
    void SetAudioVolume(BYTE nVolume);
    void SetAudioBalance(WORD nBalance);
    void SetAudioBass(WORD nBass);
    void SetAudioTreble(WORD nTreble);

    // \todo remove these
    void SetAudioLeftVolume(BYTE nGain);
    void SetAudioRightVolume(BYTE nGain);
    void SetAudioNicamVolume(BYTE nGain);
    int GetAudioLeftVolume();
    int GetAudioRightVolume();
    int GetAudioNicamVolume();

    void SetAudioSource(eAudioInputSource InputSource);
    void SetAudioStandard(eAudioStandard AudioStandard);
    void SetAudioChannel(eAudioChannel AudioChannel);
    eAudioChannel GetAudioChannel();

    BOOL IsAudioChannelDetected(eAudioChannel AudioChannel);
    
    void SetAudioSampleRate(eAudioSampleRate SampleRate);

    eTunerId AutoDetectTuner(eTVCardId CardId);
    eTVCardId AutoDetectCardType();

    BOOL InitTuner(eTunerId tunerId);
    LPCSTR GetInputName(int nVideoSource);
    int GetMaxCards();
    LPCSTR GetCardName(eTVCardId CardId);
    int GetNumInputs();
    BOOL IsInputATuner(int nInput);

    int GetInputAudioLine(int nInput);

    LPCSTR GetAudioStandardName(eAudioStandard AudioStandard);

    static BOOL APIENTRY ChipSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

    void StatGPIO();
    void EnableI2SAudioOutput(WORD wRate);
    void EnableCCIR656VideoOut();
    void ResetHPrescale(eTaskID TaskID);

    void SetPageTable(eRegionID RegionID, DWORD pPhysical, DWORD nPages);
    void SetBaseOffsets(eRegionID RegionID, DWORD dwEvenOffset, DWORD dwOddOffset, DWORD dwPitch);
    void SetBSwapAndWSwap(eRegionID RegionID, BOOL bBSwap, BOOL bWSwap);

    BOOL GetIRQEventRegion(eRegionID& RegionID, BOOL& bIsFieldOdd);
    BOOL GetProcessingRegion(eRegionID& RegionID, BOOL& bIsFieldOdd);

    void SetHPrescale(eTaskID TaskID, WORD wSourceSize, WORD wScaleSize);

    void CheckRegisters(DWORD* AOdd, DWORD* AEven, DWORD* BOdd, DWORD* BEven);

    void DumpRegisters();

    ITuner* GetTuner() const;

    // I2C stuff
    BYTE GetI2CStatus();
    void SetI2CStatus(BYTE Status);
    void SetI2CCommand(BYTE Command);
    void SetI2CData(BYTE Data);
    BYTE GetI2CData();

    BYTE DirectGetByte(DWORD dwAddress);
    void DirectSetBit(DWORD dwAddress, int nBit, BOOL bSet);


protected:
    void SetTypicalSettings();
    void SetupTasks();

    void CheckVBIAndVideoOverlap(eTaskID TaskID);
    BOOL IsVBIActive();

    void VerifyMemorySize(eRegionID RegionID);
    WORD CalculateLinesAvailable(eRegionID RegionID, WORD wBytePerLine);

    int RegionID2Channel(eRegionID RegionID);
    BYTE TaskID2TaskMask(eTaskID TaskID);

    void UpdateAudioClocksPerField(eVideoStandard VideoStandard);


private:
    ULONG GetTickCount();
    void InitializeI2C();

    DWORD m_I2CSleepCycle;
    DWORD m_I2CRegister;
    bool m_I2CInitialized;


private:
    BOOL IsCCIRSource(int nInput);
    const TCardType* GetCardSetup();

    void StandardSAA7134InputSelect(int nInput);
    void FLYVIDEO2000CardInputSelect(int nInput);
    void MEDION5044CardInputSelect(int nInput);

    void SetCh1FMDeemphasis(eAudioFMDeemphasis FMDeemphasis);
    void SetCh2FMDeemphasis(eAudioFMDeemphasis FMDeemphasis);
    void SetAudioFMDematrix(eAudioFMDematrix FMDematrix);
    void SetFilterBandwidth(eAudioFilterBandwidth FilterBandwidth);


private:
    eTVCardId       m_CardType;
    char            m_TunerType[32];

    CI2CBus*        m_I2CBus;
    ITuner*         m_Tuner;
    IAudioControls* m_AudioControls;

private:
    static const TCardType m_TVCards[];

    // Keeps track of the amount of memory assigned to DMA
    DWORD               m_DMAChannelMemorySize[7];

    // Keep track of the regions for which DMA is prepared
    BYTE                m_PreparedRegions;

    eVideoStandard      m_VideoStandard;
    eAudioStandard      m_AudioStandard;

    eAudioInputSource   m_AudioInputSource;
    eAudioSampleRate    m_AudioSampleRate;
};


#endif