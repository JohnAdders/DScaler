/////////////////////////////////////////////////////////////////////////////
// $Id: VBI_VideoText.cpp,v 1.35 2002-02-24 08:18:03 temperton Exp $
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
// Color buttons code was taken from GnomeTV. Those portions are
// Copyright (C) 1999-2000 Zoltán Sinkovics and Szabolcs Seláf
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
// Revision 1.34  2002/02/07 13:04:54  temperton
// Added Spanish and Polish teletext code pages. Thanks to Jazz (stawiarz).
//
// Revision 1.33  2002/01/19 17:50:41  robmuller
// Clear more data on channel change.
//
// Revision 1.32  2002/01/19 12:53:00  temperton
// Teletext pages updates at correct time.
// Teletext can use variable-width font.
//
// Revision 1.31  2002/01/15 20:25:45  adcockj
// removed old bitmap code
//
// Revision 1.30  2002/01/15 11:16:03  temperton
// New teletext drawing code.
//
// Revision 1.29  2002/01/12 16:56:21  adcockj
// Series of fixes to bring 4.0.0 into line with 3.1.1
//
// Revision 1.28  2001/11/26 12:48:01  temperton
// Teletext corrections
//
// Revision 1.27  2001/11/23 10:45:32  adcockj
// Added Hebrew and Hungarian codepages
//
// Revision 1.26  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.25  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.24  2001/10/22 17:28:40  temperton
// Removed tabs to comply with coding standards
//
// Revision 1.23  2001/10/22 05:55:07  temperton
// Teletext improvements
//
// Revision 1.22  2001/10/06 17:04:26  adcockj
// Fixed teletext crashing problems
//
// Revision 1.21  2001/09/22 11:09:43  adcockj
// Fixed crashing problems with new code with noisy input
//
// Revision 1.20  2001/09/21 16:43:54  adcockj
// Teletext improvements by Mike Temperton
//
// Revision 1.19  2001/09/21 15:39:02  adcockj
// Added Russian and German code pages
// Corrected UK code page
//
// Revision 1.18  2001/09/05 16:22:34  adcockj
// Fix for new teletext painting overwritting other apps
//
// Revision 1.17  2001/09/05 15:08:43  adcockj
// Updated Loging
//
// Revision 1.16  2001/09/05 06:59:13  adcockj
// Teletext fixes
//
// Revision 1.15  2001/09/02 14:17:51  adcockj
// Improved teletext code
//
// Revision 1.14  2001/08/21 09:39:46  adcockj
// Added Greek teletext Codepage
//
// Revision 1.13.2.2  2001/08/23 16:04:57  adcockj
// Improvements to dynamic menus to remove requirement that they are not empty
//
// Revision 1.13.2.1  2001/08/21 09:43:01  adcockj
// Brought branch up to date with latest code fixes
//
// Revision 1.13  2001/08/13 18:07:24  adcockj
// Added Czech code page for teletext
//
// Revision 1.12  2001/08/02 16:43:05  adcockj
// Added Debug level to LOG function
//
// Revision 1.11  2001/07/16 18:07:50  adcockj
// Added Optimisation parameter to ini file saving
//
// Revision 1.10  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.9  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "VBI_VideoText.h"
#include "VBI_CCdecode.h"
#include "VBI.h"
#include "DScaler.h"
#include "AspectRatio.h"
#include "Other.h"
#include "DebugLog.h"
#include "Crash.h"
#include "VTDrawer.h"

TPacket30 Packet30;

TVTPage VTPages[800];
CRITICAL_SECTION VTAccess;
int VTPage = 100;
int VTSubPage = 0;
bool VTSubPageLocked = false;
bool VTShowHidden = false;
bool VTShowFlashed = false;
bool VTFlashTimerSet = false;

CRITICAL_SECTION VTUpdateAccess;
TVTRedrawCache VTRedrawCache;

unsigned int VBIScanPos;

int VBIFPS;
int VTCachedPages = 0;
int VTCurrentPage = 0;
int VTCurrentSubCode = 0;
eVTCodePage VTCodePage = VT_UK_CODE_PAGE;

