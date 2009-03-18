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
#include "VBI.h"
#include "VBI_VideoText.h"
#include "VBI_VPSdecode.h"
#include "PathHelpers.h"

using namespace std;

void XmltvParser_Start( const char * pFilename );


#define    ONE_DAY                86400
#define    ONE_HOUR            3600
#define    PRELOADING_EARLIER    (4 * ONE_HOUR)
#define    PRELOADING_LATER    (6 * ONE_HOUR)
#define    DEFAULT_OUTPUT_FILE    "DScalerEPG.xml"


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
    if (   ( (DateMax == 0) || (DateMax >= m_StartTime) )
        && ( (DateMin == 0) || (DateMin < m_EndTime) )
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
    ChannelEPGName = m_ChannelEPGName;
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
    sprintf_s(Date1, 17,
            "%04u/%02u/%02u %02u:%02u",
            date_tm->tm_year+1900, date_tm->tm_mon+1, date_tm->tm_mday, date_tm->tm_hour, date_tm->tm_min);
    date_tm = localtime(&m_EndTime);
    sprintf_s(Date2, 17,
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

    m_FilesDir = GetInstallationPath();

    m_LoadedTimeMin = 0;
    m_LoadedTimeMax = 0;

    m_SearchChannel = NULL;
    m_SearchTimeMin = 0;
    m_SearchTimeMax = 0;

    SetDisplayIndexes(-1, -1, -1);

    m_ProgrammeSelected = NULL;

    m_UseProgFronBrowser = FALSE;

    m_SearchCurrent = TRUE;

    m_ShiftLines = 0;

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
                       (LPSECURITY_ATTRIBUTES) NULL,    // No security
                       (LPSECURITY_ATTRIBUTES) NULL,    // No security
                       FALSE,
//                       IDLE_PRIORITY_CLASS | CREATE_NO_WINDOW,
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
void CEPG::ImportNxtvepgEPGDB(LPCSTR Provider)
{
    string Exe = (char*)Setting_GetValue(EPG_GetSetting(EPG_NXTVEPGPATH));
    string OutputFile = m_FilesDir + "\\" + DEFAULT_OUTPUT_FILE;
    string command = "\"" + m_CMDExe + "\" /C echo NextviewEPG database export ... && \""
        + Exe + "\" -dump xml5 -provider " + Provider + " > \"" + OutputFile + "\"";

    LOG(2, "NextviewEPG database import ... (%s)", command.c_str());
    ExecuteCommand(command);

    ReloadEPGData();
}


//
// Copy the input file in DScalerEPG.xml
//
void CEPG::ImportXMLTVFile(LPCSTR file)
{
    string InputFile = file;
    string OutputFile = m_FilesDir + "\\" + DEFAULT_OUTPUT_FILE;

    if (strcmp(InputFile.c_str(), OutputFile.c_str()))
    {
        LOG(2, "Copy file %s in %s ...", InputFile.c_str(), OutputFile.c_str());
        CopyFile(InputFile.c_str(), OutputFile.c_str());

        ReloadEPGData();
    }
}


//
// Load the DScaler EPG data for the programmes between two dates
// If DateMin and DateMax are not set, load the EPG data for
// the interval [current time - 2 hours, current time + 6 hours]
//
void CEPG::LoadEPGData(time_t DateMin, time_t DateMax)
{
    ClearProgrammes();

    time_t DateTime;

    // If dates not provided, load the data for the interval
    // [current time - 2 hours, current time + one day]
    if ( (DateMin == 0) || (DateMax == 0) )
    {
        time(&DateTime);
        DateMin = DateTime - PRELOADING_EARLIER;
        DateMax = DateTime + PRELOADING_LATER;
    }

    m_LoadedTimeMin = DateMin;
    m_LoadedTimeMax = DateMax;

    string InputXMLFile = m_FilesDir + "\\" + DEFAULT_OUTPUT_FILE;
    XmltvParser_Start(InputXMLFile.c_str());
}


void CEPG::ReloadEPGData()
{
    LoadEPGData(m_LoadedTimeMin, m_LoadedTimeMax);
    OSD_Clear();
}


// Check if new EPG data have to be loaded
BOOL CEPG::LoadEPGDataIfNeeded(time_t TimeMin, time_t TimeMax, int DeltaEarlier, int DeltaLater)
{
    BOOL LoadDone = FALSE;

    if ( (TimeMin < m_LoadedTimeMin) && (TimeMax > m_LoadedTimeMax) )
    {
        LoadEPGData(TimeMin - DeltaEarlier, TimeMax + DeltaLater);
        LoadDone = TRUE;
    }
    else if (TimeMin < m_LoadedTimeMin)
    {
        LoadEPGData(TimeMin - DeltaEarlier, m_LoadedTimeMax);
        LoadDone = TRUE;
    }
    else if (TimeMax > m_LoadedTimeMax)
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
        GetViewedChannelName(Channel);
        if (Channel.length() > 0)
        {
            if (m_SearchCurrent == TRUE)
            {
                time_t TimeNow;
                time(&TimeNow);
                resu = SearchForProgramme(Channel.c_str(), TimeNow);
            }
            else
            {
                if (m_ProgrammeSelected)
                {
                    resu = TRUE;
                }
            }
        }
        else
        {
            m_ProgrammeSelected = NULL;
            m_ProgrammesSelection.clear();
            SetDisplayIndexes(-1, -1, -1);
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
    SearchForProgrammes(ChannelName);
    m_ProgrammeSelected = NULL;
    for(CProgrammes::iterator it = m_ProgrammesSelection.begin();
        it != m_ProgrammesSelection.end();
        ++it)
    {
        if ((*it)->IsProgrammeMatching(ThatTime, ThatTime, ChannelName) == TRUE)
        {
            m_ProgrammeSelected = *it;
            SetDisplayIndexes(1, m_ProgrammesSelection.size(), it - m_ProgrammesSelection.begin() + 1);
            return TRUE;
        }
    }
    m_ProgrammesSelection.clear();
    SetDisplayIndexes(-1, -1, -1);
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
            if (ChannelName != NULL)
            {
                InsertProgramme(*it, -1);
            }
            else
            {
                InsertProgramme(*it, Setting_GetValue(EPG_GetSetting(EPG_SORTING)));
            }
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


int CEPG::GetDisplayLineShift(int NbLines)
{
    if (m_ShiftLines > NbLines)
    {
        m_ShiftLines = NbLines;
    }
    return m_ShiftLines;
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
    int sec, min, hour;
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
                sprintf_s(MsgTxt, 128, "Do you want to import the database\ncorresponding to the provider %s ?", m_NextviewProviders[i]->c_str());
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

    case IDM_DISPLAY_EPG:
        if (   (m_Displayed == 1)
            && (m_UseProgFronBrowser == FALSE)
            && (Setting_GetValue(EPG_GetSetting(EPG_TOGGLEBUTTONS)) == TRUE)
           )
        {
            OSD_Clear();
            return TRUE;
        }
        if (   (m_Displayed == 1)
            || (m_UseProgFronBrowser == FALSE) )
        {
            m_UseProgFronBrowser = FALSE;
            m_SearchCurrent = TRUE;
            // Check if new EPG data have to be loaded
            time(&TimeNow);
            LoadEPGDataIfNeeded(m_LoadedTimeMin, TimeNow, PRELOADING_EARLIER, PRELOADING_LATER);
            m_ShiftLines = 0;
            // Display the OSD screen
            OSD_ShowInfosScreen(2, Setting_GetValue(EPG_GetSetting(EPG_PERCENTAGESIZE)));
            m_Displayed = 1;
        }
        else if (m_IdxShowSelectCur != -1)
        {
            m_ProgrammeSelected = m_ProgrammesSelection[m_IdxShowSelectCur-1];
            m_SearchCurrent = FALSE;
            m_ShiftLines = 0;
            // Display the OSD screen
            OSD_ShowInfosScreen(2, Setting_GetValue(EPG_GetSetting(EPG_PERCENTAGESIZE)));
            m_Displayed = 1;
        }
        return TRUE;
        break;

    case IDM_DISPLAY_EPG_NOW:
        if (   (m_Displayed == 2)
            && (Setting_GetValue(EPG_GetSetting(EPG_TOGGLEBUTTONS)) == TRUE)
           )
        {
            OSD_Clear();
            return TRUE;
        }
        if ( (m_Displayed == 1) && (m_UseProgFronBrowser == TRUE) )
        {
            // Force to recompute by the OSD which page to display
            // (the one corresponding to the current selected programme)
            SetDisplayIndexes(-1, -1, m_IdxShowSelectCur);
        }
        else
        {
            time(&TimeMin);
            TimeMax = TimeMin + ONE_HOUR * Setting_GetValue(EPG_GetSetting(EPG_TIMEFRAMEDURATION)) - 1;
            // Check if new EPG data have to be loaded
            LoadEPGDataIfNeeded(TimeMin, TimeMax, PRELOADING_EARLIER, PRELOADING_LATER);
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
            LoadEPGDataIfNeeded(TimeMin, TimeMax, PRELOADING_EARLIER, PRELOADING_LATER);
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
            LoadEPGDataIfNeeded(TimeMin, TimeMax, PRELOADING_EARLIER, PRELOADING_LATER);
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
            m_ShiftLines++;
            // Refresh the OSD screen immediately
            OSD_ShowInfosScreen(2, Setting_GetValue(EPG_GetSetting(EPG_PERCENTAGESIZE)));
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
            if (m_ShiftLines > 0)
            {
                m_ShiftLines--;
                // Refresh the OSD screen immediately
                OSD_ShowInfosScreen(2, Setting_GetValue(EPG_GetSetting(EPG_PERCENTAGESIZE)));
            }
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
        else if (m_Displayed == 1)
        {
            // Show the details of the next programme on the same channel
            if (   (m_IdxShowSelectMin != -1)
                && (m_IdxShowSelectMax != -1) )
            {
                if (m_IdxShowSelectCur < m_ProgrammesSelection.size())
                {
                    m_IdxShowSelectCur++;
                    m_ProgrammeSelected = m_ProgrammesSelection[m_IdxShowSelectCur-1];
                }
                else if (   (m_UseProgFronBrowser == FALSE)
                         && (m_IdxShowSelectCur == m_ProgrammesSelection.size()) )
                {
                    // TODO Load next programmes
                }
                m_SearchCurrent = FALSE;
                m_ShiftLines = 0;
                // Refresh the OSD screen immediately
                OSD_ShowInfosScreen(2, Setting_GetValue(EPG_GetSetting(EPG_PERCENTAGESIZE)));
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
        else if (m_Displayed == 1)
        {
            // Show the details of the previous programme on the same channel
            if (   (m_IdxShowSelectMin != -1)
                && (m_IdxShowSelectMax != -1) )
            {
                if (m_IdxShowSelectCur > 1)
                {
                    m_IdxShowSelectCur--;
                    m_ProgrammeSelected = m_ProgrammesSelection[m_IdxShowSelectCur-1];
                }
                else if (   (m_UseProgFronBrowser == FALSE)
                         && (m_IdxShowSelectCur == 1) )
                {
                    // TODO Load previous programmes
                }
                m_SearchCurrent = FALSE;
                m_ShiftLines = 0;
                // Refresh the OSD screen immediately
                OSD_ShowInfosScreen(2, Setting_GetValue(EPG_GetSetting(EPG_PERCENTAGESIZE)));
            }
        }
        return TRUE;
        break;

    case IDM_EPG_VIEW_PROG:
        if (   (m_Displayed > 0)
            && (m_IdxShowSelectCur != -1)
            && (MyEPG.GetProgrammeChannelData(m_IdxShowSelectCur-1, ChannelName, ChannelEPGName, &ChannelNumber) == TRUE) )
        {
            if (ChannelNumber >= 0)
            {
                CSource *CurrentSource = Providers_GetCurrentSource();
                if (CurrentSource && !CurrentSource->IsInTunerMode())
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

    case IDM_EPG_DAY0:
    case IDM_EPG_DAY1:
    case IDM_EPG_DAY2:
    case IDM_EPG_DAY3:
    case IDM_EPG_DAY4:
    case IDM_EPG_DAY5:
    case IDM_EPG_DAY6:
    case IDM_EPG_DAY7:
        if ( (m_Displayed != 1) || (m_UseProgFronBrowser == FALSE) )
        {
            time(&TimeMin);
            TimeMin += (LOWORD(wParam) - IDM_EPG_DAY0 - 1) * ONE_DAY;
            if ((m_Displayed != 1) && (m_SearchTimeMin != 0))
            {
                datetime_tm = localtime(&m_SearchTimeMin);
                sec = datetime_tm->tm_sec;
                min = datetime_tm->tm_min;
                hour = datetime_tm->tm_hour;
                datetime_tm = localtime(&TimeMin);
                datetime_tm->tm_sec = sec;
                datetime_tm->tm_min = min;
                datetime_tm->tm_hour = hour;
                TimeMin = mktime(datetime_tm);
            }
            else
            {
                datetime_tm = localtime(&TimeMin);
                datetime_tm->tm_sec = 0;
                datetime_tm->tm_min = 0;
                TimeMin = mktime(datetime_tm);
            }
            TimeMax = TimeMin + ONE_HOUR * Setting_GetValue(EPG_GetSetting(EPG_TIMEFRAMEDURATION)) - 1;
            // Check if new EPG data have to be loaded
            LoadEPGDataIfNeeded(TimeMin, TimeMax, PRELOADING_EARLIER, PRELOADING_LATER);
            // Select the corresponding programmes
            SearchForProgrammes(NULL, TimeMin, TimeMax);
            m_UseProgFronBrowser = TRUE;
            // Display the OSD screen
            OSD_ShowInfosScreen(3, Setting_GetValue(EPG_GetSetting(EPG_PERCENTAGESIZE)));
            m_Displayed = 2;
        }
        return TRUE;
        break;

    case IDM_EPG_HOUR0:
    case IDM_EPG_HOUR1:
    case IDM_EPG_HOUR2:
    case IDM_EPG_HOUR3:
    case IDM_EPG_HOUR4:
    case IDM_EPG_HOUR5:
    case IDM_EPG_HOUR6:
    case IDM_EPG_HOUR7:
    case IDM_EPG_HOUR8:
    case IDM_EPG_HOUR9:
    case IDM_EPG_HOUR10:
    case IDM_EPG_HOUR11:
    case IDM_EPG_HOUR12:
    case IDM_EPG_HOUR13:
    case IDM_EPG_HOUR14:
    case IDM_EPG_HOUR15:
    case IDM_EPG_HOUR16:
    case IDM_EPG_HOUR17:
    case IDM_EPG_HOUR18:
    case IDM_EPG_HOUR19:
    case IDM_EPG_HOUR20:
    case IDM_EPG_HOUR21:
    case IDM_EPG_HOUR22:
    case IDM_EPG_HOUR23:
        if ( (m_Displayed != 1) || (m_UseProgFronBrowser == FALSE) )
        {
            if ((m_Displayed != 1) && (m_SearchTimeMin != 0))
            {
                TimeMin = m_SearchTimeMin;
            }
            else
            {
                time(&TimeMin);
            }
            datetime_tm = localtime(&TimeMin);
            datetime_tm->tm_sec = 0;
            datetime_tm->tm_min = 0;
            datetime_tm->tm_hour = LOWORD(wParam) - IDM_EPG_HOUR0;
            TimeMin = mktime(datetime_tm);
            TimeMax = TimeMin + ONE_HOUR * Setting_GetValue(EPG_GetSetting(EPG_TIMEFRAMEDURATION)) - 1;
            // Check if new EPG data have to be loaded
            LoadEPGDataIfNeeded(TimeMin, TimeMax, PRELOADING_EARLIER, PRELOADING_LATER);
            // Select the corresponding programmes
            SearchForProgrammes(NULL, TimeMin, TimeMax);
            m_UseProgFronBrowser = TRUE;
            // Display the OSD screen
            OSD_ShowInfosScreen(3, Setting_GetValue(EPG_GetSetting(EPG_PERCENTAGESIZE)));
            m_Displayed = 2;
        }
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
    LoadEPGDataIfNeeded(m_LoadedTimeMin, TimeNow, PRELOADING_EARLIER, PRELOADING_LATER);

    if (   (m_Programmes.size() > 0)
        && (m_UseProgFronBrowser == FALSE) )
    {
        string Channel;
        GetViewedChannelName(Channel);
        if (Channel.length() > 0)
        {
            // Search EPG info for the currently viewed channel
            m_SearchCurrent = TRUE;
            if (SearchForProgramme(Channel.c_str(), TimeNow) == TRUE)
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

    m_SearchCurrent = TRUE;

    m_ShiftLines = 0;

    m_Displayed = 0;
}


void CEPG::GetViewedChannelName(string &Channel)
{
    Channel = "";
    static string LastChannel = "";
    static time_t LastTime = 0;
    CSource *CurrentSource = Providers_GetCurrentSource();
    if (CurrentSource)
    {
        if (CurrentSource->IsInTunerMode())
        {
            Channel = Channel_GetName();
        }
        else if (Setting_GetValue(VBI_GetSetting(CAPTURE_VBI)))
        {
            time_t CurrentTime;
            time(&CurrentTime);

            string pChannel = Channel_GetVBIName();
            if (!pChannel.empty())
            {
                Channel = pChannel;
                // Reset data each second to detect change of channel
                if ((LastTime > 0) && ((CurrentTime - LastTime) > 0))
                {
                    VBI_ChannelChange();
                }
                LastChannel = pChannel;
                LastTime = CurrentTime;
            }
            else if ((LastTime > 0) && ((CurrentTime - LastTime) < 5))
            {
                Channel = LastChannel;
            }
        }
    }
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
    if (   (Setting_GetValue(EPG_GetSetting(EPG_CHANNELFILTERING)) == TRUE)
        && !IsValidChannelName(ChannelName))
        return 0;
    // Keep only programmes scheduled between m_LoadedTimeMin and m_LoadedTimeMax
    if ( (EndTime <= m_LoadedTimeMin) || (StartTime > m_LoadedTimeMax) )
        return 0;
    return 1;
}


//
// Insert a new programme in the list keeping a certain order for the programmes
//
void CEPG::InsertProgramme(CProgramme* NewProg, int Sorting)
{
    time_t NewProgStart, NewProgEnd;
    time_t ProgStart, ProgEnd;

    NewProg->GetProgrammeDates(&NewProgStart, &NewProgEnd);

    int added = 0;

    if (Sorting == 1)
    {
        string    NewChannel;
        string    NewChannelEPG;
        string    Channel;
        string    ChannelEPG;
        int        NewNumber, Number;
        BOOL    bTime = FALSE;

        NewProg->GetProgrammeChannelData(NewChannel, NewChannelEPG, &NewNumber);

        for(CProgrammes::iterator it = m_ProgrammesSelection.begin();
            it != m_ProgrammesSelection.end();
            )
        {
            (*it)->GetProgrammeChannelData(Channel, ChannelEPG, &Number);
            if (bTime == TRUE)
            {
                if (   ( (NewNumber != -1) && (NewNumber != Number) )
                    || ( (NewNumber == -1) && _stricmp(NewChannel.c_str(), Channel.c_str()) )
                   )
                {
                    m_ProgrammesSelection.insert(it, NewProg);
                    added = 1;
                    break;
                }
                (*it)->GetProgrammeDates(&ProgStart, &ProgEnd);
                if (   (ProgStart > NewProgStart)
                    || ((ProgStart == NewProgStart) && (ProgEnd > NewProgEnd))
                   )
                {
                    m_ProgrammesSelection.insert(it, NewProg);
                    added = 1;
                    break;
                }
                ++it;
            }
            else
            {
                if (NewNumber != -1)
                {
                    if (   (Number == -1)
                        || (Number > NewNumber)
                       )
                    {
                        m_ProgrammesSelection.insert(it, NewProg);
                        added = 1;
                        break;
                    }
                    else if (NewNumber == Number)
                    {
                        bTime = TRUE;
                    }
                    else
                    {
                        ++it;
                    }
                }
                else
                {
                    if (   (Number == -1)
                        && !_stricmp(NewChannel.c_str(), Channel.c_str())
                       )
                    {
                        bTime = TRUE;
                    }
                    else
                    {
                        ++it;
                    }
                }
            }
        }
    }
    else if (Sorting == 0)
    {
        // Search the first programme having a start time later
        // or an identical start time but a end time later
        for(CProgrammes::iterator it = m_ProgrammesSelection.begin();
            it != m_ProgrammesSelection.end();
            ++it)
        {
            (*it)->GetProgrammeDates(&ProgStart, &ProgEnd);
            if (   (ProgStart > NewProgStart)
                || ((ProgStart == NewProgStart) && (ProgEnd > NewProgEnd))
               )
            {
                m_ProgrammesSelection.insert(it, NewProg);
                added = 1;
                break;
            }
        }
    }
    else if (Sorting == -1)
    {
        if (m_ProgrammesSelection.size() > 0)
        {
            m_ProgrammesSelection[m_ProgrammesSelection.size()-1]->GetProgrammeDates(&ProgStart, &ProgEnd);
            if (   (ProgStart > NewProgStart)
                || ((ProgStart == NewProgStart) && (ProgEnd > NewProgEnd))
               )
            {
                for(CProgrammes::iterator it = m_ProgrammesSelection.begin();
                    it != m_ProgrammesSelection.end();
                    ++it)
                {
                    (*it)->GetProgrammeDates(&ProgStart, &ProgEnd);
                    if (   (ProgStart > NewProgStart)
                        || ((ProgStart == NewProgStart) && (ProgEnd > NewProgEnd))
                       )
                    {
                        m_ProgrammesSelection.insert(it, NewProg);
                        added = 1;
                        break;
                    }
                }
            }
        }
    }

    if (!added)
    {
        m_ProgrammesSelection.push_back(NewProg);
    }
}


void CEPG::AddProgramme(time_t StartTime, time_t EndTime, LPCSTR Title, LPCSTR ChannelName, LPCSTR ChannelEPGName, int ChannelNumber)
{
    if ( (EndTime > m_LoadedTimeMin) && (StartTime <= m_LoadedTimeMax) )
    {
        CProgramme* newProgramme = new CProgramme(StartTime, EndTime, Title, ChannelName, ChannelEPGName, ChannelNumber, "", "", "");
        m_Programmes.push_back(newProgramme);
    }
}


void CEPG::AddProgramme(time_t StartTime, time_t EndTime, LPCSTR Title, LPCSTR ChannelName, LPCSTR ChannelEPGName, int ChannelNumber, LPCSTR SubTitle, LPCSTR Category, LPCSTR Description)
{
    if ( (EndTime > m_LoadedTimeMin) && (StartTime <= m_LoadedTimeMax) )
    {
        CProgramme* newProgramme = new CProgramme(StartTime, EndTime, Title, ChannelName, ChannelEPGName, ChannelNumber, SubTitle, Category, Description);
        m_Programmes.push_back(newProgramme);
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
    else if (Setting_GetValue(EPG_GetSetting(EPG_CHANNELFILTERING)) == FALSE)
    {
        AddProgramme(StartTime, EndTime, Title, ChannelEPGName, ChannelEPGName, -1, SubTitle, Category, Description);
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

    char* DefaultProvider = (char*)Setting_GetValue(EPG_GetSetting(EPG_NXTVEPGPROVIDER));
    if (strlen(DefaultProvider) > 0)
    {
        if (!strcmp(DefaultProvider, "merged"))
        {
            newProvider = new string(DefaultProvider);
            m_NextviewProviders.push_back(newProvider);
            return m_NextviewProviders.size();
        }

        strcpy(SearchFiles, Exe);
        c = strrchr(SearchFiles, '\\');
        if (c)
        {
            sprintf(c+1, "nxtv%s.epg", DefaultProvider);
        }
        else
        {
            sprintf(SearchFiles, "nxtv%s.epg", DefaultProvider);
        }
        if (!stat(SearchFiles, &st))
        {
            strncpy(Provider, &SearchFiles[strlen(SearchFiles)-8], 4);
            Provider[4] = '\0';
            newProvider = new string(Provider);
            m_NextviewProviders.push_back(newProvider);
            return m_NextviewProviders.size();
        }

        strcpy(SearchFiles, Exe);
        c = strrchr(SearchFiles, '\\');
        if (c)
        {
            sprintf(c+1, "nxtvdb-%s", DefaultProvider);
        }
        else
        {
            sprintf(SearchFiles, "nxtvdb-%s", DefaultProvider);
        }
        if (!stat(SearchFiles, &st))
        {
            strncpy(Provider, &SearchFiles[strlen(SearchFiles)-4], 4);
            Provider[4] = '\0';
            newProvider = new string(Provider);
            m_NextviewProviders.push_back(newProvider);
            return m_NextviewProviders.size();
        }
    }

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


void CEPG::SetMenu(HMENU hMenu)
{
    static time_t    LastDay = 0;
    time_t            Day;
    struct tm        *datetime_tm;
    char            *week_days[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

    time(&Day);
    datetime_tm = localtime(&Day);
    datetime_tm->tm_sec = 0;
    datetime_tm->tm_min = 0;
    datetime_tm->tm_hour = 0;
    Day = mktime(datetime_tm);
    if (LastDay != Day)
    {
        LastDay = Day;

        HMENU hMenuEPGDay = GetEPGDaySubmenu();
        if (hMenuEPGDay == NULL)
        {
            return;
        }

        ModifyMenu(hMenuEPGDay, IDM_EPG_DAY3, MF_STRING, IDM_EPG_DAY3, week_days[(datetime_tm->tm_wday + 2) % 7]);
        ModifyMenu(hMenuEPGDay, IDM_EPG_DAY4, MF_STRING, IDM_EPG_DAY4, week_days[(datetime_tm->tm_wday + 3) % 7]);
        ModifyMenu(hMenuEPGDay, IDM_EPG_DAY5, MF_STRING, IDM_EPG_DAY5, week_days[(datetime_tm->tm_wday + 4) % 7]);
        ModifyMenu(hMenuEPGDay, IDM_EPG_DAY6, MF_STRING, IDM_EPG_DAY6, week_days[(datetime_tm->tm_wday + 5) % 7]);
        ModifyMenu(hMenuEPGDay, IDM_EPG_DAY7, MF_STRING, IDM_EPG_DAY7, week_days[(datetime_tm->tm_wday + 6) % 7]);
    }

    BOOL bEnable = (m_Displayed != 1) || (m_UseProgFronBrowser == FALSE);

    EnableMenuItem(hMenu, IDM_EPG_DAY0, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_DAY1, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_DAY2, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_DAY3, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_DAY4, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_DAY5, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_DAY6, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_DAY7, bEnable ? MF_ENABLED : MF_GRAYED);

    EnableMenuItem(hMenu, IDM_EPG_HOUR0, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_HOUR1, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_HOUR2, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_HOUR3, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_HOUR4, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_HOUR5, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_HOUR6, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_HOUR7, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_HOUR8, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_HOUR9, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_HOUR10, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_HOUR11, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_HOUR12, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_HOUR13, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_HOUR14, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_HOUR15, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_HOUR16, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_HOUR17, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_HOUR18, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_HOUR19, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_HOUR20, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_HOUR21, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_HOUR22, bEnable ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu, IDM_EPG_HOUR23, bEnable ? MF_ENABLED : MF_GRAYED);
}


// ---------------------- Settings ---------------------


static char        ExePath[MAX_PATH] = {0};
static char*    NextviewEPGExePath = NULL;
static char*    NextviewEPGProvider = NULL;
static long        EPG_DefaultSizePerc = 5;
static long        EPG_FrameDuration = 1;
static BOOL        EPG_ChannelFiltering = FALSE;
static long        EPG_MaxCharsPerLine = 75;
static int        EPG_ProgSorting = 0;
static BOOL        EPG_ToggleButtons = FALSE;
static const char* EPG_ProgSortingLabels[2] = 
{
    "Programme start time",
    "Channel order / Programme start time",
};


BOOL ChannelFiltering_OnChange(long NewValue)
{
    OSD_Clear();
    MyEPG.ReloadEPGData();
    return FALSE;
}


BOOL ProgSorting_OnChange(long NewValue)
{
    OSD_Clear();
    MyEPG.ReloadEPGData();
    return FALSE;
}


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
    {
        "Keep only DScaler channels", YESNO, 0, (long*)&EPG_ChannelFiltering,
         FALSE, 0, 1, 1, 1,
         NULL,
        "EPG", "ChannelFiltering", ChannelFiltering_OnChange,
    },
    {
        "Max characters per line (programme description)", SLIDER, 0, (long*)&EPG_MaxCharsPerLine,
         75, 20, 128, 1, 1,
         NULL,
        "EPG", "MaxCharsPerLine", NULL,
    },
    {
        "NextviewEPG default provider", CHARSTRING, 0, (long*)&NextviewEPGProvider,
         (long)"", 0, 0, 0, 0,
         NULL,
        "EPG", "NextviewProvider", NULL,
    },
    {
        "Programmes sorting", ITEMFROMLIST, 0, (long*)&EPG_ProgSorting,
        0, 0, 1, 1, 1,
        EPG_ProgSortingLabels,
        "EPG", "Sorting", ProgSorting_OnChange,
    },
    {
        "Use keyboard shortcuts as toggle buttons", YESNO, 0, (long*)&EPG_ToggleButtons,
         FALSE, 0, 1, 1, 1,
         NULL,
        "EPG", "ToggleButtons", NULL,
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


int CheckProgrammeValidity(time_t StartTime, time_t EndTime, char * ChannelName)
{
    return     MyEPG.CheckProgrammeValidity(StartTime, EndTime, ChannelName);
}

void AddProgramme(time_t StartTime, time_t EndTime, char * Title, char * ChannelName, char * SubTitle, char * Category, char * Description)
{
    MyEPG.AddProgramme(StartTime, EndTime, Title, ChannelName, SubTitle, Category, Description);
}
