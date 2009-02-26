/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Torbjörn Jansson.  All rights reserved.
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

/**
 * @file DShowGenericAudioControls.h interface for the CDShowGenericAudioControls class.
 */

#if !defined(AFX_DSHOWGENERICAUDIOCONTROLS_H__BCC9A4B1_E307_41FE_BC89_EA708BADDA26__INCLUDED_)
#define AFX_DSHOWGENERICAUDIOCONTROLS_H__BCC9A4B1_E307_41FE_BC89_EA708BADDA26__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DShowAudioControls.h"

/**
 * Class that uses IBasicAudio to change volume and balance.
 * @see CDShowAudioControls
 */
class CDShowGenericAudioControls : public CDShowAudioControls  
{
public:
    CDShowGenericAudioControls(CComPtr<IBasicAudio> pAudio);
    virtual ~CDShowGenericAudioControls();
    
    int GetAudioCaps();
    
    /**
     * @return returns volume, zero is maximum volume and negative values is lower volume
     * @throws CDShowException this exception will be thrown if something goes wrong
     */
    long GetVolume();
    /**
     * @throws CDShowException this exception will be thrown if something goes wrong
     */
    void SetVolume(long volume);
    void GetVolumeMinMax(long &min,long &max);
    /**
     * @throws CDShowException this exception will be thrown if something goes wrong
     */
    long GetBalance();
    /**
     * @throws CDShowException this exception will be thrown if something goes wrong
     */
    void SetBalance(long balance);
    void GetBalanceMinMax(long &min,long &max);

private:
    ///Audio interface
    CComPtr<IBasicAudio> m_pAudio;
};

#endif // !defined(AFX_DSHOWGENERICAUDIOCONTROLS_H__BCC9A4B1_E307_41FE_BC89_EA708BADDA26__INCLUDED_)
