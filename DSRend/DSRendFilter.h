/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Torbjörn Jansson.  All rights reserved.
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
 * @file DSRendFilter.h Declaration of the CDSRendFilter
 */

#ifndef __DSRENDFILTER_H_
#define __DSRENDFILTER_H_

#include "resource.h"       // main symbols
#include "DSRendInPin.h"
#include "AutoLockCriticalSection.h"
#include "FieldBufferHandler.h"
#include "PersistStream.h"

/**
 * The filter itself.
 * @todo add quality-control management
 * @todo add more statistics on IQualProp, not all data is updated
 * @todo maybe create a new mediatype enumerator object
 */
class ATL_NO_VTABLE CDSRendFilter : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CDSRendFilter, &CLSID_DSRendFilter>,
    public ISpecifyPropertyPagesImpl<CDSRendFilter>,
    public IDSRendFilter,
    //public IPersistImpl<CDSRendFilter>,
    public IBaseFilter,
    public IQualProp,
    public IMediaSeeking,
    public CPersistStream
{
public:
    CDSRendFilter();
    ~CDSRendFilter();
    HRESULT FinalConstruct();
    HRESULT FinalRelease();

DECLARE_REGISTRY_RESOURCEID(IDR_DSRENDFILTER)
DECLARE_NOT_AGGREGATABLE(CDSRendFilter)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDSRendFilter)
    COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
    COM_INTERFACE_ENTRY(IDSRendFilter)
    COM_INTERFACE_ENTRY(IMediaFilter)
    COM_INTERFACE_ENTRY(IBaseFilter)
    COM_INTERFACE_ENTRY_IID(IID_IQualProp,IQualProp)
    COM_INTERFACE_ENTRY(IMediaSeeking)
    
    //COM_INTERFACE_ENTRY2 is needed because of multiple inheritance
    //IPersist exists in both IPersistStream and IBaseFilter
    COM_INTERFACE_ENTRY2(IPersist,IBaseFilter)
    COM_INTERFACE_ENTRY(IPersistStream)
END_COM_MAP()

BEGIN_PROP_MAP(CDSRendFilter)
    PROP_PAGE(CLSID_DSRendQualityPage)
    PROP_PAGE(CLSID_DSRendAboutPage)
END_PROP_MAP()

// IDSRendFilter
    STDMETHOD(SetFieldHistory)(long cFields);
    STDMETHOD(GetFields)(FieldBuffer *ppFields,long *count,BufferInfo *pBufferInfo,DWORD dwTimeout,DWORD dwLastRenderTime);
    STDMETHOD(FreeFields)();

// IPersist
    STDMETHOD(GetClassID(CLSID *pClassID));

// IMediaFilter
    STDMETHOD(Stop());
    STDMETHOD(Pause());
    STDMETHOD(Run(REFERENCE_TIME tStart));
    STDMETHOD(GetState(DWORD dwMilliSecsTimeout,FILTER_STATE *State));
    STDMETHOD(SetSyncSource(IReferenceClock *pClock));
    STDMETHOD(GetSyncSource(IReferenceClock **pClock));

// IBaseFilter
    STDMETHOD(EnumPins(IEnumPins **ppEnum));
    STDMETHOD(FindPin(LPCWSTR Id,IPin **ppPin));
    STDMETHOD(QueryFilterInfo(FILTER_INFO *pInfo));
    STDMETHOD(JoinFilterGraph(IFilterGraph *pGraph,LPCWSTR pName));
    STDMETHOD(QueryVendorInfo(LPWSTR *pVendorInfo));
    
// IQualProp
    STDMETHOD(get_AvgFrameRate(int *piAvgFrameRate));
    STDMETHOD(get_AvgSyncOffset(int *piAvg));
    STDMETHOD(get_DevSyncOffset(int *piDev));
    STDMETHOD(get_FramesDrawn(int *pcFramesDrawn));
    STDMETHOD(get_FramesDroppedInRenderer(int *pcFrames));
    STDMETHOD(get_Jitter(int *piJitter));

    // this will allow seeking to work properly
