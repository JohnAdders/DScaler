/////////////////////////////////////////////////////////////////////////////
// $Id: TiffHelper.cpp,v 1.25 2002-06-21 23:14:19 laurentg Exp $
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
// Revision 1.24  2002/05/27 22:24:27  laurentg
// When taking a still using a TIFF (RGB colorspace), overlay values are first normalized before the conversion in RGB
//
// Revision 1.23  2002/05/10 20:34:38  laurentg
// Formula for conversion RGB <=> YCbCr updated
//
// Revision 1.22  2002/05/06 15:48:53  laurentg
// Informations saved in a DScaler still updated
// Use of the comments field to show informations about a DScaler still
//
// Revision 1.21  2002/05/05 12:09:22  laurentg
// All lines have now a pitch which is a multiple of 16
// Width of picture is now forced to an even value
//
// Revision 1.20  2002/05/03 20:36:49  laurentg
// 16 byte aligned data
//
// Revision 1.19  2002/04/27 14:08:07  laurentg
// Automatic square pixels mode handling updated
//
// Revision 1.18  2002/04/15 22:50:09  laurentg
// Change again the available formats for still saving
// Automatic switch to "square pixels" AR mode when needed
//
// Revision 1.17  2002/04/14 17:25:26  laurentg
// New formats of TIFF files supported to take stills : Class R (RGB) with compression LZW or Packbits or JPEG
//
// Revision 1.16  2002/04/14 10:16:23  laurentg
// Table of TIFF compatibility updated due to update to LibTiff 3.5.7 + LibJpeg
//
// Revision 1.15  2002/04/14 00:46:49  laurentg
// Table of compatibility TIFF updated
// Log messages suppressed
//
// Revision 1.14  2002/04/13 23:51:30  laurentg
// Table of compatibility (class, compression) added
//
// Revision 1.13  2002/04/13 18:47:53  laurentg
// Management of still files improved
//
// Revision 1.12  2002/04/10 22:18:12  laurentg
// Checks to avoid loading of certain TIFF files with unsupported compression modes
//
// Revision 1.11  2002/02/26 21:24:25  laurentg
// Move the test on the still file size in order to have a global treatment later
//
// Revision 1.10  2002/02/23 00:30:47  laurentg
// NotifySizeChange
//
// Revision 1.9  2002/02/22 09:07:13  tobbej
// fixed small race condition when calling notifysizechange, workoutoverlaysize might have used the old size
//
// Revision 1.8  2002/02/19 16:03:36  tobbej
// removed CurrentX and CurrentY
// added new member in CSource, NotifySizeChange
//
// Revision 1.7  2002/02/14 23:16:59  laurentg
// Stop / start capture never needed when switching between files of the playlist
// CurrentX / CurrentY not updated in StillSource but in the main decoding loop
//
// Revision 1.6  2002/02/02 21:19:05  laurentg
// Read/write of TIFF files updated
//
// Revision 1.5  2002/02/02 12:41:44  laurentg
// CurrentX and CurrentY set when changing source and when switching between still files
//
// Revision 1.4  2001/12/08 13:43:20  adcockj
// Fixed logging and memory leak bugs
//
// Revision 1.3  2001/12/05 00:08:41  laurentg
// Use of LibTiff DLL
//
// Revision 1.2  2001/11/30 10:46:43  adcockj
// Fixed crashes and leaks
//
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
#include <time.h>
#include "stdafx.h"
#include "TiffHelper.h"
#include "DebugLog.h"
#include "Dialogs.h"
#include "OutThreads.h"
#include "..\ThirdParty\LibTiff\tiffio.h"


#define SQUARE_MARK "(square)"

#define LIMIT_RGB(x)    (((x)<0)?0:((x)>255)?255:(x))
#define LIMIT_Y(x)      (((x)<16)?16:((x)>235)?235:(x))
#define LIMIT_CbCr(x)   (((x)<16)?16:((x)>240)?240:(x))

//#define NO_CHECK_TIFF_COMPRESS


