/////////////////////////////////////////////////////////////////////////////
// $Id: DSObject.h,v 1.1 2001-12-17 19:25:05 tobbej Exp $
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
// Change Log
//
// Date          Developer             Changes
//
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
//
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
/*
typedef enum
{
	DS_TYPE_TUNER,
	DS_TYPE_CROSSBAR,
	DS_TYPE_VIDEO_RENDERER,
	DS_TYPE_TELETEXT,
	DS_TYPE_SOURCE_TV,
	DS_TYPE_SOURCE_FILE,
} eDSObjectType;
*/

/**
 * Base class for directshow objects
 */
class CDShowObject
{
public:
	CDShowObject(IGraphBuilder *pGraph);
	virtual ~CDShowObject();
	
	//type
	//virtual eDSObjectType getType()=0;
	
	//name of object
	//virtual char* getName()=0;
	
	//virtual void configure(HWND hWnd)=0;
	//virtual bool canConfigure()=0;

protected:
	///graph that the object belongs to
	CComPtr<IGraphBuilder> m_pGraph;
};

#endif // !defined(AFX_DSOBJECT_H__F5D538DA_72E3_4F81_98D3_4D6673A5E07F__INCLUDED_)
