/////////////////////////////////////////////////////////////////////////////
// $Id: EPG.cpp,v 1.18 2005-07-06 19:40:38 laurentg Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 Laurent Garnier.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//  This file is subject to the terms of the GNU General Public License as
//  published by the Free Software Foundation.  A copy of this license is
//  included with this software distribution in the file COPYING.  If you
//  do not have a copy, you may obtain a copy by writing to the Free
//  Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.17  2005/04/09 12:49:49  laurentg
// EPG: choose the NextviewEPG provider
//
// Revision 1.16  2005/04/07 23:17:27  laurentg
// EPG: import NextviewEPG database
//
// Revision 1.15  2005/04/02 14:23:44  laurentg
// EPG: little bugs fixed
//
// Revision 1.14  2005/04/02 14:04:11  laurentg
// EPG: navigation between the browser view and the programme view improved
//
// Revision 1.13  2005/04/01 22:16:32  laurentg
// EPG: new menu "Hide EPG" + new setting to define the time frame duration
//
// Revision 1.12  2005/03/29 21:08:33  laurentg
// program renamed programme
//
// Revision 1.11  2005/03/28 17:47:57  laurentg
// Navigation into EPG + change of channel
//
// Revision 1.10  2005/03/28 13:42:02  laurentg
// EPG: preparation for when new data (category, sub-title, description) will be available
//
// Revision 1.9  2005/03/28 13:11:16  laurentg
// New EPG setting to shift times during import
//
// Revision 1.8  2005/03/28 12:53:20  laurentg
// EPG: previous and next page to show programs
//
// Revision 1.7  2005/03/27 20:22:07  laurentg
// EPG: new improvements
//
// Revision 1.6  2005/03/26 19:55:33  laurentg
// Bug fixed when shifting time frame to the next or previous day
//
// Revision 1.5  2005/03/26 18:53:22  laurentg
// EPG code improved
// => possibility to set the EPG channel name in the channel setup dialog box
// => automatic loading of new data when needed
// => OSD scrrens updated
// => first step for programs "browser"
//
// Revision 1.4  2005/03/21 22:39:15  laurentg
// EPG: changes regarding OSD
//
// Revision 1.3  2005/03/20 22:56:22  laurentg
// New OSD screens added for EPG
//
// Revision 1.2  2005/03/20 16:56:26  laurentg
// Bug fixed regarding channel name containing special PERL REGEXP characters
// Files now generated in the DScaler main directory
//
// Revision 1.1  2005/03/20 09:48:58  laurentg
// XMLTV file import
//
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 16 Mar 2005   Laurent Garnier       File created
//
/////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "stdafx.h"
#include "DebugLog.h"
#include "EPG.h"
#include "ProgramList.h"
#include "OSD.h"
#include "Providers.h"
#include "DScaler.h"
#include "xmltv_db.h"


#define	ONE_DAY			86400
#define	ONE_HOUR		3600
#define	DEFAULT_INPUT_FILE		"epg.xml"
#define	DEFAULT_OUTPUT_FILE		"DScalerEPG.xml"


CEPG MyEPG;


CProgramme::CProgramme(time_t StartTime, time_t EndTime, LPCSTR Title, LPCSTR ChannelName, LPCSTR ChannelEPGName, int ChannelNumber, LPCSTR SubTitle, LPCSTR Category, LPCSTR Description)
{
	m_StartTime = StartTime;
	m_EndTime = EndTime;
	m_Title = Title;
	m_ChannelName = ChannelName;
	m_ChannelEPGName = ChannelEPGName;
	m_ChannelNumber = ChannelNumber;
	m_SubTitle = SubTitle;
	m_Category = Category;
	m_Description = Description;
	m_Length = 0;
}


CProgramme::~CProgramme()
{
}


//
// Check whether the programme matchs the channel (if provided)
// and overlaps the period of time defined by DateMin and DateMax
//
BOOL CProgramme::IsProgrammeMatching(time_t DateMin, time_t DateMax, LPCSTR Channel)
{
	if (   (DateMax >= m_StartTime)
		&& (DateMin < m_EndTime)
		&& ( (Channel == NULL) || !_stricmp(Channel, m_ChannelName.c_str()) )
	   )
	   return TRUE;

	return FALSE;
}


// Get the programme dates : start and end time
void CProgramme::GetProgrammeDates(time_t *StartTime, time_t *EndTime)
{
	*StartTime = m_StartTime;
	*EndTime = m_EndTime;
}


//
// Get the channel data : DScaler name + EPG name + number
//
void CProgramme::GetProgrammeChannelData(string &ChannelName, string &ChannelEPGName, int *ChannelNumber)
{
	ChannelName = m_ChannelName;
	ChannelEPGName = m_ChannelName;
	*ChannelNumber = m_ChannelNumber;
}


//
// Get the programme main data : start and end time + title
//
void CProgramme::GetProgrammeMainData(time_t *StartTime, time_t *EndTime, string &Channel, string &Title, string &Category)
{
	*StartTime = m_StartTime;
	*EndTime = m_EndTime;
	Channel = m_ChannelName;
	Title = m_Title;
	Category = m_Category;
}


