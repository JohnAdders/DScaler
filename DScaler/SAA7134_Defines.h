/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134_Defines.h,v 1.12 2004-12-04 00:03:33 atnak Exp $
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
// Revision 1.11  2003/10/27 10:39:54  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.10  2003/02/06 21:30:43  ittarnavsky
// changes to support primetv 7133
//
// Revision 1.9  2002/11/08 06:15:34  atnak
// Added state saving
//
// Revision 1.8  2002/10/09 13:20:45  atnak
// got rid of stray comments
//
// Revision 1.7  2002/10/04 23:40:46  atnak
// proper support for audio channels mono,stereo,lang1,lang2 added
//
// Revision 1.6  2002/10/04 13:24:46  atnak
// Audio mux select through GPIO added (for 7130 cards)
//
// Revision 1.5  2002/10/03 23:36:23  atnak
// Various changes (major): VideoStandard, AudioStandard, CSAA7134Common, cleanups, tweaks etc,
//
// Revision 1.4  2002/09/14 19:40:48  atnak
// various changes
//
// Revision 1.3  2002/09/10 12:25:05  atnak
// add some more registers info
//
//
//////////////////////////////////////////////////////////////////////////////

/** 
 * @file saa7134_defines.h saa7134_defines Header file
 */
 
#ifndef __SAA7134_DEFINES_H___
#define __SAA7134_DEFINES_H___


/*
 * philips saa7134 registers
 */

/* ------------------------------------------------------------------ */
/*
 * PCI ID's
 */
#ifndef PCI_DEVICE_ID_PHILIPS_SAA7130
# define PCI_DEVICE_ID_PHILIPS_SAA7130 0x7130
#endif
#ifndef PCI_DEVICE_ID_PHILIPS_SAA7133
# define PCI_DEVICE_ID_PHILIPS_SAA7133 0x7133
#endif
#ifndef PCI_DEVICE_ID_PHILIPS_SAA7134
# define PCI_DEVICE_ID_PHILIPS_SAA7134 0x7134
#endif

/* ------------------------------------------------------------------ */
/*
 *  registers -- 32 bit
 */

//  Channel DMA
//   0      Video A (odd even)
//   0      Planar A, Y
//   1      Video B (odd even)
//   2      VBI A (odd even)
//   3      VBI B (odd even)
//   4      Planar A, U
//   5      Planar A, V
//   5      Transport Stream
//   6      Audio

/* DMA channels, n = 0 ... 6 */
#define SAA7134_RS_BA1(n)                   (0x200|(n)<<4)
#define SAA7134_RS_BA2(n)                   (0x204|(n)<<4)
#define SAA7134_RS_PITCH(n)                 (0x208|(n)<<4)
#define SAA7134_RS_CONTROL_0(n)             (0x20C|(n)<<4)
#define SAA7134_RS_CONTROL_1(n)             (0x20D|(n)<<4)
#define SAA7134_RS_CONTROL_2(n)             (0x20E|(n)<<4)
#define   SAA7134_RS_CONTROL_2_BURST_2          (1<<5)
#define   SAA7134_RS_CONTROL_2_BURST_4          (2<<5)
#define   SAA7134_RS_CONTROL_2_BURST_8          (3<<5)
#define   SAA7134_RS_CONTROL_2_BURST_16         (4<<5)
#define   SAA7134_RS_CONTROL_2_BURST_32         (5<<5)
#define   SAA7134_RS_CONTROL_2_BURST_64         (6<<5)
#define   SAA7134_RS_CONTROL_2_BURST_MAX        (7<<5)
#define   SAA7134_RS_CONTROL_2_ME               (1<<4)
#define SAA7134_RS_CONTROL_3(n)             (0x20F|(n)<<4)
#define   SAA7134_RS_CONTROL_3_WSWAP            (1<<1)
#define   SAA7134_RS_CONTROL_3_BSWAP            (1<<0)

#define SAA7134_FIFO_SIZE                   0x2A0
#define SAA7134_THRESHOULD                  0x2A4

// main control
#define SAA7134_MAIN_CTRL                   0x2A8
#define   SAA7134_MAIN_CTRL_VPLLE               (1<<15)
#define   SAA7134_MAIN_CTRL_APLLE               (1<<14)
#define   SAA7134_MAIN_CTRL_EXOSC               (1<<13)
#define   SAA7134_MAIN_CTRL_EVFE1               (1<<12)
#define   SAA7134_MAIN_CTRL_EVFE2               (1<<11)
#define   SAA7134_MAIN_CTRL_ESFE                (1<<10)
#define   SAA7134_MAIN_CTRL_EBADC               (1<<9)
#define   SAA7134_MAIN_CTRL_EBDAC               (1<<8)
#define   SAA7134_MAIN_CTRL_TE6                 (1<<6)
#define   SAA7134_MAIN_CTRL_TE5                 (1<<5)
#define   SAA7134_MAIN_CTRL_TE4                 (1<<4)
#define   SAA7134_MAIN_CTRL_TE3                 (1<<3)
#define   SAA7134_MAIN_CTRL_TE2                 (1<<2)
#define   SAA7134_MAIN_CTRL_TE1                 (1<<1)
#define   SAA7134_MAIN_CTRL_TE0                 (1<<0)

// DMA status
#define SAA7134_DMA_STATUS                  0x2AC
#define     SAA7134_DMA_STATUS_FE4              (1<<31)
#define     SAA7134_DMA_STATUS_FE3              (1<<30)
#define     SAA7134_DMA_STATUS_FE2              (1<<29)
#define     SAA7134_DMA_STATUS_FE1              (1<<28)
#define     SAA7134_DMA_STATUS_DMA4_DONE        (1<<27)
#define     SAA7134_DMA_STATUS_DMA3_DONE        (1<<26)
#define     SAA7134_DMA_STATUS_DMA2_DONE        (1<<25)
#define     SAA7134_DMA_STATUS_DMA1_DONE        (1<<24)
#define     SAA7134_DMA_STATUS_OE4              (1<<23)
#define     SAA7134_DMA_STATUS_OE3              (1<<22)
#define     SAA7134_DMA_STATUS_OE2              (1<<21)
#define     SAA7134_DMA_STATUS_OE1              (1<<20)

// audio / video status
#define SAA7134_AV_STATUS                   0x2C0
#define   SAA7134_AV_STATUS_STEREO              (1<<17)
#define   SAA7134_AV_STATUS_DUAL                (1<<16)
#define   SAA7134_AV_STATUS_PILOT               (1<<15)
#define   SAA7134_AV_STATUS_SMB                 (1<<14)
#define   SAA7134_AV_STATUS_DMB                 (1<<13)
#define   SAA7134_AV_STATUS_VDSP                (1<<12)
#define   SAA7134_AV_STATUS_IIC_STATUS          (3<<10)
#define   SAA7134_AV_STATUS_MVM                 (7<<7)
#define   SAA7134_AV_STATUS_FIDT                (1<<6)
#define   SAA7134_AV_STATUS_INTL                (1<<5)
#define   SAA7134_AV_STATUS_RDCAP               (1<<4)
#define   SAA7134_AV_STATUS_PWR_ON              (1<<3)
#define   SAA7134_AV_STATUS_LOAD_ERR            (1<<2)
#define   SAA7134_AV_STATUS_TRIG_ERR            (1<<1)
#define   SAA7134_AV_STATUS_CONF_ERR            (1<<0)

