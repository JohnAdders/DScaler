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
# ADD CPP /nologo /G6 /MTd /W3 /GR /GX /ZI /Od /Op /I "..\api" /I ".\\" /D "_DEBUG" /D "_WINDOWS" /D "WIN32" /D "_MBCS" /D "_WIN32_DCOM" /FR /Yu"stdafx.h" /FD /c
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
# ADD LINK32 libtiff.lib libjpeg.lib ddraw.lib dxguid.lib winmm.lib COMCTL32.LIB version.lib htmlhelp.lib vfw32.lib strmiids.lib quartz.lib setupapi.lib /nologo /stack:0x4000,0x4000 /subsystem:windows /pdb:none /map:"..\Debug/DScaler.map" /debug /machine:I386 /out:"..\Debug\DScaler.exe" /libpath:"..\ThirdParty\LibTiff\\" /libpath:"..\ThirdParty\LibJpeg\\"
# Begin Special Build Tool
ProjDir=.
SOURCE="$(InputPath)"
PostBuild_Desc=Post build Steps
PostBuild_Cmds=$(ProjDir)\CopyExtraFiles.bat Debug NoDriver
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
# ADD CPP /nologo /G6 /MT /W3 /Gi /GR /GX /Ot /Ow /Oi /Op /Ob1 /Gf /Gy /I "..\api" /I ".\\" /D "NDEBUG" /D "_WINDOWS" /D "WIN32" /D "_MBCS" /D "_WIN32_DCOM" /FAcs /Fr /Yu"stdafx.h" /FD /c
# SUBTRACT CPP /Ox /Og
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
# ADD LINK32 COMMODE.OBJ libtiff.lib libjpeg.lib ddraw.lib dxguid.lib winmm.lib COMCTL32.LIB version.lib htmlhelp.lib vfw32.lib strmiids.lib quartz.lib setupapi.lib d3d9.lib /nologo /subsystem:windows /pdb:none /map:"..\Release/DScaler.map" /machine:I386 /out:"..\Release\DScaler.exe" /libpath:"..\ThirdParty\LibTiff\\" /libpath:"..\ThirdParty\LibJpeg\\"
# Begin Special Build Tool
ProjDir=.
SOURCE="$(InputPath)"
PostBuild_Desc=Post Build Steps
PostBuild_Cmds=$(ProjDir)\CopyExtraFiles.bat Release
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "DScaler - Win32 Debug"
# Name "DScaler - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\AD9882.cpp
# End Source File
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

SOURCE=.\AudioControls.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioDecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\AutoCriticalSection.cpp
# End Source File
# Begin Source File

SOURCE=.\avi.cpp
# End Source File
# Begin Source File

SOURCE=.\avi_audio.cpp
# End Source File
# Begin Source File

SOURCE=.\avi_file.cpp
# End Source File
# Begin Source File

SOURCE=.\avi_index.cpp
# End Source File
# Begin Source File

SOURCE=.\avi_video.cpp
# End Source File
# Begin Source File

SOURCE=.\dshowsource\BaseCrossbar.cpp
# End Source File
# Begin Source File

SOURCE=.\Bitmap.cpp
# End Source File
# Begin Source File

SOURCE=.\BitmapAsButton.cpp
# End Source File
# Begin Source File

SOURCE=.\BitVector.cpp
# End Source File
# Begin Source File

SOURCE=.\BT848_VoodooTV_IFdem.cpp
# End Source File
# Begin Source File

SOURCE=.\BT848Card.cpp
# End Source File
# Begin Source File

SOURCE=.\BT848Card_Atlas.cpp
# End Source File
# Begin Source File

SOURCE=.\BT848Card_Audio.cpp
# End Source File
# Begin Source File

SOURCE=.\BT848Card_PMS.cpp
# End Source File
# Begin Source File

SOURCE=.\BT848Card_Tuner.cpp
# End Source File
# Begin Source File

SOURCE=.\BT848Card_Types.cpp
# End Source File
# Begin Source File

SOURCE=.\BT848Provider.cpp
# End Source File
# Begin Source File

SOURCE=.\BT848Source.cpp
# End Source File
# Begin Source File

SOURCE=.\BT848Source_Audio.cpp
# End Source File
# Begin Source File

SOURCE=.\BT848Source_UI.cpp
# End Source File
# Begin Source File

