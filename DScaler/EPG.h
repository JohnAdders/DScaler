/////////////////////////////////////////////////////////////////////////////
// $Id: EPG.h,v 1.4 2005-03-26 18:53:23 laurentg Exp $
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
// Revision 1.3  2005/03/21 22:39:15  laurentg
// EPG: changes regarding OSD
//
// Revision 1.2  2005/03/20 22:56:22  laurentg
// New OSD screens added for EPG
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

#ifndef __EPG_H___
#define __EPG_H___


#include <stdio.h>
#include "Channels.h"


class CProgram
{
public:
	CProgram(time_t StartTime, time_t EndTime, LPCSTR Title, LPCSTR Channel);
	~CProgram();

	// Check whether the program matchs the channel (if provided)
	// and overlaps the period of time defined by DateMin and DateMax
	BOOL IsProgramMatching(time_t DateMin, time_t DateMax, LPCSTR Channel=NULL);

	// Get the program main data : start and end time + title
	void GetProgramMainData(string &Start, string &End, string &Channel, string &Title);

	// Get the program start and end times
	void CProgram::GetProgramTimes(time_t *StartTime, time_t *EndTime);

	// Dump the program main data : start and end time + channel + title
	void DumpProgramMainData();

private:
	time_t	m_StartTime;
	time_t	m_EndTime;
	string	m_Channel;
	string	m_Title;
	string	m_SubTitle;
	string	m_Category;
	string	m_Description;
	DWORD	m_Length;		// In minutes
};


class CEPG
{
public:
	CEPG();
	~CEPG();

	// Scan a XML file containing programs and generate the corresponding DScaler data files
	// The input file must be compatible with the XMLTV DTD
	// TODO Suppress the parameter delta_time as soon as XML API will be used
	int ScanXMLTVFile(LPCSTR file=NULL, int delta_time=0);

	// Load the DScaler EPG data for the programs between two dates
	// If DateMin and DateMax are not set, load the EPG data for
	// the interval [current time - 2 hours, current time + 6 hours]
	// TODO Rewrite LoadEPGData as soon as XML API will be used
	int LoadEPGData(time_t DateMin=0, time_t DateMax=0);
	int ReloadEPGData();

	// Indicate if EPG programs are available
	BOOL IsEPGAvailable();

	// Get the EPG data of the first program matching time and channel
	int GetEPGData(time_t *StartTime, time_t *EndTime, string &Title, LPCSTR Channel, time_t Date);

	void SetSearchContext(LPCSTR ChannelName, LPCSTR ChannelEPGName, time_t TimeMin, time_t TimeMax);
	void GetSearchContext(LPCSTR *ChannelName, time_t *TimeMin, time_t *TimeMax);
	int SearchForPrograms();
	int GetProgramData(int Index, time_t *StartTime, string &StartTimeStr, time_t *EndTime, string &EndTimeStr, string &Channel, string &Title);

	BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam);

	void CEPG::ShowOSD();

	// Dump the EPG data
	void DumpEPGData();

	void ClearPrograms();
	void AddProgram(time_t StartTime, time_t EndTime, LPCSTR Title, LPCSTR Channel);

private:
	// Convert the DScaler_tmp.txt file to the DScaler.txt final file
	// TODO Suppress ConvertXMLtoTXT as soon as XML API will be used
	int ConvertXMLtoTXT(int delta_time=0);
	
	// Execute a command using the Windows command interpreter
	int ExecuteCommand(string command);

	// Retrieve a line of text in a file (stream)
	BOOL GetFileLine(FILE *Stream, char *Buffer, int MaxLen);

	// Check whether a channel name belongs to the list of channels
	// defined in DScaler
	BOOL IsValidChannelName(LPCSTR Name);

    typedef vector<CProgram*> CPrograms;

    CPrograms	m_Programs;
    CPrograms	m_ProgramsSelection;

	string		m_CMDExe;
	string		m_FilesDir;
	string		m_XMLTVExe;
	LPCSTR		m_SearchName;
	LPCSTR		m_SearchEPGName;
	time_t		m_LoadedTimeMin;
	time_t		m_LoadedTimeMax;
	time_t		m_SearchTimeMin;
	time_t		m_SearchTimeMax;
};


extern CUserChannels MyChannels;
extern CEPG MyEPG;


#endif