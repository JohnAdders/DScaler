/////////////////////////////////////////////////////////////////////////////
// $Id: RegSpy.cpp,v 1.2 2002-12-04 00:20:32 atnak Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Atsushi Nakagawa.  All rights reserved.
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.1  2002/12/03 20:51:42  atnak
// Added new app for monitoring register settings while other programs are running
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "../RegLog/GenericCard.h"
#include "../DScaler/BT848_Defines.h"
#include "../DScaler/CX2388X_Defines.h"
#include "../DScaler/SAA7134_Defines.h"

enum
{
    REG_FONT_HEIGHT     = 12,
    MAX_LOG_STATES      = 6,
    REG_REFRESH_TICKS   = 200,
};


typedef struct _Register    TRegister;
typedef struct _Chip        TChip;
typedef struct _Source      TSource;

typedef void (__cdecl REGSPYFUNC)(TRegister** hRegisterListTail);


struct _Chip
{
    char*       Name;
    WORD        VendorId;
    WORD        DeviceId;
    REGSPYFUNC* SetupFunction;
};


struct _Source
{
    _Source*    Next;
    TChip*      Chip;
    int         DeviceIndex;
    DWORD       SubSystemId;
};


struct _Register
{
    _Register*  Next;
    DWORD       Offset;
    char*       Name;
    BYTE        Size;
    DWORD       LastValue;
    DWORD       LogStates[MAX_LOG_STATES];
    BYTE        FadeTicks;
    BOOL        bDraw;
};


#define AddBRegister(Reg) \
    { \
        *hRegisterListTail = (TRegister*)malloc(sizeof(TRegister)); \
        if(*hRegisterListTail != NULL) \
        { \
            (*hRegisterListTail)->Name = #Reg; \
            (*hRegisterListTail)->Offset = Reg; \
            (*hRegisterListTail)->Size = 1; \
            (*hRegisterListTail)->bDraw = FALSE; \
            *(hRegisterListTail = &(*hRegisterListTail)->Next) = NULL; \
        } \
    }
#define AddWRegister(Reg) \
    { \
        *hRegisterListTail = (TRegister*)malloc(sizeof(TRegister)); \
        if(*hRegisterListTail != NULL) \
        { \
            (*hRegisterListTail)->Name = #Reg; \
            (*hRegisterListTail)->Offset = Reg; \
            (*hRegisterListTail)->Size = 2; \
            (*hRegisterListTail)->bDraw = FALSE; \
            *(hRegisterListTail = &(*hRegisterListTail)->Next) = NULL; \
        } \
    }
#define AddDWRegister(Reg) \
    { \
        *hRegisterListTail = (TRegister*)malloc(sizeof(TRegister)); \
        if(*hRegisterListTail != NULL) \
        { \
            (*hRegisterListTail)->Name = #Reg; \
            (*hRegisterListTail)->Offset = Reg; \
            (*hRegisterListTail)->Size = 4; \
            (*hRegisterListTail)->bDraw = FALSE; \
            *(hRegisterListTail = &(*hRegisterListTail)->Next) = NULL; \
        } \
    }


void __cdecl BT848RegSpy(TRegister** hRegisterListTail)
{
    AddBRegister(BT848_IFORM);
    AddBRegister(BT848_FCNTR);
    AddBRegister(BT848_PLL_F_LO);
    AddBRegister(BT848_PLL_F_HI);
    AddBRegister(BT848_PLL_XCI);
    AddBRegister(BT848_TGCTRL);
    AddBRegister(BT848_TDEC);
    AddBRegister(BT848_E_CROP);
    AddBRegister(BT848_O_CROP);
    AddBRegister(BT848_E_VDELAY_LO);
    AddBRegister(BT848_O_VDELAY_LO);
    AddBRegister(BT848_E_VACTIVE_LO);
    AddBRegister(BT848_O_VACTIVE_LO);
    AddBRegister(BT848_E_HDELAY_LO);
    AddBRegister(BT848_O_HDELAY_LO);
    AddBRegister(BT848_E_HACTIVE_LO);
    AddBRegister(BT848_O_HACTIVE_LO);
    AddBRegister(BT848_E_HSCALE_HI);
    AddBRegister(BT848_O_HSCALE_HI);
    AddBRegister(BT848_E_HSCALE_LO);
    AddBRegister(BT848_O_HSCALE_LO);
    AddBRegister(BT848_BRIGHT);
    AddBRegister(BT848_E_CONTROL);
    AddBRegister(BT848_O_CONTROL);
    AddBRegister(BT848_CONTRAST_LO);
    AddBRegister(BT848_SAT_U_LO);
    AddBRegister(BT848_SAT_V_LO);
    AddBRegister(BT848_HUE);
    AddBRegister(BT848_E_SCLOOP);
    AddBRegister(BT848_O_SCLOOP);
    AddBRegister(BT848_WC_UP);
    AddBRegister(BT848_WC_DOWN);
    AddBRegister(BT848_VTOTAL_LO);
    AddBRegister(BT848_VTOTAL_HI);
    AddBRegister(BT848_DVSIF);
    AddBRegister(BT848_OFORM);
    AddBRegister(BT848_E_VSCALE_HI);
    AddBRegister(BT848_O_VSCALE_HI);
    AddBRegister(BT848_E_VSCALE_LO);
    AddBRegister(BT848_O_VSCALE_LO);
    AddBRegister(BT848_ADC);
    AddBRegister(BT848_E_VTC);
    AddBRegister(BT848_O_VTC);
    AddBRegister(BT848_COLOR_FMT);
    AddBRegister(BT848_COLOR_CTL);
    AddBRegister(BT848_CAP_CTL);
    AddBRegister(BT848_VBI_PACK_SIZE);
    AddBRegister(BT848_VBI_PACK_DEL);
    AddDWRegister(BT848_INT_MASK);
    AddDWRegister(BT848_GPIO_OUT_EN);
    AddDWRegister(BT848_GPIO_OUT_EN_HIBYTE);
    AddDWRegister(BT848_GPIO_DATA);
    AddDWRegister(BT848_RISC_STRT_ADD);
    AddWRegister(BT848_GPIO_DMA_CTL);
}


