#include "stdafx.h"
#include <stack>
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "setting.h"
#include "settings.h"
#include "providers.h"
#include "DebugLog.h"
#include "ProgramList.h"
#include "SettingsPerChannel.h"

/// Channel setting structures
 
typedef struct {
        int  Channel;
        int  Flag;                      //Channel Flag
                                        //bit 0: Changed
                                        //bit 1: channel value read        
        long Value;
        long LastValue;
} TChannelSpecificSetting;

class TChannelSetting {
private:
        CSimpleSetting *CSSetting;      //C++ style setting
        SETTING        *Setting;        //C style setting
        long           **pToggleValue;  //Direct link to toggle enabled                        
public:        
        std::vector<TChannelSpecificSetting*> vChannelSpecificSettings;// Values for channels        
        BOOL           bInDefaultState;
        long           DefaultStateValue; 
        // Common
        long           DefaultValue;    //Default value
        int            CommonFlag;      //bit 2: default value read
        std::string    sSubSection;     //Subsection of setting
        // Current channel        
        long           LastValue;
        int            Flag;
        //
        SETTING *ToggleSetting;         //Enable/disable setting for GUI & SaverPerChannel ini section
        int ToggleSettingIsLink;        //Link to another GUI setting (for grouping)
private:
        BOOL bEnabled;                  //Setting monitoring enabled
        std::string sDisplayName;       // Storage for ToggleSetting
        std::string sIniSection;        // ..
        std::string sIniEntry;          // ..        
public:
        TChannelSetting(long** pTVal = NULL) { CSSetting=NULL; Setting=NULL; pToggleValue=pTVal; }
        TChannelSetting(CSimpleSetting *cs1, long** pTVal = NULL) { CSSetting=cs1; Setting=NULL; pToggleValue=pTVal; }
        TChannelSetting(SETTING *s1, long** pTVal = NULL) { CSSetting=NULL; Setting=s1; pToggleValue=pTVal; }
  
        long GetValue();
        void SetValue(long NewValue, BOOL bNoOnChange = FALSE);
        long GetDefault();
        
        int  ChannelIndex(int Channel, BOOL bNewIfNotExist = FALSE);

        void LoadChannelSpecifics(int Channel, BOOL bNoOnChange = FALSE);
        void SaveChannelSpecifics(int Channel);

        void DefaultState(BOOL bOnOff);

        BOOL ReadFromIni(int Channel, const char *szSection = NULL);
        void WriteToIni(int Channel, BOOL bOptimizeFileAccess);

        BOOL Enabled() { return *ToggleSetting->pValue; };
        BOOL Valid() { return ((Setting!=NULL) || (CSSetting!=NULL)); };

        void Start();
        void Stop();
        void Enable();
        void Disable();

        BOOL CompareSetting(SETTING *s1) { return (Setting == s1); };
        BOOL CompareSetting(CSimpleSetting *cs1) { return (CSSetting == cs1); };
        BOOL CompareToggleValue(long **pVal) { return (pToggleValue == pVal); };

        BOOL CompareToggleSetting(const char *szName,const char *szDescription);

        void MakeToggleSetting(const char *szName,const char *szDescription,BOOL bDefault);
        void LinkToToggleSetting(SETTING *ToggleSetting);
        
};

// Local variables

static std::vector<TChannelSetting*> vSbcChannelSettings;    //List of settings
static BOOL bSbcEnabled = FALSE;
static std::string sSbcSubSection;
static int iSbcLastChannel = -1;
static int iSbcLastLoadedChannel = -1;
static std::vector< SETTINGSPERCHANNEL_ONSETUP* > vSbcOnSetupList;
static std::vector< void* > vSbcOnSetupThisList;

/// Internal functions

BOOL SettingsPerChannel_OnOff(long NewValue);
TChannelSetting *SettingsPerChannel_RegisterAddSetting(const char *szSubSection, CSimpleSetting *CSSetting, SETTING *Setting, long **pToggleValue);
int SettingsPerChannel_RegisterSetting(const char *szName,const char *szDescription,BOOL bDefault, CSimpleSetting *CSSetting, SETTING *Setting, long **pToggleValue);

