/////////////////////////////////////////////////////////////////////////////
// $Id: StillSource.cpp,v 1.41 2002-03-08 06:54:07 trbarry Exp $
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
// Revision 1.38  2002/02/27 20:47:21  laurentg
// Still settings
//
// Revision 1.37  2002/02/26 21:24:25  laurentg
// Move the test on the still file size in order to have a global treatment later
//
// Revision 1.36  2002/02/23 00:30:47  laurentg
// NotifySizeChange
//
// Revision 1.35  2002/02/22 23:32:12  laurentg
// Reset width and height when still source is stopped to be sure to have a notification of size change when coming back to this still source
//
// Revision 1.34  2002/02/19 16:03:36  tobbej
// removed CurrentX and CurrentY
// added new member in CSource, NotifySizeChange
//
// Revision 1.33  2002/02/14 23:16:59  laurentg
// Stop / start capture never needed when switching between files of the playlist
// CurrentX / CurrentY not updated in StillSource but in the main decoding loop
//
// Revision 1.32  2002/02/13 00:23:24  laurentg
// Optimizations to avoid memory reallocation and to use an optimized memcpy
//
// Revision 1.31  2002/02/11 21:33:13  laurentg
// Patterns as a new source from the Still provider
//
// Revision 1.30  2002/02/10 09:23:45  laurentg
// Only display the basename of the file path in the still menu
//
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
#include "Other.h"
#include "SettingsDlg.h"


#define TIMER_SLIDESHOW 50

int __stdcall SimpleResize_InitTables(unsigned int* hControl, unsigned int* vOffsets, 
		unsigned int* vWeights, int m_Width, int m_Height, int NewWidth, int NewHeight);

static eStillFormat FormatSaving = STILL_TIFF_RGB;
static int SlideShowDelay = 5;

BYTE* DumbAlignedMalloc(int siz)
{
	BYTE* x = (BYTE*)malloc(siz+16);
	BYTE** y = (BYTE**) (x+16);
	y = (BYTE**) (((unsigned int) y & 0xfffffff0) - 4);
	*y = x;
	return (BYTE*) y+4;
}

BYTE* DumbAlignedFree(BYTE* x)
{
	BYTE* y =  *(BYTE**)(x-4);
	free(y);
	return 0;
}


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
    m_pMemcpy = NULL;
}

CStillSource::~CStillSource()
{
    if (m_StillFrame.pData != NULL)
    {
        DumbAlignedFree(m_StillFrame.pData);
    }
    if (m_OriginalFrame.pData != NULL)
    {
        free(m_OriginalFrame.pData);
    }
    ClearPlayList();
    KillTimer(hWnd, TIMER_SLIDESHOW);
}

BOOL CStillSource::LoadPlayList(LPCSTR FileName)
{
    char BufferLine[512];
    char *Buffer;
    struct stat st;
    BOOL FirstItemAdded = FALSE;
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
                        if (!FirstItemAdded)
                        {
                            m_Position = m_PlayList.size() - 1;
                            FirstItemAdded = TRUE;
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
                        if (!FirstItemAdded)
                        {
                            m_Position = m_PlayList.size() - 1;
                            FirstItemAdded = TRUE;
                        }
                    }
                }
            }
        }
        fclose(Playlist);
    }

    return FirstItemAdded;
}

