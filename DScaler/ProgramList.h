/////////////////////////////////////////////////////////////////////////////
// ProgramList.h
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
// 11 Mar 2001   Laurent Garnier       Previous Channel feature added
//
// 06 Apr 2001   Laurent Garnier       New menu to select channel
//
// 26 May 2001   Eric Schmidt          Added Custom Channel Order.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __PROGRAMLIST_H___
#define __PROGRAMLIST_H___

#define MAXPROGS 255

typedef struct
{
	char FilterName[5];
	unsigned char FilterId;
	unsigned short PID;
} TPIDFilters;

typedef struct
{
	char Name[255];
    unsigned long freq;
    unsigned long chan;
    char Typ;
    BOOL  Tuner_Auto;
	BOOL  PMT_Auto;
	BOOL  PID_Auto;
    int power;             /* LNB power 0=off/pass through, 1=on */
	int volt;              /* 14/18V (V=0/H=1) */
	int afc;
	int ttk;               /* 22KHz */
	int diseqc;            /* Diseqc input select */
	unsigned int srate;
	int qam;
	int fec;
	int norm;
	unsigned short  tp_id;
	unsigned short  Video_pid;
	unsigned short  Audio_pid;
    unsigned short  TeleText_pid;          /* Teletext PID */
	unsigned short  PMT_pid;
    unsigned short  PCR_pid;
	unsigned short  PMC_pid;
	unsigned short  SID_pid;
	unsigned short  AC3_pid;
	unsigned short  EMM_pid;
	unsigned short  ECM_pid;
	unsigned char   TVType; //  == 00 PAL ; 11 == NTSC
	unsigned char   ServiceTyp;
    unsigned char   CA_ID;
	unsigned short  Temp_Audio;
    unsigned char   Buffer[10];   // For later Use
	unsigned short  FilterNumber;
    TPIDFilters Filters[12];
} TProgramm;


void Write_Program_List();
void Write_Program_List_ASCII();
void Load_Program_List();
void Load_Program_List_ASCII();

BOOL APIENTRY ProgramListProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY ChannelNumberProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY AnalogScanProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
void ChangeChannel(int NewChannel);
void Channels_UpdateMenu(HMENU hMenu);
void Channels_SetMenu(HMENU hMenu);

extern TProgramm Programm[MAXPROGS+1];
extern int CountryCode;
extern long CurrentProgramm;
extern long PreviousProgramm;
extern BOOL bCustomChannelOrder;

#endif
