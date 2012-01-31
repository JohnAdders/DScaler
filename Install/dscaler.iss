;////////////////////////////////////////////////////////////////////////////
;// $Id: dscaler.iss,v 1.37 2008-03-26 15:13:53 adcockj Exp $
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

#define MyAppVersion GetFileVersion("..\Release\DScaler.exe")

[Setup]
AppName=DScaler
AppVerName=DScaler {#MyAppVersion}
AppPublisherURL=http://www.dscalerproject.org
AppSupportURL=http://www.dscalerproject.org/phpBB/
AppUpdatesURL=http://www.dscalerproject.org
AppVersion={#MyAppVersion}
DefaultDirName={pf}\DScaler
DefaultGroupName=DScaler
AllowNoIcons=yes
InfoBeforeFile=Warning.rtf
LicenseFile=gpl.rtf
AppMutex=DScaler
;required for installing the driver on NT platforms
PrivilegesRequired=Admin
DisableStartupPrompt=yes
AppCopyright=Copyright (C) 2012 DScaler Team
VersionInfoDescription=DScaler Setup
VersionInfoProductVersion={#MyAppVersion}
VersionInfoVersion={#MyAppVersion}
SetupIconFile=..\DScaler\Res\DScaler.ico


[Messages]
BeveledLabel=DScaler
WizardLicense=GPL License Agreement
LicenseLabel3=Do you want to continue to install [name]?.
LicenseAccepted=Yes, I would like to &continue
LicenseNotAccepted=&No
WizardInfoBefore=Warning
InfoBeforeLabel=Please read the following important warning before continuing.
InfoBeforeClickLabel=When you are ready and happy to continue with Setup, click Next.

[Components]
Name: "main"; Description: "Main Files"; Types: full compact custom; Flags: fixed
Name: "extra"; Description: "Rarely used or Historical Plug-ins"; Types: full
Name: "beta"; Description: "Plug-ins in Development"; Types:

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"; MinVersion: 4,4
Name: "quicklaunchicon"; Description: "Create a &Quick Launch icon"; GroupDescription: "Additional icons:"; MinVersion: 4,4; Flags: unchecked

[Dirs]
Name: "{userappdata}\DScaler4"; MinVersion: 5,5; AfterInstall: CopyFilesToUsersArea();

[Files]
; main
Source: "..\Release\DScaler.exe"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\SendMsg.exe"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\RegSpy.exe"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\DI_Adaptive.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\DI_Greedy.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\DI_GreedyH.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\DI_MoComp2.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\DI_OldGame.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\DI_ScalerBob.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\DI_TomsMoComp.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\DI_Weave.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\FLT_AdaptiveNoise.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\FLT_Gamma.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\FLT_GradualNoise.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\FLT_Sharpness.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\FLT_TemporalComb.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\FLT_TNoise.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\FLT_LogoKill.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\FLT_Histogram.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\FLT_Mirror.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\DScalerRes.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\DSRend.dll"; DestDir: "{app}"; Flags: ignoreversion regserver; Components: main
Source: "..\Release\dscaler.d3u"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\dscaler_intro.tif"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\DSDrv4.sys"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\DSDrv4ia64.sys"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\DSDrv4amd64.sys"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\DSDrv4nt4.sys"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\DSDrv4.vxd"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\channel.txt"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\DScaler.chm"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\cx2388xCards.ini"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\SAA713xCards.ini"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\Patterns\*.pat"; DestDir: "{app}\Patterns"; Flags: ignoreversion; Components: main
Source: "..\Release\Patterns\*.d3u"; DestDir: "{app}\Patterns"; Flags: ignoreversion; Components: main
Source: "..\Release\Skins\*.bmp"; DestDir: "{app}\Skins"; Flags: ignoreversion recursesubdirs; Components: main
Source: "..\Release\Skins\*.ini"; DestDir: "{app}\Skins"; Flags: ignoreversion recursesubdirs; Components: main

; extra
Source: "..\Release\DI_BlendedClip.dll"; DestDir: "{app}"; Flags: ignoreversion;  Components: extra
Source: "..\Release\DI_Bob.dll"; DestDir: "{app}"; Flags: ignoreversion;  Components: extra
Source: "..\Release\DI_EvenOnly.dll"; DestDir: "{app}"; Flags: ignoreversion;  Components: extra
Source: "..\Release\DI_OddOnly.dll"; DestDir: "{app}"; Flags: ignoreversion;  Components: extra
Source: "..\Release\DI_TwoFrame.dll"; DestDir: "{app}"; Flags: ignoreversion;  Components: extra
Source: "..\Release\DI_VideoBob.dll"; DestDir: "{app}"; Flags: ignoreversion;  Components: extra
Source: "..\Release\DI_VideoWeave.dll"; DestDir: "{app}"; Flags: ignoreversion;  Components: extra
Source: "..\Release\DI_Greedy2Frame.dll"; DestDir: "{app}"; Flags: ignoreversion;  Components: extra
Source: "..\Release\FLT_ColourInversion.dll"; DestDir: "{app}"; Flags: ignoreversion;  Components: extra
Source: "..\Release\FLT_Colorimetry.dll"; DestDir: "{app}"; Flags: ignoreversion;  Components: extra
Source: "..\Release\FLT_Chroma.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: extra
Source: "..\Release\FLT_LinearCorrection.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: extra

; beta
Source: "..\Release\FLT_LuminChromaShift.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: beta
Source: "..\Release\FLT_DScopeVIDEO.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: beta
Source: "..\Release\FLT_BlackWhite.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: beta

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
Type: files; Name: "{app}\dscaler.ini"; OnlyBelowVersion: 5,5;
Type: files; Name: "{userappdata}\DScaler4\dscaler.ini"; MinVersion: 5,5;
Type: files; Name: "{app}\program.txt"; OnlyBelowVersion: 5,5;
Type: files; Name: "{userappdata}\DScaler4\program.txt"; MinVersion: 5,5;
Type: files; Name: "{app}\programs.xml"; OnlyBelowVersion: 5,5;
Type: files; Name: "{userappdata}\DScaler4\programs.xml"; MinVersion: 5,5;
Type: files; Name: "{app}\dscaler.log"; OnlyBelowVersion: 5,5;
Type: files; Name: "{userappdata}\DScaler4\dscaler.log"; MinVersion: 5,5;
Type: files; Name: "{app}\crashinfo.txt"; OnlyBelowVersion: 5,5;
Type: files; Name: "{userappdata}\DScaler4\crashinfo.txt"; MinVersion: 5,5;
Type: files; Name: "{app}\minidump.dmp"; OnlyBelowVersion: 5,5;
Type: files; Name: "{userappdata}\DScaler4\minidump.dmp"; MinVersion: 5,5;
Type: dirifempty; Name: "{userappdata}\DScaler4"; MinVersion: 5,5;

;////////////////////////////////////////////////////////////////////////////
;// Copy all the settings files from the app dir to the
;// User app data directory if they exist
;// Should only be called XP and up
;// As call is attached to MinVersion 5,5
;/////////////////////////////////////////////////////////////////////////////
[Code]
procedure MoveFileToUsersArea(FileName: String);
var
  OldFile: String;
  NewFile: String;
begin
  OldFile := ExpandConstant('{app}') + '\' + FileName;
  if FileExists(OldFile) then begin
    NewFile := ExpandConstant('{userappdata}') + '\DScaler4\' + FileName;
    FileCopy(OldFile, NewFile, True);
    DeleteFile(OldFile);
  end;
end;

procedure CopyFilesToUsersArea();
begin
  MoveFileToUsersArea('DScaler.ini');
  MoveFileToUsersArea('DScaler.log');
  MoveFileToUsersArea('program.txt');
  MoveFileToUsersArea('programs.xml');
end;






