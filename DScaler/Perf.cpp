/////////////////////////////////////////////////////////////////////////////
// $Id: Perf.cpp,v 1.7 2002-05-18 13:11:40 tobbej Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Laurent Garnier.  All rights reserved.
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
// Revision 1.6  2002/05/06 13:13:59  laurentg
// Initialization of CPerf members
//
// Revision 1.5  2002/05/02 16:43:32  laurentg
// ceil replaced by floor
//
// Revision 1.4  2002/01/31 13:02:46  robmuller
// Improved accuracy and reliability of the performance statistics.
//
// Revision 1.3  2001/12/16 18:40:28  laurentg
// Reset statistics
//
// Revision 1.2  2001/12/16 16:31:43  adcockj
// Bug fixes
//
// Revision 1.1  2001/12/16 13:00:51  laurentg
// New statistics
//
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Perf.h"
#include "FieldTiming.h"

ULONG GetAccurateTickCount()
{
    ULONGLONG ticks;
    ULONGLONG frequency;

    QueryPerformanceFrequency((PLARGE_INTEGER)&frequency);
    QueryPerformanceCounter((PLARGE_INTEGER)&ticks);
    ticks = (ticks & 0xFFFFFFFF00000000) / frequency * 10000000 +
            (ticks & 0xFFFFFFFF) * 10000000 / frequency;
    return ((ULONG)(ticks / 10000));
}

static const char* PerfNames[PERF_TYPE_LASTONE] = 
{
    "Wait for next field",
    "Input filters",
    "Output filters",
    "Pulldown detection",
    "Deinterlacing",
    "Aspect ratio detection",
    "Calibration",
    "Timeshifting",
    "VBI data decoding",
    "Overlay locking",
    "Overlay unlocking",
    "Overlay flipping",
};

CPerf* pPerf = NULL;

CPerfItem::CPerfItem(const char* Name)
{
    m_Name = Name;
    m_LastDuration = 0;
    m_SumDuration = 0;
    m_NbCounts = 0;
    m_MinDuration = 10000;
    m_MaxDuration = 0;
    m_TickStart = 0;
    m_IsCounting = FALSE;
    m_SuspendCounter = 0;
}

CPerfItem::~CPerfItem()
{
}

void CPerfItem::Reset()
{
    m_LastDuration = 0;
    m_SumDuration = 0;
    m_NbCounts = 0;
    m_MinDuration = 10000;
    m_MaxDuration = 0;
    m_TickStart = 0;
    m_IsCounting = FALSE;
    m_SuspendCounter = 0;
}

void CPerfItem::StartCount()
{
    if(m_SuspendCounter <= 0)
    {
        m_TickStart = GetAccurateTickCount();
        m_IsCounting = TRUE;
    }
}

void CPerfItem::StopCount()
{
    DWORD Duration;

    if (!m_IsCounting || m_SuspendCounter > 0)
    {
        return;
    }

    Duration = GetAccurateTickCount() - m_TickStart;
    m_LastDuration = Duration;
    m_SumDuration += Duration;
    ++m_NbCounts;
    if (Duration < m_MinDuration)
    {
        m_MinDuration = Duration;
    }
    if (Duration > m_MaxDuration)
    {
        m_MaxDuration = Duration;
    }
    m_TickStart = 0;
    m_IsCounting = FALSE;
}

// To not contaminate the statistics call this before doing any lenghty concurrent operation.
// If the counter is running it will be aborted and will not be included in the overall statistics.
// You must call Resume() afterwards. Suspend() and Resume() can be nested.
void CPerfItem::Suspend()
{
    m_SuspendCounter++;
}

void CPerfItem::Resume()
{
    m_SuspendCounter--;
}

const char* CPerfItem::GetName()
{
    return m_Name;
}

DWORD CPerfItem::GetLastDuration()
{
    return m_LastDuration;
}

unsigned int CPerfItem::GetNbCounts()
{
    return m_NbCounts;
}

DWORD CPerfItem::GetAverageDuration()
{
    if (m_NbCounts > 0)
        return m_SumDuration * 10 / m_NbCounts;
    else
        return 0;
}

DWORD CPerfItem::GetMinDuration()
{
    return m_MinDuration;
}

DWORD CPerfItem::GetMaxDuration()
{
    return m_MaxDuration;
}


CPerf::CPerf()
{
    for (int i(0) ; i < PERF_TYPE_LASTONE ; ++i)
    {
        m_PerfItems[i] = new CPerfItem(PerfNames[i]);
        m_PerfCalculated[i] = FALSE;
    }
    m_TickStart = 0;
    m_TickStartLastSec = m_TickStart;
    m_TotalDroppedFields = 0;
    m_TotalUsedFields = 0;
    m_DroppedFieldsLastSec = 0.0;
    m_UsedFieldsLastSec = 0.0;
    m_ResetRequested = TRUE;
}

CPerf::~CPerf()
{
    for (int i(0) ; i < PERF_TYPE_LASTONE ; ++i)
    {
        delete m_PerfItems[i];
    }
}

