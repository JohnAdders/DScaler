/////////////////////////////////////////////////////////////////////////////
// $Id: DShowFileSource.h,v 1.1 2002-02-07 22:05:43 tobbej Exp $
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
 * @file DShowFileSource.h interface for the CDShowFileSource class.
 */

#if !defined(AFX_DSHOWFILESOURCE_H__D8C80F6D_F29B_4805_82AD_2B6D1B2FCD96__INCLUDED_)
#define AFX_DSHOWFILESOURCE_H__D8C80F6D_F29B_4805_82AD_2B6D1B2FCD96__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DShowBaseSource.h"

class CDShowFileSource : public CDShowBaseSource  
{
public:
	CDShowFileSource(IGraphBuilder *pGraph,string filename);
	virtual ~CDShowFileSource();
	
	eDSObjectType getObjectType() {return DSHOW_TYPE_SOURCE_FILE;}
	void connect(CComPtr<IBaseFilter> filter);
	bool isConnected() {return m_bIsConnected;};
	
	string getFileName() {return m_file;}

private:
	string m_file;
	CComPtr<IBaseFilter> m_pFileSource;
	CComPtr<ICaptureGraphBuilder2> m_pBuilder;
	bool m_bIsConnected;
};

#endif // !defined(AFX_DSHOWFILESOURCE_H__D8C80F6D_F29B_4805_82AD_2B6D1B2FCD96__INCLUDED_)
