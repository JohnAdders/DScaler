/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Card_Audio.cpp,v 1.19 2002-09-12 21:59:52 ittarnavsky Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.18  2002/09/07 20:54:49  kooiman
// Added equalizer, loudness, spatial effects for MSP34xx
//
// Revision 1.17  2002/08/27 22:02:32  kooiman
// Added Get/Set input for video and audio for all sources. Added source input change notification.
//
// Revision 1.16  2002/07/02 20:00:07  adcockj
// New setting for MSP input pin selection
//
// Revision 1.15  2002/04/07 10:37:53  adcockj
// Made audio source work per input
//
// Revision 1.14  2002/02/12 02:27:45  ittarnavsky
// fixed the hardware info dialog
//
// Revision 1.13  2002/02/01 04:43:55  ittarnavsky
// some more audio related fixes
// removed the handletimermessages and getaudioname methods
// which break the separation of concerns oo principle
//
// Revision 1.12  2002/01/23 22:57:29  robmuller
// Revision D/G improvements. The code is following the documentation much closer now.
//
// Revision 1.11  2002/01/21 08:40:27  robmuller
// Removed unnecessary delay in SetAudioSource().
//
// Revision 1.10  2001/12/19 19:24:44  ittarnavsky
// prepended SOUNDCHANNEL_ to all members of the eSoundChannel enum
//
// Revision 1.9  2001/12/18 23:36:01  adcockj
// Split up the MSP chip support into two parts to avoid probelms when deleting objects
//
// Revision 1.8  2001/12/18 13:12:11  adcockj
// Interim check-in for redesign of card specific settings
//
// Revision 1.7  2001/12/05 21:45:10  ittarnavsky
// added changes for the AudioDecoder and AudioControls support
//
// Revision 1.6  2001/11/29 17:30:51  adcockj
// Reorgainised bt848 initilization
// More Javadoc-ing
//
// Revision 1.5  2001/11/26 13:02:27  adcockj
// Bug Fixes and standards changes
//
// Revision 1.4  2001/11/25 01:58:34  ittarnavsky
// initial checkin of the new I2C code
//
// Revision 1.3  2001/11/23 10:49:16  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.2  2001/11/02 16:30:07  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.1.2.1  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "BT848Card.h"
#include "BT848_Defines.h"

#include "MSP34x0.h"
#include "MSP34x0AudioControls.h"

#include "Bt8x8GPIOAudioDecoderAverTVPhoneNew.h"
#include "Bt8x8GPIOAudioDecoderAverTVPhoneOld.h"
#include "Bt8x8GPIOAudioDecoderGVBCTV3.h"
#include "Bt8x8GPIOAudioDecoderLT9415.h"
#include "Bt8x8GPIOAudioDecoderPVBt878P9B.h"
#include "Bt8x8GPIOAudioDecoderTerraTV.h"
#include "Bt8x8GPIOAudioDecoderWinDVR.h"
#include "Bt8x8GPIOAudioDecoderWinFast2000.h"

