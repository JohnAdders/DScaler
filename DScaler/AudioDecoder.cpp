//
// $Id$
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

/**
 * @file AudioDecoder.cpp Audio Decoder Classes
 */

#include "stdafx.h"
#include "AudioDecoder.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAudioDecoder::CAudioDecoder()
{
    m_SoundChannel = SOUNDCHANNEL_MONO;
    m_AudioInput = AUDIOINPUT_TUNER;
    
    m_AudioStandard = 0;
    m_AudioStandardMajorCarrier = 0;
    m_AudioStandardMinorCarrier = 0;
}

CAudioDecoder::~CAudioDecoder()
{
}

eSoundChannel CAudioDecoder::GetSoundChannel()
{
    return m_SoundChannel;
}

void CAudioDecoder::SetSoundChannel(eSoundChannel soundChannel)
{
    m_SoundChannel = soundChannel;
}

eSoundChannel CAudioDecoder::IsAudioChannelDetected(eSoundChannel desiredAudioChannel)
{
    return desiredAudioChannel;
}

eAudioInput CAudioDecoder::GetAudioInput()
{
    return m_AudioInput;
}

void CAudioDecoder::SetAudioInput(eAudioInput audioInput)
{
    m_AudioInput = audioInput;
}

const char* CAudioDecoder::GetAudioInputName(eAudioInput audioInput)
{    
    switch (audioInput)
    {
    case AUDIOINPUT_TUNER:
        return "Tuner";
        break;
    case AUDIOINPUT_RADIO:
        return "Radio";
        break;
    case AUDIOINPUT_EXTERNAL:
        return "External";
        break;
    case AUDIOINPUT_INTERNAL:
        return "Internal";
        break;
    case AUDIOINPUT_MUTE:
        return "Mute";
        break;
    case AUDIOINPUT_STEREO:
        return "Stereo";
        break;
    }
    
    return NULL;
}

void CAudioDecoder::SetAudioStandard(long Standard, eVideoFormat VideoFormat) 
{
    m_AudioStandard = Standard;
    m_VideoFormat = VideoFormat;
    m_AudioStandardMajorCarrier = 0;
    m_AudioStandardMinorCarrier = 0;
}

long CAudioDecoder::GetAudioStandardCurrent()
{
    return m_AudioStandard;
}

const char* CAudioDecoder::GetAudioStandardName(long Standard)
{
    return NULL;
}

int CAudioDecoder::GetNumAudioStandards()
{
    return 0;
}

long CAudioDecoder::GetAudioStandard(int nIndex)
{
    return 0;
}

long CAudioDecoder::GetAudioStandardMajorCarrier(long Standard)
{
    return m_AudioStandardMajorCarrier;
}

long CAudioDecoder::GetAudioStandardMinorCarrier(long Standard)
{
    return m_AudioStandardMinorCarrier;
}

void CAudioDecoder::SetAudioStandardCarriers(long MajorCarrier, long MinorCarrier)
{
    m_AudioStandardMajorCarrier = MajorCarrier;
    m_AudioStandardMinorCarrier = MinorCarrier;
}

long CAudioDecoder::GetAudioStandardFromVideoFormat(eVideoFormat videoFormat)
{
    return 0;
}

void CAudioDecoder::DetectAudioStandard(long Interval, int SupportedSoundChannels, eSoundChannel TargetChannel)
{    
    m_StandardDetectInterval = Interval;
}

CAudioDecoder::eAudioDecoderType CAudioDecoder::GetAudioDecoderType()
{
    return AUDIODECODERTYPE_NONE;
}