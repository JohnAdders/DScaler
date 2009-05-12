/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
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
//
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file MixerDev.cpp Mixer Classes
 */

#include "stdafx.h"
#include "MixerDevClasses.h"
// only do new mixer code if we can
// don't bother with t he warning if we are using VS6
#if _MSC_VER > 1200
# include "ntverp.h"
# if VER_PRODUCTMAJORVERSION > 5
#  include "Mmdeviceapi.h"
#  define __IKsJackDescription_INTERFACE_DEFINED__
#  include "Endpointvolume.h"
#  define _DO_NEW_MIXER_CODE_
#else
# pragma message ("To get vista mixer functionality you need a more up to date SDK")
# endif
#endif


using namespace std;

//----------------------------------------------------------------------
//  CMixerLineSrc
//----------------------------------------------------------------------

CMixerLineSrc::CMixerLineSrc(HMIXER hMixer, DWORD nDstIndex, DWORD nSrcIndex)
{
    MMRESULT mmresult;

    m_hMixer = hMixer;
    m_VolumeControlID = 0xFFFFFFFF;
    m_MuteControlID = 0xFFFFFFFF;

    ZeroMemory(&m_mxl, sizeof(MIXERLINE));

    m_mxl.cbStruct      = sizeof(MIXERLINE);
    m_mxl.dwDestination = nDstIndex;
    m_mxl.dwSource      = nSrcIndex;

    mmresult = mixerGetLineInfo((HMIXEROBJ)hMixer, &m_mxl, MIXER_GETLINEINFOF_SOURCE);
    if (mmresult != MMSYSERR_NOERROR || m_mxl.cControls == 0)
    {
        _tcscpy(m_mxl.szName, _T("Error"));
        return;
    }

    MIXERLINECONTROLS mxlc;
    LPMIXERCONTROL pmxctrl;

    pmxctrl = (MIXERCONTROL*)malloc(sizeof(MIXERCONTROL) * m_mxl.cControls);

    mxlc.cbStruct   = sizeof(MIXERLINECONTROLS);
    mxlc.dwLineID   = m_mxl.dwLineID;
    mxlc.cControls  = m_mxl.cControls;
    mxlc.cbmxctrl   = sizeof(MIXERCONTROL);
    mxlc.pamxctrl   = pmxctrl;

    mmresult = mixerGetLineControls((HMIXEROBJ)hMixer, &mxlc, MIXER_GETLINECONTROLSF_ALL);

    for (DWORD i = 0; i < m_mxl.cControls; i++)
    {
        if (pmxctrl[i].dwControlType == MIXERCONTROL_CONTROLTYPE_VOLUME)
        {
            m_VolumeControlID = pmxctrl[i].dwControlID;
            m_VolumeMinimum = pmxctrl[i].Bounds.dwMinimum;
            m_VolumeMaximum = pmxctrl[i].Bounds.dwMaximum;

        }
        else if (pmxctrl[i].dwControlType == MIXERCONTROL_CONTROLTYPE_MUTE)
        {
            m_MuteControlID = pmxctrl[i].dwControlID;
        }
    }

    free(pmxctrl);
}


CMixerLineSrc::~CMixerLineSrc()
{
}


const TCHAR* CMixerLineSrc::GetName()
{
    return m_mxl.szName;
}


void CMixerLineSrc::SetMute(BOOL bEnable)
{
    if (m_MuteControlID != 0xFFFFFFFF)
    {
        MixerControlDetailsSet(m_MuteControlID, bEnable);
    }
}


BOOL CMixerLineSrc::GetMute()
{
    DWORD dwEnabled = FALSE;

    if (m_MuteControlID != 0xFFFFFFFF)
    {
        MixerControlDetailsGet(m_MuteControlID, &dwEnabled);
    }

    return dwEnabled;
}


void CMixerLineSrc::SetVolume(int volumePercentage)
{
    if (m_VolumeControlID != 0xFFFFFFFF)
    {
        // sets all channels to the same volume
        MixerControlDetailsSet(m_VolumeControlID,
            MulDiv(volumePercentage, (m_VolumeMaximum - m_VolumeMinimum), 100));
    }
}


int CMixerLineSrc::GetVolume()
{
    DWORD nVolume = 0;

    if (m_VolumeControlID != 0xFFFFFFFF)
    {
        // get average volume, ignores balance
        if (MixerControlDetailsGet(m_VolumeControlID, &nVolume))
        {
            return MulDiv(nVolume, 100, m_VolumeMaximum - m_VolumeMinimum);
        }
    }

    return nVolume;
}


void CMixerLineSrc::StoreState()
{
    MixerControlDetailsGet(m_MuteControlID, &m_StoredMute);
    MixerControlDetailsGet(m_VolumeControlID, &m_StoredVolume);
}


