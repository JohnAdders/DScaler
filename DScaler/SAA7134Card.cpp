/////////////////////////////////////////////////////////////////////////////
// $id$
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
// This software was based on BT848Card.cpp.  Those portions are
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
// $log$
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "SAA7134Card.h"
#include "SAA7134_Defines.h"
#include "Audio.h"
#include "DebugLog.h"
#include "CPU.h"
#include "TVFormats.h"
/// \todo remove need for this
#include "ProgramList.h"
/// \todo remove need for this
#include "OutThreads.h"


CSAA7134Card::CSAA7134Card(CHardwareDriver* pDriver) :
    CPCICard(pDriver),
    m_CardType(TVCARD_UNKNOWN),
    m_Tuner(NULL),
    m_PreparedRegions(0x00)
{
    m_I2CInitialized = false;
    m_I2CBus = new CSAA7134I2CBusInterface(this);
}

CSAA7134Card::~CSAA7134Card()
{
    // disable peripheral devices
    WriteByte(SAA7134_SPECIAL_MODE,0);

    // shutdown hardware
    WriteDword(SAA7134_IRQ1, 0);
    WriteDword(SAA7134_IRQ2, 0);
    WriteDword(SAA7134_MAIN_CTRL,0);

    delete m_I2CBus;
    delete m_Tuner;

    ClosePCICard();
}

void CSAA7134Card::CloseCard()
{
    ClosePCICard();
}

// RegionID = one of:
// REGIONID_VIDEO_A, REGIONID_VIDEO_B
// REGIONID_VBI_A, REGIONID_VBI_B
//
//

BOOL CSAA7134Card::GetDMA(eRegionID RegionID)
{
    DWORD Mask = 0;

    switch (RegionID) {
    case REGIONID_VIDEO_A: Mask = SAA7134_MAIN_CTRL_TE0; break;
    case REGIONID_VIDEO_B: Mask = SAA7134_MAIN_CTRL_TE1; break;
    case REGIONID_VBI_A: Mask = SAA7134_MAIN_CTRL_TE2; break;
    case REGIONID_VBI_B: Mask = SAA7134_MAIN_CTRL_TE3; break;
    }

    return (ReadDword(SAA7134_MAIN_CTRL) & Mask) > 0;
}

void CSAA7134Card::SetDMA(eRegionID RegionID, BOOL bState)
{
    BYTE    Region = 0;
    DWORD   IRQs = 0;
    DWORD   Control = 0;

    switch (RegionID)
    {
    case REGIONID_VIDEO_A:
        Region = SAA7134_REGION_ENABLE_VIDEO_A;
        Control = SAA7134_MAIN_CTRL_TE0;
        IRQs = SAA7134_IRQ1_INTE_RA0_0 | SAA7134_IRQ1_INTE_RA0_1;
        break;
    case REGIONID_VIDEO_B:
        Region = SAA7134_REGION_ENABLE_VIDEO_B;
        Control = SAA7134_MAIN_CTRL_TE1;
        IRQs = SAA7134_IRQ1_INTE_RA0_2 | SAA7134_IRQ1_INTE_RA0_3;
        break;
    case REGIONID_VBI_A:
        Region = SAA7134_REGION_ENABLE_VBI_A;
        Control = SAA7134_MAIN_CTRL_TE2;
        IRQs = SAA7134_IRQ1_INTE_RA0_4 | SAA7134_IRQ1_INTE_RA0_5;
        break;

    case REGIONID_VBI_B:
        Region = SAA7134_REGION_ENABLE_VBI_B;
        Control = SAA7134_MAIN_CTRL_TE3;
        IRQs = SAA7134_IRQ1_INTE_RA0_6 | SAA7134_IRQ1_INTE_RA0_7;
        break;
    }

    if (bState) {
        VerifyMemorySize(RegionID);

        m_PreparedRegions |= Region;
        // Don't turn on interrupts because we don't have an ISR!
        // MaskDataDword(SAA7134_IRQ1, IRQs, IRQs);
        MaskDataDword(SAA7134_MAIN_CTRL, Control, Control);
    }
    else {
        m_PreparedRegions &= ~Region;
        MaskDataByte(SAA7134_REGION_ENABLE, 0x00, Region);
        MaskDataDword(SAA7134_IRQ1, 0x00, IRQs);
        MaskDataDword(SAA7134_MAIN_CTRL, 0x00, Control);
    }
}