void CBT848Card::InitAudio()
{
    // Initialize  m_AudioDecoder and m_AudioControls
    if (m_AudioControls != NULL)
    {
        delete m_AudioControls;
    }
    if (m_AudioDecoder != NULL)
    {
        delete m_AudioDecoder;
    }


    if (GetCardSetup()->AudioDecoderType == AUDIODECODERTYPE_DETECT
        || GetCardSetup()->AudioDecoderType == AUDIODECODERTYPE_MSP34x0)
    {
        CMSP34x0AudioControls* MSPControls = new CMSP34x0AudioControls();

        MSPControls->Attach(m_I2CBus);
        MSPControls->Reset();
        ::Sleep(4);

        // setup version information
        int rev1 = MSPControls->GetVersion();
        int rev2 = MSPControls->GetProductCode();

        if (0 == rev1 && 0 == rev2)
        {
            delete MSPControls;
            m_AudioDecoder = new CAudioDecoder();
            m_AudioControls = new CAudioControls();
            return;
        }

        m_AudioControls = MSPControls;
    
        // need to create two so that we can delete all objects properly
        CMSP34x0Decoder* MSPDecoder = new CMSP34x0Decoder();
        MSPDecoder->Attach(m_I2CBus);

        m_AudioDecoder =  MSPDecoder;

        sprintf(m_AudioDecoderType, "MSP34%02d%c-%c%d", (rev2 >> 8) & 0xff, (rev1 & 0xff) + '@', ((rev1 >> 8) & 0xff) + '@', rev2 & 0x1f);
    }
    else
    {
        switch (GetCardSetup()->AudioDecoderType)
        {
        case AUDIODECODERTYPE_NONE:
            m_AudioDecoder = new CAudioDecoder();
            m_AudioControls = new CAudioControls();
            sprintf(m_AudioDecoderType, "None");
            break;
        case AUDIODECODERTYPE_TERRATV:
            m_AudioDecoder = new CBt8x8GPIOAudioDecoderTerraTV(this);
            m_AudioControls = new CAudioControls();
            sprintf(m_AudioDecoderType, "TerraTV");
            break;
        case AUDIODECODERTYPE_WINFAST2000:
            m_AudioDecoder = new CBt8x8GPIOAudioDecoderWinFast2000(this);
            m_AudioControls = new CAudioControls();
            sprintf(m_AudioDecoderType, "WinFast2000");
            break;
        case AUDIODECODERTYPE_GVBCTV3:
            m_AudioDecoder = new CBt8x8GPIOAudioDecoderGVBCTV3(this);
            m_AudioControls = new CAudioControls();
            sprintf(m_AudioDecoderType, "GVBCTV3");
            break;
        case AUDIODECODERTYPE_LT9415:
            m_AudioDecoder = new CBt8x8GPIOAudioDecoderLT9415(this);
            m_AudioControls = new CAudioControls();
            sprintf(m_AudioDecoderType, "LT9415");
            break;
        case AUDIODECODERTYPE_WINDVR:
            m_AudioDecoder = new CBt8x8GPIOAudioDecoderWinDVR(this);
            m_AudioControls = new CAudioControls();
            sprintf(m_AudioDecoderType, "WinDVR");
            break;
        case AUDIODECODERTYPE_AVER_TVPHONE_NEW:
            m_AudioDecoder = new CBt8x8GPIOAudioDecoderAverTVPhoneNew(this);
            m_AudioControls = new CAudioControls();
            sprintf(m_AudioDecoderType, "AverTVPhoneNew");
            break;
        case AUDIODECODERTYPE_AVER_TVPHONE_OLD:
            m_AudioDecoder = new CBt8x8GPIOAudioDecoderAverTVPhoneOld(this);
            m_AudioControls = new CAudioControls();
            sprintf(m_AudioDecoderType, "AverTVPhoneOld");
            break;
        }
    }

    // set volume to Mute level
    m_AudioControls->SetMute();
}


void CBT848Card::SetAudioMute()
{
    if (m_AudioControls->HasMute())
    {
        m_AudioControls->SetMute();
    }
    else
    {
        SetAudioSource(AUDIOINPUT_MUTE);
    }
}

void CBT848Card::SetAudioUnMute(WORD nVolume, eAudioInput Input)
{
    if (m_AudioControls->HasMute())
    {
        // go back from mute to same volume as before
        m_AudioControls->SetVolume(nVolume);
    }
    else
    {
        SetAudioSource(Input);
    }
}


void CBT848Card::SetAudioVolume(WORD nVolume)
{
    m_AudioControls->SetVolume(nVolume);
}


void CBT848Card::SetAudioBalance(WORD nBalance)
{
    m_AudioControls->SetBalance(nBalance);
}

void CBT848Card::SetAudioBass(WORD nBass)
{
    m_AudioControls->SetBass(nBass);
}

void CBT848Card::SetAudioTreble(WORD nTreble)
{
    m_AudioControls->SetTreble(nTreble);
}

void CBT848Card::SetAudioChannel(eSoundChannel soundChannel)
{
    m_AudioDecoder->SetSoundChannel(soundChannel);
}

eSoundChannel CBT848Card::IsAudioChannelDetected(eSoundChannel desiredSoundChannel)
{
    return m_AudioDecoder->IsAudioChannelDetected(desiredSoundChannel); 
}

