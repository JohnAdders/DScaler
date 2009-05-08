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
 * @file DSSourceBase.cpp implementation of the CDSSourceBase class.
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT

#include "dscaler.h"
#include "..\DScalerRes\resource.h"
#include "DSSourceBase.h"
#include "AutoCriticalSection.h"
#include "FieldTiming.h"
#include "DebugLog.h"
#include "OSD.h"
#include "OutThreads.h"
#include "TreeSettingsDlg.h"

using namespace std;

CDSSourceBase::CDSSourceBase(long SetMessage, long MenuId) :
    CSource(SetMessage,MenuId),
    m_pDSGraph(NULL),
    m_CurrentX(0),
    m_CurrentY(0),
    m_LastNumDroppedFrames(-1),
    m_dwRendStartTime(0),
    m_InitialSetup(FALSE)
{
    InitializeCriticalSection(&m_hOutThreadSync);
}

CDSSourceBase::~CDSSourceBase()
{
    {
        CAutoCriticalSection lock(m_hOutThreadSync);
        if(m_pDSGraph!=NULL)
        {
            delete m_pDSGraph;
            m_pDSGraph=NULL;
        }
    }
    DeleteCriticalSection(&m_hOutThreadSync);
    m_AudioDevice->WriteToIni(TRUE);
}

void CDSSourceBase::CreateSettings(LPCTSTR IniSection)
{
    m_Volume = new CVolumeSetting(this, _T("Volume"), 0, LONG_MIN, LONG_MAX, IniSection);
    m_Settings.push_back(m_Volume);

    m_Balance = new CBalanceSetting(this, _T("Balance"), 0, LONG_MIN, LONG_MAX, IniSection);
    m_Settings.push_back(m_Balance);

    m_AudioDevice = new CStringSetting(_T("Audio Device"), _T(""), IniSection, _T("AudioDevice"));
}

int CDSSourceBase::GetWidth()
{
    return m_CurrentX;
}

int CDSSourceBase::GetHeight()
{
    return m_CurrentY;
}

void CDSSourceBase::SetWidth(int w)
{
    // todo: set the width
}

void CDSSourceBase::Start()
{
    m_LastNumDroppedFrames=-1;
    m_CurrentX=0;
    m_CurrentY=0;
    try
    {
        //derived class must create the graph first
        _ASSERTE(m_pDSGraph!=NULL);

        m_pDSGraph->start();
    }
    catch(CDShowException &e)
    {
        ErrorBox(e.what());
    }
    NotifySquarePixelsCheck();
}

void CDSSourceBase::Stop()
{
    CAutoCriticalSection lock(m_hOutThreadSync);
    if(m_pDSGraph!=NULL)
    {
        try
        {
            m_pDSGraph->stop();
        }
        catch(CDShowException &e)
        {
            ErrorBox(e.what());
        }
    }
}

void CDSSourceBase::Reset()
{
    Stop();
    Start();
}

void CDSSourceBase::StopAndSeekToBeginning()
{
    CAutoCriticalSection lock(m_hOutThreadSync);
    if(m_pDSGraph!=NULL)
    {
        try
        {
            //we must ensure that the output thread don't try to get any
            //more fields. if it does a deadlock can occure.
            //the deadlock will only occure if the output thread is
            //blocked when it accesses the main threads gui,
            //since this prevents IDSRendFilter::FreeFields from being called
            m_pDSGraph->stop();

            //i don't know if this is the right place to put this
            //but it looks like its working
            CDShowSeeking *pSeeking=m_pDSGraph->GetSeeking();
            if(pSeeking!=NULL)
            {
                //here is a sample on how to get the positions
                //sometimes AM_SEEKING_CanGetCurrentPos is not set but
                //IMediaSeeking::GetCurrentPosition still works.
                if(pSeeking->GetCaps()&AM_SEEKING_CanGetDuration)
                {
                    LONGLONG pos=pSeeking->GetCurrentPos();
                    LONGLONG duration=pSeeking->GetDuration();
                    double pos1=pos/(double)10000000;
                    double duration1=duration/(double)10000000;
                    LOGD(_T("Current media position in seconds: %f / %f\n"),pos1,duration1);
                }

                //atleast one of those flags must be set to be able to seek to the begining
                if(pSeeking->GetCaps()&(AM_SEEKING_CanSeekBackwards|AM_SEEKING_CanSeekAbsolute))
                {
                    //seek to the begining
                    pSeeking->SeekTo(0);
                }
            }
        }
        catch(CDShowException &e)
        {
            ErrorBox(MakeString() << _T("Stop failed\n\n")  << e.what());
        }
    }
}

