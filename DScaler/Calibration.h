/////////////////////////////////////////////////////////////////////////////
// $Id: Calibration.h,v 1.22 2002-02-09 02:44:56 laurentg Exp $
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
// Change Log
//
// Date          Developer             Changes
//
// 23 Jul 2001   Laurent Garnier       File created
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __CALIBRATION_H___
#define __CALIBRATION_H___


#include "TVFormats.h"
#include "Setting.h"
#include "StillSource.h"


/// Define all types of content for test pattern
enum eTypeContentPattern {
    PAT_RANGE_OF_GRAY = 0,
    PAT_COLOR,
    PAT_GRAY_AND_COLOR,
    PAT_UNKNOWN,
};


/// Define all types of adjustments for a sub-pattern
enum eTypeAdjust {
    ADJ_MANUAL = 0,
    ADJ_BRIGHTNESS,
    ADJ_CONTRAST,
    ADJ_BRIGHTNESS_CONTRAST,
    ADJ_SATURATION_U,
    ADJ_SATURATION_V,
    ADJ_COLOR,
    ADJ_HUE,
};


/// Define all types of calibration
enum eTypeCalibration {
    CAL_MANUAL = 0,
    CAL_AUTO_BRIGHT_CONTRAST,
    CAL_AUTO_COLOR,
    CAL_AUTO_FULL,
};


/** Object representing a color bar in a test pattern

    A color bar is a rectangular zone from the test pattern
    in which all the pixels have same theoritical color.
*/
class CColorBar
{
public:
    CColorBar(unsigned short int left, unsigned short int right, unsigned short int top, unsigned short int bottom, BOOL YUV, unsigned char R_Y, unsigned char G_U, unsigned char B_V);
    CColorBar(CColorBar* pColorBar);

    /// This method returns the position of the color bar
    void GetPosition(unsigned short int* left, unsigned short int* right, unsigned short int* top, unsigned short int* bottom);

    /// This methode returns the reference color
    void GetRefColor(BOOL YUV, unsigned char* pR_Y, unsigned char* pG_U, unsigned char* pB_V);

    /// This methode returns the calculated average color
    void GetCurrentAvgColor(BOOL YUV, unsigned char* pR_Y, unsigned char* pG_U, unsigned char* pB_V);

    /// This methode returns the delta between reference color and calculated average color
    void GetDeltaColor(BOOL YUV, int* pR_Y, int* pG_U, int* pB_V, int* pTotal);

    /** This method analyzed the overlay buffer to calculate average color
        in the zone defined by the color bar
    */
    BOOL CalcAvgColor(BOOL reinit, unsigned int nb_calc_needed, TDeinterlaceInfo* pInfo);

    /// This method draws in the video signal a rectangle around the color bar
    void DrawPosition(TDeinterlaceInfo* pInfo);

    void Draw(BYTE* buffer, int height, int width);

protected: 
    /** Left position of the rectangular zone in the full test pattern
        Range between 0 and 10000
    */
    unsigned short int m_LeftBorder;

    /** Right position of the rectangular zone in the full test pattern
        Range between 0 and 10000
    */
    unsigned short int m_RightBorder;

    /** Top position of the rectangular zone in the full test pattern
        Range between 0 and 10000
    */
    unsigned short int m_TopBorder;

    /** Bottom position of the rectangular zone in the full test pattern
        Range between 0 and 10000
    */
    unsigned short int m_BottomBorder;

    /// Reference value for luminance
    unsigned char ref_Y_val;

    /// Reference value for saturation U
    unsigned char ref_U_val;

    /// Reference value for saturation V
    unsigned char ref_V_val;

    /// Reference value for red component
    unsigned char ref_R_val;

    /// Reference value for green component
    unsigned char ref_G_val;

    /// Reference value for blue component
    unsigned char ref_B_val;

    /// Calculated average Y value in the corresponding zone of the overlay
    unsigned char Y_val;

    /// Calculated average U value in the corresponding zone of the overlay
    unsigned char U_val;

    /// Calculated average V value in the corresponding zone of the overlay
    unsigned char V_val;

    /// Calculated average R value in the corresponding zone of the overlay
    unsigned char R_val;

    /// Calculated average G value in the corresponding zone of the overlay
    unsigned char G_val;

    /// Calculated average B value in the corresponding zone of the overlay
    unsigned char B_val;

private:
    /// Convert RGB to YUV
    void RGB2YUV(unsigned char R, unsigned char G, unsigned char B, unsigned char* pY, unsigned char* pU, unsigned char* pV);

