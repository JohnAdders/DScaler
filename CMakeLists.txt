#/////////////////////////////////////////////////////////////////////////////
#// $Id: CMakeLists.txt,v 1.4 2008-03-26 14:55:25 adcockj Exp $
#/////////////////////////////////////////////////////////////////////////////
#// Copyright (c) 2008 John Adcock.  All rights reserved.
#/////////////////////////////////////////////////////////////////////////////
#//
#//  This file is subject to the terms of the GNU General Public License as
#//  published by the Free Software Foundation.  A copy of this license is
#//  included with this software distribution in the file COPYING.  If you
#//  do not have a copy, you may obtain a copy by writing to the Free
#//  Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
#//
#//  This software is distributed in the hope that it will be useful,
#//  but WITHOUT ANY WARRANTY; without even the implied warranty of
#//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#//  GNU General Public License for more details
#//
#/////////////////////////////////////////////////////////////////////////////
#//
#// This file is the input file for the CMake process.
#// In order to create make file or visual studio projects to build the 
#// various projects you need to run cmake over this file.
#// For further help see ReadMe.txt
#//
#/////////////////////////////////////////////////////////////////////////////
#// CVS Log
#//
#// $Log: not supported by cvs2svn $
#// Revision 1.3  2008/03/10 17:41:46  adcockj
#// Update to cmake files
#//
#// Revision 1.2  2008/02/08 13:43:19  adcockj
#// Changes to support cmake compilation
#//
#//
#/////////////////////////////////////////////////////////////////////////////

#cmake_policy(VERSION 2.4)

# Names the solution
PROJECT (DScaler)

set(USEBT8X8 ON CACHE BOOL "Support Bt8x8 Chipset Input")
set(USECX2388X ON CACHE BOOL "Support CX2388X Chipset Input")
set(USESAA713X ON CACHE BOOL "Support SAA713X Chipset Input")
set(USEDSHOW ON CACHE BOOL "Support for DShow Input")

set(USESHAREDLIBS ON CACHE BOOL "Use shared libraries")

SET ( DScalerDir ${CMAKE_HOME_DIRECTORY}/DScaler )
SET ( RegLogDir ${CMAKE_HOME_DIRECTORY}/RegLog )
SET ( PlugTestDir ${CMAKE_HOME_DIRECTORY}/PlugTest )

SET ( EXECUTABLE_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY} CACHE PATH  "Single output directory for building all executables." )
SET ( LIBRARY_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY} CACHE PATH  "Single output directory for building all libaries." )
SET ( CMAKE_INSTALL_PREFIX ${CMAKE_HOME_DIRECTORY}  CACHE PATH  "Install path prefix, prepended onto install directories." )
SET ( CMAKE_BACKWARDS_COMPATIBILITY 2.4 )

