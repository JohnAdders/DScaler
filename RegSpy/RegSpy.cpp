/////////////////////////////////////////////////////////////////////////////
// $Id: RegSpy.cpp,v 1.16 2003-09-28 09:11:33 adcockj Exp $
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.15  2003/08/12 16:57:54  laurentg
// Update the prototype of GetDisplayAreaRect
//
// Revision 1.14  2003/05/05 15:30:38  atnak
// Fixes crash when custom name is too long
//
// Revision 1.13  2003/05/05 12:57:56  adcockj
// Added some support for saa7134
//
// Revision 1.12  2003/01/26 12:49:53  adcockj
// Fixed link problems
//
// Revision 1.11  2002/12/22 02:30:12  atnak
// Added SAA7133 registers
//
// Revision 1.10  2002/12/15 13:54:13  adcockj
// Added code to reset card better
//
// Revision 1.9  2002/12/11 01:01:08  atnak
// Added SAA7130 and more registers for SAA7134 to aid new card adding.
//
// Revision 1.8  2002/12/10 10:15:33  atnak
// Fixed binary display order, changed some ini stuff
//
// Revision 1.7  2002/12/06 00:28:40  atnak
// Replaced CreateDIBSection() with CreateCompatibleBitmap()
//
// Revision 1.6  2002/12/05 17:10:47  adcockj
// Added new registers for debugging sound on cx2388x
//
// Revision 1.5  2002/12/05 08:38:39  atnak
// Fixed no redraw bug in last update
//
// Revision 1.4  2002/12/05 08:06:11  atnak
// Various changes
//
// Revision 1.3  2002/12/04 14:15:06  adcockj
// Fixed RegSpy Problems
//
// Revision 1.2  2002/12/04 00:20:32  atnak
// Fixed: Registers not redrawing properly.  Some register disappearing off the
// list.  Untabified file
//
// Revision 1.1  2002/12/03 20:51:42  atnak
// Added new app for monitoring register settings while other programs are running
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "../RegLog/GenericCard.h"
#include "../DScaler/BT848_Defines.h"
#include "../DScaler/CX2388X_Defines.h"
#include "../DScaler/SAA7134_Defines.h"

enum
{
    REG_FONT_HEIGHT     = 12,
    MAX_LOG_STATES      = 6,

    MIN_DIALOG_WIDTH    = 310,

    REG_REFRESH_TICKS   = 200,
    CHANGE_FADE_TICKS   = 4000 / REG_REFRESH_TICKS,
};

#define REG_FONT_NAME       "Lucida Console"

typedef struct _Register    TRegister;
typedef struct _Chip        TChip;
typedef struct _Source      TSource;

typedef void (__cdecl REGSPYFUNC)(TRegister** hRegisterListTail);


struct _Chip
{
    char*       Name;
    WORD        VendorId;
    WORD        DeviceId;
    REGSPYFUNC* SetupFunction;
};


struct _Source
{
    _Source*    Next;
    TChip*      Chip;
    int         DeviceIndex;
    DWORD       SubSystemId;
};


struct _Register
{
    _Register*  Next;
    DWORD       Offset;
    char*       Name;
    BYTE        Size;
    BOOL        bAlloced;
    DWORD       LastValue;
    DWORD       LogStates[MAX_LOG_STATES];
    WORD        FadeTicks;
    BOOL        bDraw;
};


#define AddBRegister(Reg) \
    { \
        *hRegisterListTail = (TRegister*)malloc(sizeof(TRegister)); \
        if(*hRegisterListTail != NULL) \
        { \
            (*hRegisterListTail)->Name = #Reg; \
            (*hRegisterListTail)->Offset = Reg; \
            (*hRegisterListTail)->Size = 1; \
            (*hRegisterListTail)->bAlloced = FALSE; \
            (*hRegisterListTail)->bDraw = FALSE; \
            *(hRegisterListTail = &(*hRegisterListTail)->Next) = NULL; \
        } \
    }
#define AddWRegister(Reg) \
    { \
        *hRegisterListTail = (TRegister*)malloc(sizeof(TRegister)); \
        if(*hRegisterListTail != NULL) \
        { \
            (*hRegisterListTail)->Name = #Reg; \
            (*hRegisterListTail)->Offset = Reg; \
            (*hRegisterListTail)->Size = 2; \
            (*hRegisterListTail)->bAlloced = FALSE; \
            (*hRegisterListTail)->bDraw = FALSE; \
            *(hRegisterListTail = &(*hRegisterListTail)->Next) = NULL; \
        } \
    }
#define AddDWRegister(Reg) \
    { \
        *hRegisterListTail = (TRegister*)malloc(sizeof(TRegister)); \
        if(*hRegisterListTail != NULL) \
        { \
            (*hRegisterListTail)->Name = #Reg; \
            (*hRegisterListTail)->Offset = Reg; \
            (*hRegisterListTail)->Size = 4; \
            (*hRegisterListTail)->bAlloced = FALSE; \
            (*hRegisterListTail)->bDraw = FALSE; \
            *(hRegisterListTail = &(*hRegisterListTail)->Next) = NULL; \
        } \
    }


void __cdecl BT848RegSpy(TRegister** hRegisterListTail)
{
    AddBRegister(BT848_IFORM);
    AddBRegister(BT848_FCNTR);
    AddBRegister(BT848_PLL_F_LO);
    AddBRegister(BT848_PLL_F_HI);
    AddBRegister(BT848_PLL_XCI);
    AddBRegister(BT848_TGCTRL);
    AddBRegister(BT848_TDEC);
    AddBRegister(BT848_E_CROP);
    AddBRegister(BT848_O_CROP);
    AddBRegister(BT848_E_VDELAY_LO);
    AddBRegister(BT848_O_VDELAY_LO);
    AddBRegister(BT848_E_VACTIVE_LO);
    AddBRegister(BT848_O_VACTIVE_LO);
    AddBRegister(BT848_E_HDELAY_LO);
    AddBRegister(BT848_O_HDELAY_LO);
    AddBRegister(BT848_E_HACTIVE_LO);
    AddBRegister(BT848_O_HACTIVE_LO);
    AddBRegister(BT848_E_HSCALE_HI);
    AddBRegister(BT848_O_HSCALE_HI);
    AddBRegister(BT848_E_HSCALE_LO);
    AddBRegister(BT848_O_HSCALE_LO);
    AddBRegister(BT848_BRIGHT);
    AddBRegister(BT848_E_CONTROL);
    AddBRegister(BT848_O_CONTROL);
    AddBRegister(BT848_CONTRAST_LO);
    AddBRegister(BT848_SAT_U_LO);
    AddBRegister(BT848_SAT_V_LO);
    AddBRegister(BT848_HUE);
    AddBRegister(BT848_E_SCLOOP);
    AddBRegister(BT848_O_SCLOOP);
    AddBRegister(BT848_WC_UP);
    AddBRegister(BT848_WC_DOWN);
    AddBRegister(BT848_VTOTAL_LO);
    AddBRegister(BT848_VTOTAL_HI);
    AddBRegister(BT848_DVSIF);
    AddBRegister(BT848_OFORM);
    AddBRegister(BT848_E_VSCALE_HI);
    AddBRegister(BT848_O_VSCALE_HI);
    AddBRegister(BT848_E_VSCALE_LO);
    AddBRegister(BT848_O_VSCALE_LO);
    AddBRegister(BT848_ADC);
    AddBRegister(BT848_E_VTC);
    AddBRegister(BT848_O_VTC);
    AddBRegister(BT848_COLOR_FMT);
    AddBRegister(BT848_COLOR_CTL);
    AddBRegister(BT848_CAP_CTL);
    AddBRegister(BT848_VBI_PACK_SIZE);
    AddBRegister(BT848_VBI_PACK_DEL);
    AddDWRegister(BT848_INT_MASK);
    AddDWRegister(BT848_GPIO_OUT_EN);
    AddDWRegister(BT848_GPIO_OUT_EN_HIBYTE);
    AddDWRegister(BT848_GPIO_DATA);
    AddDWRegister(BT848_RISC_STRT_ADD);
    AddWRegister(BT848_GPIO_DMA_CTL);
}


