/////////////////////////////////////////////////////////////////////////////
// $Id: Source.cpp,v 1.8 2002-08-27 22:02:32 kooiman Exp $
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
// Revision 1.7  2002/05/06 15:38:50  laurentg
// Comments for a source
//
// Revision 1.6  2002/04/15 22:50:09  laurentg
// Change again the available formats for still saving
// Automatic switch to "square pixels" AR mode when needed
//
// Revision 1.5  2002/02/22 23:22:23  laurentg
// Only notify size change if it concerns current source
//
// Revision 1.4  2002/02/19 16:03:36  tobbej
// removed CurrentX and CurrentY
// added new member in CSource, NotifySizeChange
//
// Revision 1.3  2001/11/25 21:19:40  laurentg
// New method GetMenuLabel and method GetMenu renamed GetSourceMenu
//
// Revision 1.2  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.1  2001/11/21 12:32:11  adcockj
// Renamed CInterlacedSource to CSource in preparation for changes to DEINTERLACE_INFO
//
// Revision 1.3  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.2  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
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
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "Source.h"
#include "DScaler.h"
#include "Providers.h"


CSource::CSource(long SetMessage, long MenuId) :
    CSettingsHolder(SetMessage),
    m_FieldFrequency(0),
    m_Comments("")
{
    m_hMenu = LoadMenu(hResourceInst, MAKEINTRESOURCE(MenuId));
}

CSource::~CSource()
{
    DestroyMenu(m_hMenu);
}

double CSource::GetFieldFrequency()
{
    return m_FieldFrequency;
}

HMENU CSource::GetSourceMenu()
{
    return m_hMenu;

}

char* CSource::GetComments()
{
    return (char*)m_Comments.c_str();
}

void CSource::NotifySizeChange()
{
    if (Providers_GetCurrentSource() == this)
    {
    	//tell dscaler that size has changed, the real work will be done in the main message loop
	    PostMessage(hWnd,UWM_INPUTSIZE_CHANGE,0,0);
    }
}

void CSource::NotifySquarePixelsCheck()
{
    if (Providers_GetCurrentSource() == this)
    {
    	// Tell dscaler to check whether "square pixels" AR mode must be on or off
        // The real work will be done in the main message loop
	    PostMessage(hWnd,UWM_SQUAREPIXELS_CHECK,0,0);
    }
}


void CSource::NotifyInputChange(int Prechange, eSourceInputType InputType, int OldValue, int NewValue)
{
    for(vector<TInputChangeNotification>::iterator it = m_InputChangeNotificationList.begin();
                it != m_InputChangeNotificationList.end(); ++it)
     {
        if ( (*it).pfnNotify != NULL )
        {
            (*it).pfnNotify((*it).pThis, Prechange, InputType, OldValue, NewValue);
        }     
    }    
}

void CSource::Register_InputChangeNotification(void *pThis, INPUTCHANGE_NOTIFICATION *pfnChange)
{
    for(vector<TInputChangeNotification>::iterator it = m_InputChangeNotificationList.begin();
                it != m_InputChangeNotificationList.end(); ++it)
    {
        if ( ((*it).pThis == pThis) && ((*it).pfnNotify == pfnChange) )
        {
            return;
        }
    }
    // add new
    TInputChangeNotification ccn;
    ccn.pThis = pThis;
    ccn.pfnNotify = pfnChange;
    m_InputChangeNotificationList.push_back(ccn);
}





void CSource::Unregister_InputChangeNotification(void *pThis, INPUTCHANGE_NOTIFICATION *pfnChange)
{
    std::vector<TInputChangeNotification> NewList;
    for(vector<TInputChangeNotification>::iterator it = m_InputChangeNotificationList.begin();
                it != m_InputChangeNotificationList.end(); ++it)
    {
        if ( ((*it).pThis == pThis) && ((*it).pfnNotify == pfnChange) )
        {
            // don't copy
        }
        else
        {
            NewList.push_back((*it));
        }
    }
    m_InputChangeNotificationList = NewList;
}

