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


#include "stdafx.h"
#include "Calibration.h"
#include "resource.h"
#include "DScaler.h"
#include "Settings.h"
#include "OSD.h"
#include "DebugLog.h"


#define LIMIT(x) (((x)<0)?0:((x)>255)?255:(x))


CCalibration *pCalibration = NULL;


/////////////////////////////////////////////////////////////////////////////


CColorBar::CColorBar(unsigned short int left, unsigned short int right, unsigned short int top, unsigned short int bottom, BOOL YUV, unsigned char R_Y, unsigned char G_U, unsigned char B_V)
{ 
    left_border = left; 
    right_border = right; 
    top_border = top; 
    bottom_border = bottom;
	if (YUV)
	{
		ref_Y_val = R_Y;
		ref_U_val = G_U;
		ref_V_val = B_V;
		YUV2RGB(ref_Y_val, ref_U_val, ref_V_val, &ref_R_val, &ref_G_val, &ref_B_val);
	}
	else
	{
		ref_R_val = R_Y;
		ref_G_val = G_U;
		ref_B_val = B_V;
		RGB2YUV(ref_R_val, ref_G_val, ref_B_val, &ref_Y_val, &ref_U_val, &ref_V_val);
	}
}

void CColorBar::GetRefPixel(BOOL YUV, unsigned char *pR_Y, unsigned char *pG_U, unsigned char *pB_V)
{ 
	if (YUV)
	{
		*pR_Y = ref_Y_val;
		*pG_U = ref_U_val;
		*pB_V = ref_V_val;
	}
	else
	{
		*pR_Y = ref_R_val;
		*pG_U = ref_G_val;
		*pB_V = ref_B_val;
	}
}

void CColorBar::GetCurrentPixel(BOOL YUV, unsigned char *pR_Y, unsigned char *pG_U, unsigned char *pB_V)
{ 
	if (YUV)
	{
		*pR_Y = Y_val;
		*pG_U = U_val;
		*pB_V = V_val;
	}
	else
	{
		*pR_Y = R_val;
		*pG_U = G_val;
		*pB_V = B_val;
	}
}

void CColorBar::GetDiffPixel(BOOL YUV, int *pR_Y, int *pG_U, int *pB_V)
{
	if (YUV)
	{
		*pR_Y = Y_val - ref_Y_val;
		*pG_U = U_val - ref_U_val;
		*pB_V = V_val - ref_V_val;
	}
	else
	{
		*pR_Y = R_val - ref_R_val;
		*pG_U = G_val - ref_G_val;
		*pB_V = B_val - ref_B_val;
	}
}

int CColorBar::GetQuality(BOOL YUV)
{
	int val1, val2, val3;

	GetDiffPixel(YUV, &val1, &val2, &val3);
	if (val1 < 0)
	{
		val1 = -val1;
	}
	if (val2 < 0)
	{
		val2 = -val2;
	}
	if (val3 < 0)
	{
		val3 = -val3;
	}
//	return val1 + val2 + val3;
	return 3;
}

int CColorBar::CalcCurrentPixel(short **Lines, int height, int width)
{ 
	int left, right, top, bottom, i, j;
	unsigned int Y, U, V, nb_Y, nb_U, nb_V;
	BYTE *buf;

	left = width * left_border / 10000;
	right = width * right_border / 10000;
	top = height * top_border / 10000;
	bottom = height * bottom_border / 10000;

//	LOG("CalcCurrentPixel %d %d %d %d %d %d", height, width, left, right, top, bottom);

	Y = 0; nb_Y = 0;
	U = 0; nb_U = 0;
	V = 0; nb_V = 0;

	for (i = top ; i <= bottom ; i++)
	{
		for (j = left ; j <= right ; j++)
		{
			buf = (BYTE *)Lines[i];
			Y += buf[j*2];
			nb_Y++;
			if (j % 2)
			{
				V += buf[j*2+1];
				nb_V++;
			}
			else
			{
				U += buf[j*2+1];
				nb_U++;
			}
		}
	}

	Y_val = Y / nb_Y;
	U_val = U / nb_U;
	V_val = V / nb_V;

//	LOG("CalcCurrentPixel %d %d %d", Y_val, U_val, V_val);

	YUV2RGB(Y_val, U_val, V_val, &R_val, &G_val, &B_val);

	return 0;
}

