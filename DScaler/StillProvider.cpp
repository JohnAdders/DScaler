/////////////////////////////////////////////////////////////////////////////
// $Id: StillProvider.cpp,v 1.3 2001-11-21 12:32:11 adcockj Exp $
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
// Revision 1.2  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.1.2.2  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.1.2.1  2001/08/15 14:44:05  adcockj
// Starting to put some flesh onto the new structure
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StillProvider.h"


CStillProvider::CStillProvider()
{
    m_StillSource = new CStillSource();
}

CStillProvider::~CStillProvider()
{
    delete m_StillSource;
}

int CStillProvider::GetNumberOfSources()
{
    return 1;
}


CSource* CStillProvider::GetSource(int SourceIndex)
{
    if(SourceIndex == 0)
    {
        return m_StillSource;
    }
    else
    {
        return NULL;
    }
}
