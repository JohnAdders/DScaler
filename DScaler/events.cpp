/////////////////////////////////////////////////////////////////////////////
// $Id: events.cpp,v 1.6 2002-10-07 20:29:48 kooiman Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
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
// Revision 1.5  2002/10/02 10:52:35  kooiman
// Fixed C++ type casting for events.
//
// Revision 1.4  2002/09/28 13:34:07  kooiman
// Added sender object to events and added setting flag to treesettingsgeneric.
//
// Revision 1.3  2002/09/27 14:11:35  kooiman
// Added audio standard detect event & implemented event scheduler.
//
// Revision 1.2  2002/09/26 16:34:19  kooiman
// Lots of toolbar fixes &added EVENT_VOLUME support.
//
// Revision 1.1  2002/09/25 22:33:06  kooiman
// Event collector
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Events.h"
#include "DebugLog.h"
#include "Crash.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/**
    The event collector has one global instance 'EventCollector'.
    From everywhere in the code (also from threads), you can raise 
    an event and all registered objects get notified through
    a timer in the main program thread.
    This eliminates some thread difficulties.
    
    To raise an event, use:    
    EventCollector->RaiseEvent(...), with arguments:
        -Sender object.
         A class derived from CEventObject or NULL for a global event.
        -EventType. Defined as an enum list in Events.h
         You can add new event if you want to at the end of the list.
        -Old & new value. Depends on event. E.g. old/new channel or old/new video format
        -ComingUp event list. If you know you will send more events right
         after this one, you can specify that as an array of event types,
         ending with EVENT_ENDOFLIST
         Right now, it's up to the receiver to do something with that.
         Set to NULL if you don't need it.

    You can receive an event if you:
        1: Register the object to the event collector
           EventCollect->Register(this, EventList);
           or EventCollect->Register(EventObject, EventList) for external object           
        
           Specify which events you want to receive in the
           EventList array. Conclude the list with 'EVENT_ENDOFLIST'
           E.g.:
            eEventType EventList[] = {EVENT_CHANNEL_CHANGE,EVENT_VIDEOINPUT_CHANGE,EVENT_ENDOFLIST};

           To stop receiving events before the object's destruction,
           use EventCollector->Unregister(object).
                      
        2: Override the virtual OnEvent(...) function of the CEventObject.
           The arguments are the same as RaiseEvent.
           Test which object raised the event.
           Watch out for C++ type casting. To compare to yourself use
           if (pEventObject == (CEventObject*)this)
    
*/

CEventObject::CEventObject()
{
    //EventCollector->Register(this, List);
}

CEventObject::~CEventObject()
{
    EventCollector->Unregister(this);
}

CEventCollector::CEventCollector()
{	
	m_ScheduleTimerID = 0;
	m_EventCollectorThread = NULL;
    m_bStopThread = FALSE;
	InitializeCriticalSection(&m_EventCriticalSection);
	InitializeCriticalSection(&m_LastEventCriticalSection);
}

CEventCollector::~CEventCollector()
{
    StopThread();
	
	DeleteCriticalSection(&m_EventCriticalSection);
	DeleteCriticalSection(&m_LastEventCriticalSection);
	for (int i = 0; i < m_EventObjects.size(); i++)
    {
        if (m_EventObjects[i].EventList != NULL)
        {
            delete m_EventObjects[i].EventList;        
        }
    }
    m_EventObjects.clear();
}

eEventType *CEventCollector::CopyEventList(eEventType *EventList)
{
    if (EventList == NULL)
    {
        return NULL;
    }
    else
    {
        int n = 0;
        while (EventList[n] != 0) { n++; }
        if (n > 0)
        {
            eEventType *NewEventList = new eEventType[n+1];
            for (int i = 0; i <= n; i++) { NewEventList[i] = EventList[i]; }
            return NewEventList;
        }
        else
        {
            return NULL;
        }
    }
    return NULL;
}

void CEventCollector::Register(EVENTCALLBACK *pfnCallback, void *pThis, eEventType *EventList)
{
    Unregister(pfnCallback, pThis);
    
    TEventCallbackInfo eci;    
    eci.EventList = CopyEventList(EventList);
    eci.pThis = pThis;
    eci.pfnEventCallback = pfnCallback;
    eci.pEventObject = NULL;    
    m_EventObjects.push_back(eci);
}

void CEventCollector::Unregister(EVENTCALLBACK *pfnCallback, void *pThis)
{
    vector<TEventCallbackInfo> NewList;
    for (int i = 0; i < m_EventObjects.size(); i++)
    {
        if ((m_EventObjects[i].pThis == pThis) && (m_EventObjects[i].pfnEventCallback == pfnCallback))
        {
            if (m_EventObjects[i].EventList != NULL)
            {
                delete m_EventObjects[i].EventList;
            }
        } else {
            NewList.push_back(m_EventObjects[i]);
        }
    }
    m_EventObjects = NewList;
}

