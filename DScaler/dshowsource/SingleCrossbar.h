/////////////////////////////////////////////////////////////////////////////
// $Id: SingleCrossbar.h,v 1.1 2001-12-17 19:22:33 tobbej Exp $
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
 * @file SingleCrossbar.h interface for the CDShowSingleCrossbar class.
 */

#if !defined(AFX_SINGLECROSSBAR_H__28485589_E809_4ED1_8F98_52F0740B6A11__INCLUDED_)
#define AFX_SINGLECROSSBAR_H__28485589_E809_4ED1_8F98_52F0740B6A11__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseCrossbar.h"

/**
 * This class manages one crossbar filter.
 */
class CDShowSingleCrossbar :public CDShowBaseCrossbar
{
public:
	CDShowSingleCrossbar(CComPtr<IAMCrossbar> &pCrossbar,IGraphBuilder *pGraph);
	virtual ~CDShowSingleCrossbar();

	long GetInputCount();
	long GetInputIndex();
	bool isInputSelected(long index);

	PhysicalConnectorType GetInputType(long Index);
	void SetInputIndex(long Index,bool bSetRelated);

private:
	/// Crossbar filter
	CComPtr<IAMCrossbar> m_crossbar;
	long m_currentRoutingIndex;
};

#endif // !defined(AFX_SINGLECROSSBAR_H__28485589_E809_4ED1_8F98_52F0740B6A11__INCLUDED_)