void CDSSourceBase::GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming)
{
    CAutoCriticalSection lock(m_hOutThreadSync);
    if(m_pDSGraph==NULL)
    {
        return;
    }
    DWORD dwLastDelay=0;
    if(m_dwRendStartTime!=0)
    {
        dwLastDelay=timeGetTime()-m_dwRendStartTime;
        //LOGD(_T("Processing delay: %ld(ms)\n"),dwLastDelay);
        m_dwRendStartTime=0;
    }
    //info to return if we fail
    pInfo->bRunningLate=TRUE;
    pInfo->bMissedFrame=TRUE;
    pInfo->FrameWidth=0;
    pInfo->FrameHeight=0;

    //clear the picture history
    memset(pInfo->PictureHistory, 0, MAX_PICTURE_HISTORY * sizeof(TPicture*));
    //ClearPictureHistory(pInfo);

    //is the graph running? there is no point in continuing if it isnt
    /*if(m_pDSGraph->getState()!=State_Running)
    {
        return;
    }*/

    long size=MAX_PICTURE_HISTORY;
    FieldBuffer fields[MAX_PICTURE_HISTORY];
    BufferInfo binfo;
    if(!m_pDSGraph->GetFields(&size,fields,binfo,dwLastDelay))
    {
        m_dwRendStartTime=0;
        UpdateDroppedFields();
        return;
    }

    //width must be 16 byte aligned or optimized memcpy will not work
    //this assert will never be triggered (checked in dsrend filter)
    _ASSERTE((binfo.Width&0xf)==0);

    //check if size has changed
    if(m_CurrentX!=binfo.Width || m_CurrentY!=binfo.Height*2)
    {
        m_CurrentX=binfo.Width;
        m_CurrentY=binfo.Height*2;
        NotifySizeChange();
    }

    pInfo->FrameWidth=binfo.Width;
    pInfo->FrameHeight=binfo.Height*2;
    pInfo->LineLength=binfo.Width*2;
    pInfo->FieldHeight=binfo.Height;
    pInfo->InputPitch=pInfo->LineLength;
    pInfo->bMissedFrame=FALSE;
    pInfo->bRunningLate=FALSE;
    pInfo->CurrentFrame=binfo.CurrentFrame;

    UpdateDroppedFields();

    static BOOL FieldFlag=TRUE;
    for(int i=0;i<size;i++)
    {
        m_PictureHistory[i].pData=fields[i].pBuffer;
        m_PictureHistory[i].IsFirstInSeries=FALSE;
        if(fields[i].flags!=BUFFER_FLAGS_FIELD_UNKNOWN)
        {
            m_PictureHistory[i].Flags=fields[i].flags;
        }
        else
        {
            m_PictureHistory[i].Flags= FieldFlag==TRUE ? PICTURE_INTERLACED_EVEN : PICTURE_INTERLACED_ODD;
            FieldFlag=!FieldFlag;
        }

        pInfo->PictureHistory[i] = &m_PictureHistory[i];
    }
    Timing_IncrementUsedFields();
    m_dwRendStartTime=timeGetTime();
}