void CSAA7134Card::ResetHardware()
{
//  LOG(0, "Initial registery dump");
//  DumpRegisters();

    WriteByte(SAA7134_REGION_ENABLE, 0x00);

    WriteByte(SAA7134_SOURCE_TIMING1, 0x00);  // ??
    WriteByte(SAA7134_SOURCE_TIMING2, 0x00);  // ??
    // registers: 0x002, 0x003 ??
    // registers: 0x005, 0x008, 0x009, 0x00A, 0x00B ??
    WriteByte(SAA7134_START_GREEN, 0x00);
    WriteByte(SAA7134_START_BLUE, 0x00);
    WriteByte(SAA7134_START_RED, 0x00);

    for (int i = 0; i < 0x0F; i++)
    {
        WriteByte(SAA7134_GREEN_PATH(i), (i+1)<<4);
        WriteByte(SAA7134_BLUE_PATH(i), (i+1)<<4);
        WriteByte(SAA7134_RED_PATH(i), (i+1)<<4);
    }
    WriteByte(SAA7134_GREEN_PATH(0x0F), 0xFF);
    WriteByte(SAA7134_BLUE_PATH(0x0F), 0xFF);
    WriteByte(SAA7134_RED_PATH(0x0F), 0xFF);

    // RAM FIFO config ???
    WriteDword(SAA7134_FIFO_SIZE, 0x08070503);
    WriteDword(SAA7134_THRESHOULD, 0x02020202);


    WriteByte(SAA7134_INCR_DELAY,               0x08);
    WriteByte(SAA7134_ANALOG_IN_CTRL1,          0xC0);
    WriteByte(SAA7134_ANALOG_IN_CTRL2,          0x10);

    WriteByte(SAA7134_ANALOG_IN_CTRL3,          0x90);
    WriteByte(SAA7134_ANALOG_IN_CTRL4,          0x90);
//  WriteByte(SAA7134_HSYNC_START,              0xeb);
//  WriteByte(SAA7134_HSYNC_STOP,               0xe0);
    WriteByte(SAA7134_HSYNC_START,              0x00);
    WriteByte(SAA7134_HSYNC_STOP,               0x00);


    WriteByte(SAA7134_MODE_DELAY_CTRL,       0x00);

//  WriteByte(SAA7134_ANALOG_ADC,            0x01);
//  WriteByte(SAA7134_VGATE_START,           0x11);
    WriteByte(SAA7134_ANALOG_ADC,            0x05);
    WriteByte(SAA7134_VGATE_START,           0x15);
//  WriteByte(SAA7134_VGATE_STOP,            0xfe);
//  WriteByte(SAA7134_MISC_VGATE_MSB,        0x18); // FIXME ??
    WriteByte(SAA7134_VGATE_STOP,            0x34);
    WriteByte(SAA7134_MISC_VGATE_MSB,        0x1a); // FIXME ??
    WriteByte(SAA7134_RAW_DATA_GAIN,         0x40);
    WriteByte(SAA7134_RAW_DATA_OFFSET,       0x80);

    WriteByte(SAA7134_DEMODULATOR,               0x00);
    WriteByte(SAA7134_DCXO_IDENT_CTRL,           0x00);
    WriteByte(SAA7134_FM_DEEMPHASIS,             0x22);
    WriteByte(SAA7134_FM_DEMATRIX,               0x80);
    WriteByte(SAA7134_STEREO_DAC_OUTPUT_SELECT,  0xa0);


    // Enable audio and video processing
    WriteDword(SAA7134_MAIN_CTRL,
            SAA7134_MAIN_CTRL_VPLLE |
            SAA7134_MAIN_CTRL_APLLE |
            SAA7134_MAIN_CTRL_EXOSC |
            SAA7134_MAIN_CTRL_EVFE1 |
            SAA7134_MAIN_CTRL_EVFE2 |
            SAA7134_MAIN_CTRL_ESFE  |
            SAA7134_MAIN_CTRL_EBADC |
            SAA7134_MAIN_CTRL_EBDAC);

    // Disable IRQ's
    WriteDword(SAA7134_IRQ1, 0);
    WriteDword(SAA7134_IRQ2, 0);

    // Enable peripheral devices
    WriteByte(SAA7134_SPECIAL_MODE, 0x01);
}

void CSAA7134Card::SetCardType(int CardType)
{
    if(m_CardType != CardType)
    {
        m_CardType = (eTVCardId)CardType;

        // perform card specific init
        if(m_TVCards[m_CardType].pInitCardFunction != NULL)
        {
            // call correct function
            // this funny syntax is the only one that works
            // if you want help understanding what is going on
            // I suggest you read http://www.newty.de/
            (*this.*m_TVCards[m_CardType].pInitCardFunction)();
        }
    }
}

eTVCardId CSAA7134Card::GetCardType()
{
    return m_CardType;
}

LPCSTR CSAA7134Card::GetCardName(eTVCardId CardId)
{
    return m_TVCards[CardId].szName;
}

void CSAA7134Card::SetBrightness(BYTE Brightness)
{
    WriteByte(SAA7134_DEC_LUMA_BRIGHT, Brightness);
}

BYTE CSAA7134Card::GetBrightness()
{
    return ReadByte(SAA7134_DEC_LUMA_BRIGHT);
}

// Find SAA7134 equivalent or remove
void CSAA7134Card::SetWhiteCrushUp(BYTE WhiteCrushUp)
{
}

// Find SAA7134 equivalent or remove
BYTE CSAA7134Card::GetWhiteCrushUp()
{
    return 0x00;
}

// Find SAA7134 equivalent or remove
void CSAA7134Card::SetWhiteCrushDown(BYTE WhiteCrushDown)
{
//    WriteByte(BT848_WC_DOWN, WhiteCrushDown);
}

// Find SAA7134 equivalent or remove
BYTE CSAA7134Card::GetWhiteCrushDown()
{
    return 0x00;
}

void CSAA7134Card::SetHue(BYTE Hue)
{
    WriteByte(SAA7134_DEC_CHROMA_HUE, Hue);
}

BYTE CSAA7134Card::GetHue()
{
    return ReadByte(SAA7134_DEC_CHROMA_HUE);
}

void CSAA7134Card::SetContrast(BYTE Contrast)
{
    WriteByte(SAA7134_DEC_LUMA_CONTRAST, Contrast);
}

BYTE CSAA7134Card::GetContrast()
{
    return ReadByte(SAA7134_DEC_LUMA_CONTRAST);
}

void CSAA7134Card::SetSaturation(BYTE SaturationU)
{
    WriteByte(SAA7134_DEC_CHROMA_SATURATION, SaturationU);
}

BYTE CSAA7134Card::GetSaturation()
{
    return ReadByte(SAA7134_DEC_CHROMA_SATURATION);
}

// Find SAA7134 equivalent or remove
void CSAA7134Card::SetBDelay(BYTE BDelay)
{
}

// Find SAA7134 equivalent or remove
BYTE CSAA7134Card::GetBDelay()
{
    return 0x00;
}


LPCSTR CSAA7134Card::GetChipType()
{
    switch (m_DeviceId)
    {
    case 0x7134:
        return "Saa7134";
    }
    return "n/a";
}

