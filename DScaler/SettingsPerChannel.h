#ifndef __SETTINGSPERCHANNEL_H___
#define __SETTINGSPERCHANNEL_H___

#include "setting.h"

void SettingsPerChannel_ReadSettingsFromIni();
void SettingsPerChannel_WriteSettingsToIni(BOOL bOptimizeFileAccess);
SETTING* SettingsPerChannel_GetSetting(int iSetting);

BOOL SettingsPerChannel_IsPerChannel();
BOOL SettingsPerChannel_IsPerFormat();
BOOL SettingsPerChannel_IsPerInput();


#endif
