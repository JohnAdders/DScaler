/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Card_Tuner.cpp,v 1.5 2001-11-25 01:58:34 ittarnavsky Exp $
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


BOOL CBT848Card::InitTuner(eTunerId tunerId)
{
    switch (tunerId)
    {
    case TUNER_MT2032:
            m_Tuner = new CMT2032();
            break;
    case TUNER_AUTODETECT:
    case TUNER_USER_SETUP:
    case TUNER_ABSENT:
            m_Tuner = new CNoTuner();
            break;
    default:
            m_Tuner = new CGenericTuner(tunerId);
            break;
    }
    if (tunerId != TUNER_ABSENT) 
    {
        for (BYTE test = 0xC0; test < 0xCF; test++)
        {
            if (m_I2CBus->Write(&test, sizeof(test)))
            {
                m_Tuner->Attach(m_I2CBus, test>>1);
                sprintf(m_TunerStatus, "Tuner@I2C@0x%02x", test);
                break;
            }
        }
    }
    return TRUE;
}