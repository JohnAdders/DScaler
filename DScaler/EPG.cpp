/////////////////////////////////////////////////////////////////////////////
// $Id: EPG.cpp,v 1.2 2005-03-20 16:56:26 laurentg Exp $
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
#include "DScaler.h"


#define	ONE_DAY			86400
#define	ONE_HOUR		3600
#define	DEFAULT_INPUT_FILE		"epg.xml"
#define	DEFAULT_OUTPUT_XML_FILE	"DScalerEPG.xml"
#define	DEFAULT_OUTPUT_TXT_FILE	"DScalerEPG.txt"
#define	DEFAULT_TMP_FILE		"DScalerEPG_tmp.txt"


CEPG MyEPG;


// TODO Enhancement: display of EPG information in the OSD

// TODO Enhancement: allow linking a DScaler channel name to one or several EPG channel names
// That could be done with a new field added in the program.txt file + a new field in the
// channel setup dialog box to enter this information

// TODO Enhancement: use a XML API


CProgram::CProgram(time_t StartTime, time_t EndTime, LPCSTR Title, LPCSTR Channel)
{
	m_StartTime = StartTime;
	m_EndTime = EndTime;
	m_Title = Title;
	m_Channel = Channel;
	m_SubTitle = "";
	m_Category = "";
	m_Description = "";
	m_Length = 0;
}


CProgram::~CProgram()
{
}


//
// Check whether the program matchs time and channel
//
BOOL CProgram::IsProgramMatching(time_t DateTime, LPCSTR Channel)
{
	if (   (DateTime >= m_StartTime)
		&& (DateTime < m_EndTime)
		&& !_stricmp(Channel, m_Channel.c_str())
	   )
	   return TRUE;

	return FALSE;
}


//
// Get the program main data : start and end time + title
//
void CProgram::GetProgramMainData(string &Start, string &End, string &Title)
{
	char date[6];
	struct tm *date_tm;
	date_tm = localtime(&m_StartTime);
	sprintf(date, "%02u:%02u", date_tm->tm_hour, date_tm->tm_min);
	Start = date;
	date_tm = localtime(&m_EndTime);
	sprintf(date, "%02u:%02u", date_tm->tm_hour, date_tm->tm_min);
	End = date;
	Title = m_Title;
}


//
// Dump the program main data : start and end time + channel + title
//
void CProgram::DumpProgramMainData()
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
		Date1, Date2, m_Channel.c_str(), m_Title.c_str());
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
	m_XMLTVExe = m_FilesDir + "\\xmltv.exe";
}


