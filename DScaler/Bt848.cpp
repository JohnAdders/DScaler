/////////////////////////////////////////////////////////////////////////////
// $Id: Bt848.cpp,v 1.27 2001-08-08 16:39:17 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
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
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           Original Release
//                                     Removed use of WinDrvr
//
//  3 Nov 2000   Michael Eskin         Added override of initial BDELAY setting
//               Conexant Systems      by adding non-zero InitialBDelay in .ini
//                                     File. Changed NTSC defaults to 0x5C
//
// 02 Jan 2001   John Adcock           Made RISC Code linear
//
// 08 Jan 2001   John Adcock           Added C++ like access for strings
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 01 Mar 2001   Laurent Garnier       Auto-switch to default video format
//                                     when switching to tuner video input
//
// 11 Mar 2001   Laurent Garnier       Added WSS_Line in TTVFormat structure
//
// 31 Mar 2001   Laurent Garnier       Last used format saved per video input
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.26  2001/08/08 10:53:30  adcockj
// Preliminary changes to driver to support multiple cards
//
// Revision 1.25  2001/07/16 18:07:50  adcockj
// Added Optimisation parameter to ini file saving
//
// Revision 1.24  2001/07/13 18:13:24  adcockj
// Changed Mute to not be persisted and to work properly
//
// Revision 1.23  2001/07/13 16:14:55  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.22  2001/07/12 16:16:39  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "bt848.h"
#include "OutThreads.h"
#include "Audio.h"
#include "AspectRatio.h"
#include "Tuner.h"
#include "MixerDev.h"
#include "ProgramList.h"
#include "Other.h"
#include "VideoSettings.h"
#include "VBI.h"
#include "SettingsDlg.h"

/////////////////////////////////////////////////////////////////////////////
// Private Structures, funtions and data
/////////////////////////////////////////////////////////////////////////////
typedef struct
{
   DWORD dwMemoryBase;
   TPCICARDINFO PCICardInfo;
} TBT848;

TBT848* hBT8X8 = NULL;

BOOL BT848_Brightness_OnChange(long Brightness);
BOOL BT848_Hue_OnChange(long Hue);
BOOL BT848_Contrast_OnChange(long Contrast);
BOOL BT848_Saturation_OnChange(long Data);
BOOL BT848_SaturationU_OnChange(long Data);
BOOL BT848_SaturationV_OnChange(long Data);
BOOL BT848_BDelay_OnChange(long Data);
BOOL BT848_Overscan_OnChange(long Data);
BOOL BT848_Registers_OnChange();
BOOL BT848_WhiteCrushUp_OnChange(long NewValue);
BOOL BT848_WhiteCrushDown_OnChange(long NewValue);
BOOL CurrentX_OnChange(long NewValue);
BOOL VideoSource_OnChange(long NewValue);

PMemStruct RiscDMAMem;
PMemStruct VBIDMAMem[5];
PMemStruct DisplayDMAMem[5];

BYTE* pDisplay[5] = { NULL,NULL,NULL,NULL,NULL };
BYTE* pVBILines[5] = { NULL,NULL,NULL,NULL,NULL };

PHYS    RiscBasePhysical; 
DWORD*  RiscBaseLinear;
long BytesPerRISCField = 1;

char BTVendorID[10] = "";
char BTDeviceID[10] = "";
char BTChipType[10] = "";

// MAE 2 Nov 2000 - Start of change for Macrovision fix
// If non-zero in .ini file, will override TV table setting
long InitialBDelay = 0x00;  // Original hardware default Value was 0x5D
// MAE 2 Nov 2000 - End of change for Macrovision fix

BOOL    BtAgcDisable = FALSE;       // Luma AGC, 0 says AGC enabled
BOOL    BtCrush = TRUE;             // Adaptive AGC, 0 says Crush disabled
BOOL    BtEvenChromaAGC = TRUE;     // Even CAGC, 0 says CAGC disable
BOOL    BtOddChromaAGC = TRUE;      // Odd chroma AGC
BOOL    BtEvenLumaPeak = FALSE;     // Even Peak, 0 says normal, not Luma peak
BOOL    BtOddLumaPeak = FALSE;                          
BOOL    BtFullLumaRange = TRUE;     // Full Range Luma, 0=normal,1=full
                                    // should be 1 for NTSC
BOOL    BtEvenLumaDec = FALSE;      // Even Luma decimation,  0 says disable
BOOL    BtOddLumaDec = FALSE;
BOOL    BtEvenComb = TRUE;          // Even Comb, enabled
BOOL    BtOddComb = TRUE;
BOOL    BtColorBars = FALSE;        // Display Color Bars, 0 = no
BOOL    BtGammaCorrection = FALSE;  // Gamma Correction Removal, 0 = Enabled
BOOL    BtCoring = FALSE;           // Coring function: (0,1,2,or 3) << 5
BOOL    BtHorFilter = FALSE;        // Horizontal Filer: (0,1,2,3) << 3
                                    // maybe only 0,1 valid for full res?
BOOL    BtVertFilter = FALSE;       // Vert. Filter, only 0 and 4 valid here
BOOL    BtColorKill = TRUE;         // Kill color if B/W: (0,1) << 5
long    BtWhiteCrushUp = 0xcf;      // Crush up - entire register Value
long    BtWhiteCrushDown = 0x7f;    // Crush down - entire register Value

// MAE, 3 Nov 2000
//    Changed all BDELAY values from 5D to 5C for Macrovision fix
//
// John Adcock, 19 Dec 2000
//    Fixed PAL-N to stop it from crashing, improved PAL-M values
//    These were the old PAL-N Values that crashed dTV
//    // PAL-M 
//    { 754, 480,  910, 0x70, 0x5c, (BT848_IFORM_PAL_M|BT848_IFORM_XT0),
//        910, 754, 135, 754, 0x1a, 0, FALSE, 400},
//    // PAL-N 
//    { 922, 576, 1135, 0x7f, 0x72, (BT848_IFORM_PAL_N|BT848_IFORM_XT1),
//        1135, 922, 186, 922, 0x1c, 0, TRUE, 400},
//

TTVFormat TVFormats[FORMAT_LASTONE] =
{
    // PAL-BDGHI 
    { 
        "PAL", 576, 1135, 0x7f, 0x72, (BT848_IFORM_PAL_BDGHI|BT848_IFORM_XT1),
        186, 922, 0x24, 0, TRUE, 511, 19,
        4.43361875, TRUE, 71, 626, 15,
        16,
    },
    // NTSC 
    {
        "NTSC", 480, 910, 0x68, 0x5c, (BT848_IFORM_NTSC|BT848_IFORM_XT0),
        137, 754, 0x1a, 0, FALSE, 400, 13,
        3.579545,  FALSE, 57, 512, 11, 
        10,
    },
    // SECAM 
    {
        "SECAM", 576, 1135, 0x7f, 0xb0, (BT848_IFORM_SECAM|BT848_IFORM_XT1),
        186, 922, 0x22, 0, TRUE, 511, 19,
        4.43361875, TRUE, 71, 633, 15,
        16,
    },
    // PAL-M 
    {
        "PAL-M", 480,  910, 0x68, 0x5c, (BT848_IFORM_PAL_M|BT848_IFORM_XT0),
        137, 754, 0x1a, 0, FALSE, 400, 13,
        3.579545,  FALSE, 57, 512, 11,
        10,
    },
    // PAL-N 
    {
        "PAL-N", 576, 1135, 0x7f, 0x72, (BT848_IFORM_PAL_N|BT848_IFORM_XT1),
        186, 922, 0x20, 0, TRUE, 511, 19,
        4.43361875, TRUE,  71, 626, 15,
        16,
    },
    // NTSC Japan
    {
        "NTSC Japan", 480,  910, 0x70, 0x5c, (BT848_IFORM_NTSC_JAP|BT848_IFORM_XT0),
        135, 754, 0x1a, 0, FALSE, 400, 13,
        3.579545, FALSE, 57, 512, 11, 
        10,
    },
    // PAL-60 
    {
        "PAL60", 480, 1135, 0x7f, 0x72, (BT848_IFORM_PAL_BDGHI|BT848_IFORM_XT1),
        186, 922, 0x20, 0, FALSE, 400, 16,
        4.43361875, TRUE, 70, 626, 14,
        13,
    },
    // NTSC-50    
    {       
        "NTSC50", 576, 910, 0x68, 0x5c, (BT848_IFORM_NTSC|BT848_IFORM_XT0),
        137, 754, 0x24, 0, TRUE, 511, 19,
        3.579545,  FALSE, 71, 626, 15,      
        16, 
    },
};

const char* FormatList[FORMAT_LASTONE];

long TVFormat = -1;

// 10/19/2000 Mark Rejhon
// Better NTSC defaults
// These are the original defaults, likely optimized for PAL (could use refinement).
int InitialHue         = DEFAULT_HUE_NTSC;
int InitialBrightness  = DEFAULT_BRIGHTNESS_NTSC;
int InitialContrast    = DEFAULT_CONTRAST_NTSC;
int InitialSaturation  = (DEFAULT_SAT_U_NTSC + DEFAULT_SAT_V_NTSC) / 2;
int InitialSaturationU = DEFAULT_SAT_U_NTSC;
int InitialSaturationV = DEFAULT_SAT_V_NTSC;

long CurrentX = 720;
long CustomPixelWidth = 750;
int CurrentY;
int CurrentVBILines = 0;

eVideoSourceType VideoSource = SOURCE_COMPOSITE;

int HDelay = 0;
int VDelay = 0;

//===========================================================================
// CCIR656 Digital Input Support
//
// 13 Dec 2000 - Michael Eskin, Conexant Systems - Initial version
//
//===========================================================================
// Timing generator SRAM table values for CCIR601 720x480 NTSC
//===========================================================================

