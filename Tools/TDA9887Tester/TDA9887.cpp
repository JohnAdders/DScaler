/////////////////////////////////////////////////////////////////////////////
// $Id: TDA9887.cpp,v 1.1 2004-10-30 19:30:22 to_see Exp $
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
#include "TDA9887.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CTDA9887::CTDA9887(CPCICard* pCard) :
	m_pPCICard(pCard),
	m_I2CAdress(0)
{
}

CTDA9887::~CTDA9887()
{
}

BOOL CTDA9887::Detect()
{
	#define I2C_TDA9887_0	0x86
	#define I2C_TDA9887_1	0x96

	BYTE bDetectBytes[]	= { 0x00, 0x00, 0x54, 0x70, 0x44 };
	BYTE bI2CAdress[]	= { I2C_TDA9887_0, I2C_TDA9887_1, NULL };
	BOOL bSucces		= FALSE;

/*
	Content of bDetectBytes[]:

	bDetectBytes[0] = I2C Address of TDA9887
	bDetectBytes[1] = I2C Subadress start of writing (=0)
	bDetectBytes[2] = Byte B
	bDetectBytes[3] = Byte C
	bDetectBytes[4] = Byte E
*/

	for(int i = 0; bI2CAdress[i]; i++)
	{
		bDetectBytes[0] = bI2CAdress[i];

		if(m_pPCICard->WriteToI2C(bDetectBytes, sizeof(bDetectBytes)))
		{
			bSucces = TRUE;
			m_I2CAdress = bI2CAdress[i];
			break;
		}
	}

	return bSucces;
}

const BYTE* CTDA9887::GetI2CAdress()
{
	return &m_I2CAdress;
}

bool CTDA9887::WriteControlBytes(const BYTE* pControlBytes)
{
    BYTE bBuffer[5];
	ZeroMemory(bBuffer, sizeof(bBuffer));

/*
	Content of bBuffer[]:

	bBuffer[0] = I2C Address of TDA9887
	bBuffer[1] = I2C Subadress start of writing (=0)
	bBuffer[2] = Byte B
	bBuffer[3] = Byte C
	bBuffer[4] = Byte E
*/
	
	bBuffer[0] = m_I2CAdress;
//	bBuffer[1] = 0; // not needed
	bBuffer[2] = pControlBytes[0];
	bBuffer[3] = pControlBytes[1];
	bBuffer[4] = pControlBytes[2];

	if(m_pPCICard->WriteToI2C(bBuffer, sizeof(bBuffer)))
	{
		return true;
	}

	return false;
}