// IMediaSeeking
    STDMETHOD(GetCapabilities(DWORD *pCapabilities));
    STDMETHOD(CheckCapabilities( DWORD *pCapabilities));
    STDMETHOD(IsFormatSupported(const GUID *pFormat));
    STDMETHOD(QueryPreferredFormat(GUID *pFormat));
    STDMETHOD(GetTimeFormat(GUID *pFormat));
    STDMETHOD(IsUsingTimeFormat(const GUID *pFormat));
    STDMETHOD(SetTimeFormat(const GUID *pFormat));
    STDMETHOD(GetDuration(LONGLONG *pDuration));
    STDMETHOD(GetStopPosition(LONGLONG *pStop));
    STDMETHOD(GetCurrentPosition(LONGLONG *pCurrent));
    STDMETHOD(ConvertTimeFormat(LONGLONG *pTarget,const GUID *pTargetFormat,LONGLONG Source,const GUID *pSourceFormat));
    STDMETHOD(SetPositions(LONGLONG *pCurrent,DWORD dwCurrentFlags,LONGLONG *pStop,DWORD dwStopFlags));
    STDMETHOD(GetPositions(LONGLONG *pCurrent,LONGLONG *pStop));
    STDMETHOD(GetAvailable(LONGLONG *pEarliest,LONGLONG *pLatest));
    STDMETHOD(SetRate(double dRate));
    STDMETHOD(GetRate(double *pdRate));
    STDMETHOD(GetPreroll(LONGLONG *pllPreroll));

// CPersistStream
    long GetSize();
    HRESULT SaveToStream(IStream *pStream);
    HRESULT LoadFromStream(IStream *pStream);
    //DWORD GetVersion();
    
public:
    /**
     *
     * @param pSample sample to render
     */
    HRESULT renderSample(IMediaSample *pSample);
    HRESULT beginFlush();

    /**
     * Checks if the filter is stopped.
     * @return true if the filter is stopped
     */
    bool isStopped();
    
    HRESULT sendNotifyMsg(long eventCode,LONG_PTR param1,LONG_PTR param2);

    /**
     * Waits for correct time.
     * This function sets up an advice request with the reference clock and waits for the event to 
     * become signaled.
     */
    HRESULT waitForTime(REFERENCE_TIME rtStreamTime);
    
    /// unblock a call to waitForTime.
    void stopWait();

    ///rendering lock
    CComAutoCriticalSection m_renderLock;

    CFieldBufferHandler m_FieldBuffer;

private:
    /**
     * Helper function for IMediaSeeking interface.
     * This function gets the upstream IMediaSeeking interface,
     * if the filter is not connected it fails.
     * This is used to forward all IMediaSeeking calls to the upstream filter.
     */
    HRESULT getMediaSeeking(CComPtr<IMediaSeeking> &pSeeking);

    //IQualProp
    double m_AvgFieldRate;
    REFERENCE_TIME m_rtLastTime;
    int m_iLastDrawnFields;
    LONGLONG m_iSumLateness;
    int m_iDev;
    int m_iJitter;
    
    //IBaseFilter
    IFilterGraph *m_pGraph;
    IMediaEventSink *m_pEventSink;
    LPWSTR m_pFilterName;
    
    //IMediaFilter
    ///filter state
    FILTER_STATE m_filterState;
    ///current reference clock
    CComPtr<IReferenceClock> m_pRefClk;
    
    ///Event for waiting on a specific time, used in waitForTime
    CEvent m_refClockEvent;
    DWORD_PTR m_refClockCookie;

    ///start time
    REFERENCE_TIME m_tStart;

    ///Filter lock
    CComAutoCriticalSection m_Lock;

    ///The input pin
    CCustomComObject<CDSRendInPin,CDSRendFilter*> m_InputPin;

};

#endif //__DSRENDFILTER_H_
