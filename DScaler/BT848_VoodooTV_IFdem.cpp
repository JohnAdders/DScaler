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
 * @file BT848_VoodooTV_IFdem.cpp CPreTuneVoodooFM Implementation
 */

#include "stdafx.h"

#ifdef WANT_BT8X8_SUPPORT

#include "BT848_VoodooTV_IFdem.h"


CPreTuneVoodooFM::CPreTuneVoodooFM(CBT848Card* pBT848Card)
{
    m_pBT848Card = pBT848Card;
}

bool CPreTuneVoodooFM::Detect() 
{ 
    if ((m_pBT848Card != NULL) && 
        ((m_pBT848Card->GetCardType() == TVCARD_VOODOOTV_200)  ||
         (m_pBT848Card->GetCardType() == TVCARD_VOODOOTV_FM)))
    {
        return true; 
    }
    return false;
}

void CPreTuneVoodooFM::TunerSet(bool bPreSet, eVideoFormat videoFormat)
{
    if (bPreSet && (m_pBT848Card!=NULL))
    {
        //Set the demodulator using the GPIO port
        
        ULONG Val;
        m_pBT848Card->SetGPOE( m_pBT848Card->GetGPOE() | (1<<16));
        Val = m_pBT848Card->GetGPDATA();
            
        switch (videoFormat)
        {
            case VIDEOFORMAT_PAL_N_COMBO:
            case VIDEOFORMAT_NTSC_M:
            case VIDEOFORMAT_NTSC_M_Japan:
            case VIDEOFORMAT_NTSC_50:
                Val |= (1<<16);
                break;
            default:
                Val &= ~(1<<16);
        }

        m_pBT848Card->SetGPDATA(Val);
    }
}

#endif // WANT_BT8X8_SUPPORT