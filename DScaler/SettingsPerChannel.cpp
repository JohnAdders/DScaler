#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "setting.h"
#include "settings.h"
#include "providers.h"
#include "DebugLog.h"
#include "SettingsPerChannel.h"

typedef struct {        
        CSimpleSetting *CSSetting;
        SETTING        *Setting;
        long           **pToggleValue;
        long           orgValue;
        int Flag;        
        std::string sSubSection;
        //
        SETTING *ToggleSetting;
        int ToggleSettingIsLink;
        BOOL Enabled;
        std::string sDisplayName;
        std::string sIniSection;
        std::string sIniEntry;
        
}  TChannelSetting;


std::vector<TChannelSetting*> vSbcChannelSettings;
SETTING *SettingsPerChannel_Settings = NULL;
BOOL bSbcEnabled = FALSE;
std::string sSbcSubSection;
std::vector<SETTING*> vSbcExtraSettingList;


BOOL SettingsPerChannel_OnOff(long NewValue);


#define SETTINGSPERCHANNEL_COMMONSETTINGS_LASTONE 1

SETTING SettingsPerChannel_CommonSettings[SETTINGSPERCHANNEL_COMMONSETTINGS_LASTONE] =
{
    {
        "Settings per channel", ONOFF, 0, (long*)&bSbcEnabled, 
        0, 0, 1, 1, 1, NULL,
        "SettingsPerChannel", "SettingsPerChannelEnabled", SettingsPerChannel_OnOff,
    },
};



BOOL SettingsPerChannel()
{
    return bSbcEnabled;
}

void SettingsPerChannel_Enable()
{
    bSbcEnabled = TRUE;
}

void SettingsPerChannel_Disable()
{
    bSbcEnabled = FALSE;
}

BOOL SettingsPerChannel_OnOff(long NewValue)
{
    bSbcEnabled = NewValue;
    if (bSbcEnabled)
    {

    } 
    else 
    {
        //Turned off

        //SaveSettings
    }    
    return FALSE;
}

void SettingsPerChannel_SetSection(const char *szSubSection)
{        
    sSbcSubSection = szSubSection;
}

void SettingsPerChannel_ClearSection(const char *szSubSection)
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
                           

void SettingsPerChannel_Close()
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

void SettingsPerChannel_SettingChanged(SETTING *Setting)
{
    for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
        it != vSbcChannelSettings.end();
        ++it)
    {        
        if ( (*it)->Setting == Setting ) // && (*it)->Enabled )
        {
            (*it)->Flag|=1;            
            return;
        }
    }    
}

void SettingsPerChannel_SettingChanged(CSimpleSetting *CSSetting)
{
    for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
        it != vSbcChannelSettings.end();
        ++it)
    {        
        if ( (*it)->CSSetting == CSSetting ) // && (*it)->Enabled )
        {
            (*it)->Flag|=1;
            return;
        }
    }
    //LOG(2,"Save per channel: setting not found. (%i)",Setting->Name);
}


void SettingsPerChannel_MakeToggleSetting(TChannelSetting *ChannelSetting, const char *szName,const char *szDescription,BOOL bDefault)
{
    if (ChannelSetting->ToggleSetting != NULL)
    {
        if (!ChannelSetting->ToggleSettingIsLink)
        {
            delete ChannelSetting->ToggleSetting;
        }
    }
    ChannelSetting->ToggleSetting = new SETTING;
    ChannelSetting->sDisplayName = std::string(szDescription);
    ChannelSetting->sIniEntry = std::string(szName);
    ChannelSetting->sIniSection = std::string("SettingsPerChannel");    
    ChannelSetting->Enabled = bDefault;
    ChannelSetting->ToggleSetting->szDisplayName = (char*)ChannelSetting->sDisplayName.c_str();
    ChannelSetting->ToggleSetting->Type = ONOFF;
    ChannelSetting->ToggleSetting->LastSavedValue = bDefault; 
    ChannelSetting->ToggleSetting->pValue = (long*)&(ChannelSetting->Enabled); 
    ChannelSetting->ToggleSetting->Default = bDefault;
    ChannelSetting->ToggleSetting->MinValue = 0;
    ChannelSetting->ToggleSetting->MaxValue = 1;
    ChannelSetting->ToggleSetting->StepValue = 1;
    ChannelSetting->ToggleSetting->OSDDivider = 1;
    ChannelSetting->ToggleSetting->pszList = NULL;
    ChannelSetting->ToggleSetting->szIniSection = (char*)ChannelSetting->sIniSection.c_str();
    ChannelSetting->ToggleSetting->szIniEntry = (char*)ChannelSetting->sIniEntry.c_str();    
    ChannelSetting->ToggleSetting->pfnOnChange = NULL; //(ChannelSetting->OnChange); //SettingsPerChannel_OnChange; //NULL; //

    ChannelSetting->ToggleSettingIsLink = 0;
}

