/////////////////////////////////////////////////////////////////////////////
// mixerdev.c
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

#include "stdafx.h"
#include "MixerDev.h"
#include "DScaler.h"

unsigned int LastOpenMixer = 0xffffffff;
int MIXER_MITTE;
HMIXER hMixer = NULL;
BOOL System_In_Mute = FALSE;
BOOL USE_MIXER = FALSE;

struct TSoundSystem SoundSystem;

struct TMixerAccess Volume = {-1,0,0,0};
struct TMixerAccess Mute = {-1,0,0,0};

struct TMixerLoad MixerLoad[64];

int MIXER_LINKER_KANAL=-1;
int MIXER_RECHTER_KANAL=-1;
int MixerVolumeStep=-1;
int MixerVolumeMax=-1;


void Enumerate_Sound_SubSystem(void)
{
	MMRESULT mmresult;
	int SoundSystemLoop;
	int DestinationLoop;
	int ConnectionLoop;
	int ControlLoop;
	int KanalLoop;
	MIXERLINECONTROLS MixerLineControl;

	SoundSystem.DeviceCount = mixerGetNumDevs();
	if (SoundSystem.DeviceCount == 0)
		return;

	SoundSystem.MixerDev = (MIXERCAPS *) calloc(sizeof(MIXERCAPS), SoundSystem.DeviceCount);
	SoundSystem.To_Lines = (struct TMixerLines *) calloc(sizeof(struct TMixerLines), SoundSystem.DeviceCount);

	for (SoundSystemLoop = 0; SoundSystemLoop < SoundSystem.DeviceCount; SoundSystemLoop++)
	{
		mmresult = mixerGetDevCaps(SoundSystemLoop, &SoundSystem.MixerDev[SoundSystemLoop], sizeof(MIXERCAPS));

		if (Mixer_Open(SoundSystemLoop) == TRUE)
		{
			// Enumerate Destinations
			SoundSystem.To_Lines[SoundSystemLoop].LinesCount = SoundSystem.MixerDev[SoundSystemLoop].cDestinations;
			SoundSystem.To_Lines[SoundSystemLoop].MixerLine = (MIXERLINE *) calloc(sizeof(MIXERLINE), SoundSystem.MixerDev[SoundSystemLoop].cDestinations);
			SoundSystem.To_Lines[SoundSystemLoop].To_Connection = (struct TMixerConnections *) calloc(sizeof(struct TMixerConnections), SoundSystem.MixerDev[SoundSystemLoop].cDestinations);

			for (DestinationLoop = 0; DestinationLoop < SoundSystem.To_Lines[SoundSystemLoop].LinesCount; DestinationLoop++)
			{
				SoundSystem.To_Lines[SoundSystemLoop].MixerLine[DestinationLoop].cbStruct = sizeof(MIXERLINE);
				SoundSystem.To_Lines[SoundSystemLoop].MixerLine[DestinationLoop].dwDestination = DestinationLoop;
				mmresult = mixerGetLineInfo((HMIXEROBJ) hMixer, &SoundSystem.To_Lines[SoundSystemLoop].MixerLine[DestinationLoop], MIXER_GETLINEINFOF_DESTINATION);
				if (mmresult == MMSYSERR_NOERROR)
				{
					// Enumerate Connection
					SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].ConnectionsCount = SoundSystem.To_Lines[SoundSystemLoop].MixerLine[DestinationLoop].cConnections;
					SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].MixerConnections =
						(MIXERLINE *) calloc(sizeof(MIXERLINE), SoundSystem.To_Lines[SoundSystemLoop].MixerLine[DestinationLoop].cConnections);
					SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control =
						(struct TMixerControls *) calloc(sizeof(struct TMixerConnections), SoundSystem.To_Lines[SoundSystemLoop].MixerLine[DestinationLoop].cConnections);

					for (ConnectionLoop = 0; ConnectionLoop < SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].ConnectionsCount; ConnectionLoop++)
					{
						SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].MixerConnections[ConnectionLoop].cbStruct = sizeof(MIXERLINE);
						SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].MixerConnections[ConnectionLoop].dwDestination = DestinationLoop;
						SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].MixerConnections[ConnectionLoop].dwSource = ConnectionLoop;
						mmresult =
							mixerGetLineInfo((HMIXEROBJ) hMixer, &SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].MixerConnections[ConnectionLoop], MIXER_GETLINEINFOF_SOURCE);
						if (mmresult == MMSYSERR_NOERROR)
						{
							// Enumerate Controls

							SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].ControlsCount =
								SoundSystem.To_Lines[SoundSystemLoop].MixerLine[DestinationLoop].cControls;
							SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl =
								(MIXERCONTROL *) calloc(sizeof(MIXERCONTROL), SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].ControlsCount);
							SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerDetail =
								(MIXERCONTROLDETAILS *) calloc(sizeof(MIXERCONTROLDETAILS),
															   SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].ControlsCount);

							MixerLineControl.cbStruct = sizeof(MixerLineControl);
							MixerLineControl.dwLineID = SoundSystem.To_Lines[SoundSystemLoop].MixerLine[DestinationLoop].dwLineID;
							MixerLineControl.cControls = SoundSystem.To_Lines[SoundSystemLoop].MixerLine[DestinationLoop].cControls;
							MixerLineControl.cbmxctrl = sizeof(MIXERCONTROL);
							MixerLineControl.pamxctrl = SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl;
							mmresult = mixerGetLineControls((HMIXEROBJ) hMixer, &MixerLineControl, MIXER_GETLINECONTROLSF_ALL);

							for (ControlLoop = 0; ControlLoop < SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].ControlsCount; ControlLoop++)
							{
								if (mmresult == ERROR_SUCCESS)
								{
									// Detail Controls
									SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerDetail[ControlLoop].cbDetails = 0;
									SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerDetail[ControlLoop].paDetails = NULL;

									switch (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].
											dwControlType & MIXERCONTROL_CT_UNITS_MASK)
									{
									case MIXERCONTROL_CT_UNITS_BOOLEAN:
										SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerDetail[ControlLoop].cbDetails =
											sizeof(MIXERCONTROLDETAILS_BOOLEAN);
										SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerDetail[ControlLoop].paDetails =
											(MIXERCONTROLDETAILS_BOOLEAN *) calloc(sizeof(MIXERCONTROLDETAILS_BOOLEAN), MAXKANAELE);
										break;
									case MIXERCONTROL_CT_UNITS_SIGNED:
										SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerDetail[ControlLoop].cbDetails =
											sizeof(MIXERCONTROLDETAILS_SIGNED);
										SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerDetail[ControlLoop].paDetails =
											(MIXERCONTROLDETAILS_SIGNED *) calloc(sizeof(MIXERCONTROLDETAILS_SIGNED), MAXKANAELE);
										break;
									case MIXERCONTROL_CT_UNITS_UNSIGNED:
										SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerDetail[ControlLoop].cbDetails =
											sizeof(MIXERCONTROLDETAILS_UNSIGNED);
										SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerDetail[ControlLoop].paDetails =
											(MIXERCONTROLDETAILS_UNSIGNED *) calloc(sizeof(MIXERCONTROLDETAILS_UNSIGNED), MAXKANAELE);
										break;
									}

									KanalLoop = MAXKANAELE;
									while (KanalLoop > 0)
									{
										SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerDetail[ControlLoop].cbStruct = sizeof(MIXERCONTROLDETAILS);
										SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerDetail[ControlLoop].dwControlID =
											SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].dwControlID;
										SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerDetail[ControlLoop].cChannels = KanalLoop;
										SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerDetail[ControlLoop].cMultipleItems = 0;

										if (Get_Control_Values
											(&SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerDetail[ControlLoop],
											 SoundSystemLoop) == ERROR_SUCCESS)
										{
											KanalLoop = 0;
											switch (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].dwControlType)
											{
											case MIXERCONTROL_CONTROLTYPE_ONOFF:
												if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0] == 0x00)
												{
													strcpy(&SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0],
														   "ONOFF ID gefunden");
												}
												break;
											case MIXERCONTROL_CONTROLTYPE_MONO:
												if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0] == 0x00)
												{
													strcpy(&SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0],
														   "MONO ID gefunden");
												}
												break;
											case MIXERCONTROL_CONTROLTYPE_LOUDNESS:
												if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0] == 0x00)
												{
													strcpy(&SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0],
														   "LOUDNESS ID gefunden");
												}
												break;

											case MIXERCONTROL_CONTROLTYPE_STEREOENH:
												if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0] == 0x00)
												{
													strcpy(&SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0],
														   "STEREOENH ID gefunden");
												}
												break;

											case MIXERCONTROL_CONTROLTYPE_BUTTON:
												if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0] == 0x00)
												{
													strcpy(&SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0],
														   "BUTTON ID gefunden");
												}
												break;

											case MIXERCONTROL_CONTROLTYPE_DECIBELS:
												if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0] == 0x00)
												{
													strcpy(&SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0],
														   "DECIBELS ID gefunden");
												}
												break;
											case MIXERCONTROL_CONTROLTYPE_SIGNED:
												if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0] == 0x00)
												{
													strcpy(&SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0],
														   "SIGNED ID gefunden");
												}
												break;
											case MIXERCONTROL_CONTROLTYPE_UNSIGNED:
												if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0] == 0x00)
												{
													strcpy(&SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0],
														   "UNSIGNED ID gefunden");
												}
												break;
											case MIXERCONTROL_CONTROLTYPE_PERCENT:
												if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0] == 0x00)
												{
													strcpy(&SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0],
														   "PERCENT ID gefunden");
												}
												break;
											case MIXERCONTROL_CONTROLTYPE_SLIDER:
												if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0] == 0x00)
												{
													strcpy(&SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0],
														   "SLIDER ID gefunden");
												}
												break;
											case MIXERCONTROL_CONTROLTYPE_PAN:
												if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0] == 0x00)
												{
													strcpy(&SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0],
														   "PAN ID gefunden");
												}
												break;
											case MIXERCONTROL_CONTROLTYPE_QSOUNDPAN:
												if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0] == 0x00)
												{
													strcpy(&SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0],
														   "QSOUNDPAN ID gefunden");
												}
												break;

											case MIXERCONTROL_CONTROLTYPE_FADER:
												if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0] == 0x00)
												{
													strcpy(&SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0],
														   "FADER ID gefunden");
												}
												break;

											case MIXERCONTROL_CONTROLTYPE_BASS:
												if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0] == 0x00)
												{
													strcpy(&SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0],
														   "BASS ID gefunden");
												}
												break;

											case MIXERCONTROL_CONTROLTYPE_TREBLE:
												if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0] == 0x00)
												{
													strcpy(&SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0],
														   "TREBLE ID gefunden");
												}
												break;

											case MIXERCONTROL_CONTROLTYPE_EQUALIZER:
												if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0] == 0x00)
												{
													strcpy(&SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0],
														   "EQUALIZER ID gefunden");
												}
												break;

											case MIXERCONTROL_CONTROLTYPE_SINGLESELECT:
												if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0] == 0x00)
												{
													strcpy(&SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0],
														   "SINGLESELECT ID gefunden");
												}
												break;

											case MIXERCONTROL_CONTROLTYPE_MUX:
												if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0] == 0x00)
												{
													strcpy(&SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0],
														   "MUX ID gefunden");
												}
												break;

											case MIXERCONTROL_CONTROLTYPE_MULTIPLESELECT:
												if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0] == 0x00)
												{
													strcpy(&SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0],
														   "MULTIPLESELECT ID gefunden");
												}
												break;

											case MIXERCONTROL_CONTROLTYPE_MIXER:
												if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0] == 0x00)
												{
													strcpy(&SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0],
														   "MIXER ID gefunden");
												}
												break;

											case MIXERCONTROL_CONTROLTYPE_MICROTIME:
												if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0] == 0x00)
												{
													strcpy(&SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0],
														   "MICROTIME ID gefunden");
												}
												break;

											case MIXERCONTROL_CONTROLTYPE_MILLITIME:
												if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0] == 0x00)
												{
													strcpy(&SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0],
														   "MILLITIME ID gefunden");
												}
												break;

											case MIXERCONTROL_CONTROLTYPE_MUTE:
												if (Mute.SoundSystem == -1)
												{
													if (SoundSystem.To_Lines[SoundSystemLoop].MixerLine[DestinationLoop].dwComponentType == MIXERLINE_COMPONENTTYPE_DST_SPEAKERS)
													{
														if (
															(SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].MixerConnections[ConnectionLoop].dwComponentType ==
															 MIXERLINE_COMPONENTTYPE_SRC_LINE)
															|| (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].MixerConnections[ConnectionLoop].dwComponentType ==
																MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY))
														{
															Mute.SoundSystem = SoundSystemLoop;
															Mute.Destination = DestinationLoop;
															Mute.Connection = ConnectionLoop;
															Mute.Control = ControlLoop;
														}
													}
												}

												if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0] == 0x00)
												{
													strcpy(SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName,
														   "MUTE ID gefunden");
												};
												break;

											case MIXERCONTROL_CONTROLTYPE_VOLUME:

												if (Volume.SoundSystem == -1)
												{
													if (SoundSystem.To_Lines[SoundSystemLoop].MixerLine[DestinationLoop].dwComponentType == MIXERLINE_COMPONENTTYPE_DST_SPEAKERS)
													{
														if (
															(SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].MixerConnections[ConnectionLoop].dwComponentType ==
															 MIXERLINE_COMPONENTTYPE_SRC_LINE)
															|| (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].MixerConnections[ConnectionLoop].dwComponentType ==
																MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY))
														{

															Volume.SoundSystem = SoundSystemLoop;
															Volume.Destination = DestinationLoop;
															Volume.Connection = ConnectionLoop;
															Volume.Control = ControlLoop;
														}
													}
												}

												if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0] == 0x00)
												{
													strcpy(&SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName[0],
														   "VOLUME ID gefunden");
												}
												break;

											}
										}
										else
										{
											if (KanalLoop == 0)
												strcpy(SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl[ControlLoop].szName,
													   "Nicht ladbar");
										}
										KanalLoop--;
									}
								}

							}
						}
					}
				}
			}
		}
	}
}

