/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2007 Radoslav Masaryk (pyroteam@centrum.sk) All rights reserved.
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

#include "stdafx.h"
#include "../DScalerRes/RESOURCE.H"
#include "tinyxml.h"
#include "TimeShift.h"
#include "ProgramList.h"
#include "Channels.h"
#include "DScaler.h"
#include "ScheduledRecording.h"
#include "Crash.h"
#include "DebugLog.h"
#include <vector>

using namespace std;

SmartPtr<CScheduledRecording> pSchRec;
HANDLE  gThreadId = NULL;

enum STATE {READY,SUCCEED,CANCELED,RECORDING};

/*****************************************
*    Global initialization functions
******************************************/
void ShowSchedRecDlg()
{
    CScheduleDlg schDlg;
    schDlg.DoModal(GetMainWnd());
}

DWORD WINAPI RecordThreadProc(LPVOID lParam)
{
    DScalerThread("Scheduled Recording Thread");
    /// \todo proper thread control
    pSchRec = new CScheduledRecording();
    while(pSchRec && pSchRec->run())
    {}

    return 0;
}

/*****************************************
*    CScheduleDlg functions
******************************************/
CScheduleDlg::CScheduleDlg() :
    CDSDialog(MAKEINTRESOURCE(IDD_SCHEDULE)),
    m_bCtrlsInit(FALSE)
{
}

CScheduleDlg::~CScheduleDlg()
{
}

BOOL CScheduleDlg::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
    HANDLE_MSG(hDlg, WM_INITDIALOG, OnInitDialog);
    HANDLE_MSG(hDlg, WM_COMMAND, OnCommand);
    HANDLE_MSG(hDlg, WM_NOTIFY, OnNotify);
    default:
        return FALSE;
    }
}

void CScheduleDlg::OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
    switch(id)
    {
    case IDC_SCHEDULE_OK:
        if(BN_CLICKED == codeNotify)
        {
            OnOkClicked(hDlg);
        }
    case IDC_SCHEDULE_REMOVE:
        if(BN_CLICKED == codeNotify)
        {
            OnRemoveClicked(hDlg);
        }
        break;
    case IDC_SCHEDULE_ADD:
        if(BN_CLICKED == codeNotify)
        {
            OnAddClicked(hDlg);
        }
        break;
    case IDC_SCHEDULE_EDIT_DURATION:
        if(EN_UPDATE == codeNotify)
        {
            OnDurationUpdate(hDlg);
        }
        break;
    }
}

LRESULT CScheduleDlg::OnNotify(HWND hwnd, int id, LPNMHDR nmhdr)
{
    switch(id)
    {
    case IDC_SCHEDULE_TIMEPICKER:
        if(nmhdr->code == DTN_DATETIMECHANGE)
        {
            return OnTimePickerChanged(hwnd, nmhdr);
        }
        break;
    case IDC_SCHEDULE_ENDTIMEPICKER:
        if(nmhdr->code == DTN_DATETIMECHANGE)
        {
            return OnTimePickerChanged(hwnd, nmhdr);
        }
        break;
    }
    return 0;
}


BOOL CScheduleDlg::OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
    InitCtrls(hDlg);
    pSchRec->showRecords(hDlg);
    return TRUE;
}

void CScheduleDlg::OnDscalerExit()
{
    pSchRec->exitScheduledRecording();
}

void CScheduleDlg::OnAddClicked(HWND hDlg)
{
    pSchRec->addSchedule(hDlg);
}

void CScheduleDlg::OnDurationUpdate(HWND hDlg)
{
    if(!m_bCtrlsInit || GetFocus() != GetDlgItem(hDlg, IDC_SCHEDULE_EDIT_DURATION))
        return;

    string str_dur(GetDlgItemString(hDlg, IDC_SCHEDULE_EDIT_DURATION));

    SYSTEMTIME SysTime;

    DateTime_GetSystemtime(GetDlgItem(hDlg,IDC_SCHEDULE_TIMEPICKER), &SysTime);

    CTime new_time(SysTime);

    CTimeSpan end_timespan(0, 0, FromString<long>(str_dur), 0);

    new_time += end_timespan;

    new_time.GetAsSystemTime(SysTime);
    DateTime_SetSystemtime(GetDlgItem(hDlg, IDC_SCHEDULE_ENDTIMEPICKER), GDT_VALID, &SysTime);
}