BOOL SettingsPerChannel_ReadFromIni(int Channel, const char *szSubSection, CSimpleSetting *CSSetting);
BOOL SettingsPerChannel_ReadFromIni(int Channel, const char *szSubSection, SETTING *Setting);
void SettingsPerChannel_WriteToIni(int Channel, const char *szSubSection, CSimpleSetting *CSSetting, BOOL bOptimizeFileAccess);
void SettingsPerChannel_WriteToIni(int Channel, const char *szSubSection, SETTING *Setting, BOOL bOptimizeFileAccess);

void SettingsPerChannel_LoadChannelSettings(const char *szSubSection, int Channel);
void SettingsPerChannel_SaveChannelSettings(const char *szSubSection, int Channel);

//////////////////////////////////////////////////////////////////////////////////////
// Enable/disable channel setting ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////


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

int TChannelSetting::ChannelIndex(int Channel, BOOL bNewIfNotExist)
{
    int i;
    for (i = 0; i<vChannelSpecificSettings.size(); i++)
    {
        if (vChannelSpecificSettings[i]->Channel == Channel)
        {
            return i;
        }
    }
    if (bNewIfNotExist)
    {
      TChannelSpecificSetting *ChSpecific = new TChannelSpecificSetting;
      ChSpecific->Channel = Channel;
      ChSpecific->Flag    = 0;
      ChSpecific->Value   = DefaultValue;
      ChSpecific->LastValue = DefaultValue;
      vChannelSpecificSettings.push_back(ChSpecific);      
      return vChannelSpecificSettings.size()-1;
    }
    return -1;
}

void TChannelSetting::LoadChannelSpecifics(int Channel, BOOL bNoOnChange)
{
    int i;
    for (i = 0; i<vChannelSpecificSettings.size(); i++)
    {
        if (vChannelSpecificSettings[i]->Channel == Channel)
        {
            Flag = vChannelSpecificSettings[i]->Flag;    
            LastValue = vChannelSpecificSettings[i]->LastValue;
            if (GetValue() != vChannelSpecificSettings[i]->Value)
            {
                SetValue(vChannelSpecificSettings[i]->Value, bNoOnChange);
            }
            return;
        }
    }
    Flag    = 0;
    if (GetValue() != DefaultValue)
    {
        SetValue(DefaultValue, bNoOnChange);
    }
    LastValue = DefaultValue;
}

void TChannelSetting::SaveChannelSpecifics(int Channel)
{        
    int i;    
    for (i = 0; i<vChannelSpecificSettings.size(); i++)
    {
        if (vChannelSpecificSettings[i]->Channel == Channel)
        {
            vChannelSpecificSettings[i]->Channel = Channel;
            vChannelSpecificSettings[i]->Value = GetValue();
            vChannelSpecificSettings[i]->Flag = Flag;
            vChannelSpecificSettings[i]->LastValue = LastValue;
            return;
        }
    }    
    TChannelSpecificSetting *ChSpecific = new TChannelSpecificSetting;
    ChSpecific->Channel = Channel;
    ChSpecific->Value = GetValue();
    ChSpecific->Flag = Flag;
    ChSpecific->LastValue = LastValue;

    vChannelSpecificSettings.push_back(ChSpecific);
}

//////////////////////////////////////////////////////////////////////////////////////
// Read & write setting from/to ini file /////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

BOOL TChannelSetting::ReadFromIni(int Channel, const char *szSection)
{
    char *szTheSection = (char*)szSection;
    if (szTheSection == NULL)
    {
        szTheSection = (char*)sSubSection.c_str();
    }
    if (Setting != NULL)
    {
        return SettingsPerChannel_ReadFromIni(Channel, szTheSection , Setting);
    }
    else if (CSSetting != NULL)
    {
        return SettingsPerChannel_ReadFromIni(Channel, szTheSection, CSSetting);
    }
    return FALSE;
}

