/////////////////////////////////////////////////////////////////////////////
// $Id: VBI_VideoText.H,v 1.22 2002-03-12 23:29:44 robmuller Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
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
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           Original Release
//                                     Translated most code from German
//                                     Combined Header files
//                                     Cut out all decoding
//                                     Cut out digital hardware stuff
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __VT_H___
#define __VT_H___


#include "settings.h"

SETTING* VT_GetSetting(VT_SETTING Setting);
void VT_ReadSettingsFromIni();
void VT_WriteSettingsToIni(BOOL bOptimizeFileAccess);
void VT_SetMenu(HMENU hMenu);


enum eVTState
{
    VT_OFF,
    VT_BLACK,
    VT_MIX,
    VT_LASTSTATE,
};

enum eVTCodePage
{
    VT_UK_CODE_PAGE,
    VT_FRENCH_CODE_PAGE,
    VT_CZECH_CODE_PAGE,
    VT_GREEK_CODE_PAGE,
    VT_RUSSIAN_CODE_PAGE,
    VT_GERMAN_CODE_PAGE,
    VT_HUNGARIAN_CODE_PAGE,
    VT_HEBREW_CODE_PAGE,
    VT_SWEDISH_CODE_PAGE,
    VT_ITALIAN_CODE_PAGE,
    VT_SPANISH_CODE_PAGE,
    VT_POLISH_CODE_PAGE,
    VT_CODE_PAGE_LASTONE,
};

typedef struct
{
    // Type 0 data
    struct
    {
        unsigned nMag:3;
        unsigned nPage:8;
        unsigned:5; // Unused
        WORD nSubcode;
    } HomePage;
    WORD NetId;
    struct
    {
        char Offset;
        DWORD JulianDay;
        BYTE Hour;
        BYTE Min;
        BYTE Sec;
    } UTC;
    char Unknown[5];
    char Identifier[21];

    // Type 2 data

    struct
    {
        unsigned LCI:2;
        unsigned LUF:1;
        unsigned PRF:1;
        unsigned PCS:2;
        unsigned MI:1;
        unsigned day:5;
        unsigned month:4;
        unsigned hour:5;
        unsigned minute:6;
        unsigned:5;
        WORD CNI;
        WORD PTY;
    } PDC;
} TPacket30;

#define NUM_FLOF    6
#define FLOF_RED    0
#define FLOF_GREEN  1
#define FLOF_YELLOW 2
#define FLOF_BLUE   3
typedef struct tagVTPage
{
    WORD wCtrl;
    BOOL Fill;
    BYTE Frame[25][40];
    BYTE LineUpdate[25];
    BYTE bUpdated;
    BYTE bFlofUpdated;
    int FlofPage[NUM_FLOF];
    int FlofSubPage[NUM_FLOF];
    unsigned short SubPage;
    int MostRecentSubPage;
    unsigned short Page;
    tagVTPage *Next;
} TVTPage;

typedef struct 
{
    BOOL Line[25];
    BOOL PageErase;
} TVTRedrawCache;

#define LARGE_WIDTH 15
#define LARGE_HEIGHT 18

#define VTTimeout 60000

#define VT_LARGE_BITMAP_HEIGHT (LARGE_HEIGHT*25)
#define VT_LARGE_BITMAP_WIDTH (LARGE_WIDTH*40)


#define ROUNDUP(w) (((w)+3)&~3)
#define _BitmapDataP(bmp)   (bmp->bmiHeader.biBitCount==8?(((BYTE*)(bmp))+sizeof(BITMAPINFOHEADER)+sizeof(WORD)*256):(((BYTE*)(bmp))+sizeof(BITMAPINFOHEADER)))
#define _BitmapDataR(bmp)   (((BYTE*)(bmp))+sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD)*256)
#define _BitmapPalR(bmp)    ((RGBQUAD*)(((BYTE*)(bmp))+sizeof(BITMAPINFOHEADER)))

#define _BitmapLargeRowR(bmp,row) (_BitmapDataR(bmp)+ROUNDUP(bmp->bmiHeader.biWidth)*(bmp->bmiHeader.biHeight-(row)*LARGE_HEIGHT))
#define _BitmapLargeChar(bmp,c) (_BitmapLargeRowR(bmp,(c)/32)+((c)&0x1f)*LARGE_WIDTH)

#define _BitmapLargeRowP(bmp,row) ((_BitmapDataP(bmp))+VT_LARGE_BITMAP_WIDTH*((row)*LARGE_HEIGHT))
#define _BitmapLargeText(bmp,row,c) (_BitmapLargeRowP(bmp,row)+((c)*LARGE_WIDTH*2))


#define _BitmapSmallRowR(bmp,row) (_BitmapDataR(bmp)+ROUNDUP(bmp->bmiHeader.biWidth)*(bmp->bmiHeader.biHeight-(row)*SMALL_HEIGHT))
#define _BitmapSmallChar(bmp,c) (_BitmapSmallRowR(bmp,(c)/32)+((c)&0x1f)*SMALL_WIDTH)

#define _BitmapSmallRowP(bmp,row) ((_BitmapDataP(bmp))+VT_SMALL_BITMAP_WIDTH*((row)*SMALL_HEIGHT))
#define _BitmapSmallText(bmp,row,c) (_BitmapSmallRowP(bmp,row)+((c)*SMALL_WIDTH*2))

void VBI_VT_Init();
void VBI_VT_Exit();

BOOL APIENTRY VTInfoProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

BYTE DeHam(BYTE byte);
BYTE ReverseDeHam(BYTE b);
void StorePage(BYTE mag, BYTE page);
void StorePacket30(BYTE* p);

unsigned char unham(unsigned char* d);
unsigned char unham2(unsigned char* d);
unsigned char VBI_Scan(BYTE* Buffer, unsigned int step);

void VBI_decode_vt(unsigned char* dat);
void VBI_decode_line(unsigned char* d, int line);

void VT_ChannelChange();

void VT_DecodeLine(BYTE* VBI_Buffer, int line, BOOL IsOdd);
void VTS_DecodeLine(BYTE* VBI_Buffer);
void VT_DoUpdate_Page(int Page, int SubPage);
void VT_UpdateFlashTimerStatus();
int VT_MostRecentSubPage(int Page);
int VT_GetNextPage(int Page);
int VT_GetPreviousPage(int Page);
int VT_SubPageNext(int Page, int SubPage, int Direction, bool Cycle);
int VT_CompleteSubPages(int Page);
TVTPage* VT_PageGet(int Page, int SubPage);

bool VT_ContainsUpdatedPage();
void VT_CreateTestPage();
void VT_Redraw(HWND hWnd, HDC hDC, BOOL IsDDhDC, BOOL ShowFlashed);
void VT_RedrawClock(HWND hWnd, HDC hDC, bool RedrawHeader);
void VT_RedrawFlash(HWND hWnd, HDC hDC, BOOL ShowFlashed);
void VT_ProcessRedrawCache(HWND hWnd, HDC hDC);
void VT_PurgeRedrawCache();
void VT_ResetStation();
LPCSTR VT_GetStation();
void VT_SetCodePage(eVTCodePage Codepage);

int VT_GetPageNumberAt(int Page, int SubPage, int x, int y);
int VT_GetFlofPageNumber(int Page, int SubPage, int flof);

extern eVTState VTState;

extern int VTPage;
extern int VTSubPage;
extern bool VTSubPageLocked;
extern bool VTShowHidden;

#endif
