//
// $Id$
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
   from datasheet _T("1999 Nov 16") (supersedes _T("1999 Mar 23"))
     standard         BG    DK    I    L    L`
     picture carrier    38.90    38.90    38.90    38.90    33.95
     colour        34.47    34.47    34.47    34.47    38.38
     sound 1        33.40    32.40    32.90    32.40    40.45
     sound 2        33.16    -    -    -    -
     NICAM        33.05    33.05    32.35    33.05    39.80
 */
#define PHILIPS_SET_PAL_I        0x01 /* Bit 2 always zero !*/
#define PHILIPS_SET_PAL_BGDK    0x09
#define PHILIPS_SET_PAL_L2        0x0a
#define PHILIPS_SET_PAL_L        0x0b

/* system switching for Philips FI1216MF MK2
   from datasheet _T("1996 Jul 09"),
 */
#define PHILIPS_MF_SET_BG        0x01 /* Bit 2 must be zero, Bit 3 is system output */
#define PHILIPS_MF_SET_PAL_L    0x03
#define PHILIPS_MF_SET_PAL_L2    0x02



/// \todo not at all OO
#define TUNERDEF(TID,VFMT,T1,T2,VHFL,VHFH,UHF,CFG,IFPC) \
        m_TunerId = (TID); \
        m_VideoFormat = (VFMT); \
        m_Thresh1 = (WORD)(T1); \
        m_Thresh2 = (WORD)(T2); \
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
    case TUNER_PHILIPS_FQ1216AME_MK4:
      {
          TUNERDEF(TUNER_PHILIPS_FQ1216AME_MK4, VIDEOFORMAT_PAL_B,
              16*(160.00),16*(442.00),0x01,0x02,0x04,0xce,623);
          break;
      }
    case TUNER_PHILIPS_FQ1236A_MK4:
      {
          TUNERDEF(TUNER_PHILIPS_FQ1236A_MK4, VIDEOFORMAT_PAL_B,
              16*(160.00),16*(442.00),0x01,0x02,0x04,0x8e,732);
          break;
      }
    case TUNER_YMEC_TVF_8531MF:
      {
          TUNERDEF(TUNER_YMEC_TVF_8531MF, VIDEOFORMAT_NTSC_M,
              16*(160.00),16*(454.00),0xa0,0x90,0x30,0x8e,732);
          break;
      }
    case TUNER_YMEC_TVF_5533MF:
      {
          TUNERDEF(TUNER_YMEC_TVF_5533MF, VIDEOFORMAT_NTSC_M,
              16*(160.00),16*(454.00),0x01,0x02,0x04,0x8e,732);
          break;
      }
    case TUNER_TENA_9533_DI:
      {
          TUNERDEF(TUNER_TENA_9533_DI, VIDEOFORMAT_PAL_B,
              16*(160.25),16*(464.25),0x01,0x02,0x04,0x8e,623);
          break;
      }
    case TUNER_PHILIPS_FMD1216ME_MK3:
      {
          TUNERDEF(TUNER_PHILIPS_FMD1216ME_MK3, VIDEOFORMAT_PAL_B,
              16*(160.00),16*(442.00),0x51,0x52,0x54,0x86,623);
          break;
      }
    }
}

BOOL CGenericTuner::SetTVFrequency(long nFrequencyHz, eVideoFormat videoFormat)
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

    div = (WORD)nFrequency + m_IFPCoff;

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
        if (m_ExternalIFDemodulator)
        {
            m_ExternalIFDemodulator->TunerSet(TRUE, videoFormat);
        }

        BOOL result = m_I2CBus->Write(buffer, sizeof(buffer));

        if (m_ExternalIFDemodulator)
        {
            m_ExternalIFDemodulator->TunerSet(FALSE, videoFormat);
        }

        return result;
    }
    else
    {
        LOG(1,_T("GenericTuner: Error setting frequency. No I2C bus."));
        return FALSE;
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

BOOL CGenericTuner::HasRadio() const
{
    return TRUE;
}


BOOL CGenericTuner::SetRadioFrequency(long nFrequencyHz)
{
    // patch from bttv thanks to Sven Grothklags
    if(nFrequencyHz < 87500000 || nFrequencyHz > 108100000)
        return FALSE;

#if 1
    long nFrequency = MulDiv(nFrequencyHz, 20, 1000000);
    WORD div = (WORD)nFrequency + (WORD)(10.7*20);
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
        if (m_ExternalIFDemodulator)
        {
            m_ExternalIFDemodulator->TunerSet(TRUE, VIDEOFORMAT_LASTONE+1);
        }
        */

        BOOL result = m_I2CBus->Write(buffer, sizeof(buffer));

        /*
        if (m_ExternalIFDemodulator)
        {
            m_ExternalIFDemodulator->TunerSet(FALSE, VIDEOFORMAT_LASTONE+1);
        }
        */

        return result;
    }
    else
    {
        LOG(1,_T("GenericTuner: Error setting frequency. No I2C bus."));
        return FALSE;
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
        if (m_ExternalIFDemodulator)
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

