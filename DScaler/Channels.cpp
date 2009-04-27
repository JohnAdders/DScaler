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
 * @file Channels.cpp Channels Classes
 */

#include "stdafx.h"
#include "DebugLog.h"
#include "TVFormats.h"
#include "Channels.h"

using namespace std;

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

LPCSTR CChannel::GetName() const
{
    return m_Name.c_str();
}

LPCSTR CChannel::GetEPGName() const
{
    return m_EPGName.c_str();
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

CChannelList::CChannelList() : 
    m_MinFrequency(0),
    m_MaxFrequency(0),
    m_MinChannelNumber(0),
    m_MaxChannelNumber(0)
{

}

void CChannelList::Clear()
{
    m_Channels.clear();
    UpdateFields();
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

SmartPtr<CChannel> CChannelList::GetChannel(int index)  const
{
    return m_Channels[index];
}


SmartPtr<CChannel> CChannelList::GetChannelByNumber(int iChannelNumber)
{
    SmartPtr<CChannel> returned;

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

SmartPtr<CChannel> CChannelList::GetChannelByFrequency(DWORD dwFreq)
{
    SmartPtr<CChannel> returned;

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

void CChannelList::AddChannel(SmartPtr<CChannel> pChannel)
{
    if (pChannel)
    {
        m_Channels.push_back(pChannel);
        UpdateFields();
    }
}

void CChannelList::RemoveChannel(int index)
{
    if ((index < 0) || (index >= GetSize()))
    {
        return;
    }
    m_Channels.erase(m_Channels.begin() + index);
    UpdateFields();
}


void CChannelList::SetChannel(int index, SmartPtr<CChannel>pChannel)
{
    if ((index >= 0) && !pChannel)
    {
        if (index >= GetSize())
        {
            AddChannel(pChannel);
        }
        else
        {
            m_Channels[index] = pChannel;
            UpdateFields();
        }
    }
}


void CChannelList::SwapChannels(int a, int b)
{
    if ((a < 0) || (a >= GetSize()) || (b < 0) || (b >= GetSize()))
    {
        return;
    }
    if (a == b)
    {
        //now, that is very funny...
        return;
    }

    SmartPtr<CChannel> channelA = GetChannel(a);
    SmartPtr<CChannel> channelB = GetChannel(b);

    m_Channels[a] = channelB;
    m_Channels[b] = channelA;
}


void CChannelList::UpdateFields()
{
    if (m_MinFrequency == 0)
    {
        m_MinFrequency = (GetSize() == 0) ? 0 : GetChannel(0)->GetFrequency();
        m_MaxFrequency = m_MinFrequency;
    }

    if (m_MinChannelNumber == 0)
    {
        m_MinChannelNumber = (GetSize() == 0) ? 0 : GetChannel(0)->GetChannelNumber();
        m_MaxChannelNumber = m_MinChannelNumber;
    }

    for(int i(1); i < GetSize(); ++i)
    {
        m_MinFrequency = min(m_MinFrequency, GetChannel(i)->GetFrequency());
        m_MaxFrequency = max(m_MaxFrequency, GetChannel(i)->GetFrequency());

        m_MinChannelNumber = min(m_MinChannelNumber, GetChannel(i)->GetChannelNumber());
        m_MaxChannelNumber = max(m_MaxChannelNumber, GetChannel(i)->GetChannelNumber());
    }
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

BOOL CUserChannels::WriteFile(LPCSTR szFilename) const
{
    if (NULL == szFilename)
    {
        return FALSE;
    }

    BOOL success = FALSE;
    FILE* file = fopen(szFilename, "w");
    if (NULL != file) {
        success = WriteASCIIImpl(file);
        success = (fclose(file) == 0) && success;
        file = NULL;
    }

    return success;
}


BOOL CUserChannels::ReadFile(LPCSTR szFilename)
{
    if (NULL == szFilename)
    {
        return FALSE;
    }

    BOOL success = FALSE;
    FILE* file = fopen(szFilename, "r");
    if (NULL != file) {
        success = ReadASCIIImpl(file);
        success = (fclose(file) == 0) && success;
        file = NULL;
    }
    return success;
}

BOOL CUserChannels::ReadASCIIImpl(FILE* SettingFile)
{
    _ASSERTE(NULL != SettingFile);

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


        if(_strnicmp(sbuf, "Name:", 5) == 0)
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
        else if(_strnicmp(sbuf, "EPGName:", 8) == 0)
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
        else if(_strnicmp(sbuf, "Freq:", 5) == 0)
        {
            Frequency = atol(sbuf + 5);
            Frequency = Frequency * 1000;
        }
        else if(_strnicmp(sbuf, "Freq2:", 6) == 0)
        {
            Frequency = atol(sbuf + 6);
            Frequency = MulDiv(Frequency, 1000000, 16);
        }
        else if(_strnicmp(sbuf, "Chan:", 5) == 0)
        {
            Channel = atoi(sbuf + 5);
        }
        else if(_strnicmp(sbuf, "Form:", 5) == 0)
        {
            Format = atoi(sbuf + 5);
        }
        else if(_strnicmp(sbuf, "Active:", 7) == 0)
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
    _ASSERTE(NULL != SettingFile);

    BOOL bSuccess = FALSE;

    for(int i = 0; i < GetSize(); i++)
    {
        fprintf(SettingFile, "Name: %s\n", GetChannel(i)->GetName());
        if (strcmp(GetChannel(i)->GetName(), GetChannel(i)->GetEPGName()))
            fprintf(SettingFile, "EPGName: %s\n", GetChannel(i)->GetEPGName());
        //fprintf(SettingFile, "Freq2: %ld\n", MulDiv((*it)->GetFrequency(),16,1000000));
        fprintf(SettingFile, "Freq: %ld\n", GetChannel(i)->GetFrequency()/1000);
        fprintf(SettingFile, "Chan: %d\n", GetChannel(i)->GetChannelNumber());
        fprintf(SettingFile, "Active: %d\n", GetChannel(i)->IsActive());
        if(GetChannel(i)->GetFormat() != -1)
        {
            fprintf(SettingFile, "Form: %d\n", GetChannel(i)->GetFormat());
        }
    }

    bSuccess = TRUE;

    return bSuccess;
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
    _ASSERTE(index >= 0);
    _ASSERTE(index < m_Countries.size());
    return m_Countries[index];
}

//That is an utility function that was in ProgramList.cpp
//Used in Load_Country_Settings
eVideoFormat StrToVideoFormat(char* pszFormat)
{
    for(int a = 0; a < VIDEOFORMAT_LAST_TV; ++a)
    {
        if(!_stricmp(pszFormat, VideoFormatNames[a]))
        {
            return (eVideoFormat)a;
        }
    }

    return VIDEOFORMAT_LAST_TV;
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

    BOOL returned = ReadASCIIImpl(file);
    returned = (fclose(file) == 0) && returned;

    return returned;
}

BOOL CCountryList::ReadASCIIImpl(FILE* CountryFile)
{
    _ASSERTE(NULL != CountryFile);

    char      line[128];
    char*     Pos;
    char*     Pos1;
    char*     eol_ptr;
    string    channelName;
    SmartPtr<CCountryChannels> NewCountry;
    eVideoFormat Format = VIDEOFORMAT_LAST_TV;
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
            channelName.clear();
            channelName = eol_ptr;
            *eol_ptr = '\0';
        }
        if(eol_ptr == line)
        {
            continue;
        }
        if(((Pos = strchr(line, '[')) != 0) && ((Pos1 = strrchr(line, ']')) != 0) && Pos1 > Pos)
        {
            if(NewCountry)
            {
                m_Countries.push_back(NewCountry);
            }
            Pos++;

            char * dummy = Pos;
            dummy[Pos1-Pos] = '\0';
            channelCounter = 0;
            NewCountry = new CCountryChannels((LPCSTR)dummy);
            Format = VIDEOFORMAT_LAST_TV;
        }
        else
        {
            if (!NewCountry)
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
                            Trim(channelName);
                            //eliminate the trailing ";" in name if it exists,
                            //otherwise pickup a default name
                            SmartPtr<CChannel> NewChannel;
                            if (channelName.length() < 2)
                            {
                                channelName = MakeString() << "Channel " << channelNumber;
                                NewChannel = new CChannel(channelName.c_str(), freq, channelNumber, Format, FALSE);
                            }
                            else 
                            {
                                NewChannel = new CChannel(channelName.c_str(), freq, channelNumber, Format, FALSE);
                            }
                            NewCountry->AddChannel(NewChannel);
                        }
                        channelCounter++;
                        break;
                    }
                    Pos++;
                }
            }
        }
    }
    if(NewCountry)
    {
        m_Countries.push_back(NewCountry);
    }

    return TRUE;
}

// ------------ CCountryList END --------------
// ------------------------------------------------
