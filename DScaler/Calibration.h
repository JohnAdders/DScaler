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

// Maximum number of test patterns managed by DScaler
#define MAX_TEST_PATTERNS   20


// Define all types of calibration
enum eTypeTestPattern {
    PAT_RANGE_OF_GRAY = 0,
    PAT_COLOR,
    PAT_GRAY_AND_COLOR,
    PAT_OTHER,
};


// Define all types of test pattern
enum eTypeCalibration {
    CAL_MANUAL = 0,
    CAL_AUTO_BRIGHT_CONTRAST,
    CAL_AUTO_COLOR,
    CAL_AUTO_FULL,
};


// Define possible colors for a bar in a test pattern
enum eColor {
    COLOR_BLACK = 0,
    COLOR_WHITE,
    COLOR_RED,
    COLOR_BLUE,
    COLOR_GREEN,
    COLOR_OTHER,
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
    CColorBar(unsigned short int left, unsigned short int right, unsigned short int top, unsigned short int bottom, eColor color, BOOL YUV, unsigned char R_Y, unsigned char G_U, unsigned char B_V);

    // This method returns the abstract color
    eColor GetAbstractColor();

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

    // Abstract color of the bar
    eColor abstract_color;

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
};


////////////////////////////////////////////////////////////
// Object representing a predefined test pattern
////////////////////////////////////////////////////////////
class CTestPattern
{
public:
    CTestPattern(char* name, eTypeTestPattern type, eVideoFormat format);
    ~CTestPattern();

    // This method returns the name of the test pattern
    char *GetName();

    // This method returns the type of content in the test pattern
    eTypeTestPattern GetType();

    // This method returns the video format of the test pattern
    eVideoFormat GetVideoFormat();

    // This method allows to add a new color bar to the test pattern
    int AddColorBar(unsigned short int left, unsigned short int right, unsigned short int top, unsigned short int bottom, eColor color, BOOL YUV, unsigned char R_Y, unsigned char G_U, unsigned char B_V);

    // This method returns the (first) bar having the color searched
    CColorBar *GetFirstColorBar();
    
    // This method returns the next color bar of the test pattern
    CColorBar *GetNextColorBar();

    // This method returns the (first) bar having a specified color
    CColorBar *GetColorBar(eColor color);

    // This method analyzes the current overlay buffer
    void CalcCurrentPattern(short **Lines, int height, int width);

protected:
    // Name of the test pattern
    char pattern_name[64];

    // Type of content in the test pattenr
    eTypeTestPattern type_content;

    // Video format of the test pattern
    eVideoFormat video_format;

    // Number of color bars in the test pattern
    unsigned char nb_color_bars;

    // Color bars of the test pattern
    CColorBar *color_bars[MAX_COLOR_BARS];

private:
    // Index to current color bar
    int idx_color_bar;
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
    eTypeCalibration type_calibration;
    BOOL running;

private:
    BOOL step_init(eColor color, BT848_SETTING setting, int min, int max);
    BOOL step_process(short **Lines, int height, int width, BT848_SETTING setting, unsigned int sig_component);
    int last_tick_count;
    unsigned int initial_step;
    unsigned int nb_steps;
    unsigned int current_step;
    CColorBar *bar_to_study;
    int min_val;
    int max_val;
    int current_val;
    int best_val_min;
    int best_val_max;
    int min_dif;
};


// Global variables
extern CCalibration *pCalibration;


#endif