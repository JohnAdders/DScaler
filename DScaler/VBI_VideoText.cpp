/////////////////////////////////////////////////////////////////////////////
// $Id: VBI_VideoText.cpp,v 1.9 2001-07-12 16:16:40 adcockj Exp $
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
// CVS Log
//
// $Log: not supported by cvs2svn $
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "bt848.h"
#include "VBI_VideoText.h"
#include "VBI_CCdecode.h"
#include "VBI.h"
#include "DScaler.h"
#include "AspectRatio.h"
#include "Other.h"
#include "DebugLog.h"

TPacket30 Packet30;

TVTPage VTPages[800];
int VTPage = 100;

unsigned int VBI_spos;

int VBI_FPS;
int VTCachedPages = 0;
int VTCurrentPage = 0;
int VTCurrentSubCode = 0;
eCODEPAGE VTCodePage = VT_UK_CODE_PAGE;

char VPS_tmpName[9]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
char VPS_lastname[9]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
char VPS_chname[9]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
int VPS_namep=0;

unsigned char revham[16] = {
  0x00, 0x08, 0x04, 0x0c, 0x02, 0x0a, 0x06, 0x0e,
  0x01, 0x09, 0x05, 0x0d, 0x03, 0x0b, 0x07, 0x0f };

unsigned char unhamtab[256] = {
  0x01, 0x0f, 0x01, 0x01, 0x0f, 0x00, 0x01, 0x0f,
  0x0f, 0x02, 0x01, 0x0f, 0x0a, 0x0f, 0x0f, 0x07,
  0x0f, 0x00, 0x01, 0x0f, 0x00, 0x00, 0x0f, 0x00,
  0x06, 0x0f, 0x0f, 0x0b, 0x0f, 0x00, 0x03, 0x0f,
  0x0f, 0x0c, 0x01, 0x0f, 0x04, 0x0f, 0x0f, 0x07,
  0x06, 0x0f, 0x0f, 0x07, 0x0f, 0x07, 0x07, 0x07,
  0x06, 0x0f, 0x0f, 0x05, 0x0f, 0x00, 0x0d, 0x0f,
  0x06, 0x06, 0x06, 0x0f, 0x06, 0x0f, 0x0f, 0x07,
  0x0f, 0x02, 0x01, 0x0f, 0x04, 0x0f, 0x0f, 0x09,
  0x02, 0x02, 0x0f, 0x02, 0x0f, 0x02, 0x03, 0x0f,
  0x08, 0x0f, 0x0f, 0x05, 0x0f, 0x00, 0x03, 0x0f,
  0x0f, 0x02, 0x03, 0x0f, 0x03, 0x0f, 0x03, 0x03,
  0x04, 0x0f, 0x0f, 0x05, 0x04, 0x04, 0x04, 0x0f,
  0x0f, 0x02, 0x0f, 0x0f, 0x04, 0x0f, 0x0f, 0x07,
  0x0f, 0x05, 0x05, 0x05, 0x04, 0x0f, 0x0f, 0x05,
  0x06, 0x0f, 0x0f, 0x05, 0x0f, 0x0e, 0x03, 0x0f,
  0x0f, 0x0c, 0x01, 0x0f, 0x0a, 0x0f, 0x0f, 0x09,
  0x0a, 0x0f, 0x0f, 0x0b, 0x0a, 0x0a, 0x0a, 0x0f,
  0x08, 0x0f, 0x0f, 0x0b, 0x0f, 0x00, 0x0d, 0x0f,
  0x0f, 0x0b, 0x0b, 0x0b, 0x0a, 0x0f, 0x0f, 0x0b,
  0x0c, 0x0c, 0x0f, 0x0c, 0x0f, 0x0c, 0x0d, 0x0f,
  0x0f, 0x0c, 0x0f, 0x0f, 0x0a, 0x0f, 0x0f, 0x07,
  0x0f, 0x0c, 0x0d, 0x0f, 0x0d, 0x0f, 0x0d, 0x0d,
  0x06, 0x0f, 0x0f, 0x0b, 0x0f, 0x0e, 0x0d, 0x0f,
  0x08, 0x0f, 0x0f, 0x09, 0x0f, 0x09, 0x09, 0x09,
  0x0f, 0x02, 0x0f, 0x0f, 0x0a, 0x0f, 0x0f, 0x09,
  0x08, 0x08, 0x08, 0x0f, 0x08, 0x0f, 0x0f, 0x09,
  0x08, 0x0f, 0x0f, 0x0b, 0x0f, 0x0e, 0x03, 0x0f,
  0x0f, 0x0c, 0x0f, 0x0f, 0x04, 0x0f, 0x0f, 0x09,
  0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0e, 0x0f, 0x0f,
  0x08, 0x0f, 0x0f, 0x05, 0x0f, 0x0e, 0x0d, 0x0f,
  0x0f, 0x0e, 0x0f, 0x0f, 0x0e, 0x0e, 0x0f, 0x0e,
};