void CColorBar::RGB2YUV(unsigned char R, unsigned char G, unsigned char B, unsigned char *pY, unsigned char *pU, unsigned char *pV)
{
}

void CColorBar::YUV2RGB(unsigned char Y, unsigned char U, unsigned char V, unsigned char *pR, unsigned char *pG, unsigned char *pB)
{
	int y, cr, cb, r, g, b;

	cb = U - 128;
	cr = V - 128;
	y = Y - 16;

	r = ( 76284*y + 104595*cr             )>>16;
	g = ( 76284*y -  53281*cr -  25624*cb )>>16;
	b = ( 76284*y             + 132252*cb )>>16;

	*pR = LIMIT(r);
	*pG = LIMIT(g);
	*pB = LIMIT(b);
}


/////////////////////////////////////////////////////////////////////////////


CTestPattern::CTestPattern(char* name, eVideoFormat format, BOOL auto_calibr)
{
	strcpy(pattern_name, name);
	video_format = format;
	auto_calibr_possible = auto_calibr;
	nb_color_bars = 0;
	for (int i = 0 ; i < MAX_COLOR_BARS ; i++)
	{
		color_bars[i] = NULL;
	}
}

CTestPattern::~CTestPattern()
{
	for (int i = 0 ; i < MAX_COLOR_BARS ; i++)
	{
		if (color_bars[i] != NULL)
		{
			delete color_bars[i];
		}
	}
}

char *CTestPattern::GetName()
{
	return pattern_name;
}

eVideoFormat CTestPattern::GetVideoFormat()
{
	return video_format;
}

BOOL CTestPattern::IsAutoCalibrPossible()
{
	return auto_calibr_possible;
}

int CTestPattern::AddColorBar(unsigned short int left, unsigned short int right, unsigned short int top, unsigned short int bottom, BOOL YUV, unsigned char R_Y, unsigned char G_U, unsigned char B_V)
{
	int i;

	for (i = 0 ; i < MAX_COLOR_BARS ; i++)
	{
		if (color_bars[i] == NULL)
		{
			break;
		}
	}
	if (i < MAX_COLOR_BARS)
	{
		color_bars[i] = new CColorBar(left, right, top, bottom, YUV, R_Y, G_U, B_V);
		nb_color_bars++;
		return 0;
	}
	else
	{
		return -1;
	}
}

int CTestPattern::CalcCurrentPattern(short **Lines, int height, int width)
{
	for (int i = 0 ; i < MAX_COLOR_BARS ; i++)
	{
		if (color_bars[i] != NULL)
		{
			if (color_bars[i]->CalcCurrentPixel(Lines, height, width))
			{
				return -1;
			}
		}
	}
	return 0;
}


CColorBar *CTestPattern::GetFirstColorBar()
{
	for (idx_color_bar = 0 ; idx_color_bar < MAX_COLOR_BARS ; idx_color_bar++)
	{
		if (color_bars[idx_color_bar] != NULL)
		{
			return color_bars[idx_color_bar];
		}
	}
	idx_color_bar = -1;
	return NULL;
}


CColorBar *CTestPattern::GetNextColorBar()
{
	for (idx_color_bar++; idx_color_bar < MAX_COLOR_BARS ; idx_color_bar++)
	{
		if (color_bars[idx_color_bar] != NULL)
		{
			return color_bars[idx_color_bar];
		}
	}
	idx_color_bar = -1;
	return NULL;
}

	
/////////////////////////////////////////////////////////////////////////////


CCalibration::CCalibration()
{
	nb_test_patterns = 0;
	for (int i = 0 ; i < MAX_TEST_PATTERNS ; i++)
	{
		test_patterns[i] = NULL;
	}
	current_test_pattern = NULL;
	type_calibration = AUTO_CALIBR;
	running = FALSE;
}

