//
// $Id: MT2050.h,v 1.1 2003-12-18 15:57:41 adcockj Exp $
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
/////////////////////////////////////////////////////////////////////////////

/**
 * @file MT2050.h MT2050 Header
 */

#if !defined(__MT2050_H__)
#define __MT2050_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ITuner.h"
#include "BT848Card.h"

/**  Allows control of the MT2050 Tuner
*/
class CMT2050: public II2CTuner
{
public:
    CMT2050(eVideoFormat DefaultVideoFormat);
    WORD GetVersion();
    WORD GetVendor();
    
    // from ITuner
    eTunerId GetTunerId();
    eVideoFormat GetDefaultVideoFormat();
    bool HasRadio() const;
    bool SetRadioFrequency(long nFrequency);
    bool SetTVFrequency(long nFrequency, eVideoFormat videoFormat);

    long GetFrequency();
    eTunerLocked IsLocked();
    eTunerAFCStatus GetAFCStatus(long &nFreqDeviation);

protected:
    // from CI2CDevice
    virtual BYTE GetDefaultAddress() const;

private:
    void Initialize();
    BYTE GetRegister(BYTE reg);
    void SetRegister(BYTE reg, BYTE value);
    int ComputeFreq(int rfin, int if2, unsigned char *buf);
    void SetIFFreq(int rfin, int if2, eVideoFormat videoFormat);

    void PrepareTDA9887(bool bPrepare, eVideoFormat videoFormat);
    void PrepareVoodooTV(bool bPrepare, eVideoFormat videoFormat);

private:
    bool m_Initialized;
    eVideoFormat m_DefaultVideoFormat;
    long m_Frequency;
};

#endif // !defined(__MT2050_H__)
