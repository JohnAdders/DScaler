;////////////////////////////////////////////////////////////////////////////
;// $Id: dscaler.iss,v 1.1 2002-03-06 22:50:58 robmuller Exp $
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
;// CVS Log
;//
;// $Log: not supported by cvs2svn $
;//
;//
;/////////////////////////////////////////////////////////////////////////////
;
;  This is an InnoSetup script.
;  For more information about InnoSetup see http://www.innosetup.com

[Setup]
AppName=DScaler
AppVerName=DScaler 4.0
AppPublisherURL=http://www.dscaler.org
AppSupportURL=http://www.dscaler.org/phpBB/
AppUpdatesURL=http://www.dscaler.org
DefaultDirName={pf}\DScaler
DefaultGroupName=DScaler
AllowNoIcons=yes
LicenseFile=gpl.rtf
;required for installing the driver on NT platforms
AdminPrivilegesRequired=yes

[Messages]
BeveledLabel=DScaler

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"; MinVersion: 4,4
Name: "quicklaunchicon"; Description: "Create a &Quick Launch icon"; GroupDescription: "Additional icons:"; MinVersion: 4,4; Flags: unchecked

[Files]
Source: "..\Release\DScaler.exe"; DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "..\Release\*.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "..\Release\DScaler.dbg"; DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "..\Release\DSDrv4.sys"; DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "..\Release\DSDrv4.vxd"; DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "..\Release\channel.txt"; DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "..\Release\Patterns\*.pat"; DestDir: "{app}\Patterns"; CopyMode: alwaysoverwrite
Source: "..\Release\Patterns\*.d3u"; DestDir: "{app}\Patterns"; CopyMode: alwaysoverwrite
Source: "..\ThirdParty\LibTiff\libtiff.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite
;Help file comes here.

[INI]
Filename: "{app}\DScaler.url"; Section: "InternetShortcut"; Key: "URL"; String: "http://www.dscaler.org"
Filename: "{app}\Support.url"; Section: "InternetShortcut"; Key: "URL"; String: "http://www.dscaler.org/phpBB/"

[Icons]
Name: "{group}\DScaler"; Filename: "{app}\DScaler.exe"
Name: "{group}\DScaler on the Web"; Filename: "{app}\DScaler.url"
Name: "{group}\Support Forum"; Filename: "{app}\Support.url"
;Shortcut to help file comes here.
Name: "{userdesktop}\DScaler"; Filename: "{app}\DScaler.exe"; MinVersion: 4,4; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\DScaler"; Filename: "{app}\DScaler.exe"; MinVersion: 4,4; Tasks: quicklaunchicon

[Run]
Filename: "{app}\DScaler.exe"; Parameters: "/driverinstall"
Filename: "{app}\DScaler.exe"; Description: "Launch DScaler"; Flags: nowait postinstall skipifsilent

[UninstallRun]
Filename: "{app}\DScaler.exe"; Parameters: "/driveruninstall"

[UninstallDelete]
Type: files; Name: "{app}\DScaler.url"
Type: files; Name: "{app}\Support.url"
Type: files; Name: "{app}\dscaler.txt"
Type: files; Name: "{app}\crashinfo.txt"