//
// Get all the programme data
//
void CProgramme::GetProgrammeData(time_t *StartTime, time_t *EndTime, string &Channel, string &Title, string &SubTitle, string &Category, string &Description)
{
	*StartTime = m_StartTime;
	*EndTime = m_EndTime;
	Channel = m_ChannelName;
	Title = m_Title;
	SubTitle = m_SubTitle;
	Category = m_Category;
	Description = m_Description;
}


//
// Dump the programme main data : start and end time + channel + title
//
void CProgramme::DumpProgrammeMainData()
{
	char Date1[17];
	char Date2[17];
	struct tm *date_tm;
	date_tm = localtime(&m_StartTime);
	sprintf(Date1,
			"%04u/%02u/%02u %02u:%02u",
			date_tm->tm_year+1900, date_tm->tm_mon+1, date_tm->tm_mday, date_tm->tm_hour, date_tm->tm_min);
	date_tm = localtime(&m_EndTime);
	sprintf(Date2,
			"%04u/%02u/%02u %02u:%02u",
			date_tm->tm_year+1900, date_tm->tm_mon+1, date_tm->tm_mday, date_tm->tm_hour, date_tm->tm_min);
	LOG(1, "%s - %s : %s : %s",
		Date1, Date2, m_ChannelName.c_str(), m_Title.c_str());
}


CEPG::CEPG()
{
	char *windir = getenv("windir");
	if (!windir)
	{
		m_CMDExe = "cmd.exe";
	}
	else
	{
		struct _stat buf;
		int result;
		m_CMDExe = windir;
		m_CMDExe += "\\system32\\cmd.exe";
		result = _stat(m_CMDExe.c_str(), &buf);
		if (result != 0)
		{
			m_CMDExe = "cmd.exe";
		}
	}

    char Path[MAX_PATH];
    GetModuleFileName (NULL, Path, sizeof(Path));
    *strrchr(Path, '\\') = '\0';

	m_FilesDir = Path;

	m_LoadedTimeMin = 0;
	m_LoadedTimeMax = 0;

	m_SearchChannel = NULL;
	m_SearchTimeMin = 0;
	m_SearchTimeMax = 0;

	SetDisplayIndexes(-1, -1, -1);

	m_ProgrammeSelected = NULL;

	m_UseProgFronBrowser = FALSE;

	m_Displayed = 0;
}


CEPG::~CEPG()
{
	ClearProgrammes();
	ClearNextviewEPGProviders();
}


//
// Execute a command using the Windows command interpreter
//
int CEPG::ExecuteCommand(string command)
{
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));

	// Run the command in a new process
	// Using IDLE_PRIORITY_CLASS gives very low priority to the process
	// Using CREATE_NO_WINDOW hides the window
	//
	// We use normal priority because with idle priority it takes too much time
	// and I am even not sure what could happen when Judder Terminator is activated
	// and DScaler is using 100% of CPU
	if (!CreateProcess(NULL,
					   (char*)(command.c_str()),
					   (LPSECURITY_ATTRIBUTES) NULL,	// No security
					   (LPSECURITY_ATTRIBUTES) NULL,	// No security
					   FALSE,
//					   IDLE_PRIORITY_CLASS | CREATE_NO_WINDOW,
					   0,
					   NULL,
					   NULL,
					   &si,
					   &pi))
		return -1;

	// Wait until child process exits
    WaitForSingleObject(pi.hProcess, INFINITE);

	// Close process and thread handles
	CloseHandle(pi.hProcess); 
    CloseHandle(pi.hThread); 

	return 0;
}


//
// Import the NextviewEPG database
// Put the result in DScalerEPG.xml
//
int CEPG::ImportNxtvepgEPGDB(LPCSTR Provider)
{
	string Exe = (char*)Setting_GetValue(EPG_GetSetting(EPG_NXTVEPGPATH));
	string OutputFile = m_FilesDir + "\\" + DEFAULT_OUTPUT_FILE;
	string command = "\"" + m_CMDExe + "\" /C echo NextviewEPG database export ... && \""
		+ Exe + "\" -dump xml5ltz -provider " + Provider + " > \"" + OutputFile + "\"";

	LOG(2, "NextviewEPG database import ... (%s)", command.c_str());
	int resu = ExecuteCommand(command);
	LOG(2, "NextviewEPG database import %d", resu);

	ReloadEPGData();

	return resu;
}


//
// Copy the input file in DScalerEPG.xml
//
int CEPG::ImportXMLTVFile(LPCSTR file)
{
	// If file not provided, try with "epg.xml"
	string XMLFile;
	if (file == NULL)
		XMLFile = m_FilesDir + "\\" + DEFAULT_INPUT_FILE;
	else
		XMLFile = file;

	string OutputFile = m_FilesDir + "\\" + DEFAULT_OUTPUT_FILE;

	LOG(2, "Copy file ... (%s)", XMLFile.c_str());
	BOOL resu = CopyFile(XMLFile.c_str(), OutputFile.c_str());
	LOG(2, "Copy file %d", resu);

	ReloadEPGData();

	return resu;
}


