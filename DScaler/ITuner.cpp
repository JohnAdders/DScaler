//
// $Id: ITuner.cpp,v 1.9 2005-03-09 09:35:16 atnak Exp $
//
/////////////////////////////////////////////////////////////////////////////
//
// copyleft 2001 itt@myself.com
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: not supported by cvs2svn $
// Revision 1.8  2003/10/27 10:39:52  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.7  2003/02/06 21:27:41  ittarnavsky
// removed the tuner names from here
//
// Revision 1.6  2002/10/26 15:37:57  adcockj
// Made ITuner more abstract by removing inheritance from CI2CDevice
// New class II2CTuner created for tuners that are controled by I2C
//
// Revision 1.5  2002/10/16 21:42:36  kooiman
// Created seperate class for External IF Demodulator chips like TDA9887
//
// Revision 1.4  2002/09/04 11:58:45  kooiman
// Added new tuners & fix for new Pinnacle cards with MT2032 tuner.
//
// Revision 1.3  2002/08/03 17:57:52  kooiman
// Added new cards & tuners. Changed the tuner combobox into a sorted list.
//
// Revision 1.2  2002/01/16 19:16:20  adcockj
// added support for LG NTSC (TAPCH701P)
//
// Revision 1.1  2001/11/25 02:03:21  ittarnavsky
// initial checkin of the new I2C code
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file ITuner.cpp ITuner implementation
 */

#include "stdafx.h"
#include "ITuner.h"


//////////////////////////////////////////////////////////////////////////
// ITuner
//////////////////////////////////////////////////////////////////////////

ITuner::ITuner()
{
    m_ExternalIFDemodulator = NULL;
    m_bFreeIFDemodulatorOnDestruction = FALSE;
}

ITuner::~ITuner()
{
    if (m_bFreeIFDemodulatorOnDestruction && (m_ExternalIFDemodulator != NULL))
    {
        delete m_ExternalIFDemodulator;
    }
}

void ITuner::AttachIFDem(IExternalIFDemodulator* pExternalIFDemodulator, bool bFreeOnDestruction)
{
    m_ExternalIFDemodulator = pExternalIFDemodulator;
    m_bFreeIFDemodulatorOnDestruction = bFreeOnDestruction;
}


//////////////////////////////////////////////////////////////////////////
// II2CTuner
//////////////////////////////////////////////////////////////////////////

II2CTuner::II2CTuner()
{
}

II2CTuner::~II2CTuner()
{
}


//////////////////////////////////////////////////////////////////////////
// IExternalIFDemodulator
//////////////////////////////////////////////////////////////////////////

bool IExternalIFDemodulator::SetDetectedI2CAddress(IN CI2CBus* i2cBus)
{
    BYTE addresses = CI2CDevice::GetDefaultAddress();
    return SetDetectedI2CAddress(i2cBus, &addresses, 1);
}

bool IExternalIFDemodulator::SetDetectedI2CAddress(IN CI2CBus* i2cBus,
                                                   IN BYTE* addresses, IN size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        if (addresses[i] != 0x00)
        {
            SetI2CBus(i2cBus, addresses[i]);

            if (Detect())
            {
                return true;
            }
        }
    }
    return false;
}

