/////////////////////////////////////////////////////////////////////////////
// $Id: DSSource.h,v 1.11 2002-04-16 15:33:53 tobbej Exp $
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
// Revision 1.10  2002/04/15 22:57:27  laurentg
// Automatic switch to "square pixels" AR mode when needed
//
// Revision 1.9  2002/04/07 14:52:13  tobbej
// fixed race when changing resolution
// improved error handling
//
// Revision 1.8  2002/02/09 02:49:23  laurentg
// Overscan now stored in a setting of the source
//
// Revision 1.7  2002/02/07 22:08:23  tobbej
// changed for new file input
//
// Revision 1.6  2002/02/05 17:27:17  tobbej
// fixed alignment problems
// update dropped/drawn fields stats
//
// Revision 1.5  2002/02/03 20:05:58  tobbej
// made video format menu work
// fixed color controls
// enable/disable menu items
//
// Revision 1.4  2002/02/03 11:00:43  tobbej
// added support for picure controls
// fixed menu handling
// fixed GetNextField to work with dshow filter
//
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
 */
class CDSSource : public CSource
{
public:
	CDSSource(string device,string deviceName);
	CDSSource();
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

	ISetting* GetOverscan();

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
	BOOL IsAccessAllowed();
	LPCSTR GetMenuLabel();

    void SetOverscan();
	
	BOOL OpenMediaFile(LPCSTR FileName, BOOL NewPlayList);
	void DecodeVBI(TDeinterlaceInfo* pInfo){};

    BOOL HasSquarePixels() {return FALSE;};

private:
	void updateDroppedFields();

	CDShowGraph *m_pDSGraph;
	string m_device;
	string m_deviceName;
	
	bool m_bIsFileSource;
	string m_filename;

	long m_currentX;
	long m_currentY;

	///Array for picture history, implemented as a circular queue
	TPicture m_pictureHistory[MAX_PICTURE_HISTORY];
	BYTE* m_unalignedBuffers[MAX_PICTURE_HISTORY];

	///Index of next position in m_pictureHistory to be used
	int m_pictureHistoryPos;
	///size allocated for each field in the history
	long m_cbFieldSize;
	
	int m_bytePerPixel;
	
	bool m_bProcessingFirstField;

	///number of frames dropped at last call of updateDroppedFields()
	int m_lastNumDroppedFrames;

	CRITICAL_SECTION m_hOutThreadSync;

	DEFINE_SLIDER_CALLBACK_SETTING(CDSSource, Brightness);
	DEFINE_SLIDER_CALLBACK_SETTING(CDSSource, Contrast);
	DEFINE_SLIDER_CALLBACK_SETTING(CDSSource, Hue);
	DEFINE_SLIDER_CALLBACK_SETTING(CDSSource, Saturation);
	DEFINE_SLIDER_CALLBACK_SETTING(CDSSource, Overscan);
};

#endif // !defined(AFX_DSSOURCE_H__C552BD3D_0240_4408_805B_0783992D937E__INCLUDED_)