LRESULT CScheduleDlg::OnTimePickerChanged(HWND hDlg, NMHDR* nmhdr)
{
    if(!m_bCtrlsInit)
        return 0;

    SYSTEMTIME StartSysTime;
    SYSTEMTIME EndSysTime;

    DateTime_GetSystemtime(GetDlgItem(hDlg,IDC_SCHEDULE_TIMEPICKER), &StartSysTime);
    DateTime_GetSystemtime(GetDlgItem(hDlg,IDC_SCHEDULE_ENDTIMEPICKER), &EndSysTime);

    CTime StartTime(StartSysTime);
    CTime EndTime(EndSysTime);

    if(EndTime < StartTime)
    {
        SetDurationCtrl(hDlg, 0);
        return 0;
    }

    CTimeSpan DurTimespan = EndTime - StartTime;
    SetDurationCtrl(hDlg, (int)DurTimespan.GetTotalMinutes());
    return 0;
}

void CScheduleDlg::SetDurationCtrl(HWND hDlg, int minutes)
{
    string chDur(ToString(minutes));
    SetDlgItemText(hDlg, IDC_SCHEDULE_EDIT_DURATION, chDur.c_str());
}

void CScheduleDlg::OnRemoveClicked(HWND hDlg)
{
    pSchRec->removeSchedule(hDlg);
}

void CScheduleDlg::OnOkClicked(HWND hDlg)
{
    pSchRec->saveOnClose(hDlg);
    EndDialog(hDlg, 0);
}

void ListView_MyInsertColumn(HWND hList, int ColumnIndex, LPCSTR ColumnHeading, int Width)
{
    LVCOLUMN Column;
    Column.mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH;
    Column.pszText = (LPSTR)ColumnHeading;
    Column.fmt = 0;
    Column.cx = Width;
    ListView_InsertColumn(hList, ColumnIndex, &Column);
}

void CScheduleDlg::InitCtrls(HWND hDlg)
{
    HWND hList = GetDlgItem(hDlg, IDC_SCHEDULE_LIST);

    ListView_MyInsertColumn(hList, 0, "Name", 70);
    ListView_MyInsertColumn(hList, 1, "Program", 70);
    ListView_MyInsertColumn(hList, 2, "Date", 80);
    ListView_MyInsertColumn(hList, 3, "Start", 60);
    ListView_MyInsertColumn(hList, 4, "Duration(minutes)", 100);
    ListView_MyInsertColumn(hList, 5, "State", 60);
    SendMessage(hList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);

    CTime time_now = CTime::GetCurrentTime();

    SetDlgItemText(hDlg, IDC_SCHEDULE_EDIT_NAME, "Name");
    SetDlgItemText(hDlg, IDC_SCHEDULE_EDIT_DURATION, "90");
    SYSTEMTIME SysTime;
    time_now.GetAsSystemTime(SysTime);
    DateTime_SetSystemtime(GetDlgItem(hDlg,IDC_SCHEDULE_DATEPICKER), GDT_VALID, &SysTime);
    DateTime_SetSystemtime(GetDlgItem(hDlg,IDC_SCHEDULE_TIMEPICKER), GDT_VALID, &SysTime);

    CTimeSpan end_time_span(0,1,30,0);
    time_now += end_time_span;
    time_now.GetAsSystemTime(SysTime);
    DateTime_SetSystemtime(GetDlgItem(hDlg,IDC_SCHEDULE_ENDTIMEPICKER), GDT_VALID, &SysTime);

    SendMessage(GetDlgItem(hDlg,IDC_SCHEDULE_EDIT_NAME), EM_SETLIMITTEXT, 19, 0);
    SendMessage(GetDlgItem(hDlg,IDC_SCHEDULE_EDIT_DURATION), EM_SETLIMITTEXT, 3, 0);

    std::vector<std::string> channels;
    pSchRec->getChannels(channels);

    if(!channels.empty())
    {
        for(std::vector<std::string>::iterator iter=channels.begin(); iter != channels.end(); iter++)
        {
            ComboBox_AddString(GetDlgItem(hDlg, IDC_SCHEDULE_COMBO), iter->c_str());
        }

        ComboBox_SelectString(GetDlgItem(hDlg, IDC_SCHEDULE_COMBO), 0, channels[0].c_str());
    }

    m_bCtrlsInit = TRUE;
}

