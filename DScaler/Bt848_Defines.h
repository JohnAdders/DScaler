/////////////////////////////////////////////////////////////////////////////
// $Id: Bt848_Defines.h,v 1.14 2002-06-05 20:53:49 adcockj Exp $
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
//
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __BT848_DEFINES_H___
#define __BT848_DEFINES_H___

// Brooktree 848 registers
#define BT848_DSTATUS          0x000
#define BT848_DSTATUS_PRES     (1<<7)
#define BT848_DSTATUS_HLOC     (1<<6)
#define BT848_DSTATUS_FIELD    (1<<5)
#define BT848_DSTATUS_NUML     (1<<4)
#define BT848_DSTATUS_CSEL     (1<<3)
#define BT848_DSTATUS_PLOCK    (1<<2)
#define BT848_DSTATUS_LOF      (1<<1)
#define BT848_DSTATUS_COF      (1<<0)

#define BT848_IFORM            0x004
#define BT848_IFORM_HACTIVE    (1<<7)
#define BT848_IFORM_MUXSEL     (3<<5)
#define BT848_IFORM_MUX0       (2<<5)
#define BT848_IFORM_MUX1       (3<<5)
#define BT848_IFORM_MUX2       (1<<5)
#define BT848_IFORM_XTSEL      (3<<3)
#define BT848_IFORM_XT0        (1<<3)
#define BT848_IFORM_XT1        (2<<3)
#define BT848_IFORM_XTAUTO     (3<<3)
#define BT848_IFORM_XTBOTH     (3<<3)
#define BT848_IFORM_NTSC       1
#define BT848_IFORM_NTSC_JAP   2
#define BT848_IFORM_PAL_BDGHI  3
#define BT848_IFORM_PAL_M      4
#define BT848_IFORM_PAL_N      5
#define BT848_IFORM_SECAM      6
#define BT848_IFORM_PAL_NC     7
#define BT848_IFORM_AUTO       0
#define BT848_IFORM_NORM       7


#define BT848_FCNTR            0x0E8
#define BT848_PLL_F_LO         0x0F0
#define BT848_PLL_F_HI         0x0F4
#define BT848_PLL_XCI          0x0F8

#define BT848_TGCTRL           0x084
#define BT848_TGCTRL_TGCKI_PLL 0x08
#define BT848_TGCTRL_TGCKI_NOPLL 0x00

#define BT848_TDEC             0x008
#define BT848_TDEC_DEC_FIELD   (1<<7)
#define BT848_TDEC_FLDALIGN    (1<<6)
#define BT848_TDEC_DEC_RAT     (0x1f)

#define BT848_E_CROP           0x00C
#define BT848_O_CROP           0x08C

#define BT848_E_VDELAY_LO      0x010
#define BT848_O_VDELAY_LO      0x090

#define BT848_E_VACTIVE_LO     0x014
#define BT848_O_VACTIVE_LO     0x094

#define BT848_E_HDELAY_LO      0x018
#define BT848_O_HDELAY_LO      0x098

#define BT848_E_HACTIVE_LO     0x01C
#define BT848_O_HACTIVE_LO     0x09C

#define BT848_E_HSCALE_HI      0x020
#define BT848_O_HSCALE_HI      0x0A0

#define BT848_E_HSCALE_LO      0x024
#define BT848_O_HSCALE_LO      0x0A4

#define BT848_BRIGHT           0x028

#define BT848_E_CONTROL        0x02C
#define BT848_O_CONTROL        0x0AC
#define BT848_CONTROL_LNOTCH    (1<<7)
#define BT848_CONTROL_COMP      (1<<6)
#define BT848_CONTROL_LDEC      (1<<5)
#define BT848_CONTROL_CBSENSE   (1<<4)
#define BT848_CONTROL_CON_MSB   (1<<2)
#define BT848_CONTROL_SAT_U_MSB (1<<1)
#define BT848_CONTROL_SAT_V_MSB (1<<0)

#define BT848_CONTRAST_LO      0x030
#define BT848_SAT_U_LO         0x034
#define BT848_SAT_V_LO         0x038
#define BT848_HUE              0x03C

