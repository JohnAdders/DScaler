# Microsoft Developer Studio Project File - Name="DScaler" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=DScaler - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DScaler.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DScaler.mak" CFG="DScaler - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DScaler - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "DScaler - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DScaler - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DScaler___Win32_Debug0"
# PROP BASE Intermediate_Dir "DScaler___Win32_Debug0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /W3 /Gm /GX /Zi /Od /I "..\driver\include\\" /D "_DEBUG" /D "_WINDOWS" /D "WIN32" /D "_MBCS" /FR /YX /FD /c
# ADD CPP /nologo /G6 /MTd /W3 /GX /ZI /Od /Op /I "..\api" /D "_DEBUG" /D "_WINDOWS" /D "WIN32" /D "_MBCS" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /D "_DEBUG" /mktyplib203 /win32
# SUBTRACT BASE MTL /nologo
# ADD MTL /D "_DEBUG" /mktyplib203 /win32
# SUBTRACT MTL /nologo
# ADD BASE RSC /l 0x1 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo
# SUBTRACT BSC32 /Iu
LINK32=link.exe
# ADD BASE LINK32 ddraw.lib dxguid.lib kernel32.lib user32.lib gdi32.lib advapi32.lib winmm.lib comdlg32.lib ..\driver\bin\DScalerdrv.lib /nologo /subsystem:windows /profile /debug /machine:I386 /out:".\DScaler.exe"
# SUBTRACT BASE LINK32 /map /nodefaultlib
# ADD LINK32 ..\Driver\DSDrv\Debug\DSDrv.lib ddraw.lib dxguid.lib winmm.lib COMCTL32.LIB version.lib htmlhelp.lib vfw32.lib /nologo /subsystem:windows /pdb:none /map:"..\Debug/DScaler.map" /debug /machine:I386 /out:"..\Debug\DScaler.exe"
# SUBTRACT LINK32 /profile
# Begin Special Build Tool
ProjDir=.
SOURCE="$(InputPath)"
PostBuild_Desc=Creating dbg file
PostBuild_Cmds=$(ProjDir)\..\Debug\mapconv.exe $(ProjDir)\..\Debug\DScaler.map $(ProjDir)\..\Debug\DScaler.dbg
# End Special Build Tool

!ELSEIF  "$(CFG)" == "DScaler - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "DScaler___Win32_Release"
# PROP BASE Intermediate_Dir "DScaler___Win32_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MT /W3 /Gi /Ot /I "..\driver\include\\" /D "NDEBUG" /D "_WINDOWS" /D "WIN32" /D "_MBCS" /YX /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /G6 /MT /W3 /Gi /GX /Ot /Ow /Oi /Op /Ob1 /Gf /Gy /I "..\api" /D "NDEBUG" /D "_WINDOWS" /D "WIN32" /D "_MBCS" /FAcs /Yu"stdafx.h" /FD /c
# SUBTRACT CPP /Ox /Og /Fr
# ADD BASE MTL /D "NDEBUG" /mktyplib203 /win32
# SUBTRACT BASE MTL /nologo
# ADD MTL /D "NDEBUG" /mktyplib203 /win32
# SUBTRACT MTL /nologo
# ADD BASE RSC /l 0x420 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo
# SUBTRACT BSC32 /Iu
LINK32=link.exe
# ADD BASE LINK32 ddraw.lib dxguid.lib kernel32.lib user32.lib gdi32.lib advapi32.lib winmm.lib comdlg32.lib ..\driver\bin\hwiodrv.lib /nologo /subsystem:windows /incremental:yes /machine:I386
# SUBTRACT BASE LINK32 /profile /map /debug /nodefaultlib
# ADD LINK32 ..\Driver\DSDrv\Release\DSdrv.lib ddraw.lib dxguid.lib winmm.lib COMCTL32.LIB version.lib htmlhelp.lib vfw32.lib COMMODE.OBJ /nologo /subsystem:windows /incremental:yes /map:"..\Release/DScaler.map" /machine:I386 /out:"..\Release\DScaler.exe"
# SUBTRACT LINK32 /profile /debug /nodefaultlib
# Begin Special Build Tool
ProjDir=.
SOURCE="$(InputPath)"
PostBuild_Cmds=$(ProjDir)\..\Release\mapconv.exe $(ProjDir)\..\Release\DScaler.map $(ProjDir)\..\Release\DScaler.dbg
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "DScaler - Win32 Debug"
# Name "DScaler - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\AspectDetect.cpp
# End Source File
# Begin Source File