// interrupt
#define SAA7134_IRQ1                        0x2C4
#define   SAA7134_IRQ1_INTE_RA3_1               (1<<25) // second buffer audio
#define   SAA7134_IRQ1_INTE_RA3_0               (1<<24) // first buffer audio
#define   SAA7134_IRQ1_INTE_RA2_3               (1<<19)
#define   SAA7134_IRQ1_INTE_RA2_2               (1<<18)
#define   SAA7134_IRQ1_INTE_RA2_1               (1<<17)
#define   SAA7134_IRQ1_INTE_RA2_0               (1<<16)
#define   SAA7134_IRQ1_INTE_RA1_3               (1<<11)
#define   SAA7134_IRQ1_INTE_RA1_2               (1<<10)
#define   SAA7134_IRQ1_INTE_RA1_1               (1<<9)
#define   SAA7134_IRQ1_INTE_RA1_0               (1<<8)
#define   SAA7134_IRQ1_INTE_RA0_7               (1<<7)
#define   SAA7134_IRQ1_INTE_RA0_6               (1<<6)
#define   SAA7134_IRQ1_INTE_RA0_5               (1<<5)
#define   SAA7134_IRQ1_INTE_RA0_4               (1<<4)
#define   SAA7134_IRQ1_INTE_RA0_3               (1<<3)
#define   SAA7134_IRQ1_INTE_RA0_2               (1<<2)
#define   SAA7134_IRQ1_INTE_RA0_1               (1<<1)
#define   SAA7134_IRQ1_INTE_RA0_0               (1<<0)
#define SAA7134_IRQ2                        0x2C8
#define   SAA7134_IRQ2_INTE_SC2                 (1<<10)
#define   SAA7134_IRQ2_INTE_SC1                 (1<<9)
#define   SAA7134_IRQ2_INTE_SC0                 (1<<8)
#define   SAA7134_IRQ2_INTE_DEC5                (1<<7)
#define   SAA7134_IRQ2_INTE_DEC4                (1<<6)
#define   SAA7134_IRQ2_INTE_DEC3                (1<<5)
#define   SAA7134_IRQ2_INTE_DEC2                (1<<4)
#define   SAA7134_IRQ2_INTE_DEC1                (1<<3)
#define   SAA7134_IRQ2_INTE_DEC0                (1<<2)
#define   SAA7134_IRQ2_INTE_PE                  (1<<1)
#define   SAA7134_IRQ2_INTE_AR                  (1<<0)
#define SAA7134_IRQ_REPORT                  0x2CC
#define   SAA7134_IRQ_REPORT_LOAD_ERR           (1<<13)
#define   SAA7134_IRQ_REPORT_CONF_ERR           (1<<12)
#define   SAA7134_IRQ_REPORT_TRIG_ERR           (1<<11)
#define   SAA7134_IRQ_REPORT_MMC                (1<<10)
#define   SAA7134_IRQ_REPORT_FIDT               (1<<9)
#define   SAA7134_IRQ_REPORT_INTL               (1<<8)
#define   SAA7134_IRQ_REPORT_RDCAP              (1<<7)
#define   SAA7134_IRQ_REPORT_PWR_ON             (1<<6)
#define   SAA7134_IRQ_REPORT_PE                 (1<<5)
#define   SAA7134_IRQ_REPORT_AR                 (1<<4)
#define   SAA7134_IRQ_REPORT_DONE_RA3           (1<<3)
#define   SAA7134_IRQ_REPORT_DONE_RA2           (1<<2)
#define   SAA7134_IRQ_REPORT_DONE_RA1           (1<<1)
#define   SAA7134_IRQ_REPORT_DONE_RA0           (1<<0)
#define SAA7134_IRQ_STATUS                  0x2D0


/* ------------------------------------------------------------------ */
/*
 *  registers -- 8 bit
 */

// video decoder
#define SAA7134_INCR_DELAY                      0x101
#define     SAA7134_INCR_DELAY_IDEL                 (0x0F)
#define     SAA7134_INCR_DELAY_GUDL                 (3<<4)
#define     SAA7134_INCR_DELAY_WPOFF                (1<<6)
#define SAA7134_ANALOG_IN_CTRL1                 0x102
#define     SAA7134_ANALOG_IN_CTRL1_MODE            (0x0F)
#define     SAA7134_ANALOG_IN_CTRL1_FUSE            (3<<6)
#define SAA7134_ANALOG_IN_CTRL2                 0x103
#define     SAA7134_ANALOG_IN_CTRL2_GAI18           (1<<0)
#define     SAA7134_ANALOG_IN_CTRL2_GAI28           (1<<1)
#define     SAA7134_ANALOG_IN_CTRL2_GAFIX           (1<<2)
#define     SAA7134_ANALOG_IN_CTRL2_HOLDG           (1<<3)
#define     SAA7134_ANALOG_IN_CTRL2_CPOFF           (1<<4)
#define     SAA7134_ANALOG_IN_CTRL2_VBSL            (1<<5)
#define     SAA7134_ANALOG_IN_CTRL2_HLNRS           (1<<6)
#define     SAA7134_ANALOG_IN_CTRL2_TEST            (1<<7)
#define SAA7134_ANALOG_IN_CTRL3                 0x104
#define SAA7134_ANALOG_IN_CTRL4                 0x105
#define SAA7134_HSYNC_START                     0x106
#define SAA7134_HSYNC_STOP                      0x107
#define SAA7134_SYNC_CTRL                       0x108
// guessed these by trying them out
#define     SAA7134_SYNC_CTRL_VNOI                  (0x03)  // 0=normal (fault tolerant)
#define     SAA7134_SYNC_CTRL_HPLL                  (1<<2)  // 0=enabled
#define     SAA7134_SYNC_CTRL_STANDARD_SIGNAL       (1<<3)  // \todo bad, need to remove
#define     SAA7134_SYNC_CTRL_HTD                   (3<<3)  // 0=TV, 1=VTR, 3=fast tracking
#define     SAA7134_SYNC_CTRL_FOET                  (1<<5)  // FID generation mode
#define     SAA7134_SYNC_CTRL_FSEL                  (1<<6)  // 0=50Hz, 1=60Hz field rate (AUFD=0)
#define     SAA7134_SYNC_CTRL_AUFD                  (1<<7)  // 1=auto field rate detect
#define SAA7134_LUMA_CTRL                       0x109
// guessed these - unverified
#define     SAA7134_LUMA_CTRL_LUFI                  (0x0F)  // 0=none, 7->1 sharper, 8->F blurrier
#define     SAA7134_LUMA_CTRL_LUBW                  (1<<4)  //
#define     SAA7134_LUMA_CTRL_LDEL                  (1<<5)  // for debug
#define     SAA7134_LUMA_CTRL_YCOMB                 (1<<6)  // 0=chroma trap, 1=luma comb
#define     SAA7134_LUMA_CTRL_BYPS                  (1<<7)  // Bypass chroma trap/luma comb

