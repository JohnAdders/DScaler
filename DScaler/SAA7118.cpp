/////////////////////////////////////////////////////////////////////////////
//
// $Id: SAA7118.cpp,v 1.3 2002-09-26 16:32:34 adcockj Exp $
//
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 John Adcock.  All rights reserved.
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
// $Log: not supported by cvs2svn $
// Revision 1.2  2002/09/19 22:10:08  adcockj
// Holo3D Fixes for PAL
//
// Revision 1.1  2002/09/11 18:32:43  adcockj
// Preliminary support for H3D card
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SAA7118.h"
#include "DebugLog.h"

CSAA7118::CSAA7118()
{
}

void CSAA7118::SetBrightness(BYTE Brightness)
{
	WriteToSubAddress(0x0a, &Brightness, 1);
}

void CSAA7118::SetComponentBrightness(BYTE Brightness)
{
	WriteToSubAddress(0x2a, &Brightness, 1);
}

void CSAA7118::SetContrast(BYTE Contrast)
{
    // Contrast limited to 0-128
    Contrast = Contrast >> 1;
	WriteToSubAddress(0x0b, &Contrast, 1);
}

void CSAA7118::SetComponentContrast(BYTE Contrast)
{
    // Contrast limited to 0-128
    Contrast = Contrast >> 1;
	WriteToSubAddress(0x2b, &Contrast, 1);
}

void CSAA7118::SetHue(BYTE Hue)
{
    Hue = (BYTE)(signed char)(Hue - 0x80);
	WriteToSubAddress(0x0d, &Hue, 1);
}

void CSAA7118::SetSaturation(BYTE Saturation)
{
    // Saturation limited to 0-128
    Saturation = Saturation >> 1;
	WriteToSubAddress(0x0c, &Saturation, 1);
}


void CSAA7118::SetComponentSaturation(BYTE Saturation)
{
    // Saturation limited to 0-128
    Saturation = Saturation >> 1;
	WriteToSubAddress(0x2c, &Saturation, 1);
}

BYTE CSAA7118::GetVersion()
{
	BYTE Result(0);
	ReadFromSubAddress(0x00, &Result, 1);
	return Result;
}

void CSAA7118::SetRegister(BYTE Register, BYTE Value)
{
	WriteToSubAddress(Register, &Value, 1);
}


void CSAA7118::DumpRegister(FILE* hFile, BYTE Reg)
{
	BYTE Result(0);
	ReadFromSubAddress(Reg, &Result, 1);
	fprintf(hFile, "%02x\t%02x\n", Reg, Result);
}

void CSAA7118::DumpSettings()
{
    FILE* hFile;

    hFile = fopen("SAA7118.txt", "w");
    if(!hFile)
    {
        return;
    }

	for(int i(0); i < 0x2d; ++i)
	{
		DumpRegister(hFile, i);
	}

    for(i = 0x40; i <= 0x62; ++i)
	{
		DumpRegister(hFile, i);
	}

    for(i = 0x80; i <= 0xBF; ++i)
	{
		DumpRegister(hFile, i);
	}

	fclose(hFile);
}

BYTE CSAA7118::GetDefaultAddress()const
{
    return 0x42>>1;
}