LPCSTR CSAA7134Card::GetTunerType()
{
    return m_TunerType;
}


BOOL CSAA7134Card::IsCCIRSource(int nInput)
{
    return (m_TVCards[m_CardType].Inputs[nInput].InputType == INPUTTYPE_CCIR);
}

BOOL CSAA7134Card::IsVideoPresent()
{
    // Guessed these from register dump
    WORD CheckMask = SAA7134_STATUS_VIDEO_WIPA | SAA7134_STATUS_VIDEO_RDCAP;

    if ((ReadWord(SAA7134_STATUS_VIDEO) & CheckMask) == CheckMask)
    {
        return TRUE;
    }

    return FALSE;
}


void CSAA7134Card::StopCapture()
{
    WriteByte(SAA7134_AUDIO_MUTE_CTRL, 0xff);
    WriteByte(SAA7134_REGION_ENABLE, 0x00);
}

void CSAA7134Card::StartCapture(BOOL bCaptureVBI)
{
    BYTE Region;

    Region = SAA7134_REGION_ENABLE_VIDEO_A | SAA7134_REGION_ENABLE_VIDEO_B;

    if (bCaptureVBI)
    {
        Region |= SAA7134_REGION_ENABLE_VBI_A | SAA7134_REGION_ENABLE_VBI_B;
    }

    MaskDataByte(SAA7134_REGION_ENABLE, Region, m_PreparedRegions);
    WriteByte(SAA7134_AUDIO_MUTE_CTRL, 0xbb);
}


LPCSTR CSAA7134Card::GetInputName(int nInput)
{
    if(nInput < m_TVCards[m_CardType].NumInputs && nInput >= 0)
    {
        return m_TVCards[m_CardType].Inputs[nInput].szName;
    }
    return "Error";
}

//
//  SAA7134's I2C interface
//
BYTE CSAA7134Card::GetI2CStatus()
{
    return ReadByte(SAA7134_I2C_ATTR_STATUS) & 0x0F;
}

void CSAA7134Card::SetI2CStatus(BYTE Status)
{
    MaskDataByte(SAA7134_I2C_ATTR_STATUS, Status, 0x0F);
}

void CSAA7134Card::SetI2CStart()
{
    // 0xC0 = START
    MaskDataByte(SAA7134_I2C_ATTR_STATUS, 0xC0, 0xC0);
}

void CSAA7134Card::SetI2CContinue()
{
    // 0x80 = CONTINUE
    MaskDataByte(SAA7134_I2C_ATTR_STATUS, 0x80, 0xC0);
}

void CSAA7134Card::SetI2CStop()
{
    // 0xC0 = STOP
    MaskDataByte(SAA7134_I2C_ATTR_STATUS, 0x40, 0xC0);
}

void CSAA7134Card::SetI2CData(BYTE Data)
{
    WriteByte(SAA7134_I2C_DATA, Data);
}

BYTE CSAA7134Card::GetI2CData()
{
    return ReadByte(SAA7134_I2C_DATA);
}


ULONG CSAA7134Card::GetTickCount()
{
    ULONGLONG ticks;
    ULONGLONG frequency;

    QueryPerformanceFrequency((PLARGE_INTEGER)&frequency);
    QueryPerformanceCounter((PLARGE_INTEGER)&ticks);
    ticks = (ticks & 0xFFFFFFFF00000000) / frequency * 10000000 +
            (ticks & 0xFFFFFFFF) * 10000000 / frequency;
    return (ULONG)(ticks / 10000);
}

void CSAA7134Card::InitializeI2C()
{
    WriteByte(SAA7134_I2C_CLOCK_SELECT, 0x00);
    WriteByte(SAA7134_I2C_TIMER, 0xF0);

    m_I2CSleepCycle = 10000L;
    DWORD elapsed = 0L;
    // get a stable reading
    while (elapsed < 5)
    {
        m_I2CSleepCycle *= 10;
        DWORD start = GetTickCount();
        for (volatile DWORD i = m_I2CSleepCycle; i > 0; i--);
        elapsed = GetTickCount() - start;
    }
    // calculate how many cycles a 50kHZ is (half I2C bus cycle)
    m_I2CSleepCycle = m_I2CSleepCycle / elapsed * 1000L / 50000L;
    
    m_I2CInitialized = true;
}

void CSAA7134Card::I2CSleep()
{
    if (!m_I2CInitialized)
    {
        InitializeI2C();
    }
    for (volatile DWORD i = m_I2CSleepCycle; i > 0; i--);
}


