/////////////////////////////////////////////////////////////////////////////
// $Id: SettingsPerChannel.cpp,v 1.8 2002-08-15 14:16:45 kooiman Exp $
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

#define NO_CHANNEL -1
#define NO_VIDEOINPUT -1

// Container for channel specific settings.
typedef struct {
        int  Channel;                   // Channel number
        int  Flag;                      // Channel Flag
                                        // bit 0: Changed
                                        // bit 1: Channel value read 
                                        // bit 2: Channel value was in .ini file
        long Value;                     // Value of setting
        long LastValue;                 // Last read/written value of setting
} TChannelSpecificSetting;

// Source&input info
typedef struct {
        CSource     *pSource;           //Pointer to source class
        std::string sSourceID;          //Source ID     
        int         VideoInput;         //Video input
        int         HasTuner;           //Has tuner
        int         LastChannel;
} TSbcSourceInputInfo;


// Channel settings per source&input
typedef struct {    
    TSbcSourceInputInfo *pInfo;         //Link to source/input info
    
    // Channel specific settings. Size 1 if pInfo->HasTuner if is 0.
    std::vector<TChannelSpecificSetting> vChannels;
} TSourceInputChannelSpecificSetting;

/** 
      Channel settings.

*/
class TChannelSetting {
private:                
        CSimpleSetting *CSSetting;        //Track 2 types of settings
        SETTING        *Setting;          //
        
        long           **pToggleValue;    //Direct link to toggle enabled value
public:        
        // Channel, source & input specific settings
        std::vector<TSourceInputChannelSpecificSetting> vChannelSpecificSettingsList;
        
        BOOL           bInDefaultState;   // Settings are in default state
        long           DefaultStateValue; // Storage for value before going into default state
                
        long           DefaultValue;      // Default value for setting        
        std::string    sSubSection;       // Subsection of setting
              
        long           LastValue;         // Working variable
        int            Flag;              // Working variable 
                                          // Both retreived & stored from channel specific settings        

        SETTING *ToggleSetting;           // Enable/disable setting for GUI & SavePerChannel ini section
        int ToggleSettingIsLink;          // Is link to another GUI setting (for grouping)
private:
        BOOL bEnabled;                    // Storage for ToggleSetting
        std::string sDisplayName;         // ..
        std::string sIniSection;          // ..
        std::string sIniEntry;            // ..        
public:
        // Initialize channel setting with new setting .
        TChannelSetting(long** pTVal = NULL);
        TChannelSetting(CSimpleSetting *cs1, long** pTVal = NULL);
        TChannelSetting(SETTING *s1, long** pTVal = NULL);
        ~TChannelSetting();
  
        // Get & set value for setting
        long GetValue();
        void SetValue(long NewValue, BOOL bNoOnChange = FALSE);
        long GetDefault();
        
        // Load & save channel specific settings
        void LoadChannelSpecifics(std::string SourceID, int VideoInput, int Channel, BOOL bNoOnChange = FALSE);
        void SaveChannelSpecifics(std::string SourceID, int VideoInput, int Channel);

        // Go to default state
        void DefaultState(BOOL bOnOff);

        // Read & write from .ini file
        BOOL ReadFromIni(std::string SourceID, int VideoInput, int Channel, const char *szSection = NULL);
        void WriteToIni(std::string SourceID, int VideoInput, int Channel, BOOL bOptimizeFileAccess);

        // Monitoring enabled
        BOOL Enabled() { return *ToggleSetting->pValue; };
        // Has valid setting
        BOOL Valid() { return ((Setting!=NULL) || (CSSetting!=NULL)); };

        // Start/stop monitoring
        void Start();
        void Stop();
        void Enable();
        void Disable();

        // Compare settings
        BOOL CompareSetting(SETTING *s1) { return (Setting == s1); };
        BOOL CompareSetting(CSimpleSetting *cs1) { return (CSSetting == cs1); };
        BOOL CompareToggleValue(long **pVal) { return (pToggleValue == pVal); };

        BOOL CompareToggleSetting(const char *szName,const char *szDescription);

        // Make GUI Toggle setting
        void MakeToggleSetting(const char *szName,const char *szDescription,BOOL bDefault);
        void LinkToToggleSetting(SETTING *ToggleSetting);
        
};


typedef struct {
    void *pThis;
    SETTINGSPERCHANNEL_ONSETUP *pfnOnSetup;
} TOnSetupFunction;



// Local variables
static BOOL bSbcEnabled = FALSE;                             // Channel monitoring enabled
static BOOL bSbcSourceSpecific = FALSE;                      // Channel monitoring per provider
static BOOL bSbcVideoInputSpecific = TRUE;                   // Channel monitoring per video input

static std::vector<TChannelSetting*> vSbcChannelSettings;    // Main list of settings

static int iSbcCurrentChannel = NO_CHANNEL;                  // Current channel
static int iSbcCurrentVideoInput = NO_VIDEOINPUT;            // Current video input
static std::string sSbcCurrentSource;                        // Current provider
static BOOL bSbcFirstSource = TRUE;
static int iSbcLastLoadedChannel = NO_CHANNEL;               // Last loaded channel specific settings
static BOOL bSetupStarted = FALSE;
static std::string sSbcSubSection;                           // Current subsection for registration
static std::vector<TSbcSourceInputInfo*> vSbcSourceInputInfo;
static std::vector< TOnSetupFunction> vSbcOnSetupList;

