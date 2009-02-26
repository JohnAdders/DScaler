//
// $Id$
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 John Adcock.  All rights reserved.
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
 * @file btwdmprop.h various ir related stuff from btwincap driver
 */

#ifdef WANT_BT8X8_SUPPORT

#include <ks.h>

//{C44A1A10-4A38-11D2-8328-006097BA83AB}
GUID PROPSETID_IR =
    {0xC44A1A10, 0x4A38, 0x11D2, {0x83, 0x28, 0x00, 0x60, 0x97, 0xBA, 0x83, 0xAB}};

#define KSPROPERTY_IR_GETCODE                0x20000001    /* to read the IR remote code */
#define KSPROPERTY_IR_CAPS                    0x2000000f    /* to get IR caps */

typedef struct
{
    KSPROPERTY Property;
    ULONG Code;
} KSPROPERTY_IR_GETCODE_S, *PKSPROPERTY_IR_GETCODE_S;

#define KSPROPERTY_IR_CAPS_AVAILABLE        0x0001        /* means that IR control is available */

typedef struct
{
    KSPROPERTY Property;
    ULONG Caps;                // Capabilities (KSPROPERTY_IR_CAPS_...)
} KSPROPERTY_IR_CAPS_S, *PKSPROPERTY_IR_CAPS_S;

#endif // WANT_BT8X8_SUPPORT