#ifndef NO_CHECK_TIFF_COMPRESS
static struct {
    uint16 tag_class;
    uint16 tag_compression;
} tTiffTagClassCompress[] = {
    { PHOTOMETRIC_MINISWHITE , COMPRESSION_NONE      },
    { PHOTOMETRIC_MINISWHITE , COMPRESSION_CCITTRLE  },
    { PHOTOMETRIC_MINISWHITE , COMPRESSION_CCITTFAX3 },
    { PHOTOMETRIC_MINISWHITE , COMPRESSION_CCITTFAX4 },
    { PHOTOMETRIC_MINISWHITE , COMPRESSION_PACKBITS  },
    { PHOTOMETRIC_MINISWHITE , COMPRESSION_LZW       },
    { PHOTOMETRIC_MINISWHITE , COMPRESSION_JPEG      },
    { PHOTOMETRIC_MINISWHITE , COMPRESSION_DEFLATE   },
    { PHOTOMETRIC_MINISBLACK , COMPRESSION_NONE      },
    { PHOTOMETRIC_MINISBLACK , COMPRESSION_CCITTRLE  },
    { PHOTOMETRIC_MINISBLACK , COMPRESSION_CCITTFAX3 },
    { PHOTOMETRIC_MINISBLACK , COMPRESSION_CCITTFAX4 },
    { PHOTOMETRIC_MINISBLACK , COMPRESSION_PACKBITS  },
    { PHOTOMETRIC_MINISBLACK , COMPRESSION_LZW       },
    { PHOTOMETRIC_MINISBLACK , COMPRESSION_JPEG      },
    { PHOTOMETRIC_MINISBLACK , COMPRESSION_DEFLATE   },
    { PHOTOMETRIC_PALETTE    , COMPRESSION_NONE      },
    { PHOTOMETRIC_PALETTE    , COMPRESSION_LZW       },
    { PHOTOMETRIC_RGB        , COMPRESSION_NONE      },
    { PHOTOMETRIC_RGB        , COMPRESSION_LZW       },
    { PHOTOMETRIC_RGB        , COMPRESSION_PACKBITS  },
    { PHOTOMETRIC_RGB        , COMPRESSION_JPEG      },
    { PHOTOMETRIC_RGB        , COMPRESSION_DEFLATE   },
    { PHOTOMETRIC_SEPARATED  , COMPRESSION_NONE      },
    { PHOTOMETRIC_SEPARATED  , COMPRESSION_LZW       },
    { PHOTOMETRIC_SEPARATED  , COMPRESSION_PACKBITS  },
    { PHOTOMETRIC_SEPARATED  , COMPRESSION_JPEG      },
    { PHOTOMETRIC_YCBCR      , COMPRESSION_NONE      },
    { PHOTOMETRIC_YCBCR      , COMPRESSION_LZW       },
    { PHOTOMETRIC_YCBCR      , COMPRESSION_PACKBITS  },
    { PHOTOMETRIC_YCBCR      , COMPRESSION_JPEG      },
};
#endif

CTiffHelper::CTiffHelper(CStillSource* pParent, eTIFFClass FormatSaving) :
    CStillSourceHelper(pParent)
{
    m_FormatSaving = FormatSaving;
}

BOOL CTiffHelper::OpenMediaFile(LPCSTR FileName)
{
    int y1, y2, cr, cb, r, g, b, i, j;
    BYTE* pFrameBuf;
    BYTE* pStartFrame;
    BYTE* pDestBuf;
    TIFF* tif;
    uint32 w, h, w2;
    uint16 Class;
    size_t npixels;
    uint32* bufPackedRGB;
    int PackedABGRValue;
    uint8* bufYCbCr;
    uint8* pSrcBuf;;
    tstrip_t Strip;
    uint32* bc;
    uint32 StripSize;
    uint16 Compression;
    char* Software;
    char* Description;
    char* square_mark;
    int LinePitch;

    // Open the file
    tif = TIFFOpen(FileName, "r");
    if (!tif) {
        return FALSE;
    }

    if (!TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w) ||
        !TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h) ||
        !TIFFGetField(tif, TIFFTAG_COMPRESSION, &Compression) ||
        !TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &Class) )
    {
        LOG(1, "At least one of the tags IMAGEWIDTH, IMAGELENGTH, COMPRESSION or PHOTOMETRIC is missing in the file %s", FileName);
        TIFFClose(tif);
        return FALSE;
    }