char VPSTempName[9] = 
{ 
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
char VPSLastName[9] = 
{
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
char VPSChannelName[9] = 
{
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
int VPSNameIndex=0;

unsigned char RevHam[16] = 
{
    0x00, 0x08, 0x04, 0x0c, 0x02, 0x0a, 0x06, 0x0e,
    0x01, 0x09, 0x05, 0x0d, 0x03, 0x0b, 0x07, 0x0f
};

unsigned char UnhamTab[256] = 
{
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

TVTHeaderLine VTHeaderLine;
CVTDrawer VTDrawer;
TVTPage VisiblePage;

#define GetBit(val,bit,mask) (BYTE)(((val)>>(bit))&(mask))

eVTState VTState = VT_OFF;

typedef struct
{
    int Page;
    int SubPage;
    BOOL PageErase;
    BOOL bStarted;
} TMagState;

#define NUM_MAGAZINES 8

TMagState MagazineStates[NUM_MAGAZINES];

TVTPage* VT_PageGet(int Page, int SubPage)
{
    TVTPage* pPage;
    TVTPage* pTemp = NULL;

    EnterCriticalSection(&VTAccess);

    pPage = &VTPages[Page];
    for(;;)
    {
        if ((!pPage->Fill) || (pPage->SubPage == SubPage))
        {
            pPage->SubPage = SubPage;
            pPage->Page = Page;
            pTemp = pPage;
            break;
        }

        if (!pPage->Next) 
        {
            pPage->Next = new TVTPage;
            memset(pPage->Next, NULL, sizeof(TVTPage));
            pPage->Next->SubPage = SubPage;
            pPage->Next->Page = Page;
            pTemp = pPage->Next;
            break;
        }
        pPage = pPage->Next;
    }

    LeaveCriticalSection(&VTAccess);
    return pTemp;
}

void VT_PageFree(int Page)
{
    TVTPage* pPage;
    TVTPage* pTemp;

    EnterCriticalSection(&VTAccess);    

    pPage = &VTPages[Page];
    pPage = pPage->Next;
    VTPages[Page].Next = NULL;
    while (pPage)
    {
        pTemp = pPage->Next;
        delete pPage;
        pPage = pTemp;
    }

    LeaveCriticalSection(&VTAccess);
}

TVTPage* VT_PageNext(TVTPage* pPage)
{
    TVTPage* pTemp;
    EnterCriticalSection(&VTAccess);
    pTemp = pPage->Next;
    LeaveCriticalSection(&VTAccess);
    return pTemp;
}

void VBI_VT_Init()
{
    VT_SetCodePage(VTCodePage);

    memset(VTPages, 0, 800 * sizeof(TVTPage));
    InitializeCriticalSection(&VTAccess);
    InitializeCriticalSection(&VTUpdateAccess);

    VTPage = 100;
    VT_ChannelChange();
}

void VBI_VT_Exit()
{
    int a;

    for (a = 0; a < 800; a++)
    {
        VT_PageFree(a);
    }

    DeleteCriticalSection(&VTAccess);
    DeleteCriticalSection(&VTUpdateAccess);
}

void VBI_decode_vps(unsigned char* data)
{
    unsigned char* Info;

    Info = data;
    if ((Info[3] & 0x80))
    {
        VPSChannelName[VPSNameIndex] = 0;
        if (VPSNameIndex == 8)
        {
            if (strcpy(VPSChannelName, VPSTempName) == 0)
                memcpy(VPSLastName, VPSChannelName, 9);    // VPS-Channel-Name
            strcpy(VPSTempName, VPSChannelName);
        }
        VPSNameIndex = 0;
    }
    VPSChannelName[VPSNameIndex++] = Info[3] & 0x7f;
    if (VPSNameIndex >= 9)
        VPSNameIndex = 0;
    if (ShowVPSInfo != NULL)
        SetDlgItemText(ShowVPSInfo, IDC_TEXT1, VPSLastName);
}

unsigned char VBI_Scan(BYTE* VBI_Buffer, unsigned int step)
{
    int j;
    unsigned char dat;

    for (j = 7, dat = 0; j >= 0; j--, VBIScanPos += step)
        dat |= ((VBI_Buffer[VBIScanPos >> FPSHIFT] + VBIOffset) & 0x80) >> j;
    return dat;
}

// unham 2 bytes into 1, report 2 bit errors but ignore them
unsigned char unham(unsigned char* d)
{
    unsigned char c1, c2;

    c1 = UnhamTab[d[0]];
    c2 = UnhamTab[d[1]];
    return (c2 << 4) | (c1);
}

// unham, but with reversed nibble order for VC
unsigned char unham2(unsigned char* d)
{
    unsigned char c1, c2;

    c1 = UnhamTab[d[0]];
    c2 = UnhamTab[d[1]];
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

void VBI_decode_vt(unsigned char* dat)
{
    int i;
    unsigned char mag, pack, mpag, ftal, ft, al, page;
    unsigned int addr;
    unsigned int pnum = 0;
    unsigned short sub;
    WORD ctrl;
    BOOL bRedrawAll = FALSE;

    TVTPage* pPage;

    // dat: 55 55 27 %MPAG% 
    mpag = unham(dat + 3);
    mag = mpag & 7;
    pack = (mpag >> 3) & 0x1f;

    switch (pack)
    {
    case 0:
        //hdump(udat,4); cout << " HD\n";

        // dat: 55 55 27 %MPAG% %PAGE% %SUB%
        // 00 01 02  03 04  05 06 07-0a

        page = unham(dat + 5);
        pnum = MakePage(mag, page);
        if(pnum < 100 || pnum > 899)
        {
            MagazineStates[mag].bStarted = FALSE;
            return;
        }

        MagazineStates[mag].PageErase = ((unham(dat + 7) & 0x80) != 0);
        sub = ((unham(dat + 9) & 0x3F) << 7) | (unham(dat + 7) & 0x7F);
        sub = (sub >> 4) * 10 + (sub & 0x0F);

        VTCurrentPage = pnum;
        VTCurrentSubCode = sub;

        pnum -= 100;

        if ((pnum >= 0) && (pnum < 800))
        {

            ctrl = (UnhamTab[dat[3]] & 0x7) + ((UnhamTab[dat[8]] >> 3) << 3) + ((UnhamTab[dat[10]] >> 2) << 4) + (UnhamTab[dat[11]] << 6) + (UnhamTab[dat[12]] << 10);

            MagazineStates[mag].Page = pnum;
            MagazineStates[mag].SubPage = sub;
            MagazineStates[mag].bStarted = TRUE;
            LOG(2, "Mag %d Page %d SubCode %d", mag, pnum, sub);

            pPage = VT_PageGet(pnum, sub);
            VTPages[pnum].MostRecentSubPage = sub;
            if(pPage->bUpdated == FALSE)
            {
                ++VTCachedPages;
            }

            memcpy(&pPage->Frame[0], dat + 5, 40);
            memcpy(&VTHeaderLine[0], dat + 5, 40);
            pPage->wCtrl = ctrl;
            pPage->bUpdated = 1;
            pPage->Fill = TRUE;

            if (MagazineStates[mag].PageErase == TRUE)
            {
                memset(&pPage->Frame[1], 0x00, 24 * 40);
                memset(&pPage->LineUpdate[1], 0x00, 24);
            }

            if((VTState != VT_OFF) && (MagazineStates[mag].Page == VTPage - 100) &&
               (MagazineStates[mag].SubPage == VTSubPage))
            {
                bRedrawAll = MagazineStates[mag].PageErase;
            }
            else
            {
                PostMessage(hWnd, WM_VIDEOTEXT, VTM_REDRAWHEADER, NULL);
            }
        }
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
    case 25:
        if(MagazineStates[mag].bStarted)
        {
            int row = (pack == 25) ? 0 : pack;

            pPage = VT_PageGet(MagazineStates[mag].Page, MagazineStates[mag].SubPage);

            memcpy(&pPage->Frame[row], dat + 5, 40);
            pPage->bUpdated = 1;
            pPage->Fill = TRUE;
            pPage->LineUpdate[row] = 1;

            if((MagazineStates[mag].Page == VTPage - 100) && (!VTSubPageLocked))
            {
                if (VTSubPage != MagazineStates[mag].SubPage)
                {
                    VTShowHidden = false;
                    bRedrawAll = TRUE;                    
                    VT_PurgeRedrawCache();
                }
                VTSubPage = MagazineStates[mag].SubPage;
            }

            if((VTState != VT_OFF) && (MagazineStates[mag].Page == VTPage - 100) &&
               (MagazineStates[mag].SubPage == VTSubPage))
            {
                memcpy(&VisiblePage, pPage, sizeof(TVTPage));
                
                EnterCriticalSection(&VTUpdateAccess);
                KillTimer(hWnd, TIMER_VTUPDATE);
                VTRedrawCache.PageErase |= bRedrawAll;
                VTRedrawCache.Line[row] = true;
                SetTimer(hWnd, TIMER_VTUPDATE, TIMER_VTUPDATE_MS, NULL);
                LeaveCriticalSection(&VTUpdateAccess);
            }
        }
        break;
    
    case 26:                    // PDC
    case 27:
        if(MagazineStates[mag].bStarted)
        {
            pPage = VT_PageGet(MagazineStates[mag].Page, MagazineStates[mag].SubPage);
            if (!(UnhamTab[dat[5]] & 0x0F))
            {
                for(i = 1 ; i <= 6; i++)
                {
                    int linkmag = mag ^ ((UnhamTab[dat[ 6 * i + 3]] & 0x08) >> 3) ^ ((UnhamTab[dat[6 * i + 5]] & 0x0c) >> 1);
                    int linkpage = MakePage(linkmag, unham(dat + 6 * i));

                    if(linkpage >=100 && linkpage <= 899)
                    {
                        pPage->FlofPage[i-1] = linkpage;
                        pPage->FlofSubPage[i-1] = (unham(dat + 6 * i + 2) & 0x7F) | ((unham(dat + 6 * i + 4) & 0x3F) << 7);
                    }
                }
            }
            pPage->bFlofUpdated = TRUE;
        }
        break;
    case 28:
    case 29:
        break;
    case 30:
        StorePacket30(dat);
        break;
    case 31:
        ftal = unham(dat + 5);
        al = ftal >> 4;         // address length 
        ft = ftal & 0x0f;
        for (addr = 0, i = 0; i < al; i++)
            addr = (addr << 4) | UnhamTab[dat[7 + i]];

        switch (addr)
        {
        case 0x07:
            break;

        case 0x0f00:            // also used by ZDF and DSF, data format unknown 
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }
}

void StorePacket30(BYTE* p)
{
    DWORD d, b;
    BYTE h, m, s, a, CNI0, CNI1, CNI2, CNI3;
    int n;

    if (*p != 0x55)
        return;                 // Some error, the data should be here...
    p += 5;

    if (UnhamTab[*p] == 0)      // TSDP
    {
        p++;
        Packet30.HomePage.nPage = unham(p);
        Packet30.HomePage.nSubcode = (((UnhamTab[p[5]] & 0x3) << 12) + (UnhamTab[p[4]] << 8) + ((UnhamTab[p[3]] & 0x7) << 4) + UnhamTab[p[2]]);

        Packet30.HomePage.nMag = ((UnhamTab[p[5]] >> 1) & 0x6) + ((UnhamTab[p[3]] >> 3) & 0x1);
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
    else if (UnhamTab[*p] == 2) // PDC
    {
        p++;
        Packet30.HomePage.nPage = unham(p);
        Packet30.HomePage.nSubcode = (((UnhamTab[p[5]] & 0x3) << 12) + (UnhamTab[p[4]] << 8) + ((UnhamTab[p[3]] & 0x7) << 4) + UnhamTab[p[2]]);

        Packet30.HomePage.nMag = ((UnhamTab[p[5]] >> 1) & 0x6) + ((UnhamTab[p[3]] >> 3) & 0x1);
        p += 6;
        a = RevHam[UnhamTab[p[0]]];
        Packet30.PDC.LCI = GetBit(a, 2, 3);
        Packet30.PDC.LUF = GetBit(a, 1, 1);
        Packet30.PDC.PRF = GetBit(a, 0, 1);
        a = RevHam[UnhamTab[p[1]]];
        Packet30.PDC.PCS = GetBit(a, 2, 3);
        Packet30.PDC.MI = GetBit(a, 1, 1);
        CNI0 = RevHam[UnhamTab[p[2]]];
        b = (RevHam[UnhamTab[p[3]]] << 28) + (RevHam[UnhamTab[p[4]]] << 24) +
            (RevHam[UnhamTab[p[5]]] << 20) + (RevHam[UnhamTab[p[6]]] << 16) +
            (RevHam[UnhamTab[p[7]]] << 12) + (RevHam[UnhamTab[p[8]]] << 8) + (RevHam[UnhamTab[p[9]]] << 4) + (RevHam[UnhamTab[p[10]]]);
        CNI2 = GetBit(b, 30, 3);
        Packet30.PDC.day = GetBit(b, 25, 0x1f);
        Packet30.PDC.month = GetBit(b, 21, 0xf);
        Packet30.PDC.hour = GetBit(b, 16, 0x1f);
        Packet30.PDC.minute = GetBit(b, 10, 0x3f);
        CNI1 = GetBit(b, 6, 0xf);
        CNI3 = GetBit(b, 0, 0x3f);
        Packet30.PDC.PTY = (RevHam[UnhamTab[p[11]]] << 4) + RevHam[UnhamTab[p[12]]];;
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

void VT_DoUpdate_Page(int Page, int SubPage)
{
    TVTPage *pPage = VT_PageGet(Page, SubPage);

    memcpy(&VisiblePage, pPage, sizeof(TVTPage));
}

void VT_UpdateFlashTimerStatus()
{
    if(VTState!=VT_OFF)
    {
        bool bHasFlashingElements = false;
        for(int row = 0; (row < 25) && (!bHasFlashingElements); ++row)
        {
            for(int col = 0; (col < 40) && (!bHasFlashingElements); ++col)
            {
                bHasFlashingElements |= ((VisiblePage.Frame[row][col] & 0x7f) == 0x08);
            }
        }

        if(bHasFlashingElements)
        {
            if(!VTFlashTimerSet)
            {
                SetTimer(::hWnd, TIMER_VTFLASHER, TIMER_VTFLASHER_MS, NULL);
                VTFlashTimerSet = true;
            }
        }
        else
        {
            KillTimer(::hWnd, TIMER_VTFLASHER);
            VTFlashTimerSet = false;
        }
    }
    else
    {
        KillTimer(::hWnd, TIMER_VTFLASHER);
        VTFlashTimerSet = false;
    }

}

void VT_ChannelChange()
{
    for (int a = 0; a < 800; a++)
    {
        VT_PageFree(a);
    }
    memset(VPSLastName , 0x00, 9);
    memset(VPSChannelName , 0x00, 9);
    memset(VTPages, 0, 800 * sizeof(TVTPage));
    memset(MagazineStates, 0, sizeof(TMagState) * NUM_MAGAZINES);
    for(int i(0); i < NUM_MAGAZINES; ++i)
    {
        MagazineStates[i].Page = -1;
    }
    VTCachedPages = 0;

    VTPage = 100;
    VTSubPage = 0;
    VTSubPageLocked = false;

    VT_ResetStation();
}


void VT_Redraw(HWND hWnd, HDC hDC, BOOL IsDDhDC, BOOL ShowFlashed)
{
    RECT Rect;
    GetDestRect(&Rect);
    if(IsDDhDC)
    {
        ClientToScreen(hWnd, (LPPOINT)&Rect.left);
        ClientToScreen(hWnd, (LPPOINT)&Rect.right);
    }

    VTDrawer.SetBounds(hDC, &Rect);
    VTDrawer.Draw(&VisiblePage, &VTHeaderLine, hDC, NULL,
        (VTShowHidden ? VTDF_HIDDEN : 0) |
        ((VTState == VT_MIX) ? VTDF_MIXMODE : 0), VTCodePage, NULL);
}

void VT_RedrawFlash(HWND hWnd, HDC hDC, BOOL ShowFlashed)
{
    RECT Rect;
    GetDestRect(&Rect);

    VTDrawer.SetBounds(hDC, &Rect);
    VTDrawer.Draw(&VisiblePage, &VTHeaderLine, hDC, NULL, 
        (ShowFlashed ? VTDF_FLASHONLY : VTDF_CLEARFLASH) |
        ((VTState == VT_MIX) ? VTDF_MIXMODE : 0), VTCodePage, 0);
}

void VT_RedrawClock(HWND hWnd, HDC hDC, bool RedrawHeader)
{
    RECT Rect;
    GetDestRect(&Rect);

    VTDrawer.SetBounds(hDC, &Rect);
    VTDrawer.Draw(&VisiblePage, &VTHeaderLine, hDC, NULL, 
        (RedrawHeader ? VTDF_HEADERONLY : VTDF_CLOCKONLY) |
        ((VTState == VT_MIX) ? VTDF_MIXMODE : 0), VTCodePage, 0);
}

void VT_ProcessRedrawCache(HWND hWnd, HDC hDC)
{
    TVTRedrawCache Cache;

    EnterCriticalSection(&VTUpdateAccess);
    memcpy(&Cache, &VTRedrawCache, sizeof(TVTRedrawCache));
    memset(&VTRedrawCache, 0, sizeof(TVTRedrawCache));
    KillTimer(::hWnd, TIMER_VTUPDATE);
    LeaveCriticalSection(&VTUpdateAccess);

    if(Cache.PageErase)
    {
        VT_Redraw(hWnd, hDC, FALSE, FALSE);
    }
    else
    {
        for(int a = 0; a < 25; a++)
        {
            if(Cache.Line[a])
            {
                VTDrawer.Draw(&VisiblePage, &VTHeaderLine, hDC, NULL,
                    (VTShowHidden ? VTDF_HIDDEN : 0) |
                    ((VTState == VT_MIX) ? VTDF_MIXMODE : 0) |
                    VTDF_THISROWONLY, VTCodePage, a);
            }
        }
    }
}

void VT_PurgeRedrawCache()
{
    EnterCriticalSection(&VTUpdateAccess);
    KillTimer(hWnd, TIMER_VTUPDATE);
    memset(&VTRedrawCache, 0, sizeof(TVTRedrawCache));
    LeaveCriticalSection(&VTUpdateAccess);
}

bool VT_ContainsUpdatedPage()
{
    return (VisiblePage.bUpdated==TRUE);
}

void VT_CreateTestPage()
{
    memset(&VisiblePage, NULL, sizeof(VisiblePage));
    memset(&VisiblePage.Frame, 32, sizeof(VisiblePage.Frame));
    
    VisiblePage.Page = 800;
    memcpy(&VisiblePage.Frame[0][8], "DScaler Charset Test", 20);

    for(int row = 0; row < 3; row++)
    {
        for(int col = 0; col < 32; col++)
        {
            VisiblePage.Frame[row+10][col+3] = 32 + row * 32 + col;
        }
    }

    VisiblePage.bUpdated = true;
}

//////////////////////////////////////////////////////////////////////
void VT_DecodeLine(BYTE* VBI_Buffer, int line, BOOL IsOdd)
{
    unsigned char data[45];
    unsigned char min, max;
    int dt[256], hi[6], lo[6];
    int i, n, sync, thr;

    // remove DC. edge-detector 
    for (i = 40; i < 240; ++i)
    {
        dt[i] = VBI_Buffer[i + VTStep / FPFAC] - VBI_Buffer[i]; // amplifies the edges best.
    }

    // set barrier 
    for (i = 240; i < 256; i += 2)
    {
        dt[i] = 100;
        dt[i+1] = -100;
    }

    // find 6 rising and falling edges 
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

    // AGC and sync-reference 
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

    // search start-byte 11100100 
    //for (i = 4 * VTStep + vbi->pll_adj*VTStep/10; i < 16*VTStep; i += VTStep)
    for (i = 4 * VTStep; i < (int)(16*VTStep); i += VTStep)
    {
        if (VBI_Buffer[sync + i/FPFAC] > thr && VBI_Buffer[sync + (i+VTStep)/FPFAC] > thr) // two ones is enough...
        {
            // got it... 
            memset(data, 0, sizeof(data));
            data[0] = 0x55;
            data[1] = 0x55;
            for (n = 0; n < 43*8; ++n, i += VTStep)
            {
                if (VBI_Buffer[sync + i/FPFAC] + 
                    VBI_Buffer[sync + i/FPFAC - 1] + 
                    VBI_Buffer[sync + i/FPFAC + 1] > 3 * thr)
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
    VBIScanPos = p << FPSHIFT;
    if ((data[0] = VBI_Scan(VBI_Buffer, VPSStep)) != 0xff)
        return;
    if ((data[1] = VBI_Scan(VBI_Buffer, VPSStep)) != 0x5d)
        return;
    for (i = 2; i < 16; i++)
    {
        data[i] = VBI_Scan(VBI_Buffer, VPSStep);
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
        SetDlgItemInt(hDlg, IDC_TEXT1, VTCachedPages, FALSE);
        SetDlgItemInt(hDlg, IDC_TEXT2, VTCurrentPage, FALSE);
        SetDlgItemInt(hDlg, IDC_TEXT3, VTCurrentSubCode, FALSE);
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


void VT_SetCodePage(eVTCodePage Codepage)
{
    VTCodePage = Codepage;
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

void VT_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < VT_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(VTSettings[i]), bOptimizeFileAccess);
    }
}

bool IsNum(TVTPage *pPage, int x, int y)
{
    if((x > 39) || (y > 24) || (x < 0) || (y < 0))
    {
        return false;
    }

    int n = int(pPage->Frame[y][x] & 0x7F) - 0x30;
    if((n < 0) || (n > 9))
        return false;

    return true;
}

int VT_GetNumberHere(TVTPage *pPage, int x, int y)
{
    int xto = x;
    int xfrom = x;

    while (IsNum(pPage, xto++, y))
    {
        ;
    }
    xto-=2;

    while (IsNum(pPage, xfrom--, y))
    {
        ;
    }
    xfrom+=2;

    if(xto - xfrom < 0)
    {
        return 0;
    }

    int mul = 1; 
    int n = 0;
    for (int a = xto; a >= xfrom; --a) 
    {
        n += (pPage->Frame[y][a] & 0x7F - 0x30) * mul;
        mul*=10;
    }

    return n;
}

bool VT_IsLineHasDouble(TVTPage *pPage, int y)
{
    for(int x = 0; x < 40; x++)
    {
        if(pPage->Frame[y][x]==0x0D)
        {
            return true;
        }
    }

    return false;    
}
        
int VT_GetPageNumberAt(int Page, int SubPage, int x, int y)
{
    float dx, dy;
    float width, height;

    if ((Page < 0) || (Page > 899))
    {
        return 0;
    }

    TVTPage *pPage = VT_PageGet(Page, SubPage);
    if(!pPage->bUpdated)
    {
        return 0;
    }

    RECT dest;
    GetDestRect(&dest);
    if((dest.left <= x) && (x < dest.right) && (dest.top <= y) && (y < dest.bottom)) 
    {
        x-=dest.left;
        y-=dest.top;
        width = (dest.right - dest.left);
        height = (dest.bottom - dest.top);
        dx = width / 40.0;
        dy = height / 25.0;
        
        x = (dx) ? (float(x) / dx) : 0;
        y = (dy) ? (float(y) / dy) : 0;

        if(y > 0 && VT_IsLineHasDouble(pPage, y-1))
        {
            --y;
        }
    }
    else
    {
        return 0;
    }

    return VT_GetNumberHere(pPage, x, y);
}

int VT_GetFlofPageNumber(int Page, int SubPage, int flof)
{
    if ((Page < 0) || (Page > 799))
    {
        return 0;
    }

    TVTPage* pPage = VT_PageGet(Page, SubPage);
    if (!pPage->bFlofUpdated)
    {
        return 0;
    }

    if (pPage->FlofPage[flof] < 100 ||  pPage->FlofPage[flof] > 899)
    {
        return 0;
    }
    else
    {
        return pPage->FlofPage[flof];
    }
}

int VT_MostRecentSubPage(int Page)
{
    if ((Page < 0) || (Page > 799))
    {
        return 0;
    }

    return VTPages[Page].MostRecentSubPage;
}

int VT_CompleteSubPages(int Page)
{
    int a = 0;
    TVTPage* pPage = &VTPages[Page];

    do 
    {
        if (pPage->bUpdated)
        {
            a++;
        }
        pPage = VT_PageNext(pPage);
    } while (pPage);

    return a;
}

int VT_SubPageNext(int Page, int SubPage, int Direction, bool Cycle)
{
    if ((!Direction) || (Page < 0) || (Page > 799))
    {
        return SubPage;
    }

    TVTPage* pPage = &VTPages[Page];
    TVTPage* pBest = NULL;
    int BestSubPage = (Direction > 0) ? 65535 : -1;
    bool found = false;

    do
    {
        if ((((pPage->SubPage > SubPage) && (Direction > 0) && (pPage->SubPage < BestSubPage)) ||
            ((pPage->SubPage < SubPage) && (Direction < 0) && (pPage->SubPage > BestSubPage))) &&
            (pPage->bUpdated))
        {
            BestSubPage = pPage->SubPage;
            pBest = pPage;
        }
        pPage = VT_PageNext(pPage);
    } while (pPage);

    if ((Cycle) && (!pBest))
    {
        return VT_SubPageNext(Page, (Direction > 0) ? -1 : 65535, Direction, false);
    }

    return (pBest) ? pBest->SubPage : VT_MostRecentSubPage(Page);
}

void VT_SetMenu(HMENU hMenu)
{
    CheckMenuItemBool(hMenu, IDM_VT_UK, (VTCodePage == VT_UK_CODE_PAGE));
    CheckMenuItemBool(hMenu, IDM_VT_FRENCH, (VTCodePage == VT_FRENCH_CODE_PAGE));
    CheckMenuItemBool(hMenu, IDM_VT_CZECH, (VTCodePage == VT_CZECH_CODE_PAGE));
    CheckMenuItemBool(hMenu, IDM_VT_GREEK, (VTCodePage == VT_GREEK_CODE_PAGE));
    CheckMenuItemBool(hMenu, IDM_VT_RUSSIAN, (VTCodePage == VT_RUSSIAN_CODE_PAGE));
    CheckMenuItemBool(hMenu, IDM_VT_GERMAN, (VTCodePage == VT_GERMAN_CODE_PAGE));
    CheckMenuItemBool(hMenu, IDM_VT_HUNGARIAN, (VTCodePage == VT_HUNGARIAN_CODE_PAGE));
    CheckMenuItemBool(hMenu, IDM_VT_HEBREW, (VTCodePage == VT_HEBREW_CODE_PAGE));
    CheckMenuItemBool(hMenu, IDM_VT_SWEDISH, (VTCodePage == VT_SWEDISH_CODE_PAGE));
    CheckMenuItemBool(hMenu, IDM_VT_ITALIAN, (VTCodePage == VT_ITALIAN_CODE_PAGE));
    CheckMenuItemBool(hMenu, IDM_VT_SPANISH, (VTCodePage == VT_SPANISH_CODE_PAGE));
    CheckMenuItemBool(hMenu, IDM_VT_POLISH, (VTCodePage == VT_POLISH_CODE_PAGE));
}

