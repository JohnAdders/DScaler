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
int EditChan = -1;
BOOL InScan = FALSE;

CChannel::CChannel(LPCSTR Name, DWORD Freq, int ChannelNumber, int Format, BOOL Active)
{
	m_Name = Name;
    m_Freq = Freq;
    m_Chan = ChannelNumber;
    m_Format = Format;
    m_Active = Active;
}

CChannel::CChannel(const CChannel& CopyFrom)
{
	m_Name = CopyFrom.m_Name;
    m_Freq = CopyFrom.m_Freq;
    m_Chan = CopyFrom.m_Chan;
    m_Format = CopyFrom.m_Format;
    m_Active = CopyFrom.m_Active;
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

BOOL CChannel::IsActive()
{
    return m_Active;
}

void CChannel::SetActive(BOOL Active)
{
    m_Active = Active;
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
	if(CurrentProgramm < MyChannels.size())
	{
		return MyChannels[CurrentProgramm]->GetName();
	}
	else
	{
		return "Unknown";
	}
}

void SelectChannel(HWND hDlg, long ChannelToSelect)
{
    // then loop through looking for the correct channel
    for(int i(0); i < Countries[CountryCode]->m_Frequencies.size() + 1; ++i)
    {
        int Channel = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CHANNEL), i);
        if(Channel == ChannelToSelect)
        {
        	ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_CHANNEL), i);
        }
    }

}

void UpdateDetails(HWND hDlg)
{
	if(EditChan < MyChannels.size())
	{
        char sbuf[256];

        // set the name		
        Edit_SetText(GetDlgItem(hDlg, IDC_NAME), MyChannels[EditChan]->GetName());

        // set the frequency
		sprintf(sbuf, "%10.4f MHz", (double)MyChannels[EditChan]->GetFrequency() / 16.0);
		Edit_SetText(GetDlgItem(hDlg, IDC_FREQUENCY),sbuf);

        // set the channel
        // select none to start off with
        ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_FORMAT), 0);

        SelectChannel(hDlg, MyChannels[EditChan]->GetChannelNumber());
        
        // set format
		ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_FORMAT), MyChannels[EditChan]->GetFormat() + 1);

		// set active
		if(MyChannels[EditChan]->IsActive())
		{
			Button_SetCheck(GetDlgItem(hDlg, IDC_ACTIVE), BST_CHECKED);
		}
		else
		{
			Button_SetCheck(GetDlgItem(hDlg, IDC_ACTIVE), BST_UNCHECKED);
		}

	}
}

void ResetProgramList(HWND hDlg)
{
    CHANNELLIST::iterator it;
	ListBox_ResetContent(GetDlgItem(hDlg, IDC_PROGRAMLIST));
    CurrentProgramm = 0;
    for(it = MyChannels.begin(); it != MyChannels.end(); ++it)
    {
		delete *it;
    }
    MyChannels.clear();
    if(bCustomChannelOrder)
    {
        for(int i(0); i < Countries[CountryCode]->m_Frequencies.size(); ++i)
        {
            if(Countries[CountryCode]->m_Frequencies[i] != 0)
            {
                char sbuf[256];
                sprintf(sbuf, "%d", Countries[CountryCode]->m_MinChannel + i);
                MyChannels.push_back(new CChannel(sbuf, 
                                        Countries[CountryCode]->m_Frequencies[i],
                                        Countries[CountryCode]->m_MinChannel + i,
                                        -1,
                                        TRUE));
				ListBox_AddString(GetDlgItem(hDlg, IDC_PROGRAMLIST), sbuf);
            }
        }
        EditChan = 0;
    	ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), EditChan);
        UpdateDetails(hDlg);
    }
}

void RefreshProgramList(HWND hDlg, long ProgToSelect)
{
    CHANNELLIST::iterator it;

	ListBox_ResetContent(GetDlgItem(hDlg, IDC_PROGRAMLIST));

    for(it = MyChannels.begin(); it != MyChannels.end(); ++it)
    {
		ListBox_AddString(GetDlgItem(hDlg, IDC_PROGRAMLIST), (*it)->GetName());
    }

	ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), ProgToSelect);
}

