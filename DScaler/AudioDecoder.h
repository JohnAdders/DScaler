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
 * @file AudioDecoder.h AudioDecoder Header file
 */

#if !defined(__AUDIODECODER_H__)
#define __AUDIODECODER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SoundChannel.h"
#include "TVFormats.h"
#include "Events.h"

enum eAudioInput
{
    AUDIOINPUT_TUNER = 0,
    AUDIOINPUT_RADIO,
    AUDIOINPUT_EXTERNAL,
    AUDIOINPUT_INTERNAL,
    AUDIOINPUT_MUTE,
    AUDIOINPUT_STEREO 
};

enum eSupportedSoundChannels
{
    SUPPORTEDSOUNDCHANNEL_NONE = 0,
    SUPPORTEDSOUNDCHANNEL_MONO = 1,
    SUPPORTEDSOUNDCHANNEL_STEREO = 2,
    SUPPORTEDSOUNDCHANNEL_LANG1 = 4,
    SUPPORTEDSOUNDCHANNEL_LANG2 = 8,
};


class CAudioDecoder : public CEventObject
{
public:
    CAudioDecoder();
    virtual ~CAudioDecoder();

    // Sound Channels
    virtual void SetSoundChannel(eSoundChannel soundChannel);
    virtual eSoundChannel GetSoundChannel();
    virtual eSoundChannel IsAudioChannelDetected(eSoundChannel desiredAudioChannel);

    // Inputs
    virtual void SetAudioInput(eAudioInput audioInput);
    virtual eAudioInput GetAudioInput();    
    virtual const char* GetAudioInputName(eAudioInput audioInput);

    // Standard
    virtual void SetAudioStandard(long Standard, eVideoFormat videoformat);
    virtual long GetAudioStandardCurrent();    
    virtual const char* GetAudioStandardName(long Standard);
    virtual int GetNumAudioStandards();
    virtual long GetAudioStandard(int nIndex);
    virtual long GetAudioStandardMajorCarrier(long Standard);
    virtual long GetAudioStandardMinorCarrier(long Standard);
    virtual void SetAudioStandardCarriers(long MajorCarrier, long MinorCarrier);
    virtual long GetAudioStandardFromVideoFormat(eVideoFormat videoFormat);
    // Interval in milliseconds. <=0 for abort.
    // SupportedSoundChannels == 1: detect after standard
    // SupportedSoundChannels == 2: detect now, no standard detect
    virtual void DetectAudioStandard(long Interval, int SupportedSoundChannels, eSoundChannel TargetChannel);

    /// The type of the AudioDecoder component
    enum eAudioDecoderType
    {
        AUDIODECODERTYPE_DETECT = -1,
        AUDIODECODERTYPE_NONE,
        AUDIODECODERTYPE_MSP34x0,
        AUDIODECODERTYPE_TERRATV,
        AUDIODECODERTYPE_WINFAST2000,
        AUDIODECODERTYPE_GVBCTV3,
        AUDIODECODERTYPE_LT9415,
        AUDIODECODERTYPE_WINDVR,
        AUDIODECODERTYPE_AVER_TVPHONE_NEW,
        AUDIODECODERTYPE_AVER_TVPHONE_OLD,
        AUDIODECODERTYPE_TDA9875,
        AUDIODECODERTYPE_TDA9873,
        AUDIODECODERTYPE_TDA9874,
        AUDIODECODERTYPE_LASTONE
    };
    virtual eAudioDecoderType GetAudioDecoderType();

    // \todo: eliminate this
    virtual int SetAudioDecoderValue(int What, long Val) {return 0;};
    virtual long GetAudioDecoderValue(int What){ return 0;};

protected:
    eVideoFormat m_VideoFormat;
    eSoundChannel m_SoundChannel;
    eAudioInput m_AudioInput;

    long m_AudioStandard;             // Identification number
    long m_AudioStandardMajorCarrier; // In Hz
    long m_AudioStandardMinorCarrier; // In Hz

    long m_StandardDetectInterval;
};

#endif // !defined(__AUDIODECODER_H__)