# Macro that adds all files in a directory to a dll project
# with appropriate settings for a DScaler plugin
MACRO ( CreatePluginProject ProjectName )

    # get the file lists
    SET ( Dir ${CMAKE_HOME_DIRECTORY}/Plugins/${ProjectName} )

    # add your other extensions here (this could be probably done nicer)
    FILE ( GLOB DirSources ${Dir}/*.c ${Dir}/*.cpp ${Dir}/*.h ${Dir}/*.inl ${Dir}/*.rc ${Dir}/*.ico ${Dir}/*.asm ${Dir}/*.inc )

    # need to remove ceratin known files from projects
    FILE ( GLOB RemoveSources 
            ${Dir}/GreedyHMA.* 
            ${Dir}/DScalerCalls.c* 
            ${Dir}/TomsMoComp*.c* 
            ${Dir}/TomsMoComp*.rc 
            ${Dir}/DI_TomsMoCompSSE2.c 
            ${Dir}/TomsMoCompAllAvisynth.inc
         )
    FOREACH( REMOVESOURCE ${RemoveSources} )
        LIST ( REMOVE_ITEM DirSources ${REMOVESOURCE} )
    ENDFOREACH( REMOVESOURCE ${RemoveSources} )
    
    add_library(${ProjectName} SHARED ${DirSources} )

ENDMACRO ( CreatePluginProject )

# Macro that adds all files in a directory to a exe project
# with appropriate settings
MACRO ( CreateExeProject ProjectName ExtraSources)

    # get the file lists
    SET ( Dir ${CMAKE_HOME_DIRECTORY}/${ProjectName} )

    # add your other extensions here (this could be probably done nicer)
    FILE ( GLOB DirSources ${Dir}/*.c ${Dir}/*.cpp ${Dir}/*.h ${Dir}/*.inl ${Dir}/*.rc ${Dir}/*.ico )

    # add your other extensions here (this could be probably done nicer)
    FILE ( GLOB AsmSources ${Dir}/*.asm )

    # need to remove certain known files from projects
    FILE ( GLOB RemoveSources 
            ${Dir}/DVBT*.* 
         )
    
    FOREACH( REMOVESOURCE ${RemoveSources} )
        LIST ( REMOVE_ITEM DirSources ${REMOVESOURCE} )
    ENDFOREACH( REMOVESOURCE ${RemoveSources} )
    
    add_executable(${ProjectName} WIN32 ${DirSources} ${AsmSources} ${ExtraSources} )

ENDMACRO ( CreateExeProject )

# Macro that adds all files in a directory to a res project
# with appropriate settings
MACRO ( CreateResProject ProjectName Dir)

    # add your other extensions here (this could be probably done nicer)
    FILE ( GLOB DirSources ${Dir}/*.c ${Dir}/*.cpp ${Dir}/*.h ${Dir}/*.inl ${Dir}/*.rc ${Dir}/*.ico )

    add_library(${ProjectName} SHARED ${DirSources} )

    SET_TARGET_PROPERTIES(${ProjectName} PROPERTIES LINK_FLAGS "/NOENTRY")
ENDMACRO ( CreateResProject )

# Macro that adds all files in a directory to a exe project
# with appropriate settings
MACRO ( CreateCommandExeProject ProjectName ExtraSources)

    # get the file lists
    SET ( Dir ${CMAKE_HOME_DIRECTORY}/${ProjectName} )

    # add your other extensions here (this could be probably done nicer)
    FILE ( GLOB_RECURSE DirSources ${Dir}/*.c ${Dir}/*.cpp ${Dir}/*.h ${Dir}/*.inl ${Dir}/*.rc ${Dir}/*.ico ${Dir}/*.asm )

    # add your other extensions here (this could be probably done nicer)
    FILE ( GLOB_RECURSE AsmSources ${Dir}/*.asm )
    
    add_executable(${ProjectName} ${DirSources} ${AsmSources} ${ExtraSources})

ENDMACRO ( CreateCommandExeProject )

ADD_CUSTOM_COMMAND(
   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/VerStub.obj
   COMMAND ml /c /Zd /coff /nologo /Fo ${CMAKE_CURRENT_BINARY_DIR}/VerStub.obj ${DScalerDir}/VerStub.asm    
   DEPENDS ${DScalerDir}/VerStub.asm
   )

ADD_CUSTOM_COMMAND(
   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/FD_CommonFunctions.obj
   COMMAND ml /c /Zd /coff /nologo /Fo ${CMAKE_CURRENT_BINARY_DIR}/FD_CommonFunctions.obj ${DScalerDir}/FD_CommonFunctions.asm    
   DEPENDS ${DScalerDir}/FD_CommonFunctions.asm
   )


# Create a list of all plug-ins
SET( Plugins 
DI_Adaptive
DI_BlendedClip
DI_Bob
DI_EvenOnly
DI_Greedy
DI_Greedy2Frame
DI_GreedyH
DI_MoComp2
DI_OddOnly
DI_OldGame
DI_ScalerBob
DI_TomsMoComp
DI_TwoFrame
DI_VideoBob
DI_VideoWeave
DI_Weave
FLT_AdaptiveNoise
FLT_BlackWhite
FLT_Chroma
FLT_Colorimetry
FLT_ColourInversion
FLT_DScopeVIDEO
FLT_Gamma
FLT_GradualNoise
FLT_Histogram
FLT_LinearCorrection
FLT_LogoKill
FLT_LuminChromaShift
FLT_Mirror
FLT_Sharpness
FLT_TemporalComb
FLT_TNoise
)

# create a project for each plug-in
FOREACH( APLUGIN ${Plugins} )
    CreatePluginProject( ${APLUGIN} )
ENDFOREACH( APLUGIN ${Plugins} )

# create each of the exes and set dependencies and extra settings
FILE ( GLOB DScalerExtra
            ${DScalerDir}/dshowsource/*.cpp
            ${DScalerDir}/dshowsource/*.h
         )




CreateExeProject( DScaler "${DScalerExtra};${CMAKE_CURRENT_BINARY_DIR}/VerStub.obj;${CMAKE_CURRENT_BINARY_DIR}/FD_CommonFunctions.obj" )
CreateCommandExeProject( Mapconv "")
CreateCommandExeProject( Disasm  "")
CreateCommandExeProject( PlugTest ${DScalerDir}/CPU.cpp)
CreateCommandExeProject( VerInc "")
CreateExeProject( Reflasher "")
CreateExeProject( SendMsg "")

CreateResProject( DScalerRes ${CMAKE_HOME_DIRECTORY}/DScalerRes )
CreateResProject( BT8x8Res ${CMAKE_HOME_DIRECTORY}/DScalerResDbg/BT8x8Res )
CreateResProject( CX2388xRes ${CMAKE_HOME_DIRECTORY}/DScalerResDbg/CX2388xRes )
CreateResProject( SAA7134Res ${CMAKE_HOME_DIRECTORY}/DScalerResDbg/SAA7134Res )

FILE ( GLOB RegSpyExtra 
            ${DScalerDir}/BitVector.* 
            ${DScalerDir}/ErrorBox.* 
            ${DScalerDir}/HardwareDriver.* 
            ${DScalerDir}/PCICard.* 
         )

FILE ( GLOB RegSpyExtra2
            ${RegLogDir}/GenericCard.* 
         )
         
CreateExeProject( RegSpy "${RegSpyExtra};${RegSpyExtra2}" )
CreateExeProject( RegLog "${RegSpyExtra}")

ADD_DEPENDENCIES(DScaler ${Plugins})
ADD_DEPENDENCIES(DScaler Mapconv)
ADD_DEPENDENCIES(DScaler Disasm)
ADD_DEPENDENCIES(PlugTest ${Plugins})

include_directories( 
  ${CMAKE_HOME_DIRECTORY}/api
)

add_definitions( 
  -D_CRT_SECURE_NO_DEPRECATE
  -D_WIN32_DCOM
  -D_WINDOWS
  -D_X86_
  -DWINVER=0x0400
  -D_WIN32_WINNT=0x0400
)

if(USEBT8X8)
	add_definitions(-DWANT_BT8X8_SUPPORT)
ENDIF(USEBT8X8)

if(USESAA713X)
	add_definitions(-DWANT_SAA713X_SUPPORT)
ENDIF(USESAA713X)

if(USECX2388X)
	add_definitions(-DWANT_CX2388X_SUPPORT)
ENDIF(USECX2388X)

if(USEDSHOW)
	add_definitions(-DWANT_DSHOW_SUPPORT)
ENDIF(USEDSHOW)

add_definitions(-D_MT)
IF(USESHAREDLIBS)
    set(CMAKE_MFC_FLAG 2)
	add_definitions(-D_AFXDLL)
	add_definitions(-D_DLL)
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_AFX_ENABLE_INLINES")
ELSE(USESHAREDLIBS)
    set(CMAKE_MFC_FLAG 1)
    STRING(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    STRING(REPLACE "/MD" "/MT" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    STRING(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    STRING(REPLACE "/MD" "/MT" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
ENDIF(USESHAREDLIBS)

SET_TARGET_PROPERTIES(DScaler PROPERTIES COMPILE_FLAGS "-I${DScalerDir}")
SET_TARGET_PROPERTIES(PlugTest PROPERTIES COMPILE_FLAGS "-I${PlugTestDir}")

IF(USESHAREDLIBS)
ELSE(USESHAREDLIBS)
ENDIF(USESHAREDLIBS)

target_link_libraries( DScaler ${CMAKE_HOME_DIRECTORY}/ThirdParty/LibTiff/LibTiff.lib ${CMAKE_HOME_DIRECTORY}/ThirdParty/LibJpeg/LibJpeg.lib winmm comctl32 version htmlhelp vfw32 strmiids quartz setupapi d3d9)

ADD_CUSTOM_COMMAND(TARGET DScaler 
                    POST_BUILD
                    COMMAND ${DScalerDir}/CopyExtraFiles.bat ${CMAKE_CFG_INTDIR}
                    WORKING_DIRECTORY ${DScalerDir}
                    COMMENT "Copy Extra Files")