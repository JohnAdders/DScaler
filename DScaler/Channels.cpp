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
//                                     mostly cut/paste from ProgramList.cpp
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TVFormats.h"
#include "Channels.h"


CChannel::CChannel(LPCSTR Name, DWORD Freq, int ChannelNumber, eVideoFormat Format, BOOL Active)
{        
    m_Name = Name;
    m_Freq = Freq;
    m_Chan = ChannelNumber;
    m_Format = Format;
    m_Active = Active;
}

CChannel::CChannel(const CChannel& CopyFrom)
{
    m_Name = CopyFrom.m_Name;
    m_Freq = CopyFrom.m_Freq;
    m_Chan = CopyFrom.m_Chan;
    m_Format = CopyFrom.m_Format;
    m_Active = CopyFrom.m_Active;
}

CChannel::~CChannel()
{
}

LPCSTR CChannel::GetName() const
{
    static char sbuf[256];
    strncpy(sbuf, m_Name.c_str(), 255);
    sbuf[255] = '\0';
    return sbuf;
}

 
DWORD CChannel::GetFrequency() const
{
    return m_Freq;
}

int CChannel::GetChannelNumber() const
{
    return m_Chan;
}

eVideoFormat CChannel::GetFormat() const
{
    return m_Format;
}

BOOL CChannel::IsActive() const
{
    return m_Active;
}

void CChannel::SetActive(BOOL Active)
{
    m_Active = Active;
}


CCountry::CCountry(LPCSTR szName)
{           
    m_Name = szName;
    m_MinChannel = 0;
    m_MaxChannel = 0;       
}

CCountry::~CCountry()
{
    m_Frequencies.clear();
}


int CCountry::GetMinChannelFrequency() const 
{    
    return m_Frequencies[GetMinChannelIndex()].Freq;
}

int CCountry::GetMaxChannelFrequency() const 
{
    return m_Frequencies[GetMaxChannelIndex()].Freq;
}


//just a wee helper..makes sure the index is correct by returning a more sensible one
int boundedChannelIndex(const CCountry * const pCountry, int iIndex) 
{
    int dummy = iIndex;
    if (dummy < 0) 
    {
        dummy = 0;
    }
    else if (dummy >= pCountry->GetSize()) 
    {
        dummy = pCountry->GetSize() - 1;
    }
    return dummy;
}


//We check bounds because sometimes,
//the number of frequs is lower than the declared higher rank
//(As for the [Argentina Cable Frequency] entries I have
int CCountry::GetMinChannelIndex() const 
{   
    return boundedChannelIndex(this, m_MinChannel - 1);
}

int CCountry::GetMaxChannelIndex() const 
{
    return boundedChannelIndex(this, m_MaxChannel - 1);
}



//Adds a New TCountryChannel and returns its position
int CCountry::AddChannel(DWORD dwFrequency, int iVideoFormat)
{
    TCountryChannel channel;
    channel.Freq = dwFrequency;
    channel.Format = iVideoFormat;

    m_Frequencies.push_back(channel);
   
    return m_Frequencies.size() - 1;
}

LPCSTR CCountry::GetName() const 
{
    static char sbuf[256];
    strncpy(sbuf, m_Name.c_str(), 255);
    sbuf[255] = '\0';
    return sbuf;    
}   



//That is an utility function that was in ProgramList.cpp
//Used in Load_Country_Settings
int StrToVideoFormat(char* pszFormat)
{
    for(int a = 0; a < VIDEOFORMAT_LASTONE; ++a)
    {
        if(!stricmp(pszFormat, VideoFormatNames[a]))
        {
            return a;
        }  
    }
   
    return -1;
}