void CDSSourceBase::UpdateDroppedFields()
{
    if(m_pDSGraph==NULL)
        return;

    int dropped;
    try
    {
        dropped=m_pDSGraph->getDroppedFrames();
    }
    catch(CDShowException& e)
    {
        LOG(1, "DShow Exception - %s", e.what());
        return;
    }

    //is the m_lastNumDroppedFrames count valid?
    if(m_LastNumDroppedFrames!=-1)
    {
        if(dropped-m_LastNumDroppedFrames >0)
        {
            Timing_AddDroppedFields((dropped-m_LastNumDroppedFrames)*2);
        }
    }
    m_LastNumDroppedFrames=dropped;
}

BOOL CDSSourceBase::HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam)
{
    FILTER_STATE OldState;

    if(m_pDSGraph==NULL)
    {
        return FALSE;
    }

    switch(LOWORD(wParam))
    {
    case IDM_DSHOW_PLAY:
        if (m_pDSGraph->getState() == State_Paused)
        {
            SendMessage(hWnd, WM_COMMAND, IDM_CAPTURE_PAUSE, 0);
        }
        else if (m_pDSGraph->getState() == State_Stopped)
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
        return TRUE;
        break;

    case IDM_DSHOW_PAUSE:
        if (m_pDSGraph->getState() == State_Running)
        {
            SendMessage(hWnd, WM_COMMAND, IDM_CAPTURE_PAUSE, 0);
        }
        return TRUE;
        break;

    case IDM_DSHOW_STOP:
        OldState = m_pDSGraph->getState();
        TGUIRequest req;
        req.type = REQ_DSHOW_STOP;
        PutRequest(&req);
        if (OldState == State_Paused)
        {
            SendMessage(hWnd, WM_COMMAND, IDM_CAPTURE_PAUSE, 0);
        }
        OSD_ShowText(_T("Stop"), 0);
        return TRUE;
        break;
    case IDM_DSHOW_BACKWARD_5S:
        ChangePos(-5);
        return TRUE;
        break;
    case IDM_DSHOW_BACKWARD_10S:
        ChangePos(-10);
        return TRUE;
        break;
    case IDM_DSHOW_BACKWARD_30S:
        ChangePos(-30);
        return TRUE;
        break;
    case IDM_DSHOW_BACKWARD_300S:
        ChangePos(-300);
        return TRUE;
        break;
    case IDM_DSHOW_BACKWARD_600S:
        ChangePos(-600);
        return TRUE;
        break;
    case IDM_DSHOW_BACKWARD_1800S:
        ChangePos(-1800);
        return TRUE;
        break;
    case IDM_DSHOW_FORWARD_5S:
        ChangePos(5);
        return TRUE;
        break;
    case IDM_DSHOW_FORWARD_10S:
        ChangePos(10);
        return TRUE;
        break;
    case IDM_DSHOW_FORWARD_30S:
        ChangePos(30);
        return TRUE;
        break;
    case IDM_DSHOW_FORWARD_300S:
        ChangePos(300);
        return TRUE;
        break;
    case IDM_DSHOW_FORWARD_600S:
        ChangePos(600);
        return TRUE;
        break;
    case IDM_DSHOW_FORWARD_1800S:
        ChangePos(1800);
        return TRUE;
        break;
    case IDM_DSHOW_FILTERS:
        {
            CTreeSettingsDlg dlg(_T("Filter properties"));
            CTreeSettingsPage rootPage(_T("Filters"),IDD_TREESETTINGS_EMPTY);
            int root=dlg.AddPage(&rootPage);

            int filterIndex=0;
            CComPtr<ISpecifyPropertyPages> SpecifyPages;
            CTreeSettingsPage *pPage=NULL;

            while(m_pDSGraph->getFilterPropertyPage(filterIndex, SpecifyPages))
            {
                int filterRoot=dlg.AddPages(SpecifyPages, root);

                int subIndex=0;
                CComPtr<ISpecifyPropertyPages> SpecifyPages2;
                while(m_pDSGraph->getFilterSubPage(filterIndex,subIndex, SpecifyPages2))
                {
                    dlg.AddPages(SpecifyPages2, filterRoot);
                    subIndex++;
                }

                filterIndex++;
            }
            if(filterIndex!=0)
            {
                //show the dialog
                dlg.DoModal(hWnd);
            }
            else
            {
                ErrorBox(_T("There is no filters to show properties for"));
            }

            return TRUE;
            break;
        }
    }
    return FALSE;
}

