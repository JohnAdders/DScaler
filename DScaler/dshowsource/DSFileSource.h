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
    
    void CreateSettings(LPCSTR IniSection);

    BOOL IsAccessAllowed();
    BOOL OpenMediaFile(const std::string& FileName, BOOL NewPlayList);

    ISetting* GetBrightness(){return NULL;};
    ISetting* GetContrast(){return NULL;};
    ISetting* GetHue(){return NULL;};
    ISetting* GetSaturation(){return NULL;};
    ISetting* GetSaturationU() {return NULL;};
    ISetting* GetSaturationV() {return NULL;};
    ISetting* GetAnalogueBlanking() {return NULL;};
    ISetting* GetTopOverscan() {return NULL;};
    ISetting* GetBottomOverscan() {return NULL;};
    ISetting* GetLeftOverscan() {return NULL;};
    ISetting* GetRightOverscan() {return NULL;};
    ISetting* GetHDelay() {return NULL;};
    ISetting* GetVDelay() {return NULL;};
    void SetAspectRatioData();

    std::string GetStatus();
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
    std::string GetInputName(eSourceInputType InputType, int Nr) { return ""; };
    BOOL InputHasTuner(eSourceInputType InputType, int Nr) { return FALSE; };
    
    void UpdateMenu();
    void SetMenu(HMENU hMenu);
    std::string GetMenuLabel();

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
    std::string m_filename;
};

#endif // !defined(AFX_DSFILESOURCE_H__C14F892B_5440_4F47_9EEE_EA140CA9534A__INCLUDED_)
