/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Card.h,v 1.3 2002-09-10 12:14:35 atnak Exp $
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
// Revision 1.2  2002/09/09 14:23:29  atnak
// Fixed "log" -> "Log", "id" -> "Id"
//
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __SAA7134CARD_H___
#define __SAA7134CARD_H___

#include "PCICard.h"
#include "TVFormats.h"
#include "SAA7134_Defines.h"

#include "SAA7134I2CInterface.h"
#include "SAA7134I2CBusInterface.h"
#include "ITuner.h"
//#include "AudioDecoder.h"
#include "IAudioControls.h"
#include "NoAudioControls.h"

#define INPUTS_PER_CARD 7

enum eTaskID
{
    TASKID_A            = 0,
    TASKID_B            = 1
};

enum eRegionID
{
    REGIONID_INVALID    = -1,
    REGIONID_VIDEO_A    = 0,
    REGIONID_VIDEO_B    = 1,
    REGIONID_VBI_A      = 2,
    REGIONID_VBI_B      = 3
};


#define IsRegionIDVideo(r)      (((r) == REGIONID_VIDEO_A || (r) == REGIONID_VIDEO_B) ? TRUE : FALSE)
#define IsRegionIDVBI(r)        (((r) == REGIONID_VBI_A || (r) == REGIONID_VBI_B) ? TRUE : FALSE)
#define RegionID2TaskID(r)      (((r) == REGIONID_VIDEO_A || (r) == REGIONID_VBI_A) ? TASKID_A : TASKID_B)
#define TaskID2VideoRegion(t)   (((t) == TASKID_A) ? REGIONID_VIDEO_A : REGIONID_VIDEO_B)


/** A Generic saa7134 based capture card
    The card can cope with the standard inputs,
    a tuner and one of the supported sound chips.
    Control is given over most of the supported chip 
    functions.
*/
class CSAA7134Card : public CPCICard,
                     public ISAA7134I2CInterface,
                     public ISAA7134_Defines
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
        eAudioInputLine AudioLineSelect;
    } TInputType;

    /// Defines the specific settings for a given card
    typedef struct
    {
        LPCSTR szName;
        int NumInputs;
        TInputType Inputs[INPUTS_PER_CARD];
        eTunerId TunerId;
        /// Audio clock. Not sure what it does (@atnak)
        DWORD AudioClock;
        /// Any card specific initialization - may be NULL
        void (CSAA7134Card::*pInitCardFunction)(void);
        /** Function used to switch between sources
            Cannot be NULL
            Default is StandardBT848InputSelect
        */
        void (CSAA7134Card::*pInputSwitchFunction)(int);
        /// Any card specific method used to select stereo - may be NULL
        void (CSAA7134Card::*pSoundChannelFunction)(eSoundChannel);
        /// Bit Mask for audio GPIO operations
        DWORD GPIOMask;
    } TCardType;

    /// Used to store audio standard settings
    typedef struct
    {
        char*               Name;
        eAudioCarrier       MajorCarrier;
        eAudioCarrier       MinorCarrier;
        eAudioMonoType      MonoType;
        eAudioStereoType    StereoType;
        eFIRType            FIRType;
    } TAudioStandardDefinition;


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
    void CloseCard();
    
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

    LPCSTR GetChipType();
    LPCSTR GetTunerType();

    void RestartRISCCode(DWORD RiscBasePhysical);
    void SetGeoSize(int nInput, eVideoFormat TVFormat, long& CurrentX, long& CurrentY, long& CurrentVBILines, int VDelayOverride, int HDelayOverride);
    void SetGeoSizeTask(eTaskID TaskID, WORD SourceX, WORD SourceY, long CurrentX, long CurrentY, WORD HStart, WORD VStart);
    void SetupVBI(eTaskID TaskID, WORD HStart, WORD HStop, WORD VStart, WORD VStop);

    BOOL IsVideoPresent();
    void SetRISCStartAddress(DWORD RiscBasePhysical);
    DWORD GetRISCPos();
    void SetDMA(eRegionID RegionID, BOOL bState);
    BOOL GetDMA(eRegionID RegionID);
    void StopCapture();
    void StartCapture(BOOL bCaptureVBI);

    void InitAudio();

    void SetAudioMute();
    void SetAudioUnMute(long nVolume, eAudioInputLine Input);
    void SetAudioVolume(BYTE nVolume);
    void SetAudioBalance(WORD nBalance);
    void SetAudioBass(WORD nBass);
    void SetAudioTreble(WORD nTreble);

    void CheckStereo();

    void SetAudioStandard(eAudioStandard audioStandard);
    void SetAudioStandard(eVideoFormat videoFormat);
    void SetAudioSource(eAudioInputLine nLine);
    void SetAudioChannel(eSoundChannel audioChannel);
    void GetMSPPrintMode(LPSTR Text);
    eSoundChannel IsAudioChannelDetected(eSoundChannel desiredAudioChannel);
    
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
    BOOL IsAudioChannelAvailable(eSoundChannel soundChannel);

    static BOOL APIENTRY ChipSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

    void StatGPIO();
    void EnableI2SAudioOutput(WORD wRate);
    void EnableCCIR656VideoOut();
    void ResetHPrescale(eTaskID TaskID);
    void ResetTask(eTaskID TaskID);

    void SetPageTable(eRegionID RegionID, DWORD pPhysical, DWORD nPages);
    void SetBaseOffsets(eRegionID RegionID, DWORD dwEvenOffset, DWORD dwOddOffset, DWORD dwPitch);
    void SetBSwapAndWSwap(eRegionID RegionID, BOOL bBSwap, BOOL bWSwap);



    BOOL GetIRQEventRegion(eRegionID& RegionID, BOOL& bIsFieldOdd);
    BOOL GetProcessingRegion(eRegionID& RegionID, BOOL& bIsFieldOdd);

    void SetHPrescale(eTaskID TaskID, WORD wSourceSize, WORD wScaleSize);

    void CheckRegisters();

    void DumpRegisters();

    ITuner* GetTuner() const;

    // I2C stuff
    BYTE GetI2CStatus();
    void SetI2CStatus(BYTE Status);
    void SetI2CStart();
    void SetI2CContinue();
    void SetI2CStop();
    void SetI2CData(BYTE Data);
    BYTE GetI2CData();
    void I2CSleep();