// For NTSC CCIR656 
BYTE SRAMTable_NTSC[ 60 ] =
{
    // SRAM Timing Table for NTSC
    0x33, 
    0x0c, 0xc0, 0x00, 
    0x00, 0x90, 0xc2, 
    0x03, 0x10, 0x03, 
    0x06, 0x10, 0x34, 
    0x12, 0x12, 0x65, 
    0x02, 0x13, 0x24, 
    0x19, 0x00, 0x24, 
    0x39, 0x00, 0x96, 
    0x59, 0x08, 0x93, 
    0x83, 0x08, 0x97,
    0x03, 0x50, 0x30, 
    0xc0, 0x40, 0x30, 
    0x86, 0x01, 0x01, 
    0xa6, 0x0d, 0x62, 
    0x03, 0x11, 0x61, 
    0x05, 0x37, 0x30, 
    0xac, 0x21, 0x50
};

// For PAL CCIR656
BYTE SRAMTable_PAL[ 60 ] =
{
    // SRAM Timing Table for PAL
    0x2d,
    0x36, 0x11, 0x01,
    0x00, 0x90, 0x02,
    0x05, 0x10, 0x04,
    0x16, 0x14, 0x05,
    0x11, 0x00, 0x04,
    0x12, 0xc0, 0x00,
    0x31, 0x00, 0x06,
    0x51, 0x08, 0x03,
    0x89, 0x08, 0x07,
    0xc0, 0x44, 0x00,
    0x81, 0x01, 0x01,
    0xa9, 0x0d, 0x02,
    0x02, 0x50, 0x03,
    0x37, 0x3d, 0x00,
    0xaf, 0x21, 0x00,
};

extern SETTING BT848Settings[BT848_SETTING_LASTONE];

/////////////////////////////////////////////////////////////////////////////
// Start of "real" code
/////////////////////////////////////////////////////////////////////////////

const char* BT848_VendorID()
{
    return BTVendorID;
}

const char* BT848_DeviceID()
{
    return BTDeviceID;
}

const char* BT848_ChipType()
{
    return BTChipType;
}

//-------------------------------
BOOL BT848_FindTVCard(HWND hWnd)
{
    int ret;

    strcpy(BTVendorID, "0x109e");
    strcpy(BTDeviceID, "0x036e");

    ret = BT848_Open(0x109e, 0x36e, TRUE, FALSE);
    if (ret == 0)
    {
        strcpy(BTChipType, "BT878");
    }
    else if (ret == 3)
    {
        ErrorBox("PCI-Card with Bt878 Cannot be locked");
        return (FALSE);
    }
    else
    {
        strcpy(BTVendorID, "0x109e");
        strcpy(BTDeviceID, "0x0350");
        ret = BT848_Open(0x109e, 0x350, TRUE, FALSE);
        if (ret == 0)
        {
            strcpy(BTChipType, "BT848");
        }
        else if (ret == 3)
        {
            ErrorBox("PCI-Card with Bt848 Cannot be locked");
            return (FALSE);
        }
        else
        {
            strcpy(BTVendorID, "0x109e");
            strcpy(BTDeviceID, "0x0351");
            ret = BT848_Open(0x109e, 0x351, TRUE, FALSE);
            if (ret == 0)
            {
                strcpy(BTChipType, "BT849");
            }
            else if (ret == 3)
            {
                ErrorBox("PCI-Card with Bt849 Cannot be locked");
                return (FALSE);
            }
            else
            {
                strcpy(BTVendorID, "0x109e");
                strcpy(BTDeviceID, "0x036F");
                ret = BT848_Open(0x109e, 0x36F, TRUE, FALSE);
                if (ret == 0)
                {
                    strcpy(BTChipType, "BT878a");
                }
                else if (ret == 3)
                {
                    ErrorBox("PCI-Card with Bt878a Cannot be locked");
                    return (FALSE);
                }
            }
        }
    }

    if (ret != 0)
    {
        return (FALSE);
    }

    return (TRUE);
}


//-------------------------------
void BT848_SaveSettings(LPCSTR szFileName)
{
    FILE* SettingFile;
    unsigned short i;

    if ((SettingFile = fopen(szFileName, "w")) != NULL)
    {
        fprintf(SettingFile, "BT848_COLOR_CTL %02x\n", BT848_ReadByte(BT848_COLOR_CTL));
        fprintf(SettingFile, "BT848_CAP_CTL %02x\n", BT848_ReadByte(BT848_CAP_CTL));
        fprintf(SettingFile, "BT848_VBI_PACK_SIZE %02x\n", BT848_ReadByte(BT848_VBI_PACK_SIZE));
        fprintf(SettingFile, "BT848_VBI_PACK_DEL %02x\n", BT848_ReadByte(BT848_VBI_PACK_DEL));
        fprintf(SettingFile, "BT848_GPIO_DMA_CTL %02x\n", BT848_ReadByte(BT848_GPIO_DMA_CTL));
        fprintf(SettingFile, "BT848_IFORM %02x\n", BT848_ReadByte(BT848_IFORM));

        fprintf(SettingFile, "BT848_E_SCLOOP %02x\n", BT848_ReadByte(BT848_E_SCLOOP));
        fprintf(SettingFile, "BT848_O_SCLOOP %02x\n", BT848_ReadByte(BT848_O_SCLOOP));
        fprintf(SettingFile, "BT848_ADELAY %02x\n", BT848_ReadByte(BT848_ADELAY));
        fprintf(SettingFile, "BT848_BDELAY %02x\n", BT848_ReadByte(BT848_BDELAY));

        fprintf(SettingFile, "BT848_E_HSCALE_HI %02x\n", BT848_ReadByte(BT848_E_HSCALE_HI));
        fprintf(SettingFile, "BT848_E_HSCALE_LO %02x\n", BT848_ReadByte(BT848_E_HSCALE_LO));
        fprintf(SettingFile, "BT848_E_VSCALE_HI %02x\n", BT848_ReadByte(BT848_E_VSCALE_HI));
        fprintf(SettingFile, "BT848_E_VSCALE_LO %02x\n", BT848_ReadByte(BT848_E_VSCALE_LO));
        fprintf(SettingFile, "BT848_E_HACTIVE_LO %02x\n", BT848_ReadByte(BT848_E_HACTIVE_LO));
        fprintf(SettingFile, "BT848_E_HDELAY_LO %02x\n", BT848_ReadByte(BT848_E_HDELAY_LO));
        fprintf(SettingFile, "BT848_E_VACTIVE_LO %02x\n", BT848_ReadByte(BT848_E_VACTIVE_LO));
        fprintf(SettingFile, "BT848_E_VDELAY_LO %02x\n", BT848_ReadByte(BT848_E_VDELAY_LO));
        fprintf(SettingFile, "BT848_E_CROP %02x\n", BT848_ReadByte(BT848_E_CROP));

        fprintf(SettingFile, "BT848_O_HSCALE_HI %02x\n", BT848_ReadByte(BT848_O_HSCALE_HI));
        fprintf(SettingFile, "BT848_O_HSCALE_LO %02x\n", BT848_ReadByte(BT848_O_HSCALE_LO));
        fprintf(SettingFile, "BT848_O_VSCALE_HI %02x\n", BT848_ReadByte(BT848_O_VSCALE_HI));
        fprintf(SettingFile, "BT848_O_VSCALE_LO %02x\n", BT848_ReadByte(BT848_E_VSCALE_LO));
        fprintf(SettingFile, "BT848_O_HACTIVE_LO %02x\n", BT848_ReadByte(BT848_O_HACTIVE_LO));
        fprintf(SettingFile, "BT848_O_HDELAY_LO %02x\n", BT848_ReadByte(BT848_O_HDELAY_LO));
        fprintf(SettingFile, "BT848_O_VACTIVE_LO %02x\n", BT848_ReadByte(BT848_O_VACTIVE_LO));
        fprintf(SettingFile, "BT848_O_VDELAY_LO %02x\n", BT848_ReadByte(BT848_O_VDELAY_LO));
        fprintf(SettingFile, "BT848_O_CROP %02x\n", BT848_ReadByte(BT848_O_CROP));

        fprintf(SettingFile, "BT848_PLL_F_LO %02x\n", BT848_ReadByte(BT848_PLL_F_LO));
        fprintf(SettingFile, "BT848_PLL_F_HI %02x\n", BT848_ReadByte(BT848_PLL_F_HI));
        fprintf(SettingFile, "BT848_PLL_XCI %02x\n", BT848_ReadByte(BT848_PLL_XCI));

        fprintf(SettingFile, "BT848_BRIGHT %02x\n", BT848_ReadByte(BT848_BRIGHT));
        fprintf(SettingFile, "BT848_CONTRAST_LO %02x\n", BT848_ReadByte(BT848_CONTRAST_LO));
        fprintf(SettingFile, "BT848_SAT_V_LO %02x\n", BT848_ReadByte(BT848_SAT_V_LO));
        fprintf(SettingFile, "BT848_SAT_U_LO %02x\n", BT848_ReadByte(BT848_SAT_U_LO));
        fprintf(SettingFile, "BT848_GPIO_OUT_EN %04x\n", BT848_ReadWord(BT848_GPIO_OUT_EN));
        fprintf(SettingFile, "BT848_GPIO_OUT_EN_HIBYTE %02x\n", BT848_ReadByte(BT848_GPIO_OUT_EN_HIBYTE));

        fprintf(SettingFile, "BT848_GPIO_REG_INP %04x\n", BT848_ReadWord(BT848_GPIO_REG_INP));
        fprintf(SettingFile, "BT848_GPIO_REG_INP_HIBYTE %02x\n", BT848_ReadByte(BT848_GPIO_REG_INP_HIBYTE));

        fprintf(SettingFile, "BT848_GPIO_DATA %04x\n", BT848_ReadWord(BT848_GPIO_DATA));
        fprintf(SettingFile, "BT848_GPIO_DATA_HIBYTE %02x\n", BT848_ReadByte(BT848_GPIO_DATA_HIBYTE));
        i = ((BT848_ReadByte(BT848_GPIO_OUT_EN_HIBYTE)) << 16) + BT848_ReadWord(BT848_GPIO_OUT_EN);
        fprintf(SettingFile, "*********************************************\n");
        fprintf(SettingFile, "Ausgelesene Einträge für Eigenen KartenTyp\n");
        fprintf(SettingFile, "Eintrag für BT848_GPIO_OUT_EN  %9d     ( Schaltwert )\n", i);
        i = ((BT848_ReadByte(BT848_GPIO_REG_INP_HIBYTE)) << 16) + BT848_ReadWord(BT848_GPIO_REG_INP);
        fprintf(SettingFile, "Eintrag für BT848_GPIO_REG_INP %9d     ( Input-Control )\n", i);
        i = ((BT848_ReadByte(BT848_GPIO_DATA_HIBYTE)) << 16) + BT848_ReadWord(BT848_GPIO_DATA);
        fprintf(SettingFile, "Eintrag für BT848_GPIO_DATA    %9d     ( Eingangswunsch) \n", i);
        fprintf(SettingFile, "*********************************************\n");
        fclose(SettingFile);
    }
}

