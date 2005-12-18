///////////////////////////////////////////////////////////////////////////////
// $Id: DScalerVersion.h,v 1.12 2005-12-18 20:37:19 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
// This header file is free software; you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details
///////////////////////////////////////////////////////////////////////////////

#ifndef __DSCALER_VERSION__
#define __DSCALER_VERSION__

// primary version definitions
// should be updated before each release
#define DSCALERVERSION_MAJOR 4
#define DSCALERVERSION_MINOR 1
#define DSCALERVERSION_REVISION 15

// need to have an extra macro for stingify otherwise we get the 
// name of the definition not the value
#define MAKESTRING(Stuff) 	#Stuff
#define MAKEVERSTRING(Maj, Min, Rev) 	MAKESTRING(Maj) "." MAKESTRING(Min) "." MAKESTRING(Rev)
#define MAKEVERSTRING_ZT(Maj, Min, Rev) 	MAKESTRING(Maj) "." MAKESTRING(Min) "." MAKESTRING(Rev) "\0"

#define VERSTRING MAKEVERSTRING(DSCALERVERSION_MAJOR, DSCALERVERSION_MINOR, DSCALERVERSION_REVISION)
#define VERSTRING_ZT MAKEVERSTRING_ZT(DSCALERVERSION_MAJOR, DSCALERVERSION_MINOR, DSCALERVERSION_REVISION)

#endif
