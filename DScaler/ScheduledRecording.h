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

#if !defined(__SCHEDULED_RECORDING_H_)
#define __SCHEDULED_RECORDING_H_

#include "DSDialog.h"

class CScheduleDlg : public CDSDialog
{
public:
    CScheduleDlg();   // standard constructor
    ~CScheduleDlg();
    static void OnDscalerExit();


private:
    virtual INT_PTR DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    BOOL OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam);
    void OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify);
    LRESULT OnNotify(HWND hwnd, int id, LPNMHDR nmhdr);
    void InitCtrls(HWND hDlg);
    void SetDurationCtrl(HWND hDlg, int minutes);
    void OnAddClicked(HWND hDlg);
    void OnRemoveClicked(HWND hDlg);
    void OnOkClicked(HWND hDlg);
    void OnDurationUpdate(HWND hDlg);
    LRESULT OnTimePickerChanged(HWND hDlg, LPNMHDR nmhdr);

    BOOL m_bCtrlsInit;
};

void ShowSchedRecDlg();
DWORD WINAPI RecordThreadProc();

class CSchedule
{
public:
    CSchedule(){};
    CSchedule(const tstring& schedule_name, const tstring& program_name, int duration, int state, SYSTEMTIME time);
    ~CSchedule(){};

    const TCHAR* getName()           const   {return m_name.c_str();};
    const TCHAR* getProgramName()    const   {return m_program_name.c_str();};
    tstring getDateStr()        const;
    tstring getTimeStr()        const;
    int getDuration()               const   {return m_duration;};
    SYSTEMTIME getStartTime()            const   {return m_time_start;};
    SYSTEMTIME getTimeEnd()              const   {return m_time_end;};
    int getState()                  const   {return m_state;};

    void setTimeEnd(SYSTEMTIME time) {m_time_end = time;};
    void setState(int state) {m_state = state;};

private:
    tstring m_name;
    tstring m_program_name;

    SYSTEMTIME m_time_start;
    SYSTEMTIME m_time_end;

    int m_duration;
    int m_state;
};

class CScheduledRecording
{
public:
    CScheduledRecording();

    static void initScheduledRecordingThreadProc();
    void exitScheduledRecording();

    void showRecords(HWND hDlg);
    void addSchedule(HWND hDlg);
    void removeSchedule(HWND hDlg);
    void saveOnClose(HWND hDlg);

    void getChannels(std::vector<tstring> &channels);

    BOOL run();

private:
    void loadFromXml();
    void saveToXml(std::vector<CSchedule> schedules);
    void setRecordState(const TCHAR* schedule_name, int state);
    int  getRecordsCount();

    BOOL IsEndOfRecording();
    BOOL processSchedules();
    void stopRecording(int state);
    void startRecording(CSchedule recording_program, SYSTEMTIME time_start);

    BOOL m_bSchedulesUpdate;
    BOOL m_bExitThread;

    std::vector<CSchedule> m_schedules;
    CSchedule m_recording_program;
};

#endif