// TODO: Unchecked and untested..  Haven't looked at what this does yet
BOOL APIENTRY CSAA7134Card::ChipSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    CSAA7134Card* pThis = NULL;
    char szCardId[9] = "n/a     ";
    char szVendorId[9] = "n/a ";
    char szDeviceId[9] = "n/a ";
    DWORD dwCardId(0);

    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CSAA7134Card*)lParam; 
        SetDlgItemText(hDlg, IDC_BT_CHIP_TYPE, pThis->GetChipType());
        sprintf(szVendorId,"%04X", pThis->GetVendorId());
        SetDlgItemText(hDlg, IDC_BT_VENDOR_ID, szVendorId);
        sprintf(szDeviceId,"%04X", pThis->GetDeviceId());
        SetDlgItemText(hDlg, IDC_BT_DEVICE_ID, szDeviceId);
        SetDlgItemText(hDlg, IDC_TUNER_TYPE, pThis->GetTunerType());
        // SetDlgItemText(hDlg, IDC_AUDIO_DECODER_TYPE, pThis->GetAudioDecoderType());
        dwCardId = pThis->GetSubSystemId();
        if(dwCardId != 0 && dwCardId != 0xffffffff)
        {
            sprintf(szCardId,"%8X", dwCardId);
        }
        SetDlgItemText(hDlg, IDC_AUTODECTECTID, szCardId);

        SetDlgItemText(hDlg, IDC_TEXT18, "YUV2");
        
        // TB 20010109 added Chip Type
        if (CpuFeatureFlags & FEATURE_SSE2)
        {
            SetDlgItemText(hDlg, IDC_CPU_TYPE, "SSE2");
        }
        else if (CpuFeatureFlags & FEATURE_SSE)
        {
            SetDlgItemText(hDlg, IDC_CPU_TYPE, "SSE");
        }
        else if (CpuFeatureFlags & FEATURE_MMXEXT)
        {
            SetDlgItemText(hDlg, IDC_CPU_TYPE, "MMXEXT");
        }
        else if (CpuFeatureFlags & FEATURE_3DNOWEXT)
        {
            SetDlgItemText(hDlg, IDC_CPU_TYPE, "3DNOWEXT");
        }
        else if (CpuFeatureFlags & FEATURE_3DNOW)
        {
            SetDlgItemText(hDlg, IDC_CPU_TYPE, "3DNOW");
        }
        else
        {
            SetDlgItemText(hDlg, IDC_CPU_TYPE, "MMX");
        }

        break;

    case WM_COMMAND:

        if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
        {
            EndDialog(hDlg, TRUE);
        }

        break;
    }

    return (FALSE);
}


// this functions returns 0 if the card is in ACPI state D0 or error
// returns 3 if in D3 state (full off)
int CSAA7134Card::GetACPIStatus()
{
    // \todo Need to implement
    return 0;
}

// Set ACPIStatus to 0 for D0/full on state. 3 for D3/full off
void CSAA7134Card::SetACPIStatus(int ACPIStatus)
{
    // \todo Need to implement
}

void CSAA7134Card::DumpRegisters()
{
    for (int i = 0x000; i < 0x400; i += 16)
    {
        // WARNING! this dumps registers in Big Endian WORDs!!
        // This causes addresses to be: 0x01, 0x00, 0x03, 0x02...
        LOG(0, "WARNING! registers are dumped in Big Endian WORDs!!");
        LOG(0, "This causes addresses to be: 0x01, 0x00, 0x03, 0x02...");
        LOG(0, "%03lX: %04lx %04lx %04lx %04lx|%04lx %04lx %04lx %04lx", i,
            ReadWord(i), ReadWord(i+2), ReadWord(i+4), ReadWord(i+6),
            ReadWord(i+8), ReadWord(i+10), ReadWord(i+12), ReadWord(i+14));
    }
}

// Don't know what this is for, came from v4l2 saa7134 code
void CSAA7134Card::StatGPIO()
{
    MaskDataByte(SAA7134_GPIO_GPMODE3, 0, SAA7134_GPIO_GPRESCAN);
    MaskDataByte(SAA7134_GPIO_GPMODE3, SAA7134_GPIO_GPRESCAN, SAA7134_GPIO_GPRESCAN);

    DWORD Mode = ReadDword(SAA7134_GPIO_GPMODE0) & 0xfffffff;
    DWORD Status = ReadDword(SAA7134_GPIO_GPSTATUS0) & 0xfffffff;
    LOG(0, "debug: gpio: mode=0x%07lx in=0x%07lx out=0x%07lx\n", Mode,
            (~Mode) & Status, Mode & Status);
}

// Unused - v4l2 uses this if i2s_rate exists on the card
void CSAA7134Card::EnableI2SAudioOutput(WORD wRate)
{
    // set rate
    MaskDataByte(SAA7134_SIF_SAMPLE_FREQ, wRate == 32000 ? 0x01 : 0x03, 0x03);

    // enable I2S output -- no idea
    WriteByte(SAA7134_DSP_OUTPUT_SELECT,    0x80);
    WriteByte(SAA7134_I2S_OUTPUT_SELECT,    0x80);
    WriteByte(SAA7134_I2S_OUTPUT_FORMAT,    0x01);
    WriteByte(SAA7134_I2S_OUTPUT_LEVEL,     0x00);  
    WriteByte(SAA7134_I2S_AUDIO_OUTPUT,     0x01);
}


// Unused, (v4l2 checks video_out existance in saa7134-cards.c)
void CSAA7134Card::EnableCCIR656VideoOut()
{
    // enable video output for CCIR656
    WriteByte(SAA7134_VIDEO_PORT_CTRL0, 0x00);
    WriteByte(SAA7134_VIDEO_PORT_CTRL1, 0xB1);
    WriteByte(SAA7134_VIDEO_PORT_CTRL2, 0x00);
    WriteByte(SAA7134_VIDEO_PORT_CTRL3, 0xA1);
    WriteByte(SAA7134_VIDEO_PORT_CTRL4, 0x00);
    WriteByte(SAA7134_VIDEO_PORT_CTRL5, 0x04);
    WriteByte(SAA7134_VIDEO_PORT_CTRL6, 0x06);
    WriteByte(SAA7134_VIDEO_PORT_CTRL7, 0x00);
    WriteByte(SAA7134_VIDEO_PORT_CTRL8, 0x00);
}


/*
	v4l2 puts audio line to LINE1 on finish

  saa7134_tvaudio_fini:

    saa_andorb(SAA7134_ANALOG_IO_SELECT, 0x07, 0x00);
*/


