/////////////////////////////////////////////////////////////////////////////
// $Id: IOutput.cpp,v 1.3 2007-02-18 15:02:16 robmuller Exp $
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
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ioutput.h"
#include "DebugLog.h"
#include "OverlayOutput.h"

IOutput *ActiveOutput=NULL;


    
	

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

SETTING* Overlay_GetSetting(OTHER_SETTING Setting)
{
    if(Setting > -1 && Setting < OTHER_SETTING_LASTONE)
    {
        return &(ConfigOutput.GetOtherSettings()[Setting]);
    }
    else
    {
        return NULL;
    }
}

CSettingsHolderStandAlone OverlaySettingsHolder;

void Overlay_ReadSettingsFromIni()
{
    if(OverlaySettingsHolder.GetNumSettings() == 0)
    {
        CSettingGroup *pOverlayGroup = OverlaySettingsHolder.GetSettingsGroup("Overlay", SETTING_BY_CHANNEL | SETTING_BY_FORMAT | SETTING_BY_INPUT, FALSE);

        OverlaySettingsHolder.AddSetting(&ConfigOutput.GetOtherSettings()[OVERLAYBRIGHTNESS], pOverlayGroup);
        OverlaySettingsHolder.AddSetting(&ConfigOutput.GetOtherSettings()[OVERLAYCONTRAST], pOverlayGroup);
        OverlaySettingsHolder.AddSetting(&ConfigOutput.GetOtherSettings()[OVERLAYHUE], pOverlayGroup);
        OverlaySettingsHolder.AddSetting(&ConfigOutput.GetOtherSettings()[OVERLAYSATURATION], pOverlayGroup);
        OverlaySettingsHolder.AddSetting(&ConfigOutput.GetOtherSettings()[OVERLAYGAMMA], pOverlayGroup);
        OverlaySettingsHolder.AddSetting(&ConfigOutput.GetOtherSettings()[OVERLAYSHARPNESS], pOverlayGroup);

        OverlaySettingsHolder.AddSetting(&ConfigOutput.GetOtherSettings()[BACKBUFFERS]);
        OverlaySettingsHolder.AddSetting(&ConfigOutput.GetOtherSettings()[OVERLAYCOLOR]);
        OverlaySettingsHolder.AddSetting(&ConfigOutput.GetOtherSettings()[USEOVERLAYCONTROLS]);
        OverlaySettingsHolder.AddSetting(&ConfigOutput.GetOtherSettings()[ALLOWBOBMODE]);
#ifdef _DEBUG
        if (OTHER_SETTING_LASTONE != OverlaySettingsHolder.GetNumSettings())
        {
            LOGD("Number of settings in Overlay source is not equal to the number of settings in DS_Control.h");
            LOGD("DS_Control.h or Other.cpp are probably not in sync with each other.");
        }
#endif
    }
    OverlaySettingsHolder.DisableOnChange();
    OverlaySettingsHolder.ReadFromIni();
    OverlaySettingsHolder.EnableOnChange();
}

void Overlay_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    OverlaySettingsHolder.WriteToIni(bOptimizeFileAccess);
}
