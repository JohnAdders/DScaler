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


#include "bt848.h"


// Maximum number of color bars in a test pattern
#define MAX_COLOR_BARS      15

// Maximum number of subset of color bars in a test pattern
#define MAX_SUB_PATTERNS     8

// Maximum number of test patterns managed by DScaler
#define MAX_TEST_PATTERNS   20


// Define all types of content for test pattern
enum eTypeContentPattern {
    PAT_RANGE_OF_GRAY = 0,
    PAT_COLOR,
    PAT_GRAY_AND_COLOR,
    PAT_UNKNOWN,
};


// Define all types of adjustments for a sub-pattern
enum eTypeAdjust {
    ADJ_MANUAL = 0,
    ADJ_BRIGHTNESS,
    ADJ_CONTRAST,
    ADJ_BRIGHTNESS_CONTRAST,
    ADJ_SATURATION_U,
    ADJ_SATURATION_V,
    ADJ_COLOR,
};


// Define all types of calibration
enum eTypeCalibration {
    CAL_MANUAL = 0,
    CAL_AUTO_BRIGHT_CONTRAST,
    CAL_AUTO_COLOR,
    CAL_AUTO_FULL,
};


////////////////////////////////////////////////////////////
// Object representing a color bar in a test pattern
//
// A color bar is a rectangular zone from the test pattern
// in which all the pixels have same theoritical color.
////////////////////////////////////////////////////////////
class CColorBar
{
public:
    CColorBar(unsigned short int left, unsigned short int right, unsigned short int top, unsigned short int bottom, BOOL YUV, unsigned char R_Y, unsigned char G_U, unsigned char B_V);
    CColorBar(CColorBar *pColorBar);

    // This methode returns the position of the color bar
    void GetPosition(unsigned short int *left, unsigned short int *right, unsigned short int *top, unsigned short int *bottom);

    // This methode returns the reference color
    void GetRefColor(BOOL YUV, unsigned char *pR_Y, unsigned char *pG_U, unsigned char *pB_V);

    // This methode returns the calculated average color
    void GetCurrentAvgColor(BOOL YUV, unsigned char *pR_Y, unsigned char *pG_U, unsigned char *pB_V);

    // This methode returns the delta between reference color and calculated average color
    void GetDeltaColor(BOOL YUV, int *pR_Y, int *pG_U, int *pB_V, int *pTotal);

    // This method analyzed the overlay buffer to calculate average color
    // in the zone defined by the color bar
    void CalcAvgColor(short **Lines, int height, int width);

protected: 
    // Left position of the rectangular zone in the full test pattern
    // Range between 0 and 10000
    unsigned short int left_border;

    // Right position of the rectangular zone in the full test pattern
    // Range between 0 and 10000
    unsigned short int right_border;

    // Top position of the rectangular zone in the full test pattern
    // Range between 0 and 10000
    unsigned short int top_border;

    // Bottom position of the rectangular zone in the full test pattern
    // Range between 0 and 10000
    unsigned short int bottom_border;

    // Reference value for luminance
    unsigned char ref_Y_val;

    // Reference value for saturation U
    unsigned char ref_U_val;

    // Reference value for saturation V
    unsigned char ref_V_val;

    // Reference value for red component
    unsigned char ref_R_val;

    // Reference value for green component
    unsigned char ref_G_val;

    // Reference value for blue component
    unsigned char ref_B_val;

    // Calculated average Y value in the corresponding zone of the overlay
    unsigned char Y_val;

    // Calculated average U value in the corresponding zone of the overlay
    unsigned char U_val;

    // Calculated average V value in the corresponding zone of the overlay
    unsigned char V_val;

    // Calculated average R value in the corresponding zone of the overlay
    unsigned char R_val;

    // Calculated average G value in the corresponding zone of the overlay
    unsigned char G_val;

    // Calculated average B value in the corresponding zone of the overlay
    unsigned char B_val;

private:
    // Convert RGB to YUV
    void RGB2YUV(unsigned char R, unsigned char G, unsigned char B, unsigned char *pY, unsigned char *pU, unsigned char *pV);

    // Convert YUV to RGB
    void YUV2RGB(unsigned char Y, unsigned char U, unsigned char V, unsigned char *pR, unsigned char *pG, unsigned char *pB);
    unsigned int min_Y;
    unsigned int max_Y;
    unsigned int min_U;
    unsigned int max_U;
    unsigned int min_V;
    unsigned int max_V;
};


////////////////////////////////////////////////////////////
// Object representing a subset of a predefined test pattern
//
// A test pattern is divided in several subsets, each subset
// allows to adjust one or several particular settings
////////////////////////////////////////////////////////////
class CSubPattern
{
public:
    CSubPattern(eTypeAdjust type);
    ~CSubPattern();

    // This method returns the type of settings that can be adjusted with this sub-pattern
    eTypeAdjust GetTypeAdjust();

