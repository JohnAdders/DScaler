# Microsoft Developer Studio Project File - Name="DSDrv95" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=DSDrv95 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DSDrv95.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DSDrv95.mak" CFG="DSDrv95 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DSDrv95 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DSDrv95 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DSDrv95 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "DSDrv95___Win32_Release"
# PROP BASE Intermediate_Dir "DSDrv95___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DSDrv95_EXPORTS" /YX /FD /c
# ADD CPP /nologo /Zp1 /MT /W3 /Zi /Ot /Ow /I "..\..\Api" /D "_VXD_" /D "IS_32" /D DEBLEVEL=1 /D "WIN95" /D "WIN40COMPAT" /D "NDEBUG" /D "DSDrv95_EXPORTS" /YX /FD /Gs /c
# SUBTRACT CPP /Ox /Og /Oi
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /win32
# SUBTRACT MTL /mktyplib203
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 vxdwraps.clb /nologo /machine:I386 /nodefaultlib /out:"..\..\Release\DSDrv4.vxd" /vxd /ignore:4039,4078
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "DSDrv95 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DSDrv95___Win32_Debug"
# PROP BASE Intermediate_Dir "DSDrv95___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DSDrv95_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /Zp1 /MTd /W3 /GX /Zi /Od /I "..\..\Api" /D "_VXD_" /D "IS_32" /D DEBLEVEL=1 /D "WIN95" /D "WIN40COMPAT" /D "_DEBUG" /D "DSDrv95_EXPORTS" /YX /FD /Gs /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /win32
# SUBTRACT MTL /mktyplib203
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 vxdwraps.clb /nologo /incremental:no /map /debug /machine:I386 /nodefaultlib /out:"..\..\Debug\DSDrv4.vxd" /vxd /ignore:4039,4078
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "DSDrv95 - Win32 Release"
# Name "DSDrv95 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\CRTL.CPP
# End Source File
# Begin Source File

SOURCE=.\DEBUGOUT.CPP
# End Source File
# Begin Source File

SOURCE=.\DSDrv95.CPP
# End Source File
# Begin Source File

SOURCE=.\DSDrv95.def
# End Source File
# Begin Source File

SOURCE=.\Ioclass.cpp
# End Source File
# Begin Source File

SOURCE=.\PCIENUM.CPP
# End Source File
# Begin Source File

SOURCE=.\VXDSTUB.ASM

!IF  "$(CFG)" == "DSDrv95 - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\VXDSTUB.ASM

"$(OutDir)\vxdstub.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo NOTE: If following fails, please add a path containing ml.exe to Tools, Options, Directoroies, Executable Files. 
	ml -coff -DBLD_COFF -DIS_32 -W2 -c -Cx -DMASM6 -Fo$(OutDir)\vxdstub.obj vxdstub.asm 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "DSDrv95 - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\VXDSTUB.ASM

"$(OutDir)\vxdstub.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo NOTE: If following fails, please add a path containing ml.exe to Tools, Options, Directoroies, Executable Files. 
	ml -coff -DBLD_COFF -DIS_32 -W2 -Zi -c -Cx -DMASM6 -DDEBLEVEL=1 -DDEBUG -Fo$(OutDir)\vxdstub.obj vxdstub.asm 
	
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Basetype.h
# End Source File
# Begin Source File

SOURCE=.\DEBUGOUT.H
# End Source File
# Begin Source File

SOURCE=.\Ioclass.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\DSDrv95.rc
# PROP Exclude_From_Build 1
# End Source File
# End Group
# End Target
# End Project
