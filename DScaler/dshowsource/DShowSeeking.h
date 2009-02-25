/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 Torbjörn Jansson.  All rights reserved.
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
/////////////////////////////////////////////////////////////////////////////

/**
 * @file DShowSeeking.h interface for the CDShowSeeking class.
 */


#if !defined(AFX_DSHOWSEEKING_H__EBB86CD5_7B1E_498C_AE39_9D199FE54936__INCLUDED_)
#define AFX_DSHOWSEEKING_H__EBB86CD5_7B1E_498C_AE39_9D199FE54936__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**
 *
 */
class CDShowSeeking  
{
public:
	CDShowSeeking(CComPtr<IGraphBuilder> &pGraph);
	virtual ~CDShowSeeking();
	DWORD GetCaps();
	LONGLONG GetCurrentPos();
	LONGLONG GetDuration();
	void SeekTo(LONGLONG position);

private:
	CComPtr<IMediaSeeking> m_pSeeking;

};

#endif // !defined(AFX_DSHOWSEEKING_H__EBB86CD5_7B1E_498C_AE39_9D199FE54936__INCLUDED_)
