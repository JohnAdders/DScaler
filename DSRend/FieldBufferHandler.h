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
 * @file FieldBufferHandler.h interface for the CFieldBufferHandler class.
 */

#if !defined(AFX_FIELDBUFFERHANDLER_H__7D741E3E_2A1D_43FA_9BC4_4582BF590BC0__INCLUDED_)
#define AFX_FIELDBUFFERHANDLER_H__7D741E3E_2A1D_43FA_9BC4_4582BF590BC0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Event.h"
#include "DSRend.h"
#include "mem.h"
#include "ColorConverter.h"

/**
 * Class that takes care of buffering IMediaSamples.
 * This class tries to buffer as many IMediaSample:s as posibel and when
 * needed split the sample and/or copy the sample to its own buffer
 * All samples are asumed to be of the same type.
 */
class CFieldBufferHandler  
{
public:
    CFieldBufferHandler();
    virtual ~CFieldBufferHandler();
    
    /**
     * Sets the buffer sizes.
     * When this function is called, no buffers used internaly can be marked
     * as in use or this function will fail.
     *
     * @param FieldCount number of fileds that shoud be bufferd. must be a multiple of 2 if input is frames
     */
    HRESULT SetFieldCount(long FieldCount);
    long GetFieldCount() {return m_FieldCount;}

    /**
     * Sets maximum number of IMediaSamples to hold.
     * The caller must ensure that InsertSample is still called even when
     * MaxMediaSamples number of IMediaSamples is bufferd.
     *
     * @param MaxMediaSamples maximum number of IMediaSample that this class is allowed to hold
     */
    HRESULT SetMediaSampleCount(long MaxMediaSamples);

    /**
     * Sets the mediatype of the fields.
     * When called, no fields can be buffered.
     * @param pMt mediatype, height must be a multiple of 2
     */
    HRESULT SetMediaType(const AM_MEDIA_TYPE *pMt);

    /**
     * Inserts a new IMediaSample and updates the buffer as needed.
     * pSample MUST have the same type as used in SetMediaType()
     * @param pSample media sample
     */
    HRESULT InsertSample(CComPtr<IMediaSample> pSample);
    
    /**
     * Returns an array of field buffers.
     * @param dwTimeout timeout in miliseconds
     * @param count in/out, size of ppBuffers
     * @param ppBuffers the fields
     * @param rtRenderTime time when the newest field in the buffer shoud be rendered
     */
    HRESULT GetFields(DWORD dwTimeOut,long *count,FieldBuffer *ppBuffer,BufferInfo *pBufferInfo,REFERENCE_TIME &rtRenderTime);

    /**
     * Mark all fields returned by GetFields() as not in use.
     */
    HRESULT FreeFields();

    /**
     * Removes all fields.
     * This function is used by the filter when for example the filter is 
     * flushing and needs to remove all buffered IMediaSamples.
     */
    HRESULT RemoveFields(DWORD dwTimeout);

    ///@return number of dropped fields
    int GetDroppedFields();
    ///@return number of fields drawn
    int GetDrawnFields();
    void ResetFieldCounters();

    void SetSwapFields(bool bSwap);
    bool GetSwapFields();
    HRESULT GetStatus(DSRendStatus &status);
    void SetVertMirror(bool bVertMirror);
    bool GetVertMirror();

private:
    HRESULT IsField(AM_MEDIA_TYPE *pMt);
    ///@return true if atleast one buffer in m_Fields is marked as inuse
    bool IsBuffersInUse();
    /// retrieves BITMAPINFOHEADER from m_Mt
    HRESULT GetBitmapInfoHeader(BITMAPINFOHEADER &bmi);

    class CFieldInfo
    {
    public:
        CFieldInfo()
            :pBuffer(NULL),cbSize(0),bInUse(false),rtRenderTime(-1)
        {
        }
        void InsertSample(CComPtr<IMediaSample> sample,BYTE *buffer,ULONG size);
        BYTE* GetBufferSetSize(ULONG cbSize);

        CComPtr<IMediaSample> pSample;
        BYTE *pBuffer;
        ULONG cbSize;
        bool bInUse;
        BUFFER_FLAGS flags;
        REFERENCE_TIME rtRenderTime;
    };
    
    ///total number of fields to buffer
    long m_FieldCount;
    ///number of IMediaSample:s allowed to be held
    long m_MaxMediaSamples;
    
    //field counters
    int m_DroppedFields;
    int m_DrawnFields;
    
    CEvent m_FieldsFreed;
    bool m_bFlushing;
    
    ///signaled when a new sample has arrived
    CEvent m_NewSampleEvent;
    CEvent m_BufferReleased;
    
    CComAutoCriticalSection m_Lock;
    
    /**
     * Array with fields.
     * Oldest field is 0
     */
    CSimpleArray<CFieldInfo> m_Fields;

    ///media type of all buffered samples
    AM_MEDIA_TYPE m_Mt;

    ///field/frame indicator
    bool m_bOneFieldPerSample;

    ///function pointer to optimized memcopy
    MEMCPY_FUNC* m_pfnMemcpy;
    
    bool m_bNeedConv;
    CColorConverter m_ColorConv;
    bool m_bSwapFields;
    bool m_bVertMirror;
};

#endif // !defined(AFX_FIELDBUFFERHANDLER_H__7D741E3E_2A1D_43FA_9BC4_4582BF590BC0__INCLUDED_)
