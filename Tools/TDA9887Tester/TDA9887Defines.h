/////////////////////////////////////////////////////////////////////////////
// $Id: TDA9887Defines.h,v 1.1 2004-10-30 19:30:22 to_see Exp $
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
/////////////////////////////////////////////////////////////////////////////

#ifndef __TDA9887_DEFINES_H___
#define __TDA9887_DEFINES_H___

/* ---------------------------------------------------------------------- */

//
// TDA defines
//

//// first reg (b)
#define cVideoTrapBypassOFF     0x00    // bit b0
#define cVideoTrapBypassON      0x01    // bit b0

#define cAutoMuteFmInactive     0x00    // bit b1
#define cAutoMuteFmActive       0x02    // bit b1

#define cIntercarrier           0x00    // bit b2
#define cQSS                    0x04    // bit b2

#define cPositiveAmTV           0x00    // bit b3:4
#define cFmRadio1               0x08    // bit b3:4
#define cNegativeFmTV           0x10    // bit b3:4
#define cFmRadio2               0x18    // bit b3:4

#define cForcedMuteAudioON      0x20    // bit b5
#define cForcedMuteAudioOFF     0x00    // bit b5

#define cOutputPort1Active      0x00    // bit b6
#define cOutputPort1Inactive    0x40    // bit b6

#define cOutputPort2Active      0x00    // bit b7
#define cOutputPort2Inactive    0x80    // bit b7


//// second reg (c)
#define cTakeOverPointMin		0x00	// bit c0:4
#define cTakeOverPointDefault	0x10	// bit c0:4
#define cTakeOverPointMax		0x1f	// bit c0:4

#define cDeemphasisOFF          0x00    // bit c5
#define cDeemphasisON           0x20    // bit c5

#define cDeemphasis75           0x00    // bit c6
#define cDeemphasis50           0x40    // bit c6

#define cAudioGain0             0x00    // bit c7
#define cAudioGain6             0x80    // bit c7


//// third reg (e)
#define cAudioIF_4_5             0x00    // bit e0:1
#define cAudioIF_5_5             0x01    // bit e0:1
#define cAudioIF_6_0             0x02    // bit e0:1
#define cAudioIF_6_5             0x03    // bit e0:1


#define cVideoIF_58_75           0x00    // bit e2:4
#define cVideoIF_45_75           0x04    // bit e2:4
#define cVideoIF_38_90           0x08    // bit e2:4
#define cVideoIF_38_00           0x0C    // bit e2:4
#define cVideoIF_33_90           0x10    // bit e2:4
#define cVideoIF_33_40           0x14    // bit e2:4
#define cRadioIF_45_75           0x18    // bit e2:4
#define cRadioIF_38_90           0x1C    // bit e2:4


#define cTunerGainNormal         0x00    // bit e5
#define cTunerGainLow            0x20    // bit e5

#define cGating_18               0x00    // bit e6
#define cGating_36               0x40    // bit e6

#define cAgcOutON                0x80    // bit e7
#define cAgcOutOFF               0x00    // bit e7

#endif
