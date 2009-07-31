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
 * @file FieldBufferHandler.cpp implementation of the CFieldBufferHandler class.
 */

#include "stdafx.h"
#include "FieldBufferHandler.h"
#include "mediatypes.h"
#include "AutoLockCriticalSection.h"
#include <dvdmedia.h>
#include "cpu.h"

CFieldBufferHandler::CFieldBufferHandler()
:m_FieldCount(0),
m_MaxMediaSamples(0),
m_DroppedFields(0),
m_DrawnFields(0),
m_bOneFieldPerSample(false),
m_bFlushing(false),
m_pfnMemcpy(NULL),
m_bNeedConv(false),
m_bSwapFields(false),
m_bVertMirror(false)
{
    //clear mediatype
    memset(&m_Mt,0,sizeof(AM_MEDIA_TYPE));

    //optimized memcpy
    if(CpuFeatureFlags&FEATURE_SSE)
    {
        m_pfnMemcpy=memcpySSE;
    }
    else if(CpuFeatureFlags&FEATURE_MMX)
    {
        m_pfnMemcpy=memcpyMMX;
    }
    else
    {
        m_pfnMemcpy=memcpySimple;
    }
}

CFieldBufferHandler::~CFieldBufferHandler()
{
    freeMediaType(&m_Mt);
    ATLASSERT(m_Fields.GetSize()==0);
}

HRESULT CFieldBufferHandler::SetFieldCount(long FieldCount)
{
    ATLTRACE(_T("%s(%d) : CFieldBufferHandler::SetFieldCount\n"),__FILE__,__LINE__);
    CAutoLockCriticalSection lock(&m_Lock);
    ATLASSERT(m_Fields.GetSize()==0);
    m_FieldCount=FieldCount;
    return S_OK;
}

HRESULT CFieldBufferHandler::SetMediaSampleCount(long MaxMediaSamples)
{
    ATLTRACE(_T("%s(%d) : CFieldBufferHandler::SetMediaSampleCount\n"),__FILE__,__LINE__);
    CAutoLockCriticalSection lock(&m_Lock);
    ATLASSERT(m_Fields.GetSize()==0);
    
    //maybe check the array if MaxMediaSamples<m_MaxMediaSamples

    m_MaxMediaSamples=MaxMediaSamples;
    return S_OK;
}

HRESULT CFieldBufferHandler::SetMediaType(const AM_MEDIA_TYPE *pMt)
{
    ATLTRACE(_T("%s(%d) : CFieldBufferHandler::SetMediaType\n"),__FILE__,__LINE__);
    if(pMt==NULL)
    {
        return E_POINTER;
    }

    CAutoLockCriticalSection lock(&m_Lock);
    if(m_Fields.GetSize()!=0)
    {
        return E_UNEXPECTED;
    }

    if(pMt->majortype==MEDIATYPE_Video && pMt->cbFormat>0 && pMt->pbFormat!=NULL)
    {
        //check if color conversion is needed
        m_bNeedConv=false;
        if(pMt->subtype!=MEDIASUBTYPE_YUY2)
        {
            if(!m_ColorConv.CanCovert(pMt))
            {
                return VFW_E_INVALIDMEDIATYPE;
            }
            
            m_bNeedConv=true;
            if(!m_ColorConv.SetFormat(pMt))
            {
                return VFW_E_INVALIDMEDIATYPE;
            }
        }

        //check if the samples contain fields or frames
        if(pMt->formattype==FORMAT_VideoInfo)
        {
            m_bOneFieldPerSample=false;
        }
        else if(pMt->formattype==FORMAT_VideoInfo2)
        {
            VIDEOINFOHEADER2 *pInfo=(VIDEOINFOHEADER2*)pMt->pbFormat;
            if(pInfo->dwInterlaceFlags&AMINTERLACE_1FieldPerSample)
            {
                m_bOneFieldPerSample=true;
            }
            else
            {
                m_bOneFieldPerSample=false;
            }
        }
        else
        {
            return E_FAIL;
        }

        freeMediaType(&m_Mt);
        copyMediaType(&m_Mt,pMt);
        return S_OK;
    }
    return E_FAIL;
}