void __cdecl CX2388xRegSpy(TRegister** hRegisterListTail)
{
    AddDWRegister(CX2388X_DEVICE_STATUS);
    AddDWRegister(CX2388X_VIDEO_INPUT);
    AddDWRegister(CX2388X_TEMPORAL_DEC);
    AddDWRegister(CX2388X_AGC_BURST_DELAY);
    AddDWRegister(CX2388X_BRIGHT_CONTRAST);
    AddDWRegister(CX2388X_UVSATURATION);
    AddDWRegister(CX2388X_HUE);
    AddDWRegister(CX2388X_WHITE_CRUSH);
    AddDWRegister(CX2388X_PIXEL_CNT_NOTCH);
    AddDWRegister(CX2388X_HORZ_DELAY_EVEN);
    AddDWRegister(CX2388X_HORZ_DELAY_ODD);
    AddDWRegister(CX2388X_VERT_DELAY_EVEN);
    AddDWRegister(CX2388X_VERT_DELAY_ODD);
    AddDWRegister(CX2388X_VDELAYCCIR_EVEN);
    AddDWRegister(CX2388X_VDELAYCCIR_ODD);
    AddDWRegister(CX2388X_HACTIVE_EVEN);
    AddDWRegister(CX2388X_HACTIVE_ODD);
    AddDWRegister(CX2388X_VACTIVE_EVEN);
    AddDWRegister(CX2388X_VACTIVE_ODD);
    AddDWRegister(CX2388X_HSCALE_EVEN);
    AddDWRegister(CX2388X_HSCALE_ODD);
    AddDWRegister(CX2388X_VSCALE_EVEN);
    AddDWRegister(CX2388X_VSCALE_ODD);
    AddDWRegister(CX2388X_FILTER_EVEN);
    AddDWRegister(CX2388X_FILTER_ODD);
    AddDWRegister(CX2388X_FORMAT_2HCOMB);
    AddDWRegister(CX2388X_PLL);
    AddDWRegister(CX2388X_PLL_ADJUST);
    AddDWRegister(CX2388X_SAMPLERATECONV);
    AddDWRegister(CX2388X_SAMPLERATEFIFO);
    AddDWRegister(CX2388X_SUBCARRIERSTEP);
    AddDWRegister(CX2388X_SUBCARRIERSTEPDR);
    AddDWRegister(CX2388X_CAPTURECONTROL);
    AddDWRegister(CX2388X_VIDEO_COLOR_FORMAT);
    AddDWRegister(CX2388X_VBI_SIZE);
    AddDWRegister(CX2388X_FIELD_CAP_CNT);
    AddDWRegister(CX2388X_VIP_CONFIG);
    AddDWRegister(CX2388X_VIP_CONTBRGT);
    AddDWRegister(CX2388X_VIP_HSCALE);
    AddDWRegister(CX2388X_VIP_VSCALE);
    AddDWRegister(CX2388X_VBOS);
    AddDWRegister(CX2388X_PINMUX_IO);

    AddDWRegister(CX2388X_AGC_BACK_VBI);
    AddDWRegister(CX2388X_AGC_SYNC_SLICE);
    AddDWRegister(CX2388X_AGC_SYNC_TIP1);
    AddDWRegister(CX2388X_AGC_SYNC_TIP2);
    AddDWRegister(CX2388X_AGC_SYNC_TIP3);

    AddDWRegister(CX2388X_AGC_GAIN1);
    AddDWRegister(CX2388X_AGC_GAIN2);
    AddDWRegister(CX2388X_AGC_GAIN3);
    AddDWRegister(CX2388X_AGC_GAIN4);

    AddDWRegister(0x310700);
    AddDWRegister(0x310704);
    AddDWRegister(0x310708);
    AddDWRegister(0x31070C);
    AddDWRegister(0x310710);
    AddDWRegister(0x310714);
    AddDWRegister(0x310718);
    AddDWRegister(0x31071C);
    AddDWRegister(0x310720);
    AddDWRegister(0x310724);
    AddDWRegister(0x310728);
    AddDWRegister(0x31072C);

    AddDWRegister(MO_GP0_IO);
    AddDWRegister(MO_GP1_IO);
    AddDWRegister(MO_GP2_IO);
    AddDWRegister(MO_GP3_IO);
    AddDWRegister(MO_GPIO);
    AddDWRegister(MO_GPOE);
    
    AddDWRegister(MO_AUD_XFR_STAT);
    AddDWRegister(MO_AUDD_LNGTH);  
    AddDWRegister(MO_AUDR_LNGTH);  

    AddDWRegister(CX2388X_DEV_CNTRL2);
    
    AddDWRegister(MO_VID_DMACNTRL);
    AddDWRegister(MO_AUD_DMACNTRL);
    AddDWRegister(MO_TS_DMACNTRL);
    AddDWRegister(MO_VIP_DMACNTRL);
    AddDWRegister(MO_GPHST_DMACNTRL);
    AddDWRegister(CX2388X_PCI_INTMSK);
    AddDWRegister(CX2388X_VID_INTMSK);
    AddDWRegister(CX2388X_AUD_INTMSK);
    AddDWRegister(CX2388X_TS_INTMSK);
    AddDWRegister(CX2388X_VIP_INTMSK);
    AddDWRegister(CX2388X_GPHST_INTMSK);


    AddDWRegister(MO_DMA25_PTR2);
    AddDWRegister(MO_DMA25_CNT1);  
    AddDWRegister(MO_DMA25_CNT2);   
    AddDWRegister(MO_DMA26_PTR2);   
    AddDWRegister(MO_DMA26_CNT1);   
    AddDWRegister(MO_DMA26_CNT2);   

    AddDWRegister(0x0320d01);
    AddDWRegister(0x0320d02);
    AddDWRegister(0x0320d03);
    AddDWRegister(0x0320d04);
    AddDWRegister(0x0320d2a);
    AddDWRegister(0x0320d2b);

    AddDWRegister(AUD_POLY0_DDS_CONSTANT);
    AddDWRegister(AUD_IIR1_0_SEL);
    AddDWRegister(AUD_IIR1_1_SEL);
    AddDWRegister(AUD_IIR1_2_SEL);
    AddDWRegister(AUD_IIR1_3_SEL);
    AddDWRegister(AUD_IIR1_4_SEL);
    AddDWRegister(AUD_IIR1_5_SEL);
    AddDWRegister(AUD_IIR1_0_SHIFT);
    AddDWRegister(AUD_IIR1_1_SHIFT);
    AddDWRegister(AUD_IIR1_2_SHIFT);
    AddDWRegister(AUD_IIR1_3_SHIFT);
    AddDWRegister(AUD_IIR1_4_SHIFT);
    AddDWRegister(AUD_IIR1_5_SHIFT);
    AddDWRegister(AUD_IIR2_0_SEL);
    AddDWRegister(AUD_IIR2_1_SEL);
    AddDWRegister(AUD_IIR2_2_SEL);
    AddDWRegister(AUD_IIR2_3_SEL);
    AddDWRegister(AUD_IIR3_0_SEL);
    AddDWRegister(AUD_IIR3_1_SEL);
    AddDWRegister(AUD_IIR3_2_SEL);
    AddDWRegister(AUD_IIR3_0_SHIFT);
    AddDWRegister(AUD_IIR3_1_SHIFT);
    AddDWRegister(AUD_IIR3_2_SHIFT);
    AddDWRegister(AUD_IIR4_0_SEL);
    AddDWRegister(AUD_IIR4_1_SEL);
    AddDWRegister(AUD_IIR4_2_SEL);
    AddDWRegister(AUD_IIR4_0_SHIFT);
    AddDWRegister(AUD_IIR4_1_SHIFT);
    AddDWRegister(AUD_IIR4_2_SHIFT);
    AddDWRegister(AUD_IIR4_0_CA0);
    AddDWRegister(AUD_IIR4_0_CA1);
    AddDWRegister(AUD_IIR4_0_CA2);
    AddDWRegister(AUD_IIR4_0_CB0);
    AddDWRegister(AUD_IIR4_0_CB1);
    AddDWRegister(AUD_IIR4_1_CA0);
    AddDWRegister(AUD_IIR4_1_CA1);
    AddDWRegister(AUD_IIR4_1_CA2);
    AddDWRegister(AUD_IIR4_1_CB0);
    AddDWRegister(AUD_IIR4_1_CB1);
    AddDWRegister(AUD_IIR4_2_CA0);
    AddDWRegister(AUD_IIR4_2_CA1);
    AddDWRegister(AUD_IIR4_2_CA2);
    AddDWRegister(AUD_IIR4_2_CB0);
    AddDWRegister(AUD_IIR4_2_CB1);
    
    AddDWRegister(AUD_HP_MD_IIR4_1);
    AddDWRegister(AUD_HP_PROG_IIR4_1);
    
    AddDWRegister(AUD_DN0_FREQ);

    AddDWRegister(AUD_DN1_FREQ);
    AddDWRegister(AUD_DN1_SRC_SEL);
    AddDWRegister(AUD_DN1_SHFT);
    AddDWRegister(AUD_DN1_AFC);
    AddDWRegister(AUD_DN1_FREQ_SHIFT);

    AddDWRegister(AUD_DN2_FREQ);
    AddDWRegister(AUD_DN2_SRC_SEL);
    AddDWRegister(AUD_DN2_SHFT);
    AddDWRegister(AUD_DN2_AFC);
    AddDWRegister(AUD_DN2_FREQ_SHIFT);

    AddDWRegister(AUD_PDET_SRC);
    AddDWRegister(AUD_PDET_SHIFT);
    AddDWRegister(AUD_DEEMPH0_SRC_SEL);
    AddDWRegister(AUD_DEEMPH1_SRC_SEL);
    AddDWRegister(AUD_DEEMPH0_SHIFT);
    AddDWRegister(AUD_DEEMPH1_SHIFT);
    AddDWRegister(AUD_DEEMPH0_G0);
    AddDWRegister(AUD_DEEMPH0_A0);
    AddDWRegister(AUD_DEEMPH0_B0);
    AddDWRegister(AUD_DEEMPH0_A1);
    AddDWRegister(AUD_DEEMPH0_B1);
    AddDWRegister(AUD_DEEMPH1_G0);
    AddDWRegister(AUD_DEEMPH1_A0);
    AddDWRegister(AUD_DEEMPH1_B0);
    AddDWRegister(AUD_DEEMPH1_A1);
    AddDWRegister(AUD_DEEMPH1_B1);

    AddDWRegister(AUD_DMD_RA_DDS);
    AddDWRegister(AUD_RATE_ADJ1);
    AddDWRegister(AUD_RATE_ADJ2);
    AddDWRegister(AUD_RATE_ADJ3);
    AddDWRegister(AUD_RATE_ADJ4);
    AddDWRegister(AUD_RATE_ADJ5);
    AddDWRegister(AUD_C2_UP_THR);
    AddDWRegister(AUD_C2_LO_THR);
    AddDWRegister(AUD_C1_UP_THR);
    AddDWRegister(AUD_C1_LO_THR);
    AddDWRegister(AUD_CTL);
    
    AddDWRegister(AUD_DCOC_0_SRC);
    AddDWRegister(AUD_DCOC_1_SRC);
    AddDWRegister(AUD_DCOC1_SHIFT);
    AddDWRegister(AUD_DCOC_1_SHIFT_IN0);
    AddDWRegister(AUD_DCOC_1_SHIFT_IN1);
    AddDWRegister(AUD_DCOC_PASS_IN);
    AddDWRegister(AUD_DCOC_2_SRC);

    AddDWRegister(AUD_IIR4_0_SEL);
    AddDWRegister(AUD_POLY0_DDS_CONSTANT);
    AddDWRegister(AUD_PHASE_FIX_CTL);
    AddDWRegister(AUD_CORDIC_SHIFT_1);
    
    AddDWRegister(AUD_PLL_EN);
    AddDWRegister(AUD_PLL_PRESCALE);
    AddDWRegister(AUD_PLL_INT);
    
    AddDWRegister(AUD_AFE_12DB_EN);
    
    AddDWRegister(AUD_DBX_IN_GAIN);
    AddDWRegister(AUD_DBX_WBE_GAIN);
    AddDWRegister(AUD_DBX_SE_GAIN);
    AddDWRegister(AUD_OUT1_SHIFT);

    AddDWRegister(AUD_INIT);
    AddDWRegister(AUD_STATUS);
    AddDWRegister(AUD_VOL_CTL);
    AddDWRegister(AUD_BAL_CTL);
}


