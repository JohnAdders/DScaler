//
// $Id: AudioDecoder.cpp,v 1.1 2001-12-05 21:45:10 ittarnavsky Exp $
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
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AudioDecoder.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAudioDecoder::CAudioDecoder()
{
    m_VideoFormat = VIDEOFORMAT_NTSC_M;
    m_SoundChannel = MONO;
    m_AudioInput = AUDIOINPUT_TUNER;
}

CAudioDecoder::~CAudioDecoder()
{

}

eVideoFormat CAudioDecoder::GetVideoFormat()
{
    return m_VideoFormat;
}

void CAudioDecoder::SetVideoFormat(eVideoFormat videoFormat)
{
    m_VideoFormat = videoFormat;
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
