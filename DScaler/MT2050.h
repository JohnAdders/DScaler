/////////////////////////////////////////////////////////////////////////////
//
// MIDIMaker midimaker@yandex.ru
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
/////////////////////////////////////////////////////////////////////////////

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
    int SpurCheck(int flos1, int flos2, int fifbw, int fout);
    void SetIFFreq(int rfin, int if1, int if2, eVideoFormat videoFormat);

    void PrepareTDA9887(BOOL bPrepare, eVideoFormat videoFormat);
    void PrepareVoodooTV(BOOL bPrepare, eVideoFormat videoFormat);

private:
    BOOL m_Initialized;
    eVideoFormat m_DefaultVideoFormat;
    long m_Frequency;
    BOOL m_Locked;
};

#endif // !defined(__MT2050_H__)
