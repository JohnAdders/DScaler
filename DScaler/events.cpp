/////////////////////////////////////////////////////////////////////////////
// $Id: events.cpp,v 1.1 2002-09-25 22:33:06 kooiman Exp $
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
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Events.h"
#include "Source.h"
#include "Providers.h"
#include "ProgramList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CEventCollector::CEventCollector()
{
}

CEventCollector::~CEventCollector()
{
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
}

void CEventCollector::RaiseEvent(eEventType Event, long OldValue, long NewValue, eEventType *ComingUp)
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
                        m_EventObjects[i].pEventObject->OnEvent(Event, OldValue, NewValue, ComingUp);
                    } 
                    else if (m_EventObjects[i].pfnEventCallback != NULL) 
                    {
                        m_EventObjects[i].pfnEventCallback(m_EventObjects[i].pThis, Event, OldValue, NewValue, ComingUp);
                    }
                    break;
                }
                pList++;
            }
        }        
    }
}