SOURCE=.\Bt8x8GPIOAudioDecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\Calibration.cpp
# End Source File
# Begin Source File

SOURCE=.\dshowsource\CaptureDevice.cpp
# End Source File
# Begin Source File

SOURCE=.\Channels.cpp
# End Source File
# Begin Source File

SOURCE=.\CPU.cpp
# End Source File
# Begin Source File

SOURCE=.\Crash.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\Credits.cpp
# End Source File
# Begin Source File

SOURCE=.\CX2388xCard.cpp
# End Source File
# Begin Source File

SOURCE=.\CX2388xCard_Audio.cpp
# End Source File
# Begin Source File

SOURCE=.\CX2388xCard_H3D.cpp
# End Source File
# Begin Source File

SOURCE=.\CX2388xCard_Tuner.cpp
# End Source File
# Begin Source File

SOURCE=.\CX2388xCard_Types.cpp
# End Source File
# Begin Source File

SOURCE=.\CX2388xProvider.cpp
# End Source File
# Begin Source File

SOURCE=.\CX2388xSource.cpp
# End Source File
# Begin Source File

SOURCE=.\CX2388xSource_Audio.cpp
# End Source File
# Begin Source File

SOURCE=.\CX2388xSource_UI.cpp
# End Source File
# Begin Source File

SOURCE=.\D3D9Output.cpp
# End Source File
# Begin Source File

SOURCE=.\dshowsource\debug.cpp
# End Source File
# Begin Source File

SOURCE=.\DebugLog.cpp
# End Source File
# Begin Source File

SOURCE=.\Deinterlace.cpp
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DevEnum.cpp
# End Source File
# Begin Source File

SOURCE=.\Dialogs.cpp
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DSAudioDevicePage.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\DScaler.cpp
# End Source File
# Begin Source File

SOURCE=.\DScaler.rc
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DSFileSource.cpp
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DSGraph.cpp
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DShowAudioControls.cpp
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DShowBaseSource.cpp
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DShowDirectTuner.cpp
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DShowFileSource.cpp
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DShowGenericAudioControls.cpp
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DShowSeeking.cpp
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DShowTVAudio.cpp
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DSObject.cpp
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DSProvider.cpp
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DSSource.cpp
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DSSourceBase.cpp
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DSTVTuner.cpp
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DSVideoFormatPage.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\EPG.cpp
# End Source File
# Begin Source File

SOURCE=.\ErrorBox.cpp
# End Source File
# Begin Source File

SOURCE=.\events.cpp
# End Source File
# Begin Source File

SOURCE=.\dshowsource\exception.cpp
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

SOURCE=.\FD_Prog.cpp
# End Source File
# Begin Source File

SOURCE=.\FieldTiming.cpp
# End Source File
# Begin Source File

SOURCE=.\Filter.cpp
# End Source File
# Begin Source File

SOURCE=.\GenericTuner.cpp
# End Source File
# Begin Source File

SOURCE=.\HardwareDriver.cpp
# End Source File
# Begin Source File

SOURCE=.\HardwareMemory.cpp
# End Source File
# Begin Source File

SOURCE=.\HardwareSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\HierarchicalConfigParser.cpp
# End Source File
# Begin Source File

SOURCE=.\I2CBus.cpp
# End Source File
# Begin Source File

SOURCE=.\I2CBusForLineInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\I2CDevice.cpp
# End Source File
# Begin Source File

SOURCE=.\IOutput.cpp
# End Source File
# Begin Source File

SOURCE=.\ITuner.cpp
# End Source File
# Begin Source File

SOURCE=.\JpegHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\MixerDev.cpp
# End Source File
# Begin Source File

SOURCE=.\MSP34x0.cpp
# End Source File
# Begin Source File

SOURCE=.\MSP34x0AudioControls.cpp
# End Source File
# Begin Source File

SOURCE=.\MSP34x0AudioDecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\MSP34xx_revA.cpp
# End Source File
# Begin Source File

SOURCE=.\MSP34xx_revG.cpp
# End Source File
# Begin Source File

SOURCE=.\MT2032.cpp
# End Source File
# Begin Source File

SOURCE=.\MT2050.cpp
# End Source File
# Begin Source File

SOURCE=.\MultiFrames.cpp
# End Source File
# Begin Source File

