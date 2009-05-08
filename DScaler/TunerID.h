//
// $Id$
//
/////////////////////////////////////////////////////////////////////////////
//
// copyleft 2003 itt@myself.com
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
 * @file tunerid.h tunerid Header
 */

#if !defined(__TUNERID_H__)
#define __TUNERID_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
    TUNER_SAMSUNG_PAL_TCPM9091PD27,
    TUNER_TEMIC_4106FH5,
    TUNER_TEMIC_4012FY5,
    TUNER_TEMIC_4136FY5,
    TUNER_LG_TAPCNEW_PAL,
    TUNER_PHILIPS_FM1216ME_MK3,
    TUNER_LG_TAPCNEW_NTSC,
    TUNER_MT2032_PAL,
    TUNER_PHILIPS_FI1286_NTSC_M_J,
    TUNER_MT2050,
    TUNER_MT2050_PAL,
    TUNER_PHILIPS_4IN1,
    TUNER_TCL_2002N,
    TUNER_HITACHI_NTSC,
    TUNER_PHILIPS_PAL_MK,
    TUNER_PHILIPS_FM1236_MK3,
    TUNER_LG_NTSC_TAPE,
    TUNER_TNF_8831BGFF,
    TUNER_PHILIPS_FM1256_IH3,
    TUNER_PHILIPS_FQ1286,
    TUNER_LG_PAL_TAPE,
    TUNER_PHILIPS_FQ1216AME_MK4,
    TUNER_PHILIPS_FQ1236A_MK4,
    TUNER_TDA8275,
    TUNER_YMEC_TVF_8531MF,
    TUNER_YMEC_TVF_5533MF,
    TUNER_TENA_9533_DI,
    TUNER_PHILIPS_FMD1216ME_MK3,
    TUNER_LASTONE,
};

