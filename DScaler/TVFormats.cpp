/////////////////////////////////////////////////////////////////////////////
// $Id: TVFormats.cpp,v 1.5 2001-11-23 10:49:17 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
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
// Revision 1.4  2001/11/18 10:07:00  temperton
// Bug fix.
//
// Revision 1.3  2001/11/02 17:03:59  adcockj
// Merge in PAL_NC change again
//
// Revision 1.2  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.1.2.1  2001/08/17 16:35:14  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "TVFormats.h"
#include "BT848_Defines.h"


static TTVFormat TVFormats[FORMAT_LASTONE] =
{
    // PAL-BDGHI 
    { 
        "PAL", 576, 1135, 0x7f, 0x72, (BT848_IFORM_PAL_BDGHI|BT848_IFORM_XT1),
        186, 922, 0x24, 0, TRUE, 511, 19,
        4.43361875, TRUE, 71, 626, 15,
        16,
    },
    // NTSC 
    {
        "NTSC", 480, 910, 0x68, 0x5c, (BT848_IFORM_NTSC|BT848_IFORM_XT0),
        137, 754, 0x1a, 0, FALSE, 400, 13,
        3.579545,  FALSE, 57, 512, 11, 
        10,
    },
    // SECAM 
    {
        "SECAM", 576, 1135, 0x7f, 0xb0, (BT848_IFORM_SECAM|BT848_IFORM_XT1),
        186, 922, 0x22, 0, TRUE, 511, 19,
        4.43361875, TRUE, 71, 633, 15,
        16,
    },
    // PAL-M 
    {
        "PAL-M", 480,  910, 0x68, 0x5c, (BT848_IFORM_PAL_M|BT848_IFORM_XT0),
        137, 754, 0x1a, 0, FALSE, 400, 13,
        3.579545,  FALSE, 57, 512, 11,
        10,
    },
    // PAL-N 
    {
        "PAL-N", 576, 1135, 0x7f, 0x72, (BT848_IFORM_PAL_N|BT848_IFORM_XT1),
        186, 922, 0x20, 0, TRUE, 511, 19,
        4.43361875, TRUE,  71, 626, 15,
        16,
    },
    // NTSC Japan
    {
        "NTSC Japan", 480,  910, 0x70, 0x5c, (BT848_IFORM_NTSC_JAP|BT848_IFORM_XT0),
        135, 754, 0x1a, 0, FALSE, 400, 13,
        3.579545, FALSE, 57, 512, 11, 
        10,
    },
    // PAL-60 
    {
        "PAL60", 480, 1135, 0x7f, 0x72, (BT848_IFORM_PAL_BDGHI|BT848_IFORM_XT1),
        186, 922, 0x20, 0, FALSE, 400, 16,
        4.43361875, TRUE, 70, 626, 14,
        13,
    },
    // NTSC-50    
    {       
        "NTSC50", 576, 910, 0x68, 0x5c, (BT848_IFORM_NTSC|BT848_IFORM_XT0),
        137, 754, 0x24, 0, TRUE, 511, 19,
        3.579545,  FALSE, 71, 626, 15,      
        16, 
    },
    // PAL-NC thanks to Eduardo José Tagle
    { 
        "PAL-NC", 576, 916, 0x68, 0x5c, (BT848_IFORM_PAL_NC|BT848_IFORM_XT0), 
        149, 745, 0x20, 0, TRUE, 511, 19, 
        3.579545, FALSE, 71, 626, 15, 
        16, 
    }, 
};

TTVFormat* GetTVFormat(eVideoFormat Format)
{
    return &TVFormats[Format];
}


const char* FormatList[FORMAT_LASTONE] =
{
    "PAL", 
    "NTSC",
    "SECAM",
    "PAL-M",
    "PAL-N",
    "NTSC_J",
    "PAL60",
    "NTSC50",
    "PAL-NC"
};

