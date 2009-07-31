# Microsoft Developer Studio Project File - Name="DSRend" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=DSRend - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DSRend.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DSRend.mak" CFG="DSRend - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DSRend - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DSRend - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DSRend - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x41d /d "_DEBUG"
# ADD RSC /l 0x41d /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib strmiids.lib version.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\Debug/DSRend.dll" /pdbtype:sept
# Begin Custom Build - Performing registration
OutDir=.\Debug
TargetPath=\Source\deinterlace\DScaler\Debug\DSRend.dll
InputPath=\Source\deinterlace\DScaler\Debug\DSRend.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "DSRend - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "DSRend___Win32_Release"
# PROP BASE Intermediate_Dir "DSRend___Win32_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_ATL_STATIC_REGISTRY" /D "_ATL_MIN_CRT" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_ATL_STATIC_REGISTRY" /D "_ATL_MIN_CRT" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x41d /d "NDEBUG"
# ADD RSC /l 0x41d /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib strmiids.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib strmiids.lib version.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\Release/DSRend.dll"
# Begin Custom Build - Performing registration
OutDir=.\Release
TargetPath=\Source\deinterlace\DScaler\Release\DSRend.dll
InputPath=\Source\deinterlace\DScaler\Release\DSRend.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "DSRend - Win32 Debug"
# Name "DSRend - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AutoLockCriticalSection.cpp
# End Source File
# Begin Source File

SOURCE=.\ColorConverter.cpp
# End Source File
# Begin Source File

SOURCE=.\Cpu.cpp
# End Source File
# Begin Source File

SOURCE=.\DSRend.cpp
# End Source File
# Begin Source File

SOURCE=.\DSRend.def
# End Source File
# Begin Source File

SOURCE=.\DSRend.idl
# ADD MTL /tlb ".\DSRend.tlb" /h "DSRend.h" /iid "DSRend_i.c" /Oicf
# End Source File
# Begin Source File

SOURCE=.\DSRend.rc
# End Source File
# Begin Source File

SOURCE=.\DSRendAboutPage.cpp
# End Source File
# Begin Source File

SOURCE=.\DSRendFilter.cpp
# End Source File
# Begin Source File

SOURCE=.\DSRendInPin.cpp
# End Source File
# Begin Source File

SOURCE=.\DSRendQualityPage.cpp
# End Source File
# Begin Source File

SOURCE=.\Event.cpp
# End Source File
# Begin Source File

SOURCE=.\FieldBufferHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\mediatypes.cpp
# End Source File
# Begin Source File

SOURCE=.\mem.asm

!IF  "$(CFG)" == "DSRend - Win32 Debug"

# Begin Custom Build
IntDir=.\Debug
InputPath=.\mem.asm
InputName=mem

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Zd /coff /nologo /Fo$(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "DSRend - Win32 Release"

# Begin Custom Build
IntDir=.\Release
InputPath=.\mem.asm
InputName=mem

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo$(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\memalloc.cpp
# End Source File
# Begin Source File

SOURCE=.\PersistStream.cpp
# End Source File
# Begin Source File

SOURCE=.\SettingsPage.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AutoLockCriticalSection.h
# End Source File
# Begin Source File

SOURCE=.\ColorConverter.h
# End Source File
# Begin Source File

SOURCE=.\Cpu.h
# End Source File
# Begin Source File

SOURCE=.\CustomComObj.h
# End Source File
# Begin Source File

SOURCE=.\DSRendAboutPage.h
# End Source File
# Begin Source File

SOURCE=.\DSRendFilter.h
# End Source File
# Begin Source File

SOURCE=.\DSRendInPin.h
# End Source File
# Begin Source File

SOURCE=.\DSRendQualityPage.h
# End Source File
# Begin Source File

SOURCE=.\Event.h
# End Source File
# Begin Source File

SOURCE=.\FieldBufferHandler.h
# End Source File
# Begin Source File

SOURCE=.\mediatypes.h
# End Source File
# Begin Source File

SOURCE=.\mem.h
# End Source File
# Begin Source File

SOURCE=.\PersistStream.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\SettingsPage.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\DSRendAboutPage.rgs
# End Source File
# Begin Source File

SOURCE=.\DSRendFilter.rgs
# End Source File
# Begin Source File

SOURCE=.\DSRendInPin.rgs
# End Source File
# Begin Source File

SOURCE=.\DSRendQualityPage.rgs
# End Source File
# Begin Source File

SOURCE=.\SettingsPage.rgs
# End Source File
# End Group
# End Target
# End Project
# Section DSRend : {69562074-7573-6C61-2053-747564696F5C}
# 	1:22:IDS_TITLECSettingsPage:108
# 	1:17:IDR_CSETTINGSPAGE:111
# 	1:25:IDS_HELPFILECSettingsPage:109
# 	1:26:IDS_DOCSTRINGCSettingsPage:110
# 	1:17:IDD_CSETTINGSPAGE:112
# End Section
# Section DSRend : {03014021-60FF-0321-FF40-0103FF6825E3}
# 	1:30:IDS_DOCSTRINGDSRendQualityPage:105
# 	1:21:IDD_DSRENDQUALITYPAGE:107
# 	1:26:IDS_TITLEDSRendQualityPage:103
# 	1:29:IDS_HELPFILEDSRendQualityPage:104
# 	1:21:IDR_DSRENDQUALITYPAGE:106
# End Section
# Section DSRend : {00700000-0000-0000-0000-000077088800}
# 	1:28:IDS_DOCSTRINGDSRendAboutPage:115
# 	1:19:IDD_DSRENDABOUTPAGE:117
# 	1:24:IDS_TITLEDSRendAboutPage:113
# 	1:27:IDS_HELPFILEDSRendAboutPage:114
# 	1:19:IDR_DSRENDABOUTPAGE:116
# End Section
