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

#if !defined(__I2CBUSFORLINEINTERFACE_H__)
#define __I2CBUSFORLINEINTERFACE_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "I2CBus.h"
#include "I2CLineInterface.h"

class CI2CBusForLineInterface :public CI2CBus
{
public:
    CI2CBusForLineInterface(II2CLineInterface *lineInterface);

protected:
    void Start();
    void Stop();
    bool Write(BYTE byte);
    BYTE Read(bool last=true);
    bool GetAcknowledge();
    void SendACK();
    void SendNAK();
    void Sleep();

private:
    void SetSDALo();
    void SetSDAHi();
    void SetSCLLo();
    void SetSCLHi();

private:
    II2CLineInterface *m_LineInterface;
};

#endif // !defined(__I2CBUSFORLINEINTERFACE_H__)
