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
};

#endif // !defined(__I2CLINEINTERFACE_H__)