HRESULT CFieldBufferHandler::InsertSample(CComPtr<IMediaSample> pSample)
{
    //ATLTRACE(_T("%s(%d) : CFieldBufferHandler::InsertSample\n"),__FILE__,__LINE__);
    if(m_bFlushing==true)
    {
        //increase m_DroppedFields? or will the source filter count it as dropped?
        return S_FALSE;
    }
    
    if(pSample==NULL)
    {
        return E_POINTER;
    }
    
    m_Lock.Lock();
    //count the field as dropped directly if we dont buffer anything
    if(m_FieldCount==0)
    {
        m_DroppedFields++;
        m_Lock.Unlock();
        return S_OK;
    }
    unsigned long sampleSize=pSample->GetActualDataLength();
    BYTE *pSampleBuffer=NULL;
    HRESULT hr=pSample->GetPointer(&pSampleBuffer);
    if(FAILED(hr))
    {
        m_Lock.Unlock();
        return hr;
    }
    bool bTimeValid=false;
    REFERENCE_TIME rtStart=-1;
    REFERENCE_TIME rtEnd=-1;
    hr=pSample->GetTime(&rtStart,&rtEnd);
    if(SUCCEEDED(hr))
    {
        bTimeValid=true;
    }
    
    if(m_Fields.GetSize()>=m_FieldCount)
    {
        if(m_Fields[0].bInUse==true)
        {
            //wait for an empty buffer to use
            m_BufferReleased.ResetEvent();
            m_Lock.Unlock();
            while(true)
            {
                if(!m_BufferReleased.Wait(400))
                {
                    m_DroppedFields++;

                    //dont return failure, if we do we wont get any more samples
                    return S_OK;
                }
                m_Lock.Lock();
                if(m_Fields[0].bInUse==false)
                {
                    break;
                }
                m_Lock.Unlock();
            }
        }
    }

    BITMAPINFOHEADER bmiHeader;
    hr=GetBitmapInfoHeader(bmiHeader);
    ATLASSERT(SUCCEEDED(hr));
    ATLASSERT(bmiHeader.biHeight>0);
    ATLASSERT(sampleSize>=bmiHeader.biSizeImage);

    if(!m_bOneFieldPerSample)
    {
        //one frame per sample

        long LineSize=bmiHeader.biWidth*2;
        size_t FieldSize=(bmiHeader.biHeight/2)*LineSize;
        bool bVertMirror=m_bVertMirror;
        
        //first field
        CFieldInfo field1;
        if(m_Fields.GetSize()>=m_FieldCount)
        {
            ATLASSERT(m_Fields[0].bInUse==false);
            field1=m_Fields[0];
            m_Fields.RemoveAt(0);
            ///@todo m_DroppedFields++ but only if the buffer was never used
        }
        
        //split the field
        if(m_bNeedConv)
        {
            m_ColorConv.Convert(field1.GetBufferSetSize(FieldSize),pSampleBuffer,CColorConverter::CNV_EVEN,bVertMirror);
            field1.flags=(bVertMirror ? !m_bSwapFields : m_bSwapFields) ? BUFFER_FLAGS_FIELD_ODD : BUFFER_FLAGS_FIELD_EVEN;
        }
        else
        {
            field1.GetBufferSetSize(FieldSize);
            for(int i=0;i<bmiHeader.biHeight/2;i++)
            {
                m_pfnMemcpy(field1.pBuffer+i*LineSize,pSampleBuffer+ (bVertMirror ? (bmiHeader.biHeight-1-(2*i))*LineSize : (2*i)*LineSize),LineSize);
            }
            field1.flags=m_bSwapFields ? BUFFER_FLAGS_FIELD_ODD : BUFFER_FLAGS_FIELD_EVEN;
        }
        ATLASSERT(field1.bInUse==false);
        

        //set timestamp when sample shoud be rendered
        field1.rtRenderTime=-1;
        if(bTimeValid)
        {
            field1.rtRenderTime=rtStart;
        }

        m_Fields.Add(field1);
        //signal that a new sample has arrived
        m_NewSampleEvent.SetEvent();
        
        //second field
        //get a new buffer
        if(m_Fields.GetSize()>=m_FieldCount)
        {
            if(m_Fields[0].bInUse==true)
            {
                m_BufferReleased.ResetEvent();
                m_Lock.Unlock();
                while(true)
                {
                    if(!m_BufferReleased.Wait(400))
                    {
                        m_DroppedFields++;

                        //dont return failure, if we do we wont get any more samples
                        return S_OK;
                    }
                    m_Lock.Lock();
                    if(m_Fields[0].bInUse==false)
                    {
                        break;
                    }
                    m_Lock.Unlock();
                }
            }
        }
        CFieldInfo field2;
        if(m_Fields.GetSize()>=m_FieldCount)
        {
            ATLASSERT(m_Fields[0].bInUse==false);
            field2=m_Fields[0];
            m_Fields.RemoveAt(0);
            ///@todo m_DroppedFields++ but only if the buffer was never used
        }

        bVertMirror=m_bVertMirror;
        //split the field
        if(m_bNeedConv)
        {
            m_ColorConv.Convert(field2.GetBufferSetSize(FieldSize),pSampleBuffer,CColorConverter::CNV_ODD,bVertMirror);
            field2.flags= (bVertMirror ? !m_bSwapFields : m_bSwapFields) ? BUFFER_FLAGS_FIELD_EVEN : BUFFER_FLAGS_FIELD_ODD;
        }
        else
        {
            field2.GetBufferSetSize(FieldSize);
            for(int i=0;i<bmiHeader.biHeight/2;i++)
            {
                m_pfnMemcpy(field2.pBuffer+i*LineSize,pSampleBuffer+ (bVertMirror ? (bmiHeader.biHeight-1-(2*i+1))*LineSize : (2*i+1)*LineSize),LineSize);
            }
            field2.flags=m_bSwapFields ? BUFFER_FLAGS_FIELD_EVEN : BUFFER_FLAGS_FIELD_ODD;
        }
        ATLASSERT(field2.bInUse==false);
        
        
        //set timestamp when sample shoud be rendered
        field2.rtRenderTime=-1;
        if(bTimeValid)
        {
            if(rtEnd!=-1)
            {
                field2.rtRenderTime=rtStart+(rtEnd-rtStart)/2;
            }
        }

        m_Fields.Add(field2);
    }
    else
    {
        //one field per sample
        
        //count number of IMediaSamples that is held in the filed buffer
        int cMediaSamples=0;
        for(int i=0;i<m_Fields.GetSize();i++)
        {
            if(m_Fields[i].pSample!=NULL)
                cMediaSamples++;
        }
        
        CFieldInfo field;
        if(m_Fields.GetSize()>=m_FieldCount)
        {
            field=m_Fields[0];
            m_Fields.RemoveAt(0);
        }

        //check if this sample needs to be copied
        if(m_bNeedConv || m_bVertMirror || cMediaSamples>=m_MaxMediaSamples)
        {
            //copy IMediaSample to a new buffer
            if(m_bNeedConv)
            {
                m_ColorConv.Convert(field.GetBufferSetSize(sampleSize),pSampleBuffer,CColorConverter::CNV_ALL,m_bVertMirror);
            }
            else if(m_bVertMirror)
            {
                field.GetBufferSetSize(sampleSize);
                for(long i=0;i<bmiHeader.biHeight;i++)
                {
                    m_pfnMemcpy(field.pBuffer+i*bmiHeader.biWidth*2,pSampleBuffer+((bmiHeader.biHeight-i)*bmiHeader.biWidth*2),bmiHeader.biWidth*2);
                }
            }
            else
            {
                m_pfnMemcpy(field.GetBufferSetSize(sampleSize),pSampleBuffer,sampleSize);
            }
            ATLASSERT(field.cbSize>=sampleSize);
        }
        else
        {
            //buffer IMediaSample directly
            field.InsertSample(pSample,pSampleBuffer,sampleSize);
        }

        //get even/odd flag
        field.flags=BUFFER_FLAGS_FIELD_UNKNOWN;
        CComPtr<IMediaSample2> pSample2;
        HRESULT hr=pSample->QueryInterface(IID_IMediaSample2,(void**)&pSample2);
        if(SUCCEEDED(hr))
        {
            AM_SAMPLE2_PROPERTIES prop;
            hr=pSample2->GetProperties(sizeof(AM_SAMPLE2_PROPERTIES),(BYTE*)&prop);
            if(SUCCEEDED(hr))
            {
                switch(prop.dwTypeSpecificFlags&AM_VIDEO_FLAG_FIELD_MASK)
                {
                case AM_VIDEO_FLAG_FIELD1:
                    field.flags=(m_bVertMirror ? !m_bSwapFields: m_bSwapFields) ? BUFFER_FLAGS_FIELD_ODD : BUFFER_FLAGS_FIELD_EVEN;
                    break;
                case AM_VIDEO_FLAG_FIELD2:
                    field.flags=(m_bVertMirror ? !m_bSwapFields: m_bSwapFields) ? BUFFER_FLAGS_FIELD_EVEN : BUFFER_FLAGS_FIELD_ODD;
                    break;
                }
            }
        }
        
        //set timestamp when sample shoud be rendered
        field.rtRenderTime=-1;
        if(bTimeValid)
        {
            field.rtRenderTime=rtStart;
        }

        ATLASSERT(field.bInUse==false);
        m_Fields.Add(field);
    }
    //if this assert is triggered then something is wrong with the handling of m_Fields
    //the size of m_Fields can never be larger than m_FieldCount
    ATLASSERT(m_Fields.GetSize()<=m_FieldCount);

    //signal that a new sample has arrived
    m_NewSampleEvent.SetEvent();
    m_Lock.Unlock();

    return S_OK;
}

