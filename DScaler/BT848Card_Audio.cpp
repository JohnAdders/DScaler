/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file BT848Card.cpp CBT848Card Implementation (Audio)
 */

#include "stdafx.h"

#ifdef WANT_BT8X8_SUPPORT

#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "BT848Card.h"
#include "BT848_Defines.h"

#include "MSP34x0.h"
#include "MSP34x0AudioControls.h"
#include "MSP34x0AudioDecoder.h"

#include "PIC16C54.h"

#include "TDA9875.h"
#include "TDA9875AudioControls.h"
#include "TDA9875AudioDecoder.h"
#include "TDA9874.h"
#include "TDA9874AudioDecoder.h"
#include "TDA9873AudioDecoder.h"

#include "Bt8x8GPIOAudioDecoderAverTVPhoneNew.h"
#include "Bt8x8GPIOAudioDecoderAverTVPhoneOld.h"
#include "Bt8x8GPIOAudioDecoderGVBCTV3.h"
#include "Bt8x8GPIOAudioDecoderLT9415.h"
#include "Bt8x8GPIOAudioDecoderPVBt878P9B.h"
#include "Bt8x8GPIOAudioDecoderTerraTV.h"
#include "Bt8x8GPIOAudioDecoderWinDVR.h"
#include "Bt8x8GPIOAudioDecoderWinFast2000.h"

using namespace std;

