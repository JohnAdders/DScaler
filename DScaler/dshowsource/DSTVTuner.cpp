/////////////////////////////////////////////////////////////////////////////
// $Id: DSTVTuner.cpp,v 1.2 2002-08-15 14:19:02 kooiman Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Torbjörn Jansson.  All rights reserved.
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
// Change Log
//
// Date          Developer             Changes
//
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.1  2002/08/14 22:00:19  kooiman
// TV tuner class for DirectShow capture devices.
//
// Revision 1.0  2002/08/04 23:00:00  kooiman
// TV tuner class
//
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT

#include "DSTVTuner.h"
#include "DebugLog.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDShowTVTuner::CDShowTVTuner(IGraphBuilder *pGraph)
:CDShowObject(pGraph),
m_CountryCode(-1),
m_MinChannel(-1),
m_MaxChannel(-1),
m_InputPin(0),
m_TunerInput(-1),
m_TVTuner(NULL),
m_CustomFrequencyTable(-2)
{  
    //       
}

CDShowTVTuner::CDShowTVTuner(CComPtr<IAMTVTuner> &pTvTuner,IGraphBuilder *pGraph)
:CDShowObject(pGraph),
m_CountryCode(-1),
m_MinChannel(-1),
m_MaxChannel(-1),
m_TunerInput(-1),
m_InputPin(0),
m_CustomFrequencyTable(-2)
{
    m_TVTuner = pTvTuner;

    if (m_TVTuner != NULL)
    {
        m_TVTuner->put_Mode(AMTUNER_MODE_TV);
    }
}

CDShowTVTuner::~CDShowTVTuner()
{
    if (m_TVTuner != NULL)
    {
        // Delete tuningspace
        if (m_CustomFrequencyTable >= 0)
        {
            char szKeyName[200];
            sprintf(szKeyName,"Software\\Microsoft\\TV System Services\\TVAutoTune\\TS%d-1",m_CustomFrequencyTable);
            RegDeleteKey(HKEY_LOCAL_MACHINE, szKeyName);                
        }
    }
}


long CDShowTVTuner::getCountryCode()
{
    if (m_TVTuner == NULL)
    {
        return -1;
    }
    long lCurrentCountryCode = -1;
    if (m_CountryCode < 0)
    {
        // First time: no country code set yet
        return -1;
    }
    m_TVTuner->get_CountryCode(&lCurrentCountryCode);
    return lCurrentCountryCode;
}

void CDShowTVTuner::putCountryCode(long CountryCode)
{
    if (m_TVTuner == NULL)
    {
        return;
    }    
    m_CountryCode = CountryCode;
    m_TVTuner->put_CountryCode(m_CountryCode);

    m_TVTuner->ChannelMinMax(&m_MinChannel, &m_MaxChannel);
    m_FrequencyTable.clear();    
}

TunerInputType CDShowTVTuner::getInputType()
{
    if (m_TVTuner == NULL)
    {
        return TunerInputCable;
    }

    TunerInputType InputType;
    m_TVTuner->get_InputType(m_InputPin, &InputType);

    if (m_TunerInput < 0)
    {
        // First time: no input type set
        return (TunerInputType)-1;
    }

    return InputType;      

}


BOOL CDShowTVTuner::setInputPin(long lInputPin)
{
    m_InputPin = lInputPin;
    if (m_TVTuner->put_ConnectInput(m_InputPin) == NO_ERROR)
    {
        return TRUE;
    }
    return FALSE;
}

void CDShowTVTuner::setInputType(TunerInputType NewType)
{
    if (m_TVTuner == NULL)
    {
        return;
    }
    m_TVTuner->put_InputType(m_InputPin, NewType);
    m_TVTuner->put_ConnectInput(m_InputPin);
    m_TunerInput=(int)NewType;
    m_FrequencyTable.clear();
}

long CDShowTVTuner::getChannel()
{
    if (m_TVTuner == NULL)
    {
        return -1;
    }
    long lCurrentChannel = -1;
    long lVideoSubChannel;
    long lAudioSubChannel;
    
    if (m_TVTuner->get_Channel(&lCurrentChannel,&lVideoSubChannel, &lAudioSubChannel) != NOERROR)
    {
        return -1;
    }
    return lCurrentChannel;
}

