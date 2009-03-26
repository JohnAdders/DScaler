/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004 Torsten Seeboth. All rights reserved.
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
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "I2CBusForLineInterface.h"

CI2CBusForLineInterface::CI2CBusForLineInterface(II2CLineInterface* lineInterface)
{
    ASSERT(lineInterface != 0);
    this->m_LineInterface = lineInterface;
}

void CI2CBusForLineInterface::Sleep()
{
    ASSERT(m_LineInterface != 0);
    m_LineInterface->Sleep();
}

void CI2CBusForLineInterface::SetSDALo()
{
    m_LineInterface->SetSDA(false);
    Sleep();
}

void CI2CBusForLineInterface::SetSDAHi()
{
    m_LineInterface->SetSDA(true);
    Sleep();
}

void CI2CBusForLineInterface::SetSCLLo()
{
    m_LineInterface->SetSCL(false);
    Sleep();
}

void CI2CBusForLineInterface::SetSCLHi()
{
    m_LineInterface->SetSCL(true);
    Sleep();

    while (!m_LineInterface->GetSCL())
    {
        /* the hw knows how to read the clock line,
         * so we wait until it actually gets high.
         * This is safer as some chips may hold it low
         * while they are processing data internally.
         */
        m_LineInterface->SetSCL(true);
        /// \todo FIXME yield here/timeout
    }
}

void CI2CBusForLineInterface::Start()
{
    ASSERT(m_LineInterface != 0);

    // I2C start: SDA 1 -> 0 with SCL = 1
    // SDA   ^^^\___
    // SCL ___/^^^\_____
    //
    SetSCLLo();
    SetSDAHi();
    SetSCLHi();
    SetSDALo();
    SetSCLLo();
}

void CI2CBusForLineInterface::Stop()
{
    ASSERT(m_LineInterface != 0);

    // I2C stop: SDA 0 -> 1 with SCL = 1
    // SDA    ___/^^^
    // SCL ____/^^^
    SetSCLLo();
    SetSDALo();
    SetSCLHi();
    SetSDAHi();
}

bool CI2CBusForLineInterface::GetAcknowledge()
{
    ASSERT(m_LineInterface != 0);

    SetSCLLo();
    SetSDAHi();
    // SDA = 0 means the slave ACK'd
    bool result = m_LineInterface->GetSDA();
    SetSCLHi();
    SetSCLLo();
    return !result;
}

bool CI2CBusForLineInterface::Write(BYTE byte)
{
    ASSERT(m_LineInterface != 0);

    for(BYTE mask = 0x80; mask > 0; mask /= 2)
    {
        SetSCLLo();

        if ((byte & mask) != 0)
        {
            SetSDAHi();
        }

        else
        {
            SetSDALo();
        }

        SetSCLHi();
    }

    return GetAcknowledge();
}

unsigned char CI2CBusForLineInterface::Read(bool last)
{
    ASSERT(m_LineInterface != 0);

    BYTE result = 0;

    SetSDAHi();

    for (BYTE mask = 0x80; mask > 0; mask /= 2)
    {
        SetSCLLo();
        SetSCLHi();

        if (m_LineInterface->GetSDA())
        {
            result |= mask;
        }
    }

    if (last)
    {
        SendNAK();
    }

    else
    {
        SendACK();
    }

    return result;
}

void CI2CBusForLineInterface::SendNAK()
{
    ASSERT(m_LineInterface != 0);

    SetSCLLo();
    SetSDAHi();
    SetSCLHi();
    SetSCLLo();
    SetSDALo();
}

void CI2CBusForLineInterface::SendACK()
{
    ASSERT(m_LineInterface != 0);

    SetSCLLo();
    SetSDALo();
    SetSCLHi();
    SetSCLLo();
    SetSDAHi();
}