//
// $Id: ITuner.h,v 1.8 2003-10-27 10:39:52 adcockj Exp $
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
// Revision 1.7  2003/02/06 21:27:41  ittarnavsky
// removed the tuner names from here
//
// Revision 1.6  2003/02/06 19:47:29  ittarnavsky
// removed dependency on BT848_Defines.h, introduced new TunerID.h
//
// Revision 1.5  2002/10/26 15:37:57  adcockj
// Made ITuner more abstract by removing inheritance from CI2CDevice
// New class II2CTuner created for tuners that are controled by I2C
//
// Revision 1.4  2002/10/16 21:42:36  kooiman
// Created seperate class for External IF Demodulator chips like TDA9887
//
// Revision 1.3  2002/10/08 20:43:16  kooiman
// Added Automatic Frequency Control for tuners. Changed to Hz instead of multiple of 62500 Hz.
//
// Revision 1.2  2001/11/29 14:04:07  adcockj
// Added Javadoc comments
//
// Revision 1.1  2001/11/25 02:03:21  ittarnavsky
// initial checkin of the new I2C code
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file ituner.h ituner Header
 */

#if !defined(__ITUNER_H__)
#define __ITUNER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TunerID.h"
#include "TVFormats.h"
#include "I2CDevice.h"

#define TUNER_AFCSTATUS_NOCARRIER 1000000000L

enum eTunerLocked
{
    TUNER_LOCK_NOTSUPPORTED = -1,
    TUNER_LOCK_OFF = 0,
    TUNER_LOCK_ON = 1
};    
    
enum eTunerAFCStatus
{
    TUNER_AFC_NOTSUPPORTED = -1,
    TUNER_AFC_NOCARRIER = 0,
    TUNER_AFC_CARRIER = 1
};

class IExternalIFDemodulator;

/** Interface for control of analogue tuners
*/
class ITuner
{
public:
    ITuner();
    virtual ~ITuner();
    virtual eTunerId GetTunerId() = 0;
    virtual eVideoFormat GetDefaultVideoFormat() = 0;
    virtual bool HasRadio() const = 0;
    virtual bool SetRadioFrequency(long nFrequency) = 0;
    virtual bool SetTVFrequency(long nFrequency, eVideoFormat videoFormat) = 0;
    virtual long GetFrequency() = 0;
    virtual eTunerLocked IsLocked() = 0;
    //Automatic Frequency Control status  
      //Sets frequency deviation from optimum if there is a carrier (e.g. -62500 or 125000 Hz)
    virtual eTunerAFCStatus GetAFCStatus(long &nFreqDeviation) = 0; 

    virtual void AttachIFDem(IExternalIFDemodulator* pExternalIFDemodulator, bool bFreeOnDestruction = FALSE);
protected:
    IExternalIFDemodulator* m_ExternalIFDemodulator;
    bool m_bFreeIFDemodulatorOnDestruction;
};

class II2CTuner : public ITuner,
                  public CI2CDevice
{
public:
    II2CTuner() ;
    virtual ~II2CTuner();
};

class IExternalIFDemodulator : public CI2CDevice
{
public:    
    virtual void Init(bool bPreInit, eVideoFormat videoFormat) = 0;
    virtual void TunerSet(bool bPreSet, eVideoFormat videoFormat) = 0;

    virtual bool Detect() = 0;
        
    virtual eTunerAFCStatus GetAFCStatus(long &nFreqDeviation) = 0;
};

#endif // !defined(__ITUNER_H__)
