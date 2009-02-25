/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
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
// Revision 1.3  2007/02/18 15:02:16  robmuller
// Added CVS log.
//
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ioutput.h"
#include "DebugLog.h"
#include "OverlayOutput.h"
#include "D3D9Output.h"

IOutput *ActiveOutput=NULL;
extern COverlayOutput OverlayOutputInstance;
extern CD3D9Output D3D9OutputInstance;
   
	

IOutput::IOutput(void)
{
}

IOutput::~IOutput(void)
{
}

//-----------------------------------------------------------------------------
void IOutput::GetMonitorRect(HWND hWnd, RECT* rect)
{
    /*
	drop NT4.0 compatibility .. anybody still using it???
	if(lpMonitorFromWindow == NULL)
    {
        rect->top = 0;
        rect->left = 0;
        rect->bottom = GetSystemMetrics(SM_CYSCREEN);
        rect->right = GetSystemMetrics(SM_CXSCREEN);
        return;
    }*/

	HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO MonInfo;

	MonInfo.cbSize = sizeof(MONITORINFO);

	GetMonitorInfo(hMonitor, &MonInfo);
	memcpy(rect, &MonInfo.rcMonitor, sizeof(RECT));
	LOG(2, "GetMonitorRect %d %d %d %d", rect->left, rect->right, rect->top, rect->bottom);
}

SETTING* IOutput::GetOtherSettings()
{   
    if(!m_bSettingInitialized)
    {        
        InitOtherSettings();
        m_bSettingInitialized=true;
    }
    return OtherSettings;
}


IOutput* GetActiveOutput()
{
    return ActiveOutput;
}

void SetActiveOutput(IOutput::OUTPUTTYPES eType)
{
    if(eType == IOutput::OUT_OVERLAY)
    {
        ActiveOutput = &OverlayOutputInstance;
    }
    else
    {
        ActiveOutput = &D3D9OutputInstance;
    }
}
