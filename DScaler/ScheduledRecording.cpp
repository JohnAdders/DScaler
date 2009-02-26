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
#include <afxdtctl.h>
#include <vector>

CScheduledRecording* pSchRec;
CScheduleDlg* pSchDlg;

enum STATE {READY,SUCCEED,CANCELED,RECORDING};

/*****************************************
*    Global initialization functions
******************************************/
void ShowSchedRecDlg()
{    
    CScheduleDlg schDlg;
    schDlg.DoModal();
}

DWORD WINAPI RecordThreadProc(LPVOID lParam)
{
    pSchRec = new CScheduledRecording();
    while(pSchRec->run())
    {}
    
    return 0;
}

/*****************************************
*    CScheduleDlg functions
******************************************/
CScheduleDlg::CScheduleDlg(CWnd *pParent)
    : CDialog(IDD_SCHEDULE, pParent),
    m_bPrevTimeSet(true),
    m_bCtrlsInit(false)
{
}

CScheduleDlg::~CScheduleDlg()
{
}

BOOL CScheduleDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    pSchDlg = this;
    InitCtrls();
    pSchRec->showRecords();
            
    return TRUE;
}

BEGIN_MESSAGE_MAP(CScheduleDlg, CDialog)
    //{{AFX_MSG_MAP(CScheduleDlg)
    ON_BN_CLICKED(IDC_SCHEDULE_ADD, OnAddClicked)
    ON_BN_CLICKED(IDC_SCHEDULE_REMOVE, OnRemoveClicked)
    ON_BN_CLICKED(IDC_SCHEDULE_OK, OnOkClicked)
    ON_EN_UPDATE(IDC_SCHEDULE_EDIT_DURATION, OnDurationUpdate)
    ON_NOTIFY(DTN_DATETIMECHANGE, IDC_SCHEDULE_TIMEPICKER, OnTimePickerChanged)
    ON_NOTIFY(DTN_DATETIMECHANGE, IDC_SCHEDULE_ENDTIMEPICKER, OnEndTimePickerChanged)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CScheduleDlg::OnDscalerExit()
{
    pSchRec->exitScheduledRecording();
}

void CScheduleDlg::OnAddClicked()
{
    pSchRec->addSchedule();    
}

void CScheduleDlg::OnDurationUpdate()
{
    if(!m_bCtrlsInit || !(::GetFocus() == (GetDlgItem(IDC_SCHEDULE_EDIT_DURATION))->GetSafeHwnd()))
        return;

    CString str_dur;
    GetDlgItemText(IDC_SCHEDULE_EDIT_DURATION, str_dur);
    
    for(int i=0; i<str_dur.GetLength(); i++)
    {
        const char chEntered = str_dur.GetAt(i);
        
        if(::IsCharAlphaA(chEntered))
        {
            ((CEdit*)GetDlgItemText(IDC_SCHEDULE_EDIT_DURATION, str_dur))->SetSel(i,i);
            ((CEdit*)GetDlgItemText(IDC_SCHEDULE_EDIT_DURATION, str_dur))->Clear();
            return;
        }
    }

    if(!m_bPrevTimeSet)
    {
        ((CDateTimeCtrl*)GetDlgItem(IDC_SCHEDULE_TIMEPICKER))->GetTime(&prev_time);
        m_bPrevTimeSet = true;
    }

    CTimeSpan end_timespan(0, 0, atoi(str_dur.GetBuffer(0)), 0);
    CTime new_time(prev_time);

    new_time += end_timespan;
    ((CDateTimeCtrl*)GetDlgItem(IDC_SCHEDULE_ENDTIMEPICKER))->SetTime(&new_time);
}

void CScheduleDlg::OnTimePickerChanged(NMHDR* nmhdr, LRESULT* lResult)
{
    if(!m_bCtrlsInit)
        return;

    SYSTEMTIME end_time;
    ((CDateTimeCtrl*)GetDlgItem(IDC_SCHEDULE_ENDTIMEPICKER))->GetTime(&end_time);
    
    CTimeSpan prev_timespan(prev_time.wDay, prev_time.wHour, prev_time.wMinute, 0);
    CTimeSpan end_timespan(end_time.wDay, end_time.wHour, end_time.wMinute, 0);
    
    SYSTEMTIME start_time;
    ((CDateTimeCtrl*)GetDlgItem(IDC_SCHEDULE_TIMEPICKER))->GetTime(&start_time);
    CTimeSpan start_timespan(start_time.wDay, start_time.wHour, start_time.wMinute, 0);
    
    if(start_timespan > end_timespan)
    {
        SetDurationCtrl(0);
        return;
    }

    CTimeSpan dur_timespan = end_timespan - start_timespan;
    SetDurationCtrl((int)dur_timespan.GetTotalMinutes());
    m_bPrevTimeSet = false;
}

