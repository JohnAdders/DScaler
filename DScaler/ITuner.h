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

    // Perform initializing calls on the tuner.
    virtual bool InitializeTuner() { return true; };

    // Gets the ID of this tuner.
    virtual eTunerId GetTunerId() = 0;
    // Gets the default video format of the tuner.
    virtual eVideoFormat GetDefaultVideoFormat() = 0;

    // Returns whether or not radio is supported.
    virtual bool HasRadio() const = 0;

    // Tune the tuner into the TV frequency.
    virtual bool SetTVFrequency(long nFrequency, eVideoFormat videoFormat) = 0;
    // Tune the tuner into the radio frequency.
    virtual bool SetRadioFrequency(long nFrequency) = 0;

    // Gets the last tuned frequency.
    virtual long GetFrequency() = 0;
    // Returns whether or not the picture carrier is locked.
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
    // Sets the device to use a detected known I2C address.
    virtual bool SetDetectedI2CAddress(IN CI2CBus* i2cBus);
    // Sets the device to use an I2C address detected from the list.
    virtual bool SetDetectedI2CAddress(IN CI2CBus* i2cBus, IN BYTE* addresses, IN size_t count);

    // This is called before and after a tuner it initialized.
    virtual void Init(bool bPreInit, eVideoFormat videoFormat) = 0;
    // THis is called before and after a tuner changes channels.
    virtual void TunerSet(bool bPreSet, eVideoFormat videoFormat) = 0;

    // Return true if the demodulator exists.
    virtual bool Detect() = 0;
    // Gets the current AFC status of the demodulator.
    virtual eTunerAFCStatus GetAFCStatus(long &nFreqDeviation) = 0;
};

#endif // !defined(__ITUNER_H__)