SOURCE=.\AspectFilters.cpp
# End Source File
# Begin Source File

SOURCE=.\AspectGUI.cpp
# End Source File
# Begin Source File

SOURCE=.\AspectRatio.cpp
# End Source File
# Begin Source File

SOURCE=.\Audio.cpp
# End Source File
# Begin Source File

SOURCE=.\Bt848.cpp
# End Source File
# Begin Source File

SOURCE=.\Calibration.cpp
# End Source File
# Begin Source File

SOURCE=.\CPU.cpp
# End Source File
# Begin Source File

SOURCE=.\Crash.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\DebugLog.cpp
# End Source File
# Begin Source File

SOURCE=.\Deinterlace.cpp
# End Source File
# Begin Source File

SOURCE=.\Dialogs.cpp
# End Source File
# Begin Source File

SOURCE=.\Disasm.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\DScaler.cpp
# End Source File
# Begin Source File

SOURCE=.\DScaler.rc
# End Source File
# Begin Source File

SOURCE=.\DScalerApp.cpp
# End Source File
# Begin Source File

SOURCE=.\ErrorBox.cpp
# End Source File
# Begin Source File

SOURCE=.\FD_50Hz.cpp
# End Source File
# Begin Source File

SOURCE=.\FD_60Hz.cpp
# End Source File
# Begin Source File

SOURCE=.\FD_Common.cpp
# End Source File
# Begin Source File

SOURCE=.\FD_CommonFunctions.asm

