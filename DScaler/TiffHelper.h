/////////////////////////////////////////////////////////////////////////////
// $Id: TiffHelper.h,v 1.5 2002-04-15 22:50:09 laurentg Exp $
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.4  2002/04/14 17:25:26  laurentg
// New formats of TIFF files supported to take stills : Class R (RGB) with compression LZW or Packbits or JPEG
//
// Revision 1.3  2001/12/05 00:08:41  laurentg
// Use of LibTiff DLL
//
// Revision 1.2  2001/11/29 14:04:07  adcockj
// Added Javadoc comments
//
// Revision 1.1  2001/11/28 16:04:50  adcockj
// Major reorganization of STill support
//
// Revision 1.4  2001/11/25 21:25:02  laurentg
// Method ReadNextFrameInFile moved to public section
//
// Revision 1.3  2001/11/25 10:41:26  laurentg
// TIFF code moved from Other.cpp to TiffSource.c + still capture updated
//
// Revision 1.2  2001/11/24 22:51:20  laurentg
// Bug fixes regarding still source
//
// Revision 1.1  2001/11/24 17:55:23  laurentg
// CTiffSource class added
//
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __TIFFHELPER_H___
#define __TIFFHELPER_H___

#include "StillSource.h"


enum eTIFFClass {
    TIFF_CLASS_R = 0,
    TIFF_CLASS_R_JPEG,
    TIFF_CLASS_Y,
};


/** A helper class that can read and write TIFF files
*/
class CTiffHelper : public CStillSourceHelper
{
public:
    CTiffHelper(CStillSource* pParent, eTIFFClass FormatSaving);
    BOOL OpenMediaFile(LPCSTR FileName);
    void SaveSnapshot(LPCSTR FilePath, int Height, int Width, BYTE* pOverlay, LONG OverlayPitch);

private:
    eTIFFClass  m_FormatSaving;
};

#endif