    // This method allows to add a new color bar in the sub-pattern
    int AddColorBar(CColorBar *color_bar);

    // This method returns the first bar of the test pattern
    CColorBar *GetFirstColorBar();
    
    // This method returns the next color bar of the sub-pattern
    CColorBar *GetNextColorBar();

    // This function searches in the sub-pattern if it exists color
    // bars with same features as the one in argument
    CColorBar *FindSameCoclorBar(CColorBar *pColorBar);

    // This method analyzes the current overlay buffer
    void CalcCurrentSubPattern(short **Lines, int height, int width);

    // This methode returns the sum of delta between reference color
    // and calculated average color through all the color bars
    void GetSumDeltaColor(BOOL YUV, int *pR_Y, int *pG_U, int *pB_V, int *pTotal);

protected:
    // type of settings that can be adjusted with this sub-pattern
    eTypeAdjust type_adjust;

    // Number of color bars in the sub-pattern
    unsigned char nb_color_bars;

    // Color bars of the sub-pattern
    CColorBar *color_bars[MAX_COLOR_BARS];

private:
    // Index to current color bar
    int idx_color_bar;
};


////////////////////////////////////////////////////////////
// Object representing a predefined test pattern
//
// Several sub-patterns can be defined for a test pattern
////////////////////////////////////////////////////////////
class CTestPattern
{
public:
    CTestPattern(char* name, eVideoFormat format);
    ~CTestPattern();

    // This method returns the name of the test pattern
    char *GetName();

    // This method returns the video format of the test pattern
    eVideoFormat GetVideoFormat();

    // This method allows to add a new sub-pattern to the test pattern
    int AddSubPattern(CSubPattern *sub_pattern);

    // This method allows to create a new sub-pattern to the test pattern
    // which is a merge of all the others sub-patterns
    int CreateGlobalSubPattern();

    // This method determines the type of content in the test pattern
    // going all over the sub-patterns
    eTypeContentPattern DetermineTypeContent();

    // This method returns the (first) sub-pattern allowing to adjust particular settings
    CSubPattern *GetSubPattern(eTypeAdjust type_adjust);

protected:
    // Name of the test pattern
    char pattern_name[64];

    // Video format of the test pattern
    eVideoFormat video_format;

    // Number of sub-patterns in the test pattern
    unsigned char nb_sub_patterns;

    // Sub-patterns of the test pattern
    CSubPattern *sub_patterns[MAX_SUB_PATTERNS];

private:
    // This method returns the first sub-pattern of the test pattern
    CSubPattern *GetFirstSubPattern();
    
    // This method returns the next sub-pattern of the test pattern
    CSubPattern *GetNextSubPattern();

    // Index to current sub-pattern
    int idx_sub_pattern;
};


////////////////////////////////////////////////////////////
// Object managing the card calibration in DScaler
////////////////////////////////////////////////////////////
class CCalibration
{
public:
    CCalibration();
    ~CCalibration();

    // This method loads all the predefined test patterns
    void LoadTestPatterns();

    // This method unloads all the predefined test patterns
    void UnloadTestPatterns();

    void UpdateMenu(HMENU hMenu);
    void SetMenu(HMENU hMenu);
    void SelectTestPattern(int num);
    CTestPattern *GetCurrentTestPattern();
    CSubPattern *GetSubPattern(eTypeAdjust type_adjust);
    CSubPattern *GetCurrentSubPattern();
    void Start(eTypeCalibration type);
    void Stop();
    BOOL IsRunning();
    int GetCurrentStep();
    eTypeCalibration GetType();
    void Make(short **Lines, int height, int width, int tick_count);

protected:
    int nb_test_patterns;
    CTestPattern *test_patterns[MAX_TEST_PATTERNS];
    CTestPattern *current_test_pattern;
    CSubPattern *current_sub_pattern;
    eTypeCalibration type_calibration;
    BOOL running;

private:
    BOOL step_init(eTypeAdjust type_adjust, BT848_SETTING setting, int min, int max);
    BOOL step_process(short **Lines, int height, int width, BT848_SETTING setting, unsigned int sig_component);
    BOOL CCalibration::step_init2(eTypeAdjust type_adjust, BT848_SETTING setting1, int min1, int max1, BT848_SETTING setting2, int min2, int max2);
    BOOL CCalibration::step_process2(short **Lines, int height, int width, BT848_SETTING setting1, BT848_SETTING setting2, unsigned int sig_component);
    int last_tick_count;
    unsigned int initial_step;
    unsigned int nb_steps;
    unsigned int current_step;
    int min_val;
    int max_val;
    int current_val;
    int best_val;
    int min_val2;
    int max_val2;
    int current_val2;
    int best_val2;
    int min_dif;
    int total_dif;
    int nb_calcul;
};


// Global variables
extern CCalibration *pCalibration;


#endif