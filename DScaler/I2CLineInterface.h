//
// $Id: I2CLineInterface.h,v 1.4 2005-03-24 17:57:58 adcockj Exp $
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
// Revision 1.3  2003/10/27 10:39:52  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.2  2001/11/29 14:04:07  adcockj
// Added Javadoc comments
//
// Revision 1.1  2001/11/25 02:03:21  ittarnavsky
// initial checkin of the new I2C code
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file i2clineinterface.h i2clineinterface Header
 */

#if !defined(__I2CLINEINTERFACE_H__)
#define __I2CLINEINTERFACE_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/** Interface for an I2C bus where the SDA and SCL lines are
    controlled directly
*/
class II2CLineInterface  
{
public:
    virtual void SetSDA(bool value)=0;
    virtual void SetSCL(bool value)=0;
    virtual bool GetSDA()=0;
    virtual bool GetSCL()=0;
    virtual void Sleep()=0;
    virtual void I2CLock()=0;
    virtual void I2CUnlock()=0;
};

#endif // !defined(__I2CLINEINTERFACE_H__)
