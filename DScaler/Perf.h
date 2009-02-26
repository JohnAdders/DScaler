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
 * @file perf.h perf Header file
 */
 
#ifndef __PERF_H___
#define __PERF_H___

// uncomment the folowing line if you want to include performance statistics
//#define USE_PERFORMANCE_STATS

// define USE_PERFORMANCE_STATS when doing a debug build
#ifndef USE_PERFORMANCE_STATS
    #ifdef _DEBUG
        #define USE_PERFORMANCE_STATS
    #endif
#endif

enum ePerfType
{
    PERF_WAIT_FIELD = 0,
#ifdef USE_PERFORMANCE_STATS
    PERF_INPUT_FILTERS,
    PERF_OUTPUT_FILTERS,
    PERF_PULLDOWN_DETECT,
    PERF_DEINTERLACE,
    PERF_RATIO,
    PERF_CALIBRATION,
    PERF_TIMESHIFT,
    PERF_VBI,
    PERF_LOCK_OVERLAY,
    PERF_UNLOCK_OVERLAY,
    PERF_FLIP_OVERLAY,
#endif
    PERF_TYPE_LASTONE,
};

class CPerfItem
{
public:
    void Resume();
    void Suspend();
    CPerfItem(const char* Name);
    ~CPerfItem();
    void Reset();
    void StartCount();
    void StopCount();
    const char* GetName();
    DWORD GetLastDuration();
    unsigned int GetNbCounts();
    DWORD GetAverageDuration();
    DWORD GetMinDuration();
    DWORD GetMaxDuration();

protected:
    const char*     m_Name;
    DWORD           m_LastDuration;
    DWORD           m_SumDuration;
    unsigned int    m_NbCounts;
    DWORD           m_MinDuration;
    DWORD           m_MaxDuration;

private:
    DWORD           m_SuspendCounter;
    DWORD           m_TickStart;
    BOOL            m_IsCounting;
};

class CPerf
{
public:
    void Resume();
    void Suspend();
    CPerf();
    ~CPerf();
    void Reset();
    void InitCycle();
    void StartCount(ePerfType PerfType);
    void StopCount(ePerfType PerfType);
    int GetDurationLastCycle(ePerfType PerfType);
    unsigned int GetNbCycles(int NbFramesPerSec);
    BOOL IsValid(ePerfType PerfType);
    const char* GetName(ePerfType PerfType);
    DWORD GetLastDuration(ePerfType PerfType);
    unsigned int GetNbCounts(ePerfType PerfType);
    DWORD GetAverageDuration(ePerfType PerfType);
    DWORD GetMinDuration(ePerfType PerfType);
    DWORD GetMaxDuration(ePerfType PerfType);
    int GetNumberDroppedFields();
    int GetNumberNotWaitedFields();
    int GetNumberLateFields();
    int GetNumberUsedFields();
    int GetNumberNoFlipAtTime();
    double GetAverageDroppedFields();
    double GetAverageNotWaitedFields();
    double GetAverageLateFields();
    double GetAverageUsedFields();
    double GetAverageNoFlipAtTime();
    int GetDroppedFieldsLastSecond();
    int GetNotWaitedFieldsLastSecond();
    int GetLateFieldsLastSecond();
    int GetUsedFieldsLastSecond();
    int GetNoFlipAtTimeLastSecond();

private:
    CPerfItem*  m_PerfItems[PERF_TYPE_LASTONE];
    BOOL        m_PerfCalculated[PERF_TYPE_LASTONE];
    DWORD       m_TickStart;
    DWORD       m_TickStartLastSec;
    int         m_TotalDroppedFields;
    int         m_TotalNotWaitedFields;
    int         m_TotalLateFields;
    int         m_TotalUsedFields;
    int         m_TotalNoFlipAtTime;
    int            m_DroppedFieldsLastSec;
    int            m_NotWaitedFieldsLastSec;
    int            m_LateFieldsLastSec;
    int            m_UsedFieldsLastSec;
    int            m_NoFlipAtTimeLastSec;
    BOOL        m_ResetRequested;
};

extern CPerf* pPerf;

#endif