void CDSSourceBase::Mute()
{
    if(m_pDSGraph==NULL)
    {
        return;
    }
    CDShowAudioControls *pControls=m_pDSGraph->GetAudioControls();
    if(pControls==NULL)
    {
        LOG(3,_T("Failed to get audio controlls, mute will not work"));
        return;
    }

    if(pControls->GetAudioCaps()|DSHOW_AUDIOCAPS_HAS_VOLUME)
    {
        long min=0;
        long max=0;
        pControls->GetVolumeMinMax(min,max);

        try
        {
            pControls->SetVolume(min);
            EventCollector->RaiseEvent(this,EVENT_MUTE,0,1);
        }
        catch(CDShowException& e)
        {
            LOG(3,"Exception in Mute, Error: %s",e.what());
        }
    }
}

void CDSSourceBase::UnMute()
{
    if(m_pDSGraph==NULL)
    {
        return;
    }
    CDShowAudioControls *pControls=m_pDSGraph->GetAudioControls();
    if(pControls==NULL)
    {
        LOG(3,_T("Failed to get audio controlls, mute will not work"));
        return;
    }

    if(pControls->GetAudioCaps()|DSHOW_AUDIOCAPS_HAS_VOLUME)
    {
        long volume=m_Volume->GetValue();
        try
        {
            pControls->SetVolume(volume);
            EventCollector->RaiseEvent(this,EVENT_MUTE,1,0);
        }
        catch(CDShowException& e)
        {
            LOG(3,"Exception in UnMute, Error: %s",e.what());
        }
    }
}

CSliderSetting* CDSSourceBase::GetVolume()
{
    if(m_pDSGraph==NULL)
    {
        return NULL;
    }
    CDShowAudioControls *pControls=m_pDSGraph->GetAudioControls();
    if(pControls==NULL)
    {
        LOG(3,_T("Failed to get audio controlls, volume will not work"));
        return NULL;
    }

    if(pControls->GetAudioCaps()|DSHOW_AUDIOCAPS_HAS_VOLUME)
    {
        long min=0;
        long max=0;
        pControls->GetVolumeMinMax(min,max);
        m_Volume->SetMax(max);
        m_Volume->SetMin(min);
        return m_Volume;
    }
    else
    {
        LOG(3,_T("Audio controls don't support volume"));
        return NULL;
    }
}

CSliderSetting* CDSSourceBase::GetBalance()
{
    if(m_pDSGraph==NULL)
    {
        return NULL;
    }
    CDShowAudioControls *pControls=m_pDSGraph->GetAudioControls();
    if(pControls==NULL)
    {
        LOG(3,_T("Failed to get audio controlls, balance will not work"));
        return NULL;
    }

    if(pControls->GetAudioCaps()|DSHOW_AUDIOCAPS_HAS_BALANCE)
    {
        long min=0;
        long max=0;
        pControls->GetBalanceMinMax(min,max);
        m_Balance->SetMax(max);
        m_Balance->SetMin(min);
        return m_Balance;
    }
    else
    {
        LOG(3,_T("Audio controls don't support balance"));
        return NULL;
    }
}

void CDSSourceBase::VolumeOnChange(long NewValue, long OldValue)
{
    if(m_pDSGraph==NULL)
    {
        LOG(3,_T("Can't change volume because there is no filter graph"));
        return;
    }
    CDShowAudioControls *pControlls=m_pDSGraph->GetAudioControls();
    if(pControlls==NULL)
    {
        LOG(3,_T("Failed to get audio controlls"));
        return;
    }

    try
    {
        pControlls->SetVolume(NewValue);
        EventCollector->RaiseEvent(this, EVENT_VOLUME, OldValue, NewValue);
    }
    catch(CDShowException& e)
    {
        LOG(3,"Exception in VolumeOnChange, Error: %s",e.what());
        m_Volume->SetValue(OldValue);
        EventCollector->RaiseEvent(this, EVENT_VOLUME, OldValue, OldValue);
    }
}