HRESULT CFieldBufferHandler::GetFields(DWORD dwTimeOut,long *count,FieldBuffer *pBuffers,BufferInfo *pBufferInfo,REFERENCE_TIME &rtRenderTime)
{
    //ATLTRACE(_T("%s(%d) : CFieldBufferHandler::GetFields\n"),__FILE__,__LINE__);
    
    if(m_bFlushing==true)
    {
        return E_UNEXPECTED;
    }

    if(pBuffers==NULL || count==NULL || pBufferInfo==NULL)
    {
        return E_POINTER;
    }
    //requesting more fields than we buffer?
    if(*count>m_FieldCount || m_FieldCount==0)
    {
        return E_INVALIDARG;
    }
    
    m_Lock.Lock();
    //check if we need to wait for a new field
    if(m_Fields.GetSize()==0 || m_Fields[m_Fields.GetSize()-1].bInUse)
    {
        //if the field buffer is full, mark the oldest as not in use
        //dont need it anyway since we have to wait
        if(m_Fields.GetSize()>=m_FieldCount)
        {
            if(m_Fields[0].bInUse==true)
            {
                m_Fields[0].bInUse=false;
                m_BufferReleased.SetEvent();
            }
        }

        m_NewSampleEvent.ResetEvent();
        m_Lock.Unlock();
        //wait for a new sample to arrive
        if(!m_NewSampleEvent.Wait(dwTimeOut))
        {
            return VFW_E_TIMEOUT;
        }
        m_Lock.Lock();
    }
    
    int startPos=0;
    int i;
    //find newest field marked as in use
    for(i=m_Fields.GetSize()-1;i>=0;i--)
    {
        if(m_Fields[i].bInUse)
            break;
    }
    if(i>=0)
    {
        startPos=i+1;
        ATLASSERT(startPos>=0 && startPos<m_Fields.GetSize());
    }
    else
    {
        //didnt find any, start at the newest field
        //startPos=m_Fields.GetSize()-1;
        startPos=0;
    }
    
    //fill ppBuffer array
    int markedFields=0;
    int fieldCount=0;
    for(i=startPos;i>=0 && fieldCount<*count;i--)
    {
        if(m_Fields[i].bInUse==false)
        {
            m_Fields[i].bInUse=true;
            markedFields++;
        }
        ///@todo check buffer size
        pBuffers[fieldCount].pBuffer=m_Fields[i].pBuffer;
        pBuffers[fieldCount].flags=m_Fields[i].flags;
        fieldCount++;
    }
    m_DrawnFields+=markedFields;
#ifdef _DEBUG
    if(*count!=fieldCount)
    {
        ATLTRACE(_T("%s(%d) :  Requested %d fields, Returned %d\n"),__FILE__,__LINE__,*count,fieldCount);
    }
#endif
    *count=fieldCount;

    memset(pBufferInfo,0,sizeof(BufferInfo));
    BITMAPINFOHEADER bmi;
    GetBitmapInfoHeader(bmi);
    pBufferInfo->bIsField=m_bOneFieldPerSample;
    pBufferInfo->Width=bmi.biWidth;
    pBufferInfo->Height=m_bOneFieldPerSample==true ? bmi.biHeight : bmi.biHeight/2;
    pBufferInfo->CurrentFrame=(m_DrawnFields+m_DroppedFields)%5;
    
    //mark the unused fields at the end of the field array as not in use anymore
    for(i=0;i<=(startPos-fieldCount) && fieldCount>0;i++)
    {
        m_Fields[i].bInUse=false;
        m_BufferReleased.SetEvent();

        /*CFieldInfo info=m_Fields[0];
        if(info.pSample!=NULL)
        {
            info.pSample=NULL;
            info.pBuffer=NULL;
            info.cbSize=0;
        }
        else
        {
            aligned_free(info.pBuffer);
            info.pBuffer=NULL;
            info.cbSize=0;
        }
        m_Fields.RemoveAt(0);*/
    }
    m_FieldsFreed.ResetEvent();
    m_Lock.Unlock();

    rtRenderTime=m_Fields[startPos].rtRenderTime;

    return S_OK;
}

