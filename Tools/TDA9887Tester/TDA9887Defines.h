/////////////////////////////////////////////////////////////////////////////
// $Id: TDA9887Defines.h,v 1.2 2004-11-28 21:33:29 to_see Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004 Torsten Seeboth. All rights reserved.
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
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
// $Log: not supported by cvs2svn $
// Revision 1.1  2004/10/30 19:30:22  to_see
// initial checkin
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __TDA9887_DEFINES_H___
#define __TDA9887_DEFINES_H___

//
// TDA defines
//

#define I2C_TDA9887_0					0x86	// MAD1
#define I2C_TDA9887_1					0x96	// MAD3
#define I2C_TDA9887_2					0x84	// MAD2
#define I2C_TDA9887_3					0x94	// MAD4

//// first reg
#define TDA9887_VideoTrapBypassOFF		0x00	// bit b0
#define TDA9887_VideoTrapBypassON		0x01	// bit b0

#define TDA9887_AutoMuteFmInactive		0x00	// bit b1
#define TDA9887_AutoMuteFmActive		0x02	// bit b1

#define TDA9887_Intercarrier			0x00	// bit b2
#define TDA9887_QSS						0x04	// bit b2

#define TDA9887_PositiveAmTV			0x00	// bit b3:4
#define TDA9887_FmRadio1				0x08	// bit b3:4
#define TDA9887_NegativeFmTV			0x10	// bit b3:4
#define TDA9887_FmRadio2				0x18	// bit b3:4

#define TDA9887_ForcedMuteAudioON		0x20	// bit b5
#define TDA9887_ForcedMuteAudioOFF		0x00	// bit b5

#define TDA9887_OutputPort1Active		0x00	// bit b6
#define TDA9887_OutputPort1Inactive		0x40	// bit b6
#define TDA9887_OutputPort2Active		0x00	// bit b7
#define TDA9887_OutputPort2Inactive		0x80	// bit b7

//// second reg
#define TDA9887_TakeOverPointMin		0x00	// bit c0:4
#define TDA9887_TakeOverPointDefault	0x10	// bit c0:4
#define TDA9887_TakeOverPointMax		0x1f	// bit c0:4
#define TDA9887_DeemphasisOFF			0x00	// bit c5
#define TDA9887_DeemphasisON			0x20	// bit c5
#define TDA9887_Deemphasis75			0x00	// bit c6
#define TDA9887_Deemphasis50			0x40	// bit c6
#define TDA9887_AudioGain0				0x00	// bit c7
#define TDA9887_AudioGain6				0x80	// bit c7

//// third reg
#define TDA9887_AudioIF_4_5				0x00	// bit e0:1
#define TDA9887_AudioIF_5_5				0x01	// bit e0:1
#define TDA9887_AudioIF_6_0				0x02	// bit e0:1
#define TDA9887_AudioIF_6_5				0x03	// bit e0:1

#define TDA9887_VideoIF_58_75			0x00	// bit e2:4
#define TDA9887_VideoIF_45_75			0x04	// bit e2:4
#define TDA9887_VideoIF_38_90			0x08	// bit e2:4
#define TDA9887_VideoIF_38_00			0x0C	// bit e2:4
#define TDA9887_VideoIF_33_90			0x10	// bit e2:4
#define TDA9887_VideoIF_33_40			0x14	// bit e2:4
#define TDA9887_RadioIF_45_75			0x18	// bit e2:4
#define TDA9887_RadioIF_38_90			0x1C	// bit e2:4

#define TDA9887_TunerGainNormal			0x00	// bit e5
#define TDA9887_TunerGainLow			0x20	// bit e5

#define TDA9887_Gating_18				0x00	// bit e6
#define TDA9887_Gating_36				0x40	// bit e6

#define TDA9887_AgcOutON				0x80	// bit e7
#define TDA9887_AgcOutOFF				0x00	// bit e7

#endif
