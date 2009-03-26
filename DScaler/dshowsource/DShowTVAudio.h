/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 Torbjörn Jansson.  All rights reserved.
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
 * @file DShowTVAudio.h interface for the CDShowTVAudio class.
 */

#if !defined(AFX_DSHOWTVAUDIO_H__708EE14A_E2D0_434C_B249_720DDCF72A90__INCLUDED_)
#define AFX_DSHOWTVAUDIO_H__708EE14A_E2D0_434C_B249_720DDCF72A90__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**
 * Class for managing IAMTVAudio
 */
class CDShowTVAudio
{
public:
    CDShowTVAudio(CComPtr<IAMTVAudio> pTVAudio);
    virtual ~CDShowTVAudio();

    /**
     * @return a bitmask of TVAudioMode enumeration
     */
    long GetAvailableModes();
    /**
     * @param mode new audio mode
     */
    void SetMode(TVAudioMode mode);

    /**
     * @return current TVAudioMode
     */
    TVAudioMode GetMode();

private:
    CComPtr<IAMTVAudio> m_pTVAudio;
};

#endif // !defined(AFX_DSHOWTVAUDIO_H__708EE14A_E2D0_434C_B249_720DDCF72A90__INCLUDED_)
