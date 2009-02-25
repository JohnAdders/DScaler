/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//  This file is subject to the terms of the GNU General Public License as
//  published by the Free Software Foundation.  A copy of this license is
//  included with this software distribution in the file COPYING.  If you
//  do not have a copy, you may obtain a copy by writing to the Free
//  Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.30  2002/11/13 11:15:37  adcockj
// disabled save state for bt848 cards
//
// Revision 1.29  2002/11/07 20:33:16  adcockj
// Promoted ACPI functions so that state management works properly
//
// Revision 1.28  2002/11/07 19:02:08  adcockj
// Fixed bug in new state management code
//
// Revision 1.27  2002/11/07 13:37:42  adcockj
// Added State restoration code to PCICard
// Functionality disabled prior to testing and not done for SAA7134
//
// Revision 1.26  2002/10/11 21:36:11  ittarnavsky
// removed GetAudioDecoderType()
//
// Revision 1.25  2002/10/11 13:38:13  kooiman
// Added support for VoodooTV IF demodulator. Improved TDA9887. Added interface for GPOE/GPDATA access to make this happen.
//
// Revision 1.24  2002/09/12 22:01:26  ittarnavsky
// Removed Reference to HasMSP
// Changes due to the IAudioControls to CAudioControls transition
//
// Revision 1.23  2002/08/07 21:53:04  adcockj
// Removed todo item
//
// Revision 1.22  2002/06/16 18:54:59  robmuller
// ACPI powersafe support.
//
// Revision 1.21  2002/06/13 15:22:53  adcockj
// Honour luma range and peaking on CCIR inputs
//
// Revision 1.20  2002/04/10 07:25:02  adcockj
// Changes vdelay in CCIR mode to be format default by default
//
// Revision 1.19  2002/04/07 10:37:53  adcockj
// Made audio source work per input
//
// Revision 1.18  2002/02/12 02:27:45  ittarnavsky
// fixed the hardware info dialog
//
// Revision 1.17  2002/02/01 04:43:55  ittarnavsky
// some more audio related fixes
// removed the handletimermessages and getaudioname methods
// which break the separation of concerns oo principle
//
// Revision 1.16  2002/01/23 12:20:32  robmuller
// Added member function HandleTimerMessages(int TimerId).
//
// Revision 1.15  2001/12/18 23:36:01  adcockj
// Split up the MSP chip support into two parts to avoid probelms when deleting objects
//
// Revision 1.14  2001/12/18 13:12:11  adcockj
// Interim check-in for redesign of card specific settings
//
// Revision 1.13  2001/12/16 17:04:37  adcockj
// Debug Log improvements
//
// Revision 1.12  2001/12/12 17:12:36  adcockj
// Tidy up SetGeoSize
//
// Revision 1.11  2001/12/08 13:43:20  adcockj
// Fixed logging and memory leak bugs
//
// Revision 1.10  2001/12/05 21:45:10  ittarnavsky
// added changes for the AudioDecoder and AudioControls support
//
// Revision 1.9  2001/11/29 17:30:51  adcockj
// Reorgainised bt848 initilization
// More Javadoc-ing
//
// Revision 1.8  2001/11/26 13:02:27  adcockj
// Bug Fixes and standards changes
//
// Revision 1.7  2001/11/25 01:58:34  ittarnavsky
// initial checkin of the new I2C code
//
// Revision 1.6  2001/11/23 10:49:16  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.5  2001/11/18 17:20:19  adcockj
// Fixed svideo bug
//
// Revision 1.4  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.3  2001/11/02 16:30:07  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.1.2.7  2001/08/22 11:12:48  adcockj
// Added VBI support
//
// Revision 1.1.2.6  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.1.2.5  2001/08/18 17:09:30  adcockj
// Got to compile, still lots to do...
//
// Revision 1.1.2.4  2001/08/17 16:35:13  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
// Revision 1.1.2.3  2001/08/15 07:10:19  adcockj
// Fixed memory leak
//
// Revision 1.1.2.2  2001/08/14 16:41:36  adcockj
// Renamed driver
// Got to compile with new class based card
//
// Revision 1.1.2.1  2001/08/14 09:40:19  adcockj
// Interim version of code for multiple card support
//
// Revision 1.1  2001/08/13 12:05:12  adcockj
// Updated range for contrast and saturation
// Added more code for new driver interface
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "GenericCard.h"


CGenericCard::CGenericCard(CHardwareDriver* pDriver) :
    CPCICard(pDriver)
{
}

CGenericCard::~CGenericCard()
{
    ClosePCICard();
}