// Array of GUI toggle settings (for TreeView settings).
static SETTING* SettingsPerChannel_SettingsArray = NULL;


/// Internal functions
BOOL SettingsPerChannel_ChannelOk(int VideoInput, int Channel);
BOOL SettingsPerChannel_OnOff(long NewValue);
BOOL SettingsPerChannel_SourceSpecific_Change(long NewValue);
BOOL SettingsPerChannel_VideoInputSpecific_Change(long NewValue);

int SettingsPerChannel_RegisterAddSetting(const char *szSubSection, CSimpleSetting *CSSetting, SETTING *Setting, long **pToggleValue);
int SettingsPerChannel_RegisterSetting(const char *szName,const char *szDescription,BOOL bDefault, CSimpleSetting *CSSetting, SETTING *Setting, long **pToggleValue);

BOOL SettingsPerChannel_ReadFromIni(const char *szMainSection, const char *szSubSection, CSimpleSetting *CSSetting);
BOOL SettingsPerChannel_ReadFromIni(const char *szMainSection, const char *szSubSection, SETTING *Setting);
void SettingsPerChannel_WriteToIni(const char *szMainSection, const char *szSubSection, CSimpleSetting *CSSetting, BOOL bOptimizeFileAccess);
void SettingsPerChannel_WriteToIni(const char *szMainSection, const char *szSubSection, SETTING *Setting, BOOL bOptimizeFileAccess);

void SettingsPerChannel_LoadChannelSettings(const char *szSubSection, int Input, int Channel);
void SettingsPerChannel_SaveChannelSettings(const char *szSubSection, int Input, int Channel);

BOOL SettingsPerChannel_HasTunerNotification(CSource *pSource);
BOOL SettingsPerChannel_HasTuner(int VideoInput);

void SettingsPerChannel_InputAndChannelChange(int PreChange, CSource *pSource, int VideoInput, int InputHasTuner, int Channel);

void SettingsPerChannel_SourceChange(void *pThis, int Flags, CSource *pSource);
void SettingsPerChannel_ChannelChange(void *pThis, int PreChange, int OldChannel, int NewChannel);

TSbcSourceInputInfo *SettingsPerChannel_AddSourceAndInputInfo(CSource *pSource, std::string sSourceID, int VideoInput, int HasTuner);
//////////////////////////////////////////////////////////////////////////////////////
// Enable/disable channel setting ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

TChannelSetting::TChannelSetting(CSimpleSetting *cs1, long** pTVal)
{
    CSSetting=cs1;
    Setting = NULL;
    pToggleValue = pTVal;

    ToggleSetting = NULL;
    ToggleSettingIsLink = 0;
}


TChannelSetting::TChannelSetting(SETTING *s1, long** pTVal)
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
    *ToggleSetting->pValue = TRUE;
    Start();
    
}

