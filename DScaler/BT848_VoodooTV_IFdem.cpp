#include "stdafx.h"

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

