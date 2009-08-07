/////////////////////////////////////////////////////////////////////////////
// $Id$
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


/**
 * @file SAA7134Card.cpp CSAA7134Card Implementation (Tuner)
 */

#include "stdafx.h"

#ifdef WANT_SAA713X_SUPPORT

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
#include "TDA8275.h"
#include "TDA8290.h"
#include "DebugLog.h"

using namespace std;

BOOL CSAA7134Card::InitTuner(eTunerId tunerId)
{
    // clean up if we get called twice
    if (m_Tuner.IsValid())
    {
        m_Tuner = 0L;
    }

    // Create a tuner object for the selected tuner.
    switch (tunerId)
    {
    case TUNER_AUTODETECT:
    case TUNER_USER_SETUP:
    case TUNER_ABSENT:
        m_Tuner = new CNoTuner();
        m_TunerType = _T("None ");
        // There is nothing more to do with this tuner.
        return TRUE;

    case TUNER_MT2032:
        m_Tuner = new CMT2032(VIDEOFORMAT_NTSC_M);
        m_TunerType = _T("MT2032 ");
        break;
    case TUNER_MT2032_PAL:
        m_Tuner = new CMT2032(VIDEOFORMAT_PAL_B);
        m_TunerType = _T("MT2032 ");
        break;
    case TUNER_MT2050:
        m_Tuner = new CMT2050(VIDEOFORMAT_NTSC_M);
        m_TunerType = _T("MT2050 ");
        break;
    case TUNER_MT2050_PAL:
        m_Tuner = new CMT2050(VIDEOFORMAT_PAL_B);
        m_TunerType = _T("MT2050 ");
        break;
    case TUNER_TDA8275:
        m_Tuner = new CTDA8275();
        m_TunerType = _T("TDA8275 ");
        break;

    default:
        // The rest are handled by CGenericTuner.
        m_Tuner = new CGenericTuner(tunerId);
        m_TunerType = _T("Generic ");
        break;
    }

    // Look for possible external IF demodulator
    SmartPtr<IExternalIFDemodulator> pExternalIFDemodulator;

    // TDA8275s are paired with a TDA8290.
    if (tunerId == TUNER_TDA8275)
    {
        // Have a TDA8290 object detected and created.
        pExternalIFDemodulator = CTDA8290::CreateDetectedTDA8290(m_I2CBus.GetRawPointer());
    }

    if (!pExternalIFDemodulator.IsValid())
    {
        // bUseTDA9887 is the setting in SAA713xCards.ini.
        if (m_SAA713xCards[m_CardType].bUseTDA9887)
        {
            // Have a TDA9887 object detected and created.
            SmartPtr<CTDA9887Ex> pTDA9887Ex(CTDA9887Ex::CreateDetectedTDA9887Ex(m_I2CBus.GetRawPointer()));

            // If a TDA9887 was found.
            if (pTDA9887Ex.IsValid())
            {
                // Set card specific modes that were parsed from SAA713xCards.ini.
                size_t count = m_SAA713xCards[m_CardType].tda9887Modes.size();
                for (size_t i = 0; i < count; i++)
                {
                    pTDA9887Ex->SetModes(&m_SAA713xCards[m_CardType].tda9887Modes[i]);
                }

                // Found a valid external IF demodulator.
                pExternalIFDemodulator = pTDA9887Ex.DynamicCast<IExternalIFDemodulator>();
            }
        }
    }

    eVideoFormat videoFormat = m_Tuner->GetDefaultVideoFormat();

    if (pExternalIFDemodulator.IsValid())
    {
        // Attach the IF demodulator to the tuner.
        m_Tuner->AttachIFDem(pExternalIFDemodulator);
        // Let the IF demodulator know of pre-initialization.
        pExternalIFDemodulator->Init(TRUE, videoFormat);
    }

    BOOL bFoundTuner = FALSE;

    // Scan the I2C bus addresses 0xC0 - 0xCF for tuners.
    BYTE test = IsTEA5767PresentAtC0(m_I2CBus.GetRawPointer()) ? 0xC2 : 0xC0;
    for ( ; test < 0xCF; test += 0x02)
    {
        if (m_I2CBus->Write(&test, sizeof(test)))
        {
            m_Tuner->SetI2CBus(m_I2CBus.GetRawPointer(), test>>1);

            if (m_Tuner->InitializeTuner())
            {
                bFoundTuner = TRUE;
                m_TunerType += _T("@ I2C address 0x");
                tostringstream oss;
                oss << hex << setw(2) << setfill((TCHAR)'0') << test;
                m_TunerType += oss.str();
                LOG(1,_T("Tuner: Found at I2C address 0x%02x"), test);
                break;
            }
        }
    }

    if (pExternalIFDemodulator.IsValid())
    {
        // End initialization
        pExternalIFDemodulator->Init(FALSE, videoFormat);
    }

    if (!bFoundTuner)
    {
        LOG(1,_T("Tuner: No tuner found at I2C addresses 0xC0-0xCF"));

        m_Tuner = new CNoTuner();
        m_TunerType = _T("None ");
    }
    return bFoundTuner;
}


SmartPtr<ITuner> CSAA7134Card::GetTuner()
{
    return m_Tuner.DynamicCast<ITuner>();
}


tstring CSAA7134Card::GetTunerType()
{
    return m_TunerType;
}

#endif