/////////////////////////////////////////////////////////////////////////////
// $Id: DVBTCard.cpp,v 1.1 2001-11-02 16:30:07 adcockj Exp $
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
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DVBTCard.h"

CDVBTCard::CDVBTCard(CHardwareDriver* pDriver) :
	CPCICard(pDriver)

{
    ;
}

CDVBTCard::~CDVBTCard()
{
    ClosePCICard();
}

BOOL CDVBTCard::FindCard(int CardIndex)
{
    return OpenPCICard(0x1131, 0x7146, CardIndex);
}

void CDVBTCard::CloseCard()
{
    ClosePCICard();
}


void CDVBTCard::I2C_SetLine(BOOL bCtrl, BOOL bData)
{
}

BOOL CDVBTCard::I2C_GetLine()
{
    return FALSE;
}

BYTE CDVBTCard::I2C_Read(BYTE nAddr)
{
    return 0;
}

BOOL CDVBTCard::I2C_Write(BYTE nAddr, BYTE nData1, BYTE nData2, BOOL bSendBoth)
{
    return FALSE;
}
