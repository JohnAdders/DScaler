/////////////////////////////////////////////////////////////////////////////
// $Id$
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
#include "DShowTVAudio.h"

/**
 * Exception class for CDShowCaptureDevice
 * @see CDShowException
 */
class CDShowCaptureDeviceException: public CDShowException
{
public:
    CDShowCaptureDeviceException(const char* msg,HRESULT hr = S_OK):CDShowException(msg,hr) {};
};

 
/**
 * Class for managing a Direct Show capture device.
 */
class CDShowCaptureDevice : public CDShowBaseSource  
{
public:
    CDShowCaptureDevice(IGraphBuilder *pGraph, std::string device, std::string deviceName,bool bConnectAudio);
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
    long GetSupportedTVFormats();
    AnalogVideoStandard GetTVFormat();
    void PutTVFormat(AnalogVideoStandard format);
    bool IsHorizontalLocked();

    // IAMVideoProcAmp
    bool hasVideoProcAmp() {return m_pVideoProcAmp!=NULL;}
    void set(long prop,long value,long flags);
    void get(long prop,long *pValue,long *pFlags=NULL);
    void getRange(long prop,long *pMin,long *pMax, long *pStepSize=NULL,long *pDefault=NULL,long *pFlags=NULL);
    
    /**
     * @return pointer to CDShowTVAudio class or NULL if there is no tvaudio
     */
    CDShowTVAudio* GetTVAudio();

    //experimental btwincap style ir support
    /*bool driverSupportsIR();
    bool isRemotePresent();
    ULONG getRemoteCode();*/

private:
    
    void findIAMDroppedFrames(CComPtr<IBaseFilter> filter);
    bool m_bIsConnected;
    bool m_bConnectAudio;
    CDShowBaseCrossbar *m_pCrossbar;
    CDShowDirectTuner *m_pTVTuner;
    CDShowTVAudio *m_pTVAudio;

    ///video capture device
    CComPtr<IBaseFilter> m_vidDev;
    CComPtr<ICaptureGraphBuilder2> m_pBuilder;

    CComPtr<IAMAnalogVideoDecoder> m_pAVideoDec;
    CComPtr<IAMVideoProcAmp> m_pVideoProcAmp;

    CComPtr<IAMDroppedFrames> m_pDroppedFrames;
};

#endif // !defined(AFX_DSHOWCAPTUREDEVICE_H__50B8522B_CD99_471C_A39D_9BE98CACC7EB__INCLUDED_)