//-------------------------------
DWORD BT848_GetSubSystemID()
{
    return hBT8X8->PCICardInfo.dwSubSystemId;
}

//-------------------------------
BOOL BT848_MemoryInit(void)
{
    int i;

    if (!Alloc_DMA(83968, &RiscDMAMem, ALLOC_MEMORY_CONTIG))
    {
        ErrorBox("Risc Memory (83 KB Contiguous) not Allocated");
        return (FALSE);
    }

    RiscBaseLinear = (DWORD*)RiscDMAMem->dwUser;
    RiscBasePhysical = GetPhysicalAddress(RiscDMAMem, (BYTE*)RiscDMAMem->dwUser, 83968, NULL);
    
    for (i = 0; i < 5; i++)
    {
        // JA 02/01/2001
        // Allocate some extra memory so that we can skip
        // start of buffer that is not page aligned
        if (!Alloc_DMA(2048 * 19 * 2 + 4095, &VBIDMAMem[i], 0))
        {
            ErrorBox("VBI Memory for DMA not allocated");
            return (FALSE);
        }
        pVBILines[i] = GetFirstFullPage(VBIDMAMem[i]);
        // JA 29/12/2000
        // Allocate some extra memory so that we can skip
        // start of buffer that is not page aligned
        if (!Alloc_DMA(1024 * 576 * 2 + 4095, &DisplayDMAMem[i], 0))
        {
            ErrorBox("Display Memory for DMA not allocated");
            return (FALSE);
        }
        pDisplay[i] = GetFirstFullPage(DisplayDMAMem[i]);
    }

    return (TRUE);
}

//-------------------------------
void BT848_MemoryFree()
{
    int i;
    Free_DMA(&RiscDMAMem);
    for(i = 0; i < 5; i++)
    {
        Free_DMA(&VBIDMAMem[i]);
        Free_Display_DMA(i);
    }
}

//-------------------------------
void BT848_Restart_RISC_Code()
{
    BYTE CapCtl = BT848_ReadByte(BT848_CAP_CTL);
    BT848_MaskDataByte(BT848_CAP_CTL, 0, (BYTE) 0x0f);
    BT848_WriteDword(BT848_INT_STAT, (DWORD) 0x0fffffff);
    BT848_WriteDword(BT848_RISC_STRT_ADD, RiscBasePhysical);
    BT848_WriteByte(BT848_CAP_CTL, CapCtl);
}

//-------------------------------
int BT848_CCIRsource()
{
    switch (VideoSource)
    {
    case SOURCE_CCIR656_1:
    case SOURCE_CCIR656_2:
    case SOURCE_CCIR656_3:
    case SOURCE_CCIR656_4:
        return TRUE;
    }
    return FALSE;
}

//-------------------------------
int BT848_IsPAL()
{
    switch (TVFormat)
    {
    case FORMAT_PAL_BDGHI:
    case FORMAT_PAL_M:
    case FORMAT_PAL_N:
    case FORMAT_SECAM:  // Okay, this one is not PAL but has same number of scanlines
    case FORMAT_NTSC50: // Okay, this one is not PAL but has same number of scanlines
        return TRUE;
    }
    return FALSE;
}

//-------------------------------
int BT848_IsNTSC()
{
    switch (TVFormat)
    {
    case FORMAT_NTSC:
    case FORMAT_NTSC_J:
    case FORMAT_PAL60:  // Okay, this one is not NTSC but has same number of scanlines
        return TRUE;
    }
    return FALSE;
}

//-------------------------------
void BT848_ResetHardware()
{
    BT848_SetDMA(FALSE);
    BT848_WriteByte(BT848_SRESET, 0);
    Sleep(100);

    BT848_WriteDword(BT848_RISC_STRT_ADD, RiscBasePhysical);
    BT848_WriteByte(BT848_CAP_CTL, 0x00);
    BT848_WriteByte(BT848_VBI_PACK_SIZE, (VBI_SPL / 4) & 0xff);
    BT848_WriteByte(BT848_VBI_PACK_DEL, (VBI_SPL / 4) >> 8);
    BT848_WriteWord(BT848_GPIO_DMA_CTL, 0xfc);
    BT848_WriteByte(BT848_IFORM, BT848_IFORM_MUX1 | BT848_IFORM_XTAUTO | BT848_IFORM_PAL_BDGHI);

    BT848_SetVideoSource(VideoSource);
    BT848_SetGeoSize();
    
    BT848_WriteByte(BT848_TDEC, 0x00);

// MAE 2 Nov 2000 - Start of change for Macrovision fix
    if (InitialBDelay != 0)
    {
        // BDELAY override from .ini file
        BT848_BDelay_OnChange(InitialBDelay);
    }
// MAE 2 Nov 2000 - End of change for Macrovision fix

    BT848_WriteDword(BT848_INT_STAT, (DWORD) 0x0fffffff);
    BT848_WriteDword(BT848_INT_MASK, 0);

    BT848_SetPLL(PLL_NONE);

    if (BT848_CCIRsource()) BT848_Enable656();
    BT848_Registers_OnChange();
    BT848_Brightness_OnChange(InitialBrightness);
    BT848_Contrast_OnChange(InitialContrast);
    BT848_Hue_OnChange(InitialHue);
    BT848_SaturationU_OnChange(InitialSaturationU);
    BT848_SaturationV_OnChange(InitialSaturationV);
}

//-------------------------------
PHYS RiscLogToPhys(DWORD* pLog)
{
    return (RiscBasePhysical + (pLog - RiscBaseLinear) * 4);
}

//-------------------------------
// JA 17012001 Updated to do exactly what it says in the bt848 docs
void BT848_SetPLL(ePLLFreq PLL)
{
    int i = 6;

    // reset the TGCKI bits
    BT848_MaskDataByte(BT848_TGCTRL, BT848_TGCTRL_TGCKI_NOPLL, 0x18);

    switch(PLL)
    {
    case PLL_NONE:
        BT848_WriteByte(BT848_PLL_XCI, 0x00);
        return;
        break;
    case PLL_28:
        BT848_WriteByte(BT848_PLL_F_LO, 0xf9);
        BT848_WriteByte(BT848_PLL_F_HI, 0xdc);
        BT848_WriteByte(BT848_PLL_XCI, 0x8e);
        break;
    case PLL_35:
        BT848_WriteByte(BT848_PLL_F_LO, 0x39);
        BT848_WriteByte(BT848_PLL_F_HI, 0xB0);
        BT848_WriteByte(BT848_PLL_XCI, 0x89);
        break;
    }

    // wait for the PLL to lock
    while(i-- > 0 && BT848_ReadByte(BT848_DSTATUS) & BT848_DSTATUS_PLOCK)
    {
        BT848_WriteByte(BT848_DSTATUS, 0x00);
        Sleep(100);
    }

    // Set the TGCKI bits to use PLL rather than xtal
    BT848_MaskDataByte(BT848_TGCTRL, BT848_TGCTRL_TGCKI_PLL, 0x18);

    BT848_WhiteCrushUp_OnChange(BtWhiteCrushUp);
    BT848_WhiteCrushDown_OnChange(BtWhiteCrushDown);
    BT848_WriteByte(BT848_VTOTAL_LO, 0x00);
    BT848_WriteByte(BT848_VTOTAL_HI, 0x00);
    BT848_WriteByte(BT848_DVSIF, 0x00);
}

//-------------------------------
void BT848_SetDMA(BOOL bState)
{
    if (bState)
    {
        BT848_OrDataWord(BT848_GPIO_DMA_CTL, 3);
    }
    else
    {
        BT848_AndDataWord(BT848_GPIO_DMA_CTL, ~3);
    }
}

//-------------------------------
BOOL BT848_SetGeoSize()
{
    int vscale, hscale;
    DWORD sr;
    int hdelay, vdelay;
    int hactive, vactive;
    BYTE crop, vtc, ColourFormat;

    // Not currently supported for CCIR656 source yet
    if (BT848_CCIRsource()) return TRUE;

    CurrentY = TVFormats[TVFormat].wCropHeight;
    CurrentVBILines = TVFormats[TVFormat].VBILines;

    // set the pll on the card if appropriate
    if(TVFormats[TVFormat].NeedsPLL == TRUE && GetCardSetup()->pll != PLL_NONE)
    {
        BT848_SetPLL(GetCardSetup()->pll);
    }
    else
    {
        BT848_SetPLL(PLL_NONE);
    }

    BT848_WriteByte(BT848_ADELAY, TVFormats[TVFormat].bDelayA);
    BT848_WriteByte(BT848_BDELAY, TVFormats[TVFormat].bDelayB);

    BT848_WriteByte(BT848_VBI_PACK_SIZE, (BYTE)(TVFormats[TVFormat].VBIPacketSize & 0xff));
    BT848_WriteByte(BT848_VBI_PACK_DEL, (BYTE)(TVFormats[TVFormat].VBIPacketSize >> 8));

    BT848_MaskDataByte(BT848_IFORM, TVFormats[TVFormat].bIForm, BT848_IFORM_NORM | BT848_IFORM_XTBOTH);

    ColourFormat = (BYTE)((BT848_COLOR_FMT_YUY2 << 4) | BT848_COLOR_FMT_YUY2);

    BT848_WriteByte(BT848_COLOR_FMT, ColourFormat);

// MAE 2 Nov 2000 - Start of change for Macrovision fix
    if (InitialBDelay != 0)
    {
        // BDELAY override from .ini file
        BT848_BDelay_OnChange(InitialBDelay);
    }
// MAE 2 Nov 2000 - End of change for Macrovision fix
    BT848_Registers_OnChange();

    hactive = CurrentX & ~2;

    vtc = BtVertFilter?BT848_VTC_VFILT_2TAPZ:0;     
    if(CurrentX <= TVFormats[TVFormat].wHActivex1)
    {
        hscale = ((TVFormats[TVFormat].wHActivex1 - CurrentX) * 4096UL) / CurrentX;
    }
    else
    {
        CurrentX = TVFormats[TVFormat].wHActivex1;
        hscale = 0;
    }
    if(VDelay == 0)
    {
        vdelay = TVFormats[TVFormat].wVDelay;
    }
    else
    {
        vdelay = VDelay;
    }
    if(HDelay == 0)
    {
        hdelay = ((CurrentX * TVFormats[TVFormat].wHDelayx1) / TVFormats[TVFormat].wHActivex1) & 0x3fe;
    }
    else
    {
        hdelay = ((CurrentX * HDelay) / TVFormats[TVFormat].wHActivex1) & 0x3fe;
    }

    if(TVFormat == FORMAT_PAL60)
    {
        BT848_WriteByte(BT848_VTOTAL_LO, (BYTE)(525 & 0xff));
        BT848_WriteByte(BT848_VTOTAL_HI, (BYTE)(525 >> 8));
    }
    else if(TVFormat == FORMAT_NTSC50)
    {
        BT848_WriteByte(BT848_VTOTAL_LO, (BYTE)(625 & 0xff));
        BT848_WriteByte(BT848_VTOTAL_HI, (BYTE)(625 >> 8));
    }


    sr = (TVFormats[TVFormat].wCropHeight * 512) / CurrentY - 512;
    vscale = (WORD) (0x10000UL - sr) & 0x1fff;
    vactive = TVFormats[TVFormat].wCropHeight;
    crop = ((hactive >> 8) & 0x03) | ((hdelay >> 6) & 0x0c) | ((vactive >> 4) & 0x30) | ((vdelay >> 2) & 0xc0);

    BT848_SetGeometryEvenOdd(FALSE, vtc, hscale, vscale, hactive, vactive, hdelay, vdelay, crop);
    BT848_SetGeometryEvenOdd(TRUE, vtc, hscale, vscale, hactive, vactive, hdelay, vdelay, crop);

    return TRUE;
}