SOURCE=.\OpenDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\OSD.cpp
# End Source File
# Begin Source File

SOURCE=.\OutReso.cpp
# End Source File
# Begin Source File

SOURCE=.\OutThreads.cpp
# End Source File
# Begin Source File

SOURCE=.\OverlayOutput.cpp
# End Source File
# Begin Source File

SOURCE=.\OverlaySettings.cpp
# End Source File
# Begin Source File

SOURCE=.\PaintingHDC.cpp
# End Source File
# Begin Source File

SOURCE=.\ParsingCommon.cpp
# End Source File
# Begin Source File

SOURCE=.\Pattern.cpp
# End Source File
# Begin Source File

SOURCE=.\PCICard.cpp
# End Source File
# Begin Source File

SOURCE=.\Perf.cpp
# End Source File
# Begin Source File

SOURCE=.\PIC16C54.cpp
# End Source File
# Begin Source File

SOURCE=.\dshowsource\PinEnum.cpp
# End Source File
# Begin Source File

SOURCE=.\ProgramList.cpp
# End Source File
# Begin Source File

SOURCE=.\Providers.cpp
# End Source File
# Begin Source File

SOURCE=.\RemoteInput.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\SAA7118.cpp
# End Source File
# Begin Source File

SOURCE=.\SAA7134Card.cpp
# End Source File
# Begin Source File

SOURCE=.\SAA7134Card_Audio.cpp
# End Source File
# Begin Source File

SOURCE=.\SAA7134Card_Tuner.cpp
# End Source File
# Begin Source File

SOURCE=.\SAA7134Card_Types.cpp
# End Source File
# Begin Source File

SOURCE=.\SAA7134Card_Video.cpp
# End Source File
# Begin Source File

SOURCE=.\SAA7134Common.cpp
# End Source File
# Begin Source File

SOURCE=.\SAA7134I2CBus.cpp
# End Source File
# Begin Source File

SOURCE=.\SAA7134Provider.cpp
# End Source File
# Begin Source File

SOURCE=.\SAA7134Source.cpp
# End Source File
# Begin Source File

SOURCE=.\SAA7134Source_Audio.cpp
# End Source File
# Begin Source File

SOURCE=.\SAA7134Source_UI.cpp
# End Source File
# Begin Source File

SOURCE=.\ScheduledRecording.cpp
# End Source File
# Begin Source File

SOURCE=.\Setting.cpp
# End Source File
# Begin Source File

SOURCE=.\SettingGroup.cpp
# End Source File
# Begin Source File

SOURCE=.\SettingHolder.cpp
# End Source File
# Begin Source File

SOURCE=.\Settings.cpp
# End Source File
# Begin Source File

SOURCE=.\SettingsMaster.cpp
# End Source File
# Begin Source File

SOURCE=.\SettingsPerChannel.cpp
# End Source File
# Begin Source File

SOURCE=.\dshowsource\SingleCrossbar.cpp
# End Source File
# Begin Source File

SOURCE=.\SizeSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\Source.cpp
# End Source File
# Begin Source File

SOURCE=.\SourceProvider.cpp
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

SOURCE=.\StillProvider.cpp
# End Source File
# Begin Source File

SOURCE=.\StillSource.cpp
# End Source File
# Begin Source File

SOURCE=.\SubItemCheckboxListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\TDA8275.cpp
# End Source File
# Begin Source File

SOURCE=.\TDA8290.cpp
# End Source File
# Begin Source File

SOURCE=.\TDA9873AudioDecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\TDA9874.cpp
# End Source File
# Begin Source File

SOURCE=.\TDA9874AudioDecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\TDA9875.cpp
# End Source File
# Begin Source File

SOURCE=.\TDA9875AudioControls.cpp
# End Source File
# Begin Source File

SOURCE=.\TDA9875AudioDecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\TDA9887.cpp
# End Source File
# Begin Source File

SOURCE=.\TEA5767.cpp
# End Source File
# Begin Source File

SOURCE=.\TiffHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\TimeShift.cpp
# End Source File
# Begin Source File

SOURCE=.\tinystr.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\tinyxml.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\tinyxmlerror.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\tinyxmlparser.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\ToolbarControl.cpp
# End Source File
# Begin Source File

SOURCE=.\Toolbars.cpp
# End Source File
# Begin Source File

