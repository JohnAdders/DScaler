/////////////////////////////////////////////////////////////////////////////
// $Id: EPG.h,v 1.8 2005-03-28 13:42:02 laurentg Exp $
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
// Revision 1.7  2005/03/28 13:11:16  laurentg
// New EPG setting to shift times during import
//
// Revision 1.6  2005/03/28 12:53:20  laurentg
// EPG: previous and next page to show programs
//
// Revision 1.5  2005/03/27 20:22:20  laurentg
// EPG: new improvements
//
// Revision 1.4  2005/03/26 18:53:23  laurentg
// EPG code improved
// => possibility to set the EPG channel name in the channel setup dialog box
// => automatic loading of new data when needed
// => OSD scrrens updated
// => first step for programs "browser"
//
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
	void GetProgramMainData(time_t *StartTime, time_t *EndTime, string &Channel, string &Title);

	// Get all the program data
	void GetProgramData(time_t *StartTime, time_t *EndTime, string &Channel, string &Title, string &SubTitle, string &Category, string &Description);

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
	int ScanXMLTVFile(LPCSTR file=NULL);

	// Load the DScaler EPG data for the programs between two dates
	// If DateMin and DateMax are not set, load the EPG data for
	// the interval [current time - 2 hours, current time + 6 hours]
	// TODO Rewrite LoadEPGData as soon as XML API will be used
	int LoadEPGData(time_t DateMin=0, time_t DateMax=0);
	int ReloadEPGData();

	int GetSearchContext(LPCSTR *ChannelName, time_t *TimeMin, time_t *TimeMax);
	int SearchForPrograms(LPCSTR ChannelName, time_t TimeMin, time_t TimeMax);
	int GetProgramMainData(int Index, time_t *StartTime, time_t *EndTime, string &Channel, string &Title);
	int GetProgramData(int Index, time_t *StartTime, time_t *EndTime, string &Channel, string &Title, string &SubTitle, string &Category, string &Description);

	void SetDisplayIndexes(int IdxMin, int IdxMax);
	int GetDisplayIndexes(int *IdxMin, int *IdxMax);

	BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam);

	void ShowOSD();

	// Dump the EPG data
	void DumpEPGData();

	void ClearPrograms();
	void AddProgram(time_t StartTime, time_t EndTime, LPCSTR Title, LPCSTR Channel);

private:
	// Convert the DScaler_tmp.txt file to the DScaler.txt final file
	// TODO Suppress ConvertXMLtoTXT as soon as XML API will be used
	int ConvertXMLtoTXT();
	
	// Execute a command using the Windows command interpreter
	int ExecuteCommand(string command);

	// Check if new EPG data have to be loaded
	BOOL LoadEPGDataIfNeeded(time_t TimeMin, time_t TimeMax, int DeltaEarlier, int DeltaLater);

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

	time_t		m_LoadedTimeMin;
	time_t		m_LoadedTimeMax;

	LPCSTR		m_SearchChannel;
	time_t		m_SearchTimeMin;
	time_t		m_SearchTimeMax;
	int			m_IdxShowSelectMin;
	int			m_IdxShowSelectMax;
};


SETTING* EPG_GetSetting(EPG_SETTING Setting);
void EPG_ReadSettingsFromIni();
void EPG_WriteSettingsToIni(BOOL bOptimizeFileAccess);
CTreeSettingsGeneric* EPG_GetTreeSettingsPage();
void EPG_FreeSettings();


extern CUserChannels MyChannels;
extern CEPG MyEPG;


#endif