/////////////////////////////////////////////////////////////////////////////
// $Id: DSSource.h,v 1.37 2003-01-16 13:30:49 adcockj Exp $
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
// Revision 1.36  2003/01/10 17:38:45  adcockj
// Interrim Check in of Settings rewrite
//  - Removed SETTINGSEX structures and flags
//  - Removed Seperate settings per channel code
//  - Removed Settings flags
//  - Cut away some unused features
//
// Revision 1.35  2003/01/08 21:47:14  laurentg
// New setting for analogue blanking by source
//
// Revision 1.34  2003/01/07 23:31:24  laurentg
// New overscan settings
//
// Revision 1.33  2003/01/06 21:34:35  tobbej
// implemented GetFormat (not working yet)
// implemented fm radio support
//
// Revision 1.32  2002/12/05 21:02:55  tobbej
// fixed initial channel change so it tunes properly to the last used channel.
// renamed video format to resolution in settings dialog.
// changed so the default list of resolutions is always used if resolution menu is empty.
// removed some unused channel change notification code.
//
// Revision 1.31  2002/10/29 19:32:22  tobbej
// new tuner class for direct tuning to a frequency
// implemented IsVideoPresent, channel scaning shoud work now
//
// Revision 1.30  2002/10/27 12:17:29  tobbej
// implemented ITuner
//
// Revision 1.29  2002/10/26 08:38:59  tobbej
// fixed compile problems by reverting HasTuner and SetTunerFrequency
//
// Revision 1.28  2002/10/22 04:10:12  flibuste2
// -- Modified CSource to include virtual ITuner* GetTuner();
// -- Modified HasTuner() and GetTunerId() when relevant
//
// Revision 1.27  2002/09/28 13:36:15  kooiman
// Added sender object to events and added setting flag to treesettingsgeneric.
//
// Revision 1.26  2002/09/26 10:35:34  kooiman
// Use new event code.
//
// Revision 1.25  2002/09/11 16:42:33  tobbej
// fixed so resolutions submenu can be empty and not always replaced with defaults if all resolutions is removed
//
// Revision 1.24  2002/09/04 17:10:24  tobbej
// renamed some variables
// new video format configuration dialog (resolution)
//
// Revision 1.23  2002/08/27 22:09:39  kooiman
// Add get/set input for DS capture source.
//
// Revision 1.22  2002/08/21 20:29:20  kooiman
// Fixed settings and added setting for resolution. Fixed videoformat==lastone in dstvtuner.
//
// Revision 1.21  2002/08/20 16:21:28  tobbej
// split CDSSource into 3 different classes
//
// Revision 1.20  2002/08/15 14:20:11  kooiman
// Improved tuner support. Added setting for video input.
//
// Revision 1.19  2002/08/14 22:03:23  kooiman
// Added TV tuner support for DirectShow capture devices
//
// Revision 1.18  2002/08/13 21:04:42  kooiman
// Add IDString() to Sources for identification purposes.
//
// Revision 1.17  2002/08/11 12:06:19  laurentg
// Cut BT Card setup and general hardware setup in two different windows
//
// Revision 1.16  2002/08/10 16:54:25  tobbej
// use square pixels by default
//
// Revision 1.15  2002/08/09 13:35:11  laurentg
// Processor speed and trade off settings moved from BT source settings to DScaler settings
//
// Revision 1.14  2002/08/05 14:07:56  kooiman
// *** empty log message ***
//
// Revision 1.13  2002/07/06 16:48:11  tobbej
// new field buffering
//
// Revision 1.12  2002/05/24 15:18:32  tobbej
// changed filter properties dialog to include progpertypages from the pins
// fixed input source status
// fixed dot infront of start/pause/stop menu entries
// changed overscan settings a bit
// experimented a bit with measuring time for dscaler to process one field
//
// Revision 1.11  2002/04/16 15:33:53  tobbej
// added overscan for capture devices
// added audio mute/unmute when starting and stopping source
// added waitForNextField
//
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
 * @file DSSource.h interface for the CDSCaptureSource class.
 */

#if !defined(AFX_DSSOURCE_H__C552BD3D_0240_4408_805B_0783992D937E__INCLUDED_)
#define AFX_DSSOURCE_H__C552BD3D_0240_4408_805B_0783992D937E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DSSourceBase.h"
#include <map>

/**
 * This source controls a DirectShow capture graph.
 * @see CDSProvider
 * @see CDShowGraph
 * @see CDSSourceBase
 */
class CDSCaptureSource : public CDSSourceBase
{
public:
	CDSCaptureSource(string device,string deviceName);
	virtual ~CDSCaptureSource();
    
