/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file Perf.cpp High performance time functions
 */

#include "stdafx.h"
#include <float.h>
#include "Perf.h"
#include "FieldTiming.h"

ULONG GetAccurateTickCount()
// an overflow happens after 21 days uptime on a 10GHz machine
{
    ULONGLONG ticks;
    ULONGLONG frequency;

    QueryPerformanceFrequency((PLARGE_INTEGER)&frequency);
    QueryPerformanceCounter((PLARGE_INTEGER)&ticks);

    ticks = ticks * 1000 / frequency;
    return (ULONG)ticks;
}

static const TCHAR* PerfNames[PERF_TYPE_LASTONE] =
{
    _T("Wait for next field"),
#ifdef USE_PERFORMANCE_STATS
    _T("Input filters"),
    _T("Output filters"),
    _T("Pulldown detection"),
    _T("Deinterlacing"),
    _T("Aspect ratio detection"),
    _T("Calibration"),
    _T("Timeshifting"),
    _T("VBI data decoding"),
    _T("Overlay locking"),
    _T("Overlay unlocking"),
    _T("Overlay flipping"),
#endif
};

SmartPtr<CPerf> pPerf;

CPerfItem::CPerfItem(const TCHAR* Name)
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

const TCHAR* CPerfItem::GetName()
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
    m_TotalNotWaitedFields = 0;
    m_TotalLateFields = 0;
    m_TotalUsedFields = 0;
    m_TotalNoFlipAtTime = 0;
    m_DroppedFieldsLastSec = 0;
    m_NotWaitedFieldsLastSec = 0;
    m_LateFieldsLastSec = 0;
    m_UsedFieldsLastSec = 0;
    m_NoFlipAtTimeLastSec = 0;
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
        m_TotalNotWaitedFields = 0;
        m_TotalLateFields = 0;
        m_TotalUsedFields = 0;
        m_TotalNoFlipAtTime = 0;
        m_DroppedFieldsLastSec = 0;
        m_NotWaitedFieldsLastSec = 0;
        m_LateFieldsLastSec = 0;
        m_UsedFieldsLastSec = 0;
        m_NoFlipAtTimeLastSec = 0;
        Timing_ResetDroppedFields();
        Timing_ResetNotWaitedFields();
        Timing_ResetLateFields();
        Timing_ResetUsedFields();
        Timing_ResetNoFlipAtTime();
        for (int i(0) ; i < PERF_TYPE_LASTONE ; ++i)
        {
            m_PerfItems[i]->Reset();
        }
        m_ResetRequested = FALSE;
    }
    else if ((m_TickStartLastSec + 1000) <= CurrentTickCount)
    {
        m_TotalDroppedFields += Timing_GetDroppedFields();
        m_TotalNotWaitedFields += Timing_GetNotWaitedFields();
        m_TotalLateFields += Timing_GetLateFields();
        m_TotalUsedFields += Timing_GetUsedFields();
        m_TotalNoFlipAtTime += Timing_GetNoFlipAtTime();

        DWORD tickDiff=(CurrentTickCount - m_TickStartLastSec);

        //prevent crashing if tickDiff is invalid.
        //this fix shoud not be nessesary any more, the problems was most
        //likely caused by a missing emms instruction in memcpyMMX and this
        //is now fixed
        if(_finite(tickDiff) && !_isnan(tickDiff))
        {
            m_DroppedFieldsLastSec = Timing_GetDroppedFields() * 10000 / tickDiff;
            m_NotWaitedFieldsLastSec = Timing_GetNotWaitedFields() * 10000 / tickDiff;
            m_LateFieldsLastSec = Timing_GetLateFields() * 10000 / tickDiff;
            m_UsedFieldsLastSec = Timing_GetUsedFields() * 10000 / tickDiff;
            m_NoFlipAtTimeLastSec = Timing_GetNoFlipAtTime() * 10000 / tickDiff;
        }
        else
        {
            m_DroppedFieldsLastSec=-1;
            m_NotWaitedFieldsLastSec=-1;
            m_LateFieldsLastSec=-1;
            m_UsedFieldsLastSec=-1;
            m_NoFlipAtTimeLastSec=-1;
        }

        Timing_ResetDroppedFields();
        Timing_ResetNotWaitedFields();
        Timing_ResetLateFields();
        Timing_ResetUsedFields();
        Timing_ResetNoFlipAtTime();
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

const TCHAR* CPerf::GetName(ePerfType PerfType)
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

int CPerf::GetNumberNotWaitedFields()
{
    return m_TotalNotWaitedFields;
}

int CPerf::GetNumberLateFields()
{
    return m_TotalLateFields;
}

int CPerf::GetNumberUsedFields()
{
    return m_TotalUsedFields;
}

int CPerf::GetNumberNoFlipAtTime()
{
    return m_TotalNoFlipAtTime;
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

double CPerf::GetAverageNotWaitedFields()
{
    if (m_TickStartLastSec == m_TickStart)
    {
        return 0.0;
    }
    else
    {
        return ((double)m_TotalNotWaitedFields * 1000.0 / (double)(m_TickStartLastSec - m_TickStart));
    }
}

double CPerf::GetAverageLateFields()
{
    if (m_TickStartLastSec == m_TickStart)
    {
        return 0.0;
    }
    else
    {
        return ((double)m_TotalLateFields * 1000.0 / (double)(m_TickStartLastSec - m_TickStart));
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

double CPerf::GetAverageNoFlipAtTime()
{
    if (m_TickStartLastSec == m_TickStart)
    {
        return 0.0;
    }
    else
    {
        return ((double)m_TotalNoFlipAtTime * 1000.0 / (double)(m_TickStartLastSec - m_TickStart));
    }
}

int CPerf::GetDroppedFieldsLastSecond()
{
    return (m_DroppedFieldsLastSec + 5) / 10;
}

int CPerf::GetNotWaitedFieldsLastSecond()
{
    return (m_NotWaitedFieldsLastSec + 5) / 10;
}

int CPerf::GetLateFieldsLastSecond()
{
    return (m_LateFieldsLastSec + 5) / 10;
}

int CPerf::GetUsedFieldsLastSecond()
{
    return (m_UsedFieldsLastSec + 5) / 10;
}

int CPerf::GetNoFlipAtTimeLastSecond()
{
    return (m_NoFlipAtTimeLastSec + 5) / 10;
}

