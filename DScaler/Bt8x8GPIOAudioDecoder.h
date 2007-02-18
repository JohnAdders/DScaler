//
// $Id: Bt8x8GPIOAudioDecoder.h,v 1.3 2007-02-18 21:15:32 robmuller Exp $
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
 * @file bt8x8gpioaudiodecoder.h bt8x8gpioaudiodecoder Header
 */

#ifdef WANT_BT8X8_SUPPORT

#if !defined(__BT8X8GPIOAUDIODECODER_H__)
#define __BT8X8GPIOAUDIODECODER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PCICard.h"
#include "AudioDecoder.h"

class CBt8x8GPIOAudioDecoder : public CAudioDecoder  
{
private:
    CPCICard* m_pPCICard;
    const DWORD m_GPOE;
    const DWORD m_GPDATA_Mono;
    const DWORD m_GPDATA_Stereo;
    const DWORD m_GPDATA_Lang1;
    

public:
    CBt8x8GPIOAudioDecoder(CPCICard* pPCICard, const DWORD dwGPOE, const DWORD dwStereo, const DWORD dwMono, const DWORD dwLang1);

    virtual void SetSoundChannel(eSoundChannel soundChannel);
};

#endif // !defined(__BT8X8GPIOAUDIODECODER_H__)

#endif // #ifdef WANT_BT8X8_SUPPORT