CCalibration::~CCalibration()
{
	UnloadTestPatterns();
}

void CCalibration::LoadTestPatterns()
{
	test_patterns[nb_test_patterns] = new CTestPattern("Collector LAL - colors 4/3", FORMAT_PAL_BDGHI, FALSE);
	test_patterns[nb_test_patterns]->AddColorBar( 391, 1304, 348, 9652, FALSE, 234, 233, 231);
	test_patterns[nb_test_patterns]->AddColorBar(1565, 2477, 348, 9652, FALSE, 180, 182,  15);
	test_patterns[nb_test_patterns]->AddColorBar(2738, 3651, 348, 9652, FALSE,  17, 180, 176);
	test_patterns[nb_test_patterns]->AddColorBar(3911, 4824, 348, 9652, FALSE,  17, 181,  15);
	test_patterns[nb_test_patterns]->AddColorBar(5150, 6063, 348, 9652, FALSE, 179,  15, 177);
	test_patterns[nb_test_patterns]->AddColorBar(6323, 7236, 348, 9652, FALSE, 179,  16,  16);
	test_patterns[nb_test_patterns]->AddColorBar(7562, 8475, 348, 9652, FALSE,  17,  18, 177);
	test_patterns[nb_test_patterns]->AddColorBar(8735, 9648, 348, 9652, FALSE,  18,  18,  16);
	nb_test_patterns++;

	test_patterns[nb_test_patterns] = new CTestPattern("Collector LAL - black to white 4/3", FORMAT_PAL_BDGHI, FALSE);
	test_patterns[nb_test_patterns]->AddColorBar( 196,  847, 174, 9826, FALSE,  19,  18,  16);
	test_patterns[nb_test_patterns]->AddColorBar(1108, 1760, 174, 9826, FALSE,  38,  39,  34);
	test_patterns[nb_test_patterns]->AddColorBar(2021, 2673, 174, 9826, FALSE,  60,  61,  55);
	test_patterns[nb_test_patterns]->AddColorBar(2868, 3520, 174, 9826, FALSE,  82,  83,  77);
	test_patterns[nb_test_patterns]->AddColorBar(3781, 4433, 174, 9826, FALSE, 104, 105,  99);
	test_patterns[nb_test_patterns]->AddColorBar(4628, 5280, 174, 9826, FALSE, 125, 126, 120);
	test_patterns[nb_test_patterns]->AddColorBar(5541, 6193, 174, 9826, FALSE, 146, 147, 142);
	test_patterns[nb_test_patterns]->AddColorBar(6454, 7106, 174, 9826, FALSE, 168, 169, 163);
	test_patterns[nb_test_patterns]->AddColorBar(7301, 7953, 174, 9826, FALSE, 190, 191, 186);
	test_patterns[nb_test_patterns]->AddColorBar(8214, 1304, 174, 9826, FALSE, 212, 213, 208);
	test_patterns[nb_test_patterns]->AddColorBar(9126, 9778, 174, 9826, FALSE, 234, 233, 230);
	nb_test_patterns++;

	test_patterns[nb_test_patterns] = new CTestPattern("THX Monitor Performance - NTSC", FORMAT_NTSC, TRUE);
	test_patterns[nb_test_patterns]->AddColorBar(  97, 2229, 1167, 4313, FALSE, 251, 252, 251);
	test_patterns[nb_test_patterns]->AddColorBar(1347, 2229, 2375, 4313, FALSE, 188, 190,   0);
	test_patterns[nb_test_patterns]->AddColorBar(2611, 2229, 3556, 4313, FALSE,   0, 189, 185);
	test_patterns[nb_test_patterns]->AddColorBar(3764, 2229, 4764, 4313, FALSE,   0, 188,   0);
	test_patterns[nb_test_patterns]->AddColorBar(5028, 2229, 6000, 4313, FALSE, 187,   0, 187);
	test_patterns[nb_test_patterns]->AddColorBar(6222, 2229, 7222, 4313, FALSE, 186,   0,   0);
	test_patterns[nb_test_patterns]->AddColorBar(7458, 2229, 8472, 4313, FALSE,   0,   0, 187);
	test_patterns[nb_test_patterns]->AddColorBar(8639, 2229, 9903, 4313, FALSE,   0,   0,   0);
	test_patterns[nb_test_patterns]->AddColorBar( 111, 6708,  917, 7708, FALSE,   0,   0,   0);
	test_patterns[nb_test_patterns]->AddColorBar(1083, 6708, 1750, 7708, FALSE,  23,  24,  23);
	test_patterns[nb_test_patterns]->AddColorBar(1917, 6708, 2583, 7708, FALSE,  49,  50,  49);
	test_patterns[nb_test_patterns]->AddColorBar(6944, 6708, 7611, 7708, FALSE, 201, 203, 202);
	test_patterns[nb_test_patterns]->AddColorBar(7778, 6708, 8472, 7708, FALSE, 227, 228, 227);
	test_patterns[nb_test_patterns]->AddColorBar(8611, 6708, 9861, 7708, FALSE, 252, 253, 252);
	nb_test_patterns++;

	test_patterns[nb_test_patterns] = new CTestPattern("VE Title 18 Chapter 10", FORMAT_NTSC, FALSE);
	test_patterns[nb_test_patterns]->AddColorBar( 208, 104, 1319, 5938, FALSE, 190, 190, 190);
	test_patterns[nb_test_patterns]->AddColorBar(1597, 104, 2708, 5938, FALSE, 197, 198,   0);
	test_patterns[nb_test_patterns]->AddColorBar(3028, 104, 4139, 5938, FALSE,   0, 190, 188);
	test_patterns[nb_test_patterns]->AddColorBar(4417, 104, 5528, 5938, FALSE,   0, 190,   0);
	test_patterns[nb_test_patterns]->AddColorBar(5833, 104, 6944, 5938, FALSE, 189,   0, 188);
	test_patterns[nb_test_patterns]->AddColorBar(7250, 104, 8361, 5938, FALSE, 188,   0,   0);
	test_patterns[nb_test_patterns]->AddColorBar(8681, 104, 8403, 5938, FALSE,   0,   0, 186);
	nb_test_patterns++;

	test_patterns[nb_test_patterns] = new CTestPattern("VE Title 18 Chapter 2", FORMAT_NTSC, FALSE);
	test_patterns[nb_test_patterns]->AddColorBar( 139, 208, 1042, 9583, FALSE,   0,   0,   0);
	test_patterns[nb_test_patterns]->AddColorBar(1333, 208, 2028, 9583, FALSE,  29,  29,  29);
	test_patterns[nb_test_patterns]->AddColorBar(2292, 208, 2986, 9583, FALSE,  60,  60,  60);
	test_patterns[nb_test_patterns]->AddColorBar(3250, 208, 3944, 9583, FALSE,  90,  90,  90);
	test_patterns[nb_test_patterns]->AddColorBar(4194, 208, 4889, 9583, FALSE, 116, 118, 118);
	test_patterns[nb_test_patterns]->AddColorBar(5139, 208, 5833, 9583, FALSE, 150, 150, 150);
	test_patterns[nb_test_patterns]->AddColorBar(6111, 208, 6806, 9583, FALSE, 177, 179, 179);
	test_patterns[nb_test_patterns]->AddColorBar(7083, 208, 7778, 9583, FALSE, 206, 209, 205);
	test_patterns[nb_test_patterns]->AddColorBar(8028, 208, 8722, 9583, FALSE, 236, 236, 236);
	test_patterns[nb_test_patterns]->AddColorBar(8972, 208, 9806, 9583, FALSE, 254, 254, 254);
	nb_test_patterns++;
}