//
// Load the DScaler EPG data for the programmes between two dates
// If DateMin and DateMax are not set, load the EPG data for
// the interval [current time - 2 hours, current time + 6 hours]
//
int CEPG::LoadEPGData(time_t DateMin, time_t DateMax)
{
	ClearProgrammes();

	time_t DateTime;

	// If dates not provided, load the data for the interval
	// [current time - 2 hours, current time + one day]
	if ( (DateMin == 0) || (DateMax == 0) )
	{
		time(&DateTime);
		DateMin = DateTime - 2 * ONE_HOUR;
		DateMax = DateTime + 6 * ONE_HOUR;
	}

	m_LoadedTimeMin = DateMin;
	m_LoadedTimeMax = DateMax;

	string InputXMLFile = m_FilesDir + "\\" + DEFAULT_OUTPUT_FILE;
	Xmltv_LoadFile(InputXMLFile.c_str());

	return 0;
}


int CEPG::ReloadEPGData()
{
	int resu = LoadEPGData(m_LoadedTimeMin, m_LoadedTimeMax);

	// TODO Update the OSD

	return resu;
}


// Check if new EPG data have to be loaded
BOOL CEPG::LoadEPGDataIfNeeded(time_t TimeMin, time_t TimeMax, int DeltaEarlier, int DeltaLater)
{
	BOOL LoadDone = FALSE;

	if (TimeMin < m_LoadedTimeMin)
	{
		LoadEPGData(TimeMin - DeltaEarlier, m_LoadedTimeMax);
		LoadDone = TRUE;
	}
	if (TimeMax > m_LoadedTimeMax)
	{
		LoadEPGData(m_LoadedTimeMin, TimeMax + DeltaLater);
		LoadDone = TRUE;
	}
	return LoadDone;
}


int CEPG::GetSearchContext(LPCSTR *ChannelName, time_t *TimeMin, time_t *TimeMax)
{
	if (ChannelName)
		*ChannelName = m_SearchChannel;
	*TimeMin = m_SearchTimeMin;
	*TimeMax = m_SearchTimeMax;
	return m_ProgrammesSelection.size();
}


BOOL CEPG::SearchForProgramme(string &Channel)
{
	BOOL resu = FALSE;
	Channel = "";
	if (m_UseProgFronBrowser == FALSE)
	{
		CSource *CurrentSource = Providers_GetCurrentSource();
		if (CurrentSource && Providers_GetCurrentSource()->IsInTunerMode())
		{
			time_t TimeNow;
			time(&TimeNow);
			Channel = Channel_GetName();
			resu = SearchForProgramme(Channel.c_str(), TimeNow);
		}
	}
	else
	{
		if (m_ProgrammeSelected)
		{
			string ChannelEPG;
			int    Number;
			m_ProgrammeSelected->GetProgrammeChannelData(Channel, ChannelEPG, &Number);
			resu = TRUE;
		}
	}

	return resu;
}


BOOL CEPG::SearchForProgramme(LPCSTR ChannelName, time_t ThatTime)
{
	m_ProgrammeSelected = NULL;
    for(CProgrammes::iterator it = m_Programmes.begin();
        it != m_Programmes.end();
        ++it)
    {
		if ((*it)->IsProgrammeMatching(ThatTime, ThatTime, ChannelName) == TRUE)
		{
			m_ProgrammeSelected = *it;
			return TRUE;
		}
    }
	return FALSE;
}


int CEPG::SearchForProgrammes(LPCSTR ChannelName, time_t TimeMin, time_t TimeMax)
{
	m_SearchChannel = ChannelName;
	m_SearchTimeMin = TimeMin;
	m_SearchTimeMax = TimeMax;
    m_ProgrammesSelection.clear();
	SetDisplayIndexes(-1, -1, -1);
    for(CProgrammes::iterator it = m_Programmes.begin();
        it != m_Programmes.end();
        ++it)
    {
		if ((*it)->IsProgrammeMatching(TimeMin, TimeMax, ChannelName) == TRUE)
		{
			m_ProgrammesSelection.push_back(*it);
		}
    }
	if (m_ProgrammesSelection.size() > 0)
	{
		SetDisplayIndexes(1, m_ProgrammesSelection.size(), 1);
	}
	return m_ProgrammesSelection.size();
}


void CEPG::SetDisplayIndexes(int IdxMin, int IdxMax, int IdxCur)
{
	m_IdxShowSelectMin = IdxMin;
	m_IdxShowSelectMax = IdxMax;
	m_IdxShowSelectCur = IdxCur;
}


int CEPG::GetDisplayIndexes(int *IdxMin, int *IdxMax, int *IdxCur)
{
	*IdxMin = m_IdxShowSelectMin;
	*IdxMax = m_IdxShowSelectMax;
	*IdxCur = m_IdxShowSelectCur;
	return m_ProgrammesSelection.size();
}


