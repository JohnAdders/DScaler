/////////////////////////////////////////////////////////////////////////////
// ProgramList.c
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
// 26 Dec 2000   Eric Schmidt          Made it possible to have whitespace in
//                                     your channel names in program.txt.
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

#include "stdafx.h"
#include "resource.h"
#include "ProgramList.h"
#include "Tuner.h"
#include "BT848.h"
#include "DScaler.h"
#include "VBI.h"
#include "Status.h"
#include "Audio.h"
#include "VBI_VideoText.h"
#include "TVCards.h"
#include "MixerDev.h"
#include "OSD.h"

int CurSel;
unsigned short SelectButton;
int EditProgramm;
char KeyValue;
HWND ProgList;

typedef vector<CChannel*> CHANNELLIST;
typedef vector<CCountry*> COUNTRYLIST;

CHANNELLIST MyChannels;
COUNTRYLIST Countries;

int CountryCode = 1;

long CurrentProgramm = 0;
long PreviousProgramm = 0;
BOOL bCustomChannelOrder = FALSE;


CChannel::CChannel(LPCSTR Name, DWORD Freq, int ChannelNumber, int Format)
{
	m_Name = Name;
    m_Freq = Freq;
    m_Chan = ChannelNumber;
    m_Format = Format;
}

CChannel::~CChannel()
{
}

LPCSTR CChannel::GetName()
{
    return m_Name.c_str();
}

DWORD CChannel::GetFrequency()
{
    return m_Freq;
}

int CChannel::GetChannelNumber()
{
    return m_Chan;
}

int CChannel::GetFormat()
{
    return m_Format;
}

CCountry::CCountry()
{
    m_Name = "";
    m_MinChannel = 0;
	m_MaxChannel = 0;
	m_Frequencies.clear();
}

CCountry::~CCountry()
{
	m_Frequencies.clear();
}

void Channel_SetCurrent()
{
    Channel_Change(CurrentProgramm);
}

const char* Channel_GetName()
{
    return MyChannels[CurrentProgramm]->GetName();
}