void CMixerLineSrc::RestoreState()
{
    MixerControlDetailsSet(m_MuteControlID, m_StoredMute);
    MixerControlDetailsSet(m_VolumeControlID, m_StoredVolume);
}


BOOL CMixerLineSrc::MixerControlDetailsSet(DWORD dwControlID, DWORD dwValue)
{
    MIXERCONTROLDETAILS mxcd;

    mxcd.cbStruct       = sizeof(MIXERCONTROLDETAILS);
    mxcd.cChannels      = 1;
    mxcd.dwControlID    = dwControlID;
    mxcd.cMultipleItems = 0;
    mxcd.cbDetails      = sizeof(DWORD);
    mxcd.paDetails      = &dwValue;

    return mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mxcd,
        MIXER_SETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_HMIXER) == MMSYSERR_NOERROR;
}


BOOL CMixerLineSrc::MixerControlDetailsGet(DWORD dwControlID, LPDWORD lpdwValue)
{
    MIXERCONTROLDETAILS mxcd;

    mxcd.cbStruct       = sizeof(MIXERCONTROLDETAILS);
    mxcd.cChannels      = 1;
    mxcd.dwControlID    = dwControlID;
    mxcd.cMultipleItems = 0;
    mxcd.cbDetails      = sizeof(DWORD);
    mxcd.paDetails      = lpdwValue;

    return mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mxcd,
        MIXER_SETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_HMIXER) == MMSYSERR_NOERROR;
}


//----------------------------------------------------------------------
//  CMixerLineDst
//----------------------------------------------------------------------

CMixerLineDst::CMixerLineDst(HMIXER hMixer, DWORD nDstIndex)
{
    ZeroMemory(&m_mxl, sizeof(MIXERLINE));

    m_mxl.cbStruct = sizeof(MIXERLINE);
    m_mxl.dwDestination = nDstIndex;

    MMRESULT mmresult = mixerGetLineInfo((HMIXEROBJ)hMixer, &m_mxl,
        MIXER_GETLINEINFOF_DESTINATION);

    if (mmresult != MMSYSERR_NOERROR || m_mxl.cConnections == 0)
    {
        _tcscpy(m_mxl.szName, _T("Error"));
        return;
    }

    for (DWORD i = 0; i < m_mxl.cConnections; i++)
    {
        SmartPtr<IMixerLineSrc> NewSource = new CMixerLineSrc(hMixer, nDstIndex, i);
        m_pSourceLines.push_back(NewSource);
    }
}

CMixerLineDst::~CMixerLineDst()
{
}

const TCHAR* CMixerLineDst::GetName()
{
    return m_mxl.szName;
}


long CMixerLineDst::GetSourceCount()
{
    return m_pSourceLines.size();
}


IMixerLineSrc* CMixerLineDst::GetSourceLine(DWORD nIndex)
{
    if (nIndex >= m_pSourceLines.size())
    {
        return NULL;
    }

    return m_pSourceLines[nIndex].GetRawPointer();
}


void CMixerLineDst::StoreState()
{
    for (DWORD i = 0; i < m_pSourceLines.size(); i++)
    {
        m_pSourceLines[i]->StoreState();
    }
}


void CMixerLineDst::RestoreState()
{
    for (DWORD i = 0; i < m_pSourceLines.size(); i++)
    {
        m_pSourceLines[i]->RestoreState();
    }
}


BOOL CMixerLineDst::IsTypicalSpeakerLine()
{
    return (m_mxl.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_SPEAKERS);
}


BOOL CMixerLineDst::IsTypicalRecordingLine()
{
    return (m_mxl.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_WAVEIN);
}


//----------------------------------------------------------------------
//  CMixer
//----------------------------------------------------------------------

CMixer::CMixer(DWORD nMixerIndex)
{
    MMRESULT mmresult;

    m_hMixer = NULL;

    m_nMixerIndex = nMixerIndex;

    mmresult = mixerGetDevCaps(m_nMixerIndex, &m_mxcaps, sizeof(MIXERCAPS));
    if (mmresult != MMSYSERR_NOERROR || m_mxcaps.cDestinations == 0)
    {
        return;
    }

    mmresult = mixerOpen(&m_hMixer, m_nMixerIndex, NULL, NULL, MIXER_OBJECTF_MIXER);
    if (mmresult != MMSYSERR_NOERROR)
    {
        return;
    }

    for (DWORD i = 0; i < m_mxcaps.cDestinations; i++)
    {
        SmartPtr<IMixerLineDst> NewDest = new CMixerLineDst(m_hMixer, i);
        m_pDestinationLines.push_back(NewDest);
    }
}


CMixer::~CMixer()
{
    if (m_hMixer != NULL)
    {
        mixerClose(m_hMixer);
    }
}


