/////////////////////////////////////////////////////////////////////////////
// $Id: DSFileSource.h,v 1.3 2002-09-14 17:03:11 tobbej Exp $
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
// Revision 1.2  2002/08/27 22:09:39  kooiman
// Add get/set input for DS capture source.
//
// Revision 1.1  2002/08/20 16:21:28  tobbej
// split CDSSource into 3 different classes
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @fileDSFileSource.h interface for the CDSFileSource class.
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
	ISetting* GetOverscan() {return NULL;};
	void SetOverscan();

	LPCSTR GetStatus();
	void HandleTimerMessages(int TimerId);
	BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam);

	eTunerId GetTunerId();
	BOOL HasTuner();
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

private:
	///the file this source uses
	std::string m_filename;
};

#endif // !defined(AFX_DSFILESOURCE_H__C14F892B_5440_4F47_9EEE_EA140CA9534A__INCLUDED_)