BOOL Mixer_Open(UINT uMxId)
{
	MMRESULT mmresult;

	if (uMxId != LastOpenMixer)
	{
		if (hMixer != NULL)
		{
			mmresult = mixerClose(hMixer);
			hMixer = NULL;
		}

		mmresult = mixerOpen(&hMixer, uMxId, 0, (DWORD) hInst, MIXER_OBJECTF_AUX);
		if (mmresult != MMSYSERR_NOERROR)
		{
			// Ohhps kein auxiliary device
			mmresult = mixerOpen(&hMixer, uMxId, 0, (DWORD) hInst, MIXER_OBJECTF_MIXER);
		}
		if (mmresult == MMSYSERR_NOERROR)
		{
			LastOpenMixer = uMxId;
			return (TRUE);
		}
		return (FALSE);
	}
	return (TRUE);
}

void Mixer_Exit(void)
{
	MMRESULT mmresult;
	int SoundSystemLoop;
	int DestinationLoop;
	int ConnectionLoop;
	int ControlLoop;

	if (hMixer != NULL)
	{
		mmresult = mixerClose(hMixer);
		hMixer = 0;
	}

	for (SoundSystemLoop = 0; SoundSystemLoop < SoundSystem.DeviceCount; SoundSystemLoop++)
	{
		SoundSystem.To_Lines[SoundSystemLoop].MixerLine = (MIXERLINE *) calloc(sizeof(MIXERLINE), SoundSystem.MixerDev[SoundSystemLoop].cDestinations);
		SoundSystem.To_Lines[SoundSystemLoop].To_Connection = (struct TMixerConnections *) calloc(sizeof(struct TMixerConnections), SoundSystem.MixerDev[SoundSystemLoop].cDestinations);

		for (DestinationLoop = 0; DestinationLoop < SoundSystem.To_Lines[SoundSystemLoop].LinesCount; DestinationLoop++)
		{
			for (ConnectionLoop = 0; ConnectionLoop < SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].ConnectionsCount; ConnectionLoop++)
			{
				for (ControlLoop = 0; ControlLoop < SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].ControlsCount; ControlLoop++)
				{
					if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerDetail[ControlLoop].paDetails != NULL)
						free(SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerDetail[ControlLoop].paDetails);
				}
				if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl != NULL)
					free(SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerControl);
				if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerDetail != NULL)
					free(SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control[ConnectionLoop].MixerDetail);
			}

			if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].MixerConnections != NULL)
				free(SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].MixerConnections);
			if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control != NULL)
				free(SoundSystem.To_Lines[SoundSystemLoop].To_Connection[DestinationLoop].To_Control);
		}
		if (SoundSystem.To_Lines[SoundSystemLoop].MixerLine != NULL)
			free(SoundSystem.To_Lines[SoundSystemLoop].MixerLine);
		if (SoundSystem.To_Lines[SoundSystemLoop].To_Connection != NULL)
			free(SoundSystem.To_Lines[SoundSystemLoop].To_Connection);
	}
	if (SoundSystem.MixerDev != NULL)
		free(SoundSystem.MixerDev);
	if (SoundSystem.To_Lines != NULL)
		free(SoundSystem.To_Lines);
}

/******************************/
MMRESULT Set_Control_Values(MIXERCONTROLDETAILS * Setting, int Device)
{
	if (SoundSystem.DeviceCount == 0)
		return (-1);

	Mixer_Open(Device);
	return (mixerSetControlDetails((HMIXEROBJ) hMixer, Setting, MIXER_SETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_HMIXER));
}

MMRESULT Get_Control_Values(MIXERCONTROLDETAILS * Setting, int Device)
{
	if (SoundSystem.DeviceCount == 0)
		return (-1);

	Mixer_Open(Device);
	return (mixerGetControlDetails((HMIXEROBJ) hMixer, Setting, MIXER_GETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_HMIXER));
}

void Set_Mixer_BoolValue(MIXERCONTROLDETAILS * Setting, int Device, MIXERCONTROLDETAILS_BOOLEAN * Set)
{
	MMRESULT mmret;

	memcpy(Setting->paDetails, Set, sizeof(MIXERCONTROLDETAILS_BOOLEAN) * 4);
	mmret = Set_Control_Values(Setting, Device);
}

void Get_Mixer_BoolValue(MIXERCONTROLDETAILS * Setting, int Device, MIXERCONTROLDETAILS_BOOLEAN * Set)
{
	MMRESULT mmret;

	mmret = Get_Control_Values(Setting, Device);
	memcpy(Set, Setting->paDetails, sizeof(MIXERCONTROLDETAILS_BOOLEAN) * 4);
}

void Set_Mixer_UnsignedValue(MIXERCONTROLDETAILS * Setting, int Device, MIXERCONTROLDETAILS_UNSIGNED * Set)
{
	MMRESULT mmret;

	memcpy(Setting->paDetails, Set, sizeof(MIXERCONTROLDETAILS_UNSIGNED) * 4);
	mmret = Set_Control_Values(Setting, Device);
}

void Get_Mixer_UnsignedValue(MIXERCONTROLDETAILS * Setting, int Device, MIXERCONTROLDETAILS_UNSIGNED * Set)
{
	MMRESULT mmret;

	mmret = Get_Control_Values(Setting, Device);
	memcpy(Set, Setting->paDetails, sizeof(MIXERCONTROLDETAILS_UNSIGNED) * 4);
}

