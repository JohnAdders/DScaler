/////////////////////////////////////////////////////////////////////////////
// $Id: CT2388xCard.cpp,v 1.5 2002-09-16 19:34:18 adcockj Exp $
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.4  2002/09/15 14:20:38  adcockj
// Fixed timing problems for cx2388x chips
//
// Revision 1.3  2002/09/12 21:45:35  ittarnavsky
// changed the IAudioControls to CAudioControls
//
// Revision 1.2  2002/09/11 19:33:06  adcockj
// a few tidy ups
//
// Revision 1.1  2002/09/11 18:19:36  adcockj
// Prelimainary support for CT2388x based cards
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "CT2388xCard.h"
#include "CT2388x_Defines.h"
#include "DebugLog.h"
#include "CPU.h"
#include "TVFormats.h"
#include "NoTuner.h"
#include "MT2032.h"
#include "GenericTuner.h"


CCT2388xCard::CCT2388xCard(CHardwareDriver* pDriver) :
	CPCICard(pDriver),
    m_CardType(CT2388xCARD_CONEXANT_EVK),
    m_Tuner(NULL),
	m_SAA7118(NULL),
    m_RiscInSram(FALSE),
    m_RISCIsRunning(FALSE),
    m_RiscBasePhysical(0),
    m_CurrentInput(0)
{
    strcpy(m_TunerType,"n/a");

    m_I2CInitialized = false;
    m_I2CBus = new CI2CBusForLineInterface(this);
    m_AudioControls = new CAudioControls();
    m_AudioDecoder = new CAudioDecoder();
}

CCT2388xCard::~CCT2388xCard()
{
	delete m_I2CBus;
	delete m_AudioDecoder;
	delete m_AudioControls;
	delete m_Tuner;
	delete m_SAA7118;

    ClosePCICard();
}

void CCT2388xCard::CloseCard()
{
    ClosePCICard();
}

/*******************************************************************************
    Start the Risc Program.
    both_fields == true - Enable both fields.
                == false - Enable odd field only.
*******************************************************************************/
void CCT2388xCard::StartCapture(DWORD RiscBasePhysical)
{
    DWORD value1;
    DWORD value2;
    DWORD value3;
    DWORD dwval;

   
    // Clear Interrupt Status bits
    //BT848_WriteDword( MO_VID_INTSTAT, 0xFFFFFFFF );
    WriteDword( MO_VID_INTSTAT, 0x0000000 );
    
    value1 = ReadDword( MO_VID_DMACNTRL ) & 0xFFFFFFEE;
    value2 = (ReadDword( VID_CAPTURE_CONTROL ) & 0xFFFFFF00)| 0x06; // FIFO and capture control
    value3 = 1<<5; // RUN_RISC 

    WriteDword( SRAM_CMDS_21, RiscBasePhysical); // RISC STARTING ADDRESS 
    
    // Set or clear the ISRP bit to indicate whether the RISC progam is
    // in SRAM memory space or PCI memory space
    dwval = ReadDword(SRAM_CMDS_21 + 0x10);
    if (m_RiscInSram)
    {
        dwval |= 0x80000000;
    }
    else
    {
        dwval &= 0x7fffffff;
    }
    WriteDword( SRAM_CMDS_21 + 0x10, dwval); 

    WriteDword( MO_VID_DMACNTRL, value1 | 0x11 );     /* RISC/FIFO ENABLE */

    WriteDword( VID_CAPTURE_CONTROL, value2 );        /* CAPTURE CONTROL */

    // Clear Interrupt Status bits
    WriteDword( MO_VID_INTSTAT, 0xFFFFFFFF );

    m_RISCIsRunning = TRUE;
}

/*******************************************************************************
    Halt the Odd/Both Risc Program.
    both_fields == true - Halt both fields.
                == false - Halt odd field only.
*******************************************************************************/
void CCT2388xCard::StopCapture()
{
    DWORD value1;
    DWORD value2;
    DWORD value3;

    value1 = ReadDword( MO_VID_DMACNTRL ) & 0xFFFFFFEE;
    value2 = ReadDword( VID_CAPTURE_CONTROL ) & 0xFFFFFF00;
   
    value3 = 0;
    
    ::Sleep(100);

    // Original code before restart workaround
    WriteDword( MO_VID_DMACNTRL, value1 );            /* RISC/FIFO DISABLE */
    WriteDword( VID_CAPTURE_CONTROL, value2 );        /* CAPTURE CONTROL Off */

    m_RISCIsRunning = FALSE;
}

