//
// $Id: GenericTuner.h,v 1.1 2001-11-25 02:03:21 ittarnavsky Exp $
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
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(__GENERICTUNER_H__)
#define __GENERICTUNER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ITuner.h"

class CGenericTuner : public ITuner  
{
private:
    eTunerId m_TunerId;
    eVideoFormat m_VideoFormat;
    WORD m_Thresh1;
    WORD m_Thresh2;  
    BYTE m_VHF_L;
    BYTE m_VHF_H;
    BYTE m_UHF;
    BYTE m_Config; 
    WORD m_IFPCoff;
    
protected:
    // from CI2CDevice
    virtual BYTE GetDefaultAddress()const
    {
        return 0xC0>>1;
    }
    
public:
    CGenericTuner(eTunerId tunerId);
    
    // from ITuner
    eTunerId GetTunerId()
    {
        return TUNER_MT2032;
    }
    eVideoFormat GetDefaultVideoFormat()
    {
        return FORMAT_NTSC; // FIXME
    }
    bool HasRadio()const
    {
        return true;
    }
    bool SetRadioFrequency(long nFrequency)
    {
        return true;
    }
    bool SetTVFrequency(long nFrequency, eVideoFormat videoFormat);
};


#endif // !defined(__GENERICTUNER_H__)