!IF  "$(CFG)" == "DScaler - Win32 Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\FD_CommonFunctions.asm
InputName=FD_CommonFunctions

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Zd /coff /nologo /Fo$(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "DScaler - Win32 Release"

# Begin Custom Build
IntDir=.\Release
InputPath=.\FD_CommonFunctions.asm
InputName=FD_CommonFunctions

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo$(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\FieldTiming.cpp
# End Source File
# Begin Source File

SOURCE=.\Filter.cpp
# End Source File
# Begin Source File

SOURCE=.\HardwareDriver.cpp
# End Source File
# Begin Source File

SOURCE=.\HardwareMemory.cpp
# End Source File
# Begin Source File

SOURCE=.\HSListBox.cpp
# End Source File
# Begin Source File

SOURCE=.\I2C.cpp
# End Source File
# Begin Source File

SOURCE=.\MixerDev.cpp
# End Source File
# Begin Source File

SOURCE=.\OSD.cpp
# End Source File
# Begin Source File

SOURCE=.\Other.cpp
# End Source File
# Begin Source File

SOURCE=.\OutThreads.cpp
# End Source File
# Begin Source File

SOURCE=.\PCICard.cpp
# End Source File
# Begin Source File

SOURCE=.\ProgramList.cpp
# End Source File
# Begin Source File

SOURCE=.\Settings.cpp
# End Source File
# Begin Source File

SOURCE=.\SettingsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Splash.cpp
# End Source File
# Begin Source File

SOURCE=.\STATUS.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\TimeShift.cpp
# End Source File
# Begin Source File

SOURCE=.\TSOptionsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Tuner.cpp
# End Source File
# Begin Source File

SOURCE=.\TVCards.cpp
# End Source File
# Begin Source File

SOURCE=.\VBI.cpp
# End Source File
# Begin Source File

SOURCE=.\VBI_CCdecode.cpp
# End Source File
# Begin Source File

SOURCE=.\VBI_VideoText.cpp
# End Source File
# Begin Source File

SOURCE=.\VBI_WSSdecode.cpp
# End Source File
# Begin Source File

SOURCE=.\verstub.asm

!IF  "$(CFG)" == "DScaler - Win32 Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\verstub.asm
InputName=verstub

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Zd /coff /nologo /Fo$(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "DScaler - Win32 Release"

# Begin Custom Build
IntDir=.\Release
InputPath=.\verstub.asm
InputName=verstub

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo$(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\VideoSettings.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\AspectFilters.h
# End Source File
# Begin Source File

SOURCE=.\AspectRatio.h
# End Source File
# Begin Source File

SOURCE=.\AspectRect.h
# End Source File
# Begin Source File

SOURCE=.\Audio.h
# End Source File
# Begin Source File

SOURCE=.\bt848.h
# End Source File
# Begin Source File

SOURCE=.\Calibration.h
# End Source File
# Begin Source File

SOURCE=.\cpu.h
# End Source File
# Begin Source File

SOURCE=.\Crash.h
# End Source File
# Begin Source File

SOURCE=.\DebugLog.h
# End Source File
# Begin Source File

SOURCE=.\Deinterlace.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs.h
# End Source File
# Begin Source File

SOURCE=.\disasm.h
# End Source File
# Begin Source File

SOURCE=.\DScaler.H
# End Source File
# Begin Source File

SOURCE=.\DScalerApp.h
# End Source File
# Begin Source File

SOURCE=.\ErrorBox.h
# End Source File
# Begin Source File

SOURCE=.\FD_50Hz.h
# End Source File
# Begin Source File

SOURCE=.\FD_60Hz.h
# End Source File
# Begin Source File

SOURCE=.\FD_Common.h
# End Source File
# Begin Source File

SOURCE=.\FD_CommonFunctions.h
# End Source File
# Begin Source File

SOURCE=.\FieldTiming.h
# End Source File
# Begin Source File

SOURCE=.\Filter.h
# End Source File
# Begin Source File

SOURCE=.\HardwareDriver.h
# End Source File
# Begin Source File

SOURCE=.\HardwareMemory.h
# End Source File
# Begin Source File

SOURCE=.\HSListBox.h
# End Source File
# Begin Source File

SOURCE=.\i2c.h
# End Source File
# Begin Source File

SOURCE=.\MixerDev.h
# End Source File
# Begin Source File

SOURCE=.\OSD.h
# End Source File
# Begin Source File

SOURCE=.\Other.h
# End Source File
# Begin Source File

SOURCE=.\OutThreads.h
# End Source File
# Begin Source File

SOURCE=.\PCICard.h
# End Source File
# Begin Source File

SOURCE=.\ProgramList.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\settings.h
# End Source File
# Begin Source File

SOURCE=.\SettingsDlg.h
# End Source File
# Begin Source File

SOURCE=.\slider.h
# End Source File
# Begin Source File

SOURCE=.\Splash.h
# End Source File
# Begin Source File

SOURCE=.\status.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\TimeShift.h
# End Source File
# Begin Source File

SOURCE=.\TSOptionsDlg.h
# End Source File
# Begin Source File

SOURCE=.\Tuner.h
# End Source File
# Begin Source File

SOURCE=.\TVCards.h
# End Source File
# Begin Source File

SOURCE=.\vbi.h
# End Source File
# Begin Source File

SOURCE=.\VBI_CCdecode.h
# End Source File
# Begin Source File

SOURCE=.\VBI_VideoText.H
# End Source File
# Begin Source File

SOURCE=.\VBI_WSSdecode.h
# End Source File
# Begin Source File

SOURCE=.\VideoSettings.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\RES\DScaler.ico
# End Source File
# Begin Source File

SOURCE=.\Res\Startup.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\VTFRENCHX15X18.BMP
# End Source File
# Begin Source File

SOURCE=.\res\Vtx15x18.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\READ_ME_FIRST_NOW.txt
# End Source File
# End Target
# End Project
# Section DScaler : {F08DF954-8592-11D1-B16A-00C0F0283628}
# 	2:21:DefaultSinkHeaderFile:slider.h
# 	2:16:DefaultSinkClass:CSlider
# End Section
# Section DScaler : {F08DF952-8592-11D1-B16A-00C0F0283628}
# 	2:5:Class:CSlider
# 	2:10:HeaderFile:slider.h
# 	2:8:ImplFile:slider.cpp
# End Section
# Section DScaler : {7BF80981-BF32-101A-8BBB-00AA00300CAB}
# 	2:5:Class:CPicture
# 	2:10:HeaderFile:picture.h
# 	2:8:ImplFile:picture.cpp
# End Section