HRESULT CFieldBufferHandler::FreeFields()
{
    ATLTRACE(_T("%s(%d) : CFieldBufferHandler::FreeFields\n"),__FILE__,__LINE__);
    CAutoLockCriticalSection lock(&m_Lock);
    for(int i=0;i<m_Fields.GetSize();i++)
    {
        m_Fields[i].bInUse=false;
    }
    m_FieldsFreed.SetEvent();
    return S_OK;
}

HRESULT CFieldBufferHandler::RemoveFields(DWORD dwTimeout)
{
    ATLTRACE(_T("%s(%d) : CFieldBufferHandler::RemoveFields\n"),__FILE__,__LINE__);
    
    m_bFlushing=true;
    m_Lock.Lock();
    if(IsBuffersInUse())
    {
        m_Lock.Unlock();
        if(!m_FieldsFreed.Wait(dwTimeout))
        {
            m_bFlushing=false;
            return VFW_E_TIMEOUT;
        }
        m_Lock.Lock();
    }
    while(m_Fields.GetSize()!=0)
    {
        CFieldInfo info=m_Fields[0];
        ATLASSERT(!info.bInUse);
        m_Fields.RemoveAt(0);
        if(info.pSample!=NULL)
        {
            info.pSample=NULL;
            info.cbSize=0;
            info.pBuffer=NULL;
        }
        else
        {
            //it might be posibel to skip this, since we only need to free the IMediaSample:s
            aligned_free(info.pBuffer);
            info.pBuffer=NULL;
            info.cbSize=0;
        }
    }
    m_bFlushing=false;
    m_Lock.Unlock();
    return S_OK;
}