// Unused, might not need this for DScaler
// SAA7134_H_SCALE_INC does the scaling we need
// - This is "prescaling" as opposed to scaling.. don't know what that means
void CSAA7134Card::SetHPrescale(eTaskID TaskID, WORD wSourceSize, WORD wScaleSize)
{
    BYTE TaskMask = TaskID2TaskMask(TaskID);

    static const struct {
        int xpsc;
        int xacl;
        int xc2_1;
        int xdcg;
        int vpfy;
    } vals[] = {
        /* XPSC XACL XC2_1 XDCG VPFY */
        {    1,   0,    0,    0,   0 },
        {    2,   2,    1,    2,   2 },
        {    3,   4,    1,    3,   2 },
        {    4,   8,    1,    4,   2 },
        {    5,   8,    1,    4,   2 },
        {    6,   8,    1,    4,   3 },
        {    7,   8,    1,    4,   3 },
        {    8,  15,    0,    4,   3 },
        {    9,  15,    0,    4,   3 },
        {   10,  16,    1,    5,   3 },
    };

    int i = 2;

    WriteByte(SAA7134_H_PRESCALE(TaskMask), vals[i].xpsc);
    WriteByte(SAA7134_ACC_LENGTH(TaskMask), vals[i].xacl);
    WriteByte(SAA7134_LEVEL_CTRL(TaskMask), (vals[i].xc2_1 << 3) | (vals[i].xdcg));
    MaskDataByte(SAA7134_FIR_PREFILTER_CTRL(TaskMask),
        (vals[i].vpfy << 2) | vals[i].vpfy, 0x0F);
}

// \todo maybe VDelayOverride and HDelayOverride can be implemented
//       by shifting HStart and VStart
//       (don't shift VStart to overlap VBI when VBI is enabled!)
//       (shift HStart by 2 pixels so it follows in YUV bounds)
void CSAA7134Card::SetGeoSize(
								  int nInput,
								  eVideoFormat TVFormat,
								  long& CurrentX,
								  long& CurrentY,
								  long& CurrentVBILines,
								  int VDelayOverride,
								  int HDelayOverride
							  )
{
	// These are settings for "Auto detect" (according to v4l2'saa7134)
    BYTE SyncControl = 0x98;
    BYTE LumaControl = 0x40;
    BYTE ChromaCtrl1 = 0x8b;
    BYTE ChromaCtrl2 = 0x00;
    BYTE ChromaGain  = 0x00;

    WORD SourceWidth = 720;
    WORD SourceLines = 288;

    WORD SourceFirstPixel = 0;
    WORD SourceFirstLine  = 24;

    WORD VBIStartLine = 7;
    WORD VBIStopLine  = 22;

    if (IsPALVideoFormat(TVFormat))
    {
        SyncControl = 0x18;
        LumaControl = 0x40;
        ChromaCtrl1 = 0x81;
        ChromaCtrl2 = 0x06;
        ChromaGain  = 0x2A;

        SourceFirstLine = 22; // I see video from 21-311

        VBIStartLine = 4;
        VBIStopLine = 19;
//      VBIStartLine = 320;
//      VBIStopLine = 335;
    }
    else if (IsSECAMVideoFormat(TVFormat))
    {
        SyncControl = 0x58;
        LumaControl = 0x1B;
        ChromaCtrl1 = 0xD1;
        ChromaCtrl2 = 0x00;
        ChromaGain  = 0x80;
    }
    else if (IsNTSCVideoFormat(TVFormat))
    {
        SyncControl = 0x59;
        LumaControl = 0x40;
        ChromaCtrl1 = 0x89;
        ChromaCtrl2 = 0x0E;
        ChromaGain  = 0x2A;

        SourceFirstLine = 22;
        SourceLines = 240;

        VBIStartLine = 4;
        VBIStopLine = 22;
    }
    else
    {
        // NEVER_GET_HERE;  <-- undefined
    }

    WriteByte(SAA7134_SYNC_CTRL, SyncControl);
    // 0x80 bit is used by svideo for something
    MaskDataByte(SAA7134_LUMA_CTRL, LumaControl, 0x7F);
    WriteByte(SAA7134_CHROMA_CTRL1, ChromaCtrl1);
    WriteByte(SAA7134_CHROMA_CTRL2, ChromaCtrl2);
    WriteByte(SAA7134_CHROMA_GAIN, ChromaGain);

	// Set up VBI for both tasks
    SetupVBI(TASKID_A, 0, 719, VBIStartLine, VBIStopLine);
    SetupVBI(TASKID_B, 0, 719, VBIStartLine, VBIStopLine);

	// Set up geometry for both tasks
    SetGeoSizeTask(TASKID_A, SourceWidth, SourceLines,
        CurrentX, CurrentY, SourceFirstPixel, SourceFirstLine);
    SetGeoSizeTask(TASKID_B, SourceWidth, SourceLines,
        CurrentX, CurrentY, SourceFirstPixel, SourceFirstLine);
}

// Set up VBI
void CSAA7134Card::SetupVBI(eTaskID TaskID, WORD HStart, WORD HStop, WORD VStart, WORD VStop)
{
    BYTE TaskMask = TaskID2TaskMask(TaskID);

    WORD HorizontalBytes = 1024;
    WORD Lines = 16;

    WriteWord(SAA7134_VBI_H_START(TaskMask), HStart);
    WriteWord(SAA7134_VBI_H_STOP(TaskMask), HStop);
    WriteWord(SAA7134_VBI_V_START(TaskMask), VStart);
    WriteWord(SAA7134_VBI_V_STOP(TaskMask), VStop);

	// SAA7134_VBI_H_SCALE_INC:
    //   0x400 for 100%, 0x200 for 200%

	// DScaler wants exactly 0x0186 horizontal scaling but SAA7134
	// can't handle this scaling.  Instead, we scale 0x30C (half of
	// 0x0186) and double the bytes in SAA7134Source.cpp
    // WriteWord(SAA7134_VBI_H_SCALE_INC(TaskMask), 0x0186);
	//
    WriteWord(SAA7134_VBI_H_SCALE_INC(TaskMask), 0x030C);
    WriteByte(SAA7134_VBI_PHASE_OFFSET_LUMA(TaskMask), 0x00);
    WriteByte(SAA7134_VBI_PHASE_OFFSET_CHROMA(TaskMask), 0x00);

    WriteWord(SAA7134_VBI_H_LEN(TaskMask), HorizontalBytes);
    WriteWord(SAA7134_VBI_V_LEN(TaskMask), Lines);

    MaskDataByte(SAA7134_DATA_PATH(TaskMask), 0x00, 0xC0);

}