/*****************************************
*    CSchedule functions
******************************************/
CSchedule::CSchedule(const string& name, const string& program_name, int duration, int state, CTime time) :
    m_name(name),
    m_program_name(program_name),
    m_duration(duration),
    m_state(state),
    m_time_start(time)
{
}

string CSchedule::getDateStr() const
{
    return m_time_start.Format("%m.%d.%Y").GetBuffer(0);
};

string CSchedule::getTimeStr() const
{
    return m_time_start.Format("%H:%M").GetBuffer(0);
};

/*****************************************
*    CScheduledRecording funtions
******************************************/
CScheduledRecording::CScheduledRecording():m_bSchedulesUpdate(TRUE),m_bExitThread(FALSE)
{}

BOOL CScheduledRecording::run()
{
    while(!processSchedules())
    {
        if(m_bExitThread)
            return FALSE;
        else
            SleepEx(500,FALSE);
    }
    while(!IsEndOfRecording())
    {
        if(m_bExitThread)
            return FALSE;
        else
        {
            ShowText(GetMainWnd(), "Recording");
            SleepEx(500,FALSE);
        }
    }

    return 0;
}

void CScheduledRecording::initScheduledRecordingThreadProc()
{
    gThreadId = CreateThread(NULL,0,RecordThreadProc,NULL,0,0);
}

void CScheduledRecording::getChannels(std::vector<std::string> &channels)
{
    CUserChannels MyChannels;
    MyChannels.ReadFile(SZ_DEFAULT_PROGRAMS_FILENAME);

    int count = MyChannels.GetSize();

    std::string channel_name;
    CChannel* channel;

    for(int i=0;i<count;i++)
    {
        channel = MyChannels.GetChannel(i);

        if(!channel)
            continue;

        channel_name = channel->GetName();
        channels.push_back(channel_name);
        channel = NULL;
    }
}

void CScheduledRecording::addSchedule(HWND hDlg)
{
    HWND hList = GetDlgItem(hDlg, IDC_SCHEDULE_LIST);
    HWND hCombo = GetDlgItem(hDlg, IDC_SCHEDULE_COMBO);
    HWND hDateCtrl = GetDlgItem(hDlg, IDC_SCHEDULE_DATEPICKER);
    HWND hTimeCtrl = GetDlgItem(hDlg, IDC_SCHEDULE_TIMEPICKER);

    string name(GetDlgItemString(hDlg, IDC_SCHEDULE_EDIT_NAME));
    
    if(name.empty())
    {
        MessageBox(GetMainWnd(), "Enter name for schedule", "Error", MB_OK);
        return;
    }

    for(std::vector<CSchedule>::iterator iter=m_schedules.begin();iter!=m_schedules.end();++iter)
    {
        if(iter->getName() == name)
        {
            string err_text("Schedule name ");

            err_text += name;
            err_text += "already used";

            MessageBox(GetMainWnd(),err_text.c_str(),"Error",MB_OK);
            return;
        }
    }

    string strDuration(GetDlgItemString(hDlg, IDC_SCHEDULE_EDIT_DURATION));

    if(strDuration.empty())
    {
        MessageBox(GetMainWnd(),"Enter recording time in minutes","Error",MB_OK);
        return;
    }

    int duration(FromString<int>(strDuration));

    string program(GetDlgItemString(hDlg, IDC_SCHEDULE_COMBO));

    SYSTEMTIME date;
    SYSTEMTIME time;

    DateTime_GetSystemtime(hDateCtrl, &date);
    DateTime_GetSystemtime(hTimeCtrl, &time);

    CTime time_start(date.wYear,date.wMonth,date.wDay,time.wHour,time.wMinute,0);

    CSchedule schedule(name,program,duration,READY,time_start);

    m_schedules.push_back(schedule);
    showRecords(hDlg);
}