BOOL CEPG::GetProgrammeChannelData(int Index, string &ChannelName, string &ChannelEPGName, int *ChannelNumber)
{
	if ( (Index < 0) && (m_ProgrammeSelected == NULL))
		return FALSE;

	if ( (Index >= 0) && (Index >= m_ProgrammesSelection.size()) )
		return FALSE;

	if (Index < 0)
	{
		m_ProgrammeSelected->GetProgrammeChannelData(ChannelName, ChannelEPGName, ChannelNumber);
	}
	else
	{
		m_ProgrammesSelection[Index]->GetProgrammeChannelData(ChannelName, ChannelEPGName, ChannelNumber);
	}

	return TRUE;
}


BOOL CEPG::GetProgrammeMainData(int Index, time_t *StartTime, time_t *EndTime, string &Channel, string &Title, string &Category)
{
	if ( (Index < 0) && (m_ProgrammeSelected == NULL))
		return FALSE;

	if ( (Index >= 0) && (Index >= m_ProgrammesSelection.size()) )
		return FALSE;

	if (Index < 0)
	{
		m_ProgrammeSelected->GetProgrammeMainData(StartTime, EndTime, Channel, Title, Category);
	}
	else
	{
		m_ProgrammesSelection[Index]->GetProgrammeMainData(StartTime, EndTime, Channel, Title, Category);
	}

	return TRUE;
}


BOOL CEPG::GetProgrammeData(int Index, time_t *StartTime, time_t *EndTime, string &Channel, string &Title, string &SubTitle, string &Category, string &Description)
{
	if ( (Index < 0) && (m_ProgrammeSelected == NULL))
		return FALSE;

	if ( (Index >= 0) && (Index >= m_ProgrammesSelection.size()) )
		return FALSE;

	if (Index < 0)
	{
		m_ProgrammeSelected->GetProgrammeData(StartTime, EndTime, Channel, Title, SubTitle, Category, Description);
	}
	else
	{
		m_ProgrammesSelection[Index]->GetProgrammeData(StartTime, EndTime, Channel, Title, SubTitle, Category, Description);
	}

	return TRUE;
}


