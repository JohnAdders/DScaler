//
// $Id$
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

/**
 * @file mt2032.h mt2032 Header
 */

#if !defined(__MT2032_H__)
#define __MT2032_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ITuner.h"
#include "BT848Card.h"

/**  Allows control of the MT2032 Tuner
*/
class CMT2032: public II2CTuner
{
public:
    CMT2032(eVideoFormat DefaultVideoFormat);
    WORD GetVersion();
    WORD GetVendor();

    // from ITuner
    eTunerId GetTunerId();
    eVideoFormat GetDefaultVideoFormat();
    BOOL HasRadio() const;
    BOOL SetRadioFrequency(long nFrequency);
    BOOL SetTVFrequency(long nFrequency, eVideoFormat videoFormat);

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
    int SpurCheck(int f1, int f2, int spectrum_from, int spectrum_to);
    int ComputeFreq(int rfin, int if1, int if2, int spectrum_from, int spectrum_to,
        unsigned char *buf, int *ret_sel, int xogc);
    int CheckLOLock();
    int OptimizeVCO(int sel, int lock);
    void SetIFFreq(int rfin, int if1, int if2, int from, int to, eVideoFormat videoFormat);

    void PrepareTDA9887(BOOL bPrepare, eVideoFormat videoFormat);
    void PrepareVoodooTV(BOOL bPrepare, eVideoFormat videoFormat);

private:
    int  m_XOGC;    // holds the value of XOGC register after init
    BOOL m_Initialized;
    eVideoFormat m_DefaultVideoFormat;
    long m_Frequency;
    BOOL m_Locked;
};

#endif // !defined(__MT2032_H__)
