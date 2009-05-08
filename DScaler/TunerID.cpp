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
 * @file TunerID.cpp Tuner definitions
 */

#include "stdafx.h"
#include "TunerID.h"


extern "C" {

const TCHAR* TunerNames[TUNER_LASTONE] =
{
    // Name                                         v4l2    ID
    _T("*No Tuner/Unknown*"),                       //    4     TUNER_ABSENT = 0
    _T("Philips FI1246 [PAL I]"),                    //    1     TUNER_PHILIPS_PAL_I
    _T("Philips FI1236 [NTSC]"),                    //    2     TUNER_PHILIPS_NTSC
    _T("Philips [SECAM]"),                            //    3     TUNER_PHILIPS_SECAM
    _T("Philips [PAL]"),                            //    5     TUNER_PHILIPS_PAL
    _T("Temic 4002 FH5 [PAL B/G]"),                 //    0     TUNER_TEMIC_4002FH5_PAL
    _T("Temic 4032 FY5 [NTSC]"),                    //    6     TUNER_TEMIC_4032FY5_NTSC
    _T("Temic 4062 FY5 [PAL I]"),                    //    7     TUNER_TEMIC_4062FY5_PAL_I
    _T("Temic 4036 FY5 [NTSC]"),                    //    8     TUNER_TEMIC_4036FY5_NTSC
    _T("Alps TSBH1 [NTSC]"),                        //    9     TUNER_ALPS_TSBH1_NTSC
    _T("Alps TSBE1 [PAL]"),                            //   10     TUNER_ALPS_TSBE1_PAL
    _T("Alps TSBB5 [PAL I]"),                       //   11     TUNER_ALPS_TSBB5_PAL_I
    _T("Alps TSBE5 [PAL]"),                            //   12     TUNER_ALPS_TSBE5_PAL
    _T("Alps TSBC5 [PAL]"),                            //   13     TUNER_ALPS_TSBC5_PAL
    _T("Temic 4006 FH5 [PAL B/G]"),                 //   14     TUNER_TEMIC_4006FH5_PAL
    _T("Philips 1236D Input 1 [ATSC/NTSC]"),        //   42     TUNER_PHILIPS_1236D_NTSC_INPUT1
    _T("Philips 1236D Input 2 [ATSC/NTSC]"),        //   42     TUNER_PHILIPS_1236D_NTSC_INPUT2
    _T("Alps TSCH6 [NTSC]"),                        //   15     TUNER_ALPS_TSCH6_NTSC
    _T("Temic 4016 FY5 [PAL D/K/L]"),                //   16     TUNER_TEMIC_4016FY5_PAL
    _T("Philips MK2 [NTSC M]"),                        //   17     TUNER_PHILIPS_MK2_NTSC
    _T("Temic 4066 FY5 [PAL I]"),                   //   18     TUNER_TEMIC_4066FY5_PAL_I
    _T("Temic 4006 FN5 [PAL Auto]"),                //   19     TUNER_TEMIC_4006FN5_PAL
    _T("Temic 4009 FR5 [PAL B/G] + FM"),            //   20     TUNER_TEMIC_4009FR5_PAL
    _T("Temic 4039 FR5 [NTSC] + FM"),               //   21     TUNER_TEMIC_4039FR5_NTSC
    _T("Temic 4046 FM5 [PAL/SECAM multi]"),         //   22     TUNER_TEMIC_4046FM5_MULTI
    _T("Philips [PAL D/K]"),                        //   23     TUNER_PHILIPS_PAL_DK
    _T("Philips FQ1216ME [PAL/SECAM multi]"),       //   24     TUNER_PHILIPS_MULTI
    _T("LG TAPC-I001D [PAL I] + FM"),                //   25     TUNER_LG_I001D_PAL_I
    _T("LG TAPC-I701D [PAL I]"),                    //   26     TUNER_LG_I701D_PAL_I
    _T("LG TPI8NSR01F [NTSC] + FM"),                //   27     TUNER_LG_R01F_NTSC
    _T("LG TPI8PSB01D [PAL B/G] + FM"),                //   28     TUNER_LG_B01D_PAL
    _T("LG TPI8PSB11D [PAL B/G]"),                    //   29     TUNER_LG_B11D_PAL
    _T("Temic 4009 FN5 [PAL Auto] + FM"),           //   30     TUNER_TEMIC_4009FN5_PAL
    _T("MT2032 universal"),                         //   33     TUNER_MT2032
    _T("Sharp 2U5JF5540 [NTSC JP]"),                //   31     TUNER_SHARP_2U5JF5540_NTSC
    _T("LG TAPC-H701P [NTSC]"),                     //          TUNER_LG_TAPCH701P_NTSC
    _T("Samsung TCPM9091PD27 [PAL B/G/I/D/K]"),        //   32     TUNER_SAMSUNG_PAL_TCPM9091PD27
    _T("Temic 4106 FH5 [PAL B/G]"),                 //   34     TUNER_TEMIC_4106FH5
    _T("Temic 4012 FY5 [PAL D/K/L]"),                //   35     TUNER_TEMIC_4012FY5
    _T("Temic 4136 FY5 [NTSC]"),                    //   36     TUNER_TEMIC_4136FY5
    _T("LG TAPC-new [PAL]"),                           //   37     TUNER_LG_TAPCNEW_PAL
    _T("Philips FQ1216ME MK3 [PAL/SECAM multi]"),   //   38     TUNER_PHILIPS_FM1216ME_MK3
    _T("LG TAPC-new [NTSC]"),                       //   39     TUNER_LG_TAPCNEW_NTSC
    _T("MT2032 universal [PAL default]"),           //   33     TUNER_MT2032_PAL
    _T("Philips FI1286 [NTCS M/J]"),                //          TUNER_PHILIPS_FI1286_NTSC_M_J
    _T("MT2050 [NTSC]"),                            //   33     TUNER_MT2050
    _T("MT2050 [PAL]"),                             //   33     TUNER_MT2050_PAL
    _T("Philips 4in1 [ATI TV Wonder Pro/Conexant]"),//   44     TUNER_PHILIPS_4IN1
    _T("TCL 2002N"),                                //   50     TUNER_TCL_2002N
    _T("HITACHI V7-J180AT"),                        //   40     TUNER_HITACHI_NTSC
    _T("Philips FI1216 MK [PAL]"),                  //   41     TUNER_PHILIPS_PAL_MK
    _T("Philips FM1236 MK3 [NTSC]"),                //   43     TUNER_PHILIPS_FM1236_MK3
    _T("LG TAPE series [NTSC]"),                    //   47     TUNER_LG_NTSC_TAPE
    _T("Tenna TNF 8831 [PAL]"),                     //   48     TUNER_TNF_8831BGFF
    _T("Philips FM1256 MK3 [PAL/SECAM D]"),         //   51     TUNER_PHILIPS_FM1256_IH3
    _T("Philips FQ1286 [NTSC]"),                    //   53     TUNER_PHILIPS_FQ1286
    _T("LG TAPE series [PAL]"),                     //   55     TUNER_LG_PAL_TAPE
    _T("Philips FM1216AME [PAL/SECAM multi]"),      //   56     TUNER_PHILIPS_FQ1216AME_MK4
    _T("Philips FQ1236A MK4 [NTSC]"),               //   57     TUNER_PHILIPS_FQ1236A_MK4
    _T("Philips TDA8275"),                          //   54     TUNER_TDA8275
    _T("Ymec TVF-8531MF/8831MF/8731MF [NTSC]"),     //   58     TUNER_YMEC_TVF_8531MF
    _T("Ymec TVision TVF-5533MF [NTSC]"),           //   59     TUNER_YMEC_TVF_5533MF
    _T("Tena TNF9533-D/IF/TNF9533-B/DF [PAL]"),     //   61     TUNER_TENA_9533_DI
    _T("Philips FMD1216ME MK3 Hybrid [PAL]"),       //   63     TUNER_PHILIPS_FMD1216ME_MK3
};

}