#define BT848_E_SCLOOP         0x040 
#define BT848_O_SCLOOP         0x0C0
#define BT848_SCLOOP_LUMA_PEAK  (1<<7)
#define BT848_SCLOOP_CAGC       (1<<6)
#define BT848_SCLOOP_CKILL      (1<<5)
#define BT848_SCLOOP_HFILT_FULL (1<<3)  // TRB 12/00 max full res peakg filter
#define BT848_SCLOOP_HFILT_AUTO (0<<3)  // TRB Not sure these match doc?
#define BT848_SCLOOP_HFILT_CIF  (1<<3)
#define BT848_SCLOOP_HFILT_QCIF (2<<3)
#define BT848_SCLOOP_HFILT_ICON (3<<3)

#define BT848_WC_UP            0x044
#define BT848_WC_DOWN          0x078
#define BT848_VTOTAL_LO        0x0B0
#define BT848_VTOTAL_HI        0x0B4
#define BT848_DVSIF            0x0FC

#define BT848_OFORM            0x048
#define BT848_OFORM_RANGE      (1<<7)
#define BT848_OFORM_CORE0      (0<<5)
#define BT848_OFORM_CORE8      (1<<5)
#define BT848_OFORM_CORE16     (2<<5)
#define BT848_OFORM_CORE32     (3<<5)

#define BT848_E_VSCALE_HI      0x04C
#define BT848_O_VSCALE_HI      0x0CC
#define BT848_VSCALE_YCOMB     (1<<7)
#define BT848_VSCALE_COMB      (1<<6)
#define BT848_VSCALE_INT       (1<<5)
#define BT848_VSCALE_HI        15

#define BT848_E_VSCALE_LO      0x050
#define BT848_O_VSCALE_LO      0x0D0
#define BT848_TEST             0x054
#define BT848_ADELAY           0x060
#define BT848_BDELAY           0x064

#define BT848_ADC              0x068
#define BT848_ADC_RESERVED     (1<<6)
#define BT848_ADC_SYNC_T       (1<<5)
#define BT848_ADC_AGC_EN       (1<<4)
#define BT848_ADC_CLK_SLEEP    (1<<3)
#define BT848_ADC_Y_SLEEP      (1<<2)
#define BT848_ADC_C_SLEEP      (1<<1)
#define BT848_ADC_CRUSH        (1<<0)

#define BT848_E_VTC            0x06C
#define BT848_O_VTC            0x0EC
#define BT848_VTC_HSFMT        (1<<7)
#define BT848_VTC_VFILT_2TAP   0
#define BT848_VTC_VFILT_3TAP   1
#define BT848_VTC_VFILT_4TAP   2
#define BT848_VTC_VFILT_5TAP   3
#define BT848_VTC_VFILT_2TAPZ  4
#define BT848_SRESET           0x07C

#define BT848_COLOR_FMT             0x0D4
#define BT848_COLOR_FMT_O_RGB32     (0<<4)
#define BT848_COLOR_FMT_O_RGB24     (1<<4)
#define BT848_COLOR_FMT_O_RGB16     (2<<4)
#define BT848_COLOR_FMT_O_RGB15     (3<<4)
#define BT848_COLOR_FMT_O_YUY2      (4<<4)
#define BT848_COLOR_FMT_O_BtYUV     (5<<4)
#define BT848_COLOR_FMT_O_Y8        (6<<4)
#define BT848_COLOR_FMT_O_RGB8      (7<<4)
#define BT848_COLOR_FMT_O_YCrCb422  (8<<4)
#define BT848_COLOR_FMT_O_YCrCb411  (9<<4)
#define BT848_COLOR_FMT_O_RAW       (14<<4)
#define BT848_COLOR_FMT_E_RGB32     0
#define BT848_COLOR_FMT_E_RGB24     1
#define BT848_COLOR_FMT_E_RGB16     2
#define BT848_COLOR_FMT_E_RGB15     3
#define BT848_COLOR_FMT_E_YUY2      4
#define BT848_COLOR_FMT_E_BtYUV     5
#define BT848_COLOR_FMT_E_Y8        6
#define BT848_COLOR_FMT_E_RGB8      7
#define BT848_COLOR_FMT_E_YCrCb422  8
#define BT848_COLOR_FMT_E_YCrCb411  9
#define BT848_COLOR_FMT_E_RAW       14