#define SAA7134_DEC_LUMA_BRIGHT                 0x10A       // Decoder Brightness
#define SAA7134_DEC_LUMA_CONTRAST               0x10B       // Decoder Contrast
#define SAA7134_DEC_CHROMA_SATURATION           0x10C       // Decoder Saturation
#define SAA7134_DEC_CHROMA_HUE                  0x10D       // Decoder Hue
#define SAA7134_CHROMA_CTRL1                    0x10E
#define     SAA7134_CHROMA_CTRL1_CCOMB              (1<<0)  // Adaptive chroma comb filter
#define     SAA7134_CHROMA_CTRL1_AUTO               (1<<1)  // Auto color standard detect
#define     SAA7134_CHROMA_CTRL1_FCTC               (1<<2)  // 
#define     SAA7134_CHROMA_CTRL1_DCVF               (1<<3)  // 
#define     SAA7134_CHROMA_CTRL1_CSTD               (7<<4)  // 
#define     SAA7134_CHROMA_CTRL1_CDTO               (1<<7)  //
#define SAA7134_CHROMA_GAIN_CTRL                0x10F
#define     SAA7134_CHROMA_GAIN_CTRL_CGAIN           (0x7F)  // 0 .. 127
#define     SAA7134_CHROMA_GAIN_CTRL_ACGC            (1<<7)  // ACGC
#define SAA7134_CHROMA_CTRL2                    0x110
#define     SAA7134_CHROMA_CTRL2_LCBW               (0x07)
#define     SAA7134_CHROMA_CTRL2_CHBW               (1<<3)
#define     SAA7134_CHROMA_CTRL2_OFFV               (3<<4)
#define     SAA7134_CHROMA_CTRL2_OFFU               (3<<6)
#define SAA7134_MODE_DELAY_CTRL                 0x111

#define SAA7134_ANALOG_ADC                      0x114
#define     SAA7134_ANALOG_ADC_APCK                 (3<<0)  // 0=default
#define     SAA7134_ANALOG_ADC_AUTO                 (1<<2)  //
#define     SAA7134_ANALOG_ADC_AOSL                 (3<<4)  // 0=default (for testing)
#define     SAA7134_ANALOG_ADC_UPTCV                (1<<6)  // agc gain 0=per line, 1=per field
#define     SAA7134_ANALOG_ADC_CM99                 (1<<7)  // saa7199

#define SAA7134_VGATE_START                     0x115
#define SAA7134_VGATE_STOP                      0x116
#define SAA7134_MISC_VGATE_MSB                  0x117
#define     SAA7134_MISC_VGATE_MSB_VGB_8            (1<<0)  //
#define     SAA7134_MISC_VGATE_MSB_VGE_8            (1<<1)  //
#define     SAA7134_MISC_VGATE_MSB_VGPS             (1<<2)  // 0=second field is top
#define     SAA7134_MISC_VGATE_MSB_LATY             (3<<3)  // 1-7, color detect fields

#define SAA7134_RAW_DATA_GAIN                   0x118
#define SAA7134_RAW_DATA_OFFSET                 0x119
#define SAA7134_STATUS_VIDEO                    0x11E
#define SAA7134_STATUS_VIDEO_HIBYTE             0x11F
#define     SAA7134_STATUS_VIDEO_DCSTD              (3<<0)
#define     SAA7134_STATUS_VIDEO_WIPA               (1<<2)
#define     SAA7134_STATUS_VIDEO_GLIMB              (1<<3)
#define     SAA7134_STATUS_VIDEO_GLIMT              (1<<4)
#define     SAA7134_STATUS_VIDEO_SLTCA              (1<<5)
#define     SAA7134_STATUS_VIDEO_HLCK               (1<<6)  // 0=HPLL locked
#define     SAA7134_STATUS_VIDEO_RDCAP              (1<<8)
#define     SAA7134_STATUS_VIDEO_COPRO              (1<<9)
#define     SAA7134_STATUS_VIDEO_COLSTR             (1<<10)
#define     SAA7134_STATUS_VIDEO_TYPE3              (1<<11)
#define     SAA7134_STATUS_VIDEO_FIDT               (1<<13) // 0=50Hz, 1=60Hz
#define     SAA7134_STATUS_VIDEO_HLVLN              (1<<14) // 1=horiz/vert sync lost
#define     SAA7134_STATUS_VIDEO_INTL               (1<<15) // 1=interlaced

#define SAA7134_SOURCE_TIMING                   0x000
#define SAA7134_SOURCE_TIMING_HIBYTE            0x001
#define     SAA7134_SOURCE_TIMING_YPOS              (0xFFF)
#define     SAA7134_SOURCE_TIMING_DHED              (1<<12)
#define     SAA7134_SOURCE_TIMING_DVED              (1<<13)
#define     SAA7134_SOURCE_TIMING_DFID              (1<<14) // reverse field ID
#define SAA7134_REGION_ENABLE                   0x004
#define     SAA7134_REGION_ENABLE_VID_ENA           (1<<0)
#define     SAA7134_REGION_ENABLE_VBI_ENA           (1<<1)
#define     SAA7134_REGION_ENABLE_VID_ENB           (1<<4)
#define     SAA7134_REGION_ENABLE_VBI_ENB           (1<<5)
#define     SAA7134_REGION_ENABLE_SWRST             (1<<7)  // 0-1-0 to reset
#define SAA7134_SCALER_STATUS                   0x006
#define SAA7134_SCALER_STATUS_HIBYTE            0x007
#define     SAA7134_SCALER_STATUS_VID_A             (1<<0)
#define     SAA7134_SCALER_STATUS_VBI_A             (1<<1)
#define     SAA7134_SCALER_STATUS_VID_B             (1<<4)
#define     SAA7134_SCALER_STATUS_VBI_B             (1<<5)
#define     SAA7134_SCALER_STATUS_TRERR             (1<<8)
#define     SAA7134_SCALER_STATUS_CFERR             (1<<9)
#define     SAA7134_SCALER_STATUS_LDERR             (1<<10)
#define     SAA7134_SCALER_STATUS_WASRST            (1<<11)
#define     SAA7134_SCALER_STATUS_FIDSCI            (1<<12) // 1=odd, 0=even (v0.1 doc is wrong!)
#define     SAA7134_SCALER_STATUS_FIDSCO            (1<<13) // 1=upper, 0=lower (v0.1 doc is wrong!)
#define     SAA7134_SCALER_STATUS_D6_D5             (1<<14)
#define     SAA7134_SCALER_STATUS_TASK              (1<<15) // 0=a, 1=b

#define SAA7134_START_GREEN                     0x00C
#define SAA7134_START_BLUE                      0x00D
#define SAA7134_START_RED                       0x00E
// x = 0 .. 15
#define SAA7134_GREEN_PATH(x)                   (0x010|x)
#define SAA7134_BLUE_PATH(x)                    (0x020|x)
#define SAA7134_RED_PATH(x)                     (0x030|x)


#define SAA7134_TASK_A_MASK                     0x040
#define SAA7134_TASK_B_MASK                     0x080

#define SAA7134_TASK_CONDITIONS(t)              (0x00|t)
#define     SAA7134_TASK_CONDITIONS_UODD            (1<<0)
#define     SAA7134_TASK_CONDITIONS_UEVE            (1<<1)
#define     SAA7134_TASK_CONDITIONS_U50             (1<<2)
#define     SAA7134_TASK_CONDITIONS_U60             (1<<3)
#define     SAA7134_TASK_CONDITIONS_SLCK            (1<<4)
#define SAA7134_FIELD_HANDLING(t)               (0x01|t)
#define     SAA7134_FIELD_HANDLING_FIELDS           (0x03)  // 0-3, fields to process
#define     SAA7134_FIELD_HANDLING_SKIP             (0xFC)  // 0-63, fields to skip

#define SAA7134_DATA_PATH(t)                    (0x02|t)

