//
// $Id: AudioDecoder.h,v 1.17 2004-01-05 13:25:25 adcockj Exp $
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
// Revision 1.16  2003/10/27 10:39:50  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.15  2003/02/06 19:37:54  ittarnavsky
// changes for the new SoundChannel.h
//
// Revision 1.14  2002/10/11 21:31:47  ittarnavsky
// removed GetNumAudioInputs()
// replaced GetAudioDecoderID() with GetAudioDecoderType()
// moved eAudioDecoderType here
//
// Revision 1.13  2002/10/07 20:31:02  kooiman
// Fixed autodetect bugs.
//
// Revision 1.12  2002/10/02 10:52:35  kooiman
// Fixed C++ type casting for events.
//
// Revision 1.11  2002/09/27 14:13:27  kooiman
// Improved stereo detection & manual audio standard dialog box.
//
// Revision 1.10  2002/09/16 14:37:35  kooiman
// Added stereo autodetection.
//
// Revision 1.9  2002/09/15 15:57:27  kooiman
// Added Audio standard support.
//
// Revision 1.8  2002/09/12 21:50:05  ittarnavsky
// Removed the UseInputPin1 as this is not generic
//
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
