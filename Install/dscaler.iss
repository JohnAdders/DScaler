;////////////////////////////////////////////////////////////////////////////
;// $Id: dscaler.iss,v 1.3 2002-06-12 18:01:45 robmuller Exp $
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
;// Revision 1.2  2002/06/06 18:15:52  robmuller
;// Added new files. Added AppMutex. Remove dscaler.ini and program.txt on uninstall.
;//
;// Revision 1.1  2002/03/06 22:50:58  robmuller
;// New InnoSetup script.
;//
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
AppMutex=DScaler
;required for installing the driver on NT platforms
AdminPrivilegesRequired=yes

[Messages]
BeveledLabel=DScaler

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"; MinVersion: 4,4
Name: "quicklaunchicon"; Description: "Create a &Quick Launch icon"; GroupDescription: "Additional icons:"; MinVersion: 4,4; Flags: unchecked

[Files]
Source: "..\Release\DScaler.exe"; DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "..\Release\DI_*.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "..\Release\FLT_*.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "..\Release\DScalerRes.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "..\Release\DSRend.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Flags: regserver
Source: "..\Release\dscaler.d3u"; DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "..\Release\dscaler_intro.tif"; DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "..\Release\DScaler.dbg"; DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "..\Release\DSDrv4.sys"; DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "..\Release\DSDrv4.vxd"; DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "..\Release\channel.txt"; DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "..\Release\dscaler.chm"; DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "..\Release\Patterns\*.pat"; DestDir: "{app}\Patterns"; CopyMode: alwaysoverwrite
Source: "..\Release\Patterns\*.d3u"; DestDir: "{app}\Patterns"; CopyMode: alwaysoverwrite
Source: "..\ThirdParty\LibTiff\libtiff.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "..\ThirdParty\LibJpeg\libjpeg.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite
Source: "..\ThirdParty\zlib\zlib.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite

[INI]
Filename: "{app}\DScaler.url"; Section: "InternetShortcut"; Key: "URL"; String: "http://www.dscaler.org"
Filename: "{app}\Support.url"; Section: "InternetShortcut"; Key: "URL"; String: "http://www.dscaler.org/phpBB/"

[Icons]
Name: "{group}\DScaler"; Filename: "{app}\DScaler.exe"
Name: "{group}\DScaler on the Web"; Filename: "{app}\DScaler.url"
Name: "{group}\Support Forum"; Filename: "{app}\Support.url"
Name: "{group}\DScaler Help"; Filename: "{app}\DScaler.chm"
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
Type: files; Name: "{app}\dscaler.ini"
Type: files; Name: "{app}\program.txt"
Type: files; Name: "{app}\dscaler.log"
Type: files; Name: "{app}\crashinfo.txt"

