/////////////////////////////////////////////////////////////////////////////
// $Id: StillSource.cpp,v 1.12 2001-11-28 16:04:50 adcockj Exp $
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
// Revision 1.11  2001/11/26 13:02:27  adcockj
// Bug Fixes and standards changes
//
// Revision 1.10  2001/11/25 21:29:50  laurentg
// Take still, Open file, Close file callbacks updated
//
// Revision 1.9  2001/11/25 10:41:26  laurentg
// TIFF code moved from Other.cpp to TiffSource.c + still capture updated
//
// Revision 1.8  2001/11/24 22:51:20  laurentg
// Bug fixes regarding still source
//
// Revision 1.7  2001/11/24 17:58:06  laurentg
// Still source
//
// Revision 1.6  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.5  2001/11/21 15:21:39  adcockj
// Renamed DEINTERLACE_INFO to TDeinterlaceInfo in line with standards
// Changed TDeinterlaceInfo structure to have history of pictures.
//
// Revision 1.4  2001/11/21 12:32:11  adcockj
// Renamed CInterlacedSource to CSource in preparation for changes to DEINTERLACE_INFO
//
// Revision 1.3  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.2  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.1.2.6  2001/08/23 16:04:57  adcockj
// Improvements to dynamic menus to remove requirement that they are not empty
//
// Revision 1.1.2.5  2001/08/21 16:42:16  adcockj
// Per format/input settings and ini file fixes
//
// Revision 1.1.2.4  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.1.2.3  2001/08/18 17:09:30  adcockj
// Got to compile, still lots to do...
//
// Revision 1.1.2.2  2001/08/17 16:35:14  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
// Revision 1.1.2.1  2001/08/15 14:44:05  adcockj
// Starting to put some flesh onto the new structure
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "StillSource.h"
#include "StillProvider.h"
#include "DScaler.h"
#include "FieldTiming.h"
#include "DebugLog.h"
#include "Providers.h"
#include "TiffHelper.h"
#include "OutThreads.h"

CStillSourceHelper::CStillSourceHelper(CStillSource* pParent)
{
    m_pParent = pParent;
}


CPlayListItem::CPlayListItem(LPCSTR FileName, int SecondsToDisplay) :
    m_FileName(FileName),
    m_SecondsToDisplay(SecondsToDisplay)
{
}

LPCSTR CPlayListItem::GetFileName()
{
    return m_FileName.c_str();
}

int CPlayListItem::GetSecondsToDisplay()
{
    return m_SecondsToDisplay;
}

CStillSource::CStillSource() :
    CSource(0, IDC_STILL)
{
    CreateSettings("StillSource");
    m_Width = 0;
    m_Height = 0;
    m_StillFrame.pData = NULL;
    m_StillFrame.Flags = PICTURE_PROGRESSIVE;
    m_StillFrame.IsFirstInSeries = FALSE;
    m_OriginalFrame.pData = NULL;
    m_OriginalFrame.Flags = PICTURE_PROGRESSIVE;
    m_OriginalFrame.IsFirstInSeries = FALSE;
    m_FieldFrequency = 50.0;
    m_FrameDuration = 1000.0 / m_FieldFrequency;
    m_IsPictureRead = FALSE;
    m_Position = -1;
}

CStillSource::~CStillSource()
{
    if (m_StillFrame.pData != NULL)
    {
        free(m_StillFrame.pData);
    }
    if (m_OriginalFrame.pData != NULL)
    {
        free(m_OriginalFrame.pData);
    }
    ClearPlayList();
}

BOOL CStillSource::OpenPictureFile(LPCSTR FileName)
{
    if(strlen(FileName) > 4 && stricmp(FileName + strlen(FileName) - 4, ".tif") == 0 ||
        strlen(FileName) > 5 && stricmp(FileName + strlen(FileName) - 5, ".tiff") == 0)
    {
        CTiffHelper TiffHelper(this);
        return TiffHelper.OpenMediaFile(FileName);
    }
    else
    {
        return FALSE;
    }
}