//TODO - Transform into Class member or something
BOOL Load_Country_Settings(LPCSTR szFilename, COUNTRYLIST * pCountries)
{
    if ((NULL == szFilename) || (NULL == pCountries))
    {
        return FALSE;
    }
    
    FILE*     CountryFile;
    char      line[128];
    char*     Pos;
    char*     Pos1;
    char*     eol_ptr;
    string    Name;
    CCountry* NewCountry = NULL;
    int       Format = -1;
    
    
    if ((CountryFile = fopen(szFilename, "r")) == NULL)
    {
        return FALSE;
    }

    while (fgets(line, sizeof(line), CountryFile) != NULL)
    {
        eol_ptr = strstr(line, ";");
        if (eol_ptr == NULL)
        {
            eol_ptr = strstr(line, "\n");
        }
        if(eol_ptr != NULL)
        {
            *eol_ptr = '\0';
        }
        if(eol_ptr == line)
        {
            continue;
        }
        if(((Pos = strstr(line, "[")) != 0) && ((Pos1 = strstr(line, "]")) != 0) && Pos1 > Pos)
        {
            if(NewCountry != NULL)
            {
                pCountries->push_back(NewCountry);
            }
            Pos++;
            
            //NewCountry = new CCountry();
            //NewCountry->m_Name = Pos;
            //NewCountry->m_Name[Pos1-Pos] = '\0';
            char * dummy = Pos;
            dummy[Pos1-Pos] = '\0';                        
            NewCountry = new CCountry((LPCSTR)dummy);
            Format = -1;
        }
        else
        {
            if (NewCountry == NULL)
            {
                fclose(CountryFile);                
                return FALSE;
            }
            
            if ((Pos = strstr(line, "ChannelLow=")) != 0)
            {
                NewCountry->SetMinChannel(atoi(Pos + strlen("ChannelLow=")));
            }
            else if ((Pos = strstr(line, "ChannelHigh=")) != 0)
            {
                NewCountry->SetMaxChannel(atoi(Pos + strlen("ChannelHigh=")));
            }
            else if ((Pos = strstr(line, "Format=")) != 0)
            {
                Format = StrToVideoFormat(Pos + strlen("Format="));
            }
            else
            {
                Pos = line;
                while (*Pos != '\0')
                {
                    if ((*Pos >= '0') && (*Pos <= '9'))
                    {                        
                        //TCountryChannel Channel;
                        //Channel.Freq = atol(Pos);
                        // convert frequency in KHz to Units that the tuner wants
                        //Channel.Freq = MulDiv(Channel.Freq, 16, 1000000);
                        //Channel.Format = Format;
                        NewCountry->AddChannel(atol(Pos), Format);
                        break;
                    }
                    Pos++;
                }
            }
        }
    }
    if(NewCountry != NULL)
    {
        pCountries->push_back(NewCountry);
    }

    fclose(CountryFile);
    return TRUE;
}


CChannelList::CChannelList() : m_Channels()
{
    
  m_MinFrequency = 0;
  m_MaxFrequency = 0;

  m_MinChannelNumber = 0;
  m_MaxChannelNumber = 0;
}

CChannelList::~CChannelList()
{           
    Clear();
}


void CChannelList::Clear()
{
    for(Channels::iterator it = m_Channels.begin();
        it != m_Channels.end();
        ++it)
    {
        delete (*it);
    }
    m_Channels.clear();
}


int CChannelList::GetSize()  const
{
    return m_Channels.size();
}

int CChannelList::GetMinChannelNumber()  const
{
    return m_MinChannelNumber;
}

int CChannelList::GetMaxChannelNumber()  const
{
    return m_MaxChannelNumber;
}

DWORD CChannelList::GetLowerFrequency()  const
{
    return m_MinFrequency;
}
    
DWORD CChannelList::GetHigherFrequency()  const
{
    return m_MaxFrequency;
}

CChannel* CChannelList::GetChannel(int index)  const
{
    ASSERT(index >= 0);
    ASSERT(index < m_Channels.size());    
    return m_Channels[index];
}


