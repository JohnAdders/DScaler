/////////////////////////////////////////////////////////////////////////////
// Tuner.c
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
// 11 Aug 2000   John Adcock           Moved Tuner Functions in here
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "i2c.h"
#include "Tuner.h"

BYTE TunerDeviceWrite, TunerDeviceRead;
char TunerStatus[30] = "No Device on I2C-Bus";

TChannels Channels;
TCountries Countries[35];

const char* Tuner_Status()
{
	return TunerStatus;
}

BOOL Tuner_Init()
{
	unsigned char j;

	if (GetTunerSetup() == NULL)
		return (TRUE);

	j = 0xc0;
	TunerDeviceRead = j;
	TunerDeviceWrite = j;

	while ((j <= 0xce) && (I2CBus_AddDevice((BYTE) j) == FALSE))
	{
		j++;
		TunerDeviceRead = j;
		TunerDeviceWrite = j;
	}

	if (j > 0xce)
	{
		return (FALSE);
	}
	sprintf(TunerStatus, "Tuner I2C-Bus I/O 0x%02x", j);
	return (TRUE);
}

/*
 *	Set TSA5522 synthesizer frequency
 */
BOOL Tuner_SetFrequency(int wFrequency)
{
	BYTE config;
	WORD div;
	BOOL bAck;

	if (GetTunerSetup() == NULL)
		return (TRUE);

	if (wFrequency < GetTunerSetup()->thresh1)
		config = GetTunerSetup()->VHF_L;
	else if (wFrequency < GetTunerSetup()->thresh2)
		config = GetTunerSetup()->VHF_H;
	else
		config = GetTunerSetup()->UHF;

	div = wFrequency + GetTunerSetup()->IFPCoff;

	div &= 0x7fff;
	I2CBus_Lock();				// Lock/wait
	if (!I2CBus_Write((BYTE) TunerDeviceWrite, (BYTE) ((div >> 8) & 0x7f), (BYTE) (div & 0xff), TRUE))
	{
		Sleep(1);
		if (!I2CBus_Write((BYTE) TunerDeviceWrite, (BYTE) ((div >> 8) & 0x7f), (BYTE) (div & 0xff), TRUE))
		{
			Sleep(1);
			if (!I2CBus_Write((BYTE) TunerDeviceWrite, (BYTE) ((div >> 8) & 0x7f), (BYTE) (div & 0xff), TRUE))
			{
				ErrorBox("Tuner Device : Error Writing (1)");
				I2CBus_Unlock();	// Unlock
				return (FALSE);
			}
		}
	}
	if (!(bAck = I2CBus_Write(TunerDeviceWrite, GetTunerSetup()->config, config, TRUE)))
	{
		Sleep(1);
		if (!(bAck = I2CBus_Write(TunerDeviceWrite, GetTunerSetup()->config, config, TRUE)))
		{
			Sleep(1);
			if (!(bAck = I2CBus_Write(TunerDeviceWrite, GetTunerSetup()->config, config, TRUE)))
			{
				ErrorBox("Tuner Device : Error Writing (2)");
			}
		}
	}
	I2CBus_Unlock();			// Unlock
	if (!bAck)
		return FALSE;
	return TRUE;
}

void Load_Country_Settings()
{
	FILE *iniFile;
	char line[128];
	char *Pos;
	char *Pos1;
	char *eol_ptr;
	unsigned int i;

	for (i = 0; i < 32; i++)
	{
		Countries[i].Name[0] = 0x00;
	}

	if ((iniFile = fopen("Channel.txt", "r")) == NULL)
	{
		ErrorBox("File Channel.txt not Found");
		return;
	}
	i = 0;

	while (fgets(line, sizeof(line), iniFile) != NULL)
	{
		if (i >= 35)
		{
			ErrorBox("File Channel.txt has more than 35 settings!\nThe extra ones are ingnored.");
			fclose(iniFile);
			return;
		}
		eol_ptr = strstr(line, ";");
		if (eol_ptr == NULL)
			eol_ptr = strstr(line, "\n");

		if (((Pos = strstr(line, "[")) != 0) && (eol_ptr > Pos) && ((Pos1 = strstr(line, "]")) != 0))
		{

			Pos++;
			Pos1 = &Countries[i].Name[0];
			i++;
			while (*Pos != ']')
			{
				*(Pos1++) = *(Pos++);
				*Pos1 = 0x00;
			}
		}
	}

	fclose(iniFile);

}

void Load_Country_Specific_Settings(int LPos)
{
	unsigned short i, j, k;
	FILE *iniFile;
	char line[128];
	char txt[128];
	char *Pos;
	char *Pos1;
	char *eol_ptr;

	if ((iniFile = fopen("Channel.txt", "r")) == NULL)
	{
		ErrorBox("File Channel.txt not found");
		return;
	}
	i = 0;
	k = 0;

	while (fgets(line, sizeof(line), iniFile) != NULL)
	{

		eol_ptr = strstr(line, ";");
		if (eol_ptr == NULL)
			eol_ptr = strstr(line, "\n");

		sprintf(txt, "[%s]", Countries[LPos].Name);

		if (strstr(line, txt) != 0)
		{
			strcpy(Channels.Name, Countries[LPos].Name);
			while (fgets(line, sizeof(line), iniFile) != NULL)
			{
				eol_ptr = strstr(line, ";");
				if (eol_ptr == NULL)
					eol_ptr = strstr(line, "\n");

				if (((Pos = strstr(line, "[")) != 0) && (eol_ptr > Pos) && ((Pos1 = strstr(line, "]")) != 0))
				{
					fclose(iniFile);
					return;
				}

				if (((Pos = strstr(line, "ChannelLow=")) != 0) && (eol_ptr > Pos))
				{
					Pos = Pos + 11;
					j = 0;
					txt[j] = 0x00;
					while (Pos < eol_ptr)
					{
						if (*Pos != 0x20)
						{
							txt[j] = *Pos;
							j++;
							txt[j] = 0x00;
						}
						Pos++;
					}
					Channels.MinChannel = atoi(txt);
				}
				else if (((Pos = strstr(line, "ChannelHigh=")) != 0) && (eol_ptr > Pos))
				{
					Pos = Pos + 12;
					j = 0;
					txt[j] = 0x00;
					while (Pos < eol_ptr)
					{
						if (*Pos != 0x20)
						{
							txt[j] = *Pos;
							j++;
							txt[j] = 0x00;
						}
						Pos++;
					}
					Channels.MaxChannel = atoi(txt);
				}
				else
				{
					Pos = &line[0];
					j = 0;
					txt[j] = 0x00;
					while (Pos < eol_ptr)
					{
						if ((*Pos >= '0') && (*Pos <= '9'))
						{
							txt[j] = *Pos;
							j++;
							txt[j] = 0x00;
						}
						Pos++;
					}
					if (txt[0] != 0x00)
					{
						Channels.freq[k] = atol(txt);
						Channels.freq[k] = Channels.freq[k] / 1000;
						k++;
					}
				}
			}
		}
	}
	fclose(iniFile);
	return;
}
