//
// $Id: GenericTuner.cpp,v 1.18 2005-03-06 12:50:50 to_see Exp $
//
/////////////////////////////////////////////////////////////////////////////
//
// copyleft 2001 itt@myself.com
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: not supported by cvs2svn $
// Revision 1.17  2004/12/29 20:17:19  to_see
// Added TCL 2002N tuner (from v4l2)
//
// Revision 1.16  2004/08/27 13:11:27  to_see
// Added Philips 4in1 tuner (from v4l2)
//
// Revision 1.15  2004/06/30 17:39:00  to_see
// Added some fixes for Philips MK3 tuner & TDA9887
//
// Revision 1.14  2004/01/29 15:19:49  adcockj
// Generic tuner radio patch from Sven Grothklags
//
// Revision 1.13  2003/10/27 10:39:51  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.12  2003/02/06 21:28:33  ittarnavsky
// added the Philips M-J tuner for primetv 7133
//
// Revision 1.11  2002/10/08 20:43:16  kooiman
// Added Automatic Frequency Control for tuners. Changed to Hz instead of multiple of 62500 Hz.
//
// Revision 1.10  2002/09/04 11:58:45  kooiman
// Added new tuners & fix for new Pinnacle cards with MT2032 tuner.
//
// Revision 1.9  2002/08/20 09:48:43  kooiman
// Fixed wrong byte swapping for Philips SECAM tuner. Prevented crash when m_I2CBus is NULL.
//
// Revision 1.8  2002/08/17 11:27:23  kooiman
// Fixed tuning for Temic 4046FM5 and Philips FQ1216ME.
//
// Revision 1.7  2002/08/03 17:57:52  kooiman
// Added new cards & tuners. Changed the tuner combobox into a sorted list.
//
// Revision 1.6  2002/01/16 19:16:20  adcockj
// added support for LG NTSC (TAPCH701P)
//
// Revision 1.5  2001/12/18 23:36:01  adcockj
// Split up the MSP chip support into two parts to avoid probelms when deleting objects
//
// Revision 1.4  2001/12/05 21:45:11  ittarnavsky
// added changes for the AudioDecoder and AudioControls support
//
// Revision 1.3  2001/11/29 17:30:52  adcockj
// Reorgainised bt848 initilization
// More Javadoc-ing
//
// Revision 1.2  2001/11/26 13:02:27  adcockj
// Bug Fixes and standards changes
//
// Revision 1.1  2001/11/25 02:03:21  ittarnavsky
// initial checkin of the new I2C code
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file GenericTuner.cpp CGenericTuner Implementation
 */

#include "stdafx.h"
#include "GenericTuner.h"
#include "DebugLog.h"


#define I2CTUNER_POR       0x80
#define I2CTUNER_FL        0x40
#define I2CTUNER_MODE      0x38
#define I2CTUNER_AFC       0x07

#define I2CTUNER_STEREO    0x10 // radio mode
#define I2CTUNER_SIGNAL    0x07 // radio mode

/* tv standard selection for Temic 4046 FM5
   this value takes the low bits of control byte 2
   from datasheet Rev.01, Feb.00 
     standard     BG      I       L       L2      D
     picture IF   38.9    38.9    38.9    33.95   38.9
     sound 1      33.4    32.9    32.4    40.45   32.4
     sound 2      33.16   
     NICAM        33.05   32.348  33.05           33.05
 */

#define TEMIC_SET_PAL_I         0x05
#define TEMIC_SET_PAL_DK        0x09
#define TEMIC_SET_PAL_L         0x0a // SECAM ?
#define TEMIC_SET_PAL_L2        0x0b // change IF !
#define TEMIC_SET_PAL_BG        0x0c

/* tv tuner system standard selection for Philips FQ1216ME
   this value takes the low bits of control byte 2
   from datasheet "1999 Nov 16" (supersedes "1999 Mar 23")
     standard 		BG	DK	I	L	L`
     picture carrier	38.90	38.90	38.90	38.90	33.95
     colour		34.47	34.47	34.47	34.47	38.38
     sound 1		33.40	32.40	32.90	32.40	40.45
     sound 2		33.16	-	-	-	-
     NICAM		33.05	33.05	32.35	33.05	39.80
 */
