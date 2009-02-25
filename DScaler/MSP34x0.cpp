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
//
// $Log: not supported by cvs2svn $
// Revision 1.34  2007/02/18 21:15:31  robmuller
// Added option to not compile BT8x8 code.
//
// Revision 1.33  2003/10/27 10:39:52  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.32  2002/10/11 21:50:34  ittarnavsky
// moved the CMSP34x0Decoder to a separate file and renamed to CMSP34x0AudioDecoder
//
// Revision 1.31  2002/10/07 20:38:17  kooiman
// Abort thread before new detect to avoid problems.
//
// Revision 1.30  2002/10/02 10:52:35  kooiman
// Fixed C++ type casting for events.
//
// Revision 1.29  2002/09/28 15:04:58  kooiman
// Added object to raiseevent
//
// Revision 1.28  2002/09/28 14:49:39  tobbej
// fixed thread init/deinit for crashloging
//
// Revision 1.27  2002/09/27 14:14:22  kooiman
// MSP34xx fixes.
//
// Revision 1.26  2002/09/26 11:29:52  kooiman
// Split MSP code in 3 parts.
//
// Revision 1.25  2002/09/17 17:28:58  kooiman
// Couple of small fixes.
//
// Revision 1.24  2002/09/16 14:38:59  kooiman
// Added stereo autodetection.
//
// Revision 1.23  2002/09/16 14:37:35  kooiman
// Added stereo autodetection.
//
// Revision 1.22  2002/09/15 19:52:22  kooiman
// Adressed some NICAM AM issues.
//
// Revision 1.21  2002/09/15 15:58:33  kooiman
// Added Audio standard detection & some MSP fixes.
//
// Revision 1.20  2002/09/12 21:44:44  ittarnavsky
// split the MSP34x0 in two files one for the AudioControls the other foe AudioDecoder
//
// Revision 1.19  2002/09/07 20:54:49  kooiman
// Added equalizer, loudness, spatial effects for MSP34xx
//
// Revision 1.18  2002/07/02 20:00:09  adcockj
// New setting for MSP input pin selection
//
// Revision 1.17  2002/03/04 20:48:52  adcockj
// Reversed incorrect change
//
// Revision 1.15  2002/02/01 04:43:55  ittarnavsky
// some more audio related fixes
// removed the handletimermessages and getaudioname methods
// which break the separation of concerns oo principle
//
// Revision 1.14  2002/01/27 23:54:32  robmuller
// Removed the Auto Standard Detect of the rev G chips. + some reorganization of code.
//
// Revision 1.13  2002/01/23 22:57:28  robmuller
// Revision D/G improvements. The code is following the documentation much closer now.
//
// Revision 1.12  2002/01/21 12:06:33  robmuller
// RevA improvements.
//
// Revision 1.11  2002/01/16 19:24:28  adcockj
// Added Rob Muller's msp patch #504469
//
// Revision 1.10  2001/12/28 12:16:53  adcockj
// Sound fixes
//
// Revision 1.9  2001/12/21 11:07:31  adcockj
// Even more RevA fixes
//
// Revision 1.8  2001/12/20 23:46:20  ittarnavsky
// further RevA programming changes
//
// Revision 1.7  2001/12/20 12:55:54  adcockj
// First stab at supporting older MSP chips
//
// Revision 1.6  2001/12/19 19:26:17  ittarnavsky
// started rewrite of the sound standard selection
//
// Revision 1.5  2001/12/18 23:36:01  adcockj
// Split up the MSP chip support into two parts to avoid probelms when deleting objects
//
// Revision 1.4  2001/12/05 21:45:11  ittarnavsky
// added changes for the AudioDecoder and AudioControls support
//
// Revision 1.3  2001/12/03 17:27:56  adcockj
// SECAM NICAM patch from Quenotte
//
// Revision 1.2  2001/11/26 13:02:27  adcockj
// Bug Fixes and standards changes
//
// Revision 1.1  2001/11/25 02:03:21  ittarnavsky
// initial checkin of the new I2C code
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file MSP34x0.cpp CMSP34x0 Implementation
    All models:
        The demodulator only needs to be programmed if m_AudioInput is AUDIOINPUT_RADIO or
        AUDIOINPUT_TUNER.
    Revision G:
        Features:
        - Automatic Standard Detection. TV sound standard is detected automatically.
        - Automatic Sound Select. Automatic setup of FM matrix. If NICAM or FM/stereo is available it is
            automatically enabled. No action needed when changing tuner channels.

        This code uses Automatic Sound Select.
        There is no action needed on channel changes. The Automatic Sound Select will take care
        of switching to NICAM or analog stereo and back to mono.

    Revision D:
        Automatic Standard Detection (TV sound standard is detected automatically)
        FM matrix must be programmed manually.

        Uses currently Revision A code.

    Revision A:
        Everything is set up manually.
