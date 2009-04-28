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
        strcpy(m_mxl.szName, "Error");
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


const char* CMixerLineSrc::GetName()
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
        strcpy(m_mxl.szName, "Error");
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

const char* CMixerLineDst::GetName()
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

    return m_pSourceLines[nIndex];
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


const char* CMixer::GetName()
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


CMixerLineDst* CMixer::GetDestinationLine(DWORD nIndex)
{
    if (nIndex >= m_pDestinationLines.size())
    {
        return NULL;
    }

    return m_pDestinationLines[nIndex];
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


//----------------------------------------------------------------------
//  CMixerFinder
//----------------------------------------------------------------------

CMixerList::CMixerList()
{
    long nMixerCount = mixerGetNumDevs();
    for(long i(0); i < nMixerCount; ++i)
    {
        SmartPtr<IMixer> NewMixer = new CMixer(i);
        m_Mixers.push_back(NewMixer);
    }
}

long CMixerList::GetMixerCount()
{
    return m_Mixers.size();
}

IMixer* CMixerList::GetMixer(long nMixerIndex)
{
    return m_Mixers[nMixerIndex];
}

long CMixerList::FindMixer(const char* MixerName)
{
    for (size_t i(0); i < m_Mixers.size(); ++i)
    {
        if (lstrcmp(m_Mixers[i]->GetName(), MixerName) == 0)
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