CEPG::~CEPG()
{
	ClearPrograms();
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
// Scan a XML file containing programs and generate the corresponding DScaler data file
// The input file must be compatible with the XMLTV DTD
//
// TODO Suppress the parameter delta_time as soon as XML API will be used
//
int CEPG::ScanXMLTVFile(LPCSTR file, int delta_time)
{
	// If file not provided, try with "epg.xml"
	string XMLFile;
	if (file == NULL)
		XMLFile = m_FilesDir + "\\" + DEFAULT_INPUT_FILE;
	else
		XMLFile = file;

	//
	// Use tv_grep to select only channels defined in DScaler
	// Use tv_sort to correct the end times (if missing) and sort the program by date and time
	//
	string RegExp = "\"(?i)";
	for (int i=0; (i < MyChannels.GetSize()); i++)
	{
		// Add an escape character "\" before each character of the channel name
		// that are special characters in PERL REGEXP (like + for example)
		LPCSTR name = MyChannels.GetChannelName(i);
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
		+ m_XMLTVExe + "\" tv_grep --channel-name " + RegExp + " | \""
		+ m_XMLTVExe + "\" tv_sort --output \"" + OutputFile + "\"";

	// Use tv_to_text to convert the DScaler.xml file to a DScaler_tmp.txt file
	// TODO Suppress usage of tv_to_text as soon as XML API will be used
	string InputFile = m_FilesDir + "\\" + DEFAULT_OUTPUT_XML_FILE;
	OutputFile = m_FilesDir + "\\" + DEFAULT_TMP_FILE;
	command += " && type \"" + InputFile + "\" | \""
		+ m_XMLTVExe + "\" tv_to_text --output \"" + OutputFile + "\"";

	LOG(2, "XMLTV tv_grep/tv_sort/tv_to_text ... (%s)", command.c_str());
	int resu = ExecuteCommand(command);
	LOG(2, "XMLTV tv_grep/tv_sort/tv_to_text %d", resu);

	// TODO Suppress call to ConvertXMLtoTXT as soon as XML API will be used
	resu = ConvertXMLtoTXT(delta_time);

	LoadEPGData();

	return resu;
}


//
// Convert the DScaler_tmp.txt file to the DScaler.txt final file
//
// TODO Suppress ConvertXMLtoTXT as soon as XML API will be used
//
int CEPG::ConvertXMLtoTXT(int delta_time)
{
	int resu = -1;

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
// Load the DScaler EPG data for the programs between two dates
// If DateMin and DateMax are not set, load the EPG data for
// the interval [current time - 2 hours, current time + one day]
//
// TODO Rewrite LoadEPGData as soon as XML API will be used
//
int CEPG::LoadEPGData(time_t DateMin, time_t DateMax)
{
	ClearPrograms();

	time_t DateTime;

	// If dates not provided, load the data for the interval
	// [current time - 2 hours, current time + one day]
	if ( (DateMin == 0) || (DateMax == 0) )
	{
		time(&DateTime);
		DateMin = DateTime - 2 * ONE_HOUR;
		DateMax = DateTime + ONE_DAY;
	}

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

		// Keep only programs scheduled between DateMin and DateMax
		if ( (TimeEnd <= DateMin) || (TimeStart > DateMax) )
			continue;

		AddProgram(TimeStart, TimeEnd, Title, Channel);
	}

	fclose(InFile);

	return 0;
}


//
// Get the EPG data of the program matching time and channel
//
int CEPG::GetEPGData(string &StartTime, string &EndTime, string &Title, LPCSTR Channel, time_t Date)
{
	// If date not provided, use the current day
	time_t DateTime;
	if (Date == 0)
		time(&DateTime);
	else
		DateTime = Date;

	// If channel not provided, use the current one
	string Ch;
	if (Channel == NULL)
		Ch = Channel_GetName();
	else
		Ch = Channel;

	// Search the program overlapping the date/time set in DateTime
	// for the channel set in Ch
    for(CPrograms::iterator it = m_Programs.begin();
        it != m_Programs.end();
        ++it)
    {
		if ((*it)->IsProgramMatching(DateTime, Ch.c_str()) == TRUE)
		{
			string test;
			(*it)->GetProgramMainData(StartTime, EndTime, Title);
			return 0;
		}
    }
	return -1;
}


//
// Dump the EPG data
//
void CEPG::DumpEPGData()
{
    for(CPrograms::iterator it = m_Programs.begin();
        it != m_Programs.end();
        ++it)
    {
		(*it)->DumpProgramMainData();
    }
}


void CEPG::ClearPrograms()
{
    for(CPrograms::iterator it = m_Programs.begin();
        it != m_Programs.end();
        ++it)
    {
        delete (*it);
    }
    m_Programs.clear();
}


void CEPG::AddProgram(time_t StartTime, time_t EndTime, LPCSTR Title, LPCSTR Channel)
{
    CProgram* newProgram = new CProgram(StartTime, EndTime, Title, Channel);
	m_Programs.push_back(newProgram);
}


//
// Check whether a channel name belongs to the list of channels
// defined in DScaler
//
BOOL CEPG::IsValidChannelName(LPCSTR Name)
{
	for (int i=0; (i < MyChannels.GetSize()); i++)
	{
		if (!_stricmp(Name, MyChannels.GetChannelName(i)))
			return TRUE;
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
