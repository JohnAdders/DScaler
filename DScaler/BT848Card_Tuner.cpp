/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Card_Tuner.cpp,v 1.22 2007-02-18 21:15:31 robmuller Exp $
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.21  2005/12/27 19:29:11  to_see
// Added tea5767 auto-detection
//
// Revision 1.20  2005/03/09 15:10:45  atnak
// Added support for TDA8275 tuner and TDA8290.
//
// Revision 1.19  2005/03/09 09:49:33  atnak
// Added a new ITuner::InitializeTuner() function for performing tuner chip
// initializations.
//
// Revision 1.18  2005/03/09 09:35:16  atnak
// Renamed CI2CDevice:::Attach(...) to SetI2CBus(...) to better portray its
// non-intrusive nature.
//
// Revision 1.17  2004/04/19 20:38:37  adcockj
// Fix for previous fix (must learn to program...)
//
// Revision 1.16  2004/04/19 15:13:20  adcockj
// Fix failing to find tda9887 at alternate addresses
//
// Revision 1.15  2004/02/11 20:33:59  adcockj
// Support multiple locations of TDA9887 (thanks to Pityu)
//
// Revision 1.14  2003/12/18 15:57:41  adcockj
// Added MT2050 tuner type support (untested)
//
// Revision 1.13  2003/10/27 10:39:50  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.12  2002/10/16 21:42:36  kooiman
// Created seperate class for External IF Demodulator chips like TDA9887
//
// Revision 1.11  2002/10/11 13:38:14  kooiman
// Added support for VoodooTV IF demodulator. Improved TDA9887. Added interface for GPOE/GPDATA access to make this happen.
//
// Revision 1.10  2002/10/07 20:31:59  kooiman
// Added/fixed TDA9887 support for new Pinnacle cards
//
// Revision 1.9  2002/09/04 11:58:45  kooiman
// Added new tuners & fix for new Pinnacle cards with MT2032 tuner.
//
// Revision 1.8  2002/02/12 02:27:45  ittarnavsky
// fixed the hardware info dialog
//
// Revision 1.7  2001/12/18 23:36:01  adcockj
// Split up the MSP chip support into two parts to avoid probelms when deleting objects
//
// Revision 1.6  2001/11/26 13:02:27  adcockj
// Bug Fixes and standards changes
//
// Revision 1.5  2001/11/25 01:58:34  ittarnavsky
// initial checkin of the new I2C code
//
// Revision 1.4  2001/11/23 10:49:16  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.3  2001/11/18 02:47:08  ittarnavsky
// added all v3.1 suported cards
//
// Revision 1.2  2001/11/02 16:30:07  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.1.2.2  2001/08/22 10:40:58  adcockj
// Added basic tuner support
// Fixed recusive bug
//
// Revision 1.1.2.1  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.1.2.5  2001/08/18 17:09:30  adcockj
// Got to compile, still lots to do...
//
// Revision 1.1.2.4  2001/08/17 16:35:13  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
// Revision 1.1.2.3  2001/08/15 07:10:19  adcockj
// Fixed memory leak
//
// Revision 1.1.2.2  2001/08/14 16:41:36  adcockj
// Renamed driver
// Got to compile with new class based card
//
// Revision 1.1.2.1  2001/08/14 09:40:19  adcockj
// Interim version of code for multiple card support
//
// Revision 1.1  2001/08/13 12:05:12  adcockj
// Updated range for contrast and saturation
// Added more code for new driver interface
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file BT848Card.cpp CBT848Card Implementation (Tuner)
 */

#include "stdafx.h"

#ifdef WANT_BT8X8_SUPPORT

#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "BT848Card.h"
#include "BT848_Defines.h"
#include "NoTuner.h"
#include "MT2032.h"
#include "MT2050.h"
#include "GenericTuner.h"
#include "DebugLog.h"
#include "TDA9887.h"
#include "BT848_VoodooTV_IFdem.h"
#include "TDA8275.h"
#include "TDA8290.h"
#include "TEA5767.h"