BYTE VT_Header_Line[40];

#define GetBit(val,bit,mask) (BYTE)(((val)>>(bit))&(mask))

BITMAPINFO* VTCharSet = NULL;
BITMAPINFO* VTScreen = NULL;

VT_STATE VTState = VT_OFF;


typedef struct
{
    int Page;
    int SubPage;
    BOOL PageErase;
    BOOL bStarted;
} TMAGSTATE;

#define NUM_MAGAZINES 8

TMAGSTATE MagazineStates[NUM_MAGAZINES];

/// VideoText
unsigned short VTColourTable[9] =
{
    0,      //Black
    31744,  //Red
    992,    //Green
    32736,  //Yellow
    31,     //Blue
    31775,  //Invisible
    15871,  //Cyan
    32767,  //White
    31775,  //Transparent
};

void VBI_VT_Init()
{
    VT_SetCodePage(VTCodePage);

    memset(VTPages, 0, 800 * sizeof(TVTPage));

    VTPage = 100;
    VT_ChannelChange();

    VTScreen = (BITMAPINFO *) calloc(1, sizeof(BITMAPINFOHEADER) + sizeof(WORD) * 256 + VT_LARGE_BITMAP_WIDTH * 2 * VT_LARGE_BITMAP_HEIGHT);
    VTScreen->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    VTScreen->bmiHeader.biWidth = VT_LARGE_BITMAP_WIDTH;
    VTScreen->bmiHeader.biHeight = VT_LARGE_BITMAP_HEIGHT;
    VTScreen->bmiHeader.biPlanes = 1;
    VTScreen->bmiHeader.biBitCount = 16;
    VTScreen->bmiHeader.biCompression = BI_RGB;
    VTScreen->bmiHeader.biSizeImage = VTScreen->bmiHeader.biWidth * VTScreen->bmiHeader.biHeight * VTScreen->bmiHeader.biBitCount / 8;
    VTScreen->bmiHeader.biXPelsPerMeter = 0;
    VTScreen->bmiHeader.biYPelsPerMeter = 0;
    VTScreen->bmiHeader.biClrUsed = 0;
    VTScreen->bmiHeader.biClrImportant = 0;
}

void VBI_VT_Exit()
{
    if(VTScreen != NULL)
    {
        free(VTScreen);
        VTScreen = NULL;
    }
    
    if(VTCharSet != NULL)
    {
        DeleteObject(VTCharSet);
        VTCharSet = NULL;
    }
}

void VBI_decode_vps(unsigned char *data)
{

    unsigned char *info;

    info = data;
    if ((info[3] & 0x80))
    {
        VPS_chname[VPS_namep] = 0;
        if (VPS_namep == 8)
        {
            if (strcpy(VPS_chname, VPS_tmpName) == 0)
                memcpy(VPS_lastname, VPS_chname, 9);    // VPS-Channel-Name
            strcpy(VPS_tmpName, VPS_chname);
        }
        VPS_namep = 0;
    }
    VPS_chname[VPS_namep++] = info[3] & 0x7f;
    if (VPS_namep >= 9)
        VPS_namep = 0;
    if (ShowVPSInfo != NULL)
        SetDlgItemText(ShowVPSInfo, TEXT1, VPS_lastname);
}

unsigned char VBI_Scan(BYTE * VBI_Buffer, unsigned int step)
{
    int j;
    unsigned char dat;

    for (j = 7, dat = 0; j >= 0; j--, VBI_spos += step)
        dat |= ((VBI_Buffer[VBI_spos >> FPSHIFT] + VBI_off) & 0x80) >> j;
    return dat;
}

