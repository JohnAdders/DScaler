/////////////////////////////////////////////////////////////////////////////
// $Id: TEA5767.h,v 1.1 2005-01-13 19:44:17 to_see Exp $
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 Torsten Seeboth.  All rights reserved.
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
// CVS Log
//
// $Log: not supported by cvs2svn $
//////////////////////////////////////////////////////////////////////////////

/**
* @file TDA5767.h TEA5767 chip related functions
*/

#ifndef __TEA5767_H___
#define __TEA5767_H___

#include "I2CBus.h"

// Returns TRUE if a TEA5767 chip is found at I2C address 0xC0.
BOOL IsTEA5767PresentAtC0(IN CI2CBus* i2cBus);


#endif
