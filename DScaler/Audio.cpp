/////////////////////////////////////////////////////////////////////////////
// Audio.c
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
// Change Log
//
// Date          Developer             Changes
//
// 11 Aug 2000   John Adcock           Moved Audio Functions in here
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 08 Jan 2001   John Adcock           Maybe fixed crashing bug
//
// 26 Feb 2001   Hermes Conrad         Sound Fixes
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "i2c.h"
#include "audio.h"
#include "tvcards.h"
#include "Status.h"

BYTE AudioDeviceWrite;
BYTE AudioDeviceRead;

BOOL Has_MSP = FALSE;
char MSPStatus[40] = "";
char MSPVersion[16] = "";

int InitialVolume = 900;  // HC 26/Feb/2001 Changed to avoid digital clipping
char InitialBalance = 0x00;
char InitialLoudness = 0x00;
char InitialBass = 0x00;
char InitialTreble = 0x00;
BOOL InitialSuperBass = FALSE;
char InitialEqualizer[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
char InitialSpatial = 0x00;

AUDIOMUXTYPE AudioSource = AUDIOMUX_MUTE;

/*
0		Audio_Tuner,
1		Audio_Radio,
2		Audio_External,
3		Audio_Internal,
4		Audio_Off,
5		Audio_On,
0x80	Audio_Mute = 0x80,
0x81	Audio_UnMute = 0x81
*/

#define WriteDem(wAddr,wData) Audio_WriteMSP(MSP_WR_DEM,wAddr,wData) // I2C_MSP3400C_DEM
#define WriteDSP(wAddr,wData) Audio_WriteMSP(MSP_WR_DSP,wAddr,wData) // I2C_MSP3400C_DEM
#define ReadDem(wAddr) Audio_ReadMSP(MSP_RD_DEM,wAddr)  // I2C_MSP3400C_DFP
#define ReadDSP(wAddr) Audio_ReadMSP(MSP_RD_DSP,wAddr)  // I2C_MSP3400C_DFP

static struct MSP_INIT_DATA_DEM
{
	int fir1[6];
	int fir2[6];
	int cdo1;
	int cdo2;
	int ad_cv;
	int mode_reg;
	int dfp_src;
	int dfp_matrix;
	int autodetect; // MAE 8 Dec 2000
} MSP_init_data[] =
{
	/* AM (for carrier detect / msp3400) */
	{ { 75, 19, 36, 35, 39, 40 }, { 75, 19, 36, 35, 39, 40 },
	  MSP_CARRIER(5.5), MSP_CARRIER(5.5),
	  0x00d0, 0x0500,   0x0020, 0x3000, 1},

	/* AM (for carrier detect / msp3410) */
	{ { -1, -1, -8, 2, 59, 126 }, { -1, -1, -8, 2, 59, 126 },
	  MSP_CARRIER(5.5), MSP_CARRIER(5.5),
	  0x00d0, 0x0100,   0x0020, 0x3000, 0},

	/* FM Radio */
	{ { -8, -8, 4, 6, 78, 107 }, { -8, -8, 4, 6, 78, 107 },
	  MSP_CARRIER(10.7), MSP_CARRIER(10.7),
	  0x00d0, 0x0480, 0x0020, 0x3000, 0 },

	/* Terrestial FM-mono + FM-stereo */
	{ {  3, 18, 27, 48, 66, 72 }, {  3, 18, 27, 48, 66, 72 },
	  MSP_CARRIER(5.5), MSP_CARRIER(5.5),
	  0x00d0, 0x0480,   0x0030, 0x3000, 0}, 

	/* Sat FM-mono */
	{ {  1,  9, 14, 24, 33, 37 }, {  3, 18, 27, 48, 66, 72 },
	  MSP_CARRIER(6.5), MSP_CARRIER(6.5),
	  0x00c6, 0x0480,   0x0000, 0x3000, 0},

	/* NICAM B/G, D/K */
	{ { -2, -8, -10, 10, 50, 86 }, {  3, 18, 27, 48, 66, 72 },
	  MSP_CARRIER(5.5), MSP_CARRIER(5.5),
	  0x00d0, 0x0040,   0x0120, 0x3000, 0},

	/* NICAM I */
	{ {  2, 4, -6, -4, 40, 94 }, {  3, 18, 27, 48, 66, 72 },
	  MSP_CARRIER(6.0), MSP_CARRIER(6.0),
	  0x00d0, 0x0040,   0x0120, 0x3000, 0},

	 /* NICAM/AM -- L (6.5/5.85) */
	{ {  -2, -8, -10, 10, 50, 86 }, {  -4, -12, -9, 23, 79, 126 },
	  MSP_CARRIER(6.5), MSP_CARRIER(6.5),
	  0x00c6, 0x0140,   0x0120, 0x7c03},
};


int carrier_detect_main[4] = {
	/* main carrier */
	 MSP_CARRIER(4.5),   // 4.5   NTSC
	 MSP_CARRIER(5.5),   // 5.5   PAL B/G
	 MSP_CARRIER(6.0),   // 6.0   PAL I
	 MSP_CARRIER(6.5),   // 6.5   PAL D/K + SAT + SECAM
};

int carrier_detect[8] = {
	/* PAL B/G */
	 MSP_CARRIER(5.7421875), // 5.742 PAL B/G FM-stereo
	 MSP_CARRIER(5.85),      // 5.85  PAL B/G NICAM
	/* PAL SAT / SECAM */
	 MSP_CARRIER(5.85),      // 5.85  PAL D/K NICAM
	 MSP_CARRIER(6.2578125), //6.25  PAL D/K1 FM-stereo
	 MSP_CARRIER(6.7421875), //6.74  PAL D/K2 FM-stereo
	 MSP_CARRIER(7.02),      //7.02  PAL SAT FM-stereo s/b
	 MSP_CARRIER(7.20),      //7.20  PAL SAT FM-stereo s
	 MSP_CARRIER(7.38),      //7.38  PAL SAT FM-stereo b
};

int MSPMode = 3;
int MSPStereo = VIDEO_SOUND_STEREO; // MAE 8 Dec 2000 Added default
int MSPNewStereo;
int MSPAutoDetectValue;
BOOL MSPNicam;
int MSPMajorMode = 0; // MAE 8 Dec 2000 Added default
int MSPMinorMode = 0; // MAE 8 Dec 2000 Added default
BOOL AutoStereoSelect = FALSE; // MAE 8 Dec 2000 Changed default

// MAE Added 8 Dec 200 
BOOL Audio_Mute(void)
{
	int nVolume;
	nVolume = MulDiv(400, 0x7f0, 1400);
	WriteDSP(0, nVolume << 4);
	WriteDSP(6, nVolume << 4);
	return TRUE;
}

BOOL Audio_SetSource(AUDIOMUXTYPE nChannel)
{
	int i;
	DWORD MuxSelect;

	BT848_AndOrDataDword(BT848_GPIO_OUT_EN, GetCardSetup()->GPIOMask, ~GetCardSetup()->GPIOMask);

	switch(nChannel)
	{
	case AUDIOMUX_MSP_RADIO:
	case AUDIOMUX_MUTE:
		// just get on with it
		MuxSelect = GetCardSetup()->AudioMuxSelect[nChannel];
		break;
	default:
		// see if there is a video signal present
		i = 0;
		while ((i < 20) && (!(BT848_ReadByte(BT848_DSTATUS) & BT848_DSTATUS_PRES)))
		{
			i++;
			Sleep(50);
		}
		/* if video not in H-lock, turn audio off */
		if (i == 20)
		{
			MuxSelect = GetCardSetup()->AudioMuxSelect[AUDIOMUX_MUTE];
		}
		else
		{
			MuxSelect = GetCardSetup()->AudioMuxSelect[nChannel];
		}
		break;
	}

	/* select direct input */
	//BT848_WriteWord(BT848_GPIO_REG_INP, 0x00); // MAE 14 Dec 2000 disabled
	BT848_AndOrDataDword(BT848_GPIO_DATA, MuxSelect, ~GetCardSetup()->GPIOMask); 
	return TRUE;
}

BOOL Audio_MSP_IsPresent()
{
	return Has_MSP;
}

const char* Audio_MSP_Status()
{
	return MSPStatus;
}

const char* Audio_MSP_VersionString()
{
	return MSPVersion;
}

BOOL Audio_MSP_Init(BYTE DWrite, BYTE DRead)
{
	int i;

	AudioDeviceWrite = DWrite;
	AudioDeviceRead = DRead;

	Has_MSP = FALSE;

	if (!I2CBus_AddDevice(DRead))
	{
		return (FALSE);
	}

	if (!I2CBus_AddDevice(DWrite))
	{
		return (FALSE);
	}

	Has_MSP = TRUE;

	strncpy(MSPStatus, "MSP-Device I2C-Bus I/O 0x80/0x81",sizeof(MSPStatus));

	Audio_MSP_Reset();
	Sleep(4);
	Audio_MSP_Version();
	Sleep(4);

	// JA 20010108 Think this is right
	// MSP_init_data is indexed by MSPMode
	// elsewhere in the code
	// at the moment we go into the autodetect code
	// all the time, could be the cause of the crashes
	// was if (MSP_init_data[MSPMajorMode].autodetect)
	if (MSP_init_data[MSPMode].autodetect)
	{
		WriteDem(0x20,0x0001); // Autodetect source
	}
	else
	{
		Audio_MSP_SetMode(MSPMode);
		Sleep(4);
		Audio_MSP_Set_MajorMinor_Mode(MSPMajorMode, MSPMinorMode);
		Sleep(4);
	}

	Audio_SetVolume(InitialVolume);
	Audio_SetBalance(InitialBalance);
	Audio_SetSuperBass(InitialSuperBass);
	Audio_SetBass(InitialBass);
	Audio_SetTreble(InitialTreble);
	Audio_SetLoudness(InitialLoudness);
	Audio_SetSpatial(InitialSpatial);
	for(i = 0; i < 5; i++)
	{
		Audio_SetEqualizer(i, InitialEqualizer[i]);
	}

	Audio_MSP_SetStereo(MSPMajorMode, MSPMinorMode, MSPStereo);

	return (TRUE);
}

BOOL Audio_WriteMSP(BYTE bSubAddr, int wAddr, int wData)
{
	I2CBus_Lock();
	I2CBus_Start();
	I2CBus_SendByte(AudioDeviceWrite, 0);
	I2CBus_SendByte(bSubAddr, 0);
	if (bSubAddr != MSP_CONTROL && bSubAddr != MSP_TEST)
	{
		I2CBus_SendByte((BYTE) (wAddr >> 8), 0);
		I2CBus_SendByte((BYTE) (wAddr & 0xFF), 0);
	}
	I2CBus_SendByte((BYTE) (wData >> 8), 0);
	I2CBus_SendByte((BYTE) (wData & 0xFF), 0);
	I2CBus_Stop();
	I2CBus_Unlock();
	return TRUE;
}

WORD Audio_ReadMSP(BYTE bSubAddr, WORD wAddr)
{
	WORD wResult;
	BYTE B0, B1;

	B0 = (BYTE) (wAddr >> 8);
	B1 = (BYTE) (wAddr & 0xFF);
	I2CBus_Lock();
	I2CBus_Start();
	I2CBus_SendByte(AudioDeviceWrite, 2);
	I2CBus_SendByte(bSubAddr, 0);
	I2CBus_SendByte(B0, 0);
	I2CBus_SendByte(B1, 0);
	I2CBus_Start();
	if (I2CBus_SendByte(AudioDeviceRead, 2))
	{
		B0 = I2CBus_ReadByte(0);
		B1 = I2CBus_ReadByte(1);
		wResult = B0 << 8 | B1;
	}
	else
	{
		wResult = -1;
	}
	I2CBus_Stop();
	I2CBus_Unlock();
	return wResult;
}

BOOL Audio_SetVolume(int nVolume)
{
	Audio_SetToneControl();
	if (nVolume < 0 || nVolume > 1000)
		return FALSE;
	InitialVolume = nVolume;
	if (nVolume > 0)
		nVolume = MulDiv(nVolume + 400, 0x7f0, 1400);
	WriteDSP(0, nVolume << 4);
	WriteDSP(6, nVolume << 4);

	return TRUE;
}

BOOL Audio_SetBalance(char nBalance)
{
	Audio_SetToneControl();
	InitialBalance = nBalance;
	WriteDSP(1, nBalance << 8);
	WriteDSP(0x30, nBalance << 8);
	return TRUE;
}

BOOL Audio_SetBass(char nBass)
{
	Audio_SetToneControl();
	if (nBass < -96)
		return FALSE;
	InitialBass = nBass;
	WriteDSP(2, nBass << 8);
	WriteDSP(0x31, nBass << 8);
	return TRUE;
}

BOOL Audio_SetTreble(char nTreble)
{
	Audio_SetToneControl();
	if (nTreble < -96)
		return FALSE;
	InitialTreble = nTreble;
	WriteDSP(3, nTreble << 8);
	WriteDSP(0x32, nTreble << 8);
	return TRUE;
}

BOOL Audio_SetLoudness(BYTE nLoudness)
{
	Audio_SetToneControl();
	if (nLoudness > 68)
		return FALSE;
	InitialLoudness = nLoudness;
	WriteDSP(4, (nLoudness << 8) + (InitialSuperBass ? 0x4 : 0));
	WriteDSP(0x33, (nLoudness << 8) + (InitialSuperBass ? 0x4 : 0));
	return TRUE;
}

BOOL Audio_SetSuperBass(BOOL bSuperBass)
{
	Audio_SetToneControl();
	InitialSuperBass = bSuperBass;
	WriteDSP(4, (InitialLoudness << 8) + (bSuperBass ? 0x4 : 0));
	WriteDSP(0x33, (InitialLoudness << 8) + (bSuperBass ? 0x4 : 0));
	return TRUE;
}

BOOL Audio_SetSpatial(char nSpatial)
{
	Audio_SetToneControl();
	InitialSpatial = nSpatial;
	WriteDSP(0x5, (nSpatial << 8) + 0x8);	// Mode A, Automatic high pass gain
	return TRUE;
}

BOOL Audio_SetEqualizer(int nIndex, char nLevel)
{
	Audio_SetToneControl();
	if (nLevel < -96 || nLevel > 96)
		return FALSE;
	InitialEqualizer[nIndex] = nLevel;
	WriteDSP(0x21 + nIndex, nLevel << 8);
	return TRUE;
}

void Audio_SetToneControl()
{
	int i;

	WriteDSP(2, InitialBass << 8);	// Bass
	WriteDSP(0x31, InitialBass << 8);
	WriteDSP(3, InitialTreble << 8);	// Treble
	WriteDSP(0x32, InitialTreble << 8);
	for(i = 0; i < 5; i++)
	{
		WriteDSP(0x21 + i, InitialEqualizer[i]);	// Eq
	}
	WriteDSP(0x20, 0);		// Mode control here (need eq=0)
}

BOOL Audio_MSP_Reset()
{
	BOOL ret = TRUE;

	I2CBus_Lock();
	I2CBus_Start();
	I2CBus_SendByte(AudioDeviceWrite, 5);
	I2CBus_SendByte(0x00, 0);
	I2CBus_SendByte(0x80, 0);
	I2CBus_SendByte(0x00, 0);
	I2CBus_Stop();
	I2CBus_Start();
	if ((I2CBus_SendByte(AudioDeviceWrite, 5) == FALSE) ||
		(I2CBus_SendByte(0x00, 0) == FALSE) ||
		(I2CBus_SendByte(0x00, 0) == FALSE) ||
		(I2CBus_SendByte(0x00, 0) == FALSE))
		ret = FALSE;

	I2CBus_Stop();
	I2CBus_Unlock();

	return ret;
}

void Audio_MSP_SetCarrier(int cdo1, int cdo2)
{
	WriteDem(0x93, cdo1 & 0xfff);
	WriteDem(0x9b, cdo1 >> 12);
	WriteDem(0xa3, cdo2 & 0xfff);
	WriteDem(0xab, cdo2 >> 12);
	WriteDem(0x56, 0);
}

void Audio_MSP_SetMode(int type)
{
	int i;

	MSPMode = type;

	WriteDem(0xbb, MSP_init_data[type].ad_cv);

	for (i = 5; i >= 0; i--)
		WriteDem(0x01, MSP_init_data[type].fir1[i]);

	WriteDem(0x05, 0x0004);		/* fir 2 */
	WriteDem(0x05, 0x0040);
	WriteDem(0x05, 0x0000);

	for (i = 5; i >= 0; i--)
		WriteDem(0x05, MSP_init_data[type].fir2[i]);

	WriteDem(0x83, MSP_init_data[type].mode_reg);

	Audio_MSP_SetCarrier(MSP_init_data[type].cdo1, MSP_init_data[type].cdo2);

	WriteDSP(0x08, MSP_init_data[type].dfp_src);
	WriteDSP(0x09, MSP_init_data[type].dfp_src);
	WriteDSP(0x0a, MSP_init_data[type].dfp_src);
	WriteDSP(0x0e, MSP_init_data[type].dfp_matrix);

// msp3410 needs some more initialization
	if (MSPNicam)
		WriteDSP(0x10, 0x5a00);

}

void Audio_MSP_SetStereo(int MajorMode, int MinorMode, int mode)
{
	int nicam = 0;
	int src = 0;

	MSPStereo = mode;

	// switch demodulator
	switch (MSPMode)
	{
	case MSP_MODE_FM_TERRA:
		Audio_MSP_SetCarrier(carrier_detect[MinorMode], carrier_detect_main[MajorMode]);
		switch (MSPStereo)
		{
		case VIDEO_SOUND_STEREO:
			WriteDSP(0x15, 0x0000); // HC 22/Feb/2001 Identification Mode B/G
			WriteDSP(0x0e, 0x3001); // HC 22/Feb/2001 
			WriteDSP(0xbb, 0x00d0); // HC 22/Feb/2001 AGC On
			break;
		case VIDEO_SOUND_MONO:
		case VIDEO_SOUND_LANG1:
		case VIDEO_SOUND_LANG2:
			WriteDSP(0x0e, 0x3000); // MAE 8 Dec 2000
			break;
		}
		break;
	case MSP_MODE_FM_SAT:
		switch (MSPStereo)
		{
		case VIDEO_SOUND_MONO:
			Audio_MSP_SetCarrier(MSP_CARRIER(6.5), MSP_CARRIER(6.5));
			break;
		case VIDEO_SOUND_STEREO:
			Audio_MSP_SetCarrier(MSP_CARRIER(7.2), MSP_CARRIER(7.02));
			break;
		case VIDEO_SOUND_LANG1:
			Audio_MSP_SetCarrier(MSP_CARRIER(7.38), MSP_CARRIER(7.02));
			break;
		case VIDEO_SOUND_LANG2:
			Audio_MSP_SetCarrier(MSP_CARRIER(7.38), MSP_CARRIER(7.02));
			break;
		}
		break;
	case MSP_MODE_FM_NICAM1:
	case MSP_MODE_FM_NICAM2:
		Audio_MSP_SetCarrier(carrier_detect[MinorMode], carrier_detect_main[MajorMode]);
		nicam = 0x0100;
		break;
	default:
		// can't do stereo - abort here
		return;
	}

	// switch audio
	switch (MSPStereo)
	{
	case VIDEO_SOUND_STEREO:
		src = 0x0020 | nicam;
		break;
	case VIDEO_SOUND_MONO:
	case VIDEO_SOUND_LANG1:
		src = 0x0000 | nicam;
		break;
	case VIDEO_SOUND_LANG2:
		src = 0x0010 | nicam;
		break;
	}
	WriteDSP(0x08, src);
	WriteDSP(0x09, src);
	WriteDSP(0x0a, src);
}

BOOL Audio_MSP_Version()
{
	int rev1, rev2;

	MSPNicam = FALSE;

	rev1 = ReadDSP(0x1e);
	rev2 = ReadDSP(0x1f);
	if (0 == rev1 && 0 == rev2)
	{
		return (FALSE);
	}
	MSPAutoDetectValue = 3;

	sprintf(MSPVersion, "MSP34%02d%c-%c%d", (rev2 >> 8) & 0xff, (rev1 & 0xff) + '@', ((rev1 >> 8) & 0xff) + '@', rev2 & 0x1f);
	MSPNicam = (((rev2 >> 8) & 0xff) != 00) ? 1 : 0;
	if (MSPNicam == TRUE)
	{
		MSPAutoDetectValue = 5;

	}
	return (TRUE);
}

void Audio_MSP_Set_MajorMinor_Mode(int MajorMode, int MinorMode)
{
	MSPMajorMode = MajorMode;
	MSPMinorMode = MinorMode;

	switch (MajorMode)
	{
	case 1:					// 5.5
		if (MinorMode == 0)
		{
			// B/G FM-stereo
//              Audio_MSP_SetMode(Audio_MSP_MODE_FM_TERRA);
			Audio_MSP_SetStereo(MajorMode, MinorMode, VIDEO_SOUND_MONO);
		}
		else if (MinorMode == 1 && MSPNicam)
		{
			// B/G NICAM
//              Audio_MSP_SetMode(Audio_MSP_MODE_FM_NICAM1);
			Audio_MSP_SetCarrier(carrier_detect[MinorMode], carrier_detect_main[MajorMode]);
		}
		else
		{
//          Audio_MSP_SetMode(Audio_MSP_MODE_FM_TERRA);
			Audio_MSP_SetCarrier(carrier_detect[MinorMode], carrier_detect_main[MajorMode]);
		}
		break;
	case 2:					// 6.0
		// PAL I NICAM
//          Audio_MSP_SetMode(Audio_MSP_MODE_FM_NICAM2);
		//Audio_MSP_SetCarrier(MSP_CARRIER(6.552), carrier_detect_main[MajorMode]);
		Audio_MSP_SetCarrier(MSP_CARRIER(6.55), carrier_detect_main[MajorMode]);
		break;
	case 3:					// 6.5
		if (MinorMode == 1 || MinorMode == 2)
		{
			// D/K FM-stereo
//              Audio_MSP_SetMode( Audio_MSP_MODE_FM_TERRA);
			Audio_MSP_SetStereo(MajorMode, MinorMode, VIDEO_SOUND_MONO);
		}
		else if (MinorMode == 0 && MSPNicam)
		{
			// D/K NICAM
//              Audio_MSP_SetMode(Audio_MSP_MODE_FM_NICAM1);
			Audio_MSP_SetCarrier(carrier_detect[MinorMode], carrier_detect_main[MajorMode]);
		}
		else
		{
//          Audio_MSP_SetMode(Audio_MSP_MODE_FM_TERRA);
			Audio_MSP_SetCarrier(carrier_detect[MinorMode], carrier_detect_main[MajorMode]);
		}
		break;
	case 0:					// 4.5
	default:
//          Audio_MSP_SetMode(Audio_MSP_MODE_FM_TERRA);
//			Audio_MSP_SetStereo(MajorMode, MinorMode, VIDEO_SOUND_STEREO); // MAETEST
		Audio_MSP_SetCarrier(carrier_detect[MinorMode], carrier_detect_main[MajorMode]);
		break;
	}

}

void Audio_MSP_Print_Mode()
{
	char Text[128];

	if (Has_MSP == FALSE)
		strcpy(Text, "No MSP Audio Device");
	else
	{
		switch (MSPMode)
		{
		case 0:
			strcpy(Text, "AM (msp3400)+");
			break;
		case 1:
			strcpy(Text, "AM (msp3410)+");
			break;
		case 2:
			strcpy(Text, "FM Radio+");
			break;
		case 3:
			strcpy(Text, "TV Terrestial+");
			break;
		case 4:
			strcpy(Text, "TV Sat+");
			break;
		case 5:
			strcpy(Text, "NICAM B/G+");
			break;
		case 6:
			strcpy(Text, "NICAM I+");
			break;
		}

		switch (MSPMajorMode)
		{
		case 0:
			strcat(Text, "NTSC+");
			break;
		case 1:
			strcat(Text, "PAL B/G+");
			break;
		case 2:
			strcat(Text, "PAL I+");
			break;
		case 3:
			strcat(Text, "PAL D/K (Sat+Secam)+");
			break;
		}

		switch (MSPMinorMode)
		{
		case 0:
			strcat(Text, "FM-stereo ");
			break;
		case 1:
			strcat(Text, "NICAM ");
			break;
		case 2:
			strcat(Text, "NICAM ");
			break;
		case 3:
			strcat(Text, "D/K1 FM-Stereo ");
			break;
		case 4:
			strcat(Text, "D/K2 FM-stereo ");
			break;
		case 5:
			strcat(Text, "SAT FM-stereo s/b ");
			break;
		case 6:
			strcat(Text, "SAT FM-stereo s ");
			break;
		case 7:
			strcat(Text, "SAT FM-stereo b ");
			break;
		}

		switch (MSPStereo)
		{
		case 1:
			strcat(Text, "(Mono)");
			break;
		case 2:
			strcat(Text, "(Stereo)");
			break;
		case 3:
			strcat(Text, "(Channel 1)");
			break;
		case 4:
			strcat(Text, "(Channel 2)");
			break;
		}
	}
	StatusBar_ShowText(STATUS_AUDIO, Text);
}

void Audio_MSP_Watch_Mode()
{
	int val;
	int newstereo = MSPStereo;
	
	if(!Has_MSP) return;

	Sleep(2);
	switch (MSPMode)
	{
	case MSP_MODE_FM_TERRA:

		val = ReadDSP(0x18);
		if (val > 4096)
		{
			newstereo = VIDEO_SOUND_STEREO;
		}
		else if (val < -4096)
		{
			newstereo = VIDEO_SOUND_LANG1;
		}
		else
		{
			newstereo = VIDEO_SOUND_MONO;
		}
		break;
	case MSP_MODE_FM_NICAM1:
	case MSP_MODE_FM_NICAM2:
		val = ReadDSP(0x23);
		switch ((val & 0x1e) >> 1)
		{
		case 0:
		case 8:
			newstereo = VIDEO_SOUND_STEREO;
			break;
		default:
			newstereo = VIDEO_SOUND_MONO;
			break;
		}
		break;
	}

	if (MSPStereo != newstereo)
	{
		if (AutoStereoSelect == TRUE)
			Audio_MSP_SetStereo(MSPMajorMode, MSPMinorMode, newstereo);
		else
			MSPNewStereo = newstereo;

	}
}

