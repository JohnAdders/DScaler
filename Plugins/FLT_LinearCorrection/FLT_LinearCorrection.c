/////////////////////////////////////////////////////////////////////////////
// FLT_LinearCorrection.c
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Laurent Garnier.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DS_Filter.h"
#include "math.h"

#define	MAX_HEIGHT	576
#define	MAX_WIDTH	768

#define	Y_BLACK		16
#define	UV_BLACK	128

FILTER_METHOD LinearCorrMethod;

typedef struct _BlendStruct
{
	short	pixel1Y;	// The first pixel to use to calculate luminance
	short	pixel2Y;	// The second pixel to use to calculate luminance
	short	pixel1UV;	// The first pixel to use to calculate color
	short	pixel2UV;	// The second pixel to use to calculate color
	short	coef1;		// Coef to apply to pixel1
						// double value between 0 and 1 multiplied by 1000 to use integer
	short	coef2;		// Coef to apply to pixel2
						// double value between 0 and 1 multiplied by 1000 to use integer
} BlendStruct;

int PictureWidth = -1;
int PictureHeight = -1;
int NbPixelsPerLineTab[MAX_HEIGHT];
BlendStruct LinearFilterTab[MAX_WIDTH+1][MAX_WIDTH];
BYTE TmpBuf[MAX_WIDTH*2];

BOOL DoOnlyMasking = FALSE;
int MaskType = 0;
int MaskParam1 = 0;
int MaskParam2 = 0;
int MaskParam3 = 0;
int MaskParam4 = 0;

void UpdLinearFilterTables(int Width)
{
	int i, j;
	double Start, End;
	double pixel, pixel_before, pixel_after;
	short pixel1Y, pixel2Y, pixel1UV, pixel2UV;

	for (i=0 ; i<=Width ; i++)
	{
		Start = (double)(Width - i) / 2.0;
		End = Start + i - 1;
		for (j=0 ; j<Width ; j++)
		{
			if ((i == 0) || ((double)j < Start) || ((double)j > End))
			{
				LinearFilterTab[i][j].pixel1Y = -1;
				LinearFilterTab[i][j].pixel2Y = -1;
				LinearFilterTab[i][j].pixel1UV = -1;
				LinearFilterTab[i][j].pixel2UV = -1;
				LinearFilterTab[i][j].coef1 = 0;
				LinearFilterTab[i][j].coef2 = 0;
			}
			else
			{
				if (i > 1)
				{
					pixel = ((double)j - Start) / (double)i * (double)Width;
				}
				else
				{
					pixel = (double)(Width - 1) / 2.0;
				}
				pixel_before = floor(pixel);
				pixel_after = ceil(pixel);

				pixel1Y = (short)pixel_before;
				pixel2Y = (short)pixel_after;
				LinearFilterTab[i][j].pixel1Y = pixel1Y * 2;
				LinearFilterTab[i][j].pixel2Y = pixel2Y * 2;
	
				if (pixel1Y == pixel2Y)
				{
					if ((j % 2) == (pixel1Y % 2))
					{
						pixel1UV = pixel1Y;
					}
					else if ((j % 2) == 0)
					{
						pixel1UV = pixel1Y - 1;
					}
					else
					{
						pixel1UV = pixel1Y + 1;
					}
					pixel2UV = pixel1UV;
				}
				else
				{
					if ((j % 2) == (pixel1Y % 2))
					{
						if ((j % 2) == 0)
						{
							pixel1UV = pixel1Y;
							pixel2UV = pixel1Y;
						}
						else
						{
							pixel1UV = pixel1Y;
							pixel2UV = pixel2Y + 1;
						}
					}
					else if ((j % 2) == (pixel2Y % 2))
					{
						if ((j % 2) == 0)
						{
							pixel1UV = pixel1Y - 1;
							pixel2UV = pixel2Y;
						}
						else
						{
							pixel1UV = pixel2Y;
							pixel2UV = pixel2Y;
						}
					}
				}
				LinearFilterTab[i][j].pixel1UV = pixel1UV * 2 + 1;
				LinearFilterTab[i][j].pixel2UV = pixel2UV * 2 + 1;

				if (pixel_before < pixel_after)
				{
					LinearFilterTab[i][j].coef1 = (short)ceil((pixel_after - pixel) * 1024.0 - 0.5);
					LinearFilterTab[i][j].coef2 = (short)ceil((pixel - pixel_before) * 1024.0 - 0.5);
				}
				else
				{
					LinearFilterTab[i][j].coef1 = 512;	// 0.5 * 1024
					LinearFilterTab[i][j].coef2 = 512;	// 0.5 * 1024
				}
			}
		}
	}
}

void UpdNbPixelsPerLineTable(int Height, int Width)
{
	int x;
	double val1, val2, val3, val4;
	double a, b;
	double val;

	switch (MaskType)
	{
	case 1: // Trapezoid
		val1 = (double)((100 - 2 * MaskParam1) * Width) / 100.0;
		val2 = (double)((100 - 2 * MaskParam2) * Width) / 100.0;
		b = val1;
		a = (val2 - b) / (double)(Height - 1);
		for (x=0 ; x<Height ; x++)
		{
			val = a * x + b;
			NbPixelsPerLineTab[x] = (int)ceil(val - 0.5);
		}
		break;

	case 2:
		val1 = (double)((100 - 2 * MaskParam1) * Width) / 100.0;
		val2 = (double)((100 - 2 * MaskParam2) * Width) / 100.0;
		val3 = (double)((100 - 2 * MaskParam3) * Width) / 100.0;
		val4 = ceil((double)((Height - 1) * MaskParam4) / 100.0 - 0.5);
		b = val1;
		a = (val2 - b) / val4;
		for (x=0 ; x<(int)val4 ; x++)
		{
			val = a * x + b;
			NbPixelsPerLineTab[x] = (int)ceil(val - 0.5);
		}
		a = (val3 - val2) / ((double)(Height - 1) - val4);
		b = val3 - a * (double)(Height - 1);
		for (x=(int)val4 ; x<Height ; x++)
		{
			val = a * x + b;
			NbPixelsPerLineTab[x] = (int)ceil(val - 0.5);
		}
		break;

	default:
		for (x=0 ; x<Height ; x++)
		{
			val = (double)Width;
			NbPixelsPerLineTab[x] = (int)ceil(val - 0.5);
		}
		break;
	}
}

