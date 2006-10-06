/////////////////////////////////////////////////////////////////////////////
// $Id: Channels.cpp,v 1.12 2006-10-06 13:35:28 adcockj Exp $
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.11  2005/03/26 18:53:22  laurentg
// EPG code improved
// => possibility to set the EPG channel name in the channel setup dialog box
// => automatic loading of new data when needed
// => OSD scrrens updated
// => first step for programs "browser"
//
// Revision 1.10  2005/03/08 03:25:26  robmuller
// Added CVS keywords.
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file Channels.cpp Channels Classes
 */

#include "stdafx.h"
#include "DebugLog.h"
#include "TVFormats.h"
#include "Channels.h"


CChannel::CChannel(LPCSTR Name, LPCSTR EPGName, DWORD Freq, int ChannelNumber, eVideoFormat Format, BOOL Active)
{        
    m_Name = Name;
	m_EPGName = EPGName;
    m_Freq = Freq;
    m_Chan = ChannelNumber;
    m_Format = Format;
    m_Active = Active;
}

CChannel::CChannel(LPCSTR Name, DWORD Freq, int ChannelNumber, eVideoFormat Format, BOOL Active)
{        
    m_Name = Name;
	m_EPGName = m_Name;
    m_Freq = Freq;
    m_Chan = ChannelNumber;
    m_Format = Format;
    m_Active = Active;
}

