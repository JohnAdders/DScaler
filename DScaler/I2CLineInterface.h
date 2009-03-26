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
    virtual void SetSDA(BOOL value)=0;
    virtual void SetSCL(BOOL value)=0;
    virtual BOOL GetSDA()=0;
    virtual BOOL GetSCL()=0;
    virtual void Sleep()=0;
    virtual void I2CLock()=0;
    virtual void I2CUnlock()=0;
};

#endif // !defined(__I2CLINEINTERFACE_H__)