const eTunerId m_TunerHauppaugeAnalog[]=
{
    /* 0-9 */
    TUNER_ABSENT,                       //"None"
    TUNER_ABSENT,                        //"External"
    TUNER_ABSENT,                        //"Unspecified"
    TUNER_PHILIPS_PAL,                    //"Philips FI1216"
    TUNER_PHILIPS_SECAM,                //"Philips FI1216MF"
    TUNER_PHILIPS_NTSC,                    //"Philips FI1236"
    TUNER_PHILIPS_PAL_I,                //"Philips FI1246"
    TUNER_PHILIPS_PAL_DK,                //"Philips FI1256"
    TUNER_PHILIPS_PAL,                    //"Philips FI1216 MK2"
    TUNER_PHILIPS_SECAM,                //"Philips FI1216MF MK2"
    /* 10-19 */
    TUNER_PHILIPS_NTSC,                    //"Philips FI1236 MK2"
    TUNER_PHILIPS_PAL_I,                //"Philips FI1246 MK2"
    TUNER_PHILIPS_PAL_DK,                //"Philips FI1256 MK2"
    TUNER_TEMIC_4032FY5_NTSC,            //"Temic 4032FY5"
    TUNER_TEMIC_4002FH5_PAL,            //"Temic 4002FH5"
    TUNER_TEMIC_4062FY5_PAL_I,            //"Temic 4062FY5"
    TUNER_PHILIPS_PAL,                    //"Philips FR1216 MK2"
    TUNER_PHILIPS_SECAM,                //"Philips FR1216MF MK2"
    TUNER_PHILIPS_NTSC,                    //"Philips FR1236 MK2"
    TUNER_PHILIPS_PAL_I,                //"Philips FR1246 MK2"
    /* 20-29 */
    TUNER_PHILIPS_PAL_DK,                //"Philips FR1256 MK2"
    TUNER_PHILIPS_PAL,                    //"Philips FM1216"
    TUNER_PHILIPS_SECAM,                //"Philips FM1216MF"
    TUNER_PHILIPS_NTSC,                    //"Philips FM1236"
    TUNER_PHILIPS_PAL_I,                //"Philips FM1246"
    TUNER_PHILIPS_PAL_DK,                //"Philips FM1256"
    TUNER_TEMIC_4036FY5_NTSC,            //"Temic 4036FY5"
    TUNER_ABSENT,                        //"Samsung TCPN9082D"
    TUNER_ABSENT,                        //"Samsung TCPM9092P"
    TUNER_TEMIC_4006FH5_PAL,            //"Temic 4006FH5"
    /* 30-39 */
    TUNER_ABSENT,                        //"Samsung TCPN9085D"
    TUNER_ABSENT,                        //"Samsung TCPB9085P"
    TUNER_ABSENT,                        //"Samsung TCPL9091P"
    TUNER_TEMIC_4039FR5_NTSC,            //"Temic 4039FR5"
    TUNER_PHILIPS_MULTI,                //"Philips FQ1216 ME"
    TUNER_TEMIC_4066FY5_PAL_I,            //"Temic 4066FY5"
    TUNER_PHILIPS_NTSC,                    //"Philips TD1536"
    TUNER_PHILIPS_NTSC,                    //"Philips TD1536D"
    TUNER_PHILIPS_NTSC,                    //"Philips FMR1236"
    TUNER_ABSENT,                        //"Philips FI1256MP"
    /* 40-49 */
    TUNER_ABSENT,                        //"Samsung TCPQ9091P"
    TUNER_TEMIC_4006FN5_PAL,            //"Temic 4006FN5"
    TUNER_TEMIC_4009FR5_PAL,            //"Temic 4009FR5"
    TUNER_TEMIC_4046FM5_MULTI,            //"Temic 4046FM5"
    TUNER_TEMIC_4009FN5_PAL,            //"Temic 4009FN5"
    TUNER_ABSENT,                        //"Philips TD1536D_FH_44"
    TUNER_LG_R01F_NTSC,                    //"LG TPI8NSR01F"
    TUNER_LG_B01D_PAL,                    //"LG TPI8PSB01D"
    TUNER_LG_B11D_PAL,                    //"LG TPI8PSB11D"
    TUNER_LG_I001D_PAL_I,                //"LG TAPC-I001D"
    /* 50-59 */
    TUNER_LG_I701D_PAL_I,                //"LG TAPC-I701D"
    TUNER_ABSENT,                       //"Temic 4042FI5"
    TUNER_ABSENT,                       //"Microtune 4049 FM5"
    TUNER_ABSENT,                       //"LG TPI8NSR11F"
    TUNER_ABSENT,                       //"Microtune 4049 FM5 Alt I2C"
    TUNER_ABSENT,                       //"Philips FQ1216ME MK3"
    TUNER_PHILIPS_FM1236_MK3,           //"Philips FI1236 MK3"
    TUNER_PHILIPS_FM1216ME_MK3,         //"Philips FM1216 ME MK3"
    TUNER_PHILIPS_FM1236_MK3,           //"Philips FM1236 MK3"
    TUNER_ABSENT,                       //"Philips FM1216MP MK3"
    /* 60-69 */
    TUNER_PHILIPS_FM1216ME_MK3,         //"LG S001D MK3"
    TUNER_ABSENT,                       //"LG M001D MK3"
    TUNER_ABSENT,                       //"LG S701D MK3"
    TUNER_ABSENT,                       //"LG M701D MK3"
    TUNER_ABSENT,                       //"Temic 4146FM5"
    TUNER_TEMIC_4136FY5,                //"Temic 4136FY5"
    TUNER_TEMIC_4106FH5,                //"Temic 4106FH5"
    TUNER_ABSENT,                       //"Philips FQ1216LMP MK3"
    TUNER_LG_NTSC_TAPE,                 //"LG TAPE H001F MK3"
    TUNER_ABSENT,                       //"LG TAPE H701F MK3"
    /* 70-79 */
    TUNER_ABSENT,                       //"LG TALN H200T"
    TUNER_ABSENT,                       //"LG TALN H250T"
    TUNER_ABSENT,                       //"LG TALN M200T"
    TUNER_ABSENT,                       //"LG TALN Z200T"
    TUNER_ABSENT,                       //"LG TALN S200T"
    TUNER_ABSENT,                       //"Thompson DTT7595"
    TUNER_ABSENT,                       //"Thompson DTT7592"
    TUNER_ABSENT,                       //"Silicon TDA8275C1 8290"
    TUNER_ABSENT,                       //"Silicon TDA8275C1 8290 FM"
    TUNER_ABSENT,                       //"Thompson DTT757"
    /* 80-89 */
    TUNER_ABSENT,                       //"Philips FQ1216LME MK3"
    TUNER_ABSENT,                       //"LG TAPC G701D"
    TUNER_LG_TAPCNEW_NTSC,              //"LG TAPC H791F"
    TUNER_LG_TAPCNEW_PAL,               //"TCL 2002MB 3"
    TUNER_LG_TAPCNEW_PAL,               //"TCL 2002MI 3"
    TUNER_TCL_2002N,                    //"TCL 2002N 6A"
    TUNER_ABSENT,                       //"Philips FQ1236 MK3"
    TUNER_ABSENT,                       //"Samsung TCPN 2121P30A"
    TUNER_ABSENT,                       //"Samsung TCPE 4121P30A"
    TUNER_PHILIPS_FM1216ME_MK3,         //"TCL MFPE05 2"
    /* 90-99 */
    TUNER_ABSENT,                       //"LG TALN H202T"
    TUNER_PHILIPS_FQ1216AME_MK4,        //"Philips FQ1216AME MK4"
    TUNER_PHILIPS_FQ1236A_MK4,          //"Philips FQ1236A MK4"
    TUNER_ABSENT,                       //"Philips FQ1286A MK4"
    TUNER_ABSENT,                       //"Philips FQ1216ME MK5"
    TUNER_ABSENT,                       //"Philips FQ1236 MK5"
    TUNER_ABSENT,                       //"Unspecified"
    TUNER_LG_PAL_TAPE,                  //"LG PAL (TAPE Series)"
};

extern const TCHAR* TunerNames[TUNER_LASTONE];

#endif // !defined(__TUNERID_H__)
