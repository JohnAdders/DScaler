/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Card.h,v 1.6 2001-11-26 13:02:27 adcockj Exp $
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

#ifndef __BT848CARD_H___
#define __BT848CARD_H___

#include "PCICard.h"
#include "TVFormats.h"
#include "BT848_Defines.h"

#include "I2CLineInterface.h"
#include "I2CBusForLineInterface.h"
#include "MSP34x0.h"
#include "ITuner.h"

class CBT848Card : public CPCICard, 
                   public II2CLineInterface
{
public:
    enum eCardType
    {
        BT878,
        BT848,
        BT849,
        BT878A,
    };

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

    enum eAudioMuxType
    {
        AUDIOMUX_TUNER = 0,
        AUDIOMUX_MSP_RADIO,
        AUDIOMUX_EXTERNAL,
        AUDIOMUX_INTERNAL,
        AUDIOMUX_MUTE,
        AUDIOMUX_STEREO
    };

    CBT848Card(CHardwareDriver* pDriver);
	~CBT848Card();

    void Mute();
    void UnMute(long nVolume);

	BOOL FindCard(eCardType CardType, int CardIndex);
	void CloseCard();
    
    void SetCardType(eTVCardId CardType);
    eTVCardId GetCardType();
    
    void SetVideoSource(eCardType BtCardType, eVideoSourceType nInput);

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
    void SetGeoSize(eCardType BtCardType, eVideoSourceType nInput, eVideoFormat TVFormat, long& CurrentX, long& CurrentY, long& CurrentVBILines, int VDelay, int HDelay);

    BOOL IsVideoPresent();
    void SetRISCStartAddress(DWORD RiscBasePhysical);
    DWORD GetRISCPos();
    void SetDMA(BOOL bState);
    void SetAudioSource(eCardType BtCardType, eAudioMuxType nChannel);
    void StopCapture();
    void StartCapture(BOOL bCaptureVBI);

    BOOL HasMSP();

    void SetMSPVolume(long nVolume);
    void SetMSPBalance(long nBalance);
    void SetMSPBass(long nBass);
    void SetMSPTreble(long nTreble);
    void SetMSPSuperBassLoudness(long nLoudness, BOOL bSuperBass);
    void SetMSPSpatial(long nSpatial);
    void SetMSPEqualizer(long EqIndex, long nLevel);
    void SetMSPMode(long nMode);
    void SetMSPStereo(eSoundChannel StereoMode);
    void SetMSPMajorMinorMode(int MajorMode, int MinorMode);
    void GetMSPPrintMode(LPSTR Text);
    eSoundChannel GetMSPWatchMode(eSoundChannel desiredSoundChannel);
    
    eTunerId AutoDetectTuner(eTVCardId CardId);
    eTVCardId AutoDetectCardType();
    void CardSpecificInit(eTVCardId CardType);

    BOOL InitTuner(eTunerId tunerId);
    const char* GetSourceName(eVideoSourceType nVideoSource);

    void InitMSP();

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
    const TCardSetup* GetCardSetup(eCardType BtCardType);

    void SetAudioGVBCTV3PCI(eSoundChannel soundChannel);
    void SetAudioLT9415(eSoundChannel soundChannel);
    void SetAudioTERRATV(eSoundChannel soundChannel);
    void SetAudioAVER_TVPHONE(eSoundChannel soundChannel);
    void SetAudioWINFAST2000(eSoundChannel soundChannel);

    char m_MSPVersion[16];
    char m_TunerStatus[32];

    eTVCardId m_CardType;
    eCardType m_BtCardType;
    bool m_bHasMSP;

    CI2CBus *m_I2CBus;
    CMSP34x0 *m_MSP34x0;
    ITuner *m_Tuner;
    eAudioMuxType m_LastAudioSource;
};


#endif