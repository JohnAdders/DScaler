/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Card.h,v 1.1 2004-10-30 19:30:22 to_see Exp $
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

#if !defined(AFX_BT848CARD_H__05A58E2D_A725_456B_94F1_2B73901079CE__INCLUDED_)
#define AFX_BT848CARD_H__05A58E2D_A725_456B_94F1_2B73901079CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PCICard.h"
#include "I2CLineInterface.h"
#include "I2CBusForLineInterface.h"

class CBT848Card :	public CPCICard,
					public II2CLineInterface
{
public:
	bool WriteToI2C(const BYTE *writeBuffer, size_t writeBufferSize);
	CBT848Card(CHardwareDriver* pDriver);
	~CBT848Card();

private:
	bool GetSCL();
	bool GetSDA();
	void SetSCL(bool value);
	void SetSDA(bool value);

	void Sleep();
	void InitializeI2C();

	bool			m_I2CInitialized;
    CI2CBus*        m_I2CBus;
    DWORD			m_I2CSleepCycle;
    DWORD			m_I2CRegister;
};

#endif // !defined(AFX_BT848CARD_H__05A58E2D_A725_456B_94F1_2B73901079CE__INCLUDED_)