#ifndef NO_CHECK_TIFF_COMPRESS
    BOOL Found;
    for (Found = FALSE, i = 0 ; i < (sizeof(tTiffTagClassCompress) / sizeof(tTiffTagClassCompress[0])) ; i++)
    {
        if ( (tTiffTagClassCompress[i].tag_class == Class)
          && (tTiffTagClassCompress[i].tag_compression == Compression) )
        {
            Found = TRUE;
            LOG(3, "File %s supported (class %d compression %d)", FileName, Class, Compression);
            break;
        }
    }
    if (!Found)
    {
        LOG(1, "File %s not supported (class %d compression %d)", FileName, Class, Compression);
        TIFFClose(tif);
        return FALSE;
    }
#endif

    // The width must be even
    if (w%2)
    {
        w2 = w - 1;
    }
    else
    {
        w2 = w;
    }

    // Allocate memory buffer to store the YUYV values
    LinePitch = (w2 * 2 * sizeof(BYTE) + 15) & 0xfffffff0;
    pFrameBuf = MallocStillBuf(LinePitch * h, &pStartFrame);
    if (pFrameBuf == NULL)
    {
        TIFFClose(tif);
        return FALSE;
    }

    if ((Class == PHOTOMETRIC_YCBCR) && (Compression == COMPRESSION_NONE))
    {
        if (!TIFFGetField(tif, TIFFTAG_STRIPBYTECOUNTS, &bc))
        {
            free(pFrameBuf);
            TIFFClose(tif);
            return FALSE;
        }

        npixels = w * h;
        bufYCbCr = (uint8*) _TIFFmalloc(npixels * 2 * sizeof (uint8));
        if (bufYCbCr == NULL)
        {
            free(pFrameBuf);
            TIFFClose(tif);
            return FALSE;
        }

        pSrcBuf = bufYCbCr;
        for (Strip = 0 ; Strip < TIFFNumberOfStrips(tif) ; Strip++)
        {
            StripSize = bc[Strip];
            if (TIFFReadRawStrip(tif, Strip, pSrcBuf, StripSize) != StripSize)
            {
                free(pFrameBuf);
                _TIFFfree(bufYCbCr);
                TIFFClose(tif);
                return FALSE;
            }
            pSrcBuf += StripSize;
        }

        // YYUV => YUYV
        for (i = 0 ; i < h ; i++)
        {
            pDestBuf = pStartFrame + i * LinePitch;
            pSrcBuf = bufYCbCr + i * w * 2;
            for (j = 0 ; j < (w2/2) ; j++)
            {
                *pDestBuf = pSrcBuf[j * 4];
                ++pDestBuf;
                *pDestBuf = pSrcBuf[j * 4 + 2];
                ++pDestBuf;
                *pDestBuf = pSrcBuf[j * 4 + 1];
                ++pDestBuf;
                *pDestBuf = pSrcBuf[j * 4 + 3];
                ++pDestBuf;
            }
        }

        _TIFFfree(bufYCbCr);
    }
    else
    {
        npixels = w * h;
        bufPackedRGB = (uint32*) _TIFFmalloc(npixels * sizeof (uint32));
        if (bufPackedRGB == NULL)
        {
            free(pFrameBuf);
            TIFFClose(tif);
            return FALSE;
        }

        // RGBA buffer filled in with data from file
        if (!TIFFReadRGBAImage(tif, w, h, bufPackedRGB, 0))
        {
            free(pFrameBuf);
            _TIFFfree(bufPackedRGB);
            TIFFClose(tif);
            return FALSE;
        }

        // RGBRGB => YUYV normalized
        for (i = (h - 1) ; i >= 0 ; i--)
        {
            pDestBuf = pStartFrame + (h - 1 - i) * LinePitch;
            for (j = 0 ; j < (w2/2) ; j++)
            {
                PackedABGRValue = bufPackedRGB[i * w + j * 2];
                r = TIFFGetR(PackedABGRValue);
                g = TIFFGetG(PackedABGRValue);
                b = TIFFGetB(PackedABGRValue);

                y1 = ( ( 16829*r + 33039*g +  6416*b + 32768 ) >> 16 ) + 16;
                cb = ( ( -9714*r - 19071*g + 28784*b + 32768 ) >> 16 ) + 128;

                PackedABGRValue = bufPackedRGB[i * w + j * 2 + 1];
                r = TIFFGetR(PackedABGRValue);
                g = TIFFGetG(PackedABGRValue);
                b = TIFFGetB(PackedABGRValue);

                y2 = ( ( 16829*r + 33039*g +  6416*b + 32768 ) >> 16 ) + 16;
                cr = ( ( 28784*r - 24103*g -  4681*b + 32768 ) >> 16 ) + 128;

                *pDestBuf = LIMIT_Y(y1);
                ++pDestBuf;
                *pDestBuf = LIMIT_CbCr(cb);
                ++pDestBuf;
                *pDestBuf = LIMIT_Y(y2);
                ++pDestBuf;
                *pDestBuf = LIMIT_CbCr(cr);
                ++pDestBuf;
            }
        }

        _TIFFfree(bufPackedRGB);
    }

    if ( !TIFFGetField(tif, TIFFTAG_SOFTWARE, &Software)
      || !strstr(Software, "DScaler") )
    {
        m_pParent->m_SquarePixels = TRUE;
        m_pParent->m_Comments = "";
    }
    else if (!TIFFGetField(tif, TIFFTAG_IMAGEDESCRIPTION, &Description))
    {
        m_pParent->m_SquarePixels = FALSE;
        m_pParent->m_Comments = "";
    }
    else if (!(square_mark = strstr(Description, SQUARE_MARK)))
    {
        m_pParent->m_SquarePixels = FALSE;
        m_pParent->m_Comments = Description;
    }
    else
    {
        m_pParent->m_SquarePixels = TRUE;
        m_pParent->m_Comments.assign(Description, square_mark-Description);
    }

    // Close the file
    TIFFClose(tif);

    if (m_pParent->m_OriginalFrameBuffer != NULL)
    {
        free(m_pParent->m_OriginalFrameBuffer);
    }
    m_pParent->m_OriginalFrameBuffer = pFrameBuf;
    m_pParent->m_OriginalFrame.pData = pStartFrame;
    m_pParent->m_LinePitch = LinePitch;
    m_pParent->m_Height = h;
    m_pParent->m_Width = w2;

    return TRUE;
}


