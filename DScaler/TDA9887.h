/////////////////////////////////////////////////////////////////////////////
// $Id: TDA9887.h,v 1.7 2004-09-11 20:19:55 to_see Exp $
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
// Revision 1.6  2004/08/27 13:12:41  to_see
// Added audio support for Ati Tv Wonder Pro
//
// Revision 1.5  2004/06/23 20:15:22  to_see
// Created an new struct TControlSettings for better handling
// more cards and deleted class CTDA9887MsiMaster. Thanks to atnak.
//
// Revision 1.4  2004/05/16 19:45:08  to_see
// Added an new class for Msi Master Card
//
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

enum eTDA9887Card
{
	TDA9887_DEFAULT = 0,
	TDA9887_MSI_TV_ANYWHERE_MASTER,
	TDA9887_LEADTEK_WINFAST_EXPERT,
	TDA9887_ATI_TV_WONDER_PRO,
	// Add here new tda9887 settings.
	TDA9887_LASTONE,
};

class CTDA9887 : public IExternalIFDemodulator
{
public:
    CTDA9887();
	CTDA9887(eTDA9887Card TDA9887Card);
    ~CTDA9887();

    bool Detect();

    void Init(bool bPreInit, eVideoFormat videoFormat);
    void TunerSet(bool bPreSet, eVideoFormat videoFormat);
    
    eTunerAFCStatus GetAFCStatus(long &nFreqDeviation);
protected:
    // from CI2CDevice
    virtual BYTE GetDefaultAddress() const { return I2C_TDA9887_0; }

private:
	typedef struct
	{
		DWORD eTDA9887Card;
		BYTE Pal_BG[3];
		BYTE Pal_I[3];
		BYTE Pal_DK[3];
		BYTE Secam_L[3];
		BYTE Ntsc[3];
		BYTE Ntsc_Jp[3];
		BYTE Fm_Radio[3];

	} TControlSettings;

    static const TControlSettings m_ControlSettings[TDA9887_LASTONE];
	eTDA9887Card m_eCardID;
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

#endif
