/////////////////////////////////////////////////////////////////////////////
// $Id: SettingLimiter.cpp,v 1.1 2004-08-06 17:12:10 atnak Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004 Atsushi Nakagawa.  All rights reserved.
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
//  This file is part of the SettingRepository module.  See
//  SettingRepository.h for more information.
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SettingLimiter.h"
#include "SettingValue.h"


//////////////////////////////////////////////////////////////////////////
// CSettingLimiterClampInt
//////////////////////////////////////////////////////////////////////////

CSettingLimiterClampInt::CSettingLimiterClampInt(INT minimum, INT maximum) :
	m_minimum(minimum),
	m_maximum(maximum)
{
	if (m_maximum < m_minimum)
	{
		m_maximum = m_minimum;
	}
}


CSettingLimiterClampInt::~CSettingLimiterClampInt()
{
}


void CSettingLimiterClampInt::SetMax(IN INT maximum)
{
	m_maximum = maximum;
	if (m_minimum > m_maximum)
	{
		m_minimum = m_maximum;
	}
}


void CSettingLimiterClampInt::SetMin(IN INT minimum)
{
	m_minimum = minimum;
	if (m_maximum < m_minimum)
	{
		m_maximum = m_minimum;
	}
}


INT CSettingLimiterClampInt::GetMax() const
{
	return m_maximum;
}


INT CSettingLimiterClampInt::GetMin() const
{
	return m_minimum;
}


BOOL CSettingLimiterClampInt::OutOfLimit(IN const CSettingValue& value) const
{
	if (value.GetType() != SETTING_VALUE_INT)
		return FALSE;

	INT number = value.GetInt();
	return number < m_minimum || number > m_maximum;
}


BOOL CSettingLimiterClampInt::ApplyLimit(CSettingValue& value) const
{
	if (value.GetType() != SETTING_VALUE_INT)
		return FALSE;

	INT number = value.GetInt();
	if (number < m_minimum || number > m_maximum)
	{
		number = number < m_minimum ? m_minimum : m_maximum;
		value.SetInt(number);
		return TRUE;
	}
	return FALSE;
}

