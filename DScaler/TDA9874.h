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
//  Copyright (C) 2004 Mika Laine.  All rights reserved.
//  Copyright (C) 2000-2005 Quenotte  All rights reserved.
//  Copyright (C) 2000 - 2002 by Eduardo José Tagle.
/////////////////////////////////////////////////////////////////////////////

#if !defined(__TDA9874_H__)
#define __TDA9874_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "I2CDevice.h"

/* ---------------------------------------------------------------------- */
/* audio chip description - defines+functions for tda9874h and tda9874a   */
/* Dariusz Kowalewski <darekk@automex.pl>                                 */

/* Subaddresses for TDA9874H and TDA9874A (slave rx) */
#define TDA9874A_AGCGR		0x00	/* AGC gain */
#define TDA9874A_GCONR		0x01	/* general config */
#define TDA9874A_MSR		0x02	/* monitor select */
#define TDA9874A_C1FRA		0x03	/* carrier 1 freq. */
#define TDA9874A_C1FRB		0x04	/* carrier 1 freq. */
#define TDA9874A_C1FRC		0x05	/* carrier 1 freq. */
#define TDA9874A_C2FRA		0x06	/* carrier 2 freq. */
#define TDA9874A_C2FRB		0x07	/* carrier 2 freq. */
#define TDA9874A_C2FRC		0x08	/* carrier 2 freq. */
#define TDA9874A_DCR		0x09	/* demodulator config */
#define TDA9874A_FMER		0x0a	/* FM de-emphasis */
#define TDA9874A_FMMR		0x0b	/* FM dematrix */
#define TDA9874A_C1OLAR		0x0c	/* ch.1 output level adj. */
#define TDA9874A_C2OLAR		0x0d	/* ch.2 output level adj. */
#define TDA9874A_NCONR		0x0e	/* NICAM config */
#define TDA9874A_NOLAR		0x0f	/* NICAM output level adj. */
#define TDA9874A_NLELR		0x10	/* NICAM lower error limit */
#define TDA9874A_NUELR		0x11	/* NICAM upper error limit */
#define TDA9874A_AMCONR		0x12	/* audio mute control */
#define TDA9874A_SDACOSR	0x13	/* stereo DAC output select */
#define TDA9874A_AOSR		0x14	/* analog output select */
#define TDA9874A_DAICONR	0x15	/* digital audio interface config */
#define TDA9874A_I2SOSR		0x16	/* I2S-bus output select */
#define TDA9874A_I2SOLAR	0x17	/* I2S-bus output level adj. */
#define TDA9874A_MDACOSR	0x18	/* mono DAC output select (tda9874a) */
#define TDA9874A_ESP		0xFF	/* easy standard progr. (tda9874a) */

/* Subaddresses for TDA9874H and TDA9874A (slave tx) */
#define TDA9874A_DSR		0x00	/* device status */
#define TDA9874A_NSR		0x01	/* NICAM status */
#define TDA9874A_NECR		0x02	/* NICAM error count */
#define TDA9874A_DR1		0x03	/* add. data LSB */
#define TDA9874A_DR2		0x04	/* add. data MSB */
#define TDA9874A_LLRA		0x05	/* monitor level read-out LSB */
#define TDA9874A_LLRB		0x06	/* monitor level read-out MSB */
#define TDA9874A_SIFLR		0x07	/* SIF level */
#define TDA9874A_TR2		252		/* test reg. 2 */
#define TDA9874A_TR1		253		/* test reg. 1 */
#define TDA9874A_DIC		254		/* device id. code */
#define TDA9874A_SIC		255		/* software id. code */


class CTDA9874 : public CI2CDevice
{
public:

    CTDA9874();
	virtual ~CTDA9874() {};
    void Reset();

	bool IsDevicePresent(int& dic, int& rev);

protected:
	int m_iMode ;		/* 0: A2, 1: NICAM */
	int m_iGCONR;		/* default config. input pin: SIFSEL=0 */
	int m_iNCONR;		/* default NICAM config.: AMSEL=0,AMUTE=1 */
	int m_iESP ;		/* standard: */
	int m_iDic;			/* device id. code */

	void WriteToSubAddress(BYTE subAddress, BYTE value);

    virtual BYTE GetDefaultAddress() const;
};

#endif // !defined(__TDA9874_H__)