void CDSSourceBase::BalanceOnChange(long NewValue, long OldValue)
{
    if(m_pDSGraph==NULL)
    {
        LOG(3,_T("Can't change balance because there is no filter graph"));
        return;
    }
    CDShowAudioControls *pControlls=m_pDSGraph->GetAudioControls();
    if(pControlls==NULL)
    {
        LOG(3,_T("Failed to get audio controlls"));
        return;
    }

    try
    {
        pControlls->SetBalance(NewValue);
    }
    catch(CDShowException& e)
    {
        LOG(3,"Exception in BalanceOnChange, Error: %s",e.what());
        m_Balance->SetValue(OldValue);
    }
}

tstring CDSSourceBase::IDString()
{
    return m_IDString;
}

int CDSSourceBase::GetCurrentPos()
{
    CDShowSeeking *pSeeking=m_pDSGraph->GetSeeking();
    if(pSeeking!=NULL)
    {
        if(pSeeking->GetCaps()&AM_SEEKING_CanGetDuration)
        {
            return (int)(pSeeking->GetCurrentPos() / 1000000);
        }
    }
    return -1;
}

void CDSSourceBase::SetPos(int pos)
{
    CDShowSeeking *pSeeking=m_pDSGraph->GetSeeking();
    if(pSeeking!=NULL)
    {
        if(pSeeking->GetCaps()&AM_SEEKING_CanSeekAbsolute)
        {
            LONGLONG RealPos = (LONGLONG)pos * 1000000;
            pSeeking->SeekTo(RealPos);

            int pos1 = (int)(RealPos / 10000000);
            TCHAR text[32];
            _stprintf_s(text, 32, _T("Jump to time %d:%2.2d"), pos1 / 60, pos1 % 60);
            OSD_ShowText(text, 0);
        }
    }
}

int CDSSourceBase::GetDuration()
{
    CDShowSeeking *pSeeking=m_pDSGraph->GetSeeking();
    if(pSeeking!=NULL)
    {
        if(pSeeking->GetCaps()&AM_SEEKING_CanGetDuration)
        {
            return (int)(pSeeking->GetDuration() / 1000000);
        }
    }
    return -1;
}

void CDSSourceBase::ChangePos(int delta_sec)
{
    CDShowSeeking *pSeeking=m_pDSGraph->GetSeeking();
    if(pSeeking!=NULL)
    {
        if(pSeeking->GetCaps()&AM_SEEKING_CanGetDuration)
        {
            LONGLONG pos=pSeeking->GetCurrentPos();
            LONGLONG duration=pSeeking->GetDuration();
            LONGLONG newpos = pos + 10000000 * (LONGLONG)delta_sec;
            if (newpos < 0)
            {
                newpos = 0;
            }
            else if (newpos > duration)
            {
                newpos = duration;
            }
            if ( ( (delta_sec < 0) && (pSeeking->GetCaps()&(AM_SEEKING_CanSeekBackwards|AM_SEEKING_CanSeekAbsolute)) )
              || ( (delta_sec > 0) && (pSeeking->GetCaps()&(AM_SEEKING_CanSeekForwards|AM_SEEKING_CanSeekAbsolute)) ) )
            {
                pSeeking->SeekTo(newpos);

                int pos1 = (int)(newpos / 10000000);
                TCHAR text[32];
                _stprintf_s(text, 32, _T("Jump to time %d:%2.2d"), pos1 / 60, pos1 % 60);
                OSD_ShowText(text, 0);
            }
        }
    }
}

void CDSSourceBase::SetSourceAsCurrent()
{
    if (GetVolume() != NULL)
    {
        EventCollector->RaiseEvent(this, EVENT_VOLUME, 0, GetVolume()->GetValue());
    }
}

#endif