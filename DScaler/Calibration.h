/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Laurent Garnier.  All rights reserved.
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

/** 
 * @file calibration.h calibration Header file
 */
 
#ifndef __CALIBRATION_H___
#define __CALIBRATION_H___


#include "Pattern.h"


/// Define all types of calibration
enum eTypeCalibration {
    CAL_MANUAL = 0,
    CAL_AUTO_BRIGHT_CONTRAST,
    CAL_AUTO_COLOR,
    CAL_AUTO_FULL,
    CAL_CHECK_YUV_RANGE,
};


/** Object representing one of the BT video settings
*/
class CCalSetting
{
public:
    CCalSetting(ISetting* setting);
    ~CCalSetting();
    BOOL Update();
    void Save();
    void Restore();
    void SetFullRange();
    void SetRange(int min_val, int max_val);
    void SetRange(int delta);
    void SetRange(unsigned int* mask);
    int GetRange(unsigned int** mask, int* min_val, int* max_val);
    void AdjustMin();
    void AdjustMax();
    void AdjustDefault();
    BOOL AdjustNext();
    void AdjustBest();
    void InitResult();
    BOOL UpdateResult(int diff, int stop_threshold, BOOL only_one);
    int GetResult(unsigned int** mask, int* min_val, int* max_val);

protected:
    void Adjust(int value);
    ISetting* m_pSetting;
    int min_value;
    int max_value;
    unsigned int* mask_input;
    unsigned int* mask_output;

private:
    int min;
    int max;
    int mask_nb;
    int current_value;
    int saved_value;
    unsigned int min_diff;
    unsigned int max_diff;
    BOOL desc;
    BOOL end;
};


/** Object managing the card calibration in DScaler
*/
class CCalibration
{
public:
    CCalibration();
    ~CCalibration();

    /// This method loads all the predefined test patterns
    void LoadTestPatterns();

    /// This method unloads all the predefined test patterns
    void UnloadTestPatterns();

    void UpdateMenu(HMENU hMenu);
    void SetMenu(HMENU hMenu);
    BOOL ProcessSelection(HWND hWnd, WORD wMenuId);
    void SelectTestPattern(int num);
    CTestPattern* GetCurrentTestPattern();
    CSubPattern* GetSubPattern(eTypeAdjust type_adjust);
    CSubPattern* GetCurrentSubPattern();
    void Start(eTypeCalibration type);
    void Stop();
    BOOL IsRunning();
    int GetCurrentStep();
    eTypeCalibration GetType();
    void Make(TDeinterlaceInfo* pInfo, int tick_count);

protected:
    vector<CTestPattern*> m_TestPatterns;
    CTestPattern* m_CurTestPat;
    CSubPattern* m_CurSubPat;
    eTypeCalibration m_TypeCalibration;
    BOOL m_IsRunning;
    CCalSetting* m_Brightness;
    CCalSetting* m_Contrast;
    CCalSetting* m_Saturation_U;
    CCalSetting* m_Saturation_V;
    CCalSetting* m_Hue;

private:
    BOOL step_init(eTypeAdjust type_adjust, CCalSetting*_setting1, CCalSetting*_setting2, CCalSetting*_setting3);
    BOOL step_process(TDeinterlaceInfo* pInfo, unsigned int sig_component, unsigned int nb_calc, BOOL stop_when_found, BOOL only_one, BOOL* best_found);
    int last_tick_count;
    unsigned int initial_step;
    unsigned int nb_steps;
    int current_step;
    BOOL full_range;
    int nb_tries;
    int total_dif;
    int nb_calcul;
    BOOL first_calc;
    CCalSetting* setting1;
    CCalSetting* setting2;
    CCalSetting* setting3;
    CTestPattern* m_YUVRangePat;
};


/// Global instance of the calibration object
extern CCalibration* pCalibration;


SETTING* Calibr_GetSetting(CALIBR_SETTING Setting);
void Calibr_ReadSettingsFromIni();
void Calibr_WriteSettingsToIni(BOOL bOptimizeFileAccess);
CTreeSettingsGeneric* Calibr_GetTreeSettingsPage();


#endif
