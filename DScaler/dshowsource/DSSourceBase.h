/////////////////////////////////////////////////////////////////////////////
// $Id: DSSourceBase.h,v 1.14 2003-01-19 19:36:26 laurentg Exp $
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
// Revision 1.13  2003/01/19 11:07:40  laurentg
// New methods GetInitialWidth and GetInitialHeight to store the initial size before resizing in DScaler (for stills)
//
// Revision 1.12  2003/01/15 20:57:00  tobbej
// changed some comments
//
// Revision 1.11  2002/11/06 20:49:01  adcockj
// Changes for DSShow compiliation
//
// Revision 1.10  2002/10/29 20:01:07  tobbej
// fixed GetSettingsPage()
//
// Revision 1.9  2002/10/26 08:38:59  tobbej
// fixed compile problems by reverting HasTuner and SetTunerFrequency
//
// Revision 1.8  2002/09/25 15:11:12  adcockj
// Preliminary code for format specific support for settings per channel
//
// Revision 1.7  2002/09/24 17:15:37  tobbej
// support for volume, balance and mute/unmute
//
// Revision 1.6  2002/09/14 17:05:49  tobbej
// implemented audio output device selection
//
// Revision 1.5  2002/09/04 17:07:16  tobbej
// renamed some variables
// fixed bug in Reset(), it called the wrong Start()
//
// Revision 1.4  2002/08/27 22:09:39  kooiman
// Add get/set input for DS capture source.
//
// Revision 1.3  2002/08/26 18:25:10  adcockj
// Fixed problem with PAL/NTSC detection
//
// Revision 1.2  2002/08/21 20:29:20  kooiman
// Fixed settings and added setting for resolution. Fixed videoformat==lastone in dstvtuner.
//
// Revision 1.1  2002/08/20 16:21:28  tobbej
// split CDSSource into 3 different classes
//
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
	void GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming);
	void Start();
	void Stop();
	void Reset();

	BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam);

    void SetFormat(eVideoFormat NewFormat) {};
	
	void Mute();
	void UnMute();
	ISetting* GetVolume();
	ISetting* GetBalance();
	
	LPCSTR IDString();

	//from CSettingsHolder
	void CreateSettings(LPCSTR IniSection);
    void ChannelChange(int PreChange, int OldChannel, int NewChannel) {};

	ITuner* GetTuner() {return NULL;}
	
	CTreeSettingsPage* GetTreeSettingsPage() {return NULL;}
	
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
	
	std::string m_AudioDevice;

private:
	void UpdateDroppedFields();
	CString m_IniSection;

	DEFINE_SLIDER_CALLBACK_SETTING(CDSSourceBase, Volume);
	DEFINE_SLIDER_CALLBACK_SETTING(CDSSourceBase, Balance);
};

#endif // !defined(AFX_DSSOURCEBASE_H__E88C9FB3_4694_419D_AD7C_22F2E17260B4__INCLUDED_)