void TChannelSetting::WriteToIni(int Channel, BOOL bOptimizeFileAccess)
{
    if (Setting != NULL)
    {
        SettingsPerChannel_WriteToIni(Channel, sSubSection.c_str(), Setting, bOptimizeFileAccess);
    }
    else if (CSSetting != NULL)
    {
        SettingsPerChannel_WriteToIni(Channel, sSubSection.c_str(), CSSetting, bOptimizeFileAccess);
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

//
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
        if (iSbcLastChannel >= 0)
        {
            SettingsPerChannel_ReadSettings(NULL, iSbcLastChannel, 1);
        }
    } 
    else 
    {
        if (iSbcLastChannel >= 0)
        {
            SettingsPerChannel_SaveChannelSettings(NULL, iSbcLastChannel);
        }
        for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
                it != vSbcChannelSettings.end(); ++it)            
        {
            (*it)->Stop();
        }        
    } 
       
    return FALSE;
}



void SettingsPerChannel_ClearAll()
{
    //Clear list
    if (vSbcChannelSettings.size()>0) 
    {                               
        for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
                it != vSbcChannelSettings.end(); ++it)
        {            
            delete (*it);
        }
        vSbcChannelSettings.clear();
    }
}


/////////////////////////////////////////////////////////////////////////////////
// Register setting /////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

// Create new channel setting
TChannelSetting *SettingsPerChannel_RegisterAddSetting(const char *szSubSection, CSimpleSetting *CSSetting, SETTING *Setting, long **pToggleValue)
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
            return (*it);
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
    ChannelSetting->CommonFlag = 0;
    ChannelSetting->Flag = 0;
    ChannelSetting->ToggleSetting = NULL;
    ChannelSetting->ToggleSettingIsLink = 0;
    ChannelSetting->sSubSection = szSubSection;

    ChannelSetting->DefaultStateValue = ChannelSetting->DefaultValue;
    ChannelSetting->bInDefaultState = FALSE;



    return ChannelSetting;
}

