/////////////////////////////////////////////////////////////////////////////
// $Id: CaptureDevice.h,v 1.1 2001-12-17 19:28:26 tobbej Exp $
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
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file DShowCaptureDevice.h interface for the CDShowCaptureDevice class.
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

private:
	bool m_bIsConnected;
	CDShowBaseCrossbar *m_pCrossbar;

	///video capture device
	CComPtr<IBaseFilter> m_vidDev;
	CComPtr<ICaptureGraphBuilder2> m_pBuilder;
};

#endif // !defined(AFX_DSHOWCAPTUREDEVICE_H__50B8522B_CD99_471C_A39D_9BE98CACC7EB__INCLUDED_)
