/////////////////////////////////////////////////////////////////////////////
// $Id: DSGraph.h,v 1.5 2002-02-13 17:01:42 tobbej Exp $
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
// Revision 1.4  2002/02/07 22:09:11  tobbej
// changed for new file input
//
// Revision 1.3  2002/02/05 17:27:47  tobbej
// update dropped/drawn fields stats
//
// Revision 1.2  2002/02/03 11:02:34  tobbej
// various updates for new filter
//
// Revision 1.1  2001/12/17 19:30:24  tobbej
// class for managing the capture graph
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file DSGraph.h interface for the CDShowGraph class.
 */

#if !defined(AFX_DSGRAPH_H__21DAAF0A_220E_4F8F_A4F6_988DCB31AAE1__INCLUDED_)
#define AFX_DSGRAPH_H__21DAAF0A_220E_4F8F_A4F6_988DCB31AAE1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "exception.h"
#include "DShowBaseSource.h"

#include "..\..\..\DSRend\DSRend.h"

/**
 * Class that manages a direct show filter graph and the filters in it.
 */
class CDShowGraph
{
public:
	/**
	 * Constructor.
	 * @throws 
	 */
	CDShowGraph(string device,string deviceName);
	CDShowGraph(string filename);
	virtual ~CDShowGraph();

	CDShowBaseSource* getSourceDevice();
	bool getNextSample(CComPtr<IMediaSample> &pSample);
	void getConnectionMediatype(AM_MEDIA_TYPE *pmt);
	int getDroppedFrames();

	void start();
	void pause();
	void stop();
	FILTER_STATE getState() {return m_pGraphState;}

	bool getFilterName(int index,string &filterName,bool &hasPropertyPages);
	void showPropertyPage(HWND hParent,int index);
	
private:
	void showPropertyPage(HWND hParent,string caption,CComPtr<IBaseFilter> pFilter);
	
	void initGraph();
	void createRenderer();

	void setRes(long x,long y);
	void findStreamConfig();

	///Custom video renderer. Used for transfering the picture to dscaler
	CComPtr<IBaseFilter> m_renderer;
	CComPtr<IDSRendFilter> m_DSRend;
	CComPtr<IQualProp> m_pQualProp;
	
	///IAMStreamConfig interface for the filter connected to our renderer
	CComPtr<IAMStreamConfig> m_pStreamCfg;

	CComPtr<IGraphBuilder> m_pGraph;
	CComPtr<ICaptureGraphBuilder2> m_pBuilder;
	CComPtr<IMediaControl> m_pControl;
		
	CDShowBaseSource *m_pSource;

	FILTER_STATE m_pGraphState;

#ifdef _DEBUG
	DWORD m_hROT;
#endif
};

#endif // !defined(AFX_DSGRAPH_H__21DAAF0A_220E_4F8F_A4F6_988DCB31AAE1__INCLUDED_)