protected:
    BOOL IsDualFMAudioStandard(eAudioStandard audioStandard);
    BOOL IsNICAMAudioStandard(eAudioStandard audioStandard);

    void VerifyMemorySize(eRegionID RegionID);
    WORD CalculateLinesAvailable(eRegionID RegionID, WORD wBytePerLine);

    int RegionID2Channel(eRegionID RegionID);
    BYTE TaskID2TaskMask(eTaskID TaskID);

private:
    ULONG GetTickCount();
    void InitializeI2C();

    DWORD m_I2CSleepCycle;
    DWORD m_I2CRegister;
    bool m_I2CInitialized;


private:
    void SetGeometryEvenOdd(BOOL bOdd, int wHScale, int wVScale, int wHActive, int wVActive, int wHDelay, int wVDelay, BYTE bCrop);
    BOOL IsCCIRSource(int nInput);
    const TCardType* GetCardSetup();

    void StandardSAA7134InputSelect(int nInput);
 
    char m_TunerType[32];

private:
    eTVCardId m_CardType;

    CI2CBus*        m_I2CBus;
    ITuner*         m_Tuner;
    IAudioControls* m_AudioControls;

private:
    static const TCardType m_TVCards[];

    // Keeps track of the number of pages assigned to DMA
    DWORD           m_nDMAChannelPageCount[7];
    // If this is FALSE, m_nDMAChannelPageCount stores memory size
    BOOL            m_bDMAChannelUsesPages[7];

    // Keep track of the regions for which DMA is prepared
    BYTE            m_PreparedRegions;
    eAudioStandard  m_AudioStandard;

    // Audio standards table
    static TAudioStandardDefinition m_AudioStandards[];
};


#endif