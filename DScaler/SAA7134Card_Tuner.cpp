/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Card_Tuner.cpp,v 1.12 2004-04-19 20:38:38 adcockj Exp $
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
#include "DebugLog.h"

BOOL CSAA7134Card::InitTuner(eTunerId tunerId)
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
    BYTE IFDemDeviceAddress[2] = {0,0};
    eVideoFormat videoFormat = m_Tuner->GetDefaultVideoFormat();
    int NumAddressesToSearch = 2;

    if(LookForIFDemod)
    {        
        CTDA9887 *pTDA9887 = new CTDA9887();
        pExternalIFDemodulator = pTDA9887;
        IFDemDeviceAddress[0] = I2C_TDA9887_0;
        IFDemDeviceAddress[1] = I2C_TDA9887_1;
    }
        
    // Detect and attach IF demodulator to the tuner
    //  or delete the demodulator if the chip doesn't exist.
    if (pExternalIFDemodulator != NULL)
    {
        for(int i(0); i < NumAddressesToSearch; ++i)
        {
            if (IFDemDeviceAddress[i] != 0)
            {
                // Attach I2C bus if the demodulator chip uses it
                pExternalIFDemodulator->Attach(m_I2CBus, IFDemDeviceAddress[i]);
            }
            if (pExternalIFDemodulator->Detect())
            {
                m_Tuner->AttachIFDem(pExternalIFDemodulator, TRUE);
                pExternalIFDemodulator->Init(TRUE, videoFormat);
                break;
            }
        }
        // if didn't find anything then
        // need to delete the instance
        if(i == NumAddressesToSearch)
        {            
            delete pExternalIFDemodulator;
            pExternalIFDemodulator = NULL;
        }
    }
 
    // Scan the I2C bus addresses 0xC0 - 0xCF for tuners
    BOOL bFoundTuner = FALSE;
    
    int kk = strlen(m_TunerType);
    for (BYTE test = 0xC0; test < 0xCF; test +=2)
    {
        if (m_I2CBus->Write(&test, sizeof(test)))
        {
            m_Tuner->Attach(m_I2CBus, test>>1);
            sprintf(m_TunerType + kk, "@ I2C address 0x%02X", test);
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

ITuner* CSAA7134Card::GetTuner() const
{
    return m_Tuner;
}
