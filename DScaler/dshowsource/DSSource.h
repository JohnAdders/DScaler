/////////////////////////////////////////////////////////////////////////////
// $Id: DSSource.h,v 1.4 2002-02-03 11:00:43 tobbej Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Torbjörn Jansson.  All rights reserved.
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
// Revision 1.3  2002/02/02 01:35:18  laurentg
// Method UpdateMenu added
//
// Revision 1.2  2001/12/17 19:39:38  tobbej
// implemented the picture history and field management
// crossbar support.
//
// Revision 1.1  2001/12/09 22:01:48  tobbej
// experimental dshow support, doesnt work yet
// define WANT_DSHOW_SUPPORT if you want to try it
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file DSSource.h interface for the CDSSource class.
 */

#if !defined(AFX_DSSOURCE_H__C552BD3D_0240_4408_805B_0783992D937E__INCLUDED_)
#define AFX_DSSOURCE_H__C552BD3D_0240_4408_805B_0783992D937E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Source.h"
#include "DSGraph.h"

/**
 * This source controls a Direct show filter graph.
 * @see CDSProvider
 * @see CDShowGraph
 * @bug alignment problems in picture history (SSE only)
 */
class CDSSource : public CSource
{
public:
	CDSSource(string device,string deviceName);
	virtual ~CDSSource();

	//from CSettingsHolder
	void CreateSettings(LPCSTR IniSection);

	//from CSource
	void Start();
	void Stop();
	void Reset();

	void GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming);
	BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam);
	void HandleTimerMessages(int TimerId);

	LPCSTR GetStatus();

	void Mute(){};
	void UnMute(){};
	ISetting* GetVolume() {return NULL;};
	ISetting* GetBalance() {return NULL;};

	ISetting* GetBrightness();
	ISetting* GetContrast();
	ISetting* GetHue();
	ISetting* GetSaturation();
	ISetting* GetSaturationU() {return NULL;};
	ISetting* GetSaturationV() {return NULL;};

	eVideoFormat GetFormat();
	BOOL IsInTunerMode();
	int GetWidth();
	int GetHeight();
	
	eTunerId GetTunerId() {return TUNER_ABSENT;}
	BOOL HasTuner();
	BOOL SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat);
	BOOL IsVideoPresent();

    void UpdateMenu() {return;};
	void SetMenu(HMENU hMenu);
	BOOL IsAccessAllowed() {return TRUE;};
	LPCSTR GetMenuLabel();
	
	BOOL OpenMediaFile(LPCSTR FileName, BOOL NewPlayList) {return FALSE;};
	void DecodeVBI(TDeinterlaceInfo* pInfo){};

private:
	DEFINE_SLIDER_CALLBACK_SETTING(CDSSource, Brightness);
	DEFINE_SLIDER_CALLBACK_SETTING(CDSSource, Contrast);
	DEFINE_SLIDER_CALLBACK_SETTING(CDSSource, Hue);
	DEFINE_SLIDER_CALLBACK_SETTING(CDSSource, Saturation);
	

	int FindMenuID(CMenu *menu,UINT menuID);

	CDShowGraph *m_pDSGraph;
	string m_device;
	string m_deviceName;
	long m_currentX;
	long m_currentY;

	///Array for picture history, implemented as a circular queue
	TPicture m_pictureHistory[MAX_PICTURE_HISTORY];
	///Index of next position in m_pictureHistory to be used
	int m_pictureHistoryPos;
	///size allocated for each field in the history
	long m_cbFieldSize;
	
	int m_bytePerPixel;
	
	bool m_bProcessingFirstField;
};

#endif // !defined(AFX_DSSOURCE_H__C552BD3D_0240_4408_805B_0783992D937E__INCLUDED_)