BOOL BT848_Brightness_OnChange(long Brightness)
{
    BT848_WriteByte(BT848_BRIGHT, (BYTE) (Brightness & 0xff));
    InitialBrightness = Brightness;
    return FALSE;
}

BOOL BT848_WhiteCrushUp_OnChange(long NewValue)
{
    BtWhiteCrushUp = NewValue;
    BT848_WriteByte(BT848_WC_UP, (BYTE)BtWhiteCrushUp);         // TRB 12/00 allow parm
    return FALSE;
}

BOOL BT848_WhiteCrushDown_OnChange(long NewValue)
{
    BtWhiteCrushDown = NewValue;
    BT848_WriteByte(BT848_WC_DOWN, (BYTE)BtWhiteCrushDown);     // TRB 12/00 allow parm
    return FALSE;
}

BOOL BT848_Hue_OnChange(long Hue)
{
    BT848_WriteByte(BT848_HUE, (BYTE) (Hue & 0xff));
    InitialHue = Hue;
    return FALSE;
}

BOOL BT848_Contrast_OnChange(long Contrast)
{
    BYTE bContHi;

    bContHi = (BYTE) (Contrast >> 6) & 4;
    BT848_WriteByte(BT848_CONTRAST_LO, (BYTE) (Contrast & 0xff));
    BT848_MaskDataByte(BT848_E_CONTROL, bContHi, 4);
    BT848_MaskDataByte(BT848_O_CONTROL, bContHi, 4);
    InitialContrast = Contrast;
    return FALSE;
}

BOOL BT848_Saturation_OnChange(long Sat)
{
    long NewSaturationU = InitialSaturationU;
    long NewSaturationV = InitialSaturationV;
    long OldSaturation = (InitialSaturationU + InitialSaturationV) / 2;
    NewSaturationU += Sat - OldSaturation;
    NewSaturationV += Sat - OldSaturation;
    BT848_SaturationU_OnChange(NewSaturationU);
    BT848_SaturationV_OnChange(NewSaturationV);
    return TRUE;
}


BOOL BT848_SaturationU_OnChange(long SatU)
{
    if(SatU > 255) SatU = 255;
    if(SatU < 0) SatU = 0;

    BYTE bDataHi;
    bDataHi = (BYTE) (SatU >> 7) & 2;
    BT848_WriteByte(BT848_SAT_U_LO, (BYTE) (SatU & 0xff));
    BT848_MaskDataByte(BT848_E_CONTROL, bDataHi, 2);
    BT848_MaskDataByte(BT848_O_CONTROL, bDataHi, 2);
    InitialSaturationU = SatU;
    InitialSaturation = (InitialSaturationU + InitialSaturationV) / 2;
    BT848Settings[SATURATION].MinValue = abs(InitialSaturationU - InitialSaturationV) / 2;
    BT848Settings[SATURATION].MaxValue = 255 - abs(InitialSaturationU - InitialSaturationV) / 2;
    return TRUE;
}

BOOL BT848_SaturationV_OnChange(long SatV)
{
    if(SatV > 255) SatV = 255;
    if(SatV < 0) SatV = 0;

    BYTE bDataHi;

    bDataHi = (BYTE) (SatV >> 8) & 1;
    BT848_WriteByte(BT848_SAT_V_LO, (BYTE) (SatV & 0xff));
    BT848_MaskDataByte(BT848_E_CONTROL, bDataHi, 1);
    BT848_MaskDataByte(BT848_O_CONTROL, bDataHi, 1);
    InitialSaturationV = SatV;
    InitialSaturation = (InitialSaturationU + InitialSaturationV) / 2;
    BT848Settings[SATURATION].MinValue = abs(InitialSaturationU - InitialSaturationV) / 2;
    BT848Settings[SATURATION].MaxValue = 255 - abs(InitialSaturationU - InitialSaturationV) / 2;
    return TRUE;
}

BOOL BT848_BDelay_OnChange(long BDelay)
{
    InitialBDelay = BDelay;
    if (BDelay > 0) 
    {
        if (InitialBDelay == 0) 
        {
            // We use automatic BDelay if InitialBDelay is 0
            Reset_Capture();
            BT848Settings[BDELAY].szDisplayName = "Macrovision Timing AUTO";
        }
        else
        {
            BT848_WriteByte(BT848_BDELAY, (BYTE)BDelay);
            BT848Settings[BDELAY].szDisplayName = "Macrovision Timing";
        }
    }
    return FALSE;
}

BOOL BT848_Registers_OnChange()
{
    BYTE bNewValue;

    // use mask here as we don't want to disturb the s-video stuff
    bNewValue =  BtEvenLumaDec?BT848_CONTROL_LDEC:0;
    BT848_MaskDataByte(BT848_E_CONTROL, bNewValue, BT848_CONTROL_LDEC);

    // use mask here as we don't want to disturb the s-video stuff
    bNewValue =  BtOddLumaDec?BT848_CONTROL_LDEC:0;
    BT848_MaskDataByte(BT848_O_CONTROL, bNewValue, BT848_CONTROL_LDEC);

    bNewValue =  BtEvenChromaAGC?BT848_SCLOOP_CAGC:0;
    bNewValue |= BtEvenLumaPeak?BT848_SCLOOP_LUMA_PEAK:0;
    bNewValue |= BtColorKill?BT848_SCLOOP_CKILL:0;
    bNewValue |= BtHorFilter?BT848_SCLOOP_HFILT_FULL:0;
    BT848_WriteByte(BT848_E_SCLOOP, bNewValue);
    
    bNewValue =  BtOddChromaAGC?BT848_SCLOOP_CAGC:0;
    bNewValue |= BtOddLumaPeak?BT848_SCLOOP_LUMA_PEAK:0;
    bNewValue |= BtColorKill?BT848_SCLOOP_CKILL:0;
    bNewValue |= BtHorFilter?BT848_SCLOOP_HFILT_FULL:0;
    BT848_WriteByte(BT848_O_SCLOOP, bNewValue);

    bNewValue =  BtFullLumaRange?BT848_OFORM_RANGE:0;
    bNewValue |= BtCoring?BT848_OFORM_CORE32:0;
    BT848_WriteByte(BT848_OFORM, bNewValue);
    
    bNewValue =  BtEvenComb?BT848_VSCALE_COMB:0;
    BT848_WriteByte(BT848_E_VSCALE_HI, bNewValue);

    bNewValue =  BtOddComb?BT848_VSCALE_COMB:0;
    BT848_WriteByte(BT848_O_VSCALE_HI, bNewValue);
    
    bNewValue =  BT848_ADC_RESERVED;
    bNewValue |= BtAgcDisable?BT848_ADC_AGC_EN:0;
    bNewValue |= BtCrush?BT848_ADC_CRUSH:0;
    BT848_WriteByte(BT848_ADC, bNewValue); 

    bNewValue =  BtColorBars?BT848_COLOR_CTL_COLOR_BARS:0;
    bNewValue |= BtGammaCorrection?BT848_COLOR_CTL_GAMMA:0;
    BT848_WriteByte(BT848_COLOR_CTL, bNewValue);

    bNewValue =  BtVertFilter?BT848_VTC_VFILT_2TAPZ:0;
    BT848_MaskDataByte(BT848_E_VTC, bNewValue, BT848_VTC_VFILT_2TAPZ);
    BT848_MaskDataByte(BT848_O_VTC, bNewValue, BT848_VTC_VFILT_2TAPZ);

    return FALSE;
}