void __cdecl SAA7134RegSpy(TRegister** hRegisterListTail)
{
    AddDWRegister(SAA7134_GPIO_GPMODE);
    AddDWRegister(SAA7134_GPIO_GPSTATUS);
    AddBRegister(SAA7134_ANALOG_IN_CTRL1);      // Video input pin
    AddBRegister(SAA7134_ANALOG_IO_SELECT);     // Audio input pin
    AddDWRegister(SAA7134_AUDIO_CLOCK);         // Audio clock crystal

    // != 0 means card may have CCIR656
    AddDWRegister(SAA7134_VIDEO_PORT_CTRL0);    // CCIR656 video out
    AddDWRegister(SAA7134_VIDEO_PORT_CTRL4);    // CCIR656 video out
    AddBRegister(SAA7134_VIDEO_PORT_CTRL8);     // CCIR656 video out
    
    // != 0 means card may have i2s Audio
    AddBRegister(SAA7134_I2S_OUTPUT_SELECT);    // i2s Audio
    AddBRegister(SAA7134_I2S_OUTPUT_FORMAT);    // i2s Audio
    AddBRegister(SAA7134_I2S_OUTPUT_LEVEL);     // i2s Audio
    AddBRegister(SAA7134_I2S_AUDIO_OUTPUT);     // i2s Audio
    
    // != 0 means card may have DTV/DVB TS
    AddBRegister(SAA7134_TS_PARALLEL);          // Transport stream
    AddBRegister(SAA7134_TS_PARALLEL_SERIAL);   // Transport stream
    AddBRegister(SAA7134_TS_SERIAL0);           // Transport stream
    AddBRegister(SAA7134_TS_SERIAL1);           // Transport stream
    AddBRegister(SAA7134_TS_DMA0);              // Transport stream
    AddBRegister(SAA7134_TS_DMA1);              // Transport stream
    AddBRegister(SAA7134_TS_DMA2);              // Transport stream

    AddBRegister(SAA7134_SPECIAL_MODE);         // Propagated reset
}


void __cdecl SAA7130RegSpy(TRegister** hRegisterListTail)
{
    AddDWRegister(SAA7134_GPIO_GPMODE);
    AddDWRegister(SAA7134_GPIO_GPSTATUS);
    AddBRegister(SAA7134_ANALOG_IN_CTRL1);      // Video input pin
    AddBRegister(SAA7134_ANALOG_IO_SELECT);     // Audio input pin

    // != 0 means card may have CCIR656
    AddDWRegister(SAA7134_VIDEO_PORT_CTRL0);    // CCIR656 video out
    AddDWRegister(SAA7134_VIDEO_PORT_CTRL4);    // CCIR656 video out
    AddBRegister(SAA7134_VIDEO_PORT_CTRL8);     // CCIR656 video out
    
    // != 0 means card may have i2s Audio
    AddBRegister(SAA7134_I2S_OUTPUT_SELECT);    // i2s Audio
    AddBRegister(SAA7134_I2S_OUTPUT_FORMAT);    // i2s Audio
    AddBRegister(SAA7134_I2S_OUTPUT_LEVEL);     // i2s Audio
    AddBRegister(SAA7134_I2S_AUDIO_OUTPUT);     // i2s Audio
    
    // != 0 means card may have DTV/DVB TS
    AddBRegister(SAA7134_TS_PARALLEL);          // Transport stream
    AddBRegister(SAA7134_TS_PARALLEL_SERIAL);   // Transport stream
    AddBRegister(SAA7134_TS_SERIAL0);           // Transport stream
    AddBRegister(SAA7134_TS_SERIAL1);           // Transport stream
    AddBRegister(SAA7134_TS_DMA0);              // Transport stream
    AddBRegister(SAA7134_TS_DMA1);              // Transport stream
    AddBRegister(SAA7134_TS_DMA2);              // Transport stream

    AddBRegister(SAA7134_SPECIAL_MODE);         // Propagated reset
}


void __cdecl SAA7133RegSpy(TRegister** hRegisterListTail)
{
    AddDWRegister(SAA7134_GPIO_GPMODE);
    AddDWRegister(SAA7134_GPIO_GPSTATUS);
    AddBRegister(SAA7134_ANALOG_IN_CTRL1);      // Video input pin
    AddBRegister(SAA7134_ANALOG_IO_SELECT);     // Audio input pin
    AddDWRegister(SAA7134_AUDIO_CLOCK);         // Audio clock crystal

    // != 0 means card may have CCIR656
    AddDWRegister(SAA7134_VIDEO_PORT_CTRL0);    // CCIR656 video out
    AddDWRegister(SAA7134_VIDEO_PORT_CTRL4);    // CCIR656 video out
    AddBRegister(SAA7134_VIDEO_PORT_CTRL8);     // CCIR656 video out
    
    // != 0 means card may have i2s Audio
    AddBRegister(SAA7134_I2S_OUTPUT_SELECT);    // i2s Audio
    AddBRegister(SAA7134_I2S_OUTPUT_FORMAT);    // i2s Audio
    AddBRegister(SAA7134_I2S_OUTPUT_LEVEL);     // i2s Audio
    AddBRegister(SAA7134_I2S_AUDIO_OUTPUT);     // i2s Audio
    
    // != 0 means card may have DTV/DVB TS
    AddBRegister(SAA7134_TS_PARALLEL);          // Transport stream
    AddBRegister(SAA7134_TS_PARALLEL_SERIAL);   // Transport stream
    AddBRegister(SAA7134_TS_SERIAL0);           // Transport stream
    AddBRegister(SAA7134_TS_SERIAL1);           // Transport stream
    AddBRegister(SAA7134_TS_DMA0);              // Transport stream
    AddBRegister(SAA7134_TS_DMA1);              // Transport stream
    AddBRegister(SAA7134_TS_DMA2);              // Transport stream

    AddBRegister(SAA7134_SPECIAL_MODE);         // Propagated reset

    // Audio control registers.  For figuring out BTSC
    AddDWRegister(0x140);                       // NICAM data & status
    AddDWRegister(0x144);                       // Sound decoder status
    AddDWRegister(0x148);                       // Demodulator configuration
    AddDWRegister(0x14C);                       // SIF gain control
    AddDWRegister(0x150);                       // Carrier 1 frequency
    AddDWRegister(0x154);                       // Carrier 2 frequency
    AddDWRegister(0x158);                       // PCI capture format
    AddDWRegister(0x164);                       // FM decoder control
    AddDWRegister(0x16C);                       // NICAM decoder control
    AddDWRegister(0x174);                       // Analog I/O
}


void __cdecl SAA7146RegSpy(TRegister** hRegisterListTail)
{
    AddDWRegister(0x0E0);
    AddDWRegister(0x0DC);
    AddDWRegister(0x10C);
    AddDWRegister(0x110);
    AddDWRegister(0x114);
}