void CSAA7134Card::SetGeoSizeTask(
                                      eTaskID TaskID,
                                      WORD SourceX,
                                      WORD SourceY,
                                      long CurrentX,
                                      long CurrentY,
                                      WORD HStart,
                                      WORD VStart
                                  )
{
    BYTE TaskMask = TaskID2TaskMask(TaskID);

    WriteWord(SAA7134_VIDEO_H_START(TaskMask), HStart);
    WriteWord(SAA7134_VIDEO_H_STOP(TaskMask), HStart + SourceX - 1);
    WriteWord(SAA7134_VIDEO_V_START(TaskMask), VStart);
    WriteWord(SAA7134_VIDEO_V_STOP(TaskMask), VStart + SourceY - 1);

    // settings for no horizontal prescaling
    ResetHPrescale(TaskID);

    // 0x400 = median
    WORD HorzScale = 0x400 * SourceX / CurrentX;
    WriteWord(SAA7134_H_SCALE_INC(TaskMask), HorzScale);

    // settings for vertical scaling
    WORD VertScale = 0x400 * SourceY / (CurrentY / 2);
    WriteWord(SAA7134_V_SCALE_RATIO(TaskMask), VertScale);

    // 0x02 mask = Mirroring, 0x01 mask = LPI/ACM ??
    WriteByte(SAA7134_V_FILTER(TaskMask), 0x00);
    WriteByte(SAA7134_LUMA_CONTRAST(TaskMask), 0x40);
    WriteByte(SAA7134_CHROMA_SATURATION(TaskMask), 0x40);
    WriteByte(SAA7134_LUMA_BRIGHT(TaskMask), 0x80);

    WORD Lines = CurrentY/2;

    eRegionID RegionID = TaskID2VideoRegion(TaskID);
    if (GetDMA(RegionID))
    {
        // Make sure we aren't grabbing more lines than we have memory
        // Multiply CurrentX by 2 because YUV is two bytes per pixel
        WORD LinesAvailable = CalculateLinesAvailable(RegionID, CurrentX * 2);

        if (LinesAvailable < Lines)
            Lines = LinesAvailable;
    }

    // The number of pixels and lines to DMA
    WriteWord(SAA7134_VIDEO_PIXELS(TaskMask), CurrentX);
    WriteWord(SAA7134_VIDEO_LINES(TaskMask), Lines);

    WriteWord(SAA7134_H_PHASE_OFF_LUMA(TaskMask), 0x00);
    WriteWord(SAA7134_H_PHASE_OFF_CHROMA(TaskMask), 0x00);

    // deinterlace y offsets ?? no idea what these are
    // Odds: 0x00 default
    // Evens: 0x00 default + yscale(1024) / 0x20;
	// ---
	// We can tweak these to change the top line to even (by giving
	// even an offset of 0x20) but it doesn't change VBI
    WriteByte(SAA7134_V_PHASE_OFFSET0(TaskMask), 0x00); // Odd
    WriteByte(SAA7134_V_PHASE_OFFSET1(TaskMask), 0x00); // Even
    WriteByte(SAA7134_V_PHASE_OFFSET2(TaskMask), 0x00); // Odd
    WriteByte(SAA7134_V_PHASE_OFFSET3(TaskMask), 0x00); // Even
}


void CSAA7134Card::ResetHPrescale(eTaskID TaskID)
{
    BYTE TaskMask = TaskID2TaskMask(TaskID);

    WriteByte(SAA7134_H_PRESCALE(TaskMask), 0x01);
    WriteByte(SAA7134_ACC_LENGTH(TaskMask), 0x00);

    /* 0x07 mask = XDCG, 0x08 mask = 0xXC2_1 whatever that means */
    WriteByte(SAA7134_LEVEL_CTRL(TaskMask), 0x00);
    MaskDataByte(SAA7134_FIR_PREFILTER_CTRL(TaskMask), 0x00, 0x0F); /* VPFY ?? */
}


int CSAA7134Card::RegionID2Channel(eRegionID RegionID)
{
    switch (RegionID)
    {
    case REGIONID_VIDEO_A: return 0;
    case REGIONID_VIDEO_B: return 1;
    case REGIONID_VBI_A: return 2;
    case REGIONID_VBI_B: return 3;
    default:
        // NEVER_GET_HERE;
        break;
    }
    return 0;
}


BYTE CSAA7134Card::TaskID2TaskMask(eTaskID TaskID)
{
    switch (TaskID)
    {
    case TASKID_A: return SAA7134_TASK_A_MASK;
    case TASKID_B: return SAA7134_TASK_B_MASK;
    default:
        // NEVER_GET_HERE;
        break;
    }
    return 0;
}

//
// SAA7134 Notes
//
//  Channel DMA
//   0      Video A (odd even)
//   1      Video B (odd even)
//   2      VBI A (odd even)
//   3      VBI B (odd even)
//   4      Planar A
//   5      Planar A
//   5      Transport Stream
//   6      Audio
//