BOOL CEPG::HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam)
{
	OPENFILENAME OpenFileInfo;
	char FilePath[MAX_PATH];
	char* FileFilters;
	time_t TimeNow;
	time_t TimeMin;
	time_t TimeMax;
	struct tm *datetime_tm;
	string Title;
	string ChannelName;
	string ChannelEPGName;
	int ChannelNumber;
	char* Exe;
    struct stat st;
	int n;
	LPCSTR Provider = NULL;

	switch(LOWORD(wParam))
    {
	case IDM_IMPORT_XMLTV:
		FileFilters = "XML Files\0*.xml\0";
		FilePath[0] = 0;
		ZeroMemory(&OpenFileInfo,sizeof(OpenFileInfo));
		OpenFileInfo.lStructSize = sizeof(OpenFileInfo);
		OpenFileInfo.hwndOwner = hWnd;
		OpenFileInfo.lpstrFilter = FileFilters;
		OpenFileInfo.nFilterIndex = 1;
		OpenFileInfo.lpstrCustomFilter = NULL;
		OpenFileInfo.lpstrFile = FilePath;
		OpenFileInfo.nMaxFile = sizeof(FilePath);
		OpenFileInfo.lpstrFileTitle = NULL;
		OpenFileInfo.lpstrInitialDir = NULL;
		OpenFileInfo.lpstrTitle = NULL;
		OpenFileInfo.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
		OpenFileInfo.lpstrDefExt = NULL;
		if(GetOpenFileName(&OpenFileInfo))
		{
			ImportXMLTVFile(FilePath);
		}
        return TRUE;
		break;

	case IDM_IMPORT_NEXTVIEW:
		Exe = (char*)Setting_GetValue(EPG_GetSetting(EPG_NXTVEPGPATH));
		if ((Exe == NULL) || stat(Exe, &st))
		{
			MessageBox(hWnd, "NextviewEPG application not found.\nPlease go to the advanced settings to update its location.", "DScaler Warning", MB_ICONWARNING | MB_OK);
			return TRUE;
		}

		n = GetNextviewEPGProviders();
		if (n == 0)
		{
			MessageBox(hWnd, "No NextviewEPG database found.", "DScaler Warning", MB_ICONWARNING | MB_OK);
		}
		else if (n == 1)
		{
			Provider = m_NextviewProviders[0]->c_str();
		}
		else
		{
			for (int i=0; i<n ; i++)
			{
				char MsgTxt[128];
				sprintf(MsgTxt, "Do you want to import the database\ncorresponding to the provider %s ?", m_NextviewProviders[i]->c_str());
				if (MessageBox(hWnd, MsgTxt, "DScaler - NextviewEPG Provider", MB_YESNO | MB_ICONQUESTION /*| MB_APPLMODAL*/) == IDYES)
				{
					Provider = m_NextviewProviders[i]->c_str();
					break;
				}
			}
		}
		if (Provider != NULL)
		{
			ImportNxtvepgEPGDB(Provider);
		}
        return TRUE;
		break;

	case IDM_LOAD_EPG:
		if (!ReloadEPGData())
		{
			OSD_ShowText("EPG loaded", 0);
		}
		else
		{
			OSD_ShowText("EPG not found", 0);
		}
		// Dump the loaded EPG data in the log file
		//DumpEPGData();
        return TRUE;
		break;

	case IDM_DISPLAY_EPG:
		if (   (m_Displayed == 1)
			|| (m_UseProgFronBrowser == FALSE) )
		{
			m_UseProgFronBrowser = FALSE;
			// Check if new EPG data have to be loaded
			time(&TimeNow);
			LoadEPGDataIfNeeded(m_LoadedTimeMin, TimeNow, 0, 6 * ONE_HOUR);
			// Display the OSD screen
			OSD_ShowInfosScreen(2, Setting_GetValue(EPG_GetSetting(EPG_PERCENTAGESIZE)));
			m_Displayed = 1;
		}
		else if (m_IdxShowSelectCur != -1)
		{
			m_ProgrammeSelected = m_ProgrammesSelection[m_IdxShowSelectCur-1];
			// Display the OSD screen
			OSD_ShowInfosScreen(2, Setting_GetValue(EPG_GetSetting(EPG_PERCENTAGESIZE)));
			m_Displayed = 1;
		}
        return TRUE;
		break;

	case IDM_DISPLAY_EPG_NOW:
		if ( (m_Displayed == 1) && (m_UseProgFronBrowser == TRUE) )
		{
		}
		else
		{
			time(&TimeMin);
			TimeMax = TimeMin + ONE_HOUR * Setting_GetValue(EPG_GetSetting(EPG_TIMEFRAMEDURATION)) - 1;
			// Check if new EPG data have to be loaded
			LoadEPGDataIfNeeded(TimeMin, TimeMax, 0, 0);
			// Select the corresponding programmes
			SearchForProgrammes(NULL, TimeMin, TimeMax);
		}
		m_UseProgFronBrowser = TRUE;
		// Display the OSD screen
		OSD_ShowInfosScreen(3, Setting_GetValue(EPG_GetSetting(EPG_PERCENTAGESIZE)));
		m_Displayed = 2;
        return TRUE;
        break;

	case IDM_DISPLAY_EPG_EARLIER:
		if (m_Displayed == 2)
		{
			// If we were not at the exact beginning of an hour
			// then we shift to the beginning of that hour
			// else we go one hour earlier
			datetime_tm = localtime(&m_SearchTimeMin);
			if (   (datetime_tm->tm_min != 0)
				|| (datetime_tm->tm_sec != 0) )
			{
				datetime_tm->tm_sec = 0;
				datetime_tm->tm_min = 0;
				TimeMin = mktime(datetime_tm);
			}
			else
			{
				TimeMin = m_SearchTimeMin - ONE_HOUR * Setting_GetValue(EPG_GetSetting(EPG_TIMEFRAMEDURATION));
			}
			TimeMax = TimeMin + ONE_HOUR * Setting_GetValue(EPG_GetSetting(EPG_TIMEFRAMEDURATION)) - 1;
			// Check if new EPG data have to be loaded
			LoadEPGDataIfNeeded(TimeMin, TimeMax, 0, 0);
			// Select the corresponding programmes
			SearchForProgrammes(NULL, TimeMin, TimeMax);
		}
        return TRUE;
        break;

	case IDM_DISPLAY_EPG_LATER:
		if (m_Displayed == 2)
		{
			// If we were not at the exact beginning of an hour
			// then we shift to the beginning of the next hour
			// else we go one hour later
			datetime_tm = localtime(&m_SearchTimeMin);
			if (   (datetime_tm->tm_min != 0)
				|| (datetime_tm->tm_sec != 0) )
			{
				datetime_tm->tm_sec = 0;
				datetime_tm->tm_min = 0;
				TimeMin = mktime(datetime_tm);
				TimeMin += ONE_HOUR * Setting_GetValue(EPG_GetSetting(EPG_TIMEFRAMEDURATION));
			}
			else
			{
				TimeMin = m_SearchTimeMin + ONE_HOUR * Setting_GetValue(EPG_GetSetting(EPG_TIMEFRAMEDURATION));
			}
			TimeMax = TimeMin + ONE_HOUR * Setting_GetValue(EPG_GetSetting(EPG_TIMEFRAMEDURATION)) - 1;
			// Check if new EPG data have to be loaded
			LoadEPGDataIfNeeded(TimeMin, TimeMax, 0, 0);
			// Select the corresponding programmes
			SearchForProgrammes(NULL, TimeMin, TimeMax);
		}
        return TRUE;
        break;

	case IDM_DISPLAY_EPG_NEXT:
		if (   (m_Displayed == 2)
		    && (m_IdxShowSelectMax != -1)
			&& (m_IdxShowSelectMax < m_ProgrammesSelection.size()) )
		{
			SetDisplayIndexes(m_IdxShowSelectMax + 1, -1, m_IdxShowSelectMax + 1);
		}
		else if (m_Displayed == 1)
		{
			// TODO Show the details of the next programme on the same channel
		}
        return TRUE;
        break;

	case IDM_DISPLAY_EPG_PREV:
		if (   (m_Displayed == 2)
		    && (m_IdxShowSelectMin != -1)
			&& (m_IdxShowSelectMin > 1) )
		{
			SetDisplayIndexes(-1, m_IdxShowSelectMin - 1, m_IdxShowSelectMin - 1);
		}
		else if (m_Displayed == 1)
		{
			// TODO Show the details of the previous programme on the same channel
		}
        return TRUE;
        break;

	case IDM_DISPLAY_EPG_NEXT_IN_PAGE:
		if (m_Displayed == 2)
		{
			if (   (m_IdxShowSelectMin != -1)
				&& (m_IdxShowSelectMax != -1)
				&& (m_IdxShowSelectCur < m_IdxShowSelectMax) )
			{
				SetDisplayIndexes(m_IdxShowSelectMin, m_IdxShowSelectMax, m_IdxShowSelectCur + 1);
			}
			else if (   (m_IdxShowSelectMin != -1)
					 && (m_IdxShowSelectMax != -1)
					 && (m_IdxShowSelectCur == m_IdxShowSelectMax) )
			{
				PostMessageToMainWindow(WM_COMMAND, IDM_DISPLAY_EPG_NEXT, 0);
			}
		}
        return TRUE;
        break;

	case IDM_DISPLAY_EPG_PREV_IN_PAGE:
		if (m_Displayed == 2)
		{
			if (   (m_IdxShowSelectMin != -1)
				&& (m_IdxShowSelectMax != -1)
				&& (m_IdxShowSelectCur > m_IdxShowSelectMin) )
			{
				SetDisplayIndexes(m_IdxShowSelectMin, m_IdxShowSelectMax, m_IdxShowSelectCur - 1);
			}
			else if (   (m_IdxShowSelectMin != -1)
					 && (m_IdxShowSelectMax != -1)
					 && (m_IdxShowSelectCur == m_IdxShowSelectMin) )
			{
				PostMessageToMainWindow(WM_COMMAND, IDM_DISPLAY_EPG_PREV, 0);
			}
		}
        return TRUE;
        break;

	case IDM_EPG_VIEW_PROG:
		if (   (m_Displayed == 2)
			&& (m_IdxShowSelectCur != -1)
		    && (MyEPG.GetProgrammeChannelData(m_IdxShowSelectCur-1, ChannelName, ChannelEPGName, &ChannelNumber) == TRUE) )
		{
			if (ChannelNumber >= 0)
			{
				if (!Providers_GetCurrentSource()->IsInTunerMode())
				{            
					SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_INPUT1, 0);
				}
				Channel_Change(ChannelNumber);
			}
		}
        return TRUE;
        break;

	case IDM_HIDE_EPG:
        OSD_Clear();
        return TRUE;
        break;

    default:
        break;
    }
    return FALSE;
}


