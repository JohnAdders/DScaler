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
 * @file DSRendInPin.h Declaration of the CDSRendInPin
 */

#ifndef __DSRENDINPIN_H_
#define __DSRENDINPIN_H_

#include "resource.h"       // main symbols
#include "CustomComObj.h"
#include "Event.h"

//forward declaration of the filter class
class CDSRendFilter;

/**
 * The input pin of the renderer filter.
 * @todo implement scheduling of samples. done, but might need some tweaking.
 * @todo add support for changing format when graph is running
 */
class ATL_NO_VTABLE CDSRendInPin : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CDSRendInPin, &CLSID_DSRendInPin>,
    public ISpecifyPropertyPagesImpl<CDSRendInPin>,
    public IPin,
    public IMemInputPin,
    public IAMFilterMiscFlags,
    public IDSRendSettings
{
public:
    CDSRendInPin(CDSRendFilter *pFilter);
    ~CDSRendInPin()
    {
    
    }
    HRESULT FinalConstruct();
    HRESULT FinalRelease();

DECLARE_REGISTRY_RESOURCEID(IDR_DSRENDINPIN)
DECLARE_NOT_AGGREGATABLE(CDSRendInPin)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDSRendInPin)
    COM_INTERFACE_ENTRY(IPin)
    COM_INTERFACE_ENTRY(IMemInputPin)
    COM_INTERFACE_ENTRY(IAMFilterMiscFlags)
    COM_INTERFACE_ENTRY(IDSRendSettings)
    COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
END_COM_MAP()

BEGIN_PROP_MAP(CDSRendFilter)
    PROP_PAGE(CLSID_SettingsPage)
END_PROP_MAP()

    //allow CCustomComObject to access m_pFilter
    friend class CCustomComObject<CDSRendInPin,CDSRendFilter*>;

// IDSRendSettings
    STDMETHOD(get_Status)(DSRendStatus *pVal);
    STDMETHOD(get_SwapFields)(BOOL *pVal);
    STDMETHOD(put_SwapFields)(BOOL pVal);
    STDMETHOD(get_ForceYUY2)(BOOL *pVal);
    STDMETHOD(put_ForceYUY2)(BOOL pVal);
    STDMETHOD(get_FieldFormat)(DSREND_FIELD_FORMAT *pVal);
    STDMETHOD(put_FieldFormat)(DSREND_FIELD_FORMAT pVal);
    STDMETHOD(get_VertMirror)(/*[out, retval]*/ BOOL *pVal);
    STDMETHOD(put_VertMirror)(/*[in]*/ BOOL newVal);
    
// IPin
    STDMETHOD(Connect(IPin *pReceivePin,const AM_MEDIA_TYPE *pmt));
    STDMETHOD(ReceiveConnection(IPin *pConnector,const AM_MEDIA_TYPE *pmt));
    STDMETHOD(Disconnect());
    STDMETHOD(ConnectedTo(IPin **ppPin));
    STDMETHOD(ConnectionMediaType(AM_MEDIA_TYPE *pmt));
    STDMETHOD(QueryPinInfo(PIN_INFO *pInfo));
    STDMETHOD(QueryId(LPWSTR *Id));
    STDMETHOD(QueryAccept(const AM_MEDIA_TYPE *pmt));
    STDMETHOD(EnumMediaTypes(IEnumMediaTypes **ppEnum));
    STDMETHOD(QueryInternalConnections(IPin **apPin,ULONG *nPin));
    STDMETHOD(EndOfStream());
    STDMETHOD(BeginFlush());
    STDMETHOD(EndFlush());
    STDMETHOD(NewSegment(REFERENCE_TIME tStart,REFERENCE_TIME tStop,double dRate));
    STDMETHOD(QueryDirection(PIN_DIRECTION *pPinDir));
    
// IMemInputPin
    STDMETHOD(GetAllocator(IMemAllocator **ppAllocator));
    STDMETHOD(NotifyAllocator(IMemAllocator *pAllocator,BOOL bReadOnly));
    STDMETHOD(GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps));
    STDMETHOD(Receive(IMediaSample *pSample));
    STDMETHOD(ReceiveMultiple(IMediaSample **pSamples,long nSamples,long *nSamplesProcessed));
    STDMETHOD(ReceiveCanBlock());

// IAMFilterMiscFlags
    ULONG STDMETHODCALLTYPE GetMiscFlags(void);

// CPersistStream
    long GetSize();
    HRESULT SaveToStream(IStream *pStream);
    HRESULT LoadFromStream(IStream *pStream,DWORD dwVersion);

public:
    /// @return true if connected
    bool isConnected() {return m_pConnected!=NULL;};
    
    void resumePause() {m_resumePauseEvent.SetEvent();}

private:
    /**
     * Checks if a mediatype is acceptable
     * @param pmt mediatype to check
     */
    HRESULT CheckMediaType(const AM_MEDIA_TYPE *pmt);
    
    /// @return true if pin is flushing
    bool isFlushing() {return m_bFlushing;}

private:
    ///The filter that this pin belongs to.
    CDSRendFilter *m_pFilter;
    
    ///streaming lock
    CComAutoCriticalSection m_Lock;
    
    ///Event used to make the rendering block when filter is paused
    CEvent m_resumePauseEvent;

    ///pin that this filter is connected to. if this is NULL this pin is not connected
    CComPtr<IPin> m_pConnected;
    ///mediatype of pin connection.
    AM_MEDIA_TYPE m_mt;

    ///flag to signal flushing.
    bool m_bFlushing;

    BOOL m_bForceYUY2;
    DSREND_FIELD_FORMAT m_FieldFormat;
};

#endif //__DSRENDINPIN_H_
