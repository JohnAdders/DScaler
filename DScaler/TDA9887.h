/////////////////////////////////////////////////////////////////////////////
// $Id: TDA9887.h,v 1.4 2004-05-16 19:45:08 to_see Exp $
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
//
// $Log: not supported by cvs2svn $
// Revision 1.3  2004/02/11 20:34:00  adcockj
// Support multiple locations of TDA9887 (thanks to Pityu)
//
// Revision 1.2  2003/10/27 10:39:54  adcockj
// Updated files for better doxygen compatability
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file TDA9887.h CTDA9887 Header
 */

#ifndef _TDA_9887_H_
#define _TDA_9887_H_

#include "TVFormats.h"
#include "I2CDevice.h"
#include "ITuner.h"

#define I2C_TDA9887_0				0x86
#define I2C_TDA9887_1				0x96

class CTDA9887 : public IExternalIFDemodulator
{
public:
    CTDA9887();
    ~CTDA9887();

    bool Detect();

    void Init(bool bPreInit, eVideoFormat videoFormat);
    void TunerSet(bool bPreSet, eVideoFormat videoFormat);
    
    eTunerAFCStatus GetAFCStatus(long &nFreqDeviation);
protected:
    // from CI2CDevice
    virtual BYTE GetDefaultAddress() const { return I2C_TDA9887_0; }
};

class CTDA9887Pinnacle : public CTDA9887
{
public:
    CTDA9887Pinnacle(int CardId);
    ~CTDA9887Pinnacle();

    void Init(bool bPreInit, eVideoFormat videoFormat);
    void TunerSet(bool bPreSet, eVideoFormat videoFormat);    
private:
    int m_CardId;
    eVideoFormat m_LastVideoFormat;
};

class CTDA9887MsiMaster : public CTDA9887
{
public:
    CTDA9887MsiMaster();
    ~CTDA9887MsiMaster();

    void TunerSet(bool bPreSet, eVideoFormat videoFormat);    
};

#endif