void CCalibration::UnloadTestPatterns()
{
	for (int i = 0 ; i < MAX_TEST_PATTERNS ; i++)
	{
		if (test_patterns[i] != NULL)
		{
			delete test_patterns[i];
			test_patterns[i] = NULL;
		}
	}
	nb_test_patterns = 0;
}

void CCalibration::UpdateMenu(HMENU hMenu)
{
    HMENU           hMenuPatterns;
    MENUITEMINFO    MenuItemInfo;
    int             i, j;
	char			*name;

    hMenuPatterns = GetPatternsSubmenu();
    if (hMenuPatterns == NULL) return;

    i = GetMenuItemCount(hMenuPatterns) - 1;
    while (i>=2)
    {
        RemoveMenu(hMenuPatterns, i, MF_BYPOSITION);
        i--;
    }

	for (i=0,j=2 ; i < MAX_TEST_PATTERNS ; i++)
	{
		if (test_patterns[i] != NULL)
        {
			name = test_patterns[i]->GetName();
			if (strlen (name) > 0)
			{
	            MenuItemInfo.cbSize = sizeof (MenuItemInfo);
		        MenuItemInfo.fType = MFT_STRING;
			    MenuItemInfo.dwTypeData = name;
				MenuItemInfo.cch = strlen (name);

	            MenuItemInfo.fMask = MIIM_TYPE | MIIM_ID;
		        MenuItemInfo.wID = IDM_PATTERN_SELECT + i + 1;
			    InsertMenuItem(hMenuPatterns, j, TRUE, &MenuItemInfo);

				j++;
			}
        }
    }
}

