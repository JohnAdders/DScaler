/////////////////////////////////////////////////////////////////////////////
// $Id: DSGraph.h,v 1.10 2002-05-02 19:50:39 tobbej Exp $
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
// Revision 1.9  2002/04/16 15:26:55  tobbej
// fixed filter reference leak when geting filter names (filters submenu)
// added waitForNextField
//
// Revision 1.8  2002/04/07 14:52:13  tobbej
// fixed race when changing resolution
// improved error handling
//
// Revision 1.7  2002/03/17 21:43:23  tobbej
// added input resolution submenu
//
// Revision 1.6  2002/03/15 23:07:16  tobbej
// changed dropped frames counter to include dropped frames in source filter.
// added functions to enable/disable graph clock.
// started to make changing resolution posibel.
//
// Revision 1.5  2002/02/13 17:01:42  tobbej
// new filter properties menu
//
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
#include "TreeSettingsOleProperties.h"

//if you get an error here, that means you have not checked out the DSRend filter
//or compiled it atleast once.
//it has to be checked out in the same directory as dscaler is checked out in.
#include "..\..\..\DSRend\DSRend.h"

/**
 * Class that manages a direct show filter graph and the filters in it.
 */
class CDShowGraph
{
public:
	/**
	 * Creates a filtergraph with a capture device as source.
	 * @throws CDShowException
	 */
	CDShowGraph(string device,string deviceName);

	/**
	 * Creates a filtergraph with a file as source
	 * @throws CDShowException
	 */
	CDShowGraph(string filename);
	///Destructor
	virtual ~CDShowGraph();
	
	/**
	 * @return pointer to the source device or NULL if there is no source
	 */
	CDShowBaseSource* getSourceDevice();
	bool getNextSample(CComPtr<IMediaSample> &pSample);
	void getConnectionMediatype(AM_MEDIA_TYPE *pmt);
	void waitForNextField();

	/**
	 * Get number of dropped frames.
	 * This function return the total number of dropped frames from the
	 * renderer filter and the source
	 * @return number of dropped frames
	 */
	long getDroppedFrames();

	void start();
	void pause();
	void stop();
	FILTER_STATE getState() {return m_pGraphState;}

	//bool getFilterName(int index,string &filterName,bool &hasPropertyPages);

	/**
	 * Creates a propertypage for a filter.
	 * This function creates a propertypage for the filter indicated by index.
	 * To get all pages call this function with index set to 0 and then
	 * increase it until the function returns false.
	 * The caller is responsibel of deleting the returnd propertypage with
	 * delete.
	 *
	 * @param index filter index
	 * @param ppPage pointer to created propertypage
	 * @return true if the propertypage was successfully created
	 */
	bool getFilterPropertyPage(int index,CTreeSettingsPage **ppPage);

	/**
	 * Change the resolution
	 * @throws CDShowException
	 */
	void changeRes(long x,long y);

	/**
	 * Checks if a resolution is valid and can be selected.
	 * @param x width
	 * @param y height
	 * @return true if valid
	 */
	bool isValidRes(long x, long y);

	/**
	 * Disables the graph reference clock
	 * @throws CDShowException
	 */
	void disableClock();

	/**
	 * Restored the old clock after a call to disableClock()
	 * @throws CDShowException
	 */
	void restoreClock();

private:
	void initGraph();
	void createRenderer();

	void findStreamConfig();
	
	/// updates m_filter vector with filters in the graph
	void buildFilterList();

	///Custom video renderer. Used for transfering the picture to dscaler
	CComPtr<IBaseFilter> m_renderer;
	///Interface used for geting media samples from the renderer filter
	CComPtr<IDSRendFilter> m_DSRend;
	CComPtr<IQualProp> m_pQualProp;
	
	///IAMStreamConfig interface for the filter connected to our renderer
	CComPtr<IAMStreamConfig> m_pStreamCfg;

	CComPtr<IGraphBuilder> m_pGraph;
	CComPtr<ICaptureGraphBuilder2> m_pBuilder;
	CComPtr<IMediaControl> m_pControl;
		
	CDShowBaseSource *m_pSource;

	FILTER_STATE m_pGraphState;

	CComPtr<IReferenceClock> m_pOldRefClk;
	
	/// used in getFilterName and showPropertyPage
	vector<CComPtr<IBaseFilter> > m_filters;

#ifdef _DEBUG
	DWORD m_hROT;
#endif
};

#endif // !defined(AFX_DSGRAPH_H__21DAAF0A_220E_4F8F_A4F6_988DCB31AAE1__INCLUDED_)