void ApplyLinearFilter(BYTE* pLine, int NewWidth, MEMCPY_FUNC *pCopy)
{
	int i;
	BYTE *t;
	BlendStruct *tab = LinearFilterTab[NewWidth];

	if (DoOnlyMasking)
	{
		t = pLine;
		for (i=0 ; i<PictureWidth ; i++,t+=2,tab++)
		{
			if (tab->pixel1Y == -1)
			{
				// Color the pixel in black
				t[0] = Y_BLACK;
				t[1] = UV_BLACK;
			}
		}
	}
	else
	{
		// Build the new line
		t = TmpBuf;
		for (i=0 ; i<PictureWidth ; i++,t+=2,tab++)
		{
			if (tab->pixel1Y == -1)
			{
				// Color the pixel in black
				t[0] = Y_BLACK;
				t[1] = UV_BLACK;
			}
			else
			{
				t[0] = (pLine[tab->pixel1Y] * tab->coef1 + pLine[tab->pixel2Y] * tab->coef2) / 1024;
				t[1] = (pLine[tab->pixel1UV] * tab->coef1 + pLine[tab->pixel2UV] * tab->coef2) / 1024;
			}
		}

		// Replace the old line by the new line
		pCopy(pLine, TmpBuf, PictureWidth*2);
	}
}

BOOL LinearCorrection(DEINTERLACE_INFO *info)
{
	int i;

	if (info->Overlay == NULL)
	{
		return FALSE;
	}

	// If there is a change concerning the height or the width of the picture
	if ((info->FrameWidth != PictureWidth) || (info->FrameHeight != PictureHeight))
	{
		// Verify that the filter can manage this size of picture
		if ((info->FrameWidth > MAX_WIDTH) || (info->FrameHeight > MAX_HEIGHT))
		{
			return FALSE;
		}

		// Update the internal tables of the filter
		UpdNbPixelsPerLineTable(info->FrameHeight, info->FrameWidth);
		UpdLinearFilterTables(info->FrameWidth);
		PictureWidth = info->FrameWidth;
		PictureHeight = info->FrameHeight;
	}

	// Update each line of the picture
	for (i=0 ; i<PictureHeight ; i++)
	{
		if (NbPixelsPerLineTab[i] != PictureWidth)
		{
			ApplyLinearFilter(info->Overlay + i * info->OverlayPitch, NbPixelsPerLineTab[i], info->pMemcpy);
		}
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING FLT_LinearCorrSettings[FLT_LINEAR_CORR_SETTING_LASTONE] =
{
	{
		"Linear Correction Filter", ONOFF, 0, &(LinearCorrMethod.bActive),
		FALSE, 0, 1, 1, 1,
		NULL,
		"LinearCorrectionFilter", "UseLinearCorrFilter", NULL,
	},
	{
		"Do Only Masking", ONOFF, 0, &DoOnlyMasking,
		FALSE, 0, 1, 1, 1,
		NULL,
		"LinearCorrectionFilter", "DoOnlyMasking", NULL,
	},
	{
		"Mask Type", NUMBER, 0, &MaskType,
		0, 0, 2, 1, 1,
		NULL,
		"LinearCorrectionFilter", "MaskType", NULL,
	},
	{
		"Mask Parameter 1", NUMBER, 0, &MaskParam1,
		0, 0, 100, 1, 1,
		NULL,
		"LinearCorrectionFilter", "MaskParam1", NULL,
	},
	{
		"Mask Parameter 2", NUMBER, 0, &MaskParam2,
		0, 0, 100, 1, 1,
		NULL,
		"LinearCorrectionFilter", "MaskParam2", NULL,
	},
	{
		"Mask Parameter 3", NUMBER, 0, &MaskParam3,
		0, 0, 100, 1, 1,
		NULL,
		"LinearCorrectionFilter", "MaskParam3", NULL,
	},
	{
		"Mask Parameter 4", NUMBER, 0, &MaskParam4,
		0, 0, 100, 1, 1,
		NULL,
		"LinearCorrectionFilter", "MaskParam4", NULL,
	},
};

FILTER_METHOD LinearCorrMethod =
{
	sizeof(FILTER_METHOD),
	FILTER_CURRENT_VERSION,
	"Linear Correction Filter",
	"&Linear Correction (experimental)",
	FALSE,
	FALSE,
	LinearCorrection, 
	0,
	TRUE,
	NULL,
	NULL,
	NULL,
	FLT_LINEAR_CORR_SETTING_LASTONE,
	FLT_LinearCorrSettings,
	WM_FLT_LINEAR_CORR_GETVALUE - WM_USER,
};


__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo(long CpuFeatureFlags)
{
	return &LinearCorrMethod;
}

