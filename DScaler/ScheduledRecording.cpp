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

const __int64 SystemTimeUnitsPerSec(10000000);

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
    DScalerThread thisThread(_T("Scheduled Recording Thread"));
    /// \todo proper thread control
    pSchRec = new CScheduledRecording();
    while(pSchRec.IsValid() && pSchRec->run())
    {}

    return 0;
}

// returns true if times are within 1 second of each other
BOOL AreTimesClose(const SYSTEMTIME& lhs, const SYSTEMTIME& rhs)
{
    __int64 FileTimeLeft;
    __int64 FileTimeRight;
    SystemTimeToFileTime(&lhs, (LPFILETIME)&FileTimeLeft);
    SystemTimeToFileTime(&rhs, (LPFILETIME)&FileTimeRight);
    __int64 diff(FileTimeRight- FileTimeLeft);
    return ((diff > -SystemTimeUnitsPerSec) && (diff < SystemTimeUnitsPerSec));
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

INT_PTR CScheduleDlg::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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

    tstring str_dur(GetDlgItemString(hDlg, IDC_SCHEDULE_EDIT_DURATION));

    SYSTEMTIME SysTime;

    DateTime_GetSystemtime(GetDlgItem(hDlg,IDC_SCHEDULE_TIMEPICKER), &SysTime);

    __int64 FileTime;
    SystemTimeToFileTime(&SysTime, (LPFILETIME)&FileTime);

    FileTime += FromString<long>(str_dur) * 60 * SystemTimeUnitsPerSec;

    FileTimeToSystemTime((LPFILETIME)&FileTime, &SysTime);

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

    __int64 StartFileTime;
    __int64 EndFileTime;

    SystemTimeToFileTime(&StartSysTime, (LPFILETIME)&StartFileTime);
    SystemTimeToFileTime(&EndSysTime, (LPFILETIME)&EndFileTime);

    if(EndFileTime < StartFileTime)
    {
        SetDurationCtrl(hDlg, 0);
        return 0;
    }

    __int64 DurTimespan = (EndFileTime - StartFileTime) / (60 * SystemTimeUnitsPerSec);
    SetDurationCtrl(hDlg, (int)DurTimespan);
    return 0;
}