    /// Convert YUV to RGB
    void YUV2RGB(unsigned char Y, unsigned char U, unsigned char V, unsigned char* pR, unsigned char* pG, unsigned char* pB);

    unsigned int cpt_Y;
    unsigned int cpt_U;
    unsigned int cpt_V;
    unsigned int cpt_nb;
};


/** Object representing a subset of a predefined test pattern

    A test pattern is divided in several subsets, each subset
    allows to adjust one or several particular settings
*/
class CSubPattern
{
public:
    CSubPattern(eTypeAdjust type);
    ~CSubPattern();

    /// This method returns the type of settings that can be adjusted with this sub-pattern
    eTypeAdjust GetTypeAdjust();

    /// This method analyzes the current overlay buffer
    BOOL CalcCurrentSubPattern(BOOL reinit, unsigned int nb_calc_needed, TDeinterlaceInfo* pInfo);

    /** This methode returns the sum of absolute delta between reference color
        and calculated average color through all the color bars
    */
    void GetSumDeltaColor(BOOL YUV, int* pR_Y, int* pG_U, int* pB_V, int* pTotal);

    /// This method draws in the video signal rectangles around each color bar of the sub-pattern
    void DrawPositions(TDeinterlaceInfo* pInfo);

    /// Color bars of the sub-pattern
    vector<CColorBar*> m_ColorBars;

protected:
    /// type of settings that can be adjusted with this sub-pattern
    eTypeAdjust m_TypeAdjust;
};


/** Object representing a predefined test pattern

    Several sub-patterns can be defined for a test pattern
*/
class CTestPattern
{
public:
    CTestPattern(char* name, int height);
    CTestPattern(LPCSTR FileName);
    ~CTestPattern();

    /// This method returns the name of the test pattern
    char* GetName();

    /// This method returns the width of the test pattern
    int GetWidth();

    /// This method returns the height (number of lines) of the test pattern
    int GetHeight();

    /// This method allows to create a new sub-pattern to the test pattern
    /// which is a merge of all the others sub-patterns
    void CreateGlobalSubPattern();

    /** This method determines the type of content in the test pattern
        going all over the sub-patterns
    */
    eTypeContentPattern DetermineTypeContent();

    /// This method returns the (first) sub-pattern allowing to adjust particular settings
    CSubPattern* GetSubPattern(eTypeAdjust type_adjust);

    void Draw(BYTE* buffer);

    /// Color bars of the test pattern
    vector<CColorBar*> m_ColorBars;

    /// Sub-patterns of the test pattern
    vector<CSubPattern*> m_SubPatterns;

protected:
    /// Name of the test pattern
    char m_PatternName[64];

    /// Width of the test pattern
    int m_Width;

    /// Height (number of lines) of the test pattern
    int m_Height;

private:
    void Log();
};


/** Object representing one of the BT video settings
*/
class CCalSetting
{
public:
    CCalSetting(CSimpleSetting* setting);
    BOOL Update();
    void Save();
    void Restore();
    void SetFullRange();
    void SetRange(int min_val, int max_val);
    void SetRange(int delta);
    void SetRange(int* mask);
    int GetRange(int* mask, int* min_val, int* max_val);
    void AdjustMin();
    void AdjustMax();
    void AdjustDefault();
    BOOL AdjustNext();
    void AdjustBest();
    void InitResult();
    BOOL UpdateResult(int diff, int stop_threshold, BOOL only_one);
    int GetResult(int* mask, int* min_val, int* max_val);

protected:
    void Adjust(int value);
    CSimpleSetting* m_pSetting;
    int min_value;
    int max_value;
    unsigned int mask_input[16];
    unsigned int mask_output[16];

private:
    int min;
    int max;
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
    CCalSetting* brightness;
    CCalSetting* contrast;
    CCalSetting* saturation_U;
    CCalSetting* saturation_V;
    CCalSetting* hue;

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
};


class CPatternHelper : public CStillSourceHelper
{
public:
    CPatternHelper(CStillSource* pParent);
    BOOL OpenMediaFile(LPCSTR FileName);
    void SaveSnapshot(LPCSTR FilePath, int Height, int Width, BYTE* pOverlay, LONG OverlayPitch);
};


/// Global instance of the calibration object
extern CCalibration* pCalibration;


SETTING* Calibr_GetSetting(CALIBR_SETTING Setting);
void Calibr_ReadSettingsFromIni();
void Calibr_WriteSettingsToIni(BOOL bOptimizeFileAccess);


#endif