#define BT848_COLOR_FMT_RGB32       0x00
#define BT848_COLOR_FMT_RGB24       0x11
#define BT848_COLOR_FMT_RGB16       0x22
#define BT848_COLOR_FMT_RGB15       0x33
#define BT848_COLOR_FMT_YUY2        0x44
#define BT848_COLOR_FMT_BtYUV       0x55
#define BT848_COLOR_FMT_Y8          0x66
#define BT848_COLOR_FMT_RGB8        0x77
#define BT848_COLOR_FMT_YCrCb422    0x88
#define BT848_COLOR_FMT_YCrCb411    0x99
#define BT848_COLOR_FMT_RAW         0xee

#define BT848_COLOR_CTL                0x0D8
#define BT848_COLOR_CTL_EXT_FRMRATE    (1<<7)
#define BT848_COLOR_CTL_COLOR_BARS     (1<<6)
#define BT848_COLOR_CTL_RGB_DED        (1<<5)
#define BT848_COLOR_CTL_GAMMA          (1<<4)
#define BT848_COLOR_CTL_WSWAP_ODD      (1<<3)
#define BT848_COLOR_CTL_WSWAP_EVEN     (1<<2)
#define BT848_COLOR_CTL_BSWAP_ODD      (1<<1)
#define BT848_COLOR_CTL_BSWAP_EVEN     (1<<0)

#define BT848_CAP_CTL                  0x0DC
#define BT848_CAP_CTL_DITH_FRAME       (1<<4)
#define BT848_CAP_CTL_CAPTURE_VBI_ODD  (1<<3)
#define BT848_CAP_CTL_CAPTURE_VBI_EVEN (1<<2)
#define BT848_CAP_CTL_CAPTURE_ODD      (1<<1)
#define BT848_CAP_CTL_CAPTURE_EVEN     (1<<0)

#define BT848_VBI_PACK_SIZE    0x0E0

#define BT848_VBI_PACK_DEL     0x0E4
#define BT848_VBI_PACK_DEL_VBI_HDELAY 0xfc
#define BT848_VBI_PACK_DEL_EXT_FRAME  2
#define BT848_VBI_PACK_DEL_VBI_PKT_HI 1

#define BT848_INT_STAT         0x100
#define BT848_INT_MASK         0x104

#define BT848_INT_ETBF         (1<<23)

#define BT848_INT_RISCS   (0xf<<28)
#define BT848_INT_RISC_EN (1<<27)
#define BT848_INT_RACK    (1<<25)
#define BT848_INT_FIELD   (1<<24)
#define BT848_INT_SCERR   (1<<19)
#define BT848_INT_OCERR   (1<<18)
#define BT848_INT_PABORT  (1<<17)
#define BT848_INT_RIPERR  (1<<16)
#define BT848_INT_PPERR   (1<<15)
#define BT848_INT_FDSR    (1<<14)
#define BT848_INT_FTRGT   (1<<13)
#define BT848_INT_FBUS    (1<<12)
#define BT848_INT_RISCI   (1<<11)
#define BT848_INT_GPINT   (1<<9)
#define BT848_INT_I2CDONE (1<<8)
#define BT848_INT_VPRES   (1<<5)
#define BT848_INT_HLOCK   (1<<4)
#define BT848_INT_OFLOW   (1<<3)
#define BT848_INT_HSYNC   (1<<2)
#define BT848_INT_VSYNC   (1<<1)
#define BT848_INT_FMTCHG  (1<<0)


