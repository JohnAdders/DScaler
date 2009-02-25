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
//
// Change Log
//
// $Log: not supported by cvs2svn $
// Revision 1.4  2003/01/19 11:09:11  laurentg
// New methods GetInitialWidth and GetInitialHeight to store the initial size before resizing in DScaler (for stills)
//
// Revision 1.3  2003/01/18 10:52:11  laurentg
// SetOverscan renamed SetAspectRatioData
// Unnecessary call to SetOverscan deleted
// Overscan setting specific to calibration deleted
//
// Revision 1.2  2002/11/01 13:09:19  laurentg
// Management of the still capture context slightly updated - works now even with stills in memory
//
// Revision 1.1  2002/10/29 20:58:11  laurentg
// Calibration source cut in Calibration + Pattern
//
//
/////////////////////////////////////////////////////////////////////////////

/** 
 * @file pattern.h pattern Header file
 */
 
#ifndef __PATTERN_H___
#define __PATTERN_H___


#include "StillSource.h"


//#define TEST_CONV_COLORSPACE


/// Define all types of content for test pattern
enum eTypeContentPattern {
    PAT_RANGE_OF_GRAY = 0,
    PAT_COLOR,
    PAT_GRAY_AND_COLOR,
    PAT_UNKNOWN,
};


enum eTypeDraw {
    DRAW_BORDER = 0,
    DRAW_FILLED,
    DRAW_LINEH,
    DRAW_LINEV,
    DRAW_LINEX,
    DRAW_GRADATIONH,
    DRAW_GRADATIONV,
    DRAW_NO,
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


/** Object representing a color bar in a test pattern

    A color bar is a rectangular zone from the test pattern
    in which all the pixels have same theoritical color.
*/
class CColorBar
{
public:
    CColorBar(unsigned short int left, unsigned short int right, unsigned short int top, unsigned short int bottom, eTypeDraw type_draw, int param1_draw, int param2_draw, BOOL YUV, unsigned char R_Y, unsigned char G_U, unsigned char B_V, unsigned char R_Y_2, unsigned char G_U_2, unsigned char B_V_2);
    CColorBar(CColorBar* pColorBar);

    /// This method returns the position of the color bar
    void GetPosition(unsigned short int* left, unsigned short int* right, unsigned short int* top, unsigned short int* bottom);

    /// This method returns the type of draw for the color bar
    eTypeDraw GetTypeDraw(int* pParam1Draw, int* pParam2Draw);

    /// This methode returns the reference color
    void GetRefColor(BOOL YUV, unsigned char* pR_Y, unsigned char* pG_U, unsigned char* pB_V);

    /// This methode returns the second reference color
    void GetRefColor2(BOOL YUV, unsigned char* pR_Y, unsigned char* pG_U, unsigned char* pB_V);

    /// This methode returns the calculated average color
    BOOL GetCurrentAvgColor(BOOL YUV, unsigned char* pR_Y, unsigned char* pG_U, unsigned char* pB_V);

    /// This methode returns the delta between reference color and calculated average color
    BOOL GetDeltaColor(BOOL YUV, int* pR_Y, int* pG_U, int* pB_V, int* pTotal);

    /** This method analyzed the overlay buffer to calculate average color
        in the zone defined by the color bar
    */
    BOOL CalcAvgColor(BOOL reinit, unsigned int nb_calc_needed, TDeinterlaceInfo* pInfo);

    BOOL GetMinColor(unsigned char* pY, unsigned char* pU, unsigned char* pV);

    BOOL GetMaxColor(unsigned char* pY, unsigned char* pU, unsigned char* pV);

    void Draw(BYTE* Buffer, int Pitch, int Height, int Width, int Overscan, int LCrop, int RCrop);

#ifdef TEST_CONV_COLORSPACE
    /// Convert RGB to YUV
    void RGB2YUV(unsigned char R, unsigned char G, unsigned char B, unsigned char* pY, unsigned char* pU, unsigned char* pV);

    /// Convert YUV to RGB
    void YUV2RGB(unsigned char Y, unsigned char U, unsigned char V, unsigned char* pR, unsigned char* pG, unsigned char* pB);
#endif

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

    /// Calculated average value in the corresponding zone of the overlay available in the YUV colorspace
    BOOL YUV_val_available;

    /// Calculated average Y value in the corresponding zone of the overlay
    unsigned char Y_val;

    /// Calculated average U value in the corresponding zone of the overlay
    unsigned char U_val;

    /// Calculated average V value in the corresponding zone of the overlay
    unsigned char V_val;

    /// Calculated average value in the corresponding zone of the overlay available in the RGB colorspace
    BOOL RGB_val_available;

    /// Calculated average R value in the corresponding zone of the overlay
    unsigned char R_val;

    /// Calculated average G value in the corresponding zone of the overlay
    unsigned char G_val;

    /// Calculated average B value in the corresponding zone of the overlay
    unsigned char B_val;

    /// Second value for luminance
    unsigned char ref_Y_val2;

    /// Second value for saturation U
    unsigned char ref_U_val2;

    /// Second value for saturation V
    unsigned char ref_V_val2;

    /// Second value for red component
    unsigned char ref_R_val2;

    /// Second value for green component
    unsigned char ref_G_val2;

    /// Second value for blue component
    unsigned char ref_B_val2;

    /// Type of draw
    eTypeDraw m_TypeDraw;

    /// Parameters
    int m_Param1Draw;
    int m_Param2Draw;

private:
#ifndef TEST_CONV_COLORSPACE
    /// Convert RGB to YUV
    void RGB2YUV(unsigned char R, unsigned char G, unsigned char B, unsigned char* pY, unsigned char* pU, unsigned char* pV);

    /// Convert YUV to RGB
    void YUV2RGB(unsigned char Y, unsigned char U, unsigned char V, unsigned char* pR, unsigned char* pG, unsigned char* pB);
#endif

    unsigned int component_cpt[3];
    unsigned int cpt_nb;
    BOOL min_available;
    unsigned char component_min[3];
    BOOL max_available;
    unsigned char component_max[3];
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
    void Draw(TDeinterlaceInfo* pInfo);

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
    CTestPattern(char* name, int width, int height);
    CTestPattern(LPCSTR FileName);
    ~CTestPattern();

    /// This method returns the name of the test pattern
    char* GetName();

    int GetInitialWidth() {return GetWidth();};
    int GetInitialHeight() {return GetHeight();};

    /// This method returns the width of the test pattern
    int GetWidth();

    /// This method returns the height (number of lines) of the test pattern
    int GetHeight();

    void SetSize(int width, int height);

    /// This method allows to create a new sub-pattern to the test pattern
    /// which is a merge of all the others sub-patterns
    void CreateGlobalSubPattern();

    /** This method determines the type of content in the test pattern
        going all over the sub-patterns
    */
    eTypeContentPattern DetermineTypeContent();

    /// This method returns the (first) sub-pattern allowing to adjust particular settings
    CSubPattern* GetSubPattern(eTypeAdjust type_adjust);

    void Draw(BYTE* buffer, int Pitch);

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


class CPatternHelper : public CStillSourceHelper
{
public:
    CPatternHelper(CStillSource* pParent);
    BOOL OpenMediaFile(LPCSTR FileName);
    void SaveSnapshot(LPCSTR FilePath, int Height, int Width, BYTE* pOverlay, LONG OverlayPitch, char* Context);
};


extern long LeftCropping;
extern long RightCropping;


#endif
