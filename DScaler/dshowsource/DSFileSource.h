/////////////////////////////////////////////////////////////////////////////
// $Id: DSFileSource.h,v 1.16 2003-10-27 10:39:56 adcockj Exp $
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
// Change Log
//
// Date          Developer             Changes
//
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.15  2003/08/12 19:10:05  laurentg
// Move some methods from CDSFileSource to CDSSourceBase
//
// Revision 1.14  2003/08/11 22:49:27  laurentg
// Time slider in the media player toolbar
//
// Revision 1.13  2003/07/22 22:30:20  laurentg
// Correct handling of pause (P key) for video file playing
//
// Revision 1.12  2003/01/18 13:54:31  laurentg
// New methods GetHDelay and GetVDelay
//
// Revision 1.11  2003/01/18 10:49:10  laurentg
// SetOverscan renamed SetAspectRatioData
//
// Revision 1.10  2003/01/16 13:30:49  adcockj
// Fixes for various settings problems reported by Laurent 15/Jan/2003
//
// Revision 1.9  2003/01/10 17:38:43  adcockj
// Interrim Check in of Settings rewrite
//  - Removed SETTINGSEX structures and flags
//  - Removed Seperate settings per channel code
//  - Removed Settings flags
//  - Cut away some unused features
//
// Revision 1.8  2003/01/08 20:49:49  laurentg
// New settings for analogue blanking by source
//
// Revision 1.7  2003/01/07 23:31:23  laurentg
// New overscan settings
//
// Revision 1.6  2002/10/26 08:38:59  tobbej
// fixed compile problems by reverting HasTuner and SetTunerFrequency
//
// Revision 1.5  2002/10/22 04:09:49  flibuste2
// -- Modified CSource to include virtual ITuner* GetTuner();
// -- Modified HasTuner() and GetTunerId() when relevant
//
// Revision 1.4  2002/09/29 09:16:25  tobbej
// changed GetStatus to return filename only
//
// Revision 1.3  2002/09/14 17:03:11  tobbej
// implemented audio output device selection
//
// Revision 1.2  2002/08/27 22:09:39  kooiman
// Add get/set input for DS capture source.
//
// Revision 1.1  2002/08/20 16:21:28  tobbej
// split CDSSource into 3 different classes
//
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
	BOOL OpenMediaFile(LPCSTR FileName, BOOL NewPlayList);

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

	LPCSTR GetStatus();
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
	const char* GetInputName(eSourceInputType InputType, int Nr) { return NULL; };
	BOOL InputHasTuner(eSourceInputType InputType, int Nr) { return FALSE; };
	
	void UpdateMenu();
	void SetMenu(HMENU hMenu);
	LPCSTR GetMenuLabel();

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
	///used as a temporary storage of status text
	std::string m_status;
};

#endif // !defined(AFX_DSFILESOURCE_H__C14F892B_5440_4F47_9EEE_EA140CA9534A__INCLUDED_)
