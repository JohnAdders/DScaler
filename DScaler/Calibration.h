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


#define	MAX_COLOR_BARS		15
#define	MAX_TEST_PATTERNS	20


enum eTypeCalibration {
	AUTO_CALIBR,
	MANUAL_CALIBR,
};


class CColorBar
{
public:
    CColorBar(unsigned short int left, unsigned short int right, unsigned short int top, unsigned short int bottom, BOOL YUV, unsigned char R_Y, unsigned char G_U, unsigned char B_V);
	void GetRefPixel(BOOL YUV, unsigned char *pR_Y, unsigned char *pG_U, unsigned char *pB_V);
	void GetCurrentPixel(BOOL YUV, unsigned char *pR_Y, unsigned char *pG_U, unsigned char *pB_V);
	void GetDiffPixel(BOOL YUV, int *pR_Y, int *pG_U, int *pB_V);
	int GetQuality(BOOL YUV);
	int CalcCurrentPixel(short **Lines, int height, int width);
protected: 
    unsigned short int left_border;		// range between 0 and 10000
    unsigned short int right_border;	// range between 0 and 10000
    unsigned short int top_border;		// range between 0 and 10000
    unsigned short int bottom_border;	// range between 0 and 10000
	unsigned char ref_Y_val;
	unsigned char ref_U_val;
	unsigned char ref_V_val;
	unsigned char ref_R_val;
	unsigned char ref_G_val;
	unsigned char ref_B_val;
	unsigned char Y_val;
	unsigned char U_val;
	unsigned char V_val;
	unsigned char R_val;
	unsigned char G_val;
	unsigned char B_val;
private:
	void RGB2YUV(unsigned char R, unsigned char G, unsigned char B, unsigned char *pY, unsigned char *pU, unsigned char *pV);
	void YUV2RGB(unsigned char Y, unsigned char U, unsigned char V, unsigned char *pR, unsigned char *pG, unsigned char *pB);
};


class CTestPattern
{
public:
	CTestPattern(char* name, eVideoFormat format, BOOL auto_calibr);
	~CTestPattern();
	char *GetName();
	eVideoFormat GetVideoFormat();
	BOOL IsAutoCalibrPossible();
	int AddColorBar(unsigned short int left, unsigned short int right, unsigned short int top, unsigned short int bottom, BOOL YUV, unsigned char R_Y, unsigned char G_U, unsigned char B_V);
	int CalcCurrentPattern(short **Lines, int height, int width);
	CColorBar *GetFirstColorBar();
	CColorBar *GetNextColorBar();
protected:
	char pattern_name[64];
	eVideoFormat video_format;
	BOOL auto_calibr_possible;
	unsigned char nb_color_bars;
	CColorBar *color_bars[MAX_COLOR_BARS];
private:
	int idx_color_bar;
};


class CCalibration
{
public:
	CCalibration();
	~CCalibration();
	void LoadTestPatterns();
	void UnloadTestPatterns();
	void UpdateMenu(HMENU hMenu);
	void SetMenu(HMENU hMenu);
	void SelectTestPattern(int num);
	CTestPattern *GetCurrentTestPattern();
	void Start(eTypeCalibration type);
	void Stop();
	BOOL IsRunning();
	eTypeCalibration GetType();
protected:
	int nb_test_patterns;
	CTestPattern *test_patterns[MAX_TEST_PATTERNS];
	CTestPattern *current_test_pattern;
	eTypeCalibration type_calibration;
	BOOL running;
};


extern CCalibration *pCalibration;


#endif