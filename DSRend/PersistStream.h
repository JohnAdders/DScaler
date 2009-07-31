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
 * @file PersistStream.h interface for the CPersistStream class.
 */

#if !defined(AFX_PERSISTSTREAM_H__5C9B3612_6CD4_4AC3_A7EB_1D6FB740F777__INCLUDED_)
#define AFX_PERSISTSTREAM_H__5C9B3612_6CD4_4AC3_A7EB_1D6FB740F777__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPersistStream :
    public IPersistStream
{
public:
    CPersistStream();
    virtual ~CPersistStream();
    
    //IPersist
    HRESULT STDMETHODCALLTYPE GetClassID(CLSID *pClsid)=0;

    //IPersistStream
    HRESULT STDMETHODCALLTYPE IsDirty();
    HRESULT STDMETHODCALLTYPE Load(IStream *pStm);
    HRESULT STDMETHODCALLTYPE Save(IStream *pStm, BOOL fClearDirty);
    HRESULT STDMETHODCALLTYPE GetSizeMax(ULARGE_INTEGER *pcbSize);
    
    void SetDirty(bool bDirty);
    
protected:
    virtual long GetSize()=0;
    virtual HRESULT SaveToStream(IStream *pStream)=0;
    virtual HRESULT LoadFromStream(IStream *pStream)=0;
    virtual DWORD GetVersion();
    
    DWORD m_dwVersion;

private:
    bool m_bIsDirty;
};

#endif // !defined(AFX_PERSISTSTREAM_H__5C9B3612_6CD4_4AC3_A7EB_1D6FB740F777__INCLUDED_)
