/////////////////////////////////////////////////////////////////////////////
// $Id: CaptureDevice.h,v 1.3 2002-02-05 17:24:13 tobbej Exp $
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
// Revision 1.2  2002/02/03 11:04:01  tobbej
// pal/ntsc selection support
// picture controls support
//
// Revision 1.1  2001/12/17 19:28:26  tobbej
// class for capture devices
//
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

#include "DSObject.h"
#include "SingleCrossbar.h"

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
 * fixme
 */
class CDShowCaptureDevice : public CDShowObject  
{
public:
	CDShowCaptureDevice(IGraphBuilder *pGraph,string device,string deviceName);
	virtual ~CDShowCaptureDevice();
	
	void connect(CComPtr<IBaseFilter> filter);
	bool isConnected() {return m_bIsConnected;};

	/**
	 * This method returns a pointer to the crossbar if any.
	 * If no crossbar is present it returns NULL
	 * @return pointer to the crossbar
	 */
	CDShowBaseCrossbar* getCrossbar();
	
	// IAMAnalogVideoDecoder
	bool hasVideoDec() {return m_pAVideoDec!=NULL;}
	long getSupportedTVFormats();
	long getTVFormat();
	void putTVFormat(long format);

	// IAMVideoProcAmp
	bool hasVideoProcAmp() {return m_pVideoProcAmp!=NULL;}
	void set(long prop,long value,long flags);
	void get(long prop,long *pValue,long *pFlags=NULL);
	void getRange(long prop,long *pMin,long *pMax, long *pStepSize=NULL,long *pDefault=NULL,long *pFlags=NULL);

private:
	bool m_bIsConnected;
	CDShowBaseCrossbar *m_pCrossbar;

	///video capture device
	CComPtr<IBaseFilter> m_vidDev;
	CComPtr<ICaptureGraphBuilder2> m_pBuilder;

	CComPtr<IAMAnalogVideoDecoder> m_pAVideoDec;
	CComPtr<IAMVideoProcAmp> m_pVideoProcAmp;
};

#endif // !defined(AFX_DSHOWCAPTUREDEVICE_H__50B8522B_CD99_471C_A39D_9BE98CACC7EB__INCLUDED_)
