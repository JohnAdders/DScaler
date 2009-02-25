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
// Change Log
//
// Date          Developer             Changes
//
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

/**
 * @file DShowAudioControls.h interface for the CDShowAudioControls class.
 */

#if !defined(AFX_DSHOWAUDIOCONTROLS_H__78FFEF49_18A2_4959_A728_4A91D37C9249__INCLUDED_)
#define AFX_DSHOWAUDIOCONTROLS_H__78FFEF49_18A2_4959_A728_4A91D37C9249__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define DSHOW_AUDIOCAPS_HAS_VOLUME 1
#define DSHOW_AUDIOCAPS_HAS_BALANCE 2

/**
 * Generic audio controlls interface for directshow.
 * This coud be extended to support btwincap:s audio controlls,
 * like equalizer and dolby controlls
 */
class CDShowAudioControls
{
public:
	CDShowAudioControls();
	virtual ~CDShowAudioControls();
	
	/**
	 * Determines which audio controls is available.
	 * @return one or more of DSHOW_AUDIOCAPS_
	 */
	virtual int GetAudioCaps()=0;

	/**
	 * @return current volume
	 */
	virtual long GetVolume()=0;

	/**
	 * Changes volume.
	 * @param volume new volume
	 */
	virtual void SetVolume(long volume)=0;

	/**
	 * Retrieves the minumum and maximum allowable volume
	 * @param min reference to a variable that gets the minumum volume
	 * @param max reference to a variable that gets the maximum volume
	 */
	virtual void GetVolumeMinMax(long &min,long &max)=0;
	
	/**
	 * @return current balance
	 */
	virtual long GetBalance()=0;
	
	/**
	 * Changes balance.
	 * @param balance new balance
	 */
	virtual void SetBalance(long balance)=0;
	
	/**
	 * Retrieves the minumum and maximum allowable balance
	 * @param min reference to a variable that gets the minumum balance
	 * @param max reference to a variable that gets the maximum balance
	 */
	virtual void GetBalanceMinMax(long &min,long &max)=0;
};

#endif // !defined(AFX_DSHOWAUDIOCONTROLS_H__78FFEF49_18A2_4959_A728_4A91D37C9249__INCLUDED_)
