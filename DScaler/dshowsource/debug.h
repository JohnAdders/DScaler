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