void CEPG::ShowOSD()
{
	// Check if new EPG data have to be loaded
	time_t TimeNow;
	time(&TimeNow);
	LoadEPGDataIfNeeded(m_LoadedTimeMin, TimeNow, 0, 6 * ONE_HOUR);

	if (   (m_Programmes.size() > 0)
		&& (m_UseProgFronBrowser == FALSE) )
	{
		CSource *CurrentSource = Providers_GetCurrentSource();
		if (CurrentSource && Providers_GetCurrentSource()->IsInTunerMode())
		{
			// Search EPG info for the currently viewed channel
			if (MyEPG.SearchForProgramme(Channel_GetName(), TimeNow) == TRUE)
			{
				if (m_Displayed == 0)
				{
					// Display the OSD screen
					OSD_ShowInfosScreen(1, Setting_GetValue(EPG_GetSetting(EPG_PERCENTAGESIZE)));
				}
			}
		}
	}
}


void CEPG::HideOSD()
{
	m_SearchChannel = NULL;
	m_SearchTimeMin = 0;
	m_SearchTimeMax = 0;

	SetDisplayIndexes(-1, -1, -1);

	m_ProgrammeSelected = NULL;
    m_ProgrammesSelection.clear();

	m_UseProgFronBrowser = FALSE;

	m_Displayed = 0;
}


//
// Dump the EPG data
//
void CEPG::DumpEPGData()
{
    for(CProgrammes::iterator it = m_Programmes.begin();
        it != m_Programmes.end();
        ++it)
    {
		(*it)->DumpProgrammeMainData();
    }
}


void CEPG::ClearProgrammes()
{
	m_ProgrammeSelected = NULL;
    m_ProgrammesSelection.clear();
    for(CProgrammes::iterator it = m_Programmes.begin();
        it != m_Programmes.end();
        ++it)
    {
        delete (*it);
    }
    m_Programmes.clear();
}


