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

class CScheduleDlg : public CDialog
{
public:
    CScheduleDlg(CWnd *pParent = NULL);   // standard constructor
    ~CScheduleDlg();
    static void OnDscalerExit();

protected:
    //{{AFX_MSG(ScheduleDlg)
    virtual BOOL OnInitDialog();
    afx_msg void OnAddClicked();
    afx_msg void OnRemoveClicked();
    afx_msg void OnOkClicked();
    afx_msg void OnDurationUpdate();
    afx_msg void OnTimePickerChanged(NMHDR* nmhdr, LRESULT* lResult);
    afx_msg void OnEndTimePickerChanged(NMHDR* nmhdr, LRESULT* lResult);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    void InitCtrls();
    void SetDurationCtrl(int minutes);

    SYSTEMTIME prev_time;
    BOOL m_bPrevTimeSet;
    BOOL m_bCtrlsInit;
};

void ShowSchedRecDlg();
DWORD WINAPI RecordThreadProc();

class CSchedule
{
public:
    CSchedule(){};
    CSchedule(const std::string& schedule_name, const std::string& program_name, int duration, int state, CTime time);
    ~CSchedule(){};
        
    const char* getName()         const   {return m_name.c_str();};
    const char* getProgramName()  const   {return m_program_name.c_str();};
    std::string getDateStr() const   {return m_time_start.Format("%m.%d.%Y").GetBuffer(0);};  
    std::string getTimeStr() const   {return m_time_start.Format("%H:%M").GetBuffer(0);};  
    int getDuration()             const   {return m_duration;};
    CTime getStartTime()          const   {return m_time_start;};
    CTime getTimeEnd()            const   {return m_time_end;};
    int getState()                const   {return m_state;};
    
    void setTimeEnd(CTime time) {m_time_end = time;};
    void setState(int state) {m_state = state;};

private:
    std::string m_name;
    std::string m_program_name;
    
    CTime m_time_start;
    CTime m_time_end;
    
    int m_duration;
    int m_state;
};

class CScheduledRecording
{
public:
    CScheduledRecording();
    
    static void initScheduledRecordingThreadProc();
    void exitScheduledRecording();

    void showRecords();
    void addSchedule();
    void removeSchedule();
    void saveOnClose();
    
    void getChannels(std::vector<std::string> &channels);
    
    BOOL run();
                
private:
    void loadFromXml();
    void saveToXml(std::vector<CSchedule> schedules);
    void setRecordState(const char* schedule_name, int state);
    int  getRecordsCount();
    
    BOOL IsEndOfRecording();
    BOOL processSchedules();
    void stopRecording(int state);
    void startRecording(CSchedule recording_program, CTime time_start);

    BOOL m_bSchedulesUpdate;
    BOOL m_bExitThread;

    std::vector<CSchedule> m_schedules;
    CSchedule m_recording_program;
};