void Set_Mixer_SignedValue(MIXERCONTROLDETAILS * Setting, int Device, MIXERCONTROLDETAILS_SIGNED * Set)
{
	MMRESULT mmret;

	memcpy(Setting->paDetails, Set, sizeof(MIXERCONTROLDETAILS_SIGNED) * 4);
	mmret = Set_Control_Values(Setting, Device);
}

void Get_Mixer_SignedValue(MIXERCONTROLDETAILS * Setting, int Device, MIXERCONTROLDETAILS_SIGNED * Set)
{
	MMRESULT mmret;

	mmret = Get_Control_Values(Setting, Device);
	memcpy(Set, Setting->paDetails, sizeof(MIXERCONTROLDETAILS_SIGNED) * 4);
}

void Get_Volume_Param(void)
{
	MixerVolumeMax = -1;
	MixerVolumeStep = -1;
	if (SoundSystem.DeviceCount == 0)
		return;
	if (Volume.SoundSystem < 0)
		return;
	MixerVolumeMax = SoundSystem.To_Lines[Volume.SoundSystem].To_Connection[Volume.Destination].To_Control[Volume.Connection].MixerControl[Volume.Control].Bounds.dwMaximum;
	MixerVolumeStep = SoundSystem.To_Lines[Volume.SoundSystem].To_Connection[Volume.Destination].To_Control[Volume.Connection].MixerControl[Volume.Control].Metrics.cSteps;
}

void Mixer_Get_Volume(int *Links, int *Rechts)
{
	MIXERCONTROLDETAILS_UNSIGNED UnsignedWert[4];

	if (SoundSystem.DeviceCount == 0)
		return;
	if (Volume.SoundSystem < 0)
		return;
	Get_Mixer_UnsignedValue(&SoundSystem.To_Lines[Volume.SoundSystem].To_Connection[Volume.Destination].To_Control[Volume.Connection].MixerDetail[Volume.Control], Volume.SoundSystem, UnsignedWert);
	*Links = UnsignedWert[0].dwValue;
	*Rechts = UnsignedWert[1].dwValue;
}

void Mixer_Set_Volume(int Links, int Rechts)
{
	MIXERCONTROLDETAILS_UNSIGNED UnsignedWert[4];

	if (SoundSystem.DeviceCount == 0)
		return;
	if (Volume.SoundSystem < 0)
		return;
	UnsignedWert[0].dwValue = Links;
	UnsignedWert[1].dwValue = Rechts;
	Set_Mixer_UnsignedValue(&SoundSystem.To_Lines[Volume.SoundSystem].To_Connection[Volume.Destination].To_Control[Volume.Connection].MixerDetail[Volume.Control], Volume.SoundSystem, UnsignedWert);
}

void Mixer_Set_Defaults(void)
{
	int i;

	MIXERCONTROLDETAILS_UNSIGNED UnsignedWert[4];

	if (SoundSystem.DeviceCount == 0)
		return;
	for (i = 0; i < 64; i++)
	{
		if (MixerLoad[i].MixerAccess.SoundSystem >= 0)
		{
			UnsignedWert[0].dwValue = MixerLoad[i].MixerValues.Kanal1;
			UnsignedWert[1].dwValue = MixerLoad[i].MixerValues.Kanal2;
			UnsignedWert[2].dwValue = MixerLoad[i].MixerValues.Kanal3;
			UnsignedWert[3].dwValue = MixerLoad[i].MixerValues.Kanal4;
			Set_Mixer_UnsignedValue(&SoundSystem.To_Lines[MixerLoad[i].MixerAccess.SoundSystem].To_Connection[MixerLoad[i].MixerAccess.Destination].To_Control[MixerLoad[i].MixerAccess.Connection].
									MixerDetail[MixerLoad[i].MixerAccess.Control], MixerLoad[i].MixerAccess.SoundSystem, UnsignedWert);
		}
	}
}

void Mixer_Mute(void)
{
	MIXERCONTROLDETAILS_BOOLEAN BoolWert[4];

	if (Mute.SoundSystem < 0)
		return;
	if (SoundSystem.DeviceCount == 0)
		return;
	BoolWert[0].fValue = 1;
	BoolWert[1].fValue = 1;
	BoolWert[2].fValue = 1;
	BoolWert[3].fValue = 1;
	Set_Mixer_BoolValue(&SoundSystem.To_Lines[Mute.SoundSystem].To_Connection[Mute.Destination].To_Control[Mute.Connection].MixerDetail[Mute.Control], Mute.SoundSystem, BoolWert);
}

void Mixer_UnMute(void)
{
	MIXERCONTROLDETAILS_BOOLEAN BoolWert[4];

	if (Mute.SoundSystem < 0)
		return;
	if (SoundSystem.DeviceCount == 0)
		return;
	BoolWert[0].fValue = 0;
	BoolWert[1].fValue = 0;
	BoolWert[2].fValue = 0;
	BoolWert[3].fValue = 0;
	Set_Mixer_BoolValue(&SoundSystem.To_Lines[Mute.SoundSystem].To_Connection[Mute.Destination].To_Control[Mute.Connection].MixerDetail[Mute.Control], Mute.SoundSystem, BoolWert);
}

int Get_Access_Slot(int C1, int C2, int C3, int C4)
{
	int i;

	i = 0;
	while (i < 64)
	{
		if ((MixerLoad[i].MixerAccess.SoundSystem == C1) && (MixerLoad[i].MixerAccess.Destination == C2) && (MixerLoad[i].MixerAccess.Connection == C3) && (MixerLoad[i].MixerAccess.Control == C4))
			return (i);
		i++;
	}
	return (-1);
}

