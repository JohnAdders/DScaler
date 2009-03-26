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
 * @file AutoCriticalSection.h interface for the CAutoCriticalSection class.
 */

#if !defined(AFX_AUTOCRITICALSECTION_H__38C52AC0_B93A_4DAB_9D84_35BA3314CC6F__INCLUDED_)
#define AFX_AUTOCRITICALSECTION_H__38C52AC0_B93A_4DAB_9D84_35BA3314CC6F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**
 * Class that automaticaly locks a critical section.
 * This is a simple class that locks the critical section in the constructor
 * and unlocks it in the destructor.
 */
class CAutoCriticalSection
{
public:
    CAutoCriticalSection(CRITICAL_SECTION &pCriticalSection);
    virtual ~CAutoCriticalSection();

private:
    CRITICAL_SECTION *m_pCriticalSection;
};

#endif // !defined(AFX_AUTOCRITICALSECTION_H__38C52AC0_B93A_4DAB_9D84_35BA3314CC6F__INCLUDED_)
