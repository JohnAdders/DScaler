/////////////////////////////////////////////////////////////////////////////
//
// $Id: MultiMon.cpp,v 1.2 2008-03-26 14:55:26 adcockj Exp $
//
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2006 Rob Muller.  All rights reserved.
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
//
// $Log: not supported by cvs2svn $
// Revision 1.1  2006/12/12 23:43:10  robmuller
// Fix compile errors with Visual Studio 2005 Express.
//
//
/////////////////////////////////////////////////////////////////////////////

// Make sure the multi-monitor stubs are included only once.
// This also forces the code to be actually included, solving unresolved externals.

#pragma once

#include "stdafx.h"

// need stubs with VS6 and with shared library builds
#if _MSC_VER < 1300 || defined(_AFXDLL)
#define COMPILE_MULTIMON_STUBS
#include <multimon.h>
#endif