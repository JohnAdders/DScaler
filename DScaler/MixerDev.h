/////////////////////////////////////////////////////////////////////////////
// MixerDev.h
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
//
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           Original Release
//                                     Translated most code from German
//                                     Combined Header files
//                                     Cut out all decoding
//                                     Cut out digital hardware stuff
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __MIXERDEV_H___
#define __MIXERDEV_H___

#define MAXKANAELE 4

struct TMixerAccess
{
    int SoundSystem;
    int Destination;
    int Connection;
    int Control;
};

struct TMixerValues
{
    int Kanal1;
    int Kanal2;
    int Kanal3;
    int Kanal4;
};


struct TMixerLoad
{
	struct TMixerAccess MixerAccess;
	struct TMixerValues MixerValues;
};

struct TMixerControls
{
	int ControlsCount;
	MIXERCONTROL          *MixerControl;
	MIXERCONTROLDETAILS   *MixerDetail;
};

struct TMixerConnections
{
	int ConnectionsCount;
	MIXERLINE *MixerConnections;
	struct TMixerControls *To_Control;
};

struct TMixerLines
{
	int LinesCount;
	MIXERLINE *MixerLine;
	struct TMixerConnections *To_Connection;
};

struct TSoundSystem
{
	int DeviceCount;
	MIXERCAPS *MixerDev;
	struct TMixerLines *To_Lines;
};




MMRESULT Set_Control_Values(MIXERCONTROLDETAILS * Setting, int Device);
MMRESULT Get_Control_Values(MIXERCONTROLDETAILS * Setting, int Device);
BOOL APIENTRY MixerSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY MixerSetupProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
int Get_Access_Slot(int C1, int C2, int C3, int C4);
BOOL Mixer_Open(UINT uMxId);
void Mixer_UnMute(void);
void Mixer_Mute(void);
void Mixer_Set_Defaults(void);
void Mixer_Set_Volume(int Links, int Rechts);
void Mixer_Get_Volume(int *Links, int *Rechts);
void Mixer_Exit(void);
void Get_Volume_Param(void);
void Get_Mixer_SignedValue(MIXERCONTROLDETAILS * Setting, int Device, MIXERCONTROLDETAILS_SIGNED * Set);
void Set_Mixer_SignedValue(MIXERCONTROLDETAILS * Setting, int Device, MIXERCONTROLDETAILS_SIGNED * Set);
void Get_Mixer_UnsignedValue(MIXERCONTROLDETAILS * Setting, int Device, MIXERCONTROLDETAILS_UNSIGNED * Set);
void Set_Mixer_UnsignedValue(MIXERCONTROLDETAILS * Setting, int Device, MIXERCONTROLDETAILS_UNSIGNED * Set);
void Get_Mixer_BoolValue(MIXERCONTROLDETAILS * Setting, int Device, MIXERCONTROLDETAILS_BOOLEAN * Set);
void Set_Mixer_BoolValue(MIXERCONTROLDETAILS * Setting, int Device, MIXERCONTROLDETAILS_BOOLEAN * Set);
MMRESULT Get_Control_Values(MIXERCONTROLDETAILS * Setting, int Device);
MMRESULT Set_Control_Values(MIXERCONTROLDETAILS * Setting, int Device);
void Enumerate_Sound_SubSystem(void);

extern struct TMixerAccess Volume;
extern struct TMixerAccess Mute;
extern struct TMixerLoad MixerLoad[64];
extern BOOL System_In_Mute;
extern BOOL USE_MIXER;

extern int MIXER_LINKER_KANAL;
extern int MIXER_RECHTER_KANAL;
extern int MixerVolumeMax;
extern int MixerVolumeStep;
extern struct TSoundSystem SoundSystem;


#endif