BOOL BT848_SetVideoSource(eVideoSourceType nInput)
{
    DWORD MuxSel;
    // 0= Tuner,
    // 1= Composite,
    // 2= SVideo,
    // 3= Other 1
    // 4= Other 2
    // 5= Composite via SVideo

    BT848_AndOrDataDword(BT848_GPIO_OUT_EN, GetCardSetup()->GPIOMuxMask, ~GetCardSetup()->GPIOMuxMask);
    BT848_AndDataByte(BT848_IFORM, ~BT848_IFORM_MUXSEL);

    // set the comp bit for svideo
    switch (nInput)
    {
    case SOURCE_TUNER:
        BT848_AndDataByte(BT848_E_CONTROL, ~BT848_CONTROL_COMP);
        BT848_AndDataByte(BT848_O_CONTROL, ~BT848_CONTROL_COMP);
        MuxSel = GetCardSetup()->MuxSelect[GetCardSetup()->TunerInput & 7];
        break;
    case SOURCE_SVIDEO:
        BT848_OrDataByte(BT848_E_CONTROL, BT848_CONTROL_COMP);
        BT848_OrDataByte(BT848_O_CONTROL, BT848_CONTROL_COMP);
        MuxSel = GetCardSetup()->MuxSelect[GetCardSetup()->SVideoInput & 7];
        break;
    case SOURCE_COMPVIASVIDEO:
        BT848_AndDataByte(BT848_E_CONTROL, ~BT848_CONTROL_COMP);
        BT848_AndDataByte(BT848_O_CONTROL, ~BT848_CONTROL_COMP);
        MuxSel = GetCardSetup()->MuxSelect[GetCardSetup()->SVideoInput & 7];
        break;
    case SOURCE_COMPOSITE:
    case SOURCE_OTHER1:
    case SOURCE_OTHER2:
    case SOURCE_CCIR656_1:
    case SOURCE_CCIR656_2:
    case SOURCE_CCIR656_3:
    case SOURCE_CCIR656_4:
    default:
        BT848_AndDataByte(BT848_E_CONTROL, ~BT848_CONTROL_COMP);
        BT848_AndDataByte(BT848_O_CONTROL, ~BT848_CONTROL_COMP);
        MuxSel = GetCardSetup()->MuxSelect[nInput];
        break;
    }
    
    BT848_MaskDataByte(BT848_IFORM, (BYTE) (((MuxSel) & 3) << 5), BT848_IFORM_MUXSEL);
    BT848_AndOrDataDword(BT848_GPIO_DATA, MuxSel >> 4, ~GetCardSetup()->GPIOMuxMask);
    return TRUE;
}

void BT848_SetGeometryEvenOdd(BOOL bOdd, BYTE bVtc, int wHScale, int wVScale, int wHActive, int wVActive, int wHDelay, int wVDelay, BYTE bCrop)
{
    int nOff = bOdd ? 0x80 : 0x00;

    BT848_WriteByte(BT848_E_VTC + nOff, bVtc);
    BT848_WriteByte(BT848_E_HSCALE_HI + nOff, (BYTE) (wHScale >> 8));
    BT848_WriteByte(BT848_E_HSCALE_LO + nOff, (BYTE) (wHScale & 0xFF));
    BT848_MaskDataByte(BT848_E_VSCALE_HI + nOff, (BYTE) (wVScale >> 8), 0x1F);
    BT848_WriteByte(BT848_E_VSCALE_LO + nOff, (BYTE) (wVScale & 0xFF));
    BT848_WriteByte(BT848_E_HACTIVE_LO + nOff, (BYTE) (wHActive & 0xFF));
    BT848_WriteByte(BT848_E_HDELAY_LO + nOff, (BYTE) (wHDelay & 0xFF));
    BT848_WriteByte(BT848_E_VACTIVE_LO + nOff, (BYTE) (wVActive & 0xFF));
    BT848_WriteByte(BT848_E_VDELAY_LO + nOff, (BYTE) (wVDelay & 0xFF));
    BT848_WriteByte(BT848_E_CROP + nOff, bCrop);
}

BOOL BT848_IsVideoPresent()
{
    return ((BT848_ReadByte(BT848_DSTATUS) & (BT848_DSTATUS_PRES | BT848_DSTATUS_HLOC)) == (BT848_DSTATUS_PRES | BT848_DSTATUS_HLOC)) ? TRUE : FALSE;
}

// Creates the RISC code
// First syncs to field
// Then waits for data
// Then tells the bt848 where to put each line of data
void BT848_CreateRiscCode(int nFlags)
{
    DWORD* pRiscCode;
    WORD nField;
    WORD nLine;
    LPBYTE pUser;
    PHYS pPhysical;
    DWORD GotBytesPerLine;
    DWORD BytesPerLine = 0;

    pRiscCode = (DWORD*)RiscDMAMem->dwUser;
    // we create the RISC code for 10 fields
    // the first one (0) is even
    // last one (9) is odd
    for(nField = 0; nField < 10; nField++)
    {
        // First we sync onto either the odd or even field
        if(nField & 1)
        {
            *(pRiscCode++) = (DWORD) (BT848_RISC_SYNC | BT848_RISC_RESYNC | BT848_FIFO_STATUS_VRO);
        }
        else
        {
            *(pRiscCode++) = (DWORD) (BT848_RISC_SYNC | BT848_RISC_RESYNC | BT848_FIFO_STATUS_VRE);
        }
        *(pRiscCode++) = 0;

        // Create VBI code of required
        if (bCaptureVBI)
        {
            *(pRiscCode++) = (DWORD) (BT848_RISC_SYNC | BT848_FIFO_STATUS_FM1);
            *(pRiscCode++) = 0;

            pUser = pVBILines[nField / 2];
            if((nField & 1) == 1)
            {
                pUser += CurrentVBILines * 2048;
            }
            for (nLine = 0; nLine < CurrentVBILines; nLine++)
            {
                pPhysical = GetPhysicalAddress(VBIDMAMem[nField / 2], pUser, VBI_SPL, &GotBytesPerLine);
                if(pPhysical == 0 || VBI_SPL > GotBytesPerLine)
                {
                    return;
                }
                *(pRiscCode++) = BT848_RISC_WRITE | BT848_RISC_SOL | BT848_RISC_EOL | VBI_SPL;
                *(pRiscCode++) = pPhysical;
                pUser += 2048;
            }
        }

        *(pRiscCode++) = (DWORD) (BT848_RISC_SYNC | BT848_FIFO_STATUS_FM1);
        *(pRiscCode++) = 0;


        // work out the position of the first line
        // first line is line zero an even line
        pUser = pDisplay[nField / 2];
        if(nField & 1)
        {
            pUser += 2048;
        }
        BytesPerLine = CurrentX * 2;
        for (nLine = 0; nLine < CurrentY / 2; nLine++)
        {

            pPhysical = GetPhysicalAddress(DisplayDMAMem[nField / 2], pUser, BytesPerLine, &GotBytesPerLine);
            if(pPhysical == 0 || BytesPerLine > GotBytesPerLine)
            {
                return;
            }
            *(pRiscCode++) = BT848_RISC_WRITE | BT848_RISC_SOL | BT848_RISC_EOL | BytesPerLine;
            *(pRiscCode++) = pPhysical;
            // since we are doing all the lines of the same
            // polarity at the same time we skip two lines
            pUser += 4096;
        }
    }

    BytesPerRISCField = ((long)pRiscCode - (long)RiscDMAMem->dwUser) / 10;
    *(pRiscCode++) = BT848_RISC_JUMP;
    *(pRiscCode++) = RiscBasePhysical;

    BT848_WriteDword(BT848_RISC_STRT_ADD, RiscBasePhysical);
}

// Works out a field number between 0-9 indicating which field we are currently
// sending to memory
int BT848_GetRISCPosAsInt()
{
    int CurrentPos = 10;
    while(CurrentPos > 9)
    {
        DWORD CurrentRiscPos = BT848_ReadDword(BT848_RISC_COUNT);
        CurrentPos = (CurrentRiscPos - RiscBasePhysical) / BytesPerRISCField;
    }

    return CurrentPos;
}

void BT848_Close()
{
    if(hBT8X8 != NULL)
    {
        BT848_WriteByte(BT848_SRESET, 0);
    
        memoryUnmap(hBT8X8->PCICardInfo.dwMemoryAddress, hBT8X8->PCICardInfo.dwMemoryLength);

        free(hBT8X8);
        hBT8X8 = NULL;
    }
}

int BT848_Open(DWORD dwVendorID, DWORD dwDeviceID, DWORD options, BOOL Lock)
{
    int Ret = 0;

    if(hBT8X8 != NULL)
    {
        BT848_Close();
    }
    
    hBT8X8 = (TBT848*) malloc(sizeof(TBT848));

    memset(hBT8X8, 0, sizeof(TBT848));

    Ret = pciGetHardwareResources( 
                                    dwVendorID,  
                                    dwDeviceID,
                                    0,
                                    &(hBT8X8->PCICardInfo)
                                 );


    // check if handle valid & version OK
    if (Ret != ERROR_SUCCESS)
    {
        Ret = 2;
        // error - Cannot find PCI card
        goto Exit;
    }

    hBT8X8->dwMemoryBase = memoryMap(
                                        hBT8X8->PCICardInfo.dwBusNumber,
                                        hBT8X8->PCICardInfo.dwMemoryAddress, 
                                        hBT8X8->PCICardInfo.dwMemoryLength
                                    );
    if(hBT8X8->dwMemoryBase == 0)
    {
        Ret = 3;
        goto Exit;
    }

    // Open finished OK
    return 0;

  Exit:
    // Error during Open
    free(hBT8X8);
    hBT8X8 = NULL;
    return Ret;
}

BOOL Alloc_DMA(DWORD dwSize, PMemStruct* dma, int Option)
{
    *dma = NULL;

    memoryAlloc(dwSize, Option, dma);

    if (*dma == NULL)
    {
        return (FALSE);
    }
    return TRUE;
}

void Free_DMA(PMemStruct* dma)
{
    memoryFree(*dma);
}

void Free_Display_DMA(int NR)
{
    if (DisplayDMAMem[NR] == NULL)
    {
        return;
    }
    memoryFree(DisplayDMAMem[NR]);
    DisplayDMAMem[NR] = NULL;
}

PHYS GetPhysicalAddress(PMemStruct pMem, LPBYTE pLinear, DWORD dwSizeWanted, DWORD* pdwSizeAvailable)
{
    PPageStruct pPages = (PPageStruct)(pMem + 1);
    DWORD Offset;
    DWORD i; 
    DWORD sum;
    DWORD pRetVal = 0;

    Offset = (DWORD)pLinear - (DWORD)pMem->dwUser;
    sum = 0; 
    i = 0;
    while (i < pMem->dwPages)
    {
        if (sum + pPages[i].dwSize > (unsigned)Offset)
        {
            Offset -= sum;
            pRetVal = pPages[i].dwPhysical + Offset;    
            if ( pdwSizeAvailable != NULL )
            {
                *pdwSizeAvailable = pPages[i].dwSize - Offset;
            }
            break;
        }
        sum += pPages[i].dwSize; 
        i++;
    }
    if(pRetVal == 0)
    {
        sum++;
    }
    if ( pdwSizeAvailable != NULL )
    {
        if (*pdwSizeAvailable < dwSizeWanted)
        {
            sum++;
        }
    }

    return pRetVal; 
}