SOURCE=.\ToolbarWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\TreeSettingsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\TreeSettingsGeneric.cpp
# End Source File
# Begin Source File

SOURCE=.\TreeSettingsOleProperties.cpp
# End Source File
# Begin Source File

SOURCE=.\TreeSettingsPage.cpp
# End Source File
# Begin Source File

SOURCE=.\TSCompressionDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\TSOptionsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\TunerID.cpp
# End Source File
# Begin Source File

SOURCE=.\TVFormats.cpp
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

SOURCE=.\VBI_VPSdecode.cpp
# End Source File
# Begin Source File

SOURCE=.\VBI_WSSdecode.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\VTCharacterSet.cpp
# End Source File
# Begin Source File

SOURCE=.\VTCommon.cpp
# End Source File
# Begin Source File

SOURCE=.\VTDecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\VTDrawer.cpp
# End Source File
# Begin Source File

SOURCE=.\VTMosaicGraphics.cpp
# End Source File
# Begin Source File

SOURCE=.\VTTopText.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowBorder.cpp
# End Source File
# Begin Source File

SOURCE=.\xml_cdata.cpp
# End Source File
# Begin Source File

SOURCE=.\xml_hash.cpp
# End Source File
# Begin Source File

SOURCE=.\xml_prolog.tab.cpp
# End Source File
# Begin Source File

SOURCE=.\xml_scan.yy.cpp
# End Source File
# Begin Source File

SOURCE=.\xmltv_db.cpp
# End Source File
# Begin Source File

SOURCE=.\xmltv_tags.cpp
# End Source File
# Begin Source File

SOURCE=.\xmltv_timestamp.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\AD9882.h
# End Source File
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

SOURCE=.\AudioControls.h
# End Source File
# Begin Source File

SOURCE=.\AudioDecoder.h
# End Source File
# Begin Source File

SOURCE=.\AutoCriticalSection.h
# End Source File
# Begin Source File

SOURCE=.\avi.h
# End Source File
# Begin Source File

SOURCE=.\dshowsource\BaseCrossbar.h
# End Source File
# Begin Source File

SOURCE=.\Bitmap.h
# End Source File
# Begin Source File

SOURCE=.\BitmapAsButton.h
# End Source File
# Begin Source File

SOURCE=.\BitVector.h
# End Source File
# Begin Source File

SOURCE=.\Bt848_Defines.h
# End Source File
# Begin Source File

SOURCE=.\BT848_VoodooTV_IFdem.h
# End Source File
# Begin Source File

SOURCE=.\BT848Card.h
# End Source File
# Begin Source File

SOURCE=.\BT848Provider.h
# End Source File
# Begin Source File

SOURCE=.\BT848Source.h
# End Source File
# Begin Source File

SOURCE=.\Bt8x8GPIOAudioDecoder.h
# End Source File
# Begin Source File

SOURCE=.\Bt8x8GPIOAudioDecoderAverTVPhoneNew.h
# End Source File
# Begin Source File

SOURCE=.\Bt8x8GPIOAudioDecoderAverTVPhoneOld.h
# End Source File
# Begin Source File

SOURCE=.\Bt8x8GPIOAudioDecoderGVBCTV3.h
# End Source File
# Begin Source File

SOURCE=.\Bt8x8GPIOAudioDecoderLT9415.h
# End Source File
# Begin Source File

SOURCE=.\Bt8x8GPIOAudioDecoderPVBt878P9B.h
# End Source File
# Begin Source File

SOURCE=.\Bt8x8GPIOAudioDecoderTerraTV.h
# End Source File
# Begin Source File

SOURCE=.\Bt8x8GPIOAudioDecoderWinDVR.h
# End Source File
# Begin Source File

SOURCE=.\Bt8x8GPIOAudioDecoderWinFast2000.h
# End Source File
# Begin Source File

SOURCE=.\dshowsource\btwdmprop.h
# End Source File
# Begin Source File

SOURCE=.\Calibration.h
# End Source File
# Begin Source File

SOURCE=.\dshowsource\CaptureDevice.h
# End Source File
# Begin Source File

SOURCE=.\Channels.h
# End Source File
# Begin Source File

SOURCE=.\cpu.h
# End Source File
# Begin Source File

SOURCE=.\Crash.h
# End Source File
# Begin Source File

SOURCE=.\Credits.h
# End Source File
# Begin Source File