#define PHILIPS_SET_PAL_I		0x01 /* Bit 2 always zero !*/
#define PHILIPS_SET_PAL_BGDK	0x09
#define PHILIPS_SET_PAL_L2		0x0a
#define PHILIPS_SET_PAL_L		0x0b	

/* system switching for Philips FI1216MF MK2
   from datasheet "1996 Jul 09",
 */
#define PHILIPS_MF_SET_BG		0x01 /* Bit 2 must be zero, Bit 3 is system output */
#define PHILIPS_MF_SET_PAL_L	0x03
#define PHILIPS_MF_SET_PAL_L2	0x02



/// \todo not at all OO
#define TUNERDEF(TID,VFMT,T1,T2,VHFL,VHFH,UHF,CFG,IFPC) \
        m_TunerId = (TID); \
        m_VideoFormat = (VFMT); \
        m_Thresh1 = (T1); \
        m_Thresh2 = (T2); \
        m_VHF_L = (VHFL); \
        m_VHF_H = (VHFH); \
        m_UHF = (UHF); \
        m_Config = (CFG); \
        m_IFPCoff = (IFPC)

CGenericTuner::CGenericTuner(eTunerId tunerId) :
  m_Frequency(0)
{
    switch (tunerId)
    {
    default:
        {
            TUNERDEF(TUNER_ABSENT, VIDEOFORMAT_NTSC_M,
                0,0,0,0,0,0,0);
            break;
        }
    case TUNER_PHILIPS_PAL_I:
        { 
            TUNERDEF(TUNER_PHILIPS_PAL_I, VIDEOFORMAT_PAL_I, 
                16*140.25, 16*463.25, 0xa0, 0x90, 0x30, 0x8e, 623);
            break;
        }
    case TUNER_PHILIPS_NTSC:
        { 
            TUNERDEF(TUNER_PHILIPS_NTSC, VIDEOFORMAT_NTSC_M, 
                16*157.25, 16*451.25, 0xA0, 0x90, 0x30, 0x8e, 732);
            break;
        }
    case TUNER_PHILIPS_SECAM:
        { 
            TUNERDEF(TUNER_PHILIPS_SECAM, VIDEOFORMAT_SECAM_D, 
                16*168.25, 16*447.25, 0xA7, 0x97, 0x37, 0x8e, 623);
            break;
        }
    case TUNER_PHILIPS_PAL:
        { 
            TUNERDEF(TUNER_PHILIPS_PAL, VIDEOFORMAT_PAL_B, 
                16*168.25, 16*447.25, 0xA0, 0x90, 0x30, 0x8e, 623);
            break;
        }
    case TUNER_TEMIC_4002FH5_PAL:
        { 
            TUNERDEF(TUNER_TEMIC_4002FH5_PAL, VIDEOFORMAT_PAL_B,
                16*140.25, 16*463.25, 0x02, 0x04, 0x01, 0x8e, 623);
            break;
        }
    case TUNER_TEMIC_4032FY5_NTSC:
        {
            TUNERDEF(TUNER_TEMIC_4032FY5_NTSC, VIDEOFORMAT_NTSC_M, 
                16*157.25, 16*463.25, 0x02, 0x04, 0x01, 0x8e, 732);
            break;
        }
    case TUNER_TEMIC_4062FY5_PAL_I:
        {
            TUNERDEF(TUNER_TEMIC_4062FY5_PAL_I, VIDEOFORMAT_PAL_I, 
                16*170.00, 16*450.00, 0x02, 0x04, 0x01, 0x8e, 623);
            break;
        }
    case TUNER_TEMIC_4036FY5_NTSC:
        {
            TUNERDEF(TUNER_TEMIC_4036FY5_NTSC, VIDEOFORMAT_NTSC_M, 
                16*157.25, 16*463.25, 0xa0, 0x90, 0x30, 0x8e, 732);
            break;
        }
    case TUNER_ALPS_TSBH1_NTSC:
        {
            TUNERDEF(TUNER_ALPS_TSBH1_NTSC, VIDEOFORMAT_NTSC_M, 
                16*137.25, 16*385.25, 0x01, 0x02, 0x08, 0x8e, 732);
            break;
        }
    case TUNER_ALPS_TSBE1_PAL:
        {
            TUNERDEF(TUNER_ALPS_TSBE1_PAL, VIDEOFORMAT_PAL_B, 
                16*137.25, 16*385.25, 0x01, 0x02, 0x08, 0x8e, 732);
            break;
        }
    case TUNER_ALPS_TSBB5_PAL_I:
        {
            TUNERDEF(TUNER_ALPS_TSBB5_PAL_I, VIDEOFORMAT_PAL_I, 
                16*133.25, 16*351.25, 0x01, 0x02, 0x08, 0x8e, 632);
            break;
        }
    case TUNER_ALPS_TSBE5_PAL:
        {
            TUNERDEF(TUNER_ALPS_TSBE5_PAL, VIDEOFORMAT_PAL_B, 
                16*133.25, 16*351.25, 0x01, 0x02, 0x08, 0x8e, 622);
            break;
        }
    case TUNER_ALPS_TSBC5_PAL:
        {
            TUNERDEF(TUNER_ALPS_TSBC5_PAL, VIDEOFORMAT_PAL_B, 
                16*133.25, 16*351.25, 0x01, 0x02, 0x08, 0x8e, 608);
            break;
        }
    case TUNER_TEMIC_4006FH5_PAL:
        {
            TUNERDEF(TUNER_TEMIC_4006FH5_PAL, VIDEOFORMAT_PAL_B, 
                16*170.00,16*450.00, 0xa0, 0x90, 0x30, 0x8e, 623);
            break;
        }
    case TUNER_PHILIPS_1236D_NTSC_INPUT1:
        {
            TUNERDEF(TUNER_PHILIPS_1236D_NTSC_INPUT1, VIDEOFORMAT_NTSC_M, 
                2516, 7220, 0xA3, 0x93, 0x33, 0xCE, 732);
            break;
        }
    case TUNER_PHILIPS_1236D_NTSC_INPUT2:
        {
            TUNERDEF(TUNER_PHILIPS_1236D_NTSC_INPUT2, VIDEOFORMAT_NTSC_M, 
                2516, 7220, 0xA2, 0x92, 0x32, 0xCE, 732);
            break;
        }
    case TUNER_ALPS_TSCH6_NTSC:
        {
            TUNERDEF(TUNER_ALPS_TSCH6_NTSC, VIDEOFORMAT_NTSC_M,
                16*137.25, 16*385.25, 0x14, 0x12, 0x11, 0x8e, 732);
            break;
        }
    case TUNER_TEMIC_4016FY5_PAL:
        {
            TUNERDEF(TUNER_TEMIC_4016FY5_PAL, VIDEOFORMAT_PAL_B,
                16*136.25, 16*456.25, 0xa0, 0x90, 0x30, 0x8e, 623);
            break;
        }
    case TUNER_PHILIPS_MK2_NTSC:
        {
            TUNERDEF(TUNER_PHILIPS_MK2_NTSC, VIDEOFORMAT_NTSC_M,
                16*160.00,16*454.00,0xa0,0x90,0x30,0x8e,732);
            break;
        }
    case TUNER_TEMIC_4066FY5_PAL_I:
        {
            TUNERDEF(TUNER_TEMIC_4066FY5_PAL_I, VIDEOFORMAT_PAL_I,
                16*169.00, 16*454.00, 0xa0,0x90,0x30,0x8e,623);
            break;
        }
    case TUNER_TEMIC_4006FN5_PAL:
        {
            TUNERDEF(TUNER_TEMIC_4006FN5_PAL, VIDEOFORMAT_PAL_B,
                16*169.00, 16*454.00, 0xa0,0x90,0x30,0x8e,623);
            break;
        }
    case TUNER_TEMIC_4009FR5_PAL:
        { 
            TUNERDEF(TUNER_TEMIC_4009FR5_PAL, VIDEOFORMAT_PAL_B,
                16*141.00, 16*464.00, 0xa0,0x90,0x30,0x8e,623);
            break;
        }
    case TUNER_TEMIC_4039FR5_NTSC:
        {
            TUNERDEF(TUNER_TEMIC_4039FR5_NTSC, VIDEOFORMAT_NTSC_M,
                16*158.00, 16*453.00, 0xa0,0x90,0x30,0x8e,732);
            break;
        }
    case TUNER_TEMIC_4046FM5_MULTI:
        { 
            TUNERDEF(TUNER_TEMIC_4046FM5_MULTI, VIDEOFORMAT_PAL_B,
                16*169.00, 16*454.00, 0xa0,0x90,0x30,0x8e,623);
            break;
        }
    case TUNER_PHILIPS_PAL_DK:
        { 
            TUNERDEF(TUNER_PHILIPS_PAL_DK, VIDEOFORMAT_PAL_D,
                16*170.00, 16*450.00, 0xa0, 0x90, 0x30, 0x8e, 623);
            break;
        }
    case TUNER_PHILIPS_MULTI:
        { 
            TUNERDEF(TUNER_PHILIPS_MULTI, VIDEOFORMAT_PAL_B,
                16*170.00,16*450.00,0xa0,0x90,0x30,0x8e,623);
            break;
        }
    case TUNER_LG_I001D_PAL_I:
        { 
            TUNERDEF(TUNER_LG_I001D_PAL_I, VIDEOFORMAT_PAL_I,
                16*170.00,16*450.00,0xa0,0x90,0x30,0x8e,623);
            break;
        }
    case TUNER_LG_I701D_PAL_I:
        { 
            TUNERDEF(TUNER_LG_I701D_PAL_I, VIDEOFORMAT_PAL_I,
                16*170.00,16*450.00,0xa0,0x90,0x30,0x8e,623);
            break;
        }
    case TUNER_LG_R01F_NTSC:
        { 
            TUNERDEF(TUNER_LG_R01F_NTSC, VIDEOFORMAT_NTSC_M,
                16*210.00,16*497.00,0xa0,0x90,0x30,0x8e,732);
            break;
        }
    case TUNER_LG_B01D_PAL:
        { 
            TUNERDEF(TUNER_LG_B01D_PAL, VIDEOFORMAT_PAL_B,
                16*170.00,16*450.00,0xa0,0x90,0x30,0x8e,623);
            break;
        }
    case TUNER_LG_B11D_PAL:
        { 
            TUNERDEF(TUNER_LG_B11D_PAL, VIDEOFORMAT_PAL_B,
                16*170.00,16*450.00,0xa0,0x90,0x30,0x8e,623);
            break;
        }
    case TUNER_TEMIC_4009FN5_PAL:
        { 
            TUNERDEF(TUNER_TEMIC_4009FN5_PAL, VIDEOFORMAT_PAL_B,
                16*141.00, 16*464.00, 0xa0,0x90,0x30,0x8e,623);
            break;
        }
    case TUNER_SHARP_2U5JF5540_NTSC:
        {
            TUNERDEF(TUNER_SHARP_2U5JF5540_NTSC, VIDEOFORMAT_NTSC_M_Japan,
                16*137.25, 16*317.25, 0x01, 0x02, 0x08, 0x8e, 940);
            break;
        }
    case TUNER_LG_TAPCH701P_NTSC:
        {
            TUNERDEF(TUNER_LG_TAPCH701P_NTSC, VIDEOFORMAT_NTSC_M, 
                16*165.00, 16*450.00, 0x01, 0x02, 0x08, 0x8e, 732);
            break;
        }
    case TUNER_SAMSUNG_PAL_TCPM9091PD27:
      {
          TUNERDEF(TUNER_SAMSUNG_PAL_TCPM9091PD27, VIDEOFORMAT_PAL_I,
              16*(169),16*(464),0xA0,0x90,0x30,0x8e,623);
          break;
      }
    case TUNER_TEMIC_4106FH5:
      {
          TUNERDEF(TUNER_TEMIC_4106FH5, VIDEOFORMAT_PAL_B,
              16*(141.00),16*(464.00),0xa0,0x90,0x30,0x8e,623);
          break;
      }
    case TUNER_TEMIC_4012FY5:
      {
          TUNERDEF(TUNER_TEMIC_4012FY5, VIDEOFORMAT_SECAM_D,
              16*(140.25),16*(460.00),0x02,0x04,0x01,0x8e,608);
          break;
      }
    case TUNER_TEMIC_4136FY5:
      {
          TUNERDEF(TUNER_TEMIC_4136FY5, VIDEOFORMAT_NTSC_M,
              16*(158.00),16*(453.00),0xa0,0x90,0x30,0x8e,732);
          break;
      }
    case TUNER_LG_TAPCNEW_PAL:
      {
          TUNERDEF(TUNER_LG_TAPCNEW_PAL, VIDEOFORMAT_PAL_B,
              16*(170.00),16*(450.00),0x01,0x02,0x08,0x8e,623);
          break;
      }
    case TUNER_PHILIPS_FM1216ME_MK3:
      {
          TUNERDEF(TUNER_PHILIPS_FM1216ME_MK3, VIDEOFORMAT_PAL_B,
              16*(160.00),16*(442.00),0x01,0x02,0x04,0x8e,623);
          break;
      }
    case TUNER_LG_TAPCNEW_NTSC:
      {
          TUNERDEF(TUNER_LG_TAPCNEW_NTSC, VIDEOFORMAT_NTSC_M,
              16*(170.00),16*(450.00),0x01,0x02,0x08,0x8e,732);
          break;
      }
    case TUNER_PHILIPS_FI1286_NTSC_M_J:
      {
          TUNERDEF(TUNER_PHILIPS_FI1286_NTSC_M_J, VIDEOFORMAT_NTSC_M,
              16*(160.00),16*(454.00),0x01,0x02,0x04,0x8e,940);
          break;
      }
    case TUNER_PHILIPS_4IN1:
      {
          TUNERDEF(TUNER_PHILIPS_4IN1, VIDEOFORMAT_NTSC_M,
              16*(160.00),16*(442.00),0x01,0x02,0x04,0x8e,732);
          break;
      }
    case TUNER_TCL_2002N:
      {
          TUNERDEF(TUNER_TCL_2002N, VIDEOFORMAT_NTSC_M,
              16*(172.00),16*(448.00),0x01,0x02,0x08,0x8e,732);
          break;
      }
    case TUNER_HITACHI_NTSC:
      {
          TUNERDEF(TUNER_HITACHI_NTSC, VIDEOFORMAT_NTSC_M,
              16*(170.00),16*(450.00),0x01,0x02,0x08,0x8e,940);
          break;
      }
    case TUNER_PHILIPS_PAL_MK:
      {
          TUNERDEF(TUNER_PHILIPS_PAL_MK, VIDEOFORMAT_PAL_B,
              16*(140.00),16*(463.25),0x01,0xc2,0xcf,0x8e,623);
          break;
      }
    case TUNER_PHILIPS_FM1236_MK3:
      {
          TUNERDEF(TUNER_PHILIPS_FM1236_MK3, VIDEOFORMAT_NTSC_M,
              16*(160.00),16*(442.00),0x01,0x02,0x04,0x8e,732);
          break;
      }
    case TUNER_LG_NTSC_TAPE:
      {
          TUNERDEF(TUNER_LG_NTSC_TAPE, VIDEOFORMAT_NTSC_M,
              16*(160.00),16*(442.00),0x01,0x02,0x04,0x8e,732);
          break;
      }
    case TUNER_TNF_8831BGFF:
      {
          TUNERDEF(TUNER_TNF_8831BGFF, VIDEOFORMAT_PAL_B,
              16*(161.25),16*(463.25),0xa0,0x90,0x30,0x8e,623);
          break;
      }
    case TUNER_PHILIPS_FM1256_IH3:
      {
          TUNERDEF(TUNER_PHILIPS_FM1256_IH3, VIDEOFORMAT_PAL_B,
              16*(160.00),16*(442.00),0x01,0x02,0x04,0x8e,623);
          break;
      }
    case TUNER_PHILIPS_FQ1286:
      {
          TUNERDEF(TUNER_PHILIPS_FQ1286, VIDEOFORMAT_NTSC_M,
              16*(160.00),16*(454.00),0x41,0x42,0x04,0x8e,940);// UHF band untested
          break;
      }
    case TUNER_LG_PAL_TAPE:
      {
          TUNERDEF(TUNER_LG_PAL_TAPE, VIDEOFORMAT_PAL_B,
              16*(170.00),16*(450.00),0x01,0x02,0x08,0xce,623);
          break;
      }
    case TUNER_PHILIPS_FM1216ME:
      {
          TUNERDEF(TUNER_PHILIPS_FM1216ME, VIDEOFORMAT_PAL_B,
              16*(160.00),16*(442.00),0x01,0x02,0x04,0xce,623);
          break;
      }
    case TUNER_PHILIPS_FQ1236A_MK4:
      {
          TUNERDEF(TUNER_PHILIPS_FQ1236A_MK4, VIDEOFORMAT_PAL_B,
              16*(160.00),16*(442.00),0x01,0x02,0x04,0x8e,732);
          break;
      }
    }
}

