# Microsoft Developer Studio Project File - Name="DSDrvNT" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=DSDrvNT - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DSDrvNT.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DSDrvNT.mak" CFG="DSDrvNT - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DSDrvNT - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DSDrvNT - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DSDrvNT - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DSDrvNT___Win32_Debug0"
# PROP BASE Intermediate_Dir "DSDrvNT___Win32_Debug0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gz /W3 /Z7 /Oi /Gy /I "c:\ddk" /I "..\dll" /I "..\include" /D "_DEBUG" /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D _NT1X_=100 /D WINNT=1 /D _WIN32_WINNT=0x0400 /D WIN32_LEAN_AND_MEAN=1 /D DBG=1 /D DEVL=1 /D FPO=0 /D "NDEBUG" /D _DLL=1 /D _X86_=1 /D "_NTKERNEL_" /YX /FD /Zel -cbstring /QIfdiv- /QIf /GF /c
# ADD CPP /nologo /Gz /W3 /Z7 /Oi /Gy /I "..\..\api" /D "_DEBUG" /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D _NT1X_=100 /D WINNT=1 /D _WIN32_WINNT=0x0400 /D WIN32_LEAN_AND_MEAN=1 /D DBG=1 /D DEVL=1 /D FPO=0 /D "NDEBUG" /D _DLL=1 /D _X86_=1 /D "_NTKERNEL_" /YX /FD /Zel -cbstring /QIfdiv- /QIf /GF /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /i "i:\ddk.nt" /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 int64.lib ntoskrnl.lib hal.lib /nologo /base:"0x10000" /version:4.0 /entry:"DriverEntry" /pdb:none /debug /debugtype:both /machine:IX86 /nodefaultlib /out:"..\bin/DSDrvNT.sys" /libpath:"l:\ddk.nt\i386\checked" /driver /debug:notmapped,FULL /IGNORE:4001,4037,4039,4065,4078,4087,4089,4096 /MERGE:_PAGE=PAGE /MERGE:_TEXT=.text /SECTION:INIT,d /MERGE:.rdata=.text /FULLBUILD /RELEASE /FORCE:MULTIPLE /OPT:REF /OPTIDATA /align:0x20 /osversion:4.00 /subsystem:native
# ADD LINK32 int64.lib ntoskrnl.lib hal.lib /nologo /base:"0x10000" /version:4.0 /entry:"DriverEntry" /pdb:none /map:"DSDrvNT.map" /debug /machine:IX86 /nodefaultlib /out:"..\..\Debug\DSDrvNT.sys" /driver /debug:notmapped,FULL /IGNORE:4001,4037,4039,4065,4078,4087,4089,4096 /MERGE:_PAGE=PAGE /MERGE:_TEXT=.text /SECTION:INIT,d /MERGE:.rdata=.text /FULLBUILD /RELEASE /FORCE:MULTIPLE /OPT:REF /OPTIDATA /align:0x20 /osversion:4.00 /subsystem:native

!ELSEIF  "$(CFG)" == "DSDrvNT - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "DSDrvNT___Win32_Release"
# PROP BASE Intermediate_Dir "DSDrvNT___Win32_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gz /W3 /WX /Oy /Gy /I "c:\ddk" /I "..\dll" /I "..\include" /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D _NT1X_=100 /D WINNT=1 /D _WIN32_WINNT=0x0400 /D WIN32_LEAN_AND_MEAN=1 /D DEVL=1 /D FPO=1 /D "_IDWBUILD" /D "NDEBUG" /D _DLL=1 /D _X86_=1 /D "_NTKERNEL_" /Oxs /Zel -cbstring /QIfdiv- /QIf /GF /c
# ADD CPP /nologo /Gz /W3 /WX /Oy /Gy /I "..\..\api" /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D _NT1X_=100 /D WINNT=1 /D _WIN32_WINNT=0x0400 /D WIN32_LEAN_AND_MEAN=1 /D DEVL=1 /D FPO=1 /D "_IDWBUILD" /D "NDEBUG" /D _DLL=1 /D _X86_=1 /D "_NTKERNEL_" /FR /Oxs /Zel -cbstring /QIfdiv- /QIf /GF /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /i "$(BASEDIR)\inc" /i "..\inc" /d "NDEBUG"
# ADD RSC /l 0x409 /i "..\inc" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 int64.lib ntoskrnl.lib hal.lib /nologo /base:"0x10000" /version:4.0 /entry:"DriverEntry" /pdb:none /debug /debugtype:coff /machine:IX86 /nodefaultlib /out:".\i386\free\DSDrvNT.sys" /libpath:"$(BASEDIR)\lib\i386\free" /driver /debug:notmapped,MINIMAL /IGNORE:4001,4037,4039,4065,4070,4078,4087,4089,4096 /MERGE:_PAGE=PAGE /MERGE:_TEXT=.text /SECTION:INIT,d /MERGE:.rdata=.text /FULLBUILD /RELEASE /FORCE:MULTIPLE /OPT:REF /OPTIDATA /align:0x20 /osversion:4.00 /subsystem:native
# ADD LINK32 int64.lib ntoskrnl.lib hal.lib /nologo /base:"0x10000" /version:4.0 /entry:"DriverEntry" /pdb:none /debug /debugtype:coff /machine:IX86 /nodefaultlib /out:"..\..\Release\DSDrvNT.sys" /libpath:"$(BASEDIR)\lib\i386\free" /driver /debug:notmapped,MINIMAL /IGNORE:4001,4037,4039,4065,4070,4078,4087,4089,4096 /MERGE:_PAGE=PAGE /MERGE:_TEXT=.text /SECTION:INIT,d /MERGE:.rdata=.text /FULLBUILD /RELEASE /FORCE:MULTIPLE /OPT:REF /OPTIDATA /align:0x20 /osversion:4.00 /subsystem:native