void CScheduleDlg::OnEndTimePickerChanged(NMHDR* nmhdr, LRESULT* lResult)
{
    if(!m_bCtrlsInit)
        return;

    CTime start_time, end_time;
    
    ((CDateTimeCtrl*)GetDlgItem(IDC_SCHEDULE_TIMEPICKER))->GetTime(start_time);
    ((CDateTimeCtrl*)GetDlgItem(IDC_SCHEDULE_ENDTIMEPICKER))->GetTime(end_time);

    if(end_time < start_time)
    {
        SetDurationCtrl(0);
        return;
    }
    
    CTimeSpan dur_timespan = end_time - start_time;
    SetDurationCtrl((int)dur_timespan.GetTotalMinutes());
}

void CScheduleDlg::SetDurationCtrl(int minutes)
{
    char chDur[20];
    _itoa(minutes, chDur, 10);
    SetDlgItemText(IDC_SCHEDULE_EDIT_DURATION, chDur);
}

void CScheduleDlg::OnRemoveClicked()
{
    pSchRec->removeSchedule();
}

void CScheduleDlg::OnOkClicked()
{
    pSchRec->saveOnClose();
    int i = 0;
    this->EndDialog(i);
}

void CScheduleDlg::InitCtrls()
{       
    CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_SCHEDULE_LIST);
    pList->InsertColumn(0, "Name", 0, 70);
    pList->InsertColumn(1, "Program",0, 70);
    pList->InsertColumn(2, "Date", 0, 80);
    pList->InsertColumn(3, "Start", 0, 60);
    pList->InsertColumn(4, "Duration(minutes)", 0, 100);
    pList->InsertColumn(5, "State", 0, 60);
    pList->SetExtendedStyle(LVS_EX_FULLROWSELECT);
    
    CTime time_now = CTime::GetCurrentTime();
    
    SetDlgItemText(IDC_SCHEDULE_EDIT_NAME,"Name");
    SetDlgItemText(IDC_SCHEDULE_EDIT_DURATION,"90");
    ((CDateTimeCtrl*)GetDlgItem(IDC_SCHEDULE_DATEPICKER))->SetTime(&time_now);
    ((CDateTimeCtrl*)GetDlgItem(IDC_SCHEDULE_TIMEPICKER))->SetTime(&time_now);
    
    CTimeSpan end_time_span(0,1,30,0);
    time_now += end_time_span;
    ((CDateTimeCtrl*)GetDlgItem(IDC_SCHEDULE_ENDTIMEPICKER))->SetTime(&time_now);
    
    ((CEdit*)GetDlgItem(IDC_SCHEDULE_EDIT_NAME))->SetLimitText(19);
    ((CEdit*)GetDlgItem(IDC_SCHEDULE_EDIT_DURATION))->SetLimitText(3);
    
    std::vector<std::string> channels;
    pSchRec->getChannels(channels);
    
    if(channels.empty())
        return;

    for(std::vector<std::string>::iterator iter=channels.begin(); iter != channels.end(); iter++)
    {
        ((CComboBox*)GetDlgItem(IDC_SCHEDULE_COMBO))->AddString(iter->c_str());
    }
    
    ((CComboBox*)GetDlgItem(IDC_SCHEDULE_COMBO))->SelectString(0, channels[0].c_str());
    ((CDateTimeCtrl*)GetDlgItem(IDC_SCHEDULE_TIMEPICKER))->GetTime(&prev_time);

    m_bCtrlsInit = true;
}

/*****************************************
*    CSchedule functions
******************************************/
CSchedule::CSchedule(char* name, char* program_name, int duration, int state, CTime time)
{    
    strcpy(m_name,name);
    strcpy(m_program_name,program_name);

    m_duration = duration;
    m_state = state;
    m_time_start = time;
}

/*****************************************
*    CScheduledRecording funtions
******************************************/
CScheduledRecording::CScheduledRecording():m_bSchedulesUpdate(true),m_bExitThread(false)
{}

bool CScheduledRecording::run()
{
    while(!processSchedules())
    {
        if(m_bExitThread)
            return false;
        else
            SleepEx(500,false);
    }
    while(!IsEndOfRecording())
    {
        if(m_bExitThread)
            return false;
        else
        {
            ShowText(GetMainWnd(), "Recording");
            SleepEx(500,false);
        }
    }
    
    return true;
}

void CScheduledRecording::initScheduledRecordingThreadProc()
{
    CreateThread(NULL,0,RecordThreadProc,NULL,0,0);
}

