/////////////////////////////////////////////////////////////////////////////
// $Id: VBI_VPSdecode.cpp,v 1.4 2003-10-27 10:39:54 adcockj Exp $
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
// 29 Dec 2002   Atsushi Nakagawa      Moved VPS stuff out of VBI_VideoText.*
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.3  2003/01/07 16:49:11  adcockj
// Changes to allow variable sampling rates for VBI
//
// Revision 1.2  2003/01/05 18:35:45  laurentg
// Init function for VBI added
//
// Revision 1.1  2003/01/01 20:35:32  atnak
// Moved VPS stuff out of VBI_VideoText.*
//
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file VBI_VPSdecode.cpp VBI VPS functions
 */

#include "stdafx.h"
#include "DScaler.h"
#include "VBI.h"
#include "VBI_VPSdecode.h"


char VPSTempName[9];
char VPSLastName[9];
char VPSChannelName[9];

int VPSNameIndex = 0;
int VBIFPS = 0;
HWND hVPSInfoWnd = NULL;

int VPSStep;


void VBI_VPS_Init()
{
    VPS_ChannelChange();
}


void VBI_VPS_Exit()
{
    ;
}


void VPS_Init_Data(double VBI_Frequency)
{
    VPSStep = 2 * (int) ((VBI_Frequency / 5.0) * FPFAC + 0.5);
}


void VPS_ChannelChange()
{
    memset(VPSTempName, 0, 9);
    memset(VPSLastName, 0, 9);
    memset(VPSChannelName, 0, 9);

    VPSNameIndex = 0;
}


void VPS_GetChannelName(LPSTR lpBuffer, LONG nLength)
{
    if (nLength > 10)
    {
        nLength = 10;
    }

    ASSERT(nLength > 0);

    lpBuffer[--nLength] = '\0';
    memcpy(lpBuffer, VPSLastName, nLength);
}


void VPS_DecodeLine(BYTE* data)
{
    if ((data[3] & 0x80))
    {
        VPSChannelName[VPSNameIndex] = 0;

        if (VPSNameIndex == 8)
        {
            if (strcpy(VPSChannelName, VPSTempName) == 0)
            {
                memcpy(VPSLastName, VPSChannelName, 9);    // VPS-Channel-Name
            }
            strcpy(VPSTempName, VPSChannelName);
        }

        VPSNameIndex = 0;
    }

    VPSChannelName[VPSNameIndex++] = data[3] & 0x7f;

    if (VPSNameIndex >= 9)
    {
        VPSNameIndex = 0;
    }
    if (hVPSInfoWnd != NULL)
    {
        SetDlgItemText(hVPSInfoWnd, IDC_TEXT1, VPSLastName);
    }
}


BYTE VBI_Scan(BYTE* VBI_Buffer, UINT step, UINT* scanPos)
{
    int j;
    BYTE dat;

    for (j = 7, dat = 0; j >= 0; j--, *scanPos += step)
    {
        dat |= ((VBI_Buffer[*scanPos >> FPSHIFT] + VBIOffset) & 0x80) >> j;
    }
    return dat;
}


void VBI_DecodeLine_VPS(BYTE* VBI_Buffer)
{
    BYTE data[45];
    int i, p;
    UINT scanPos;

    p = VPSStep * 21 / FPFAC;
    while ((VBI_Buffer[p] < VBI_thresh) && (p < (VPSStep * 37 / FPFAC)))
    {
        p++;
    }

    scanPos = (p + 2) << FPSHIFT;
    if ((data[0] = VBI_Scan(VBI_Buffer, VPSStep, &scanPos)) != 0xff)
    {
        return;
    }
    if ((data[1] = VBI_Scan(VBI_Buffer, VPSStep, &scanPos)) != 0x5d)
    {
        return;
    }
    for (i = 2; i < 16; i++)
    {
        data[i] = VBI_Scan(VBI_Buffer, VPSStep, &scanPos);
    }

    VPS_DecodeLine(data);
}


BOOL APIENTRY VPSInfoProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{

    switch (message)
    {
    case WM_INITDIALOG:
        hVPSInfoWnd = hDlg;
        SetTimer(hDlg, 100, 1000, NULL);
        break;

    case WM_TIMER:
        SetDlgItemInt(hDlg, IDC_VBI_FPS, VBIFPS, FALSE);
        break;

    case WM_COMMAND:
        if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
        {
            hVPSInfoWnd = NULL;
            KillTimer(hDlg, 100);
            EndDialog(hDlg, TRUE);
        }
        break;
    }

    return (FALSE);
}