void CBT848Card::SetAudioStandard(eVideoFormat videoFormat)
{
    m_AudioDecoder->SetVideoFormat(videoFormat);
}

// Do not call this function before changing the video source, it is checking to see if a video
// signal is present. Audio is muted if no video signal is detected. 
// This might not be the best place to do this check.
void CBT848Card::SetAudioSource(eAudioInput nChannel)
{
    DWORD MuxSelect;

    AndOrDataDword(BT848_GPIO_OUT_EN, GetCardSetup()->GPIOMask, ~GetCardSetup()->GPIOMask);

    switch(nChannel)
    {
    case AUDIOINPUT_RADIO:
    case AUDIOINPUT_MUTE:
        // just get on with it
        MuxSelect = GetCardSetup()->AudioMuxSelect[nChannel];
        break;
    default:
        // see if there is a video signal present
        // if video not in H-lock, turn audio off 
        if (!(ReadByte(BT848_DSTATUS) & BT848_DSTATUS_PRES))
        {
            MuxSelect = GetCardSetup()->AudioMuxSelect[AUDIOINPUT_MUTE];
        }
        else
        {
            MuxSelect = GetCardSetup()->AudioMuxSelect[nChannel];
        }
        break;
    }
    
    // select direct input 
    //BT848_WriteWord(BT848_GPIO_REG_INP, 0x00); // MAE 14 Dec 2000 disabled
    AndOrDataDword(BT848_GPIO_DATA, MuxSelect, ~GetCardSetup()->GPIOMask); 

    m_AudioDecoder->SetAudioInput(nChannel);
}


eAudioInput CBT848Card::GetAudioInput()
{
    return m_AudioDecoder->GetAudioInput();
}

LPCSTR CBT848Card::GetAudioInputName(eAudioInput nInput)
{
    return m_AudioDecoder->GetAudioInputName(nInput);
}


int CBT848Card::GetNumAudioInputs()
{
    return m_AudioDecoder->GetNumAudioInputs();
}


void CBT848Card::SetAudioLoudness(WORD nLevel)
{
    m_AudioControls->SetLoudness(nLevel);
}

void CBT848Card::SetAudioBassBoost(bool bBoost)
{
    m_AudioControls->SetBassBoost(bBoost);
}

void CBT848Card::SetAudioEqualizerLevel(WORD nIndex, WORD nLevel)
{
    if (!m_AudioControls->HasEqualizers() 
        && nIndex >= 0 && nIndex < m_AudioControls->GetEqualizerCount())
    {
        return;
    }
    m_AudioControls->SetEqualizerLevel(nIndex, nLevel);
}

void CBT848Card::SetAudioSpatialEffect(WORD nLevel)
{
    if (!m_AudioControls->HasSpatialEffect())
    {
        return;
    }
    m_AudioControls->SetSpatialEffect(nLevel);
}


void CBT848Card::SetAudioDolby(WORD nMode, WORD nNoise, WORD nSpatial, WORD nPan, WORD nPanorama)
{
    if (!m_AudioControls->HasDolby())
    {
        return;
    }
    m_AudioControls->SetDolby(nMode, nNoise, nSpatial, nPan, nPanorama);
}


bool CBT848Card::HasAudioEqualizers()
{
    return m_AudioControls->HasEqualizers();
}


bool CBT848Card::HasAudioSpatialEffect()
{
    return m_AudioControls->HasSpatialEffect();
}


bool CBT848Card::HasAudioDolby()
{
    return m_AudioControls->HasDolby();
}

bool CBT848Card::HasAudioBassBoost()
{
    return m_AudioControls->HasBassBoost();
}

bool CBT848Card::HasAudioVolume()
{
    return m_AudioControls->HasVolume();
}

bool CBT848Card::HasAudioTreble()
{
    return m_AudioControls->HasTreble();
}

bool CBT848Card::HasAudioLoudness()
{
    return m_AudioControls->HasLoudness();
}

bool CBT848Card::HasAudioBass()
{
    return m_AudioControls->HasBass();
}

bool CBT848Card::HasAudioBalance()
{
    return m_AudioControls->HasBalance();
}