bool CGenericTuner::SetTVFrequency(long nFrequencyHz, eVideoFormat videoFormat)
{
    BYTE config;
    WORD div;
    static long m_LastFrequency = 0;

    long nFrequency = MulDiv(nFrequencyHz, 16, 1000000);

    if (nFrequency < m_Thresh1)
    {
        config = m_VHF_L;
    }
    else if (nFrequency < m_Thresh2)
    {
        config = m_VHF_H;
    }
    else
    {
        config = m_UHF;
    }
    
    div = nFrequency + m_IFPCoff;
    
    // handle Mode on Philips SECAM tuners
    // they can also recive PAL if the Mode is set properly
    if (m_TunerId == TUNER_PHILIPS_SECAM)
    {
        if (IsSECAMVideoFormat(videoFormat))
        {
            config |= 0x02;
        }
        else
        {
            config &= ~0x02;
        }
    }
    else if (m_TunerId == TUNER_TEMIC_4046FM5_MULTI)
    {
        config &= ~0x0f;
        switch (videoFormat)
        {
            case VIDEOFORMAT_PAL_I:
                config |= TEMIC_SET_PAL_I;
                break;
            case VIDEOFORMAT_PAL_D:
                config |= TEMIC_SET_PAL_DK;
                break;
            case VIDEOFORMAT_SECAM_L:
                config |= TEMIC_SET_PAL_L;
                break;
            case VIDEOFORMAT_PAL_B:
            case VIDEOFORMAT_PAL_G:
                config |= TEMIC_SET_PAL_BG;
                break;
        }
    }
    else if (m_TunerId == TUNER_PHILIPS_MULTI) //FQ1216ME
    {
        config &= ~0x0f;
        switch (videoFormat)
        {
            case VIDEOFORMAT_PAL_I:
                config |= PHILIPS_SET_PAL_I;
                break;
            case VIDEOFORMAT_SECAM_L:
                config |= PHILIPS_SET_PAL_L;
                break;
            case VIDEOFORMAT_PAL_B:
            case VIDEOFORMAT_PAL_G:
            case VIDEOFORMAT_PAL_D:
            case VIDEOFORMAT_SECAM_K:
                config |= PHILIPS_SET_PAL_BGDK;
                break;
        }
    }

    //if (FastTune) {
		//    config |= 0x40;
	  //}
    
    div &= 0x7fff;

    

    BYTE buffer[] = {(BYTE) m_DeviceAddress << 1, (BYTE) ((div >> 8) & 0x7f), (BYTE) (div & 0xff), m_Config, config};

    if ((m_TunerId == TUNER_PHILIPS_SECAM) && (nFrequencyHz < m_Frequency))
    {
        //specification says to send config data before frequency in case (wanted frequency < current frequency).

        //swap order
        BYTE temp1 = buffer[1];
        BYTE temp2 = buffer[2];
        buffer[1] = buffer[3];
        buffer[2] = buffer[4];
        buffer[3] = temp1;
        buffer[4] = temp2;
    }
    m_Frequency = nFrequencyHz;

    if (m_I2CBus != NULL)
    {
		if (m_ExternalIFDemodulator != NULL)
		{
			m_ExternalIFDemodulator->TunerSet(TRUE, videoFormat);
		}

        bool result = m_I2CBus->Write(buffer, sizeof(buffer));

		if (m_ExternalIFDemodulator != NULL)
		{
			m_ExternalIFDemodulator->TunerSet(FALSE, videoFormat);
		}
        
		return result;
    }
    else
    {
        LOG(1,"GenericTuner: Error setting frequency. No I2C bus.");
        return false;
    }
}