int CEPG::CheckProgrammeValidity(time_t StartTime, time_t EndTime, LPCSTR ChannelName)
{
	if (!IsValidChannelName(ChannelName))
		return 0;
	// Keep only programmes scheduled between m_LoadedTimeMin and m_LoadedTimeMax
	if ( (EndTime <= m_LoadedTimeMin) || (StartTime > m_LoadedTimeMax) )
		return 0;
	return 1;
}


//
// Insert a new programme in the list keeping an order by dates of the programmes
//
void CEPG::InsertProgramme(CProgramme* NewProg)
{
	time_t NewProgStart, NewProgEnd;
	time_t ProgStart, ProgEnd;

	NewProg->GetProgrammeDates(&NewProgStart, &NewProgEnd);

	int added = 0;
	// Search the first programme having a start time later
	// or an identical start time but a end time later
	for(CProgrammes::iterator it = m_Programmes.begin();
		it != m_Programmes.end();
		++it)
	{
		(*it)->GetProgrammeDates(&ProgStart, &ProgEnd);
		if (   (ProgStart > NewProgStart)
			|| ((ProgStart == NewProgStart) && (ProgEnd > NewProgEnd))
		   )
		{
			m_Programmes.insert(it, NewProg);
			added = 1;
			break;
		}
	}
	if (!added)
	{
		m_Programmes.push_back(NewProg);
	}
}


void CEPG::AddProgramme(time_t StartTime, time_t EndTime, LPCSTR Title, LPCSTR ChannelName, LPCSTR ChannelEPGName, int ChannelNumber)
{
	if ( (EndTime > m_LoadedTimeMin) && (StartTime <= m_LoadedTimeMax) )
	{
		CProgramme* newProgramme = new CProgramme(StartTime, EndTime, Title, ChannelName, ChannelEPGName, ChannelNumber, "", "", "");
		InsertProgramme(newProgramme);
	}
}


void CEPG::AddProgramme(time_t StartTime, time_t EndTime, LPCSTR Title, LPCSTR ChannelName, LPCSTR ChannelEPGName, int ChannelNumber, LPCSTR SubTitle, LPCSTR Category, LPCSTR Description)
{
	if ( (EndTime > m_LoadedTimeMin) && (StartTime <= m_LoadedTimeMax) )
	{
		CProgramme* newProgramme = new CProgramme(StartTime, EndTime, Title, ChannelName, ChannelEPGName, ChannelNumber, SubTitle, Category, Description);
		InsertProgramme(newProgramme);
	}
}


void CEPG::AddProgramme(time_t StartTime, time_t EndTime, LPCSTR Title, LPCSTR ChannelEPGName, LPCSTR SubTitle, LPCSTR Category, LPCSTR Description)
{
	LPCSTR ChannelName;
	int  ChannelNumber;

	if (IsValidChannelName(ChannelEPGName, &ChannelName, &ChannelNumber))
	{
		AddProgramme(StartTime, EndTime, Title, ChannelName, ChannelEPGName, ChannelNumber, SubTitle, Category, Description);
	}
}


//
// Check whether a channel name belongs to the list of channels
// defined in DScaler
//
BOOL CEPG::IsValidChannelName(LPCSTR EPGName, LPCSTR *Name, int *Number)
{
	if (Name)
	{
		*Name = NULL;
	}
	if (Number)
	{
		*Number = -1;
	}
	for (int i=0; (i < MyChannels.GetSize()); i++)
	{
		if (!_stricmp(EPGName, MyChannels.GetChannelEPGName(i)))
		{
			if (Name)
			{
				*Name = MyChannels.GetChannelName(i);
			}
			if (Number)
			{
				*Number = i;
			}
			return TRUE;
		}
	}
	return FALSE;
}


//
// Copy a file
//
BOOL CEPG::CopyFile(LPCSTR InPath, LPCSTR OutPath)
{
	FILE *InStream;
	FILE *OutStream;

	InStream = fopen(InPath, "r");
	if (InStream == NULL)
	{
		return FALSE;
	}
	OutStream = fopen(OutPath, "w");
	if (OutStream == NULL)
	{
		fclose(InStream);
		return FALSE;
	}

	while (feof(InStream) == 0)
	{
		char buffer[512];
		int n = fread(buffer, 1, 512, InStream);
		fwrite(buffer, 1, n, OutStream);
	}

	fclose(InStream);
	fclose(OutStream);

	return TRUE;
}


void CEPG::ClearNextviewEPGProviders()
{
    for(strings::iterator it = m_NextviewProviders.begin();
        it != m_NextviewProviders.end();
        ++it)
    {
        delete (*it);
    }
	m_NextviewProviders.clear();
}