bool CFieldBufferHandler::IsBuffersInUse()
{
    for(int i=0;i<m_Fields.GetSize();i++)
    {
        if(m_Fields[i].bInUse)
            return true;
    }
    return false;
}

HRESULT CFieldBufferHandler::IsField(AM_MEDIA_TYPE *pMt)
{
    if(pMt==NULL)
        return E_POINTER;

    if(pMt->majortype==MEDIATYPE_Video && pMt->cbFormat>0 && pMt->pbFormat!=NULL)
    {
        if(pMt->formattype==FORMAT_VideoInfo)
        {
            return S_FALSE;
        }
        else if(pMt->formattype==FORMAT_VideoInfo2)
        {
            VIDEOINFOHEADER2 *pInfo=(VIDEOINFOHEADER2*)pMt->pbFormat;
            if(pInfo->dwInterlaceFlags&AMINTERLACE_1FieldPerSample)
            {
                return S_OK;
            }
            else
            {
                return S_FALSE;
            }
        }
    }
    return E_FAIL;
}

HRESULT CFieldBufferHandler::GetBitmapInfoHeader(BITMAPINFOHEADER &bmi)
{
    //ATLASSERT(m_Mt!=NULL);
    memset(&bmi,0,sizeof(BITMAPINFOHEADER));
    if(m_Mt.majortype==MEDIATYPE_Video)
    {
        if(m_Mt.formattype==FORMAT_VideoInfo)
        {
            VIDEOINFOHEADER *vh=(VIDEOINFOHEADER *)m_Mt.pbFormat;
            bmi=vh->bmiHeader;
            return S_OK;
        }
        else if(m_Mt.formattype==FORMAT_VideoInfo2)
        {
            VIDEOINFOHEADER2 *vh2=(VIDEOINFOHEADER2 *)m_Mt.pbFormat;
            //ATLTRACE(_T(" fps=%d\n"),(int)(1/(vh2->AvgTimePerFrame/(double)10000000)));
            bmi=vh2->bmiHeader;
            return S_OK;
        }
    }
    return E_FAIL;
}

