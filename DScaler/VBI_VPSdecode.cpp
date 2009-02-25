/////////////////////////////////////////////////////////////////////////////
// $Id$
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
// Copyright (C) 1999-2000 Zolt�n Sinkovics and Szabolcs Sel�f
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
// Revision 1.11  2005/08/14 15:13:48  to_see
// Added PCS, PTY to VPS Info dialog
//
// Revision 1.10  2005/08/07 18:47:28  to_see
// Fixed: No VPS data at Pixel Width 720 and CX2388x chip
//
// Revision 1.9  2005/08/03 19:53:05  to_see
// VPS Info Dialog improved
// Added Audio info
//
// Revision 1.8  2005/08/02 19:57:17  to_see
// Improved VPS decoding
//
// Revision 1.7  2005/07/29 16:29:35  to_see
// Fixed VPS decoding bug
//
// Revision 1.6  2005/07/27 22:57:45  laurentg
// New function to search the channel with the VPS CNI
//
// Revision 1.5  2004/01/16 09:14:03  adcockj
// Fixed a couple of bugs spotted by Robert Schlabbach
//
// Revision 1.4  2003/10/27 10:39:54  adcockj
// Updated files for better doxygen compatability
//
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
#include "VBI_VideoText.h"

int VPSStep;

HWND hVPSInfoWnd = NULL;

// VPS decoded data
TVPSDataStruct VPS_Data = {FALSE, 0, 0, 0, 0, 0, 0, 0, 0, 0, VPSAUDIO_UNKNOWN, 0};


void VBI_VPS_Init()
{
    VPS_Clear_Data();
}


void VBI_VPS_Exit()
{
}


void VPS_ChannelChange()
{
    VPS_Clear_Data();
}


void VPS_Init_Data(double VBI_Frequency)
{
    VPSStep = (int) ((VBI_Frequency / 4.995627) * FPFAC + 0.5);
}

void VPS_Clear_Data()
{
    VPS_Data.Valid      = FALSE;
    VPS_Data.CNI        = 0;
    VPS_Data.LabelIndex = 0;
    VPS_Data.Month      = 0;
    VPS_Data.Day        = 0;
    VPS_Data.Hour       = 0;
    VPS_Data.Minute     = 0;
    VPS_Data.Audio      = VPSAUDIO_UNKNOWN;
    VPS_Data.PTY        = 0;
    ZeroMemory(VPS_Data.LabelTemp, 9);
    ZeroMemory(VPS_Data.LabelLast, 9);
    ZeroMemory(VPS_Data.LabelCurr, 9);
}

void VPS_GetChannelNameFromCNI(LPSTR lpBuffer, LONG nLength)
{
    ASSERT(nLength > 0);

    lpBuffer[0] = '\0';

	if (VPS_Data.CNI != 0)
	{
		//LOG(1, "VPS CNI Code %x", VPS_Data.CNI);
		for (int i(0); i < iNbRegisteredCNICodes; i++)
		{
			if (RegisteredCNICodes[i].wCNI_VPS == VPS_Data.CNI)
			{
				strncpy(lpBuffer, RegisteredCNICodes[i].sNetwork, nLength-1);
				lpBuffer[nLength] = '\0';
				break;
			}
		}
	}
}


BYTE ReverseBitOrder( BYTE b )
{
    BYTE result = b & 0x1;

    for (int i=0; i < 7; i++)
    {
        b >>= 1;
        result <<= 1;
        result |= b & 0x1;
    }

    return result;
}

//
// VPS Byte 3 and 4 are marked in datasheet as "not relevant for VPS"
// but I do get any ASCII chars from Byte 3. I named it "Label" in hope it is ok.
//
void VPS_DecodeLabel(BYTE b)
{
    if(b & 0x80)
    {
        VPS_Data.LabelCurr[VPS_Data.LabelIndex] = 0;

        if(VPS_Data.LabelIndex == 8)
        {
            if(strcmp(VPS_Data.LabelCurr, VPS_Data.LabelTemp) == 0)
            {
                memcpy(VPS_Data.LabelLast, VPS_Data.LabelCurr, 9);
            }

            strcpy(VPS_Data.LabelTemp, VPS_Data.LabelCurr);
        }

        VPS_Data.LabelIndex = 0;
    }
        
    VPS_Data.LabelCurr[VPS_Data.LabelIndex++] = b & 0x7f;

    if(VPS_Data.LabelIndex >= 9)
    {
        VPS_Data.LabelIndex = 0;
    }
}


void VPS_DecodeLine(BYTE* data)
{
    VPS_DecodeLabel(ReverseBitOrder(data[3]));

    VPS_Data.CNI    = ((data[12] & 0x3) << 10) | ((data[13] & 0xc0) << 2) |
                      ((data[10] & 0xc0)) | (data[13] & 0x3f);
    VPS_Data.Month  = ((data[11] & 0xe0) >> 5) | ((data[10] & 1) << 3);
    VPS_Data.Day    = (data[10] & 0x3e) >> 1;
    VPS_Data.Hour   = (data[11] & 0x1f);
    VPS_Data.Minute = (data[12] >> 2);
    VPS_Data.Audio  = (eVPSAudio) ((data[4] & 0xc0) >> 6);
    VPS_Data.PTY    = data[14];
}


BYTE VBI_Scan(BYTE* VBI_Buffer, UINT step, UINT* scanPos)
{
    int j;
    BYTE dat;

    for(j = 7, dat = 0; j >= 0; j--, *scanPos += step)
    {
        dat |= ((VBI_Buffer[*scanPos >> FPSHIFT] + VBIOffset) & 0x80) >> j;
    }
    
    return dat;
}