int CEPG::GetNextviewEPGProviders()
{
	ClearNextviewEPGProviders();

    struct stat st;
	char* Exe = (char*)Setting_GetValue(EPG_GetSetting(EPG_NXTVEPGPATH));
	if ((Exe == NULL) || stat(Exe, &st))
	{
		return m_NextviewProviders.size();
	}

    WIN32_FIND_DATA FindFileData;
    HANDLE hFindFile;
	char SearchFiles[MAX_PATH];
    BOOL RetVal;
	char Provider[5];
	string* newProvider;
	char* c;

	strcpy(SearchFiles, Exe);
	c = strrchr(SearchFiles, '\\');
	if (c)
	{
		strcpy(c+1, "nxtv*.epg");
	}
	else
	{
		strcpy(SearchFiles, "nxtv*.epg");
	}

    hFindFile = FindFirstFile(SearchFiles, &FindFileData);

    if (hFindFile != INVALID_HANDLE_VALUE)
    {
        RetVal = TRUE;
        while(RetVal != 0)
        {
            // check to see if the extension is .epg
			// and * matchs 4 characters
            if(   (_stricmp(".epg", &FindFileData.cFileName[strlen(FindFileData.cFileName)-4]) == 0)
			   && (strlen(&FindFileData.cFileName[0]) == 12) )
            {
				strncpy(Provider, &FindFileData.cFileName[strlen(FindFileData.cFileName)-8], 4);
				Provider[4] = '\0';
				newProvider = new string(Provider);
				m_NextviewProviders.push_back(newProvider);
            }
            RetVal = FindNextFile(hFindFile, &FindFileData);
        }
        FindClose(hFindFile);
    }

	strcpy(SearchFiles, Exe);
	c = strrchr(SearchFiles, '\\');
	if (c)
	{
		strcpy(c+1, "nxtvdb-*");
	}
	else
	{
		strcpy(SearchFiles, "nxtvdb-*");
	}

    hFindFile = FindFirstFile(SearchFiles, &FindFileData);

    if (hFindFile != INVALID_HANDLE_VALUE)
    {
        RetVal = TRUE;
        while(RetVal != 0)
        {
            // check * matchs 4 characters
            if(strlen(&FindFileData.cFileName[0]) == 11)
            {
				strncpy(Provider, &FindFileData.cFileName[strlen(FindFileData.cFileName)-4], 4);
				Provider[4] = '\0';
				newProvider = new string(Provider);
				m_NextviewProviders.push_back(newProvider);
            }
            RetVal = FindNextFile(hFindFile, &FindFileData);
        }
        FindClose(hFindFile);
    }

	return m_NextviewProviders.size();
}


// ---------------------- Settings ---------------------


static char		ExePath[MAX_PATH] = {0};
static char*	NextviewEPGExePath = NULL;
static long		EPG_DefaultSizePerc = 5;
static long		EPG_FrameDuration = 1;


SETTING EPGSettings[EPG_SETTING_LASTONE] =
{
    {
        "Text Size", SLIDER, 0, (long*)&EPG_DefaultSizePerc,
         5, 2, 7, 1, 1,
         NULL,
        "EPG", "DefaultSizePerc", NULL,
    },
    {
        "Time frame duration (hours)", SLIDER, 0, (long*)&EPG_FrameDuration,
         1, 1, 12, 1, 1,
         NULL,
        "EPG", "TimeFrameDuration", NULL,
    },
    {
        "nxtvepg.exe file path", CHARSTRING, 0, (long*)&NextviewEPGExePath,
         (long)ExePath, 0, 0, 0, 0,
         NULL,
        "EPG", "nxtvepg.exe", NULL,
    },
};


SETTING* EPG_GetSetting(EPG_SETTING Setting)
{
    if(Setting > -1 && Setting < EPG_SETTING_LASTONE)
    {
        return &(EPGSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void EPG_ReadSettingsFromIni()
{
    int i;

	GetModuleFileName (NULL, ExePath, sizeof(ExePath));
	*(strrchr(ExePath, '\\')) = '\0';
	strcat(ExePath, "\\nxtvepg.exe");

    for(i = 0; i < EPG_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(EPGSettings[i]));
    }

    if (NextviewEPGExePath == NULL)
    {
		Setting_SetValue(EPG_GetSetting(EPG_NXTVEPGPATH), (long)ExePath);
    }
}

void EPG_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < EPG_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(EPGSettings[i]), bOptimizeFileAccess);
    }
}

CTreeSettingsGeneric* EPG_GetTreeSettingsPage()
{
    return new CTreeSettingsGeneric("EPG Settings", EPGSettings, EPG_SETTING_LASTONE);
}

void EPG_FreeSettings()
{
    int i;
    for(i = 0; i < EPG_SETTING_LASTONE; i++)
    {
        Setting_Free(&EPGSettings[i]);
    }
}


// -----------------------------------------------------


extern "C"
{

int CheckProgrammeValidity(time_t StartTime, time_t EndTime, char * ChannelName)
{
	return 	MyEPG.CheckProgrammeValidity(StartTime, EndTime, ChannelName);
}

void AddProgramme(time_t StartTime, time_t EndTime, char * Title, char * ChannelName, char * SubTitle, char * Category, char * Description)
{
	MyEPG.AddProgramme(StartTime, EndTime, Title, ChannelName, SubTitle, Category, Description);
}

}
