/////////////////////////////////////////////////////////////////////////////
// $Id: CaptureDevice.h,v 1.13 2002-10-29 19:32:22 tobbej Exp $
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
// Revision 1.12  2002/10/27 12:18:49  tobbej
// renamed GetTVTuner
// changed FindInterface
//
// Revision 1.11  2002/09/24 17:22:19  tobbej
// a few renamed function
//
// Revision 1.10  2002/09/14 17:03:11  tobbej
// implemented audio output device selection
//
// Revision 1.9  2002/08/14 22:03:23  kooiman
// Added TV tuner support for DirectShow capture devices
//
// Revision 1.8  2002/07/17 19:18:09  tobbej
// try to connect the videoport pin first if there is one.
// commented out unused ir code
//
// Revision 1.7  2002/04/16 15:30:53  tobbej
// fixed dropped frames counter, previously it didnt find the IAMDroppedFrames when it was on one of the output pins
//
// Revision 1.6  2002/03/15 23:08:59  tobbej
// changed dropped frames counter to include dropped frames in source filter.
// experimented a bit with btwincap:s ir support
//
// Revision 1.5  2002/02/13 16:58:22  tobbej
// changed some comments
//
// Revision 1.4  2002/02/07 22:05:43  tobbej
// new classes for file input
// rearanged class inheritance a bit
//
// Revision 1.3  2002/02/05 17:24:13  tobbej
// changed javadoc coments
//
// Revision 1.2  2002/02/03 11:04:01  tobbej
// pal/ntsc selection support
// picture controls support
//
// Revision 1.1  2001/12/17 19:28:26  tobbej
// class for capture devices
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file CaptureDevice.h interface for the CDShowCaptureDevice class.
 */

#if !defined(AFX_DSHOWCAPTUREDEVICE_H__50B8522B_CD99_471C_A39D_9BE98CACC7EB__INCLUDED_)
#define AFX_DSHOWCAPTUREDEVICE_H__50B8522B_CD99_471C_A39D_9BE98CACC7EB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DShowBaseSource.h"
#include "SingleCrossbar.h"
#include "DShowDirectTuner.h"

/**
 * Exception class for CDShowCaptureDevice
 * @see CDShowException
 */
class CDShowCaptureDeviceException: public CDShowException
{
public:
	CDShowCaptureDeviceException(CString msg,HRESULT hr):CDShowException(msg,hr) {};
	CDShowCaptureDeviceException(CString msg):CDShowException(msg) {};
};

 
/**
 * Class for managing a Direct Show capture device.
 */
class CDShowCaptureDevice : public CDShowBaseSource  
{
public:
	CDShowCaptureDevice(IGraphBuilder *pGraph,string device,string deviceName);
	virtual ~CDShowCaptureDevice();
	
	eDSObjectType getObjectType() {return DSHOW_TYPE_SOURCE_CAPTURE;}

	void Connect(CComPtr<IBaseFilter> VideoFilter);
	bool IsConnected() {return m_bIsConnected;};
	long GetNumDroppedFrames();

	/**
	 * This method returns a pointer to the crossbar if any.
	 * If no crossbar is present it returns NULL
	 * @return pointer to the crossbar
	 */
	CDShowBaseCrossbar* getCrossbar();

	// IAMTVTuner
	CDShowDirectTuner *GetTuner();
	
	// IAMAnalogVideoDecoder
	bool hasVideoDec() {return m_pAVideoDec!=NULL;}
	long getSupportedTVFormats();
	long getTVFormat();
	void putTVFormat(long format);
	bool IsHorizontalLocked();

	// IAMVideoProcAmp
	bool hasVideoProcAmp() {return m_pVideoProcAmp!=NULL;}
	void set(long prop,long value,long flags);
	void get(long prop,long *pValue,long *pFlags=NULL);
	void getRange(long prop,long *pMin,long *pMax, long *pStepSize=NULL,long *pDefault=NULL,long *pFlags=NULL);
	

	//experimental btwincap style ir support
	/*bool driverSupportsIR();
	bool isRemotePresent();
	ULONG getRemoteCode();*/

private:
	
	void findIAMDroppedFrames(CComPtr<IBaseFilter> filter);
	bool m_bIsConnected;
	CDShowBaseCrossbar *m_pCrossbar;
	CDShowDirectTuner *m_pTVTuner;

	///video capture device
	CComPtr<IBaseFilter> m_vidDev;
	CComPtr<ICaptureGraphBuilder2> m_pBuilder;

	CComPtr<IAMAnalogVideoDecoder> m_pAVideoDec;
	CComPtr<IAMVideoProcAmp> m_pVideoProcAmp;

	CComPtr<IAMDroppedFrames> m_pDroppedFrames;
};

#endif // !defined(AFX_DSHOWCAPTUREDEVICE_H__50B8522B_CD99_471C_A39D_9BE98CACC7EB__INCLUDED_)