//
int SettingsPerChannel_RegisterSetting(const char *szName,const char *szDescription,BOOL bDefault, CSimpleSetting *CSSetting, SETTING *Setting, long **pToggleValue)
{    
    TChannelSetting *ChannelSetting = SettingsPerChannel_RegisterAddSetting(sSbcSubSection.c_str(), CSSetting, Setting, pToggleValue);

    if (ChannelSetting == NULL)
    {
        return -1;
    }
    vSbcChannelSettings.push_back(ChannelSetting);

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
    
    return 0;
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
               delete (*it)->ToggleSetting;
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

void SettingsPerChannel_WriteToIni(int Channel, const char *szSubSection, CSimpleSetting *CSSetting, BOOL bOptimizeFileAccess)
{
    char szNewSection[100];
    char szNewEntry[200];
    //sprintf(szSection,"Channel_%d_%s",Channel,szSubSection);                
    sprintf(szNewSection,"Channel_%d",Channel+1);
    char oldEntry[100];
    char oldSection[100];    
    strcpy(oldEntry, CSSetting->GetEntry());
    strcpy(oldSection, CSSetting->GetSection());
    sprintf(szNewEntry,"%s_%s",szSubSection,oldEntry);
    CSSetting->SetSection(szNewSection);
    CSSetting->SetEntry(szNewEntry);
    CSSetting->WriteToIni(bOptimizeFileAccess);        
    CSSetting->SetEntry(oldEntry);
    CSSetting->SetSection(oldSection);    
}

void SettingsPerChannel_WriteToIni(int Channel, const char *szSubSection, SETTING *Setting, BOOL bOptimizeFileAccess)
{
    char szNewSection[100];
    char szNewEntry[200];
    //sprintf(szSection,"Channel_%d_%s",Channel,szSubSection);                
    sprintf(szNewSection,"Channel_%d",Channel+1);
    
    char *szIniSection = Setting->szIniSection;        
    char *szIniEntry = Setting->szIniEntry;
    sprintf(szNewEntry,"%s_%s",szSubSection,szIniEntry);

    Setting->szIniSection = szNewSection;
    Setting->szIniEntry = szNewEntry;
    Setting_WriteToIni(Setting,bOptimizeFileAccess);                    
    Setting->szIniSection = szIniSection;
    Setting->szIniEntry = szIniEntry;    
}

BOOL SettingsPerChannel_ReadFromIni(int Channel, const char *szSubSection, CSimpleSetting *CSSetting)
{
    BOOL result = FALSE;
    char szNewSection[100];
    char szNewEntry[200];

    if (Channel>=0)
    {                        
        sprintf(szNewSection,"Channel_%d",Channel+1);
    }
    else
    {
        strcpy(szNewSection, szSubSection);
    }

        char oldEntry[100];
        char oldSection[100];
        
        strcpy(oldSection, CSSetting->GetSection());
        CSSetting->SetSection(szNewSection);
        
        if (Channel>=0)
        {
            strcpy(oldEntry, CSSetting->GetEntry());
            sprintf(szNewEntry,"%s_%s",szSubSection,oldEntry);        
            CSSetting->SetEntry(szNewEntry);
        }
        
        result = CSSetting->ReadFromIni(TRUE);
        
        if (Channel>=0)
        {
            CSSetting->SetEntry(oldEntry);
        }
        CSSetting->SetSection(oldSection);
    return result;
}

BOOL SettingsPerChannel_ReadFromIni(int Channel, const char *szSubSection, SETTING *Setting)
{
    BOOL result = FALSE;
    char szNewSection[100];
    char szNewEntry[200];

    if (Channel>=0)
    {
        sprintf(szNewSection,"Channel_%d",Channel+1);
    }
    else
    {
        strcpy(szNewSection, szSubSection);
    }

        char *szIniSection = Setting->szIniSection;        
        char *szIniEntry = Setting->szIniEntry;
        sprintf(szNewEntry,"%s_%s",szSubSection,szIniEntry);

        Setting->szIniSection = szNewSection;
        Setting->szIniEntry = szNewEntry;        

        result = Setting_ReadFromIni(Setting, TRUE);
        
        Setting->szIniSection = szIniSection;
        Setting->szIniEntry = szIniEntry;
    
    return result;
}




void SettingsPerChannel_ReadSettings(const char *szSubSection, int Channel, int DefaultValuesFirst)
{
    LOG(2,"Read settings for channel %d.",Channel+1);
    if (!bSbcEnabled)
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
                CSetting->LoadChannelSpecifics(Channel, TRUE);                
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
                    if (CSetting->ReadFromIni(Channel))
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

void SettingsPerChannel_ReadDefaults(const char *szSubSection, int Channel)
{
    SettingsPerChannel_ReadSettings(szSubSection, Channel, -1);
}


void SettingsPerChannel_WriteSettings(const char *szSubSection, int Channel, BOOL bOptimizeFileAccess)
{
    LOG(2,"Store settings for channel %d, subsection %s.",Channel,(szSubSection==NULL)?"":szSubSection);    
    if (!bSbcEnabled)
    {
        //return;
    } 
   
   if (bSbcEnabled && (iSbcLastChannel >= 0))
    {
        for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
            it != vSbcChannelSettings.end();
            ++it)
        {        
            TChannelSetting *CSetting = (*it);
            if ( CSetting->Enabled() && CSetting->Valid() )          
            {        
                CSetting->SaveChannelSpecifics(iSbcLastChannel);              
            }
        }              
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
            TChannelSpecificSetting *ChSpecific;
            
            int OldValue = CSetting->GetValue();
            
            int i;
            for (i = 0; i< CSetting->vChannelSpecificSettings.size(); i++)
            {
                ChSpecific = CSetting->vChannelSpecificSettings[i];
                if ((Channel == -1) || (Channel == ChSpecific->Channel))
                {            
                    // Write of write bit of Flag is set, or if value has changed
                    if ( (ChSpecific->Flag&1) || (ChSpecific->Value != ChSpecific->LastValue) )
                    {
                        if (ChSpecific->Value != CSetting->GetValue()) 
                        {
                            CSetting->SetValue(ChSpecific->Value, TRUE);
                        }
                        CSetting->WriteToIni(ChSpecific->Channel, FALSE);
                    }            
                    // Clear write flag
                    ChSpecific->Flag &= ~1;            
    
                    // Set new lastValue
                    ChSpecific->LastValue = ChSpecific->Value;            
                }
            }
            if (OldValue != CSetting->GetValue()) 
            { 
                CSetting->SetValue(OldValue, TRUE);
            }
        }        
    }        

    
}