void CScheduledRecording::removeSchedule(HWND hDlg)
{
    HWND list = GetDlgItem(hDlg, IDC_SCHEDULE_LIST);
    int i;
    for(i=0; i < ListView_GetItemCount(list); ++i)
    {
        if(ListView_GetItemState(list, i, LVIS_SELECTED) == LVIS_SELECTED)
        {
            vector<char> item_text(MAX_PATH);
            ListView_GetItemText(list, i, 0, &item_text[0], MAX_PATH);

            for(std::vector<CSchedule>::iterator iter = m_schedules.begin(); iter!=m_schedules.end(); ++iter)
            {
                if(strcmp(iter->getName(), &item_text[0]) == 0)
                {
                    if(iter->getState() == RECORDING)
                    {
                        MessageBox(GetMainWnd(),"Schedule is already recording...cannot be removed","Error",MB_OK);
                        return;
                    }

                    m_schedules.erase(iter);

                    break;
                }
            }
            break;
        }
    }

    ListView_DeleteItem(list, i);
    showRecords(hDlg);
}
void CScheduledRecording::saveOnClose(HWND hDlg)
{
    saveToXml(m_schedules);
}

void CScheduledRecording::saveToXml(std::vector<CSchedule> schedules)
{
    TiXmlDocument doc("records.xml");

    TiXmlElement* records_table = new TiXmlElement ("RecordsTable");
    doc.LinkEndChild(records_table);

    int r_count = schedules.size();
    int r_pos = 0;

    for(std::vector<CSchedule>::iterator iter = schedules.begin();iter != schedules.end();iter++)
    {
        string chRecord_number("R");
        char chBuff[10];

        _itoa_s(r_pos++,chBuff,10,10);
        chRecord_number += chBuff;

        TiXmlElement* record = new TiXmlElement(chRecord_number);
        records_table->LinkEndChild(record);

        TiXmlElement* name = new TiXmlElement("Name");
        name->LinkEndChild(new TiXmlText(iter->getName()));
        record->LinkEndChild(name);

        TiXmlElement* program = new TiXmlElement("Program");
        program->LinkEndChild(new TiXmlText(iter->getProgramName()));
        record->LinkEndChild(program);

        CTime time_start;
        time_start = iter->getStartTime();

        TiXmlElement* year = new TiXmlElement("Year");
        _itoa_s(time_start.GetYear(),chBuff,10,10);
        year->LinkEndChild(new TiXmlText(chBuff));
        record->LinkEndChild(year);

        TiXmlElement* month = new TiXmlElement("Month");
        memset(chBuff,0,sizeof(char)*10);
        _itoa_s(time_start.GetMonth(),chBuff,10,10);
        month->LinkEndChild(new TiXmlText(chBuff));
        record->LinkEndChild(month);

        TiXmlElement* day = new TiXmlElement("Day");
        memset(chBuff,0,sizeof(char)*10);
        _itoa_s(time_start.GetDay(),chBuff,10,10);
        day->LinkEndChild(new TiXmlText(chBuff));
        record->LinkEndChild(day);

        TiXmlElement* hour = new TiXmlElement("Hour");
        memset(chBuff,0,sizeof(char)*10);
        _itoa_s(time_start.GetHour(),chBuff,10,10);
        hour->LinkEndChild(new TiXmlText(chBuff));
        record->LinkEndChild(hour);

        TiXmlElement* minute = new TiXmlElement("Minute");
        memset(chBuff,0,sizeof(char)*10);
        _itoa_s(time_start.GetMinute(),chBuff,10,10);
        minute->LinkEndChild(new TiXmlText(chBuff));
        record->LinkEndChild(minute);

        TiXmlElement* duration = new TiXmlElement("Duration");
        memset(chBuff,0,sizeof(char)*10);
        _itoa_s(iter->getDuration(), chBuff,10,10);
        duration->LinkEndChild(new TiXmlText(chBuff));
        record->LinkEndChild(duration);

        TiXmlElement* state = new TiXmlElement("State");
        memset(chBuff,0,sizeof(char)*10);
        _itoa_s(iter->getState(),chBuff,10,10);
        state->LinkEndChild(new TiXmlText(chBuff));
        record->LinkEndChild(state);
    }

    TiXmlElement* records_count = new TiXmlElement("RecordsCount");
    records_table->LinkEndChild(records_count);

    char chRecord_count[4];
    _itoa_s(r_count,chRecord_count,4,10);

    records_count->LinkEndChild(new TiXmlText((const char*)chRecord_count));

    doc.SaveFile();
}

