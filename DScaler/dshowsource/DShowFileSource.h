/////////////////////////////////////////////////////////////////////////////
// $Id: DShowFileSource.h,v 1.5 2002-09-14 17:04:24 tobbej Exp $
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
// Revision 1.4  2002/08/01 20:22:13  tobbej
// improved error messages when opening files.
// corrected some smal problems when opening .grf files
//
// Revision 1.3  2002/03/15 23:01:54  tobbej
// changed dropped frames counter to include dropped frames in source filter
//
// Revision 1.2  2002/02/13 16:58:22  tobbej
// changed some comments
//
// Revision 1.1  2002/02/07 22:05:43  tobbej
// new classes for file input
// rearanged class inheritance a bit
//
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
#include "exception.h"
#include <string>
/**
 * Exception class used in CDShowFileSource.
 * @see CDShowException
 * @see CDShowFileSource
 */
class CDShowUnsupportedFileException: public CDShowException
{
public:
	CDShowUnsupportedFileException(CString msg,HRESULT hr):CDShowException(msg,hr) {};
	CDShowUnsupportedFileException(CString msg):CDShowException(msg) {};
};

/**
 * Direct Show file input source.
 */
class CDShowFileSource : public CDShowBaseSource
{
public:
	CDShowFileSource(IGraphBuilder *pGraph,string filename);
	virtual ~CDShowFileSource();
	
	eDSObjectType getObjectType() {return DSHOW_TYPE_SOURCE_FILE;}
	void Connect(CComPtr<IBaseFilter> VideoFilter,CComPtr<IBaseFilter> AudioFilter);
	bool isConnected() {return m_bIsConnected;};
	long getNumDroppedFrames(){return 0;};
	
	string getFileName() {return m_file;}

private:
	string m_file;
	CComPtr<IBaseFilter> m_pFileSource;
	CComPtr<ICaptureGraphBuilder2> m_pBuilder;
	bool m_bIsConnected;
};

#endif // !defined(AFX_DSHOWFILESOURCE_H__D8C80F6D_F29B_4805_82AD_2B6D1B2FCD96__INCLUDED_)
