# Microsoft Developer Studio Project File - Name="LibTiff" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=LibTiff - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "LibTiff.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "LibTiff.mak" CFG="LibTiff - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "LibTiff - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "LibTiff - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "LibTiff - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\libjpeg" /I "..\zlib" /D "WIN32" /D "_DEBUG" /D "_LIB" /D "_UNICODE" /GZ PRECOMP_VC7_TOBEREMOVED /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\libjpeg" /I "..\zlib" /D "WIN32" /D "_DEBUG" /D "_LIB" /D "_MCBS" /FR /GZ /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "LibTiff - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /I "..\libjpeg" /I "..\zlib" /D "WIN32" /D "NDEBUG" /D "_LIB" /D "_UNICODE" PRECOMP_VC7_TOBEREMOVED /c
# ADD CPP /nologo /MT /W3 /GX /Zi /I "..\libjpeg" /I "..\zlib" /D "WIN32" /D "NDEBUG" /D "_LIB" /D "_UNICODE" /FR /c
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

# Name "LibTiff - Win32 Debug"
# Name "LibTiff - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cc;cxx;def;odl;idl;hpj;bat;asm;asmx"
# Begin Source File

SOURCE=.\tif_aux.c
DEP_CPP_TIF_A=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tif_predict.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_close.c
DEP_CPP_TIF_C=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_codec.c
DEP_CPP_TIF_CO=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_color.c
DEP_CPP_TIF_COL=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_compress.c
DEP_CPP_TIF_COM=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_dir.c
DEP_CPP_TIF_D=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_dirinfo.c
DEP_CPP_TIF_DI=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_dirread.c
DEP_CPP_TIF_DIR=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_dirwrite.c
DEP_CPP_TIF_DIRW=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_dumpmode.c
DEP_CPP_TIF_DU=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_error.c
DEP_CPP_TIF_E=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_extension.c
DEP_CPP_TIF_EX=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_fax3.c
DEP_CPP_TIF_F=\
	".\t4.h"\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tif_fax3.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_fax3sm.c
DEP_CPP_TIF_FA=\
	".\tif_fax3.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_flush.c
DEP_CPP_TIF_FL=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_getimage.c
DEP_CPP_TIF_G=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_jpeg.c
DEP_CPP_TIF_J=\
	"..\LibJpeg\jconfig.h"\
	"..\LibJpeg\jerror.h"\
	"..\LibJpeg\jmorecfg.h"\
	"..\LibJpeg\jpegint.h"\
	"..\LibJpeg\jpeglib.h"\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_luv.c
DEP_CPP_TIF_L=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	".\uvcode.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_lzw.c
DEP_CPP_TIF_LZ=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tif_predict.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_msdos.c
DEP_CPP_TIF_M=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_next.c
DEP_CPP_TIF_N=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_ojpeg.c
DEP_CPP_TIF_O=\
	"..\LibJpeg\jconfig.h"\
	"..\LibJpeg\jerror.h"\
	"..\LibJpeg\jmorecfg.h"\
	"..\LibJpeg\jpegint.h"\
	"..\LibJpeg\jpeglib.h"\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_open.c
DEP_CPP_TIF_OP=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_packbits.c
DEP_CPP_TIF_P=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_pixarlog.c
DEP_CPP_TIF_PI=\
	"..\zlib\zconf.h"\
	"..\zlib\zlib.h"\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tif_predict.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_predict.c
DEP_CPP_TIF_PR=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tif_predict.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_print.c
DEP_CPP_TIF_PRI=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_read.c
DEP_CPP_TIF_R=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_stream.cxx
DEP_CPP_TIF_S=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_strip.c
DEP_CPP_TIF_ST=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_swab.c
DEP_CPP_TIF_SW=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_thunder.c
DEP_CPP_TIF_T=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_tile.c
DEP_CPP_TIF_TI=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_version.c
DEP_CPP_TIF_V=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_warning.c
DEP_CPP_TIF_W=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_write.c
DEP_CPP_TIF_WR=\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tif_zip.c
DEP_CPP_TIF_Z=\
	"..\zlib\zconf.h"\
	"..\zlib\zlib.h"\
	".\tif_config.h"\
	".\tif_dir.h"\
	".\tif_predict.h"\
	".\tiff.h"\
	".\tiffconf.h"\
	".\tiffio.h"\
	".\tiffiop.h"\
	".\tiffvers.h"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;inc;xsd"
# Begin Source File

SOURCE=.\tif_config.h
# End Source File
# Begin Source File

SOURCE=.\tif_dir.h
# End Source File
# Begin Source File

SOURCE=.\tif_fax3.h
# End Source File
# Begin Source File

SOURCE=.\tif_predict.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe;resx;tiff;tif;png;wav"
# End Group
# End Target
# End Project
