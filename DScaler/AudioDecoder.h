//
// $Id: AudioDecoder.h,v 1.8 2002-09-12 21:50:05 ittarnavsky Exp $
//
/////////////////////////////////////////////////////////////////////////////
//
// copyleft 2001 itt@myself.com
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: not supported by cvs2svn $
// Revision 1.7  2002/08/27 22:02:32  kooiman
// Added Get/Set input for video and audio for all sources. Added source input change notification.
//
// Revision 1.6  2002/07/02 20:00:06  adcockj
// New setting for MSP input pin selection
//
// Revision 1.5  2002/02/01 04:43:55  ittarnavsky
// some more audio related fixes
// removed the handletimermessages and getaudioname methods
// which break the separation of concerns oo principle
//
// Revision 1.4  2002/01/23 22:52:01  robmuller
// Added member function GetAudioName()
//
// Revision 1.3  2002/01/23 12:20:32  robmuller
// Added member function HandleTimerMessages(int TimerId).
//
// Revision 1.2  2001/12/19 19:24:44  ittarnavsky
// prepended SOUNDCHANNEL_ to all members of the eSoundChannel enum
//
// Revision 1.1  2001/12/05 21:45:10  ittarnavsky
// added changes for the AudioDecoder and AudioControls support
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(__AUDIODECODER_H__)
#define __AUDIODECODER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Bt848_Defines.h"
#include "TVFormats.h"

enum eAudioInput
{
    AUDIOINPUT_TUNER = 0,
    AUDIOINPUT_RADIO,
    AUDIOINPUT_EXTERNAL,
    AUDIOINPUT_INTERNAL,
    AUDIOINPUT_MUTE,
    AUDIOINPUT_STEREO 
};

class CAudioDecoder
{
public:
    CAudioDecoder();
    virtual ~CAudioDecoder();

    // Transmission standard
    virtual void SetVideoFormat(eVideoFormat videoFormat);
    virtual eVideoFormat GetVideoFormat();

    // Sound Channels
    virtual void SetSoundChannel(eSoundChannel soundChannel);
    virtual eSoundChannel GetSoundChannel();
    virtual eSoundChannel IsAudioChannelDetected(eSoundChannel desiredAudioChannel);

    // Inputs
    virtual void SetAudioInput(eAudioInput audioInput);
    virtual eAudioInput GetAudioInput();    
    virtual const char* GetAudioInputName(eAudioInput audioInput);
    virtual int GetNumAudioInputs() { return AUDIOINPUT_STEREO+1; }

protected:
    eSoundChannel m_SoundChannel;
    eVideoFormat m_VideoFormat;
    eAudioInput m_AudioInput;
};

#endif // !defined(__AUDIODECODER_H__)