BYTE VPS_Scan(const BYTE* VBI_Buffer, UINT step, UINT* scanPos, BOOL* pError)
{
    int j;
    BYTE bit;
    BYTE dat;

    for(j = 0, dat = 0; j < 8; j++, *scanPos += step * 2)
    {
        bit = ((VBI_Buffer[*scanPos >> FPSHIFT] + VBIOffset) & 0x80);
        
        if (bit == ((VBI_Buffer[(*scanPos + step) >> FPSHIFT] + VBIOffset) & 0x80))
        {
            // bi-phase encoding error, i.e. bit is not followed by it's inverse
            *pError = TRUE;
            break;
        }
        
        dat |= bit >> j;
    }

    return dat;
}


void VBI_DecodeLine_VPS(BYTE* VBI_Buffer)
{
    BOOL bError = TRUE;

    int p = VPSStep * 12 / FPFAC;
    
    while((VBI_Buffer[p] < VBI_thresh) && (VPSStep * 37 / FPFAC))
    {
        p++;
    }

    UINT scanPos = (p + 2) << FPSHIFT;

    if( (VBI_Scan(VBI_Buffer, VPSStep, &scanPos) == 0x55) &&    // VPS run in
        (VBI_Scan(VBI_Buffer, VPSStep, &scanPos) == 0x55) &&
        (VBI_Scan(VBI_Buffer, VPSStep, &scanPos) == 0x51) &&    // VPS start code
        (VBI_Scan(VBI_Buffer, VPSStep, &scanPos) == 0x99) )
    {
        bError = FALSE;
        BYTE data[15] = {0xff, 0x5d};

        for(int i = 2; i <= 14; i++)
        {
            data[i] = VPS_Scan(VBI_Buffer, VPSStep, &scanPos, &bError);
        }

        if(!bError)
        {
            // for debug only
            // TRACE("%02x %02x %02x %02x %02x %02x %02x %02x ",  data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
            // TRACE("%02x %02x %02x %02x %02x %02x %02x %02x\n", data[8], data[9], data[10], data[11], data[12], data[13], data[14]);

            VPS_DecodeLine(data);
        }
    }

    VPS_Data.Valid = !bError;
}


BOOL APIENTRY VPSInfoProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    char buffer[50];

    switch (message)
    {
    case WM_INITDIALOG:
        hVPSInfoWnd = hDlg;
        SetTimer(hDlg, 100, 500, NULL);
        // no break

    case WM_TIMER:
        if(VPS_Data.Valid)
        {
            VPS_GetChannelNameFromCNI(buffer, sizeof(buffer));
            SetDlgItemText(hDlg, IDC_VPS_NAME, buffer);

            sprintf(buffer, "0x%0x", VPS_Data.CNI);
            SetDlgItemText(hDlg, IDC_VPS_CNI, buffer);
            
            if(VPS_Data.Day == 0)
            {

                switch(VPS_Data.Hour)
                {
                case 31:
                    strcpy(buffer, "Timer Control");
                    break;
                case 30:
                    strcpy(buffer, "Pause");
                    break;
                case 29:
                    strcpy(buffer, "Interrupt");
                    break;
                case 28:
                    strcpy(buffer, "Continue");
                    break;
                default:
                    strcpy(buffer, "Unknown");
                    break;
                }

                SetDlgItemText(hDlg, IDC_VPS_SERVICE, buffer);
                SetDlgItemText(hDlg, IDC_VPS_MONTH, "");
                SetDlgItemText(hDlg, IDC_VPS_TIME,  "");
            }

            else
            {
                SetDlgItemText(hDlg, IDC_VPS_SERVICE, "");
                
                sprintf(buffer, "%02d.%02d", VPS_Data.Day, VPS_Data.Month);
                SetDlgItemText(hDlg, IDC_VPS_MONTH, buffer);

                sprintf(buffer, "%02d:%02d", VPS_Data.Hour, VPS_Data.Minute);
                SetDlgItemText(hDlg, IDC_VPS_TIME, buffer);
            }

            switch(VPS_Data.Audio)
            {
            case VPSAUDIO_UNKNOWN:
                strcpy(buffer, "Don't know");
                break;
            case VPSAUDIO_MONO:
                strcpy(buffer, "Mono");
                break;
            case VPSAUDIO_STEREO:
                strcpy(buffer, "Stereo");
                break;
            case VPSAUDIO_DUAL:
                strcpy(buffer, "Dual Sound");
                break;
            default:
                strcpy(buffer, "Error");
                break;
            }
            
            SetDlgItemText(hDlg, IDC_VPS_AUDIO, buffer);
            SetDlgItemText(hDlg, IDC_VPS_LABEL, VPS_Data.LabelTemp);

            sprintf(buffer, "0x%02x", VPS_Data.PTY);
            SetDlgItemText(hDlg, IDC_VPS_PTY, buffer);
        }

        else
        {
            SetDlgItemText(hDlg, IDC_VPS_NAME,    "");
            SetDlgItemText(hDlg, IDC_VPS_CNI,     "");
            SetDlgItemText(hDlg, IDC_VPS_MONTH,   "");
            SetDlgItemText(hDlg, IDC_VPS_TIME,    "");
            SetDlgItemText(hDlg, IDC_VPS_AUDIO,   "");
            SetDlgItemText(hDlg, IDC_VPS_SERVICE, "");
            SetDlgItemText(hDlg, IDC_VPS_LABEL,   "");
            SetDlgItemText(hDlg, IDC_VPS_PTY,     "");
        }

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
