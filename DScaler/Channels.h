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
    CChannel(LPCSTR Name, DWORD Freq, int ChannelNumber, int Format, BOOL Active);
    CChannel(const CChannel& CopyFrom);
    ~CChannel();
    LPCSTR GetName() const;
    DWORD GetFrequency() const;
    int GetChannelNumber() const;
    int GetFormat() const;
    BOOL IsActive() const;
    void SetActive(BOOL Active);
private:
    string m_Name;
    DWORD m_Freq;
    int m_Chan;
    int m_Format;
    BOOL m_Active;
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
typedef vector<CChannel*> CHANNELLIST;

//TODO->Should be methods
void Unload_Program_List_ASCII(CHANNELLIST * pChannels);
BOOL Load_Program_List_ASCII(LPCSTR szFilename, CHANNELLIST * pChannels);
BOOL Write_Program_List_ASCII(LPCSTR szFilename, CHANNELLIST * pChannels);


//TODO->Transform into class
typedef vector<CCountry*> COUNTRYLIST;

//TODO->Should be methods
void Unload_Country_Settings(COUNTRYLIST *);
BOOL Load_Country_Settings(LPCSTR szFilename, COUNTRYLIST * pCountries);

#endif
