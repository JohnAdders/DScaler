/////////////////////////////////////////////////////////////////////////////
// $Id: TiffSource.h,v 1.4 2001-11-25 21:25:02 laurentg Exp $
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

#ifndef __TIFFSOURCE_H___
#define __TIFFSOURCE_H___

#include "StillSource.h"


#pragma pack(1)


// Field data types.
enum eTiffDataType
{
    Byte = 1,
    String = 2,
    Short = 3,
    Long = 4
};


// A TIFF image-file directory entry.  There are a bunch of
// these in a TIFF file.
typedef struct 
{
    WORD Tag;       // Entry Type
    WORD Type;      // 1=byte, 2=C string, 3=word, 4=dword (we always use dword)
    DWORD Count;    // Number of units (of Type specified by "Type") in Value
    DWORD Value;
} TTiffDirEntry;


// A TIFF header with some hardwired fields.
typedef struct 
{
    char byteOrder[2];
    WORD version;
    DWORD firstDirOffset;

    // TIFF files contain a bunch of extra information, each of which is a
    // tagged "directory" entry.  The entries must be in ascending numerical
    // order.

    WORD numDirEntries;
    TTiffDirEntry fileType;      // What kind of file this is (Tag 254)
    TTiffDirEntry width;         // Width of image (Tag 256)
    TTiffDirEntry height;            // Height of image (Tag 257)
    TTiffDirEntry bitsPerSample; // Number of bits per channel per pixel (Tag 258)
    TTiffDirEntry compression;   // Compression settings (Tag 259)
    TTiffDirEntry photometricInterpretation; // What kind of pixel data this is (Tag 262)
    TTiffDirEntry description;   // Image description (Tag 270)
    TTiffDirEntry make;          // "Scanner" maker, aka DScaler's URL (Tag 271)
    TTiffDirEntry model;         // "Scanner" model, aka DScaler version (Tag 272)
    TTiffDirEntry stripOffset;   // Offset to image data (Tag 273)
    TTiffDirEntry samplesPerPixel; // Number of color channels (Tag 277)
    TTiffDirEntry rowsPerStrip;  // Number of rows in a strip (Tag 278)
    TTiffDirEntry stripByteCounts; // Number of bytes per strip (Tag 279)
    TTiffDirEntry planarConfiguration; // Are channels interleaved? (Tag 284)
    DWORD nextDirOffset;

    char descriptionText[80];
    char makeText[40];
    char modelText[16];
    WORD bitCounts[3];
} TTiffHeader;


class CTiffSource : public CStillSource
{
public:
    CTiffSource(LPCSTR FilePath);
    CTiffSource::CTiffSource(LPCSTR FilePath, int FrameHeight, int FrameWidth, BYTE* pOverlay, LONG OverlayPitch);
    BOOL ReadNextFrameInFile();
    BOOL WriteFrameInFile();

private:
    void FillTiffDirEntry(TTiffDirEntry* entry, WORD Tag, DWORD Value, eTiffDataType Type);
    void FillTiffHeader(TTiffHeader* head, char* description, char* make, char* model);
};

#endif