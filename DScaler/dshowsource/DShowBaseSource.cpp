/////////////////////////////////////////////////////////////////////////////
// $Id: DShowBaseSource.cpp,v 1.1 2002-02-07 22:05:43 tobbej Exp $
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
 * @file DShowBaseSource.cpp implementation of the CDShowBaseSource class.
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT

#include "dscaler.h"
#include "DShowBaseSource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CDShowBaseSource::CDShowBaseSource(IGraphBuilder *pGraph)
:CDShowObject(pGraph)
{

}

CDShowBaseSource::~CDShowBaseSource()
{

}

#endif