void CEventCollector::Register(CEventObject *pObject, eEventType *EventList)
{
    Unregister(pObject);
    
    TEventCallbackInfo eci;    
    eci.EventList = CopyEventList(EventList);
    eci.pThis = NULL;
    eci.pfnEventCallback = NULL;
    eci.pEventObject = pObject;
    m_EventObjects.push_back(eci);
}

void CEventCollector::Unregister(CEventObject *pObject)
{
	vector<TEventCallbackInfo> NewList;
    for (int i = 0; i < m_EventObjects.size(); i++)
    {
        if (m_EventObjects[i].pEventObject == pObject)
        {
            if (m_EventObjects[i].EventList != NULL)
            {
                delete m_EventObjects[i].EventList;
            }
        } else {
            NewList.push_back(m_EventObjects[i]);
        }
    }
    m_EventObjects = NewList;

	//Remove from last event list
	EnterCriticalSection(&m_LastEventCriticalSection);
	vector<TEventInfo> NewList2;
	for (i = 0; i < m_LastEvents.size(); i++)
	{
		if (m_LastEvents[i].pEventObject == pObject)
		{
			//remove
		}
		else
		{
			NewList2.push_back(m_LastEvents[i]);
		}
	}
	m_LastEvents = NewList2;
	LeaveCriticalSection(&m_LastEventCriticalSection);
}

void CEventCollector::RaiseEvent(CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp)
{    
	ScheduleEvent(pEventObject, Event, OldValue, NewValue, ComingUp);
}

void CEventCollector::RaiseScheduledEvent(CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp)
{
	for (int i = 0; i < m_EventObjects.size(); i++)
    {
        BOOL bCall = TRUE;

        eEventType *pList = m_EventObjects[i].EventList;

        if (pList!=NULL)
        {
            int j = 0;            
            while (*pList != 0)
            {        
                if (Event == *pList)
                {
                    if (m_EventObjects[i].pEventObject != NULL)
                    {
                        m_EventObjects[i].pEventObject->OnEvent(pEventObject, Event, OldValue, NewValue, ComingUp);
                    } 
                    else if (m_EventObjects[i].pfnEventCallback != NULL) 
                    {
                        m_EventObjects[i].pfnEventCallback(m_EventObjects[i].pThis, pEventObject, Event, OldValue, NewValue, ComingUp);
                    }
                    break;
                }
                pList++;
            }
        }        
    }
	
	EnterCriticalSection(&m_LastEventCriticalSection);
	for (i = 0; i < m_LastEvents.size(); i++)
	{
		if ((m_LastEvents[i].Event == Event) && (m_LastEvents[i].pEventObject == pEventObject))
		{
			m_LastEvents[i].OldValue = OldValue;
			m_LastEvents[i].NewValue = NewValue;
			break;
		}
	}
	
	if (i >= m_LastEvents.size()) //new
	{
		TEventInfo EventInfo;
		EventInfo.pEventObject = pEventObject;
		EventInfo.Event = Event;
		EventInfo.OldValue = OldValue;
		EventInfo.NewValue = NewValue;
		m_LastEvents.push_back(EventInfo);		
	}		
	LeaveCriticalSection(&m_LastEventCriticalSection);
}


int CEventCollector::LastEventValues(eEventType Event, CEventObject **pEventObject, long *OldValue, long *NewValue)
{
	EnterCriticalSection(&m_LastEventCriticalSection);
	for (int i = 0; i < m_LastEvents.size(); i++)
	{
		if (m_LastEvents[i].Event == Event)
		{			
			if (pEventObject!=NULL) 
			{ 
				*pEventObject = m_LastEvents[i].pEventObject; 
			}
			if (OldValue!=NULL) 
			{ 
				*OldValue = m_LastEvents[i].OldValue; 
			}
			if (NewValue!=NULL) 
			{ 
				*NewValue = m_LastEvents[i].NewValue; 
			}
			LeaveCriticalSection(&m_LastEventCriticalSection);
			return 1;
		}
	}
	LeaveCriticalSection(&m_LastEventCriticalSection);
	return 0;
}

int CEventCollector::LastEventValues(CEventObject *pEventObject, eEventType Event, long *OldValue, long *NewValue)
{
	EnterCriticalSection(&m_LastEventCriticalSection);
	for (int i = 0; i < m_LastEvents.size(); i++)
	{
		if ((m_LastEvents[i].Event == Event) && (m_LastEvents[i].pEventObject == pEventObject))
		{			
			if (OldValue!=NULL) 
			{ 
				*OldValue = m_LastEvents[i].OldValue; 
			}
			if (NewValue!=NULL) 
			{ 
				*NewValue = m_LastEvents[i].NewValue; 
			}
			LeaveCriticalSection(&m_LastEventCriticalSection);
			return 1;
		}
	}
	LeaveCriticalSection(&m_LastEventCriticalSection);
	return 0;
}

int CEventCollector::NumEventsWaiting()
{
	int Num;
	EnterCriticalSection(&m_EventCriticalSection);
	Num = m_ScheduledEventList.size();	
	LeaveCriticalSection(&m_EventCriticalSection);
	return Num;
}