*/

#include "stdafx.h"

#ifdef WANT_BT8X8_SUPPORT

#include "MSP34x0.h"
#include "DebugLog.h"
#include "DScaler.h"
#include "Crash.h"
#include "Events.h"
#include "Providers.h"

#define MSP_UNCARRIER(carrier) ((double)(carrier)*18.432/(double)(1<<24))
#define MSP_CARRIER_HZ(freq) ((int)((double)(freq/18432000.0)*(1<<24)))
#define MSP_UNCARRIER_HZ(carrier) ((long)((double)(carrier)*18432000.0/double(1<<24)))

CMSP34x0::CMSP34x0()
{
}

BYTE CMSP34x0::GetDefaultAddress()const
{
    return 0x80>>1;
}

WORD CMSP34x0::GetRegister(BYTE subAddress, WORD reg)
{
    BYTE write[] = {(reg >> 8) & 0xFF, reg & 0xFF};
    BYTE result[2] = {0, 0};
    ReadFromSubAddress(subAddress, write, sizeof(write), result, sizeof(result));
    LOG(2, "MSP: %02X %02X %02X=%02X %02X",
        subAddress,
        write[0],
        write[1],
        result[0],
        result[1]);
    return ((WORD)result[0]) << 8 | result[1];
}

void CMSP34x0::SetRegister(BYTE subAddress, WORD reg, WORD value)
{
    BYTE write[] = {(reg >> 8) & 0xFF, reg & 0xFF, (value >> 8) & 0xFF, value & 0xFF};
    LOG(2, "MSP: %02X %02X %02X %02X %02X",
        subAddress,
        write[0],
        write[1],
        write[2],
        write[3]);

    WriteToSubAddress(subAddress, write, sizeof(write));
}

WORD CMSP34x0::GetDEMRegister(eDEMReadRegister reg)
{
    return GetRegister(0x11, (WORD)reg);
}

void CMSP34x0::SetDEMRegister(eDEMWriteRegister reg, WORD value)
{
    SetRegister(0x10, (WORD)reg, value);
}

WORD CMSP34x0::GetDSPRegister(eDSPReadRegister reg)
{
    return GetRegister(0x13, (WORD)reg);
}

void CMSP34x0::SetDSPRegister(eDSPWriteRegister reg, WORD value)
{
    SetRegister(0x12, (WORD)reg, value);
}

void CMSP34x0::Reset()
{
    BYTE reset[2] = {0x80, 0};
    LOG(2, "MSP: %02X %02X %02X",
        0,
        reset[0],
        reset[1]);
    WriteToSubAddress(0, reset, sizeof(reset));
    reset[0] = 0;
    LOG(2, "MSP: %02X %02X %02X",
        0,
        reset[0],
        reset[1]);
    WriteToSubAddress(0, reset, sizeof(reset));
}

WORD CMSP34x0::GetVersion()
{
    WORD result = GetDSPRegister(DSP_RD_VERSION_CODE);
    return result;
}

WORD CMSP34x0::GetProductCode()
{
    WORD result = GetDSPRegister(DSP_RD_PRODUCT_CODE);
    return result;
}

#endif // WANT_BT8X8_SUPPORT
