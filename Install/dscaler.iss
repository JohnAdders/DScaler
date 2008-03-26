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
;// CVS Log
;//
;// $Log: not supported by cvs2svn $
;// Revision 1.36  2007/01/27 14:42:30  adcockj
;// release prep
;//
;// Revision 1.35  2005/12/18 20:37:20  adcockj
;// prepare for release
;//
;// Revision 1.34  2005/10/19 18:39:53  adcockj
;// version prep
;//
;// Revision 1.33  2005/10/12 10:54:38  adcockj
;// Release Prep
;//
;// Revision 1.32  2005/03/09 20:26:52  adcockj
;// put help back
;//
;// Revision 1.31  2005/03/08 13:48:04  adcockj
;// Moved items based on Rob's comments
;//
;// Revision 1.30  2005/03/07 18:01:58  adcockj
;// Prep for release
;//
;// Revision 1.29  2004/09/23 18:48:25  adcockj
;// Added new files to both projects
;//
;// Revision 1.28  2004/08/12 17:03:08  adcockj
;// getting ready for 4.1.10
;//
;// Revision 1.27  2004/03/13 14:02:13  adcockj
;// Prepare version 4.1.9
;//
;// Revision 1.26  2003/10/01 12:22:28  adcockj
;// more pre relaese stuff
;//
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
AppName=DScaler 4.1.17
AppVerName=DScaler 4.1.17
AppPublisherURL=http://www.dscaler.org
AppSupportURL=http://www.dscaler.org/phpBB/
AppUpdatesURL=http://www.dscaler.org
DefaultDirName={pf}\DScaler
DefaultGroupName=DScaler
AllowNoIcons=yes
InfoBeforeFile=Warning.rtf
LicenseFile=gpl.rtf
AppMutex=DScaler
;required for installing the driver on NT platforms
PrivilegesRequired=Admin
DisableStartupPrompt=yes

[Messages]
BeveledLabel=DScaler
WizardLicense=GPL License Agreement
LicenseLabel3=Do you want to continue to install [name]? If you choose No, Setup will close.
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
Source: "..\Release\DScaler.vdi"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\DSDrv4.sys"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\DSDrv4ia64.sys"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\DSDrv4amd64.sys"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\DSDrv4.vxd"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\channel.txt"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\DScaler.chm"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\cx2388xCards.ini"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\SAA713xCards.ini"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\Release\Patterns\*.pat"; DestDir: "{app}\Patterns"; Flags: ignoreversion; Components: main
Source: "..\Release\Patterns\*.d3u"; DestDir: "{app}\Patterns"; Flags: ignoreversion; Components: main
Source: "..\Release\Skins\*.bmp"; DestDir: "{app}\Skins"; Flags: ignoreversion recursesubdirs; Components: main
Source: "..\Release\Skins\*.ini"; DestDir: "{app}\Skins"; Flags: ignoreversion recursesubdirs; Components: main
Source: "..\ThirdParty\LibTiff\libtiff.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\ThirdParty\LibJpeg\libjpeg.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\ThirdParty\zlib\zlib.dll"; DestDir: "{app}"; Flags: ignoreversion; Components: main

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
Type: files; Name: "{app}\dscaler.ini"
Type: files; Name: "{app}\program.txt"
Type: files; Name: "{app}\programs.xml"
Type: files; Name: "{app}\dscaler.log"
Type: files; Name: "{app}\crashinfo.txt"