void RefreshChannelList(HWND hDlg)
{
	ComboBox_ResetContent(GetDlgItem(hDlg, IDC_CHANNEL));
	int Index = ComboBox_AddString(GetDlgItem(hDlg, IDC_CHANNEL), "None");
    SendMessage(GetDlgItem(hDlg, IDC_CHANNEL), CB_SETITEMDATA, Index, 0);
    for(int i(0); i < Countries[CountryCode]->m_Frequencies.size(); ++i)
    {
        if(Countries[CountryCode]->m_Frequencies[i] != 0)
        {
            char sbuf[256];
            sprintf(sbuf, "%d", Countries[CountryCode]->m_MinChannel + i);
	    	Index = ComboBox_AddString(GetDlgItem(hDlg, IDC_CHANNEL), sbuf);
            ComboBox_SetItemData(GetDlgItem(hDlg, IDC_CHANNEL), Index, Countries[CountryCode]->m_MinChannel + i);
        }
    }
}

void ScanCustomChannel(HWND hDlg, int ChannelNum)
{
    char sbuf[256];

    if(ChannelNum < 0 || ChannelNum >= MyChannels.size())
    {
        return;
    }
    int i = 0;

	MyChannels[ChannelNum]->SetActive(FALSE);

    EditChan = ChannelNum;
	UpdateDetails(hDlg);
	ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), ChannelNum);

    DWORD Freq = MyChannels[ChannelNum]->GetFrequency();

    if (!Tuner_SetFrequency(Freq))
	{
		sprintf(sbuf, "SetFrequency %10.2f Failed.", (float) Freq / 16.0);
		ErrorBox(sbuf);
		return;
	}

	Sleep(100);

	while ((i < 75) && (BT848_IsVideoPresent() == FALSE))
	{
       	MSG msg;
 		if (PeekMessage(&msg, NULL, 0, 0xffffffff, PM_REMOVE) == TRUE)
 		{
 			SendMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
 		}
 
		i++;
		Sleep(3);
	}

	MyChannels[ChannelNum]->SetActive(BT848_IsVideoPresent());
}

void ScanFrequency(HWND hDlg, int FreqNum)
{
    if(FreqNum < 0 || FreqNum >= Countries[CountryCode]->m_Frequencies.size())
    {
        return;
    }

    int i = 0;
    char sbuf[256];

    SelectChannel(hDlg, FreqNum);
	DWORD Freq = Countries[CountryCode]->m_Frequencies[FreqNum];
	sprintf(sbuf, "%10.4f MHz", (double)Freq / 16.0);
    Edit_SetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf);

	if (!Tuner_SetFrequency(Freq))
	{
		sprintf(sbuf, "SetFrequency %10.2f Failed.", (float) Freq / 16.0);
		ErrorBox(sbuf);
		return;
	}

    Sleep(100);

	while ((i < 75) && (BT848_IsVideoPresent() == FALSE))
	{
       	MSG msg;
 		if (PeekMessage(&msg, NULL, 0, 0xffffffff, PM_REMOVE) == TRUE)
 		{
 			SendMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
 		}
		i++;
		Sleep(3);
	}
    
    if(BT848_IsVideoPresent())
    {
        char sbuf[256];
        ++EditChan;
        sprintf(sbuf, "Channel %d", EditChan);
    	MyChannels.push_back(new CChannel(sbuf, Freq, EditChan, -1, TRUE));
    }
}

void RefreshControls(HWND hDlg)
{
    Button_Enable(GetDlgItem(hDlg, IDC_ADD), (bCustomChannelOrder == FALSE));
    Button_Enable(GetDlgItem(hDlg, IDC_REMOVE), (bCustomChannelOrder == FALSE));
    Button_Enable(GetDlgItem(hDlg, IDC_UP), (bCustomChannelOrder == FALSE));
    Button_Enable(GetDlgItem(hDlg, IDC_DOWN), (bCustomChannelOrder == FALSE));
    ComboBox_Enable(GetDlgItem(hDlg, IDC_CHANNEL), (bCustomChannelOrder == FALSE));
}

