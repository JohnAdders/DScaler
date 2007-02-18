/////////////////////////////////////////////////////////////////////////////
// $Id: Channels.h,v 1.9 2007-02-18 17:33:57 robmuller Exp $
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
// Change Log
//
// Date          Developer             Changes
//
// 09 Oct 2002   Denis Balazuc         Original Release
//                                     Copy/paste from ProgramList.h mostly
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.8  2005/03/26 18:53:22  laurentg
// EPG code improved
// => possibility to set the EPG channel name in the channel setup dialog box
// => automatic loading of new data when needed
// => OSD scrrens updated
// => first step for programs "browser"
//
// Revision 1.7  2005/03/08 03:23:27  robmuller
// Added CVS keywords.
//
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
    
    CChannel(const CChannel& CopyFrom);
    ~CChannel();
    LPCSTR GetName() const;
    LPCSTR GetEPGName() const;
    DWORD GetFrequency() const;
    void SetFrequency(DWORD newFrequency);
    int GetChannelNumber() const;
    eVideoFormat GetFormat() const;
    BOOL IsActive() const;
    void SetActive(BOOL Active);
private:
    string m_Name;
    string m_EPGName;
    DWORD m_Freq;
    int m_Chan;
    eVideoFormat m_Format;
    BOOL m_Active;
};


class CChannelList 
{


public :
    enum FileFormat {
        FILE_FORMAT_ASCII = 0,
        FILE_FORMAT_XML,
        FILE_FORMAT_LASTONE
    };

private:
    //Just in case I change my mind later on the type of this
    typedef vector<CChannel*> Channels;

public :
    CChannelList();
    CChannelList(const CChannelList&);
    CChannelList(DWORD dwBeginFrequency, DWORD dwEndFrequency, DWORD dwSteps);
    ~CChannelList();

    void Clear();

    int GetSize() const;
    
    int GetMinChannelNumber() const;

    int GetMaxChannelNumber() const;

    DWORD GetLowerFrequency() const;

    DWORD GetHigherFrequency() const;

    CChannel* GetChannel(int iChannelIndex) const;

    CChannel* GetChannelByNumber(int iChannelNumber); //May return NULL

	CChannel* GetChannelByFrequency(DWORD dwFreq); //May return NULL 

    inline BOOL AddChannel(LPCSTR szName, DWORD dwFreq, int iChannelNumber, int eFormat, BOOL bActive = TRUE) 
    {
        return AddChannel(szName, dwFreq, iChannelNumber, (eVideoFormat)eFormat, bActive);
    }


    BOOL AddChannel(LPCSTR szName, DWORD dwFreq, int iChannelNumber, eVideoFormat eFormat, BOOL bActive = TRUE); 

    BOOL AddChannel(LPCSTR szName, LPCSTR szEPGName, DWORD dwFreq, int iChannelNumber, eVideoFormat eFormat, BOOL bActive = TRUE); 

    BOOL AddChannel(LPCSTR szName, DWORD dwFreq, eVideoFormat eFormat, BOOL bActive = TRUE); 

    BOOL AddChannel(LPCSTR szName, LPCSTR szEPGName, DWORD dwFreq, eVideoFormat eFormat, BOOL bActive = TRUE); 

    //Generates a name for you as the channels are not named
    //in the channels.txt file (channels/country)
    //It would be nice to use the comment next to the frequency
    //but some are really too large..
    BOOL AddChannel(DWORD dwFrequency, int iChannelNumber, eVideoFormat eVideoFormat = VIDEOFORMAT_LAST_TV, BOOL bActive = TRUE);

    //The given channel will be destroyed when list is cleared
    BOOL AddChannel(CChannel*);
    
    BOOL RemoveChannel(int index);

    BOOL SetChannel(int index, CChannel* pChannel);

    BOOL SwapChannels(int, int );

    int AddChannels(const CChannelList* const);
    
    BOOL WriteFile(LPCSTR, CChannelList::FileFormat)  const;

    BOOL ReadFile(LPCSTR, CChannelList::FileFormat);

    
    //Read/Write using the legacy "program.txt" file format
    inline BOOL WriteASCII(LPCSTR szFilename)  const {return WriteFile(szFilename, CChannelList::FILE_FORMAT_ASCII);};
    
    inline BOOL ReadASCII(LPCSTR szFilename) {return ReadFile(szFilename, CChannelList::FILE_FORMAT_ASCII);};
    
    inline BOOL WriteXML(LPCSTR szFilename)  const {return WriteFile(szFilename, CChannelList::FILE_FORMAT_XML);};
    
    inline BOOL ReadXML(LPCSTR szFilename) {return ReadFile(szFilename, CChannelList::FILE_FORMAT_XML);};
    
    
    //Shortcuts..
    inline LPCSTR GetChannelName(int index) const {return GetChannel(index)->GetName();};

    inline LPCSTR GetChannelEPGName(int index) const {return GetChannel(index)->GetEPGName();};

    inline BOOL GetChannelActive(int index) const {return GetChannel(index)->IsActive();};

    inline void SetChannelActive(int index, BOOL bActive)
    {
        GetChannel(index)->SetActive(bActive);
    }

    inline int GetChannelNumber(int index) const {return GetChannel(index)->GetChannelNumber();};

    inline DWORD GetChannelFrequency(int index) const {return GetChannel(index)->GetFrequency();};

    inline eVideoFormat GetChannelFormat(int index) const {return GetChannel(index)->GetFormat();};

   
protected :

    void SetMinChannelNumber(int);
    void SetMaxChannelNumber(int);

    //update the members holding the boundary values 
    //when a modification to the list is done (by adding/removing channels)
    virtual void UpdateFields();

    virtual BOOL WriteASCIIImpl(FILE*)  const= 0;
    virtual BOOL ReadASCIIImpl(FILE*) = 0;

    virtual BOOL WriteXMLImpl(FILE*)  const= 0;
    virtual BOOL ReadXMLImpl(FILE*) = 0;

private:

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

protected :
    
    BOOL WriteASCIIImpl(FILE*) const;
    BOOL ReadASCIIImpl(FILE*);

    BOOL WriteXMLImpl(FILE*) const;
    BOOL ReadXMLImpl(FILE*);

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
    
protected :
   
    BOOL WriteASCIIImpl(FILE*) const;
    BOOL ReadASCIIImpl(FILE*);

    BOOL WriteXMLImpl(FILE*) const;
    BOOL ReadXMLImpl(FILE*);

private:
    string m_szName;
    eVideoFormat m_Format;
};


class CCountryList 
{
private:
    //could be a map too..would be better for lookups
    typedef vector<CCountryChannels*> Countries;

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

    //Adds the content of the given channel list
    //(makes a deep copy)    
    BOOL AddChannels(LPCSTR szName, CChannelList*);
    
    //Makes a shallow copy only
    BOOL AddChannels(CCountryChannels*);

    BOOL RemoveChannels(int);
   
    void Clear();
    

    BOOL ReadASCII(FILE*);
    BOOL ReadASCII(LPCSTR);

    BOOL WriteXML(FILE*) const;
    BOOL WriteXML(LPCSTR) const;

private:
    Countries m_Countries;

};


#endif
