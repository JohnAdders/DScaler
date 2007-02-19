/////////////////////////////////////////////////////////////////////////////
// $Id: ScheduledRecording.h,v 1.1 2007-02-19 00:28:04 robmuller Exp $
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
// CVS Log
//
// $Log: not supported by cvs2svn $
//
///////////////////////////////////////////////////////////////////////////////

class ScheduleDlg :public CDialog
{
public:
	ScheduleDlg(CWnd *pParent = NULL);
	virtual BOOL OnInitDialog();
	
	void setCtrls();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnAddClicked();
	afx_msg void OnRemoveClicked();
	afx_msg void OnOkClicked();
	afx_msg void OnDurationUpdate();
	afx_msg void OnTimePickerChanged(NMHDR* nmhdr, LRESULT* lResult);
	afx_msg void OnEndTimePickerChanged(NMHDR* nmhdr, LRESULT* lResult);
	afx_msg LRESULT OnDscalerExit(WPARAM wParam, LPARAM lParam);

private:
	void setDurationCtrl(int minutes);

	SYSTEMTIME prev_time;
	bool m_bPrevTimeSet;
	bool m_bCtrlsInit;
};

void ScheduledRecordingDlg();
DWORD WINAPI RecordThreadProc();

class CSchedule
{
public:
	CSchedule(){};
	CSchedule(char* schedule_name, char* program_name, int duration, int state, CTime time);
	~CSchedule(){};
		
	char* getName()        const   {return m_name;};
	char* getProgramName() const   {return m_program_name;};
    char* getDateStr()     const;  
	char* getTimeStr()     const;  
	int getDuration()      const   {return m_duration;};
	CTime getStartTime()   const   {return m_time_start;};
	CTime getTimeEnd()     const   {return m_time_end;};
	int getState()         const   {return m_state;};
	
	void setTimeEnd(CTime time) {m_time_end = time;};
	void setState(int state) {m_state = state;};

	void clearData();
	
private:
	char* m_name;
	char* m_program_name;
	
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
	
	bool run();
				
private:
	void loadFromXml();
	void saveToXml(std::vector<CSchedule> schedules);
	void setRecordState(char* schedule_name, int state);
	int  getRecordsCount();
	
	bool IsEndOfRecording();
	bool processSchedules();
	void stopRecording(int state);
	void startRecording(CSchedule recording_program, CTime time_start);

	bool m_bSchedulesUpdate;
	bool m_bExitThread;

	std::vector<CSchedule> m_schedules;
	CSchedule m_recording_program;
};
