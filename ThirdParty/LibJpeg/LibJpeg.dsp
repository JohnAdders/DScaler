# Microsoft Developer Studio Project File - Name="LibJpeg" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=LibJpeg - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "LibJpeg.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "LibJpeg.mak" CFG="LibJpeg - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "LibJpeg - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "LibJpeg - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "LibJpeg - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "$(SolutionDir)$(ConfigurationName)"
# PROP BASE Intermediate_Dir "$(ConfigurationName)"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_LIB" /D "_UNICODE" /GZ PRECOMP_VC7_TOBEREMOVED /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_LIB" /D "_MCBS" /GZ /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "LibJpeg - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "$(SolutionDir)$(ConfigurationName)"
# PROP BASE Intermediate_Dir "$(ConfigurationName)"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /D "WIN32" /D "NDEBUG" /D "_LIB" /D "_UNICODE" PRECOMP_VC7_TOBEREMOVED /c
# ADD CPP /nologo /MT /W3 /GX /Zi /D "WIN32" /D "NDEBUG" /D "_LIB" /D "_MCBS" /FR /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "LibJpeg - Win32 Debug"
# Name "LibJpeg - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cc;cxx;def;odl;idl;hpj;bat;asm;asmx"
# Begin Source File

SOURCE=.\jcapimin.c
DEP_CPP_JCAPI=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jcapistd.c
DEP_CPP_JCAPIS=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jccoefct.c
DEP_CPP_JCCOE=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jccolor.c
DEP_CPP_JCCOL=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jcdctmgr.c
DEP_CPP_JCDCT=\
	".\jconfig.h"\
	".\jdct.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jchuff.c
DEP_CPP_JCHUF=\
	".\jchuff.h"\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jcinit.c
DEP_CPP_JCINI=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jcmainct.c
DEP_CPP_JCMAI=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jcmarker.c
DEP_CPP_JCMAR=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jcmaster.c
DEP_CPP_JCMAS=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jcomapi.c
DEP_CPP_JCOMA=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jcparam.c
DEP_CPP_JCPAR=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jcphuff.c
DEP_CPP_JCPHU=\
	".\jchuff.h"\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jcprepct.c
DEP_CPP_JCPRE=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jcsample.c
DEP_CPP_JCSAM=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jctrans.c
DEP_CPP_JCTRA=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jdapimin.c
DEP_CPP_JDAPI=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jdapistd.c
DEP_CPP_JDAPIS=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jdatadst.c
DEP_CPP_JDATA=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jdatasrc.c
DEP_CPP_JDATAS=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jdcoefct.c
DEP_CPP_JDCOE=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jdcolor.c
DEP_CPP_JDCOL=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jddctmgr.c
DEP_CPP_JDDCT=\
	".\jconfig.h"\
	".\jdct.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jdhuff.c
DEP_CPP_JDHUF=\
	".\jconfig.h"\
	".\jdhuff.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jdinput.c
DEP_CPP_JDINP=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jdmainct.c
DEP_CPP_JDMAI=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jdmarker.c
DEP_CPP_JDMAR=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jdmaster.c
DEP_CPP_JDMAS=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jdmerge.c
DEP_CPP_JDMER=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jdphuff.c
DEP_CPP_JDPHU=\
	".\jconfig.h"\
	".\jdhuff.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jdpostct.c
DEP_CPP_JDPOS=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jdsample.c
DEP_CPP_JDSAM=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jdtrans.c
DEP_CPP_JDTRA=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jerror.c
DEP_CPP_JERRO=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	".\jversion.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jfdctflt.c
DEP_CPP_JFDCT=\
	".\jconfig.h"\
	".\jdct.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jfdctfst.c
DEP_CPP_JFDCTF=\
	".\jconfig.h"\
	".\jdct.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jfdctint.c
DEP_CPP_JFDCTI=\
	".\jconfig.h"\
	".\jdct.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jidctflt.c
DEP_CPP_JIDCT=\
	".\jconfig.h"\
	".\jdct.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jidctfst.c
DEP_CPP_JIDCTF=\
	".\jconfig.h"\
	".\jdct.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jidctint.c
DEP_CPP_JIDCTI=\
	".\jconfig.h"\
	".\jdct.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jidctred.c
DEP_CPP_JIDCTR=\
	".\jconfig.h"\
	".\jdct.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jmemmgr.c
DEP_CPP_JMEMM=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmemsys.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jmemnobs.c
DEP_CPP_JMEMN=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmemsys.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jpegtran.c
DEP_CPP_JPEGT=\
	".\cderror.h"\
	".\cdjpeg.h"\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	".\jversion.h"\
	".\transupp.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jquant1.c
DEP_CPP_JQUAN=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jquant2.c
DEP_CPP_JQUANT=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\jutils.c
DEP_CPP_JUTIL=\
	".\jconfig.h"\
	".\jerror.h"\
	".\jinclude.h"\
	".\jmorecfg.h"\
	".\jpegint.h"\
	".\jpeglib.h"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;inc;xsd"
# Begin Source File

SOURCE=.\jchuff.h
# End Source File
# Begin Source File

SOURCE=.\jconfig.h
# End Source File
# Begin Source File

SOURCE=.\jdct.h
# End Source File
# Begin Source File

SOURCE=.\jdhuff.h
# End Source File
# Begin Source File

SOURCE=.\jerror.h
# End Source File
# Begin Source File

SOURCE=.\jinclude.h
# End Source File
# Begin Source File

SOURCE=.\jmemsys.h
# End Source File
# Begin Source File

SOURCE=.\jmorecfg.h
# End Source File
# Begin Source File

SOURCE=.\jpegint.h
# End Source File
# Begin Source File

SOURCE=.\jpeglib.h
# End Source File
# Begin Source File

SOURCE=.\jversion.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe;resx;tiff;tif;png;wav"
# End Group
# End Target
# End Project
