/////////////////////////////////////////////////////////////////////////////
// $Id$
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

using namespace std;

BOOL CBT848Card::InitTuner(eTunerId tunerId)
{
    BOOL LookForIFDemod = FALSE;

    // clean up if we get called twice
    m_Tuner = 0L;

    switch (tunerId)
    {
    case TUNER_MT2032:
        m_Tuner = new CMT2032(VIDEOFORMAT_NTSC_M);
        LookForIFDemod = TRUE;
        m_TunerType = _T("MT2032 ");
        break;
    case TUNER_MT2032_PAL:
        m_Tuner = new CMT2032(VIDEOFORMAT_PAL_B);
        LookForIFDemod = TRUE;
        m_TunerType = _T("MT2032 ");
        break;
    case TUNER_MT2050:
        m_Tuner = new CMT2050(VIDEOFORMAT_NTSC_M);
        LookForIFDemod = TRUE;
        m_TunerType = _T("MT2050 ");
        break;
    case TUNER_MT2050_PAL:
        m_Tuner = new CMT2050(VIDEOFORMAT_PAL_B);
        LookForIFDemod = TRUE;
        m_TunerType = _T("MT2050 ");
        break;
    case TUNER_TDA8275:
        m_Tuner = new CTDA8275();
        m_TunerType = _T("TDA8275 ");
        LookForIFDemod = TRUE;
        break;
    case TUNER_AUTODETECT:
    case TUNER_USER_SETUP:
    case TUNER_ABSENT:
        m_Tuner = new CNoTuner();
        m_TunerType = _T("None ");
        break;
    case TUNER_PHILIPS_FM1216ME_MK3:
        LookForIFDemod = TRUE;
        // deliberate drop down
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
    if (pExternalIFDemodulator)
    {
        if (pExternalIFDemodulator->SetDetectedI2CAddress(m_I2CBus))
        {
            m_Tuner->AttachIFDem(pExternalIFDemodulator);
            pExternalIFDemodulator->Init(TRUE, videoFormat);
        }
        else
        {
            // if didn't find anything then
            // need to delete the instance
            pExternalIFDemodulator = 0L;
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
                m_TunerType += _T("@ I2C address 0x");
                tostringstream oss;
                oss << hex << setw(2) << setfill((TCHAR)'0') << test;
                m_TunerType += oss.str();
                LOG(1,_T("Tuner: Found at I2C address 0x%02x"), test);
                break;
            }
        }
    }

    if (pExternalIFDemodulator)
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

SmartPtr<ITuner> CBT848Card::GetTuner() const
{
    SmartPtr<ITuner> retVal(m_Tuner);
    return retVal;
}

#endif // WANT_BT8X8_SUPPORT