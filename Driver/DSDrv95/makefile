#****************************************************************************
#                                                                           *
# THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY     *
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE       *
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR     *
# PURPOSE.                                                                  *
#                                                                           *
# Copyright 1993-95  Microsoft Corporation.  All Rights Reserved.           *
#                                                                           *
#****************************************************************************

BUILD_TYPE=base
!INCLUDE $(BASEDIR)\inc\win_me\master.mk

DEVICE = DsDrv4

OBJDIR = obj

all: ..\..\Release\$(DEVICE).vxd

CFLAGS     = -nologo -DWIN32 -DCON -Di386 -D_X86_ -D_NTWIN -W3 -Gs -D_DEBUG -Zi -I$(INC32)
CVXDFLAGS  = -nologo -Zdp -Gs -c -D_VXD_ -DWIN95 -DIS_32 -Zl -DDEBLEVEL=1 -I$(INC32) -I..\..\Api
LFLAGS     = -machine:i386 -debug:notmapped,full -debugtype:cv -subsystem:console kernel32.lib 
AFLAGS     = -nologo -coff -DBLD_COFF -DIS_32 -W2 -Zd -c -Cx -DMASM6 -DDEBLEVEL=1 -DDEBUG -I$(INC32)

########## Base inference rules ###################

$(OBJDIR):
        @if not exist $(OBJDIR)\nul md $(OBJDIR)

$(OBJDIR)\DSDrv95.res: DSDrv95.rc
        rc -Fo$@ %s

$(OBJDIR)\Ioclass.obj: Ioclass.cpp
        cl $(CVXDFLAGS) -Fo$@ %s

$(OBJDIR)\PciEnum.obj: PciEnum.cpp
        cl $(CVXDFLAGS) -Fo$@ %s

$(OBJDIR)\DSDrv95.obj: DSDrv95.cpp
        cl $(CVXDFLAGS) -Fo$@ %s

$(OBJDIR)\Crtl.obj: Crtl.cpp
        cl $(CVXDFLAGS) -Fo$@ %s

$(OBJDIR)\DebugOut.obj: DebugOut.cpp
        cl $(CVXDFLAGS) -Fo$@ %s

$(OBJDIR)\vxdstub.obj: vxdstub.asm
        ml $(AFLAGS) -Fo$@ %s

$(OBJDIR)\vkdgko.obj: vkdgko.asm
        ml -nologo -coff -I$(INC32) -DBLD_COFF -W2 -c -Cx -DMASM6 -DSEGNUM=1 -Fo$@ %s

..\..\Release\DSDrv4.vxd: $(OBJDIR) $(OBJDIR)\Ioclass.obj $(OBJDIR)\DSDrv95.obj $(OBJDIR)\Crtl.obj \
    $(OBJDIR)\DebugOut.obj $(OBJDIR)\vxdstub.obj $(OBJDIR)\PCIEnum.obj $(OBJDIR)\DSDrv95.res \
                  DSDrv95.def
        echo >NUL @<<DSDrv4.crf
-MACHINE:i386 -DEBUG:NONE -PDB:NONE -ignore:4078 -ignore:4039
-DEF:DSDrv95.def -OUT:..\..\Release\DSDrv4.vxd -MAP:$(OBJDIR)\DSDrv4.map -LIBPATH:$(LIB32)
-VXD $(DDKROOT)\lib\win_me\vxdwraps.clb $(OBJDIR)\Ioclass.obj $(OBJDIR)\DSDrv95.obj $(OBJDIR)\Crtl.obj $(OBJDIR)\DebugOut.obj $(OBJDIR)\vxdstub.obj $(OBJDIR)\PCIEnum.obj  $(OBJDIR)\DSDrv95.res
<<
!ifndef PASS0ONLY
		@echo link -OUT:$@
        link -nologo @DSDrv4.crf
!endif

clean:
        -@if exist $(OBJDIR)\*.obj del $(OBJDIR)\*.obj
        -@if exist $(OBJDIR)\*.vxd del $(OBJDIR)\*.vxd
        -@if exist $(OBJDIR)\*.exp del $(OBJDIR)\*.exp
        -@if exist $(OBJDIR)\*.map del $(OBJDIR)\*.map
        -@if exist $(OBJDIR)\*.sym del $(OBJDIR)\*.sym
        -@if exist $(OBJDIR)\*.lib del $(OBJDIR)\*.lib
        -@if exist $(OBJDIR)\nul   rd  $(OBJDIR)
        -@if exist *.pdb del *.pdb
        -@if exist *.bak del *.bak
        