void CScheduleDlg::SetDurationCtrl(HWND hDlg, int minutes)
{
    tstring chDur(ToString(minutes));
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

void ListView_MyInsertColumn(HWND hList, int ColumnIndex, LPCTSTR ColumnHeading, int Width)
{
    LVCOLUMN Column;
    Column.mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH;
    Column.pszText = (LPTSTR)ColumnHeading;
    Column.fmt = 0;
    Column.cx = Width;
    ListView_InsertColumn(hList, ColumnIndex, &Column);
}

void CScheduleDlg::InitCtrls(HWND hDlg)
{
    HWND hList = GetDlgItem(hDlg, IDC_SCHEDULE_LIST);

    ListView_MyInsertColumn(hList, 0, _T("Name"), 70);
    ListView_MyInsertColumn(hList, 1, _T("Program"), 70);
    ListView_MyInsertColumn(hList, 2, _T("Date"), 80);
    ListView_MyInsertColumn(hList, 3, _T("Start"), 60);
    ListView_MyInsertColumn(hList, 4, _T("Duration(minutes)"), 100);
    ListView_MyInsertColumn(hList, 5, _T("State"), 60);
    SendMessage(hList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);

    SYSTEMTIME SysTime;
    GetLocalTime(&SysTime);

    SetDlgItemText(hDlg, IDC_SCHEDULE_EDIT_NAME, _T("Name"));
    SetDlgItemText(hDlg, IDC_SCHEDULE_EDIT_DURATION, _T("90"));
    DateTime_SetSystemtime(GetDlgItem(hDlg,IDC_SCHEDULE_DATEPICKER), GDT_VALID, &SysTime);
    DateTime_SetSystemtime(GetDlgItem(hDlg,IDC_SCHEDULE_TIMEPICKER), GDT_VALID, &SysTime);

    __int64 FileTime;
    SystemTimeToFileTime(&SysTime, (LPFILETIME)&FileTime);
    FileTime += 90 * SystemTimeUnitsPerSec;
    FileTimeToSystemTime((LPFILETIME)&FileTime, &SysTime);

    DateTime_SetSystemtime(GetDlgItem(hDlg,IDC_SCHEDULE_ENDTIMEPICKER), GDT_VALID, &SysTime);

    SendMessage(GetDlgItem(hDlg,IDC_SCHEDULE_EDIT_NAME), EM_SETLIMITTEXT, 19, 0);
    SendMessage(GetDlgItem(hDlg,IDC_SCHEDULE_EDIT_DURATION), EM_SETLIMITTEXT, 3, 0);

    std::vector<tstring> channels;
    pSchRec->getChannels(channels);

    if(!channels.empty())
    {
        for(std::vector<tstring>::iterator iter=channels.begin(); iter != channels.end(); iter++)
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
CSchedule::CSchedule(const tstring& name, const tstring& program_name, int duration, int state, SYSTEMTIME time) :
    m_name(name),
    m_program_name(program_name),
    m_duration(duration),
    m_state(state),
    m_time_start(time)
{
}

tstring CSchedule::getDateStr() const
{
    TCHAR Buffer[200];
    _sntprintf(Buffer, 200, _T("%d.%d.%d"), m_time_start.wYear, m_time_start.wMonth, m_time_start.wDay);
    return Buffer;
}

tstring CSchedule::getTimeStr() const
{
    TCHAR Buffer[200];
    _sntprintf(Buffer, 200, _T("%d:%d"), m_time_start.wHour, m_time_start.wMinute);
    return Buffer;
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
            ShowText(GetMainWnd(), _T("Recording"));
            SleepEx(500,FALSE);
        }
    }

    return 0;
}

void CScheduledRecording::initScheduledRecordingThreadProc()
{
    gThreadId = CreateThread(NULL,0,RecordThreadProc,NULL,0,0);
}

void CScheduledRecording::getChannels(std::vector<tstring> &channels)
{
    CUserChannels MyChannels;
    MyChannels.ReadFile();

    int count = MyChannels.GetSize();

    tstring channel_name;
    const CChannel* channel;

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

    tstring name(GetDlgItemString(hDlg, IDC_SCHEDULE_EDIT_NAME));
    
    if(name.empty())
    {
        MessageBox(GetMainWnd(), _T("Enter name for schedule"), _T("Error"), MB_OK);
        return;
    }

    for(std::vector<CSchedule>::iterator iter=m_schedules.begin();iter!=m_schedules.end();++iter)
    {
        if(iter->getName() == name)
        {
            tstring err_text(_T("Schedule name "));

            err_text += name;
            err_text += _T("already used");

            MessageBox(GetMainWnd(),err_text.c_str(),_T("Error"),MB_OK);
            return;
        }
    }

    tstring strDuration(GetDlgItemString(hDlg, IDC_SCHEDULE_EDIT_DURATION));

    if(strDuration.empty())
    {
        MessageBox(GetMainWnd(),_T("Enter recording time in minutes"),_T("Error"),MB_OK);
        return;
    }

    int duration(FromString<int>(strDuration));

    tstring program(GetDlgItemString(hDlg, IDC_SCHEDULE_COMBO));

    SYSTEMTIME date;
    SYSTEMTIME time;

    DateTime_GetSystemtime(hDateCtrl, &date);
    DateTime_GetSystemtime(hTimeCtrl, &time);

    SYSTEMTIME time_start;
    time_start.wYear = date.wYear;
    time_start.wMonth = date.wMonth;
    time_start.wDay = date.wDay;
    time_start.wHour = time.wHour;
    time_start.wMinute = time.wMinute;
    time_start.wSecond = 0;
    time_start.wMilliseconds = 0;

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
            vector<TCHAR> item_text(MAX_PATH);
            ListView_GetItemText(list, i, 0, &item_text[0], MAX_PATH);

            for(std::vector<CSchedule>::iterator iter = m_schedules.begin(); iter!=m_schedules.end(); ++iter)
            {
                if(_tcscmp(iter->getName(), &item_text[0]) == 0)
                {
                    if(iter->getState() == RECORDING)
                    {
                        MessageBox(GetMainWnd(),_T("Schedule is already recording...cannot be removed"),_T("Error"),MB_OK);
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
        CHAR chBuff[10];

        chRecord_number += _itoa(r_pos++,chBuff, 10);

        TiXmlElement* record = new TiXmlElement(chRecord_number);
        records_table->LinkEndChild(record);

        TiXmlElement* name = new TiXmlElement("Name");
        name->LinkEndChild(new TiXmlText(TStringToMBCS(iter->getName())));
        record->LinkEndChild(name);

        TiXmlElement* program = new TiXmlElement("Program");
        program->LinkEndChild(new TiXmlText(TStringToMBCS(iter->getProgramName())));
        record->LinkEndChild(program);

        SYSTEMTIME time_start = iter->getStartTime();

        TiXmlElement* year = new TiXmlElement("Year");
        year->LinkEndChild(new TiXmlText(ToMBCSString(time_start.wYear)));
        record->LinkEndChild(year);

        TiXmlElement* month = new TiXmlElement("Month");
        month->LinkEndChild(new TiXmlText(ToMBCSString(time_start.wMonth)));
        record->LinkEndChild(month);

        TiXmlElement* day = new TiXmlElement("Day");
        day->LinkEndChild(new TiXmlText(ToMBCSString(time_start.wDay)));
        record->LinkEndChild(day);

        TiXmlElement* hour = new TiXmlElement("Hour");
        hour->LinkEndChild(new TiXmlText(ToMBCSString(time_start.wHour)));
        record->LinkEndChild(hour);

        TiXmlElement* minute = new TiXmlElement("Minute");
        minute->LinkEndChild(new TiXmlText(ToMBCSString(time_start.wMinute)));
        record->LinkEndChild(minute);

        TiXmlElement* duration = new TiXmlElement("Duration");
        duration->LinkEndChild(new TiXmlText(ToMBCSString(iter->getDuration())));
        record->LinkEndChild(duration);

        TiXmlElement* state = new TiXmlElement("State");
        state->LinkEndChild(new TiXmlText(ToMBCSString(iter->getState())));
        record->LinkEndChild(state);
    }

    TiXmlElement* records_count = new TiXmlElement("RecordsCount");
    records_table->LinkEndChild(records_count);

    records_count->LinkEndChild(new TiXmlText(ToMBCSString(r_count)));

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
        string chRecord_number("R");

        chRecord_number += ToMBCSString(r_pos);

        hRoot = hDoc.FirstChildElement("RecordsTable").FirstChildElement(chRecord_number).Element();
        hRecord = TiXmlHandle(hRoot);

        string chName(hRecord.FirstChildElement("Name").Element()->GetText());
        string chProgram(hRecord.FirstChildElement("Program").Element()->GetText());

        int year = atoi(hRecord.FirstChildElement("Year").Element()->GetText());
        int month = atoi(hRecord.FirstChildElement("Month").Element()->GetText());
        int day = atoi(hRecord.FirstChildElement("Day").Element()->GetText());
        int hour = atoi(hRecord.FirstChildElement("Hour").Element()->GetText());
        int minute = atoi(hRecord.FirstChildElement("Minute").Element()->GetText());

        SYSTEMTIME time_start = {year,month,-1, day,hour,minute,0, 0};

        int duration = atoi(hRecord.FirstChildElement("Duration").Element()->GetText());
        int state = atoi(hRecord.FirstChildElement("State").Element()->GetText());


        CSchedule schedule(
                            MBCSToTString(chName),
                            MBCSToTString(chProgram),
                            duration,
                            state,
                            time_start
                          );
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

    SYSTEMTIME time_now;
    GetLocalTime(&time_now);

    for(std::vector<CSchedule>::iterator iter = m_schedules.begin();iter != m_schedules.end();iter++)
    {
        SYSTEMTIME time_schedule = iter->getStartTime();

        if(AreTimesClose(time_now, time_schedule) && iter->getState() == READY)
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
        TCHAR chBuff[4];
        _itot(iter->getDuration(),chBuff,10);

        LVITEM ListItem;
        memset(&ListItem, 0, sizeof(LVITEM));
        ListItem.mask = LVIF_TEXT;
        ListItem.iItem = 0;
        ListItem.pszText = (TCHAR*)iter->getName();
        ListView_InsertItem(list, &ListItem);
        ListView_SetItemText(list, 0, 1, (LPTSTR)iter->getProgramName());

        tstring chDate(iter->getDateStr());
        ListView_SetItemText(list, 0, 2, (LPTSTR)chDate.c_str());

        tstring chTime(iter->getTimeStr());
        ListView_SetItemText(list, 0, 3, (LPTSTR)chTime.c_str());

        ListView_SetItemText(list, 0, 4, chBuff);

        switch(iter->getState())
        {
        case CANCELED:
            ListView_SetItemText(list, 0,5,_T("Canceled"));
            break;
        case READY:
            ListView_SetItemText(list, 0,5,_T("Ready"));
            break;
        case SUCCEED:
            ListView_SetItemText(list, 0,5,_T("Succeed"));
            break;
        case RECORDING:
            ListView_SetItemText(list, 0,5,_T("Recording"));
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


void CScheduledRecording::startRecording(CSchedule recording_program, SYSTEMTIME time_start)
{
    __int64 FileTime;
    SystemTimeToFileTime(&time_start, (LPFILETIME)&FileTime);
    FileTime += recording_program.getDuration() * 60 * SystemTimeUnitsPerSec;
    SYSTEMTIME time_end;
    FileTimeToSystemTime((LPFILETIME)&FileTime, &time_end);
    recording_program.setTimeEnd(time_end);

    recording_program.setState(RECORDING);
    m_recording_program = recording_program;

    setRecordState(m_recording_program.getName(),RECORDING);

    tstring current_channel = Channel_GetName();

    HMENU hMenu = LoadMenu(hResourceInst, MAKEINTRESOURCE(IDC_DSCALERMENU));

    if(current_channel == m_recording_program.getProgramName())
    {
        if(TimeShiftRecord())
        {
            ShowText(GetMainWnd(), _T("Recording"));
            TimeShiftOnSetMenu(hMenu);
        }
    }
    else
    {
        std::vector<tstring> channels;
        int channel_index = 0;

        getChannels(channels);

        for(std::vector<tstring>::iterator iter=channels.begin();iter != channels.end();iter++)
        {
            if(_tcscmp(iter->c_str(),m_recording_program.getProgramName()) == 0)
            {
                CUserChannels MyChannels;

                MyChannels.ReadFile();
                int channel_number = MyChannels.GetChannel(++channel_index)->GetChannelNumber();

                Channel_ChangeToNumber(channel_number);

                if(TimeShiftRecord())
                {
                    ShowText(GetMainWnd(), _T("Recording"));
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
    SYSTEMTIME time_now;
    GetLocalTime(&time_now);

    if(!AreTimesClose(m_recording_program.getTimeEnd(), time_now) && TimeShiftIsRunning())
        return FALSE;
    else
    {
        if(!TimeShiftIsRunning())
            stopRecording(CANCELED);

        if(AreTimesClose(m_recording_program.getTimeEnd(), time_now))
            stopRecording(SUCCEED);

        return TRUE;
    }

}

void CScheduledRecording::setRecordState(const TCHAR* schedule_name, int state)
{
    for(std::vector<CSchedule>::iterator iter=m_schedules.begin();iter!=m_schedules.end();iter++)
        if(_tcscmp(iter->getName(),schedule_name) == 0 )
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
            LOG(3,_T("Timeout waiting for RecordingThread to exit, terminating it via TerminateThread()"));
            TerminateThread(gThreadId, 0);
        }

        CloseHandle(gThreadId);
        gThreadId = NULL;
    }
}
