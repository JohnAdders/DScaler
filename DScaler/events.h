/////////////////////////////////////////////////////////////////////////////
// $Id: events.h,v 1.1 2002-09-25 22:33:06 kooiman Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Jeroen Kooiman.  All rights reserved.
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

#ifndef __EVENTS_H___
#define __EVENTS_H___

#include <vector>

enum eEventType
{
    EVENT_NONE = 0,
    EVENT_INIT,
    EVENT_DESTROY,
    EVENT_SOURCE_PRECHANGE,
    EVENT_SOURCE_CHANGE,
    EVENT_VIDEOINPUT_PRECHANGE,
    EVENT_VIDEOINPUT_CHANGE,
    EVENT_AUDIOINPUT_PRECHANGE,
    EVENT_AUDIOINPUT_CHANGE,
    EVENT_VIDEOFORMAT_PRECHANGE,
    EVENT_VIDEOFORMAT_CHANGE,
    EVENT_CHANNEL_PRECHANGE,
    EVENT_CHANNEL_CHANGE
};
#define EVENT_ENDOFLIST EVENT_NONE
typedef void (__cdecl EVENTCALLBACK)(void *pThis, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp);

//class CSource;


class CEventObject
{
public:
    virtual void OnEvent(eEventType Event, long OldValue, long NewValue, eEventType *ComingUp) {;}
};

class CEventCollector
{
protected:
    typedef struct 
    {
        eEventType *EventList;
        void *pThis;
        EVENTCALLBACK *pfnEventCallback;
        CEventObject *pEventObject;
    } TEventCallbackInfo;
    
    vector<TEventCallbackInfo> m_EventObjects;

protected:
    eEventType *CEventCollector::CopyEventList(eEventType *EventList);
    
public:
    CEventCollector();
    ~CEventCollector();

    void Register(EVENTCALLBACK *pfnCallback, void *pThis, eEventType *EventList);
    void Unregister(EVENTCALLBACK *pfnCallback, void *pThis);

    void Register(CEventObject *pObject, eEventType *EventList);
    void Unregister(CEventObject *pObject);

    void RaiseEvent(eEventType Event, long OldValue, long NewValue, eEventType *ComingUp = NULL);    
};

//Defined, allocated & destroyed in Dscaler.cpp
extern CEventCollector *EventCollector;

#endif
