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
 * @file mediatypes.cpp
 */

#include "stdafx.h"
#include "mediatypes.h"

HRESULT CopyMT::copy(AM_MEDIA_TYPE** p1, AM_MEDIA_TYPE** p2)
{
    ATLTRACE(_T("%s(%d) : CopyMT::copy\n"),__FILE__,__LINE__);
    ATLASSERT(p1!=NULL && *p2!=NULL);
    
    //init p1, this might leak some memory, but acording to the documentation
    //p1 shoud already have been initialized via init function, but that is not always true.
    init(p1);
    if(!copyMediaType(*p1,*p2))
    {
        return E_FAIL;
    }

    return S_OK;
}

void CopyMT::init(AM_MEDIA_TYPE** p)
{
    ATLTRACE(_T("%s(%d) : CopyMT::init\n"),__FILE__,__LINE__);
    ATLASSERT(p!=NULL);

    *p=(AM_MEDIA_TYPE*)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
    memset(*p,0,sizeof(AM_MEDIA_TYPE));
    (*p)->lSampleSize=1;
    (*p)->bFixedSizeSamples=TRUE;
}

void CopyMT::destroy(AM_MEDIA_TYPE** p)
{
    ATLTRACE(_T("%s(%d) : CopyMT::destroy\n"),__FILE__,__LINE__);
    ATLASSERT(*p!=NULL);

    freeMediaType(*p);
    CoTaskMemFree(*p);
}

bool copyMediaType(AM_MEDIA_TYPE *pDest,const AM_MEDIA_TYPE *pSource)
{
    //ATLTRACE(_T("%s(%d) : copyMediaType\n"),__FILE__,__LINE__);
    ATLASSERT(pDest!=NULL && pSource!=NULL);
    
    //dealocate format block of the destination mediatype
    //freeMediaType(pDest);
    
    *pDest=*pSource;

    //is there any format block?
    if(pSource->cbFormat!=0)
    {
        pDest->pbFormat=(BYTE*)CoTaskMemAlloc(pSource->cbFormat);
        if(pDest->pbFormat==NULL)
        {
            pDest->cbFormat=0;
            return false;
        }
        memcpy(pDest->pbFormat,pSource->pbFormat,pSource->cbFormat);
    }
    //addref the IUnknown pointer if any
    if(pDest->pUnk!=NULL)
        pDest->pUnk->AddRef();
    return true;
}

void freeMediaType(AM_MEDIA_TYPE *pmt)
{
    //ATLTRACE(_T("%s(%d) : freeMediaType\n"),__FILE__,__LINE__);
    ATLASSERT(pmt!=NULL);
    
    if(pmt->cbFormat!=0 && pmt->pbFormat!=NULL)
    {
        CoTaskMemFree(pmt->pbFormat);
        pmt->pbFormat=NULL;
        pmt->cbFormat=NULL;
    }
    if(pmt->pUnk!=NULL)
    {
        pmt->pUnk->Release();
        pmt->pUnk=NULL;
    }
}