BOOL CStillSource::OpenPictureFile(LPCSTR FileName)
{
    BOOL FileRead = FALSE;
    int h = m_Height;
    int w = m_Width;

    if (m_OriginalFrame.pData != NULL)
    {
        free(m_OriginalFrame.pData);
        m_OriginalFrame.pData = NULL;
    }
    m_IsPictureRead = FALSE;

    if(strlen(FileName) > 4 && stricmp(FileName + strlen(FileName) - 4, ".tif") == 0 ||
        strlen(FileName) > 5 && stricmp(FileName + strlen(FileName) - 5, ".tiff") == 0)
    {
        CTiffHelper TiffHelper(this, TIFF_CLASS_R);
        FileRead = TiffHelper.OpenMediaFile(FileName);
    }
    else if(strlen(FileName) > 4 && stricmp(FileName + strlen(FileName) - 4, ".pat") == 0)
    {
        CPatternHelper CPatternHelper(this);
        FileRead = CPatternHelper.OpenMediaFile(FileName);
    }

    if (FileRead && ((m_Width > DSCALER_MAX_WIDTH) || (m_Height > DSCALER_MAX_HEIGHT)) )
    {
        int NewWidth;
        int NewHeight;
/*
        // The file can be read by DScaler but its size is too high for DScaler
        if ( (m_Width - DSCALER_MAX_WIDTH) >= (m_Height - DSCALER_MAX_HEIGHT) )
        {
            NewWidth = DSCALER_MAX_WIDTH;
            NewHeight = m_Height - m_Width + DSCALER_MAX_WIDTH;
        }
        else
        {
            NewHeight = DSCALER_MAX_HEIGHT;
            NewWidth = m_Width - m_Height + DSCALER_MAX_HEIGHT;
        }
*/
		// Laurent, not sure about the above. did you mean something like this? - TRB 3/8/2002
		NewHeight = m_Height;
		NewWidth = m_Width;

        if (m_Width > DSCALER_MAX_WIDTH)
        {
            NewWidth = DSCALER_MAX_WIDTH;
            NewHeight = m_Height * DSCALER_MAX_WIDTH / m_Width;
        }

        if (NewHeight > DSCALER_MAX_HEIGHT)
        {
            NewWidth = NewWidth * DSCALER_MAX_HEIGHT / NewHeight;
            NewHeight = DSCALER_MAX_HEIGHT;
        }
		NewHeight = NewHeight & 0xfffffffe;		// even height
		NewWidth = NewWidth & 0xfffffffc;       // wid mod 4

        if (!ResizeOriginalFrame(NewWidth, NewHeight))
        {
            free(m_OriginalFrame.pData);
            FileRead = FALSE;
        }
    }

    if (!FileRead)
    {
        m_OriginalFrame.pData = NULL;
    }
    else
    {
        m_IsPictureRead = TRUE;

        //check if size has changed
        if(m_Height != h || m_Width != w)
        {
            NotifySizeChange();
        }
    }

    return FileRead;
}