BOOL CBT848Card::InitTuner(eTunerId tunerId)
{
    BOOL LookForIFDemod = FALSE;

    // clean up if we get called twice
    if(m_Tuner != NULL)
    {
        delete m_Tuner;
        m_Tuner = NULL;
    }

    switch (tunerId)
    {
    case TUNER_MT2032:
        m_Tuner = new CMT2032(VIDEOFORMAT_NTSC_M);
        LookForIFDemod = TRUE;
        strcpy(m_TunerType, "MT2032 ");
        break;
    case TUNER_MT2032_PAL:
        m_Tuner = new CMT2032(VIDEOFORMAT_PAL_B);
        LookForIFDemod = TRUE;
        strcpy(m_TunerType, "MT2032 ");
        break;
    case TUNER_MT2050:
        m_Tuner = new CMT2050(VIDEOFORMAT_NTSC_M);
        LookForIFDemod = TRUE;
        strcpy(m_TunerType, "MT2050 ");
        break;
    case TUNER_MT2050_PAL:
        m_Tuner = new CMT2050(VIDEOFORMAT_PAL_B);
        LookForIFDemod = TRUE;
        strcpy(m_TunerType, "MT2050 ");
        break;
    case TUNER_TDA8275:
        m_Tuner = new CTDA8275();
        strcpy(m_TunerType, "TDA8275 ");
		LookForIFDemod = TRUE;
        break;
    case TUNER_AUTODETECT:
    case TUNER_USER_SETUP:
    case TUNER_ABSENT:
        m_Tuner = new CNoTuner();
        strcpy(m_TunerType, "None ");
        break;
    case TUNER_PHILIPS_FM1216ME_MK3:
        LookForIFDemod = TRUE;
        // deliberate drop down
    default:
        m_Tuner = new CGenericTuner(tunerId);
        strcpy(m_TunerType, "Generic ");
        break;
    }

    // Finished if tuner type is CNoTuner
    switch (tunerId)
    {
    case TUNER_AUTODETECT:
    case TUNER_USER_SETUP:
    case TUNER_ABSENT:
        return TRUE;
    }


	// Look for possible external IF demodulator
    IExternalIFDemodulator *pExternalIFDemodulator = NULL;
    if (LookForIFDemod)
    {
        switch (m_CardType)
        {
        case TVCARD_MIRO:
        case TVCARD_MIROPRO:
        case TVCARD_PINNACLERAVE:
        case TVCARD_PINNACLEPRO:
           {
               //Get Card ID
                WriteDword(BT848_GPIO_OUT_EN,( 0x0000 )&0x00FFFFFFL);
                long Id = ReadDword(BT848_GPIO_DATA);
                Id = ((Id >> 10) & 63) - 1;

                if (Id>=32)
                {
                    // Only newer cards use MT2032 & TDA9885/6/7
                    Id = 63 - Id;
                    pExternalIFDemodulator = new CTDA9887Pinnacle(Id);
                }
            }
            break;

        case TVCARD_VOODOOTV_200:
        case TVCARD_VOODOOTV_FM:
            pExternalIFDemodulator = new CPreTuneVoodooFM(this);
            break;

		case TUNER_TDA8275:
			pExternalIFDemodulator = new CTDA8290();
			break;

        default:
            //Detect TDA 9887
            pExternalIFDemodulator = new CTDA9887();
            break;
        }
    }

	eVideoFormat videoFormat = m_Tuner->GetDefaultVideoFormat();

	// Detect and attach IF demodulator to the tuner
    //  or delete the demodulator if the chip doesn't exist.
    if (pExternalIFDemodulator != NULL)
    {
		if (pExternalIFDemodulator->SetDetectedI2CAddress(m_I2CBus))
		{
			m_Tuner->AttachIFDem(pExternalIFDemodulator, TRUE);
			pExternalIFDemodulator->Init(TRUE, videoFormat);
		}
		else
		{
			// if didn't find anything then
			// need to delete the instance
			delete pExternalIFDemodulator;
			pExternalIFDemodulator = NULL;
		}
    }

    // Scan the I2C bus addresses 0xC0 - 0xCF for tuners
    BOOL bFoundTuner = FALSE;

	// Scan the I2C bus addresses 0xC0 - 0xCF for tuners.
	BYTE test = IsTEA5767PresentAtC0(m_I2CBus) ? 0xC2 : 0xC0;
	for ( ; test < 0xCF; test += 0x02)
    {
        if (m_I2CBus->Write(&test, sizeof(test)))
        {
            m_Tuner->SetI2CBus(m_I2CBus, test>>1);

            // Initialize the tuner.
            if (m_Tuner->InitializeTuner())
            {
				bFoundTuner = TRUE;
				int length = strlen(m_TunerType);
				sprintf(m_TunerType + length, "@ I2C address 0x%02X", test);
				LOG(1,"Tuner: Found at I2C address 0x%02x", test);
				break;
            }
        }
    }

    if (pExternalIFDemodulator != NULL)
    {
        //End initialization
        pExternalIFDemodulator->Init(FALSE, videoFormat);
    }

    if (!bFoundTuner)
    {
        LOG(1,"Tuner: No tuner found at I2C addresses 0xC0-0xCF");

        delete m_Tuner;
        m_Tuner = new CNoTuner();
        strcpy(m_TunerType, "None ");
    }
    return bFoundTuner;
}

ITuner* CBT848Card::GetTuner() const
{
    return m_Tuner;
}

#endif // WANT_BT8X8_SUPPORT