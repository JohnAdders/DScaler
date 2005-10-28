/////////////////////////////////////////////////////////////////////////////
// $Id: TunerID.cpp,v 1.1 2005-10-28 16:43:13 to_see Exp $
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
//
// $Log: not supported by cvs2svn $
// Revision 1.4  2004/08/27 13:11:27  to_see
// Added Philips 4in1 tuner (from v4l2)
//
// Revision 1.3  2003/12/18 15:57:41  adcockj
// Added MT2050 tuner type support (untested)
//
// Revision 1.2  2003/10/27 10:39:54  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.1  2003/02/06 21:26:37  ittarnavsky
// moved the tuner names here
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file TunerID.cpp Tuner definitions
 */

#include "stdafx.h"
#include "TunerID.h"


extern "C" {

const char* TunerNames[TUNER_LASTONE + 2] =
{
    "*Auto Detect*",                            //          TUNER_AUTODETECT = -2
    "*User Setup*",                             //          TUNER_USER_SETUP = -1
    "*No Tuner/Unknown*",                       //          TUNER_ABSENT = 0

    // Name                                         v4l2    ID
    "Philips FI1246 [PAL I]",	                //    1     TUNER_PHILIPS_PAL_I		
    "Philips FI1236 [NTSC]",	                //    2     TUNER_PHILIPS_NTSC		
    "Philips [SECAM]",	                        //    3     TUNER_PHILIPS_SECAM		
    "Philips [PAL]",		                    //    5     TUNER_PHILIPS_PAL
    "Temic 4002 FH5 [PAL B/G]",                 //    0     TUNER_TEMIC_4002FH5_PAL
    "Temic 4032 FY5 [NTSC]",		            //    6     TUNER_TEMIC_4032FY5_NTSC
    "Temic 4062 FY5 [PAL I]",	                //    7     TUNER_TEMIC_4062FY5_PAL_I
    "Temic 4036 FY5 [NTSC]",                    //    8     TUNER_TEMIC_4036FY5_NTSC		
    "Alps TSBH1 [NTSC]",	                    //    9     TUNER_ALPS_TSBH1_NTSC                             
    "Alps TSBE1 [PAL]",	                        //   10     TUNER_ALPS_TSBE1_PAL                                    
    "Alps TSBB5 [PAL I]",                       //   11     TUNER_ALPS_TSBB5_PAL_I                                  
    "Alps TSBE5 [PAL]",	                        //   12     TUNER_ALPS_TSBE5_PAL                                    
    "Alps TSBC5 [PAL]",	                        //   13     TUNER_ALPS_TSBC5_PAL                                    
    "Temic 4006 FH5 [PAL B/G]",                 //   14     TUNER_TEMIC_4006FH5_PAL		
    "Philips 1236D Input 1 [ATSC/NTSC]",        //   42     TUNER_PHILIPS_1236D_NTSC_INPUT1
    "Philips 1236D Input 2 [ATSC/NTSC]",        //   42     TUNER_PHILIPS_1236D_NTSC_INPUT2
    "Alps TSCH6 [NTSC]",	                    //   15     TUNER_ALPS_TSCH6_NTSC		                               
    "Temic 4016 FY5 [PAL D/K/L]",	            //   16     TUNER_TEMIC_4016FY5_PAL
    "Philips MK2 [NTSC M]",	                    //   17     TUNER_PHILIPS_MK2_NTSC		
    "Temic 4066 FY5 [PAL I]",                   //   18     TUNER_TEMIC_4066FY5_PAL_I
    "Temic 4006 FN5 [PAL Auto]",                //   19     TUNER_TEMIC_4006FN5_PAL
    "Temic 4009 FR5 [PAL B/G] + FM",            //   20     TUNER_TEMIC_4009FR5_PAL
    "Temic 4039 FR5 [NTSC] + FM",               //   21     TUNER_TEMIC_4039FR5_NTSC
    "Temic 4046 FM5 [PAL/SECAM multi]",         //   22     TUNER_TEMIC_4046FM5_MULTI
    "Philips [PAL D/K]",		                //   23     TUNER_PHILIPS_PAL_DK
    "Philips FQ1216ME [PAL/SECAM multi]",       //   24     TUNER_PHILIPS_MULTI		
    "LG TAPC-I001D [PAL I] + FM",			    //   25     TUNER_LG_I001D_PAL_I
    "LG TAPC-I701D [PAL I]",			        //   26     TUNER_LG_I701D_PAL_I
    "LG TPI8NSR01F [NTSC] + FM",			    //   27     TUNER_LG_R01F_NTSC
    "LG TPI8PSB01D [PAL B/G] + FM",			    //   28     TUNER_LG_B01D_PAL
    "LG TPI8PSB11D [PAL B/G]",				    //   29     TUNER_LG_B11D_PAL		
    "Temic 4009 FN5 [PAL Auto] + FM",           //   30     TUNER_TEMIC_4009FN5_PAL
    "MT2032 universal",                         //   33     TUNER_MT2032
    "Sharp 2U5JF5540 [NTSC JP]",	            //   31     TUNER_SHARP_2U5JF5540_NTSC
    "LG TAPC-H701P [NTSC]",                     //          TUNER_LG_TAPCH701P_NTSC
    "Samsung TCPM9091PD27 [PAL B/G/I/D/K]",	    //   32     TUNER_SAMSUNG_PAL_TCPM9091PD27
    "Temic 4106 FH5 [PAL B/G]", 				//   34     TUNER_TEMIC_4106FH5  
    "Temic 4012 FY5 [PAL D/K/L]",				//   35     TUNER_TEMIC_4012FY5		
    "Temic 4136 FY5 [NTSC]",				    //   36     TUNER_TEMIC_4136FY5
    "LG TAPC-new [PAL]", 		  	            //   37     TUNER_LG_TAPCNEW_PAL		
    "Philips FQ1216ME MK3 [PAL/SECAM multi]",   //   38     TUNER_PHILIPS_FM1216ME_MK3
    "LG TAPC-new [NTSC]",                       //   39     TUNER_LG_TAPCNEW_NTSC
    "MT2032 universal [PAL default]",           //   33     TUNER_MT2032_PAL
    "Philips FI1286 [NTCS M/J]",                //          TUNER_PHILIPS_FI1286_NTSC_M_J
    "MT2050 [NTSC]",                            //   33     TUNER_MT2050
    "MT2050 [PAL]",                             //   33     TUNER_MT2050_PAL
    "Philips 4in1 [ATI TV Wonder Pro/Conexant]",//   44     TUNER_PHILIPS_4IN1
    "TCL 2002N",                                //   50     TUNER_TCL_2002N
    "HITACHI V7-J180AT",                        //   40     TUNER_HITACHI_NTSC
    "Philips FI1216 MK [PAL]",                  //   41     TUNER_PHILIPS_PAL_MK
    "Philips FM1236 MK3 [NTSC]",                //   43     TUNER_PHILIPS_FM1236_MK3
    "LG TAPE series [NTSC]",                    //   47     TUNER_LG_NTSC_TAPE
    "Tenna TNF 8831 [PAL]",                     //   48     TUNER_TNF_8831BGFF
    "Philips FM1256 MK3 [PAL/SECAM D]",         //   51     TUNER_PHILIPS_FM1256_IH3
    "Philips FQ1286 [NTSC]",                    //   53     TUNER_PHILIPS_FQ1286
    "LG TAPE series [PAL]",                     //   55     TUNER_LG_PAL_TAPE
    "Philips FM1216AME [PAL/SECAM multi]",      //   56     TUNER_PHILIPS_FQ1216AME_MK4
    "Philips FQ1236A MK4 [NTSC]",               //   57     TUNER_PHILIPS_FQ1236A_MK4
    "Philips TDA8275",                          //   54     TUNER_TDA8275
    "Ymec TVF-8531MF/8831MF/8731MF [NTSC]",     //   58     TUNER_YMEC_TVF_8531MF
    "Ymec TVision TVF-5533MF [NTSC]",           //   59     TUNER_YMEC_TVF_5533MF
    "Tena TNF9533-D/IF/TNF9533-B/DF [PAL]",     //   61     TUNER_TENA_9533_DI
    "Philips FMD1216ME MK3 Hybrid [PAL]",       //   63     TUNER_PHILIPS_FMD1216ME_MK3
};

}
