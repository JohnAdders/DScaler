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
 * @file mixerdev.h mixerdev Header file
 */

#ifndef __MIXERDEVPRIVATE_H___
#define __MIXERDEVPRIVATE_H___

//  Internal classes

/**  Individual lines of a destination (e.g. MIDI, Line-In, CD-Audio)
*/
class IMixerLineSrc
{
public:
    virtual ~IMixerLineSrc() {};
    virtual const char* GetName() = 0;

    virtual void        SetMute(BOOL bEnable) = 0;
    virtual BOOL        GetMute() = 0;

    virtual void        SetVolume(int volumePercentage) = 0;
    virtual int         GetVolume() = 0;

    virtual void        StoreState() = 0;
    virtual void        RestoreState() = 0;
};


class CMixerLineSrc : public IMixerLineSrc
{
public:
    CMixerLineSrc(HMIXER hMixer, DWORD nDstIndex, DWORD nSrcIndex);
    virtual ~CMixerLineSrc();

    const char* GetName();

    void        SetMute(BOOL bEnable);
    BOOL        GetMute();

    void        SetVolume(int volumePercentage);
    int         GetVolume();

    void        StoreState();
    void        RestoreState();

private:
    BOOL        MixerControlDetailsSet(DWORD dwControlID, DWORD dwValue);
    BOOL        MixerControlDetailsGet(DWORD dwControlID, LPDWORD lpdwValue);

private:
    HMIXER      m_hMixer;
    MIXERLINE   m_mxl;

    DWORD       m_VolumeControlID;
    DWORD       m_VolumeMinimum;
    DWORD       m_VolumeMaximum;

    DWORD       m_MuteControlID;

    DWORD       m_StoredVolume;
    DWORD       m_StoredMute;
};


/**  System mixer destination lines
*/
class IMixerLineDst
{
public:
    virtual ~IMixerLineDst() {};
    virtual const char*     GetName() = 0;

    virtual long            GetSourceCount() = 0;
    virtual IMixerLineSrc*  GetSourceLine(DWORD nIndex) = 0;

    virtual void            StoreState() = 0;
    virtual void            RestoreState() = 0;

    virtual BOOL            IsTypicalSpeakerLine() = 0;
    virtual BOOL            IsTypicalRecordingLine() = 0;
};


class CMixerLineDst : public IMixerLineDst
{
public:
    CMixerLineDst(HMIXER hMixer, DWORD nDstIndex);
    virtual ~CMixerLineDst();

    const char*     GetName();

    long            GetSourceCount();
    IMixerLineSrc*  GetSourceLine(DWORD nIndex);

    void            StoreState();
    void            RestoreState();

    BOOL            IsTypicalSpeakerLine();
    BOOL            IsTypicalRecordingLine();

private:
    MIXERLINE       m_mxl;
    std::vector< SmartPtr<IMixerLineSrc> > m_pSourceLines;
};


/**  System mixer control
*/
class IMixer
{
public:
    virtual ~IMixer() {};
    virtual const char*     GetName() = 0;
    virtual DWORD           GetIndex() = 0;

    virtual long            GetDestinationCount() = 0;
    virtual IMixerLineDst*  GetDestinationLine(DWORD nIndex) = 0;

    virtual void            StoreState() = 0;
    virtual void            RestoreState() = 0;

};

class CMixer : public IMixer
{
public:
    CMixer(DWORD nMixerIndex);
    virtual ~CMixer();

    const char*     GetName();
    DWORD           GetIndex();

    long            GetDestinationCount();
    IMixerLineDst*  GetDestinationLine(DWORD nIndex);

    void            StoreState();
    void            RestoreState();

private:
    HMIXER          m_hMixer;
    MIXERCAPS       m_mxcaps;
    DWORD           m_nMixerIndex;
    std::vector< SmartPtr<IMixerLineDst> > m_pDestinationLines;
};


/**  Provides functions for finding mixers
*/

class CMixerList
{
public:
    long GetMixerCount();
    IMixer* GetMixer(long nMixerIndex);
    long FindMixer(const char* MixerName);
    static CMixerList* getInstance();
private:
    CMixerList();
    std::vector< SmartPtr<CMixer> > m_Mixers;
};

#endif