SOURCE=.\CX2388x_Defines.h
# End Source File
# Begin Source File

SOURCE=.\CX2388xCard.h
# End Source File
# Begin Source File

SOURCE=.\CX2388xProvider.h
# End Source File
# Begin Source File

SOURCE=.\CX2388xSource.h
# End Source File
# Begin Source File

SOURCE=.\D3D9Output.h
# End Source File
# Begin Source File

SOURCE=.\dshowsource\debug.h
# End Source File
# Begin Source File

SOURCE=.\DebugLog.h
# End Source File
# Begin Source File

SOURCE=.\Deinterlace.h
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DevEnum.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs.h
# End Source File
# Begin Source File

SOURCE=.\disasm.h
# End Source File
# Begin Source File

SOURCE=..\Api\DS_Control.h
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DSAudioDevicePage.h
# End Source File
# Begin Source File

SOURCE=.\DScaler.H
# End Source File
# Begin Source File

SOURCE=.\DScalerApp.h
# End Source File
# Begin Source File

SOURCE=.\DScalerUtils.h
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DSFileSource.h
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DSGraph.h
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DShowAudioControls.h
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DShowBaseSource.h
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DShowDirectTuner.h
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DShowFileSource.h
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DShowGenericAudioControls.h
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DShowSeeking.h
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DShowTVAudio.h
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DSObject.h
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DSProvider.h
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DSSource.h
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DSSourceBase.h
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DSTVTuner.h
# End Source File
# Begin Source File

SOURCE=.\dshowsource\DSVideoFormatPage.h
# End Source File
# Begin Source File

SOURCE=.\EPG.h
# End Source File
# Begin Source File

SOURCE=.\ErrorBox.h
# End Source File
# Begin Source File

SOURCE=.\events.h
# End Source File
# Begin Source File

SOURCE=.\dshowsource\exception.h
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

SOURCE=.\FD_Prog.h
# End Source File
# Begin Source File

SOURCE=.\FieldTiming.h
# End Source File
# Begin Source File

SOURCE=.\Filter.h
# End Source File
# Begin Source File

SOURCE=.\GenericTuner.h
# End Source File
# Begin Source File

SOURCE=.\GradientStatic.h
# End Source File
# Begin Source File

SOURCE=.\HardwareDriver.h
# End Source File
# Begin Source File

SOURCE=.\HardwareMemory.h
# End Source File
# Begin Source File

SOURCE=.\HardwareSettings.h
# End Source File
# Begin Source File

SOURCE=.\HierarchicalConfigParser.h
# End Source File
# Begin Source File

SOURCE=.\HSListBox.h
# End Source File
# Begin Source File

SOURCE=.\I2CBus.h
# End Source File
# Begin Source File

SOURCE=.\I2CBusForLineInterface.h
# End Source File
# Begin Source File

SOURCE=.\I2CDevice.h
# End Source File
# Begin Source File

SOURCE=.\I2CLineInterface.h
# End Source File
# Begin Source File

SOURCE=.\IOutput.h
# End Source File
# Begin Source File

SOURCE=.\ISetting.h
# End Source File
# Begin Source File

SOURCE=.\ITuner.h
# End Source File
# Begin Source File

SOURCE=.\JpegHelper.h
# End Source File
# Begin Source File

SOURCE=.\list.h
# End Source File
# Begin Source File

SOURCE=.\MixerDev.h
# End Source File
# Begin Source File

SOURCE=.\MSP34x0.h
# End Source File
# Begin Source File

SOURCE=.\MSP34x0AudioControls.h
# End Source File
# Begin Source File

SOURCE=.\MSP34x0AudioDecoder.h
# End Source File
# Begin Source File

SOURCE=.\MT2032.h
# End Source File
# Begin Source File

SOURCE=.\MT2050.h
# End Source File
# Begin Source File

SOURCE=.\MultiFrames.h
# End Source File
# Begin Source File

SOURCE=.\NoTuner.h
# End Source File
# Begin Source File

SOURCE=.\OpenDlg.h
# End Source File
# Begin Source File

SOURCE=.\opendml.h
# End Source File
# Begin Source File

SOURCE=.\OSD.h
# End Source File
# Begin Source File

SOURCE=.\Other.h
# End Source File
# Begin Source File

SOURCE=.\OutReso.h
# End Source File
# Begin Source File