eTunerId CGenericTuner::GetTunerId()
{
    return m_TunerId;
}

eVideoFormat CGenericTuner::GetDefaultVideoFormat()
{
    return m_VideoFormat;
}

bool CGenericTuner::HasRadio() const
{
    return true;
}


bool CGenericTuner::SetRadioFrequency(long nFrequencyHz)
{
    // patch from bttv thanks to Sven Grothklags
    if(nFrequencyHz < 87500000 || nFrequencyHz > 108100000)
        return false;

#if 1
    long nFrequency = MulDiv(nFrequencyHz, 20, 1000000);
    WORD div = nFrequency + (WORD)(10.7*20);
    BYTE config = 0x88;
#else
    long nFrequency = MulDiv(nFrequencyHz, 16, 1000000);
    WORD div = nFrequency + (WORD)(10.7*16);
    BYTE config = m_Config;
#endif

    BYTE buffer[] = {(BYTE) m_DeviceAddress << 1, (BYTE) ((div >> 8) & 0x7f), (BYTE) (div & 0xff), config, 0xa4};

    m_Frequency = nFrequencyHz;

    if (m_I2CBus != NULL)
    {
		// not working but needed for FM Radio
		/*
		if (m_ExternalIFDemodulator != NULL)
		{
			m_ExternalIFDemodulator->TunerSet(TRUE, VIDEOFORMAT_LASTONE+1);
		}
		*/

        bool result = m_I2CBus->Write(buffer, sizeof(buffer));

		/*
		if (m_ExternalIFDemodulator != NULL)
		{
			m_ExternalIFDemodulator->TunerSet(FALSE, VIDEOFORMAT_LASTONE+1);
		}
        */

		return result;
    }
    else
    {
        LOG(1,"GenericTuner: Error setting frequency. No I2C bus.");
        return false;
    }
}

