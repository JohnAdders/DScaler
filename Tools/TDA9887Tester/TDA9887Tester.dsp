# Microsoft Developer Studio Project File - Name="TDA9887Tester" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=TDA9887Tester - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "TDA9887Tester.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "TDA9887Tester.mak" CFG="TDA9887Tester - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "TDA9887Tester - Win32 Release" (basierend auf  "Win32 (x86) Application")
!MESSAGE "TDA9887Tester - Win32 Debug" (basierend auf  "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TDA9887Tester - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x407 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386 /out:"..\..\Release/TDA9887Tester.exe"

!ELSEIF  "$(CFG)" == "TDA9887Tester - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x407 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /out:"..\..\Debug/TDA9887Tester.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "TDA9887Tester - Win32 Release"
# Name "TDA9887Tester - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BT848Card.cpp
# End Source File
# Begin Source File

SOURCE=.\CX2388xCard.cpp
# End Source File
# Begin Source File

SOURCE=.\HardwareDriver.cpp
# End Source File
# Begin Source File

SOURCE=.\I2CBus.cpp
# End Source File
# Begin Source File

SOURCE=.\I2CBusForLineInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\PCICard.cpp
# End Source File
# Begin Source File

SOURCE=.\SAA7134Card.cpp
# End Source File
# Begin Source File

SOURCE=.\SAA7134I2CBus.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\TDA9887.cpp
# End Source File
# Begin Source File

SOURCE=.\TDA9887Tester.cpp
# End Source File
# Begin Source File

SOURCE=.\TDA9887Tester.rc
# End Source File
# Begin Source File

SOURCE=.\TDA9887TesterDlg.cpp
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\DScaler\Bt848_Defines.h
# End Source File
# Begin Source File

SOURCE=.\BT848Card.h
# End Source File
# Begin Source File

SOURCE=..\..\DScaler\CX2388x_Defines.h
# End Source File
# Begin Source File

SOURCE=.\CX2388xCard.h
# End Source File
# Begin Source File

SOURCE=.\HardwareDriver.h
# End Source File
# Begin Source File

SOURCE=.\I2CBus.h
# End Source File
# Begin Source File

SOURCE=.\I2CBusForLineInterface.h
# End Source File
# Begin Source File

SOURCE=.\I2CLineInterface.h
# End Source File
# Begin Source File

SOURCE=.\PCICard.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\SAA7134_Defines.h
# End Source File
# Begin Source File

SOURCE=.\SAA7134Card.h
# End Source File
# Begin Source File

SOURCE=.\SAA7134I2CBus.h
# End Source File
# Begin Source File

SOURCE=..\..\DScaler\SAA7134I2CInterface.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TDA9887.h
# End Source File
# Begin Source File

SOURCE=.\Tda9887Defines.h
# End Source File
# Begin Source File

SOURCE=.\TDA9887Tester.h
# End Source File
# Begin Source File

SOURCE=.\TDA9887TesterDlg.h
# End Source File
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\TDA9887Tester.ico
# End Source File
# Begin Source File

SOURCE=.\res\TDA9887Tester.rc2
# End Source File
# Begin Source File

SOURCE=.\res\xptheme.bin
# End Source File
# End Group
# End Target
# End Project
