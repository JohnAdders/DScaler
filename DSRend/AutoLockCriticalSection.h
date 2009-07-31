/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file AutoLockCriticalSection.h interface for the CAutoLockCriticalSection class.
 */

#if !defined(AFX_AUTOLOCKCRITICALSECTION_H__810D87A4_1887_4204_BE10_E1CC17682B54__INCLUDED_)
#define AFX_AUTOLOCKCRITICALSECTION_H__810D87A4_1887_4204_BE10_E1CC17682B54__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**
 * Class for automaticly lock a critical section.
 */
class CAutoLockCriticalSection
{
public:
    CAutoLockCriticalSection(CComAutoCriticalSection *pLock);
    ~CAutoLockCriticalSection();

private:
    ///the critical section
    CComAutoCriticalSection *m_pLock;
};

#endif // !defined(AFX_AUTOLOCKCRITICALSECTION_H__810D87A4_1887_4204_BE10_E1CC17682B54__INCLUDED_)
