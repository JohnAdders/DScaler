//
// $Id: I2CBusForLineInterface.h,v 1.1 2001-11-25 02:03:21 ittarnavsky Exp $
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
private:
    II2CLineInterface *m_LineInterface;
    void SetSDALo();
    void SetSDAHi();
    void SetSCLLo();
    void SetSCLHi();

protected:
    void Start();
    void Stop();
    bool Write(BYTE byte);
    BYTE Read(bool last=true);
    bool GetAcknowledge();
    void SendACK();
    void SendNAK();

    void Sleep()
    {
        ASSERT(m_LineInterface != 0);
        m_LineInterface->Sleep();
    }

public:
    CI2CBusForLineInterface(II2CLineInterface *lineInterface)
    {
        ASSERT(lineInterface != 0);
        this->m_LineInterface = lineInterface;
    }
};

#endif // !defined(__I2CBUSFORLINEINTERFACE_H__)
