/////////////////////////////////////////////////////////////////////////////
// $Id: SettingRepository.h,v 1.1 2004-08-06 17:12:10 atnak Exp $
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

#ifndef __SETTINGREPOSITORY_H__
#define __SETTINGREPOSITORY_H__

#include <Windows.h>
#include <crtdbg.h>
#include <list>
#include <new>

#ifndef ASSERT
#ifdef _DEBUG
#define ASSERT(x)				{if (!(x)) _CrtDbgBreak();}
#else
#define ASSERT(x)
#endif
#endif

#define OUT_OF_MEMORY_ERROR		throw std::bad_alloc()

// Declaration of class pointers
typedef class CSettingRepository*		PSETTINGREPOSITORY;
typedef class CSettingGroup*			PSETTINGGROUP;
typedef class CSettingGroupEx*			PSETTINGGROUPEX;
typedef class CSettingKey*				PSETTINGKEY;
typedef class CSettingObject*			PSETTINGOBJECT;
typedef const class CSettingObject*		PCSETTINGOBJECT;
typedef class CSettingLimiter*			PSETTINGLIMITER;
typedef class CSettingValue*			PSETTINGVALUE;
typedef class CSettingConfig*			PSETTINGCONFIG;

// Declaration of class references
typedef class CSettingValue&			RSETTINGVALUE;
typedef const class CSettingValue&		RCSETTINGVALUE;

// Handle used by setting groups.
typedef PVOID HSETTING;
// Dependency bits storage type used by groups ex.
typedef BYTE DBIT;


// Basic headers that are necessary in using setting repository
#include "SettingValue.h"
#include "SettingGroup.h"
#include "SettingKey.h"
#include "SettingConfig.h"


//
// Class purpose affiliation graph:
//
// CSettingRepository -- creates --> CSettingGroup* <-- simplification -- CSettingKey*
//         ^                              |               wrapper
//         |                             /          (for setting access)
//         |                         contains
//          \                          /
//           ----- load/save -- CSettingObject <-- limits values -- CSettingLimiter*
//                                    |                                   
//                                 stores                                
//                                    |                                 
//                              CSettingValue                          
//
//   CSettingConfig* -- indirectly uses --> [CSettingGroup]
//
// [Astericks (*) beside the name means the class has multiple derivatives.]
//
//
// Class purpose summary:
//
// CSettingValue      - Common setting value primitive for passing setting values
//                      around both internally and to external.
//
// CSettingGroup      - Every setting is housed and owned by one of these.  An
//                      operational setting is always part of a CSettingGroup.
//
// CSettingKey        - Used to refer to settings owned by CSettingGroup.  Offers
//                      simplifications to setting accesses.
// 
// CSettingLimiter    - Class whose primary purpose is for limiting the values a
//                      setting can have.
//
// CSettingRepository - Can be used to create CSettingGroup objects, in which case
//                      its freeing is taken care of.  It is also owns access to a
//                      repository file.  A CSettingGroup needs access one of these
//                      classes to be able to save and load.
//
// CSettingObject     - Internal only.  Stores and works with value information about
//                      a setting.
//
// CSettingConfig     - Class for describing and standardizing configuration user
//                      interfaces for settings.
//


//////////////////////////////////////////////////////////////////////////
// CSettingRepository
//////////////////////////////////////////////////////////////////////////
class CSettingRepository
{
public:
	CSettingRepository(LPCSTR repositoryFilename);
	virtual ~CSettingRepository();

	virtual PSETTINGGROUP CreateGroup(IN LPCSTR section);
	virtual PSETTINGGROUPEX CreateGroupEx(IN LPCSTR section);
	virtual void DeleteGroup(IN PSETTINGGROUP group);

	virtual void SaveEveryGroupSettings();

	virtual BOOL LoadSettingString(IN LPCSTR section, IN LPCSTR key, OUT LPSTR string, IN DWORD size);
	virtual void SaveSettingString(IN LPCSTR section, IN LPCSTR key, IN LPCSTR string);

protected:
	typedef std::list<PSETTINGGROUP> GROUPLIST;

	LPCSTR					m_repositoryFilename;
	GROUPLIST				m_groupList;
};


#endif