void CBT848Card::InitAudio(BOOL UsePin1)
{
    // Initialize  m_AudioDecoder and m_AudioControls
    m_AudioControls = 0L;
    m_AudioDecoder = 0L;


    if ((GetCardSetup()->AudioDecoderType == CAudioDecoder::AUDIODECODERTYPE_DETECT
         || GetCardSetup()->AudioDecoderType == CAudioDecoder::AUDIODECODERTYPE_TDA9873))
    {
        SmartPtr<CTDA9873AudioDecoder> TDADecoder = new CTDA9873AudioDecoder();
        TDADecoder->SetI2CBus(m_I2CBus);
        if(TDADecoder->Initialize())
        {
            m_AudioDecoder = TDADecoder;

            m_AudioControls = new CAudioControls();

            m_AudioDecoderType = "TDA9873";
        }
    }

    if (!m_AudioDecoder &&
        (GetCardSetup()->AudioDecoderType == CAudioDecoder::AUDIODECODERTYPE_DETECT
         || GetCardSetup()->AudioDecoderType == CAudioDecoder::AUDIODECODERTYPE_MSP34x0))
    {
        SmartPtr<CMSP34x0AudioControls> MSPControls = new CMSP34x0AudioControls();

        MSPControls->SetI2CBus(m_I2CBus);
        MSPControls->Reset();
        ::Sleep(4);

        // setup version information
        int rev1 = MSPControls->GetVersion();
        int rev2 = MSPControls->GetProductCode();

        if (0 != rev1 || 0 != rev2)
        {
            m_AudioControls = MSPControls;
    
            // need to create two so that we can delete all objects properly
            SmartPtr<CMSP34x0AudioDecoder> MSPDecoder = new CMSP34x0AudioDecoder();
            MSPDecoder->SetUseInputPin1(UsePin1);
            MSPDecoder->SetI2CBus(m_I2CBus);

            m_AudioDecoder =  MSPDecoder;

            ostringstream oss;
            oss << "MSP34" << setw(2)<< setfill('0') << ((rev2 >> 8) & 0xff);
            oss << (((rev1 >> 8) & 0xff) + '@') << "-";
            oss << (rev2 & 0x1f);
            m_AudioDecoderType = oss.str();
        }
    }

    if(!m_AudioDecoder &&
        (GetCardSetup()->AudioDecoderType == CAudioDecoder::AUDIODECODERTYPE_DETECT
         || GetCardSetup()->AudioDecoderType == CAudioDecoder::AUDIODECODERTYPE_TDA9875))
    {
        // TDA9875 autodetect
        SmartPtr<CTDA9875AudioControls> TDA9875Controls = new CTDA9875AudioControls();

        TDA9875Controls->SetI2CBus(m_I2CBus);
        TDA9875Controls->Reset();
        ::Sleep(4);

        // setup version information
        int dic;
        int rev;
        if (TDA9875Controls->IsDevicePresent(dic, rev))
        {
            m_AudioControls = TDA9875Controls;
    
            SmartPtr<CTDA9875AudioDecoder> TDA9875Decoder = new CTDA9875AudioDecoder(TDA9875Controls);
            TDA9875Decoder->SetUseInputPin1(UsePin1);
            TDA9875Decoder->SetI2CBus(m_I2CBus);

            m_AudioDecoder = TDA9875Decoder; //TDA9875Decoder;

            ostringstream oss;

            oss << "TDA9875";
            oss << (dic==0) ? "":"A";
            oss << " Rev ";
            oss << rev;

            m_AudioDecoderType = oss.str();
        }
    }

    if (!m_AudioDecoder&&
        (GetCardSetup()->AudioDecoderType == CAudioDecoder::AUDIODECODERTYPE_DETECT
        || GetCardSetup()->AudioDecoderType == CAudioDecoder::AUDIODECODERTYPE_TDA9874))
    {        
        SmartPtr<CTDA9874> TDADecoder = new CTDA9874();
        TDADecoder->SetI2CBus(m_I2CBus);
        
        int dic, sic;
        BOOL isPresent = TDADecoder->IsDevicePresent(dic, sic);

        if(isPresent)
        {
            SmartPtr<CTDA9874AudioDecoder> TDA9874Decoder = new CTDA9874AudioDecoder();
            TDA9874Decoder->SetI2CBus(m_I2CBus);
            TDA9874Decoder->Initialize();
            m_AudioDecoder = TDA9874Decoder; //TDA9874Decoder;                    
            m_AudioControls = new CAudioControls();            

            // need to switch unmute from PIC one time only
            // used only for PV951 (Hercules SmartTV)
            CPIC16C54 pic = CPIC16C54();
            pic.SetI2CBus(m_I2CBus);
            BOOL picAlive = pic.IsDevicePresent();
            ostringstream oss;

            oss << "TDA9874" << ((dic==0x11) ? "A":"H");
            oss << " Rev. " << sic << (picAlive ? "" : " + Pic16c54");
            m_AudioDecoderType = oss.str(); 
        }
    }

    if (!m_AudioDecoder)
    {
        switch (GetCardSetup()->AudioDecoderType)
        {
        default:
            m_AudioDecoder = new CAudioDecoder();
            m_AudioControls = new CAudioControls();
            m_AudioDecoderType = "None";
            break;
        case CAudioDecoder::AUDIODECODERTYPE_TERRATV:
            m_AudioDecoder = new CBt8x8GPIOAudioDecoderTerraTV(this);
            m_AudioControls = new CAudioControls();
            m_AudioDecoderType = "TerraTV";
            break;
        case CAudioDecoder::AUDIODECODERTYPE_WINFAST2000:
            m_AudioDecoder = new CBt8x8GPIOAudioDecoderWinFast2000(this);
            m_AudioControls = new CAudioControls();
            m_AudioDecoderType = "WinFast2000";
            break;
        case CAudioDecoder::AUDIODECODERTYPE_GVBCTV3:
            m_AudioDecoder = new CBt8x8GPIOAudioDecoderGVBCTV3(this);
            m_AudioControls = new CAudioControls();
            m_AudioDecoderType = "GVBCTV3";
            break;
        case CAudioDecoder::AUDIODECODERTYPE_LT9415:
            m_AudioDecoder = new CBt8x8GPIOAudioDecoderLT9415(this);
            m_AudioControls = new CAudioControls();
            m_AudioDecoderType = "LT9415";
            break;
        case CAudioDecoder::AUDIODECODERTYPE_WINDVR:
            m_AudioDecoder = new CBt8x8GPIOAudioDecoderWinDVR(this);
            m_AudioControls = new CAudioControls();
            m_AudioDecoderType = "WinDVR";
            break;
        case CAudioDecoder::AUDIODECODERTYPE_AVER_TVPHONE_NEW:
            m_AudioDecoder = new CBt8x8GPIOAudioDecoderAverTVPhoneNew(this);
            m_AudioControls = new CAudioControls();
            m_AudioDecoderType = "AverTVPhoneNew";
            break;
        case CAudioDecoder::AUDIODECODERTYPE_AVER_TVPHONE_OLD:
            m_AudioDecoder = new CBt8x8GPIOAudioDecoderAverTVPhoneOld(this);
            m_AudioControls = new CAudioControls();
            m_AudioDecoderType = "AverTVPhoneOld";
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

long CBT848Card::GetAudioStandardFromVideoFormat(eVideoFormat videoFormat)
{
    return m_AudioDecoder->GetAudioStandardFromVideoFormat(videoFormat);
}

void CBT848Card::DetectAudioStandard(long Interval, int SupportedSoundChannels, eSoundChannel TargetChannel)
{
    m_AudioDecoder->DetectAudioStandard(Interval, SupportedSoundChannels, TargetChannel);
}


CAudioDecoder::eAudioDecoderType CBT848Card::GetAudioDecoderType()
{
    return m_AudioDecoder->GetAudioDecoderType();
}

int CBT848Card::SetAudioDecoderValue(int What, long Val)
{
    return m_AudioDecoder->SetAudioDecoderValue(What, Val);
}

long CBT848Card::GetAudioDecoderValue(int What)
{
    return m_AudioDecoder->GetAudioDecoderValue(What);
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

void CBT848Card::SetAudioLoudness(WORD nLevel)
{
    m_AudioControls->SetLoudness(nLevel);
}

void CBT848Card::SetAudioBassBoost(BOOL bBoost)
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

void CBT848Card::SetAudioSpatialEffect(int nLevel)
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

void CBT848Card::SetAudioAutoVolumeCorrection(long nDecayTimeIndex)
{
    if (!m_AudioControls->HasAutoVolumeCorrection())
    {
        return;
    }
    m_AudioControls->SetAutoVolumeCorrection(nDecayTimeIndex);
}

void CBT848Card::SetAudioStandard(long Standard, eVideoFormat VideoFormat)
{
    m_AudioDecoder->SetAudioStandard(Standard, VideoFormat);
}

long CBT848Card::GetAudioStandardCurrent()
{
    return m_AudioDecoder->GetAudioStandardCurrent();
}

const char* CBT848Card::GetAudioStandardName(long Standard)
{
    return m_AudioDecoder->GetAudioStandardName(Standard);
}

int CBT848Card::GetNumAudioStandards()
{
    return m_AudioDecoder->GetNumAudioStandards();
}

long CBT848Card::GetAudioStandard(int nIndex)
{
    return m_AudioDecoder->GetAudioStandard(nIndex);
}

long CBT848Card::GetAudioStandardMajorCarrier(long Standard)
{
    return m_AudioDecoder->GetAudioStandardMajorCarrier(Standard);
}

long CBT848Card::GetAudioStandardMinorCarrier(long Standard)
{
    return m_AudioDecoder->GetAudioStandardMinorCarrier(Standard);
}

void CBT848Card::SetAudioStandardCarriers(long MajorCarrier, long MinorCarrier)
{
    m_AudioDecoder->SetAudioStandardCarriers(MajorCarrier, MinorCarrier);
}


BOOL CBT848Card::HasAudioEqualizers()
{
    return m_AudioControls->HasEqualizers();
}


BOOL CBT848Card::HasAudioSpatialEffect()
{
    return m_AudioControls->HasSpatialEffect();
}


BOOL CBT848Card::HasAudioDolby()
{
    return m_AudioControls->HasDolby();
}

BOOL CBT848Card::HasAudioBassBoost()
{
    return m_AudioControls->HasBassBoost();
}

BOOL CBT848Card::HasAudioVolume()
{
    return m_AudioControls->HasVolume();
}

BOOL CBT848Card::HasAudioTreble()
{
    return m_AudioControls->HasTreble();
}

BOOL CBT848Card::HasAudioLoudness()
{
    return m_AudioControls->HasLoudness();
}

BOOL CBT848Card::HasAudioBass()
{
    return m_AudioControls->HasBass();
}

BOOL CBT848Card::HasAudioBalance()
{
    return m_AudioControls->HasBalance();
}

BOOL CBT848Card::HasAudioAutoVolumeCorrection()
{
    return m_AudioControls->HasAutoVolumeCorrection();
}

BOOL CBT848Card::GetHasUseInputPin1()
{
    SmartPtr<CMSP34x0AudioDecoder> MSP34x0AudioDecoder(m_AudioDecoder);
    if(MSP34x0AudioDecoder)
    {
        return TRUE;
    }
    return FALSE;
}

BOOL CBT848Card::GetUseInputPin1()
{
    SmartPtr<CMSP34x0AudioDecoder> MSP34x0AudioDecoder(m_AudioDecoder);
    if(MSP34x0AudioDecoder)
    {
        return MSP34x0AudioDecoder->GetUseInputPin1();
    }
    return FALSE;
}

void CBT848Card::SetUseInputPin1(BOOL AValue)
{
    SmartPtr<CMSP34x0AudioDecoder> MSP34x0AudioDecoder(m_AudioDecoder);
    SmartPtr<CTDA9875AudioDecoder> TDA9875AudioDecoder(m_AudioDecoder);
    if(MSP34x0AudioDecoder)
    {
        MSP34x0AudioDecoder->SetUseInputPin1(AValue);
    }
    else if (TDA9875AudioDecoder)
    {
        TDA9875AudioDecoder->SetUseInputPin1(AValue);
    }

}

BOOL CBT848Card::IsMyAudioDecoder(SmartPtr<CAudioDecoder> pAudioDecoder)
{
    return (m_AudioDecoder == pAudioDecoder);
}

#endif // WANT_BT8X8_SUPPORT