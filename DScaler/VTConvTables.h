/////////////////////////////////////////////////////////////////////////////
// $Id: VTConvTables.h,v 1.2 2003-10-27 10:39:54 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Mike Temperton.  All rights reserved.
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
//  GNU Library General Public License for more details
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.1  2002/01/15 11:16:03  temperton
// New teletext drawing code.
//
//
/////////////////////////////////////////////////////////////////////////////

/** 
 * @file vtconvtables.h vtconvtables Header file
 */
 
#ifndef __VTCONVTABLES_H__
#define __VTCONVTABLES_H__

#include "stdafx.h"
#include "VBI_VideoText.h"

WORD VTCharToUnicode(eVTCodePage VTCharSet, int VTChar);

#endif
