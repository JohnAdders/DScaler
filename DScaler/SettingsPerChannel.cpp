/////////////////////////////////////////////////////////////////////////////
// $Id: SettingsPerChannel.cpp,v 1.23 2002-10-07 20:33:05 kooiman Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 DScaler team.  All rights reserved.
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
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.22  2002/10/02 10:52:35  kooiman
// Fixed C++ type casting for events.
//
// Revision 1.21  2002/09/29 13:56:30  adcockj
// Fixed some cursor hide problems
//
// Revision 1.20  2002/09/28 18:08:20  adcockj
// Fixed crashing due to bad cast
//
// Revision 1.19  2002/09/28 13:34:08  kooiman
// Added sender object to events and added setting flag to treesettingsgeneric.
//
// Revision 1.18  2002/09/26 11:33:42  kooiman
// Use event collector
//
// Revision 1.16  2002/09/25 15:11:12  adcockj
// Preliminary code for format specific support for settings per channel
//
// Revision 1.15  2002/09/15 15:00:05  kooiman
// Added check for Providers_GetCurrentSource() == NULL.
//
// Revision 1.14  2002/09/06 15:12:31  kooiman
// Get the correct video input in time.
//
// Revision 1.13  2002/09/02 19:07:21  kooiman
// Added BT848 advanced settings to advanced settings dialog
//
// Revision 1.12  2002/09/01 15:15:56  kooiman
// Fixed bug introduced by bug fix.
//
// Revision 1.11  2002/08/31 16:30:17  kooiman
// Fix duplicate names when changing sources.
//
// Revision 1.10  2002/08/27 22:02:32  kooiman
// Added Get/Set input for video and audio for all sources. Added source input change notification.
//
// Revision 1.9  2002/08/21 20:27:13  kooiman
// Improvements and cleanup of some settings per channel code.
//
// Revision 1.8  2002/08/15 14:16:45  kooiman
// More small improvements & bug fixes
//
// Revision 1.7  2002/08/14 22:04:25  kooiman
// Bug fix
//
// Revision 1.6  2002/08/13 21:21:24  kooiman
// Improved settings per channel to account for source and input changes.
//
// Revision 1.5  2002/08/08 21:17:05  kooiman
// Fixed some bugs.
//
// Revision 1.4  2002/08/08 20:00:00 kooiman
// Revised version of settings per channel
//
//
/////////////////////////////////////////////////////////////////////////////
 
#include "stdafx.h"
#include <stack>
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "setting.h"
#include "settings.h"
#include "providers.h"
#include "DebugLog.h"
#include "ProgramList.h"
#include "Source.h"
#include "SettingsPerChannel.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#define NO_CHANNEL -1
#define NO_VIDEOINPUT -1
#define NO_VIDEOFORMAT -1

// Container for channel specific settings.
typedef struct 
{
        int  Channel;                   // Channel number
        int  Flag;                      // Channel Flag
                                        // bit 0: Changed
                                        // bit 1: Channel value read 
                                        // bit 2: Channel value was in .ini file
        long Value;                     // Value of setting
        long LastValue;                 // Last read/written value of setting
} TChannelSpecificSetting;

// Source & input info
typedef struct 
{
        CSource*     pSource;            //Pointer to source class
        std::string  sSourceID;          //Source ID     
        int          VideoInput;         //Video input
        eVideoFormat VideoFormat;
        int          HasTuner;           //Has tuner
        int          LastChannel;
} TSpcSourceInputInfo;


// Channel settings per source&input
typedef struct 
{    
    TSpcSourceInputInfo* pInfo;         //Link to source/input info
    // Channel specific settings. Size 1 if pInfo->HasTuner if is 0.
    std::vector<TChannelSpecificSetting> vChannels;
} TSourceInputChannelSpecificSetting;

/** 
      Channel settings.

*/
class TChannelSetting
{
private:                
        CSimpleSetting* CSSetting;        //Track 2 types of settings
        SETTING*        Setting;          //
        
        long**          pToggleValue;     //Direct link to toggle enabled value
public:        
        // Channel, source & input specific settings
        std::vector<TSourceInputChannelSpecificSetting> vChannelSpecificSettingsList;
        
        BOOL            bInDefaultState;   // Settings are in default state
        long            DefaultStateValue; // Storage for value before going into default state
                
        long            DefaultValue;      // Default value for setting        
        std::string     sSubSection;       // Subsection of setting
              
        long            LastValue;         // Working variable
        int             Flag;              // Working variable 
                                           // Both retreived & stored from channel specific settings        

        SETTING*        ToggleSetting;     // Enable/disable setting for GUI & SavePerChannel ini section
        int             ToggleSettingIsLink;   // Is link to another GUI setting (for grouping)
private:
        BOOL bEnabled;                    // Storage for ToggleSetting
        std::string sDisplayName;         // ..
        std::string sIniSection;          // ..
        std::string sIniEntry;            // ..        
public:
        // Initialize channel setting with new setting .
        TChannelSetting(long** pTVal = NULL);
        TChannelSetting(CSimpleSetting* cs1, long** pTVal = NULL);
        TChannelSetting(SETTING* s1, long** pTVal = NULL);
        ~TChannelSetting();
  
        // Get & set value for setting
        long GetValue();
        void SetValue(long NewValue, BOOL bNoOnChange = FALSE);
        long GetDefault();

        char* IniEntry();
        
        // Load & save channel specific settings
        void LoadChannelSpecifics(std::string SourceID, int VideoInput, int Channel, BOOL bNoOnChange = FALSE);
        void SaveChannelSpecifics(std::string SourceID, int VideoInput, int Channel, eVideoFormat VideoFormat);

        // Go to default state
        void DefaultState(BOOL bOnOff);

        // Read & write from .ini file
        BOOL ReadFromIni(std::string SourceID, int VideoInput, int Channel, eVideoFormat VideoFormat, const char* szSection = NULL);
        void WriteToIni(std::string SourceID, int VideoInput, int Channel, eVideoFormat VideoFormat, BOOL bOptimizeFileAccess);

        // Monitoring enabled
        BOOL Enabled() { if (ToggleSetting == NULL) return FALSE; return *(ToggleSetting->pValue); };
        // Has valid setting
        BOOL Valid() { return ((Setting!=NULL) || (CSSetting!=NULL)); };

        // Start/stop monitoring
        void Start();
        void Stop();
        void Enable();
        void Disable();

        // Compare settings
        BOOL CompareSetting(SETTING* s1) { return (Setting == s1); };
        BOOL CompareSetting(CSimpleSetting* cs1) { return (CSSetting == cs1); };
        BOOL CompareToggleValue(long** pVal) { return (pToggleValue == pVal); };

        BOOL CompareToggleSetting(const char* szName,const char* szDescription);

        // Make GUI Toggle setting
        void MakeToggleSetting(const char* szName, const char* szDescription, BOOL bDefault);
        void LinkToToggleSetting(SETTING* ToggleSetting);
};


typedef struct
{
    void* pThis;
    SETTINGSPERCHANNEL_ONSETUP* pfnOnSetup;
} TOnSetupFunction;



// Local variables
static BOOL bSpcEnabled = FALSE;                             // Channel monitoring enabled
static BOOL bSpcSourceSpecific = FALSE;                      // Channel monitoring per provider
static BOOL bSpcVideoInputSpecific = TRUE;                   // Channel monitoring per video input
static BOOL bSpcVideoFormatSpecific = TRUE;                  // Channel monitoring per video format

static std::vector<TChannelSetting*> vSpcChannelSettings;    // Main list of settings

static int iSpcCurrentChannel = NO_CHANNEL;                  // Current channel
static int iSpcCurrentVideoInput = NO_VIDEOINPUT;            // Current video input
static eVideoFormat iSpcCurrentVideoFormat = (eVideoFormat)NO_VIDEOINPUT;            // Current video input
static std::string sSpcCurrentSource;                        // Current provider
static BOOL bSpcFirstSource = TRUE;
static int iSpcLastLoadedChannel = NO_CHANNEL;               // Last loaded channel specific settings
static BOOL bSetupStarted = FALSE;
static BOOL bSpcLoadedNewDefaults = FALSE;
static std::string sSpcSubSection;                           // Current subsection for registration
static std::vector<TSpcSourceInputInfo*> vSpcSourceInputInfo;
static std::vector< TOnSetupFunction> vSpcOnSetupList;

// Array of GUI toggle settings (for TreeView settings).
static SETTING* SettingsPerChannel_SettingsArray = NULL;