BOOL APIENTRY ProgramListProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	int i;
    char sbuf[256];

    switch (message)
	{
	case WM_INITDIALOG:
        InScan = FALSE;
		EditChan = CurrentProgramm;
		SetCapture(hDlg);
        RefreshControls(hDlg);
		ListBox_ResetContent(GetDlgItem(hDlg, IDC_PROGRAMLIST));
        RefreshProgramList(hDlg, CurrentProgramm);

		Button_SetCheck(GetDlgItem(hDlg, IDC_CUTOMCHANNELORDER), bCustomChannelOrder?BST_CHECKED:BST_UNCHECKED);

		SetFocus(GetDlgItem(hDlg, IDC_PROGRAMLIST)); 
		
		ScrollBar_SetRange(GetDlgItem(hDlg, IDC_FINETUNE), 0, 100, FALSE);
		ScrollBar_SetPos(GetDlgItem(hDlg, IDC_FINETUNE), 50, FALSE);

        // fill the formats box
        ComboBox_AddString(GetDlgItem(hDlg, IDC_FORMAT), "Same as Tuner");
		ComboBox_AddString(GetDlgItem(hDlg, IDC_FORMAT), "PAL");
		ComboBox_AddString(GetDlgItem(hDlg, IDC_FORMAT), "NTSC");
		ComboBox_AddString(GetDlgItem(hDlg, IDC_FORMAT), "PAL-M");
		ComboBox_AddString(GetDlgItem(hDlg, IDC_FORMAT), "PAL-N");
		ComboBox_AddString(GetDlgItem(hDlg, IDC_FORMAT), "NTSC-J");
		ComboBox_AddString(GetDlgItem(hDlg, IDC_FORMAT), "PAL60");
        ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_FORMAT), 0);


        // load up the counrty settings
    	Load_Country_Settings();
        if(Countries.size() > 0)
        {
		    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_COUNTRY));
            i = 0;
            for(COUNTRYLIST::iterator it = Countries.begin(); 
                it != Countries.end(); 
                ++it)
            {
			    ComboBox_AddString(GetDlgItem(hDlg, IDC_COUNTRY), ((*it)->m_Name.c_str()));
                i++;
		    }
		    ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_COUNTRY), CountryCode);

            RefreshChannelList(hDlg);

			// if we have any channels then also fill the details box with the current program
			UpdateDetails(hDlg);

        }
        else
        {
            ErrorBox("No counries Loaded, Channels.txt must be missing");
			EndDialog(hDlg, 0);
        }
		break;

    case WM_HSCROLL:
        if(LOWORD(wParam) == SB_LEFT ||
            LOWORD(wParam) == SB_PAGELEFT ||
            LOWORD(wParam) == SB_LINELEFT)
        {
            char sbuf[256];
            char* cLast;
            Edit_GetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf, 255);
            double dFreq = strtod(sbuf, &cLast);
            long Freq = (long)(dFreq * 16.0);
            --Freq;
		    sprintf(sbuf, "%10.4f MHz", (double)Freq / 16.0);
            Edit_SetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf);
            Tuner_SetFrequency(Freq);
        }
        else if(LOWORD(wParam) == SB_RIGHT ||
            LOWORD(wParam) == SB_PAGERIGHT ||
            LOWORD(wParam) == SB_LINERIGHT)
        {
            char sbuf[256];
            char* cLast;
            Edit_GetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf, 255);
            double dFreq = strtod(sbuf, &cLast);
            long Freq = (long)(dFreq * 16.0);
            ++Freq;
		    sprintf(sbuf, "%10.4f MHz", (double)Freq / 16.0);
            Edit_SetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf);
            Tuner_SetFrequency(Freq);
        }
        break;
    case WM_USER:
		if(lParam == 101)
		{
			if(InScan == TRUE)
			{
				if(bCustomChannelOrder)
				{
					ScanCustomChannel(hDlg, wParam);
					if(wParam < MyChannels.size())
					{
						PostMessage(hDlg, WM_USER, wParam + 1, 101);
					}
					else
					{
						InScan = FALSE;
						PostMessage(hDlg, WM_USER, -1, 101);
					}
				}
				else
				{
					EditChan = 0;
					ScanFrequency(hDlg, wParam);
					if(wParam < Countries[CountryCode]->m_Frequencies.size())
					{
						PostMessage(hDlg, WM_USER, wParam + 1, 101);
					}
					else
					{
						InScan = FALSE;
						PostMessage(hDlg, WM_USER, -1, 101);
					}
				}
			}
			else
			{
				Button_SetText(GetDlgItem(hDlg, IDC_SCAN), "Scan");
				if(MyChannels.size() > 0)
				{
					EditChan = 0;
					Channel_Change(0);
					UpdateDetails(hDlg);
				}
				else
				{
					Edit_SetText(GetDlgItem(hDlg, IDC_FREQUENCY), "");
				}
			}
		}
        break;
	case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_PROGRAMLIST:
			if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				i = ListBox_GetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST));

				if ((i >= 0) && (i < MyChannels.size()))
				{
					CurrentProgramm = i;
					EditChan = i;
					Channel_Change(CurrentProgramm);
                    UpdateDetails(hDlg);
				}
				else
				{
					EditChan = -1;
				}
			}
            break;

        case IDC_COUNTRY:
            ResetProgramList(hDlg);
            CountryCode = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_COUNTRY));
            RefreshChannelList(hDlg);
            break;

        case IDC_CUTOMCHANNELORDER:
            bCustomChannelOrder = (Button_GetCheck(GetDlgItem(hDlg, IDC_CUTOMCHANNELORDER)) == BST_CHECKED);
            RefreshControls(hDlg);
            ResetProgramList(hDlg);
            break;
        
        case IDC_CHANNEL:
            {
                char sbuf[256];
                // set the frequency
				int Channel = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_CHANNEL));
				Channel = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CHANNEL), Channel);
		        sprintf(sbuf, "%10.4f MHz", (double)Countries[CountryCode]->m_Frequencies[Channel - Countries[CountryCode]->m_MinChannel] / 16.0);
		        Edit_SetText(GetDlgItem(hDlg, IDC_FREQUENCY),sbuf);
				ScrollBar_SetPos(GetDlgItem(hDlg, IDC_FINETUNE), 50, FALSE);
            }
            break;

        case IDC_CHANGE:
            if(EditChan >= 0)
            {
                char* cLast;
                Edit_GetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf, 255);
                double dFreq = strtod(sbuf, &cLast);
                long Freq = (long)(dFreq * 16.0);
                int Channel = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_CHANNEL));
                Channel = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CHANNEL), Channel);
                int Format = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_FORMAT)) - 1;
                delete MyChannels[EditChan];
                Edit_GetText(GetDlgItem(hDlg, IDC_NAME), sbuf , 255);
                BOOL Active = (Button_GetCheck(GetDlgItem(hDlg, IDC_ACTIVE)) == BST_CHECKED);
                MyChannels[EditChan] = new CChannel(sbuf, Freq, Channel, Format, Active);
				ListBox_DeleteString(GetDlgItem(hDlg, IDC_PROGRAMLIST), EditChan);
				ListBox_InsertString(GetDlgItem(hDlg, IDC_PROGRAMLIST), EditChan, sbuf);
            }
            break;
        case IDC_ADD:
            {
                char* cLast;
                Edit_GetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf, 255);
                double dFreq = strtod(sbuf, &cLast);
                long Freq = (long)(dFreq * 16.0);

                int Channel = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_CHANNEL));
                Channel = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CHANNEL), Channel);
                int Format = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_FORMAT)) - 1;
                Edit_GetText(GetDlgItem(hDlg, IDC_NAME), sbuf , 255);
                BOOL Active = (Button_GetCheck(GetDlgItem(hDlg, IDC_ACTIVE)) == BST_CHECKED);
                MyChannels.push_back(new CChannel(sbuf, Freq, Channel, Format, Active));
                EditChan = ListBox_AddString(GetDlgItem(hDlg, IDC_PROGRAMLIST), sbuf);
				ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), EditChan);
            }
            break;
        case IDC_REMOVE:
            if(EditChan >= 0 && EditChan < MyChannels.size())
            {
                delete MyChannels[EditChan];
                MyChannels.erase(&MyChannels[EditChan]);
                ListBox_DeleteString(GetDlgItem(hDlg, IDC_PROGRAMLIST), EditChan);
				EditChan = -1;
				Edit_SetText(GetDlgItem(hDlg, IDC_NAME), "");
				Edit_SetText(GetDlgItem(hDlg, IDC_FREQUENCY), "");
            }
            break;
        case IDC_UP:
            if(EditChan > 0)
            {
                CChannel* Temp = MyChannels[EditChan];
                MyChannels[EditChan] = MyChannels[EditChan - 1];
                MyChannels[EditChan - 1] = Temp;
				--EditChan; 
                RefreshProgramList(hDlg, EditChan - 1);
            }
            break;
        case IDC_DOWN:
            if(EditChan < MyChannels.size() - 1)
            {
                CChannel* Temp = MyChannels[EditChan];
                MyChannels[EditChan] = MyChannels[EditChan + 1];
				++EditChan; 
                MyChannels[EditChan + 1] = Temp;
                RefreshProgramList(hDlg, EditChan + 1);
            }
            break;
        case IDC_SCAN:
            if(InScan == TRUE)
            {
                InScan = FALSE;
                PostMessage(hDlg, WM_USER, -1, 101);
            }
            else
            {
                InScan = TRUE;
                Button_SetText(GetDlgItem(hDlg, IDC_SCAN), "Cancel");
                if(!bCustomChannelOrder)
                {
                    ResetProgramList(hDlg);
				    Edit_SetText(GetDlgItem(hDlg, IDC_NAME), "");
				    ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_FORMAT), 0);
                }
				else
                PostMessage(hDlg, WM_USER, 0, 101);
            }
            break;
        case IDOK:
			Write_Program_List_ASCII();
    		Unload_Country_Settings();
			EndDialog(hDlg, TRUE);
            break;
        case IDCANCEL:
			Load_Program_List_ASCII();
    		Unload_Country_Settings();
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
			fprintf(SettingFile, "Freq2: %ld\n", (*it)->GetFrequency());
			fprintf(SettingFile, "Chan: %d\n", (*it)->GetChannelNumber());
			fprintf(SettingFile, "Active: %d\n", (*it)->IsActive());
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
	BOOL Active = TRUE;
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
			if(Frequency != -1)
			{
				MyChannels.push_back(new CChannel(Name.c_str(), Frequency, Channel, Format, Active));
			}
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
                MyChannels.push_back(new CChannel(Name.c_str(), Frequency, Channel, Format, Active));
            }
            Name = sbuf + 5;
            Frequency = -1;
            ++Channel;
            Format = -1;
			Active = TRUE;
        }
        // cope with old style frequencies
        else if(strnicmp(sbuf, "Freq:", 5) == 0)
        {
            Frequency = atol(sbuf + 5);
            Frequency = MulDiv(Frequency, 16, 1000);
        }
        else if(strnicmp(sbuf, "Freq2:", 6) == 0)
        {
            Frequency = atol(sbuf + 6);
        }
        else if(strnicmp(sbuf, "Chan:", 5) == 0)
        {
            Channel = atoi(sbuf + 5);
        }
        else if(strnicmp(sbuf, "Form:", 5) == 0)
        {
            Format = atoi(sbuf + 5);
        }
        else if(strnicmp(sbuf, "Active:", 7) == 0)
        {
            Active = (atoi(sbuf + 7) != 0);
        }
        else
        {
            ; //some other rubbish
        }
    }

    if(Frequency != -1)
    {
        MyChannels.push_back(new CChannel(Name.c_str(), Frequency, Channel, Format, Active));
    }

	fclose(SettingFile);
	return;
}