#define SAA7134_VBI_H_START(t)                  (0x04|t) 
#define SAA7134_VBI_H_START_HIBYTE(t)           (0x05|t)
#define SAA7134_VBI_H_STOP(t)                   (0x06|t)
#define SAA7134_VBI_H_STOP_HIBYTE(t)            (0x07|t)
#define SAA7134_VBI_V_START(t)                  (0x08|t)
#define SAA7134_VBI_V_START_HIBYTE(t)           (0x09|t)
#define SAA7134_VBI_V_STOP(t)                   (0x0a|t)
#define SAA7134_VBI_V_STOP_HIBYTE(t)            (0x0b|t)
#define SAA7134_VBI_H_LEN(t)                    (0x0c|t)
#define SAA7134_VBI_H_LEN_HIBYTE(t)             (0x0d|t)
#define SAA7134_VBI_V_LEN(t)                    (0x0e|t)
#define SAA7134_VBI_V_LEN_HIBYTE(t)             (0x0f|t)

#define SAA7134_VIDEO_H_START(t)                (0x14|t)
#define SAA7134_VIDEO_H_START_HIBYTE(t)         (0x15|t)
#define SAA7134_VIDEO_H_STOP(t)                 (0x16|t)
#define SAA7134_VIDEO_H_STOP_HIBYTE(t)          (0x17|t)
#define SAA7134_VIDEO_V_START(t)                (0x18|t)
#define SAA7134_VIDEO_V_START_HIBYTE(t)         (0x19|t)
#define SAA7134_VIDEO_V_STOP(t)                 (0x1A|t)
#define SAA7134_VIDEO_V_STOP_HIBYTE(t)          (0x1B|t)
#define SAA7134_VIDEO_PIXELS(t)                 (0x1C|t)
#define SAA7134_VIDEO_PIXELS_HIBYTE(t)          (0x1D|t)
#define SAA7134_VIDEO_LINES(t)                  (0x1E|t)
#define SAA7134_VIDEO_LINES_HIBYTE(t)           (0x1F|t)

#define SAA7134_H_PRESCALE(t)                   (0x20|t)
#define SAA7134_ACC_LENGTH(t)                   (0x21|t)
#define SAA7134_LEVEL_CTRL(t)                   (0x22|t)
#define SAA7134_FIR_PREFILTER_CTRL(t)           (0x23|t)
#define SAA7134_LUMA_BRIGHT(t)                  (0x24|t)
#define SAA7134_LUMA_CONTRAST(t)                (0x25|t)
#define SAA7134_CHROMA_SATURATION(t)            (0x26|t)
#define SAA7134_VBI_H_SCALE_INC(t)              (0x28|t)
#define SAA7134_VBI_H_SCALE_INC_HIBYTE(t)       (0x29|t)
#define SAA7134_VBI_PHASE_OFFSET_LUMA(t)        (0x2A|t)
#define SAA7134_VBI_PHASE_OFFSET_CHROMA(t)      (0x2B|t)
#define SAA7134_H_SCALE_INC(t)                  (0x2C|t)
#define SAA7134_H_SCALE_INC_HIBYTE(t)           (0x2D|t)
#define SAA7134_H_PHASE_OFF_LUMA(t)             (0x2E|t)
#define SAA7134_H_PHASE_OFF_CHROMA(t)           (0x2F|t)
#define SAA7134_V_SCALE_RATIO(t)                (0x30|t)
#define SAA7134_V_SCALE_RATIO_HIBYTE(t)         (0x31|t)
#define SAA7134_V_FILTER(t)                     (0x32|t)
#define     SAA7134_V_FILTER_YMODE                  (1<<0)
#define     SAA7134_V_FILTER_YMIR                   (1<<1)  // mirror
#define SAA7134_V_PHASE_OFFSET0(t)              (0x34|t)
#define SAA7134_V_PHASE_OFFSET1(t)              (0x35|t)
#define SAA7134_V_PHASE_OFFSET2(t)              (0x36|t)
#define SAA7134_V_PHASE_OFFSET3(t)              (0x37|t)

// clipping & dma
#define SAA7134_OFMT_VIDEO_A                    0x300       // formats
#define SAA7134_OFMT_DATA_A                     0x301
#define SAA7134_OFMT_VIDEO_B                    0x302
#define SAA7134_OFMT_DATA_B                     0x303
#define SAA7134_ALPHA_NOCLIP                    0x304
#define SAA7134_ALPHA_CLIP                      0x305
#define SAA7134_UV_PIXEL                        0x308
#define SAA7134_CLIP_RED                        0x309
#define SAA7134_CLIP_GREEN                      0x30a
#define SAA7134_CLIP_BLUE                       0x30b

// added, unverified
// Clips points, n = 0..15
#define SAA7134_CLIP_H_ACTIVE(n)                (0x380|(n)<<8)
#define SAA7134_CLIP_H_NOIDEA(n)                (0x381|(n)<<8)
#define SAA7134_CLIP_H_POS(n)                   (0x382|(n)<<8)
#define SAA7134_CLIP_H_POS_HIBYTE(n)            (0x383|(n)<<8)
#define SAA7134_CLIP_V_ACTIVE(n)                (0x384|(n)<<8)
#define SAA7134_CLIP_V_NOIDEA(n)                (0x385|(n)<<8)
#define SAA7134_CLIP_V_POS(n)                   (0x386|(n)<<8)
#define SAA7134_CLIP_V_POS_HIBYTE(n)            (0x387|(n)<<8)


// audio
#define SAA7134_NICAM_ADD_DATA1                 0x140
#define SAA7134_NICAM_ADD_DATA2                 0x141
#define SAA7134_NICAM_STATUS                    0x142
#define     SAA7134_NICAM_STATUS_SIN                (3<<0)
#define     SAA7134_NICAM_STATUS_CFC                (1<<2)
#define     SAA7134_NICAM_STATUS_C0S                (1<<3)
#define     SAA7134_NICAM_STATUS_C1                 (1<<4)
#define     SAA7134_NICAM_STATUS_C2                 (1<<5)
#define     SAA7134_NICAM_STATUS_C3                 (1<<6)
#define     SAA7134_NICAM_STATUS_C4                 (1<<7)
#define SAA7134_AUDIO_STATUS                    0x143
#define SAA7134_NICAM_ERROR_COUNT               0x144
#define SAA7134_IDENT_SIF                       0x145
#define     SAA7134_IDENT_SIF_SIFL                  (0x1F)
#define     SAA7134_IDENT_SIF_IDP                   (0xE0)
#define SAA7134_LEVEL_READOUT                   0x146
#define SAA7134_LEVEL_READOUT_HIBYTE            0x147
#define SAA7134_NICAM_ERROR_LOW                 0x148
#define SAA7134_NICAM_ERROR_HIGH                0x149
#define SAA7134_DCXO_IDENT_CTRL                 0x14A
#define     SAA7134_DCXO_IDENT_CTRL_IDAREA          (1<<0)
#define     SAA7134_DCXO_IDENT_CTRL_IDMOD           (3<<1)
#define SAA7134_DEMODULATOR                     0x14B
#define     SAA7134_DEMODULATOR_CH2MOD0             (1<<0)
#define     SAA7134_DEMODULATOR_CH1MODE             (1<<1)
#define     SAA7134_DEMODULATOR_FILTBW              (3<<2)
#define     SAA7134_DEMODULATOR_CH2MOD1             (1<<4)

