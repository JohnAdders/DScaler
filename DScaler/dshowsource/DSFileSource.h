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
 * @file DSFileSource.h interface for the CDSFileSource class.
 */

#if !defined(AFX_DSFILESOURCE_H__C14F892B_5440_4F47_9EEE_EA140CA9534A__INCLUDED_)
#define AFX_DSFILESOURCE_H__C14F892B_5440_4F47_9EEE_EA140CA9534A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DSSourceBase.h"

/**
 * This source handles DirectShow files.
 */
class CDSFileSource : public CDSSourceBase
{
public:
    CDSFileSource();
    virtual ~CDSFileSource();

    void CreateSettings(LPCTSTR IniSection);

    BOOL IsAccessAllowed();
    BOOL OpenMediaFile(const tstring& FileName, BOOL NewPlayList);

    CSliderSetting* GetBrightness(){return NULL;};
    CSliderSetting* GetContrast(){return NULL;};
    CSliderSetting* GetHue(){return NULL;};
    CSliderSetting* GetSaturation(){return NULL;};
    CSliderSetting* GetSaturationU() {return NULL;};
    CSliderSetting* GetSaturationV() {return NULL;};
    CYesNoSetting* GetAnalogueBlanking() {return NULL;};
    CSliderSetting* GetTopOverscan() {return NULL;};
    CSliderSetting* GetBottomOverscan() {return NULL;};
    CSliderSetting* GetLeftOverscan() {return NULL;};
    CSliderSetting* GetRightOverscan() {return NULL;};
    CSliderSetting* GetHDelay() {return NULL;};
    CSliderSetting* GetVDelay() {return NULL;};
    void SetAspectRatioData();

    tstring GetStatus();
    void HandleTimerMessages(int TimerId);
    BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam);

    BOOL SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat);
    BOOL IsInTunerMode();

    BOOL IsVideoPresent();
    void DecodeVBI(TDeinterlaceInfo* pInfo){};
    BOOL HasSquarePixels();
    void ChangeSettingsBasedOnHW(int ProcessorSpeed, int TradeOff) {;};
    eVideoFormat GetFormat();

    int  NumInputs(eSourceInputType InputType) { return 0; };
    BOOL SetInput(eSourceInputType InputType, int Nr) { return FALSE; };
    int GetInput(eSourceInputType InputType) { return -1; };
    tstring GetInputName(eSourceInputType InputType, int Nr) { return _T(""); };
    BOOL InputHasTuner(eSourceInputType InputType, int Nr) { return FALSE; };

    void UpdateMenu();
    void SetMenu(HMENU hMenu);
    tstring GetMenuLabel();

    void Start();

    void Pause();
    void UnPause();

    BOOL HasMediaControl() {return TRUE;};

protected:
    void ChangeDefaultsForVideoFormat(BOOL bDontSetValue) {};
    void ChangeDefaultsForVideoInput(BOOL bDontSetValue) {};
    void ChangeDefaultsForAudioInput(BOOL bDontSetValue) {};

private:
    ///the file this source uses
    tstring m_filename;
};

#endif // !defined(AFX_DSFILESOURCE_H__C14F892B_5440_4F47_9EEE_EA140CA9534A__INCLUDED_)