BOOL APIENTRY ProgramListProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	int i;
    CHANNELLIST::iterator it;
    char sbuf[256];
    static int EditChan;

	switch (message)
	{
	case WM_INITDIALOG:
		SetCapture(hDlg);
		SendMessage(GetDlgItem(hDlg, IDC_PROGRAMLIST), LB_RESETCONTENT, 0, 0);

		i = 0;

        for(it = MyChannels.begin(); it != MyChannels.end(); ++it)
        {
			SendMessage(GetDlgItem(hDlg, IDC_PROGRAMLIST), LB_ADDSTRING, 0, (LPARAM) (*it)->GetName());
        }

		SendMessage(GetDlgItem(hDlg, IDC_PROGRAMLIST), LB_SETCURSEL, CurrentProgramm, (LPARAM) 0);

		SetFocus(GetDlgItem(hDlg, IDC_PROGRAMLIST)); 

		SendMessage(GetDlgItem(hDlg, IDC_FORMAT), CB_ADDSTRING, 0, (LPARAM) "Same as Tuner");
		SendMessage(GetDlgItem(hDlg, IDC_FORMAT), CB_ADDSTRING, 0, (LPARAM) "PAL");
		SendMessage(GetDlgItem(hDlg, IDC_FORMAT), CB_ADDSTRING, 0, (LPARAM) "NTSC");
		SendMessage(GetDlgItem(hDlg, IDC_FORMAT), CB_ADDSTRING, 0, (LPARAM) "PAL-M");
		SendMessage(GetDlgItem(hDlg, IDC_FORMAT), CB_ADDSTRING, 0, (LPARAM) "PAL-N");
		SendMessage(GetDlgItem(hDlg, IDC_FORMAT), CB_ADDSTRING, 0, (LPARAM) "NTSC-J");
		SendMessage(GetDlgItem(hDlg, IDC_FORMAT), CB_ADDSTRING, 0, (LPARAM) "PAL60");
        EditChan = CurrentProgramm;
        SendMessage(GetDlgItem(hDlg, IDC_NAME), WM_SETTEXT, 0, (LPARAM)MyChannels[EditChan]->GetName());
        sprintf(sbuf, "%d", MyChannels[EditChan]->GetFrequency());
        SendMessage(GetDlgItem(hDlg, IDC_FREQUENCY), WM_SETTEXT, 0, (LPARAM)sbuf);
        sprintf(sbuf, "%d", MyChannels[EditChan]->GetChannelNumber());
        SendMessage(GetDlgItem(hDlg, IDC_CHANNEL), WM_SETTEXT, 0, (LPARAM)sbuf);
        MyChannels[EditChan]->GetChannelNumber();
        SendMessage(GetDlgItem(hDlg, IDC_FORMAT), CB_SETCURSEL, MyChannels[EditChan]->GetFormat() + 1, 0);
		break;

	case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_PROGRAMLIST:
			if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				i = SendMessage(GetDlgItem(hDlg, IDC_PROGRAMLIST), LB_GETCURSEL, 0, 0);

				if ((i >= 0) && (i < MyChannels.size()))
				{
					if (i != CurrentProgramm)
					{
						Channel_Change(i);
					}
                    EditChan = i;
                    SendMessage(GetDlgItem(hDlg, IDC_NAME), WM_SETTEXT, 0, (LPARAM)MyChannels[EditChan]->GetName());
                    sprintf(sbuf, "%d", MyChannels[EditChan]->GetFrequency());
                    SendMessage(GetDlgItem(hDlg, IDC_FREQUENCY), WM_SETTEXT, 0, (LPARAM)sbuf);
                    sprintf(sbuf, "%d", MyChannels[EditChan]->GetChannelNumber());
                    SendMessage(GetDlgItem(hDlg, IDC_CHANNEL), WM_SETTEXT, 0, (LPARAM)sbuf);
                    MyChannels[EditChan]->GetChannelNumber();
                    SendMessage(GetDlgItem(hDlg, IDC_FORMAT), CB_SETCURSEL, MyChannels[EditChan]->GetFormat() + 1, 0);
				}
			}
            break;
        case IDC_CHANGE:
            {
                int Format;
                DWORD Freq;
                int Channel;
                SendMessage(GetDlgItem(hDlg, IDC_FREQUENCY), WM_GETTEXT, 255, (LPARAM)sbuf);
                Freq = atol(sbuf);
                SendMessage(GetDlgItem(hDlg, IDC_CHANNEL), WM_GETTEXT, 255, (LPARAM)sbuf);
                Channel = atoi(sbuf);
                Format = SendMessage(GetDlgItem(hDlg, IDC_FORMAT), CB_GETCURSEL, 0, 0) - 1;
                delete MyChannels[EditChan];
                SendMessage(GetDlgItem(hDlg, IDC_NAME), WM_GETTEXT, 255, (LPARAM)sbuf);
                MyChannels[EditChan] = new CChannel(sbuf, Freq, Channel, Format);
            }
            break;
        case IDOK:
        case IDCANCEL:
			EndDialog(hDlg, TRUE);
            break;
        }

		break;
	}
	return (FALSE);
}

// 
// Save ascii formatted program list
//
// 9 Novemeber 2000 - Michael Eskin, Conexant Systems
//
// List is a simple text file with the following format:
// Name <display_name>
// Freq <frequency_KHz>
// Name <display_name>
// Freq <frequency_KHz>
// ...
//
void Write_Program_List_ASCII()
{
	FILE *SettingFile;
    CHANNELLIST::iterator it;
	
	if ((SettingFile = fopen("program.txt", "w")) != NULL)
	{
        for(it = MyChannels.begin(); it != MyChannels.end(); ++it)
        {
			fprintf(SettingFile, "Name: %s\n", (*it)->GetName());
			fprintf(SettingFile, "Freq: %ld\n", (*it)->GetFrequency());
			fprintf(SettingFile, "Chan: %d\n", (*it)->GetChannelNumber());
            if((*it)->GetFormat() != -1)
            {
			    fprintf(SettingFile, "Form: %d\n", (*it)->GetFormat());
            }
		}
		fclose(SettingFile);
	}
}