void CScheduledRecording::loadFromXml()
{
    TiXmlDocument doc("records.xml");
    doc.LoadFile();

    TiXmlHandle hDoc(&doc);
    TiXmlElement* hRoot = NULL;
    TiXmlHandle hRecord(0);

    int r_count = getRecordsCount();

    for(int r_pos=0;r_pos<r_count;r_pos++)
    {
        char chBuff[10];
        string chRecord_number("R");

        _itoa_s(r_pos,chBuff,10,10);
        chRecord_number[1] = 0;
        chRecord_number += chBuff;

        hRoot = hDoc.FirstChildElement("RecordsTable").FirstChildElement(chRecord_number).Element();
        hRecord = TiXmlHandle(hRoot);

        string chName(hRecord.FirstChildElement("Name").Element()->GetText());
        string chProgram(hRecord.FirstChildElement("Program").Element()->GetText());

        int year = atoi(hRecord.FirstChildElement("Year").Element()->GetText());
        int month = atoi(hRecord.FirstChildElement("Month").Element()->GetText());
        int day = atoi(hRecord.FirstChildElement("Day").Element()->GetText());
        int hour = atoi(hRecord.FirstChildElement("Hour").Element()->GetText());
        int minute = atoi(hRecord.FirstChildElement("Minute").Element()->GetText());

        CTime time_start(year,month,day,hour,minute,0);

        int duration = atoi(hRecord.FirstChildElement("Duration").Element()->GetText());
        int state = atoi(hRecord.FirstChildElement("State").Element()->GetText());


        CSchedule schedule(chName,chProgram,duration,state,time_start);
        m_schedules.push_back(schedule);

    }
}

BOOL CScheduledRecording::processSchedules()
{
    if(m_bSchedulesUpdate)
        loadFromXml();

    if(!m_schedules.size())
        return FALSE;

    m_bSchedulesUpdate = FALSE;

    CTime time_now = CTime::GetCurrentTime();

    for(std::vector<CSchedule>::iterator iter = m_schedules.begin();iter != m_schedules.end();iter++)
    {
        CTime time_schedule = iter->getStartTime();

        if(time_now == time_schedule && iter->getState() == READY)
        {
            startRecording(*iter,time_schedule);
            return TRUE;
        }
    }
    return FALSE;
}

void CScheduledRecording::showRecords(HWND hDlg)
{
    HWND list = GetDlgItem(hDlg, IDC_SCHEDULE_LIST);
    ListView_DeleteAllItems(list);

    if(!m_schedules.size())
        return;

    for(std::vector<CSchedule>::iterator iter=m_schedules.begin();iter!=m_schedules.end();iter++)
    {
        char chBuff[4];
        _itoa_s(iter->getDuration(),chBuff,4,10);

        LVITEM ListItem;
        memset(&ListItem, 0, sizeof(LVITEM));
        ListItem.mask = LVIF_TEXT;
        ListItem.iItem = 0;
        ListItem.pszText = (char*)iter->getName();
        ListView_InsertItem(list, &ListItem);
        ListView_SetItemText(list, 0, 1, (LPSTR)iter->getProgramName());

        string chDate(iter->getDateStr());
        ListView_SetItemText(list, 0, 2, (LPSTR)chDate.c_str());

        string chTime(iter->getTimeStr());
        ListView_SetItemText(list, 0, 3, (LPSTR)chTime.c_str());

        ListView_SetItemText(list, 0, 4, chBuff);

        switch(iter->getState())
        {
        case CANCELED:
            ListView_SetItemText(list, 0,5,"Canceled");
            break;
        case READY:
            ListView_SetItemText(list, 0,5,"Ready");
            break;
        case SUCCEED:
            ListView_SetItemText(list, 0,5,"Succeed");
            break;
        case RECORDING:
            ListView_SetItemText(list, 0,5,"Recording");
            break;
        }
    }
}