#define BT848_GPIO_DMA_CTL             0x10C
#define BT848_GPIO_DMA_CTL_GPINTC      (1<<15)
#define BT848_GPIO_DMA_CTL_GPINTI      (1<<14)
#define BT848_GPIO_DMA_CTL_GPWEC       (1<<13)
#define BT848_GPIO_DMA_CTL_GPIOMODE    (3<<11)
#define BT848_GPIO_DMA_CTL_GPCLKMODE   (1<<10)
#define BT848_GPIO_DMA_CTL_PLTP23_4    (0<<6)
#define BT848_GPIO_DMA_CTL_PLTP23_8    (1<<6)
#define BT848_GPIO_DMA_CTL_PLTP23_16   (2<<6)
#define BT848_GPIO_DMA_CTL_PLTP23_32   (3<<6)
#define BT848_GPIO_DMA_CTL_PLTP1_4     (0<<4)
#define BT848_GPIO_DMA_CTL_PLTP1_8     (1<<4)
#define BT848_GPIO_DMA_CTL_PLTP1_16    (2<<4)
#define BT848_GPIO_DMA_CTL_PLTP1_32    (3<<4)
#define BT848_GPIO_DMA_CTL_PKTP_4      (0<<2)
#define BT848_GPIO_DMA_CTL_PKTP_8      (1<<2)
#define BT848_GPIO_DMA_CTL_PKTP_16     (2<<2)
#define BT848_GPIO_DMA_CTL_PKTP_32     (3<<2)
#define BT848_GPIO_DMA_CTL_RISC_ENABLE (1<<1)
#define BT848_GPIO_DMA_CTL_FIFO_ENABLE (1<<0)

#define BT848_I2C              0x110
#define BT848_I2C_DIV          (0xf<<4)
#define BT848_I2C_SYNC         (1<<3)
#define BT848_I2C_W3B          (1<<2)
#define BT848_I2C_SCL          (1<<1)
#define BT848_I2C_SDA          (1<<0)


#define BT848_RISC_STRT_ADD    0x114
#define BT848_GPIO_OUT_EN      0x118
#define BT848_GPIO_OUT_EN_HIBYTE      0x11A
#define BT848_GPIO_REG_INP     0x11C
#define BT848_GPIO_REG_INP_HIBYTE     0x11E
#define BT848_RISC_COUNT       0x120
#define BT848_GPIO_DATA        0x200
#define BT848_GPIO_DATA_HIBYTE 0x202


// Bt848 RISC commands 

// only for the SYNC RISC command 
#define BT848_FIFO_STATUS_FM1  0x06
#define BT848_FIFO_STATUS_FM3  0x0e
#define BT848_FIFO_STATUS_SOL  0x02
#define BT848_FIFO_STATUS_EOL4 0x01
#define BT848_FIFO_STATUS_EOL3 0x0d
#define BT848_FIFO_STATUS_EOL2 0x09
#define BT848_FIFO_STATUS_EOL1 0x05
#define BT848_FIFO_STATUS_VRE  0x04
#define BT848_FIFO_STATUS_VRO  0x0c
#define BT848_FIFO_STATUS_PXV  0x00

#define BT848_RISC_RESYNC      (1<<15)

// WRITE and SKIP 
// disable which bytes of each DWORD 
#define BT848_RISC_BYTE0       (1<<12)
#define BT848_RISC_BYTE1       (1<<13)
#define BT848_RISC_BYTE2       (1<<14)
#define BT848_RISC_BYTE3       (1<<15)
#define BT848_RISC_BYTE_ALL    (0x0f<<12)
#define BT848_RISC_BYTE_NONE   0
// cause RISCI 
#define BT848_RISC_IRQ         (1<<24)
// RISC command is last one in this line 
#define BT848_RISC_EOL         (1<<26)
// RISC command is first one in this line 
#define BT848_RISC_SOL         (1<<27)

#define BT848_RISC_WRITE       (0x01<<28)
#define BT848_RISC_SKIP        (0x02<<28)
#define BT848_RISC_WRITEC      (0x05<<28)
#define BT848_RISC_JUMP        (0x07<<28)
#define BT848_RISC_SYNC        (0x08<<28)

#define BT848_RISC_WRITE123    (0x09<<28)
#define BT848_RISC_SKIP123     (0x0a<<28)
#define BT848_RISC_WRITE1S23   (0x0b<<28)

// 13 Dec 2000 - Michael Eskin, Conexant Systems
// Added TGCTRL register masks for CCIR656 Input
//
#define BT848_TBLG                      0x80
#define BT848_TGCTRL_TGMODE_ENABLE      0x01
#define BT848_TGCTRL_TGMODE_RESET       0x02
#define BT848_TGCTRL_TGMODE_INCREMENT   0x04
#define BT848_TGCTRL_TGCKI_GPCLK        0x10
#define BT848_TGCTRL_TGCKI_CPCLK_INV    0x11