SOURCE=.\OutThreads.h
# End Source File
# Begin Source File

SOURCE=.\OverlayOutput.h
# End Source File
# Begin Source File

SOURCE=.\OverlaySettings.h
# End Source File
# Begin Source File

SOURCE=.\PaintingHDC.h
# End Source File
# Begin Source File

SOURCE=.\ParsingCommon.h
# End Source File
# Begin Source File

SOURCE=.\Pattern.h
# End Source File
# Begin Source File

SOURCE=.\PCICard.h
# End Source File
# Begin Source File

SOURCE=.\Perf.h
# End Source File
# Begin Source File

SOURCE=.\PIC16C54.h
# End Source File
# Begin Source File

SOURCE=.\dshowsource\PinEnum.h
# End Source File
# Begin Source File

SOURCE=.\ProgramList.h
# End Source File
# Begin Source File

SOURCE=.\Providers.h
# End Source File
# Begin Source File

SOURCE=.\RemoteInput.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\SAA7118.h
# End Source File
# Begin Source File

SOURCE=.\SAA7134_Defines.h
# End Source File
# Begin Source File

SOURCE=.\SAA7134Card.h
# End Source File
# Begin Source File

SOURCE=.\SAA7134Common.h
# End Source File
# Begin Source File

SOURCE=.\SAA7134I2CBus.h
# End Source File
# Begin Source File

SOURCE=.\SAA7134I2CInterface.h
# End Source File
# Begin Source File

SOURCE=.\SAA7134Provider.h
# End Source File
# Begin Source File

SOURCE=.\SAA7134Source.h
# End Source File
# Begin Source File

SOURCE=.\SchedMessageBox.h
# End Source File
# Begin Source File

SOURCE=.\ScheduledRecording.h
# End Source File
# Begin Source File

SOURCE=.\Setting.h
# End Source File
# Begin Source File

SOURCE=.\SettingConfig.h
# End Source File
# Begin Source File

SOURCE=.\SettingGroup.h
# End Source File
# Begin Source File

SOURCE=.\SettingHolder.h
# End Source File
# Begin Source File

SOURCE=.\SettingKey.h
# End Source File
# Begin Source File

SOURCE=.\SettingLimiter.h
# End Source File
# Begin Source File

SOURCE=.\SettingObject.h
# End Source File
# Begin Source File

SOURCE=.\SettingRepository.h
# End Source File
# Begin Source File

SOURCE=.\settings.h
# End Source File
# Begin Source File

SOURCE=.\SettingsMaster.h
# End Source File
# Begin Source File

SOURCE=.\SettingsPerChannel.h
# End Source File
# Begin Source File

SOURCE=.\SettingValue.h
# End Source File
# Begin Source File

SOURCE=.\dshowsource\SingleCrossbar.h
# End Source File
# Begin Source File

SOURCE=.\SizeSettings.h
# End Source File
# Begin Source File

SOURCE=.\slider.h
# End Source File
# Begin Source File

SOURCE=.\SoundChannel.h
# End Source File
# Begin Source File

SOURCE=.\Source.h
# End Source File
# Begin Source File

SOURCE=.\SourceProvider.h
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

SOURCE=.\StillProvider.h
# End Source File
# Begin Source File

SOURCE=.\StillSource.h
# End Source File
# Begin Source File

SOURCE=.\SubItemCheckboxListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\TDA8275.h
# End Source File
# Begin Source File

SOURCE=.\TDA8290.h
# End Source File
# Begin Source File

SOURCE=.\TDA9873AudioDecoder.h
# End Source File
# Begin Source File

SOURCE=.\TDA9874.h
# End Source File
# Begin Source File

SOURCE=.\TDA9874AudioDecoder.h
# End Source File
# Begin Source File

SOURCE=.\TDA9875.h
# End Source File
# Begin Source File

SOURCE=.\TDA9875AudioControls.h
# End Source File
# Begin Source File

SOURCE=.\TDA9875AudioDecoder.h
# End Source File
# Begin Source File

SOURCE=.\TDA9887.h
# End Source File
# Begin Source File

SOURCE=.\TEA5767.h
# End Source File
# Begin Source File

SOURCE=.\TiffHelper.h
# End Source File
# Begin Source File

SOURCE=.\TimeShift.h
# End Source File
# Begin Source File

