/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file stillsource.h stillsource Header file
 */

#ifndef __STILLSOURCE_H___
#define __STILLSOURCE_H___

#include "Source.h"
#include "Bt848_Defines.h"

#define MAX_PLAYLIST_SIZE   50

enum eStillFormat
{
    STILL_TIFF_RGB = 0,
    STILL_TIFF_YCbCr,
    STILL_JPEG,
    STILL_FORMAT_LASTONE,
};

enum eStillNewFileRequest
{
    STILL_REQ_NONE = 0,
    STILL_REQ_THIS_ONE,
    STILL_REQ_NEXT,
    STILL_REQ_NEXT_CIRC,
    STILL_REQ_PREVIOUS,
    STILL_REQ_PREVIOUS_CIRC,
};

class CStillSource;
class CTiffHelper;
class CJpegHelper;
class CPatternHelper;

/** Base class for file type specific code that CStillSource uses to read
    Still files
*/
class CStillSourceHelper
{
public:
    CStillSourceHelper(CStillSource* pParent);
    virtual BOOL OpenMediaFile(const tstring& FileName) = 0;
    virtual void SaveSnapshot(const tstring& FilePath, int Height, int Width, BYTE* pOverlay, LONG OverlayPitch, const tstring& Context) = 0;
protected:
    CStillSource* m_pParent;
};

/** An item in a CStillSource playlist
*/
class CPlayListItem
{
public:
    CPlayListItem(const tstring& FileName);
    CPlayListItem(BYTE* FrameBuffer, int FrameHeight, int FrameWidth, int LinePitch, BOOL SquarePixels, const tstring& Context);
    const tstring& GetFileName();
    void SetFileName(const tstring& FileName);
    BOOL GetMemoryInfo(BYTE** pFrameBuffer, int* pFrameHeight, int* pFrameWidth, int* pLinePitch, BOOL* pSquarePixels, tstring& pContext);
    BOOL IsInMemory();
    time_t GetTimeStamp();
    BOOL IsSupported();
    void SetSupported(BOOL Supported);
    void FreeBuffer();
private:
    tstring m_FileName;
    BOOL m_Supported;
    BYTE* m_FrameBuffer;
    int m_FrameHeight;
    int m_FrameWidth;
    int m_LinePitch;
    BOOL m_SquarePixels;
    time_t m_TimeStamp;
    tstring m_Context;
};