void CPerf::Reset()
{
    m_ResetRequested = TRUE;
}

void CPerf::InitCycle()
{
    DWORD CurrentTickCount = GetAccurateTickCount();

    if (m_ResetRequested)
    {
        m_TickStart = CurrentTickCount;
        m_TickStartLastSec = m_TickStart;
        m_TotalDroppedFields = 0;
        m_TotalUsedFields = 0;
        m_DroppedFieldsLastSec = 0.0;
        m_UsedFieldsLastSec = 0.0;
        Timing_ResetDroppedFields();
        Timing_ResetUsedFields();
        for (int i(0) ; i < PERF_TYPE_LASTONE ; ++i)
        {
            m_PerfItems[i]->Reset();
        }
        m_ResetRequested = FALSE;
    }
    else if ((m_TickStartLastSec + 1000) <= CurrentTickCount)
    {
        m_TotalDroppedFields += Timing_GetDroppedFields();
        m_TotalUsedFields += Timing_GetUsedFields();
        
        double tickDiff=(double)(CurrentTickCount - m_TickStartLastSec);
        
        //prevent crashing if tickDiff is invalid
        if(_finite(tickDiff) && !_isnan(tickDiff))
        {
            m_DroppedFieldsLastSec = (double)Timing_GetDroppedFields() * 1000.0 / tickDiff;
            m_UsedFieldsLastSec = (double)Timing_GetUsedFields() * 1000.0 / tickDiff;
        }
        else
        {
            m_DroppedFieldsLastSec=-1;
            m_UsedFieldsLastSec=-1;
        }
        
        Timing_ResetDroppedFields();
        Timing_ResetUsedFields();
        m_TickStartLastSec = CurrentTickCount;
    }

    for (int i(0) ; i < PERF_TYPE_LASTONE ; ++i)
    {
        m_PerfCalculated[i] = FALSE;
    }
}

void CPerf::StartCount(ePerfType PerfType)
{
    m_PerfItems[PerfType]->StartCount();
}

void CPerf::StopCount(ePerfType PerfType)
{
    m_PerfItems[PerfType]->StopCount();
    m_PerfCalculated[PerfType] = TRUE;
}


void CPerf::Suspend()
{
    for (int i(0) ; i < PERF_TYPE_LASTONE ; ++i)
    {
        m_PerfItems[i]->Suspend();
    }
}

void CPerf::Resume()
{
    for (int i(0) ; i < PERF_TYPE_LASTONE ; ++i)
    {
        m_PerfItems[i]->Resume();
    }
}

int CPerf::GetDurationLastCycle(ePerfType PerfType)
{
    if (m_PerfCalculated[PerfType])
    {
        return GetLastDuration(PerfType);
    }
    else
    {
        return -1;
    }
}

unsigned int CPerf::GetNbCycles(int NbFramesPerSec)
{
    DWORD TotalDuration = GetAccurateTickCount() - m_TickStart;

    return TotalDuration * NbFramesPerSec / 1000;
}

BOOL CPerf::IsValid(ePerfType PerfType)
{
    return (GetNbCounts(PerfType) > 0) ? TRUE : FALSE;
}

const char* CPerf::GetName(ePerfType PerfType)
{
    return m_PerfItems[PerfType]->GetName();
}

DWORD CPerf::GetLastDuration(ePerfType PerfType)
{
    return m_PerfItems[PerfType]->GetLastDuration();
}

unsigned int CPerf::GetNbCounts(ePerfType PerfType)
{
    return m_PerfItems[PerfType]->GetNbCounts();
}

DWORD CPerf::GetAverageDuration(ePerfType PerfType)
{
    return m_PerfItems[PerfType]->GetAverageDuration();
}

DWORD CPerf::GetMinDuration(ePerfType PerfType)
{
    return m_PerfItems[PerfType]->GetMinDuration();
}

DWORD CPerf::GetMaxDuration(ePerfType PerfType)
{
    return m_PerfItems[PerfType]->GetMaxDuration();
}

int CPerf::GetNumberDroppedFields()
{
    return m_TotalDroppedFields;
}

int CPerf::GetNumberUsedFields()
{
    return m_TotalUsedFields;
}

double CPerf::GetAverageDroppedFields()
{
    if (m_TickStartLastSec == m_TickStart)
    {
        return 0.0;
    }
    else
    {
        return ((double)m_TotalDroppedFields * 1000.0 / (double)(m_TickStartLastSec - m_TickStart));
    }
}

double CPerf::GetAverageUsedFields()
{
    if (m_TickStartLastSec == m_TickStart)
    {
        return 0.0;
    }
    else
    {
        return ((double)m_TotalUsedFields * 1000.0 / (double)(m_TickStartLastSec - m_TickStart));
    }
}

int CPerf::GetDroppedFieldsLastSecond()
{
    return (int)floor(m_DroppedFieldsLastSec + 0.5);
}

int CPerf::GetUsedFieldsLastSecond()
{
    return (int)floor(m_UsedFieldsLastSec + 0.5);
}