// JA 29/12/2000
// This function returns the user space address of the first page aligned
// section of an allocated buffer.

BYTE* GetFirstFullPage(PMemStruct pMem)
{
    PPageStruct pPages = (PPageStruct)(pMem + 1);
    DWORD pRetVal;

    pRetVal = (DWORD)pMem->dwUser;

    if(pPages[0].dwSize != 4096)
    {
        pRetVal += pPages[0].dwSize;
    }
    return (BYTE*)pRetVal;  
}

BYTE BT848_ReadByte(DWORD dwOffset)
{
    return memoryReadBYTE(hBT8X8->dwMemoryBase + dwOffset);
}

WORD BT848_ReadWord(DWORD dwOffset)
{
    return memoryReadWORD(hBT8X8->dwMemoryBase + dwOffset);
}

DWORD BT848_ReadDword(DWORD dwOffset)
{
    return memoryReadDWORD(hBT8X8->dwMemoryBase + dwOffset);
}

void BT848_WriteByte(DWORD dwOffset,BYTE data)
{
    memoryWriteBYTE(hBT8X8->dwMemoryBase + dwOffset, data);
}

void BT848_WriteWord(DWORD dwOffset,WORD data) 
{
    memoryWriteWORD(hBT8X8->dwMemoryBase + dwOffset, data);
}

void BT848_WriteDword(DWORD dwOffset, DWORD data)
{
    memoryWriteDWORD(hBT8X8->dwMemoryBase + dwOffset, data);
}

void BT848_MaskDataByte(int Offset, BYTE d, BYTE m)
{
    BYTE a;

    a = BT848_ReadByte(Offset);
    a = (a & ~(m)) | ((d) & (m));
    BT848_WriteByte(Offset, a);
}

void BT848_MaskDataWord(int Offset, WORD d, WORD m)
{
    WORD a;

    a = BT848_ReadWord(Offset);
    a = (a & ~(m)) | ((d) & (m));
    BT848_WriteWord(Offset, a);
}
void BT848_AndOrDataDword(int Offset, DWORD d, DWORD m)
{
    DWORD a;

    a = BT848_ReadDword(Offset);
    a = (a & m) | d;
    BT848_WriteDword(Offset, a);
}

void BT848_AndDataByte(int Offset, BYTE d)
{
    BYTE a;

    a = BT848_ReadByte(Offset);
    a &= d;
    BT848_WriteByte(Offset, a);
}

void BT848_AndDataWord(int Offset, WORD d)
{
    WORD a;

    a = BT848_ReadWord(Offset);
    a &= d;
    BT848_WriteWord(Offset, a);
}

void BT848_AndDataWord(int Offset, DWORD d)
{
    DWORD a;

    a = BT848_ReadDword(Offset);
    a &= d;
    BT848_WriteDword(Offset, a);
}


void BT848_OrDataByte(int Offset, BYTE d)
{
    BYTE a;

    a = BT848_ReadByte(Offset);
    a |= d;
    BT848_WriteByte(Offset, a);
}

void BT848_OrDataWord(int Offset, WORD d)
{
    WORD a;

    a = BT848_ReadWord(Offset);
    a |= d;
    BT848_WriteWord(Offset, a);
}

void BT848_OrDataDword(int Offset, DWORD d)
{
    DWORD a;

    a = BT848_ReadDword(Offset);
    a |= d;
    BT848_WriteDword(Offset, a);
}


//===========================================================================
// Enable CCIR656 Input Mode
//===========================================================================
BOOL BT848_Enable656()
{
    int vscale, hscale;
    int hdelay, vdelay;
    int hactive, vactive;
    BYTE crop;
    int i;

    CurrentX = 720;
    CurrentY = 480;

    // MAE 20Mar2001 
    if (BT848_IsPAL()) CurrentY = 576;

    // Disable TG Mode
    BT848_MaskDataByte(BT848_TGCTRL, 0, BT848_TGCTRL_TGMODE_ENABLE);
    
    // Reset the TG address
    BT848_MaskDataByte(BT848_TGCTRL, 0, BT848_TGCTRL_TGMODE_RESET);
    BT848_MaskDataByte(BT848_TGCTRL, BT848_TGCTRL_TGMODE_RESET, BT848_TGCTRL_TGMODE_RESET);
    BT848_MaskDataByte(BT848_TGCTRL, 0, BT848_TGCTRL_TGMODE_RESET);

    // MAE 20Mar2001
    if (BT848_IsPAL())
    {
        // Load up the TG table for CCIR656
        for (i=0;i<SRAMTable_PAL[0];++i)
        {
            BT848_WriteByte(BT848_TBLG,SRAMTable_PAL[i+1]);
        }
    }
    else
    {
        // Load up the TG table for CCIR656
        for (i=0;i<SRAMTable_NTSC[0];++i)
        {
            BT848_WriteByte(BT848_TBLG,SRAMTable_NTSC[i+1]);
        }
    }
    
    // Enable TG Mode
    BT848_MaskDataByte(BT848_TGCTRL, BT848_TGCTRL_TGMODE_ENABLE, BT848_TGCTRL_TGMODE_ENABLE);

    // Enable the GPCLOCK
    BT848_MaskDataByte(BT848_TGCTRL, BT848_TGCTRL_TGCKI_GPCLK, BT848_TGCTRL_TGCKI_GPCLK);

    // Set the PLL Mode
    BT848_WriteByte(BT848_PLL_XCI, 0x00);

    // Enable 656 Mode, bypass chroma filters
    BT848_WriteByte(BT848_DVSIF, BT848_DVSIF_VSIF_BCF | BT848_DVSIF_CCIR656);
    
    // MAE 20Mar2001
    if (BT848_IsPAL())
    {
        // Enable PAL Mode (or SECAM)
        BT848_MaskDataByte(BT848_IFORM, (BT848_IFORM_PAL_BDGHI | BT848_IFORM_XTBOTH), (BT848_IFORM_NORM | BT848_IFORM_XTBOTH));
    }
    else
    {
        // Enable NTSC Mode (or PAL60)
        BT848_MaskDataByte(BT848_IFORM, (BT848_IFORM_NTSC | BT848_IFORM_XTBOTH), (BT848_IFORM_NORM | BT848_IFORM_XTBOTH));
    }

    // Disable full range luma
    BT848_WriteByte(BT848_OFORM, 0);

    // Enable the SC loop luma peaking filters
    BT848_WriteByte(BT848_E_SCLOOP, BT848_SCLOOP_LUMA_PEAK);
    BT848_WriteByte(BT848_O_SCLOOP, BT848_SCLOOP_LUMA_PEAK);

    // Standard NTSC 525 line Count
    BT848_WriteByte(BT848_VTOTAL_LO, 0x00);
    BT848_WriteByte(BT848_VTOTAL_HI, 0x00);

    // YUV 4:2:2 linear pixel format
    BT848_WriteByte(BT848_COLOR_FMT, (BYTE)((BT848_COLOR_FMT_YUY2 << 4) | BT848_COLOR_FMT_YUY2));

    // Setup parameters for overlay scale and crop calculation
    hactive = CurrentX;
    vactive = CurrentY;
    hscale = 0;
    vdelay = 16;
    hdelay = 0x80;
    vscale = 0;

    crop = ((hactive >> 8) & 0x03) | ((hdelay >> 6) & 0x0c) | ((vactive >> 4) & 0x30) | ((vdelay >> 2) & 0xc0);

    BT848_SetGeometryEvenOdd(FALSE, BT848_VTC_HSFMT_32, hscale, vscale, hactive, vactive, hdelay, vdelay, crop);
    BT848_SetGeometryEvenOdd(TRUE, BT848_VTC_HSFMT_32, hscale, vscale, hactive, vactive, hdelay, vdelay, crop);

    return TRUE;
}

TTVFormat* BT848_GetTVFormat()
{
    return TVFormats + TVFormat;
}

