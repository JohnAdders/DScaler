/////////////////////////////////////////////////////////////////////////////
// $Id: EPG.h,v 1.19 2005-07-19 21:41:54 laurentg Exp $
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
// Revision 1.18  2005/07/11 12:49:00  laurentg
// New menus to browse EPG at a certain day and time
//
// Revision 1.17  2005/07/09 13:43:43  laurentg
// Two new EPG settings + Possibility to display next and previous programmes info
//
// Revision 1.16  2005/07/06 20:27:54  laurentg
// Copy the file if source different from destination
//
// Revision 1.15  2005/07/06 19:40:44  laurentg
// New EPG code using Tom Zoerner's XMLTV parser
//
// Revision 1.14  2005/04/09 12:49:49  laurentg
// EPG: choose the NextviewEPG provider
//
// Revision 1.13  2005/04/07 23:17:28  laurentg
// EPG: import NextviewEPG database
//
// Revision 1.12  2005/04/02 14:04:12  laurentg
// EPG: navigation between the browser view and the programme view improved
//
// Revision 1.11  2005/04/01 22:16:33  laurentg
// EPG: new menu "Hide EPG" + new setting to define the time frame duration
//
// Revision 1.10  2005/03/29 21:08:34  laurentg
// program renamed programme
//
// Revision 1.9  2005/03/28 17:48:10  laurentg
// Navigation into EPG + change of channel
//
// Revision 1.8  2005/03/28 13:42:02  laurentg
// EPG: preparation for when new data (category, sub-title, description) will be available
//
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


class CProgramme
{
public:
	CProgramme(time_t StartTime, time_t EndTime, LPCSTR Title, LPCSTR ChannelName, LPCSTR ChannelEPGName, int ChannelNumber, LPCSTR SubTitle, LPCSTR Category, LPCSTR Description);
	~CProgramme();

	// Check whether the programme matchs the channel (if provided)
	// and overlaps the period of time defined by DateMin and DateMax
	BOOL IsProgrammeMatching(time_t DateMin, time_t DateMax, LPCSTR Channel=NULL);

	// Check whether the programme matchs the channel
	BOOL IsProgrammeMatching(LPCSTR Channel=NULL);

	// Get the programme dates : start and end time
	void GetProgrammeDates(time_t *StartTime, time_t *EndTime);

	// Get the channel data : DScaler name + EPG name + number
	void GetProgrammeChannelData(string &ChannelName, string &ChannelEPGName, int *ChannelNumber);

	// Get the programme main data : start and end time + title
	void GetProgrammeMainData(time_t *StartTime, time_t *EndTime, string &Channel, string &Title, string &Category);

	// Get all the programme data
	void GetProgrammeData(time_t *StartTime, time_t *EndTime, string &Channel, string &Title, string &SubTitle, string &Category, string &Description);

	// Dump the programme main data : start and end time + channel + title
	void DumpProgrammeMainData();

private:
	time_t	m_StartTime;
	time_t	m_EndTime;
	string	m_ChannelName;
	string	m_ChannelEPGName;
	int		m_ChannelNumber;
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

	// Copy the input file in DScalerEPG.xml
	void ImportXMLTVFile(LPCSTR file);

	// Import the NextviewEPG database
	// Put the result in DScalerEPG.xml
	void ImportNxtvepgEPGDB(LPCSTR Provider);

	// Load the DScaler EPG data for the programmes between two dates
	// If DateMin and DateMax are not set, load the EPG data for
	// the interval [current time - 2 hours, current time + 6 hours]
	void LoadEPGData(time_t DateMin=0, time_t DateMax=0);
	void ReloadEPGData();

	int GetSearchContext(LPCSTR *ChannelName, time_t *TimeMin, time_t *TimeMax);
	BOOL SearchForProgramme(string &Channel);
	BOOL SearchForProgramme(LPCSTR ChannelName, time_t ThatTime, int PrevNextProg = 0);
	int SearchForProgrammes(LPCSTR ChannelName, time_t TimeMin, time_t TimeMax);
	BOOL GetProgrammeChannelData(int Index, string &ChannelName, string &ChannelEPGName, int *ChannelNumber);
	BOOL GetProgrammeMainData(int Index, time_t *StartTime, time_t *EndTime, string &Channel, string &Title, string &Category);
	BOOL GetProgrammeData(int Index, time_t *StartTime, time_t *EndTime, string &Channel, string &Title, string &SubTitle, string &Category, string &Description);

	void SetDisplayIndexes(int IdxMin, int IdxMax, int IdxCur);
	int GetDisplayIndexes(int *IdxMin, int *IdxMax, int *IdxCur);
	int GetDisplayLineShift(int NbLines);

	BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam);

	void ShowOSD(int PrevNextProg = 0);
	void HideOSD();

	// Dump the EPG data
	void DumpEPGData();

	void ClearProgrammes();
	int CheckProgrammeValidity(time_t StartTime, time_t EndTime, LPCSTR ChannelName);
	void AddProgramme(time_t StartTime, time_t EndTime, LPCSTR Title, LPCSTR ChannelName, LPCSTR ChannelEPGName, int ChannelNumber);
	void AddProgramme(time_t StartTime, time_t EndTime, LPCSTR Title, LPCSTR ChannelName, LPCSTR ChannelEPGName, int ChannelNumber, LPCSTR SubTitle, LPCSTR Category, LPCSTR Description);
	void AddProgramme(time_t StartTime, time_t EndTime, LPCSTR Title, LPCSTR ChannelEPGName, LPCSTR SubTitle, LPCSTR Category, LPCSTR Description);

	void SetMenu(HMENU hMenu);

private:
	// Execute a command using the Windows command interpreter
	int ExecuteCommand(string command);

	// Check if new EPG data have to be loaded
	BOOL LoadEPGDataIfNeeded(time_t TimeMin, time_t TimeMax, int DeltaEarlier, int DeltaLater);

	// Copy a file
	BOOL CopyFile(LPCSTR InPath, LPCSTR OutPath);

	// Check whether a channel name belongs to the list of channels
	// defined in DScaler
	BOOL IsValidChannelName(LPCSTR EPGName, LPCSTR *Name=NULL, int *Number=NULL);

	// Insert a new programme in the list keeping an order by dates of the programmes
	void InsertProgramme(CProgramme* NewProg);

	void ClearNextviewEPGProviders();
	int GetNextviewEPGProviders();

    typedef vector<CProgramme*> CProgrammes;

    CProgrammes	m_Programmes;
    CProgrammes	m_ProgrammesSelection;
	CProgramme*	m_ProgrammeSelected;

    typedef vector<string*> strings;

	strings		m_NextviewProviders;

	string		m_CMDExe;
	string		m_FilesDir;

	time_t		m_LoadedTimeMin;
	time_t		m_LoadedTimeMax;

	LPCSTR		m_SearchChannel;
	time_t		m_SearchTimeMin;
	time_t		m_SearchTimeMax;
	int			m_IdxShowSelectMin;
	int			m_IdxShowSelectMax;
	int			m_IdxShowSelectCur;
	BOOL		m_UseProgFronBrowser;
	int			m_PrevNextProg;
	int			m_ShiftLines;

	int			m_Displayed;
};


SETTING* EPG_GetSetting(EPG_SETTING Setting);
void EPG_ReadSettingsFromIni();
void EPG_WriteSettingsToIni(BOOL bOptimizeFileAccess);
CTreeSettingsGeneric* EPG_GetTreeSettingsPage();
void EPG_FreeSettings();


extern CUserChannels MyChannels;
extern CEPG MyEPG;


#endif