void CScheduledRecording::getChannels(std::vector<std::string> &channels)
{
    CUserChannels MyChannels;
    MyChannels.ReadASCII(SZ_DEFAULT_PROGRAMS_FILENAME);
        
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

void CScheduledRecording::addSchedule()
{
    CListCtrl *list = (CListCtrl*) pSchDlg->GetDlgItem(IDC_SCHEDULE_LIST);
    CComboBox *comboCtrl = (CComboBox*) pSchDlg->GetDlgItem(IDC_SCHEDULE_COMBO);
    CDateTimeCtrl *dateCtrl = (CDateTimeCtrl*) pSchDlg->GetDlgItem(IDC_SCHEDULE_DATEPICKER);
    CDateTimeCtrl *timeCtrl = (CDateTimeCtrl*) pSchDlg->GetDlgItem(IDC_SCHEDULE_TIMEPICKER);

    CString name;
    CString program;
    CString strDuration;
        
    pSchDlg->GetDlgItemText(IDC_SCHEDULE_EDIT_NAME,name);
    
    if(name.IsEmpty())
    {
        MessageBox(GetMainWnd(),"Enter name for schedule","Error",MB_OK);
        return;
    }

    for(std::vector<CSchedule>::iterator iter=m_schedules.begin();iter!=m_schedules.end();iter++)
        if(strcmp(iter->getName(),name.GetBuffer(0)) == 0)
        {
            char err_text[50];
            strcpy(err_text,"Schedule name ");
            
            strcat(err_text,name.GetBuffer(0));
            strcat(err_text,"already used");
            
            MessageBox(GetMainWnd(),err_text,"Error",MB_OK);
            return;
        }
    
    pSchDlg->GetDlgItemText(IDC_SCHEDULE_EDIT_DURATION,strDuration);

    if(strDuration.IsEmpty())
    {
        MessageBox(GetMainWnd(),"Enter recording time in minutes","Error",MB_OK);
        return;
    }

    int duration = atoi(strDuration.GetBuffer(0));
    
    comboCtrl->GetWindowText(program);
    
    SYSTEMTIME date;
    SYSTEMTIME time;
    
    dateCtrl->GetTime(&date);  
    timeCtrl->GetTime(&time);

    CTime time_start(date.wYear,date.wMonth,date.wDay,time.wHour,time.wMinute,0);
    
    char chName[20];
    char chProgramName[15];

    strcpy(chName,name.GetBuffer(0));
    strcpy(chProgramName,program.GetBuffer(0));
    
    CSchedule schedule(chName,chProgramName,duration,READY,time_start);
    
    m_schedules.push_back(schedule);
    showRecords();
}

void CScheduledRecording::removeSchedule()
{
    CListCtrl *list;
    
    list = (CListCtrl*)pSchDlg->GetDlgItem(IDC_SCHEDULE_LIST);
    int i;
    for(i=0;i<list->GetItemCount();i++)
    {
        if(list->GetItemState(i,LVIS_SELECTED) == LVIS_SELECTED)
        {    
            CString item_text;
            item_text = list->GetItemText(i,0);
            
            for(std::vector<CSchedule>::iterator iter=m_schedules.begin();iter!=m_schedules.end();iter++)
                if(strcmp(iter->getName(),item_text.GetBuffer(0)) == 0)
                {
                    if(iter->getState() == RECORDING)
                    {
                        MessageBox(GetMainWnd(),"Schedule is already recording...cannot be removed","Error",MB_OK);
                        return;
                    }

                    m_schedules.erase(iter);
                    
                    break;
                }
            break;
        }
    }
    
    list->DeleteItem(i);
    showRecords();
}
void CScheduledRecording::saveOnClose()
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
        char chRecord_number[10];
        char chBuff[10];
        
        _itoa(r_pos++,chBuff,10);
        strcpy(chRecord_number,"R");
        chRecord_number[1] = 0;
        strcat(chRecord_number,(const char*)chBuff);
        
        TiXmlElement* record = new TiXmlElement((const char*)chRecord_number);
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
        _itoa(time_start.GetYear(),chBuff,10);
        year->LinkEndChild(new TiXmlText(chBuff));
        record->LinkEndChild(year);

        TiXmlElement* month = new TiXmlElement("Month");
        memset(chBuff,0,sizeof(char)*10);
        _itoa(time_start.GetMonth(),chBuff,10);
        month->LinkEndChild(new TiXmlText(chBuff));
        record->LinkEndChild(month);

        TiXmlElement* day = new TiXmlElement("Day");
        memset(chBuff,0,sizeof(char)*10);
        _itoa(time_start.GetDay(),chBuff,10);
        day->LinkEndChild(new TiXmlText(chBuff));
        record->LinkEndChild(day);

        TiXmlElement* hour = new TiXmlElement("Hour");
        memset(chBuff,0,sizeof(char)*10);
        _itoa(time_start.GetHour(),chBuff,10);
        hour->LinkEndChild(new TiXmlText(chBuff));
        record->LinkEndChild(hour);
    
        TiXmlElement* minute = new TiXmlElement("Minute");
        memset(chBuff,0,sizeof(char)*10);
        _itoa(time_start.GetMinute(),chBuff,10);
        minute->LinkEndChild(new TiXmlText(chBuff));
        record->LinkEndChild(minute);

        TiXmlElement* duration = new TiXmlElement("Duration");
        memset(chBuff,0,sizeof(char)*10);
        _itoa(iter->getDuration(),chBuff,10);
        duration->LinkEndChild(new TiXmlText(chBuff));
        record->LinkEndChild(duration);

        TiXmlElement* state = new TiXmlElement("State");
        memset(chBuff,0,sizeof(char)*10);
        _itoa(iter->getState(),chBuff,10);
        state->LinkEndChild(new TiXmlText(chBuff));
        record->LinkEndChild(state);
    }

    TiXmlElement* records_count = new TiXmlElement("RecordsCount");
    records_table->LinkEndChild(records_count);
    
    char chRecord_count[4];
    _itoa(r_count,chRecord_count,10);
    
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
        char chRecord_number[10];
        
        _itoa(r_pos,chBuff,10);
        strcpy(chRecord_number,"R");
        chRecord_number[1] = 0;
        strcat(chRecord_number,chBuff);
        
        hRoot = hDoc.FirstChildElement("RecordsTable").FirstChildElement(chRecord_number).Element();
        hRecord = TiXmlHandle(hRoot);
        
        char chName[20];
        char chProgram[15];
        
        strcpy(chName,hRecord.FirstChildElement("Name").Element()->GetText());
        strcpy(chProgram,hRecord.FirstChildElement("Program").Element()->GetText());
        
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