BOOL CStillSource::OpenMediaFile(LPCSTR FileName)
{

    ClearPlayList();
    m_Position = 0;

    // test for the correct extension and work out the 
    // correct helper for the file type
    if(strlen(FileName) > 4 && stricmp(FileName + strlen(FileName) - 4, ".d3u") == 0)
    {
        char Buffer[512];
        FILE* Playlist = fopen(FileName, "r");
        if(Playlist != NULL)
        {
            while(!feof(Playlist))
            {
                if(fgets(Buffer, 512, Playlist))
                {
                    Buffer[511] = '\0';
                    if(Buffer[0] != '#' && Buffer[0] != ';')
                    {
                        // take care of stuff that is at end of the line
                        while(strlen(Buffer) > 0 && Buffer[strlen(Buffer) - 1] <= ' ')
                        {
                            Buffer[strlen(Buffer) - 1] = '\0';
                        }
                        CPlayListItem* Item = new CPlayListItem(Buffer, 10);
                        m_PlayList.push_back(Item);
                    }
                }
            }
            fclose(Playlist);
        }
    }
    else
    {
        CPlayListItem* Item = new CPlayListItem(FileName, 10);
        m_PlayList.push_back(Item);
    }

    return ShowNextInPlayList();
}

BOOL CStillSource::ShowNextInPlayList()
{
    while(m_Position < m_PlayList.size())
    {
        if(OpenPictureFile(m_PlayList[m_Position]->GetFileName()))
        {
            return TRUE;
        }
        ++m_Position;
    }
    return FALSE;
}

BOOL CStillSource::ShowPreviousInPlayList()
{
    while(m_Position >= 0)
    {
        if(OpenPictureFile(m_PlayList[m_Position]->GetFileName()))
        {
            return TRUE;
        }
        --m_Position;
    }
    return FALSE;
}



void CStillSource::SaveSnapshot(LPCSTR FilePath, int FrameHeight, int FrameWidth, BYTE* pOverlay, LONG OverlayPitch)
{
    CTiffHelper TiffHelper(this);
    TiffHelper.SaveSnapshot(FilePath, FrameHeight, FrameWidth, pOverlay, OverlayPitch);
}

void CStillSource::CreateSettings(LPCSTR IniSection)
{
}

void CStillSource::Start()
{
    m_LastTickCount = 0;
}

void CStillSource::Stop()
{
    if (m_StillFrame.pData != NULL)
    {
        free(m_StillFrame.pData);
        m_StillFrame.pData = NULL;
    }
}

void CStillSource::Reset()
{
}

void CStillSource::GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming)
{
    BOOL bSlept = FALSE;
    DWORD CurrentTickCount;
    int Diff;
    
    if(!ReadNextFrameInFile())
    {
        pInfo->bRunningLate = TRUE;
        pInfo->bMissedFrame = TRUE;
        pInfo->FrameWidth = 10;
        pInfo->FrameHeight = 10;
        return;
    }

    CurrentTickCount = GetTickCount();
    if (m_LastTickCount == 0)
    {
        m_LastTickCount = CurrentTickCount;
    }
    Diff = (int)((double)(CurrentTickCount - m_LastTickCount) / m_FrameDuration);
    while(Diff < 1)
    {
        Timing_SmartSleep(pInfo, FALSE, bSlept);
        pInfo->bRunningLate = FALSE;            // if we waited then we are not late
        CurrentTickCount = GetTickCount();
        Diff = (int)((double)(CurrentTickCount - m_LastTickCount) / m_FrameDuration);
    }
    m_LastTickCount += (int)floor((double)Diff * m_FrameDuration + 0.5);
    if(Diff > 1)
    {
        // delete all history
        memset(pInfo->PictureHistory, 0, MAX_PICTURE_HISTORY * sizeof(TPicture*));
        pInfo->bMissedFrame = TRUE;
        Timing_AddDroppedFields(Diff - 1);
        LOG(2, " Dropped Frame");
    }
    else
    {
        pInfo->bMissedFrame = FALSE;
        if (pInfo->bRunningLate)
        {
            Timing_AddDroppedFields(1);
            LOG(2, "Running Late");
        }
    }

    pInfo->LineLength = m_Width * 2;
    pInfo->FrameWidth = m_Width;
    pInfo->FrameHeight = m_Height;
    pInfo->FieldHeight = m_Height;
    pInfo->InputPitch = pInfo->LineLength;
    pInfo->PictureHistory[0] = &m_StillFrame;
}