//  struct TChip:
//  {
//      char*       Name;
//      DWORD       VendorId;
//      DWORD       DeviceId;
//      REGSPYFUNC* SetupFunction;
//  }

TChip Chips[] =
{
    {
        "BT848",
        0x109e,
        0x0350,
        BT848RegSpy,
    },
    {
        "BT849",
        0x109e,
        0x0351,
        BT848RegSpy,
    },
    {
        "BT878",
        0x109e,
        0x036e,
        BT848RegSpy,
    },
    {
        "BT878A",
        0x109e,
        0x036f,
        BT848RegSpy,
    },
    {
        "CX2388x",
        0x14F1,
        0x8800,
        CX2388xRegSpy,
    },
    {
        "SAA7134",
        0x1131,
        0x7134,
        SAA7134RegSpy,
    },
    {
        "SAA7130",
        0x1131,
        0x7130,
        SAA7130RegSpy,
    },
    {
        "SAA7133",
        0x1131,
        0x7133,
        SAA7133RegSpy,
    },
    {
        "SAA7146",
        0x1131,
        0x7146,
        SAA7146RegSpy,
        }

};



typedef struct _REGDISPLAYINFO
{
    TRegister**         hRegisterList;
    LPCRITICAL_SECTION  pRegisterListMutex;
    BYTE*               pLogState;
    LONG                nScrollPos;
    BOOL                bRedrawAll;
} TREGDISPLAYINFO;

BOOL APIENTRY MainWindowProc(HWND hDlg, UINT uMsg, UINT wParam, LONG lParam);
LRESULT CALLBACK RegDisplayProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);


HWND                ghMainDialog = NULL;
HHOOK               ghKeyboardHook = NULL;
TREGDISPLAYINFO     gRegDisplayInfo;


//  Converts a number into a string represented binary
//  NOTE: lpBuffer must be at least Size * 8 + 6 in size!
void NumberToBinary(char *lpBuffer, DWORD Number, BYTE Size, DWORD Mask)
{
    BYTE    Byte, Bmask;
    WORD    Word, Wmask;
    int     i;

    *lpBuffer++ = '(';

    // Avoid endian and bit order differences
    switch(Size)
    {
    case 1:
        Byte = (BYTE)Number;
        Bmask = (BYTE)Mask;
        for(i = 7; i >= 0; i--)
        {
            *lpBuffer++ = ((Bmask & (1 << i)) ? ((Byte & (1 << i)) ? '1' : '0') : '-');
        }
        break;

    case 2:
        Word = (WORD)Number;
        Wmask = (WORD)Mask;
        for(i = 15; i >= 0; i--)
        {
            *lpBuffer++ = ((Mask & (1 << i)) ? ((Word & (1 << i)) ? '1' : '0') : '-');
            if(i == 8)
            {
                *lpBuffer++ = ' ';
            }
        }
        break;

    case 4:
        for(i = 31; i >= 0; i--)
        {
            *lpBuffer++ = ((Mask & (1 << i)) ? ((Number & (1 << i)) ? '1' : '0') : '-');
            if((i % 8 == 0) && (i != 0))
            {
                *lpBuffer++ = ' ';
            }
        }
        break;

    default:
        break;
    }

    *lpBuffer++ = ')';
    *lpBuffer = '\0';
}


//  Saves the register list to a user prompted file
BOOL SaveToFile(HWND hOwner, TSource* pSource, TRegister* pRegisterList, BYTE nLogState)
{
    char szFilename[256];

    sprintf(szFilename, "%s_%d.txt", pSource->Chip->Name, pSource->DeviceIndex);

    OPENFILENAME OpenFileName;
    ZeroMemory(&OpenFileName, sizeof(OPENFILENAME));
    OpenFileName.lStructSize = sizeof(OPENFILENAME);
    OpenFileName.hwndOwner = hOwner;
    OpenFileName.lpstrFilter = "Text Documents (*.txt)\0*.txt\0All Files\0*.*\0\0";
    OpenFileName.lpstrCustomFilter = NULL;
    OpenFileName.nMaxCustFilter = 0;
    OpenFileName.nFilterIndex = 1;
    OpenFileName.lpstrFile = szFilename;
    OpenFileName.nMaxFile = sizeof(szFilename);
    OpenFileName.lpstrFileTitle = NULL;
    OpenFileName.lpstrInitialDir = NULL;
    OpenFileName.lpstrTitle = NULL;
    OpenFileName.Flags = OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST ;
    OpenFileName.lpstrDefExt = "txt";

    if(GetSaveFileName(&OpenFileName) == FALSE)
    {
        return FALSE;
    }

    FILE* hFile = fopen(szFilename, "w");
    if(hFile == NULL)
    {

        return FALSE;
    }

    TRegister*  pRegister;
    char        szFormat[32];
    char        szBinary[38];
    char        szHex[11];
    char        szWas[16];
    char        szSameAs[MAX_LOG_STATES*3 + 10];
    char        szName[64];
    char        cStar;
    DWORD       Value;
    DWORD*      pNewValue;
    LONG        nChanges;


    fprintf(hFile, "%s Card [%d]:\n\n", pSource->Chip->Name,    pSource->DeviceIndex);
    fprintf(hFile, "%-20s 0x%04x\n", "Vendor ID:",  pSource->Chip->VendorId);
    fprintf(hFile, "%-20s 0x%04x\n", "Device ID:", pSource->Chip->DeviceId);
    fprintf(hFile, "%-20s 0x%08x\n", "Subsystem ID:", pSource->SubSystemId);

    if(nLogState > 0)
    {
        fprintf(hFile, "\n\n%d states dumped\n", nLogState+1);
    }

    fprintf(hFile, "\n");

    for(int i(0); i < nLogState; i++)
    {
        fprintf(hFile, "----------------------------------------------------------------------------------\n\n");
        fprintf(hFile, "%s Card - State %d:\n", pSource->Chip->Name, i);

        for(pRegister = pRegisterList; pRegister != NULL; pRegister = pRegister->Next)
        {
            Value       = pRegister->LogStates[i];

            if(i + 1 == nLogState)
            {
                pNewValue = &pRegister->LastValue;
            }
            else
            {
                pNewValue = &pRegister->LogStates[i+1];
            }
            cStar       = (*pNewValue == Value) ? ' ' : '*';

            if((i > 0) && (pRegister->LogStates[i-1] != Value))
            {
                sprintf(szFormat, "(was: %%0%dx)", pRegister->Size*2);
                sprintf(szWas, szFormat, pRegister->LogStates[i-1]);
            }
            else
            {
                *szWas = '\0';
            }

            NumberToBinary(szBinary, Value, pRegister->Size, 0xFFFFFFFF);

            sprintf(szName, "%s:", pRegister->Name);
            sprintf(szFormat, "%%0%dx %c", pRegister->Size*2, cStar);
            sprintf(szHex, szFormat, Value);

            fprintf(hFile, "%-32s %-10s %-37s  %-15s\n", szName, szHex, szBinary, szWas);
        }

        if(i + 1 == nLogState)
        {
            fprintf(hFile, "\n\nChanges: State %d -> Register Dump:\n", i, i+1);
        }
        else
        {
            fprintf(hFile, "\n\nChanges: State %d -> State %d:\n", i, i+1);
        }

        nChanges = 0;
        for(pRegister = pRegisterList; pRegister != NULL; pRegister = pRegister->Next)
        {
            pNewValue   = (i+1 == nLogState ? &pRegister->LastValue : &pRegister->LogStates[i+1]);
            if(*pNewValue != pRegister->LogStates[i])
            {
                Value       = pRegister->LogStates[i];
                NumberToBinary(szBinary, Value, pRegister->Size, Value ^ *pNewValue);

                *szSameAs = '\0';
                for(int j(0); j < i; j++)
                {
                    if(pRegister->LogStates[j] == *pNewValue)
                    {
                        sprintf(szSameAs, "%s%s %d", szSameAs,
                            (*szSameAs == '\0' ? "" : ","), j);
                    }
                }
                if(*szSameAs != '\0')
                {
                    memmove(&szSameAs[8], szSameAs, strlen(szSameAs)+1);
                    memcpy(szSameAs, "(same as", 8);
                    strcat(szSameAs, ")");
                }

                sprintf(szName, "%s:", pRegister->Name);

                sprintf(szFormat, "%%0%dx", pRegister->Size*2);
                sprintf(szHex, szFormat, Value);
                fprintf(hFile, "%-32s %-8s -> ", szName, szHex);

                sprintf(szFormat, "%%0%dx", pRegister->Size*2);
                sprintf(szHex, szFormat, *pNewValue);
                fprintf(hFile, "%-8s  %-37s  %s\n", szHex, szBinary, szSameAs);
                nChanges++;
            }
        }

        fprintf(hFile, "\n%d changes\n\n\n", nChanges);
    }

    fprintf(hFile, "=================================================================================\n\n");
    fprintf(hFile, "%s Card - Register Dump:\n", pSource->Chip->Name);

    for(pRegister = pRegisterList; pRegister != NULL; pRegister = pRegister->Next)
    {
        Value       = pRegister->LastValue;

        if((nLogState > 0) && (pRegister->LogStates[nLogState-1] != Value))
        {
            sprintf(szFormat, "(was: %%0%dx)", pRegister->Size*2);
            sprintf(szWas, szFormat, pRegister->LogStates[nLogState-1]);
        }
        else
        {
            *szWas = '\0';
        }

        NumberToBinary(szBinary, Value, pRegister->Size, 0xFFFFFFFF);

        sprintf(szName, "%s:", pRegister->Name);
        sprintf(szFormat, "%%0%dx", pRegister->Size*2);
        sprintf(szHex, szFormat, Value);

        fprintf(hFile, "%-32s %-8s   %-37s  %-15s\n", szName, szHex, szBinary, szWas);
    }

    fprintf(hFile, "\nend of dump\n");
    fclose(hFile);

    return TRUE;
}


