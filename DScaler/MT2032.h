//
// $Id: MT2032.h,v 1.6 2002-10-08 20:43:16 kooiman Exp $
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
// Revision 1.5  2002/10/07 20:32:00  kooiman
// Added/fixed TDA9887 support for new Pinnacle cards
//
// Revision 1.4  2002/09/04 11:58:45  kooiman
// Added new tuners & fix for new Pinnacle cards with MT2032 tuner.
//
// Revision 1.3  2001/11/29 14:04:07  adcockj
// Added Javadoc comments
//
// Revision 1.2  2001/11/26 13:02:27  adcockj
// Bug Fixes and standards changes
//
// Revision 1.1  2001/11/25 02:03:21  ittarnavsky
// initial checkin of the new I2C code
//
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(__MT2032_H__)
#define __MT2032_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ITuner.h"

/**  Allows control of the MT2032 Tuner
*/
class CMT2032: public ITuner  
{
public:
    CMT2032(eVideoFormat DefaultVideoFormat, eTVCardId TVCardId);
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
    int SpurCheck(int f1, int f2, int spectrum_from, int spectrum_to);
    int ComputeFreq(int rfin, int if1, int if2, int spectrum_from, int spectrum_to,
        unsigned char *buf, int *ret_sel, int xogc);
    int CheckLOLock();
    int OptimizeVCO(int sel, int lock);
    void SetIFFreq(int rfin, int if1, int if2, int from, int to, eVideoFormat videoFormat);

    void PrepareTDA9887(bool bPrepare, eVideoFormat videoFormat);

private:
    int  m_XOGC;    // holds the value of XOGC register after init
    bool m_Initialized;
    eTVCardId m_TVCardId;
    eVideoFormat m_DefaultVideoFormat;
    long m_Frequency;
    bool m_Locked;
	bool m_HasTDA9887;
};

#endif // !defined(__MT2032_H__)