BOOL APIENTRY AdvVideoSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:

        // Luma AGC, 0 says AGC enabled
        CheckDlgButton(hDlg, IDC_AGC, !BtAgcDisable);
        // Adaptive AGC, 0 says Crush disabled
        CheckDlgButton(hDlg, IDC_CRUSH, BtCrush);
        // Even CAGC, 0 says CAGC disable
        CheckDlgButton(hDlg, IDC_E_CAGC, BtEvenChromaAGC); 
        // Odd CAGC
        CheckDlgButton(hDlg, IDC_O_CAGC, BtOddChromaAGC);
        // Even Peak, 0 says normal, not Luma peak
        CheckDlgButton(hDlg, IDC_E_LUMA_PEAK, BtEvenLumaPeak);
        // Odd Peak
        CheckDlgButton(hDlg, IDC_O_LUMA_PEAK, BtOddLumaPeak);
        // Luma Output Range, 0 says Luma Normal, 1=Full    
        CheckDlgButton(hDlg, IDC_LUMA_RANGE, BtFullLumaRange);
        // Even Luma decimation,  0 says disable
        CheckDlgButton(hDlg, IDC_E_LUMA_DEC, BtEvenLumaDec);
        // Odd Luma decimation
        CheckDlgButton(hDlg, IDC_O_LUMA_DEC, BtOddLumaDec);
        // Even COMB, 0 = disable
        CheckDlgButton(hDlg, IDC_E_COMB, BtEvenComb);
        // Odd COMB
        CheckDlgButton(hDlg, IDC_O_COMB, BtOddComb);
        // Color Bars, 0 = disable
        CheckDlgButton(hDlg, IDC_COLOR_BARS, BtColorBars);
        // Gamma correction removal, 0=enabled
        CheckDlgButton(hDlg, IDC_GAMMA_CORR, !BtGammaCorrection);
        // More Vertical Filter, 0=no, 4=yes, other values no good at our res
        // (Z filter)   TRB 12/19/00
        CheckDlgButton(hDlg, IDC_VERT_FILTER, BtVertFilter);
        // More Horizontal Filter, 0=no, else max full res filter TRB 12/19/00
        CheckDlgButton(hDlg, IDC_HOR_FILTER, BtHorFilter);
        break;

    case WM_MOUSEMOVE:
        return (FALSE);

    case WM_COMMAND:

        switch LOWORD(wParam)
        {
        case IDOK:                          // Is Done
			WriteSettingsToIni(TRUE);
            EndDialog(hDlg, TRUE);
            break;

        case IDC_AGC:                       // Changed AGC
            BtAgcDisable = (BST_CHECKED != IsDlgButtonChecked(hDlg, IDC_AGC));
            BT848_Registers_OnChange();
            break;  

        case IDC_CRUSH:                     // Changed Adaptive AGC
            BtCrush = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CRUSH));
            BT848_Registers_OnChange();
            break;  

        case IDC_E_CAGC:                    // Changed Even CAGC
            BtEvenChromaAGC = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_E_CAGC));
            BT848_Registers_OnChange();
            break;

        case IDC_O_CAGC:                    // Changed Odd CAGC
            BtOddChromaAGC = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_O_CAGC));
            BT848_Registers_OnChange();
            break;

        case IDC_E_LUMA_PEAK:               // Changed Even Peak
            BtEvenLumaPeak = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_E_LUMA_PEAK));
            BT848_Registers_OnChange();
            break;

        case IDC_O_LUMA_PEAK:               // Changed Odd Peak
            BtOddLumaPeak = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_O_LUMA_PEAK));
            BT848_Registers_OnChange();
            break;

        case IDC_LUMA_RANGE:                // Luma Output Range
            BtFullLumaRange = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_LUMA_RANGE));
            BT848_Registers_OnChange();
            break;

        case IDC_E_LUMA_DEC:                // Changed Even L.decimation
            BtEvenLumaDec = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_E_LUMA_DEC));
            BT848_Registers_OnChange();
            break;

        case IDC_O_LUMA_DEC:                // Changed Odd L.decimation
            BtOddLumaDec = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_O_LUMA_DEC));
            BT848_Registers_OnChange();
            break;

        case IDC_E_COMB:                    // Changed Even COMB
            BtEvenComb = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_E_COMB));
            BT848_Registers_OnChange();
            break;

        case IDC_O_COMB:                    // Changed Odd COMB
            BtOddComb = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_O_COMB));
            BT848_Registers_OnChange();
            break;

        case IDC_COLOR_BARS:                // Color Bars
            BtColorBars = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_COLOR_BARS));
            BT848_Registers_OnChange();
            break;

        case IDC_GAMMA_CORR:                // Gamma correction removal
            BtGammaCorrection = (BST_CHECKED != IsDlgButtonChecked(hDlg, IDC_GAMMA_CORR));
            BT848_Registers_OnChange();
            break;

        case IDC_VERT_FILTER:               // Use vertical z-filter
            BtVertFilter = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_VERT_FILTER));
            BT848_Registers_OnChange();
            break;

        case IDC_HOR_FILTER:                // Use Hor peaking filter
            BtHorFilter = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_HOR_FILTER));
            BT848_Registers_OnChange();
            break;

        }
        break;
    }
    return (FALSE);
}

BOOL VideoSource_OnChange(long NewValue)
{
    Stop_Capture();
    Audio_Mute();
    VideoSettings_SaveTVFormat();
    VideoSettings_Save();
    VideoSource = (eVideoSourceType)NewValue;
    VideoSettings_LoadTVFormat();
    VideoSettings_Load();
    switch(NewValue)
    {
    case SOURCE_TUNER:
        BT848_ResetHardware();
        BT848_SetGeoSize();
        WorkoutOverlaySize();
        if(Audio_MSP_IsPresent())
        {
            AudioSource = AUDIOMUX_MSP_RADIO;
        }
        else
        {
            AudioSource = AUDIOMUX_TUNER;
        }
        Channel_SetCurrent();
        break;

    // MAE 13 Dec 2000 for CCIR656 Digital input
    case SOURCE_CCIR656_1:
    case SOURCE_CCIR656_2:
    case SOURCE_CCIR656_3:
    case SOURCE_CCIR656_4:
    case SOURCE_COMPOSITE:
    case SOURCE_SVIDEO:
    case SOURCE_OTHER1:
    case SOURCE_OTHER2:
    case SOURCE_COMPVIASVIDEO:
        BT848_ResetHardware();
        BT848_SetGeoSize();
        WorkoutOverlaySize();
        AudioSource = AUDIOMUX_EXTERNAL;
        break;
    default:
        break;
    }

    Audio_Unmute();
    Start_Capture();
    return FALSE;
}

BOOL TVFormat_OnChange(long NewValue)
{
    Stop_Capture();
    VideoSettings_SaveTVFormat();
    VideoSettings_Save();
    TVFormat = NewValue;
    VideoSettings_Load();
    BT848_ResetHardware();
    BT848_SetGeoSize();
    WorkoutOverlaySize();
    Start_Capture();
    return FALSE;
}

BOOL CurrentX_OnChange(long NewValue)
{
    CurrentX = NewValue;

    if(CurrentX != 768 &&
        CurrentX != 754 &&
        CurrentX != 720 &&
        CurrentX != 640 &&
        CurrentX != 384 &&
        CurrentX != 320)
    {
        CustomPixelWidth = CurrentX;
    }
    Stop_Capture();
    BT848_SetGeoSize();
    WorkoutOverlaySize();
    Start_Capture();
    return FALSE;
}

BOOL HDelay_OnChange(long NewValue)
{
    HDelay = NewValue;
    if(hBT8X8 != NULL)
    {
        Stop_Capture();
        BT848_SetGeoSize();
        Start_Capture();
    }
    return FALSE;
}

BOOL VDelay_OnChange(long NewValue)
{
    VDelay = NewValue;
    if(hBT8X8 != NULL)
    {
        Stop_Capture();
        BT848_SetGeoSize();
        Start_Capture();
    }
    return FALSE;
}

BOOL BtAgcDisable_OnChange(long NewValue)
{
    BtAgcDisable = NewValue;
    BT848_Registers_OnChange();
    return FALSE;
}

BOOL BtCrush_OnChange(long NewValue)
{
    BtCrush = NewValue;
    BT848_Registers_OnChange();
    return FALSE;
}

BOOL BtEvenChromaAGC_OnChange(long NewValue)
{
    BtEvenChromaAGC = NewValue;
    BT848_Registers_OnChange();
    return FALSE;
}

BOOL BtOddChromaAGC_OnChange(long NewValue)
{
    BtOddChromaAGC = NewValue;
    BT848_Registers_OnChange();
    return FALSE;
}

BOOL BtEvenLumaPeak_OnChange(long NewValue)
{
    BtEvenLumaPeak = NewValue;
    BT848_Registers_OnChange();
    return FALSE;
}

BOOL BtOddLumaPeak_OnChange(long NewValue)
{
    BtOddLumaPeak = NewValue;
    BT848_Registers_OnChange();
    return FALSE;
}

BOOL BtFullLumaRange_OnChange(long NewValue)
{
    BtFullLumaRange = NewValue;
    BT848_Registers_OnChange();
    return FALSE;
}

BOOL BtEvenLumaDec_OnChange(long NewValue)
{
    BtEvenLumaDec = NewValue;
    BT848_Registers_OnChange();
    return FALSE;
}

BOOL BtOddLumaDec_OnChange(long NewValue)
{
    BtOddLumaDec = NewValue;
    BT848_Registers_OnChange();
    return FALSE;
}

BOOL BtEvenComb_OnChange(long NewValue)
{
    BtEvenComb = NewValue;
    BT848_Registers_OnChange();
    return FALSE;
}

BOOL BtOddComb_OnChange(long NewValue)
{
    BtOddComb = NewValue;
    BT848_Registers_OnChange();
    return FALSE;
}

BOOL BtColorBars_OnChange(long NewValue)
{
    BtColorBars = NewValue;
    BT848_Registers_OnChange();
    return FALSE;
}

BOOL BtGammaCorrection_OnChange(long NewValue)
{
    BtGammaCorrection = NewValue;
    BT848_Registers_OnChange();
    return FALSE;
}

BOOL BtCoring_OnChange(long NewValue)
{
    BtCoring = NewValue;
    BT848_Registers_OnChange();
    return FALSE;
}

BOOL BtHorFilter_OnChange(long NewValue)
{
    BtHorFilter = NewValue;
    BT848_Registers_OnChange();
    return FALSE;
}

BOOL BtVertFilter_OnChange(long NewValue)
{
    BtVertFilter = NewValue;
    BT848_Registers_OnChange();
    return FALSE;
}