BOOL CStillSource::HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam)
{
    switch(LOWORD(wParam))
    {
    case IDM_PLAYLIST_PREVIOUS:
        if(m_Position > 0)
        {
            --m_Position;
            Stop_Capture();
            ShowPreviousInPlayList();
            Start_Capture();
        }
        return TRUE;
        break;
    case IDM_PLAYLIST_NEXT:
        if(m_Position < m_PlayList.size() - 1)
        {
            ++m_Position;
            Stop_Capture();
            ShowNextInPlayList();
            Start_Capture();
        }
        return TRUE;
        break;
    }
    return FALSE;
}

BOOL CStillSource::ReadNextFrameInFile()
{
    if (m_IsPictureRead)
    {
        if(m_StillFrame.pData != NULL)
        {
            free(m_StillFrame.pData);
        }
        m_StillFrame.pData = (BYTE*)malloc(m_Width * 2 * m_Height * sizeof(BYTE));
        if (m_StillFrame.pData != NULL)
        {
            memcpy(m_StillFrame.pData, m_OriginalFrame.pData, m_Width * 2 * m_Height * sizeof(BYTE));;
            return TRUE;
        }
        return FALSE;
    }
    else
    {
        return FALSE;
    }
}


LPCSTR CStillSource::GetStatus()
{
    if(m_Position != -1 && m_PlayList.size() > 0)
    {
        LPCSTR FileName;

        FileName = strrchr(m_PlayList[m_Position]->GetFileName(), '\\');
        if (FileName == NULL)
        {
            FileName = m_PlayList[m_Position]->GetFileName();
        }
        else
        {
            ++FileName;
        }
        return FileName;
    }
    else
    {
        return "No File";
    }
}

eVideoFormat CStillSource::GetFormat()
{
    return FORMAT_PAL_BDGHI;
}

int CStillSource::GetWidth()
{
    return m_Width;
}

int CStillSource::GetHeight()
{
    return m_Height;
}

void CStillSource::ClearPlayList()
{
    for(vector<CPlayListItem*>::iterator it = m_PlayList.begin(); 
        it != m_PlayList.end(); 
        ++it)
    {
        delete (*it);
    }
    m_PlayList.clear();
}


void CStillSource::SetMenu(HMENU hMenu)
{
    if(m_PlayList.size() > 1)
    {
        if(m_Position > 0)
        {
            EnableMenuItem(hMenu, IDM_PLAYLIST_PREVIOUS, MF_ENABLED);
        }
        else
        {
            EnableMenuItem(hMenu, IDM_PLAYLIST_PREVIOUS, MF_GRAYED);
        }
        if(m_Position < m_PlayList.size() - 1)
        {
            EnableMenuItem(hMenu, IDM_PLAYLIST_NEXT, MF_ENABLED);
        }
        else
        {
            EnableMenuItem(hMenu, IDM_PLAYLIST_NEXT, MF_GRAYED);
        }
    }
    else
    {
        EnableMenuItem(hMenu, IDM_PLAYLIST_PREVIOUS, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_PLAYLIST_NEXT, MF_GRAYED);
    }
}

void CStillSource::HandleTimerMessages(int TimerId)
{
}

LPCSTR CStillSource::GetMenuLabel()
{
    return "Playlist";
}