#define SAA7134_AGC_GAIN_SELECT                 0x14C
#define     SAA7134_AGC_GAIN_SELECT_SIFGAIN         (0x1F)
#define     SAA7134_AGC_GAIN_SELECT_AGCSLOW         (1<<5)
#define     SAA7134_AGC_GAIN_SELECT_AGCOFF          (1<<6)
#define SAA7134_CARRIER1_FREQ                   0x150
#define SAA7134_CARRIER1_FREQ_HIWORD            0x152
#define SAA7134_CARRIER2_FREQ                   0x154
#define SAA7134_CARRIER2_FREQ_HIWORD            0x156
#define SAA7134_NUM_SAMPLES                     0x158 // 22 bits
#define     SAA7134_NUM_SAMPLES_MASK                (0x3FFFFF)
#define SAA7134_AUDIO_FORMAT_CTRL               0x15B
#define SAA7134_MONITOR_SELECT                  0x160
#define SAA7134_FM_DEEMPHASIS                   0x161
#define SAA7134_FM_DEMATRIX                     0x162
#define SAA7134_CHANNEL1_LEVEL                  0x163 // -15..15, 0 = normal
#define SAA7134_CHANNEL2_LEVEL                  0x164 // -15..15, 0 = normal
#define SAA7134_NICAM_CONFIG                    0x165
#define SAA7134_NICAM_LEVEL_ADJUST              0x166 // -15..15, 0 = normal
#define SAA7134_STEREO_DAC_OUTPUT_SELECT        0x167
#define     SAA7134_STEREO_DAC_OUTPUT_SELECT_SDOS   (3<<0)
#define     SAA7134_STEREO_DAC_OUTPUT_SELECT_AVL    (3<<4)
#define     SAA7134_STEREO_DAC_OUTPUT_SELECT_SDGS   (3<<6)
#define SAA7134_I2S_OUTPUT_FORMAT               0x168
#define SAA7134_I2S_OUTPUT_SELECT               0x169
#define SAA7134_I2S_OUTPUT_LEVEL                0x16A
#define SAA7134_DSP_OUTPUT_SELECT               0x16B
#define     SAA7134_DSP_OUTPUT_SELECT_CSM           (7<<4)
#define     SAA7134_DSP_OUTPUT_SELECT_AASDMA        (1<<7)
#define SAA7134_AUDIO_MUTE_CTRL                 0x16C
#define     SAA7134_AUDIO_MUTE_CTRL_MUTSOUT         (1<<2)
#define     SAA7134_AUDIO_MUTE_CTRL_MUTI2S          (1<<6)
#define SAA7134_SIF_SAMPLE_FREQ                 0x16D
#define     SAA7134_SIF_SAMPLE_FREQ_SFS             (3<<0)
#define     SAA7134_SIF_SAMPLE_FREQ_HPEN            (1<<2)
#define     SAA7134_SIF_SAMPLE_FREQ_SIFP            (3<<4)
#define     SAA7134_SIF_SAMPLE_FREQ_SIFA            (3<<6)
#define SAA7134_ANALOG_IO_SELECT                0x16E
#define     SAA7134_ANALOG_IO_SELECT_OCS            (7<<0)
#define     SAA7134_ANALOG_IO_SELECT_ICS            (1<<3)
#define     SAA7134_ANALOG_IO_SELECT_VSEL1          (1<<4)
#define     SAA7134_ANALOG_IO_SELECT_VSEL2          (1<<5)
#define SAA7134_AUDIO_CLOCK                     0x170 // 22 bits
#define     SAA7134_AUDIO_CLOCK_MASK                (0x3FFFFF)
#define SAA7134_AUDIO_PLL_CTRL                  0x173
#define     SAA7134_AUDIO_PLL_CTRL_SWLOOP           (1<<0)
#define     SAA7134_AUDIO_PLL_CTRL_APLL             (1<<1)
#define SAA7134_AUDIO_CLOCKS_PER_FIELD          0x174
#define     SAA7134_AUDIO_CLOCKS_PER_FIELD_MASK     (0x03FFFF)

// i2c bus
#define SAA7134_I2C_ATTR_STATUS                 0x180
#define SAA7134_I2C_DATA                        0x181
#define SAA7134_I2C_CLOCK_SELECT                0x182
#define SAA7134_I2C_TIMER                       0x183

// video port output
#define SAA7134_VIDEO_PORT_CTRL0                0x190
#define SAA7134_VIDEO_PORT_CTRL1                0x191
#define SAA7134_VIDEO_PORT_CTRL2                0x192
#define SAA7134_VIDEO_PORT_CTRL3                0x193
#define SAA7134_VIDEO_PORT_CTRL4                0x194
#define SAA7134_VIDEO_PORT_CTRL5                0x195
#define SAA7134_VIDEO_PORT_CTRL6                0x196
#define SAA7134_VIDEO_PORT_CTRL7                0x197
#define SAA7134_VIDEO_PORT_CTRL8                0x198

// transport stream interface
#define SAA7134_TS_PARALLEL                     0x1A0
#define SAA7134_TS_PARALLEL_SERIAL              0x1A1
#define SAA7134_TS_SERIAL0                      0x1A2
#define SAA7134_TS_SERIAL1                      0x1A3
#define SAA7134_TS_DMA0                         0x1A4
#define SAA7134_TS_DMA1                         0x1A5
#define SAA7134_TS_DMA2                         0x1A6

// GPIO Controls
#define SAA7134_GPIO_GPMODE                     0x1B0
#define     SAA7134_GPIO_GPMODE_GPMOD_23_0          (0x00FFFFFF)
#define     SAA7134_GPIO_GPMODE_GPMOD_27_25         (0x0E000000)
#define     SAA7134_GPIO_GPMODE_GPRESCN             (0x80000000)
#define SAA7134_GPIO_GPSTATUS                   0x1B4
#define     SAA7134_GPIO_GPSTATUS_GPSTA_23_0        (0x00FFFFFF)
#define     SAA7134_GPIO_GPSTATUS_GPSTA_27_25       (0x0E000000)

// I2S output
#define SAA7134_I2S_AUDIO_OUTPUT                0x1C0

// test modes
#define SAA7134_SPECIAL_MODE                    0x1D0


//////////////////////////////////////////////////////////////////////////
// SAA7133/5: Audio and sound processing registers
//////////////////////////////////////////////////////////////////////////

// #define SAA713x_REGISTER				reg-offset, reg-length
// #define     SAA713x_REGISTER_SUB			sub-offset, sub-length