/// Internal functions
BOOL SettingsPerChannel_ChannelOk(int VideoInput, int Channel, eVideoFormat VideoFormat);
BOOL SettingsPerChannel_OnOff(long NewValue);
BOOL SettingsPerChannel_SourceSpecific_Change(long NewValue);
BOOL SettingsPerChannel_VideoInputSpecific_Change(long NewValue);
BOOL SettingsPerChannel_VideoFormatSpecific_Change(long NewValue);

int SettingsPerChannel_RegisterAddSetting(const char* szSubSection, CSimpleSetting* CSSetting, SETTING* Setting, long* *pToggleValue);
int SettingsPerChannel_RegisterSetting(const char* szName,const char* szDescription,BOOL bDefault, CSimpleSetting* CSSetting, SETTING* Setting, long* *pToggleValue);

BOOL SettingsPerChannel_ReadFromIni(const char* szMainSection, const char* szSubSection, CSimpleSetting* CSSetting);
BOOL SettingsPerChannel_ReadFromIni(const char* szMainSection, const char* szSubSection, SETTING* Setting);
void SettingsPerChannel_WriteToIni(const char* szMainSection, const char* szSubSection, CSimpleSetting* CSSetting, BOOL bOptimizeFileAccess);
void SettingsPerChannel_WriteToIni(const char* szMainSection, const char* szSubSection, SETTING* Setting, BOOL bOptimizeFileAccess);

void SettingsPerChannel_LoadChannelSettings(const char* szSubSection, int Input, int Channel, eVideoFormat VideoFormat);
void SettingsPerChannel_SaveChannelSettings(const char* szSubSection, int Input, int Channel, eVideoFormat VideoFormat);

BOOL SettingsPerChannel_HasTunerNotification(CSource* pSource);
BOOL SettingsPerChannel_HasTuner(int VideoInput);
            
void SettingsPerChannel_InputAndChannelChange(int PreChange, CSource* pSource, int VideoInput, int InputHasTuner, int Channel, eVideoFormat VideoFormat);

void SettingsPerChannel_SourceChange(void* pThis, int Flags, CSource* pSource);
void SettingsPerChannel_ChannelChange(void* pThis, int PreChange, int OldChannel, int NewChannel);

TSpcSourceInputInfo* SettingsPerChannel_AddSourceAndInputInfo(CSource* pSource, std::string sSourceID, int VideoInput, eVideoFormat VideoFormat, int HasTuner);
//////////////////////////////////////////////////////////////////////////////////////
// Enable/disable channel setting ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

TChannelSetting::TChannelSetting(CSimpleSetting* cs1, long** pTVal)
{
    CSSetting=cs1;
    Setting = NULL;
    pToggleValue = pTVal;

    ToggleSetting = NULL;
    ToggleSettingIsLink = 0;
}


TChannelSetting::TChannelSetting(SETTING* s1, long** pTVal)
{
    CSSetting = NULL;
    Setting = s1;
    pToggleValue = pTVal;

    ToggleSetting = NULL;
    ToggleSettingIsLink = 0;
}

TChannelSetting::TChannelSetting(long** pTVal)
{
    CSSetting = NULL;
    Setting = NULL;
    pToggleValue = pTVal;

    ToggleSetting = NULL;
    ToggleSettingIsLink = 0;
}

TChannelSetting::~TChannelSetting()
{
    if (!ToggleSettingIsLink)
    {
        if (ToggleSetting != NULL)
        {
            delete ToggleSetting;
            ToggleSetting = NULL;
        }
    }
}

void TChannelSetting::Start()
{    
    DefaultValue = GetValue();    
}

void TChannelSetting::Stop()
{
    SetValue(DefaultValue);
}

void TChannelSetting::Enable()
{
    if (ToggleSetting == NULL)
    {
        return;
    }
    *ToggleSetting->pValue = TRUE;
    Start();
    
}

void TChannelSetting::Disable()
{
    if (ToggleSetting == NULL)
    {
        return;
    }
    *ToggleSetting->pValue = FALSE;
    Stop();    
}

//////////////////////////////////////////////////////////////////////////////////////
// Get/Set setting value /////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

void TChannelSetting::DefaultState(BOOL bOnOff)
{
    if (bOnOff)
    {
        if (!bInDefaultState)
        {
            DefaultStateValue = GetValue();
            SetValue(DefaultValue, TRUE);
        }
    }
    else
    {
        SetValue(DefaultStateValue, TRUE);        
    }
}


long TChannelSetting::GetValue()
{
    if (Setting != NULL)
    {
        return Setting_GetValue(Setting);
    }
    else if (CSSetting != NULL)
    {
        return CSSetting->GetValue();
    }
    return -1;
}

void TChannelSetting::SetValue(long NewValue, BOOL bNoOnChange)
{
    if (Setting != NULL)
    {        
        try 
        {
            Setting_SetValue(Setting, NewValue, bNoOnChange?-1:1);
        } 
        catch (...)
        {
            LOG(1,"SPC: Crash while setting value of '%s' to %d",Setting->szDisplayName,NewValue); //,(LPCSTR)e.getErrorText());
        }    

    }
    else if (CSSetting != NULL)
    {
        try 
        {
            CSSetting->SetValue(NewValue, bNoOnChange?ONCHANGE_NONE:ONCHANGE_SET_FORCE);
        } 
        catch (...)
        {
            char szBuffer[200];
            CSSetting->GetDisplayText(szBuffer);
            LOG(1,"SPC: Crash while setting value of '%s' to %d",szBuffer,NewValue); //,(LPCSTR)e.getErrorText());
        }    
    }    
}

long TChannelSetting::GetDefault()
{
    if (Setting != NULL)
    {
        //return Setting_GetDefault(Setting);
        return Setting->Default;
    }
    else if (CSSetting != NULL)
    {
        return CSSetting->GetDefault();
    }
    return -1;
}


char* TChannelSetting::IniEntry()
{    
    if (Setting != NULL)
    {
        return Setting->szIniEntry;
    }
    else if (CSSetting != NULL)
    {        
        return (char*)CSSetting->GetEntry();
    }
    return "";    
}

//////////////////////////////////////////////////////////////////////////////////////
// Channel specific settings /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

void TChannelSetting::LoadChannelSpecifics(std::string SourceID, int VideoInput, int Channel, BOOL bNoOnChange)
{
    int i;
    int s;    
    for (s = 0; s < vChannelSpecificSettingsList.size(); s++)
    {        
        TSpcSourceInputInfo* pInfo = vChannelSpecificSettingsList[s].pInfo;
        if ( (pInfo->sSourceID == SourceID) && (pInfo->VideoInput == VideoInput) )
        {
            for (i = 0; i<vChannelSpecificSettingsList[s].vChannels.size(); i++)
            {                
                TChannelSpecificSetting* ChannelSpecific = &(vChannelSpecificSettingsList[s].vChannels[i]);
                if (ChannelSpecific->Channel == Channel)
                {
                    Flag = ChannelSpecific->Flag;    
                    LastValue = ChannelSpecific->LastValue;
                
                    if (GetValue() != ChannelSpecific->Value)
                    {
                        SetValue(ChannelSpecific->Value, bNoOnChange);
                    }
                    return;
                }
            }
        }
    }
    
    Flag    = 0;
    if (GetValue() != DefaultValue)
    {
        SetValue(DefaultValue, bNoOnChange);
    }
    LastValue = DefaultValue;
}

void TChannelSetting::SaveChannelSpecifics(std::string sSourceID, int VideoInput, int Channel, eVideoFormat VideoFormat)
{        
    int i;    
    int s;
    for (s = 0; s < vChannelSpecificSettingsList.size(); s++)
    {
        TSpcSourceInputInfo* pInfo = vChannelSpecificSettingsList[s].pInfo;
        if ( (pInfo->sSourceID == sSourceID) && (pInfo->VideoInput == VideoInput) )
        {
            for (i = 0; i<vChannelSpecificSettingsList[s].vChannels.size(); i++)
            {
                TChannelSpecificSetting* ChannelSpecific = &(vChannelSpecificSettingsList[s].vChannels[i]);
                if (ChannelSpecific->Channel == Channel)
                {
                    ChannelSpecific->Channel = Channel;
                    ChannelSpecific->Value = GetValue();
                    ChannelSpecific->Flag = Flag;
                    ChannelSpecific->LastValue = LastValue;
                    return;
                }
            }

        }
        
    }    

    TChannelSpecificSetting ChSpecific;
    ChSpecific.Channel = Channel;
    ChSpecific.Value = GetValue();
    ChSpecific.Flag = Flag;
    ChSpecific.LastValue = LastValue;
    

    TSourceInputChannelSpecificSetting SICSpecific;        
    SICSpecific.pInfo = SettingsPerChannel_AddSourceAndInputInfo(NULL, sSourceID, VideoInput, VideoFormat, -1);    
    SICSpecific.vChannels.push_back(ChSpecific);
    vChannelSpecificSettingsList.push_back(SICSpecific);    
}



