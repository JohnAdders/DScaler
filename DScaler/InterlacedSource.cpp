/////////////////////////////////////////////////////////////////////////////
// $Id: InterlacedSource.cpp,v 1.2 2001-11-02 16:30:08 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.1.2.4  2001/08/23 16:04:57  adcockj
// Improvements to dynamic menus to remove requirement that they are not empty
//
// Revision 1.1.2.3  2001/08/21 16:42:16  adcockj
// Per format/input settings and ini file fixes
//
// Revision 1.1.2.2  2001/08/15 14:44:05  adcockj
// Starting to put some flesh onto the new structure
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InterlacedSource.h"
#include "DScaler.h"

CInterlacedSource::CInterlacedSource(long SetMessage, long MenuId) :
    CSettingsHolder(SetMessage),
    m_FieldFrequency(0)
{
    m_hMenu = LoadMenu(hInst, MAKEINTRESOURCE(MenuId));
}

CInterlacedSource::~CInterlacedSource()
{
    DestroyMenu(m_hMenu);
}

double CInterlacedSource::GetFieldFrequency()
{
    return m_FieldFrequency;
}

HMENU CInterlacedSource::GetMenu()
{
    return m_hMenu;

}