// 
// Load ascii formatted program list
//
// 9 Novemeber 2000 - Michael Eskin, Conexant Systems
//
// List is a simple text file with the following format:
// Name <display_name>
// Freq <frequency_KHz>
// Name <display_name>
// Freq <frequency_KHz>
// ...
//

void Load_Program_List_ASCII()
{
	char sbuf[256];
	FILE *SettingFile;
    CHANNELLIST::iterator it;
    DWORD Frequency = -1;
    int Channel = 1;
    int Format = -1;
    string Name;

	// Zero out the program list
    for(it = MyChannels.begin(); it != MyChannels.end(); ++it)
    {
        delete (*it);
    }
    MyChannels.clear();

    SettingFile = fopen("program.txt", "r");
	if (SettingFile == NULL)
    {
        return;
    }
	while(!feof(SettingFile))
    {
        if(fgets(sbuf, 255, SettingFile) == NULL)
        {
    	    fclose(SettingFile);
            return;
        }
		char* eol_ptr = strstr(sbuf, ";");
		if (eol_ptr == NULL)
        {
			eol_ptr = strstr(sbuf, "\n");
        }
		if (eol_ptr != NULL)
        {
            *eol_ptr = '\0';
        }


        if(strnicmp(sbuf, "Name:", 5) == 0)
        {
            if(Frequency != -1)
            {
                MyChannels.push_back(new CChannel(Name.c_str(), Frequency, Channel, Format));
            }
            Name = sbuf + 5;
            Frequency = -1;
            ++Channel;
            Format = -1;
        }
        else if(strnicmp(sbuf, "Freq:", 5) == 0)
        {
            Frequency = atol(sbuf + 5);
        }
        else if(strnicmp(sbuf, "Chan:", 5) == 0)
        {
            Channel = atoi(sbuf + 5);
        }
        else if(strnicmp(sbuf, "Form:", 5) == 0)
        {
            Format = atoi(sbuf + 5);
        }
        else
        {
            ; //some other rubbish
        }
    }

    if(Frequency != -1)
    {
        MyChannels.push_back(new CChannel(Name.c_str(), Frequency, Channel, Format));
    }

	fclose(SettingFile);
	return;
}


