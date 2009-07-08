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
#include "PathHelpers.h"

using namespace std;

CChannel::CChannel(LPCTSTR Name, LPCTSTR EPGName, DWORD Freq, int ChannelNumber, eVideoFormat Format, BOOL Active)
{
    m_Name = Name;
    m_EPGName = EPGName;
    m_Freq = Freq;
    m_Chan = ChannelNumber;
    m_Format = Format;
    m_Active = Active;
}

CChannel::CChannel(LPCTSTR Name, DWORD Freq, int ChannelNumber, eVideoFormat Format, BOOL Active)
{
    m_Name = Name;
    m_EPGName = m_Name;
    m_Freq = Freq;
    m_Chan = ChannelNumber;
    m_Format = Format;
    m_Active = Active;
}

LPCTSTR CChannel::GetName() const
{
    return m_Name.c_str();
}

LPCTSTR CChannel::GetEPGName() const
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

const CChannel* CChannelList::GetChannel(int index)  const
{
    return m_Channels[index];
}


const CChannel* CChannelList::GetChannelByNumber(int iChannelNumber)
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

const CChannel* CChannelList::GetChannelByFrequency(DWORD dwFreq)
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
    if (pChannel.IsValid())
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

    SmartPtr<CChannel> channelA(m_Channels[a]);
    SmartPtr<CChannel> channelB(m_Channels[b]);

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

BOOL CUserChannels::WriteFile() const
{
    tstring FileName(GetFileName());
    BOOL success = FALSE;
    FILE* file = _tfopen(FileName.c_str(), _T("w"));
    if (NULL != file)
    {
        success = WriteASCIIImpl(file);
        success = (fclose(file) == 0) && success;
        file = NULL;
    }

    return success;
}


BOOL CUserChannels::ReadFile()
{
    tstring FileName(GetFileName());
    BOOL success = FALSE;
    FILE* file = _tfopen(FileName.c_str(), _T("r"));
    if (NULL != file) 
    {
        success = ReadASCIIImpl(file);
        success = (fclose(file) == 0) && success;
        file = NULL;
    }
    return success;
}

tstring CUserChannels::GetFileName() const
{
    return GetUserFilePath() + _T("program.txt");
}

