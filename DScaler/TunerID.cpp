//
// $Id: TunerID.cpp,v 1.4 2004-08-27 13:11:27 to_see Exp $
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
//
// $Log: not supported by cvs2svn $
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

const char *TunerNames[TUNER_LASTONE] =
{
        "*No Tuner/Unknown*",                        // TUNER_ABSENT = 0
        "Philips [PAL_I]",	                        // TUNER_PHILIPS_PAL_I		
		"Philips [NTSC]",	                          // TUNER_PHILIPS_NTSC		
		"Philips [SECAM]",	                        // TUNER_PHILIPS_SECAM		
		"Philips [PAL]",		                        // TUNER_PHILIPS_PAL
		"Temic 4002 FH5 [PAL B/G]",                 // TUNER_TEMIC_4002FH5_PAL
		"Temic 4032 FY5 [NTSC]",		                // TUNER_TEMIC_4032FY5_NTSC
		"Temic 4062 FY5 [PAL I]",	                  // TUNER_TEMIC_4062FY5_PAL_I
		"Temic 4036 FY5 [NTSC]",                    // TUNER_TEMIC_4036FY5_NTSC		
        "Alps TSBH1 [NTSC]",	                      // TUNER_ALPS_TSBH1_NTSC                             
		"Alps TSBE1 [PAL]",	                        // TUNER_ALPS_TSBE1_PAL                                    
		"Alps TSBB5 [PAL I]",                       // TUNER_ALPS_TSBB5_PAL_I                                  
		"Alps TSBE5 [PAL]",	                        // TUNER_ALPS_TSBE5_PAL                                    
		"Alps TSBC5 [PAL]",	                        // TUNER_ALPS_TSBC5_PAL                                    
		"Temic 4006 FH5 [PAL B/G]",                 // TUNER_TEMIC_4006FH5_PAL		
		"Philips 1236D Input 1 [ATSC/NTSC]",        // TUNER_PHILIPS_1236D_NTSC_INPUT1
		"Philips 1236D Input 2 [ATSC/NTSC]",        // TUNER_PHILIPS_1236D_NTSC_INPUT2
		"Alps TSCH6 [NTSC]",	                      // TUNER_ALPS_TSCH6_NTSC		                               
		"Temic 4016 FY5 [PAL D/K/L]",	              // TUNER_TEMIC_4016FY5_PAL
		"Philips MK2           [NTSC_M]",	          // TUNER_PHILIPS_MK2_NTSC		
		"Temic 4066 FY5 [PAL I]",                   // TUNER_TEMIC_4066FY5_PAL_I
		"Temic 4006 FN5 [PAL Auto]",                // TUNER_TEMIC_4006FN5_PAL
		"Temic 4009 FR5 [PAL B/G] + FM",            // TUNER_TEMIC_4009FR5_PAL
		"Temic 4039 FR5 [NTSC] + FM",               // TUNER_TEMIC_4039FR5_NTSC
		"Temic 4046 FM5 [PAL/SECAM multi]",         // TUNER_TEMIC_4046FM5_MULTI
		"Philips [PAL_DK]",		                      // TUNER_PHILIPS_PAL_DK
		"Philips FQ1216ME      [PAL/SECAM multi]",  // TUNER_PHILIPS_MULTI		
		"LG TAPC-I001D [PAL I] + FM",			          // TUNER_LG_I001D_PAL_I
		"LG TAPC-I701D [PAL I]",			              // TUNER_LG_I701D_PAL_I
		"LG TPI8NSR01F [NTSC] + FM",			          // TUNER_LG_R01F_NTSC
		"LG TPI8PSB01D [PAL B/G] + FM",			        // TUNER_LG_B01D_PAL
		"LG TPI8PSB11D [PAL B/G]",				          // TUNER_LG_B11D_PAL		
		"Temic 4009 FN5 [PAL Auto] + FM",           // TUNER_TEMIC_4009FN5_PAL
		"MT2032 universal",                         // TUNER_MT2032
		"Sharp 2U5JF5540 [NTSC_JP]",	              // TUNER_SHARP_2U5JF5540_NTSC
		"LG TAPC-H701P [NTSC]",                     // TUNER_LG_TAPCH701P_NTSC
		"Samsung TCPM9091PD27 [PAL B/G/I/D/K]",	    // TUNER_SAMSUNG_PAL_TCPM9091PD27
		"Temic 4106 FH5 [PAL B/G]", 				        // TUNER_TEMIC_4106FH5  
		"Temic 4012 FY5 [PAL D/K/L]",				        // TUNER_TEMIC_4012FY5		
		"Temic 4136 FY5 [NTSC]",				            // TUNER_TEMIC_4136FY5
        "LG TAPC-new   [PAL]", 		  	            	// TUNER_LG_TAPCNEW_PAL		
        "Philips FQ1216ME MK3  [PAL/SECAM multi]",
	    "LG TAPC-new   [NTSC]",
        "MT2032 universal [PAL default]",
        "Philips FI1286 [NTCS M-J]",
        "MT2050 [NTSC]",
        "MT2050 [PAL]",
		"Philips 4in1 [ATI TV Wonder Pro/Conexant]",	// TUNER_PHILIPS_4IN1
};

}