void CEventCollector::ScheduleEvent(CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp)
{
	if (Event == EVENT_NONE)
	{
		return;
	}
	EnterCriticalSection(&m_EventCriticalSection);

	if (m_ScheduledEventList.size()>0)
	{
		deque<TEventInfo> NewEventList;
		for (int i = 0; i < m_ScheduledEventList.size(); i++)
		{
			if ((m_ScheduledEventList[i].pEventObject == pEventObject)
				&& (m_ScheduledEventList[i].Event == Event))
			{
				//Old duplicate event. remove
			}
			else
			{
				NewEventList.push_back(m_ScheduledEventList[i]);
			}
		}
		m_ScheduledEventList = NewEventList;
	}

	TEventInfo ei;
	ei.pEventObject = pEventObject;
	ei.Event = Event;
	ei.OldValue = OldValue;
	ei.NewValue = NewValue;
	ei.ComingUp = CopyEventList(ComingUp);
	m_ScheduledEventList.push_back(ei);	
	//Use function outside of this class, because sometimes timers 
	//  mysteriously get killed.
	
	/*if (m_ScheduleTimerID == 0)
	{
		m_ScheduleTimerID = SetTimer(NULL,NULL,1,StaticEventTimerWrap);
		///\todo part of message loop instead of timer
	}*/	
	LeaveCriticalSection(&m_EventCriticalSection);
	//StartThread();
}


void CEventCollector::EventTimer()
{	
	/*EnterCriticalSection(&m_EventCriticalSection);
	::KillTimer(NULL, m_ScheduleTimerID);
	m_ScheduleTimerID = 0;
	LeaveCriticalSection(&m_EventCriticalSection);
	*/
	TEventInfo ei;

	ei.Event = EVENT_NONE;
	do
	{			
		EnterCriticalSection(&m_EventCriticalSection);
		if (m_ScheduledEventList.size()>0)
		{
			ei = m_ScheduledEventList.front();
			m_ScheduledEventList.pop_front();
			if (m_ScheduledEventList.size() == 0)
			{				
				//m_ScheduleTimerID = 0;
			}
		}
		LeaveCriticalSection(&m_EventCriticalSection);

	
		if (ei.Event != EVENT_NONE)
		{
			LOG(2,"Event: %d (%d,%d)",ei.Event,ei.OldValue,ei.NewValue);
			RaiseScheduledEvent(ei.pEventObject, ei.Event, ei.OldValue, ei.NewValue, ei.ComingUp);	
			if (ei.ComingUp != NULL) { delete[] ei.ComingUp; }
			ei.Event = EVENT_NONE;
		}
	//	Sleep(0);
	//} while (!m_bStopThread);
	} while (ei.Event != EVENT_NONE);
}

VOID CALLBACK CEventCollector::StaticEventTimerWrap(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{	
	EventCollector->EventTimer();
}



DWORD WINAPI EventCollectorThreadProc(LPVOID lpThreadParameter)
{
    if (lpThreadParameter != NULL)
    {        
        __try 
        {
            ((CEventCollector*)lpThreadParameter)->EventTimer();
        }        
        __except (CrashHandler((EXCEPTION_POINTERS*)_exception_info())) 
        {             
            LOG(1, "Crash in CEventCollector event loop");
            ExitThread(1);
            return 1;
        }
        ExitThread(0);
        return 0;
    }
    ExitThread(1);
    return 1;
}

void CEventCollector::StartThread()
{
    DWORD LinkThreadID;

    if (m_EventCollectorThread != NULL)
    {
        // Already started
        return;
    }
    
    m_bStopThread = FALSE;

    m_EventCollectorThread = CreateThread((LPSECURITY_ATTRIBUTES) NULL,  // No security.
                             (DWORD) 0,                     // Same stack size.
                             EventCollectorThreadProc,                  // Thread procedure.
                             (LPVOID)this,                          // Parameter.
                             (DWORD) 0,                     // Start immediatly.
                             (LPDWORD) & LinkThreadID);     // Thread ID.    
}

void CEventCollector::StopThread()
{
    DWORD ExitCode;
    int i;
    BOOL Thread_Stopped = FALSE;

    if (m_EventCollectorThread != NULL)
    {
        i = 10;
        m_bStopThread = TRUE;
        while(i-- > 0 && !Thread_Stopped)
        {
            if (GetExitCodeThread(m_EventCollectorThread, &ExitCode) == TRUE)
            {
                if (ExitCode != STILL_ACTIVE)
                {
                    Thread_Stopped = TRUE;
                }
                else
                {
                    Sleep(50);
                }
            }
            else
            {
                Thread_Stopped = TRUE;
            }
        }

        if (Thread_Stopped == FALSE)
        {
            TerminateThread(m_EventCollectorThread, 0);
            Sleep(50);
        }
        CloseHandle(m_EventCollectorThread);
        m_EventCollectorThread = NULL;
    }
}
