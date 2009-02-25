//
// $Id$
//
/////////////////////////////////////////////////////////////////////////////
//
// copyleft 2002 itt@myself.com
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
// Revision 1.2  2003/10/27 10:39:50  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.1  2002/09/12 21:47:34  ittarnavsky
// Bt8x8 GPIO based AudioDecoders (actually sound channel selectors)
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file bt8x8gpioaudiodecoderterratv.h bt8x8gpioaudiodecoderterratv Header
 */

#ifdef WANT_BT8X8_SUPPORT

#if !defined(__BT8X8GPIOAUDIODECODERTERRATV_H__)
#define __BT8X8GPIOAUDIODECODERTERRATV_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Bt8x8GPIOAudioDecoder.h"

class CBt8x8GPIOAudioDecoderTerraTV : public CBt8x8GPIOAudioDecoder  
{
public:
    CBt8x8GPIOAudioDecoderTerraTV(CPCICard* pPCICard) :
        CBt8x8GPIOAudioDecoder(pPCICard, 0x180000, 0x180000, 0x000000, 0x080000) {}
};

#endif // !defined(__BT8X8GPIOAUDIODECODERTERRATV_H__)

#endif // #ifdef WANT_BT8X8_SUPPORT