// If pages aren't being used, nPages should contain buffer size
void CSAA7134Card::SetPageTable(eRegionID RegionID, DWORD pPhysical, DWORD nPages)
{
    int Channel = RegionID2Channel(RegionID);

    m_nDMAChannelPageCount[Channel] = nPages;
    m_bDMAChannelUsesPages[Channel] = (pPhysical != 0UL);
    
    if (m_bDMAChannelUsesPages[Channel])
    {
        DWORD Page = pPhysical >> 12;

        // ME must be enabled for page tables to work
        WriteByte(SAA7134_RS_CONTROL_0(Channel), Page & 0xFF);
        WriteByte(SAA7134_RS_CONTROL_1(Channel), Page >> 8 & 0xFF);
        WriteByte(SAA7134_RS_CONTROL_2(Channel), Page >> 16 |
            SAA7134_RS_CONTROL_2_ME | SAA7134_RS_CONTROL_2_BURST_MAX);
    }
    else
    {
        WriteByte(SAA7134_RS_CONTROL_0(Channel), 0x00);
        WriteByte(SAA7134_RS_CONTROL_1(Channel), 0x00);
        WriteByte(SAA7134_RS_CONTROL_2(Channel), SAA7134_RS_CONTROL_2_BURST_MAX);
    }
}


void CSAA7134Card::SetBaseOffsets(eRegionID RegionID, DWORD dwEvenOffset, DWORD dwOddOffset, DWORD dwPitch)
{
    int Channel = RegionID2Channel(RegionID);

	// WARNING!!  SAA7134 thinks the top line is odd but DScaler expects
	//            it to be even.  Do the conversion in here in SAA7134Card.cpp
	//            so we don't confuse ourselves... (I hope)  Make sure everything
	//            we give back to SAA7134Source.cpp has already been converted.
	//            GetProcessingRegion() and GetIRQEventRegion() and should have
	//            the conversion back.

    // Number bytes to offset into every page
	// Give the even offset as odd and odd offset as even
    WriteDword(SAA7134_RS_E_BA(Channel), dwOddOffset);
    WriteDword(SAA7134_RS_O_BA(Channel), dwEvenOffset);

    // Number of bytes to spend per line
    WriteDword(SAA7134_RS_PITCH(Channel), dwPitch);
}


void CSAA7134Card::SetBSwapAndWSwap(eRegionID RegionID, BOOL bBSwap, BOOL bWSwap)
{
    int Channel = RegionID2Channel(RegionID);

    WriteByte(SAA7134_RS_CONTROL_3(Channel),
            (bBSwap ? SAA7134_RS_CONTROL_3_BSWAP : 0x00) |
            (bWSwap ? SAA7134_RS_CONTROL_3_WSWAP : 0x00)
        );
}


void CSAA7134Card::VerifyMemorySize(eRegionID RegionID)
{
    WORD LinesAvailable;
    WORD BytesPerLine;

    // Only Video and VBI memory size checking is implemented
    if (!IsRegionIDVideo(RegionID) && !IsRegionIDVBI(RegionID))
    {
        return;
    }

    BYTE TaskMask = TaskID2TaskMask(RegionID2TaskID(RegionID));

    if (IsRegionIDVBI(RegionID))
    {
        BytesPerLine = ReadWord(SAA7134_VBI_H_LEN(TaskMask));
    }
    else
    {
        // YUV is 2 bytes per pixel
        BytesPerLine = ReadWord(SAA7134_VIDEO_PIXELS(TaskMask)) * 2;
    }

    LinesAvailable = CalculateLinesAvailable(RegionID, BytesPerLine);

    if (IsRegionIDVBI(RegionID))
    {
        if (LinesAvailable < ReadWord(SAA7134_VBI_V_LEN(TaskMask)))
        {
            WriteWord(SAA7134_VBI_H_LEN(TaskMask), LinesAvailable);
        }
    }
    else
    {
        if (LinesAvailable < ReadWord(SAA7134_VIDEO_LINES(TaskMask)))
        {
            WriteWord(SAA7134_VIDEO_LINES(TaskMask), LinesAvailable);
        }
    }
}


#define PAGE_SIZE (1<<12)

WORD CSAA7134Card::CalculateLinesAvailable(eRegionID RegionID, WORD wBytesPerLine)
{
    int Channel = RegionID2Channel(RegionID);

    DWORD EvenOffset    = ReadDword(SAA7134_RS_E_BA(Channel));
    DWORD OddOffset     = ReadDword(SAA7134_RS_O_BA(Channel));
    DWORD Pitch         = ReadDword(SAA7134_RS_PITCH(Channel));

    DWORD MaxBaseOffset = (EvenOffset > OddOffset) ? EvenOffset : OddOffset;
    DWORD MinimumBytesAvailable;

    if (m_bDMAChannelUsesPages[Channel])
    {
        MinimumBytesAvailable = m_nDMAChannelPageCount[Channel] * PAGE_SIZE;
    }
    else
    {
        // m_nDMAChannelPageCount[n] should contain the memory size
        MinimumBytesAvailable = m_nDMAChannelPageCount[Channel];
    }

    MinimumBytesAvailable -= MaxBaseOffset;
    if (MinimumBytesAvailable < wBytesPerLine)
    {
        return 0;
    }

    return (MinimumBytesAvailable - wBytesPerLine) / Pitch + 1;
}

#undef PAGE_SIZE


// TODO2: might move this into ResetHardware()
void CSAA7134Card::ResetTask(eTaskID TaskID)
{
    BYTE TaskMask = TaskID2TaskMask(TaskID);

    // 0x0F mask = YUV  ??
    MaskDataByte(SAA7134_DATA_PATH(TaskMask), 0x00, 0x3F);

    // 0x00 for YUV
    if (TaskID == TASKID_A)
    {
        WriteByte(SAA7134_OFMT_VIDEO_A, 0x00);
        WriteByte(SAA7134_OFMT_DATA_A, 0x06);
    }
    else
    {
        WriteByte(SAA7134_OFMT_VIDEO_B, 0x00);
        WriteByte(SAA7134_OFMT_DATA_B, 0x06);
    }

    // 0x03 mask, 1 = odd first, 2 = even first
	// This just changes which field is grabbed first. It
	// doesn't change fact that odd is the top line.
    WriteByte(SAA7134_TASK_CONDITIONS(TaskMask), 0x0d);

    // 0x02: handle 2 fields and repeat ?
    WriteByte(SAA7134_FIELD_HANDLING(TaskMask), 0x02);
}


