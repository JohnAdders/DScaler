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

#ifndef __CHANNELS_H___
#define __CHANNELS_H___


//TODO->We should at least use a StringTable resource
const LPCSTR SZ_DEFAULT_CHANNELS_FILENAME  = "channel.txt"; //default channels per country file
const LPCSTR SZ_DEFAULT_PROGRAMS_FILENAME  = "program.txt"; //default user channels file

class CChannel
{
public:
    
    CChannel(LPCSTR Name, DWORD Freq, int ChannelNumber, eVideoFormat Format, BOOL Active = TRUE);
    
    CChannel(const CChannel& CopyFrom);
    ~CChannel();
    LPCSTR GetName() const;
    DWORD GetFrequency() const;
    int GetChannelNumber() const;
    eVideoFormat GetFormat() const;
    BOOL IsActive() const;
    void SetActive(BOOL Active);
private:
    string m_Name;
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
        FILE_FORMAT_LASTONE
    };

private:
    //Just in case I change my mind later on the type of this
    typedef vector<CChannel*> Channels;

public :
    CChannelList();
    ~CChannelList();

    void Clear();

    int GetSize() const;
    
    int GetMinChannelNumber() const;

    int GetMaxChannelNumber() const;

    DWORD GetLowerFrequency() const;

    DWORD GetHigherFrequency() const;

    CChannel* GetChannel(int iChannelIndex) const;

    CChannel* GetChannelByNumber(int iChannelNumber); //May return NULL

    BOOL AddChannel(CChannel*); //The given channel will be destroyed when list is cleared

    inline BOOL AddChannel(LPCSTR szName, DWORD dwFreq, int iChannelNumber, int eFormat, BOOL bActive = TRUE) 
    {
        return AddChannel(szName, dwFreq, iChannelNumber, (eVideoFormat)eFormat, bActive);
    }


    BOOL AddChannel(LPCSTR szName, DWORD dwFreq, int iChannelNumber, eVideoFormat eFormat, BOOL bActive = TRUE); 

    BOOL RemoveChannel(int index);

    BOOL SetChannel(int index, CChannel* pChannel);

    BOOL SwapChannels(int, int );

    
    BOOL WriteFile(LPCSTR, CChannelList::FileFormat);

    BOOL ReadFile(LPCSTR, CChannelList::FileFormat);

    
    //Read/Write using the legacy "program.txt" file format
    inline BOOL WriteASCII(LPCSTR szFilename) {return WriteFile(szFilename, CChannelList::FILE_FORMAT_ASCII);};
    
    inline BOOL ReadASCII(LPCSTR szFilename) {return ReadFile(szFilename, CChannelList::FILE_FORMAT_ASCII);};
    
    //CChannel* operator[](int index) {return m_Channels[index];};
    
    //Shortcuts..
    inline LPCSTR GetChannelName(int index) const {return GetChannel(index)->GetName();};

    inline BOOL GetChannelActive(int index) const {return GetChannel(index)->IsActive();};

    inline void SetChannelActive(int index, BOOL bActive)
    {
        GetChannel(index)->SetActive(bActive);
    }

    inline int GetChannelNumber(int index) const {return GetChannel(index)->GetChannelNumber();};

    inline DWORD GetChannelFrequency(int index) const {return GetChannel(index)->GetFrequency();};

    inline eVideoFormat GetChannelFormat(int index) const {return GetChannel(index)->GetFormat();};


private:

    BOOL WriteASCIIImpl(LPCSTR szFilename);
    BOOL ReadASCIIImpl(LPCSTR szFilename);

private:

    DWORD m_MinFrequency;
    DWORD m_MaxFrequency;

    int m_MinChannelNumber;
    int m_MaxChannelNumber;

    Channels m_Channels;
};

class CCountry
{

protected :
    //This is legacy code and should go away...
    //I think all the classes herein can actually be gathered together
    //in just a wee bunch by using encapsulated vectors and maps
    typedef struct 
    {
        DWORD Freq;
        int Format;
    } TCountryChannel;

public:

    CCountry(LPCSTR szName);
    ~CCountry();    


    LPCSTR CCountry::GetName() const;

    inline int GetMinChannel() const {return m_MinChannel;};
    inline void SetMinChannel(int iChannelNumber)
    {
        m_MinChannel = iChannelNumber;
    }

    inline int GetMaxChannel() const {return m_MaxChannel;};
    inline void SetMaxChannel(int iChannelNumber)
    {
        m_MaxChannel = iChannelNumber;
    }
         
    
    int GetMinChannelIndex() const;
    int GetMaxChannelIndex() const;
    
    int GetMinChannelFrequency() const;
    int GetMaxChannelFrequency() const;
   
    inline int GetSize() const {return m_Frequencies.size();};

    inline DWORD GetFrequency(int iIndex) const {return m_Frequencies[iIndex].Freq;};
    inline void SetFrequency(int iIndex, DWORD dwFrequency)
    {
        m_Frequencies[iIndex].Freq = dwFrequency;
    }

    inline int GetFormat(int iIndex) const {return m_Frequencies[iIndex].Format;};
    inline void SetFormat(int iIndex, int iVideoFormat) 
    {
        m_Frequencies[iIndex].Format = iVideoFormat;
    }

   //Adds a New TCountryChannel and returns its position
   int AddChannel(DWORD dwFrequency, int iVideoFormat);
   
private :
    int m_MinChannel;
    int m_MaxChannel;   

    string m_Name;

    vector<TCountryChannel> m_Frequencies;
};


//TODO->Transform into class
typedef vector<CCountry*> COUNTRYLIST;

//TODO->Should be methods
void Unload_Country_Settings(COUNTRYLIST *);
BOOL Load_Country_Settings(LPCSTR szFilename, COUNTRYLIST * pCountries);


//Some more helpers...

#endif