void __cdecl CX2388xRegSpy(TRegister** hRegisterListTail)
{
    AddDWRegister(CX2388X_DEVICE_STATUS);
    AddDWRegister(CX2388X_VIDEO_INPUT);
    AddDWRegister(CX2388X_TEMPORAL_DEC);
    AddDWRegister(CX2388X_AGC_BURST_DELAY);
    AddDWRegister(CX2388X_BRIGHT_CONTRAST); 
    AddDWRegister(CX2388X_UVSATURATION);    
    AddDWRegister(CX2388X_HUE);             
    AddDWRegister(CX2388X_WHITE_CRUSH);
    AddDWRegister(CX2388X_PIXEL_CNT_NOTCH);
    AddDWRegister(CX2388X_HORZ_DELAY_EVEN);
    AddDWRegister(CX2388X_HORZ_DELAY_ODD);
    AddDWRegister(CX2388X_VERT_DELAY_EVEN);
    AddDWRegister(CX2388X_VERT_DELAY_ODD);
    AddDWRegister(CX2388X_VDELAYCCIR_EVEN);
    AddDWRegister(CX2388X_VDELAYCCIR_ODD);
    AddDWRegister(CX2388X_HACTIVE_EVEN);
    AddDWRegister(CX2388X_HACTIVE_ODD);
    AddDWRegister(CX2388X_VACTIVE_EVEN);    
    AddDWRegister(CX2388X_VACTIVE_ODD);     
    AddDWRegister(CX2388X_HSCALE_EVEN);     
    AddDWRegister(CX2388X_HSCALE_ODD);      
    AddDWRegister(CX2388X_VSCALE_EVEN);     
    AddDWRegister(CX2388X_VSCALE_ODD);      
    AddDWRegister(CX2388X_FILTER_EVEN);     
    AddDWRegister(CX2388X_FILTER_ODD);      
    AddDWRegister(CX2388X_FORMAT_2HCOMB);
    AddDWRegister(CX2388X_PLL);
    AddDWRegister(CX2388X_PLL_ADJUST);
    AddDWRegister(CX2388X_SAMPLERATECONV);  
    AddDWRegister(CX2388X_SAMPLERATEFIFO);  
    AddDWRegister(CX2388X_SUBCARRIERSTEP);  
    AddDWRegister(CX2388X_SUBCARRIERSTEPDR);
    AddDWRegister(CX2388X_CAPTURECONTROL);  
    AddDWRegister(CX2388X_VIDEO_COLOR_FORMAT);
    AddDWRegister(CX2388X_VBI_SIZE);
    AddDWRegister(CX2388X_FIELD_CAP_CNT);
    AddDWRegister(CX2388X_VIP_CONFIG);
    AddDWRegister(CX2388X_VIP_CONTBRGT);
    AddDWRegister(CX2388X_VIP_HSCALE);
    AddDWRegister(CX2388X_VIP_VSCALE);
    AddDWRegister(CX2388X_VBOS);

    AddDWRegister(MO_GP0_IO);
    AddDWRegister(MO_GP1_IO);   
    AddDWRegister(MO_GP2_IO);
    AddDWRegister(MO_GP3_IO);
    AddDWRegister(MO_GPIO);
    AddDWRegister(MO_GPOE);

    AddDWRegister(0x02320d01);
    AddDWRegister(0x02320d02);
    AddDWRegister(0x02320d03);
    AddDWRegister(0x02320d04);
    AddDWRegister(0x02320d2a);
    AddDWRegister(0x02320d2b);

    AddDWRegister(AUD_DN0_FREQ);
    AddDWRegister(AUD_POLY0_DDS_CONSTANT);
    AddDWRegister(AUD_IIR1_0_SEL);
    AddDWRegister(AUD_IIR1_1_SEL);
    AddDWRegister(AUD_IIR1_2_SEL);
    AddDWRegister(AUD_IIR1_3_SEL);
    AddDWRegister(AUD_IIR1_4_SEL);
    AddDWRegister(AUD_IIR1_5_SEL);
    AddDWRegister(AUD_IIR1_0_SHIFT);
    AddDWRegister(AUD_IIR1_1_SHIFT);
    AddDWRegister(AUD_IIR1_2_SHIFT);
    AddDWRegister(AUD_IIR1_3_SHIFT);
    AddDWRegister(AUD_IIR1_4_SHIFT);
    AddDWRegister(AUD_IIR1_5_SHIFT);
    AddDWRegister(AUD_IIR2_0_SEL);
    AddDWRegister(AUD_IIR2_1_SEL);
    AddDWRegister(AUD_IIR2_2_SEL);
    AddDWRegister(AUD_IIR2_3_SEL);
    AddDWRegister(AUD_IIR3_0_SEL);
    AddDWRegister(AUD_IIR3_1_SEL);
    AddDWRegister(AUD_IIR3_2_SEL);
    AddDWRegister(AUD_IIR3_0_SHIFT);
    AddDWRegister(AUD_IIR3_1_SHIFT);
    AddDWRegister(AUD_IIR3_2_SHIFT);
    AddDWRegister(AUD_IIR4_0_SEL);
    AddDWRegister(AUD_IIR4_1_SEL);
    AddDWRegister(AUD_IIR4_2_SEL);
    AddDWRegister(AUD_IIR4_0_SHIFT);
    AddDWRegister(AUD_IIR4_1_SHIFT);
    AddDWRegister(AUD_IIR4_2_SHIFT);
    AddDWRegister(AUD_IIR4_0_CA0);
    AddDWRegister(AUD_IIR4_0_CA1);
    AddDWRegister(AUD_IIR4_0_CA2);
    AddDWRegister(AUD_IIR4_0_CB0);
    AddDWRegister(AUD_IIR4_0_CB1);
    AddDWRegister(AUD_IIR4_1_CA0);
    AddDWRegister(AUD_IIR4_1_CA1);
    AddDWRegister(AUD_IIR4_1_CA2);
    AddDWRegister(AUD_IIR4_1_CB0);
    AddDWRegister(AUD_IIR4_1_CB1);
    AddDWRegister(AUD_IIR4_2_CA0);
    AddDWRegister(AUD_IIR4_2_CA1);
    AddDWRegister(AUD_IIR4_2_CA2);
    AddDWRegister(AUD_IIR4_2_CB0);
    AddDWRegister(AUD_IIR4_2_CB1);
    AddDWRegister(AUD_HP_MD_IIR4_1);
    AddDWRegister(AUD_HP_PROG_IIR4_1);
    AddDWRegister(AUD_DN1_FREQ);
    AddDWRegister(AUD_DN1_SRC_SEL);
    AddDWRegister(AUD_DN1_SHFT);
    AddDWRegister(AUD_DN1_AFC);
    AddDWRegister(AUD_DN1_FREQ_SHIFT);
    AddDWRegister(AUD_DN2_SRC_SEL);
    AddDWRegister(AUD_DN2_SHFT);
    AddDWRegister(AUD_DN2_AFC);
    AddDWRegister(AUD_DN2_FREQ);
    AddDWRegister(AUD_DN2_FREQ_SHIFT);
    AddDWRegister(AUD_PDET_SRC);
    AddDWRegister(AUD_PDET_SHIFT);
    AddDWRegister(AUD_DEEMPH0_SRC_SEL);
    AddDWRegister(AUD_DEEMPH1_SRC_SEL);
    AddDWRegister(AUD_DEEMPH0_SHIFT);
    AddDWRegister(AUD_DEEMPH1_SHIFT);
    AddDWRegister(AUD_DEEMPH0_G0);
    AddDWRegister(AUD_DEEMPH0_A0);
    AddDWRegister(AUD_DEEMPH0_B0);
    AddDWRegister(AUD_DEEMPH0_A1);
    AddDWRegister(AUD_DEEMPH0_B1);
    AddDWRegister(AUD_DEEMPH1_G0);
    AddDWRegister(AUD_DEEMPH1_A0);
    AddDWRegister(AUD_DEEMPH1_B0);
    AddDWRegister(AUD_DEEMPH1_A1);
    AddDWRegister(AUD_DEEMPH1_B1);
    AddDWRegister(AUD_PLL_EN);
    AddDWRegister(AUD_DMD_RA_DDS);
    AddDWRegister(AUD_RATE_ADJ1);
    AddDWRegister(AUD_RATE_ADJ2);
    AddDWRegister(AUD_RATE_ADJ3);
    AddDWRegister(AUD_RATE_ADJ4);
    AddDWRegister(AUD_RATE_ADJ5);
    AddDWRegister(AUD_C2_UP_THR);
    AddDWRegister(AUD_C2_LO_THR);
    AddDWRegister(AUD_C1_UP_THR);
    AddDWRegister(AUD_C1_LO_THR);
    AddDWRegister(AUD_CTL);
    AddDWRegister(AUD_DCOC_0_SRC);
    AddDWRegister(AUD_DCOC_1_SRC);
    AddDWRegister(AUD_DCOC1_SHIFT);
    AddDWRegister(AUD_DCOC_1_SHIFT_IN0);
    AddDWRegister(AUD_DCOC_1_SHIFT_IN1);
    AddDWRegister(AUD_DCOC_PASS_IN);
    AddDWRegister(AUD_IIR4_0_SEL);
    AddDWRegister(AUD_DN1_AFC);
    AddDWRegister(AUD_DCOC_2_SRC);
    AddDWRegister(AUD_IIR4_1_SEL);
    AddDWRegister(AUD_CTL);
    AddDWRegister(AUD_DN2_SRC_SEL);
    AddDWRegister(AUD_DN2_FREQ);
    AddDWRegister(AUD_POLY0_DDS_CONSTANT);
    AddDWRegister(AUD_PHASE_FIX_CTL);
    AddDWRegister(AUD_CORDIC_SHIFT_1);
    AddDWRegister(AUD_PLL_PRESCALE);
    AddDWRegister(AUD_PLL_INT);
    AddDWRegister(AUD_AFE_12DB_EN);
}