BOOL APIENTRY AnalogScanProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	char line[80];
	char Text[128];
	static HBITMAP RedBulb;
	static HBITMAP GreenBulb;

	int i;
	
	static int progindex = 0;

	PAINTSTRUCT wps;			/* paint structure           */
	HDC hdc;
	HDC hMemDC;
	HBITMAP hOldBm;
	BITMAP bm;

	static BOOL STOP;
	static unsigned int Freq;
	static int ChannelNr;
	static unsigned int FirstFreq = 0;
    COUNTRYLIST::iterator it;
    CHANNELLIST::iterator ChanIt;
    string Name;

	MSG msg;

	switch (message)
	{
	case WM_INITDIALOG:
    	Load_Country_Settings();
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_RESETCONTENT, 0, 0);
        i = 0;
        for(it = Countries.begin(); it != Countries.end(); ++it)
        {
			SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_INSERTSTRING, i, (LPARAM)(LPSTR)((*it)->m_Name.c_str()));
            i++;
		}
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_SETCURSEL, CountryCode, 0);

		RedBulb = LoadBitmap(hInst, "REDBULB");
		GreenBulb = LoadBitmap(hInst, "GREENBULB");

		return (TRUE);

	case WM_PAINT:
		hdc = BeginPaint(hDlg, &wps);
		hMemDC = CreateCompatibleDC(hdc);
		
		if(BT848_IsVideoPresent() == TRUE)
			hOldBm = (HBITMAP)SelectObject(hMemDC, GreenBulb);
		else
			hOldBm = (HBITMAP)SelectObject(hMemDC, RedBulb);

		GetObject(RedBulb, sizeof(BITMAP), (LPSTR) & bm);

		BitBlt(hdc, 170, 80, bm.bmWidth, bm.bmHeight, hMemDC, 0, 0, SRCCOPY);	// Signal

		SelectObject(hMemDC, hOldBm);
		DeleteDC(hMemDC);
		DeleteDC(hdc);
		EndPaint(hDlg, &wps);
		return (FALSE);

	case WM_USER:
		if (wParam == 0x101)
		{
			if(Freq != 0)
			{
				sprintf(line, "%10.2f", (float) Freq / 1000);
				SetDlgItemText(hDlg, IDC_EDIT2, line);
				SetDlgItemText(hDlg, IDC_TEXT20, "Scanning...");

				i = 0;

				Sleep(100);

				while ((i < 75) && (BT848_IsVideoPresent() == FALSE))
				{
					i++;
					if (PeekMessage(&msg, NULL, 0, 0xffffffff, PM_REMOVE) == TRUE)
					{
						SendMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
					}

					if (STOP == TRUE)
						return (TRUE);

					Sleep(3);
				}

				if (BT848_IsVideoPresent())
				{
					if (FirstFreq == 0)
						FirstFreq = Freq;

					SetDlgItemText(hDlg, IDC_TEXT20, "Video Signal Found");
					
					VPS_lastname[0] = 0x00;
					
					VT_ResetStation();

					Freq = Freq + 2500;
					if (Capture_VBI == TRUE)
					{
						Name =  "<No PDC>";

						i = 0;
						while (i < 100)
						{
							if (PeekMessage(&msg, NULL, 0, 0xffffffff, PM_REMOVE) == TRUE)
							{
								SendMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
							}
							Sleep(2);
							if (VPS_lastname[0] != 0x00)
							{
								Name =  VPS_lastname;
								i = 100;
							}
							else if (VT_GetStation()[0] != '\0')
							{
								Name =  VT_GetStation();
								i = 100;
							}
							i++;
						}
					}
					// MAE 7 Nov 2000 Added to get right channel names
					else
					{
						// MAE 7 Nov 2000 Added to get right channel names
						sprintf(Text,"%d",Countries[CountryCode]->m_MinChannel+ChannelNr);

						Name =  Text;
					}
					
					progindex++;

					if (progindex > MAXPROGS)
					{
						ErrorBox("All storage space occupied");
						return (TRUE);
					}
                    else
                    {
                        MyChannels.push_back(new CChannel(Name.c_str(), Freq, Countries[CountryCode]->m_MinChannel + ChannelNr, -1));
                    }
				
					InvalidateRect(hDlg, NULL, FALSE);
					UpdateWindow(hDlg);

				}
			}

			ChannelNr++;

			if (STOP == TRUE)
				return (TRUE);

			if (ChannelNr <= (Countries[CountryCode]->m_MaxChannel - Countries[CountryCode]->m_MinChannel))
			{
				Freq = Countries[CountryCode]->m_Frequencies[ChannelNr];
				if (!Tuner_SetFrequency(MulDiv((Freq * 1000), 16, 1000000)))
				{
					sprintf(Text, "SetFrequency %10.2f Failed.", (float) Freq / 1000);
					ErrorBox(Text);
					return (TRUE);
				}

				// MAE 7 Nov 2000 Added to get right channel names
				sprintf(Text,"%d",Countries[CountryCode]->m_MinChannel+ChannelNr);
				SetDlgItemText(hDlg, IDC_EDIT15, Text);
				
				InvalidateRect(hDlg, NULL, FALSE);
				UpdateWindow(hDlg);

				if (STOP == FALSE)
				{
					PostMessage(hDlg, WM_USER, 0x101, 0);
				}
			}
			else
			{
				if (FirstFreq != 0)
				{
					(void) Tuner_SetFrequency(MulDiv((MyChannels[0]->GetFrequency() * 1000), 16, 1000000));
					SetDlgItemText(hDlg, IDC_EDIT15, MyChannels[0]->GetName());
                    CurrentProgramm = 0;
					
					InvalidateRect(hDlg, NULL, FALSE);
					UpdateWindow(hDlg);

					EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
				}
			}	
		}
		break;

	case WM_COMMAND:

		if ((HWND) lParam == GetDlgItem(hDlg, IDC_COMBO1))
		{
			if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				CountryCode = SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0);
			}
		}

		if (LOWORD(wParam) == IDSTART)
		{

			Freq = Countries[CountryCode]->m_Frequencies[0];
			ChannelNr = 0;

			// Zero out the program list
            for(ChanIt = MyChannels.begin(); ChanIt != MyChannels.end(); ++ChanIt)
            {
                delete (*ChanIt);
            }
            MyChannels.clear();

			progindex = 0;


			STOP = FALSE;
			sprintf(line, "%10.2f", (float) Freq / 1000);
			SetDlgItemText(hDlg, IDC_EDIT2, line);

			if (!Tuner_SetFrequency(MulDiv((Freq * 1000), 16, 1000000)))
			{
				sprintf(Text, "Set Frequency %10.2f Mhz fialed", (float) Freq / 1000);
				StatusBar_ShowText(STATUS_TEXT, Text);
			}

			// MAE 7 Nov 2000 Added to get right channel names
			sprintf(Text,"%d",Countries[CountryCode]->m_MinChannel+ChannelNr);
			SetDlgItemText(hDlg, IDC_EDIT15, Text);
			
			InvalidateRect(hDlg, NULL, FALSE);
			UpdateWindow(hDlg);

			PostMessage(hDlg, WM_USER, 0x101, 0);
			EnableWindow(GetDlgItem(hDlg, IDSTART), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);

		}

		if (LOWORD(wParam) == IDOK)
		{
			Write_Program_List_ASCII();
			Channels_UpdateMenu(GetMenu(hWnd));
			DeleteObject(RedBulb);
			DeleteObject(GreenBulb);
            Unload_Country_Settings();
			EndDialog(hDlg, TRUE);
		}

		if (LOWORD(wParam) == IDCANCEL)
		{
			STOP = TRUE;
			Load_Program_List_ASCII();
			DeleteObject(RedBulb);
			DeleteObject(GreenBulb);
            Unload_Country_Settings();
			EndDialog(hDlg, TRUE);
		}
		break;
	}
	return (FALSE);
	UNREFERENCED_PARAMETER(lParam);
}