!ENDIF 

# Begin Target

# Name "DSDrvNT - Win32 Debug"
# Name "DSDrvNT - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter ".c;.cpp"
# Begin Source File

SOURCE=.\DEBUGOUT.CPP
DEP_CPP_DEBUG=\
	".\Basetype.h"\
	".\DEBUGOUT.H"\
	{$(INCLUDE)}"alpharef.h"\
	{$(INCLUDE)}"basetsd.h"\
	{$(INCLUDE)}"bugcodes.h"\
	{$(INCLUDE)}"guiddef.h"\
	{$(INCLUDE)}"ia64reg.h"\
	{$(INCLUDE)}"ntddk.h"\
	{$(INCLUDE)}"ntdef.h"\
	{$(INCLUDE)}"ntiologc.h"\
	{$(INCLUDE)}"ntstatus.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DRVALLOC.CPP
DEP_CPP_DRVAL=\
	{$(INCLUDE)}"alpharef.h"\
	{$(INCLUDE)}"basetsd.h"\
	{$(INCLUDE)}"bugcodes.h"\
	{$(INCLUDE)}"guiddef.h"\
	{$(INCLUDE)}"ia64reg.h"\
	{$(INCLUDE)}"ntddk.h"\
	{$(INCLUDE)}"ntdef.h"\
	{$(INCLUDE)}"ntiologc.h"\
	{$(INCLUDE)}"ntstatus.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DSDrvNT.cpp
DEP_CPP_DSDRV=\
	"..\..\api\dsdrv.h"\
	".\Basetype.h"\
	".\DEBUGOUT.H"\
	".\Ioclass.h"\
	{$(INCLUDE)}"alpharef.h"\
	{$(INCLUDE)}"basetsd.h"\
	{$(INCLUDE)}"bugcodes.h"\
	{$(INCLUDE)}"devioctl.h"\
	{$(INCLUDE)}"guiddef.h"\
	{$(INCLUDE)}"ia64reg.h"\
	{$(INCLUDE)}"ntddk.h"\
	{$(INCLUDE)}"ntdef.h"\
	{$(INCLUDE)}"ntiologc.h"\
	{$(INCLUDE)}"ntstatus.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Ioclass.cpp
DEP_CPP_IOCLA=\
	"..\..\api\dsdrv.h"\
	".\Basetype.h"\
	".\DEBUGOUT.H"\
	".\Ioclass.h"\
	{$(INCLUDE)}"alpharef.h"\
	{$(INCLUDE)}"basetsd.h"\
	{$(INCLUDE)}"bugcodes.h"\
	{$(INCLUDE)}"devioctl.h"\
	{$(INCLUDE)}"guiddef.h"\
	{$(INCLUDE)}"ia64reg.h"\
	{$(INCLUDE)}"ntddk.h"\
	{$(INCLUDE)}"ntdef.h"\
	{$(INCLUDE)}"ntiologc.h"\
	{$(INCLUDE)}"ntstatus.h"\
	
# End Source File
# Begin Source File

SOURCE=.\PCIENUM.CPP
DEP_CPP_PCIEN=\
	"..\..\api\dsdrv.h"\
	".\Basetype.h"\
	".\DEBUGOUT.H"\
	".\Ioclass.h"\
	{$(INCLUDE)}"alpharef.h"\
	{$(INCLUDE)}"basetsd.h"\
	{$(INCLUDE)}"bugcodes.h"\
	{$(INCLUDE)}"devioctl.h"\
	{$(INCLUDE)}"guiddef.h"\
	{$(INCLUDE)}"ia64reg.h"\
	{$(INCLUDE)}"ntddk.h"\
	{$(INCLUDE)}"ntdef.h"\
	{$(INCLUDE)}"ntiologc.h"\
	{$(INCLUDE)}"ntstatus.h"\
	
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter ".rc;.mc"
# Begin Source File

SOURCE=.\DSDrvNT.RC
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=.\Basetype.h
# End Source File
# Begin Source File

SOURCE=..\common\DEBUGOUT.H
# End Source File
# Begin Source File

SOURCE=.\DEBUGOUT.H
# End Source File
# Begin Source File

SOURCE=.\Ioclass.h
# End Source File
# Begin Source File

SOURCE=.\RESOURCE.H
# End Source File
# End Group
# End Target
# End Project
