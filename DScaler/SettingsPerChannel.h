
#ifndef __SETTINGSPERCHANNEL_H___
#define __SETTINGSPERCHANNEL_H___

#include "setting.h"
#include "settings.h"

typedef void (__cdecl SETTINGSPERCHANNEL_ONSETUP)(void *pThis,int Start);

BOOL SettingsPerChannel();
void SettingsPerChannel_Enable();
void SettingsPerChannel_Disable();

void SettingsPerChannel_ToDefaultState(BOOL bTrue);

void SettingsPerChannel_ClearAll();

int  SettingsPerChannel_RegisterSetting(const char *szName,const char *szDescription,BOOL bDefault, CSimpleSetting *CSSetting);
int  SettingsPerChannel_RegisterSetting(const char *szName,const char *szDescription,BOOL bDefault, SETTING *Setting);
int  SettingsPerChannel_RegisterSetting(const char *szName,const char *szDescription,BOOL bDefault, long** pValue);

void SettingsPerChannel_RegisterSetSection(const char *szSubSection);
void SettingsPerChannel_UnregisterSection(const char *szSubSection);


void SettingsPerChannel_ReadSettings(const char *szSubSection, int Channel, int DefaultValuesFirst = 1);
void SettingsPerChannel_WriteSettings(const char *szSubSection, int Channel, BOOL bOptimizeFileAccess);
void SettingsPerChannel_ReadDefaults(const char *szSubSection, int Channel);
void SettingsPerChannel_ClearSettings(const char *szSubSection, int Channel, int ClearIniSection);

void SettingsPerChannel_SaveChannelSettings(const char *szSubSection, int Channel);

void SettingsPerChannel_Setup(int Start, int Channel);
void SettingsPerChannel_RegisterOnSetup(void *pThis, SETTINGSPERCHANNEL_ONSETUP* pfnSetup);


CTreeSettingsGeneric* SettingsPerChannel_GetTreeSettingsPage();
void SettingsPerChannel_ReadSettingsFromIni();
void SettingsPerChannel_WriteSettingsToIni(BOOL bOptimizeFileAccess);
SETTING* SettingsPerChannel_GetSetting(int iSetting);


#endif