int SettingsPerChannel_RegisterAddSetting(const char *szSubSection, CSimpleSetting *CSSetting, SETTING *Setting, long **pToggleValue)
{
    int n = 0;
    for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
        it != vSbcChannelSettings.end();
        ++it)
    {        
        if  (    ( ( (Setting != NULL) && ((*it)->Setting == Setting) ) 
                || ( (CSSetting != NULL) && ((*it)->CSSetting == CSSetting) ) 
                || ( (pToggleValue != NULL) && ((*it)->pToggleValue == pToggleValue ) ) )
              && ( (szSubSection==NULL) || (std::string(szSubSection) == (*it)->sSubSection) ) )
        {
            //already exists
            return n;
        }
        n++;
    }    
    
    // Add new
    TChannelSetting *ChannelSetting = new TChannelSetting;

    ChannelSetting->Setting = Setting;        
    ChannelSetting->CSSetting = CSSetting;
    ChannelSetting->orgValue = -1;
    if (Setting != NULL) 
    {
        ChannelSetting->orgValue = *Setting->pValue;    
    }
    ChannelSetting->pToggleValue = pToggleValue;
    ChannelSetting->Flag = 0;
    ChannelSetting->ToggleSetting = NULL;
    ChannelSetting->ToggleSettingIsLink = 0;
    ChannelSetting->sSubSection = szSubSection;

    if (ChannelSetting->pToggleValue != NULL)
    {
        *ChannelSetting->pToggleValue = NULL;
    }

    n = vSbcChannelSettings.size();
    vSbcChannelSettings.push_back(ChannelSetting);
    return n;
}

int SettingsPerChannel_RegisterSetting(const char *szName,const char *szDescription,BOOL bDefault, CSimpleSetting *CSSetting, SETTING *Setting, long **pToggleValue)
{    
    int n = SettingsPerChannel_RegisterAddSetting(sSbcSubSection.c_str(), CSSetting, Setting, pToggleValue);

    TChannelSetting *ChannelSetting = vSbcChannelSettings[n];    

    SettingsPerChannel_MakeToggleSetting(ChannelSetting, szName, szDescription, bDefault);
    if (ChannelSetting->pToggleValue != NULL)
    {
        *ChannelSetting->pToggleValue = ChannelSetting->ToggleSetting->pValue;
    }
    Setting_ReadFromIni(ChannelSetting->ToggleSetting);
    return n;
}



int SettingsPerChannel_RegisterSetting(int i, CSimpleSetting *CSSetting, SETTING *Setting, long **pToggleValue)
{    
    if (i<0) 
    {
        return -1;
    }
    int n = SettingsPerChannel_RegisterAddSetting(sSbcSubSection.c_str(), CSSetting, Setting, pToggleValue);
    TChannelSetting *ChannelSetting = vSbcChannelSettings[n];
    
    if ( (ChannelSetting->ToggleSetting != NULL) && (!ChannelSetting->ToggleSettingIsLink) 
         && (ChannelSetting->ToggleSetting != vSbcChannelSettings[i]->ToggleSetting) )
    {
        delete ChannelSetting->ToggleSetting;
        ChannelSetting->ToggleSetting = NULL;
    }
    ChannelSetting->ToggleSetting = vSbcChannelSettings[i]->ToggleSetting;
    ChannelSetting->ToggleSettingIsLink = 1;    
    if (ChannelSetting->pToggleValue != NULL)
    {
        *ChannelSetting->pToggleValue = ChannelSetting->ToggleSetting->pValue;
    }

    Setting_ReadFromIni(ChannelSetting->ToggleSetting);    
    return n;    
}