const TCHAR* CMixer::GetName()
{
    return m_mxcaps.szPname;
}


DWORD CMixer::GetIndex()
{
    return m_nMixerIndex;
}


long CMixer::GetDestinationCount()
{
    return m_pDestinationLines.size();
}


IMixerLineDst* CMixer::GetDestinationLine(DWORD nIndex)
{
    if (nIndex >= m_pDestinationLines.size())
    {
        return NULL;
    }

    return m_pDestinationLines[nIndex].GetRawPointer();
}


void CMixer::StoreState()
{
    for (DWORD i = 0; i < m_pDestinationLines.size(); i++)
    {
        m_pDestinationLines[i]->StoreState();
    }
}


void CMixer::RestoreState()
{
    for (DWORD i = 0; i < m_pDestinationLines.size(); i++)
    {
        m_pDestinationLines[i]->RestoreState();
    }
}

#ifdef _DO_NEW_MIXER_CODE_
// get a name for the device
// may not be very friendly, need to test
tstring GetDeviceName(CComPtr<IMMDevice>& Device)
{
    LPWSTR Id = NULL;
    Device->GetId(&Id);
    if(Id == NULL)
    {
        throw logic_error("Can't get device name");
    }
    tstring result(UnicodeToTString(Id));
    CoTaskMemFree(Id);
    return result;
}

//----------------------------------------------------------------------
//  CVistaMixerLineSrc
//  corresponds to an input audio endpoint
//----------------------------------------------------------------------
class CVistaMixerLineSrc : public IMixerLineSrc
{
public:
    CVistaMixerLineSrc(CComPtr<IMMDevice>& Device) :
        m_Name(GetDeviceName(Device))
    {
        if(FAILED(Device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (VOID**)&m_AudioEndPointVolume)))
        {
            throw logic_error("Can't get endpoint volume control");
        }
    }

    virtual ~CVistaMixerLineSrc()
    {
    }

    const TCHAR* GetName()
    {
        return m_Name.c_str();
    }

    void SetMute(BOOL bEnable)
    {
        m_AudioEndPointVolume->SetMute(bEnable, NULL);
    }

    BOOL GetMute()
    {
        BOOL Mute(FALSE);
        m_AudioEndPointVolume->GetMute(&Mute);
        return Mute;
    }

    void SetVolume(int volumePercentage)
    {
        m_AudioEndPointVolume->SetMasterVolumeLevelScalar(float(volumePercentage / 100.0), NULL);
    }
    
    int GetVolume()
    {
        float Volume(0.0);
        m_AudioEndPointVolume->GetMasterVolumeLevelScalar(&Volume);
        return long(Volume * 100.0);
    }

    void StoreState()
    {
        m_AudioEndPointVolume->GetMasterVolumeLevelScalar(&m_StoredVolume);
        m_AudioEndPointVolume->GetMute(&m_StoredMute);
    }
    
    void RestoreState()
    {
        m_AudioEndPointVolume->SetMasterVolumeLevelScalar(m_StoredVolume, NULL);
        m_AudioEndPointVolume->SetMute(m_StoredMute, NULL);
    }

private:
    CComPtr<IAudioEndpointVolume> m_AudioEndPointVolume;
    float m_StoredVolume;
    BOOL m_StoredMute;
    tstring m_Name;
};

//----------------------------------------------------------------------
//  CVistaMixerLineDst
//  Corresponds to an output audio endpoint
//  Note that we dont try and match the input sto the outputs in terms
//  of topology, we'll see if this matters in practice after some testing
//----------------------------------------------------------------------
class CVistaMixerLineDst : public IMixerLineDst
{
public:
    CVistaMixerLineDst(CComPtr<IMMDevice>& Device, CComPtr<IMMDeviceEnumerator> DeviceEnum) :
        m_Device(Device),
        m_Name(GetDeviceName(Device))
    {
        CComPtr<IMMDeviceCollection> DeviceCollection;
        if(FAILED(DeviceEnum->EnumAudioEndpoints(eCapture, DEVICE_STATEMASK_ALL, &DeviceCollection)))
        {
            throw logic_error("Couldn't enumerate audio endpoints");
        }
        UINT Count(0);
        if(FAILED(DeviceCollection->GetCount(&Count)))
        {
            throw logic_error("Couldn't count items in endpoint collection");
        }
        for(UINT i(0); i < Count; ++i)
        {
            CComPtr<IMMDevice> Device;
            if(FAILED(DeviceCollection->Item(i, &Device)))
            {
                throw logic_error("Couldn't get item in endpoint collection");
            }

            SmartPtr<IMixerLineSrc> NewSource = new CVistaMixerLineSrc(Device);
            m_SourceLines.push_back(NewSource);

            // done with this copy of pointer
            Device.Release();
        }
    }

