/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Card.cpp,v 1.1 2004-10-30 19:30:22 to_see Exp $
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
// CVS Log
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SAA7134_Defines.h"
#include "SAA7134Card.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CSAA7134Card::CSAA7134Card(CHardwareDriver* pDriver) : CPCICard(pDriver)
{
    m_I2CBus = new CSAA7134I2CBus(this);
}

CSAA7134Card::~CSAA7134Card()
{
    delete m_I2CBus;
}

BYTE CSAA7134Card::GetI2CStatus()
{
    return ReadByte(SAA7134_I2C_ATTR_STATUS) & 0x0F;
}

void CSAA7134Card::SetI2CStatus(BYTE Status)
{
    MaskDataByte(SAA7134_I2C_ATTR_STATUS, Status, 0x0F);
}

void CSAA7134Card::SetI2CCommand(BYTE Command)
{
    MaskDataByte(SAA7134_I2C_ATTR_STATUS, Command, 0xC0);
}

void CSAA7134Card::SetI2CData(BYTE Data)
{
    WriteByte(SAA7134_I2C_DATA, Data);
}

BYTE CSAA7134Card::GetI2CData()
{
    return ReadByte(SAA7134_I2C_DATA);
}

bool CSAA7134Card::WriteToI2C(const BYTE *writeBuffer, size_t writeBufferSize)
{
	if(m_I2CBus->Write(writeBuffer, writeBufferSize))
	{
		return true;
	}

	return false;
}