void CCT2388xCard::SetCardType(int CardType)
{
    if(m_CardType != CardType)
    {
        m_CardType = (eCT2388xCardId)CardType;

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

eCT2388xCardId CCT2388xCard::GetCardType()
{
    return m_CardType;
}

LPCSTR CCT2388xCard::GetCardName(eCT2388xCardId CardId)
{
    return m_TVCards[CardId].szName;
}

void CCT2388xCard::SetBrightness(BYTE Brightness)
{
    (*this.*m_TVCards[m_CardType].pSetBrightness)(Brightness);
}

void CCT2388xCard::SetHue(BYTE Hue)
{
    (*this.*m_TVCards[m_CardType].pSetHue)(Hue);
}

void CCT2388xCard::SetContrast(BYTE Contrast)
{
    (*this.*m_TVCards[m_CardType].pSetContrast)(Contrast);
}

void CCT2388xCard::SetSaturationU(BYTE SaturationU)
{
    (*this.*m_TVCards[m_CardType].pSetSaturationU)(SaturationU);
}

void CCT2388xCard::SetSaturationV(BYTE SaturationV)
{
    (*this.*m_TVCards[m_CardType].pSetSaturationV)(SaturationV);
}

void CCT2388xCard::StandardSetFormat(int nInput, eVideoFormat Format, BOOL IsProgressive)
{
    // do nothing
}

void CCT2388xCard::SetAnalogBrightness(BYTE Brightness)
{
    DWORD dwval = ReadDword(CT2388X_BRIGHT_CONTRAST);
    dwval &= 0xffffff00;
    Brightness = (BYTE)(unsigned char)(Brightness - 0x80);
    dwval |= (Brightness & 0xff);
    WriteDword(CT2388X_BRIGHT_CONTRAST,dwval);
}

void CCT2388xCard::SetAnalogHue(BYTE Hue)
{
    DWORD dwval = ReadDword(CT2388X_HUE);
    dwval &= 0xffffff00;
    Hue = (BYTE)(unsigned char)(Hue - 0x80);
    dwval |= (Hue & 0xff);
    WriteDword(CT2388X_HUE, dwval);
}

void CCT2388xCard::SetAnalogContrast(BYTE Contrast)
{
    DWORD dwval = ReadDword(CT2388X_BRIGHT_CONTRAST); // Brightness/contrast register
    dwval &= 0xffff00ff;
    dwval |= ((Contrast & 0xFF) << 8 );
    WriteDword(CT2388X_BRIGHT_CONTRAST,dwval);
}

void CCT2388xCard::SetAnalogSaturationU(BYTE SaturationU)
{
    DWORD dwval = ReadDword(CT2388X_UVSATURATION);
    dwval &= 0xffffff00;
    dwval |= (SaturationU & 0xff);
    WriteDword(CT2388X_UVSATURATION,dwval);
}

void CCT2388xCard::SetAnalogSaturationV(BYTE SaturationV)
{
    DWORD dwval = ReadDword(CT2388X_UVSATURATION);
    dwval &= 0xffff00ff;
    dwval |= ((SaturationV & 0xFF) << 8 );
    WriteDword(CT2388X_UVSATURATION,dwval);
}


void CCT2388xCard::SetVIPBrightness(BYTE Brightness)
{
    DWORD dwval = ReadDword(CT2388X_VIP_CONTBRGT);
    dwval &= 0xffffff00;
    Brightness = (BYTE)(unsigned char)(Brightness - 0x80);
    dwval |= (Brightness & 0xFF);
    WriteDword(CT2388X_VIP_CONTBRGT, dwval);
}

void CCT2388xCard::SetVIPContrast(BYTE Contrast)
{
    DWORD dwval = ReadDword(CT2388X_VIP_CONTBRGT);
    dwval &= 0xffff00ff;
    dwval |= ((Contrast & 0xFF) << 8 );
    WriteDword(CT2388X_VIP_CONTBRGT, dwval);
}

void CCT2388xCard::SetVIPSaturation(BYTE Saturation)
{
    DWORD dwval = ReadDword(CT2388X_VIP_SATURATION);
    dwval &= 0xffffff00;
    dwval |= (Saturation & 0xFF);
    WriteDword(CT2388X_VIP_SATURATION, dwval);
}


LPCSTR CCT2388xCard::GetTunerType()
{
    return m_TunerType;
}

// Sets up card to support size and format requested
// at the moment we insist on 720 pixel width.
void CCT2388xCard::SetGeoSize(int nInput, eVideoFormat TVFormat, long& CurrentX, long& CurrentY, int VDelayOverride, int HDelayOverride, BOOL IsProgressive)
{
    int HorzDelay;
    int VertDelay;
    int HorzScale;

    CurrentX = 720;
    CurrentY = GetTVFormat(TVFormat)->wCropHeight;

    if(IsCCIRSource(nInput))
    {
        CurrentX = 720;
		WriteByte(MO_PINMUX_IO, 0x02);

        SetPLL( 27e6, 2, FALSE );
        SetSampleRateConverter(28.63636E6);

        // Since we are digital here we don't really care which
        // format we choose as long as it has the right number of lines
		DWORD VideoInput = ReadDword(CT2388X_VIDEO_INPUT);
		VideoInput &= 0xfffffff0;

        if (CurrentY == 576)
        {
            // Enable PAL Mode (or SECAM)
            VideoInput |= VideoFormatPALBDGHI;
        }
        else
        {
            // Enable NTSC Mode (or PAL60)
            VideoInput |= VideoFormatNTSC;
        }

        WriteDword(CT2388X_VIDEO_INPUT, VideoInput);

        if(VDelayOverride != 0)
        {
            VertDelay = VDelayOverride;
        }
        else
        {
            if (CurrentY == 576)
            {
                VertDelay = 0x0E;
            }
            else
            {
                VertDelay = 0x0C;
            }
        }

        WriteDword(CT2388X_VDELAYCCIR_EVEN, 0x0c);
        WriteDword(CT2388X_VDELAYCCIR_ODD, 0x0e);
    }
    else
    {
        DWORD HTotal(0);

        CurrentX = 720;

        SetPLL( 27e6, 2, FALSE );
        SetSampleRateConverter(28.63636E6);

        // Setup correct format
        // \todo work out correct way of storing this with format info
		DWORD VideoInput = ReadDword(CT2388X_VIDEO_INPUT);
		VideoInput &= 0xfffffff0;

        switch(TVFormat)
        {
        case VIDEOFORMAT_PAL_B:
        case VIDEOFORMAT_PAL_D:
        case VIDEOFORMAT_PAL_G:
        case VIDEOFORMAT_PAL_H:
        case VIDEOFORMAT_PAL_I:
            VideoInput |= VideoFormatPALBDGHI;
            HTotal = HLNotchFilter135PAL | 864;
            break;
        case VIDEOFORMAT_PAL_N:
            VideoInput |= VideoFormatPALN;
            HTotal = HLNotchFilter135PAL | 864;
            break;
        case VIDEOFORMAT_PAL_M:
            VideoInput |= VideoFormatPALM;
            HTotal = HLNotchFilter135NTSC | 858;
            break;
        case VIDEOFORMAT_PAL_60:
            VideoInput |= VideoFormatPAL60;
            HTotal = HLNotchFilter135NTSC | 858;
            break;
        case VIDEOFORMAT_PAL_N_COMBO:
            VideoInput |= VideoFormatPALNC;
            HTotal = HLNotchFilter135PAL | 864;
            break;
        case VIDEOFORMAT_SECAM_B:
        case VIDEOFORMAT_SECAM_D:
        case VIDEOFORMAT_SECAM_G:
        case VIDEOFORMAT_SECAM_H:
        case VIDEOFORMAT_SECAM_K:
        case VIDEOFORMAT_SECAM_K1:
        case VIDEOFORMAT_SECAM_L:
        case VIDEOFORMAT_SECAM_L1:
            VideoInput |= VideoFormatSECAM;
            HTotal = HLNotchFilter135PAL | 864;
            break;
        case VIDEOFORMAT_NTSC_M:
            VideoInput |= VideoFormatNTSC;
            HTotal = HLNotchFilter135NTSC | 858;
            break;
        case VIDEOFORMAT_NTSC_M_Japan:
            VideoInput |= VideoFormatNTSCJapan;
            HTotal = HLNotchFilter135NTSC | 858;
            break;
        case VIDEOFORMAT_NTSC_50:
            VideoInput |= VideoFormatNTSC443;
            HTotal = HLNotchFilter135PAL | 864;
            break;
        default:
            VideoInput |= VideoFormatAuto;
            HTotal = HLNotchFilter135NTSC | 858;
            break;
        }

        WriteDword(CT2388X_VIDEO_INPUT, VideoInput);
        WriteDword(CT2388X_PIXEL_CNT_NOTCH, HTotal);

        // set up subcarrier frequency
        DWORD reg_value = (DWORD)( ((8 * GetTVFormat(TVFormat)->Fsc) / 27) * (double)(1<<22) + 0.5 );
        WriteDword( CT2388X_SUBCARRIERSTEP, reg_value & 0x7FFFFF );

        if(VDelayOverride != 0)
        {
            VertDelay = VDelayOverride;
        }
        else
        {
            if (CurrentY == 576)
            {
                VertDelay = 0x26;
            }
            else
            {
                VertDelay = 0x14;
            }
        }

        WriteDword(CT2388X_VERT_DELAY_EVEN, VertDelay);
        WriteDword(CT2388X_VERT_DELAY_ODD, VertDelay);
    }

    if(HDelayOverride != 0)
    {
        HorzDelay = HDelayOverride;
    }
    else
    {
        if (CurrentY == 576)
        {
            HorzDelay = 0x94;
        }
        else
        {
            HorzDelay = 0x7E;
        }
    }

    if (CurrentY == 576)
    {
        HorzScale = 0x0504;
    }
    else
    {
        HorzScale = 0x00F8;
    }


    WriteDword(CT2388X_HACTIVE_EVEN, CurrentX);
    WriteDword(CT2388X_HACTIVE_ODD, CurrentX);

    WriteDword(CT2388X_VACTIVE_EVEN, CurrentY);
    WriteDword(CT2388X_VACTIVE_ODD, CurrentY);

    WriteDword(CT2388X_HORZ_DELAY_EVEN, HorzDelay);
    WriteDword(CT2388X_HORZ_DELAY_ODD, HorzDelay);

    WriteDword(CT2388X_HSCALE_EVEN, HorzScale);
    WriteDword(CT2388X_HSCALE_ODD, HorzScale);

    WriteDword(CT2388X_VSCALE_EVEN, 0);
    WriteDword(CT2388X_VSCALE_ODD, 0);

    // call any card specific format setup
    (*this.*m_TVCards[m_CardType].pSetFormat)(nInput, TVFormat, IsProgressive);
}


BOOL CCT2388xCard::IsCCIRSource(int nInput)
{
    return (m_TVCards[m_CardType].Inputs[nInput].InputType == INPUTTYPE_CCIR);
}

BOOL CCT2388xCard::IsVideoPresent()
{
    DWORD dwval = ReadDword(CT2388X_DEVICE_STATUS);
    return ((dwval & CT2388X_DEVICE_STATUS_HLOCK) == CT2388X_DEVICE_STATUS_HLOCK);
}

DWORD CCT2388xCard::GetRISCPos()
{
    return ReadDword(0x0031C020);
}

void CCT2388xCard::ResetHardware()
{
    DWORD dwval,dwaddr;

    DWORD i,j;

    // \todo log what's there already for key settings

    // Clear out the SRAM Channel Management data structures
    // for all 12 devices
    for (i=1;i<=12;++i)
    {   
        dwaddr = 0x180000+i*0x40;
        for (j=0;j<5;++j)
        {
            WriteDword(dwaddr+(j*4),0);
        }
    }

    // Reset the chip
    WriteDword( 0x310304, 0x1 );

    ::Sleep(500);

    /* RISC Controller Enable */
    WriteDword( MO_DEV_CNTRL2, 1<<5 );

    ::Sleep(500);

    // Setup SRAM tables
        
    // Instruction Queue Base
    WriteDword(SRAM_CMDS_21 + 0x0c, SRAM_INSTRUCTION_QUEUE_BASE);
    
    // Instruction Queue Size
    dwval = ReadDword(SRAM_CMDS_21 + 0x10);
    dwval = (dwval & 0x80000000) | 0x30;  // 48 DWORD instruction queue (was 16)
    WriteDword(SRAM_CMDS_21 + 0x10, dwval);
    
    // Cluster table base 
    WriteDword(SRAM_CMDS_21 + 0x04, SRAM_CLUSTER_TABLE_BASE); 

    // Cluster table size is 6 QWORDS
    WriteDword(SRAM_CMDS_21 + 0x08, 0x06);    // 3 cluster buffers * 2 QWORDS/entry

    // Fill in cluster buffer entries
    WriteDword(SRAM_CLUSTER_TABLE_BASE, SRAM_CLUSTER_BUFFER_1); // Buffer one
    
    WriteDword(SRAM_CLUSTER_TABLE_BASE + 0x10,SRAM_CLUSTER_BUFFER_2); // Buffer two

    WriteDword(SRAM_CLUSTER_TABLE_BASE + 0x20,SRAM_CLUSTER_BUFFER_3); // Buffer three
    
    // Copy the cluster buffer info to the DMAC 
    
    // Set the DMA Cluster Table Address
    WriteDword( MO_DMA21_PTR2, SRAM_CLUSTER_TABLE_BASE );
    
    // Set the DMA buffer limit (size (number of INT64's))
    WriteDword( MO_DMA21_CNT1, (0x5A0 / 8) - 1 );
    
    // Set the DMA Cluster Table Size
    WriteDword( MO_DMA21_CNT2, 0x06 );


    // set format to YUY2
    MaskDataDword(CT2388X_VIDEO_COLOR_FORMAT, 0x00000044, 0x000000FF);

    // Test from Mike Asbury's regtool init code
    WriteDword( MO_PDMA_STHRSH, 0x0807 ); // Fifo source Threshhold
    WriteDword( MO_PDMA_DTHRSH, 0x0807 ); // Fifo Threshhold

    WriteDword( MO_VID_INTSTAT, 0xFFFFFFFF ); // Clear PIV int
    WriteDword( MO_PCI_INTSTAT, 0xFFFFFFFF ); // Clear PCI int
    WriteDword( MO_INT1_STAT, 0xFFFFFFFF );   // Clear RISC int

    //
    // Fixes for flashing suggested by Ben Felts
    //
    // 1.  Set bits 16:9 of register 0xE4310208 to 0x00.  
    //     The default value is 0x03803C0F, which becomes 0x0380000F with this change.
    WriteDword( 0x00310208, 0x0380000F ); 

    //2.  Set bits 27:26 of register 0xE4310200 to 0x0.  The default value is
    //    0x0CE00555, which becomes 0x00E00555 with this change.
    WriteDword( 0x00310200, 0x00E00555 ); 

    // Disable all of the interrupts
    WriteDword( MO_VID_INTMSK, 0x00000000 );
}    


LPCSTR CCT2388xCard::GetInputName(int nInput)
{
    if(nInput < m_TVCards[m_CardType].NumInputs && nInput >= 0)
    {
        return m_TVCards[m_CardType].Inputs[nInput].szName;
    }
    return "Error";
}

ULONG CCT2388xCard::GetTickCount()
{
    ULONGLONG ticks;
    ULONGLONG frequency;

    QueryPerformanceFrequency((PLARGE_INTEGER)&frequency);
    QueryPerformanceCounter((PLARGE_INTEGER)&ticks);
    ticks = (ticks & 0xFFFFFFFF00000000) / frequency * 10000000 +
            (ticks & 0xFFFFFFFF) * 10000000 / frequency;
    return (ULONG)(ticks / 10000);
}

void CCT2388xCard::InitializeI2C()
{
    WriteDword(0x368000, 1);
    m_I2CRegister = ReadDword(CT2338X_I2C);

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

void CCT2388xCard::Sleep()
{
    for (volatile DWORD i = m_I2CSleepCycle; i > 0; i--);
}

void CCT2388xCard::SetSDA(bool value)
{
    if (!m_I2CInitialized)
    {
        InitializeI2C();
    }
    if (value)
    {
        m_I2CRegister |= CT2338X_I2C_SDA;
    }
    else
    {
        m_I2CRegister &= ~CT2338X_I2C_SDA;
    }
    WriteDword(CT2338X_I2C, m_I2CRegister);
}

void CCT2388xCard::SetSCL(bool value)
{
    if (!m_I2CInitialized)
    {
        InitializeI2C();
    }
    if (value)
    {
        m_I2CRegister |= CT2338X_I2C_SCL;
    }
    else
    {
        m_I2CRegister &= ~CT2338X_I2C_SCL;
    }
    WriteDword(CT2338X_I2C, m_I2CRegister);
}

bool CCT2388xCard::GetSDA()
{
    if (!m_I2CInitialized)
    {
        InitializeI2C();
    }
    bool state = ReadDword(CT2338X_I2C) & CT2338X_I2C_SDA ? true : false;
    return state;
}

bool CCT2388xCard::GetSCL()
{
    if (!m_I2CInitialized)
    {
        InitializeI2C();
    }
    bool state = ReadDword(CT2338X_I2C) & CT2338X_I2C_SCL ? true : false;
    return state;
}



/*******************************************************************************
    Program the PLL to a specific output frequency.
*******************************************************************************/
void CCT2388xCard::SetPLL( double output_freq, int pre_scaler, BOOL integer_only )
{
    DWORD reg_value;
    double pll_value;
    int pll_int;
    int pll_frac;

    // Set Pre scaler bits
    if( pre_scaler <= 2 )
    {
        reg_value = 0;
        pre_scaler = 2;
    }
    else if( pre_scaler == 3 )
    {
        reg_value = 3<<26;
    }
    else if( pre_scaler == 4 )
    {
        reg_value = 2<<26;
    }
    else
    {
        reg_value = 1<<26;
        pre_scaler = 5;
    }
    
    pll_value = output_freq * 8.0 * (double)pre_scaler / 28.63636E6;

    // Handle integer only bits
    if( integer_only )
    {
        reg_value |= 1 << 28;
        pll_value = (int)(pll_value + 0.5);
    }

    pll_int = (int)pll_value;
    pll_frac = (int)((pll_value - (double)pll_int) * (double)(1<<20) + 0.5);

    // Check for illegal PLL values
    if( pll_int < 14 || pll_int > 63 )
    {
        return;
    }
    
    // Set register int and frac values
    reg_value |= pll_int << 20;
    reg_value |= pll_frac & 0xFFFFF;
    
    WriteDword(CT2388X_PLL , reg_value );
}

void CCT2388xCard::SetSampleRateConverter(double PLLFreq)
{
    DWORD reg_value = (int)(28.63636E6 / PLLFreq * (double)(1<<17) + 0.5);
    WriteDword( CT2388X_SAMPLERATECONV, reg_value & 0x7FFFF );
}

eTunerId CCT2388xCard::AutoDetectTuner(eCT2388xCardId CardId)
{
    return TUNER_ABSENT;
}

BOOL CCT2388xCard::InitTuner(eTunerId tunerId)
{
    // clean up if we get called twice
    if(m_Tuner != NULL)
    {
        delete m_Tuner; 
        m_Tuner = NULL;
    }

    switch (tunerId)
    {
    case TUNER_MT2032:
        // \todo fix this
        //m_Tuner = new CMT2032();
        //strcpy(m_TunerType, "MT2032 ");
        break;
    case TUNER_AUTODETECT:
    case TUNER_USER_SETUP:
    case TUNER_ABSENT:
        m_Tuner = new CNoTuner();
        strcpy(m_TunerType, "None ");
        break;
    default:
        m_Tuner = new CGenericTuner(tunerId);
        strcpy(m_TunerType, "Generic ");
        break;
    }
    if (tunerId != TUNER_ABSENT) 
    {
        int kk = strlen(m_TunerType);
        for (BYTE test = 0xC0; test < 0xCF; test +=2)
        {
            if (m_I2CBus->Write(&test, sizeof(test)))
            {
                m_Tuner->Attach(m_I2CBus, test>>1);
                sprintf(m_TunerType + kk, " @I2C@0x%02x", test);
                break;
            }
        }
    }
    return TRUE;
}

void CCT2388xCard::SetRISCStartAddress(DWORD RiscBasePhysical)
{
    WriteDword( SRAM_CMDS_21, RiscBasePhysical); // RISC STARTING ADDRESS

    // Set as PCI address
    DWORD dwval = ReadDword(SRAM_CMDS_21 + 0x10);
    dwval &= 0x7fffffff;
    WriteDword( SRAM_CMDS_21 + 0x10, dwval); 
}

ITuner* CCT2388xCard::GetTuner() const
{
    return m_Tuner;
}

#define DumpRegister(Reg) fprintf(hFile, #Reg "\t%08x\n", ReadDword(Reg))

void CCT2388xCard::DumpChipStatus()
{
    FILE* hFile;

    hFile = fopen("CT2388x.txt", "w");
    if(!hFile)
    {
        return;
    }

    fprintf(hFile, "SubSystemId\t%08x\n", m_SubSystemId);

    DumpRegister(CT2388X_DEVICE_STATUS);
    DumpRegister(CT2388X_VIDEO_INPUT);
    DumpRegister(CT2388X_TEMPORAL_DEC);
    DumpRegister(CT2388X_AGC_BURST_DELAY);
    DumpRegister(CT2388X_BRIGHT_CONTRAST); 
    DumpRegister(CT2388X_UVSATURATION);    
    DumpRegister(CT2388X_HUE);             
    DumpRegister(CT2388X_WHITE_CRUSH);
    DumpRegister(CT2388X_PIXEL_CNT_NOTCH);
    DumpRegister(CT2388X_HORZ_DELAY_EVEN);
    DumpRegister(CT2388X_HORZ_DELAY_ODD);
    DumpRegister(CT2388X_VERT_DELAY_EVEN);
    DumpRegister(CT2388X_VERT_DELAY_ODD);
    DumpRegister(CT2388X_VDELAYCCIR_EVEN);
    DumpRegister(CT2388X_VDELAYCCIR_ODD);
    DumpRegister(CT2388X_HACTIVE_EVEN);
    DumpRegister(CT2388X_HACTIVE_ODD);
    DumpRegister(CT2388X_VACTIVE_EVEN);    
    DumpRegister(CT2388X_VACTIVE_ODD);     
    DumpRegister(CT2388X_HSCALE_EVEN);     
    DumpRegister(CT2388X_HSCALE_ODD);      
    DumpRegister(CT2388X_VSCALE_EVEN);     
    DumpRegister(CT2388X_VSCALE_ODD);      
    DumpRegister(CT2388X_FILTER_EVEN);     
    DumpRegister(CT2388X_FILTER_ODD);      
    DumpRegister(CT2388X_FORMAT_2HCOMB);
    DumpRegister(CT2388X_PLL);
    DumpRegister(CT2388X_PLL_ADJUST);
    DumpRegister(CT2388X_SAMPLERATECONV);  
    DumpRegister(CT2388X_SAMPLERATEFIFO);  
    DumpRegister(CT2388X_SUBCARRIERSTEP);  
    DumpRegister(CT2388X_SUBCARRIERSTEPDR);
    DumpRegister(CT2388X_CAPTURECONTROL);  
    DumpRegister(CT2388X_VIDEO_COLOR_FORMAT);
    DumpRegister(CT2388X_VBI_SIZE);
    DumpRegister(CT2388X_FIELD_CAP_CNT);
    DumpRegister(CT2388X_VIP_CONFIG);
    DumpRegister(CT2388X_VIP_CONTBRGT);
    DumpRegister(CT2388X_VIP_HSCALE);
    DumpRegister(CT2388X_VIP_VSCALE);
    DumpRegister(CT2388X_VBOS);

    fclose(hFile);
}
