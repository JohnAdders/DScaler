/////////////////////////////////////////////////////////////////////////////
// $Id: CX2388xCard_Tuner.cpp,v 1.1 2004-12-20 18:55:35 to_see Exp $
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

BOOL CCX2388xCard::InitTuner(eTunerId tunerId)
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
    case TUNER_AUTODETECT:
    case TUNER_USER_SETUP:
    case TUNER_ABSENT:
        m_Tuner = new CNoTuner();
        strcpy(m_TunerType, "None ");
        break;
    case TUNER_PHILIPS_FM1216ME_MK3:
	case TUNER_PHILIPS_4IN1:
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
    eVideoFormat videoFormat = m_Tuner->GetDefaultVideoFormat();

    if(LookForIFDemod)
    {        
        CTDA9887* pTDA9887 = NULL;

		switch (m_CardType)
        {        
        case CX2388xCARD_MSI_TV_ANYWHERE_MASTER_PAL:
            pTDA9887 = new CTDA9887(TDA9887_MSI_TV_ANYWHERE_MASTER);
            break;

        case CX2388xCARD_LEADTEK_WINFAST_EXPERT:
            pTDA9887 = new CTDA9887(TDA9887_LEADTEK_WINFAST_EXPERT);
            break;

		case CX2388xCARD_ATI_WONDER_PRO:
            pTDA9887 = new CTDA9887(TDA9887_ATI_TV_WONDER_PRO);
            break;

		case CX2388xCARD_AVERTV_303:
            pTDA9887 = new CTDA9887(TDA9887_AVERTV_303);
            break;

        default:
            // detect TDA9887 with standard settings
            pTDA9887 = new CTDA9887();
            break;
        }

		// Detect to make sure an IF demodulator exists.
		if(pTDA9887->DetectAttach(m_I2CBus))
		{
			// Found a valid external IF demodulator.
			pExternalIFDemodulator = pTDA9887;
		}

		else
		{
			// A TDA9887 device wasn't detected.
			delete pTDA9887;
		}
    }
        
    if (pExternalIFDemodulator != NULL)
    {
		// Attach the IF demodulator to the tuner.
		m_Tuner->AttachIFDem(pExternalIFDemodulator, TRUE);
		// Let the IF demodulator know of pre-initialization.
		pExternalIFDemodulator->Init(TRUE, videoFormat);
    }
                
    // Scan the I2C bus addresses 0xC0 - 0xCF for tuners
    BOOL bFoundTuner = FALSE;

    int kk = strlen(m_TunerType);
	BYTE StartAddress;
    //there check what this is not TV@nywhere Master, which have TEA5767 at 0xC0
	if (m_CardType != CX2388xCARD_MSI_TV_ANYWHERE_MASTER_PAL)
	{
		StartAddress = 0xC0;
	}
	else
	{
		StartAddress = 0xC2;
	}

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
