/////////////////////////////////////////////////////////////////////////////
// $Id: PerSettings.h,v 1.1 2003-06-12 10:28:45 atnak Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Atsushi Nakagawa.  All rights reserved.
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
// 12 Jun 2003   Atsushi Nakagawa      Initial Release
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __PERSETTINGS_H___
#define __PERSETTINGS_H___


// Per settings
enum PERSETTING
{
    SETTING_PER_SOURCE  = 0,
    SETTING_PER_VIDEOINPUT,
    SETTING_PER_VIDEOFORMAT,
    SETTING_PER_AUDIOINPUT,
    SETTING_PER_CHANNEL,

	// Non-specific per settings
	SETTING_PER_UNDEFINED1,
	SETTING_PER_UNDEFINED2,
	SETTING_PER_UNDEFINED3,
	SETTING_PER_UNDEFINED4,
	SETTING_PER_SAVED1,
	SETTING_PER_SAVED2,

    SETTING_PER_LASTONE
};


// Per bits
enum
{
    PER_SOURCE      = 1 << SETTING_PER_SOURCE,
    PER_VIDEOINPUT  = 1 << SETTING_PER_VIDEOINPUT,
    PER_VIDEOFORMAT = 1 << SETTING_PER_VIDEOFORMAT,
    PER_AUDIOINPUT  = 1 << SETTING_PER_AUDIOINPUT,
    PER_CHANNEL     = 1 << SETTING_PER_CHANNEL,

    PER_UNDEFINED1  = 1 << SETTING_PER_UNDEFINED1,
    PER_UNDEFINED2  = 1 << SETTING_PER_UNDEFINED2,
    PER_UNDEFINED3  = 1 << SETTING_PER_UNDEFINED3,
    PER_UNDEFINED4  = 1 << SETTING_PER_UNDEFINED4,
    PER_SAVED1      = 1 << SETTING_PER_SAVED1,
    PER_SAVED2      = 1 << SETTING_PER_SAVED2,

	// Special internal code
	PER_QUERYALL	= 0,
};


typedef WORD	PERBITS;


typedef struct
{
	PERSETTING	nPerSetting;
	char*		szDescribePrefix;
	void		(*pLongToString)(long lValue, char szBuffer[32]);
} tPERSETTING_DESCRIBE;


class CPerSetting
{
public:
	CPerSetting();
	virtual ~CPerSetting();

	virtual PERBITS	GetMasterBits() { return m_fMasterBits; };
	virtual PERBITS	GetPerBits() { return m_fPerBits; };

	// Perform preliminary checking on the setting and return
	// TRUE if the setting changes
	virtual BOOL	QueryChange(PERBITS fChangingBits, void* pTidbit) = 0;
	// Commit any pending changes from QueryChange()
	virtual BOOL	CommitChange(void* pTidbit) = 0;

	// Return the new value that will be changed to on commit
	virtual long	GetQueriedValue() = 0;
	// Return the current value of the setting
	virtual long	GetCommittedValue() = 0;

protected:
    PERBITS         m_fPerBits;
    PERBITS         m_fMasterBits;
};


class CPerActiveBitsVector
{
public:
	CPerActiveBitsVector();
	virtual ~CPerActiveBitsVector() { };

	virtual void Set(PERSETTING nPerBit, long lValue);
	virtual BOOL Get(PERSETTING nPerBit, long* pValue);
	virtual BOOL Check(PERSETTING nPerBit, long lValue);

	virtual void Deactivate(PERSETTING nPerBit);

	virtual PERBITS GetDescribedBits(PERBITS fBitsMask);
	virtual LONG Describe(PERBITS fBitsMask, LPSTR buffer, LONG bufferLength);

protected:
	PERBITS	m_fActiveBits;
	long	m_lActiveValues[SETTING_PER_LASTONE];
};


#endif