BOOL CStillSource::OpenMediaFile(LPCSTR FileName, BOOL NewPlayList)
{
    int i;

    if (NewPlayList)
    {
        ClearPlayList();
        m_Position = -1;
    }

    i = m_Position;

    // test for the correct extension and work out the 
    // correct helper for the file type
    if(strlen(FileName) > 4 && stricmp(FileName + strlen(FileName) - 4, ".d3u") == 0)
    {
        if (LoadPlayList(FileName))
        {
            if (!ShowNextInPlayList())
            {
                if (i != -1)
                {
                    m_Position = i;
                    ShowNextInPlayList();
                }
            }
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
    switch ((eStillFormat)Setting_GetValue(Still_GetSetting(FORMATSAVING)))
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
}

void CStillSource::Start()
{
    if (!m_IsPictureRead && IsAccessAllowed())
    {
        ShowNextInPlayList();
        // The file is loaded and notification of size change is done
        // no need to call NotifySizeChange in this case
    }
    else
    {
        // Necessary if this still source was not the
        // current source when the file was loaded
        NotifySizeChange();
    }
    m_LastTickCount = 0;
    m_NewFileRequested = STILL_REQ_NONE;
}

void CStillSource::Stop()
{
    if (m_SlideShowActive)
    {
        m_SlideShowActive = FALSE;
        KillTimer(hWnd, TIMER_SLIDESHOW);
    }
    if (m_StillFrame.pData != NULL)
    {
        DumbAlignedFree(m_StillFrame.pData);
        m_StillFrame.pData = NULL;
    }
    if (m_OriginalFrame.pData != NULL)
    {
        free(m_OriginalFrame.pData);
        m_OriginalFrame.pData = NULL;
    }
    m_IsPictureRead = FALSE;
    // Reset the sizes to 0 to be sure the next time
    // a file will be loaded, a new notification of
    // size change will be generated
    m_Width = 0;
    m_Height = 0;
}

void CStillSource::Reset()
{
}

void CStillSource::GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming)
{
    BOOL bSlept = FALSE;
    DWORD CurrentTickCount;
    int Diff;
    
    m_pMemcpy = pInfo->pMemcpy;

    if(!ReadNextFrameInFile())
    {
        pInfo->bRunningLate = TRUE;
        pInfo->bMissedFrame = TRUE;
        pInfo->FrameWidth = 720;
        pInfo->FrameHeight = 480;
        pInfo->PictureHistory[0] = NULL;
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

    if ((m_Position == -1) || (m_PlayList.size() == 0) || (m_NewFileRequested != STILL_REQ_NONE))
        return FALSE;

    if ( (LOWORD(wParam) >= IDM_PLAYLIST_FILES) && (LOWORD(wParam) < (IDM_PLAYLIST_FILES+MAX_PLAYLIST_SIZE)) )
    {
        m_SlideShowActive = FALSE;
        m_NewFileRequested = STILL_REQ_THIS_ONE;
        m_NewFileReqPos = LOWORD(wParam) - IDM_PLAYLIST_FILES;
        return TRUE;
    }

    switch(LOWORD(wParam))
    {
    case IDM_PLAYLIST_PREVIOUS:
        if(m_Position > 0)
        {
            m_SlideShowActive = FALSE;
            m_NewFileRequested = STILL_REQ_PREVIOUS;
            m_NewFileReqPos = m_Position - 1;
        }
        return TRUE;
        break;
    case IDM_PLAYLIST_NEXT:
        if(m_Position < m_PlayList.size() - 1)
        {
            m_SlideShowActive = FALSE;
            m_NewFileRequested = STILL_REQ_NEXT;
            m_NewFileReqPos = m_Position + 1;
        }
        return TRUE;
        break;
    case IDM_PLAYLIST_FIRST:
        if(m_Position != 0)
        {
            m_SlideShowActive = FALSE;
            m_NewFileRequested = STILL_REQ_NEXT;
            m_NewFileReqPos = 0;
        }
        return TRUE;
        break;
    case IDM_PLAYLIST_LAST:
        if(m_Position != m_PlayList.size() - 1)
        {
            m_SlideShowActive = FALSE;
            m_NewFileRequested = STILL_REQ_PREVIOUS;
            m_NewFileReqPos = m_PlayList.size() - 1;
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
            SetTimer(hWnd, TIMER_SLIDESHOW, Setting_GetValue(Still_GetSetting(SLIDESHOWDELAY)) * 1000, NULL);
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
            m_NewFileRequested = STILL_REQ_NEXT_CIRC;
            m_NewFileReqPos = m_Position;
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
    BOOL Realloc = FALSE;

    switch (m_NewFileRequested)
    {
    case STILL_REQ_THIS_ONE:
        m_Position = m_NewFileReqPos;
        OpenPictureFile(m_PlayList[m_Position]->GetFileName());
        Realloc = TRUE;
        break;
    case STILL_REQ_NEXT:
        m_Position = m_NewFileReqPos;
        ShowNextInPlayList();
        Realloc = TRUE;
        break;
    case STILL_REQ_NEXT_CIRC:
        m_Position = m_NewFileReqPos;
        if (!ShowNextInPlayList())
        {
            m_Position = 0;
            ShowNextInPlayList();
        }
        Realloc = TRUE;
        break;
    case STILL_REQ_PREVIOUS:
        m_Position = m_NewFileReqPos;
        ShowPreviousInPlayList();
        Realloc = TRUE;
        break;
    case STILL_REQ_PREVIOUS_CIRC:
        m_Position = m_NewFileReqPos;
        if (!ShowPreviousInPlayList())
        {
            m_Position = m_PlayList.size() - 1;
            ShowPreviousInPlayList();
        }
        Realloc = TRUE;
        break;
    case STILL_REQ_NONE:
    default:
        break;
    }
    m_NewFileRequested = STILL_REQ_NONE;

    if (m_IsPictureRead)
    {
        if (Realloc && m_StillFrame.pData != NULL)
        {
            DumbAlignedFree(m_StillFrame.pData);
            m_StillFrame.pData = NULL;
        }
        if (m_StillFrame.pData == NULL)
        {
            m_StillFrame.pData = (BYTE*)DumbAlignedMalloc(m_Width * 2 * m_Height * sizeof(BYTE));
//>>>            m_StillFrame.pData = (BYTE*)malloc(m_Width * 2 * m_Height * sizeof(BYTE));
        }
        if (m_StillFrame.pData != NULL && m_OriginalFrame.pData != NULL)
        {
            if (m_pMemcpy == NULL)
            {
                memcpy(m_StillFrame.pData, m_OriginalFrame.pData, m_Width * 2 * m_Height * sizeof(BYTE));;
            }
            else
            {
                memcpy(m_StillFrame.pData, m_OriginalFrame.pData, m_Width * 2 * m_Height * sizeof(BYTE));;
//                  m_pMemcpy(m_StillFrame.pData, m_OriginalFrame.pData, m_Width * 2 * m_Height * sizeof(BYTE));;
            }
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
    if (m_NewFileRequested != STILL_REQ_NONE)
        return;

    if ( (TimerId == TIMER_SLIDESHOW) && m_SlideShowActive )
    {
        m_NewFileRequested = STILL_REQ_NEXT_CIRC;
        m_NewFileReqPos = m_Position + 1;
        m_SlideShowActive = TRUE;
        SetTimer(hWnd, TIMER_SLIDESHOW, Setting_GetValue(Still_GetSetting(SLIDESHOWDELAY)) * 1000, NULL);
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

/////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////

SETTING StillSettings[STILL_SETTING_LASTONE] =
{
    {
        "Format of saving", SLIDER, 0, (long*)&FormatSaving,
         STILL_TIFF_RGB, STILL_TIFF_RGB, STILL_FORMAT_LASTONE - 1, 1, 1,
         NULL,
        "Still", "FormatSaving", NULL,
    },
    {
        "Slide Show Delay", SLIDER, 0, (long*)&SlideShowDelay,
         5, 1, 60, 1, 1,
         NULL,
        "Still", "SlideShowDelay", NULL,
    },
};


SETTING* Still_GetSetting(STILL_SETTING Setting)
{
    if(Setting > -1 && Setting < STILL_SETTING_LASTONE)
    {
        return &(StillSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void Still_ReadSettingsFromIni()
{
    int i;
    for(i = 0; i < STILL_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(StillSettings[i]));
    }
}

void Still_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < STILL_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(StillSettings[i]), bOptimizeFileAccess);
    }
}

void Still_ShowUI()
{
    CSettingsDlg::ShowSettingsDlg("Still Settings",StillSettings, STILL_SETTING_LASTONE);
}


BOOL CStillSource::ResizeOriginalFrame(int NewWidth, int NewHeight)
{
	int OldPitch = m_Width * 2;
	int NewPitch = NewWidth * 2;
	unsigned int* hControl;		// weighting masks, alternating dwords for Y & UV
								// 1 qword for mask, 1 dword for src offset, 1 unused dword
	unsigned int* vOffsets;		// Vertical offsets of the source lines we will use
	unsigned int* vWeights;		// weighting masks, alternating dwords for Y & UV
	unsigned int* vWorkY;		// weighting masks 0Y0Y 0Y0Y...
	unsigned int* vWorkUV;		// weighting masks UVUV UVUV...

	int vWeight1[4];
	int vWeight2[4];

	// how do I do a __m128 constant, init and aligned on 16 byte boundar?
	const __int64 YMask[2]    = {0x00ff00ff00ff00ff,0x00ff00ff00ff00ff}; // keeps only luma
	const __int64 FPround1[2] = {0x0080008000800080,0x0080008000800080}; // round words
	const __int64 FPround2[2] = {0x0000008000000080,0x0000008000000080};// round dwords

    unsigned char* dstp;
    unsigned char* srcp1;
    unsigned char* srcp2;

    LOG(1, "m_Width %d, NewWidth %d, m_Height %d, NewHeight %d", m_Width, NewWidth, m_Height, NewHeight);

    // Allocate memory for the new YUYV buffer
    BYTE* NewBuf = (BYTE*)malloc(NewWidth * 2 * NewHeight * sizeof(BYTE));
	dstp = NewBuf;

    // TODO : resize m_OriginalFrame.pData into NewBuf
    // Size of m_OriginalFrame.pData is m_Width x m_Height (*2?)
    // Size of NewBuf is NewWidth x NewHeight

	// SimpleResize Init code
	hControl = (unsigned int*) malloc(NewWidth*12+128);  
	vOffsets = (unsigned int*) malloc(NewHeight*4);  
	vWeights = (unsigned int*) malloc(NewHeight*4);  
	vWorkY =   (unsigned int*) malloc(2*m_Width+128);   
	vWorkUV =  (unsigned int*) malloc(m_Width+128);   

	SimpleResize_InitTables(hControl, vOffsets, vWeights,
		m_Width, m_Height, NewWidth, NewHeight);


	// SimpleResize resize code

    if (NewBuf == NULL || hControl == NULL || vOffsets == NULL || vWeights == NULL
		|| vWorkY == NULL || vWorkUV == NULL)
    {
        return FALSE;
    }
	
	for (int y = 0; y < NewHeight; y++)
	{

		vWeight1[0] = vWeight1[1] = vWeight1[2] = vWeight1[3] = 
			(256-vWeights[y]) << 16 | (256-vWeights[y]);
		vWeight2[0] = vWeight2[1] = vWeight2[2] = vWeight2[3] = 
			vWeights[y] << 16 | vWeights[y];

		srcp1 = m_OriginalFrame.pData + vOffsets[y] * OldPitch;
		
		srcp2 = (y < NewHeight-1)
			? srcp1 + OldPitch
			: srcp1;

		_asm		
		{
			push	ecx						// have to save this?
			mov		ecx, OldPitch
			shr		ecx, 3					// 8 bytes a time
			mov		esi, srcp1				// top of 2 src lines to get
			mov		edx, srcp2				// next "
			mov		edi, vWorkY				// luma work destination line
			mov		ebx, vWorkUV			// luma work destination line
			xor		eax, eax

			movq	mm7, YMask				// useful luma mask constant
			movq	mm5, vWeight1
			movq	mm6, vWeight2
			movq	mm0, FPround1			// useful rounding constant
		    align	16
	vLoopMMX:	
			movq	mm1, qword ptr[esi+eax*2] // top of 2 lines to interpolate
			movq	mm2, qword ptr[edx+eax*2] // 2nd of 2 lines

			movq	mm3, mm1				// copy top bytes
			pand	mm1, mm7				// keep only luma	
			pxor	mm3, mm1				// keep only chroma
			psrlw	mm3, 8					// right just chroma
			pmullw	mm1, mm5				// mult by weighting factor
			pmullw	mm3, mm5				// mult by weighting factor

			movq	mm4, mm2				// copy 2nd bytes
			pand	mm2, mm7				// keep only luma	
			pxor	mm4, mm2				// keep only chroma
			psrlw	mm4, 8					// right just chroma
			pmullw	mm2, mm6				// mult by weighting factor
			pmullw	mm4, mm6				// mult by weighting factor

			paddw	mm1, mm2				// combine lumas
			paddusw	mm1, mm0				// round
			psrlw	mm1, 8					// right adjust luma
			movq	qword ptr[edi+eax*2], mm1	// save lumas in our work area

			paddw	mm3, mm4				// combine chromas
			paddusw	mm3, mm0				// round
			psrlw	mm3, 8					// right adjust chroma
			packuswb mm3,mm3				// pack UV's into low dword
			movd	dword ptr[ebx+eax], mm3	// save in our work area

			lea     eax, [eax+4]
			loop	vloopMMX
	

// We've taken care of the vertical scaling, now do horizontal
			movq	mm7, YMask			// useful 0U0U..  mask constant
			movq	mm6, FPround2			// useful rounding constant, dwords
			mov		esi, hControl		// @ horiz control bytes			
			mov		ecx, NewPitch
			shr		ecx, 2				// 4 bytes a time, 2 pixels
			mov     edx, vWorkY			// our luma data, as 0Y0Y 0Y0Y..
			mov		edi, dstp			// the destination line
			mov		ebx, vWorkUV		// chroma data, as UVUV UVUV...
			align 16
	hLoopMMX:   
			mov		eax, [esi+16]		// get data offset in pixels, 1st pixel pair
			movd mm0, [edx+eax*2]		// copy luma pair
			shr		eax, 1				// div offset by 2
			movd	mm1, [ebx+eax*2]	// copy UV pair VUVU
			psllw   mm1, 8				// shift out V, keep 0000U0U0	

			mov		eax, [esi+20]		// get data offset in pixels, 2nd pixel pair
			punpckldq mm0, [edx+eax*2]		// copy luma pair
			shr		eax, 1				// div offset by 2
			punpckldq mm1, [ebx+eax*2]	// copy UV pair VUVU
			psrlw   mm1, 8				// shift out U0, keep 0V0V 0U0U	
		
			pmaddwd mm0, [esi]			// mult and sum lumas by ctl weights
			paddusw	mm0, mm6			// round
			psrlw	mm0, 8				// right just 2 luma pixel value 000Y,000Y

			pmaddwd mm1, [esi+8]		// mult and sum chromas by ctl weights
			paddusw	mm1, mm6			// round
			pslld	mm1, 8				// shift into low bytes of different words
			pand	mm1, mm7			// keep only 2 chroma values 0V00,0U00
			por		mm0, mm1			// combine luma and chroma, 0V0Y,0U0Y
			packuswb mm0,mm0			// pack all into low dword, xxxxVYUY
			movd	dword ptr[edi], mm0	// done with 2 pixels

			lea    esi, [esi+24]		// bump to next control bytest
			lea    edi, [edi+4]			// bump to next output pixel addr
            loop   hLoopMMX				// loop for more

			pop		ecx
            emms
		}                               // done with one line
        dstp += NewPitch;
    }


	// SimpleResize cleanup code
	free(hControl);
	free(vOffsets);
	free(vWeights);
	free(vWorkY);
	free(vWorkUV);

    // Replace the old YUYV buffer by the new one
    free(m_OriginalFrame.pData);
    m_OriginalFrame.pData = NewBuf;
    m_Width = NewWidth;
    m_Height = NewHeight;

    return TRUE;
}


// This function accepts a position from 0 to 1 and warps it, to 0 through 1 based
// upon the wFact var. The warp equations are designed to:
// 
// * Always be rising but yield results from 0 to 1
//
// * Have a first derivative that doesn't go to 0 or infinity, at least close
//   to the center of the screen
//
// * Have a curvature (absolute val of 2nd derivative) that is small in the
//   center and smoothly rises towards the edges. We would like the curvature
//   to be everywhere = 0 when the warp factor = 1
//

// For each horizontal output pixel there are 2 4 byte masks, and a src offset. The first gives
// the weights of the 4 surrounding luma values in the loaded qword, the second gives
// the weights of the chroma pixels. At most 2 values will be non-zero in each mask.
// The hControl offsets in the table are to where to load the qword of pixel data. Usually the
// 2nd & 3rd pixel values in that qword are used, but boundary conditions may change
// that and you can't average 2 adjacent chroma values in YUY2 format because they will
// contain YU YV YU YV YU YV...
// Horizontal weights are scaled 0-128, Vertical weights are scaled 0-256.

int __stdcall SimpleResize_InitTables(unsigned int* hControl, unsigned int* vOffsets, 
		unsigned int* vWeights, int m_Width, int m_Height, int NewWidth, int NewHeight)
{
	int i;
	int j;
	int k;
	int wY1;
	int wY2;
	int wUV1;
	int wUV2;

	// First set up horizontal table
	for(i=0; i < NewWidth; i+=2)
	{
		j = i * 256 * (m_Width-1) / (NewWidth-1);

		k = j>>8;
		wY2 = j - (k << 8);				// luma weight of right pixel
		wY1 = 256 - wY2;				// luma weight of left pixel
		wUV2 = (k%2)
			? 128 + (wY2 >> 1)
			: wY2 >> 1;
		wUV1 = 256 - wUV2;

// the right hand edge luma will be off by one pixel currently to handle edge conditions.
// I should figure a better way aound this without a performance hit. But I can't see 
// the difference so it is lower prority.
		if (k > m_Width - 3)
		{
			hControl[i*3+4] = m_Width - 3;	 //	point to last byte
			hControl[i*3] =   0x01000000;    // use 100% of rightmost Y
			hControl[i*3+2] = 0x01000000;    // use 100% of rightmost U
		}
		else
		{
			hControl[i*3+4] = k;			// pixel offset
			hControl[i*3] = wY2 << 16 | wY1; // luma weights
			hControl[i*3+2] = wUV2 << 16 | wUV1; // chroma weights
		}

		j = (i+1) * 256 * (m_Width-1) / (NewWidth-1);

		k = j>>8;
		wY2 = j - (k << 8);				// luma weight of right pixel
		wY1 = 256 - wY2;				// luma weight of left pixel
		wUV2 = (k%2)
			? 128 + (wY2 >> 1)
			: wY2 >> 1;
		wUV1 = 256 - wUV2;

		if (k > m_Width - 3)
		{
			hControl[i*3+5] = m_Width - 3;	 //	point to last byte
			hControl[i*3+1] = 0x01000000;    // use 100% of rightmost Y
			hControl[i*3+3] = 0x01000000;    // use 100% of rightmost V
		}
		else
		{
			hControl[i*3+5] = k;			// pixel offset
			hControl[i*3+1] = wY2 << 16 | wY1; // luma weights
			hControl[i*3+3] = wUV2 << 16 | wUV1; // chroma weights
		}
	}

	hControl[NewWidth*3+4] =  2 * (m_Width-1);		// give it something to prefetch at end
	hControl[NewWidth*3+5] =  2 * (m_Width-1);		// "

	// Next set up vertical table. The offsets are measured in lines and will be mult
	// by the source pitch later 
	for(i=0; i< NewHeight; ++i)
	{
		j = i * 256 * (m_Height-1) / (NewHeight-1);
		k = j >> 8;
		vOffsets[i] = k;
		wY2 = j - (k << 8); 
		vWeights[i] = wY2;				// weight to give to 2nd line
	}

	return 0;
}