	//from CSettingsHolder
	void CreateSettings(LPCSTR IniSection);

	BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam);
	void HandleTimerMessages(int TimerId);

	LPCSTR GetStatus();

	ISetting* GetBrightness();
	ISetting* GetContrast();
	ISetting* GetHue();
	ISetting* GetSaturation();
	ISetting* GetSaturationU() {return NULL;};
	ISetting* GetSaturationV() {return NULL;};

    ISetting* GetAnalogueBlanking() {return NULL;};
	ISetting* GetTopOverscan();
	ISetting* GetBottomOverscan();
	ISetting* GetLeftOverscan();
	ISetting* GetRightOverscan();

	eVideoFormat GetFormat();
	BOOL IsInTunerMode();
	
	BOOL SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat);
	BOOL IsVideoPresent();

	int  NumInputs(eSourceInputType InputType);
	BOOL SetInput(eSourceInputType InputType, int Nr);
	int GetInput(eSourceInputType InputType);
	const char* GetInputName(eSourceInputType InputType, int Nr);
	BOOL InputHasTuner(eSourceInputType InputType, int Nr);


	void UpdateMenu() {return;};
	void SetMenu(HMENU hMenu);
	BOOL IsAccessAllowed();
	LPCSTR GetMenuLabel();

	void SetOverscan();
	
	BOOL OpenMediaFile(LPCSTR FileName, BOOL NewPlayList);
	void DecodeVBI(TDeinterlaceInfo* pInfo){};
	
	///@todo this probably have to be changed
	BOOL HasSquarePixels() {return FALSE;};

	void ChangeSettingsBasedOnHW(int ProcessorSpeed, int TradeOff) {;};

	static void ChannelChange(void *pThis,int PreChange,int OldChannel,int NewChannel);
	
	void Start();
	void Stop();

	ITuner* GetTuner();

private:

	/**
	 * Dummy ITuner.
	 * The purpose of this implementation of ITuner is only to get the
	 * channels menu active.
	 */
	class CDummyTuner : public ITuner
	{
	public:
		eTunerId GetTunerId();
		eVideoFormat GetDefaultVideoFormat();
		bool HasRadio() const;
		bool SetRadioFrequency(long nFrequency);
		bool SetTVFrequency(long nFrequency, eVideoFormat videoFormat);
		long GetFrequency();
		eTunerLocked IsLocked();
		eTunerAFCStatus GetAFCStatus(long &nFreqDeviation);
	};

	///coverts from dscaler eVideoFormat to directshow AnalogVideoStandard
	AnalogVideoStandard ConvertVideoStd(eVideoFormat fmt);
	
	///coverts from directshow AnalogVideoStandard to dscaler eVideoFormat
	eVideoFormat ConvertVideoStd(AnalogVideoStandard fmt);

	///resets m_VideoFmt to default
	void CreateDefaultVideoFmt();

	string m_Device;
	string m_DeviceName;
	
	vector<CDShowGraph::CVideoFormat> m_VideoFmt;

	BOOL m_HaveInputList;
	vector<int> m_VideoInputList;
	vector<int> m_AudioInputList;
	CDummyTuner m_Tuner;

	DEFINE_SLIDER_CALLBACK_SETTING(CDSCaptureSource, Brightness);
	DEFINE_SLIDER_CALLBACK_SETTING(CDSCaptureSource, Contrast);
	DEFINE_SLIDER_CALLBACK_SETTING(CDSCaptureSource, Hue);
	DEFINE_SLIDER_CALLBACK_SETTING(CDSCaptureSource, Saturation);
	DEFINE_SLIDER_CALLBACK_SETTING(CDSCaptureSource, TopOverscan);
	DEFINE_SLIDER_CALLBACK_SETTING(CDSCaptureSource, BottomOverscan);
	DEFINE_SLIDER_CALLBACK_SETTING(CDSCaptureSource, LeftOverscan);
	DEFINE_SLIDER_CALLBACK_SETTING(CDSCaptureSource, RightOverscan);

	DEFINE_SLIDER_CALLBACK_SETTING(CDSCaptureSource, VideoInput);
	DEFINE_SLIDER_CALLBACK_SETTING(CDSCaptureSource, AudioInput);

	DEFINE_SLIDER_CALLBACK_SETTING(CDSCaptureSource, Resolution);
protected:
    void ChangeDefaultsForVideoFormat(BOOL bDontSetValue) {};
    void ChangeDefaultsForVideoInput(BOOL bDontSetValue) {};
    void ChangeDefaultsForAudioInput(BOOL bDontSetValue) {};
};

#endif // !defined(AFX_DSSOURCE_H__C552BD3D_0240_4408_805B_0783992D937E__INCLUDED_)