#define BT848_DVSIF_CCIR656             0x01
#define BT848_DVSIF_VSIF_SVREF          0x0C
#define BT848_DVSIF_VSIF_ESO            0x20
#define BT848_DVSIF_VSIF_BCF            0x40

#define BT848_VTC_HSFMT_64              0x00
#define BT848_VTC_HSFMT_48              0x40
#define BT848_VTC_HSFMT_32              0x80
#define BT848_VTC_HSFMT_16              0xC0

#define BT848_I2C_DELAY 0
#define BT848_I2C_TIMING (0x7<<4)
#define BT848_I2C_COMMAND (BT848_I2C_TIMING | BT848_I2C_SCL | BT848_I2C_SDA)

#define BT848_MSP_WRITE                 0x80
#define BT848_MSP_READ                  0x81

#define VBI_SPL 2044

enum eTVCardId
{
    TVCARD_UNKNOWN = 0,
    TVCARD_MIRO,
    TVCARD_HAUPPAUGE,
    TVCARD_STB,           
    TVCARD_INTEL,         
    TVCARD_DIAMOND,        
    TVCARD_AVERMEDIA,      
    TVCARD_MATRIX_VISION,
    TVCARD_FLYVIDEO,      
    TVCARD_TURBOTV,       
    TVCARD_HAUPPAUGE878,  
    TVCARD_MIROPRO,       
    TVCARD_ADSTECH_TV,    
    TVCARD_AVERMEDIA98,   
    TVCARD_VHX,           
    TVCARD_ZOLTRIX,       
    TVCARD_PIXVIEWPLAYTV, 
    TVCARD_WINVIEW_601,   
    TVCARD_AVEC_INTERCAP, 
    TVCARD_LIFE_FLYKIT,   
    TVCARD_CEI_RAFFLES,   
    TVCARD_CONFERENCETV,  
    TVCARD_PHOEBE_TVMAS,  
    TVCARD_MODTEC_205,    
    TVCARD_MAGICTVIEW061, 
    TVCARD_VOBIS_BOOSTAR, 
    TVCARD_HAUPPAUG_WCAM, 
    TVCARD_MAXI,          
    TVCARD_TERRATV,       
    TVCARD_PXC200,        
    TVCARD_FLYVIDEO_98,   
    TVCARD_IPROTV,        
    TVCARD_INTEL_C_S_PCI, 
    TVCARD_TERRATVALUE,   
    TVCARD_WINFAST2000,   
    TVCARD_CHRONOS_VS2,   
    TVCARD_TYPHOON_TVIEW, 
    TVCARD_PXELVWPLTVPRO, 
    TVCARD_MAGICTVIEW063, 
    TVCARD_PINNACLERAVE,  
    TVCARD_STB2,          
    TVCARD_AVPHONE98,     
    TVCARD_PV951,         
    TVCARD_ONAIR_TV,      
    TVCARD_SIGMA_TVII_FM, 
    TVCARD_MATRIX_VISION2d,
    TVCARD_ZOLTRIX_GENIE, 
    TVCARD_TERRATVRADIO, 
    TVCARD_DYNALINK,
    // MAE 20 Nov 2000 Start of change
    TVCARD_CONEXANTNTSCXEVK,
    TVCARD_ROCKWELLNTSCXEVK,
    // MAE 20 Nov 2000 End of change
    // MAE 5 Dec 2000 Start of change
    TVCARD_CONEXANTFOGHORNREVA,
    TVCARD_CONEXANTFOGHORNREVB,
    TVCARD_CONEXANTFOGHORNREVC,
    // MAE 5 Dec 2000 End of change
    TVCARD_RS_BT,
    TVCARD_CYBERMAIL,
    TVCARD_VIEWCAST,
    TVCARD_ATI_TVWONDER,
    TVCARD_ATI_TVWONDERVE,
    TVCARD_GVBCTV3PCI,
    TVCARD_PROLINK,
    TVCARD_EAGLE,
    TVCARD_PINNACLEPRO,
    TVCARD_THYPHOON,
    TVCARD_LIFETEC,
    TVCARD_BESTBUY_OLD,
    TVCARD_FLYVIDEO_98FM,
    TVCARD_GRANDTEC,
    TVCARD_PHOEBE,
    TVCARD_TVCAPTURER,
    TVCARD_MM100PCTV,
    TVCARD_GMV1,
    TVCARD_BESTBUY_NEW,
    TVCARD_SASEM4CHNLSVID_S,
    TVCARD_PINNACLESAT,
    TVCARD_VHXOLD,           
    TVCARD_VOODOOTV_200,
    TVCARD_VOODOOTV_FM,
    TVCARD_STANDARD_BT878_NO_INIT_MSP,
    TVCARD_TERRATVALUER,
    TVCARD_FLYVIDEO98EZ,
    TVCARD_AIMSS,
    TVCARD_FLYVIDEO2000,
    TVCARD_GVBCTV4PCI,
    TVCARD_PXELVWPLTVPAK,
    TVCARD_TVIEW_RDS_FM,
    TVCARD_SASEM4CHNLSVID_C,
    TVCARD_SDISILK_S,
    TVCARD_SDISILK_C,
    TVCARD_SKYWELL_MAGIC,
    TVCARD_LASTONE,
};