void TChannelSetting::Disable()
{
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
        Setting_SetValue(Setting, NewValue, bNoOnChange?-1:1);
    }
    else if (CSSetting != NULL)
    {
        CSSetting->SetValue(NewValue, bNoOnChange);
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


//////////////////////////////////////////////////////////////////////////////////////
// Channel specific settings /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

void TChannelSetting::LoadChannelSpecifics(std::string SourceID, int VideoInput, int Channel, BOOL bNoOnChange)
{
    int i;
    int s;    
    for (s = 0; s < vChannelSpecificSettingsList.size(); s++)
    {        
        TSbcSourceInputInfo *pInfo = vChannelSpecificSettingsList[s].pInfo;
        if ( (pInfo->sSourceID == SourceID) && (pInfo->VideoInput == VideoInput) )
        {
            for (i = 0; i<vChannelSpecificSettingsList[s].vChannels.size(); i++)
            {                
                TChannelSpecificSetting *ChannelSpecific = &(vChannelSpecificSettingsList[s].vChannels[i]);
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

void TChannelSetting::SaveChannelSpecifics(std::string sSourceID, int VideoInput, int Channel)
{        
    int i;    
    int s;
    for (s = 0; s < vChannelSpecificSettingsList.size(); s++)
    {
        TSbcSourceInputInfo *pInfo = vChannelSpecificSettingsList[s].pInfo;
        if ( (pInfo->sSourceID == sSourceID) && (pInfo->VideoInput == VideoInput) )
        {
            for (i = 0; i<vChannelSpecificSettingsList[s].vChannels.size(); i++)
            {
                TChannelSpecificSetting *ChannelSpecific = &(vChannelSpecificSettingsList[s].vChannels[i]);
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
    SICSpecific.pInfo = SettingsPerChannel_AddSourceAndInputInfo(NULL, sSourceID, VideoInput, -1);    
    SICSpecific.vChannels.push_back(ChSpecific);
    vChannelSpecificSettingsList.push_back(SICSpecific);    
}



//////////////////////////////////////////////////////////////////////////////////////
// Read & write setting from/to ini file /////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

void SettingsPerChannel_SetIniSection(char *szMainSection, const char *szSourceID, int VideoInput, int Channel)
{
    if ((szSourceID != NULL) && (szSourceID[0] != 0) )
    {                
       sprintf(szMainSection,"Channel_%d_Input_%d_%s",Channel, VideoInput, szSourceID);                    
    }
    else
    {
       sprintf(szMainSection,"Channel_%d_Input_%d", Channel, VideoInput );
    }
}

BOOL TChannelSetting::ReadFromIni(std::string SourceID, int VideoInput, int Channel, const char *szSection)
{
    char *szTheSection = (char*)szSection;
    if (szTheSection == NULL)
    {
        szTheSection = (char*)sSubSection.c_str();
    }
    char szMainSection[200];
    SettingsPerChannel_SetIniSection(szMainSection, SourceID.c_str(), VideoInput, Channel);    
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

void TChannelSetting::WriteToIni(std::string SourceID, int VideoInput, int Channel, BOOL bOptimizeFileAccess)
{
    char szMainSection[200];
    SettingsPerChannel_SetIniSection(szMainSection, SourceID.c_str(), VideoInput, Channel);    
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

void TChannelSetting::MakeToggleSetting(const char *szName,const char *szDescription,BOOL bDefault)
{
    if (ToggleSetting != NULL)
    {
        if (!ToggleSettingIsLink)
        {
            delete ToggleSetting;
            ToggleSetting = NULL;
        }
    }
    ToggleSetting = new SETTING;
    sDisplayName = std::string(szDescription);
    sIniEntry = std::string(szName);
    sIniSection = std::string("SettingsPerChannel");    
    bEnabled = bDefault;
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

void TChannelSetting::LinkToToggleSetting(SETTING *ToggleSetting1)
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

BOOL TChannelSetting::CompareToggleSetting(const char *szName,const char *szDescription)
{
   if ((ToggleSetting == NULL) || (ToggleSettingIsLink) )
   {
      return FALSE;
   }
   return ( (sIniEntry == std::string(szName)) && (sDisplayName == std::string(szDescription)) );
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

BOOL SettingsPerChannel()
{
    return bSbcEnabled;
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
    if (!bSbcEnabled)
    {
        return;
    }
        
    for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
                it != vSbcChannelSettings.end(); ++it)
    {
         (*it)->DefaultState(bTrue);
    }
}

BOOL SettingsPerChannel_OnOff(long NewValue)
{
    bSbcEnabled = NewValue;
    if (bSbcEnabled)
    {
        for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
                it != vSbcChannelSettings.end(); ++it)
        {
            (*it)->Start();
        }
        //Read
        if (SettingsPerChannel_ChannelOk(iSbcCurrentVideoInput, iSbcCurrentChannel))
        {
            SettingsPerChannel_ReadSettings(NULL, iSbcCurrentVideoInput, iSbcCurrentChannel, 1);
        }
    } 
    else 
    {
        if (SettingsPerChannel_ChannelOk(iSbcCurrentVideoInput, iSbcCurrentChannel))
        {
            SettingsPerChannel_SaveChannelSettings(NULL, iSbcCurrentVideoInput, iSbcCurrentChannel);
        }
        for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
                it != vSbcChannelSettings.end(); ++it)            
        {
            (*it)->Stop();
        }        
    } 
       
    return FALSE;
}


BOOL SettingsPerChannel_SourceSpecific_Change(long NewValue)
{
    bSbcSourceSpecific = NewValue;
    // Save settings    
    SettingsPerChannel_SourceChange(NULL, SOURCECHANGE_PROVIDER | SOURCECHANGE_PRECHANGE, Providers_GetCurrentSource());
    // Load settings
    SettingsPerChannel_SourceChange(NULL, SOURCECHANGE_PROVIDER, Providers_GetCurrentSource());
    return FALSE;
}

BOOL SettingsPerChannel_VideoInputSpecific_Change(long NewValue)
{
    bSbcVideoInputSpecific = NewValue;

    if (!SettingsPerChannel())
    {
        return FALSE;
    }

    // save
    SettingsPerChannel_InputAndChannelChange(1, Providers_GetCurrentSource(), iSbcCurrentVideoInput, -1, iSbcCurrentChannel);

    if (bSbcVideoInputSpecific)
    {
      // load defaults
      if (bSetupStarted && SettingsPerChannel_ChannelOk(iSbcCurrentVideoInput, iSbcCurrentChannel))
      {                          
          SettingsPerChannel_ReadSettings(NULL,iSbcCurrentVideoInput,iSbcCurrentChannel,-1); 
      }
    } 
    else
    {
       // load
       SettingsPerChannel_InputAndChannelChange(0, Providers_GetCurrentSource(), iSbcCurrentVideoInput, -1, iSbcCurrentChannel);
    }

    return FALSE;
}


void SettingsPerChannel_ClearAll()
{
    if (SettingsPerChannel_SettingsArray != NULL)
    {
        delete[] SettingsPerChannel_SettingsArray;
        SettingsPerChannel_SettingsArray = NULL;
    }

    //Clear list
    if (vSbcChannelSettings.size()>0) 
    {                               
        int n = 0;
        for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
                it != vSbcChannelSettings.end(); ++it)
        {            
            if ( (it != NULL) && ((*it) != NULL) )
            {
                delete (*it);
            }
            n++;
        }
        vSbcChannelSettings.clear();
    }
    //clear source info list
    int i;
    for (i = 0; i < vSbcSourceInputInfo.size(); i++)
    {
         delete vSbcSourceInputInfo[i];
    }
    vSbcSourceInputInfo.clear();
}


/////////////////////////////////////////////////////////////////////////////////
// Register setting /////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

// Create new channel setting
int SettingsPerChannel_RegisterAddSetting(const char *szSubSection, CSimpleSetting *CSSetting, SETTING *Setting, long **pToggleValue)
{
    int n = 0;
    for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
        it != vSbcChannelSettings.end();
        ++it)
    {                
        if  (    ( ( (Setting != NULL) && ((*it)->CompareSetting(Setting)) ) 
                || ( (CSSetting != NULL) && ((*it)->CompareSetting(CSSetting)) ) 
                || ( (pToggleValue != NULL) && ((*it)->CompareToggleValue(pToggleValue) ) ) )
              && ( (szSubSection==NULL) || (std::string(szSubSection) == (*it)->sSubSection) ) )
        {
            //setting already exists
            return -n;
        }
        n++;
    }    
    
    // Add new
    TChannelSetting *ChannelSetting;
    
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

    vSbcChannelSettings.push_back(ChannelSetting);
    return n;
}

//
int SettingsPerChannel_RegisterSetting(const char *szName,const char *szDescription,BOOL bDefault, CSimpleSetting *CSSetting, SETTING *Setting, long **pToggleValue)
{    
    int n = SettingsPerChannel_RegisterAddSetting(sSbcSubSection.c_str(), CSSetting, Setting, pToggleValue);
    TChannelSetting *ChannelSetting;
    if (n < 0)
    {
        n = -n;
        ChannelSetting = vSbcChannelSettings[n];
        if ( ChannelSetting == NULL )
        {
            return -1;
        }
        if (ChannelSetting->CompareToggleSetting (szName, szDescription))
        {
            // Duplicate
            return -1;
        }
    }
    ChannelSetting = vSbcChannelSettings[n];
    if ( ChannelSetting == NULL )
    {
        return -1;
    }

    // Check if toggle settting already exists

    BOOL bToggleSettingExists = FALSE;
    for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
        it != vSbcChannelSettings.end();
        ++it)
    {
        if ((*it)->CompareToggleSetting (szName, szDescription))        
        {                    
            ChannelSetting->LinkToToggleSetting((*it)->ToggleSetting);
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
int SettingsPerChannel_RegisterSetting(const char *szName,const char *szDescription,BOOL bDefault, SETTING *Setting)
{
    return SettingsPerChannel_RegisterSetting(szName,szDescription,bDefault,NULL, Setting,NULL);
}

int SettingsPerChannel_RegisterSetting(const char *szName,const char *szDescription,BOOL bDefault, CSimpleSetting *CSSetting)
{
    return SettingsPerChannel_RegisterSetting(szName,szDescription,bDefault,CSSetting, NULL,NULL);
}

int SettingsPerChannel_RegisterSetting(const char *szName, const char *szDescription,BOOL bDefault, long** pValue)
{    
    return SettingsPerChannel_RegisterSetting(szName, szDescription, bDefault, NULL,NULL,pValue);
}

int SettingsPerChannel_RegisterSetting(const char *szName, const char *szDescription,BOOL bDefault)
{    
    return SettingsPerChannel_RegisterSetting(szName, szDescription, bDefault, NULL,NULL,NULL);
}



void SettingsPerChannel_RegisterSetSection(const char *szSubSection)
{        
    sSbcSubSection = szSubSection;
}

void SettingsPerChannel_UnregisterSection(const char *szSubSection)
{
    std::vector<TChannelSetting*> newSettings;

    for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
                it != vSbcChannelSettings.end(); ++it)
    {
        if ((*it)->sSubSection == std::string(szSubSection))
        {
            if (!(*it)->ToggleSettingIsLink)
            {
               if ((*it)->ToggleSetting != NULL)
               {
                  delete (*it)->ToggleSetting;
                  (*it)->ToggleSetting = NULL;
               }
            }
            delete (*it);
        }
        else
        {
          newSettings.push_back((*it));
        }
    }           
    vSbcChannelSettings = newSettings;
}

 

/////////////////////////////////////////////////////////////////////////////////
// Read & write channel settings ////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

void SettingsPerChannel_WriteToIni(const char *szMainSection, const char *szSubSection, CSimpleSetting *CSSetting, BOOL bOptimizeFileAccess)
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

void SettingsPerChannel_WriteToIni(const char *szMainSection, const char *szSubSection, SETTING *Setting, BOOL bOptimizeFileAccess)
{
    char *szIniSection = Setting->szIniSection;        
    char *szIniEntry = Setting->szIniEntry;
    std::string sNewEntry = szSubSection + std::string("_") + szIniEntry;

    Setting->szIniSection = (char*)szMainSection;
    Setting->szIniEntry = (char*)sNewEntry.c_str();
    Setting_WriteToIni(Setting,bOptimizeFileAccess);                    
    Setting->szIniSection = szIniSection;
    Setting->szIniEntry = szIniEntry;    
}

BOOL SettingsPerChannel_ReadFromIni(const char *szMainSection, const char *szSubSection, CSimpleSetting *CSSetting)
{
    BOOL result = FALSE;
    char *szNewSection;

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

BOOL SettingsPerChannel_ReadFromIni(const char *szMainSection, const char *szSubSection, SETTING *Setting)
{
    BOOL result = FALSE;
    char *szNewSection;

    if (szMainSection != NULL)
    {                        
        szNewSection = (char*)szMainSection;
    }
    else
    {
        szNewSection = (char*)szSubSection;
    }
    
    char *szIniSection = Setting->szIniSection;        
    char *szIniEntry = Setting->szIniEntry;

    std::string sNewEntry = szSubSection + std::string("_") + szIniEntry;

    Setting->szIniSection = szNewSection;
    Setting->szIniEntry = (char*)sNewEntry.c_str();

    result = Setting_ReadFromIni(Setting, TRUE);
        
    Setting->szIniSection = szIniSection;
    Setting->szIniEntry = szIniEntry;
    
    return result;
}




void SettingsPerChannel_ReadSettings(const char *szSubSection, int VideoInput, int Channel, int DefaultValuesFirst)
{
    LOG(2,"SBC: Read settings for input %d, channel %d.",VideoInput,Channel);
    if (!SettingsPerChannel() || !SettingsPerChannel_ChannelOk(VideoInput, Channel))
    {
        return;
    }

    for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
        it != vSbcChannelSettings.end();
        ++it)
    {        
        TChannelSetting *CSetting = (*it);
        
        // If setting enabled, setting is available and in right subsection
        if ( CSetting->Enabled()  && CSetting->Valid()
             && ( (szSubSection == NULL) || (CSetting->sSubSection == std::string(szSubSection)) ) )
        {            
            int OldValue = CSetting->GetValue();

            if (iSbcLastLoadedChannel != Channel)
            {
                LOG(2,"SBC: Read settings: Load channel specifics");                
                CSetting->LoadChannelSpecifics(sSbcCurrentSource, VideoInput, Channel, TRUE);                
            }

            int NewDefaultValue = CSetting->DefaultValue;
            int NewValue = CSetting->GetValue();

            
            // Read value from current channel section
            if (DefaultValuesFirst>=0)
            {                                
                if (CSetting->Flag&4)
                {
                    // Already read                    
                }
                else 
                {
                    if (CSetting->ReadFromIni(sSbcCurrentSource, VideoInput, Channel))
                    {
                        NewValue = CSetting->GetValue();
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
                CSetting->SetValue(NewValue);
            }
        }
    }
    iSbcLastLoadedChannel = Channel;
}

void SettingsPerChannel_ReadDefaults(const char *szSubSection, int Input, int Channel)
{
    SettingsPerChannel_ReadSettings(szSubSection, Input, Channel, -1);
}


void SettingsPerChannel_WriteSettings(BOOL bOptimizeFileAccess)
{
    
    // Save current channel settings (if enabled)
    if (SettingsPerChannel() && SettingsPerChannel_ChannelOk(iSbcCurrentVideoInput, iSbcCurrentChannel))
    {
        for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
            it != vSbcChannelSettings.end();
            ++it)
        {        
            TChannelSetting *CSetting = (*it);
            if ( CSetting->Enabled() && CSetting->Valid() )          
            {        
                CSetting->SaveChannelSpecifics(sSbcCurrentSource,iSbcCurrentVideoInput, iSbcCurrentChannel);              
            }
        }              
    }
    
    // Write all changed settings
    for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
        it != vSbcChannelSettings.end();
        ++it)
    {                
        TChannelSetting *CSetting = (*it);

        // If setting enabled, and in right subsection
        if ( CSetting->Enabled() && CSetting->Valid() )
             //&& ( (szSubSection == NULL) || (CSetting->sSubSection == std::string(szSubSection)) ) )
        {        
            int OldValue = CSetting->GetValue();
            int s;
            int i;
            for (s = 0; s < CSetting->vChannelSpecificSettingsList.size(); s++)
            {
                TSbcSourceInputInfo *pInfo = CSetting->vChannelSpecificSettingsList[s].pInfo;
                //if ( (pInfo->sSourceID == sSourceID) && (pInfo->VideoInput == VideoInput) )                
                {
                    for (i = 0; i<CSetting->vChannelSpecificSettingsList[s].vChannels.size(); i++)
                    {
                        TChannelSpecificSetting *ChSpecific = &(CSetting->vChannelSpecificSettingsList[s].vChannels[i]);
                        
                        //if ((Channel == NO_CHANNEL) || (Channel == ChSpecific->Channel))
                        {            
                            // Write of write bit of Flag is set, or if value has changed
                            if ( (ChSpecific->Flag&1) || (ChSpecific->Value != ChSpecific->LastValue) )
                            {
                                if (ChSpecific->Value != CSetting->GetValue()) 
                                {
                                    CSetting->SetValue(ChSpecific->Value, TRUE);
                                }
                                CSetting->WriteToIni(pInfo->sSourceID, pInfo->VideoInput, ChSpecific->Channel, FALSE);
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


void SettingsPerChannel_ClearSettings(const char *szSubSection, int VideoInput, int Channel, int ClearIniSection) //, std::vector<std::string> *vsDefaultSections)
{
    if (VideoInput == -2)
    {
        VideoInput = iSbcCurrentVideoInput;
    }
    if (Channel == -2)
    {
        Channel = iSbcCurrentChannel;
    }
    SettingsPerChannel_ReadSettings(szSubSection,VideoInput,Channel,2); //,vsDefaultSections);
    if (ClearIniSection)
    {
       
    }
    //SettingsPerChannel_WriteSettings(szSubSection, Channel, TRUE);      
    SettingsPerChannel_SaveChannelSettings(szSubSection,VideoInput, Channel);
}


/////////////////////////////////////////////////////////////////////////////////
// Load/Save channel settings ///////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


void SettingsPerChannel_LoadChannelSettings(const char *szSubSection, int VideoInput, int Channel)
{
    if (!SettingsPerChannel())
    {
        return;
    }

    if (!SettingsPerChannel_ChannelOk(VideoInput, Channel))
    {
        return;
    }

    for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
        it != vSbcChannelSettings.end();
        ++it)
    {        
        
        TChannelSetting *CSetting = (*it);
        // If setting enabled, and in right subsection
        if ( CSetting->Enabled() && CSetting->Valid() 
             && ( (szSubSection == NULL) || (CSetting->sSubSection == std::string(szSubSection)) ) )
        {        
            CSetting->LoadChannelSpecifics(sSbcCurrentSource,VideoInput, Channel);
        }        

    }        

    iSbcLastLoadedChannel = Channel;    
}

void SettingsPerChannel_SaveChannelSettings(const char *szSubSection, int VideoInput, int Channel)
{
    for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
        it != vSbcChannelSettings.end();
        ++it)
    {        
        
        TChannelSetting *CSetting = (*it);
        // If setting enabled, and in right subsection
        if ( CSetting->Enabled() && CSetting->Valid() 
             && ( (szSubSection == NULL) || (CSetting->sSubSection == std::string(szSubSection)) ) )
        {        
            CSetting->SaveChannelSpecifics(sSbcCurrentSource, VideoInput, Channel);
        }        

    }        
}

/////////////////////////////////////////////////////////////////////////////////
// Tuner notification ///////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

//
   
BOOL SettingsPerChannel_ChannelOk(int VideoInput, int Channel)
{
    if (Providers_GetCurrentSource() == NULL) 
    {
        return FALSE;
    }
    
    int i;
    for (i = 0; i < vSbcSourceInputInfo.size(); i++)
    {
        if (   (vSbcSourceInputInfo[i]->pSource == Providers_GetCurrentSource()) 
            && (vSbcSourceInputInfo[i]->VideoInput == VideoInput) )
        {
            if (vSbcSourceInputInfo[i]->HasTuner == 0)
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


TSbcSourceInputInfo *SettingsPerChannel_AddSourceAndInputInfo(CSource *pSource, std::string sSourceID, int VideoInput, int HasTuner)
{    
    int i;
    if (pSource != NULL)
    {
        sSourceID = pSource->IDString();
    }
    for (i = 0; i < vSbcSourceInputInfo.size(); i++)
    {
        if (   (vSbcSourceInputInfo[i]->sSourceID == sSourceID) 
            && (vSbcSourceInputInfo[i]->VideoInput == VideoInput) )
        {
            if (HasTuner >= 0)
            {
                vSbcSourceInputInfo[i]->HasTuner = HasTuner;
            }   
            return (vSbcSourceInputInfo[i]);
        }
    }

   
   // New source & input info
   TSbcSourceInputInfo *SSIInfo = new TSbcSourceInputInfo;

   SSIInfo->pSource = pSource;
   SSIInfo->sSourceID = sSourceID;
   SSIInfo->VideoInput = VideoInput;
   SSIInfo->HasTuner = HasTuner;
   SSIInfo->LastChannel = NO_CHANNEL;
   if (HasTuner >= 0)
   {
       SSIInfo->LastChannel = 0;
   }      

   i = vSbcSourceInputInfo.size();
   vSbcSourceInputInfo.push_back(SSIInfo);

   return (vSbcSourceInputInfo[i]);
}

/////////////////////////////////////////////////////////////////////////////////
// Channel/source change ////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

void SettingsPerChannel_InputAndChannelChange(int PreChange, CSource *pSource, int VideoInput, int InputHasTuner, int Channel)
{      
   if (pSource == NULL)
   {
        // No provider
        return;
   }

   if (PreChange)
   {            
      // Save settings if input & channel are valid
      if (bSetupStarted && SettingsPerChannel() && SettingsPerChannel_ChannelOk(iSbcCurrentVideoInput, iSbcCurrentChannel)
          && (bSbcVideoInputSpecific || (InputHasTuner==1)) )
      {                    
          LOG(2,"SBC: Change input/channel: save settings for in=%d, ch=%d (%s)", iSbcCurrentVideoInput, iSbcCurrentChannel, sSbcCurrentSource.c_str());
          for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
            it != vSbcChannelSettings.end();
            ++it)
          {        
              TChannelSetting *CSetting = (*it);
              if ( CSetting->Enabled() && CSetting->Valid() )          
              {        
                 CSetting->SaveChannelSpecifics(sSbcCurrentSource,iSbcCurrentVideoInput, iSbcCurrentChannel);
              }
          }  
      }            
   }
   else
   {
      BOOL bVideoInputMayLoadSettings = FALSE;
      if (iSbcCurrentVideoInput != VideoInput)
      {
         //changed
         TSbcSourceInputInfo *pInfo;
         int HadTuner = -1;
         if ( (iSbcCurrentVideoInput != NO_CHANNEL) || (iSbcCurrentVideoInput != NO_VIDEOINPUT) )
         {
            pInfo = SettingsPerChannel_AddSourceAndInputInfo(pSource, pSource->IDString(), iSbcCurrentVideoInput, -1);
            pInfo->LastChannel = iSbcCurrentChannel;
            HadTuner = pInfo->HasTuner;
         }
         iSbcLastLoadedChannel = NO_CHANNEL;

         if (Channel == NO_CHANNEL)
         {
             // get last one
             pInfo = SettingsPerChannel_AddSourceAndInputInfo(pSource, pSource->IDString(), VideoInput, InputHasTuner);
             Channel = pInfo->LastChannel;
             if (!bSbcVideoInputSpecific && (HadTuner != pInfo->HasTuner) && (pInfo->HasTuner == 0))
             {                
                // Left tuner mode
                bVideoInputMayLoadSettings = TRUE;
             }
         }
         iSbcCurrentVideoInput = VideoInput;
      }
          
      if ( (iSbcCurrentChannel != Channel) && (Channel != NO_CHANNEL) )
      {
          //changed
         iSbcLastLoadedChannel = NO_CHANNEL; 
         iSbcCurrentChannel = Channel;
      }
      

      if (bSetupStarted && SettingsPerChannel() && SettingsPerChannel_ChannelOk(iSbcCurrentVideoInput, iSbcCurrentChannel)
          && (bSbcVideoInputSpecific || bVideoInputMayLoadSettings || pSource->IsInTunerMode()) )
      {                    
          int DefaultSettings = 1;
          if (bVideoInputMayLoadSettings)
          {
              // Just left tuner mode, load default settings
              DefaultSettings = -1;
          }
          SettingsPerChannel_ReadSettings(NULL,iSbcCurrentVideoInput,iSbcCurrentChannel,DefaultSettings); 
      }
   }   
}


void SettingsPerChannel_SourceChange(void *pThis, int Flags, CSource *pSource)
{    
    LOG(2,"SBC: Source change %i (%s)",Flags,(pSource==NULL)?"NULL":pSource->IDString());
    if (Flags&SOURCECHANGE_PRECHANGE)
    {
         SettingsPerChannel_Setup(2);

         // before change
         if (!SettingsPerChannel() || !SettingsPerChannel_ChannelOk(iSbcCurrentVideoInput, iSbcCurrentChannel))
         {
            return;
         }
         LOG(2,"Change source: save channel settings");
         SettingsPerChannel_SaveChannelSettings(NULL, iSbcCurrentVideoInput, iSbcCurrentChannel);         
         iSbcLastLoadedChannel = NO_CHANNEL; // force read of new channel values
    }
    else
    {        
        // after change
        std::string sLastSource = sSbcCurrentSource;
        BOOL bChanged = FALSE;
        if (Flags & SOURCECHANGE_PROVIDER)
        {            
            if (pSource == NULL)
            {
                sSbcCurrentSource = "";
            } 
            else
            {
                sSbcCurrentSource = pSource->IDString();
            }
            if (bSbcFirstSource && (sSbcCurrentSource != sLastSource))
            {                
                bChanged = TRUE;
            }
            bSbcFirstSource = FALSE;
            if (!bSbcSourceSpecific)
            {
                sSbcCurrentSource = "";
            }
        }
        if (!SettingsPerChannel()) 
        {            
            return;
        }

        if (bChanged && !bSbcSourceSpecific)
        {
            LOG(2,"SBC: Source change: reset channel specific settings");
            //reset channels & inputs
            for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
                it != vSbcChannelSettings.end();
                ++it)
            {
                (*it)->vChannelSpecificSettingsList.clear();                
            }
            //clear source info list
            int i; 
            for (i = 0; i < vSbcSourceInputInfo.size(); i++)
            {
                delete vSbcSourceInputInfo[i];
            }
            vSbcSourceInputInfo.clear();
        }
        if (bChanged)
        {
            iSbcCurrentVideoInput = NO_VIDEOINPUT;
            iSbcCurrentChannel = NO_CHANNEL;
            iSbcLastLoadedChannel = NO_CHANNEL;
            SettingsPerChannel_Setup(3);
        }

        if (SettingsPerChannel_ChannelOk(iSbcCurrentVideoInput, iSbcCurrentChannel))
        {            
            SettingsPerChannel_ReadSettings(NULL,iSbcCurrentVideoInput,iSbcCurrentChannel,1);      
        }
    }
}


void SettingsPerChannel_ChannelChange(void *pThis, int PreChange, int OldChannel, int NewChannel)
{
   try 
   {
      SettingsPerChannel_InputAndChannelChange(PreChange, Providers_GetCurrentSource(), iSbcCurrentVideoInput, -1, NewChannel);
   } 
   catch (...)
   {
      LOG(1,"Crash in SettingsPerChannel_ChannelChange");
   }
}

void SettingsPerChannel_VideoInputChange(CSource *pSource, int PreChange, int Input, int IsTuner)
{   
   if (Input < -1)
   {
      return;
   }
   try 
   {
      SettingsPerChannel_InputAndChannelChange(PreChange, pSource, Input, IsTuner, NO_CHANNEL);
   } 
   catch (...)
   {
      LOG(1,"Crash in SettingsPerChannel_VideoInputChange");
   }
}

/////////////////////////////////////////////////////////////////////////////////
// Setup ////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


void SettingsPerChannel_RegisterOnSetup(void *pThis, SETTINGSPERCHANNEL_ONSETUP* pfnSetup)
{
    int i;    
    for (i = 0; i < vSbcOnSetupList.size(); i++)
    { 
        if ((vSbcOnSetupList[i].pfnOnSetup == pfnSetup) && (vSbcOnSetupList[i].pThis == pThis))
        {
            // Already in list
            return;
        }
    }

    TOnSetupFunction OnSetup;

    OnSetup.pThis = pThis;
    OnSetup.pfnOnSetup = pfnSetup;

    vSbcOnSetupList.push_back(OnSetup);
    
}

void SettingsPerChannel_Setup(int Start)
{
    int i;    

    bSetupStarted = TRUE;

    if (Start == 1)
    {
        if (Providers_GetCurrentSource() != NULL)
        {
            sSbcCurrentSource = Providers_GetCurrentSource()->IDString();
            if (!bSbcSourceSpecific)
            {
                sSbcCurrentSource = "";
            }
        }
        Providers_Register_SourceChangeNotification(NULL, SettingsPerChannel_SourceChange);
    }
    
    for (i = 0; i < vSbcOnSetupList.size(); i++)
    {    
       vSbcOnSetupList[i].pfnOnSetup(vSbcOnSetupList[i].pThis, Start); 
    }
    if (Start==1)
    {
       Channel_Register_Change_Notification(NULL, SettingsPerChannel_ChannelChange);       
       SettingsPerChannel_InputAndChannelChange(0,Providers_GetCurrentSource(), iSbcCurrentVideoInput, -1, iSbcCurrentChannel);        
    }
    if (Start==0)
    {
       Providers_Unregister_SourceChangeNotification(NULL, SettingsPerChannel_SourceChange);

       Channel_UnRegister_Change_Notification(NULL, SettingsPerChannel_ChannelChange);
       SettingsPerChannel_ClearAll();

       bSetupStarted = FALSE;
    }
}



/////////////////////////////////////////////////////////////////////////////////
// On/off settings //////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


#define SETTINGSPERCHANNEL_COMMONSETTINGS_LASTONE 3

SETTING SettingsPerChannel_CommonSettings[SETTINGSPERCHANNEL_COMMONSETTINGS_LASTONE] =
{
    {
        "Settings per channel", ONOFF, 0, (long*)&bSbcEnabled, 
        0, 0, 1, 1, 1, NULL,
        "SettingsPerChannel", "SettingsPerChannelEnabled", SettingsPerChannel_OnOff,
    },
    {
        "Save per source", ONOFF, 0, (long*)&bSbcSourceSpecific,
        0, 0, 1, 1, 1, NULL,
        "SettingsPerChannel", "SettingsPerProvider", SettingsPerChannel_SourceSpecific_Change,
    },
    {
        "Save per input", ONOFF, 0, (long*)&bSbcVideoInputSpecific,
        0, 0, 1, 1, 1, NULL,
        "SettingsPerChannel", "SettingsPerVideoInput", SettingsPerChannel_VideoInputSpecific_Change,
    },
};


int SettingsPerChannel_BuildSettingsArray(SETTING* &SettingsPerChannel_Settings)
{
    if (SettingsPerChannel_Settings != NULL)
    {
        delete[] SettingsPerChannel_Settings;

    }
    SettingsPerChannel_Settings = new SETTING[SETTINGSPERCHANNEL_COMMONSETTINGS_LASTONE +  vSbcChannelSettings.size()];
    int i;
    for ( i=0; i<SETTINGSPERCHANNEL_COMMONSETTINGS_LASTONE; i++)
    {
       SettingsPerChannel_Settings[i] = SettingsPerChannel_CommonSettings[i]; 
    }
    for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
                it != vSbcChannelSettings.end(); ++it)
    {
        if (!(*it)->ToggleSettingIsLink)
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
    for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
        it != vSbcChannelSettings.end();
        ++it)
    {        
        if (!(*it)->ToggleSettingIsLink)
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
          
    for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
        it != vSbcChannelSettings.end();
        ++it)
    {        
        if (!(*it)->ToggleSettingIsLink)
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

    for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
        it != vSbcChannelSettings.end();
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

