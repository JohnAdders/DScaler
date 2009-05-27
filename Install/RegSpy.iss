;////////////////////////////////////////////////////////////////////////////
;// $Id: RegSpy.iss,v 1.1 2002-12-23 13:00:54 adcockj Exp $
;/////////////////////////////////////////////////////////////////////////////
;// Copyright (c) 2002 Rob Muller.  All rights reserved.
;/////////////////////////////////////////////////////////////////////////////
;//
;//  This file is subject to the terms of the GNU General Public License as
;//  published by the Free Software Foundation.  A copy of this license is
;//  included with this software distribution in the file COPYING.  If you
;//  do not have a copy, you may obtain a copy by writing to the Free
;//  Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
;//
;//  This software is distributed in the hope that it will be useful,
;//  but WITHOUT ANY WARRANTY; without even the implied warranty of
;//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;//  GNU General Public License for more details
;/////////////////////////////////////////////////////////////////////////////
;
;  This is an InnoSetup script.
;  For more information about InnoSetup see http://www.innosetup.com

[Setup]
AppName=RegSpy 23-Dec-2002
AppVerName=RegSpy 23-Dec-2002
AppPublisherURL=http://www.dscaler.org
AppSupportURL=http://www.dscaler.org/phpBB/
AppUpdatesURL=http://www.dscaler.org
DefaultDirName={pf}\RegSpy
DefaultGroupName=DScaler
AllowNoIcons=yes
LicenseFile=gpl.rtf
AppMutex=RegSpy
;required for installing the driver on NT platforms
PrivilegesRequired=Admin
DisableStartupPrompt=yes

[Messages]
BeveledLabel=RegSpy

[Components]
Name: "main"; Description: "Main Files"; Types: full; Flags: fixed

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"; MinVersion: 4,4

[Files]
Source: "..\Release\RegSpy.exe"; DestDir: "{app}"; Flags: ignoreversion;
Source: "..\Release\DScalerRes.dll"; DestDir: "{app}"; Flags: ignoreversion;
Source: "..\Release\DSDrv4.sys"; DestDir: "{app}"; Flags: ignoreversion;
Source: "..\Release\DSDrv4.vxd"; DestDir: "{app}"; Flags: ignoreversion;

[INI]
Filename: "{app}\DScaler.url"; Section: "InternetShortcut"; Key: "URL"; String: "http://www.dscaler.org"
Filename: "{app}\Support.url"; Section: "InternetShortcut"; Key: "URL"; String: "http://www.dscaler.org/phpBB/"

[Icons]
Name: "{group}\RegSpy"; Filename: "{app}\RegSpy.exe"
Name: "{group}\DScaler on the Web"; Filename: "{app}\DScaler.url"
Name: "{group}\Support Forum"; Filename: "{app}\Support.url"
Name: "{userdesktop}\RegSpy"; Filename: "{app}\RegSpy.exe"; MinVersion: 4,4; Tasks: desktopicon

[UninstallDelete]
Type: files; Name: "{app}\DScaler.url"
Type: files; Name: "{app}\Support.url"