//---------------------------------------------------------------------------
void Channel_Change(int NewChannel)
{
	if (GetTunerSetup() != NULL)
	{
		if(NewChannel >= 0 && NewChannel < MAXPROGS)
		{
			if (MyChannels[NewChannel]->GetFrequency() != 0)
			{
				if(!bSystemInMute)
				{
					Audio_SetSource(AUDIOMUX_MUTE);
					Mixer_Mute();
				}
				PreviousProgramm = CurrentProgramm;
				CurrentProgramm = NewChannel;
				Tuner_SetFrequency(MulDiv(MyChannels[CurrentProgramm]->GetFrequency() * 1000, 16, 1000000));
                if(MyChannels[CurrentProgramm]->GetFormat() != -1)
                {
        			Setting_SetValue(BT848_GetSetting(TVFORMAT), MyChannels[CurrentProgramm]->GetFormat());
                }
				Sleep(20);
				VT_ChannelChange();
				if(!bSystemInMute)
				{
					Audio_SetSource(AudioSource);
					Mixer_UnMute();
				}
				StatusBar_ShowText(STATUS_KEY, MyChannels[CurrentProgramm]->GetName());
				OSD_ShowText(hWnd,MyChannels[CurrentProgramm]->GetName(), 0);
			}
		}
	}
}