void GetCustomRegisterList(LPSTR lpSection, TRegister** hRegisterListTail)
{
    char    szBuffer[5120];
    char    szFullPath[256];
    char    szName[32];
    DWORD   dwReg;
    BYTE    nSize;

    DWORD nLength = GetModuleFileName(NULL, szFullPath, sizeof(szFullPath));

    if(nLength < 4)
    {
        return;
    }

    memcpy(&szFullPath[nLength-4], ".ini", 4);

    DWORD nRead = GetPrivateProfileSection(lpSection, szBuffer,
        sizeof(szBuffer), szFullPath);

    BOOL bHex;
    BYTE nDigit;

    char* c = szBuffer;

    while(c < &szBuffer[nRead])
    {
        // Get the register size
        nSize = 0;
        switch(*c)
        {
        case 'b':
        case 'B':
            nSize = 1;
            break;
        case 'w':
        case 'W':
            nSize = 2;
            break;
        case 'd':
        case 'D':
            nSize = 4;
            break;
        }

        // Use BYTE if unspecified
        if(nSize == 0)
        {
            nSize = 1;
        }
        else if(*++c == ',')
        {
            c++;
        }

        // Init the pointer for creating the name
        char* s = szName;

        // Check for a "0x" or "x" prefix
        if(*c == '0')
        {
            *s++ = *c++;
        }
        if(*c == 'x' || *c == 'X')
        {
            *s++ = *c++;
            bHex = TRUE;
        }
        else
        {
            bHex = FALSE;
        }

        dwReg = 0UL;

        // Parse the register
        for( ; *c != '=' && *c != '\0'; c++)
        {
            // Save the characters of c as we go
            if(s < &szName[sizeof(szName)-1])
            {
                *s++ = *c;
            }

            if(*c < '0' || *c > '9')
            {
                if(!bHex || ((*c < 'a' || *c > 'f') && (*c < 'A' || *c > 'F')))
                {
                    break;
                }
                if(*c >= 'a')
                {
                    *c -= 'a' - 'A';
                }
            }

            if(*c >= 'A' && *c <= 'F')
            {
                nDigit = *c - 'A' + 0xA;
            }
            else
            {
                nDigit = *c - '0';
            }

            if(bHex)
            {
                dwReg *= 0x10;
            }
            else
            {
                dwReg *= 10;
            }

            dwReg += nDigit;
        }

        if(*c != '=' && *c != '\0')
        {
            // Skip this set
            while(*c++ != '\0')
            {
                // do nothing
            }
            continue;
        }

        if(*c == '=')
        {
            if(*++c != '\0')
            {
                // Get the name
                for(s = szName; *c != '\0' && s < &szName[sizeof(szName)-1]; )
                {
                    *s++ = *c++;
                }

				while(*c != '\0')
				{
					c++;
				}
            }
        }

        // Add the null terminator
        *s++ = *c++;

        *hRegisterListTail = (TRegister*)malloc(sizeof(TRegister));
        if(*hRegisterListTail == NULL)
        {
            break;
        }

        (*hRegisterListTail)->Name = (char*)malloc(s - szName);
        strcpy((*hRegisterListTail)->Name, szName);
        (*hRegisterListTail)->bAlloced = TRUE;
        (*hRegisterListTail)->Offset = dwReg;
        (*hRegisterListTail)->Size = nSize;
        (*hRegisterListTail)->bDraw = FALSE;
        *(hRegisterListTail = &(*hRegisterListTail)->Next) = NULL;
    }
}


//  Moves and resizes window using delta values
BOOL DeltaWindow(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint)
{
    RECT Rect;

    GetWindowRect(hWnd, &Rect);

    Rect.right += nWidth - Rect.left;
    Rect.bottom += nHeight - Rect.top;
    Rect.left += X;
    Rect.top += Y;

    ScreenToClient(GetParent(hWnd), (LPPOINT)&Rect);

    return MoveWindow(hWnd, Rect.left, Rect.top, Rect.right, Rect.bottom, bRepaint);
}


//  Creates the font for the register display
//  Caller must delete hFont with DeleteObject() when done.
HFONT CreateRegDisplayFont()
{
    HFONT hFont;

    hFont = CreateFont(-REG_FONT_HEIGHT, 0, 0, 0, FW_NORMAL, 0, 0, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH, REG_FONT_NAME);

    return hFont;
}


//  This proc handles the drawing of the register
//  display box
LRESULT CALLBACK RegDisplayProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HDC      hOffscreenDC        = NULL;
    static HBITMAP  hOffscreenBitmap    = NULL;
    static HBRUSH   hLightStripeBrush   = NULL;
    static HFONT    hDisplayFont        = NULL;
    static LONG     nDisplayWidth       = 1;
    static LONG     nDisplayHeight      = 1;
    static LONG     nLastScrollPos      = 0;
    BOOL            bRedrawAll          = FALSE;

    switch(uMsg)
    {
    case WM_PAINT:

        // Make sure there is an update region
        if(GetUpdateRect(hWnd, NULL, FALSE))
        {
            PAINTSTRUCT PaintStruct;

            HDC hDC = BeginPaint(hWnd, &PaintStruct);

            // Create the font if we need to
            if(hDisplayFont == NULL)
            {
                if((hDisplayFont = CreateRegDisplayFont()) == NULL)
                {
                    MessageBox(GetParent(hWnd), "Can't create font", "RegSpy", MB_OK);
                    SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0);
                    break;
                }
            }

            // Create the offscreen buffer if we need to
            if(hOffscreenDC == NULL)
            {
                if((hOffscreenDC = CreateCompatibleDC(hDC)) == NULL)
                {
                    MessageBox(GetParent(hWnd), "Can't create display", "RegSpy", MB_OK);
                    SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0);
                    break;
                }

                SelectObject(hOffscreenDC, hDisplayFont);
            }

            bRedrawAll = gRegDisplayInfo.bRedrawAll;

            // Create the bitmap for the offscreen buffer
            if(hOffscreenBitmap == NULL)
            {
                RECT Rect;

                GetClientRect(hWnd, &Rect);

                nDisplayWidth = Rect.right;
                nDisplayHeight = Rect.bottom;

                hOffscreenBitmap = CreateCompatibleBitmap(hDC,
                    nDisplayWidth, nDisplayHeight);

                if(hOffscreenBitmap == NULL)
                {
                    MessageBox(GetParent(hWnd), "Can't create display", "RegSpy", MB_OK);
                    SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0);
                    break;
                }

                SelectObject(hOffscreenDC, hOffscreenBitmap);
                FillRect(hOffscreenDC, &Rect, GetSysColorBrush(COLOR_3DFACE));
                SetBkMode(hOffscreenDC, TRANSPARENT);

                bRedrawAll = TRUE;
            }

            // Create the lighter shade brush for striping
            if(hLightStripeBrush == NULL)
            {
                COLORREF ColorRef = GetSysColor(COLOR_3DFACE);

                // This won't always work
                ColorRef += 0x000A0A0A;

                hLightStripeBrush = CreateSolidBrush(ColorRef);
            }

            // Get mutex over the register list
            EnterCriticalSection(gRegDisplayInfo.pRegisterListMutex);

            TRegister* pRegisterList = *gRegDisplayInfo.hRegisterList;

            if(pRegisterList != NULL)
            {
                RECT        TextRect    = { 0, 0, nDisplayWidth, REG_FONT_HEIGHT };
                COLORREF    TextColor;
                LONG        nScrollPos  = gRegDisplayInfo.nScrollPos;
                BYTE        nLogState   = *gRegDisplayInfo.pLogState;
                TRegister*  pRegister;
                char        szFormat[8];
                char        szBuffer[9];

                OffsetRect(&TextRect, 0, -nScrollPos);

                if(nScrollPos != nLastScrollPos)
                {
                    bRedrawAll = TRUE;
                    nLastScrollPos = nScrollPos;
                }

                for(pRegister = pRegisterList;
                    pRegister != NULL;
                    pRegister = pRegister->Next)
                {
                    if(TextRect.bottom <= 0)
                    {
                        OffsetRect(&TextRect, 0, REG_FONT_HEIGHT);
                        continue;
                    }
                    if(TextRect.top >= nDisplayHeight)
                    {
                        break;
                    }

                    if(bRedrawAll == TRUE || pRegister->bDraw == TRUE)
                    {
                        if(pRegister->FadeTicks > 0)
                        {
                            TextColor = 0x000000a0;
                        }
                        else
                        {
                            TextColor = GetSysColor(COLOR_BTNTEXT);

                            if(nLogState > 0)
                            {
                                // Is it different from the last state?
                                if(pRegister->LogStates[nLogState-1] != pRegister->LastValue)
                                {
                                    TextColor = 0x00d00000;
                                }
                                else
                                {
                                    // Is it different from any previous state?
                                    for(int i(0); i < nLogState-1; i++)
                                    {
                                        if(pRegister->LogStates[i] != pRegister->LastValue)
                                        {
                                            TextColor = 0x00800000;
                                            break;
                                        }
                                    }
                                }
                            }
                        }

                        sprintf(szFormat, "%%0%dx", pRegister->Size*2);
                        sprintf(szBuffer, szFormat, pRegister->LastValue);

                        HBRUSH hBackgroundBrush = GetSysColorBrush(COLOR_3DFACE);

                        if((TextRect.top + nScrollPos) % (REG_FONT_HEIGHT * 2) != 0)
                        {
                            if(hLightStripeBrush != NULL)
                            {
                                hBackgroundBrush = hLightStripeBrush;
                            }
                        }

                        FillRect(hOffscreenDC, &TextRect, hBackgroundBrush);

                        SetTextColor(hOffscreenDC, TextColor);
                        SetTextAlign(hOffscreenDC, TA_LEFT);
                        TextOut(hOffscreenDC, TextRect.left, TextRect.top, pRegister->Name, strlen(pRegister->Name));
                        SetTextAlign(hOffscreenDC, TA_RIGHT);
                        TextOut(hOffscreenDC, TextRect.right, TextRect.top, szBuffer, strlen(szBuffer));

                        pRegister->bDraw = FALSE;
                    }

                    OffsetRect(&TextRect, 0, REG_FONT_HEIGHT);
                }
            }
            else
            {
                RECT Rect = { 0, 0, nDisplayWidth, nDisplayHeight };
                FillRect(hOffscreenDC, &Rect, GetSysColorBrush(COLOR_3DFACE));
            }

            // Release the mutex
            LeaveCriticalSection(gRegDisplayInfo.pRegisterListMutex);

            gRegDisplayInfo.bRedrawAll = FALSE;

            // Blit the offscreen buffer onto the main display
            BitBlt(hDC, 0, 0, nDisplayWidth, nDisplayHeight, hOffscreenDC, 0, 0, SRCCOPY);

            EndPaint(hWnd, &PaintStruct);
        }
        break;

    case WM_DESTROY:
        if(hDisplayFont != NULL)
        {
            DeleteObject(hDisplayFont);
        }
        if(hOffscreenBitmap != NULL)
        {
            DeleteObject(hOffscreenBitmap);
        }
        if(hOffscreenDC != NULL)
        {
            DeleteDC(hOffscreenDC);
        }
        if(hLightStripeBrush != NULL)
        {
            DeleteObject(hLightStripeBrush);
        }
        break;

    case WM_SIZE:
        if(wParam != SIZE_MINIMIZED)
        {
            // Delete the bitmap and it will be recreated for the
            // new size in WM_PAINT
            if(hOffscreenBitmap != NULL)
            {
                DeleteObject(hOffscreenBitmap);
                hOffscreenBitmap = NULL;
            }
        }
        break;

    case WM_MOUSEACTIVATE:
        SetFocus(hWnd);
        break;

    default:
        break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