void __cdecl SAA7134RegSpy(TRegister** hRegisterListTail)
{
    AddBRegister(SAA7134_INCR_DELAY);
    AddBRegister(SAA7134_ANALOG_IN_CTRL1);
    AddBRegister(SAA7134_ANALOG_IN_CTRL2);
    AddBRegister(SAA7134_ANALOG_IN_CTRL3);
    AddBRegister(SAA7134_ANALOG_IN_CTRL4);

    AddBRegister(SAA7134_HSYNC_START);
    AddBRegister(SAA7134_HSYNC_STOP);
    AddBRegister(SAA7134_SYNC_CTRL);
    AddBRegister(SAA7134_LUMA_CTRL);
    AddBRegister(SAA7134_DEC_LUMA_BRIGHT);
    AddBRegister(SAA7134_DEC_LUMA_CONTRAST);
    AddBRegister(SAA7134_DEC_CHROMA_SATURATION);
    AddBRegister(SAA7134_DEC_CHROMA_HUE);
    AddBRegister(SAA7134_CHROMA_CTRL1);
    AddBRegister(SAA7134_CHROMA_GAIN_CTRL);
    AddBRegister(SAA7134_CHROMA_CTRL2);
    AddBRegister(SAA7134_MODE_DELAY_CTRL);
    AddBRegister(SAA7134_ANALOG_ADC);
    AddBRegister(SAA7134_VGATE_START);
    AddBRegister(SAA7134_VGATE_STOP);
    AddBRegister(SAA7134_MISC_VGATE_MSB);
    AddBRegister(SAA7134_RAW_DATA_GAIN);
    AddBRegister(SAA7134_RAW_DATA_OFFSET);
    AddBRegister(SAA7134_STATUS_VIDEO);
    AddBRegister(SAA7134_STATUS_VIDEO_HIBYTE);

    AddDWRegister(SAA7134_GPIO_GPMODE);
    AddDWRegister(SAA7134_GPIO_GPSTATUS);

    AddBRegister(SAA7134_OFMT_VIDEO_A);
    AddBRegister(SAA7134_OFMT_DATA_A);
    AddBRegister(SAA7134_OFMT_VIDEO_B);
    AddBRegister(SAA7134_OFMT_DATA_B);
    AddBRegister(SAA7134_ALPHA_NOCLIP);
    AddBRegister(SAA7134_ALPHA_CLIP);
    AddBRegister(SAA7134_UV_PIXEL);
    AddBRegister(SAA7134_CLIP_RED);
    AddBRegister(SAA7134_CLIP_GREEN);
    AddBRegister(SAA7134_CLIP_BLUE);}


//  struct TChip:
//  {
//      char*       Name;
//      DWORD       VendorId;
//      DWORD       DeviceId;
//      REGSPYFUNC* SetupFunction;
//  }

TChip Chips[] = 
{
    {
        "BT848",
        0x109e,
        0x0350,
        BT848RegSpy,
    },
    {
        "BT849",
        0x109e,
        0x0351,
        BT848RegSpy,
    },
    {
        "BT878",
        0x109e,
        0x036e,
        BT848RegSpy,
    },
    {
        "BT878A",
        0x109e,
        0x036f,
        BT848RegSpy,
    },
    {
        "CX2388x",
        0x14F1, 
        0x8800, 
        CX2388xRegSpy,
    },
    {
        "SAA7134",
        0x1131,
        0x7134,
        SAA7134RegSpy,
    },
};



BOOL CreateRegDisplay(HWND, HDC*, HBITMAP*, HFONT*);
void DeleteRegDisplay(HDC*, HBITMAP*, HFONT*);

BOOL APIENTRY MainWindowProc(HWND hDlg, UINT uMsg, UINT wParam, LONG lParam);
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);

HWND    ghMainDialog = NULL;
HHOOK   ghKeyboardHook = NULL;