SOURCE=.\tinystr.h
# End Source File
# Begin Source File

SOURCE=.\tinyxml.h
# End Source File
# Begin Source File

SOURCE=.\ToolbarControl.h
# End Source File
# Begin Source File

SOURCE=.\Toolbars.h
# End Source File
# Begin Source File

SOURCE=.\ToolbarWindow.h
# End Source File
# Begin Source File

SOURCE=.\TreeSettingsAssociations.h
# End Source File
# Begin Source File

SOURCE=.\TreeSettingsDlg.h
# End Source File
# Begin Source File

SOURCE=.\TreeSettingsGeneric.h
# End Source File
# Begin Source File

SOURCE=.\TreeSettingsOleProperties.h
# End Source File
# Begin Source File

SOURCE=.\TreeSettingsPage.h
# End Source File
# Begin Source File

SOURCE=.\TreeSettingsSettingConfig.h
# End Source File
# Begin Source File

SOURCE=.\TSCompressionDlg.h
# End Source File
# Begin Source File

SOURCE=.\TSOptionsDlg.h
# End Source File
# Begin Source File

SOURCE=.\TunerID.h
# End Source File
# Begin Source File

SOURCE=.\TVFormats.h
# End Source File
# Begin Source File

SOURCE=.\vbi.h
# End Source File
# Begin Source File

SOURCE=.\VBI_CCdecode.h
# End Source File
# Begin Source File

SOURCE=.\VBI_VideoText.h
# End Source File
# Begin Source File

SOURCE=.\VBI_VPSdecode.h
# End Source File
# Begin Source File

SOURCE=.\VBI_WSSdecode.h
# End Source File
# Begin Source File

SOURCE=.\VideoSettings.h
# End Source File
# Begin Source File

SOURCE=.\VTCharacterSet.h
# End Source File
# Begin Source File

SOURCE=.\VTCommon.h
# End Source File
# Begin Source File

SOURCE=.\VTDecoder.h
# End Source File
# Begin Source File

SOURCE=.\VTDrawer.h
# End Source File
# Begin Source File

SOURCE=.\VTMosaicGraphics.h
# End Source File
# Begin Source File

SOURCE=.\VTTopText.h
# End Source File
# Begin Source File

SOURCE=.\WindowBorder.h
# End Source File
# Begin Source File

SOURCE=.\xml_cdata.h
# End Source File
# Begin Source File

SOURCE=.\xml_hash.h
# End Source File
# Begin Source File

SOURCE=.\xml_prolog.tab.h
# End Source File
# Begin Source File

SOURCE=.\xmltv_db.h
# End Source File
# Begin Source File

SOURCE=.\xmltv_debug.h
# End Source File
# Begin Source File

SOURCE=.\xmltv_tags.h
# End Source File
# Begin Source File

SOURCE=.\xmltv_timestamp.h
# End Source File
# Begin Source File

SOURCE=.\xmltv_types.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\RES\DScaler.exe.manifest
# End Source File
# Begin Source File

SOURCE=.\RES\DScaler.ico
# End Source File
# Begin Source File

SOURCE=.\RES\DScaler.rc2
# End Source File
# Begin Source File

SOURCE=.\Res\Startup.bmp
# End Source File
# End Group
# Begin Group "ReadMe"

# PROP Default_Filter ".txt"
# Begin Source File

SOURCE=..\ReadMe.VS6.txt
# End Source File
# End Group
# Begin Group "CardList Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Release\CX2388xCards.ini
# End Source File
# Begin Source File

SOURCE=..\Release\SAA713xCards.ini
# End Source File
# End Group
# End Target
# End Project
# Section DScaler : {7BF80981-BF32-101A-8BBB-00AA00300CAB}
# 	2:5:Class:CPicture
# 	2:10:HeaderFile:picture.h
# 	2:8:ImplFile:picture.cpp
# End Section
# Section DScaler : {F08DF954-8592-11D1-B16A-00C0F0283628}
# 	2:21:DefaultSinkHeaderFile:slider.h
# 	2:16:DefaultSinkClass:CSlider
# End Section
# Section DScaler : {F08DF952-8592-11D1-B16A-00C0F0283628}
# 	2:5:Class:CSlider
# 	2:10:HeaderFile:slider.h
# 	2:8:ImplFile:slider.cpp
# End Section
