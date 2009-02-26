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

#if !defined(AFX_SAA7134CARD_H__13F16573_35A0_403E_BCA0_A187A8E7F745__INCLUDED_)
#define AFX_SAA7134CARD_H__13F16573_35A0_403E_BCA0_A187A8E7F745__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PCICard.h"
#include "SAA7134I2CInterface.h"
#include "SAA7134I2CBus.h"

class CSAA7134Card : public CPCICard,  
                     public ISAA7134I2CInterface
{
public:
    bool WriteToI2C(const BYTE *writeBuffer, size_t writeBufferSize);
    CSAA7134Card(CHardwareDriver* pDriver);
    ~CSAA7134Card();

private:
    CI2CBus*         m_I2CBus;

private:
    BYTE GetI2CStatus();
    void SetI2CStatus(BYTE Status);
    void SetI2CCommand(BYTE Command);
    void SetI2CData(BYTE Data);
    BYTE GetI2CData();

};

#endif // !defined(AFX_SAA7134CARD_H__13F16573_35A0_403E_BCA0_A187A8E7F745__INCLUDED_)
