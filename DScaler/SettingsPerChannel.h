
#ifndef __SETTINGSPERCHANNEL_H___
#define __SETTINGSPERCHANNEL_H___

#include "setting.h"
#include "settings.h"

BOOL SettingsPerChannel();
void SettingsPerChannel_Enable();
void SettingsPerChannel_Disable();

void SettingsPerChannel_SetSection(const char *szSubSection);
void SettingsPerChannel_ClearSection(const char *szSubSection);
void SettingsPerChannel_Close();

int  SettingsPerChannel_RegisterSetting(const char *szName,const char *szDescription,BOOL bDefault, CSimpleSetting *CSSetting);
int  SettingsPerChannel_RegisterSetting(const char *szName,const char *szDescription,BOOL bDefault, SETTING *Setting);
int  SettingsPerChannel_RegisterSetting(const char *szName,const char *szDescription,BOOL bDefault, long** pValue);
int  SettingsPerChannel_RegisterSetting(int n, CSimpleSetting *CSSetting);
int  SettingsPerChannel_RegisterSetting(int n, SETTING *Setting);
void SettingsPerChannel_SettingChanged(CSimpleSetting *CSSetting);
void SettingsPerChannel_SettingChanged(SETTING *Setting);

void SettingsPerChannel_LoadSettings(const char *szSubSection, int Channel, int DefaultValuesFirst = 0, std::vector<std::string> *sDefaultSections = NULL);
void SettingsPerChannel_SaveSettings(const char *szSubSection, int Channel, BOOL bOptimizeFileAccess);
void SettingsPerChannel_ClearSettings(const char *szSubSection, int Channel, int ClearIniSection, std::vector<std::string> *vsDefaultSections);

CTreeSettingsGeneric* SettingsPerChannel_GetTreeSettingsPage();
void SettingsPerChannel_ReadSettingsFromIni();
void SettingsPerChannel_WriteSettingsToIni(BOOL bOptimizeFileAccess);
SETTING* SettingsPerChannel_GetSetting(int iSetting);


#endif