// Control register map (read direct, write direct)
#define SAA7133_EPICS_ACCESS_STATUS				0x580, 4
#define 	SAA7133_EPICS_ACCESS_STATUS_WRR			0, 1
#define 	SAA7133_EPICS_ACCESS_STATUS_RDR			1, 1
#define 	SAA7133_EPICS_ACCESS_STATUS_IDA			2, 1
#define 	SAA7133_EPICS_ACCESS_STATUS_ERR			3, 1
#define SAA7133_SIF_LVL							0x581, 5
#define SAA7133_AU2DMA_STATUS					0x582, 1
#define		SAA7133_AUD2DMA_STATUS_TOUT				0, 1
#define SAA7133_AGC_GAIN_CONTROL				0x584, 7
#define		SAA7133_AGC_GAIN_CONTROL_AGC			0, 5
#define		SAA7133_AGC_GAIN_CONTROL_AGCSLOW		5, 1
#define		SAA7133_AGC_GAIN_CONTROL_AGCOFF			6, 1
#define SAA7133_PROGRAM_COUNT					0x585, 2
#define		SAA7133_PROGRAM_COUNT_PCH				0, 1
#define		SAA7133_PROGRAM_COUNT_PCR				0, 1
#define SAA7133_STATUS_RESET					0x586, 2
#define		SAA7133_STATUS_RESET_RERR				0, 1
#define		SAA7133_STATUS_RESET_RRDR				1, 1
#define SAA7133_NUM_SAMPLES						0x588, 22
#define SAA7133_AUDIO_SELECT					0x58B, 2
#define		SAA7133_AUDIO_SELECT_AUSEL				0, 2
#define SAA7133_AUDIO_CHANNEL					0x58C,	24
#define		SAA7133_AUDIO_CHANNEL_AUCHA0			0, 4
#define		SAA7133_AUDIO_CHANNEL_AUCHA1			4, 4
#define		SAA7133_AUDIO_CHANNEL_AUCHA2			8, 4
#define		SAA7133_AUDIO_CHANNEL_AUCHA3			12, 4
#define		SAA7133_AUDIO_CHANNEL_AUCHA4			16, 4
#define		SAA7133_AUDIO_CHANNEL_AUCHA5			20, 4
#define SAA7133_AUDIO_FORMAT					0x58F, 8
#define		SAA7133_AUDIO_FORMAT_FORMAT				0, 4
#define		SAA7133_AUDIO_FORMAT_NCH				4, 4
#define	SAA7133_SIF_CONTROL						0x590, 8
#define		SAA7133_SIF_CONTROL_HPEN				2, 1
#define		SAA7133_SIF_CONTROL_SIFP				4, 1
#define		SAA7133_SIF_CONTROL_SIFG				6, 2
#define	SAA7133_IIS_CONTROL						0x591, 2
#define		SAA7133_IIS_CONTROL_IISFORM				0, 1
#define		SAA7133_IIS_CONTROL_IISJAPAN			1, 1
#define	SAA7133_ANALOG_IO_SELECT				0x594, 6
#define		SAA7133_ANALOG_IO_SELECT_OCS			0, 3
#define		SAA7133_ANALOG_IO_SELECT_ICS			3, 1
#define		SAA7133_ANALOG_IO_SELECT_VSEL			4, 2
#define	SAA7133_AUDIO_CLOCK_NOMINAL				0x598, 22
#define	SAA7133_PLL_CONTROL						0x59B, 8
#define		SAA7133_PLL_CONTROL_SWLOOP				0, 1
#define		SAA7133_PLL_CONTROL_APLL				1, 1
#define		SAA7133_PLL_CONTROL_ACS					4, 4
#define	SAA7133_AUDIO_CLOCKS_PER_FIELD			0x59C, 18
#define	SAA7133_XRAM_YRAM_DATA					0x5A0, 31
#define		SAA7133_XRAM_YRAM_DATA_RDATA			0, 24
#define		SAA7133_XRAM_YRAM_DATA_ERR				28, 1
#define		SAA7133_XRAM_YRAM_DATA_WRR				29, 1
#define		SAA7133_XRAM_YRAM_DATA_RDR				30, 1
#define	SAA7133_XRAM_YRAM_ADDRESS				0x5A4, 16
#define	SAA7133_READ_WRITE_SELECT				0x5A7, 1
#define		SAA7133_READ_WRITE_SELECT_RWSEL			0, 1

// XREG register map (read direct, write w/ DSP)
// Prefixed with "E" for distinguishing.
#define	SAA7133_E_TRANSFERRED_ADDRESS			0x500, 16
#define	SAA7133_E_HW_STATUS						0x504, 22
#define		SAA7133_E_HW_STATUS_APILOT				0, 1
#define		SAA7133_E_HW_STATUS_ADU					1, 1
#define		SAA7133_E_HW_STATUS_AST					2, 1
#define		SAA7133_E_HW_STATUS_SRD					3, 2
#define		SAA7133_E_HW_STATUS_ERR_OUT				5, 8
#define		SAA7133_E_HW_STATUS_CFC					13, 1
#define		SAA7133_E_HW_STATUS_CO_LOCKED			14, 1
#define		SAA7133_E_HW_STATUS_NACB				15, 4
#define		SAA7133_E_HW_STATUS_VDSP				19, 1
#define		SAA7133_E_HW_STATUS_NICST				20, 1
#define		SAA7133_E_HW_STATUS_NICDU				21, 1
#define	SAA7133_E_NICAM_STATUS					0x508, 15
#define		SAA7133_E_NICAM_STATUS_ADW				0, 11
#define		SAA7133_E_NICAM_STATUS_PARCHK			11, 2
#define		SAA7133_E_NICAM_STATUS_SAD				13, 1
#define		SAA7133_E_NICAM_STATUS_OVW				14, 1
#define	SAA7133_E_MPX_PILOT_LVL					0x50C, 24
#define		SAA7133_E_MPX_PILOT_LVL_MPX_PLEV		6, 18
#define	SAA7133_E_NOISE_LVL						0x510, 24
#define	SAA7133_E_HW_CONFIG						0x514, 19
#define		SAA7133_E_HW_CFG_CH2MOD					0, 2
#define		SAA7133_E_HW_CFG_CH1MOD					2, 1
#define		SAA7133_E_HW_CFG_INITLPF				3, 1
#define		SAA7133_E_HW_CFG_FILTBW					5, 2
#define		SAA7133_E_HW_CFG_IDMOD					7, 2
#define		SAA7133_E_HW_CFG_IDAREA					9, 2
#define		SAA7133_E_HW_CFG_BPILCAN				11, 1
#define		SAA7133_E_HW_CFG_FM_MPX					12, 1
#define		SAA7133_E_HW_CFG_ID_DC_LEVEL			13, 2
#define		SAA7133_E_HW_CFG_ID_BYBPF				15, 1
#define		SAA7133_E_HW_CFG_NOISELEV_EN			16, 1
#define		SAA7133_E_HW_CFG_PILOTLEV_EN			17, 1
#define		SAA7133_E_HW_CFG_ID_PGAIN				18, 1
#define	SAA7133_E_CARRIER1						0x518, 24
#define	SAA7133_E_CARRIER2						0x51C, 24
#define	SAA7133_E_MPX_CONFIG					0x520, 24
#define		SAA7133_E_MPX_CFG_MPX_PLL_BW			1, 1
#define		SAA7133_E_MPX_CFG_MPX_FREQ				2, 22
#define	SAA7133_E_FMSUB_CONFIG					0x524, 24
#define		SAA7133_E_FMSUB_CFG_FMSUB_BW			0, 1
#define		SAA7133_E_FMSUB_CFG_EIAJ_DELAY			1, 2
#define		SAA7133_E_FMSUB_CFG_NDETCH				3, 1
#define		SAA7133_E_FMSUB_CFG_NDETPB				4, 1
#define		SAA7133_E_FMSUB_CFG_FMSUB_FREQ			8, 16
#define	SAA7133_E_DEMDEC_STATUS					0x528, 24
#define	SAA7133_E_MONITOR_LVL					0x52C, 24
#define	SAA7133_E_FLAG_CONFIG1					0x530, 13
#define	SAA7133_E_FLAG_CONFIG2					0x534, 13

