//
// $Id: I2CBus.h,v 1.2 2001-11-26 13:02:27 adcockj Exp $
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
// Revision 1.1  2001/11/25 02:03:21  ittarnavsky
// initial checkin of the new I2C code
//
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(__I2CBUS_H__)
#define __I2CBUS_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CI2CBus  
{
public:
    /**
    @return true if sucessful
    */
    virtual bool Read(
                        const BYTE *writeBuffer,
                        size_t writeBufferSize,
                        BYTE *readBuffer,
                        size_t readBufferSize
                     );
    
    virtual bool Write(const BYTE *writeBuffer, size_t writeBufferSize);

protected:
    virtual void Start()=0;
    virtual void Stop()=0;
    virtual bool Write(BYTE byte)=0;
    virtual BYTE Read(bool last=true)=0;
    virtual bool GetAcknowledge()=0;
    virtual void Sleep()=0;
    virtual void SendACK()=0;
    virtual void SendNAK()=0;
};

#endif // !defined(__I2CBUS_H__)
