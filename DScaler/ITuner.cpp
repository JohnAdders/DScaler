//
// $Id: ITuner.cpp,v 1.6 2002-10-26 15:37:57 adcockj Exp $
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
// Revision 1.5  2002/10/16 21:42:36  kooiman
// Created seperate class for External IF Demodulator chips like TDA9887
//
// Revision 1.4  2002/09/04 11:58:45  kooiman
// Added new tuners & fix for new Pinnacle cards with MT2032 tuner.
//
// Revision 1.3  2002/08/03 17:57:52  kooiman
// Added new cards & tuners. Changed the tuner combobox into a sorted list.
//
// Revision 1.2  2002/01/16 19:16:20  adcockj
// added support for LG NTSC (TAPCH701P)
//
// Revision 1.1  2001/11/25 02:03:21  ittarnavsky
// initial checkin of the new I2C code
//
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ITuner.h"

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
        "MT2032 universal [PAL default]"
};

}



ITuner::ITuner()
{
    m_ExternalIFDemodulator = NULL;
    m_bFreeIFDemodulatorOnDestruction = FALSE;
}

ITuner::~ITuner()
{
    if (m_bFreeIFDemodulatorOnDestruction && (m_ExternalIFDemodulator != NULL))
    {
        delete m_ExternalIFDemodulator;
    }
}

void ITuner::AttachIFDem(IExternalIFDemodulator* pExternalIFDemodulator, bool bFreeOnDestruction)
{
    m_ExternalIFDemodulator = pExternalIFDemodulator;
    m_bFreeIFDemodulatorOnDestruction = bFreeOnDestruction;
}

II2CTuner::II2CTuner()
{
}

II2CTuner::~II2CTuner()
{
}
