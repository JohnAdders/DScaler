//
// $Id$
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

void ITuner::AttachIFDem(IExternalIFDemodulator* pExternalIFDemodulator, BOOL bFreeOnDestruction)
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

BOOL IExternalIFDemodulator::SetDetectedI2CAddress(IN CI2CBus* i2cBus)
{
    BYTE addresses = GetDefaultAddress();
    return SetDetectedI2CAddress(i2cBus, &addresses, 1);
}

BOOL IExternalIFDemodulator::SetDetectedI2CAddress(IN CI2CBus* i2cBus,
                                                   IN BYTE* addresses, IN size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        if (addresses[i] != 0x00)
        {
            SetI2CBus(i2cBus, addresses[i]);

            if (Detect())
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