void Channels_UpdateMenu(HMENU hMenu)
{
	HMENU			hMenuChannels;
	MENUITEMINFO	MenuItemInfo;
	int				j;
    CHANNELLIST::iterator it;
	hMenuChannels = GetChannelsSubmenu();
	if(hMenuChannels == NULL) return;

	j = GetMenuItemCount(hMenuChannels) - 1;
	while (j>=2)
	{
		RemoveMenu(hMenuChannels, j, MF_BYPOSITION);
		--j;
	}
    
    j = 2;
	for (it = MyChannels.begin(); it != MyChannels.end(); ++it)
	{
		if ((*it)->GetFrequency() != 0)
		{
			MenuItemInfo.cbSize = sizeof (MenuItemInfo);
			MenuItemInfo.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
			MenuItemInfo.fType = MFT_STRING;
			MenuItemInfo.dwTypeData = (LPSTR) (*it)->GetName();
			MenuItemInfo.cch = strlen ((*it)->GetName());
			MenuItemInfo.fState = (CurrentProgramm == j - 2) ? MFS_CHECKED : MFS_ENABLED;
			MenuItemInfo.wID = IDM_CHANNEL_SELECT + j - 1;
			InsertMenuItem(hMenuChannels, j, TRUE, &MenuItemInfo);
			j++;
		}
	}
}

void Channel_Increment()
{
    // MAE 8 Nov 2000 Added wrap around
    if (CurrentProgramm + 1 < MyChannels.size())
    {
	    Channel_Change(CurrentProgramm + 1);
    }
    else
    {
	    Channel_Change(0);
    }
	StatusBar_ShowText(STATUS_KEY, MyChannels[CurrentProgramm]->GetName());
	OSD_ShowText(hWnd,MyChannels[CurrentProgramm]->GetName(), 0);
}

void Channel_Decrement()
{
	// MAE 8 Nov 2000 Added wrap around
	if (CurrentProgramm != 0)
	{
		Channel_Change(CurrentProgramm - 1);
	}
	else
	{
		Channel_Change(MyChannels.size() - 1);
	}
	StatusBar_ShowText(STATUS_KEY, MyChannels[CurrentProgramm]->GetName());
	OSD_ShowText(hWnd,MyChannels[CurrentProgramm]->GetName(), 0);
}

void Channel_Previous()
{
	if (MyChannels[PreviousProgramm]->GetFrequency() != 0)
		Channel_Change(PreviousProgramm);

	StatusBar_ShowText(STATUS_KEY, MyChannels[CurrentProgramm]->GetName());
	OSD_ShowText(hWnd,MyChannels[CurrentProgramm]->GetName(), 0);
}

void Channel_ChangeToNumber(int ChannelNumber)
{
    BOOL found = FALSE;

    if (bCustomChannelOrder)
    {
        // Find the channel the user typed.
        for (int j = 0; j < MAXPROGS; ++j)
        {
            if (MyChannels[j]->GetFrequency() != 0 && int(MyChannels[j]->GetChannelNumber()) == ChannelNumber)
            {
                found = TRUE;
                ChannelNumber = j;
                break;
            }
        }
    }
    else
    {
        found = TRUE;
        ChannelNumber = ChannelNumber - 1;
    }

    if (found)
    {
        Channel_Change(ChannelNumber);
        found = CurrentProgramm == ChannelNumber;
    }

	if (found)
	{
		StatusBar_ShowText(STATUS_KEY, MyChannels[CurrentProgramm]->GetName());
		OSD_ShowText(hWnd, MyChannels[CurrentProgramm]->GetName(), 0);
	}
	else
	{
		StatusBar_ShowText(STATUS_KEY, "Not Found");
		OSD_ShowText(hWnd, "Not Found", 0);
	}
}

void Unload_Country_Settings()
{
    COUNTRYLIST::iterator it;

    // Zero out the program list
    for(it = Countries.begin(); it != Countries.end(); ++it)
    {
        delete (*it);
    }
    Countries.clear();
}


