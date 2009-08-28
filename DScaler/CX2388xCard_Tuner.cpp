/////////////////////////////////////////////////////////////////////////////
// $Id$
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

#include "stdafx.h"

#ifdef WANT_CX2388X_SUPPORT

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

using namespace std;

BOOL CCX2388xCard::InitTuner(eTunerId tunerId)
{
    // clean up if we get called twice
    if(m_Tuner.IsValid())
    {
        m_Tuner = 0L;
    }

    switch (tunerId)
    {
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
    case TUNER_AUTODETECT:
    case TUNER_USER_SETUP:
    case TUNER_ABSENT:
        m_Tuner = new CNoTuner();
        m_TunerType = _T("None ");
        break;
    default:
        m_Tuner = new CGenericTuner(tunerId);
        m_TunerType = _T("Generic ");
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
    SmartPtr<IExternalIFDemodulator> pExternalIFDemodulator;

    // TDA8275s are paired with a TDA8290.
    if (tunerId == TUNER_TDA8275)
    {
        // Have a TDA8290 object detected and created.
        pExternalIFDemodulator = CTDA8290::CreateDetectedTDA8290(m_I2CBus.GetRawPointer());
    }

    if (!pExternalIFDemodulator.IsValid())
    {
        // bUseTDA9887 is the setting in CX2388xCards.ini.
        if (m_CX2388xCards[m_CardType].bUseTDA9887)
        {
            // Have a TDA9887 object detected and created.
            SmartPtr<CTDA9887Ex> pTDA9887(CTDA9887Ex::CreateDetectedTDA9887Ex(m_I2CBus.GetRawPointer()));

            // If a TDA9887 was found.
            if (pTDA9887.IsValid())
            {
                // Set card specific modes that were parsed from CX2388xCards.ini.
                size_t count = m_CX2388xCards[m_CardType].tda9887Modes.size();
                for (size_t i = 0; i < count; i++)
                {
                    pTDA9887->SetModes(&m_CX2388xCards[m_CardType].tda9887Modes[i]);
                }

                // Found a valid external IF demodulator.
                pExternalIFDemodulator = DynamicPtrCast<IExternalIFDemodulator>(pTDA9887);
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

    // Scan the I2C bus addresses for tuners
    BOOL bFoundTuner = FALSE;

    // Scan the I2C bus addresses 0xC0 - 0xCF for tuners.
    BYTE test = IsTEA5767PresentAtC0(m_I2CBus.GetRawPointer()) ? 0xC2 : 0xC0;
    for ( ; test < 0xCF; test += 0x02)
    {
        if (m_I2CBus->Write(&test, sizeof(test)))
        {
            m_Tuner->SetI2CBus(m_I2CBus.GetRawPointer(), test>>1);

            // Initialize the tuner.
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
        //End initialization
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

SmartPtr<ITuner> CCX2388xCard::GetTuner()
{
    return DynamicPtrCast<ITuner>(m_Tuner);
}

tstring CCX2388xCard::GetTunerType()
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

        // Note: tstring in card-ini is _T("Hauppauge WinTV 34xxx models")
        const TCHAR* pszCardHauppaugeAnalog = _T("Hauppauge WinTV 34");

        // Bytes 0:7 are used for PCI SubId, data starts at byte 8
        #define CX_EEPROM_OFFSET 8

        if(_tcsncmp(pCard->szName, pszCardHauppaugeAnalog, _tcslen(pszCardHauppaugeAnalog)) == 0)
        {
            if (Eeprom[CX_EEPROM_OFFSET + 0] != 0x84 || Eeprom[CX_EEPROM_OFFSET + 2] != 0)
            {
                LOG(1, _T("AutoDetectTuner: Hauppauge CX2388x Card fails."));
            }
            else
            {
                LOG(2, _T("AutoDetectTuner: Hauppauge CX2388x Card. TunerId: 0x%02X"),Eeprom[CX_EEPROM_OFFSET + 9]);
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

#endif // WANT_CX2388X_SUPPORT
