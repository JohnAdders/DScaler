/////////////////////////////////////////////////////////////////////////////
// $Id: CX2388xCard_Tuner.cpp,v 1.9 2005-12-27 19:29:35 to_see Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 John Adcock.  All rights reserved.
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
// This code is based on a version of dTV modified by Michael Eskin and
// others at Connexant.  Those parts are probably (c) Connexant 2002
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.8  2005/05/12 20:06:22  to_see
// Moved m_TunerHauppaugeAnalog to TunerID.h for common using for BT and CX cards.
//
// Revision 1.7  2005/03/09 15:10:45  atnak
// Added support for TDA8275 tuner and TDA8290.
//
// Revision 1.6  2005/03/09 09:49:34  atnak
// Added a new ITuner::InitializeTuner() function for performing tuner chip
// initializations.
//
// Revision 1.5  2005/03/09 09:35:16  atnak
// Renamed CI2CDevice:::Attach(...) to SetI2CBus(...) to better portray its
// non-intrusive nature.
//
// Revision 1.4  2005/03/06 14:05:51  to_see
// Hauppauge Autodetection updated
//
// Revision 1.3  2005/01/13 19:44:17  to_see
// Added TEA5767 autodetection
//
// Revision 1.2  2004/12/25 22:40:18  to_see
// Changed the card list to an ini file
//
// Revision 1.1  2004/12/20 18:55:35  to_see
// Moved tuner code to new file CX2388xCard_Tuner.cpp
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CX2388xCard.h"
#include "DebugLog.h"
#include "NoTuner.h"
#include "MT2032.h"
#include "MT2050.h"
#include "GenericTuner.h"
#include "TDA9887.h"
#include "TEA5767.h"
#include "TDA8275.h"
#include "TDA8290.h"

BOOL CCX2388xCard::InitTuner(eTunerId tunerId)
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
    case TUNER_TDA8275:
        m_Tuner = new CTDA8275();
        strcpy(m_TunerType, "TDA8275 ");
        break;
    case TUNER_AUTODETECT:
    case TUNER_USER_SETUP:
    case TUNER_ABSENT:
        m_Tuner = new CNoTuner();
        strcpy(m_TunerType, "None ");
        break;
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

    // TDA8275s are paired with a TDA8290.
    if (tunerId == TUNER_TDA8275)
    {
        // Have a TDA8290 object detected and created.
        pExternalIFDemodulator = CTDA8290::CreateDetectedTDA8290(m_I2CBus);
    }

    if (pExternalIFDemodulator == NULL)
    {
        // bUseTDA9887 is the setting in CX2388xCards.ini.
        if (m_CX2388xCards[m_CardType].bUseTDA9887)
        {
            // Have a TDA9887 object detected and created.
            CTDA9887Ex *pTDA9887 = CTDA9887Ex::CreateDetectedTDA9887Ex(m_I2CBus);

            // If a TDA9887 was found.
            if (pTDA9887 != NULL)
            {
                // Set card specific modes that were parsed from CX2388xCards.ini.
                size_t count = m_CX2388xCards[m_CardType].tda9887Modes.size();
                for (size_t i = 0; i < count; i++)
                {
                    pTDA9887->SetModes(&m_CX2388xCards[m_CardType].tda9887Modes[i]);
                }

                // Found a valid external IF demodulator.
                pExternalIFDemodulator = pTDA9887;
            }
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
                
    // Scan the I2C bus addresses for tuners
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

ITuner* CCX2388xCard::GetTuner() const
{
    return m_Tuner;
}

LPCSTR CCX2388xCard::GetTunerType()
{
    return m_TunerType;
}

eTunerId CCX2388xCard::AutoDetectTuner(eCX2388xCardId CardId)
{
	eTunerId TunerId = TUNER_ABSENT;
    TCardType* pCard = &m_CX2388xCards[CardId];

	if(pCard->TunerId == TUNER_USER_SETUP)
	{
		return TUNER_ABSENT;
	}

	else if(pCard->TunerId == TUNER_AUTODETECT)
	{
		eTunerId Tuner = TUNER_ABSENT;
		
		// Read the whole EEPROM without using I2C from SRAM
		BYTE Eeprom[256];
		for (int i=0; i<256; i += 4)
		{
			// DWORD alignment needed
			DWORD dwVal = ReadDword(MAP_EEPROM_DATA + i);
			Eeprom[i+0] = LOBYTE(LOWORD(dwVal));
			Eeprom[i+1] = HIBYTE(LOWORD(dwVal));
			Eeprom[i+2] = LOBYTE(HIWORD(dwVal));
			Eeprom[i+3] = HIBYTE(HIWORD(dwVal));
		}

        // Note: string in card-ini is "Hauppauge WinTV 34xxx models"
        const char* pszCardHauppaugeAnalog = "Hauppauge WinTV 34";

        // Bytes 0:7 are used for PCI SubId, data starts at byte 8
        #define CX_EEPROM_OFFSET 8

        if(strncmp(pCard->szName, pszCardHauppaugeAnalog, strlen(pszCardHauppaugeAnalog)) == 0)
        {
			if (Eeprom[CX_EEPROM_OFFSET + 0] != 0x84 || Eeprom[CX_EEPROM_OFFSET + 2] != 0)
			{
				LOG(1, "AutoDetectTuner: Hauppauge CX2388x Card fails.");
			}
			else
			{
				LOG(2, "AutoDetectTuner: Hauppauge CX2388x Card. TunerId: 0x%02X",Eeprom[CX_EEPROM_OFFSET + 9]);
				if (Eeprom[CX_EEPROM_OFFSET + 9] < (sizeof(m_TunerHauppaugeAnalog) / sizeof(m_TunerHauppaugeAnalog[0]))) 
				{
					Tuner = m_TunerHauppaugeAnalog[Eeprom[CX_EEPROM_OFFSET + 9]];
				}
			}
		}
        
		return Tuner;
	}

	else
	{
		return pCard->TunerId;
	}
}