CChannel::CChannel(const CChannel& CopyFrom)
{
    m_Name = CopyFrom.m_Name;
    m_EPGName = CopyFrom.m_EPGName;
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

LPCSTR CChannel::GetEPGName() const
{
    static char sbuf[256];
    strncpy(sbuf, m_EPGName.c_str(), 255);
    sbuf[255] = '\0';
    return sbuf;
}
 
DWORD CChannel::GetFrequency() const
{
    return m_Freq;
}

void CChannel::SetFrequency(DWORD newFrequency) 
{
    m_Freq = newFrequency;
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


// ------------ CChannelList BEGIN ------------
// --------------------------------------------

CChannelList::CChannelList() : m_Channels()
{
    
    m_MinFrequency = 0;
    m_MaxFrequency = 0;

    m_MinChannelNumber = 0;
    m_MaxChannelNumber = 0;
}

CChannelList::CChannelList(const CChannelList& pCopy) : m_Channels()
{
    
    m_MinFrequency = pCopy.m_MinFrequency;
    m_MaxFrequency = pCopy.m_MaxFrequency;

    m_MinChannelNumber = pCopy.m_MinChannelNumber;
    m_MaxChannelNumber = pCopy.m_MaxChannelNumber;

    AddChannels(&pCopy);
}


CChannelList::~CChannelList()
{              
    Clear();
}


void CChannelList::Clear()
{
    m_MinFrequency = 0;
    m_MaxFrequency = 0;

    m_MinChannelNumber = 0;
    m_MaxChannelNumber = 0;

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

void CChannelList::SetMinChannelNumber(int newMin)
{
    m_MinChannelNumber = newMin;
}
    

int CChannelList::GetMaxChannelNumber()  const
{
    return m_MaxChannelNumber;
}


void CChannelList::SetMaxChannelNumber(int newMax)
{
    m_MaxChannelNumber = newMax;
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

CChannel* CChannelList::GetChannelByFrequency(DWORD dwFreq) 
{
    CChannel* returned = NULL;
    
    for(Channels::iterator it = m_Channels.begin();
        it != m_Channels.end();
        ++it)
    {
        if ((*it)->GetFrequency() == dwFreq)
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
        m_Channels.push_back(pChannel);
        UpdateFields();
        returned = TRUE;
    }
    return returned;
}


BOOL CChannelList::AddChannel(LPCSTR szName, DWORD dwFreq, int iChannelNumber, eVideoFormat eFormat, BOOL bActive)
{
    CChannel* newChannel = new CChannel(szName, dwFreq, iChannelNumber, eFormat, bActive);
    return AddChannel(newChannel);
}

BOOL CChannelList::AddChannel(LPCSTR szName, LPCSTR szEPGName, DWORD dwFreq, int iChannelNumber, eVideoFormat eFormat, BOOL bActive)
{
    CChannel* newChannel = new CChannel(szName, szEPGName, dwFreq, iChannelNumber, eFormat, bActive);
    return AddChannel(newChannel);
}

BOOL CChannelList::AddChannel(LPCSTR szName, DWORD dwFreq, eVideoFormat eFormat, BOOL bActive)
{    
    return AddChannel(szName, dwFreq, m_MaxChannelNumber + 1, eFormat, bActive);
}

BOOL CChannelList::AddChannel(LPCSTR szName, LPCSTR szEPGName, DWORD dwFreq, eVideoFormat eFormat, BOOL bActive)
{    
    return AddChannel(szName, szEPGName, dwFreq, m_MaxChannelNumber + 1, eFormat, bActive);
}

BOOL CChannelList::AddChannel(DWORD dwFrequency, int iChannelNumber, eVideoFormat eVideoFormat, BOOL bActive)
{
    static char sbuf[256];           
    sprintf(sbuf, "Channel %d", iChannelNumber);    
    return AddChannel(sbuf, dwFrequency, iChannelNumber, eVideoFormat, bActive);
}

int CChannelList::AddChannels(const CChannelList* const pChannels)
{
    ASSERT(NULL != pChannels);

    int count = 0;
    
    for (int i = 0; i < pChannels->GetSize(); i++)
    {
      if (AddChannel(new CChannel(*pChannels->GetChannel(i)))) 
      {
          count++;
      }
    }

    ASSERT(count == pChannels->GetSize());
    return count; //s/b=pChannels->GetSize()
}

BOOL CChannelList::RemoveChannel(int index)
{
    if ((index < 0) || (index >= GetSize())) 
    {
        return FALSE;
    }
    //this sequence is truly bizarre...
    delete m_Channels[index];
    m_Channels.erase(m_Channels.begin() + index);      
    UpdateFields();
    return TRUE;
}


BOOL CChannelList::SetChannel(int index, CChannel* pChannel)
{
    BOOL returned  = FALSE;
        
    if ((index >= 0) && (NULL != pChannel))     
    {
        if (index >= GetSize())
        {
            returned = AddChannel(pChannel);
        }   
        else 
        {
            CChannel* oldChannel = GetChannel(index);            
            //if same - do nothing 
            if ((NULL != oldChannel) && (pChannel != oldChannel))
            {                        
                delete oldChannel;
                m_Channels[index] = pChannel;
                UpdateFields();
                returned = TRUE;
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
    

void CChannelList::UpdateFields()
{  
    if (m_MinFrequency == 0)
    {
        m_MinFrequency = (GetSize() == 0) ? 0 : GetChannel(0)->GetFrequency();
    }

    if (m_MinChannelNumber == 0)
    {
        m_MinChannelNumber = (GetSize() == 0) ? 0 : GetChannel(0)->GetChannelNumber();
    }


    CChannel* returned = NULL;
    
    for(int i = 0; i < GetSize(); i++)
    {
        m_MinFrequency = min(m_MinFrequency, GetChannelFrequency(i));
        m_MaxFrequency = max(m_MaxFrequency, GetChannelFrequency(i));
        
        m_MinChannelNumber = min(m_MinChannelNumber, GetChannelNumber(i));
        m_MaxChannelNumber = max(m_MaxChannelNumber, GetChannelNumber(i));
    }   
}

BOOL CChannelList::WriteFile(LPCSTR szFilename, CChannelList::FileFormat format)  const
{
    if (NULL == szFilename)
    {
        return FALSE;
    }

    BOOL success = FALSE;
    FILE* file = fopen(szFilename, "w");
    if (NULL != file) {
        switch (format)
        {
        case FILE_FORMAT_ASCII:
            success = WriteASCIIImpl(file);        
            break;

        case FILE_FORMAT_XML:
            success = WriteXMLImpl(file);        
            break;
        default :
            break;
        }//switch
        success = (fclose(file) == 0) && success;
        file = NULL;
    }

    return success;
}


BOOL CChannelList::ReadFile(LPCSTR szFilename, CChannelList::FileFormat format) 
{
    if (NULL == szFilename)
    {
        return FALSE;
    }

    BOOL success = FALSE;
    FILE* file = fopen(szFilename, "r");
    if (NULL != file) {
        switch (format)
        {
        case FILE_FORMAT_ASCII:
            success = ReadASCIIImpl(file);        
            break;

        case FILE_FORMAT_XML:
            success = ReadXMLImpl(file);        
            break;

        default :
            break;

        }//switch
        success = (fclose(file) == 0) && success;            
        file = NULL;
    }
    return success;
}


// ------------ CChannelList END --------------
// --------------------------------------------

// ------------ CUserChannels BEGIN ------------
// --------------------------------------------

CUserChannels::CUserChannels() : CChannelList()
{
}

CUserChannels::CUserChannels(const CUserChannels& pCopy) : CChannelList((CChannelList&)pCopy)
{
}


CUserChannels::~CUserChannels()
{
}


BOOL CUserChannels::ReadASCIIImpl(FILE* SettingFile)
{
    ASSERT(NULL != SettingFile);

    char sbuf[256];    
    DWORD Frequency = -1;
    int Channel = 1;
    int Format = -1;
    BOOL Active = TRUE;
    string Name;
    string EPGName;
    
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
                AddChannel(new CChannel(Name.c_str(), EPGName.c_str(), Frequency, Channel, (eVideoFormat)Format, Active));
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
			EPGName = Name;
            Frequency = -1;
            ++Channel;
            Format = -1;
            Active = TRUE;
        }
        else if(strnicmp(sbuf, "EPGName:", 8) == 0)
        {
            // skip "Name:"
            char* StartChar = sbuf + 8;

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
                EPGName = StartChar;
            }
        }
        // cope with old style frequencies
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
        AddChannel(new CChannel(Name.c_str(), EPGName.c_str(), Frequency, Channel, (eVideoFormat)Format, Active));
    }
    
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

BOOL CUserChannels::WriteASCIIImpl(FILE* SettingFile)  const
{
    ASSERT(NULL != SettingFile);
    
    BOOL bSuccess = FALSE;
    
    for(int i = 0; i < GetSize(); i++)
    {
        fprintf(SettingFile, "Name: %s\n", GetChannelName(i));
		if (strcmp(GetChannelName(i), GetChannelEPGName(i)))
	        fprintf(SettingFile, "EPGName: %s\n", GetChannelEPGName(i));
        //fprintf(SettingFile, "Freq2: %ld\n", MulDiv((*it)->GetFrequency(),16,1000000));
        fprintf(SettingFile, "Freq: %ld\n", GetChannelFrequency(i)/1000);
        fprintf(SettingFile, "Chan: %d\n", GetChannelNumber(i));
        fprintf(SettingFile, "Active: %d\n", GetChannelActive(i));
        if(GetChannelFormat(i) != -1)
        {
            fprintf(SettingFile, "Form: %d\n", GetChannelFormat(i));
        }
    }
    
    bSuccess = TRUE;
    
    return bSuccess;
}

BOOL CUserChannels::ReadXMLImpl(FILE* file)
{
    BOOL returned = FALSE;

    

    return returned;
}

BOOL CUserChannels::WriteXMLImpl(FILE* SettingFile)  const
{
    //quick and dirty
    ASSERT(NULL != SettingFile);
    
    BOOL bSuccess = FALSE;
    //fprintf(SettingFile, "<?xml-stylesheet type=\"text/xsl\" href=\"program-list.xsl\"?>");
    fprintf(SettingFile, "<channel-list>\n");
    for(int i = 0; i < GetSize(); i++)
    {
        fprintf(SettingFile, "\t<channel id=\"%d\">\n", i);
        fprintf(SettingFile, "\t\t<name>%s</name>\n", GetChannelName(i));            
		if (strcmp(GetChannelName(i), GetChannelEPGName(i)))
	        fprintf(SettingFile, "\t\t<EPGname>%s</EPGname>\n", GetChannelEPGName(i));            
        fprintf(SettingFile, "\t\t<frequency>%ld</frequency>\n", GetChannelFrequency(i));//watch out. ASCII has freq/1000
        fprintf(SettingFile, "\t\t<number>%d</number>\n", GetChannelNumber(i));
        fprintf(SettingFile, "\t\t<active>%d</active>\n", GetChannelActive(i));
        if(GetChannelFormat(i) != -1)
        {
            fprintf(SettingFile, "\t\t<format>%d</format>\n", GetChannelFormat(i));
        }
        fprintf(SettingFile, "\t</channel>\n");
    }//for
    fprintf(SettingFile, "</channel-list>\n");
    bSuccess = TRUE;
    
    return bSuccess;//TRUE
}


// ------------ CUserChannels END --------------
// --------------------------------------------

// ------------ CCountryChannels BEGIN ------------
// --------------------------------------------


CCountryChannels::CCountryChannels(LPCSTR szSomeIdentifierString, eVideoFormat eCountryVideoFormat) : CChannelList()
{
    m_szName = szSomeIdentifierString;
    m_Format = eCountryVideoFormat;
}


CCountryChannels::CCountryChannels(const CCountryChannels& pCopy) : CChannelList((CChannelList&)pCopy)
{        
    //XXX->I'm not sure about this...
    m_szName = pCopy.GetCountryName();
    m_Format = pCopy.GetCountryFormat();
}


CCountryChannels::~CCountryChannels()
{    
}

const LPCSTR CCountryChannels::GetCountryName() const 
{
    static char sbuf[256];
    strncpy(sbuf, m_szName.c_str(), 255);
    sbuf[255] = '\0';
    return sbuf;    
}  

const eVideoFormat CCountryChannels::GetCountryFormat() const 
{
    return m_Format;
}


BOOL CCountryChannels::WriteASCIIImpl(FILE* file) const
{
    return FALSE; //never been implemented...
}


BOOL CCountryChannels::ReadASCIIImpl(FILE* file)
{
    return FALSE;
}


BOOL CCountryChannels::WriteXMLImpl(FILE* xmlCountryFile)  const 
{
   
    ASSERT(NULL != xmlCountryFile);
    
    BOOL bSuccess = FALSE;
        
    fprintf(xmlCountryFile, "\t<country>\n");
    fprintf(xmlCountryFile, "\t\t<name>%s</name>\n", GetCountryName());
    fprintf(xmlCountryFile, "\t\t<format>%d</format>\n", GetCountryFormat());
    for(int i = 0; i < GetSize(); i++)
    {
        fprintf(xmlCountryFile, "\t\t<channel id=\"%d\">\n", i);
        fprintf(xmlCountryFile, "\t\t\t<name>%s</name>\n", GetChannelName(i));            
        fprintf(xmlCountryFile, "\t\t\t<frequency>%ld</frequency>\n", GetChannelFrequency(i));
        fprintf(xmlCountryFile, "\t\t\t<number>%d</number>\n", GetChannelNumber(i));
        fprintf(xmlCountryFile, "\t\t\t<active>%d</active>\n", GetChannelActive(i));
        if(GetChannelFormat(i) != -1)
        {
            fprintf(xmlCountryFile, "\t\t\t<format>%d</format>\n", GetChannelFormat(i));
        }
        fprintf(xmlCountryFile, "\t\t</channel>\n");
    }//for each channel
    fprintf(xmlCountryFile, "\t</country>\n");
    
    bSuccess = TRUE;    
    return bSuccess; 
}

BOOL CCountryChannels::ReadXMLImpl(FILE* file)
{
    return FALSE;
}


// ------------ CCountryChannels END --------------
// ------------------------------------------------

// ------------------------------------------------
// ------------ CCountryList BEGIN ------------------

CCountryList::CCountryList() : m_Countries()
{
}

CCountryList::~CCountryList()
{
    Clear();
}

int CCountryList::GetSize() const
{
    return m_Countries.size();
}

void CCountryList::Clear()
{   
    for(Countries::iterator it = m_Countries.begin();
        it != m_Countries.end();
        ++it)
    {
        delete (*it);
    }
    m_Countries.clear();
}


const CCountryChannels* CCountryList::GetChannels(int index)  const
{
    ASSERT(index >= 0);
    ASSERT(index < m_Countries.size());    
    return m_Countries[index];
}


BOOL CCountryList::AddChannels(LPCSTR szName, CChannelList* pChannels)
{
    BOOL returned = FALSE;
    if (NULL != pChannels) 
    {
        CCountryChannels * newChannelList = new CCountryChannels(szName);
        for (int i = 0; i < pChannels->GetSize(); i++)
        {
            CChannel channel = *pChannels->GetChannel(i);               
            CChannel * newChannel = new CChannel(channel);
            newChannelList->CChannelList::AddChannel(newChannel);
        }

        m_Countries.push_back(newChannelList);            
        returned = TRUE; 
    }
    return returned;
}


BOOL CCountryList::AddChannels(CCountryChannels* pChannels)
{    
    BOOL returned = FALSE;
    if (NULL != pChannels) 
    {
        //avoid adding twice the same pointer
        BOOL found = FALSE;
        for (int i = 0; i < GetSize(); i++)
        {
            const CCountryChannels* channels = GetChannels(i);               
            if (channels == pChannels)
            {
                found = TRUE;
                break;
            }
        }//for        
        if (FALSE == found)
        {
            m_Countries.push_back(pChannels);
            returned = TRUE;
        }        
    }
    return returned;
}


BOOL CCountryList::RemoveChannels(int index)
{
    if ((index < 0) || (index >= GetSize())) 
    {
        return FALSE;
    }
    
    delete m_Countries[index];
    m_Countries.erase(m_Countries.begin() + index);
    return TRUE;
}


//That is an utility function that was in ProgramList.cpp
//Used in Load_Country_Settings
eVideoFormat StrToVideoFormat(char* pszFormat)
{
    for(int a = 0; a < VIDEOFORMAT_LASTONE; ++a)
    {
        if(!stricmp(pszFormat, VideoFormatNames[a]))
        {
            return (eVideoFormat)a;
        }  
    }
   
    return VIDEOFORMAT_LASTONE;
}

BOOL CCountryList::ReadASCII(LPCSTR szFilename) 
{
    if (NULL == szFilename) {
        return FALSE;
    }

    FILE* file = fopen(szFilename, "r");
    if (NULL == file) {
        return FALSE;
    }

    BOOL returned = ReadASCII(file);
    returned = (fclose(file) == 0) && returned;
    
    return returned;
}

BOOL CCountryList::ReadASCII(FILE* CountryFile)
{
    ASSERT(NULL != CountryFile);
    
    char      line[128];
    char*     Pos;
    char*     Pos1;
    char*     eol_ptr;
    CString   channelName;
    CCountryChannels* NewCountry = NULL;
    eVideoFormat Format = VIDEOFORMAT_LASTONE;     
    int channelCounter = 0;
   
    while (fgets(line, sizeof(line), CountryFile) != NULL)
    {
        eol_ptr = strstr(line, ";");
        if (eol_ptr == NULL)
        {
            eol_ptr = strstr(line, "\n");
        }
        if(eol_ptr != NULL)
        {
            channelName.ReleaseBuffer();
            channelName = eol_ptr;
            //_snprintf(Name, 254,"%s", eol_ptr);
            //Name[255] = '\0';
            *eol_ptr = '\0';            
        }
        if(eol_ptr == line)
        {
            continue;
        }
        if(((Pos = strchr(line, '[')) != 0) && ((Pos1 = strrchr(line, ']')) != 0) && Pos1 > Pos)
        {
            if(NewCountry != NULL)
            {
                AddChannels(NewCountry);
            }
            Pos++;
 
            char * dummy = Pos;
            dummy[Pos1-Pos] = '\0';                          
            channelCounter = 0;
            NewCountry = new CCountryChannels((LPCSTR)dummy);
            Format = VIDEOFORMAT_LASTONE;
        }
        else
        {
            if (NewCountry == NULL)
            {                                
                return FALSE;
            }
            
            if ((Pos = strstr(line, "ChannelLow=")) != 0)
            {                
                NewCountry->SetMinChannelNumber(atoi(Pos + strlen("ChannelLow=")));                
            }
            else if ((Pos = strstr(line, "ChannelHigh=")) != 0)
            {
                NewCountry->SetMaxChannelNumber(atoi(Pos + strlen("ChannelHigh=")));
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
                        //increment channels for 0 freqs, so that you see gaps...
                        //but do not add the channel (that is the legacy behaviour)
                        int channelNumber = NewCountry->GetMinChannelNumber() + channelCounter;
                        DWORD freq = (DWORD)atol(Pos);
                        if (freq > 0)
                        {                            
                            channelName.TrimLeft();
                            channelName.TrimRight();
                            //eliminate the trailing ";" in name if it exists,
                            //otherwise pickup a default name
                            if (channelName.GetLength() < 2) {
                                NewCountry->AddChannel(freq, channelNumber, Format, FALSE);                            
                            }
                            else {                                
                                NewCountry->AddChannel(channelName.Right(channelName.GetLength() - 2), freq, channelNumber, Format, FALSE);                            
                            }
                        }
                        channelCounter++;
                        break;
                    }
                    Pos++;
                }
            }
        }
    }
    if(NewCountry != NULL)
    {
        AddChannels(NewCountry);
    }
    
    return TRUE;
}

BOOL CCountryList::WriteXML(LPCSTR szFilename)  const
{
    if (NULL == szFilename) {
        return FALSE;
    }

    FILE* file = fopen(szFilename, "w");
    if (NULL == file) {
        return FALSE;
    }

    BOOL returned = WriteXML(file);
    returned = (fclose(file) == 0) && returned;
    
    return returned;
}


BOOL CCountryList::WriteXML(FILE* xmlCountryFile)  const 
{
    ASSERT(NULL != xmlCountryFile);
    
    BOOL bSuccess = FALSE;
    
    fprintf(xmlCountryFile, "<channel-list>\n");
    for (int j = 0; j < GetSize(); j++) {
        const CCountryChannels* channels = GetChannels(j);
        channels->WriteXMLImpl(xmlCountryFile);        
    }//for each channel list
    fprintf(xmlCountryFile, "</channel-list>\n");
    
    bSuccess = TRUE;
    
    return bSuccess;
}


// ------------ CCountryList END --------------
// ------------------------------------------------


