/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Card_Audio.cpp,v 1.14 2002-02-12 02:27:45 ittarnavsky Exp $
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

BOOL CBT848Card::HasMSP()
{
    return m_bHasMSP;
}

void CBT848Card::InitAudio()
{
    CMSP34x0Controls* MSPControls = new CMSP34x0Controls();

    MSPControls->Attach(m_I2CBus);
    MSPControls->Reset();
    ::Sleep(4);

    // setup version information
    int rev1 = MSPControls->GetVersion();
    int rev2 = MSPControls->GetProductCode();

    if (0 == rev1 && 0 == rev2)
    {
        delete MSPControls;
        m_bHasMSP = false;
        return;
    }
    m_bHasMSP = true;

	delete m_AudioControls;
    m_AudioControls = MSPControls;
    
	// need to create two so that we can delete all objects properly
	CMSP34x0Decoder* MSPDecoder = new CMSP34x0Decoder();
	MSPDecoder->Attach(m_I2CBus);
    
	delete m_AudioDecoder;
	m_AudioDecoder =  MSPDecoder;

    // set volume to Mute level
    m_AudioControls->SetMute();

    sprintf(m_AudioDecoderType, "MSP34%02d%c-%c%d", (rev2 >> 8) & 0xff, (rev1 & 0xff) + '@', ((rev1 >> 8) & 0xff) + '@', rev2 & 0x1f);
}


void CBT848Card::SetAudioMute()
{
    if(m_bHasMSP)
    {
        m_AudioControls->SetMute();
    }
    else
    {
        SetAudioSource(AUDIOINPUT_MUTE);
    }
}

void CBT848Card::SetAudioUnMute(long nVolume)
{
    if(m_bHasMSP)
    {
        // go back from mute to same volume as before
        m_AudioControls->SetVolume(nVolume);
    }
    else
    {
        SetAudioSource(m_LastAudioSource);
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
    if(m_bHasMSP)
    {
        m_AudioDecoder->SetSoundChannel(soundChannel);
    }
    else
    {
        if(m_TVCards[m_CardType].pSoundChannelFunction != NULL)
        {
            // call correct function
            // this funny syntax is the only one that works
            // if you want help understanding what is going on
            // I suggest you read http://www.newty.de/
            (*this.*m_TVCards[m_CardType].pSoundChannelFunction)(soundChannel);
        }
    }
}

void CBT848Card::GetMSPPrintMode(LPSTR Text)
{
    if (m_bHasMSP == FALSE)
    {
        strcpy(Text, "No Audio Decoder");
    }
    else
    {
        eAudioInput audioInput = m_AudioDecoder->GetAudioInput();
        switch (audioInput)
        {
        case AUDIOINPUT_TUNER:
            strcpy(Text, "Tuner: ");
            break;
        case AUDIOINPUT_RADIO:
            strcpy(Text, "Radio: ");
            break;
        case AUDIOINPUT_EXTERNAL:
            strcpy(Text, "External: ");
            break;
        case AUDIOINPUT_INTERNAL:
            strcpy(Text, "Internal: ");
            break;
        case AUDIOINPUT_MUTE:
            strcpy(Text, "Mute: ");
            break;
        case AUDIOINPUT_STEREO:
            strcpy(Text, "Stereo: ");
            break;
        }
        eVideoFormat videoFormat = m_AudioDecoder->GetVideoFormat();
        strcat(Text, VideoFormatNames[videoFormat]);
        switch (m_AudioDecoder->GetSoundChannel())
        {
        case SOUNDCHANNEL_MONO:
            strcat(Text, " (Mono)");
            break;
        case SOUNDCHANNEL_STEREO:
            strcat(Text, " (Stereo)");
            break;
        case SOUNDCHANNEL_LANGUAGE1:
            strcat(Text, " (Language 1)");
            break;
        case SOUNDCHANNEL_LANGUAGE2:
            strcat(Text, " (Language 2)");
            break;
        }
    }
}

eSoundChannel CBT848Card::IsAudioChannelDetected(eSoundChannel desiredSoundChannel)
{
    if(!m_bHasMSP)
    {
        return desiredSoundChannel;
    }
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
        m_LastAudioSource = nChannel;
        break;
    }
    
    // select direct input 
    //BT848_WriteWord(BT848_GPIO_REG_INP, 0x00); // MAE 14 Dec 2000 disabled
    AndOrDataDword(BT848_GPIO_DATA, MuxSelect, ~GetCardSetup()->GPIOMask); 

    m_AudioDecoder->SetAudioInput(nChannel);
}

