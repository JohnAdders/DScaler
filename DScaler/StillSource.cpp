/////////////////////////////////////////////////////////////////////////////
// $Id: StillSource.cpp,v 1.30 2002-02-10 09:23:45 laurentg Exp $
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
// Revision 1.29  2002/02/09 21:12:28  laurentg
// Old test patterns restored
// Loading of d3u files improved (absolute or relative path)
//
// Revision 1.28  2002/02/09 02:44:56  laurentg
// Overscan now stored in a setting of the source
//
// Revision 1.27  2002/02/08 19:11:43  laurentg
// Don't add the file to the playlist if the file is not supported
//
// Revision 1.26  2002/02/08 00:36:06  laurentg
// Support of a new type of file : DScaler patterns
//
// Revision 1.25  2002/02/02 01:31:18  laurentg
// Access to the files of the playlist added in the menus
// Save Playlist added
// "Video Adjustments ..." restored in the popup menu
//
// Revision 1.24  2002/02/01 00:41:58  laurentg
// Playlist code updated
//
// Revision 1.23  2002/01/17 22:22:06  robmuller
// Added member function GetTunerId().
//
// Revision 1.22  2001/12/18 14:53:00  adcockj
// Fixed overlay problem when running on machine with no tv card
//
// Revision 1.21  2001/12/16 10:15:45  laurentg
// Calculation of used fields restored
// Close function added
//
// Revision 1.20  2001/12/08 20:00:24  laurentg
// Access control on sources
//
// Revision 1.19  2001/12/08 17:39:14  laurentg
// Slide show feature added
//
// Revision 1.18  2001/12/08 14:23:33  laurentg
// Debug traces deleted
//
// Revision 1.17  2001/12/08 13:48:40  laurentg
// New StillSource for snapshots done during the DScaler session
//
// Revision 1.16  2001/12/08 12:04:07  laurentg
// New setting m_StillFormat
//
// Revision 1.15  2001/12/05 21:45:11  ittarnavsky
// added changes for the AudioDecoder and AudioControls support
//
// Revision 1.14  2001/12/05 00:08:41  laurentg
// Use of LibTiff DLL
//
// Revision 1.13  2001/11/30 10:46:43  adcockj
// Fixed crashes and leaks
//
// Revision 1.12  2001/11/28 16:04:50  adcockj
// Major reorganization of STill support
//
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
#include "Calibration.h"
#include "OutThreads.h"
#include "AspectRatio.h"


#define TIMER_SLIDESHOW 50


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

CStillSource::CStillSource(LPCSTR IniSection) :
    CSource(0, IDC_STILL),
    m_Section(IniSection)
{
    CreateSettings(IniSection);
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
    m_SlideShowActive = FALSE;
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
    KillTimer(hWnd, TIMER_SLIDESHOW);
}

BOOL CStillSource::OpenPictureFile(LPCSTR FileName)
{
    if(strlen(FileName) > 4 && stricmp(FileName + strlen(FileName) - 4, ".tif") == 0 ||
        strlen(FileName) > 5 && stricmp(FileName + strlen(FileName) - 5, ".tiff") == 0)
    {
        CTiffHelper TiffHelper(this, TIFF_CLASS_R);
        return TiffHelper.OpenMediaFile(FileName);
    }
    else if(strlen(FileName) > 4 && stricmp(FileName + strlen(FileName) - 4, ".pat") == 0)
    {
        CPatternHelper CPatternHelper(this);
        return CPatternHelper.OpenMediaFile(FileName);
    }
    else
    {
        return FALSE;
    }
}