    virtual ~CVistaMixerLineDst()
    {
    }
    
    const TCHAR* GetName()
    {
        return m_Name.c_str();
    }

    long GetSourceCount()
    {
        return m_SourceLines.size();
    }
    
    IMixerLineSrc* GetSourceLine(DWORD nIndex)
    {
        return m_SourceLines[nIndex];
    }

    void StoreState()
    {
        for (DWORD i = 0; i < m_SourceLines.size(); i++)
        {
            m_SourceLines[i]->StoreState();
        }
    }
    
    void RestoreState()
    {
        for (DWORD i = 0; i < m_SourceLines.size(); i++)
        {
            m_SourceLines[i]->RestoreState();
        }
    }

    BOOL IsTypicalSpeakerLine()
    {
        return TRUE;
    }

    BOOL IsTypicalRecordingLine()
    {
        return FALSE;
    }

private:
    CComPtr<IMMDevice> m_Device;
    std::vector< SmartPtr<IMixerLineSrc> > m_SourceLines;
    tstring m_Name;
};

//----------------------------------------------------------------------
//  CVistaMixer
//  In Vista the sound design is different
//  the ideaof an audo end point seems to correspond to the concept of 
//  dest line and there is no coresponding conecpt to Mixer so we'll just
// have Mixer has one Dest line
//----------------------------------------------------------------------
class CVistaMixer : public IMixer
{
public:
    CVistaMixer(DWORD nMixerIndex, CComPtr<IMMDevice>& Device, CComPtr<IMMDeviceEnumerator> DeviceEnum) :
        m_nMixerIndex(nMixerIndex),
        m_DestinationLine(Device, DeviceEnum)
    {
    }

    ~CVistaMixer()
    {
    }

    const TCHAR* GetName()
    {
        return m_DestinationLine.GetName();
    }
    DWORD GetIndex()
    {
        return m_nMixerIndex;
    }

    long GetDestinationCount()
    {
        return 1;
    }
    
    IMixerLineDst* GetDestinationLine(DWORD nIndex)
    {
        return &m_DestinationLine;
    }

    void StoreState()
    {
        m_DestinationLine.StoreState();
    }

    void RestoreState()
    {
        m_DestinationLine.RestoreState();
    }

private:
    DWORD m_nMixerIndex;

    CVistaMixerLineDst m_DestinationLine;
};
#endif

//----------------------------------------------------------------------
//  CMixerFinder
//----------------------------------------------------------------------

CMixerList::CMixerList()
{
#ifdef _DO_NEW_MIXER_CODE_
    CComPtr<IMMDeviceEnumerator> DeviceEnum;
    if(FALSE && SUCCEEDED(DeviceEnum.CoCreateInstance(__uuidof(MMDeviceEnumerator))))
    {
        // do new Vista style approach
        CComPtr<IMMDeviceCollection> DeviceCollection;
        if(FAILED(DeviceEnum->EnumAudioEndpoints(eRender, DEVICE_STATEMASK_ALL, &DeviceCollection)))
        {
            throw logic_error("Couldn't enumerate audio endpoints");
        }
        UINT Count(0);
        if(FAILED(DeviceCollection->GetCount(&Count)))
        {
            throw logic_error("Couldn't count items in endpoint collection");
        }
        for(UINT i(0); i < Count; ++i)
        {
            CComPtr<IMMDevice> Device;
            if(FAILED(DeviceCollection->Item(i, &Device)))
            {
                throw logic_error("Couldn't get item in endpoint collection");
            }

            SmartPtr<IMixer> NewMixer = new CVistaMixer(i, Device, DeviceEnum);
            m_Mixers.push_back(NewMixer);

            // done with this copy of pointer
            Device.Release();
        }
    }
    else
#endif
    {
        // do old school mixer based classed
        long nMixerCount = mixerGetNumDevs();
        for(long i(0); i < nMixerCount; ++i)
        {
            SmartPtr<IMixer> NewMixer = new CMixer(i);
            m_Mixers.push_back(NewMixer);
        }
    }
}

long CMixerList::GetMixerCount()
{
    return m_Mixers.size();
}

IMixer* CMixerList::GetMixer(long nMixerIndex)
{
	if(nMixerIndex >= 0 && nMixerIndex <= m_Mixers.size())
	{
		return m_Mixers[nMixerIndex].GetRawPointer();
	}
	else
	{
		return NULL;
	}
}

long CMixerList::FindMixer(const tstring& MixerName)
{
    for (size_t i(0); i < m_Mixers.size(); ++i)
    {
        if (MixerName == m_Mixers[i]->GetName())
        {
            return i;
        }
    }
    return -1;
}

CMixerList* CMixerList::getInstance()
{
    static CMixerList TheInstance;
    return &TheInstance;
}