//---------------------------------------------------------------------------
void Channel_Change(int NewChannel)
{
	if (GetTunerSetup() != NULL)
	{
		if(NewChannel >= 0 && NewChannel < MyChannels.size())
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
				Tuner_SetFrequency(MyChannels[CurrentProgramm]->GetFrequency());
                if(MyChannels[CurrentProgramm]->GetFormat() != -1)
                {
        			Setting_SetValue(BT848_GetSetting(TVFORMAT), MyChannels[CurrentProgramm]->GetFormat());
                }
				else
				{
        			Setting_SetValue(BT848_GetSetting(TVFORMAT), GetTunersTVFormat());
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
    // look for next active channel
    ++CurrentProgramm;
    while(CurrentProgramm < MyChannels.size() && 
        !MyChannels[CurrentProgramm]->IsActive())
    {
        ++CurrentProgramm;
    }

    // see if we looped around
    if(CurrentProgramm == MyChannels.size())
    {
        CurrentProgramm = 0;
        while(CurrentProgramm < MyChannels.size() && 
            !MyChannels[CurrentProgramm]->IsActive())
        {
            ++CurrentProgramm;
        }

        // see if we looped around again
        if(CurrentProgramm == MyChannels.size())
        {
            CurrentProgramm = 0;
        }
    }
	
    Channel_Change(CurrentProgramm);

	StatusBar_ShowText(STATUS_KEY, MyChannels[CurrentProgramm]->GetName());
	OSD_ShowText(hWnd,MyChannels[CurrentProgramm]->GetName(), 0);
}

void Channel_Decrement()
{
    // look for next active channel
    --CurrentProgramm;
    while(CurrentProgramm > -1 && 
        !MyChannels[CurrentProgramm]->IsActive())
    {
        --CurrentProgramm;
    }

    // see if we looped around
    if(CurrentProgramm == -1)
    {
        CurrentProgramm = MyChannels.size() - 1;
        while(CurrentProgramm > -1  && 
            !MyChannels[CurrentProgramm]->IsActive())
        {
            --CurrentProgramm;
        }

        // see if we looped around again
        if(CurrentProgramm == -1)
        {
            CurrentProgramm = 0;
        }
    }
	
    Channel_Change(CurrentProgramm);

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
        for (int j = 0; j < MyChannels.size(); ++j)
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
                    // convert frequency in KHz to Units that the tuner wants
                    long Freq = atol(Pos);
                    Freq = MulDiv(Freq, 16, 1000);
                    NewCountry->m_Frequencies.push_back(Freq);
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
