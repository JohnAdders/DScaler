/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Card_Tuner.cpp,v 1.10 2002-10-07 20:31:59 kooiman Exp $
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

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "BT848Card.h"
#include "BT848_Defines.h"
#include "NoTuner.h"
#include "MT2032.h"
#include "GenericTuner.h"
#include "DebugLog.h"


BOOL CBT848Card::InitTuner(eTunerId tunerId)
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
        m_Tuner = new CMT2032(VIDEOFORMAT_NTSC_M, m_CardType);
        strcpy(m_TunerType, "MT2032 ");
        break;
    case TUNER_MT2032_PAL:
        m_Tuner = new CMT2032(VIDEOFORMAT_PAL_B, m_CardType);
        strcpy(m_TunerType, "MT2032 ");
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
    
    if (tunerId != TUNER_ABSENT) 
    {                
        BOOL bFoundTuner = FALSE;

        switch (m_CardType)
        {
          case TVCARD_MIRO:
          case TVCARD_MIROPRO:
          case TVCARD_PINNACLERAVE:
          case TVCARD_PINNACLEPRO:            
            if ((tunerId == TUNER_MT2032) || (tunerId == TUNER_MT2032_PAL))
            {
                // If Card has TDA9887, try to instruct it to enable the tuner
				BYTE tda9887_tuneron[] = {0x86, 0x00, 0xD4, 0x70, 0x49};
				BYTE tda9887_tuneroff[] = {0x86, 0x00, 0x54, 0x70, 0x49};
                
				m_I2CBus->Write(tda9887_tuneron, 5);
				m_I2CBus->Write(tda9887_tuneroff, 5);
				m_I2CBus->Write(tda9887_tuneron, 5);
				m_I2CBus->Write(tda9887_tuneron, 5);
            }
        }

        int kk = strlen(m_TunerType);
        for (BYTE test = 0xC0; test < 0xCF; test +=2)
        {
            if (m_I2CBus->Write(&test, sizeof(test)))
            {
                m_Tuner->Attach(m_I2CBus, test>>1);
                sprintf(m_TunerType + kk, " @I2C@0x%02x", test);
                bFoundTuner = TRUE;
                LOG(1,"Tuner: Found at I2C address 0x%02x",test);
                break;
            }
       }


       switch (m_CardType)
       {
          case TVCARD_MIRO:
          case TVCARD_MIROPRO:
          case TVCARD_PINNACLERAVE:
          case TVCARD_PINNACLEPRO:            
            if (tunerId == TUNER_MT2032)
            {
                // If Card has TDA9887, try to instruct it to disable the tuner
				BYTE tda9887_tuneroff[] = {0x86, 0x00, 0x54, 0x70, 0x49};
                
				m_I2CBus->Write(tda9887_tuneroff, 5);
            }
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

    return TRUE;
}

ITuner* CBT848Card::GetTuner() const
{
    return m_Tuner;
}
