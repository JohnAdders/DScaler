/////////////////////////////////////////////////////////////////////////////
// $Id: CX2388xCard_Tuner.cpp,v 1.4 2005-03-06 14:05:51 to_see Exp $
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

	// bUseTDA9887 is the setting in CX2388xCards.ini.
    if (m_CX2388xCards[m_CardType].bUseTDA9887)
    {
        CTDA9887Ex *pTDA9887 = new CTDA9887Ex();

		// Detect to make sure an IF demodulator exists.
		if (pTDA9887->DetectAttach(m_I2CBus))
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
                
    // Scan the I2C bus addresses for tuners
    BOOL bFoundTuner = FALSE;

    // Try to detect TEA5767 FM-Radio chip 
    BYTE StartAddress;
    if(IsTEA5767PresentAtC0(m_I2CBus))
    {
        StartAddress = 0xC2;
    }
    else
    {
        StartAddress = 0xC0;
    }

    int kk = strlen(m_TunerType);
    for (BYTE test = StartAddress; test < 0xCF; test +=2)
    {
        if (m_I2CBus->Write(&test, sizeof(test)))
        {
            m_Tuner->Attach(m_I2CBus, test>>1);
            sprintf(m_TunerType + kk, " at I2C address 0x%02x", test);
            bFoundTuner = TRUE;
            LOG(1,"Tuner: Found at I2C address 0x%02x",test);
            break;
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

const eTunerId CCX2388xCard::m_TunerHauppaugeAnalog[]=
{
	/* 0-9 */
    TUNER_ABSENT,
	TUNER_ABSENT,						//"External"
	TUNER_ABSENT,						//"Unspecified"
	TUNER_PHILIPS_PAL,					//"Philips FI1216"
	TUNER_PHILIPS_SECAM,				//"Philips FI1216MF"
	TUNER_PHILIPS_NTSC,					//"Philips FI1236"
	TUNER_PHILIPS_PAL_I,				//"Philips FI1246"
	TUNER_PHILIPS_PAL_DK,				//"Philips FI1256"
	TUNER_PHILIPS_PAL,					//"Philips FI1216 MK2"
	TUNER_PHILIPS_SECAM,				//"Philips FI1216MF MK2"
	/* 10-19 */
    TUNER_PHILIPS_NTSC,					//"Philips FI1236 MK2"
	TUNER_PHILIPS_PAL_I,				//"Philips FI1246 MK2"
	TUNER_PHILIPS_PAL_DK,				//"Philips FI1256 MK2"
	TUNER_TEMIC_4032FY5_NTSC,			//"Temic 4032FY5"
	TUNER_TEMIC_4002FH5_PAL,			//"Temic 4002FH5"
	TUNER_TEMIC_4062FY5_PAL_I,			//"Temic 4062FY5"
	TUNER_PHILIPS_PAL,					//"Philips FR1216 MK2"
	TUNER_PHILIPS_SECAM,				//"Philips FR1216MF MK2"
	TUNER_PHILIPS_NTSC,					//"Philips FR1236 MK2"
	TUNER_PHILIPS_PAL_I,				//"Philips FR1246 MK2"
	/* 20-29 */
	TUNER_PHILIPS_PAL_DK,				//"Philips FR1256 MK2"
	TUNER_PHILIPS_PAL,					//"Philips FM1216"
	TUNER_PHILIPS_SECAM,				//"Philips FM1216MF"
	TUNER_PHILIPS_NTSC,					//"Philips FM1236"
	TUNER_PHILIPS_PAL_I,				//"Philips FM1246"
	TUNER_PHILIPS_PAL_DK,				//"Philips FM1256"
	TUNER_TEMIC_4036FY5_NTSC,			//"Temic 4036FY5"
	TUNER_ABSENT,						//"Samsung TCPN9082D"
	TUNER_ABSENT,						//"Samsung TCPM9092P"
	TUNER_TEMIC_4006FH5_PAL,			//"Temic 4006FH5"
	/* 30-39 */
    TUNER_ABSENT,						//"Samsung TCPN9085D"
	TUNER_ABSENT,						//"Samsung TCPB9085P"
	TUNER_ABSENT,						//"Samsung TCPL9091P"
	TUNER_TEMIC_4039FR5_NTSC,			//"Temic 4039FR5"
	TUNER_PHILIPS_MULTI,				//"Philips FQ1216 ME"
	TUNER_TEMIC_4066FY5_PAL_I,			//"Temic 4066FY5"
	TUNER_PHILIPS_NTSC,					//"Philips TD1536"
	TUNER_PHILIPS_NTSC,					//"Philips TD1536D"
	TUNER_PHILIPS_NTSC,					//"Philips FMR1236"
    TUNER_ABSENT,						//"Philips FI1256MP"
	/* 40-49 */
	TUNER_ABSENT,						//"Samsung TCPQ9091P"
	TUNER_TEMIC_4006FN5_PAL,			//"Temic 4006FN5"
	TUNER_TEMIC_4009FR5_PAL,			//"Temic 4009FR5"
	TUNER_TEMIC_4046FM5_MULTI,			//"Temic 4046FM5"
	TUNER_TEMIC_4009FN5_PAL,			//"Temic 4009FN5"
	TUNER_ABSENT,						//"Philips TD1536D_FH_44"
	TUNER_LG_R01F_NTSC,					//"LG TPI8NSR01F"
	TUNER_LG_B01D_PAL,					//"LG TPI8PSB01D"
	TUNER_LG_B11D_PAL,					//"LG TPI8PSB11D"
	TUNER_LG_I001D_PAL_I,				//"LG TAPC-I001D"
	/* 50-59 */
    TUNER_LG_I701D_PAL_I,				//"LG TAPC-I701D"
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
    TUNER_ABSENT,                       //"LG S001D MK3"
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
    TUNER_ABSENT,                       //"TCL 2002MB 3"
    TUNER_ABSENT,                       //"TCL 2002MI 3"
    TUNER_TCL_2002N,                    //"TCL 2002N 6A"
    TUNER_ABSENT,                       //"Philips FQ1236 MK3"
    TUNER_ABSENT,                       //"Samsung TCPN 2121P30A"
    TUNER_ABSENT,                       //"Samsung TCPE 4121P30A"
    TUNER_ABSENT,                       //"TCL MFPE05 2"
	/* 90-99 */
    TUNER_ABSENT,                       //"LG TALN H202T"
    TUNER_ABSENT,                       //"Philips FQ1216AME MK4"
    TUNER_PHILIPS_FQ1236A_MK4,          //"Philips FQ1236A MK4"
    TUNER_ABSENT,                       //"Philips FQ1286A MK4"
    TUNER_ABSENT,                       //"Philips FQ1216ME MK5"
    TUNER_ABSENT,                       //"Philips FQ1236 MK5"
};

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
		
		// Read the whole EEPROM
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

        if(strncmp(pCard->szName, pszCardHauppaugeAnalog, strlen(pszCardHauppaugeAnalog)) == 0)
        {
			if (Eeprom[8+0] != 0x84 || Eeprom[8+2] != 0)
			{
				LOG(1, "AutoDetectTuner: Hauppauge CX2388x Card fails.");
			}
			else
			{
				LOG(2, "AutoDetectTuner: Hauppauge CX2388x Card. TunerId: 0x%02X",Eeprom[8+9]);
				if (Eeprom[8+9] < (sizeof(m_TunerHauppaugeAnalog) / sizeof(m_TunerHauppaugeAnalog[0]))) 
				{
					Tuner = m_TunerHauppaugeAnalog[Eeprom[8+9]];
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