BOOL CStillSource::OpenMediaFile(LPCSTR FileName, BOOL NewPlayList)
{
    if (NewPlayList)
    {
        ClearPlayList();
        m_Position = -1;
    }

    // test for the correct extension and work out the 
    // correct helper for the file type
    if(strlen(FileName) > 4 && stricmp(FileName + strlen(FileName) - 4, ".d3u") == 0)
    {
        char BufferLine[512];
        char *Buffer;
        struct stat st;
        BOOL FirstNewAdded = FALSE;
        FILE* Playlist = fopen(FileName, "r");
        if(Playlist != NULL)
        {
            while(!feof(Playlist))
            {
                if(fgets(BufferLine, 512, Playlist))
                {
                    BufferLine[511] = '\0';
                    Buffer = BufferLine;
                    while(strlen(Buffer) > 0 && *Buffer <= ' ')
                    {
                        Buffer++;
                    }
                    if(strlen(Buffer) == 0 || *Buffer == '#' || *Buffer == ';')
                    {
                        continue;
                    }
                    // take care of stuff that is at end of the line
                    while(strlen(Buffer) > 0 && Buffer[strlen(Buffer) - 1] <= ' ')
                    {
                        Buffer[strlen(Buffer) - 1] = '\0';
                    }
                    if (strlen(Buffer) == 0)
                    {
                        continue;
                    }
                    if (!strncmp(&Buffer[1], ":\\", 2) || (Buffer[0] == '\\'))
                    {
                        if (!stat(Buffer, &st))
                        {
                            CPlayListItem* Item = new CPlayListItem(Buffer, 10);
                            m_PlayList.push_back(Item);
                            if (!FirstNewAdded)
                            {
                                m_Position = m_PlayList.size() - 1;
                                FirstNewAdded = TRUE;
                            }
                        }
                    }
                    else
                    {
                        char FilePath[MAX_PATH];
                        char* pStr = strrchr(FileName, '\\');
                        if (pStr == NULL)
                        {
                            strcpy(FilePath, FileName);
                        }
                        else
                        {
                            strncpy(FilePath, FileName, pStr - FileName + 1);
                            FilePath[pStr - FileName + 1] = '\0';
                            strcat(FilePath, Buffer);
                        }
                        if (!stat(FilePath, &st))
                        {
                            CPlayListItem* Item = new CPlayListItem(FilePath, 10);
                            m_PlayList.push_back(Item);
                            if (!FirstNewAdded)
                            {
                                m_Position = m_PlayList.size() - 1;
                                FirstNewAdded = TRUE;
                            }
                        }
                    }
                }
            }
            fclose(Playlist);
        }

        if (FirstNewAdded)
        {
            ShowNextInPlayList();
            return TRUE;
        }
    }
    else if(OpenPictureFile(FileName))
    {
        CPlayListItem* Item = new CPlayListItem(FileName, 10);
        m_PlayList.push_back(Item);
        m_Position = m_PlayList.size() - 1;
        return TRUE;
    }
    else if ((m_Position != -1) && (m_PlayList.size() > 0))
    {
        OpenPictureFile(m_PlayList[m_Position]->GetFileName());
    }

    return FALSE;
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
    m_Position = m_PlayList.size() - 1;
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
    m_Position = 0;
    return FALSE;
}

void CStillSource::SaveSnapshot(LPCSTR FilePath, int FrameHeight, int FrameWidth, BYTE* pOverlay, LONG OverlayPitch)
{
    switch ((eStillFormat)m_StillFormat->GetValue())
    {
    case STILL_TIFF_RGB:
        {
            CTiffHelper TiffHelper(this, TIFF_CLASS_R);
            TiffHelper.SaveSnapshot(FilePath, FrameHeight, FrameWidth, pOverlay, OverlayPitch);
            OpenMediaFile(FilePath, FALSE);
            break;
        }
    case STILL_TIFF_YCbCr:
        {
            CTiffHelper TiffHelper(this, TIFF_CLASS_Y);
            TiffHelper.SaveSnapshot(FilePath, FrameHeight, FrameWidth, pOverlay, OverlayPitch);
            OpenMediaFile(FilePath, FALSE);
            break;
        }
    default:
        break;
    }
}