BOOL CUserChannels::ReadASCIIImpl(FILE* SettingFile)
{
    _ASSERTE(NULL != SettingFile);

    TCHAR sbuf[256];
    DWORD Frequency = -1;
    int Channel = 1;
    int Format = -1;
    BOOL Active = TRUE;
    tstring Name;
    tstring EPGName;

    while(!feof(SettingFile))
    {
        sbuf[0] = '\0';

        _fgetts(sbuf, 255, SettingFile);

        TCHAR* eol_ptr = _tcsstr(sbuf, _T(";"));
        if (eol_ptr == NULL)
        {
            eol_ptr = _tcsstr(sbuf, _T("\n"));
        }
        if (eol_ptr != NULL)
        {
            *eol_ptr = '\0';
        }


        if(_tcsnicmp(sbuf, _T("Name:"), 5) == 0)
        {
            if(Frequency != -1)
            {
                AddChannel(new CChannel(Name.c_str(), EPGName.c_str(), Frequency, Channel, (eVideoFormat)Format, Active));
            }

            // skip _T("Name:")
            TCHAR* StartChar = sbuf + 5;

            // skip any spaces
            while(iswspace(*StartChar))
            {
                ++StartChar;
            }
            if(_tcslen(StartChar) > 0)
            {
                TCHAR* EndChar = StartChar + _tcslen(StartChar) - 1;
                while(EndChar > StartChar && iswspace(*EndChar))
                {
                    *EndChar = '\0';
                    --EndChar;
                }
                Name = StartChar;
            }
            else
            {
                Name = _T("Empty");
            }
            EPGName = Name;
            Frequency = -1;
            ++Channel;
            Format = -1;
            Active = TRUE;
        }
        else if(_tcsnicmp(sbuf, _T("EPGName:"), 8) == 0)
        {
            // skip _T("Name:")
            TCHAR* StartChar = sbuf + 8;

            // skip any spaces
            while(iswspace(*StartChar))
            {
                ++StartChar;
            }
            if(_tcslen(StartChar) > 0)
            {
                TCHAR* EndChar = StartChar + _tcslen(StartChar) - 1;
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
        else if(_tcsnicmp(sbuf, _T("Freq:"), 5) == 0)
        {
            Frequency = FromString<DWORD>(sbuf + 5);
            Frequency = Frequency * 1000;
        }
        else if(_tcsnicmp(sbuf, _T("Freq2:"), 6) == 0)
        {
            Frequency = FromString<DWORD>(sbuf + 6);
            Frequency = MulDiv(Frequency, 1000000, 16);
        }
        else if(_tcsnicmp(sbuf, _T("Chan:"), 5) == 0)
        {
            Channel = FromString<int>(sbuf + 5);
        }
        else if(_tcsnicmp(sbuf, _T("Form:"), 5) == 0)
        {
            Format = FromString<int>(sbuf + 5);
        }
        else if(_tcsnicmp(sbuf, _T("Active:"), 7) == 0)
        {
            Active = (FromString<int>(sbuf + 7) != 0);
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
        _ftprintf(SettingFile, _T("Name: %s\n"), GetChannel(i)->GetName());
        if (_tcscmp(GetChannel(i)->GetName(), GetChannel(i)->GetEPGName()))
            _ftprintf(SettingFile, _T("EPGName: %s\n"), GetChannel(i)->GetEPGName());
        //_ftprintf(SettingFile, _T("Freq2: %ld\n"), MulDiv((*it)->GetFrequency(),16,1000000));
        _ftprintf(SettingFile, _T("Freq: %ld\n"), GetChannel(i)->GetFrequency()/1000);
        _ftprintf(SettingFile, _T("Chan: %d\n"), GetChannel(i)->GetChannelNumber());
        _ftprintf(SettingFile, _T("Active: %d\n"), GetChannel(i)->IsActive());
        if(GetChannel(i)->GetFormat() != -1)
        {
            _ftprintf(SettingFile, _T("Form: %d\n"), GetChannel(i)->GetFormat());
        }
    }

    bSuccess = TRUE;

    return bSuccess;
}

// ------------ CUserChannels END --------------
// --------------------------------------------

// ------------ CCountryChannels BEGIN ------------
// --------------------------------------------


CCountryChannels::CCountryChannels(LPCTSTR szSomeIdentifierString, eVideoFormat eCountryVideoFormat) : CChannelList()
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

const LPCTSTR CCountryChannels::GetCountryName() const
{
    static TCHAR sbuf[256];
    _tcsncpy(sbuf, m_szName.c_str(), 255);
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
}

int CCountryList::GetSize() const
{
    return m_Countries.size();
}

void CCountryList::Clear()
{
    m_Countries.clear();
}


const CCountryChannels* CCountryList::GetChannels(int index)  const
{
    _ASSERTE(index >= 0);
    _ASSERTE(index < m_Countries.size());
    return m_Countries[index].GetRawPointer();
}

//That is an utility function that was in ProgramList.cpp
//Used in Load_Country_Settings
eVideoFormat StrToVideoFormat(const tstring& Format)
{
    for(int a = 0; a < VIDEOFORMAT_LAST_TV; ++a)
    {
        if(AreEqualInsensitive(Format, MBCSToTString(VideoFormatNames[a])))
        {
            return (eVideoFormat)a;
        }
    }

    return VIDEOFORMAT_LAST_TV;
}

BOOL CCountryList::ReadASCII(LPCTSTR szFilename)
{
    if (NULL == szFilename) {
        return FALSE;
    }

    FILE* file = _tfopen(szFilename, _T("r"));
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

    TCHAR      line[128];
    TCHAR*     Pos;
    TCHAR*     Pos1;
    TCHAR*     eol_ptr;
    tstring    channelName;
    SmartPtr<CCountryChannels> NewCountry;
    eVideoFormat Format = VIDEOFORMAT_LAST_TV;
    int channelCounter = 0;

    while (_fgetts(line, sizeof(line), CountryFile) != NULL)
    {
        eol_ptr = _tcsstr(line, _T(";"));
        if (eol_ptr == NULL)
        {
            eol_ptr = _tcsstr(line, _T("\n"));
        }
        if(eol_ptr != NULL)
        {
            channelName = _T("");
            channelName = eol_ptr;
            *eol_ptr = '\0';
        }
        if(eol_ptr == line)
        {
            continue;
        }
        if(((Pos = _tcschr(line, '[')) != 0) && ((Pos1 = _tcsrchr(line, ']')) != 0) && Pos1 > Pos)
        {
            if(NewCountry.IsValid())
            {
                m_Countries.push_back(NewCountry);
            }
            Pos++;

            TCHAR*  dummy = Pos;
            dummy[Pos1-Pos] = '\0';
            channelCounter = 0;
            NewCountry = new CCountryChannels((LPCTSTR)dummy);
            Format = VIDEOFORMAT_LAST_TV;
        }
        else
        {
            if (!NewCountry)
            {
                return FALSE;
            }

            if ((Pos = _tcsstr(line, _T("ChannelLow="))) != 0)
            {
                NewCountry->SetMinChannelNumber(_ttoi(Pos + _tcslen(_T("ChannelLow="))));
            }
            else if ((Pos = _tcsstr(line, _T("ChannelHigh="))) != 0)
            {
                NewCountry->SetMaxChannelNumber(_ttoi(Pos + _tcslen(_T("ChannelHigh="))));
            }
            else if ((Pos = _tcsstr(line, _T("Format="))) != 0)
            {
                Format = StrToVideoFormat(Pos + _tcslen(_T("Format=")));
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
                        DWORD freq = (DWORD)_ttol(Pos);
                        if (freq > 0)
                        {
                            Trim(channelName);
                            //eliminate the trailing _T(";") in name if it exists,
                            //otherwise pickup a default name
                            SmartPtr<CChannel> NewChannel;
                            if (channelName.length() < 2)
                            {
                                channelName = MakeString() << _T("; Channel ") << channelNumber;
                            }
                            NewChannel = new CChannel(channelName.c_str() + 2, freq, channelNumber, Format, FALSE);
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
    if(NewCountry.IsValid())
    {
        m_Countries.push_back(NewCountry);
    }

    return TRUE;
}

// ------------ CCountryList END --------------
// ------------------------------------------------
