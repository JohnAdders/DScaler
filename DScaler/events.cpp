/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file Events.cpp CEventCollector implementation
 */

#include "stdafx.h"
#include "Events.h"
#include "DebugLog.h"
#include "Crash.h"
#include "DScaler.h"
#include "SettingsMaster.h"
#include "Providers.h"

using namespace std;

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
    if(EventCollector.IsValid())
    {
        EventCollector->Unregister(this);
    }
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
                delete [] m_EventObjects[i].EventList;
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
    int i;
    for (i = 0; i < m_EventObjects.size(); i++)
    {
        if (m_EventObjects[i].pEventObject == pObject)
        {
            if (m_EventObjects[i].EventList != NULL)
            {
                delete [] m_EventObjects[i].EventList;
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
    int i;
    for (i = 0; i < m_EventObjects.size(); i++)
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

    // this is where we keep track of what the current status
    // so lets use here as a good place to maitain the settings holder
    // with the current set up
    // This is used instead of listening for the event in the settings master
    // as we need to be able to load and save settings synchronously
    switch (Event)
    {
    case EVENT_SOURCE_CHANGE:
        SettingsMaster->SetSource((CSource*)NewValue);
        SettingsMaster->SetChannelName(-1);
        SettingsMaster->SetVideoInput(-1);
        SettingsMaster->SetAudioInput(-1);
        SettingsMaster->SetVideoFormat(-1);
        break;

    case EVENT_CHANNEL_CHANGE:
        SettingsMaster->SetChannelName(NewValue);
        break;

    case EVENT_VIDEOINPUT_CHANGE:
        SettingsMaster->SetVideoInput(NewValue);
        SettingsMaster->SetChannelName(-1);
        break;

    case EVENT_AUDIOINPUT_CHANGE:
        SettingsMaster->SetAudioInput(NewValue);
        break;

    case EVENT_VIDEOFORMAT_CHANGE:
        SettingsMaster->SetVideoFormat(NewValue);
        break;
    }

    LeaveCriticalSection(&m_EventCriticalSection);

    // we want to signal the event and then run away
    // so use Post rather than Send.  Using send also causes
    // threading problems
    PostMessageToMainWindow(UWM_EVENTADDEDTOQUEUE, 0, 0);
}


void CEventCollector::ProcessEvents()
{
    // JA 2/12/2002
    // attempt to get settings working properly
    // using messages rather than a thread
    // this should force all changes to come
    // from the main thread
    while(TRUE)
    {
        TEventInfo ei;
        ei.Event = EVENT_NONE;

        EnterCriticalSection(&m_EventCriticalSection);
        if (m_ScheduledEventList.size()>0)
        {
            ei = m_ScheduledEventList.front();
            m_ScheduledEventList.pop_front();
        }
        LeaveCriticalSection(&m_EventCriticalSection);

        if (ei.Event != EVENT_NONE)
        {
            LOG(2,_T("Event: %d (%d,%d)"),ei.Event,ei.OldValue,ei.NewValue);
            RaiseScheduledEvent(ei.pEventObject, ei.Event, ei.OldValue, ei.NewValue, ei.ComingUp);
            if (ei.ComingUp != NULL)
            {
                delete [] ei.ComingUp;
            }
        }
        else
        {
            return;
        }
    }
}