//  Creates and initializes all the variables necessary
//  for the register display in the main dialog
BOOL CreateRegDisplay(HWND hWnd, HDC* phDC, HBITMAP* phBitmap, HFONT* phFont)
{
    HDC hDC = GetDC(hWnd);
    RECT Rect;
    VOID* pvBits;

    *phFont = CreateFont(-REG_FONT_HEIGHT, 0, 0, 0, FW_NORMAL, 0, 0, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH, "Lucida Console");

    *phDC = CreateCompatibleDC(hDC);

    if(*phDC == NULL || *phDC == NULL)
    {
        DeleteRegDisplay(phDC, phBitmap, phFont);
        return FALSE;
    }

    GetClientRect(hWnd, &Rect);
    OffsetRect(&Rect, -Rect.left, -Rect.top);
    
    // Add some more for scrolling adjust
    Rect.bottom += 30;

    BITMAPINFO BitmapInfo;
    ZeroMemory(&BitmapInfo, sizeof(BITMAPINFO));
    BitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    BitmapInfo.bmiHeader.biWidth = Rect.right;
    BitmapInfo.bmiHeader.biHeight = -Rect.bottom;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 24;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    *phBitmap = CreateDIBSection(
                                    *phDC,
                                    &BitmapInfo,
                                    DIB_RGB_COLORS,
                                    &pvBits,
                                    NULL,
                                    0
                                );

    if(*phBitmap == NULL)
    {
        DeleteRegDisplay(phDC, phBitmap, phFont);
        return FALSE;
    }

    SelectObject(*phDC, *phBitmap);
    SelectObject(*phDC, *phFont);

    SetBkMode(*phDC, TRANSPARENT);

    FillRect(*phDC, &Rect, GetSysColorBrush(COLOR_3DFACE));

    ReleaseDC(hWnd, hDC);

    return TRUE;
}


//  Deletes all variables created by 
void DeleteRegDisplay(HDC* phDC, HBITMAP* phBitmap, HFONT* phFont)
{
    if(*phFont != NULL)
    {
        DeleteObject(*phFont);
        *phFont = NULL;
    }

    if(*phBitmap != NULL)
    {
        DeleteObject(*phBitmap);
        *phBitmap = NULL;
    }

    if(*phDC != NULL)
    {
        DeleteDC(*phDC);
        *phDC = NULL;
    }
}


//  Converts a number into a string represented binary
//  NOTE: lpBuffer must be at least Size * 8 + 6 in size!
void NumberToBinary(char *lpBuffer, DWORD Number, BYTE Size, DWORD Mask)
{
    BYTE    Byte, Bmask;
    WORD    Word, Wmask;
    int     i;

    *lpBuffer++ = '(';

    // Avoid endian and bit order differences
    switch(Size)
    {
    case 1:
        Byte = (BYTE)Number;
        Bmask = (BYTE)Mask;
        for(i = 0; i < 8; i++)
        {
            *lpBuffer++ = ((Bmask & (1 << i)) ? ((Byte & (1 << i)) ? '1' : '0') : '-');
        }
        break;

    case 2:
        Word = (WORD)Number;
        Wmask = (WORD)Mask;
        for(i = 0; i < 16; i++)
        {
            *lpBuffer++ = ((Mask & (1 << i)) ? ((Word & (1 << i)) ? '1' : '0') : '-');
            if(i == 7)
            {
                *lpBuffer++ = ' ';
            }
        }
        break;

    case 4:
        for(i = 0; i < 32; i++)
        {
            *lpBuffer++ = ((Mask & (1 << i)) ? ((Number & (1 << i)) ? '1' : '0') : '-');
            if((i % 8 == 7) && (i != 31))
            {
                *lpBuffer++ = ' ';
            }
        }
        break;

    default:
        break;
    }

    *lpBuffer++ = ')';
    *lpBuffer = '\0';
}


//  Saves the register list to a user prompted file
BOOL SaveToFile(HWND hOwner, TSource* pSource, TRegister* pRegisterList, BYTE nLogState)
{
    char szFilename[256];

    sprintf(szFilename, "%s_%d.txt", pSource->Chip->Name, pSource->DeviceIndex);

    OPENFILENAME OpenFileName;
    ZeroMemory(&OpenFileName, sizeof(OPENFILENAME));
    OpenFileName.lStructSize = sizeof(OPENFILENAME);
    OpenFileName.hwndOwner = hOwner;
    OpenFileName.lpstrFilter = "Text Documents (*.txt)\0*.txt\0All Files\0*.*\0\0";
    OpenFileName.lpstrCustomFilter = NULL;
    OpenFileName.nMaxCustFilter = 0;
    OpenFileName.nFilterIndex = 1;
    OpenFileName.lpstrFile = szFilename;
    OpenFileName.nMaxFile = sizeof(szFilename);
    OpenFileName.lpstrFileTitle = NULL;
    OpenFileName.lpstrInitialDir = NULL;
    OpenFileName.lpstrTitle = NULL;
    OpenFileName.Flags = OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST ;
    OpenFileName.lpstrDefExt = "txt";

    if(GetSaveFileName(&OpenFileName) == FALSE)
    {
        return FALSE;
    }

    FILE* hFile = fopen(szFilename, "w");
    if(hFile == NULL)
    {

        return FALSE;
    }

    TRegister*  pRegister;
    char        szFormat[32];
    char        szBinary[38];
    char        szHex[11];
    char        szWas[16];
    char        szSameAs[MAX_LOG_STATES*3 + 10];
    char        szName[64];
    char        cStar;
    DWORD       Value;
    DWORD*      pNewValue;
    LONG        nChanges;


    fprintf(hFile, "%s Card [%d]:\n\n", pSource->Chip->Name,    pSource->DeviceIndex);
    fprintf(hFile, "%-20s 0x%04x\n", "Vendor ID:",  pSource->Chip->VendorId);
    fprintf(hFile, "%-20s 0x%04x\n", "Device ID:", pSource->Chip->DeviceId);
    fprintf(hFile, "%-20s 0x%08x\n", "Subsystem ID:", pSource->SubSystemId);

    if(nLogState > 0)
    {
        fprintf(hFile, "\n\n%d states dumped\n", nLogState+1);
    }

    fprintf(hFile, "\n");

    for(int i(0); i < nLogState; i++)
    {
        fprintf(hFile, "----------------------------------------------------------------------------------\n\n");
        fprintf(hFile, "%s Card - State %d:\n", pSource->Chip->Name, i);

        for(pRegister = pRegisterList; pRegister != NULL; pRegister = pRegister->Next)
        {
            Value       = pRegister->LogStates[i];

            if(i + 1 == nLogState)
            {
                pNewValue = &pRegister->LastValue;
            }
            else
            {
                pNewValue = &pRegister->LogStates[i+1];
            }
            cStar       = (*pNewValue == Value) ? ' ' : '*';

            if((i > 0) && (pRegister->LogStates[i-1] != Value))
            {
                sprintf(szFormat, "(was: %%0%dx)", pRegister->Size*2);
                sprintf(szWas, szFormat, pRegister->LogStates[i-1]);
            }
            else
            {
                *szWas = '\0';
            }

            NumberToBinary(szBinary, Value, pRegister->Size, 0xFFFFFFFF);

            sprintf(szName, "%s:", pRegister->Name);
            sprintf(szFormat, "%%0%dx %c", pRegister->Size*2, cStar);
            sprintf(szHex, szFormat, Value);

            fprintf(hFile, "%-32s %-10s %-37s  %-15s\n", szName, szHex, szBinary, szWas);
        }

        if(i + 1 == nLogState)
        {
            fprintf(hFile, "\n\nChanges: State %d -> Register Dump:\n", i, i+1);
        }
        else
        {
            fprintf(hFile, "\n\nChanges: State %d -> State %d:\n", i, i+1);
        }

        nChanges = 0;
        for(pRegister = pRegisterList; pRegister != NULL; pRegister = pRegister->Next)
        {
            pNewValue   = (i+1 == nLogState ? &pRegister->LastValue : &pRegister->LogStates[i+1]);
            if(*pNewValue != pRegister->LogStates[i])
            {
                Value       = pRegister->LogStates[i];
                NumberToBinary(szBinary, Value, pRegister->Size, Value ^ *pNewValue);

                *szSameAs = '\0';
                for(int j(0); j < i; j++)
                {
                    if(pRegister->LogStates[j] == *pNewValue)
                    {
                        sprintf(szSameAs, "%s%s %d", szSameAs,
                            (*szSameAs == '\0' ? "" : ","), j);
                    }
                }
                if(*szSameAs != '\0')
                {
                    memmove(&szSameAs[8], szSameAs, strlen(szSameAs)+1);
                    memcpy(szSameAs, "(same as", 8);
                    strcat(szSameAs, ")");
                }

                sprintf(szName, "%s:", pRegister->Name);

                sprintf(szFormat, "%%0%dx", pRegister->Size*2);
                sprintf(szHex, szFormat, Value);
                fprintf(hFile, "%-32s %-8s -> ", szName, szHex);

                sprintf(szFormat, "%%0%dx", pRegister->Size*2);
                sprintf(szHex, szFormat, *pNewValue);
                fprintf(hFile, "%-8s  %-37s  %s\n", szHex, szBinary, szSameAs);
                nChanges++;
            }
        }

        fprintf(hFile, "\n%d changes\n\n\n", nChanges);
    }

    fprintf(hFile, "=================================================================================\n\n");
    fprintf(hFile, "%s Card - Register Dump:\n", pSource->Chip->Name);

    for(pRegister = pRegisterList; pRegister != NULL; pRegister = pRegister->Next)
    {
        Value       = pRegister->LastValue;

        if((nLogState > 0) && (pRegister->LogStates[nLogState-1] != Value))
        {
            sprintf(szFormat, "(was: %%0%dx)", pRegister->Size*2);
            sprintf(szWas, szFormat, pRegister->LogStates[nLogState-1]);
        }
        else
        {
            *szWas = '\0';
        }

        NumberToBinary(szBinary, Value, pRegister->Size, 0xFFFFFFFF);

        sprintf(szName, "%s:", pRegister->Name);
        sprintf(szFormat, "%%0%dx", pRegister->Size*2);
        sprintf(szHex, szFormat, Value);

        fprintf(hFile, "%-32s %-8s   %-37s  %-15s\n", szName, szHex, szBinary, szWas);
    }

    fprintf(hFile, "\nend of dump\n");
    fclose(hFile);

    return TRUE;
}


