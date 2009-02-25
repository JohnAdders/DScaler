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
// CVS Log
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MY2388XCARD_H__A8D4B201_8B94_498E_8958_D591C1D7449C__INCLUDED_)
#define AFX_MY2388XCARD_H__A8D4B201_8B94_498E_8958_D591C1D7449C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PCICard.h"
#include "I2CLineInterface.h"
#include "I2CBusForLineInterface.h"

class CCX2388xCard :	public CPCICard,
						public II2CLineInterface
{
public:
	bool WriteToI2C(const BYTE *writeBuffer, size_t writeBufferSize);
	CCX2388xCard(CHardwareDriver* pDriver);
	~CCX2388xCard();


private:
	bool GetSCL();
	bool GetSDA();
	void SetSCL(bool value);
	void SetSDA(bool value);

	void InitializeI2C();
	void Sleep();

	bool			m_I2CInitialized;
    CI2CBus*        m_I2CBus;
    DWORD			m_I2CSleepCycle;
    DWORD			m_I2CRegister;
};

#endif // !defined(AFX_MY2388XCARD_H__A8D4B201_8B94_498E_8958_D591C1D7449C__INCLUDED_)