BOOL CDShowTVTuner::setChannel(long lChannel)
{
    if (m_TVTuner == NULL)
    {
        return FALSE;
    }
    
    if ( (lChannel < m_MinChannel) || (lChannel > m_MaxChannel) )
    {
       return FALSE;
    }

    // Set channel
    long lVideoSubChannel;
    long lAudioSubChannel;

    lVideoSubChannel = AMTUNER_SUBCHAN_DEFAULT;
    lAudioSubChannel = AMTUNER_SUBCHAN_DEFAULT;

    LOG(2,"DShowTVTuner: Set to channel %d",lChannel);
    if (m_TVTuner->put_Channel(lChannel, lVideoSubChannel, lAudioSubChannel) == NOERROR)
    {
       return TRUE;
    }
    return FALSE;
}

BOOL CDShowTVTuner::setTunerFrequency(long dwFrequency)
{
    if (m_TVTuner == NULL)
    {
       return FALSE;
    }

    // Fixed frequency tables
    if (m_CustomFrequencyTable == -1)
    {
        if (m_FrequencyTable.size() == 0)
        {
            getCountryCode();
            LoadFrequencyTable(getCountryCode(), TunerInputCable);
        }
        
        //Find Closest channel (also called frequency index by MS) method
        int Channel = FrequencyToChannel(dwFrequency);
        LOG(2,"DShowTVTuner: setTunerFrequency (%d) -> channel %d",dwFrequency,Channel);
        if (Channel < 0)
        {
            return FALSE;
        }
        return setChannel(Channel);
    }
    else
    {
        // Registry method
    
        dwFrequency = MulDiv(dwFrequency, 1000000, 16);

        // Use registry custom channel table as a frequency cache
    
        static long *AutoTuneInfo = NULL;
        static long *AutoTuneTag = NULL;
        static long AutoTuneTagCounter = 0;
    
        if (AutoTuneInfo == NULL)
        {
            long Length = m_MaxChannel - m_MinChannel + 1;
            AutoTuneInfo = new long[Length];
            AutoTuneTag  = new long[Length];
            int i;
            for (i = 0; i < Length; i++)
            {
                AutoTuneInfo[i] = 0;
                AutoTuneTag[i] = 0;
            }
            AutoTuneTagCounter = 0;
        }
    
        // Find frequency index
        int  Index = 0;
        int  OldestIndex = 0;
        long OldestTag = 0x7ffffffL;
        int  NumChannels = m_MaxChannel - m_MinChannel + 1;
        while ( (Index < NumChannels) && (AutoTuneInfo[Index] != 0) )
        {
            if (AutoTuneInfo[Index] == dwFrequency)
            {           
                AutoTuneTag[Index] = AutoTuneTagCounter++;
                LOG(2,"DShowTVTuner: Set channel to index %d (cached)", Index+m_MinChannel);
                if ( getChannel() == (Index+m_MinChannel) )
                {
                    return TRUE;
                }
                return setChannel( Index + m_MinChannel ); 
            }
            if (AutoTuneTag[Index] < OldestTag)
            {
                OldestTag = AutoTuneTag[Index];
                OldestIndex = Index;
            }
            Index++;
        }
        if (Index == NumChannels)
        {
            //Table is full, use the oldest frequency index
            Index = OldestIndex;
        }
    
        // Store new frequency
        LOG(2,"DShowTVTuner: Try open registrykey for custom frequency");

        HKEY RegKey = NULL;
    
        if (RegKey == NULL)
        {
            DWORD dwDisposition;
            REGSAM regsam = KEY_WRITE | KEY_READ | DELETE;
            SECURITY_ATTRIBUTES secatt = {sizeof(SECURITY_ATTRIBUTES),NULL,FALSE};
            char szKeyName[100];
            int TunerSpace = m_CustomFrequencyTable;
            int Errors = 0;
            if (TunerSpace < 0) 
            {
                TunerSpace = 0;
            }

            do 
            {                
                sprintf(szKeyName,"Software\\Microsoft\\TV System Services\\TVAutoTune\\TS%d-1",TunerSpace);
                if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,szKeyName, 0,NULL,REG_OPTION_VOLATILE,regsam,&secatt,&RegKey,&dwDisposition) != ERROR_SUCCESS)
                {                    
                    if (m_CustomFrequencyTable == -2)
                    {
                        Errors++; 
                        LOG(2,"DShowTVTuner: Error opening %s ()",szKeyName);
                    }
                    else
                    {
                        LOG(2,"DShowTVTuner: Error opening %s",szKeyName);
                        // Failed
                        return FALSE;
                    }
                }
                else
                {                   
                   if (dwDisposition == REG_CREATED_NEW_KEY)
                   {
                      // Found free tunerspace
                      m_CustomFrequencyTable = TunerSpace;
                      LOG(2,"DShowTVTuner: Found free tunerspace %d",TunerSpace);
                   }
                }
                TunerSpace++;
            } while ( (m_CustomFrequencyTable == -2) && (Errors <= 2) );

            if (m_CustomFrequencyTable < 0)
            {
                LOG(2,"DShowTVTuner: Error finding tunerspace. Change to fixed freq tables");
                // Use existing fixed frequency tables
                m_CustomFrequencyTable = -1;
                return setTunerFrequency(MulDiv(dwFrequency, 16, 1000000));
            }
        
            // Set new values
            LOG(2,"DShowTVTuner: Set registrykey custom frequency (%d Hz)",dwFrequency);
            char szChannel[10];
            itoa(Index+m_MinChannel,szChannel,10);
            if (RegSetValueEx(RegKey, szChannel, 0, REG_DWORD, (BYTE*)&dwFrequency, sizeof(DWORD)) == ERROR_SUCCESS)
            {
                AutoTuneInfo[Index] = dwFrequency;
                AutoTuneTag[Index] = AutoTuneTagCounter++;
                LOG(2,"DShowTVTuner: Cache tag value %ld",AutoTuneTagCounter);

                if (RegSetValueEx(RegKey, "AutoTune", 0, REG_BINARY, (BYTE*)AutoTuneInfo, (m_MaxChannel - m_MinChannel + 1)*sizeof(long)) == ERROR_SUCCESS)
                {
                    //Succeeded
                }
                RegCloseKey(RegKey);
                m_TVTuner->put_TuningSpace(m_CustomFrequencyTable);
                LOG(2,"DShowTVTuner: Set channel to index %d", Index+m_MinChannel);
                return setChannel(Index + m_MinChannel);
            }
            else
            {   
                RegCloseKey(RegKey);
            }

           return FALSE;
        }
    }
    return FALSE;
}