void CStillSource::CreateSettings(LPCSTR IniSection)
{
    m_StillFormat = new CSliderSetting("Format of Still Pictures", STILL_TIFF_RGB, STILL_TIFF_RGB, STILL_FORMAT_LASTONE - 1, IniSection, "StillFormat");
    m_Settings.push_back(m_StillFormat);

    m_SlideShowDelay = new CSliderSetting("Slide Show Delay", 5, 1, 60, IniSection, "SlideShowDelay");
    m_Settings.push_back(m_SlideShowDelay);

    ReadFromIni();
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
    if (m_SlideShowActive)
    {
        m_SlideShowActive = FALSE;
        KillTimer(hWnd, TIMER_SLIDESHOW);
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
        pInfo->FrameWidth = 720;
        pInfo->FrameHeight = 480;
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

    Timing_IncrementUsedFields();
}

BOOL CStillSource::HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam)
{
    CPlayListItem* Item;
    vector<CPlayListItem*>::iterator it;
    OPENFILENAME SaveFileInfo;
    char FilePath[MAX_PATH];
    char* FileFilters = "DScaler Playlists\0*.d3u\0";

    if ((m_Position == -1) || (m_PlayList.size() == 0))
        return FALSE;

    if ( (LOWORD(wParam) >= IDM_PLAYLIST_FILES) && (LOWORD(wParam) < (IDM_PLAYLIST_FILES+MAX_PLAYLIST_SIZE)) )
    {
        m_SlideShowActive = FALSE;
        m_Position = LOWORD(wParam) - IDM_PLAYLIST_FILES;
        Stop_Capture();
        OpenPictureFile(m_PlayList[m_Position]->GetFileName());
        Start_Capture();
        return TRUE;
    }

    switch(LOWORD(wParam))
    {
    case IDM_PLAYLIST_PREVIOUS:
        if(m_Position > 0)
        {
            m_SlideShowActive = FALSE;
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
            m_SlideShowActive = FALSE;
            ++m_Position;
            Stop_Capture();
            ShowNextInPlayList();
            Start_Capture();
        }
        return TRUE;
        break;
    case IDM_PLAYLIST_FIRST:
        if(m_Position != 0)
        {
            m_SlideShowActive = FALSE;
            m_Position = 0;
            Stop_Capture();
            ShowNextInPlayList();
            Start_Capture();
        }
        return TRUE;
        break;
    case IDM_PLAYLIST_LAST:
        if(m_Position != m_PlayList.size() - 1)
        {
            m_SlideShowActive = FALSE;
            m_Position = m_PlayList.size() - 1;
            Stop_Capture();
            ShowPreviousInPlayList();
            Start_Capture();
        }
        return TRUE;
        break;
    case IDM_PLAYLIST_UP:
        if(m_Position > 0)
        {
            m_SlideShowActive = FALSE;
            Item = m_PlayList[m_Position];
            it = m_PlayList.erase(m_PlayList.begin() + m_Position);
            --it;
            m_PlayList.insert(it, Item);
            --m_Position;
            UpdateMenu();
        }
        return TRUE;
        break;
    case IDM_PLAYLIST_DOWN:
        if(m_Position < m_PlayList.size() - 1)
        {
            m_SlideShowActive = FALSE;
            Item = m_PlayList[m_Position];
            it = m_PlayList.erase(m_PlayList.begin() + m_Position);
            ++it;
            if (it != m_PlayList.end())
            {
                m_PlayList.insert(it, Item);
            }
            else
            {
                m_PlayList.push_back(Item);
            }
            ++m_Position;
            UpdateMenu();
        }
        return TRUE;
        break;
    case IDM_PLAYLIST_SLIDESHOW:
        m_SlideShowActive = !m_SlideShowActive;
        if (m_SlideShowActive)
        {
            SetTimer(hWnd, TIMER_SLIDESHOW, m_SlideShowDelay->GetValue() * 1000, NULL);
        }
        else
        {
            KillTimer(hWnd, TIMER_SLIDESHOW);
        }
        return TRUE;
        break;
    case IDM_CLOSE_FILE:
        m_SlideShowActive = FALSE;
        m_PlayList.erase(m_PlayList.begin() + m_Position);
        if (m_Position >= m_PlayList.size())
        {
            m_Position = m_PlayList.size() - 1;
        }
        UpdateMenu();
        if (m_Position == -1)
        {
            PostMessage(hWnd, WM_COMMAND, IDM_SOURCE_FIRST, 0);
        }
        else
        {
            Stop_Capture();
            ShowNextInPlayList();
            Start_Capture();
        }
        return TRUE;
        break;
    case IDM_CLOSE_ALL:
        m_SlideShowActive = FALSE;
        ClearPlayList();
        m_Position = -1;
        UpdateMenu();
        PostMessage(hWnd, WM_COMMAND, IDM_SOURCE_FIRST, 0);
        return TRUE;
        break;
    case IDM_PLAYLIST_SAVE:
        SaveFileInfo.lStructSize = sizeof(SaveFileInfo);
        SaveFileInfo.hwndOwner = hWnd;
        SaveFileInfo.lpstrFilter = FileFilters;
        SaveFileInfo.lpstrCustomFilter = NULL;
        FilePath[0] = 0;
        SaveFileInfo.lpstrFile = FilePath;
        SaveFileInfo.nMaxFile = sizeof(FilePath);
        SaveFileInfo.lpstrFileTitle = NULL;
        SaveFileInfo.lpstrInitialDir = NULL;
        SaveFileInfo.lpstrTitle = "Save Playlist As";
        SaveFileInfo.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT;
        SaveFileInfo.lpstrDefExt = "d3u";
        if (GetSaveFileName(&SaveFileInfo))
        {
            if (!SavePlayList(FilePath))
            {
                MessageBox(hWnd, "Playlist not saved", "DScaler Warning", MB_OK);
            }
        }
        return TRUE;
        break;
    default:
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

eTunerId CStillSource::GetTunerId()
{
    return TUNER_ABSENT;
}

eVideoFormat CStillSource::GetFormat()
{
    return VIDEOFORMAT_PAL_B;
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

BOOL CStillSource::SavePlayList(LPCSTR FileName)
{
    FILE* Playlist = fopen(FileName, "w");
    if (Playlist == NULL)
        return FALSE;

    for(vector<CPlayListItem*>::iterator it = m_PlayList.begin(); 
        it != m_PlayList.end(); 
        ++it)
    {
        fprintf(Playlist, "%s\n", (*it)->GetFileName());
    }

    fclose(Playlist);

    return TRUE;
}

void CStillSource::UpdateMenu()
{
    HMENU           hSubMenu;
    HMENU           hMenuFiles;
    MENUITEMINFO    MenuItemInfo;
    int             j;

    hSubMenu = GetSubMenu(m_hMenu, 0);
    if(hSubMenu == NULL) return;
    hMenuFiles = GetSubMenu(hSubMenu, 4);
    if(hMenuFiles == NULL)
    {
        if (ModifyMenu(hSubMenu, 4, MF_STRING | MF_BYPOSITION | MF_POPUP, (UINT)CreatePopupMenu(), "F&iles"))
        {
            hMenuFiles = GetSubMenu(hSubMenu, 4);
        }
    }
    if(hMenuFiles == NULL) return;

    j = GetMenuItemCount(hMenuFiles);
    while (j)
    {
        --j;
        RemoveMenu(hMenuFiles, j, MF_BYPOSITION);
    }
    
    j = 0;
    for (vector<CPlayListItem*>::iterator it = m_PlayList.begin(); 
        it != m_PlayList.end(); 
        ++it, ++j)
    {
        LPCSTR FileName = strrchr((*it)->GetFileName(), '\\');
        if (FileName == NULL)
        {
            FileName = (*it)->GetFileName();
        }
        else
        {
            ++FileName;
        }
        MenuItemInfo.cbSize = sizeof (MenuItemInfo);
        MenuItemInfo.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
        MenuItemInfo.fType = MFT_STRING;
        MenuItemInfo.dwTypeData = (LPSTR) FileName;
        MenuItemInfo.cch = strlen (FileName);
        MenuItemInfo.fState = (m_Position == j) ? MFS_CHECKED : MFS_UNCHECKED;
        MenuItemInfo.wID = IDM_PLAYLIST_FILES + j;
        InsertMenuItem(hMenuFiles, j, TRUE, &MenuItemInfo);
    }
}

void CStillSource::SetMenu(HMENU hMenu)
{
    HMENU           hSubMenu;
    HMENU           hMenuFiles;

    CheckMenuItemBool(hMenu, IDM_PLAYLIST_SLIDESHOW, m_SlideShowActive);
    if(m_PlayList.size() > 1)
    {
        if(m_Position > 0)
        {
            EnableMenuItem(hMenu, IDM_PLAYLIST_PREVIOUS, MF_ENABLED);
            EnableMenuItem(hMenu, IDM_PLAYLIST_FIRST, MF_ENABLED);
            EnableMenuItem(hMenu, IDM_PLAYLIST_UP, MF_ENABLED);
        }
        else
        {
            EnableMenuItem(hMenu, IDM_PLAYLIST_PREVIOUS, MF_GRAYED);
            EnableMenuItem(hMenu, IDM_PLAYLIST_FIRST, MF_GRAYED);
            EnableMenuItem(hMenu, IDM_PLAYLIST_UP, MF_GRAYED);
        }
        if(m_Position < m_PlayList.size() - 1)
        {
            EnableMenuItem(hMenu, IDM_PLAYLIST_NEXT, MF_ENABLED);
            EnableMenuItem(hMenu, IDM_PLAYLIST_LAST, MF_ENABLED);
            EnableMenuItem(hMenu, IDM_PLAYLIST_DOWN, MF_ENABLED);
        }
        else
        {
            EnableMenuItem(hMenu, IDM_PLAYLIST_NEXT, MF_GRAYED);
            EnableMenuItem(hMenu, IDM_PLAYLIST_LAST, MF_GRAYED);
            EnableMenuItem(hMenu, IDM_PLAYLIST_DOWN, MF_GRAYED);
        }
        EnableMenuItem(hMenu, IDM_PLAYLIST_SLIDESHOW, MF_ENABLED);
    }
    else
    {
        EnableMenuItem(hMenu, IDM_PLAYLIST_PREVIOUS, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_PLAYLIST_NEXT, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_PLAYLIST_FIRST, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_PLAYLIST_LAST, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_PLAYLIST_UP, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_PLAYLIST_DOWN, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_PLAYLIST_SLIDESHOW, MF_GRAYED);
    }

    hSubMenu = GetSubMenu(m_hMenu, 0);
    if(hSubMenu == NULL) return;
    hMenuFiles = GetSubMenu(hSubMenu, 4);
    if(hMenuFiles == NULL) return;

    for (int i(0); i < GetMenuItemCount(hMenuFiles); ++i)
    {
        if (m_Position == i)
        {
            CheckMenuItem(hMenuFiles, i, MF_BYPOSITION | MF_CHECKED);
        }
        else
        {
            CheckMenuItem(hMenuFiles, i, MF_BYPOSITION | MF_UNCHECKED);
        }
    }
}

void CStillSource::HandleTimerMessages(int TimerId)
{
    if ( (TimerId == TIMER_SLIDESHOW) && m_SlideShowActive )
    {
        Stop_Capture();
        ++m_Position;
        if (!ShowNextInPlayList())
        {
            m_Position = 0;
            ShowNextInPlayList();
        }
        Start_Capture();
        m_SlideShowActive = TRUE;
        SetTimer(hWnd, TIMER_SLIDESHOW, m_SlideShowDelay->GetValue() * 1000, NULL);
    }
}

LPCSTR CStillSource::GetMenuLabel()
{
    return m_Section.c_str();
}

BOOL CStillSource::IsVideoPresent()
{
    return m_IsPictureRead;
}

BOOL CStillSource::IsAccessAllowed()
{
    return (m_Position != -1 && m_PlayList.size() > 0);
}

void CStillSource::SetOverscan()
{
    AspectSettings.InitialOverscan = 0;
}
