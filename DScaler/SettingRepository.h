/////////////////////////////////////////////////////////////////////////////
// $Id: SettingRepository.h,v 1.7 2005-10-25 08:17:59 adcockj Exp $
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
// Revision 1.6  2005/03/18 16:19:06  atnak
// Synchronizing work in progress.
//
// Revision 1.5  2005/03/05 12:15:20  atnak
// Syncing files.
//
// Revision 1.4  2004/09/08 07:20:21  atnak
// Sync with SettingGroup.h in HSETTING change to a more enforcable type.
//
// Revision 1.3  2004/08/14 13:45:23  adcockj
// Fixes to get new settings code working under VS6
//
// Revision 1.2  2004/08/13 08:53:50  atnak
// Added usage documentation for setting repository.
//
// Revision 1.1  2004/08/06 17:12:10  atnak
// Setting repository initial upload.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __SETTINGREPOSITORY_H__
#define __SETTINGREPOSITORY_H__

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
typedef class CSettingGroup_*			PSETTINGGROUP;
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
typedef class _HSETTING *HSETTING;
// Dependency bits storage type used by groups ex.
typedef BYTE DBIT;


// Basic headers that are necessary in using setting repository
#include "SettingValue.h"
#include "SettingGroup.h"
#include "SettingKey.h"
#include "SettingConfig.h"


//
// Using setting repository:
//
// 1. Creating a setting repository:
//
//     CSettingRepository repository(".\\filename.ini");
//   or
//     PSETTINGREPOSITORY pRepository = CSettingRepository(".\\filename.ini");
//
//
// 2. Creating a setting group:
//
//     CSettingGroup_ group("ini_section", &repository);
//   or 
//     PSETTINGROUP pGroup = new CSettingGroup_("ini_section", &repository);
//   or
//     PSETTINGROUP pGroup = repository.CreateGroup("ini_section");
//
//   Note: If the group is created using the last method, it should not
//   be deleted manually.  'repository' will take care if its deletion
//   when it is deleted.
//
//
// 3. Adding a setting to a setting group:
//
//     HSETTING setting = group.AddSetting(CSettingKeyLong::NewSetting("setting_entry", 0, -10, 10));
//
//
// 4. Loading and saving settings:
//
//     group.LoadSettings();
//     group.SaveSettings();
//   or
//     group.LoadSetting(setting);
//     group.SaveSetting(setting);
//
//
// 5. Working with setting values:
//
//   Setting the value:
//     group.SetValue(setting, value);
//
//   Getting the value:
//     value = group.GetValue(setting);
//
//   Setting the default value:
//     group.UseDefault(setting);
//
//   Changing the default value:
//     group.SetDefault(setting, value);
//
//   Getting the default value:
//     value = group.GetDefault(setting);
//
//   Note: 'value' is of type CSettingValue.
//
//
// 6. Using a setting key:
//
//   Setting keys simplify the process of working with setting values.
//
//   To use a setting key, first declare a setting key:
//     CSettingKeyLong key;
//
//   When creating a setting, use this alternate method:
//     group.AddSetting(CSettingKeyLong::NewSetting("setting_entry", 0, -10, 10), &key);
//
//   To save and load values:
//     key.LoadSetting();
//     key.SaveSetting();
//
//   To access setting values:
//     key.SetValue(iValue);
//     iValue = key.GetValue();
//
//     ...
//
//
// 7. Setting change notifications:
//
//   Registering a global callback for receiving notifications when any
//   setting in a group changes its value:
//
//     group.SetNotifyProc(NotifyProc, contextValue);
//
//   Note: See "SettingGroup.h" for more information about various
//   notifications.
//
//
//   To have a single callback for individual settings, use a special
//   setting key definition macro:
//
//     CSETTINGKEY_P_CALLBACK_LONG(SettingName) globalKey;
//   or
//     CSETTINGKEY_C_CALLBACK_LONG(ClassName, SettingName) classKey;
//     classKey.Setup(..., classInstance, ...);
//
//   Note: The callbacks from the above two declarations will be called
//   SettingNameOnNotify() and ClassName::SettingNameOnNotify() respectively.
//   See the bottom of "SettingKey.h" for more information.
//
//
// 8. Settings with per setting associations:
//
//   Note: See CSettingGroupEx documentation in "SettingGroup.h".
//
//
// 9. Configuration interfaces for settings:
//
//   CSettingConfigContainer container("Setting Collection Name");
//
//   container.AddSetting(new CSettingConfigSlider("Setting Name", &group, setting, -10, 10));
//   container.AddSetting(new CSettingConfigSlider(&key, -10, 10));
//
//   container.SetPurgable(FALSE);
//
//   return new CTreeSettingsSettingConfig(&container);
//
//   Note: The last depends on other implementation factors.
//
//
//   For association configurations:
//
//     CSettingConfigContainer* container = groupex.CreateAssociationConfig("Setting Collection Name");
//
//     container->AddSetting(new CSettingConfigDependent("Setting Name", &group, setting));
//
//   return new CTreeSettingsAssociations(container);
//
//   Note: The last depends on other implementation factors.
//


//
// Class purpose affiliation graph:
//
// CSettingRepository -- creates --> CSettingGroup_* <-- simplification -- CSettingKey*
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
//   CSettingConfig* -- indirectly uses --> [CSettingGroup_]
//
// [Astericks (*) beside the name means the class has multiple derivatives.]
//
//
// Class purpose summary:
//
// CSettingValue      - Common setting value primitive for passing setting values
//                      around both internally and to external.
//
// CSettingGroup_      - Every setting is housed and owned by one of these.  An
//                      operational setting is always part of a CSettingGroup_.
//
// CSettingKey        - Used to refer to settings owned by CSettingGroup_.  Offers
//                      simplifications to setting accesses.
// 
// CSettingLimiter    - Class whose primary purpose is for limiting the values a
//                      setting can have.
//
// CSettingRepository - Can be used to create CSettingGroup_ objects, in which case
//                      its freeing is taken care of.  It is also owns access to a
//                      repository file.  A CSettingGroup_ needs access one of these
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

	LPSTR					m_repositoryFilename;
	GROUPLIST				m_groupList;
};


#endif