long CDShowTVTuner::getTunerFrequency()
{
    int Channel = getChannel();
    if ((Channel >= m_MinChannel) && (Channel<= m_MaxChannel))
    {
        return m_FrequencyTable[Channel - m_MinChannel];
    }
    return 0;
}


static void *GetRcData(LPCTSTR lpFileName, LPCTSTR lpName, LPCTSTR lpType, HMODULE &hDSTunerModule)
{
   if (hDSTunerModule == NULL)
   {
      hDSTunerModule = LoadLibrary(lpFileName);
   }
   
   HRSRC hDSTunerResource = FindResource(hDSTunerModule, lpName, lpType);
   if (hDSTunerResource == NULL)
   {
      return NULL;
   }

   HGLOBAL hDSTunerTable = LoadResource(hDSTunerModule, hDSTunerResource);
   if (hDSTunerTable == NULL)
   {
       return NULL;
   }
   
   return LockResource(hDSTunerTable);      
}

BOOL CDShowTVTuner::LoadFrequencyTable(int CountryCode, TunerInputType InputType)
{
      typedef struct 
      {
          WORD CountryCode;
          WORD CableFreqTable;
          WORD BroadcastFreqTable;
          WORD VideoStandard;    
          WORD UnKnown;
      } TRCCountryList;

      long *FreqTable = NULL;
      HMODULE hDSTunerModule = NULL;
      char szRCDATA[10];
           
      
      sprintf(szRCDATA,"#%d",RT_RCDATA);
      TRCCountryList *RcCountryList = (TRCCountryList*)GetRcData("kstvtune.ax","#9999",szRCDATA, hDSTunerModule);

      LOG(2,"DShowTVTuner: Load country table %d (%s) (%s)",CountryCode, ((InputType==TunerInputCable)?"Cable":"Broadcast"),((RcCountryList==NULL)?"Failed":"Ok"));
      
      int Index = 0;
      while ((RcCountryList != NULL) && (RcCountryList[Index].CountryCode != 0))      
      {                    
          if (RcCountryList[Index].CountryCode == CountryCode)
          {
              char lName[20];
              if (InputType == TunerInputCable)
              {
                  sprintf(lName,"#%d",RcCountryList[Index].CableFreqTable);
              }
              else
              {
                  sprintf(lName,"#%d",RcCountryList[Index].BroadcastFreqTable);
              }
              FreqTable = (long*)GetRcData("kstvtune.ax", lName, szRCDATA, hDSTunerModule);
              LOG(2,"DShowTVTuner: Load freq table (%s)",(FreqTable==NULL)?"Failed":"Ok");
              break;
          }
          Index++;
      }
      if (hDSTunerModule != NULL)
      {
          FreeLibrary(hDSTunerModule);
      }

      if (FreqTable == NULL)
      {
          // Failed. Use default table
          m_FrequencyTable.clear();
          int i = 0;
          while (DSUniFreqTable[2 + i] != 0)
          {
              m_FrequencyTable.push_back(DSUniFreqTable[i+2]);
              i++;
          }                            
          return FALSE;
      }        
              
      m_FrequencyTable.clear();
          
      //m_MinChannel =  FreqTable[0];
      //m_MaxChannel =  FreqTable[1];

      LOG(2,"DShowTVTuner: Freq table [%i .. %i]",m_MinChannel,m_MaxChannel);

      int tabIndex = 2;
      int ch;
      for (ch = m_MinChannel; ch <= m_MaxChannel; ch++)
      {
         m_FrequencyTable.push_back( MulDiv(FreqTable[tabIndex++], 16, 1000000) );
      }
      
      return TRUE;
}   