int CFieldBufferHandler::GetDroppedFields()
{
    return m_DroppedFields;
}

int CFieldBufferHandler::GetDrawnFields()
{
    return m_DrawnFields;
}

void CFieldBufferHandler::ResetFieldCounters()
{
    m_DroppedFields=0;
    m_DrawnFields=0;
}

void CFieldBufferHandler::SetSwapFields(bool bSwap)
{
    m_bSwapFields=bSwap;
}

bool CFieldBufferHandler::GetSwapFields()
{
    return m_bSwapFields;
}

void CFieldBufferHandler::SetVertMirror(bool bVertMirror)
{
    m_bVertMirror=bVertMirror;
}

bool CFieldBufferHandler::GetVertMirror()
{
    return m_bVertMirror;
}

HRESULT CFieldBufferHandler::GetStatus(DSRendStatus &status)
{
    BITMAPINFOHEADER bmi;
    HRESULT hr=GetBitmapInfoHeader(bmi);
    if(SUCCEEDED(hr))
    {
        status.Height=bmi.biHeight;
        status.Width=bmi.biWidth;
        status.bFieldInput=m_bOneFieldPerSample;
        status.bNeedConv=m_bNeedConv;
    }
    return hr;
}

void CFieldBufferHandler::CFieldInfo::InsertSample(CComPtr<IMediaSample> sample,BYTE *buffer,ULONG size)
{
    if(pSample==NULL && pBuffer!=NULL)
    {
        aligned_free(pBuffer);
        pBuffer=NULL;
        cbSize=0;
    }

    pSample=sample;
    cbSize=size;
    pBuffer=buffer;
}

BYTE* CFieldBufferHandler::CFieldInfo::GetBufferSetSize(ULONG size)
{
    if(pSample!=NULL)
    {
        pSample=NULL;
        pBuffer=(BYTE*)aligned_malloc(size,0x10);
        cbSize=size;
    }
    else
    {
        //check if the old buffer is large enough
        if(cbSize<size)
        {
            if(pBuffer!=NULL)
            {
                aligned_free(pBuffer);
            }
            pBuffer=(BYTE*)aligned_malloc(size,0x10);
            cbSize=size;
        }
    }
    return pBuffer;
}