//  This call back monitors keyboard events in the thread
//  and passes them onto the main dialog
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if(nCode == HC_ACTION)
    {
        SendMessage(ghMainDialog, ((HIWORD(lParam) & KF_UP) ? WM_KEYUP : WM_KEYDOWN),
            wParam, lParam);
    }
    return CallNextHookEx(ghKeyboardHook, nCode, wParam, lParam);
}


//  This is the main proc that handles all events in
//  the main dialog.  Warning:  BIG function!
BOOL APIENTRY MainWindowProc(HWND hDlg, UINT uMsg, UINT wParam, LONG lParam)
{
    static CHardwareDriver* pHardwareDriver;
    static CGenericCard*    pCard = NULL;
    static TSource*         pSourceList;
    static TRegister*       pRegisterList = NULL;
    static TRegister**      hRegisterListTail = &pRegisterList;
    static CRITICAL_SECTION RegisterListMutex;
    static HWND             hSourceSelect = NULL;
    static HWND             hRegDisplay = NULL;
    static HWND             hRegScroll = NULL;
    static HWND             hMoveUpChanges = NULL;
    static HWND             hLogState = NULL;
    static HWND             hDumpToFile = NULL;
    static LONG             ScrollPos = 0;
    static LONG             ScrollMax = 0;
    static LONG             ScrollRange = 0;
    static LONG             ScrollPage = 0;
    static BYTE             nLogState;
    static BOOL             bFirstRead = TRUE;
    static BOOL             bMoveUpChanges = FALSE;
    static BOOL             bResetLogState = FALSE;
    static LONG             nDialogWidth;
    static LONG             nDialogHeight;
    TSource*                pSource;
    TRegister*              pRegister;
    RECT                    Rect;
    int                     ItemIndex;
    char                    szBuffer[64];

    switch(uMsg)
    {
    case WM_INITDIALOG:
        pSourceList     = (TSource*)        ((void**)lParam)[0];
        pHardwareDriver = (CHardwareDriver*)((void**)lParam)[1];

        ghMainDialog    = hDlg;

        hSourceSelect   = GetDlgItem(hDlg, IDC_SOURCESELECT);
        hRegDisplay     = GetDlgItem(hDlg, IDC_REGDISPLAY);
        hRegScroll      = GetDlgItem(hDlg, IDC_REGSCROLL);
        hMoveUpChanges  = GetDlgItem(hDlg, IDC_MOVEUPCHANGES);
        hLogState       = GetDlgItem(hDlg, IDC_LOGSTATE);
        hDumpToFile     = GetDlgItem(hDlg, IDC_DUMPTOFILE);


        // Fill the source select combo box with cards
        for(pSource = pSourceList; pSource != NULL; pSource = pSource->Next)
        {
            sprintf(szBuffer, "%s Card [%d]", pSource->Chip->Name, pSource->DeviceIndex);
            ItemIndex = SendMessage(hSourceSelect, CB_ADDSTRING, 0, (LPARAM)szBuffer);
            SendMessage(hSourceSelect, CB_SETITEMDATA, ItemIndex, (LPARAM)pSource);
            if(pSource == pSourceList)
            {
                SendMessage(hSourceSelect, CB_SETCURSEL, ItemIndex, 0);
            }
        }

        // Remember the dialog size
        GetClientRect(hDlg, &Rect);

        nDialogWidth = Rect.right;
        nDialogHeight = Rect.bottom;

        // Install a keyboard hook so we can easily monitor keypresses
        ghKeyboardHook = SetWindowsHookEx(WH_KEYBOARD, KeyboardHookProc,
            NULL, GetCurrentThreadId());

        // Initialize the register list mutex
        InitializeCriticalSection(&RegisterListMutex);

        // Create the register display control
        gRegDisplayInfo.hRegisterList       = &pRegisterList;
        gRegDisplayInfo.pRegisterListMutex  = &RegisterListMutex;
        gRegDisplayInfo.pLogState           = &nLogState;
        gRegDisplayInfo.nScrollPos          = 0;

        SetWindowLong(hRegDisplay, GWL_WNDPROC, (LONG)RegDisplayProc);

        if(pSourceList == NULL)
        {
            ItemIndex = SendMessage(hSourceSelect, CB_ADDSTRING, 0, (LPARAM)"No Card Found");
            SendMessage(hSourceSelect, CB_SETITEMDATA, ItemIndex, NULL);
            SendMessage(hSourceSelect, CB_SETCURSEL, ItemIndex, 0);
        }
        else
        {
            EnableWindow(hSourceSelect, TRUE);
            SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_SOURCESELECT, CBN_SELCHANGE), 0);
        }
        return 0;

    case WM_TIMER:
        if (wParam == IDC_REFRESH)
        {
            KillTimer(hDlg, IDC_REFRESH);
            EnterCriticalSection(&RegisterListMutex);

            if(pCard != NULL && pRegisterList != NULL)
            {
                DWORD       Value;
                BOOL        bChanged = FALSE;
                TRegister** hUpInsert = &pRegisterList;
                TRegister** hPreviousNext = &pRegisterList;
                TRegister*  pUptoRemoved = NULL;

                for(pRegister = pRegisterList;
                    pRegister != NULL;
                    pRegister = *hPreviousNext)
                {
                    switch(pRegister->Size)
                    {
                    case 1: Value = pCard->ReadByte(pRegister->Offset); break;
                    case 2: Value = pCard->ReadWord(pRegister->Offset); break;
                    case 4: Value = pCard->ReadDword(pRegister->Offset); break;
                    default: pRegister->Size = 4; Value = 0UL; break;
                    }

                    if(bFirstRead == TRUE)
                    {
                        pRegister->FadeTicks = 0;
                        pRegister->LastValue = Value;
                        pRegister->bDraw = TRUE;
                        bChanged = TRUE;
                    }
                    else if(Value != pRegister->LastValue)
                    {
                        pRegister->FadeTicks = CHANGE_FADE_TICKS;
                        pRegister->LastValue = Value;
                        pRegister->bDraw = TRUE;
                        bChanged = TRUE;

                        if(bMoveUpChanges == TRUE)
                        {
                            if(pRegister != *hUpInsert)
                            {
                                // Remove from original position
                                *hPreviousNext = pRegister->Next;

                                // Remember the last non-removed object
                                pUptoRemoved = pRegister->Next;

                                // Insert up
                                pRegister->Next = *hUpInsert;
                                *hUpInsert = pRegister;
                                hUpInsert = &pRegister->Next;
                                continue;
                            }
                        }
                    }
                    else if(pRegister->FadeTicks > 0)
                    {
                        if(--pRegister->FadeTicks == 0)
                        {
                            pRegister->bDraw = TRUE;
                            bChanged = TRUE;
                        }
                    }
                    hPreviousNext = &pRegister->Next;
                }

                bFirstRead = FALSE;

                if(bChanged == TRUE)
                {
                    if(hUpInsert != &pRegisterList)
                    {
                        // Mark all the ones that shifted as a
                        // result of others moving up
                        for(pRegister = *hUpInsert;
                            pRegister != pUptoRemoved;
                            pRegister = pRegister->Next)
                        {
                            pRegister->bDraw = TRUE;
                        }
                    }

                    InvalidateRect(hRegDisplay, NULL, FALSE);
                }

                SetTimer(hDlg, IDC_REFRESH, REG_REFRESH_TICKS, NULL);
            }

            LeaveCriticalSection(&RegisterListMutex);
        }
        return 0;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
        case IDCANCEL:
            KillTimer(hDlg, IDC_REFRESH);
            EnterCriticalSection(&RegisterListMutex);

            if(pCard != NULL)
            {
                delete pCard;
                pCard = NULL;
            }

            while((pRegister = pRegisterList) != NULL)
            {
                pRegisterList = pRegister->Next;
                if(pRegister->bAlloced)
                {
                    free(pRegister->Name);
                }
                free(pRegister);
            }

            LeaveCriticalSection(&RegisterListMutex);
            DeleteCriticalSection(&RegisterListMutex);

            EndDialog(hDlg, TRUE);
            break;

        case IDC_SOURCESELECT:
            if(HIWORD(wParam) == CBN_SELCHANGE)
            {
                KillTimer(hDlg, IDC_REFRESH);
                EnterCriticalSection(&RegisterListMutex);

                hRegisterListTail = &pRegisterList;
                while((pRegister = pRegisterList) != NULL)
                {
                    pRegisterList = pRegister->Next;
                    if(pRegister->bAlloced)
                    {
                        free(pRegister->Name);
                    }
                    free(pRegister);
                }

                if(pCard != NULL)
                {
                    delete pCard;
                    pCard = NULL;
                }

                nLogState = 0;

                // Find out which one is selected
                ItemIndex = SendMessage(hSourceSelect, CB_GETCURSEL, 0, 0);
                pSource = (TSource*)SendMessage(hSourceSelect, CB_GETITEMDATA, ItemIndex, 0);

                if(pSource != NULL)
                {
                    // Set the hardware info
                    SetDlgItemText(hDlg, IDC_CHIPTYPE, pSource->Chip->Name);

                    sprintf(szBuffer,"0x%04X", pSource->Chip->VendorId);
                    SetDlgItemText(hDlg, IDC_VENDORID, szBuffer);

                    sprintf(szBuffer,"0x%04X", pSource->Chip->DeviceId);
                    SetDlgItemText(hDlg, IDC_DEVICEID, szBuffer);

                    sprintf(szBuffer,"0x%08X", pSource->SubSystemId);
                    SetDlgItemText(hDlg, IDC_SUBSYSTEMID, szBuffer);

                    // Open the card
                    pCard = new CGenericCard(pHardwareDriver);
                    if(pCard->OpenPCICard(
                                            pSource->Chip->VendorId,
                                            pSource->Chip->DeviceId,
                                            pSource->DeviceIndex
                                         ) == TRUE)
                    {
                        GetCustomRegisterList(pSource->Chip->Name, hRegisterListTail);
                        if(pRegisterList == NULL)
                        {
                            // Get the default register list
                            (pSource->Chip->SetupFunction)(hRegisterListTail);
                        }

                        if(pRegisterList != NULL)
                        {
                            int Count = 0;

                            // Count the registers
                            for(pRegister = pRegisterList;
                                pRegister != NULL;
                                pRegister = pRegister->Next)
                            {
                                Count++;
                            }

                            GetClientRect(hRegDisplay, &Rect);

                            ScrollPos   = 0;
                            ScrollRange = Count * REG_FONT_HEIGHT - 1;
                            ScrollPage  = Rect.bottom;

                            if(ScrollPage < ScrollRange)
                            {
                                ScrollMax = ScrollRange - ScrollPage;

                                SCROLLINFO ScrollInfo;
                                ScrollInfo.cbSize = sizeof(SCROLLINFO);
                                ScrollInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
                                ScrollInfo.nMin = 0;
                                ScrollInfo.nMax = ScrollRange - 1;
                                ScrollInfo.nPage = ScrollPage;
                                ScrollInfo.nPos = ScrollPos;
                                ScrollInfo.nTrackPos = 0;

                                EnableWindow(hRegScroll, TRUE);
                                SendMessage(hRegScroll, SBM_SETSCROLLINFO, TRUE, (LPARAM)&ScrollInfo);
                            }
                            else
                            {
                                EnableWindow(hRegScroll, FALSE);
                            }

                            EnableWindow(hMoveUpChanges, Count > 1);
                            EnableWindow(hLogState, TRUE);
                            EnableWindow(hDumpToFile, TRUE);

                            if(GetFocus() == GetDlgItem(hDlg, IDC_SOURCESELECT))
                            {
                                SetFocus(hDlg);
                            }

                            bFirstRead = TRUE;
                            PostMessage(hDlg, WM_TIMER, IDC_REFRESH, 0);
                        }
                    }
                    else
                    {
                        delete pCard;
                        pCard = NULL;
                        MessageBox(NULL, "Can't open card", "RegSpy", MB_OK);
                    }
                }
                else
                {
                    EnableWindow(hRegScroll, FALSE);
                    EnableWindow(hMoveUpChanges, FALSE);
                    EnableWindow(hLogState, FALSE);
                    EnableWindow(hDumpToFile, FALSE);
                }
                LeaveCriticalSection(&RegisterListMutex);
            }
            break;

        case IDC_LOGSTATE:
            if(bResetLogState == TRUE)
            {
                nLogState = 0;
                EnableWindow(hLogState, FALSE);
                SetDlgItemText(hDlg, IDC_DUMPTOFILE, "Dump To File ...");

                gRegDisplayInfo.bRedrawAll = TRUE;
                InvalidateRect(hRegDisplay, NULL, FALSE);
            }
            else if(nLogState != MAX_LOG_STATES)
            {
                EnterCriticalSection(&RegisterListMutex);
                for(pRegister = pRegisterList; pRegister != NULL; pRegister = pRegister->Next)
                {
                    pRegister->LogStates[nLogState] = pRegister->LastValue;
                    pRegister->bDraw = TRUE;
                }
                nLogState++;
                LeaveCriticalSection(&RegisterListMutex);

                sprintf(szBuffer, "Log State (%d)", nLogState);
                SetDlgItemText(hDlg, IDC_LOGSTATE, szBuffer);
                SetDlgItemText(hDlg, IDC_DUMPTOFILE, "Save To File ...");

                if(nLogState == MAX_LOG_STATES)
                {
                    EnableWindow(hLogState, FALSE);
                }
                InvalidateRect(hRegDisplay, NULL, FALSE);
            }
            break;

        case IDC_DUMPTOFILE:
            KillTimer(hDlg, IDC_REFRESH);
            EnterCriticalSection(&RegisterListMutex);

            ItemIndex = SendMessage(hSourceSelect, CB_GETCURSEL, 0, 0);
            pSource = (TSource*)SendMessage(hSourceSelect, CB_GETITEMDATA, ItemIndex, 0);

            if(pSource != NULL)
            {
                if(SaveToFile(hDlg, pSource, pRegisterList, nLogState))
                {
                    nLogState = 0;
                    SetDlgItemText(hDlg, IDC_LOGSTATE, "Log State");
                    SetDlgItemText(hDlg, IDC_DUMPTOFILE, "Dump To File ...");

                    gRegDisplayInfo.bRedrawAll = TRUE;
                    InvalidateRect(hRegDisplay, NULL, FALSE);
                }
            }

            PostMessage(hDlg, WM_TIMER, IDC_REFRESH, 0);
            LeaveCriticalSection(&RegisterListMutex);
            break;

        case IDC_MOVEUPCHANGES:
            bMoveUpChanges = IsDlgButtonChecked(hDlg, IDC_MOVEUPCHANGES);
            break;

        default:
            break;
        }
        break;

    case WM_SIZE:
        if(wParam != SIZE_MINIMIZED)
        {
            LONG DeltaX = LOWORD(lParam) - nDialogWidth;
            LONG DeltaY = HIWORD(lParam) - nDialogHeight;

            if(DeltaX != 0 || DeltaY != 0)
            {
                DeltaWindow(hSourceSelect, DeltaX, 0, 0, 0, FALSE);
                DeltaWindow(hRegDisplay, 0, 0, DeltaX, DeltaY, FALSE);
                DeltaWindow(hRegScroll, DeltaX, 0, 0, DeltaY, FALSE);
                DeltaWindow(hMoveUpChanges, 0, DeltaY, 0, 0, FALSE);
                DeltaWindow(hLogState, DeltaX, DeltaY, 0, 0, FALSE);
                DeltaWindow(hDumpToFile, 0, DeltaY, 0, 0, FALSE);
                DeltaWindow(GetDlgItem(hDlg, IDC_HARDWARE), 0, 0, DeltaX, 0, FALSE);
                DeltaWindow(GetDlgItem(hDlg, IDC_REGISTERS), 0, 0, DeltaX, DeltaY, FALSE);
                DeltaWindow(GetDlgItem(hDlg, IDC_SEPARATOR), 0, DeltaY, DeltaX, 0, FALSE);
                DeltaWindow(GetDlgItem(hDlg, IDOK), DeltaX, DeltaY, 0, 0, FALSE);

                nDialogWidth += DeltaX;
                nDialogHeight += DeltaY;

                GetClientRect(hRegDisplay, &Rect);
                ScrollPage = Rect.bottom;

                if(ScrollPage < ScrollRange)
                {
                    ScrollMax = ScrollRange - ScrollPage;

                    SCROLLINFO ScrollInfo;
                    ZeroMemory(&ScrollInfo, sizeof(SCROLLINFO));
                    ScrollInfo.cbSize = sizeof(SCROLLINFO);
                    ScrollInfo.fMask = SIF_PAGE;
                    ScrollInfo.nPage = ScrollPage;

                    EnableWindow(hRegScroll, TRUE);
                    SendMessage(hRegScroll, SBM_SETSCROLLINFO, FALSE, (LPARAM)&ScrollInfo);

                    if(ScrollPos > ScrollMax)
                    {
                        ScrollPos = ScrollMax;
                        gRegDisplayInfo.nScrollPos = ScrollPos;
                    }
                }
                else
                {
                    EnableWindow(hRegScroll, FALSE);
                }

                InvalidateRect(hDlg, NULL, TRUE);
            }
        }
        break;

    case WM_SIZING:
        {
            LPRECT lpRect = (LPRECT)lParam;

            CopyRect(&Rect, lpRect);
            SendMessage(hDlg, WM_NCCALCSIZE, FALSE, (LPARAM)&Rect);

            LONG DeltaY = Rect.bottom - Rect.top - nDialogHeight;

            if(Rect.right - Rect.left < MIN_DIALOG_WIDTH)
            {
                LONG AdjustX = MIN_DIALOG_WIDTH - (Rect.right - Rect.left);

                switch(wParam)
                {
                case WMSZ_TOPLEFT:
                case WMSZ_LEFT:
                case WMSZ_BOTTOMLEFT:
                    lpRect->left -= AdjustX;
                    break;

                default:
                    lpRect->right += AdjustX;
                    break;
                }
            }
            if(ScrollPage + DeltaY < REG_FONT_HEIGHT)
            {
                switch(wParam)
                {
                case WMSZ_TOP:
                case WMSZ_TOPLEFT:
                case WMSZ_TOPRIGHT:
                    lpRect->top += (ScrollPage - REG_FONT_HEIGHT) + DeltaY;
                    break;

                default:
                    lpRect->bottom -= (ScrollPage - REG_FONT_HEIGHT) + DeltaY;
                    break;
                }
            }
        }
        return TRUE;

    case WM_VSCROLL:
        if((HWND)lParam == GetDlgItem(hDlg, IDC_REGSCROLL))
        {
            LONG Pos = ScrollPos;

            switch(LOWORD(wParam))
            {
            case SB_LEFT:
                Pos = 0;
                break;
            case SB_RIGHT:
                Pos = ScrollMax;
                break;
            case SB_LINELEFT:
                Pos = ScrollPos - REG_FONT_HEIGHT;
                break;
            case SB_LINERIGHT:
                Pos = ScrollPos + REG_FONT_HEIGHT;
                break;
            case SB_PAGELEFT:
                Pos = ScrollPos - ScrollPage;
                break;
            case SB_PAGERIGHT:
                Pos = ScrollPos + ScrollPage;
                break;
            case SB_THUMBPOSITION:
            case SB_THUMBTRACK:
                Pos = HIWORD(wParam);
                break;
            default:
                break;
            }

            if(Pos < 0)
            {
                Pos = 0;
            }
            if(Pos > ScrollMax)
            {
                Pos = ScrollMax;
            }

            if(Pos != ScrollPos)
            {
                ScrollPos = Pos;
                SendMessage(hRegScroll, SBM_SETPOS, ScrollPos, TRUE);

                gRegDisplayInfo.nScrollPos = ScrollPos;
                InvalidateRect(hRegDisplay, NULL, FALSE);
            }

            if(GetFocus() == GetDlgItem(hDlg, IDC_SOURCESELECT))
            {
                SetFocus(hDlg);
            }
            break;
        }
        break;

    case WM_MOUSEWHEEL:
        if(ScrollPage < ScrollRange)
        {
            int nScrollLines;
            SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &nScrollLines, 0);

            short Lines = nScrollLines * (short)HIWORD(wParam) / -WHEEL_DELTA;

            LONG TrackPosition = ScrollPos + Lines * REG_FONT_HEIGHT;

            if(TrackPosition < 0)
            {
                TrackPosition = 0;
            }
            else if(TrackPosition > ScrollMax)
            {
                TrackPosition = ScrollMax;
            }

            SendMessage(hDlg, WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, TrackPosition),
                (LPARAM)GetDlgItem(hDlg, IDC_REGSCROLL));
        }
        break;

    case WM_KEYDOWN:
        if(wParam == VK_CONTROL)
        {
            if(bResetLogState == FALSE)
            {
                bResetLogState = TRUE;
                SetDlgItemText(hDlg, IDC_LOGSTATE, "Reset States");
                EnableWindow(hLogState, nLogState != 0);
            }
        }
        break;

    case WM_KEYUP:
        if(wParam == VK_CONTROL)
        {
            if(bResetLogState == TRUE)
            {
                bResetLogState = FALSE;
                if(nLogState > 0)
                {
                    sprintf(szBuffer, "Log State (%d)", nLogState);
                    EnableWindow(hLogState, nLogState != MAX_LOG_STATES);
                }
                else
                {
                    sprintf(szBuffer, "Log State");
                    EnableWindow(hLogState, TRUE);
                }
                SetDlgItemText(hDlg, IDC_LOGSTATE, szBuffer);
            }
        }
        break;

    default:
        break;
    }
    return (FALSE);
}


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    TSource*    SourceList = NULL;
    TSource**   SourceListTail = &SourceList;
    TSource*    Source;

    CHardwareDriver* pHardwareDriver = new CHardwareDriver();
    if(pHardwareDriver->LoadDriver() == FALSE)
    {
        delete pHardwareDriver;
        MessageBox(NULL, "Can't open driver", "RegSpy", MB_OK);
        return 1;
    }

    for(int i(0); i < sizeof(Chips)/sizeof(TChip); ++i)
    {
        DWORD SubSystemId;
        int DeviceIndex(0);

        while(pHardwareDriver->DoesThisPCICardExist(
                                                        Chips[i].VendorId,
                                                        Chips[i].DeviceId,
                                                        DeviceIndex,
                                                        SubSystemId
                                                   ) == TRUE)
        {
            Source = (TSource*) malloc(sizeof(TSource));
            Source->DeviceIndex = DeviceIndex;
            Source->Chip        = &Chips[i];
            Source->SubSystemId = SubSystemId;
            Source->Next        = NULL;

            *SourceListTail = Source;
            SourceListTail = &Source->Next;

            DeviceIndex++;
        }
    }

    void* ContextPtr[] = { SourceList, pHardwareDriver };

    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAINDIALOG), NULL, MainWindowProc, (LPARAM)ContextPtr);

    while((Source = SourceList) != NULL)
    {
        SourceList = Source->Next;
        free(Source);
    }

    delete pHardwareDriver;
    return 0;
}

void __cdecl LOG(int , LPCSTR, ...)
{
}

///////////////////////////////////////////////////////////////////////////////
// This load of junk required so that we can link with
// some of the Dscaler files
///////////////////////////////////////////////////////////////////////////////

HWND hWnd = NULL;

void __cdecl HideSplashScreen()
{
}

extern "C"
{
    long gBuildNum = 0;
}

void __cdecl OSD_Redraw(struct HWND__ *,struct HDC__ *)
{
}

void __cdecl OSD_ShowTextPersistent(struct HWND__ *,char const *,double)
{
}

void __cdecl OSD_ProcessDisplayUpdate(struct HDC__ *,struct tagRECT *)
{
}

int __cdecl GetDisplayAreaRect(struct HWND__ *,struct tagRECT *, BOOL)
{
    return FALSE;
}

void __cdecl OSD_ShowTextPersistent(char const *,double)
{
}