/** Source class that can read files and playlists and display
    then as faked video
*/
class CStillSource : public CSource
{
public:
    CStillSource(const tstring& IniSection);
    ~CStillSource();
    void CreateSettings(LPCTSTR IniSection);
    void Start();
    void Stop();
    void Reset();
    void GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming);
    BOOL HandleWindowsCommands(HWND hWnd, WPARAM wParam, LPARAM lParam);
    tstring GetStatus();
    CSliderSetting* GetVolume() {return NULL;};
    CSliderSetting* GetBalance() {return NULL;};
    void Mute() {;};
    void UnMute() {;};
    CSliderSetting* GetBrightness() {return NULL;};
    CSliderSetting* GetContrast() {return NULL;};
    CSliderSetting* GetHue() {return NULL;};
    CSliderSetting* GetSaturation() {return NULL;};
    CSliderSetting* GetSaturationU() {return NULL;};
    CSliderSetting* GetSaturationV() {return NULL;};
    CYesNoSetting* GetAnalogueBlanking() {return NULL;};
    CSliderSetting* GetTopOverscan() {return NULL;};
    CSliderSetting* GetBottomOverscan() {return NULL;};
    CSliderSetting* GetLeftOverscan() {return NULL;};
    CSliderSetting* GetRightOverscan() {return NULL;};
    CSliderSetting* GetHDelay() {return NULL;};
    CSliderSetting* GetVDelay() {return NULL;};

    SmartPtr<CSettingsHolder> GetSettingsPage() {return SmartPtr<CSettingsHolder>();};

    eVideoFormat GetFormat();
    BOOL IsInTunerMode() {return FALSE;};
    int GetInitialWidth();
    int GetInitialHeight();
    int GetWidth();
    int GetHeight();
    void SetWidth(int w) {return;};

    void UpdateMenu();
    void SetMenu(HMENU hMenu);
    void HandleTimerMessages(int TimerId);
    void SetFormat(eVideoFormat NewFormat) {};
    BOOL SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat) {return FALSE;};
    BOOL IsVideoPresent();
    void DecodeVBI(TDeinterlaceInfo* pInfo) {;};
    tstring GetMenuLabel();
    BOOL ReadNextFrameInFile();
    BOOL LoadPlayList(const tstring& FileName);
    void SaveSnapshotInFile(int FrameHeight, int FrameWidth, BYTE* pFrameBuffer, LONG LinePitch);
    void SaveSnapshotInMemory(int FrameHeight, int FrameWidth, BYTE* pAllocBuffer, LONG LinePitch);
    void SaveInFile(int pos);
    BOOL OpenMediaFile(const tstring& FileName, BOOL NewPlayList);
    BOOL IsAccessAllowed();
    void SetAspectRatioData();
    BOOL HasSquarePixels() {return m_SquarePixels;};
    void ChangeSettingsBasedOnHW(int ProcessorSpeed, int TradeOff) {;};
    tstring IDString() { return m_IDString; }
    void SetNavigOnly(BOOL NavigOnly);
    BOOL IsNavigOnly();
    int  NumInputs(eSourceInputType InputType) { return 0; };
    BOOL SetInput(eSourceInputType InputType, int Nr) { return FALSE; };
    int GetInput(eSourceInputType InputType) { return -1; };
    tstring GetInputName(eSourceInputType InputType, int Nr) { return NULL; };
    BOOL InputHasTuner(eSourceInputType InputType, int Nr) { return FALSE; };

    SmartPtr<ITuner> GetTuner() {return SmartPtr<ITuner>();}

    void Pause() {return;};
    void UnPause() {return;};

    BOOL HasMediaControl() {return FALSE;};

    BOOL IsAudioMixerAccessAllowed() {return FALSE;};

    int     GetPlaylistPosition();
    int CountMemoryUsage();

    BOOL IsOneItemInMemory();

    friend class CTiffHelper;
    friend class CJpegHelper;
    friend class CPatternHelper;

private:
    void FreeOriginalFrameBuffer();
    void ClearPlayList();
    BOOL ShowNextInPlayList();
    BOOL ShowPreviousInPlayList();
    BOOL OpenPictureFile(const tstring& FileName);
    BOOL OpenPictureMemory(BYTE* FrameBuffer, int FrameHeight, int FrameWidth, int LinePitch, BOOL SquarePixels, const tstring& Context);
    BOOL SavePlayList(const tstring& FileName);
    BOOL IsItemInList(const tstring& FileName);
    BOOL FindFileName(time_t TimeStamp, tstring& FileName);

protected:
    int         m_InitialWidth;
    int         m_InitialHeight;
    int         m_Width;
    int         m_Height;
    BYTE*       m_StillFrameBuffer;
    TPicture    m_StillFrame;
    BYTE*       m_OriginalFrameBuffer;
    TPicture    m_OriginalFrame;
    std::vector<CPlayListItem*> m_PlayList;
    int         m_Position;
    BOOL        m_IsPictureRead;
    BOOL        m_SquarePixels;
    BOOL        m_NavigOnly;
    int         m_LinePitch;
    tstring m_IDString;

protected:
    void ChangeDefaultsForVideoFormat(BOOL bDontSetValue) {};
    void ChangeDefaultsForVideoInput(BOOL bDontSetValue) {};
    void ChangeDefaultsForAudioInput(BOOL bDontSetValue) {};

private:
    DWORD       m_LastTickCount;
    DWORD        m_FrameDuration;
    BOOL        m_SlideShowActive;

    eStillNewFileRequest    m_NewFileRequested;
    int                     m_NewFileReqPos;

    MEMCPY_FUNC*    m_pMemcpy;

    tstring m_Section;
};


//        BYTE** y = (BYTE**) (x+16);
//        y = (BYTE**) ((unsigned int) y & 0xfffffff0);
#define    START_ALIGNED16(buf)    ((buf) + 16 - ((DWORD)(buf) % 16))

BOOL ResizeFrame(BYTE* OldBuf, int OldPitch, int OldWidth, int OldHeight, BYTE* NewBuf, int NewPitch, int NewWidth, int NewHeight);

tstring BuildDScalerContext();

tstring GetStillsPath();

SETTING* Still_GetSetting(STILL_SETTING Setting);

#endif