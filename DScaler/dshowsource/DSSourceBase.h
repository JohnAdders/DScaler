/////////////////////////////////////////////////////////////////////////////
// $Id: DSSourceBase.h,v 1.3 2002-08-26 18:25:10 adcockj Exp $
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

class CDSSourceBase : public CSource  
{
public:
	CDSSourceBase(long SetMessage, long MenuId);
	virtual ~CDSSourceBase();
	
	int GetWidth();
	int GetHeight();
	void GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming);
	void Start(int Run = 1);
	void Stop();
	void Reset();

	BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam);

    void SetFormat(eVideoFormat NewFormat) = {};
	
	void Mute();
	void UnMute();
	ISetting* GetVolume();
	ISetting* GetBalance();
	
	LPCSTR IDString();
	
protected:
	CDShowGraph *m_pDSGraph;
	long m_currentX;
	long m_currentY;
	
	///Array for picture history.
	TPicture m_PictureHistory[MAX_PICTURE_HISTORY];

	///number of frames dropped at last call of updateDroppedFields()
	int m_lastNumDroppedFrames;

	CRITICAL_SECTION m_hOutThreadSync;

	///used for measuring how long it takes for dscaler to process one field
	DWORD m_dwRendStartTime;

	std::string  m_IDString;

private:
	void UpdateDroppedFields();
};

#endif // !defined(AFX_DSSOURCEBASE_H__E88C9FB3_4694_419D_AD7C_22F2E17260B4__INCLUDED_)
