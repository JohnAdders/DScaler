/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Card.h,v 1.9 2001-12-05 21:45:10 ittarnavsky Exp $
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

/** A Generic bt848 based capture card
    The card can cope with the standard inputs,
    a tuner and one of the supported sound chips.
    Control is given over most of the supported chip 
    functions.
*/
class CBT848Card : public CPCICard, 
                   public II2CLineInterface
{
public:
    /// The standard inputs on a bt848 card, the CCIR656 ones will be moved later
    enum eVideoSourceType
    {
        SOURCE_TUNER = 0,
        SOURCE_COMPOSITE,
        SOURCE_SVIDEO,
        SOURCE_OTHER1,
        SOURCE_OTHER2,
        SOURCE_COMPVIASVIDEO,
        SOURCE_CCIR656_1,
        SOURCE_CCIR656_2,
        SOURCE_CCIR656_3,
        SOURCE_CCIR656_4,
    };

    CBT848Card(CHardwareDriver* pDriver);
	~CBT848Card();

	BOOL FindCard(WORD VendorID, WORD DeviceID, int CardIndex);
	void CloseCard();
    
    void SetCardType(eTVCardId CardType);
    eTVCardId GetCardType();
    
    void SetVideoSource(eTVCardId CardType, eVideoSourceType nInput);

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
    void SetGeoSize(eTVCardId BtCardType, eVideoSourceType nInput, eVideoFormat TVFormat, long& CurrentX, long& CurrentY, long& CurrentVBILines, int VDelay, int HDelay);

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
    void SetAudioSource(eTVCardId tvCardId, eAudioInput nChannel);
    void SetAudioChannel(eSoundChannel audioChannel);
    void GetMSPPrintMode(LPSTR Text);
    eSoundChannel IsAudioChannelDetected(eSoundChannel desiredAudioChannel);
    
    eTunerId AutoDetectTuner(eTVCardId CardId);
    eTVCardId AutoDetectCardType();
    void CardSpecificInit(eTVCardId CardType);

    BOOL InitTuner(eTunerId tunerId);
    const char* GetSourceName(eVideoSourceType nVideoSource);

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
    BOOL IsCCIRSource(eVideoSourceType nInput);
    
    void HauppaugeBootMSP34xx(int pin);
    void InitPXC200();
    void CtrlTDA8540(int SLV, int SUB, int SW1, int GCO, int OEN);
    const TCardSetup* GetCardSetup(eTVCardId CardType);

    void SetAudioGVBCTV3PCI(eSoundChannel soundChannel);
    void SetAudioLT9415(eSoundChannel soundChannel);
    void SetAudioTERRATV(eSoundChannel soundChannel);
    void SetAudioAVER_TVPHONE(eSoundChannel soundChannel);
    void SetAudioWINFAST2000(eSoundChannel soundChannel);

    char m_MSPVersion[16];
    char m_TunerStatus[32];

    eTVCardId m_CardType;
    bool m_bHasMSP;

    CI2CBus*        m_I2CBus;
    ITuner*         m_Tuner;
    CAudioDecoder*  m_AudioDecoder;
    IAudioControls* m_AudioControls;
    eAudioInput     m_LastAudioSource;
};


#endif