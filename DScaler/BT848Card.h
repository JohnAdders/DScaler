/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Card.h,v 1.3 2001-11-02 16:30:07 adcockj Exp $
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
#include "I2C.h"
#include "TVFormats.h"
#include "BT848_Defines.h"

class CBT848Card : public CPCICard, 
                   public CI2C
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
    void SetTunerType(eTunerId TunerType);
    eTunerId GetTunerType();
    
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
    void WriteMSP(BYTE bSubAddr, int wAddr, int wData);
    WORD ReadMSP(BYTE bSubAddr, WORD wAddr);
    BOOL ResetMSP();

    void SetMSPVolume(long nVolume);
    void SetMSPBalance(long nBalance);
    void SetMSPBass(long nBass);
    void SetMSPTreble(long nTreble);
    void SetMSPSuperBassLoudness(long nLoudness, BOOL bSuperBass);
    void SetMSPSpatial(long nSpatial);
    void SetMSPEqualizer(long EqIndex, long nLevel);
    void SetMSPCarrier(int cdo1, int cdo2);
    void SetMSPMode(long NewValue);
    void SetMSPStereo(int MajorMode, int MinorMode, int StereoMode, int MSPMode);
    void SetMSPMajorMinorMode(int MajorMode, int MinorMode, int MSPMode);
    void GetMSPPrintMode(LPSTR Text, int MSPMajorMode, int MSPMinorMode, int MSPMode, int MSPStereo);
    int GetMSPWatchMode(int MSPMode, int MSPStereo);
    
    eTunerId AutoDetectTuner(eTVCardId CardId);
    eTVCardId AutoDetectCardType();
    void CardSpecificInit(eTVCardId CardType);


    BOOL SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat, eTunerId TunerId);
    BOOL InitTuner();
    const char* GetTunerStatus();
    const char* GetSourceName(eVideoSourceType nVideoSource);

    static BOOL APIENTRY ChipSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

protected:
    void I2C_SetLine(BOOL bCtrl, BOOL bData);
    BOOL I2C_GetLine();
    BYTE I2C_Read(BYTE nAddr);
    BOOL I2C_Write(BYTE nAddr, BYTE nData1, BYTE nData2, BOOL bSendBoth);

private:
    void SetGeometryEvenOdd(BOOL bOdd, int wHScale, int wVScale, int wHActive, int wVActive, int wHDelay, int wVDelay, BYTE bCrop);
    void SetPLL(ePLLFreq PLL);
    BOOL IsCCIRSource(eVideoSourceType nInput);
    void InitMSP();
    
    void HauppaugeBootMSP34xx();
    void InitPXC200();
    void CtrlTDA8540(int SLV, int SUB, int SW1, int GCO, int OEN);
    const TCardSetup* GetCardSetup(eCardType BtCardType);

    void SetAudioGVBCTV3PCI(int StereoMode);
    void SetAudioLT9415(int StereoMode);
    void SetAudioTERRATV(int StereoMode);
    void SetAudioAVER_TVPHONE(int StereoMode);
    void SetAudioWINFAST2000(int StereoMode);

    char m_MSPVersion[16];
    BOOL m_MSPNicam;

    eTVCardId m_CardType;
    eTunerId m_TunerType;
    eCardType m_BtCardType;
    BOOL m_bHasMSP;
    BYTE m_TunerDevice;
    char m_TunerStatus[30];
};


#endif