BOOL APIENTRY MixerSetupProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	char Text[255];
	int i, j, x, y;
	double d;

	switch (message)
	{
	case WM_INITDIALOG:
	case WM_USER:
		if ((SoundSystem.DeviceCount == 0) || (USE_MIXER == FALSE))
		{

			SetDlgItemText(hDlg, TEXT1, "Kein Soundsystem gefunden oder nicht aktiviert");
			SetDlgItemText(hDlg, TEXT2, "Volume nicht zugeordnet");
			SetDlgItemText(hDlg, TEXT3, "Mute nicht zugeordnet");
			ShowWindow(GetDlgItem(hDlg, TEXT4), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, TEXT5), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, TEXT6), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, TEXT7), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, TEXT8), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, TEXT9), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, TEXT10), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, TEXT11), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, TEXT12), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, TEXT13), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, TEXT14), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, TEXT15), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_BUTTON1), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_CHECK1), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_D1), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_D2), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_D3), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_S1), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_S2), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_S3), SW_HIDE);

			break;
		}

		ShowWindow(GetDlgItem(hDlg, TEXT4), SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, TEXT5), SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, TEXT6), SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, TEXT7), SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, TEXT8), SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, TEXT9), SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, TEXT10), SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, TEXT11), SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, TEXT12), SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, TEXT13), SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, TEXT14), SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, TEXT15), SW_SHOW);

		ShowWindow(GetDlgItem(hDlg, IDC_BUTTON1), SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_CHECK1), SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_D1), SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_D2), SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_D3), SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_S1), SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_S2), SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_S3), SW_SHOW);

		CheckDlgButton(hDlg, IDC_CHECK2, USE_MIXER);

		if (Volume.SoundSystem >= 0)
		{
			strncpy(Text, SoundSystem.MixerDev[Volume.SoundSystem].szPname, sizeof(Text));
		}
		else if (Mute.SoundSystem >= 0)
		{
			strncpy(Text, SoundSystem.MixerDev[Mute.SoundSystem].szPname, sizeof(Text));
		}
		else
		{
			strncpy(Text, SoundSystem.MixerDev[0].szPname, sizeof(Text));
		}
		SetDlgItemText(hDlg, TEXT1, Text);
		if (Volume.SoundSystem >= 0)
		{
			sprintf(Text, "%s -> %s  %s", SoundSystem.To_Lines[Volume.SoundSystem].To_Connection[Volume.Destination].MixerConnections[Volume.Connection].szName,
					SoundSystem.To_Lines[Volume.SoundSystem].MixerLine[Volume.Destination].szName,
					SoundSystem.To_Lines[Volume.SoundSystem].To_Connection[Volume.Destination].To_Control[Volume.Connection].MixerControl[Volume.Control].szName);
			SetDlgItemText(hDlg, TEXT2, Text);
		}
		else
			SetDlgItemText(hDlg, TEXT2, "Volume nicht zugeordnet");

		if (Mute.SoundSystem >= 0)
		{
			sprintf(Text, "%s -> %s  %s", SoundSystem.To_Lines[Mute.SoundSystem].To_Connection[Mute.Destination].MixerConnections[Mute.Connection].szName,
					SoundSystem.To_Lines[Mute.SoundSystem].MixerLine[Mute.Destination].szName,
					SoundSystem.To_Lines[Mute.SoundSystem].To_Connection[Mute.Destination].To_Control[Mute.Connection].MixerControl[Mute.Control].szName);
			SetDlgItemText(hDlg, TEXT3, Text);
		}
		else
			SetDlgItemText(hDlg, TEXT2, "Mute nicht zugeordnet");

		SetDlgItemInt(hDlg, IDC_D1, MIXER_LINKER_KANAL, FALSE);
		SetDlgItemInt(hDlg, IDC_D2, MIXER_RECHTER_KANAL, FALSE);
		MIXER_MITTE = (MIXER_LINKER_KANAL + MIXER_RECHTER_KANAL) / 2;
		SetDlgItemInt(hDlg, IDC_D3, MIXER_MITTE, FALSE);
		i = MIXER_LINKER_KANAL / MixerVolumeStep;
		j = MixerVolumeMax / MixerVolumeStep;
		d = (double) i / (double) j;
		y = (int) (240 - ((double) d * 160.0));
		MoveWindow(GetDlgItem(hDlg, IDC_S1), 258, y - 4, 22, 8, TRUE);

		i = MIXER_RECHTER_KANAL / MixerVolumeStep;
		d = (double) i / (double) j;
		y = (int) (240 - ((double) d * 160.0));
		MoveWindow(GetDlgItem(hDlg, IDC_S2), 408, y - 4, 22, 8, TRUE);

		i = MIXER_MITTE / MixerVolumeStep;
		d = (double) i / (double) j;
		y = (int) (240 - ((double) d * 160.0));
		MoveWindow(GetDlgItem(hDlg, IDC_S3), 333, y - 4, 22, 8, TRUE);

		break;

	case WM_MOUSEMOVE:
		if (Volume.SoundSystem < 0)
			break;
		if (SoundSystem.DeviceCount == 0)
			break;
		y = HIWORD(lParam);
		x = LOWORD(lParam);
		if (wParam == MK_LBUTTON)
		{
			if ((y >= 75) && (y <= 235))
			{
				if ((x >= 255) && (x <= 280))
				{
					d = ((double) (235 - y) / 160.0);
					d = d * MixerVolumeMax;
					i = (int) d;
					MoveWindow(GetDlgItem(hDlg, IDC_S1), 258, y - 2, 22, 8, TRUE);
					MIXER_LINKER_KANAL = i;
					SetDlgItemInt(hDlg, IDC_D1, MIXER_LINKER_KANAL, FALSE);
					MIXER_MITTE = (MIXER_LINKER_KANAL + MIXER_RECHTER_KANAL) / 2;
					SetDlgItemInt(hDlg, IDC_D3, MIXER_MITTE, FALSE);
					j = MixerVolumeMax / MixerVolumeStep;
					i = MIXER_MITTE / MixerVolumeStep;
					d = (double) i / (double) j;
					y = (int) (240 - ((double) d * 160.0));
					MoveWindow(GetDlgItem(hDlg, IDC_S3), 333, y - 2, 22, 8, TRUE);
					Mixer_Set_Volume(MIXER_LINKER_KANAL, MIXER_RECHTER_KANAL);
				}

				if ((x >= 405) && (x <= 430))
				{
					d = ((double) (235 - y) / 160.0);
					d = d * MixerVolumeMax;
					i = (int) d;
					MoveWindow(GetDlgItem(hDlg, IDC_S2), 408, y - 2, 22, 8, TRUE);
					MIXER_RECHTER_KANAL = i;
					SetDlgItemInt(hDlg, IDC_D2, MIXER_RECHTER_KANAL, FALSE);
					MIXER_MITTE = (MIXER_LINKER_KANAL + MIXER_RECHTER_KANAL) / 2;
					SetDlgItemInt(hDlg, IDC_D3, MIXER_MITTE, FALSE);
					j = MixerVolumeMax / MixerVolumeStep;
					i = MIXER_MITTE / MixerVolumeStep;
					d = (double) i / (double) j;
					y = (int) (240 - ((double) d * 160.0));
					MoveWindow(GetDlgItem(hDlg, IDC_S3), 333, y - 2, 22, 8, TRUE);
					Mixer_Set_Volume(MIXER_LINKER_KANAL, MIXER_RECHTER_KANAL);

				}

				if ((x >= 331) && (x <= 356))
				{
					d = ((double) (235 - y) / 160.0);
					d = d * MixerVolumeMax;
					i = (int) d;
					j = MIXER_MITTE - i;
					j = -j;
					if ((j + MIXER_LINKER_KANAL <= MixerVolumeMax) && (j + MIXER_RECHTER_KANAL <= MixerVolumeMax) && (j + MIXER_LINKER_KANAL >= 0) && (j + MIXER_RECHTER_KANAL >= 0))
					{
						MoveWindow(GetDlgItem(hDlg, IDC_S3), 333, y - 2, 22, 8, TRUE);
						MIXER_RECHTER_KANAL += j;
						MIXER_LINKER_KANAL += j;
						MIXER_MITTE = i;
						SetDlgItemInt(hDlg, IDC_D3, MIXER_MITTE, FALSE);
						SetDlgItemInt(hDlg, IDC_D2, MIXER_RECHTER_KANAL, FALSE);
						SetDlgItemInt(hDlg, IDC_D1, MIXER_LINKER_KANAL, FALSE);
						j = MixerVolumeMax / MixerVolumeStep;
						i = MIXER_RECHTER_KANAL / MixerVolumeStep;
						d = (double) i / (double) j;
						y = (int) (240 - ((double) d * 160.0));
						MoveWindow(GetDlgItem(hDlg, IDC_S2), 408, y - 2, 22, 8, TRUE);
						i = MIXER_LINKER_KANAL / MixerVolumeStep;
						d = (double) i / (double) j;
						y = (int) (240 - ((double) d * 160.0));
						MoveWindow(GetDlgItem(hDlg, IDC_S1), 258, y - 2, 22, 8, TRUE);
						Mixer_Set_Volume(MIXER_LINKER_KANAL, MIXER_RECHTER_KANAL);

					}
				}

			}
		}
		break;

	case WM_COMMAND:

		switch (LOWORD(wParam))
		{

		case IDC_CHECK2:
			if (IsDlgButtonChecked(hDlg, IDC_CHECK2))
			{
				if (SoundSystem.DeviceCount == 0)
					Enumerate_Sound_SubSystem();
				if (SoundSystem.DeviceCount > 0)
				{
					if (MIXER_LINKER_KANAL == -1)
						Mixer_Get_Volume(&MIXER_LINKER_KANAL, &MIXER_RECHTER_KANAL);
					Mixer_Set_Defaults();
					Get_Volume_Param();
					Mixer_Set_Volume(MIXER_LINKER_KANAL, MIXER_RECHTER_KANAL);
					USE_MIXER = TRUE;
				}
			}
			else
			{
				USE_MIXER = FALSE;
			}
			SendMessage(hDlg, WM_USER, 0, 0);

			break;

		case IDC_CHECK1:
			if (IsDlgButtonChecked(hDlg, IDC_CHECK1))
			{
				System_In_Mute = TRUE;
				Mixer_Mute();
			}
			else
			{
				Mixer_UnMute();
				System_In_Mute = FALSE;
			}
			break;

		case IDC_BUTTON1:
			DialogBox(hInst, "MIXERSETTINGS", hDlg, MixerSettingProc);
			break;

		case IDOK:
		case IDCANCEL:
			EndDialog(hDlg, 0);
			break;
		}
		break;
	}
	return (FALSE);
}