BOOL BtColorKill_OnChange(long NewValue)
{
    BtColorKill = NewValue;
    BT848_Registers_OnChange();
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////

SETTING BT848Settings[BT848_SETTING_LASTONE] =
{
    {
        "Brightness", SLIDER, 0, (long*)&InitialBrightness,
        DEFAULT_BRIGHTNESS_NTSC, -128, 127, 1, 1,
        NULL,
        "Hardware", "InitialBrightness", BT848_Brightness_OnChange,
    },
    {
        "Contrast", SLIDER, 0, (long*)&InitialContrast,
        DEFAULT_CONTRAST_NTSC, 0, 255, 1, 1,
        NULL,
        "Hardware", "InitialContrast", BT848_Contrast_OnChange,
    },
    {
        "Hue", SLIDER, 0, (long*)&InitialHue,
        DEFAULT_HUE_NTSC, -128, 127, 1, 1,
        NULL,
        "Hardware", "InitialHue", BT848_Hue_OnChange,
    },
    {
        "Saturation", SLIDER, 0, (long*)&InitialSaturation,
        (DEFAULT_SAT_V_NTSC + DEFAULT_SAT_U_NTSC) / 2, 0, 255, 1, 1,
        NULL,
        NULL, NULL, BT848_Saturation_OnChange,
    },
    {
        "Blue Saturation", SLIDER, 0, (long*)&InitialSaturationU,
        DEFAULT_SAT_U_NTSC, 0, 255, 1, 1,
        NULL,
        "Hardware", "InitialSaturationU", BT848_SaturationU_OnChange,
    },
    {
        "Red Saturation", SLIDER, 0, (long*)&InitialSaturationV,
        DEFAULT_SAT_V_NTSC, 0, 255, 1, 1,
        NULL,
        "Hardware", "InitialSaturationV", BT848_SaturationV_OnChange,
    },
    {
        "Macrovision Timing", SLIDER, 0, (long*)&InitialBDelay,
        0, 0, 255, 1, 1,
        NULL,
        "Hardware", "InitialBDelay", BT848_BDelay_OnChange,
    },
    {
        "AGC Disable", ONOFF, 0, (long*)&BtAgcDisable,
        FALSE, 0, 1, 1, 1, 
        NULL,
        "Hardware", "BtAgcDisable", BtAgcDisable_OnChange,
    },
    {
        "Crush", ONOFF, 0, (long*)&BtCrush,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Hardware", "BtCrush", BtCrush_OnChange,
    },
    {
        "Even Chroma AGC", ONOFF, 0, (long*)&BtEvenChromaAGC,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Hardware", "BtEvenChromaAGC", BtEvenChromaAGC_OnChange,
    },
    {
        "Odd Chroma AGC", ONOFF, 0, (long*)&BtOddChromaAGC,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Hardware", "BtOddChromaAGC", BtOddChromaAGC_OnChange,
    },
    {
        "Even Luma Peak", ONOFF, 0, (long*)&BtEvenLumaPeak,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Hardware", "BtEvenLumaPeak", BtEvenLumaPeak_OnChange,
    },
    {
        "Odd Luma Peak", ONOFF, 0, (long*)&BtOddLumaPeak,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Hardware", "BtOddLumaPeak", BtOddLumaPeak_OnChange,
    },
    {
        "Full Luma Range", ONOFF, 0, (long*)&BtFullLumaRange,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Hardware", "BtFullLumaRange", BtFullLumaRange_OnChange,
    },
    {
        "Even Luma Dec", ONOFF, 0, (long*)&BtEvenLumaDec,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Hardware", "BtEvenLumaDec", BtEvenLumaDec_OnChange,
    },
    {
        "Odd Luma Dec", ONOFF, 0, (long*)&BtOddLumaDec,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Hardware", "BtOddLumaDec", BtOddLumaDec_OnChange,
    },
    {
        "Even Comb", ONOFF, 0, (long*)&BtEvenComb,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Hardware", "BtEvenComb", BtEvenComb_OnChange,
    },
    {
        "Odd Comb", ONOFF, 0, (long*)&BtOddComb,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Hardware", "BtOddComb", BtOddComb_OnChange,
    },
    {
        "Color Bars", ONOFF, 0, (long*)&BtColorBars,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Hardware", "BtColorBars", BtColorBars_OnChange,
    },
    {
        "Gamma Correction", ONOFF, 0, (long*)&BtGammaCorrection,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Hardware", "BtGammaCorrection", BtGammaCorrection_OnChange,
    },
    {
        "Coring", ONOFF, 0, (long*)&BtCoring,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Hardware", "BtCoring", BtCoring_OnChange,
    },
    {
        "Horiz Filter", ONOFF, 0, (long*)&BtHorFilter,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Hardware", "BtHorFilter", BtHorFilter_OnChange,
    },
    {
        "Vert Filter", ONOFF, 0, (long*)&BtVertFilter,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Hardware", "BtVertFilter", BtVertFilter_OnChange,
    },
    {
        "Color Kill", ONOFF, 0, (long*)&BtColorKill,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Hardware", "BtColorKill", BtColorKill_OnChange,
    },
    {
        "White Crush Upper", SLIDER, 0, (long*)&BtWhiteCrushUp,
        0xcf, 0, 255, 1, 1,
        NULL,
        "Hardware", "BtWhiteCrushUp", BT848_WhiteCrushUp_OnChange,
    },
    {
        "White Crush Lower", SLIDER, 0, (long*)&BtWhiteCrushDown,
        0x7f, 0, 255, 1, 1,
        NULL,
        "Hardware", "BtWhiteCrushDown", BT848_WhiteCrushDown_OnChange,
    },
    {
        "Sharpness", SLIDER, 0, (long*)&CurrentX,
        720, 120, DSCALER_MAX_WIDTH, 2, 1,
        NULL,
        "MainWindow", "CurrentX", CurrentX_OnChange,
    },
    {
        "Custom Pixel Width", SLIDER, 0, (long*)&CustomPixelWidth,
        750, 120, DSCALER_MAX_WIDTH, 2, 1,
        NULL,
        "MainWindow", "CustomPixelWidth", NULL,
    },
    {
        "Video Source", SLIDER, 0, (long*)&VideoSource,
        SOURCE_COMPOSITE, SOURCE_TUNER, SOURCE_CCIR656_4, 1, 1,
        NULL,
        "Hardware", "VideoSource", VideoSource_OnChange,
    },
    {
        "Video Format", ITEMFROMLIST, 0, (long*)&TVFormat,
        FORMAT_NTSC, 0, FORMAT_LASTONE - 1, 1, 1,
        FormatList,
        "Hardware", "TVType", TVFormat_OnChange,
    },
    {
        "Horiz Delay", SLIDER, 0, (long*)&HDelay,
        0, 0, 255, 1, 1,
        NULL,
        "Hardware", "HDelay", HDelay_OnChange,
    },
    {
        "Vert Delay", SLIDER, 0, (long*)&VDelay,
        0, 0, 254, 2, 1,
        NULL,
        "Hardware", "VDelay", VDelay_OnChange,
    },
};

SETTING* BT848_GetSetting(BT848_SETTING Setting)
{
    if(Setting > -1 && Setting < BT848_SETTING_LASTONE)
    {
        return &(BT848Settings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void BT848_ReadSettingsFromIni()
{
    int i;
    for(i = 0; i < FORMAT_LASTONE; i++)
    {
        FormatList[i] = TVFormats[i].szDesc;
    }
    for(i = 0; i < BT848_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(BT848Settings[i]));
    }
}

void BT848_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < BT848_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(BT848Settings[i]), bOptimizeFileAccess);
    }
}

void BT848_SetMenu(HMENU hMenu)
{
    BOOL DoneWidth = FALSE;
    CheckMenuItemBool(hMenu, IDM_SOURCE_TUNER, (VideoSource == 0));
    CheckMenuItemBool(hMenu, IDM_SOURCE_COMPOSITE, (VideoSource == 1));
    CheckMenuItemBool(hMenu, IDM_SOURCE_SVIDEO, (VideoSource == 2));
    CheckMenuItemBool(hMenu, IDM_SOURCE_OTHER1, (VideoSource == 3));
    CheckMenuItemBool(hMenu, IDM_SOURCE_OTHER2, (VideoSource == 4));
    CheckMenuItemBool(hMenu, IDM_SOURCE_COMPVIASVIDEO, (VideoSource == 5));
    CheckMenuItemBool(hMenu, IDM_SOURCE_CCIR656_1, (VideoSource == 6));
    CheckMenuItemBool(hMenu, IDM_SOURCE_CCIR656_2, (VideoSource == 7));
    CheckMenuItemBool(hMenu, IDM_SOURCE_CCIR656_3, (VideoSource == 8));
    CheckMenuItemBool(hMenu, IDM_SOURCE_CCIR656_4, (VideoSource == 9));

    if(BT848_GetTVFormat()->wHActivex1 < 768)
    {
        EnableMenuItem(hMenu, ID_SETTINGS_PIXELWIDTH_768, MF_GRAYED);
    }
    else
    {
        EnableMenuItem(hMenu, ID_SETTINGS_PIXELWIDTH_768, MF_ENABLED);
    }
    CheckMenuItemBool(hMenu, ID_SETTINGS_PIXELWIDTH_768, (CurrentX == 768));
    DoneWidth |= (CurrentX == 768);
    CheckMenuItemBool(hMenu, ID_SETTINGS_PIXELWIDTH_754, (CurrentX == 754));
    DoneWidth |= (CurrentX == 754);
    CheckMenuItemBool(hMenu, ID_SETTINGS_PIXELWIDTH_720, (CurrentX == 720));
    DoneWidth |= (CurrentX == 720);
    CheckMenuItemBool(hMenu, ID_SETTINGS_PIXELWIDTH_640, (CurrentX == 640));
    DoneWidth |= (CurrentX == 640);
    CheckMenuItemBool(hMenu, ID_SETTINGS_PIXELWIDTH_384, (CurrentX == 384));
    DoneWidth |= (CurrentX == 384);
    CheckMenuItemBool(hMenu, ID_SETTINGS_PIXELWIDTH_320, (CurrentX == 320));
    DoneWidth |= (CurrentX == 320);
    CheckMenuItemBool(hMenu, ID_SETTINGS_PIXELWIDTH_CUSTOM, !DoneWidth);

    CheckMenuItemBool(hMenu, IDM_TYPEFORMAT_0, (TVFormat == 0));
    CheckMenuItemBool(hMenu, IDM_TYPEFORMAT_1, (TVFormat == 1));
    CheckMenuItemBool(hMenu, IDM_TYPEFORMAT_2, (TVFormat == 2));
    CheckMenuItemBool(hMenu, IDM_TYPEFORMAT_3, (TVFormat == 3));
    CheckMenuItemBool(hMenu, IDM_TYPEFORMAT_4, (TVFormat == 4));
    CheckMenuItemBool(hMenu, IDM_TYPEFORMAT_5, (TVFormat == 5));
    CheckMenuItemBool(hMenu, IDM_TYPEFORMAT_6, (TVFormat == 6));
    CheckMenuItemBool(hMenu, IDM_TYPEFORMAT_7, (TVFormat == 7));
    CheckMenuItemBool(hMenu, IDM_TYPEFORMAT_8, (TVFormat == 8));
    CheckMenuItemBool(hMenu, IDM_TYPEFORMAT_9, (TVFormat == 9));
}

void BT848_ShowUI()
{
    CSettingsDlg::ShowSettingsDlg("BT848 Chip Settings",BT848Settings, BT848_SETTING_LASTONE);
}
