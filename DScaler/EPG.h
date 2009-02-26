/////////////////////////////////////////////////////////////////////////////
// $Id$
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

#ifndef __EPG_H___
#define __EPG_H___


#include <stdio.h>
#include "Channels.h"


class CProgramme
{
public:
    CProgramme(time_t StartTime, time_t EndTime, LPCSTR Title, LPCSTR ChannelName, LPCSTR ChannelEPGName, int ChannelNumber, LPCSTR SubTitle, LPCSTR Category, LPCSTR Description);
    ~CProgramme();

    // Check whether the programme matchs the channel
    // and overlaps the period of time defined by DateMin and DateMax
    BOOL IsProgrammeMatching(time_t DateMin, time_t DateMax, LPCSTR Channel);

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
    time_t    m_StartTime;
    time_t    m_EndTime;
    string    m_ChannelName;
    string    m_ChannelEPGName;
    int        m_ChannelNumber;
    string    m_Title;
    string    m_SubTitle;
    string    m_Category;
    string    m_Description;
    DWORD    m_Length;        // In minutes
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
    BOOL SearchForProgramme(LPCSTR ChannelName, time_t ThatTime);
    int SearchForProgrammes(LPCSTR ChannelName, time_t TimeMin=0, time_t TimeMax=0);
    BOOL GetProgrammeChannelData(int Index, string &ChannelName, string &ChannelEPGName, int *ChannelNumber);
    BOOL GetProgrammeMainData(int Index, time_t *StartTime, time_t *EndTime, string &Channel, string &Title, string &Category);
    BOOL GetProgrammeData(int Index, time_t *StartTime, time_t *EndTime, string &Channel, string &Title, string &SubTitle, string &Category, string &Description);

    void SetDisplayIndexes(int IdxMin, int IdxMax, int IdxCur);
    int GetDisplayIndexes(int *IdxMin, int *IdxMax, int *IdxCur);
    int GetDisplayLineShift(int NbLines);

    BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam);

    void ShowOSD();
    void HideOSD();

    void GetViewedChannelName(string &Channel);

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

    // Insert a new programme in the list keeping a certain order for the programmes
    void InsertProgramme(CProgramme* NewProg, int Sorting);

    void ClearNextviewEPGProviders();
    int GetNextviewEPGProviders();

    typedef vector<CProgramme*> CProgrammes;

    CProgrammes    m_Programmes;
    CProgrammes    m_ProgrammesSelection;
    CProgramme*    m_ProgrammeSelected;

    typedef vector<string*> strings;

    strings        m_NextviewProviders;

    string        m_CMDExe;
    string        m_FilesDir;

    time_t        m_LoadedTimeMin;
    time_t        m_LoadedTimeMax;

    LPCSTR        m_SearchChannel;
    time_t        m_SearchTimeMin;
    time_t        m_SearchTimeMax;
    int            m_IdxShowSelectMin;
    int            m_IdxShowSelectMax;
    int            m_IdxShowSelectCur;
    BOOL        m_UseProgFronBrowser;
    BOOL        m_SearchCurrent;
    int            m_ShiftLines;

    int            m_Displayed;
};


SETTING* EPG_GetSetting(EPG_SETTING Setting);
void EPG_ReadSettingsFromIni();
void EPG_WriteSettingsToIni(BOOL bOptimizeFileAccess);
CTreeSettingsGeneric* EPG_GetTreeSettingsPage();
void EPG_FreeSettings();


extern CUserChannels MyChannels;
extern CEPG MyEPG;


#endif