//////////////////////////////////////////////////////////////////////////////////////
// Read & write setting from/to ini file /////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

void SettingsPerChannel_SetIniSection(char* szMainSection, const char* szSourceID, int VideoInput, int Channel, eVideoFormat VideoFormat)
{
    if ((szSourceID != NULL) && (szSourceID[0] != 0) )
    {                
        if(VideoFormat >= 0)
        {
            sprintf(szMainSection,"Channel_%d_Input_%d_%s_%s",Channel+1, VideoInput, VideoFormatNames[VideoFormat], szSourceID);                    
        }
        else
        {
            sprintf(szMainSection,"Channel_%d_Input_%d_%s",Channel+1, VideoInput, szSourceID);                    
        }
    }
    else
    {
        if(VideoFormat >= 0)
        {
            sprintf(szMainSection,"Channel_%d_Input_%d_%s",Channel+1, VideoInput, VideoFormatNames[VideoFormat]);                    
        }
        else
        {
            sprintf(szMainSection,"Channel_%d_Input_%d", Channel+1, VideoInput );
        }
    }
}

BOOL TChannelSetting::ReadFromIni(std::string SourceID, int VideoInput, int Channel, eVideoFormat VideoFormat, const char* szSection)
{
    char* szTheSection = (char*)szSection;
    if (szTheSection == NULL)
    {
        szTheSection = (char*)sSubSection.c_str();
    }
    char szMainSection[512];
    SettingsPerChannel_SetIniSection(szMainSection, SourceID.c_str(), VideoInput, Channel, VideoFormat);    
    if (Setting != NULL)
    {
        return SettingsPerChannel_ReadFromIni(szMainSection, szTheSection , Setting);
    }
    else if (CSSetting != NULL)
    {
        return SettingsPerChannel_ReadFromIni(szMainSection, szTheSection, CSSetting);
    }
    return FALSE;
}

void TChannelSetting::WriteToIni(std::string SourceID, int VideoInput, int Channel, eVideoFormat VideoFormat, BOOL bOptimizeFileAccess)
{
    char szMainSection[512];
    SettingsPerChannel_SetIniSection(szMainSection, SourceID.c_str(), VideoInput, Channel, VideoFormat);    
    if (Setting != NULL)
    {
        SettingsPerChannel_WriteToIni(szMainSection, sSubSection.c_str(), Setting, bOptimizeFileAccess);
    }
    else if (CSSetting != NULL)
    {
        SettingsPerChannel_WriteToIni(szMainSection, sSubSection.c_str(), CSSetting, bOptimizeFileAccess);
    }    
}

//////////////////////////////////////////////////////////////////////////////////////
// Make toggle setting ///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

void TChannelSetting::MakeToggleSetting(const char* szName,const char* szDescription,BOOL bDefault)
{
    BOOL bStartValue = bDefault;
    if (ToggleSetting != NULL)
    {
        if (!ToggleSettingIsLink)
        {
            bStartValue = *(ToggleSetting->pValue);
            delete ToggleSetting;
            ToggleSetting = NULL;
        }
    }
    ToggleSetting = new SETTING;
    sDisplayName = std::string(szDescription);
    sIniEntry = std::string(szName);
    sIniSection = std::string("SettingsPerChannel");    
    bEnabled = bStartValue;
    ToggleSetting->szDisplayName = (char*)sDisplayName.c_str();
    ToggleSetting->Type = ONOFF;
    ToggleSetting->LastSavedValue = bDefault; 
    ToggleSetting->pValue = (long*)&(bEnabled); 
    ToggleSetting->Default = bDefault;
    ToggleSetting->MinValue = 0;
    ToggleSetting->MaxValue = 1;
    ToggleSetting->StepValue = 1;
    ToggleSetting->OSDDivider = 1;
    ToggleSetting->pszList = NULL;
    ToggleSetting->szIniSection = (char*)sIniSection.c_str();
    ToggleSetting->szIniEntry = (char*)sIniEntry.c_str();    
    ToggleSetting->pfnOnChange = NULL; //SettingsPerChannel_OnChange; //NULL;

    ToggleSettingIsLink = 0;
    
    if (pToggleValue != NULL)
    {
        *pToggleValue = ToggleSetting->pValue;
    }
}

void TChannelSetting::LinkToToggleSetting(SETTING* ToggleSetting1)
{
    if ( (ToggleSetting != NULL) && (!ToggleSettingIsLink) 
         && (ToggleSetting != ToggleSetting1) )
    {
        delete ToggleSetting;
        ToggleSetting = NULL;
    }

    ToggleSetting = ToggleSetting1;
    ToggleSettingIsLink = 1;
    if (pToggleValue != NULL)
    {
        *pToggleValue = ToggleSetting->pValue;
    }
}

