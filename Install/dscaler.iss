;////////////////////////////////////////////////////////////////////////////
;// $Id: dscaler.iss,v 1.26 2003-10-01 12:22:28 adcockj Exp $
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
;// Revision 1.25  2003/06/26 17:55:39  adcockj
;// Prep for new version
;//
;// Revision 1.24  2003/04/23 08:19:26  adcockj
;// Chnaged text on licencing page so that users don't have to accept the GPL
;//
;// Revision 1.23  2003/02/22 12:36:46  adcockj
;// Prepare for new alpha
;//
;// Revision 1.22  2003/02/04 17:23:09  adcockj
;// Added Warning to install
;//
;// Revision 1.21  2003/01/21 18:40:10  adcockj
;// Prepare for release 4.1.5
;//
;// Revision 1.20  2003/01/09 12:53:51  robmuller
;// Disable startup prompt. Upgrade logo killer and mirror filter.
;// Add "Components: main" to files that are always installed.
;//
;// Revision 1.19  2002/12/12 18:42:42  adcockj
;// Version 4.1.3
;//
;// Revision 1.18  2002/12/07 10:43:27  adcockj
;// Version changes used when compiling recent alpha
;//
;// Revision 1.17  2002/12/02 21:37:16  robmuller
;// Remove programs.xml on uninstall.
;//
;// Revision 1.16  2002/11/25 09:52:13  adcockj
;// Updated version
;//
;// Revision 1.15  2002/11/06 22:15:08  adcockj
;// Include DSRend in alpha build
;//
;// Revision 1.14  2002/11/02 11:42:37  robmuller
;// Added Skins folder.
;//
;// Revision 1.13  2002/09/17 17:32:36  tobbej
;// updated crashloging to same version as in latest virtualdub
;//
;// Revision 1.12  2002/08/19 19:00:57  adcockj
;// Updated Version
;//
;// Revision 1.11  2002/08/08 12:24:18  robmuller
;// Added SendMsg.
;//
;// Revision 1.10  2002/08/07 19:15:02  adcockj
;// Added Colour Inversion Filter
;//
;// Revision 1.9  2002/07/27 17:01:52  adcockj
;// Updated install for new alpha version
;//
;// Revision 1.8  2002/07/08 18:16:42  adcockj
;// final fixes fro alpha 3
;//
;// Revision 1.7  2002/07/08 17:41:14  adcockj
;// Getting REady for new version
;//
;// Revision 1.6  2002/06/22 20:53:04  robmuller
;// Added uninstall icon to the DScaler start menu.
;//
;// Revision 1.5  2002/06/19 18:51:05  adcockj
;// Changed text to include palha status
;//
;// Revision 1.4  2002/06/13 08:16:08  adcockj
;// Added Component groups and removed DSRend.dll temporarily
;//
;// Revision 1.3  2002/06/12 18:01:45  robmuller
;// (un)register dsrend.dll when (un)installing.
;// New name of debug log file.
;//
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
AppName=DScaler 4.1.8
AppVerName=DScaler 4.1.8
AppPublisherURL=http://www.dscaler.org
AppSupportURL=http://www.dscaler.org/phpBB/
AppUpdatesURL=http://www.dscaler.org
DefaultDirName={pf}\DScaler
DefaultGroupName=DScaler
AllowNoIcons=yes
AlwaysCreateUninstallIcon=yes
InfoBeforeFile=Warning.rtf
LicenseFile=gpl.rtf
AppMutex=DScaler
;required for installing the driver on NT platforms
AdminPrivilegesRequired=yes
DisableStartupPrompt=yes

[Messages]
BeveledLabel=DScaler
WizardLicense=GPL License Agreement
LicenseLabel2=Do you want to continue to install [name]? If you choose No, Setup will close.

[Components]
Name: "main"; Description: "Main Files"; Types: full compact custom; Flags: fixed
Name: "extra"; Description: "Rarely used or Historical Plug-ins"; Types: full
Name: "beta"; Description: "Plug-ins in Development"; Types:

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"; MinVersion: 4,4
Name: "quicklaunchicon"; Description: "Create a &Quick Launch icon"; GroupDescription: "Additional icons:"; MinVersion: 4,4; Flags: unchecked

[Files]
; main
Source: "..\Release\DScaler.exe"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\SendMsg.exe"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\RegSpy.exe"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\DI_Adaptive.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\DI_Greedy.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\DI_GreedyH.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\DI_MoComp2.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: beta
Source: "..\Release\DI_OldGame.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\DI_ScalerBob.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\DI_TomsMoComp.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\DI_Weave.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\FLT_AdaptiveNoise.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\FLT_Chroma.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: beta
Source: "..\Release\FLT_Gamma.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\FLT_GradualNoise.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\FLT_LinearCorrection.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: beta
Source: "..\Release\FLT_Sharpness.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\FLT_TemporalComb.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\FLT_TNoise.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\FLT_LogoKill.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\FLT_Histogram.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\FLT_Mirror.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\DScalerRes.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\DSRend.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Flags: regserver; Components: main
Source: "..\Release\dscaler.d3u"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\dscaler_intro.tif"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\DScaler.vdi"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\DSDrv4.sys"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\DSDrv4.vxd"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\channel.txt"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\dscaler.chm"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\Patterns\*.pat"; DestDir: "{app}\Patterns"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\Patterns\*.d3u"; DestDir: "{app}\Patterns"; CopyMode: alwaysoverwrite; Components: main
Source: "..\Release\Skins\default\*.bmp"; DestDir: "{app}\Skins\default"; CopyMode: alwaysoverwrite; Flags: recursesubdirs; Components: main
Source: "..\Release\Skins\default\*.ini"; DestDir: "{app}\Skins\default"; CopyMode: alwaysoverwrite; Flags: recursesubdirs; Components: main
Source: "..\ThirdParty\LibTiff\libtiff.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\ThirdParty\LibJpeg\libjpeg.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main
Source: "..\ThirdParty\zlib\zlib.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: main

; extra
Source: "..\Release\DI_BlendedClip.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite;  Components: extra
Source: "..\Release\DI_Bob.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite;  Components: extra
Source: "..\Release\DI_EvenOnly.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite;  Components: extra
Source: "..\Release\DI_OddOnly.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite;  Components: extra
Source: "..\Release\DI_TwoFrame.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite;  Components: extra
Source: "..\Release\DI_VideoBob.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite;  Components: extra
Source: "..\Release\DI_VideoWeave.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite;  Components: extra
Source: "..\Release\DI_Greedy2Frame.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite;  Components: extra
Source: "..\Release\FLT_ColourInversion.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite;  Components: extra
Source: "..\Release\FLT_Colorimetry.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite;  Components: extra

; beta
Source: "..\Release\FLT_LuminChromaShift.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: beta
Source: "..\Release\FLT_DScopeVIDEO.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: beta
Source: "..\Release\FLT_BlackWhite.dll"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Components: beta

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
Type: files; Name: "{app}\programs.xml"
Type: files; Name: "{app}\dscaler.log"
Type: files; Name: "{app}\crashinfo.txt"

