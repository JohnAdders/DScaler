/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Card_Tuner.cpp,v 1.18 2005-03-09 09:35:16 atnak Exp $
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
// This software was based on BT848Card_Tuner.cpp.  Those portions are
// Copyright (c) 2001 John Adcock.
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
// Revision 1.17  2005/01/15 01:27:42  atnak
// Added TEA5767 autodetection.
//
// Revision 1.16  2004/11/28 06:54:34  atnak
// Changed IF demodulator scanning to use CTDA9887's DetectAttach().
//
// Revision 1.15  2004/11/27 19:31:57  atnak
// Updated to use CTDA9887Ex class and tda9887 settings in card list.
//
// Revision 1.14  2004/11/23 19:25:13  to_see
// Added comments for Atsushi.
//
// Revision 1.13  2004/11/20 14:20:09  atnak
// Changed the card list to an ini file.
//
// Revision 1.12  2004/04/19 20:38:38  adcockj
// Fix for previous fix (must learn to program...)
//
// Revision 1.11  2004/04/19 15:13:20  adcockj
// Fix failing to find tda9887 at alternate addresses
//
// Revision 1.10  2004/02/11 20:34:00  adcockj
// Support multiple locations of TDA9887 (thanks to Pityu)
//
// Revision 1.9  2003/12/18 15:57:41  adcockj
// Added MT2050 tuner type support (untested)
//
// Revision 1.8  2003/10/27 10:39:53  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.7  2003/01/28 07:22:28  atnak
// Visual changes
//
// Revision 1.6  2002/10/26 05:24:23  atnak
// Minor cleanups
//
// Revision 1.5  2002/10/16 21:43:04  kooiman
// Created seperate class for External IF Demodulator chips like TDA9887
//
// Revision 1.4  2002/10/11 13:39:37  kooiman
// Adapted for changed MT2032 initialization.
//
// Revision 1.3  2002/09/14 19:40:48  atnak
// various changes
//
//
//
//////////////////////////////////////////////////////////////////////////////


/**
 * @file SAA7134Card.cpp CSAA7134Card Implementation (Tuner)
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "SAA7134Card.h"
#include "SAA7134_Defines.h"
#include "NoTuner.h"
#include "MT2032.h"
#include "MT2050.h"
#include "GenericTuner.h"
#include "TDA9887.h"
#include "TEA5767.h"
#include "DebugLog.h"


BOOL CSAA7134Card::InitTuner(eTunerId tunerId)
{
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
        strcpy(m_TunerType, "MT2032 ");
        break;
    case TUNER_MT2032_PAL:
        m_Tuner = new CMT2032(VIDEOFORMAT_PAL_B);
        strcpy(m_TunerType, "MT2032 ");
        break;
    case TUNER_MT2050:
        m_Tuner = new CMT2050(VIDEOFORMAT_NTSC_M);
        strcpy(m_TunerType, "MT2050 ");
        break;
    case TUNER_MT2050_PAL:
        m_Tuner = new CMT2050(VIDEOFORMAT_PAL_B);
        strcpy(m_TunerType, "MT2050 ");
        break;
    case TUNER_AUTODETECT:
    case TUNER_USER_SETUP:
    case TUNER_ABSENT:
        m_Tuner = new CNoTuner();
        strcpy(m_TunerType, "None ");
        break;
    case TUNER_PHILIPS_FM1216ME_MK3:
        // deliberate drop down
    default:
        m_Tuner = new CGenericTuner(tunerId);
        strcpy(m_TunerType, "Generic ");
        break;
    }

    // Finished if tuner type is CNoTuner
	if (m_Tuner->GetTunerId() == TUNER_ABSENT)
	{
		return TRUE;
	}

    // Look for possible external IF demodulator
    IExternalIFDemodulator *pExternalIFDemodulator = NULL;

	// bUseTDA9887 is the setting in SAA713xCards.ini.
    if (m_SAA713xCards[m_CardType].bUseTDA9887)
    {
        CTDA9887Ex *pTDA9887 = new CTDA9887Ex();

		// Detect to make sure an IF demodulator exists.
		if (pTDA9887->SetDetectedI2CAddress(m_I2CBus))
		{
			// Set card specific modes that were parsed from SAA713xCards.ini.
			size_t count = m_SAA713xCards[m_CardType].tda9887Modes.size();
			for (size_t i = 0; i < count; i++)
			{
				pTDA9887->SetModes(&m_SAA713xCards[m_CardType].tda9887Modes[i]);
			}

			// Found a valid external IF demodulator.
			pExternalIFDemodulator = pTDA9887;
		}
		else
		{
			// A TDA9887 device wasn't detected.
			delete pTDA9887;
		}
    }

	eVideoFormat videoFormat = m_Tuner->GetDefaultVideoFormat();

	if (pExternalIFDemodulator != NULL)
	{
		// Attach the IF demodulator to the tuner.
		m_Tuner->AttachIFDem(pExternalIFDemodulator, TRUE);
		// Let the IF demodulator know of pre-initialization.
		pExternalIFDemodulator->Init(TRUE, videoFormat);
	}

    // Scan the I2C bus addresses 0xC0 - 0xCF for tuners
    BOOL bFoundTuner = FALSE;

	BYTE test = IsTEA5767PresentAtC0(m_I2CBus) ? 0xC2 : 0xC0;
    for ( ; test < 0xCF; test += 0x02)
    {
        if (m_I2CBus->Write(&test, sizeof(test)))
        {
			int kk = strlen(m_TunerType);
			m_Tuner->SetI2CBus(m_I2CBus, test>>1);
			sprintf(m_TunerType + kk, "@ I2C address 0x%02X", test);
            bFoundTuner = TRUE;
            LOG(1,"Tuner: Found at I2C address 0x%02x",test);
            break;
        }
    }

    if (pExternalIFDemodulator != NULL)
    {
        // End initialization
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


ITuner* CSAA7134Card::GetTuner() const
{
    return m_Tuner;
}


LPCSTR CSAA7134Card::GetTunerType()
{
    return m_TunerType;
}

