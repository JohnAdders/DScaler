//
// $Id: GenericTuner.h,v 1.6 2002-10-26 15:37:57 adcockj Exp $
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
// Revision 1.5  2002/10/08 20:43:15  kooiman
// Added Automatic Frequency Control for tuners. Changed to Hz instead of multiple of 62500 Hz.
//
// Revision 1.4  2002/08/17 11:27:23  kooiman
// Fixed tuning for Temic 4046FM5 and Philips FQ1216ME.
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

#if !defined(__GENERICTUNER_H__)
#define __GENERICTUNER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ITuner.h"

/** Implemenation of a simple tuner connected via I2C
*/
class CGenericTuner : public II2CTuner
{
public:
    CGenericTuner(eTunerId tunerId);
    
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
    eTunerId m_TunerId;
    eVideoFormat m_VideoFormat;
    WORD m_Thresh1;
    WORD m_Thresh2;  
    BYTE m_VHF_L;
    BYTE m_VHF_H;
    BYTE m_UHF;
    BYTE m_Config; 
    WORD m_IFPCoff;
    long m_Frequency;
};


#endif // !defined(__GENERICTUNER_H__)
