/////////////////////////////////////////////////////////////////////////////
// $Id: TiffSource.cpp,v 1.2 2001-11-24 22:51:20 laurentg Exp $
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
// Revision 1.1  2001/11/24 17:55:23  laurentg
// CTiffSource class added
//
//
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include "stdafx.h"
#include "TiffSource.h"
#include "Other.h"
#include "DebugLog.h"

// A TIFF image-file directory entry.  There are a bunch of
// these in a TIFF file.
typedef struct 
{
    WORD Tag;       // Entry Type
    WORD Type;      // 1=byte, 2=C string, 3=word, 4=dword (we always use dword)
    DWORD Count;    // Number of units (of Type specified by "Type") in Value
    DWORD Value;
} TTiffDirEntry;

// Field data types.
enum eTiffDataType
{
    Byte = 1,
    String = 2,
    Short = 3,
    Long = 4
};

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


#define LIMIT(x) (((x)<0)?0:((x)>255)?255:(x))


CTiffSource::CTiffSource(LPCSTR FilePath) :
    CStillSource(FilePath)
{
}

BOOL CTiffSource::ReadNextFrameInFile()
{
    FILE* file;
    TTiffHeader head;
    TTiffDirEntry entry;
    BYTE buf[255];
    int i, j;
    int y1, y2, cr, cb, r, g, b;
    BYTE*   pBuf;

    if (m_AlreadyTryToRead)
    {
        return (m_StillFrame.pData != NULL);
    }

    m_AlreadyTryToRead = TRUE;

    memset(&head, 0, sizeof(head));

    LOG(2, "Graphic file %s", m_FilePath);
    file = fopen(m_FilePath,"rb");
    if (!file)
    {
        return FALSE;
    }
    LOG(2, "Graphic file opened");

    //
    // File Header
    //
    if (fread(head.byteOrder, sizeof(head.byteOrder), 1, file) != 1)
    {
        fclose(file);
        return FALSE;
    }
    if (fread(&(head.version), sizeof(head.version), 1, file) != 1)
    {
        fclose(file);
        return FALSE;
    }
    if (fread(&(head.firstDirOffset), sizeof(head.firstDirOffset), 1, file) != 1)
    {
        fclose(file);
        return FALSE;
    }
    i = fread(buf, head.firstDirOffset - sizeof(head.byteOrder) - sizeof(head.version) - sizeof(head.firstDirOffset), 1, file);
    if ((i != 0) && (i != 1))
    {
        fclose(file);
        return FALSE;
    }
    if (fread(&(head.numDirEntries), sizeof(head.numDirEntries), 1, file) != 1)
    {
        fclose(file);
        return FALSE;
    }
    for (i=0 ; i<head.numDirEntries ; i++)
    {
        if (fread(&entry, sizeof(entry), 1, file) != 1)
        {
            fclose(file);
            return FALSE;
        }
        switch (entry.Tag)
        {
        case 254:
            memcpy(&(head.fileType), &entry, sizeof(entry));
            break;
        case 256:
            memcpy(&(head.width), &entry, sizeof(entry));
            break;
        case 257:
            memcpy(&(head.height), &entry, sizeof(entry));
            break;
        case 258:
            memcpy(&(head.description), &entry, sizeof(entry));
            break;
        case 259:
            memcpy(&(head.compression), &entry, sizeof(entry));
            break;
        case 262:
            memcpy(&(head.photometricInterpretation), &entry, sizeof(entry));
            break;
        case 270:
            memcpy(&(head.description), &entry, sizeof(entry));
            break;
        case 271:
            memcpy(&(head.make), &entry, sizeof(entry));
            break;
        case 272:
            memcpy(&(head.model), &entry, sizeof(entry));
            break;
        case 273:
            memcpy(&(head.stripOffset), &entry, sizeof(entry));
            break;
        case 277:
            memcpy(&(head.samplesPerPixel), &entry, sizeof(entry));
            break;
        case 278:
            memcpy(&(head.rowsPerStrip), &entry, sizeof(entry));
            break;
        case 279:
            memcpy(&(head.stripByteCounts), &entry, sizeof(entry));
            break;
        case 284:
            memcpy(&(head.planarConfiguration), &entry, sizeof(entry));
            break;
        default:
            break;
        }
    }
    if (fread(&(head.nextDirOffset), sizeof(head.nextDirOffset), 1, file) != 1)
    {
        fclose(file);
        return FALSE;
    }
    if (fread(head.descriptionText, sizeof(head.descriptionText), 1, file) != 1)
    {
        fclose(file);
        return FALSE;
    }
    if (fread(head.makeText, sizeof(head.makeText), 1, file) != 1)
    {
        fclose(file);
        return FALSE;
    }
    if (fread(head.modelText, sizeof(head.modelText), 1, file) != 1)
    {
        fclose(file);
        return FALSE;
    }
    if (fread(&(head.bitCounts[0]), sizeof(head.bitCounts[0]), 1, file) != 1)
    {
        fclose(file);
        return FALSE;
    }
    if (fread(&(head.bitCounts[1]), sizeof(head.bitCounts[1]), 1, file) != 1)
    {
        fclose(file);
        return FALSE;
    }
    if (fread(&(head.bitCounts[2]), sizeof(head.bitCounts[2]), 1, file) != 1)
    {
        fclose(file);
        return FALSE;
    }
    LOG(2, "Graphic file header read");

    if ((head.fileType.Value != 0)
     || (head.compression.Value != 1)
     || (head.photometricInterpretation.Value != 2)
     || (head.samplesPerPixel.Value != 3)
     || (head.planarConfiguration.Value != 1)
     || (head.bitCounts[0] != 8)
     || (head.bitCounts[1] != 8)
     || (head.bitCounts[2] != 8)
     || (head.height.Value > DSCALER_MAX_HEIGHT)
     || (head.width.Value > DSCALER_MAX_WIDTH))
    {
        fclose(file);
        return FALSE;
    }
    LOG(2, "Graphic file header verified");

    m_Height = head.height.Value;
    m_Width = head.width.Value;
    LOG(2, "Graphic file height = %d width = %d", m_Height, m_Width);

    m_StillFrame.pData = (BYTE*)malloc(m_Width * 2 * m_Height * sizeof(BYTE));
    if (m_StillFrame.pData == NULL)
    {
        fclose(file);
        return FALSE;
    }
    LOG(2, "Frame buffer allocated");

    //
    // RGB Pixels
    //
    pBuf = m_StillFrame.pData;
    for (i=0 ; i<m_Height ; i++)
    {
        for (j=0 ; j<(m_Width/2) ; j++)
        {
            if (fread(buf, 3, 1, file) != 1)
            {
                fclose(file);
                free(m_StillFrame.pData);
                m_StillFrame.pData = NULL;
                return FALSE;
            }

            r = buf[0];
            g = buf[1];
            b = buf[2];

            y1 = ( 16840*r + 33058*g +  6405*b + 1048576)>>16;
            cb = ( -9713*r - 19068*g + 28781*b + 8388608)>>16;

            if (fread(buf, 3, 1, file) != 1)
            {
                fclose(file);
                free(m_StillFrame.pData);
                m_StillFrame.pData = NULL;
                return FALSE;
            }

            r = buf[0];
            g = buf[1];
            b = buf[2];

            y2 = ( 16840*r + 33058*g +  6405*b + 1048576)>>16;
            cr = ( 28781*r - 24110*g -  4671*b + 8388608)>>16;

            *pBuf = LIMIT(y1);
            ++pBuf;
            *pBuf = LIMIT(cb);
            ++pBuf;
            *pBuf = LIMIT(y2);
            ++pBuf;
            *pBuf = LIMIT(cr);
            ++pBuf;
        }
    }

    fclose(file);
    LOG(1, "Graphic file loaded");
    return TRUE;
}
/*
    BYTE*   pBuf;
    BYTE    Y;
    BYTE    U;
    BYTE    V;

    m_Width = 720;
    m_Height = 480;
    m_StillFrame.pData = (BYTE*)malloc(m_Width * 2 * m_Height * sizeof(BYTE));

    pBuf = m_StillFrame.pData;
    for (int i(0); i < m_Height; ++i)
    {
        U = (i % 256);
        V = 255 - (i % 256);
//        U = V = 128;
        for (int j(0); j < m_Width; ++j)
        {
            Y = (j % 256);
            *pBuf = Y;
            ++pBuf;
            if ((j % 2) == 0)
            {
                *pBuf = U;
            }
            else
            {
                *pBuf = V;
            }
            ++pBuf;
        }
    }
*/