BOOL APIENTRY MixerSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	int i, C1, C2, C3, C4;
	int ret;

	char Text[128];
	char Text1[128];
	BOOL Has_Changed;

	MIXERCONTROLDETAILS_BOOLEAN BoolWert[4];
	MIXERCONTROLDETAILS_SIGNED SignedWert[4];
	MIXERCONTROLDETAILS_UNSIGNED UnsignedWert[4];

	switch (message)
	{
	case WM_INITDIALOG:
		SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_RESETCONTENT, 0, 0);
		if (Volume.SoundSystem == -1)
		{
			SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_ADDSTRING, 0, (LONG) (LPSTR) "Volume nicht zugeordnet");
		}
		else
		{
			sprintf(Text, "Volume-Control");
			SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_ADDSTRING, 0, (LONG) (LPSTR) Text);
		}

		if (Mute.SoundSystem == -1)
		{
			SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_ADDSTRING, 0, (LONG) (LPSTR) "Mute nicht zugeordnet");
		}
		else
		{
			sprintf(Text, "Mute-Control");
			SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_ADDSTRING, 0, (LONG) (LPSTR) Text);
		}

		for (i = 0; i < 64; i++)
		{
			if (MixerLoad[i].MixerAccess.SoundSystem >= 0)
			{
				sprintf(Text, "Startup-Sequenz %d", i);
				SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_ADDSTRING, 0, (LONG) (LPSTR) Text);
			}
		}
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_RESETCONTENT, 0, 0);
		SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_RESETCONTENT, 0, 0);
		SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_RESETCONTENT, 0, 0);
		SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_RESETCONTENT, 0, 0);

		for (i = 0; i < SoundSystem.DeviceCount; i++)
		{
			SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_ADDSTRING, 0, (LONG) (LPSTR) SoundSystem.MixerDev[i].szPname);
		}
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_SETCURSEL, 0, 0);

		for (i = 0; i < SoundSystem.To_Lines[0].LinesCount; i++)
		{
			SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_ADDSTRING, 0, (LONG) (LPSTR) SoundSystem.To_Lines[0].MixerLine[i].szName);
		}
		SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_SETCURSEL, 0, 0);

		for (i = 0; i < SoundSystem.To_Lines[0].To_Connection[0].ConnectionsCount; i++)
		{
			SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_ADDSTRING, 0, (LONG) (LPSTR) SoundSystem.To_Lines[0].To_Connection[0].MixerConnections[i].szName);
		}
		SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_SETCURSEL, 0, 0);

		for (i = 0; i < SoundSystem.To_Lines[0].To_Connection[0].To_Control[0].ControlsCount; i++)
		{
			SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_ADDSTRING, 0, (LONG) (LPSTR) SoundSystem.To_Lines[0].To_Connection[0].To_Control[0].MixerControl[i].szName);
		}
		SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_SETCURSEL, 0, 0);

		SetTimer(hDlg, 0, 250, NULL);
		break;

	case WM_TIMER:
		SendMessage(hDlg, WM_USER, 4, 0);
		break;

	case WM_USER:

		if (wParam == 1)
		{
			C1 = SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0);
			if ((C1 >= 0) && (C1 < SoundSystem.DeviceCount))
			{
				SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_RESETCONTENT, 0, 0);
				SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_RESETCONTENT, 0, 0);
				SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_RESETCONTENT, 0, 0);

				for (i = 0; i < SoundSystem.To_Lines[C1].LinesCount; i++)
				{
					SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_ADDSTRING, 0, (LONG) (LPSTR) SoundSystem.To_Lines[C1].MixerLine[i].szName);
				}
				SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_SETCURSEL, 0, 0);

				for (i = 0; i < SoundSystem.To_Lines[C1].To_Connection[0].ConnectionsCount; i++)
				{
					SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_ADDSTRING, 0, (LONG) (LPSTR) SoundSystem.To_Lines[C1].To_Connection[0].MixerConnections[i].szName);
				}
				SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_SETCURSEL, 0, 0);

				for (i = 0; i < SoundSystem.To_Lines[C1].To_Connection[0].To_Control[0].ControlsCount; i++)
				{
					SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_ADDSTRING, 0, (LONG) (LPSTR) SoundSystem.To_Lines[C1].To_Connection[0].To_Control[0].MixerControl[i].szName);
				}
				SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_SETCURSEL, 0, 0);
			}
		}

		if (wParam == 2)
		{
			C1 = SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0);
			C2 = SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_GETCURSEL, 0, 0);
			if ((C2 >= 0) && (C2 < SoundSystem.To_Lines[C1].LinesCount))
			{
				SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_RESETCONTENT, 0, 0);
				SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_RESETCONTENT, 0, 0);

				for (i = 0; i < SoundSystem.To_Lines[C1].To_Connection[C2].ConnectionsCount; i++)
				{
					SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_ADDSTRING, 0, (LONG) (LPSTR) SoundSystem.To_Lines[C1].To_Connection[C2].MixerConnections[i].szName);
				}
				SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_SETCURSEL, 0, 0);

				for (i = 0; i < SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[0].ControlsCount; i++)
				{
					SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_ADDSTRING, 0, (LONG) (LPSTR) SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[0].MixerControl[i].szName);
				}
				SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_SETCURSEL, 0, 0);
			}
		}

		if (wParam == 3)
		{
			C1 = SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0);
			C2 = SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_GETCURSEL, 0, 0);
			C3 = SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_GETCURSEL, 0, 0);
			if ((C3 >= 0) && (C3 < SoundSystem.To_Lines[C1].To_Connection[C2].ConnectionsCount))
			{
				SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_RESETCONTENT, 0, 0);

				for (i = 0; i < SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].ControlsCount; i++)
				{
					SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_ADDSTRING, 0, (LONG) (LPSTR) SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[i].szName);
				}
				SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_SETCURSEL, 0, 0);
			}
		}

		if (wParam == 4)
		{
			C1 = SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0);
			C2 = SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_GETCURSEL, 0, 0);
			C3 = SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_GETCURSEL, 0, 0);
			C4 = SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_GETCURSEL, 0, 0);
			if ((C4 >= 0) && (C4 < SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].ControlsCount))
			{
				sprintf(Text, "%03d %03d %03d %03d", C1, C2, C3, C4);
				GetDlgItemText(hDlg, TEXT20, (LPSTR) Text1, 128);
				Has_Changed = TRUE;
				if (strcmp(Text, Text1) == 0)
					Has_Changed = FALSE;

				SetDlgItemText(hDlg, TEXT20, Text);
				sprintf(Text, "%d", SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].dwControlID);
				SetDlgItemText(hDlg, TEXT1, Text);
				sprintf(Text, "0x%08x", SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].dwControlType);
				SetDlgItemText(hDlg, TEXT2, Text);
				sprintf(Text, "%d", SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].fdwControl);
				SetDlgItemText(hDlg, TEXT3, Text);
				sprintf(Text, "%d", SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].cMultipleItems);
				SetDlgItemText(hDlg, TEXT4, Text);
				sprintf(Text, "%s", SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].szShortName);
				SetDlgItemText(hDlg, TEXT5, Text);
				sprintf(Text, "%s", SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].szName);
				SetDlgItemText(hDlg, TEXT6, Text);
				sprintf(Text, "%d", SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Bounds.dwMinimum);
				SetDlgItemText(hDlg, TEXT7, Text);
				sprintf(Text, "%d", SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Bounds.dwMaximum);
				SetDlgItemText(hDlg, TEXT8, Text);
				sprintf(Text, "%d", SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Metrics.cSteps);
				SetDlgItemText(hDlg, TEXT9, Text);

				sprintf(Text, "%d", SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4].cChannels);
				SetDlgItemText(hDlg, TEXT10, Text);
				if (Has_Changed == TRUE)
				{
					ShowWindow(GetDlgItem(hDlg, IDC_TEXT12), SW_HIDE);
					ShowWindow(GetDlgItem(hDlg, IDC_TEXT13), SW_HIDE);
					ShowWindow(GetDlgItem(hDlg, IDC_TEXT14), SW_HIDE);
					ShowWindow(GetDlgItem(hDlg, IDC_TEXT15), SW_HIDE);
					ShowWindow(GetDlgItem(hDlg, TEXT12), SW_HIDE);
					ShowWindow(GetDlgItem(hDlg, TEXT13), SW_HIDE);
					ShowWindow(GetDlgItem(hDlg, TEXT14), SW_HIDE);
					ShowWindow(GetDlgItem(hDlg, TEXT15), SW_HIDE);

					ShowWindow(GetDlgItem(hDlg, IDC_BUTTON1), SW_HIDE);
					ShowWindow(GetDlgItem(hDlg, IDC_BUTTON2), SW_HIDE);
					ShowWindow(GetDlgItem(hDlg, IDC_BUTTON3), SW_HIDE);
					ShowWindow(GetDlgItem(hDlg, IDC_BUTTON4), SW_HIDE);
					ShowWindow(GetDlgItem(hDlg, IDC_BUTTON5), SW_HIDE);
					ShowWindow(GetDlgItem(hDlg, IDC_BUTTON6), SW_HIDE);
					ShowWindow(GetDlgItem(hDlg, IDC_BUTTON7), SW_HIDE);
					ShowWindow(GetDlgItem(hDlg, IDC_BUTTON8), SW_HIDE);
					ShowWindow(GetDlgItem(hDlg, IDC_BUTTON9), SW_HIDE);
					ShowWindow(GetDlgItem(hDlg, IDC_BUTTON10), SW_HIDE);
					ShowWindow(GetDlgItem(hDlg, IDC_BUTTON11), SW_HIDE);
					ShowWindow(GetDlgItem(hDlg, IDC_BUTTON12), SW_HIDE);
					ShowWindow(GetDlgItem(hDlg, IDC_CHECK1), SW_SHOW);

					CheckDlgButton(hDlg, IDC_CHECK1, FALSE);

					if (Get_Access_Slot(C1, C2, C3, C4) >= 0)
						CheckDlgButton(hDlg, IDC_CHECK1, TRUE);
				}

				switch (SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].dwControlType & MIXERCONTROL_CT_UNITS_MASK)
				{
				case MIXERCONTROL_CT_UNITS_BOOLEAN:
					strcpy(Text, "Boolean");
					Get_Mixer_BoolValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, BoolWert);
					if (Has_Changed == TRUE)
					{
						ShowWindow(GetDlgItem(hDlg, IDC_CHECK3), SW_HIDE);
						ShowWindow(GetDlgItem(hDlg, IDC_CHECK2), SW_SHOW);
						CheckDlgButton(hDlg, IDC_CHECK2, FALSE);
						if ((Mute.SoundSystem == C1) && (Mute.Destination == C2) && (Mute.Connection == C3) && (Mute.Control == C4))
						{
							CheckDlgButton(hDlg, IDC_CHECK2, TRUE);
						}
						SetDlgItemText(hDlg, IDC_TEXT12, (LPSTR) "Schalter");
						ShowWindow(GetDlgItem(hDlg, IDC_TEXT12), SW_SHOW);
						ShowWindow(GetDlgItem(hDlg, TEXT12), SW_SHOW);
						ShowWindow(GetDlgItem(hDlg, IDC_BUTTON1), SW_SHOW);
					}
					if (BoolWert[0].fValue == 1)
						SetDlgItemText(hDlg, TEXT12, (LPSTR) "TRUE");
					else
						SetDlgItemText(hDlg, TEXT12, (LPSTR) "FALSE");

					if (SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4].cChannels > 1)
					{
						if (Has_Changed == TRUE)
						{
							SetDlgItemText(hDlg, IDC_TEXT13, (LPSTR) "Schalter");
							ShowWindow(GetDlgItem(hDlg, IDC_TEXT13), SW_SHOW);
							ShowWindow(GetDlgItem(hDlg, TEXT13), SW_SHOW);
							ShowWindow(GetDlgItem(hDlg, IDC_BUTTON4), SW_SHOW);
						}
						if (BoolWert[1].fValue == 1)
							SetDlgItemText(hDlg, TEXT13, (LPSTR) "TRUE");
						else
							SetDlgItemText(hDlg, TEXT13, (LPSTR) "FALSE");
					}
					if (SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4].cChannels > 2)
					{
						if (Has_Changed == TRUE)
						{
							SetDlgItemText(hDlg, IDC_TEXT14, (LPSTR) "Schalter");
							ShowWindow(GetDlgItem(hDlg, IDC_TEXT14), SW_SHOW);
							ShowWindow(GetDlgItem(hDlg, TEXT14), SW_SHOW);
							ShowWindow(GetDlgItem(hDlg, IDC_BUTTON7), SW_SHOW);
						}
						if (BoolWert[2].fValue == 1)
							SetDlgItemText(hDlg, TEXT14, (LPSTR) "TRUE");
						else
							SetDlgItemText(hDlg, TEXT14, (LPSTR) "FALSE");
					}

					if (SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4].cChannels > 3)
					{
						if (Has_Changed == TRUE)
						{
							SetDlgItemText(hDlg, IDC_TEXT15, (LPSTR) "Schalter");
							ShowWindow(GetDlgItem(hDlg, IDC_TEXT15), SW_SHOW);
							ShowWindow(GetDlgItem(hDlg, TEXT15), SW_SHOW);
							ShowWindow(GetDlgItem(hDlg, IDC_BUTTON10), SW_SHOW);
						}
						if (BoolWert[3].fValue == 1)
							SetDlgItemText(hDlg, TEXT15, (LPSTR) "TRUE");
						else
							SetDlgItemText(hDlg, TEXT15, (LPSTR) "FALSE");
					}

					break;
				case MIXERCONTROL_CT_UNITS_SIGNED:
					Get_Mixer_SignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, SignedWert);
					if (Has_Changed == TRUE)
					{
						CheckDlgButton(hDlg, IDC_CHECK3, FALSE);
						if ((Volume.SoundSystem == C1) && (Volume.Destination == C2) && (Volume.Connection == C3) && (Volume.Control == C4))
						{
							CheckDlgButton(hDlg, IDC_CHECK3, TRUE);
						}

						ShowWindow(GetDlgItem(hDlg, IDC_CHECK3), SW_SHOW);
						ShowWindow(GetDlgItem(hDlg, IDC_CHECK2), SW_HIDE);
						SetDlgItemText(hDlg, IDC_TEXT12, (LPSTR) "Wert");
						ShowWindow(GetDlgItem(hDlg, IDC_TEXT12), SW_SHOW);
						ShowWindow(GetDlgItem(hDlg, TEXT12), SW_SHOW);
						ShowWindow(GetDlgItem(hDlg, IDC_BUTTON2), SW_SHOW);
						ShowWindow(GetDlgItem(hDlg, IDC_BUTTON3), SW_SHOW);
					}
					sprintf(Text, "%d", SignedWert[0].lValue);
					SetDlgItemText(hDlg, TEXT12, (LPSTR) Text);
					if (SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4].cChannels > 1)
					{
						if (Has_Changed == TRUE)
						{
							SetDlgItemText(hDlg, IDC_TEXT13, (LPSTR) "Wert");
							ShowWindow(GetDlgItem(hDlg, IDC_TEXT13), SW_SHOW);
							ShowWindow(GetDlgItem(hDlg, TEXT13), SW_SHOW);
							ShowWindow(GetDlgItem(hDlg, IDC_BUTTON5), SW_SHOW);
							ShowWindow(GetDlgItem(hDlg, IDC_BUTTON6), SW_SHOW);
						}
						sprintf(Text, "%d", SignedWert[1].lValue);
						SetDlgItemText(hDlg, TEXT13, (LPSTR) Text);
					}
					if (SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4].cChannels > 2)
					{
						if (Has_Changed == TRUE)
						{
							SetDlgItemText(hDlg, IDC_TEXT14, (LPSTR) "Wert");
							ShowWindow(GetDlgItem(hDlg, IDC_TEXT14), SW_SHOW);
							ShowWindow(GetDlgItem(hDlg, TEXT14), SW_SHOW);
							ShowWindow(GetDlgItem(hDlg, IDC_BUTTON8), SW_SHOW);
							ShowWindow(GetDlgItem(hDlg, IDC_BUTTON9), SW_SHOW);
						}
						sprintf(Text, "%d", SignedWert[2].lValue);
						SetDlgItemText(hDlg, TEXT14, (LPSTR) Text);
					}
					if (SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4].cChannels > 3)
					{
						if (Has_Changed == TRUE)
						{
							SetDlgItemText(hDlg, IDC_TEXT15, (LPSTR) "Wert");
							ShowWindow(GetDlgItem(hDlg, IDC_TEXT15), SW_SHOW);
							ShowWindow(GetDlgItem(hDlg, TEXT15), SW_SHOW);
							ShowWindow(GetDlgItem(hDlg, IDC_BUTTON11), SW_SHOW);
							ShowWindow(GetDlgItem(hDlg, IDC_BUTTON12), SW_SHOW);
						}
						sprintf(Text, "%d", SignedWert[3].lValue);
						SetDlgItemText(hDlg, TEXT15, (LPSTR) Text);
					}
					sprintf(Text, "%s", "Signed");

					break;
				case MIXERCONTROL_CT_UNITS_UNSIGNED:
					Get_Mixer_UnsignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, UnsignedWert);
					if (Has_Changed == TRUE)
					{
						CheckDlgButton(hDlg, IDC_CHECK3, FALSE);
						if ((Volume.SoundSystem == C1) && (Volume.Destination == C2) && (Volume.Connection == C3) && (Volume.Control == C4))
						{
							CheckDlgButton(hDlg, IDC_CHECK3, TRUE);
						}

						ShowWindow(GetDlgItem(hDlg, IDC_CHECK3), SW_SHOW);
						ShowWindow(GetDlgItem(hDlg, IDC_CHECK2), SW_HIDE);
						SetDlgItemText(hDlg, IDC_TEXT12, (LPSTR) "Wert");
						ShowWindow(GetDlgItem(hDlg, IDC_TEXT12), SW_SHOW);
						ShowWindow(GetDlgItem(hDlg, TEXT12), SW_SHOW);
						ShowWindow(GetDlgItem(hDlg, IDC_BUTTON2), SW_SHOW);
						ShowWindow(GetDlgItem(hDlg, IDC_BUTTON3), SW_SHOW);

					}
					sprintf(Text, "%d", UnsignedWert[0].dwValue);
					SetDlgItemText(hDlg, TEXT12, (LPSTR) Text);
					if (SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4].cChannels > 1)
					{
						if (Has_Changed == TRUE)
						{
							SetDlgItemText(hDlg, IDC_TEXT13, (LPSTR) "Wert");
							ShowWindow(GetDlgItem(hDlg, IDC_TEXT13), SW_SHOW);
							ShowWindow(GetDlgItem(hDlg, TEXT13), SW_SHOW);
							ShowWindow(GetDlgItem(hDlg, IDC_BUTTON5), SW_SHOW);
							ShowWindow(GetDlgItem(hDlg, IDC_BUTTON6), SW_SHOW);
						}
						sprintf(Text, "%d", UnsignedWert[1].dwValue);
						SetDlgItemText(hDlg, TEXT13, (LPSTR) Text);
					}
					if (SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4].cChannels > 2)
					{
						if (Has_Changed == TRUE)
						{
							SetDlgItemText(hDlg, IDC_TEXT14, (LPSTR) "Wert");
							ShowWindow(GetDlgItem(hDlg, IDC_TEXT14), SW_SHOW);
							ShowWindow(GetDlgItem(hDlg, TEXT14), SW_SHOW);
							ShowWindow(GetDlgItem(hDlg, IDC_BUTTON8), SW_SHOW);
							ShowWindow(GetDlgItem(hDlg, IDC_BUTTON9), SW_SHOW);
						}
						sprintf(Text, "%d", UnsignedWert[2].dwValue);
						SetDlgItemText(hDlg, TEXT14, (LPSTR) Text);
					}
					if (SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4].cChannels > 3)
					{
						if (Has_Changed == TRUE)
						{
							SetDlgItemText(hDlg, IDC_TEXT15, (LPSTR) "Wert");
							ShowWindow(GetDlgItem(hDlg, IDC_TEXT15), SW_SHOW);
							ShowWindow(GetDlgItem(hDlg, TEXT15), SW_SHOW);
							ShowWindow(GetDlgItem(hDlg, IDC_BUTTON11), SW_SHOW);
							ShowWindow(GetDlgItem(hDlg, IDC_BUTTON12), SW_SHOW);
						}
						sprintf(Text, "%d", UnsignedWert[3].dwValue);
						SetDlgItemText(hDlg, TEXT15, (LPSTR) Text);
					}
					sprintf(Text, "%s", "Unsigned");
					break;

				default:
					strcpy(Text, "Nicht ausgewertet");
					break;

				}

				SetDlgItemText(hDlg, TEXT11, Text);
			}

		}

		if (wParam == 5)
		{

			SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_RESETCONTENT, 0, 0);
			if (Volume.SoundSystem == -1)
			{
				SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_ADDSTRING, 0, (LONG) (LPSTR) "Volume nicht zugeordnet");
			}
			else
			{
				sprintf(Text, "Volume-Control");
				SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_ADDSTRING, 0, (LONG) (LPSTR) Text);
			}

			if (Mute.SoundSystem == -1)
			{
				SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_ADDSTRING, 0, (LONG) (LPSTR) "Mute nicht zugeordnet");
			}
			else
			{
				sprintf(Text, "Mute-Control");
				SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_ADDSTRING, 0, (LONG) (LPSTR) Text);
			}

			for (i = 0; i < 64; i++)
			{
				if (MixerLoad[i].MixerAccess.SoundSystem >= 0)
				{
					sprintf(Text, "Startup-Sequenz %d", i);
					SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_ADDSTRING, 0, (LONG) (LPSTR) Text);
				}
			}

		}

		break;
	case WM_COMMAND:

		switch (LOWORD(wParam))
		{

		case IDC_COMBO1:
			if (HIWORD(wParam) == CBN_SELCHANGE)
				SendMessage(hDlg, WM_USER, 1, 0);
			break;
		case IDC_COMBO2:
			if (HIWORD(wParam) == CBN_SELCHANGE)
				SendMessage(hDlg, WM_USER, 2, 0);
			break;
		case IDC_COMBO3:
			if (HIWORD(wParam) == CBN_SELCHANGE)
				SendMessage(hDlg, WM_USER, 3, 0);
			break;
		case IDC_COMBO4:
			if (HIWORD(wParam) == CBN_SELCHANGE)
				SendMessage(hDlg, WM_USER, 4, 0);
			break;

		case IDC_CHECK1:
			C1 = SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0);
			C2 = SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_GETCURSEL, 0, 0);
			C3 = SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_GETCURSEL, 0, 0);
			C4 = SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_GETCURSEL, 0, 0);

			if (IsDlgButtonChecked(hDlg, IDC_CHECK1))
			{

				i = Get_Access_Slot(C1, C2, C3, C4);

				if (i < 0)
				{
					i = 0;
					while (i < 64)
					{
						if (MixerLoad[i].MixerAccess.SoundSystem == -1)
							break;
						i++;
					}
				}

				if (i >= 64)
				{
					ErrorBoxDlg(hDlg, "All Default-Slots full");
					return (TRUE);
				}

				Get_Mixer_UnsignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, UnsignedWert);
				MixerLoad[i].MixerAccess.SoundSystem = C1;
				MixerLoad[i].MixerAccess.Destination = C2;
				MixerLoad[i].MixerAccess.Connection = C3;
				MixerLoad[i].MixerAccess.Control = C4;
				MixerLoad[i].MixerValues.Kanal1 = UnsignedWert[0].dwValue;
				MixerLoad[i].MixerValues.Kanal2 = UnsignedWert[1].dwValue;
				MixerLoad[i].MixerValues.Kanal3 = UnsignedWert[2].dwValue;
				MixerLoad[i].MixerValues.Kanal4 = UnsignedWert[3].dwValue;

			}
			else
			{

				i = Get_Access_Slot(C1, C2, C3, C4);
				if (i >= 0)
				{
					MixerLoad[i].MixerAccess.SoundSystem = -1;
					MixerLoad[i].MixerAccess.Destination = 0;
					MixerLoad[i].MixerAccess.Connection = 0;
					MixerLoad[i].MixerAccess.Control = 0;
					MixerLoad[i].MixerValues.Kanal1 = 0;
					MixerLoad[i].MixerValues.Kanal2 = 0;
					MixerLoad[i].MixerValues.Kanal3 = 0;
					MixerLoad[i].MixerValues.Kanal4 = 0;
				}

			}
			SendMessage(hDlg, WM_USER, 5, 0);
			break;

		case IDC_CHECK2:

			if (IsDlgButtonChecked(hDlg, IDC_CHECK2))
			{
				C1 = SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0);
				C2 = SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_GETCURSEL, 0, 0);
				C3 = SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_GETCURSEL, 0, 0);
				C4 = SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_GETCURSEL, 0, 0);

				Mute.SoundSystem = C1;
				Mute.Destination = C2;
				Mute.Connection = C3;
				Mute.Control = C4;
			}
			else
			{
				Mute.SoundSystem = -1;
				Mute.Destination = 0;
				Mute.Connection = 0;
				Mute.Control = 0;
			}
			SendMessage(hDlg, WM_USER, 5, 0);

			break;

		case IDC_CHECK3:
			if (IsDlgButtonChecked(hDlg, IDC_CHECK3))
			{
				C1 = SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0);
				C2 = SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_GETCURSEL, 0, 0);
				C3 = SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_GETCURSEL, 0, 0);
				C4 = SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_GETCURSEL, 0, 0);

				Volume.SoundSystem = C1;
				Volume.Destination = C2;
				Volume.Connection = C3;
				Volume.Control = C4;
			}
			else
			{
				Volume.SoundSystem = -1;
				Volume.Destination = 0;
				Volume.Connection = 0;
				Volume.Control = 0;
			}
			SendMessage(hDlg, WM_USER, 5, 0);

			break;

		case IDC_LIST1:
			if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				i = SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_GETCURSEL, 0, 0);
				SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_GETTEXT, i, (LPARAM) Text);
				if (strncmp(Text, "Volume-", 7) == 0)
				{
					if (((Volume.SoundSystem >= 0) && (Volume.SoundSystem < SoundSystem.DeviceCount)) &&
						((Volume.Destination >= 0) && (Volume.Destination < SoundSystem.To_Lines[Volume.SoundSystem].LinesCount)) &&
						((Volume.Connection >= 0) && (Volume.Connection < SoundSystem.To_Lines[Volume.SoundSystem].To_Connection[Volume.Destination].ConnectionsCount)) &&
						((Volume.Control >= 0) && (Volume.Control < SoundSystem.To_Lines[Volume.SoundSystem].To_Connection[Volume.Destination].To_Control[Volume.Connection].ControlsCount)))
					{

						SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_SETCURSEL, Volume.SoundSystem, 0);
						SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_SETCURSEL, Volume.Destination, 0);
						SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_SETCURSEL, Volume.Connection, 0);
						SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_SETCURSEL, Volume.Control, 0);

					}
					else
					{
						ErrorBoxDlg(hDlg, "Volume-Control incorrectly assigned");
					}
				}
				else if (strncmp(Text, "Mute-", 5) == 0)
				{
					if (((Mute.SoundSystem >= 0) && (Mute.SoundSystem < SoundSystem.DeviceCount)) &&
						((Mute.Destination >= 0) && (Mute.Destination < SoundSystem.To_Lines[Mute.SoundSystem].LinesCount)) &&
						((Mute.Connection >= 0) && (Mute.Connection < SoundSystem.To_Lines[Mute.SoundSystem].To_Connection[Mute.Destination].ConnectionsCount)) &&
						((Mute.Control >= 0) && (Mute.Control < SoundSystem.To_Lines[Mute.SoundSystem].To_Connection[Mute.Destination].To_Control[Mute.Connection].ControlsCount)))
					{

						SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_SETCURSEL, Mute.SoundSystem, 0);
						SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_SETCURSEL, Mute.Destination, 0);
						SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_SETCURSEL, Mute.Connection, 0);
						SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_SETCURSEL, Mute.Control, 0);
					}
					else
					{
						ErrorBoxDlg(hDlg, "Mute-Control incorrectly assigned");
					}
				}
				else if (strncmp(Text, "Startup-Sequenz", 15) == 0)
				{
					ret = sscanf(Text, "%s %d", &Text1, &i);
					if (ret == 2)
					{
						if (((MixerLoad[i].MixerAccess.SoundSystem >= 0) && (MixerLoad[i].MixerAccess.SoundSystem < SoundSystem.DeviceCount)) &&
							((MixerLoad[i].MixerAccess.Destination >= 0) && (MixerLoad[i].MixerAccess.Destination < SoundSystem.To_Lines[MixerLoad[i].MixerAccess.SoundSystem].LinesCount)) &&
							((MixerLoad[i].MixerAccess.Connection >= 0)
							 && (MixerLoad[i].MixerAccess.Connection <
								 SoundSystem.To_Lines[MixerLoad[i].MixerAccess.SoundSystem].To_Connection[MixerLoad[i].MixerAccess.Destination].ConnectionsCount))
							&& ((MixerLoad[i].MixerAccess.Control >= 0)
								&& (MixerLoad[i].MixerAccess.Control <
									SoundSystem.To_Lines[MixerLoad[i].MixerAccess.SoundSystem].To_Connection[MixerLoad[i].MixerAccess.Destination].To_Control[MixerLoad[i].MixerAccess.Connection].
									ControlsCount)))
						{
							SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_SETCURSEL, MixerLoad[i].MixerAccess.SoundSystem, 0);
							SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_SETCURSEL, MixerLoad[i].MixerAccess.Destination, 0);
							SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_SETCURSEL, MixerLoad[i].MixerAccess.Connection, 0);
							SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_SETCURSEL, MixerLoad[i].MixerAccess.Control, 0);

							UnsignedWert[0].dwValue = MixerLoad[i].MixerValues.Kanal1;
							UnsignedWert[1].dwValue = MixerLoad[i].MixerValues.Kanal2;
							UnsignedWert[2].dwValue = MixerLoad[i].MixerValues.Kanal3;
							UnsignedWert[3].dwValue = MixerLoad[i].MixerValues.Kanal4;
							Set_Mixer_UnsignedValue(&SoundSystem.To_Lines[MixerLoad[i].MixerAccess.SoundSystem].To_Connection[MixerLoad[i].MixerAccess.Destination].
													To_Control[MixerLoad[i].MixerAccess.Connection].MixerDetail[MixerLoad[i].MixerAccess.Control], MixerLoad[i].MixerAccess.SoundSystem, UnsignedWert);

						}
						else
						{
							ErrorBoxDlg(hDlg, "Startup-Control incorrectly assigned");
						}
					}
				}

			}
			break;

		case IDC_BUTTON1:
			C1 = SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0);
			C2 = SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_GETCURSEL, 0, 0);
			C3 = SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_GETCURSEL, 0, 0);
			C4 = SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_GETCURSEL, 0, 0);
			Get_Mixer_BoolValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, BoolWert);
			if (BoolWert[0].fValue == 1)
				BoolWert[0].fValue = 0;
			else
				BoolWert[0].fValue = 1;
			Set_Mixer_BoolValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, BoolWert);
			break;

		case IDC_BUTTON2:
			C1 = SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0);
			C2 = SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_GETCURSEL, 0, 0);
			C3 = SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_GETCURSEL, 0, 0);
			C4 = SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_GETCURSEL, 0, 0);
			if ((SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].dwControlType & MIXERCONTROL_CT_UNITS_MASK) == MIXERCONTROL_CT_UNITS_SIGNED)
			{
				Get_Mixer_SignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, SignedWert);
				if (SignedWert[0].lValue < SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Bounds.lMaximum)
					SignedWert[0].lValue += SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Metrics.cSteps;
				Set_Mixer_SignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, SignedWert);
			}

			if ((SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].dwControlType & MIXERCONTROL_CT_UNITS_MASK) == MIXERCONTROL_CT_UNITS_UNSIGNED)
			{
				Get_Mixer_UnsignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, UnsignedWert);
				if (UnsignedWert[0].dwValue < SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Bounds.dwMaximum)
					UnsignedWert[0].dwValue += SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Metrics.cSteps;
				Set_Mixer_UnsignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, UnsignedWert);

			}

			break;

		case IDC_BUTTON3:
			C1 = SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0);
			C2 = SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_GETCURSEL, 0, 0);
			C3 = SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_GETCURSEL, 0, 0);
			C4 = SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_GETCURSEL, 0, 0);
			if ((SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].dwControlType & MIXERCONTROL_CT_UNITS_MASK) == MIXERCONTROL_CT_UNITS_SIGNED)
			{
				Get_Mixer_SignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, SignedWert);
				if (SignedWert[0].lValue > SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Bounds.lMinimum)
					SignedWert[0].lValue -= SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Metrics.cSteps;
				Set_Mixer_SignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, SignedWert);
			}

			if ((SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].dwControlType & MIXERCONTROL_CT_UNITS_MASK) == MIXERCONTROL_CT_UNITS_UNSIGNED)
			{
				Get_Mixer_UnsignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, UnsignedWert);
				if (UnsignedWert[0].dwValue > SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Bounds.dwMinimum)
					UnsignedWert[0].dwValue -= SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Metrics.cSteps;
				Set_Mixer_UnsignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, UnsignedWert);
			}

			break;

		case IDC_BUTTON4:
			C1 = SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0);
			C2 = SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_GETCURSEL, 0, 0);
			C3 = SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_GETCURSEL, 0, 0);
			C4 = SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_GETCURSEL, 0, 0);
			Get_Mixer_BoolValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, BoolWert);
			if (BoolWert[1].fValue == 1)
				BoolWert[1].fValue = 0;
			else
				BoolWert[1].fValue = 1;
			Set_Mixer_BoolValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, BoolWert);
			break;

		case IDC_BUTTON5:
			C1 = SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0);
			C2 = SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_GETCURSEL, 0, 0);
			C3 = SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_GETCURSEL, 0, 0);
			C4 = SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_GETCURSEL, 0, 0);
			if ((SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].dwControlType & MIXERCONTROL_CT_UNITS_MASK) == MIXERCONTROL_CT_UNITS_SIGNED)
			{
				Get_Mixer_SignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, SignedWert);
				if (SignedWert[1].lValue < SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Bounds.lMaximum)
					SignedWert[1].lValue += SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Metrics.cSteps;
				Set_Mixer_SignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, SignedWert);
			}

			if ((SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].dwControlType & MIXERCONTROL_CT_UNITS_MASK) == MIXERCONTROL_CT_UNITS_UNSIGNED)
			{
				Get_Mixer_UnsignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, UnsignedWert);
				if (UnsignedWert[1].dwValue < SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Bounds.dwMaximum)
					UnsignedWert[1].dwValue += SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Metrics.cSteps;
				Set_Mixer_UnsignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, UnsignedWert);

			}

			break;

		case IDC_BUTTON6:
			C1 = SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0);
			C2 = SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_GETCURSEL, 0, 0);
			C3 = SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_GETCURSEL, 0, 0);
			C4 = SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_GETCURSEL, 0, 0);
			if ((SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].dwControlType & MIXERCONTROL_CT_UNITS_MASK) == MIXERCONTROL_CT_UNITS_SIGNED)
			{
				Get_Mixer_SignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, SignedWert);
				if (SignedWert[1].lValue > SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Bounds.lMinimum)
					SignedWert[1].lValue -= SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Metrics.cSteps;
				Set_Mixer_SignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, SignedWert);
			}

			if ((SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].dwControlType & MIXERCONTROL_CT_UNITS_MASK) == MIXERCONTROL_CT_UNITS_UNSIGNED)
			{
				Get_Mixer_UnsignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, UnsignedWert);
				if (UnsignedWert[1].dwValue > SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Bounds.dwMinimum)
					UnsignedWert[1].dwValue -= SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Metrics.cSteps;
				Set_Mixer_UnsignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, UnsignedWert);
			}

			break;

		case IDC_BUTTON7:
			C1 = SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0);
			C2 = SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_GETCURSEL, 0, 0);
			C3 = SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_GETCURSEL, 0, 0);
			C4 = SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_GETCURSEL, 0, 0);
			Get_Mixer_BoolValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, BoolWert);
			if (BoolWert[2].fValue == 1)
				BoolWert[2].fValue = 0;
			else
				BoolWert[2].fValue = 1;
			Set_Mixer_BoolValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, BoolWert);
			break;

		case IDC_BUTTON8:
			C1 = SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0);
			C2 = SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_GETCURSEL, 0, 0);
			C3 = SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_GETCURSEL, 0, 0);
			C4 = SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_GETCURSEL, 0, 0);
			if ((SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].dwControlType & MIXERCONTROL_CT_UNITS_MASK) == MIXERCONTROL_CT_UNITS_SIGNED)
			{
				Get_Mixer_SignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, SignedWert);
				if (SignedWert[2].lValue < SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Bounds.lMaximum)
					SignedWert[2].lValue += SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Metrics.cSteps;
				Set_Mixer_SignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, SignedWert);
			}

			if ((SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].dwControlType & MIXERCONTROL_CT_UNITS_MASK) == MIXERCONTROL_CT_UNITS_UNSIGNED)
			{
				Get_Mixer_UnsignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, UnsignedWert);
				if (UnsignedWert[2].dwValue < SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Bounds.dwMaximum)
					UnsignedWert[2].dwValue += SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Metrics.cSteps;
				Set_Mixer_UnsignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, UnsignedWert);

			}

			break;

		case IDC_BUTTON9:
			C1 = SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0);
			C2 = SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_GETCURSEL, 0, 0);
			C3 = SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_GETCURSEL, 0, 0);
			C4 = SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_GETCURSEL, 0, 0);
			if ((SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].dwControlType & MIXERCONTROL_CT_UNITS_MASK) == MIXERCONTROL_CT_UNITS_SIGNED)
			{
				Get_Mixer_SignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, SignedWert);
				if (SignedWert[2].lValue > SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Bounds.lMinimum)
					SignedWert[2].lValue -= SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Metrics.cSteps;
				Set_Mixer_SignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, SignedWert);
			}

			if ((SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].dwControlType & MIXERCONTROL_CT_UNITS_MASK) == MIXERCONTROL_CT_UNITS_UNSIGNED)
			{
				Get_Mixer_UnsignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, UnsignedWert);
				if (UnsignedWert[2].dwValue > SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Bounds.dwMinimum)
					UnsignedWert[2].dwValue -= SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Metrics.cSteps;
				Set_Mixer_UnsignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, UnsignedWert);
			}

			break;

		case IDC_BUTTON10:

			C1 = SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0);
			C2 = SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_GETCURSEL, 0, 0);
			C3 = SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_GETCURSEL, 0, 0);
			C4 = SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_GETCURSEL, 0, 0);
			Get_Mixer_BoolValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, BoolWert);
			if (BoolWert[3].fValue == 1)
				BoolWert[3].fValue = 0;
			else
				BoolWert[3].fValue = 1;
			Set_Mixer_BoolValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, BoolWert);
			break;

		case IDC_BUTTON11:
			C1 = SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0);
			C2 = SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_GETCURSEL, 0, 0);
			C3 = SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_GETCURSEL, 0, 0);
			C4 = SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_GETCURSEL, 0, 0);
			if ((SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].dwControlType & MIXERCONTROL_CT_UNITS_MASK) == MIXERCONTROL_CT_UNITS_SIGNED)
			{
				Get_Mixer_SignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, SignedWert);
				if (SignedWert[0].lValue < SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Bounds.lMaximum)
					SignedWert[0].lValue += SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Metrics.cSteps;
				Set_Mixer_SignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, SignedWert);
			}

			if ((SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].dwControlType & MIXERCONTROL_CT_UNITS_MASK) == MIXERCONTROL_CT_UNITS_UNSIGNED)
			{
				Get_Mixer_UnsignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, UnsignedWert);
				if (UnsignedWert[3].dwValue < SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Bounds.dwMaximum)
					UnsignedWert[3].dwValue += SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Metrics.cSteps;
				Set_Mixer_UnsignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, UnsignedWert);

			}

			break;

		case IDC_BUTTON12:
			C1 = SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0);
			C2 = SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_GETCURSEL, 0, 0);
			C3 = SendMessage(GetDlgItem(hDlg, IDC_COMBO3), CB_GETCURSEL, 0, 0);
			C4 = SendMessage(GetDlgItem(hDlg, IDC_COMBO4), CB_GETCURSEL, 0, 0);
			if ((SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].dwControlType & MIXERCONTROL_CT_UNITS_MASK) == MIXERCONTROL_CT_UNITS_SIGNED)
			{
				Get_Mixer_SignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, SignedWert);
				if (SignedWert[3].lValue > SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Bounds.lMinimum)
					SignedWert[3].lValue -= SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Metrics.cSteps;
				Set_Mixer_SignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, SignedWert);
			}

			if ((SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].dwControlType & MIXERCONTROL_CT_UNITS_MASK) == MIXERCONTROL_CT_UNITS_UNSIGNED)
			{
				Get_Mixer_UnsignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, UnsignedWert);
				if (UnsignedWert[3].dwValue > SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Bounds.dwMinimum)
					UnsignedWert[3].dwValue -= SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerControl[C4].Metrics.cSteps;
				Set_Mixer_UnsignedValue(&SoundSystem.To_Lines[C1].To_Connection[C2].To_Control[C3].MixerDetail[C4], C1, UnsignedWert);
			}

			break;

		case IDOK:

			EndDialog(hDlg, TRUE);
			break;

		case IDCANCEL:

			EndDialog(hDlg, TRUE);
			break;
		}

		return (TRUE);

	}

	return (FALSE);
}
