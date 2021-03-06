//
// $Id$
//
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

/**
 * @file BT848_VOODOOTV_IFDEM.h  CPreTuneVoodooFM Header file
 */

#ifdef WANT_BT8X8_SUPPORT

#ifndef _BT848_VOODOOTV_IFDEM_H_
#define _BT848_VOODOOTV_IFDEM_H_

#include "ITuner.h"
#include "BT848Card.h"

class CPreTuneVoodooFM : public IExternalIFDemodulator
{
public:
    CPreTuneVoodooFM(CBT848Card* pBT848Card);

    void TunerSet(BOOL bPreSet, eVideoFormat videoFormat);
    BOOL Detect();

    void Init(BOOL bPreInit, eVideoFormat videoFormat) { TunerSet(bPreInit, videoFormat); }
    eTunerAFCStatus GetAFCStatus(long &nFreqDeviation) { return TUNER_AFC_NOTSUPPORTED; }
protected:
    // from CI2CDevice
    virtual BYTE GetDefaultAddress() const { return 0; }
private:
    CBT848Card* m_pBT848Card;
};

#endif

#endif // WANT_BT8X8_SUPPORT