// DEBUG: eventually get rid of this
void CSAA7134Card::CheckRegisters()
{
    static DWORD OldReport = 0;
    static DWORD OldStatus = 0;

    DWORD Report = ReadDword(SAA7134_IRQ_REPORT);
    DWORD Status = ReadDword(SAA7134_IRQ_STATUS);
    WriteDword(SAA7134_IRQ_REPORT, Report);

    if (Report == OldReport && Status == OldStatus)
        return;

    OldReport = Report;
    OldStatus = Status;

    LOG(0, "IRQ, Report: %08lx, Status: %08lx", Report, Status);

//  if (Report & SAA7134_IRQ_REPORT_DONE_RA0) {
        LOG(0, " | RA0=%s,%s,%s,%ld",
               (Status & 0x40) ? "vbi"  : "video",
               (Status & 0x20) ? "b"    : "a",
               (Status & 0x10) ? "odd"  : "even",
               (Status & 0x0f));
//  }
}


// Gets the field the card just finished processing (if IRQs work)
BOOL CSAA7134Card::GetIRQEventRegion(eRegionID& RegionID, BOOL& bIsFieldOdd)
{
    DWORD Report;
    DWORD Status;

    Report = ReadDword(SAA7134_IRQ_REPORT);
    Status = ReadDword(SAA7134_IRQ_STATUS);
    WriteDword(SAA7134_IRQ_REPORT, Report);

    if (Report == 0)
        return FALSE;

    BOOL bIsTaskA = (Status & 0x20);

    if (Status & 0x40)
        RegionID = bIsTaskA ? REGIONID_VBI_A : REGIONID_VBI_B;
    else
        RegionID = bIsTaskA ? REGIONID_VIDEO_A : REGIONID_VIDEO_B;

    bIsFieldOdd = (Status & 0x10);

	// Everything above is SAA7134 style, here we convert evens
	// to odd and odds to even so SAA7137Source can take even
	// has to top line --instead of odd which SAA7137 uses.
	//   ala. CSAACard::SetBaseOffsets()
	bIsFieldOdd = !bIsFieldOdd;

    return TRUE;
}

// Gets the field the card is currently processing
BOOL CSAA7134Card::GetProcessingRegion(eRegionID& RegionID, BOOL& bIsFieldOdd)
{
    WORD Status;

    Status  = ReadWord(SAA7134_SCALER_STATUS);

    if (Status & (SAA7134_SCALER_STATUS_TRERR |
        SAA7134_SCALER_STATUS_CFERR |
        SAA7134_SCALER_STATUS_LDERR |
        SAA7134_SCALER_STATUS_WASRST))
    {
        LOG(0, "Scaler Status, Error: %d", Status);
        return FALSE;
    }

    if (Status & SAA7134_SCALER_STATUS_D6_D5)
        return FALSE;

    if (Status & SAA7134_SCALER_STATUS_VID_A)
    {
        RegionID = REGIONID_VIDEO_A;
    }
    else if (Status & SAA7134_SCALER_STATUS_VID_B)
    {
        RegionID = REGIONID_VIDEO_B;
    }
    else
    {
        return FALSE;
    }

    //  Guessed this by looking at reg dump
    if (Status & SAA7134_SCALER_STATUS_FIDSCO)
    {
        bIsFieldOdd = TRUE;
    }
    else
    {
        bIsFieldOdd = FALSE;
    }

	// Everything above is SAA7134 style, here we convert evens
	// to odd and odds to even so SAA7137Source can take even
	// has to top line --instead of odd which SAA7137 uses.
	//   ala. CSAACard::SetBaseOffsets()
	bIsFieldOdd = !bIsFieldOdd;

    return TRUE;
}


/* might need this for future debugging
    static WORD OldStatus = 0;
    Status = ReadWord(SAA7134_STATUS_VIDEO);

    if (Status != OldStatus)
    {
        OldStatus = Status;
        LOG(0, "Video Status: %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
            (Status & SAA7134_STATUS_VIDEO_DCSTD0) ? "DCSTD0 " : "",
            (Status & SAA7134_STATUS_VIDEO_DCSCT1) ? "DCSCT1 " : "",
            (Status & SAA7134_STATUS_VIDEO_WIPA) ? "WIPA " : "",
            (Status & SAA7134_STATUS_VIDEO_GLIMB) ? "GLIMB " : "",
            (Status & SAA7134_STATUS_VIDEO_GLIMT) ? "GLIMT " : "",
            (Status & SAA7134_STATUS_VIDEO_SLTCA) ? "SLTCA " : "",
            (Status & SAA7134_STATUS_VIDEO_HLCK) ? "HLCK " : "",
            (Status & SAA7134_STATUS_VIDEO_RDCAP) ? "RDCAP " : "",
            (Status & SAA7134_STATUS_VIDEO_COPRO) ? "COPRO " : "",
            (Status & SAA7134_STATUS_VIDEO_COLSTR) ? "COLSTR " : "",
            (Status & SAA7134_STATUS_VIDEO_TYPE3) ? "TYPE3 " : "",
            (Status & SAA7134_STATUS_VIDEO_FIDT) ? "FIDT " : "",
            (Status & SAA7134_STATUS_VIDEO_HLVLN) ? "HLVLN " : "",
            (Status & SAA7134_STATUS_VIDEO_INTL) ? "INTL " : "",
            (Status & (1<<7)) ? "Unknown1 " : "",
            (Status & (1<<12)) ? "Unknown2" : ""
            );
    }
*/