// XRAM register map (read w/ DSP, write w/ DSP)
// Prefixed with "A" for distinguishing.
#define	SAA7133_A_MAIN_STATUS					0x404, 21
#define		SAA7133_A_MAIN_STATUS_STDRES			0, 5
#define		SAA7133_A_MAIN_STATUS_GST				5, 1
#define		SAA7133_A_MAIN_STATUS_GDU				6, 1
#define		SAA7133_A_MAIN_STATUS_APILOT			7, 1
#define		SAA7133_A_MAIN_STATUS_ADU				8, 1
#define		SAA7133_A_MAIN_STATUS_AST				9, 1
#define		SAA7133_A_MAIN_STATUS_AAMUT				10, 1
#define		SAA7133_A_MAIN_STATUS_BPILOT			11, 1
#define		SAA7133_A_MAIN_STATUS_SAPDET			12, 1
#define		SAA7133_A_MAIN_STATUS_BAMUT				13, 1
#define		SAA7133_A_MAIN_STATUS_SAMUT				14, 1
#define		SAA7133_A_MAIN_STATUS_VDSP_C			15, 1
#define		SAA7133_A_MAIN_STATUS_NICST_C			16, 1
#define		SAA7133_A_MAIN_STATUS_NICDU_C			17, 1
#define		SAA7133_A_MAIN_STATUS_NAMUT				18, 1
#define		SAA7133_A_MAIN_STATUS_RSSF				19, 1
#define		SAA7133_A_MAIN_STATUS_INITSTAT			20, 1
#define	SAA7133_A_NICAM_STATUS					0x408, 17
#define		SAA7133_A_NICAM_STATUS_ERR_OUT			0, 8
#define		SAA7133_A_NICAM_STATUS_CFC				8, 1
#define		SAA7133_A_NICAM_STATUS_CO_LOCKED		9, 1
#define		SAA7133_A_NICAM_STATUS_NACB				10, 4
#define		SAA7133_A_NICAM_STATUS_VDSP				14, 1
#define		SAA7133_A_NICAM_STATUS_NICST			15, 1
#define		SAA7133_A_NICAM_STATUS_NICDU			16, 1
#define	SAA7133_A_MONITOR_LVL					0x410, 24
#define	SAA7133_A_MPX_LVL						0x414, 24
#define		SAA7133_A_MPX_LVL_MPXPLEV				6, 18
#define	SAA7133_A_DC_OFFSET						0x418, 24
#define	SAA7133_A_FM_MAGNITUDE					0x41C, 24
#define	SAA7133_A_NOISE_LVL						0x420, 24
#define	SAA7133_A_SOFTWARE_REVISION				0x424, 24
#define	SAA7133_A_DEMDEC_CONFIG					0x428, 22
#define		SAA7133_A_DEMDEC_CFG_DECPATH			0, 3
#define		SAA7133_A_DEMDEC_CFG_FMDEEM				3, 3
#define		SAA7133_A_DEMDEC_CFG_CH2MOD				6, 2
#define		SAA7133_A_DEMDEC_CFG_CH1MOD				8, 3
#define		SAA7133_A_DEMDEC_CFG_FLTBW_M			11, 2
#define		SAA7133_A_DEMDEC_CFG_IDMOD_M			13, 2
#define		SAA7133_A_DEMDEC_CFG_IDAREA				15, 2
#define		SAA7133_A_DEMDEC_CFG_BPILCA				17, 1
#define		SAA7133_A_DEMDEC_CFG_FM_MPX				18, 1
#define		SAA7133_A_DEMDEC_CFG_ID_DC_LEV			19, 2
#define		SAA7133_A_DEMDEC_CFG_ID_BYBPF			21, 1
#define	SAA7133_A_DEMDEC_CARRIER1				0x42C, 24
#define	SAA7133_A_DEMDEC_CARRIER2				0x430, 24
#define	SAA7133_A_MPX_CONFIG					0x434, 24
#define		SAA7133_A_MPX_CFG_MPX_PLL_BW			1, 1
#define		SAA7133_A_MPX_CFG_MPX_FREQ				2, 22
#define	SAA7133_A_FMSUB_CONFIG					0x438, 24
#define		SAA7133_A_FMSUB_CFG_FMSUB_BW			0, 1
#define		SAA7133_A_FMSUB_CFG_EIAJ_DELAY			1, 2
#define		SAA7133_A_FMSUB_CFG_NDETCH				3, 1
#define		SAA7133_A_FMSUB_CFG_NDETPB				4, 1
#define		SAA7133_A_FMSUB_CFG_FMSUB_FREQ			8, 16
#define	SAA7133_A_DEMDEC_OUT_CONFIG				0x43C, 24
#define		SAA7133_A_DEMDEC_OUT_CFG_DECSEL			0, 2
#define		SAA7133_A_DEMDEC_OUT_CFG_FMDEMAT		2, 3
#define		SAA7133_A_DEMDEC_OUT_CFG_MONOSEL		5, 1
#define		SAA7133_A_DEMDEC_OUT_CFG_MUTE_DMONO		6, 1
#define		SAA7133_A_DEMDEC_OUT_CFG_MUTE_SAP		7, 1
#define		SAA7133_A_DEMDEC_OUT_CFG_FM_SCALE		10, 2
#define		SAA7133_A_DEMDEC_OUT_CFG_AMLG_SCALE		12, 12
#define	SAA7133_A_MAG_DET						0x440, 23
#define		SAA7133_A_MAG_DET_MPX_PILOT_THR_UP		0, 4
#define		SAA7133_A_MAG_DET_MPX_PILOT_THR_LO		4, 4
#define		SAA7133_A_MAG_DET_SAP_CAR_THR_UP		8, 4
#define		SAA7133_A_MAG_DET_SAP_CAR_THR_LO		12, 4
#define		SAA7133_A_MAG_DET_ASD_SC1_THR			18, 5
#define	SAA7133_A_NOISE_DET_CFG1				0x444, 18
#define		SAA7133_A_NOISE_DET_CFG1_SAP_THR		0, 5
#define		SAA7133_A_NOISE_DET_CFG1_SAP_HYST		5, 4
#define		SAA7133_A_NOISE_DET_CFG1_SC2_THR		9, 5
#define		SAA7133_A_NOISE_DET_CFG1_SC2_HYST		14, 4
#define	SAA7133_A_NOISE_DET_CFG2				0x448, 18
#define		SAA7133_A_NOISE_DET_CFG2_BTSC_THR		0, 5
#define		SAA7133_A_NOISE_DET_CFG2_BTSC_HYST		5, 4
#define		SAA7133_A_NOISE_DET_CFG2_FMRA_THR		9, 5
#define		SAA7133_A_NOISE_DET_CFG2_FMRA_HYST		14, 4
#define	SAA7133_A_NOISE_DET_CFG3				0x44C, 9
#define		SAA7133_A_NOISE_DET_CFG3_EIAJ_THR		0, 5
#define		SAA7133_A_NOISE_DET_CFG3_EIAJ_HYST		5, 4
#define	SAA7133_A_NICAM_CFG						0x450, 21
#define		SAA7133_A_NICAM_CFG_EXTAM				2, 1
#define		SAA7133_A_NICAM_CFG_NDEEM				3, 1
#define		SAA7133_A_NICAM_CFG_AMUTE				4, 1
#define		SAA7133_A_NICAM_CFG_NICLOERRLIM			5, 8
#define		SAA7133_A_NICAM_CFG_NICUPERRLIM			13, 8
#define	SAA7133_A_EASY_PROGRAMMING				0x454, 20
#define		SAA7133_A_EASY_PROGRAMMING_EPMODE		0, 2
#define		SAA7133_A_EASY_PROGRAMMING_STDSEL		2, 5
#define		SAA7133_A_EASY_PROGRAMMING_REST			7, 1
#define		SAA7133_A_EASY_PROGRAMMING_OVMADPT		8, 1
#define		SAA7133_A_EASY_PROGRAMMING_DDMUTE		9, 1
#define		SAA7133_A_EASY_PROGRAMMING_FILTBW		10, 2
#define		SAA7133_A_EASY_PROGRAMMING_IDMOD		12, 2
#define		SAA7133_A_EASY_PROGRAMMING_SAPDBX		16, 1
#define		SAA7133_A_EASY_PROGRAMMING_FHPAL		17, 1
#define		SAA7133_A_EASY_PROGRAMMING_OVMTHR		18, 2
#define	SAA7133_A_INPUT_LVL_ADJUST1				0x458, 20
#define		SAA7133_A_INPUT_LVL_ADJUST1_DECLEV		0, 5
#define		SAA7133_A_INPUT_LVL_ADJUST1_MONOLEV		5, 5
#define		SAA7133_A_INPUT_LVL_ADJUST1_NICLEV		10, 5
#define		SAA7133_A_INPUT_LVL_ADJUST1_SAPLEV		15, 5
#define	SAA7133_A_INPUT_LVL_ADJUST2				0x45C, 5
#define		SAA7133_A_INPUT_LVL_ADJUST2_ADCLEV		0, 5
#define	SAA7133_A_DIGITAL_INPUT_XBAR1			0x464, 16
#define		SAA7133_A_DIGITAL_INPUT_XBAR1_MAINSS	0, 5
#define		SAA7133_A_DIGITAL_INPUT_XBAR1_MAINDM	5, 3
#define		SAA7133_A_DIGITAL_INPUT_XBAR1_DPLSS		8, 5
#define		SAA7133_A_DIGITAL_INPUT_XBAR1_DPLDM		13, 3
#define	SAA7133_A_DIGITAL_INPUT_XBAR2			0x468, 16
#define		SAA7133_A_DIGITAL_INPUT_XBAR2_AUX1SS	0, 5
#define		SAA7133_A_DIGITAL_INPUT_XBAR2_AUS1DM	5, 3
#define		SAA7133_A_DIGITAL_INPUT_XBAR2_AUS2SS	8, 5
#define		SAA7133_A_DIGITAL_INPUT_XBAR2_AUS2DM	13, 3
#define	SAA7133_A_DIGITAL_OUTPUT_SEL1			0x46C, 24
#define		SAA7133_A_DIGITAL_OUTPUT_SEL1_ASDMA1	0, 4
#define		SAA7133_A_DIGITAL_OUTPUT_SEL1_ASDMA2	4, 4
#define		SAA7133_A_DIGITAL_OUTPUT_SEL1_ASDMA3	8, 4
#define		SAA7133_A_DIGITAL_OUTPUT_SEL1_ASDMA4	12, 4
#define		SAA7133_A_DIGITAL_OUTPUT_SEL1_ASDMA5	16, 4
#define		SAA7133_A_DIGITAL_OUTPUT_SEL1_ASDMA6	20, 4
#define	SAA7133_A_DIGITAL_OUTPUT_SEL2			0x470, 24
#define		SAA7133_A_DIGITAL_OUTPUT_SEL2_ASDACL	0, 4
#define		SAA7133_A_DIGITAL_OUTPUT_SEL2_ASDACR	4, 4
#define		SAA7133_A_DIGITAL_OUTPUT_SEL2_ASI2S1L	8, 4
#define		SAA7133_A_DIGITAL_OUTPUT_SEL2_ASI2S1R	12, 4
#define		SAA7133_A_DIGITAL_OUTPUT_SEL2_ASI2S2L	16, 4
#define		SAA7133_A_DIGITAL_OUTPUT_SEL2_ASI2S2R	20, 4
#define	SAA7133_A_SOFTMUTE_CTRL					0x474, 5
#define		SAA7133_A_SOFTMUTE_CTRL_MAINMUT			0, 1
#define		SAA7133_A_SOFTMUTE_CTRL_MAINLMUT		1, 1
#define		SAA7133_A_SOFTMUTE_CTRL_MAINRMUT		2, 1
#define		SAA7133_A_SOFTMUTE_CTRL_AUX1MUT			3, 1
#define		SAA7133_A_SOFTMUTE_CTRL_AUX2MUT			4, 1
#define	SAA7133_A_SND_FEATURE_CTRL				0x478, 15
#define		SAA7133_A_SND_FEATURE_CTRL_SOMOCTRL		0, 2
#define		SAA7133_A_SND_FEATURE_CTRL_DPLCTRL		2, 1
#define		SAA7133_A_SND_FEATURE_CTRL_SRCCTRL		3, 1
#define		SAA7133_A_SND_FEATURE_CTRL_INSOEF		4, 3
#define		SAA7133_A_SND_FEATURE_CTRL_AVLMOD		7, 3
#define		SAA7133_A_SND_FEATURE_CTRL_ALVWEIGHT	10, 1
#define		SAA7133_A_SND_FEATURE_CTRL_AVLLEV		11, 4
#define	SAA7133_A_DOLBY_PROLOGIC_CTRL			0x47C, 14
#define		SAA7133_A_DOLBY_PROLOGIC_CTRL_VDSMIXLEV	0, 3
#define		SAA7133_A_DOLBY_PROLOGIC_CTRL_NOISEON	8, 1
#define		SAA7133_A_DOLBY_PROLOGIC_CTRL_X3ST		9, 1
#define		SAA7133_A_DOLBY_PROLOGIC_CTRL_PH_CTR	10, 1
#define		SAA7133_A_DOLBY_PROLOGIC_CTRL_FLAT_F	11, 1
#define		SAA7133_A_DOLBY_PROLOGIC_CTRL_B_TYPE	12, 1
#define		SAA7133_A_DOLBY_PROLOGIC_CTRL_ABALCFG	13, 1
#define	SAA7133_A_VOLUME_CTRL1					0x480, 16
#define		SAA7133_A_VOLUME_CTRL1_MAINVOLL			0, 8
#define		SAA7133_A_VOLUME_CTRL1_MAINVOLR			8, 8
#define	SAA7133_A_VOLUME_CTRL2					0x484, 16
#define		SAA7133_A_VOLUME_CTRL2_AUX1VOL			0, 8
#define		SAA7133_A_VOLUME_CTRL2_AUX2VOL			8, 8
#define	SAA7133_A_MAIN_TONE_CTRL				0x488, 10
#define		SAA7133_A_MAIN_TONE_CTRL_MAINBASS		0, 5
#define		SAA7133_A_MAIN_TONE_CTRL_MAINTREB		5, 5
#define	SAA7133_A_MONITOR_CTRL					0x48C, 10
#define		SAA7133_A_MONITOR_CTRL_MONISS			0, 5
#define		SAA7133_A_MONITOR_CTRL_MON_DET			5, 2
#define		SAA7133_A_MONITOR_CTRL_MON_MAT			7, 3
#define	SAA7133_A_EXPERT_MODE1					0x4A4, 1
#define		SAA7133_A_EXPERT_MODE1_EXPON
#define	SAA7133_A_EXPERT_MODE2					0x4A8, 24
#define		SAA7133_A_EXPERT_MODE2_EXPREG1			0, 24
#define	SAA7133_A_EXPERT_MODE3					0x4AC, 24
#define		SAA7133_A_EXPERT_MODE3_EXPREG2			0, 24
#define	SAA7133_A_EXPERT_MODE4					0x4B0, 24
#define		SAA7133_A_EXPERT_MODE4_EXPREG3			0, 24
#define	SAA7133_A_EXPERT_MODE5					0x4B4, 24
#define		SAA7133_A_EXPERT_MODE5_EXPREG4			0, 24
#define	SAA7133_A_EXPERT_MODE6					0x4B8, 24
#define		SAA7133_A_EXPERT_MODE6_EXPREG5			0, 24

#endif