bool CScheduledRecording::processSchedules()
{
    if(m_bSchedulesUpdate)
        loadFromXml();
        
    if(!m_schedules.size())
        return false;

    m_bSchedulesUpdate = false;
    
    CTime time_now = CTime::GetCurrentTime();
        
    for(std::vector<CSchedule>::iterator iter = m_schedules.begin();iter != m_schedules.end();iter++)
    {    
        CTime time_schedule = iter->getStartTime();
        
        if(time_now == time_schedule && iter->getState() == READY)
        {
            startRecording(*iter,time_schedule);
            return true;
        }
    }
    return false;
}

void CScheduledRecording::showRecords()
{
    CListCtrl *list = (CListCtrl*)pSchDlg->GetDlgItem(IDC_SCHEDULE_LIST);
    list->DeleteAllItems();
    
    if(!m_schedules.size())
        return;

    for(std::vector<CSchedule>::iterator iter=m_schedules.begin();iter!=m_schedules.end();iter++)
    {    
        char chBuff[4];
        _itoa(iter->getDuration(),chBuff,10);

        list->InsertItem(0,iter->getName());
        list->SetItem(0,1,LVIF_TEXT,iter->getProgramName(),0,0,0,0);

        char chDate[13];
        iter->getDateStr(chDate);
        list->SetItem(0,2,LVIF_TEXT,chDate,0,0,0,0);

        char chTime[6];
        iter->getTimeStr(chTime);
        list->SetItem(0,3,LVIF_TEXT,chTime,0,0,0,0);

        list->SetItem(0,4,LVIF_TEXT,chBuff,0,0,0,0);
        
        switch(iter->getState())
        {
        case CANCELED:
            list->SetItem(0,5,LVIF_TEXT,"Canceled",0,0,0,0);
            break;
        case READY:
            list->SetItem(0,5,LVIF_TEXT,"Ready",0,0,0,0);
            break;
        case SUCCEED:
            list->SetItem(0,5,LVIF_TEXT,"Succeed",0,0,0,0);
            break;
        case RECORDING:
            list->SetItem(0,5,LVIF_TEXT,"Recording",0,0,0,0);
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
    
    const char* current_channel = Channel_GetName();
    
    HMENU hMenu = LoadMenu(hResourceInst, MAKEINTRESOURCE(IDC_DSCALERMENU));
    
    if(strcmp(current_channel,m_recording_program.getProgramName()) == 0)      
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

                MyChannels.ReadASCII(SZ_DEFAULT_PROGRAMS_FILENAME);
                int channel_number = MyChannels.GetChannelNumber(++channel_index);

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

bool CScheduledRecording::IsEndOfRecording()
{
    CTime time_now = CTime::GetCurrentTime();
    
    if(m_recording_program.getTimeEnd() != time_now && TimeShiftIsRunning())
        return false;
    else
    {
        if(!TimeShiftIsRunning())
            stopRecording(CANCELED);
        
        if(m_recording_program.getTimeEnd() == time_now)
            stopRecording(SUCCEED);
        
        return true;
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
    
    m_bExitThread = true;
}
