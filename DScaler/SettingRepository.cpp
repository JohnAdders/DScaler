/////////////////////////////////////////////////////////////////////////////
// $Id: SettingRepository.cpp,v 1.2 2005-03-05 12:15:20 atnak Exp $
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
// Revision 1.1  2004/08/06 17:12:10  atnak
// Setting repository initial upload.
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SettingRepository.h"
#include "SettingGroup.h"
#include <list>


//////////////////////////////////////////////////////////////////////////
// CSettingRepository
//////////////////////////////////////////////////////////////////////////

CSettingRepository::CSettingRepository(LPCSTR repositoryFilename)
{
	m_repositoryFilename = new CHAR[strlen(repositoryFilename)+1];
	if (m_repositoryFilename == NULL)
	{
		OUT_OF_MEMORY_ERROR;
	}
	strcpy((LPSTR)m_repositoryFilename, repositoryFilename);
}


CSettingRepository::~CSettingRepository()
{
	// Delete all groups.
	GROUPLIST::iterator it = m_groupList.begin();
	GROUPLIST::iterator ti = m_groupList.end();
	for ( ; it != ti; it++)
	{
		delete *it;
	}
	if (m_repositoryFilename != NULL)
	{
		delete m_repositoryFilename;
	}
}


PSETTINGGROUP CSettingRepository::CreateGroup(LPCSTR section)
{
	PSETTINGGROUP group = new CSettingGroup_(section, this);
	if (group == NULL)
	{
		OUT_OF_MEMORY_ERROR;
	}
	m_groupList.push_back(group);
	return group;
}


PSETTINGGROUPEX CSettingRepository::CreateGroupEx(LPCSTR section)
{
	PSETTINGGROUPEX group = new CSettingGroupEx(section, this);
	if (group == NULL)
	{
		OUT_OF_MEMORY_ERROR;
	}
	m_groupList.push_back(group);
	return group;
}


void CSettingRepository::DeleteGroup(IN PSETTINGGROUP group)
{
	m_groupList.remove(group);
	delete group;
}


void CSettingRepository::SaveEveryGroupSettings()
{
	GROUPLIST::iterator it = m_groupList.begin();
	GROUPLIST::iterator ti = m_groupList.end();
	for ( ; it != ti; it++)
	{
		(*it)->SaveSettings();
	}
}


BOOL CSettingRepository::LoadSettingString(IN LPCSTR section, IN LPCSTR key, OUT LPSTR string, IN DWORD size)
{
	// Call the Win32 API function to load the value from file.  Use an
	// uncommon string for default to detect if default was used.
	DWORD length = GetPrivateProfileStringA(section, key, "\"", string, size, m_repositoryFilename);
	// Check if the setting was in the file.
	return *string != '"' || length != 1;
}


void CSettingRepository::SaveSettingString(IN LPCSTR section, IN LPCSTR key, IN LPCSTR string)
{
	// Call the Win32 API function to write the value to file.
	WritePrivateProfileStringA(section, key, string, m_repositoryFilename);
}



