/////////////////////////////////////////////////////////////////////////////
// $Id: PerSettings.cpp,v 1.2 2003-06-15 06:01:35 atnak Exp $
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
// Revision 1.1  2003/06/12 10:28:45  atnak
// Initial release
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PerSettings.h"

static void SourceIdToString(long lValue, char szBuffer[32]);

static const tPERSETTING_DESCRIBE g_PerSettingDescribeTable[] =
{
    { SETTING_PER_SOURCE,		"",		SourceIdToString },
    { SETTING_PER_VIDEOINPUT,	"_VI",	NULL },
    { SETTING_PER_VIDEOFORMAT,	"_VF",	NULL },
    { SETTING_PER_AUDIOINPUT,	"_AI",	NULL },
    { SETTING_PER_CHANNEL,		"_CH",	NULL },
    { SETTING_PER_SAVED1,		"_M1",	NULL },
    { SETTING_PER_SAVED2,		"_M2",	NULL },
};


// These two will be set in InitializeTableValues
static long		g_nMaxTables = 0;
static PERBITS	g_fSavedBits = 0;


static void SourceIdToString(long lValue, char szBuffer[32])
{
	WORD wProvider = HIWORD(lValue);
	WORD wIndex = LOWORD(lValue);

	// \todo: convert wProvider to string

	_snprintf(szBuffer, 32, "%x%d", wProvider, wIndex);
}


static void InitializeTableValues(void)
{
	if (g_nMaxTables != 0)
	{
		// Values are already initialized
		return;
	}

	g_nMaxTables = sizeof(g_PerSettingDescribeTable);
	g_nMaxTables /= sizeof(g_PerSettingDescribeTable[0]);

	for (int i = 0; i < g_nMaxTables; i++)
	{
		g_fSavedBits |= 1 << g_PerSettingDescribeTable[i].nPerSetting;
	}
}


//--------------------------------------------------------------------
// CPerActiveBitsVector
//--------------------------------------------------------------------


CPerActiveBitsVector::CPerActiveBitsVector()
{
	InitializeTableValues();
	m_fActiveBits = 0;
}


void CPerActiveBitsVector::Set(PERSETTING nPerBit, long lValue)
{
	m_fActiveBits |= (1 << nPerBit);
	m_lActiveValues[nPerBit] = lValue;
}


BOOL CPerActiveBitsVector::Get(PERSETTING nPerBit, long* pValue)
{
	if (m_fActiveBits & (1 << nPerBit))
	{
		if (pValue != NULL)
		{
			*pValue = m_lActiveValues[nPerBit];
		}
		return TRUE;
	}
	return FALSE;
}


BOOL CPerActiveBitsVector::Check(PERSETTING nPerBit, long lValue)
{
	if (m_fActiveBits & (1 << nPerBit))
	{
		return (lValue == m_lActiveValues[nPerBit]);
	}
	return FALSE;	
}


void CPerActiveBitsVector::Deactivate(PERSETTING nPerBit)
{
	m_fActiveBits &= ~(1 << nPerBit);
}


PERBITS CPerActiveBitsVector::GetDescribedBits(PERBITS fBitsMask)
{
	return fBitsMask & m_fActiveBits & g_fSavedBits;
}


LONG CPerActiveBitsVector::Describe(PERBITS fBitsMask, LPSTR buffer, LONG bufferLength)
{
	char* s = buffer;
	char* l = buffer + bufferLength;
	int length;

	*s = '\0';

	for (int i = 0; i < g_nMaxTables; i++)
	{
		PERSETTING nPerBit = g_PerSettingDescribeTable[i].nPerSetting;

		if (m_fActiveBits & fBitsMask & (1 << nPerBit))
		{
			length = _snprintf(s, l-s, "%s", g_PerSettingDescribeTable[i].szDescribePrefix);

			if (length >= 0)
			{
				s += length;

				if (g_PerSettingDescribeTable[i].pLongToString != NULL)
				{
					char szBuffer[32];

					(g_PerSettingDescribeTable[i].pLongToString)
						(m_lActiveValues[nPerBit], szBuffer);

					length = _snprintf(s, l-s, "%s", szBuffer);
				}
				else
				{
					length = _snprintf(s, l-s, "%ld", m_lActiveValues[nPerBit]);
				}
			}
			
			if (length < 0)
			{
				buffer[bufferLength-1] = '\0';
				return -1;
			}

			s += length;
		}
	}

	return l-s;
}

