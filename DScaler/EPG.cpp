/////////////////////////////////////////////////////////////////////////////
// $Id: EPG.cpp,v 1.14 2005-04-02 14:04:11 laurentg Exp $
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


#define	ONE_DAY			86400
#define	ONE_HOUR		3600
#define	DEFAULT_INPUT_FILE		"epg.xml"
#define	DEFAULT_OUTPUT_XML_FILE	"DScalerEPG.xml"
#define	DEFAULT_OUTPUT_TXT_FILE	"DScalerEPG.txt"
#define	DEFAULT_TMP_FILE		"DScalerEPG_tmp.txt"


CEPG MyEPG;


// TODO Enhancement: use a XML API


CProgramme::CProgramme(time_t StartTime, time_t EndTime, LPCSTR Title, LPCSTR ChannelName, LPCSTR ChannelEPGName, int ChannelNumber)
{
	m_StartTime = StartTime;
	m_EndTime = EndTime;
	m_Title = Title;
	m_ChannelName = ChannelName;
	m_ChannelEPGName = ChannelEPGName;
	m_ChannelNumber = ChannelNumber;
	m_SubTitle = "";
	m_Category = "";
	m_Description = "";
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
void CProgramme::GetProgrammeMainData(time_t *StartTime, time_t *EndTime, string &Channel, string &Title)
{
	*StartTime = m_StartTime;
	*EndTime = m_EndTime;
	Channel = m_ChannelName;
	Title = m_Title;
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
// Scan a XML file containing programmes and generate the corresponding DScaler data file
// The input file must be compatible with the XMLTV DTD
//
int CEPG::ScanXMLTVFile(LPCSTR file)
{
	string XMLTVExe = (char*)Setting_GetValue(EPG_GetSetting(EPG_XMLTVPATH));

	// If file not provided, try with "epg.xml"
	string XMLFile;
	if (file == NULL)
		XMLFile = m_FilesDir + "\\" + DEFAULT_INPUT_FILE;
	else
		XMLFile = file;

	//
	// Use tv_grep to select only channels defined in DScaler
	// Use tv_sort to correct the end times (if missing) and sort the programme by date and time
	//
	string RegExp = "\"(?i)";
	for (int i=0; (i < MyChannels.GetSize()); i++)
	{
		// Add an escape character "\" before each character of the channel name
		// that are special characters in PERL REGEXP (like + for example)
		LPCSTR name = MyChannels.GetChannelEPGName(i);
		char name2[64];
		for (int j=0, k=0; j<strlen(name); j++)
		{
			if (   (name[j] == '^')
				|| (name[j] == '$')
				|| (name[j] == '.')
				|| (name[j] == '*')
				|| (name[j] == '+')
				|| (name[j] == '?')
				|| (name[j] == '|')
				|| (name[j] == '(')
				|| (name[j] == ')')
				|| (name[j] == '[')
				|| (name[j] == ']')
				|| (name[j] == '{')
				|| (name[j] == '}')
				|| (name[j] == '\\'))
				name2[k++] = '\\';
			name2[k++] = name[j];
		}
		name2[k] = '\0';

		RegExp += "^";
		RegExp += name2;
		RegExp += "$";
		if (i < (MyChannels.GetSize() - 1))
			RegExp += "|";
	}
	RegExp += "\"";
	string OutputFile = m_FilesDir + "\\" + DEFAULT_OUTPUT_XML_FILE;
	string command = "\"" + m_CMDExe + "\" /C echo \"XMLTV file analysis ...\" && type \"" + XMLFile + "\" | \""
		+ XMLTVExe + "\" tv_grep --channel-name " + RegExp + " | \""
		+ XMLTVExe + "\" tv_sort --output \"" + OutputFile + "\"";

	// Use tv_to_text to convert the DScaler.xml file to a DScaler_tmp.txt file
	// TODO Suppress usage of tv_to_text as soon as XML API will be used
	string InputFile = m_FilesDir + "\\" + DEFAULT_OUTPUT_XML_FILE;
	OutputFile = m_FilesDir + "\\" + DEFAULT_TMP_FILE;
	command += " && type \"" + InputFile + "\" | \""
		+ XMLTVExe + "\" tv_to_text --output \"" + OutputFile + "\"";

	LOG(2, "XMLTV tv_grep/tv_sort/tv_to_text ... (%s)", command.c_str());
	int resu = ExecuteCommand(command);
	LOG(2, "XMLTV tv_grep/tv_sort/tv_to_text %d", resu);

	// TODO Suppress call to ConvertXMLtoTXT as soon as XML API will be used
	resu = ConvertXMLtoTXT();

	ReloadEPGData();

	return resu;
}


//
// Convert the DScaler_tmp.txt file to the DScaler.txt final file
//
// TODO Suppress ConvertXMLtoTXT as soon as XML API will be used
//
int CEPG::ConvertXMLtoTXT()
{
	int resu = -1;
	int delta_time = Setting_GetValue(EPG_GetSetting(EPG_SHIFTTIMES)) * 60;

	string InputFile = m_FilesDir + "\\" + DEFAULT_TMP_FILE;
	string OutputFile = m_FilesDir + "\\" + DEFAULT_OUTPUT_TXT_FILE;
    LOG(2, "XMLTV %s => %s ...", InputFile.c_str(), OutputFile.c_str());
	FILE *InFile = fopen(InputFile.c_str(), "r");
	FILE *OutFile = fopen(OutputFile.c_str(), "w");
	if (InFile != NULL && OutFile != NULL)
	{
		char Line[384];
		char *start;
		char *end;
		char *Time;
		char *Title;
		char *Channel;

		// Construct a default Date YYYY/MM/DD using the current date
		time_t DateTime;
		time(&DateTime);
		struct tm *date_tm = localtime(&DateTime);
		int year = date_tm->tm_year+1900;
		int month = date_tm->tm_mon+1;
		int day = date_tm->tm_mday;
		char Date1[11];
		char Date2[11];
		sprintf(Date1, "%04u/%02u/%02u", year, month, day);
		sprintf(Date2, "%04u/%02u/%02u", year, month, day);

		while (GetFileLine(InFile, Line, 383) == TRUE)
		{
			Time = NULL;
			Title = NULL;
			Channel = NULL;
			start = Line;
			end = strchr(start, '-');
			if (!end)
				continue;
			if ((end-start) == 2)
			{
				*end = '\0';
				month = atoi(start);
				start = end+1;
				end = strchr(start, ' ');
				if (!end || ((end- start) != 2))
					continue;
				*end = '\0';
				day = atoi(start);
				sprintf(Date1, "%04u/%02u/%02u", year, month, day);
				sprintf(Date2, "%04u/%02u/%02u", year, month, day);
				date_tm->tm_year = year-1900;
				date_tm->tm_mon = month-1;
				date_tm->tm_mday = day;
				DateTime = mktime(date_tm);
				continue;
			}
			start = Line;
			end = strchr(start, '\t');
			if (!end)
				continue;
			Time = start;
			*end = '\0';
			start = end+1;
			end = strchr(start, '\t');
			if (!end)
				continue;
			Title = start;
			*end = '\0';
			Channel = end+1;
			if (!*Time || !*Title || !*Channel)
				continue;
			if (!IsValidChannelName(Channel))
				continue;
			if (strlen(Time) != 12)
				continue;
			start = Time;
			end = &Time[7];
			Time[5] = '\0';
			if (delta_time != 0)
			{
				int hour1;
				int hour2;
				int hour;
				int min;

				start[2] = '\0';
				hour1 = hour = atoi(start);
				min = atoi(&start[3]);
				min += delta_time;
				if (min >= 60)
				{
					hour += (min / 60);
					min %= 60;
				}
				else if (min < 0)
				{
					min *= -1;
					if ((min % 60) == 0)
					{
						hour -= (min / 60);
						min = 0;
					}
					else
					{
						hour -= ((min / 60) + 1);
						min = 60 - (min % 60);
					}
				}
				if (hour >= 24)
				{
					time_t NextDay = DateTime + ONE_DAY * (hour / 24);
					date_tm = localtime(&NextDay);
					hour %= 24;
				}
				else if (hour < 0)
				{
					time_t PrevDay;
					if ((-hour % 24) == 0)
					{
						PrevDay = DateTime - ONE_DAY * (-hour / 24);
						hour = 0;
					}
					else
					{
						PrevDay = DateTime - ONE_DAY * ((-hour / 24) + 1);
						hour = 24 - (-hour % 24);
					}
					date_tm = localtime(&PrevDay);
				}
				else
				{
					date_tm = localtime(&DateTime);
				}
				year = date_tm->tm_year+1900;
				month = date_tm->tm_mon+1;
				day = date_tm->tm_mday;
				sprintf(Date1, "%04u/%02u/%02u", year, month, day);
				sprintf(start, "%02u:%02u", hour, min);

				end[2] = '\0';
				hour2 = hour = atoi(end);
				min = atoi(&end[3]);
				min += delta_time;
				if (min >= 60)
				{
					hour += (min / 60);
					min %= 60;
				}
				else if (min < 0)
				{
					min *= -1;
					if ((min % 60) == 0)
					{
						hour -= (min / 60);
						min = 0;
					}
					else
					{
						hour -= ((min / 60) + 1);
						min = 60 - (min % 60);
					}
				}
				if (hour >= 24)
				{
					time_t NextDay = DateTime + ONE_DAY * (hour / 24);
					date_tm = localtime(&NextDay);
					hour %= 24;
				}
				else if (hour < 0)
				{
					time_t PrevDay;
					if ((-hour % 24) == 0)
					{
						PrevDay = DateTime - ONE_DAY * (-hour / 24);
						hour = 0;
					}
					else
					{
						PrevDay = DateTime - ONE_DAY * ((-hour / 24) + 1);
						hour = 24 - (-hour % 24);
					}
					date_tm = localtime(&PrevDay);
				}
				else
				{
					if (hour1 > hour2)
					{
						time_t NextDay = DateTime + ONE_DAY;
						date_tm = localtime(&NextDay);
					}
					else
					{
						date_tm = localtime(&DateTime);
					}
				}
				year = date_tm->tm_year+1900;
				month = date_tm->tm_mon+1;
				day = date_tm->tm_mday;
				sprintf(Date2, "%04u/%02u/%02u", year, month, day);
				sprintf(end, "%02u:%02u", hour, min);
			}
			else
			{
				start[2] = '\0';
				int hour1 = atoi(start);
				start[2] = ':';
				end[2] = '\0';
				int hour2 = atoi(end);
				end[2] = ':';
				if (hour1 > hour2)
				{
					time_t NextDay = DateTime + ONE_DAY;
					date_tm = localtime(&NextDay);
				}
				else
				{
					date_tm = localtime(&DateTime);
				}
				year = date_tm->tm_year+1900;
				month = date_tm->tm_mon+1;
				day = date_tm->tm_mday;
				sprintf(Date2, "%04u/%02u/%02u", year, month, day);
			}
			fprintf(OutFile, "%s\t%s\t%s\t%s\t%s\t%s\n", Date1, start, Date2, end, Title, Channel);
		}
		resu = 0;
	}
	if (InFile != NULL)
		fclose(InFile);
	if (OutFile != NULL)
		fclose(OutFile);
	unlink(InputFile.c_str());

	return resu;
}


//
// Load the DScaler EPG data for the programmes between two dates
// If DateMin and DateMax are not set, load the EPG data for
// the interval [current time - 2 hours, current time + 6 hours]
//
// TODO Rewrite LoadEPGData as soon as XML API will be used
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

	if ( (m_LoadedTimeMin == 0) || (DateMin < m_LoadedTimeMin) )
		m_LoadedTimeMin = DateMin;
	if ( (m_LoadedTimeMax == 0) || (DateMax > m_LoadedTimeMax) )
		m_LoadedTimeMax = DateMax;

	//
	// Read the file DScaler.txt
	//
	string InputFile = m_FilesDir + "\\" + DEFAULT_OUTPUT_TXT_FILE;
	FILE *InFile = fopen(InputFile.c_str(), "r");
	if (InFile == NULL)
		return -1;

	char Line[384];
	char *start;
	char *end;
	char *StartDate;
	char *StartTime;
	char *EndDate;
	char *EndTime;
	char *Title;
	char *Channel;
	LPCSTR ChannelName;
	int  ChannelNumber;
	while (GetFileLine(InFile, Line, 383) == TRUE)
	{
		StartDate = NULL;
		StartTime = NULL;
		EndDate = NULL;
		EndTime = NULL;
		Title = NULL;
		Channel = NULL;
		start = Line;
		end = strchr(start, '\t');
		if (!end)
			continue;
		StartDate = start;
		*end = '\0';
		start = end+1;
		end = strchr(start, '\t');
		if (!end)
			continue;
		StartTime = start;
		*end = '\0';
		start = end+1;
		end = strchr(start, '\t');
		if (!end)
			continue;
		EndDate = start;
		*end = '\0';
		start = end+1;
		end = strchr(start, '\t');
		if (!end)
			continue;
		EndTime = start;
		*end = '\0';
		start = end+1;
		end = strchr(start, '\t');
		if (!end)
			continue;
		Title = start;
		*end = '\0';
		Channel = end+1;
		if (!*StartDate || !*StartTime || !*EndDate || !*EndTime || !*Title || !*Channel)
			continue;
		if (!IsValidChannelName(Channel, &ChannelName, &ChannelNumber))
			continue;

		int year;
		int month;
		int day;
		int hour;
		int min;
		struct tm *date_tm;

		StartDate[4] = '\0';
		StartDate[7] = '\0';
		year = atoi(StartDate);
		month = atoi(&StartDate[5]);
		day = atoi(&StartDate[8]);
		StartTime[2] = '\0';
		hour = atoi(StartTime);
		min = atoi(&StartTime[3]);

		date_tm = localtime(&DateMin);
		date_tm->tm_year = year-1900;
		date_tm->tm_mon = month-1;
		date_tm->tm_mday = day;
		date_tm->tm_hour = hour;
		date_tm->tm_min = min;
		date_tm->tm_sec = 0;
		time_t TimeStart = mktime(date_tm);

		EndDate[4] = '\0';
		EndDate[7] = '\0';
		year = atoi(EndDate);
		month = atoi(&EndDate[5]);
		day = atoi(&EndDate[8]);
		EndTime[2] = '\0';
		hour = atoi(EndTime);
		min = atoi(&EndTime[3]);

		date_tm = localtime(&DateMin);
		date_tm->tm_year = year-1900;
		date_tm->tm_mon = month-1;
		date_tm->tm_mday = day;
		date_tm->tm_hour = hour;
		date_tm->tm_min = min;
		date_tm->tm_sec = 0;
		time_t TimeEnd = mktime(date_tm);

		// Keep only programmes scheduled between DateMin and DateMax
		if ( (TimeEnd <= DateMin) || (TimeStart > DateMax) )
			continue;

		AddProgramme(TimeStart, TimeEnd, Title, ChannelName, Channel, ChannelNumber);
	}

	fclose(InFile);

	return 0;
}


int CEPG::ReloadEPGData()
{
	return LoadEPGData(m_LoadedTimeMin, m_LoadedTimeMax);
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


BOOL CEPG::GetProgrammeMainData(int Index, time_t *StartTime, time_t *EndTime, string &Channel, string &Title)
{
	if ( (Index < 0) && (m_ProgrammeSelected == NULL))
		return FALSE;

	if ( (Index >= 0) && (Index >= m_ProgrammesSelection.size()) )
		return FALSE;

	if (Index < 0)
	{
		m_ProgrammeSelected->GetProgrammeMainData(StartTime, EndTime, Channel, Title);
	}
	else
	{
		m_ProgrammesSelection[Index]->GetProgrammeMainData(StartTime, EndTime, Channel, Title);
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
			ScanXMLTVFile(FilePath);
		}
        return TRUE;
		break;

	case IDM_IMPORT_NEXTVIEW:
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
		// DumpEPGData();
        return TRUE;
		break;

	case IDM_DISPLAY_EPG:
		if (m_UseProgFronBrowser == FALSE)
		{
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

	if (m_Programmes.size() > 0)
	{
		CSource *CurrentSource = Providers_GetCurrentSource();
		if (CurrentSource && Providers_GetCurrentSource()->IsInTunerMode())
		{
			// Search EPG info for the currently viewed channel
			if (MyEPG.SearchForProgramme(Channel_GetName(), TimeNow) == TRUE)
			{
				// Display the OSD screen
				OSD_ShowInfosScreen(1, Setting_GetValue(EPG_GetSetting(EPG_PERCENTAGESIZE)));
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


void CEPG::AddProgramme(time_t StartTime, time_t EndTime, LPCSTR Title, LPCSTR ChannelName, LPCSTR ChannelEPGName, int ChannelNumber)
{
    CProgramme* newProgramme = new CProgramme(StartTime, EndTime, Title, ChannelName, ChannelEPGName, ChannelNumber);
	m_Programmes.push_back(newProgramme);
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
// Retrieve a line of text in a file (stream)
//
BOOL CEPG::GetFileLine(FILE *Stream, char *Buffer, int MaxLen)
{
	int pos=0;
	int i, ch;

	ch = fgetc(Stream);
	for (i=0 ; (i<MaxLen) && (feof(Stream) == 0) ; i++)
	{
		if  (((char)ch) == '\n')
		{
			Buffer[i] = '\0';
			return TRUE;
		}
		Buffer[i] = (char)ch;
		ch = fgetc(Stream);
	}
	Buffer[i] = '\0';

	return (i>0) ? TRUE : FALSE;
}


// ---------------------- Settings ---------------------


static char		ExePath[MAX_PATH] = {0};
static char*	XMLTVExePath = NULL;
static long		EPG_DefaultSizePerc = 5;
static long		EPG_ShiftTimes = 0;
static long		EPG_FrameDuration = 1;


SETTING EPGSettings[EPG_SETTING_LASTONE] =
{
    {
        "xmltv.exe file path", CHARSTRING, 0, (long*)&XMLTVExePath,
         (long)ExePath, 0, 0, 0, 0,
         NULL,
        "EPG", "xmltv.exe", NULL,
    },
    {
        "Text Size", SLIDER, 0, (long*)&EPG_DefaultSizePerc,
         5, 2, 7, 1, 1,
         NULL,
        "EPG", "DefaultSizePerc", NULL,
    },
    {
        "Shift times during import (hours)", SLIDER, 0, (long*)&EPG_ShiftTimes,
         0, -12, 12, 1, 1,
         NULL,
        "EPG", "ImportShiftTimes", NULL,
    },
    {
        "Time frame duration (hours)", SLIDER, 0, (long*)&EPG_FrameDuration,
         1, 1, 12, 1, 1,
         NULL,
        "EPG", "TimeFrameDuration", NULL,
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
    struct stat st;

	GetModuleFileName (NULL, ExePath, sizeof(ExePath));
	*(strrchr(ExePath, '\\')) = '\0';
	strcat(ExePath, "\\xmltv.exe");

    for(i = 0; i < EPG_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(EPGSettings[i]));
    }

    if ((XMLTVExePath == NULL) || stat(XMLTVExePath, &st))
    {
        LOG(1, "Incorrect path for xmltv.exe; using %s", ExePath);
		Setting_SetValue(EPG_GetSetting(EPG_XMLTVPATH), (long)ExePath);
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