CChannel* CChannelList::GetChannelByNumber(int iChannelNumber) 
{
    CChannel* returned = NULL;
    
    for(Channels::iterator it = m_Channels.begin();
        it != m_Channels.end();
        ++it)
    {
        if ((*it)->GetChannelNumber() == iChannelNumber)
        {
            returned = (*it);
            break;
        }
    }
    return returned;        
}


BOOL CChannelList::AddChannel(CChannel* pChannel)
{
    BOOL returned = FALSE;
    if (NULL != pChannel) 
    {
        //We dont want to add the same channel twice
        //("Same" meaning, a shallow ==)
        BOOL found = FALSE;
        for (int i = 0; i < GetSize(); i++)
        {
            CChannel* dummy = GetChannel(i);
            if (dummy == pChannel)
            {
                found = TRUE;
                break;
            }           
        }    
        if (found == FALSE)
        {
            m_Channels.push_back(pChannel);
            returned = TRUE;
        }
    }
    return returned;
}


BOOL CChannelList::AddChannel(LPCSTR szName, DWORD dwFreq, int iChannelNumber, eVideoFormat eFormat, BOOL bActive)
{
    CChannel* newChannel = new CChannel(szName, dwFreq, iChannelNumber, eFormat, bActive);
    return AddChannel(newChannel);
}


BOOL CChannelList::RemoveChannel(int index)
{
    if ((index < 0) || (index >= GetSize())) 
    {
        return FALSE;
    }
    //this sequence is truly bizarre...
    delete m_Channels[index];
    m_Channels.erase(&m_Channels[index]);      
    return TRUE;
}


BOOL CChannelList::SetChannel(int index, CChannel* pChannel)
{
    BOOL returned  = FALSE;
        
    if ((index >= 0) && (NULL != pChannel))     
    {
        if (index == GetSize() - 1)
        {
            returned = AddChannel(pChannel);
        }   
        else 
        {
            CChannel* oldChannel = GetChannel(index);
            if (pChannel == oldChannel)
            {
                //same - do nothing (and not null)
                returned = TRUE;
            }
            else if (NULL != oldChannel)
            {                
                delete oldChannel;
                m_Channels[index] = pChannel;
            }
        }
    }
    return returned;
}


BOOL CChannelList::SwapChannels(int a, int b)
{
    if ((a < 0) || (a >= GetSize()) || (b < 0) || (b >= GetSize()))
    {
        return FALSE;
    }
    if (a == b) 
    {
        //now, that is very funny...
        return TRUE;
    }

    CChannel* channelA = GetChannel(a);
    CChannel* channelB = GetChannel(b);    

    m_Channels[a] = channelB;
    m_Channels[b] = channelA;
    
    return true;
}
    

BOOL CChannelList::WriteFile(LPCSTR szFilename, CChannelList::FileFormat format)
{
    if (NULL == szFilename)
    {
        return FALSE;
    }

    switch (format)
    {
    case FILE_FORMAT_ASCII:
        return WriteASCIIImpl(szFilename);        

    default :
        return false;
    }
}


BOOL CChannelList::ReadFile(LPCSTR szFilename, CChannelList::FileFormat format)
{
    if (NULL == szFilename)
    {
        return FALSE;
    }
    
    //DB Note : note sure you want this...
    Clear();

    switch (format)
    {
    case FILE_FORMAT_ASCII:
        return ReadASCIIImpl(szFilename);        

    default :
        return false;
    }
}