int SettingsPerChannel_RegisterSetting(int i, CSimpleSetting *CSSetting)
{  
    return SettingsPerChannel_RegisterSetting(i, CSSetting, NULL, NULL);
}

int SettingsPerChannel_RegisterSetting(int i, SETTING *Setting)
{  
    return SettingsPerChannel_RegisterSetting(i, NULL, Setting, NULL);
}


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
        //sprintf(szSection,"Channel_%d_%s",Channel,szSubSection);                
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
        
        //long oldValue = CSSetting->GetValue();        
        result = CSSetting->ReadFromIni(TRUE);
        /*if (result)
        {
            *pNewVal = CSSetting->GetValue();
        }        
        else
        {
            CSSetting->SetValue(oldValue);
        } */
        
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
        //sprintf(szSection,"Channel_%d_%s",Channel,szSubSection);                
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

void SettingsPerChannel_LoadSettings(const char *szSubSection, int Channel, int DefaultValuesFirst, std::vector<std::string> *vsDefaultSections)
{
    LOG(2,"Load settings for channel %d.",Channel+1);
    if (!bSbcEnabled)
    {
        return;
    }

    //char szSection[100];
    int iChanged;
    long NewDefaultValue;
    
    for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
        it != vSbcChannelSettings.end();
        ++it)
    {        
        if (    ((*it)->Enabled) 
             && ( ((*it)->Setting != NULL) || ((*it)->CSSetting != NULL) ) 
             && ( (szSubSection == NULL) || ((*it)->sSubSection == std::string(szSubSection)) ) )
        {
            CSimpleSetting *CSSetting = (*it)->CSSetting;
            SETTING *Setting = (*it)->Setting;                                     
            int OldValue;

            if (Setting == NULL)
            {
                OldValue = CSSetting->GetValue();
                NewDefaultValue = CSSetting->GetDefault();
            }
            else
            {
                OldValue = *Setting->pValue;
                NewDefaultValue = Setting->Default;
            }
            iChanged = 0;
            
            
            if (DefaultValuesFirst && (vsDefaultSections!=NULL))
            {                        
                for (vector<std::string>::iterator is = (*vsDefaultSections).begin(); 
                      is != (*vsDefaultSections).end(); 
                      ++is)
                {
                    if (Setting != NULL)
                    {
                        if (SettingsPerChannel_ReadFromIni(-1, is->c_str(), Setting))
                        {
                            NewDefaultValue = *Setting->pValue;
                            iChanged = 1;
                            break;                    
                        }
                    }
                    if (CSSetting != NULL)
                    {
                        if (SettingsPerChannel_ReadFromIni(-1, is->c_str(), CSSetting))
                        {
                            NewDefaultValue = CSSetting->GetValue();
                            iChanged = 1;
                            break;                    
                        }
                        
                    }

                }
            }
        
            if (DefaultValuesFirst>=0)
            {                
                //sprintf(szSection,"Channel_%d_%s",Channel,(*it)->sSubSection.c_str());                
                if (Setting != NULL)
                {
                        if (SettingsPerChannel_ReadFromIni(Channel, (*it)->sSubSection.c_str(), Setting)) 
                        {
                            iChanged = 1;    
                            if (DefaultValuesFirst == 2)
                            {                                
                               *Setting->pValue=NewDefaultValue;
                               iChanged = 3;
                            }
                        } 
                        else 
                        {
                              if (DefaultValuesFirst && !iChanged)
                              {
                                  *Setting->pValue=NewDefaultValue;
                                  iChanged = 2;                            
                              }                              
                        }
                }
                if (CSSetting != NULL)
                {
                        if (SettingsPerChannel_ReadFromIni(Channel, (*it)->sSubSection.c_str(), CSSetting)) 
                        {
                            iChanged = 1;    
                            if (DefaultValuesFirst == 2)
                            {                                
                               CSSetting->SetValue(NewDefaultValue);
                               iChanged = 3;
                            }
                        }
                        else 
                        {
                              if (DefaultValuesFirst && !iChanged)
                              {
                                  CSSetting->SetValue(NewDefaultValue);
                                  iChanged = 2;
                              }                              
                        }
                }                                                
            }
            
            if ( (Setting != NULL) && iChanged)
            {                
                long NewValue = *Setting->pValue;
                *Setting->pValue = OldValue;
                Setting_SetValue(Setting, NewValue);
                /*if (Setting->pfnOnChange != NULL)
                {
                    
                    Setting->pfnOnChange(NewValue);
                }*/                
            }
            if ( (CSSetting != NULL) && (iChanged == 1) )
            {
                CSSetting->SetValue(CSSetting->GetValue());
            }

            (*it)->Flag &= ~1;

            if (Setting != NULL)
            {
                (*it)->orgValue = *Setting->pValue;
            }
            if (CSSetting != NULL)
            {
                (*it)->orgValue = CSSetting->GetValue();
            }


            if ((DefaultValuesFirst == 2) && (iChanged==3))
            {
                (*it)->Flag |= 1;
            }                
        }
    }
}