// unham 2 bytes into 1, report 2 bit errors but ignore them
unsigned char unham(unsigned char *d)
{
    unsigned char c1, c2;

    c1 = unhamtab[d[0]];
    c2 = unhamtab[d[1]];
    return (c2 << 4) | (c1);
}

// unham, but with reversed nibble order for VC
unsigned char unham2(unsigned char *d)
{
    unsigned char c1, c2;

    c1 = unhamtab[d[0]];
    c2 = unhamtab[d[1]];
    return (c1 << 4) | (c2);
}

int MakePage(int mag, int page)
{
    int Low =  page & 0x0f;
    int High = page >> 4;
    if(Low > 9 || High > 9)
    {
        return -1;
    }
    if(mag == 0)
    {
        return 800 + High * 10 + Low;
    }
    else
    {
        return 100 * mag + High * 10 + Low;
    }
}


void VBI_decode_vt(unsigned char *dat)
{
    int i;
    unsigned char mag, pack, mpag, ftal, ft, al, page;
    unsigned int addr;
    unsigned int pnum = 0;
    unsigned short sub;
    int j;
    WORD ctrl;

    int nPage;
    int nPage1;

    /* dat: 55 55 27 %MPAG% */
    mpag = unham(dat + 3);
    mag = mpag & 7;

    pack = (mpag >> 3) & 0x1f;

    switch (pack)
    {
    case 0:
        //hdump(udat,4); cout << " HD\n";

        /* dat: 55 55 27 %MPAG% %PAGE% %SUB%
           00 01 02  03 04  05 06 07-0a
         */

        if(MagazineStates[mag].bStarted)
        {
            if(VTState != VT_OFF && MagazineStates[mag].Page == VTPage - 100)
            {
                RECT Dest;
                VT_DoUpdate_Page(VTPage - 100);
                GetDestRect(&Dest);
                InvalidateRect(hWnd, &Dest, FALSE);
            }
        }

        page = unham(dat + 5);


        pnum = MakePage(mag, page);
        if(pnum < 100)
        {
            MagazineStates[mag].bStarted = FALSE;
            return;
        }

        sub = (unham(dat + 9) << 8) | unham(dat + 7);

        MagazineStates[mag].PageErase = ((sub & 0x80) != 0);

        j = sub;
        sub = sub & 0x3F;
        nPage = (sub / 16);
        nPage1 = sub - (nPage * 16);

        sub = nPage * 10 + nPage1;

        VTCurrentPage = pnum;
        VTCurrentSubCode = sub;

        pnum -= 100;

        if ((pnum >= 0) && (pnum < 800))
        {

            ctrl = (unhamtab[dat[3]] & 0x7) + ((unhamtab[dat[8]] >> 3) << 3) + ((unhamtab[dat[10]] >> 2) << 4) + (unhamtab[dat[11]] << 6) + (unhamtab[dat[12]] << 10);

            if (sub > 0)
                sub--;

            MagazineStates[mag].Page = pnum;
            MagazineStates[mag].SubPage = sub;
            MagazineStates[mag].bStarted = TRUE;
            LOG("Mag %d Page %d SubCode %d", mag, pnum, sub);

            if(VTPages[pnum].bUpdated == FALSE)
            {
                ++VTCachedPages;
            }
            VTPages[pnum].wCtrl = ctrl;
            if (MagazineStates[mag].PageErase == TRUE)
            {
                memset(&VTPages[pnum].Frame[1], 0x00, 24 * 40);
                memset(&VTPages[pnum].LineUpdate[1], 0x00, 24);
            }
            memcpy(&VTPages[pnum].Frame[0], dat + 5, 40);
            memcpy(&VT_Header_Line[0], dat + 5, 40);
            VTPages[pnum].bUpdated = 1;
            VTPages[pnum].Fill = TRUE;
            VTPages[pnum].SubPage = sub + 1;
        }
        break;

    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
        if(MagazineStates[mag].bStarted)
        {
            memcpy(&VTPages[MagazineStates[mag].Page].Frame[pack], dat + 5, 40);

            VTPages[MagazineStates[mag].Page].bUpdated = 1;
            VTPages[MagazineStates[mag].Page].Fill = TRUE;
            VTPages[MagazineStates[mag].Page].LineUpdate[pack] = 1;
        }
        break;
    
    case 25:
        if(MagazineStates[mag].bStarted)
        {
            memcpy(&VTPages[MagazineStates[mag].Page].Frame[0], dat + 5, 40);
            VTPages[MagazineStates[mag].Page].bUpdated = 1;
            VTPages[MagazineStates[mag].Page].Fill = TRUE;
            VTPages[MagazineStates[mag].Page].LineUpdate[0] = 1;
        }
        break;
    case 26:                    // PDC
    case 27:
    case 28:
    case 29:
        break;
    case 30:
        StorePacket30(dat);
        break;
    case 31:
        ftal = unham(dat + 5);
        al = ftal >> 4;         /* address length */
        ft = ftal & 0x0f;
        for (addr = 0, i = 0; i < al; i++)
            addr = (addr << 4) | unhamtab[dat[7 + i]];

        switch (addr)
        {
        case 0x07:
            break;

        case 0x0f00:            /* also used by ZDF and DSF, data format unknown */
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }
}

void StorePacket30(BYTE * p)
{
    DWORD d, b;
    BYTE h, m, s, a, CNI0, CNI1, CNI2, CNI3;
    int n;

    if (*p != 0x55)
        return;                 // Some error, the data should be here...
    p += 5;

    if (unhamtab[*p] == 0)      // TSDP
    {
        p++;
        Packet30.HomePage.nPage = unham(p);
        Packet30.HomePage.nSubcode = (((unhamtab[p[5]] & 0x3) << 12) + (unhamtab[p[4]] << 8) + ((unhamtab[p[3]] & 0x7) << 4) + unhamtab[p[2]]);

        Packet30.HomePage.nMag = ((unhamtab[p[5]] >> 1) & 0x6) + ((unhamtab[p[3]] >> 3) & 0x1);
        p += 6;
        Packet30.NetId = (p[1] << 8) + p[0];
        p += 2;
        Packet30.UTC.Offset = ((*p >> 1) & 0x1f) * (*p & 0x40) ? -1 : 1;
        p++;
        d = (((*p) - 0x01) << 16) + (((*(p + 1)) - 0x11) << 8) + ((*(p + 2)) - 0x11);
        Packet30.UTC.JulianDay = (((d & 0xF0000) >> 16) * 10000) + (((d & 0x0F000) >> 12) * 1000) + (((d & 0x00F00) >> 8) * 100) + (((d & 0x000F0) >> 4) * 10) + (d & 0x0000F);
        p += 3;
        h = (*p) - 0x11;
        m = (*(p + 1)) - 0x11;
        s = (*(p + 2)) - 0x11;
        Packet30.UTC.Hour = (h >> 4) * 10 + (h & 0x0f);
        Packet30.UTC.Min = (m >> 4) * 10 + (m & 0x0f);
        Packet30.UTC.Sec = (s >> 4) * 10 + (s & 0x0f);
        p += 3;
        for (n = 0; n < 4; n++)
            Packet30.Unknown[n] = p[n] & 0x7f;
        Packet30.Unknown[n] = '\0';
        p += 4;
        for (n = 0; n < 20; n++)
            Packet30.Identifier[n] = p[n] & 0x7f;
        Packet30.Identifier[n] = '\0';

    }
    else if (unhamtab[*p] == 2) // PDC
    {
        p++;
        Packet30.HomePage.nPage = unham(p);
        Packet30.HomePage.nSubcode = (((unhamtab[p[5]] & 0x3) << 12) + (unhamtab[p[4]] << 8) + ((unhamtab[p[3]] & 0x7) << 4) + unhamtab[p[2]]);

        Packet30.HomePage.nMag = ((unhamtab[p[5]] >> 1) & 0x6) + ((unhamtab[p[3]] >> 3) & 0x1);
        p += 6;
        a = revham[unhamtab[p[0]]];
        Packet30.PDC.LCI = GetBit(a, 2, 3);
        Packet30.PDC.LUF = GetBit(a, 1, 1);
        Packet30.PDC.PRF = GetBit(a, 0, 1);
        a = revham[unhamtab[p[1]]];
        Packet30.PDC.PCS = GetBit(a, 2, 3);
        Packet30.PDC.MI = GetBit(a, 1, 1);
        CNI0 = revham[unhamtab[p[2]]];
        b = (revham[unhamtab[p[3]]] << 28) + (revham[unhamtab[p[4]]] << 24) +
            (revham[unhamtab[p[5]]] << 20) + (revham[unhamtab[p[6]]] << 16) +
            (revham[unhamtab[p[7]]] << 12) + (revham[unhamtab[p[8]]] << 8) + (revham[unhamtab[p[9]]] << 4) + (revham[unhamtab[p[10]]]);
        CNI2 = GetBit(b, 30, 3);
        Packet30.PDC.day = GetBit(b, 25, 0x1f);
        Packet30.PDC.month = GetBit(b, 21, 0xf);
        Packet30.PDC.hour = GetBit(b, 16, 0x1f);
        Packet30.PDC.minute = GetBit(b, 10, 0x3f);
        CNI1 = GetBit(b, 6, 0xf);
        CNI3 = GetBit(b, 0, 0x3f);
        Packet30.PDC.PTY = (revham[unhamtab[p[11]]] << 4) + revham[unhamtab[p[12]]];;
        Packet30.PDC.CNI = (CNI0 << 12) + (CNI1 << 8) + (CNI2 << 4) + CNI3;
        p += 13;
        for (n = 0; n < 20; n++)
            Packet30.Identifier[n] = p[n] & 0x7f;
        Packet30.Identifier[n] = '\0';
    }
}

int VT_GetPage(int nPage)
{
    if (nPage < 100 || nPage > 899)
        return (-1);
    if (VTPages[nPage - 100].bUpdated == FALSE)
        return (-1);
    return nPage - 100;
}

void VT_DoUpdate_Page(int Page)
{

    BOOL bGraph, bHoldGraph, bSepGraph, bBox, bFlash, bDouble, bConceal, bHasDouble;
    BYTE nLastGraph;
    unsigned short CurrentFg, CurrentBkg, RealBkg;
    WORD wCharHeight, wCharWidth;
    BYTE c, ch;
    int endrow;
    int n, row, x, y;
    char tmp[41];
    BITMAPINFO *pCharSet;
    char tmp2[9];
    int VT_Bitmap_width;

    BYTE *src, *dest;
    unsigned short *dest1;

    BOOL bHideTopLine, bForceShowTopLine;
    unsigned short Black, ForceBlack, ForceTransparent;

    if (VTPages[Page].bUpdated == FALSE)
    {
        Black = VTColourTable[0];   //
        bHideTopLine = FALSE;
        ForceTransparent = VTColourTable[8];
        ForceBlack = VTColourTable[0];
    }
    else
    {

        if ((VTPages[Page].wCtrl & (3 << 4)) || VTState == VT_MIX)
            Black = VTColourTable[8];
        else
            Black = VTColourTable[0];   // 0

        ForceTransparent = VTColourTable[8];
        ForceBlack = VTColourTable[0];

        if (VTPages[Page].wCtrl & (3 << 4))
            bHideTopLine = TRUE;
        else
            bHideTopLine = FALSE;
    }

    wCharWidth = LARGE_WIDTH;
    wCharHeight = LARGE_HEIGHT;
    pCharSet = VTCharSet;
    VT_Bitmap_width = VT_LARGE_BITMAP_WIDTH;

    bForceShowTopLine = TRUE;

    bHasDouble = FALSE;
    endrow = 25;

    if (VTPages[Page].bUpdated == FALSE)
    {
        endrow = 1;
    }
    else
    {
        if (VTPages[Page].bUpdated == 0)
            endrow = 1;
    }

    for (row = 0; row < endrow; row++)
    {
        if (bHasDouble)
        {
            bHasDouble = FALSE;
            continue;
        }
        bGraph = bHoldGraph = bSepGraph = bBox = bFlash = bDouble = bConceal = bHasDouble = FALSE;
        nLastGraph = 32;

        if (VTPages[Page].bUpdated == FALSE)
        {

            sprintf(tmp2, "  P%-3d \x7", VTPage);
            for (n = 0; n < 40; n++)
            {
                tmp[n] = VT_Header_Line[n] & 0x7f;
                if (tmp[n] == 0x0d)
                    bHasDouble = TRUE;
                strncpy(tmp, tmp2, 8);
            }

        }
        else
        {

            for (n = 0; n < 40; n++)
            {
                tmp[n] = VTPages[Page].Frame[row][n] & 0x7f;
                if (tmp[n] == 0x0d)
                    bHasDouble = TRUE;
            }
            tmp[n] = '\0';

            if (row == 0)
            {
                sprintf(tmp2, "  P%-3d \x7", VTPage);
                strncpy(tmp, tmp2, 8);
                if (VTPages[Page].Fill == FALSE)
                {
                    for (n = 0; n < 40; n++)
                    {
                        tmp[n] = VT_Header_Line[n] & 0x7f;
                        if (tmp[n] == 0x0d)
                            bHasDouble = TRUE;
                        strncpy(tmp, tmp2, 8);
                    }
                }
                else
                {
                    for (n = 30; n < 40; n++)
                    {
                        tmp[n] = VT_Header_Line[n] & 0x7f;
                        if (tmp[n] == 0x0d)
                            bHasDouble = TRUE;
                    }

                }
            }
        }

        RealBkg = ForceBlack;
        if (Page != 0)
        {
            if (row == 0 && ((VTPages[Page].bUpdated == FALSE) || (bForceShowTopLine || (!bHideTopLine && VTState != VT_MIX))))
                CurrentBkg = ForceBlack;
            else
                CurrentBkg = Black;

            if ((bHideTopLine && !bForceShowTopLine) && row == 0 && (VTPages[Page].bUpdated == TRUE))
                memset(tmp, 32, 40);
        }
        else
            CurrentBkg = ForceBlack;

        CurrentFg = VTColourTable[7];
        for (n = 0; n < 40; n++)
        {
            c = tmp[n];
            ch = c;
            if (c < 32)
            {
                if (c < 8)
                {
                    CurrentFg = VTColourTable[c];
                    bGraph = FALSE;
                }
                if (c >= 0x10 && c <= 0x17)
                {
                    bGraph = TRUE;
                    CurrentFg = VTColourTable[c - 0x10];
                }
                if (c == 0x1d)
                {
                    CurrentBkg = (bBox || Black != ForceTransparent) ? CurrentFg : ForceTransparent;
                    RealBkg = CurrentFg;
                }
                if (c == 0x1c)
                {
                    CurrentBkg = bBox ? ForceBlack : Black;
                    RealBkg = ForceBlack;
                }
                if (c == 0x08)
                    bFlash = TRUE;
                if (c == 0x09)
                    bFlash = FALSE;
                if (c == 0x0c)
                    bDouble = FALSE;
                if (c == 0x0d)
                    bDouble = TRUE;
                if (c == 0x18)
                    bConceal = TRUE;
                if (c == 0x19)
                    bSepGraph = FALSE;
                if (c == 0x1a)
                    bSepGraph = TRUE;
                if (c == 0x1e)
                    bHoldGraph = TRUE;
                if (c == 0x1f)
                    bHoldGraph = FALSE;
                ch = bHoldGraph ? nLastGraph : 32;
            }
            if ((CurrentFg == VTColourTable[7]) && (VTPages[Page].bUpdated == FALSE) && (!row && n > 7))
                CurrentFg = VTColourTable[2];
            nLastGraph = 32;
            if (bGraph && (ch & 0x20))
            {
                nLastGraph = ch;
                ch = (ch & 0x1f) | ((ch & 0x40) >> 1);
                ch += 96;
                if (bSepGraph)
                    ch += 64;
            }
            else
                ch -= 32;

            src = _BitmapLargeChar(pCharSet, ch);
            dest = _BitmapDataP(VTScreen);
            dest += VTScreen->bmiHeader.biSizeImage;
            dest -= ((40 - n) * (wCharWidth * 2)) + ((wCharWidth / 2) * 2);
            dest -= (row * VT_Bitmap_width * wCharHeight * 2);

            for (y = 0; y < ((bDouble && (row < 24)) ? wCharHeight * 2 : wCharHeight); y++)
            {
                if (!bDouble || (bDouble && (!(y & 1))))
                    src -= ROUNDUP(pCharSet->bmiHeader.biWidth);

                for (x = 0; x < wCharWidth; x++)
                {
                     dest1 = (WORD*)(dest + x * 2);
                    *(dest1) = (unsigned short) (*(src + x) ? CurrentFg : CurrentBkg);
                }
                dest -= VT_Bitmap_width * 2;

            }
            if (!bDouble && bHasDouble && row < 24)
            {
                for (y = 0; y < wCharHeight; y++)
                {
                    for (x = 0; x < wCharWidth; x++)
                    {
                        dest1 = (WORD*)(dest + x * 2);
                        *(dest1) = CurrentBkg;
                    }
                    dest -= VT_Bitmap_width * 2;
                }

            }

            if (c < 32)
            {
                if (c == 0x0a)  // Box off
                {
                    CurrentBkg = (Black == ForceTransparent) ? ForceTransparent : RealBkg;
                    bBox = FALSE;
                }
                if (c == 0x0b)  // Box on
                {
                    CurrentBkg = RealBkg;
                    bBox = TRUE;
                }
            }
        }
    }
}

void VT_ChannelChange()
{
    memset(VTPages, 0, 800 * sizeof(TVTPage));
    memset(MagazineStates, 0, sizeof(TMAGSTATE) * NUM_MAGAZINES);
    VTCachedPages = 0;
}


void VT_Redraw(HWND hWnd, HDC hDC)
{
    RECT Rect;
    GetDestRect(&Rect);

    StretchDIBits(hDC,                              // handle to DC
                    Rect.left,                      // x-coord of destination upper-left corner
                    Rect.top,                       // y-coord of destination upper-left corner
                    Rect.right - Rect.left,         // width of destination rectangle
                    Rect.bottom - Rect.top,         // height of destination rectangle
                    0,                              // x-coord of source upper-left corner
                    0,                              // y-coord of source upper-left corner
                    VT_LARGE_BITMAP_WIDTH,          // width of source rectangle
                    VT_LARGE_BITMAP_HEIGHT,         // height of source rectangle
                    _BitmapDataP(VTScreen),         // bitmap bits
                    VTScreen,                       // bitmap data
                    DIB_PAL_COLORS,                 // usage options
                    SRCCOPY);                       // raster operation code
                
}

//////////////////////////////////////////////////////////////////////
void VT_DecodeLine(BYTE* VBI_Buffer, int line, BOOL IsOdd)
{
    unsigned char data[45];
    unsigned char min, max;
    int dt[256], hi[6], lo[6];
    int i, n, sync, thr;

    /* remove DC. edge-detector */
    for (i = 40; i < 240; ++i)
    {
        dt[i] = VBI_Buffer[i + vtstep / FPFAC] - VBI_Buffer[i]; // amplifies the edges best.
    }

    /* set barrier */
    for (i = 240; i < 256; i += 2)
    {
        dt[i] = 100;
        dt[i+1] = -100;
    }

    /* find 6 rising and falling edges */
    for (i = 40, n = 0; n < 6; ++n)
    {
        while (dt[i] < 32)
        {
            i++;
        }
        hi[n] = i;
        while (dt[i] > -32)
        {
            i++;
        }
        lo[n] = i;
    }
    if (i >= 240)
    {
        return; // not enough periods found
    }

    i = hi[5] - hi[1];  // length of 4 periods (8 bits), normally 40.9
    if (i < 39 || i > 42)
    {
        return; // bad frequency
    }

    /* AGC and sync-reference */
    min = 255;
    max = 0;
    sync = 0;
    
    for (i = hi[4]; i < hi[5]; ++i)
    {
        if (VBI_Buffer[i] > max)
        {
            max = VBI_Buffer[i];
            sync = i;
        }
    }
    for (i = lo[4]; i < lo[5]; ++i)
    {
        if (VBI_Buffer[i] < min)
        {
            min = VBI_Buffer[i];
        }
    }
    
    thr = (min + max) / 2;

    /* search start-byte 11100100 */
    //for (i = 4 * vtstep + vbi->pll_adj*vtstep/10; i < 16*vtstep; i += vtstep)
    for (i = 4 * vtstep; i < (int)(16*vtstep); i += vtstep)
    {
        if (VBI_Buffer[sync + i/FPFAC] > thr && VBI_Buffer[sync + (i+vtstep)/FPFAC] > thr) // two ones is enough...
        {
            /* got it... */
            memset(data, 0, sizeof(data));
            data[0] = 0x55;
            data[1] = 0x55;
            for (n = 0; n < 43*8; ++n, i += vtstep)
            {
                if (VBI_Buffer[sync + i/FPFAC] > thr)
                {
                    data[2 + n/8] |= 1 << (n % 8);
                }
            }

            if (data[2] != 0x27)    // really 11100100? (rev order!)
            {
                return;
            }

            VBI_decode_vt(data);
        }
    }
    return;
}

void VTS_DecodeLine(BYTE* VBI_Buffer)
{
    unsigned char data[45];
    int i, p;

    p = 150;
    while ((VBI_Buffer[p] < VBI_thresh) && (p < 260))
        p++;
    p += 2;
    VBI_spos = p << FPSHIFT;
    if ((data[0] = VBI_Scan(VBI_Buffer, vpsstep)) != 0xff)
        return;
    if ((data[1] = VBI_Scan(VBI_Buffer, vpsstep)) != 0x5d)
        return;
    for (i = 2; i < 16; i++)
    {
        data[i] = VBI_Scan(VBI_Buffer, vpsstep);
    }
    VBI_decode_vps(data);
}

void VT_ResetStation()
{
    memset(Packet30.Identifier, 0, 21);
}

LPCSTR VT_GetStation()
{
    return Packet30.Identifier;
}

BOOL APIENTRY VTInfoProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{

    switch (message)
    {
    case WM_INITDIALOG:
        SetTimer(hDlg, 0, 2000, NULL);
    case WM_TIMER:
        SetDlgItemInt(hDlg, TEXT1, VTCachedPages, FALSE);
        SetDlgItemInt(hDlg, TEXT2, VTCurrentPage, FALSE);
        SetDlgItemInt(hDlg, TEXT2, VTCurrentSubCode, FALSE);
        break;

    case WM_COMMAND:
        if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
        {
            KillTimer(hDlg, 0);
            EndDialog(hDlg, TRUE);
        }
        break;
    }

    return (FALSE);
}


void VT_SetCodePage(eCODEPAGE Codepage)
{
    if(VTCharSet != NULL)
    {
        DeleteObject(VTCharSet);
        VTCharSet = NULL;
    }
    VTCodePage = Codepage;
    switch(Codepage)
    {
    case VT_FRENCH_CODE_PAGE:
        VTCharSet = (BITMAPINFO *)LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDB_FRENCH_VTCHARS), RT_BITMAP));
        break;
    case VT_UK_CODE_PAGE:
    default:
        VTCharSet = (BITMAPINFO *)LoadResource(hInst, FindResource(hInst, MAKEINTRESOURCE(IDB_VTCHARS), RT_BITMAP));
        break;
    }
}

SETTING VTSettings[VT_SETTING_LASTONE] =
{
    {
        "VT Code Page", SLIDER, 0, (long*)&VTCodePage,
        VT_UK_CODE_PAGE, VT_UK_CODE_PAGE, VT_CODE_PAGE_LASTONE - 1, 1, 1,
        NULL,
        "VT", "CodePage", NULL,
    },
};

SETTING* VT_GetSetting(VT_SETTING Setting)
{
    if(Setting > -1 && Setting < VT_SETTING_LASTONE)
    {
        return &(VTSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void VT_ReadSettingsFromIni()
{
    int i;
    for(i = 0; i < VT_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(VTSettings[i]));
    }
}

void VT_WriteSettingsToIni()
{
    int i;
    for(i = 0; i < VT_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(VTSettings[i]));
    }
}

void VT_SetMenu(HMENU hMenu)
{
    CheckMenuItem(hMenu, IDM_VT_UK, (VTCodePage == VT_UK_CODE_PAGE)?MF_CHECKED:MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_VT_FRENCH, (VTCodePage == VT_FRENCH_CODE_PAGE)?MF_CHECKED:MF_UNCHECKED);
}
