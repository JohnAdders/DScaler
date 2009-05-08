/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Torbjörn Jansson.  All rights reserved.
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
 * @file DSObject.h interface for the CDShowObject class.
 */

#if !defined(AFX_DSOBJECT_H__F5D538DA_72E3_4F81_98D3_4D6673A5E07F__INCLUDED_)
#define AFX_DSOBJECT_H__F5D538DA_72E3_4F81_98D3_4D6673A5E07F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//object types
typedef enum
{
    DSHOW_TYPE_TUNER,
    DSHOW_TYPE_CROSSBAR,
    DSHOW_TYPE_SOURCE_CAPTURE,
    DSHOW_TYPE_SOURCE_FILE,
} eDSObjectType;


/**
 * Base class for directshow objects.
 */
class CDShowObject
{
public:
    CDShowObject(IGraphBuilder *pGraph);
    virtual ~CDShowObject();

    /**
     * This is used to tell the different object types apart, istead of using RTTI.
     * @return type of object
     */
    virtual eDSObjectType getObjectType()=0;

    //name of object
    //virtual TCHAR* getName()=0;

    //virtual void configure(HWND hWnd)=0;
    //virtual BOOL canConfigure()=0;

protected:
    ///graph that the object belongs to
    CComPtr<IGraphBuilder> m_pGraph;
};

#endif // !defined(AFX_DSOBJECT_H__F5D538DA_72E3_4F81_98D3_4D6673A5E07F__INCLUDED_)
