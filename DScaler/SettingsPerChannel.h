#ifndef __SETTINGSPERCHANNEL_H___
#define __SETTINGSPERCHANNEL_H___

#include "setting.h"
#include "settings.h"
#include "source.h"

typedef void (__cdecl SETTINGSPERCHANNEL_ONSETUP)(void *pThis,int Start);

#define VIDEOINPUT_UNKNOWN 0x80000000L

BOOL SettingsPerChannel();
void SettingsPerChannel_Enable();
void SettingsPerChannel_Disable();

// Temporary change setting values to defaults
void SettingsPerChannel_ToDefaultState(BOOL bTrue);

void SettingsPerChannel_ClearAll();

// Register new settings
int  SettingsPerChannel_RegisterSetting(const char *szName,const char *szDescription,BOOL bDefault);
int  SettingsPerChannel_RegisterSetting(const char *szName,const char *szDescription,BOOL bDefault, CSimpleSetting *CSSetting);
int  SettingsPerChannel_RegisterSetting(const char *szName,const char *szDescription,BOOL bDefault, SETTING *Setting);
int  SettingsPerChannel_RegisterSetting(const char *szName,const char *szDescription,BOOL bDefault, long** pValue);

// Change registration section
void SettingsPerChannel_RegisterSetSection(const char *szSubSection);
void SettingsPerChannel_UnregisterSection(const char *szSubSection);

// Read & write settings from .ini file
void SettingsPerChannel_ReadSettings(const char *szSubSection, int Input, int Channel, int DefaultValuesFirst = 1);
void SettingsPerChannel_WriteSettings(BOOL bOptimizeFileAccess);
void SettingsPerChannel_ReadDefaults(const char *szSubSection, int Input, int Channel);
void SettingsPerChannel_ClearSettings(const char *szSubSection, int Input, int Channel, int ClearIniSection);
 
void SettingsPerChannel_NewDefaults(const char* szSubSection,BOOL bCurrentValue);
void SettingsPerChannel_SaveChannelSettings(const char *szSubSection, int Input, int Channel);

// Setup startup/shutdown functions
void SettingsPerChannel_Setup(int Start);
void SettingsPerChannel_RegisterOnSetup(void *pThis, SETTINGSPERCHANNEL_ONSETUP* pfnSetup);

// Tuner status changed
void SettingsPerChannel_VideoInputChange(CSource *pSource, int PreChange, int NewInput, int IsTuner);

CTreeSettingsGeneric* SettingsPerChannel_GetTreeSettingsPage();
void SettingsPerChannel_ReadSettingsFromIni();
void SettingsPerChannel_WriteSettingsToIni(BOOL bOptimizeFileAccess);
SETTING* SettingsPerChannel_GetSetting(int iSetting);


#endif