void SettingsPerChannel_ClearSettings(const char *szSubSection, int Channel, int ClearIniSection) //, std::vector<std::string> *vsDefaultSections)
{
    SettingsPerChannel_ReadSettings(szSubSection,Channel,2); //,vsDefaultSections);
    if (ClearIniSection)
    {
       
    }
    //SettingsPerChannel_WriteSettings(szSubSection, Channel, TRUE);      
    SettingsPerChannel_SaveChannelSettings(szSubSection, Channel);
}


/////////////////////////////////////////////////////////////////////////////////
// Load/Save channel settings ///////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


void SettingsPerChannel_LoadChannelSettings(const char *szSubSection, int Channel)
{
    if (!bSbcEnabled)
    {
        return;
    }

    if (iSbcLastLoadedChannel == Channel)
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
            CSetting->LoadChannelSpecifics(Channel);
        }        

    }        

    iSbcLastLoadedChannel = Channel;    
}

void SettingsPerChannel_SaveChannelSettings(const char *szSubSection, int Channel)
{
    if (!bSbcEnabled)
    {
        //return;
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
            CSetting->SaveChannelSpecifics(Channel);
        }        

    }        
}


/////////////////////////////////////////////////////////////////////////////////
// Channel change ///////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

void SettingsPerChannel_ChannelChange(void *pThis, int PreChange, int OldChannel, int NewChannel)
{      
   if (PreChange)
   {
      if (!SettingsPerChannel())
      {
          return;
      }
         
      if (iSbcLastChannel < 0)       
      {
          OldChannel = -1;
      }
    
      for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
        it != vSbcChannelSettings.end();
        ++it)
      {        
          TChannelSetting *CSetting = (*it);
          if ( CSetting->Enabled() && CSetting->Valid() )          
          {        
             if (OldChannel >= 0)
             {
                  CSetting->SaveChannelSpecifics(OldChannel);              
             }
             //CSetting->LoadChannelSpecifics(NewChannel);             
          }  
      }       
   } 
   else
   {
      iSbcLastChannel = NewChannel;   

      if (!SettingsPerChannel())
      {
          return;
      }   
      SettingsPerChannel_ReadSettings(NULL,NewChannel,1);      
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
        if ((vSbcOnSetupList[i] == pfnSetup) && (vSbcOnSetupThisList[i] == pThis))
        {
            // Already in list
            return;
        }
    }

    vSbcOnSetupList.push_back(pfnSetup);
    vSbcOnSetupThisList.push_back(pThis);
}

void SettingsPerChannel_Setup(int Start, int StartChannel)
{
    int i;    
    for (i = 0; i < vSbcOnSetupList.size(); i++)
    {    
       vSbcOnSetupList[i](vSbcOnSetupThisList[i], Start); 
    }
    if (Start)
    {
       Channel_Register_Change_Notification(NULL, SettingsPerChannel_ChannelChange);
       SettingsPerChannel_ChannelChange(NULL, 0, -1, StartChannel);
    }
    else
    {
       Channel_UnRegister_Change_Notification(NULL, SettingsPerChannel_ChannelChange);
    }
}



/////////////////////////////////////////////////////////////////////////////////
// On/off settings //////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


#define SETTINGSPERCHANNEL_COMMONSETTINGS_LASTONE 1

SETTING SettingsPerChannel_CommonSettings[SETTINGSPERCHANNEL_COMMONSETTINGS_LASTONE] =
{
    {
        "Settings per channel", ONOFF, 0, (long*)&bSbcEnabled, 
        0, 0, 1, 1, 1, NULL,
        "SettingsPerChannel", "SettingsPerChannelEnabled", SettingsPerChannel_OnOff,
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
    static SETTING* SettingsPerChannel_Settings = NULL;
    int NumSettings = SettingsPerChannel_BuildSettingsArray(SettingsPerChannel_Settings);
    return new CTreeSettingsGeneric("Settings per Channel", SettingsPerChannel_Settings, NumSettings);
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

