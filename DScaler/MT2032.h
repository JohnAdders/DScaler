//
// $Id: MT2032.h,v 1.1 2001-11-25 02:03:21 ittarnavsky Exp $
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

#if !defined(__MT2032_H__)
#define __MT2032_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ITuner.h"

class CMT2032: public ITuner  
{
private:
    int m_XOGC;    // holds the value of XOGC register after init
    bool m_Initialized;
    void Initialize();
    BYTE GetRegister(BYTE reg);
    void SetRegister(BYTE reg, BYTE value);
    int SpurCheck(int f1, int f2, int spectrum_from, int spectrum_to);
    int ComputeFreq(int rfin, int if1, int if2, int spectrum_from, int spectrum_to,
        unsigned char *buf, int *ret_sel, int xogc);
    int CheckLOLock();
    int OptimizeVCO(int sel, int lock);
    void SetIFFreq(int rfin, int if1, int if2, int from, int to);
    
protected:
    // from CI2CDevice
    virtual BYTE GetDefaultAddress()const
    {
        return 0xC0>>1;
    }
    
public:
    WORD GetVersion();
    WORD GetVendor();
    
    // from CI2CDevice
    void Attach(CI2CBus *i2cBus, BYTE address=0)
    {
        CI2CDevice::Attach(i2cBus, address);
        m_Initialized = false;
    }
    
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

#endif // !defined(__MT2032_H__)