enum eTunerId
{
    TUNER_AUTODETECT = -2,
    TUNER_USER_SETUP = -1,
    TUNER_ABSENT = 0,           
    TUNER_PHILIPS_PAL_I,        
    TUNER_PHILIPS_NTSC,     
    TUNER_PHILIPS_SECAM,        
    TUNER_PHILIPS_PAL,      
    TUNER_TEMIC_4002FH5_PAL,
    TUNER_TEMIC_4032FY5_NTSC,       
    TUNER_TEMIC_4062FY5_PAL_I,      
    TUNER_TEMIC_4036FY5_NTSC,
    TUNER_ALPS_TSBH1_NTSC,  
    TUNER_ALPS_TSBE1_PAL,   
    TUNER_ALPS_TSBB5_PAL_I,     
    TUNER_ALPS_TSBE5_PAL,   
    TUNER_ALPS_TSBC5_PAL,   
    TUNER_TEMIC_4006FH5_PAL,
    TUNER_PHILIPS_1236D_NTSC_INPUT1,
    TUNER_PHILIPS_1236D_NTSC_INPUT2,
    TUNER_ALPS_TSCH6_NTSC,
    TUNER_TEMIC_4016FY5_PAL,
    TUNER_PHILIPS_MK2_NTSC,
    TUNER_TEMIC_4066FY5_PAL_I,
    TUNER_TEMIC_4006FN5_PAL,
    TUNER_TEMIC_4009FR5_PAL,
    TUNER_TEMIC_4039FR5_NTSC,
    TUNER_TEMIC_4046FM5_MULTI,
    TUNER_PHILIPS_PAL_DK,
    TUNER_PHILIPS_MULTI,
    TUNER_LG_I001D_PAL_I,
    TUNER_LG_I701D_PAL_I,
    TUNER_LG_R01F_NTSC,
    TUNER_LG_B01D_PAL,
    TUNER_LG_B11D_PAL,
    TUNER_TEMIC_4009FN5_PAL,
    TUNER_MT2032,
    TUNER_SHARP_2U5JF5540_NTSC,
    TUNER_LG_TAPCH701P_NTSC,
    TUNER_LASTONE,
};

enum eSoundChannel
{
    SOUNDCHANNEL_MONO = 1,
    SOUNDCHANNEL_STEREO,
    SOUNDCHANNEL_LANGUAGE1,
    SOUNDCHANNEL_LANGUAGE2,
};

#define TIMER_MSP           8
#define TIMER_MSP_MS        1000

// 10/19/2000 Mark Rejhon
// Better NTSC defaults
#define DEFAULT_HUE_NTSC 0
#define DEFAULT_BRIGHTNESS_NTSC 0
#define DEFAULT_CONTRAST_NTSC 0x7F
#define DEFAULT_SAT_U_NTSC 254
#define DEFAULT_SAT_V_NTSC 219

// These are the original defaults, likely optimized for PAL (could use refinement).
#define DEFAULT_HUE_PAL 0
#define DEFAULT_BRIGHTNESS_PAL 0
#define DEFAULT_CONTRAST_PAL 0x7F
#define DEFAULT_SAT_U_PAL 0xfe
#define DEFAULT_SAT_V_PAL 0xb4


#endif