BYTE CGenericTuner::GetDefaultAddress()const
{
    return 0xC0>>1;
}

long CGenericTuner::GetFrequency()
{
    return m_Frequency;
}

eTunerLocked CGenericTuner::IsLocked()
{
    if (m_I2CBus != NULL)
    {
        BYTE addr;
        BYTE result;
        addr = (BYTE)(m_DeviceAddress << 1);
        if (m_I2CBus->Read(&addr, 1, &result, 1))
        {
            if ((result & I2CTUNER_FL) != 0)
            {
                return TUNER_LOCK_ON;
            }
            else
            {
                return TUNER_LOCK_OFF;
            }
        }
    }
    return TUNER_LOCK_NOTSUPPORTED;
}

eTunerAFCStatus CGenericTuner::GetAFCStatus(long &nFreqDeviation)
{
    if (m_I2CBus != NULL)
    {
        if (m_ExternalIFDemodulator != NULL)
		{
			eTunerAFCStatus AFCStatus = m_ExternalIFDemodulator->GetAFCStatus(nFreqDeviation);
			return AFCStatus;
		}

		else
		{
			BYTE addr;
			BYTE result;
			addr = (BYTE)(m_DeviceAddress << 1);
			if (m_I2CBus->Read(&addr, 1, &result, 1))
			{
				int afc = (result&I2CTUNER_AFC);

				afc -= 2;
				switch (afc)
				{
				case -2:
					nFreqDeviation = -125000;
					break;
				case -1:
					nFreqDeviation = -62500;
					break;
				case 0:
					nFreqDeviation = 0;
					break;
				case 1:
					nFreqDeviation = 62500;
					break;
				//case 2:
				//    nFreqDeviation = 125000;
				//   break;
				default:
					return TUNER_AFC_NOCARRIER;
				}
				return TUNER_AFC_CARRIER;
			}
		}
    }
    return TUNER_AFC_NOTSUPPORTED;
}

