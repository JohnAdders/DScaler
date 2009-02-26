;///////////////////////////////////////////////////////////////////////////
; $Id
;///////////////////////////////////////////////////////////////////////////
; Copyright (c) 2000 John Adcock.  All rights reserved.
;///////////////////////////////////////////////////////////////////////////
;
;   This file is subject to the terms of the GNU General Public License as
;   published by the Free Software Foundation.  A copy of this license is
;   included with this software distribution in the file COPYING.  If you
;   do not have a copy, you may obtain a copy by writing to the Free
;   Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
;
;   This software is distributed in the hope that it will be useful,
;   but WITHOUT ANY WARRANTY; without even the implied warranty of
;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;   GNU General Public License for more details
;///////////////////////////////////////////////////////////////////////////
;
; This software was based on hwiodrv from the FreeTV project Those portions are
; Copyright (C) Mathias Ellinger
;
;///////////////////////////////////////////////////////////////////////////

PAGE 58,132

DSDrv4_DYNAMIC   EQU 1            ; this equate makes the VXD dynamically
DDK_VERSION      EQU 400h

 .386p
 .xlist
 include vmm.inc
 .list

;============================================================================
;        V I R T U A L   D E V I C E   D E C L A R A T I O N
;============================================================================

DECLARE_VIRTUAL_DEVICE DSDrv4, 1, 0, DSDrv4_Control, Undefined_Device_ID,\
                               UNDEFINED_INIT_ORDER,0,0,0



;===========================================================================
;
;   PROCEDURE: hwioControl
;
;   DESCRIPTION:
;    Device control procedure for the hwio95 VxD
;
;   ENTRY:
;    EAX = Control call ID
;
;   EXIT:
;    If carry clear then
;        Successful
;    else
;        Control call failed
;
;   USES:
;    EAX, EBX, ECX, EDX, ESI, EDI, Flags
;
;============================================================================

Begin_Control_Dispatch DSDrv4
  Control_Dispatch   SYS_DYNAMIC_DEVICE_INIT, DSDrvDynamicInit, sCall
  Control_Dispatch   SYS_DYNAMIC_DEVICE_EXIT, DSDrvDynamicExit, sCall
  Control_Dispatch   INIT_COMPLETE,           DSDrvInitComplete, sCall
  Control_Dispatch   W32_DEVICEIOCONTROL,     DSDrvW32DeviceIOControl, sCall, <ecx, ebx, edx, esi>
End_Control_Dispatch DSDrv4

end