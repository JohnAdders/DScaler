/////////////////////////////////////////////////////////////////////////////
// $Id: TiffHelper.cpp,v 1.2 2001-11-30 10:46:43 adcockj Exp $
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
// Revision 1.1  2001/11/28 16:04:50  adcockj
// Major reorganization of STill support
//
// Revision 1.3  2001/11/25 10:41:26  laurentg
// TIFF code moved from Other.cpp to TiffSource.c + still capture updated
//
// Revision 1.2  2001/11/24 22:51:20  laurentg
// Bug fixes regarding still source
//
// Revision 1.1  2001/11/24 17:55:23  laurentg
// CTiffHelper class added
//
//
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include "stdafx.h"
#include "TiffHelper.h"
#include "Other.h"
#include "Deinterlace.h"
#include "DebugLog.h"


#define LIMIT(x) (((x)<0)?0:((x)>255)?255:(x))

#define STRUCT_OFFSET(s,f)  ((int)(((BYTE*)&(s)->f) - (BYTE*)(s)))

CTiffHelper::CTiffHelper(CStillSource* pParent) :
    CStillSourceHelper(pParent)
{
}

BOOL CTiffHelper::OpenMediaFile(LPCSTR FileName)
{
    FILE* file;
    TTiffHeader head;
    TTiffDirEntry entry;
    BYTE buf[255];
    int i, j;
    int y1, y2, cr, cb, r, g, b;
    BYTE*   pBuf;

    m_pParent->m_IsPictureRead = FALSE;

    memset(&head, 0, sizeof(head));

    LOG(2, "Graphic file %s", FileName);
    file = fopen(FileName,"rb");
    if (!file)
    {
        return FALSE;
    }
    LOG(2, "Graphic file opened");

    if(m_pParent->m_OriginalFrame.pData != NULL)
    {
        free(m_pParent->m_OriginalFrame.pData);
        m_pParent->m_OriginalFrame.pData = NULL;
    }


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

    m_pParent->m_Height = head.height.Value;
    m_pParent->m_Width = head.width.Value;
    LOG(2, "Graphic file height = %d width = %d", m_pParent->m_Height, m_pParent->m_Width);

    m_pParent->m_OriginalFrame.pData = (BYTE*)malloc(m_pParent->m_Width * 2 * m_pParent->m_Height * sizeof(BYTE));
    if (m_pParent->m_OriginalFrame.pData == NULL)
    {
        free(m_pParent->m_OriginalFrame.pData);
        m_pParent->m_OriginalFrame.pData = NULL;
        fclose(file);
        return FALSE;
    }
    LOG(2, "Frame buffer allocated");

    //
    // RGB Pixels
    //
    pBuf = m_pParent->m_OriginalFrame.pData;
    for (i=0 ; i < m_pParent->m_Height ; i++)
    {
        for (j=0 ; j < (m_pParent->m_Width/2) ; j++)
        {
            if (fread(buf, 3, 1, file) != 1)
            {
                fclose(file);
                free(m_pParent->m_OriginalFrame.pData);
                m_pParent->m_OriginalFrame.pData = NULL;
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
                free(m_pParent->m_OriginalFrame.pData);
                m_pParent->m_OriginalFrame.pData = NULL;
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
    m_pParent->m_IsPictureRead = TRUE;
    LOG(1, "Graphic file loaded");
    return TRUE;
}


//-----------------------------------------------------------------------------
// Fill a TIFF directory entry with information.
void CTiffHelper::FillTiffDirEntry(TTiffDirEntry* entry, WORD Tag, DWORD Value, eTiffDataType Type)
{
    BYTE bValue;
    WORD wValue;

    entry->Tag = Tag;
    entry->Count = 1;
    entry->Type = (int) Type;

    switch (Type) {
    case Byte:
        bValue = (BYTE) Value;
        memcpy(&entry->Value, &bValue, 1);
        break;

    case Short:
        wValue = (WORD) Value;
        memcpy(&entry->Value, &wValue, 2);
        break;

    case String:    // in which case it's a file offset
    case Long:
        entry->Value = Value;
        break;
    }
}


//-----------------------------------------------------------------------------
// Fill a TIFF header with information about the current image.
void CTiffHelper::FillTiffHeader(TTiffHeader* head, char* description, char* make, char* model, int Height, int Width)
{
    memset(head, 0, sizeof(TTiffHeader));

    strcpy(head->byteOrder, "II");      // Intel byte order
    head->version = 42;                 // We're TIFF 5.0 compliant, but the version field is unused
    head->firstDirOffset = STRUCT_OFFSET(head, numDirEntries);
    head->numDirEntries = 14;
    head->nextDirOffset = 0;            // No additional directories

    strcpy(head->descriptionText, description);
    strcpy(head->makeText, make);
    strcpy(head->modelText, model);
    head->bitCounts[0] = head->bitCounts[1] = head->bitCounts[2] = 8;

    head->description.Tag = 270;
    head->description.Type = 2;
    head->description.Count = strlen(description) + 1;
    head->description.Value = STRUCT_OFFSET(head, descriptionText);

    head->make.Tag = 271;
    head->make.Type = 2;
    head->make.Count = strlen(make) + 1;
    head->make.Value = STRUCT_OFFSET(head, makeText);

    head->model.Tag = 272;
    head->model.Type = 2;
    head->model.Count = strlen(model) + 1;
    head->model.Value = STRUCT_OFFSET(head, modelText);
    
    head->bitsPerSample.Tag = 258;
    head->bitsPerSample.Type = Short;
    head->bitsPerSample.Count = 3;
    head->bitsPerSample.Value = STRUCT_OFFSET(head, bitCounts);

    FillTiffDirEntry(&head->fileType, 254, 0, Long);                        // Just the image, no thumbnails
    FillTiffDirEntry(&head->width, 256, Width, Short);
    FillTiffDirEntry(&head->height, 257, Height, Short);
    FillTiffDirEntry(&head->compression, 259, 1, Short);                    // No compression
    FillTiffDirEntry(&head->photometricInterpretation, 262, 2, Short);      // RGB image data
    FillTiffDirEntry(&head->stripOffset, 273, sizeof(TTiffHeader), Long);    // Image comes after header
    FillTiffDirEntry(&head->samplesPerPixel, 277, 3, Short);                // RGB = 3 channels/pixel
    FillTiffDirEntry(&head->rowsPerStrip, 278, Height, Short);            // Whole image is one strip
    FillTiffDirEntry(&head->stripByteCounts, 279, Width * Height * 3, Long);   // Size of image data
    FillTiffDirEntry(&head->planarConfiguration, 284, 1, Short);            // RGB bytes are interleaved
}

void CTiffHelper::SaveSnapshot(LPCSTR FilePath, int Height, int Width, BYTE* pOverlay, LONG OverlayPitch)
{
    int y, cr, cb, r, g, b, i, j, n = 0;
    FILE* file;
    BYTE rgb[3];
    BYTE* buf;
    TTiffHeader head;
    char description[80];

    file = fopen(FilePath,"wb");
    if (!file)
    {
        return;
    }

    LOG(2, "WriteFrameInFile Width = %d Height %d", Width, Height);

    sprintf(description, "DScaler image, deinterlace Mode %s", GetDeinterlaceModeName());
    // How do we figure out our version number?!?!
    FillTiffHeader(&head, description, "http://deinterlace.sourceforge.net/", "DScaler version 2.x", Height, Width);
    fwrite(&head, sizeof(head), 1, file);

    for (i = 0; i < Height; i++)
    {
        buf = pOverlay + i * OverlayPitch;
        for (j = 0; j < Width ; j+=2)
        {
            cb = buf[1] - 128;
            cr = buf[3] - 128;
            y = buf[0] - 16;

            r = ( 76284*y + 104595*cr             )>>16;
            g = ( 76284*y -  53281*cr -  25624*cb )>>16;
            b = ( 76284*y             + 132252*cb )>>16;
            rgb[0] = LIMIT(r);
            rgb[1] = LIMIT(g);
            rgb[2] = LIMIT(b);

            fwrite(rgb,3,1,file) ;

            y = buf[2] - 16;
            r = ( 76284*y + 104595*cr             )>>16;
            g = ( 76284*y -  53281*cr -  25624*cb )>>16;
            b = ( 76284*y             + 132252*cb )>>16;
            rgb[0] = LIMIT(r);
            rgb[1] = LIMIT(g);
            rgb[2] = LIMIT(b);
            fwrite(rgb,3,1,file);

            buf += 4;
        }
    }
    fclose(file);
    return;
}