int CScheduledRecording::getRecordsCount()
{
    TiXmlDocument doc("records.xml");

    if(!doc.LoadFile())
        return 0;

    TiXmlHandle hDoc(&doc);
    TiXmlElement* element;

    element = hDoc.FirstChildElement().Element()->FirstChildElement("RecordsCount");

    if(!element)
        return 0;

    return atoi(element->GetText());
}


void CScheduledRecording::startRecording(CSchedule recording_program, CTime time_start)
{
    CTimeSpan time_duration(0,0,recording_program.getDuration(),0);
    CTime time_end = time_start;
    time_end += time_duration;
    recording_program.setTimeEnd(time_end);

    recording_program.setState(RECORDING);
    m_recording_program = recording_program;

    setRecordState(m_recording_program.getName(),RECORDING);

    string current_channel = Channel_GetName();

    HMENU hMenu = LoadMenu(hResourceInst, MAKEINTRESOURCE(IDC_DSCALERMENU));

    if(current_channel == m_recording_program.getProgramName())
    {
        if(TimeShiftRecord())
        {
            ShowText(GetMainWnd(), "Recording");
            TimeShiftOnSetMenu(hMenu);
        }
    }
    else
    {
        std::vector<std::string> channels;
        int channel_index = 0;

        getChannels(channels);

        for(std::vector<std::string>::iterator iter=channels.begin();iter != channels.end();iter++)
        {
            if(strcmp(iter->c_str(),m_recording_program.getProgramName()) == 0)
            {
                CUserChannels MyChannels;

                MyChannels.ReadFile(SZ_DEFAULT_PROGRAMS_FILENAME);
                int channel_number = MyChannels.GetChannel(++channel_index)->GetChannelNumber();

                Channel_ChangeToNumber(channel_number);

                if(TimeShiftRecord())
                {
                    ShowText(GetMainWnd(), "Recording");
                    TimeShiftOnSetMenu(hMenu);
                    break;
                }
            }
            channel_index++;
        }
    }
}

BOOL CScheduledRecording::IsEndOfRecording()
{
    CTime time_now = CTime::GetCurrentTime();

    if(m_recording_program.getTimeEnd() != time_now && TimeShiftIsRunning())
        return FALSE;
    else
    {
        if(!TimeShiftIsRunning())
            stopRecording(CANCELED);

        if(m_recording_program.getTimeEnd() == time_now)
            stopRecording(SUCCEED);

        return TRUE;
    }

}

void CScheduledRecording::setRecordState(const char* schedule_name, int state)
{
    for(std::vector<CSchedule>::iterator iter=m_schedules.begin();iter!=m_schedules.end();iter++)
        if(strcmp(iter->getName(),schedule_name) == 0 )
            iter->setState(state);
}
void CScheduledRecording::stopRecording(int state)
{
    switch(state)
    {
    case SUCCEED :
        setRecordState(m_recording_program.getName(),SUCCEED);
        break;
    case CANCELED :
        setRecordState(m_recording_program.getName(),CANCELED);
        break;
    }

    TimeShiftStop();
    saveToXml(m_schedules);
}

void CScheduledRecording::exitScheduledRecording()
{
    if(m_recording_program.getState() == RECORDING)
    {
        setRecordState(m_recording_program.getName(),CANCELED);
        TimeShiftStop();
        saveToXml(m_schedules);
    }

    if (gThreadId != NULL)
    {
        SetThreadPriority(gThreadId, THREAD_PRIORITY_NORMAL);

        // Signal the stop
        m_bExitThread = TRUE;

        // Wait one second for the thread to exit gracefully
        DWORD dwResult = WaitForSingleObject(gThreadId, 1000);

        if (dwResult != WAIT_OBJECT_0)
        {
            LOG(3,"Timeout waiting for RecordingThread to exit, terminating it via TerminateThread()");
            TerminateThread(gThreadId, 0);
        }

        CloseHandle(gThreadId);
        gThreadId = NULL;
    }
}