void SettingsPerChannel_SaveSettings(const char *szSubSection, int Channel, BOOL bOptimizeFileAccess)
{
    LOG(2,"Store settings for channel %d, subsection %s.",Channel,(szSubSection==NULL)?"":szSubSection);    
    if (!bSbcEnabled)
    {
        return;
    }

    for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
        it != vSbcChannelSettings.end();
        ++it)
    {        
        CSimpleSetting *CSSetting = (*it)->CSSetting;
        SETTING *Setting = (*it)->Setting;

        if ( ((*it)->Enabled) && ( (szSubSection == NULL) || ((*it)->sSubSection == szSubSection) ) )
        {
                            
            if ( (Setting != NULL) && ( ((*it)->Flag&1) || (*Setting->pValue != (*it)->orgValue) ) )
            {
                 SettingsPerChannel_WriteToIni(Channel, (*it)->sSubSection.c_str(), Setting, bOptimizeFileAccess);
            }
            if ( (CSSetting != NULL) && ( ((*it)->Flag&1) || (CSSetting->GetValue() != (*it)->orgValue) ) )
            {
                  SettingsPerChannel_WriteToIni(Channel, (*it)->sSubSection.c_str(), CSSetting, bOptimizeFileAccess);
            }
            (*it)->Flag&=~1;            

            if (Setting != NULL)
            {
                (*it)->orgValue = *Setting->pValue;
            }
            if (CSSetting != NULL)
            {
                (*it)->orgValue = CSSetting->GetValue();
            }
        }        

    }        
}


void SettingsPerChannel_ClearSettings(const char *szSubSection, int Channel, int ClearIniSection, std::vector<std::string> *vsDefaultSections)
{
    SettingsPerChannel_LoadSettings(szSubSection,Channel,2,vsDefaultSections);
    if (ClearIniSection)
    {
       
    }
    SettingsPerChannel_SaveSettings(szSubSection, Channel, TRUE); 
     
}

int Rebuild_SettingsPerChannel_Settings()
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
    int NumSettings = Rebuild_SettingsPerChannel_Settings();
    return new CTreeSettingsGeneric("Settings per Channel", SettingsPerChannel_Settings, NumSettings);
}


void SettingsPerChannel_ReadSettingsFromIni()
{
    int i;
    
    for ( i=0; i<SETTINGSPERCHANNEL_COMMONSETTINGS_LASTONE; i++)
    {       
       Setting_ReadFromIni(&SettingsPerChannel_CommonSettings[i]);
    }
    
          
    for(vector<TChannelSetting*>::iterator it = vSbcChannelSettings.begin();
        it != vSbcChannelSettings.end();
        ++it)
    {        
        Setting_ReadFromIni((*it)->ToggleSetting);
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
        Setting_WriteToIni((*it)->ToggleSetting, bOptimizeFileAccess);
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
        if (iSetting == 0)
        {
            return (*it)->ToggleSetting;
        } 
        iSetting--;        
    }
    return NULL;
}

