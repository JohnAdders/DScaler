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
 * @file i2cbusforlineinterface.h i2cbusforlineinterface Header
 */

#if !defined(__I2CBUSFORLINEINTERFACE_H__)
#define __I2CBUSFORLINEINTERFACE_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "I2CBus.h"
#include "I2CLineInterface.h"

/** An standard implemenation of the I2C interfac
    where the SDA and SCL lines are controled directly
*/
class CI2CBusForLineInterface :public CI2CBus
{
public:
    CI2CBusForLineInterface(II2CLineInterface *lineInterface);

protected:
    void Start();
    void Stop();
    BOOL Write(BYTE byte);
    BYTE Read(BOOL last=TRUE);
    BOOL GetAcknowledge();
    void SendACK();
    void SendNAK();
    void Sleep();
    void Lock();
    void Unlock();

private:
    void SetSDALo();
    void SetSDAHi();
    void SetSCLLo();
    void SetSCLHi();

private:
    II2CLineInterface *m_LineInterface;
};

#endif // !defined(__I2CBUSFORLINEINTERFACE_H__)