void Load_Country_Settings()
{
	FILE *CountryFile;
	char line[128];
	char *Pos;
	char *Pos1;
	char *eol_ptr;
	unsigned int i;
    string Name;
    CCountry* NewCountry = NULL;

    if ((CountryFile = fopen("Channel.txt", "r")) == NULL)
	{
		ErrorBox("File Channel.txt not Found");
		return;
	}
	i = 0;

	while (fgets(line, sizeof(line), CountryFile) != NULL)
	{
		eol_ptr = strstr(line, ";");
		if (eol_ptr == NULL)
        {
			eol_ptr = strstr(line, "\n");
        }
        if(eol_ptr != NULL)
        {
            *eol_ptr = '\0';
        }

		if(((Pos = strstr(line, "[")) != 0) && ((Pos1 = strstr(line, "]")) != 0) && Pos1 > Pos)
		{
            if(NewCountry != NULL)
            {
                Countries.push_back(NewCountry);
            }
			Pos++;
            NewCountry = new CCountry();
            NewCountry->m_Name = Pos;
            NewCountry->m_Name[Pos1-Pos] = '\0';
		}
        else if ((Pos = strstr(line, "ChannelLow=")) != 0)
        {
            NewCountry->m_MinChannel = atoi(Pos + strlen("ChannelLow="));
        }
        else if ((Pos = strstr(line, "ChannelHigh=")) != 0)
        {
            NewCountry->m_MaxChannel = atoi(Pos + strlen("ChannelHigh="));
        }
        else
        {
			Pos = line;
			while (*Pos != '\0')
			{
				if ((*Pos >= '0') && (*Pos <= '9'))
				{
                    NewCountry->m_Frequencies.push_back(atol(Pos));
                    break;
				}
				Pos++;
			}
        }
	}
    if(NewCountry != NULL)
    {
        Countries.push_back(NewCountry);
    }

	fclose(CountryFile);
}

void Channels_SetMenu(HMENU hMenu)
{
	HMENU	hMenuChannels;
	int		i;
	int		nb;

	hMenuChannels = GetChannelsSubmenu();
	if(hMenuChannels == NULL) return;

	nb = GetMenuItemCount(hMenuChannels) - 2;
	i = nb + 1;
	while (i>=2)
	{
		CheckMenuItem(hMenuChannels, i, MF_BYPOSITION | MF_UNCHECKED);
		i--;
	}
	if (CurrentProgramm < nb)
	{
		CheckMenuItem(hMenuChannels, CurrentProgramm + 2, MF_BYPOSITION | MF_CHECKED);
	}
    CheckMenuItem(hMenu, IDM_CHANNEL_CUSTOMORDER, bCustomChannelOrder ? MF_CHECKED : MF_UNCHECKED);
}
////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING ChannelsSettings[CHANNELS_SETTING_LASTONE] =
{
	{
		"CountryCode", SLIDER, 0, (long*)&CountryCode,
		1, 0, 100, 1, 1,
		NULL,
		"Show", "CountryCode", NULL,
	},
	{
		"Current Program", SLIDER, 0, (long*)&CurrentProgramm,
		0, 0, MAXPROGS, 1, 1,
		NULL,
		"Show", "LastProgram", NULL,
	},
	{
		"Custom Channel Order", ONOFF, 0, (long*)&bCustomChannelOrder,
		FALSE, 0, 1, 1, 1,
		NULL,
		"Show", "CustomChannelOrder", NULL,
	},
};

SETTING* Channels_GetSetting(CHANNELS_SETTING Setting)
{
	if(Setting > -1 && Setting < CHANNELS_SETTING_LASTONE)
	{
		return &(ChannelsSettings[Setting]);
	}
	else
	{
		return NULL;
	}
}

void Channels_ReadSettingsFromIni()
{
	int i;
	for(i = 0; i < CHANNELS_SETTING_LASTONE; i++)
	{
		Setting_ReadFromIni(&(ChannelsSettings[i]));
	}
}

void Channels_WriteSettingsToIni()
{
	int i;
	for(i = 0; i < CHANNELS_SETTING_LASTONE; i++)
	{
		Setting_WriteToIni(&(ChannelsSettings[i]));
	}
}