// Look for channel in frequency table
int CDShowTVTuner::FrequencyToChannel(long dwFrequency)
{
    int i;
    int iClose = -1, iDistance;
    for (i = 0; i < m_FrequencyTable.size(); i++)
    {
        DWORD dwFreqFromTable = m_FrequencyTable[i];
        if (dwFreqFromTable == dwFrequency)
        {
            //LOG(2,"Frequency to channel: %d -> %d. exact match",dwFrequency,i+m_MinChannel);
            return i+m_MinChannel;
        }         
        if ((iClose < 0) || (abs(dwFreqFromTable - dwFrequency) < abs(iDistance)) )
        {
           // Closest match       
           iClose = i;
           iDistance = dwFreqFromTable - dwFrequency;
        }        
    }
    if ((iClose >= 0) && (abs(iDistance) <= 4*16))
    {
        //LOG(2,"Frequency to channel: %d -> %d. close match (%d off)",dwFrequency,i+m_MinChannel,iDistance);
        return iClose+m_MinChannel;
    }
    return -1;
}


const long CDShowTVTuner::DSUniFreqTable[] = 
{
      1L,
    368L,
    45250000L,
    45750000L,
    46250000L,
    48250000L,
    49750000L,
    53750000L,
    55250000L,
    57250000L,
    57750000L,
    59250000L,
    59750000L,
    61250000L,
    61750000L,
    62250000L,
    63250000L,
    64250000L,
    65250000L,
    65750000L,
    67250000L,
    69250000L,
    73250000L,
    76250000L,
    77250000L,
    82750000L,
    83250000L,
    85250000L,
    86250000L,
    90250000L,
    91250000L,
    95250000L,
    97250000L,
    102250000L,
    103250000L,
    105250000L,
    109250000L,
    112250000L,
    115250000L,
    119250000L,
    120250000L,
    121250000L,
    126250000L,
    127250000L,
    128250000L,
    133250000L,
    136250000L,
    138250000L,
    139250000L,
    140250000L,
    144250000L,
    145250000L,
    147250000L,
    151250000L,
    152250000L,
    154250000L,
    157250000L,
    160250000L,
    161250000L,
    163250000L,
    165250000L,
    168250000L,
    169250000L,
    171250000L,
    175250000L,
    176250000L,
    177250000L,
    181250000L,
    182250000L,
    183250000L,
    183750000L,
    184250000L,
    187250000L,
    189250000L,
    191250000L,
    192250000L,
    193250000L,
    196250000L,
    199250000L,
    200250000L,
    201250000L,
    203250000L,
    205250000L,
    207250000L,
    208250000L,
    209250000L,
    210250000L,
    211250000L,
    215250000L,
    216250000L,
    217250000L,
    223250000L,
    224250000L,
    225250000L,
    229250000L,
    231250000L,
    232250000L,
    235250000L,
    237250000L,
    238250000L,
    240250000L,
    241250000L,
    243250000L,
    245250000L,
    247250000L,
    248250000L,
    249250000L,
    252250000L,
    253250000L,
    256250000L,
    259250000L,
    264250000L,
    265250000L,
    266250000L,
    271250000L,
    272250000L,
    273250000L,
    277250000L,
    280250000L,
    283250000L,
    287250000L,
    288250000L,
    289250000L,
    294250000L,
    295250000L,
    301250000L,
    303250000L,
    307250000L,
    311250000L,
    313250000L,
    319250000L,
    325250000L,
    327250000L,
    331250000L,
    335250000L,
    337250000L,
    343250000L,
    349250000L,
    351250000L,
    355250000L,
    359250000L,
    361250000L,
    367250000L,
    373250000L,
    375250000L,
    379250000L,
    383250000L,
    385250000L,
    391250000L,
    397250000L,
    399250000L,
    403250000L,
    407250000L,
    409250000L,
    415250000L,
    421250000L,
    423250000L,
    427250000L,
    431250000L,
    433250000L,
    439250000L,
    445250000L,
    447250000L,
    451250000L,
    455250000L,
    457250000L,
    463250000L,
    469250000L,
    471250000L,
    475250000L,
    477250000L,
    479250000L,
    481250000L,
    481750000L,
    483250000L,
    487250000L,
    489250000L,
    492250000L,
    493250000L,
    495250000L,
    499250000L,
    501250000L,
    502750000L,
    503250000L,
    505250000L,
    507250000L,
    511250000L,
    513250000L,
    517250000L,
    519250000L,
    523250000L,
    525250000L,
    527250000L,
    529250000L,
    531250000L,
    534250000L,
    535250000L,
    537250000L,
    541250000L,
    543250000L,
    545750000L,
    547250000L,
    548250000L,
    549250000L,
    551250000L,
    553250000L,
    555250000L,
    556250000L,
    559250000L,
    561250000L,
    562250000L,
    565250000L,
    566750000L,
    567250000L,
    569250000L,
    571250000L,
    573250000L,
    575250000L,
    576250000L,
    577250000L,
    579250000L,
    583250000L,
    585250000L,
    589250000L,
    591250000L,
    595250000L,
    597250000L,
    599250000L,
    601250000L,
    603250000L,
    604250000L,
    607250000L,
    609250000L,
    611250000L,
    613250000L,
    615250000L,
    618250000L,
    619250000L,
    620250000L,
    621250000L,
    623250000L,
    625250000L,
    627250000L,
    631250000L,
    632250000L,
    633250000L,
    637250000L,
    639250000L,
    641250000L,
    643250000L,
    645250000L,
    646250000L,
    647250000L,
    649250000L,
    651250000L,
    651750000L,
    653250000L,
    655250000L,
    657250000L,
    660250000L,
    661250000L,
    662250000L,
    663250000L,
    667250000L,
    669250000L,
    671250000L,
    672750000L,
    673250000L,
    674250000L,
    675250000L,
    679250000L,
    681250000L,
    683250000L,
    685250000L,
    687250000L,
    688250000L,
    691250000L,
    693250000L,
    695250000L,
    697250000L,
    699250000L,
    702250000L,
    703250000L,
    705250000L,
    709250000L,
    711250000L,
    715250000L,
    715750000L,
    716250000L,
    717250000L,
    719250000L,
    721250000L,
    723250000L,
    726250000L,
    727250000L,
    729250000L,
    730250000L,
    733250000L,
    735250000L,
    736750000L,
    737250000L,
    739250000L,
    741250000L,
    743250000L,
    744250000L,
    745250000L,
    747250000L,
    751250000L,
    753250000L,
    757250000L,
    758250000L,
    759250000L,
    763250000L,
    765250000L,
    767250000L,
    769250000L,
    772250000L,
    775250000L,
    779250000L,
    781250000L,
    783250000L,
    786250000L,
    787250000L,
    790250000L,
    791250000L,
    793250000L,
    799250000L,
    800250000L,
    800750000L,
    805250000L,
    807250000L,
    811250000L,
    814250000L,
    815250000L,
    817250000L,
    823250000L,
    829250000L,
    831250000L,
    832250000L,
    835250000L,
    839250000L,
    841250000L,
    847250000L,
    853250000L,
    855250000L,
    859250000L,
    863250000L,
    865250000L,
    871250000L,
    877250000L,
    883250000L,
    889250000L,
    895250000L,
    901250000L,
    907250000L,
    913250000L,
    919250000L,
    925250000L,
    931250000L,
    937250000L,
    943250000L,
    949250000L,
    955250000L,
    961250000L,
    967250000L,
    973250000L,
    979250000L,
    985250000L,
    991250000L,
    997250000L,
    0L
};




#endif