BOOL TChannelSetting::CompareToggleSetting(const char* szName,const char* szDescription)
{   
   if (ToggleSetting == NULL) 
   {
      return FALSE;
   }
   if (ToggleSettingIsLink)
   {
      return ( (std::string(ToggleSetting->szIniEntry) == std::string(szName)) && (std::string(ToggleSetting->szDisplayName) == std::string(szDescription)) );
   }
   else
   {      
      return ( (sIniEntry == std::string(szName)) && (sDisplayName == std::string(szDescription)) );
  }
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

BOOL SettingsPerChannel()
{
    return bSpcEnabled;
}

BOOL SettingsPerInput()
{
    return bSpcVideoInputSpecific;
}

BOOL SettingsPerFormat()
{
    return bSpcVideoFormatSpecific;
}

void SettingsPerChannel_Enable()
{        
    SettingsPerChannel_OnOff(TRUE);
}

void SettingsPerChannel_Disable()
{    
    SettingsPerChannel_OnOff(FALSE);
}

void SettingsPerChannel_ToDefaultState(BOOL bTrue)
{    
    if (!bSpcEnabled)
    {
        return;
    }
        
    for(vector<TChannelSetting*>::iterator it = vSpcChannelSettings.begin();
                it != vSpcChannelSettings.end(); ++it)
    {
         (*it)->DefaultState(bTrue);
    }
}

BOOL SettingsPerChannel_OnOff(long NewValue)
{
    bSpcEnabled = NewValue;
    
    LOG(2,"SPC: Use settings per channel %s.",(bSpcEnabled)?"on":"off");
    
    if (bSpcEnabled)
    {
        LOG(3,"SPC: Collect default values.");
        for(vector<TChannelSetting*>::iterator it = vSpcChannelSettings.begin();
                it != vSpcChannelSettings.end(); ++it)
        {
            (*it)->Start();
        }
        //Read
        if (SettingsPerChannel_ChannelOk(iSpcCurrentVideoInput, iSpcCurrentChannel, iSpcCurrentVideoFormat))
        {            
            SettingsPerChannel_ReadSettings(NULL, iSpcCurrentVideoInput, iSpcCurrentChannel, iSpcCurrentVideoFormat, 1);
        }
    } 
    else 
    {
        if (SettingsPerChannel_ChannelOk(iSpcCurrentVideoInput, iSpcCurrentChannel, iSpcCurrentVideoFormat))
        {
            SettingsPerChannel_SaveChannelSettings(NULL, iSpcCurrentVideoInput, iSpcCurrentChannel, iSpcCurrentVideoFormat);
        }
        LOG(3,"SPC: Reset default values.");
        for(vector<TChannelSetting*>::iterator it = vSpcChannelSettings.begin();
                it != vSpcChannelSettings.end(); ++it)            
        {
            (*it)->Stop();
        }        
    } 
       
    return FALSE;
}


void SettingsPerChannel_NewDefaults(const char* szSubSection,BOOL bCurrentValue)
{    
    for(vector<TChannelSetting*>::iterator it = vSpcChannelSettings.begin();
                it != vSpcChannelSettings.end(); ++it)
    {
       if ( (szSubSection == NULL) || ((*it)->sSubSection == std::string(szSubSection)) )
       {
          if (bCurrentValue)
          {
              (*it)->DefaultValue = (*it)->GetValue();
          }
          else
          {
              (*it)->DefaultValue = (*it)->GetDefault();
          }
       }
    }
    bSpcLoadedNewDefaults = TRUE;
}

BOOL SettingsPerChannel_SourceSpecific_Change(long NewValue)
{    
    if (Providers_GetCurrentSource() == NULL)
    {
        return FALSE;
    }

    // Save settings    
    SettingsPerChannel_SourceChange(NULL, 1, Providers_GetCurrentSource());

    bSpcSourceSpecific = NewValue;

    // Load settings
    SettingsPerChannel_SourceChange(NULL, 0, Providers_GetCurrentSource());
    return FALSE;
}

BOOL SettingsPerChannel_VideoInputSpecific_Change(long NewValue)
{
    if (Providers_GetCurrentSource() == NULL)
    {
        return FALSE;
    }

    bSpcVideoInputSpecific = NewValue;

    // save
    SettingsPerChannel_InputAndChannelChange(1, Providers_GetCurrentSource(), iSpcCurrentVideoInput, -1, iSpcCurrentChannel, iSpcCurrentVideoFormat);

    if (bSpcVideoInputSpecific)
    {
      // load defaults
      if (bSetupStarted && SettingsPerChannel_ChannelOk(iSpcCurrentVideoInput, iSpcCurrentChannel, iSpcCurrentVideoFormat))
      {                          
          SettingsPerChannel_ReadSettings(NULL,iSpcCurrentVideoInput,iSpcCurrentChannel,iSpcCurrentVideoFormat, -1); 
      }
    } 
    else
    {
       // load
       SettingsPerChannel_InputAndChannelChange(0, Providers_GetCurrentSource(), iSpcCurrentVideoInput, -1, iSpcCurrentChannel, iSpcCurrentVideoFormat);
    }

    return FALSE;
}

BOOL SettingsPerChannel_VideoFormatSpecific_Change(long NewValue)
{
    if (Providers_GetCurrentSource() == NULL)
    {
        return FALSE;
    }

    bSpcVideoFormatSpecific = NewValue;

    // save
    SettingsPerChannel_InputAndChannelChange(1, Providers_GetCurrentSource(), iSpcCurrentVideoInput, -1, iSpcCurrentChannel, iSpcCurrentVideoFormat);

    if (bSpcVideoInputSpecific)
    {
      // load defaults
      if (bSetupStarted && SettingsPerChannel_ChannelOk(iSpcCurrentVideoInput, iSpcCurrentChannel, iSpcCurrentVideoFormat))
      {                          
          SettingsPerChannel_ReadSettings(NULL,iSpcCurrentVideoInput,iSpcCurrentChannel, iSpcCurrentVideoFormat, -1); 
      }
    } 
    else
    {
       // load
       SettingsPerChannel_InputAndChannelChange(0, Providers_GetCurrentSource(), iSpcCurrentVideoInput, -1, iSpcCurrentChannel, iSpcCurrentVideoFormat);
    }

    return FALSE;
}

void SettingsPerChannel_ClearAll()
{
    LOG(3,"SPC: Clear all.");
    if (SettingsPerChannel_SettingsArray != NULL)
    {
        delete[] SettingsPerChannel_SettingsArray;
        SettingsPerChannel_SettingsArray = NULL;
    }

    //Clear list
    if (vSpcChannelSettings.size()>0) 
    {                               
        int n = 0;
        for(vector<TChannelSetting*>::iterator it = vSpcChannelSettings.begin();
                it != vSpcChannelSettings.end(); ++it)
        {            
            if ( (it != NULL) && ((*it) != NULL) )
            {
                delete (*it);
            }
            n++;
        }
        vSpcChannelSettings.clear();
    }
    //clear source info list
    int i;
    for (i = 0; i < vSpcSourceInputInfo.size(); i++)
    {
         delete vSpcSourceInputInfo[i];
    }
    vSpcSourceInputInfo.clear();
}


/////////////////////////////////////////////////////////////////////////////////
// Register setting /////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

// Create new channel setting
int SettingsPerChannel_RegisterAddSetting(const char* szSubSection, CSimpleSetting* CSSetting, SETTING* Setting, long* *pToggleValue)
{
    int n = 0;
    for(vector<TChannelSetting*>::iterator it = vSpcChannelSettings.begin();
        it != vSpcChannelSettings.end();
        ++it)
    {                
        if  (    ( ( (Setting != NULL) && ((*it)->CompareSetting(Setting)) ) 
                || ( (CSSetting != NULL) && ((*it)->CompareSetting(CSSetting)) ) 
                || ( (Setting==NULL) && (CSSetting==NULL) && (pToggleValue != NULL) && ((*it)->CompareToggleValue(pToggleValue) ) ) ) )
                //&& ( (szSubSection==NULL) || (std::string(szSubSection) == (*it)->sSubSection) ) )
        {
            //setting already exists
            return -n;
        }        
        n++;
    }    
    
    // Add new
    TChannelSetting* ChannelSetting;
    
    if (Setting != NULL)
    {
      ChannelSetting = new TChannelSetting(Setting, pToggleValue);
    } else if (CSSetting != NULL)
    {
      ChannelSetting = new TChannelSetting(CSSetting, pToggleValue);
    } else {
      ChannelSetting = new TChannelSetting(pToggleValue);
    }

    ChannelSetting->LastValue = ChannelSetting->GetValue();
    ChannelSetting->DefaultValue = ChannelSetting->GetValue(); //GetDefault();    
    ChannelSetting->Flag = 0;
    ChannelSetting->ToggleSetting = NULL;
    ChannelSetting->ToggleSettingIsLink = 0;
    ChannelSetting->sSubSection = szSubSection;

    ChannelSetting->DefaultStateValue = ChannelSetting->DefaultValue;
    ChannelSetting->bInDefaultState = FALSE;

    vSpcChannelSettings.push_back(ChannelSetting);
    return n;
}

//
int SettingsPerChannel_RegisterSetting(const char* szName,const char* szDescription,BOOL bDefault, CSimpleSetting* CSSetting, SETTING* Setting, long* *pToggleValue)
{    
    if ((CSSetting == NULL) && (Setting==NULL) && (pToggleValue==NULL))
    {
        for(vector<TChannelSetting*>::iterator it = vSpcChannelSettings.begin();
            it != vSpcChannelSettings.end();
            ++it)
        {
            if (!(*it)->ToggleSettingIsLink && (*it)->CompareToggleSetting (szName, szDescription))        
            {
                return -1;
            }
        }    
    }

    int n = SettingsPerChannel_RegisterAddSetting(sSpcSubSection.c_str(), CSSetting, Setting, pToggleValue);
    TChannelSetting* ChannelSetting;
    if (n < 0)
    {
        n = -n;
        ChannelSetting = vSpcChannelSettings[n];
        if ( ChannelSetting == NULL )
        {
            //Should never happen
            return -1;
        }
        //if (ChannelSetting->CompareToggleSetting (szName, szDescription))
        {
            LOG(3,"SPC: Register setting: already exists: %s (%s)",ChannelSetting->IniEntry(),ChannelSetting->sSubSection.c_str());
            // Duplicate
            return -1;
        }
    }
    ChannelSetting = vSpcChannelSettings[n];
    if ( ChannelSetting == NULL )
    {
        //Should never happen
        return -1;
    }
    if ((CSSetting == NULL) && (Setting==NULL) && (pToggleValue==NULL))
    {
        LOG(3,"SPC: Register name: %s (%s)",szName,szDescription);
    }
    else
    {
        LOG(3,"SPC: Register setting: %s (%s)",ChannelSetting->IniEntry(),ChannelSetting->sSubSection.c_str());
    }

    // Check if toggle settting already exists

    BOOL bToggleSettingExists = FALSE;
    for(vector<TChannelSetting*>::iterator it = vSpcChannelSettings.begin();
        it != vSpcChannelSettings.end();
        ++it)
    {
        if (!(*it)->ToggleSettingIsLink && (*it)->CompareToggleSetting (szName, szDescription))        
        {                    
            if ((CSSetting == NULL) && (Setting==NULL) && (pToggleValue==NULL))
            {
                // Already exists. No setting, so don't do anything
            }
            else
            {
                ChannelSetting->LinkToToggleSetting((*it)->ToggleSetting);
            }
            bToggleSettingExists = TRUE;
            break;
        }
    }
    
    if (bToggleSettingExists == FALSE)
    {
        ChannelSetting->MakeToggleSetting(szName, szDescription, bDefault);
        Setting_ReadFromIni(ChannelSetting->ToggleSetting);
    }
    
    return n;
}


// More obvious for external access
int SettingsPerChannel_RegisterSetting(const char* szName,const char* szDescription,BOOL bDefault, SETTING* Setting)
{
    return SettingsPerChannel_RegisterSetting(szName,szDescription,bDefault,NULL, Setting,NULL);
}

int SettingsPerChannel_RegisterSetting(const char* szName,const char* szDescription,BOOL bDefault, CSimpleSetting* CSSetting)
{
    return SettingsPerChannel_RegisterSetting(szName,szDescription,bDefault,CSSetting, NULL,NULL);
}

int SettingsPerChannel_RegisterSetting(const char* szName, const char* szDescription,BOOL bDefault, long** pValue)
{    
    return SettingsPerChannel_RegisterSetting(szName, szDescription, bDefault, NULL,NULL,pValue);
}

int SettingsPerChannel_RegisterSetting(const char* szName, const char* szDescription,BOOL bDefault)
{    
    return SettingsPerChannel_RegisterSetting(szName, szDescription, bDefault, NULL,NULL,NULL);
}



void SettingsPerChannel_RegisterSetSection(const char* szSubSection)
{        
    sSpcSubSection = szSubSection;
}

void SettingsPerChannel_UnregisterSection(const char* szSubSection)
{
    std::vector<TChannelSetting*> newSettings;

    for(vector<TChannelSetting*>::iterator it = vSpcChannelSettings.begin();
                it != vSpcChannelSettings.end(); ++it)
    {
        if ((*it)->sSubSection == std::string(szSubSection))
        {
            if (!(*it)->ToggleSettingIsLink)
            {
               if ((*it)->ToggleSetting != NULL)
               {
                  Setting_WriteToIni((*it)->ToggleSetting, TRUE);
                  delete (*it)->ToggleSetting;
                  (*it)->ToggleSetting = NULL;
               }
            }
            LOG(3,"SPC: Unregister setting: %s (%s)",(*it)->IniEntry(),(*it)->sSubSection.c_str());
            delete (*it);
        }
        else
        {
            newSettings.push_back((*it));
        }
    }           
    vSpcChannelSettings = newSettings;
}

 

/////////////////////////////////////////////////////////////////////////////////
// Read & write channel settings ////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

void SettingsPerChannel_WriteToIni(const char* szMainSection, const char* szSubSection, CSimpleSetting* CSSetting, BOOL bOptimizeFileAccess)
{    
    std::string sOldEntry = CSSetting->GetEntry();;
    std::string sOldSection = CSSetting->GetSection();;  
    std::string sNewEntry = szSubSection + std::string("_") + sOldEntry;
    CSSetting->SetSection(szMainSection);
    CSSetting->SetEntry(sNewEntry.c_str());
    CSSetting->WriteToIni(bOptimizeFileAccess);        
    CSSetting->SetEntry(sOldEntry.c_str());
    CSSetting->SetSection(sOldSection.c_str());
}

void SettingsPerChannel_WriteToIni(const char* szMainSection, const char* szSubSection, SETTING* Setting, BOOL bOptimizeFileAccess)
{
    char* szIniSection = Setting->szIniSection;        
    char* szIniEntry = Setting->szIniEntry;
    std::string sNewEntry = szSubSection + std::string("_") + szIniEntry;

    Setting->szIniSection = (char*)szMainSection;
    Setting->szIniEntry = (char*)sNewEntry.c_str();
    Setting_WriteToIni(Setting,bOptimizeFileAccess);                    
    Setting->szIniSection = szIniSection;
    Setting->szIniEntry = szIniEntry;    
}

BOOL SettingsPerChannel_ReadFromIni(const char* szMainSection, const char* szSubSection, CSimpleSetting* CSSetting)
{
    BOOL result = FALSE;
    char* szNewSection;

    if (szMainSection != NULL)
    {                        
        szNewSection = (char*)szMainSection;
    }
    else
    {
        szNewSection = (char*)szSubSection;
    }

    std::string sOldEntry;
    std::string sNewEntry;
    
    std::string sOldSection = CSSetting->GetSection();;  
    
    CSSetting->SetSection(szNewSection);
        
    if (szMainSection != NULL)
    {
        sOldEntry = CSSetting->GetEntry();;
        sNewEntry = szSubSection + std::string("_") + sOldEntry;
        CSSetting->SetEntry(sNewEntry.c_str());
    }
        
    result = CSSetting->ReadFromIni(TRUE);
        
    if (szMainSection != NULL)
    {
       CSSetting->SetEntry(sOldEntry.c_str());
    }
    CSSetting->SetSection(sOldSection.c_str());
    return result;
}

BOOL SettingsPerChannel_ReadFromIni(const char* szMainSection, const char* szSubSection, SETTING* Setting)
{
    BOOL result = FALSE;
    char* szNewSection;

    if (szMainSection != NULL)
    {                        
        szNewSection = (char*)szMainSection;
    }
    else
    {
        szNewSection = (char*)szSubSection;
    }
    
    char* szIniSection = Setting->szIniSection;        
    char* szIniEntry = Setting->szIniEntry;

    std::string sNewEntry = szSubSection + std::string("_") + szIniEntry;

    Setting->szIniSection = szNewSection;
    Setting->szIniEntry = (char*)sNewEntry.c_str();

    result = Setting_ReadFromIni(Setting, TRUE);
        
    Setting->szIniSection = szIniSection;
    Setting->szIniEntry = szIniEntry;
    
    return result;
}




void SettingsPerChannel_ReadSettings(const char* szSubSection, int VideoInput, int Channel, eVideoFormat VideoFormat, int DefaultValuesFirst)
{
    LOG(2,"SPC: Read settings for input %d, channel %d.",VideoInput,Channel);
    if (!SettingsPerChannel() || !SettingsPerChannel_ChannelOk(VideoInput, Channel, VideoFormat))
    {
        return;
    }

    if (iSpcLastLoadedChannel != Channel)
    {
        LOG(3,"SPC: Read settings: Load channel specifics");
    }

    for(vector<TChannelSetting*>::iterator it = vSpcChannelSettings.begin();
        it != vSpcChannelSettings.end();
        ++it)
    {        
        TChannelSetting* CSetting = (*it);
        
        // If setting enabled, setting is available and in right subsection
        if ( CSetting->Enabled()  && CSetting->Valid()
             && ( (szSubSection == NULL) || (CSetting->sSubSection == std::string(szSubSection)) ) )
        {            
            int OldValue = CSetting->GetValue();
            
            if (iSpcLastLoadedChannel != Channel)
            {                
                CSetting->LoadChannelSpecifics(sSpcCurrentSource, VideoInput, Channel, TRUE);                
            }

            int NewDefaultValue = CSetting->DefaultValue;
            int NewValue = CSetting->GetValue();

            LOG(3,"SPC: Read settings. (%s). Default = %d; Channel value = %d",CSetting->IniEntry(),NewDefaultValue,NewValue);
            
            // Read value from current channel section
            if (DefaultValuesFirst>=0)
            {                                                
                if (CSetting->Flag&4)
                {
                    // Already read                    
                }
                else 
                {
                    if (CSetting->ReadFromIni(sSpcCurrentSource, VideoInput, Channel, VideoFormat))
                    {                        
                        NewValue = CSetting->GetValue();
                        LOG(3,"SPC: Read settings. (%s). Found in .ini file: %d",CSetting->IniEntry(),NewValue);
                        CSetting->LastValue = NewValue;

                        CSetting->Flag |= 8;  //physically in inifile                        
                    }                    
                    CSetting->Flag |= 4;  //mark read
                }
            }

            if (DefaultValuesFirst == 2)
            {
                NewValue = NewDefaultValue;
                if (CSetting->Flag & 8)
                {
                    CSetting->Flag |= 1;
                }
            }

            // Restore default value
            if (DefaultValuesFirst<0)
            {
                NewValue = NewDefaultValue;                
            }
            
            if (OldValue != NewValue)
            {
                LOG(3,"SPC: Read settings. (%s). Set new value: %d -> %d",CSetting->IniEntry(),OldValue,NewValue);
                CSetting->SetValue(NewValue);
            }
            else
            {
                LOG(3,"SPC: Read settings. (%s). Value not changed: %d",CSetting->IniEntry(),NewValue);
            }
        }
    }
    iSpcLastLoadedChannel = Channel;
}

void SettingsPerChannel_ReadDefaults(const char* szSubSection, int Input, int Channel, eVideoFormat VideoFormat)
{
    SettingsPerChannel_ReadSettings(szSubSection, Input, Channel, VideoFormat, -1);
}


void SettingsPerChannel_WriteSettings(BOOL bOptimizeFileAccess)
{
    
    // Save current channel settings (if enabled)
    if (SettingsPerChannel() && SettingsPerChannel_ChannelOk(iSpcCurrentVideoInput, iSpcCurrentChannel, iSpcCurrentVideoFormat))
    {
        for(vector<TChannelSetting*>::iterator it = vSpcChannelSettings.begin();
            it != vSpcChannelSettings.end();
            ++it)
        {        
            TChannelSetting* CSetting = (*it);
            if ( CSetting->Enabled() && CSetting->Valid() )          
            {        
                LOG(3,"SPC: Write settings. (%s). Save value to (%d,%d): %d",CSetting->IniEntry(),iSpcCurrentVideoInput,iSpcCurrentChannel,CSetting->GetValue());
                CSetting->SaveChannelSpecifics(sSpcCurrentSource,iSpcCurrentVideoInput, iSpcCurrentChannel, iSpcCurrentVideoFormat);              
            }
        }              
    }
    
    // Write all changed settings
    for(vector<TChannelSetting*>::iterator it = vSpcChannelSettings.begin();
        it != vSpcChannelSettings.end();
        ++it)
    {                
        TChannelSetting* CSetting = (*it);

        // If setting enabled, and in right subsection
        if ( CSetting->Enabled() && CSetting->Valid() )
             //&& ( (szSubSection == NULL) || (CSetting->sSubSection == std::string(szSubSection)) ) )
        {        
            int OldValue = CSetting->GetValue();
            int s;
            int i;
            for (s = 0; s < CSetting->vChannelSpecificSettingsList.size(); s++)
            {
                TSpcSourceInputInfo* pInfo = CSetting->vChannelSpecificSettingsList[s].pInfo;
                //if ( (pInfo->sSourceID == sSourceID) && (pInfo->VideoInput == VideoInput) )                
                {
                    for (i = 0; i<CSetting->vChannelSpecificSettingsList[s].vChannels.size(); i++)
                    {
                        TChannelSpecificSetting* ChSpecific = &(CSetting->vChannelSpecificSettingsList[s].vChannels[i]);
                        
                        //if ((Channel == NO_CHANNEL) || (Channel == ChSpecific->Channel))
                        {            
                            // Write of write bit of Flag is set, or if value has changed
                            if ( (ChSpecific->Flag&1) || (ChSpecific->Value != ChSpecific->LastValue) )
                            {
                                LOG(3,"SPC: Write settings. (%s). Write %d",CSetting->IniEntry(),ChSpecific->Value);
                                if (ChSpecific->Value != CSetting->GetValue()) 
                                {
                                    CSetting->SetValue(ChSpecific->Value, TRUE);
                                }
                                CSetting->WriteToIni(pInfo->sSourceID, pInfo->VideoInput, ChSpecific->Channel, pInfo->VideoFormat, FALSE);
                            }            
                            // Clear write flag
                            ChSpecific->Flag &= ~1;            
    
                            // Set new lastValue
                            ChSpecific->LastValue = ChSpecific->Value;            
                        }
                    }
                }
            } 
            if (OldValue != CSetting->GetValue()) 
            { 
                CSetting->SetValue(OldValue, TRUE);
            }
        }        
    }        

    
}


void SettingsPerChannel_ClearSettings(const char* szSubSection, int VideoInput, int Channel, eVideoFormat VideoFormat, int ClearIniSection) //, std::vector<std::string>* vsDefaultSections)
{
    if (VideoInput == -2)
    {
        VideoInput = iSpcCurrentVideoInput;
    }
    if (Channel == -2)
    {
        Channel = iSpcCurrentChannel;
    }
    if (VideoFormat == -2)
    {
        VideoFormat = iSpcCurrentVideoFormat;
    }
    SettingsPerChannel_ReadSettings(szSubSection,VideoInput,Channel,VideoFormat, 2); //,vsDefaultSections);
    if (ClearIniSection)
    {
       
    }
    //SettingsPerChannel_WriteSettings(szSubSection, Channel, TRUE);      
    SettingsPerChannel_SaveChannelSettings(szSubSection,VideoInput, Channel, VideoFormat);
}


/////////////////////////////////////////////////////////////////////////////////
// Load/Save channel settings ///////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


void SettingsPerChannel_LoadChannelSettings(const char* szSubSection, int VideoInput, int Channel, eVideoFormat VideoFormat)
{
    if (!SettingsPerChannel())
    {
        return;
    }

    if (!SettingsPerChannel_ChannelOk(VideoInput, Channel, VideoFormat))
    {
        return;
    }

    for(vector<TChannelSetting*>::iterator it = vSpcChannelSettings.begin();
        it != vSpcChannelSettings.end();
        ++it)
    {        
        
        TChannelSetting* CSetting = (*it);
        // If setting enabled, and in right subsection
        if ( CSetting->Enabled() && CSetting->Valid() 
             && ( (szSubSection == NULL) || (CSetting->sSubSection == std::string(szSubSection)) ) )
        {        
            CSetting->LoadChannelSpecifics(sSpcCurrentSource,VideoInput, Channel);
        }        

    }        

    iSpcLastLoadedChannel = Channel;    
}

void SettingsPerChannel_SaveChannelSettings(const char* szSubSection, int VideoInput, int Channel, eVideoFormat VideoFormat)
{
    for(vector<TChannelSetting*>::iterator it = vSpcChannelSettings.begin();
        it != vSpcChannelSettings.end();
        ++it)
    {        
        
        TChannelSetting* CSetting = (*it);
        // If setting enabled, and in right subsection
        if ( CSetting->Enabled() && CSetting->Valid() 
             && ( (szSubSection == NULL) || (CSetting->sSubSection == std::string(szSubSection)) ) )
        {        
            LOG(3,"SPC: Save settings. (%s). Value = %d",CSetting->IniEntry(),CSetting->GetValue());
            CSetting->SaveChannelSpecifics(sSpcCurrentSource, VideoInput, Channel, VideoFormat);
        }        

    }        
}

/////////////////////////////////////////////////////////////////////////////////
// Tuner notification ///////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

//
   
BOOL SettingsPerChannel_ChannelOk(int VideoInput, int Channel, eVideoFormat VideoFormat)
{
    if (Providers_GetCurrentSource() == NULL) 
    {
        return FALSE;
    }
    
    int i;
    for (i = 0; i < vSpcSourceInputInfo.size(); i++)
    {
        if ((vSpcSourceInputInfo[i]->pSource == Providers_GetCurrentSource()) &&
            (vSpcSourceInputInfo[i]->VideoInput == VideoInput) &&
            (vSpcSourceInputInfo[i]->VideoFormat == VideoFormat))
        {
            if (vSpcSourceInputInfo[i]->HasTuner == 0)
            {
                return (Channel == 0) ? TRUE : FALSE;               
            }
            else
            {
                return (Channel >= 0) ? TRUE : FALSE;               
            }
        }
    }
    
    if (Providers_GetCurrentSource()->IsInTunerMode())
    {
        return (Channel >= 0) ? TRUE : FALSE;               
    }
    else
    {
        return (Channel == 0) ? TRUE : FALSE;
    }
}


TSpcSourceInputInfo* SettingsPerChannel_AddSourceAndInputInfo(CSource* pSource, std::string sSourceID, int VideoInput, eVideoFormat VideoFormat, int HasTuner)
{    
    int i;
    if (pSource != NULL)
    {
        sSourceID = pSource->IDString();
    }
    for (i = 0; i < vSpcSourceInputInfo.size(); i++)
    {
        if ((vSpcSourceInputInfo[i]->sSourceID == sSourceID) && 
            (vSpcSourceInputInfo[i]->VideoInput == VideoInput) &&
            (vSpcSourceInputInfo[i]->VideoFormat == VideoFormat))
        {
            if (HasTuner >= 0)
            {
                vSpcSourceInputInfo[i]->HasTuner = HasTuner;
            }   
            return (vSpcSourceInputInfo[i]);
        }
    }

   
   // New source & input info
   TSpcSourceInputInfo* SSIInfo = new TSpcSourceInputInfo;

   SSIInfo->pSource = pSource;
   SSIInfo->sSourceID = sSourceID;
   SSIInfo->VideoInput = VideoInput;
   SSIInfo->HasTuner = HasTuner;
   SSIInfo->LastChannel = NO_CHANNEL;
   if (HasTuner >= 0)
   {
       SSIInfo->LastChannel = 0;
   }      

   i = vSpcSourceInputInfo.size();
   vSpcSourceInputInfo.push_back(SSIInfo);

   return (vSpcSourceInputInfo[i]);
}

/////////////////////////////////////////////////////////////////////////////////
// Channel/source change ////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

void SettingsPerChannel_InputAndChannelChange(int PreChange, CSource* pSource, int VideoInput, int InputHasTuner, int Channel, eVideoFormat VideoFormat)
{      
   if (pSource == NULL)
   {
        // No provider
        return;
   }

   if (PreChange)
   {            
      // Save settings if input & channel are valid
      if (bSetupStarted && SettingsPerChannel() && SettingsPerChannel_ChannelOk(iSpcCurrentVideoInput, iSpcCurrentChannel, iSpcCurrentVideoFormat)
          && (bSpcVideoInputSpecific || bSpcVideoFormatSpecific || (InputHasTuner==1)) )
      {                    
          LOG(2,"SPC: Change input/channel: save settings for in=%d, ch=%d (%s)", iSpcCurrentVideoInput, iSpcCurrentChannel, sSpcCurrentSource.c_str());
          for(vector<TChannelSetting*>::iterator it = vSpcChannelSettings.begin();
            it != vSpcChannelSettings.end();
            ++it)
          {        
              TChannelSetting* CSetting = (*it);
              if ( CSetting->Enabled() && CSetting->Valid() )          
              {        
                 CSetting->SaveChannelSpecifics(sSpcCurrentSource,iSpcCurrentVideoInput, iSpcCurrentChannel, iSpcCurrentVideoFormat);
              }
          }  
      }            
   }
   else
   {
      BOOL bVideoInputMayLoadSettings = FALSE;
      if (iSpcCurrentVideoInput != VideoInput)
      {
         //changed
         TSpcSourceInputInfo* pInfo;
         int HadTuner = -1;
         if ( (iSpcCurrentVideoInput != NO_CHANNEL) || (iSpcCurrentVideoInput != NO_VIDEOINPUT) || (iSpcCurrentVideoFormat != NO_VIDEOFORMAT))
         {
            pInfo = SettingsPerChannel_AddSourceAndInputInfo(pSource, pSource->IDString(), iSpcCurrentVideoInput, iSpcCurrentVideoFormat, -1);
            pInfo->LastChannel = iSpcCurrentChannel;
            HadTuner = pInfo->HasTuner;
         }
         iSpcLastLoadedChannel = NO_CHANNEL;

         if (Channel == NO_CHANNEL)
         {
             // get last one
             pInfo = SettingsPerChannel_AddSourceAndInputInfo(pSource, pSource->IDString(), VideoInput, VideoFormat, InputHasTuner);
             Channel = pInfo->LastChannel;
             if (!bSpcVideoInputSpecific && !bSpcVideoFormatSpecific && (HadTuner != pInfo->HasTuner) && (pInfo->HasTuner == 0))
             {                
                // Left tuner mode
                bVideoInputMayLoadSettings = TRUE;
             }
         }
         iSpcCurrentVideoInput = VideoInput;
      }

      if (iSpcCurrentVideoFormat != VideoFormat)
      {
         iSpcCurrentVideoFormat = VideoFormat;
      }
          
      if ( (iSpcCurrentChannel != Channel) && (Channel != NO_CHANNEL) )
      {
          //changed
         iSpcLastLoadedChannel = NO_CHANNEL; 
         iSpcCurrentChannel = Channel;
      }
      
      LOG(3,"SPC: Input/Channel change. Now %d,%d",iSpcCurrentVideoInput,iSpcCurrentChannel);

      if (bSetupStarted && SettingsPerChannel() && SettingsPerChannel_ChannelOk(iSpcCurrentVideoInput, iSpcCurrentChannel, iSpcCurrentVideoFormat)
          && (bSpcVideoInputSpecific || bSpcVideoFormatSpecific|| bVideoInputMayLoadSettings || pSource->IsInTunerMode()) )
      {                    
          int DefaultSettings = 1;
          if (bVideoInputMayLoadSettings)
          {
              // Just left tuner mode, load default settings
              DefaultSettings = -1;
          }
          SettingsPerChannel_ReadSettings(NULL,iSpcCurrentVideoInput,iSpcCurrentChannel, iSpcCurrentVideoFormat, DefaultSettings); 
      }
   }   
}


void SettingsPerChannel_SourceChange(void* pThis, int Flags, CSource* pSource)
{    
    LOG(2,"SPC: Source (%s)change (%i): %s",(Flags&1)?"pre":"",Flags,(pSource==NULL)?"NULL":pSource->IDString());
    if (Flags&1)
    {
         SettingsPerChannel_Setup(2);

         // before change
         if (!SettingsPerChannel() || !SettingsPerChannel_ChannelOk(iSpcCurrentVideoInput, iSpcCurrentChannel, iSpcCurrentVideoFormat))
         {
            return;
         }
         LOG(3,"SPC: Change source: save channel settings");
         SettingsPerChannel_SaveChannelSettings(NULL, iSpcCurrentVideoInput, iSpcCurrentChannel, iSpcCurrentVideoFormat);
         iSpcLastLoadedChannel = NO_CHANNEL; // force read of new channel values         
    }
    else
    {        
        // after change
        std::string sLastSource = sSpcCurrentSource;
        BOOL bChanged = FALSE;
        if (1)
        {            
            if (pSource == NULL)
            {
                sSpcCurrentSource = "";
            } 
            else
            {
                sSpcCurrentSource = pSource->IDString();
            }
            if (bSpcFirstSource && (sSpcCurrentSource != sLastSource))
            {                
                bChanged = TRUE;
            }
            bSpcFirstSource = FALSE;
            if (!bSpcSourceSpecific)
            {
                sSpcCurrentSource = "";
            }
        }
        
        if (bChanged && !bSpcSourceSpecific)
        {
            LOG(3,"SPC: Source change: reset channel specific settings");
            //reset channels & inputs
            for(vector<TChannelSetting*>::iterator it = vSpcChannelSettings.begin();
                it != vSpcChannelSettings.end();
                ++it)
            {
                (*it)->vChannelSpecificSettingsList.clear();                
            }
            //clear source info list
            int i; 
            for (i = 0; i < vSpcSourceInputInfo.size(); i++)
            {
                delete vSpcSourceInputInfo[i];
            }
            vSpcSourceInputInfo.clear();
        }

        if (bChanged)
        {
            LOG(3,"SPC: Source change: setup settings per channel (3).");
            iSpcCurrentVideoInput = NO_VIDEOINPUT;
            iSpcCurrentChannel = NO_CHANNEL;            
            iSpcLastLoadedChannel = NO_CHANNEL;

            if (pSource != NULL)
			{
				iSpcCurrentVideoInput = pSource->GetInput(VIDEOINPUT);
			}

            bSpcLoadedNewDefaults = FALSE;
            SettingsPerChannel_Setup(3);            
            //New defaults
            if (!bSpcLoadedNewDefaults)
            {
                SettingsPerChannel_NewDefaults(NULL,TRUE);
            }
        }

        if (SettingsPerChannel() && SettingsPerChannel_ChannelOk(iSpcCurrentVideoInput, iSpcCurrentChannel, iSpcCurrentVideoFormat))
        {            
            SettingsPerChannel_ReadSettings(NULL,iSpcCurrentVideoInput,iSpcCurrentChannel,iSpcCurrentVideoFormat,1);      
        }
    }
}


void SettingsPerChannel_EventHandler(void *pThis, CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp)
{
	if (!bSetupStarted) {  return; }
	try 
   {
       if (Event == EVENT_SOURCE_PRECHANGE) 
       {
            SettingsPerChannel_SourceChange(pThis, 1, (CSource*)OldValue);
       } 
       else if (Event == EVENT_SOURCE_CHANGE) 
       {
            SettingsPerChannel_SourceChange(pThis, 0, (CSource*)NewValue);
       } 
       else if ((Event == EVENT_VIDEOINPUT_PRECHANGE) || (Event == EVENT_VIDEOINPUT_CHANGE))
       {           
           CSource* pSource;
           try
           {
               pSource = dynamic_cast<CSource*>(pEventObject);
           }
           catch (...)
           {
                pSource = NULL;
           }           
           if(pSource != NULL)
           {
			    SettingsPerChannel_InputAndChannelChange((Event == EVENT_VIDEOINPUT_PRECHANGE), pSource, ((Event == EVENT_VIDEOINPUT_PRECHANGE)?OldValue:NewValue), 
                    pSource->InputHasTuner(VIDEOINPUT,((Event == EVENT_VIDEOINPUT_PRECHANGE)?OldValue:NewValue)), NO_CHANNEL, iSpcCurrentVideoFormat);
           }
		   //}
       } 
       else if ((Event == EVENT_CHANNEL_PRECHANGE) || (Event == EVENT_CHANNEL_CHANGE))
       {
           CSource* pSource;
           try
           {
               pSource = dynamic_cast<CSource*>(pEventObject);
           }
           catch (...)
           {
                pSource = NULL;
           }
           if(pSource != NULL)
           {
    		   SettingsPerChannel_InputAndChannelChange((Event == EVENT_CHANNEL_PRECHANGE), pSource, iSpcCurrentVideoInput, -1, NewValue, iSpcCurrentVideoFormat);
           }
       } 
   } 
   catch (...)
   {
      LOG(1,"Crash in SettingsPerChannel Event handler (Event id = %d)",Event);
   }
}


/////////////////////////////////////////////////////////////////////////////////
// Setup ////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


void SettingsPerChannel_RegisterOnSetup(void* pThis, SETTINGSPERCHANNEL_ONSETUP* pfnSetup)
{
    int i;    
    for (i = 0; i < vSpcOnSetupList.size(); i++)
    { 
        if ((vSpcOnSetupList[i].pfnOnSetup == pfnSetup) && (vSpcOnSetupList[i].pThis == pThis))
        {
            // Already in list
            return;
        }
    }

    TOnSetupFunction OnSetup;

    OnSetup.pThis = pThis;
    OnSetup.pfnOnSetup = pfnSetup;

    vSpcOnSetupList.push_back(OnSetup);
    
}

void SettingsPerChannel_Setup(int Start)
{
    int i; 
    
    LOG(2,"SPC: Setup (%d)",Start);

    bSetupStarted = TRUE;

    if (Start == 1)
    {
        if (Providers_GetCurrentSource() != NULL)
        {
            sSpcCurrentSource = Providers_GetCurrentSource()->IDString();
            if (!bSpcSourceSpecific)
            {
                sSpcCurrentSource = "";
            }
            iSpcCurrentVideoInput = Providers_GetCurrentSource()->GetInput(VIDEOINPUT);
        }
        //Providers_Register_SourceChangeNotification(NULL, SettingsPerChannel_SourceChange);
        eEventType EventList[] = {  EVENT_SOURCE_PRECHANGE,
                                    EVENT_SOURCE_CHANGE,
                                    EVENT_VIDEOINPUT_PRECHANGE,
                                    EVENT_VIDEOINPUT_CHANGE,
                                    EVENT_AUDIOINPUT_PRECHANGE,
                                    EVENT_AUDIOINPUT_CHANGE,
                                    EVENT_VIDEOFORMAT_PRECHANGE,
                                    EVENT_VIDEOFORMAT_CHANGE,
                                    EVENT_CHANNEL_PRECHANGE,
                                    EVENT_CHANNEL_CHANGE,
                                    EVENT_ENDOFLIST};

        EventCollector->Register(SettingsPerChannel_EventHandler,NULL, EventList);
    }
    
    for (i = 0; i < vSpcOnSetupList.size(); i++)
    {    
       vSpcOnSetupList[i].pfnOnSetup(vSpcOnSetupList[i].pThis, Start); 
    }
    if (Start==1)
    {
       SettingsPerChannel_InputAndChannelChange(0,Providers_GetCurrentSource(), iSpcCurrentVideoInput, -1, iSpcCurrentChannel, iSpcCurrentVideoFormat);        
    }
    
    if (Start==0)
    {
       SettingsPerChannel_WriteSettingsToIni(TRUE);

       EventCollector->Unregister(SettingsPerChannel_EventHandler,NULL);
       SettingsPerChannel_ClearAll();

       bSetupStarted = FALSE;
    }
    if (Start==2) //write settings before before source change
    {
       SettingsPerChannel_WriteSettingsToIni(FALSE);
    }
}



/////////////////////////////////////////////////////////////////////////////////
// On/off settings //////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


#define SETTINGSPERCHANNEL_COMMONSETTINGS_LASTONE 4

SETTING SettingsPerChannel_CommonSettings[SETTINGSPERCHANNEL_COMMONSETTINGS_LASTONE] =
{
    {
        "Settings per channel", ONOFF, 0, (long*)&bSpcEnabled, 
        0, 0, 1, 1, 1, NULL,
        "SettingsPerChannel", "SettingsPerChannelEnabled", SettingsPerChannel_OnOff,
    },
    {
        "Save per source", ONOFF, 0, (long*)&bSpcSourceSpecific,
        1, 0, 1, 1, 1, NULL,
        "SettingsPerChannel", "SettingsPerProvider", SettingsPerChannel_SourceSpecific_Change,
    },
    {
        "Save per input", ONOFF, 0, (long*)&bSpcVideoInputSpecific,
        1, 0, 1, 1, 1, NULL,
        "SettingsPerChannel", "SettingsPerVideoInput", SettingsPerChannel_VideoInputSpecific_Change,
    },
    {
        "Save per Format", ONOFF, 0, (long*)&bSpcVideoFormatSpecific,
        1, 0, 1, 1, 1, NULL,
        "SettingsPerChannel", "SettingsPerVideoFormat", SettingsPerChannel_VideoFormatSpecific_Change,
    },
};


int SettingsPerChannel_BuildSettingsArray(SETTING*& SettingsPerChannel_Settings)
{
    if (SettingsPerChannel_Settings != NULL)
    {
        delete[] SettingsPerChannel_Settings;
    }
    SettingsPerChannel_Settings = new SETTING[SETTINGSPERCHANNEL_COMMONSETTINGS_LASTONE +  vSpcChannelSettings.size()];
    
    int i;
    for (i = 0; i < SETTINGSPERCHANNEL_COMMONSETTINGS_LASTONE; i++)
    {
       SettingsPerChannel_Settings[i] = SettingsPerChannel_CommonSettings[i]; 
    }

    for(vector<TChannelSetting*>::iterator it = vSpcChannelSettings.begin();
                it != vSpcChannelSettings.end(); ++it)
    {
        if (!(*it)->ToggleSettingIsLink && ((*it)->ToggleSetting != NULL))
        {
            SettingsPerChannel_Settings[i] = *((*it)->ToggleSetting);
            i++;    
        }
    }
    return i;
}


CTreeSettingsGeneric* SettingsPerChannel_GetTreeSettingsPage()
{    
    int NumSettings = SettingsPerChannel_BuildSettingsArray(SettingsPerChannel_SettingsArray);
    return new CTreeSettingsGeneric("Settings per Channel", SettingsPerChannel_SettingsArray, NumSettings);
}


void SettingsPerChannel_ReadSettingsFromIni()
{
    int i;
    
    // Common settings
    for ( i=0; i<SETTINGSPERCHANNEL_COMMONSETTINGS_LASTONE; i++)
    {       
       Setting_ReadFromIni(&SettingsPerChannel_CommonSettings[i]);
    }    
          
    // Dynamically registered 'on/off' settings.
    for(vector<TChannelSetting*>::iterator it = vSpcChannelSettings.begin();
        it != vSpcChannelSettings.end();
        ++it)
    {        
        if (!(*it)->ToggleSettingIsLink && ((*it)->ToggleSetting != NULL))
        {
            Setting_ReadFromIni((*it)->ToggleSetting);
        }
    }
}

void SettingsPerChannel_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for ( i=0; i<SETTINGSPERCHANNEL_COMMONSETTINGS_LASTONE; i++)
    {       
       Setting_WriteToIni(&SettingsPerChannel_CommonSettings[i], bOptimizeFileAccess);
    }
          
    for(vector<TChannelSetting*>::iterator it = vSpcChannelSettings.begin();
        it != vSpcChannelSettings.end();
        ++it)
    {        
        if (!(*it)->ToggleSettingIsLink && (*it)->ToggleSetting!=NULL)
        {
            Setting_WriteToIni((*it)->ToggleSetting, bOptimizeFileAccess);
        }
    }    
}


SETTING* SettingsPerChannel_GetSetting(int iSetting)
{
    if (iSetting < 0) 
    {
        return NULL;
    }
    if (iSetting <= 0)
    {
        return &(SettingsPerChannel_CommonSettings[iSetting]);
    }

    iSetting--;

    for(vector<TChannelSetting*>::iterator it = vSpcChannelSettings.begin();
        it != vSpcChannelSettings.end();
        ++it)
    {    
        if (!(*it)->ToggleSettingIsLink)
        {
            if (iSetting == 0)
            {
                return (*it)->ToggleSetting;
            } 
            iSetting--;        
        }
    }
    return NULL;
}

