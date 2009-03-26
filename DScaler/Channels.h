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
 * @file channels.h channels Header file
 */

#ifndef __CHANNELS_H___
#define __CHANNELS_H___


//TODO->We should at least use a StringTable resource
const LPCSTR SZ_DEFAULT_CHANNELS_FILENAME  = "channel.txt"; //default channels per country file
const LPCSTR SZ_DEFAULT_PROGRAMS_FILENAME  = "program.txt"; //default user channels file

class CChannel
{
public:

    CChannel(LPCSTR Name, LPCSTR EPGName, DWORD Freq, int ChannelNumber, eVideoFormat Format, BOOL Active = TRUE);
    CChannel(LPCSTR Name, DWORD Freq, int ChannelNumber, eVideoFormat Format, BOOL Active = TRUE);
    LPCSTR GetName() const;
    LPCSTR GetEPGName() const;
    DWORD GetFrequency() const;
    void SetFrequency(DWORD newFrequency);
    int GetChannelNumber() const;
    eVideoFormat GetFormat() const;
    BOOL IsActive() const;
    void SetActive(BOOL Active);
private:
    std::string m_Name;
    std::string m_EPGName;
    DWORD m_Freq;
    int m_Chan;
    eVideoFormat m_Format;
    BOOL m_Active;
};


class CChannelList
{
public :
    CChannelList();
    virtual ~CChannelList() {};

    void Clear();

    int GetSize() const;

    int GetMinChannelNumber() const;

    int GetMaxChannelNumber() const;

    DWORD GetLowerFrequency() const;

    DWORD GetHigherFrequency() const;

    SmartPtr<CChannel> GetChannel(int iChannelIndex) const;

    SmartPtr<CChannel> GetChannelByNumber(int iChannelNumber); //May return NULL

    SmartPtr<CChannel> GetChannelByFrequency(DWORD dwFreq); //May return NULL

    //The given channel will be destroyed when list is cleared
    void AddChannel(SmartPtr<CChannel>);

    void RemoveChannel(int index);

    void SetChannel(int index, SmartPtr<CChannel> pChannel);

    void SwapChannels(int, int );

protected :

    void SetMinChannelNumber(int);
    void SetMaxChannelNumber(int);

    //update the members holding the boundary values
    //when a modification to the list is done (by adding/removing channels)
    virtual void UpdateFields();

private:
    typedef std::vector< SmartPtr<CChannel> > Channels;

    DWORD m_MinFrequency;
    DWORD m_MaxFrequency;

    int m_MinChannelNumber;
    int m_MaxChannelNumber;

    Channels m_Channels;
};


class CUserChannels : public CChannelList
{
public:
    CUserChannels();
    CUserChannels(const CUserChannels&);

    ~CUserChannels();

    //Read/Write using the legacy "program.txt" file format
    BOOL WriteFile(LPCSTR szFilename) const;
    BOOL ReadFile(LPCSTR szFilename);

private :
    BOOL WriteASCIIImpl(FILE*) const;
    BOOL ReadASCIIImpl(FILE*);
};

class CCountryList;

class CCountryChannels : public CChannelList
{
    friend class CCountryList;

public:
    CCountryChannels(LPCSTR szSomeIdentifierString, eVideoFormat eCountryVideoFormat = VIDEOFORMAT_LAST_TV);
    CCountryChannels(const CCountryChannels&);

    ~CCountryChannels();

    const LPCSTR GetCountryName() const;
    const eVideoFormat GetCountryFormat() const;

private:
    virtual BOOL WriteASCIIImpl(FILE*) const;
    virtual BOOL ReadASCIIImpl(FILE*);
    std::string m_szName;
    eVideoFormat m_Format;
};


class CCountryList
{
public :
    CCountryList();
    ~CCountryList();

    const CCountryChannels* GetChannels(int) const;

    int GetSize() const;

    int GetSize(int index) const {return GetChannels(index)->GetSize();};

    inline LPCSTR GetCountryName(int index) const {return GetChannels(index)->GetCountryName();};

    inline int GetMinChannelNumber(int index) const {return GetChannels(index)->GetMinChannelNumber();};

    inline int GetMaxChannelNumber(int index) const {return GetChannels(index)->GetMaxChannelNumber();};

    inline DWORD GetLowerFrequency(int index) const {return GetChannels(index)->GetLowerFrequency();};

    inline DWORD GetHigherFrequency(int index) const {return GetChannels(index)->GetHigherFrequency();};

    void Clear();

    BOOL ReadASCII(LPCSTR);
private:
    BOOL ReadASCIIImpl(FILE*);

    //could be a map too..would be better for lookups
    typedef std::vector<CCountryChannels*> Countries;
    Countries m_Countries;
};


#endif
