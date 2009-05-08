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
 * @file DSFileSource.cpp implementation of the CDSFileSource class.
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT
#include "dscaler.h"
#include "DSFileSource.h"
#include "DShowFileSource.h"
#include "DebugLog.h"
#include "AspectRatio.h"
#include "TreeSettingsDlg.h"
#include "DSAudioDevicePage.h"
#include "OSD.h"

using namespace std;

CDSFileSource::CDSFileSource()
:CDSSourceBase(0,IDC_DSHOW_FILESOURCE_MENU)
{
    m_IDString = tstring(_T("DS_DShowFileInput"));
    CreateSettings(_T("DShowFileInput"));
}

CDSFileSource::~CDSFileSource()
{

}

void CDSFileSource::CreateSettings(LPCTSTR IniSection)
{
    CDSSourceBase::CreateSettings(IniSection);

}

BOOL CDSFileSource::HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam)
{
    if(CDSSourceBase::HandleWindowsCommands(hWnd,wParam,lParam)==TRUE)
    {
        return TRUE;
    }

    if(LOWORD(wParam)==IDM_DSHOW_SETTINGS)
    {
        CTreeSettingsDlg dlg(_T("DirectShow Settings"));
        CDSAudioDevicePage AudioDevice(_T("Audio output"),m_AudioDevice);

        dlg.AddPage(&AudioDevice);
        dlg.DoModal(hWnd);
        return TRUE;
    }
    return FALSE;
}


BOOL CDSFileSource::IsAccessAllowed()
{
    if(m_filename.size()>0)
    {
        return TRUE;
    }
    return FALSE;
}

BOOL CDSFileSource::OpenMediaFile(const tstring& FileName, BOOL NewPlayList)
{
    //The output thread MUST be stoped when calling this function
    //or the output thread will probably crash.
    //There is two problems that causes this
    //1: the delete of m_pDSGraph while output thread is running
    //2: in Stop() all fields from DSRend is marked as not in use, this must
    //be done since dsrend must release all buffered IMediaSamples or stop
    //will deadlock. So that makes all buffers used in the output thread invalid.

    if(m_pDSGraph!=NULL)
    {
        Stop();
        delete m_pDSGraph;
        m_pDSGraph=NULL;
    }

    m_filename=_T("");
    try
    {
        m_pDSGraph=new CDShowGraph(FileName,m_AudioDevice->GetValue());
        m_filename=FileName;
        m_pDSGraph->start();
        return TRUE;
    }
    catch(CDShowUnsupportedFileException& e)
    {
        LOG(1, "CDShowUnsupportedFileException - %s", e.what());
        return FALSE;
    }
    catch(CDShowException& e)
    {
        ErrorBox(e.what());
        LOG(1, "Failed to open DShow file - %s", e.what());
        return FALSE;
    }
}

void CDSFileSource::SetAspectRatioData()
{
    AspectSettings.InitialTopOverscan = 0;
    AspectSettings.InitialBottomOverscan = 0;
    AspectSettings.InitialLeftOverscan = 0;
    AspectSettings.InitialRightOverscan = 0;
    AspectSettings.bAnalogueBlanking = FALSE;
}

tstring CDSFileSource::GetStatus()
{
    if(!m_filename.empty())
    {
        tstring status=m_filename;
        tstring::size_type pos=status.rfind('\\');
        if(pos!=tstring::npos)
        {
            status=status.substr(pos+1);
        }
        return status;
    }
    return _T("No file loaded");
}


BOOL CDSFileSource::SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat)
{
    return FALSE;
}

BOOL CDSFileSource::IsInTunerMode()
{
    return FALSE;
}

eVideoFormat CDSFileSource::GetFormat()
{
    //there is no videoformat that fits
    return VIDEOFORMAT_PAL_B;
}

BOOL CDSFileSource::IsVideoPresent()
{
    return TRUE;
}

BOOL CDSFileSource::HasSquarePixels()
{
    return FALSE;
}

tstring CDSFileSource::GetMenuLabel()
{
    ///@todo remove path from filename
    return m_filename;
}

void CDSFileSource::Start()
{

    try
    {
        if(m_pDSGraph==NULL)
        {
            m_pDSGraph=new CDShowGraph(m_filename,m_AudioDevice->GetValue());
        }
        CDSSourceBase::Start();
    }
    catch(CDShowException &e)
    {
        ErrorBox(e.what());
    }
}

void CDSFileSource::HandleTimerMessages(int TimerId)
{

}

void CDSFileSource::UpdateMenu()
{

}

void CDSFileSource::SetMenu(HMENU hMenu)
{
    if(m_pDSGraph==NULL)
    {
        return;
    }
    HMENU menu = GetSubMenu(hMenu, 0);

    //set a radio checkmark in front of the current play/pause/stop menu entry
    FILTER_STATE state=m_pDSGraph->getState();
    UINT pos=2-state;
    CheckMenuRadioItem(menu, 0,2,pos,MF_BYPOSITION);
}

void CDSFileSource::Pause()
{
    if (m_pDSGraph != NULL && m_pDSGraph->getState() == State_Running)
    {
        try
        {
            m_pDSGraph->pause();
        }
        catch(CDShowException &e)
        {
            ErrorBox(MakeString() << _T("Pause failed\n\n") << e.what());
        }
        OSD_ShowText(_T("Pause"), 0);
    }
}

void CDSFileSource::UnPause()
{
    if (m_pDSGraph != NULL && m_pDSGraph->getState() == State_Paused)
    {
        try
        {
            m_pDSGraph->start();
            //VideoInputOnChange(m_VideoInput->GetValue(), m_VideoInput->GetValue());
        }
        catch(CDShowException &e)
        {
            ErrorBox(MakeString() << _T("Play failed\n\n") << e.what());
        }
        OSD_ShowText(_T("Play"), 0);
    }
}

#endif