//  This call back monitors keyboard events in the thread
//  and passes them onto the main dialog
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    static LPARAM param = 0;
    if(nCode == HC_ACTION)
    {
        PostMessage(ghMainDialog, ((HIWORD(lParam) & KF_UP) ? WM_KEYUP : WM_KEYDOWN),
            wParam, lParam);
    }
    return CallNextHookEx(ghKeyboardHook, nCode, wParam, lParam);
}


//  This is the main proc that handles all events in
//  the main dialog.  Warning:  BIG function!
BOOL APIENTRY MainWindowProc(HWND hDlg, UINT uMsg, UINT wParam, LONG lParam)
{
    static CHardwareDriver* pHardwareDriver;
    static CGenericCard* pCard = NULL;
    static TSource*     pSourceList;
    static TRegister*   pRegisterList = NULL;
    static TRegister**  hRegisterListTail = &pRegisterList;
    static HDC          hRegDisplayDC = NULL;
    static HBITMAP      hRegDisplayBitmap = NULL;
    static HFONT        hRegDisplayFont = NULL;
    static HBRUSH       hLightStripeBrush = NULL;
    static CRITICAL_SECTION RegisterListMutex;
    static LONG         ScrollPos = 0;
    static LONG         ScrollMax = 0;
    static RECT         RegDisplayRect;
    static LONG         nRegDisplayHeight;
    static LONG         nRegDisplayWidth;
    static BYTE         nLogState;
    static BOOL         bFirstRead = TRUE;
    static BOOL         bRedrawAll = FALSE;
    static BOOL         bMoveUpChanges = FALSE;
    static BOOL         bResetLogState = FALSE;
    TSource*            pSource;
    TRegister*          pRegister;
    RECT                Rect;
    char                szFormat[8];
    char                szBuffer[64];
    int                 ItemIndex;

    switch(uMsg)
    {
    case WM_INITDIALOG:
        {
            pSourceList     = (TSource*)        ((void**)lParam)[0];
            pHardwareDriver = (CHardwareDriver*)((void**)lParam)[1];

            ghMainDialog = hDlg;

            HWND hWnd = GetDlgItem(hDlg, IDC_SOURCESELECT);
            for(pSource = pSourceList; pSource != NULL; pSource = pSource->Next)
            {
                sprintf(szBuffer, "%s Card [%d]", pSource->Chip->Name, pSource->DeviceIndex);
                ItemIndex = SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)szBuffer);
                SendMessage(hWnd, CB_SETITEMDATA, ItemIndex, (LPARAM)pSource);
                if(pSource == pSourceList)
                {
                    SendMessage(hWnd, CB_SETCURSEL, ItemIndex, 0);
                }
            }

            if(CreateRegDisplay(GetDlgItem(hDlg, IDC_REGDISPLAY),
                                    &hRegDisplayDC,
                                    &hRegDisplayBitmap,
                                    &hRegDisplayFont) == FALSE)
            {
                MessageBox(NULL, "Can't create display", "RegSpy", MB_OK);
                EndDialog(hDlg, FALSE);
                break;
            }

            hLightStripeBrush = CreateSolidBrush(GetSysColor(COLOR_3DFACE) + 0x000A0A0A);

            GetClientRect(GetDlgItem(hDlg, IDC_REGDISPLAY), &RegDisplayRect);

            nRegDisplayWidth = (RegDisplayRect.right - RegDisplayRect.left);
            nRegDisplayHeight = (RegDisplayRect.bottom - RegDisplayRect.top);

            // Install a keyboard hook so we can easily monitor keypresses
            ghKeyboardHook = SetWindowsHookEx(WH_KEYBOARD, KeyboardHookProc,
                NULL, GetCurrentThreadId());
            
            InitializeCriticalSection(&RegisterListMutex);

            if(pSourceList == NULL)
            {
                ItemIndex = SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)"No Card Found");
                SendMessage(hWnd, CB_SETITEMDATA, ItemIndex, NULL);
                SendMessage(hWnd, CB_SETCURSEL, ItemIndex, 0);
            }
            else
            {
                EnableWindow(hWnd, TRUE);
                SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_SOURCESELECT, CBN_SELCHANGE), 0);
            }
        }
        return FALSE;

    case WM_TIMER:
        if (wParam == IDC_REFRESH)
        {
            KillTimer(hDlg, IDC_REFRESH);
            EnterCriticalSection(&RegisterListMutex);

            if(pCard != NULL && pRegisterList != NULL)
            {
                DWORD       Value;
                BOOL        bChanged = FALSE;
                TRegister** hUpInsert = &pRegisterList;
                TRegister** hPreviousNext = &pRegisterList;
                TRegister*  pUptoRemoved = NULL;

                for(pRegister = pRegisterList;
                    pRegister != NULL;
                    pRegister = *hPreviousNext)
                {
                    switch(pRegister->Size)
                    {
                    case 1: Value = pCard->ReadByte(pRegister->Offset); break;
                    case 2: Value = pCard->ReadWord(pRegister->Offset); break;
                    case 4: Value = pCard->ReadDword(pRegister->Offset); break;
                    default: pRegister->Size = 4; Value = 0UL; break;
                    }

                    if(bFirstRead == TRUE)
                    {
                        pRegister->FadeTicks = 0;
                        pRegister->LastValue = Value;
                        pRegister->bDraw = TRUE;
                        bChanged = TRUE;
                    }
                    else if(Value != pRegister->LastValue)
                    {
                        pRegister->FadeTicks = 20;
                        pRegister->LastValue = Value;
                        pRegister->bDraw = TRUE;
                        bChanged = TRUE;

                        if(bMoveUpChanges == TRUE)
                        {
                            if(pRegister != *hUpInsert)
                            {
                                // Remove from original position
                                *hPreviousNext = pRegister->Next;

                                // Remember the last non-removed object
                                pUptoRemoved = pRegister->Next;

                                // Insert up
                                pRegister->Next = *hUpInsert;
                                *hUpInsert = pRegister;
                                hUpInsert = &pRegister->Next;
                                continue;
                            }
                        }
                    }
                    else if(pRegister->FadeTicks > 0)
                    {
                        if(--pRegister->FadeTicks == 0)
                        {
                            pRegister->bDraw = TRUE;
                            bChanged = TRUE;
                        }
                    }

                    hPreviousNext = &pRegister->Next;
                }

                bFirstRead = FALSE;

                if(bChanged == TRUE)
                {
                    if(hUpInsert != &pRegisterList)
                    {
                        // Tag all the ones that shifted as a result of others moving up
                        for(pRegister = *hUpInsert; pRegister != pUptoRemoved; pRegister = pRegister->Next)
                        {
                            pRegister->bDraw = TRUE;
                        }
                    }

                    CopyRect(&Rect, &RegDisplayRect);

                    ClientToScreen(GetDlgItem(hDlg, IDC_REGDISPLAY), ((LPPOINT)&Rect));
                    ClientToScreen(GetDlgItem(hDlg, IDC_REGDISPLAY), ((LPPOINT)&Rect) + 1);
                    ScreenToClient(hDlg, ((LPPOINT)&Rect));
                    ScreenToClient(hDlg, ((LPPOINT)&Rect) + 1);

                    InvalidateRect(hDlg, &Rect, FALSE);
                }

                SetTimer(hDlg, IDC_REFRESH, REG_REFRESH_TICKS, NULL);
            }

            LeaveCriticalSection(&RegisterListMutex);
        }
        return TRUE;

    case WM_PAINT:
        EnterCriticalSection(&RegisterListMutex);
        if(pRegisterList != NULL && hRegDisplayDC != NULL)
        {
            COLORREF    TextColor;
            RECT        TextRect;
            BOOL        bChanged = FALSE;

            SetRect(&TextRect, 0, -ScrollPos, nRegDisplayWidth, REG_FONT_HEIGHT - ScrollPos);

            for(pRegister = pRegisterList; pRegister != NULL; pRegister = pRegister->Next)
            {
                if(TextRect.bottom <= 0)
                {
                    OffsetRect(&TextRect, 0, REG_FONT_HEIGHT);
                    continue;
                }
                if(TextRect.top >= nRegDisplayHeight)
                {
                    break;
                }

                if(bRedrawAll == TRUE || pRegister->bDraw == TRUE)
                {
                    if(pRegister->FadeTicks > 0)
                    {
                        TextColor = 0x000000A0;
                    }
                    else
                    {
                        TextColor = 0x00000000;
                        if(nLogState > 0)
                        {
                            if(pRegister->LogStates[nLogState-1] != pRegister->LastValue)
                            {
                                TextColor = 0x00D00000;
                            }
                            else
                            {
                                for(int i(0); i < nLogState-1; i++)
                                {
                                    if(pRegister->LogStates[i] != pRegister->LastValue)
                                    {
                                        TextColor = 0x00800000;
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    sprintf(szFormat, "%%0%dx", pRegister->Size*2);
                    sprintf(szBuffer, szFormat, pRegister->LastValue);

                    HBRUSH hBackgroundBrush = GetSysColorBrush(COLOR_3DFACE);
                    if((TextRect.top + ScrollPos) % (REG_FONT_HEIGHT * 2) != 0)
                    {
                        if(hLightStripeBrush != NULL)
                        {
                            hBackgroundBrush = hLightStripeBrush;
                        }
                    }

                    FillRect(hRegDisplayDC, &TextRect, hBackgroundBrush);

                    SetTextColor(hRegDisplayDC, TextColor);
                    SetTextAlign(hRegDisplayDC, TA_LEFT);
                    TextOut(hRegDisplayDC, TextRect.left, TextRect.top, pRegister->Name, strlen(pRegister->Name));
                    SetTextAlign(hRegDisplayDC, TA_RIGHT);
                    TextOut(hRegDisplayDC, TextRect.right, TextRect.top, szBuffer, strlen(szBuffer));

                    pRegister->bDraw = FALSE;
                    bChanged = TRUE;
                }

                OffsetRect(&TextRect, 0, REG_FONT_HEIGHT);
            }

            LeaveCriticalSection(&RegisterListMutex);

            PAINTSTRUCT PaintStruct;
            HWND hWnd = GetDlgItem(hDlg, IDC_REGDISPLAY);

            HDC hDC = BeginPaint(hWnd, &PaintStruct);
            BitBlt(hDC, RegDisplayRect.left, RegDisplayRect.top,
                nRegDisplayWidth, nRegDisplayHeight,
                hRegDisplayDC, 0, 0, SRCCOPY);
            EndPaint(hWnd, &PaintStruct);
        }
        else
        {
            LeaveCriticalSection(&RegisterListMutex);
        }
        break;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
        case IDCANCEL:
            KillTimer(hDlg, IDC_REFRESH);
            EnterCriticalSection(&RegisterListMutex);

            if(pCard != NULL)
            {
                delete pCard;
                pCard = NULL;
            }

            while(pRegister = pRegisterList)
            {
                pRegisterList = pRegister->Next;
                free(pRegister);
            }

            LeaveCriticalSection(&RegisterListMutex);
            DeleteCriticalSection(&RegisterListMutex);

            if(hLightStripeBrush != NULL)
            {
                DeleteObject(hLightStripeBrush);
            }

            DeleteRegDisplay(&hRegDisplayDC,
                                &hRegDisplayBitmap,
                                &hRegDisplayFont);

            EndDialog(hDlg, TRUE);
            return TRUE;

        case IDC_SOURCESELECT:
            if(HIWORD(wParam) == CBN_SELCHANGE)
            {
                KillTimer(hDlg, IDC_REFRESH);
                EnterCriticalSection(&RegisterListMutex);

                HWND hWnd = GetDlgItem(hDlg, IDC_SOURCESELECT);

                ItemIndex = SendMessage(hWnd, CB_GETCURSEL, 0, 0);
                pSource = (TSource*)SendMessage(hWnd, CB_GETITEMDATA, ItemIndex, 0);

                nLogState   = 0;
                bFirstRead  = TRUE;

                if(pSource != NULL)
                {
                    SetDlgItemText(hDlg, IDC_CHIPTYPE, pSource->Chip->Name);

                    sprintf(szBuffer,"0x%04X", pSource->Chip->VendorId);
                    SetDlgItemText(hDlg, IDC_VENDORID, szBuffer);

                    sprintf(szBuffer,"0x%04X", pSource->Chip->DeviceId);
                    SetDlgItemText(hDlg, IDC_DEVICEID, szBuffer);
                    
                    sprintf(szBuffer,"0x%08X", pSource->SubSystemId);
                    SetDlgItemText(hDlg, IDC_SUBSYSTEMID, szBuffer);

                    hRegisterListTail = &pRegisterList;
                    while(pRegister = pRegisterList)
                    {
                        pRegisterList = pRegister->Next;
                        free(pRegister);
                    }

                    if(pCard != NULL)
                    {
                        delete pCard;
                    }

                    pCard = new CGenericCard(pHardwareDriver);
                    if(pCard->OpenPCICard(
                                            pSource->Chip->VendorId,
                                            pSource->Chip->DeviceId,
                                            pSource->DeviceIndex
                                         ) == TRUE)
                    {
                        int Count = 0;

                        (pSource->Chip->SetupFunction)(hRegisterListTail);

                        for(pRegister = pRegisterList; pRegister != NULL; pRegister = pRegister->Next)
                        {
                            Count++;
                        }
                        
                        if(pRegisterList != NULL)
                        {
                            ScrollPos = 0;
                            ScrollMax = Count * REG_FONT_HEIGHT - 1;

                            if(nRegDisplayHeight < ScrollMax)
                            {
                                SCROLLINFO ScrollInfo;
                                ScrollInfo.cbSize = sizeof(SCROLLINFO);
                                ScrollInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
                                ScrollInfo.nMin = 0;
                                ScrollInfo.nMax = ScrollMax;
                                ScrollInfo.nPage = nRegDisplayHeight;
                                ScrollInfo.nPos = ScrollPos;
                                ScrollInfo.nTrackPos = 0;

                                hWnd = GetDlgItem(hDlg, IDC_REGSCROLL);

                                EnableWindow(hWnd, TRUE);
                                SendMessage(hWnd, SBM_SETSCROLLINFO, TRUE, (LPARAM)&ScrollInfo);
                            }
                            else
                            {
                                EnableWindow(GetDlgItem(hDlg, IDC_REGSCROLL), FALSE);
                            }

                            EnableWindow(GetDlgItem(hDlg, IDC_MOVEUPCHANGES), Count > 1);
                            EnableWindow(GetDlgItem(hDlg, IDC_LOGSTATE), TRUE);
                            EnableWindow(GetDlgItem(hDlg, IDC_DUMPTOFILE), TRUE);

                            if(GetFocus() == GetDlgItem(hDlg, IDC_SOURCESELECT))
                            {
                                SetFocus(hDlg);
                            }

                            SendMessage(hDlg, WM_TIMER, IDC_REFRESH, 0);
                        }
                    }
                    else
                    {
                        delete pCard;
                        pCard = NULL;
                        MessageBox(NULL, "Can't open card", "RegSpy", MB_OK);
                    }
                }
                else
                {
                    EnableWindow(GetDlgItem(hDlg, IDC_REGSCROLL), FALSE);
                    EnableWindow(GetDlgItem(hDlg, IDC_MOVEUPCHANGES), FALSE);
                    EnableWindow(GetDlgItem(hDlg, IDC_LOGSTATE), FALSE);
                    EnableWindow(GetDlgItem(hDlg, IDC_DUMPTOFILE), FALSE);
                }
                LeaveCriticalSection(&RegisterListMutex);
            }
            return TRUE;

        case IDC_LOGSTATE:
            if(bResetLogState == TRUE)
            {
                nLogState = 0;
                EnableWindow(GetDlgItem(hDlg, IDC_LOGSTATE), FALSE);
                SetDlgItemText(hDlg, IDC_DUMPTOFILE, "Dump To File ...");
            }
            else if(nLogState != MAX_LOG_STATES)
            {
                EnterCriticalSection(&RegisterListMutex);
                for(pRegister = pRegisterList; pRegister != NULL; pRegister = pRegister->Next)
                {
                    pRegister->LogStates[nLogState] = pRegister->LastValue;
                }
                LeaveCriticalSection(&RegisterListMutex);

                nLogState++;
                sprintf(szBuffer, "Log State (%d)", nLogState);
                SetDlgItemText(hDlg, IDC_LOGSTATE, szBuffer);
                SetDlgItemText(hDlg, IDC_DUMPTOFILE, "Save To File ...");

                if(nLogState == MAX_LOG_STATES)
                {
                    EnableWindow(GetDlgItem(hDlg, IDC_LOGSTATE), FALSE);
                }

                CopyRect(&Rect, &RegDisplayRect);

                ClientToScreen(GetDlgItem(hDlg, IDC_REGDISPLAY), ((LPPOINT)&Rect));
                ClientToScreen(GetDlgItem(hDlg, IDC_REGDISPLAY), ((LPPOINT)&Rect) + 1);
                ScreenToClient(hDlg, ((LPPOINT)&Rect));
                ScreenToClient(hDlg, ((LPPOINT)&Rect) + 1);

                bRedrawAll = TRUE;
                InvalidateRect(hDlg, &Rect, FALSE);
            }
            return TRUE;

        case IDC_DUMPTOFILE:
            {
                KillTimer(hDlg, IDC_REFRESH);
                EnterCriticalSection(&RegisterListMutex);

                HWND hWnd = GetDlgItem(hDlg, IDC_SOURCESELECT);

                ItemIndex = SendMessage(hWnd, CB_GETCURSEL, 0, 0);
                pSource = (TSource*)SendMessage(hWnd, CB_GETITEMDATA, ItemIndex, 0);
                if(pSource != NULL)
                {
                    if(SaveToFile(hDlg, pSource, pRegisterList, nLogState))
                    {
                        nLogState = 0;
                        SetDlgItemText(hDlg, IDC_LOGSTATE, "Log State");
                        SetDlgItemText(hDlg, IDC_DUMPTOFILE, "Dump To File ...");
                    }
                }

                SendMessage(hDlg, WM_TIMER, IDC_REFRESH, 0);

                LeaveCriticalSection(&RegisterListMutex);
            }
            return TRUE;

        case IDC_MOVEUPCHANGES:
            {
                bMoveUpChanges = IsDlgButtonChecked(hDlg, IDC_MOVEUPCHANGES);
            }
            return TRUE;

        default:
            break;
        }
        break;

    case WM_VSCROLL:
        if((HWND)lParam == GetDlgItem(hDlg, IDC_REGSCROLL))
        {
            LONG Pos = ScrollPos;

            switch(LOWORD(wParam))
            {
            case SB_LEFT:
                Pos = 0;
                break;
            case SB_RIGHT:
                Pos = ScrollMax - nRegDisplayHeight;
                break;
            case SB_LINELEFT:
                Pos = ScrollPos - REG_FONT_HEIGHT;
                break;
            case SB_LINERIGHT:
                Pos = ScrollPos + REG_FONT_HEIGHT;
                break;
            case SB_PAGELEFT:
                Pos = ScrollPos - nRegDisplayHeight;
                break;
            case SB_PAGERIGHT:
                Pos = ScrollPos + nRegDisplayHeight;
                break;
            case SB_THUMBPOSITION:
            case SB_THUMBTRACK:
                Pos = HIWORD(wParam);
                break;
            default:
                break;
            }

            if(Pos < 0)
            {
                Pos = 0;
            }
            if(Pos > ScrollMax - nRegDisplayHeight)
            {
                Pos = ScrollMax - nRegDisplayHeight;
            }

            if(Pos != ScrollPos)
            {
                ScrollPos = Pos;
                SendMessage(GetDlgItem(hDlg, IDC_REGSCROLL), SBM_SETPOS, ScrollPos, TRUE);

                CopyRect(&Rect, &RegDisplayRect);

                ClientToScreen(GetDlgItem(hDlg, IDC_REGDISPLAY), ((LPPOINT)&Rect));
                ClientToScreen(GetDlgItem(hDlg, IDC_REGDISPLAY), ((LPPOINT)&Rect) + 1);
                ScreenToClient(hDlg, ((LPPOINT)&Rect));
                ScreenToClient(hDlg, ((LPPOINT)&Rect) + 1);

                bRedrawAll = TRUE;
                InvalidateRect(hDlg, &Rect, FALSE);
            }

            if(GetFocus() == GetDlgItem(hDlg, IDC_SOURCESELECT))
            {
                SetFocus(hDlg);
            }
            return TRUE;
        }
        break;

    case WM_MOUSEWHEEL:
        if(nRegDisplayHeight < ScrollMax)
        {
            int nScrollLines;
            SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &nScrollLines, 0);

            short Lines = nScrollLines * (short)HIWORD(wParam) / -WHEEL_DELTA;

            LONG TrackPosition = ScrollPos + Lines * REG_FONT_HEIGHT;

            if(TrackPosition < 0)
            {
                TrackPosition = 0;
            }
            else if(TrackPosition > ScrollMax - nRegDisplayHeight)
            {
                TrackPosition = ScrollMax - nRegDisplayHeight;
            }
            
            SendMessage(hDlg, WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, TrackPosition),
                (LPARAM)GetDlgItem(hDlg, IDC_REGSCROLL));
        }
        return TRUE;

    case WM_KEYDOWN:
        if(wParam == VK_CONTROL)
        {
            if(bResetLogState == FALSE)
            {
                bResetLogState = TRUE;
                SetDlgItemText(hDlg, IDC_LOGSTATE, "Reset States");
                if(nLogState == 0)
                {
                    EnableWindow(GetDlgItem(hDlg, IDC_LOGSTATE), FALSE);
                }
            }
        }
        break;

    case WM_KEYUP:
        if(wParam == VK_CONTROL)
        {
            if(bResetLogState == TRUE)
            {
                bResetLogState = FALSE;
                if(nLogState > 0)
                {
                    sprintf(szBuffer, "Log State (%d)", nLogState);
                    if(nLogState != MAX_LOG_STATES)
                    {
                        EnableWindow(GetDlgItem(hDlg, IDC_LOGSTATE), TRUE);
                    }
                }
                else
                {
                    sprintf(szBuffer, "Log State");
                    EnableWindow(GetDlgItem(hDlg, IDC_LOGSTATE), TRUE);
                }
                SetDlgItemText(hDlg, IDC_LOGSTATE, szBuffer);
            }
        }
        break;

    default:
        break;
    }
    return (FALSE);
}


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    TSource*    SourceList = NULL;
    TSource**   SourceListTail = &SourceList;
    TSource*    Source;

    CHardwareDriver* pHardwareDriver = new CHardwareDriver();
    if(pHardwareDriver->LoadDriver() == FALSE)
    {
        delete pHardwareDriver;
        MessageBox(NULL, "Can't open driver", "RegSpy", MB_OK);
        return 1;
    }

    for(int i(0); i < sizeof(Chips)/sizeof(TChip); ++i)
    {
        DWORD SubSystemId;
        int DeviceIndex(0);

        while(pHardwareDriver->DoesThisPCICardExist( 
                                                        Chips[i].VendorId, 
                                                        Chips[i].DeviceId, 
                                                        DeviceIndex, 
                                                        SubSystemId
                                                   ) == TRUE)
        {
            Source = (TSource*) malloc(sizeof(TSource));
            Source->DeviceIndex = DeviceIndex;
            Source->Chip        = &Chips[i];
            Source->SubSystemId = SubSystemId;
            Source->Next        = NULL;

            *SourceListTail = Source;
            SourceListTail = &Source->Next;

            DeviceIndex++;
        }
    }

    void* ContextPtr[] = { SourceList, pHardwareDriver };

    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAINDIALOG), NULL, MainWindowProc, (LPARAM)ContextPtr);

    while(Source = SourceList)
    {
        SourceList = Source->Next;
        free(Source);
    }

    delete pHardwareDriver;
    return 0;
}

void __cdecl LOG(int , LPCSTR, ...)
{
}

HWND hWnd = NULL;

void __cdecl HideSplashScreen()
{
}

extern "C"
{
    long gBuildNum = 0;
}
