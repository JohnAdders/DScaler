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
 * @file DSSourceBase.h interface for the CDSSourceBase class.
 */

#if !defined(AFX_DSSOURCEBASE_H__E88C9FB3_4694_419D_AD7C_22F2E17260B4__INCLUDED_)
#define AFX_DSSOURCEBASE_H__E88C9FB3_4694_419D_AD7C_22F2E17260B4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Source.h"
#include "DSGraph.h"

/**
 * @bug if the output thread is terminated by a call to TerminateThread when
 * m_hOutThreadSync is held problems will occur, for example Stop() will
 * block for ever. (not 100% sure this is what is happening, but atleast
 * Stop() blocks forever sometimes)
 *
 * @bug sometimes accessing the menu while the filter graph is building
 * causes problems, probably fixed.
 */
class CDSSourceBase : public CSource
{
public:
    CDSSourceBase(long SetMessage, long MenuId);
    virtual ~CDSSourceBase();

    int GetInitialWidth() {return GetWidth();};
    int GetInitialHeight() {return GetHeight() / 2;};
    int GetWidth();
    int GetHeight();
    void SetWidth(int w);
    void GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming);
    void Start();
    void Stop();
    void Reset();
    void StopAndSeekToBeginning();

    BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam);

    void SetFormat(eVideoFormat NewFormat) {};

    void Mute();
    void UnMute();
    CSliderSetting* GetVolume();
    CSliderSetting* GetBalance();

    std::string IDString();

    //from CSettingsHolder
    void CreateSettings(LPCSTR IniSection);
    void ChannelChange(int PreChange, int OldChannel, int NewChannel) {};

    SmartPtr<ITuner> GetTuner() {return NULL;}

    SmartPtr<CSettingsHolder> GetSettingsPage() {return 0L;}

    void SetSourceAsCurrent();

    void Pause() {return;};
    void UnPause() {return;};

    BOOL HasMediaControl() {return FALSE;};

    int GetCurrentPos();
    void SetPos(int pos);
    int GetDuration();
    void ChangePos(int delta_sec);

    BOOL IsAudioMixerAccessAllowed() {return FALSE;};

    BOOL IsInitialSetup() {return m_InitialSetup;};

protected:
    CDShowGraph *m_pDSGraph;
    long m_CurrentX;
    long m_CurrentY;

    ///Array for picture history.
    TPicture m_PictureHistory[MAX_PICTURE_HISTORY];

    ///number of frames dropped at last call of updateDroppedFields()
    int m_LastNumDroppedFrames;

    CRITICAL_SECTION m_hOutThreadSync;

    ///used for measuring how long it takes for dscaler to process one field
    DWORD m_dwRendStartTime;

    std::string m_IDString;

    SmartPtr<CStringSetting> m_AudioDevice;

private:
    void UpdateDroppedFields();

    BOOL m_InitialSetup;

    DEFINE_SLIDER_CALLBACK_SETTING(CDSSourceBase, Volume);
    DEFINE_SLIDER_CALLBACK_SETTING(CDSSourceBase, Balance);
};

#endif // !defined(AFX_DSSOURCEBASE_H__E88C9FB3_4694_419D_AD7C_22F2E17260B4__INCLUDED_)