void CTiffHelper::SaveSnapshot(LPCSTR FilePath, int Height, int Width, BYTE* pOverlay, LONG OverlayPitch)
{
    int y, cr, cb, r, g, b;
    char* description;
    TIFF* tif;
    BYTE *pBufOverlay;
    uint8* buffer;
    uint8* pBuf;
    tsize_t size;
    int w;
    float ycbcrCoeffs[3] = { 0.299f, 0.587f, 0.114f };
    float refBlackWhite[6] = { 15., 235., 128., 240., 128., 240. };
    uint16 Compression;
    struct tm *tm_time;
    time_t long_time;

    w = (Width % 2) ? Width-1 : Width;
    if (m_FormatSaving == TIFF_CLASS_Y)
    {
        size = Height * w * 2;
    }
    else
    {
        size = Height * Width * 3;
    }
    buffer = (uint8*) _TIFFmalloc(size * sizeof (uint8));
    if (buffer == NULL)
    {
        return;
    }

    pBuf = buffer;
    for (int i = 0; i < Height; i++)
    {
        pBufOverlay = pOverlay + i * OverlayPitch;
        for (int j = 0; j < w ; j+=2)
        {
            if (m_FormatSaving == TIFF_CLASS_Y)
            {
                *pBuf++ = pBufOverlay[0];
                *pBuf++ = pBufOverlay[2];
                *pBuf++ = pBufOverlay[1];
                *pBuf++ = pBufOverlay[3];
            }
            else
            {
                // YUYV is first normalized : Y in range [16,235] and Cb/Cr in range [16,240]
                // and then converted in RGB
                y = LIMIT_Y(pBufOverlay[0]) - 16;
                cb = LIMIT_CbCr(pBufOverlay[1]) - 128;
                cr = LIMIT_CbCr(pBufOverlay[3]) - 128;
                r = ( 76309*y             + 104597*cr + 32768 ) >> 16;
                g = ( 76309*y -  25675*cb -  53279*cr + 32768 ) >> 16;
                b = ( 76309*y + 132201*cb             + 32768 ) >> 16;
                *pBuf++ = LIMIT_RGB(r);
                *pBuf++ = LIMIT_RGB(g);
                *pBuf++ = LIMIT_RGB(b);

                y = LIMIT_Y(pBufOverlay[2]) - 16;
                r = ( 76309*y             + 104597*cr + 32768 ) >> 16;
                g = ( 76309*y -  25675*cb -  53279*cr + 32768 ) >> 16;
                b = ( 76309*y + 132201*cb             + 32768 ) >> 16;
                *pBuf++ = LIMIT_RGB(r);
                *pBuf++ = LIMIT_RGB(g);
                *pBuf++ = LIMIT_RGB(b);
            }

            pBufOverlay += 4;
        }
        if ((w != Width) && (m_FormatSaving != TIFF_CLASS_Y))
        {
            y = LIMIT_Y(pBufOverlay[0]) - 16;
            cb = LIMIT_CbCr(pBufOverlay[1]) - 128;
            cr = LIMIT_CbCr(pBufOverlay[-1]) - 128;
            r = ( 76309*y             + 104597*cr + 32768 ) >> 16;
            g = ( 76309*y -  25675*cb -  53279*cr + 32768 ) >> 16;
            b = ( 76309*y + 132201*cb             + 32768 ) >> 16;
            *pBuf++ = LIMIT_RGB(r);
            *pBuf++ = LIMIT_RGB(g);
            *pBuf++ = LIMIT_RGB(b);
        }
    }

    // Open the file
    tif = TIFFOpen(FilePath, "w");
    if (!tif)
    {
        _TIFFfree(buffer);
        return;
    }

    //
    // Fields of the directory
    //
    description = BuildDScalerContext();
    if (m_pParent->m_SquarePixels)
    {
        strcat(description, SQUARE_MARK);
    }
    time(&long_time);
    tm_time = localtime(&long_time);

    if (!TIFFSetField(tif, TIFFTAG_SUBFILETYPE, 0) ||
        !TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8) ||                 // 8 bits for each channel
        !TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, description) ||
        !TIFFSetField(tif, TIFFTAG_IMAGELENGTH, Height) ||
        !TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG) ||         // RGB bytes are interleaved
        !TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, Height) ||             // Whole image is one strip
        !TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3) ||               // RGB = 3 channels/pixel
        !TIFFSetField(tif, TIFFTAG_SOFTWARE, GetProductNameAndVersion()) ||
        !TIFFSetField(tif, TIFFTAG_DATETIME, asctime(tm_time)))
    {
        _TIFFfree(buffer);
        TIFFClose(tif);
        return;
    }
    if (m_FormatSaving == TIFF_CLASS_Y)
    {
        if (!TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, w) ||
            !TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_YCBCR) ||
            !TIFFSetField(tif, TIFFTAG_YCBCRCOEFFICIENTS, ycbcrCoeffs) ||
            !TIFFSetField(tif, TIFFTAG_YCBCRPOSITIONING, YCBCRPOSITION_COSITED) ||
            !TIFFSetField(tif, TIFFTAG_YCBCRSUBSAMPLING, 2, 1) ||
            !TIFFSetField(tif, TIFFTAG_REFERENCEBLACKWHITE, refBlackWhite))
        {
            _TIFFfree(buffer);
            TIFFClose(tif);
            return;
        }
    }
    else
    {
        if (!TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, Width) ||
            !TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB))
        {
            _TIFFfree(buffer);
            TIFFClose(tif);
            return;
        }
    }
    switch (m_FormatSaving)
    {
    case TIFF_CLASS_R_JPEG:
        Compression = COMPRESSION_JPEG;
        break;
    case TIFF_CLASS_R:
    case TIFF_CLASS_Y:
    default:
        Compression = COMPRESSION_NONE;
        break;
    }
    if (!TIFFSetField(tif, TIFFTAG_COMPRESSION, Compression))
    {
        _TIFFfree(buffer);
        TIFFClose(tif);
        return;
    }

    // Write the strip (data) in the file
    TIFFWriteEncodedStrip(tif, 0, buffer, size);

    _TIFFfree(buffer);

    // Close the file
    TIFFClose(tif);

    return;
}