void CCalibration::SetMenu(HMENU hMenu)
{
    HMENU   hMenuPatterns;
    int     i, j;
    int     NbItems;
	char	*name;

    hMenuPatterns = GetPatternsSubmenu();
    if (hMenuPatterns == NULL) return;

    NbItems = GetMenuItemCount(hMenuPatterns);
    for (i=0,j=2 ; (j<NbItems) && (i<MAX_TEST_PATTERNS) ; i++)
    {
		if (test_patterns[i] != NULL)
        {
			name = test_patterns[i]->GetName();
			if (strlen (name) > 0)
			{
				EnableMenuItem(hMenuPatterns, j, running ? MF_BYPOSITION | MF_GRAYED : MF_BYPOSITION | MF_ENABLED);
//				EnableMenuItem(hMenuPatterns, j, (running || (current_test_pattern->GetVideoFormat() != Setting_GetValue(BT848_GetSetting(TVFORMAT)))) ? MF_BYPOSITION | MF_GRAYED : MF_BYPOSITION | MF_ENABLED);
				CheckMenuItem(hMenuPatterns, j, (current_test_pattern == test_patterns[i]) ? MF_BYPOSITION | MF_CHECKED : MF_BYPOSITION | MF_UNCHECKED);
				j++;
			}
        }
    }
	
	EnableMenuItem(hMenu, IDM_START_AUTO_CALIBRATION, (running || (current_test_pattern == NULL) || !current_test_pattern->IsAutoCalibrPossible()) ? MF_GRAYED : MF_ENABLED);
	EnableMenuItem(hMenu, IDM_START_MANUAL_CALIBRATION, (running || (current_test_pattern == NULL)) ? MF_GRAYED : MF_ENABLED);
	EnableMenuItem(hMenu, IDM_STOP_CALIBRATION, (!running || (current_test_pattern == NULL)) ? MF_GRAYED : MF_ENABLED);
}

void CCalibration::SelectTestPattern(int num)
{
	if (test_patterns[num] != NULL)
	{
		current_test_pattern = test_patterns[num];
	}
	else
	{
		current_test_pattern = NULL;
	}
}

CTestPattern *CCalibration::GetCurrentTestPattern()
{
	return current_test_pattern;
}

void CCalibration::Start(eTypeCalibration type)
{
	type_calibration = type;
	running = TRUE;
//    OSD_ShowInfosScreen(hWnd, 3, 0);
}

void CCalibration::Stop()
{
	running = FALSE;
//    OSD_Clear(hWnd);
}

BOOL CCalibration::IsRunning()
{
	return running;
}

eTypeCalibration CCalibration::GetType()
{
	return type_calibration;
}