//TODO - Transform into Class member or something
BOOL CChannelList::ReadASCIIImpl(LPCSTR szFilename)
{
    ASSERT(NULL != szFilename);

    char sbuf[256];
    FILE* SettingFile;  
    DWORD Frequency = -1;
    int Channel = 1;
    int Format = -1;
    BOOL Active = TRUE;
    string Name;
    
    SettingFile = fopen(szFilename, "r");
    if (SettingFile == NULL)
    {
        return FALSE;
    }
    while(!feof(SettingFile))
    {
        sbuf[0] = '\0';

        fgets(sbuf, 255, SettingFile);

        char* eol_ptr = strstr(sbuf, ";");
        if (eol_ptr == NULL)
        {
            eol_ptr = strstr(sbuf, "\n");
        }
        if (eol_ptr != NULL)
        {
            *eol_ptr = '\0';
        }


        if(strnicmp(sbuf, "Name:", 5) == 0)
        {
            if(Frequency != -1)
            {
                AddChannel(new CChannel(Name.c_str(), Frequency, Channel, (eVideoFormat)Format, Active));
            }

            // skip "Name:"
            char* StartChar = sbuf + 5;

            // skip any spaces
            while(iswspace(*StartChar))
            {
                ++StartChar;
            }
            if(strlen(StartChar) > 0)
            {
                char* EndChar = StartChar + strlen(StartChar) - 1;
                while(EndChar > StartChar && iswspace(*EndChar))
                {
                    *EndChar = '\0';
                    --EndChar;
                }
                Name = StartChar;
            }
            else
            {
                Name = "Empty";
            }
            Frequency = -1;
            ++Channel;
            Format = -1;
            Active = TRUE;
        }
        // cope with old style frequencies
        else if(strnicmp(sbuf, "Freq:", 5) == 0)
        {
            Frequency = atol(sbuf + 5);           
            Frequency = Frequency * 1000;
        }
        else if(strnicmp(sbuf, "Freq2:", 6) == 0)
        {
            Frequency = atol(sbuf + 6);
            Frequency = MulDiv(Frequency, 1000000, 16);
        }
        else if(strnicmp(sbuf, "Chan:", 5) == 0)
        {
            Channel = atoi(sbuf + 5);
        }
        else if(strnicmp(sbuf, "Form:", 5) == 0)
        {
            Format = atoi(sbuf + 5);
        }
        else if(strnicmp(sbuf, "Active:", 7) == 0)
        {
            Active = (atoi(sbuf + 7) != 0);
        }
        else
        {
            ; //some other rubbish
        }
    }

    if(Frequency != -1)
    {
        AddChannel(new CChannel(Name.c_str(), Frequency, Channel, (eVideoFormat)Format, Active));
    }

    fclose(SettingFile);
    return TRUE;
}


// 
// Save ascii formatted program list
//
// 9 Novemeber 2000 - Michael Eskin, Conexant Systems
// List is a simple text file with the following format:
// Name <display_name>
// Freq <frequency_KHz>
// Name <display_name>
// Freq <frequency_KHz>
// ...
// 10 October 2002 - Denis Balazuc
// Moved to Channels.cpp

BOOL CChannelList::WriteASCIIImpl(LPCSTR szFilename)
{
    ASSERT(NULL != szFilename);
    
    BOOL bSuccess = FALSE;
    FILE* SettingFile;    
    
    if ((SettingFile = fopen(szFilename, "w")) != NULL)
    {
        for(int i = 0; i < GetSize(); i++)
        {
            fprintf(SettingFile, "Name: %s\n", GetChannelName(i));
            //fprintf(SettingFile, "Freq2: %ld\n", MulDiv((*it)->GetFrequency(),16,1000000));
            fprintf(SettingFile, "Freq: %ld\n", GetChannelFrequency(i)/1000);
            fprintf(SettingFile, "Chan: %d\n", GetChannelNumber(i));
            fprintf(SettingFile, "Active: %d\n", GetChannelActive(i));
            if(GetChannelFormat(i) != -1)
            {
                fprintf(SettingFile, "Form: %d\n", GetChannelFormat(i));
            }
        }
        fclose(SettingFile);
        bSuccess = TRUE;
    }    
    return bSuccess;
}



//TODO - Transform into Class member or something
void Unload_Country_Settings(COUNTRYLIST * pCountries)
{
    if (NULL == pCountries) 
    {
        return;
    }
    
    COUNTRYLIST::iterator it;

    // Zero out the program list
    for(it = pCountries->begin(); it != pCountries->end(); ++it)
    {
        delete (*it);
    }
    pCountries->clear();
}
