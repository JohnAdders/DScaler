/////////////////////////////////////////////////////////////////////////////
// $Id: debug.h,v 1.3 2004-12-11 21:53:09 laurentg Exp $
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
// Revision 1.2  2003/03/05 21:44:22  tobbej
// new debuging functions
//
// Revision 1.1  2001/12/09 22:01:48  tobbej
// experimental dshow support, doesnt work yet
// define WANT_DSHOW_SUPPORT if you want to try it
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file debug.h some dshow debuging functions
 */

//#ifdef _DEBUG

/**
 * This function makes it posible to use graphedit to view/change a running filter graph.
 * @param pUnkGraph filter graph
 * @param pdwRegister
 */
HRESULT AddToRot(IUnknown *pUnkGraph, DWORD *pdwRegister);

/**
 * Removes the filter graph from rot.
 * @param pdwRegister value returned from AddToRot
 */
void RemoveFromRot(DWORD pdwRegister);

//#endif

/**
 * @param pGraph
 * @param text
 */
void DumpGraph(IFilterGraph *pGraph,std::string &text);

/**
 * @param pFilter filter to dump
 * @param text textual description of filter
 */
void DumpFilter(CComPtr<IBaseFilter> pFilter,std::string &text);

/**
 * Dumps the preferred mediatypes of all pins on a filter.
 * @param pFilter
 * @param text
 */
void DumpPreferredMediaTypes(CComPtr<IBaseFilter> pFilter,std::string &text);

/**
 * Decodes a AM_MEDIA_TYPE struct to text.
 * @param mt mediatype to dump
 * @param text textual description of the mediatype
 */
void DumpMediaType(AM_MEDIA_TYPE *mt,std::string &text);