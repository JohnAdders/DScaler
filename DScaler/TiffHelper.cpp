/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file TiffHelper.cpp Tiff Helper Functions
 */

#include <stdio.h>
#include <time.h>
#include "stdafx.h"
#include "TiffHelper.h"
#include "DebugLog.h"
#include "Dialogs.h"
#include "OutThreads.h"
#include "..\ThirdParty\LibTiff\tiffio.h"

using namespace std;

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

BOOL CTiffHelper::OpenMediaFile(const tstring& FileName)
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
    tif = TIFFOpen(TStringToMBCS(FileName).c_str(), "r");
    if (!tif) {
        return FALSE;
    }

    if (!TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w) ||
        !TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h) ||
        !TIFFGetField(tif, TIFFTAG_COMPRESSION, &Compression) ||
        !TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &Class) )
    {
        LOG(1, _T("At least one of the tags IMAGEWIDTH, IMAGELENGTH, COMPRESSION or PHOTOMETRIC is missing in the file %s"), FileName);
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
            LOG(3, _T("File %s supported (class %d compression %d)"), FileName, Class, Compression);
            break;
        }
    }
    if (!Found)
    {
        LOG(1, _T("File %s not supported (class %d compression %d)"), FileName, Class, Compression);
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
    pFrameBuf = (BYTE*)malloc(LinePitch * h + 16);
    if (pFrameBuf == NULL)
    {
        TIFFClose(tif);
        return FALSE;
    }
    pStartFrame = START_ALIGNED16(pFrameBuf);

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
        m_pParent->m_Comments = _T("");
    }
    else if (!TIFFGetField(tif, TIFFTAG_IMAGEDESCRIPTION, &Description))
    {
        m_pParent->m_SquarePixels = FALSE;
        m_pParent->m_Comments = _T("");
    }
    else if (!(square_mark = strstr(Description, SQUARE_MARK)))
    {
        m_pParent->m_SquarePixels = FALSE;
        m_pParent->m_Comments = MBCSToTString(Description);
    }
    else
    {
        m_pParent->m_SquarePixels = TRUE;
        m_pParent->m_Comments.assign(Description, square_mark);
    }

    // Close the file
    TIFFClose(tif);

    m_pParent->FreeOriginalFrameBuffer();
    m_pParent->m_OriginalFrameBuffer = pFrameBuf;
    m_pParent->m_OriginalFrame.pData = pStartFrame;
    m_pParent->m_LinePitch = LinePitch;
    m_pParent->m_InitialHeight = h;
    m_pParent->m_InitialWidth = w2;
    m_pParent->m_Height = h;
    m_pParent->m_Width = w2;

    return TRUE;
}


void CTiffHelper::SaveSnapshot(const tstring& FilePath, int Height, int Width, BYTE* pOverlay, LONG OverlayPitch, const tstring& Context)
{
    int y, cr, cb, r, g, b;
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
    tif = TIFFOpen(TStringToMBCS(FilePath).c_str(), "w");
    if (!tif)
    {
        _TIFFfree(buffer);
        return;
    }

    //
    // Fields of the directory
    //
    time(&long_time);
    tm_time = localtime(&long_time);

    if (!TIFFSetField(tif, TIFFTAG_SUBFILETYPE, 0) ||
        !TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8) ||                 // 8 bits for each channel
        !TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, TStringToMBCS(Context).c_str()) ||
        !TIFFSetField(tif, TIFFTAG_IMAGELENGTH, Height) ||
        !TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG) ||         // RGB bytes are interleaved
        !TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, Height) ||             // Whole image is one strip
        !TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3) ||               // RGB = 3 channels/pixel
        !TIFFSetField(tif, TIFFTAG_SOFTWARE, TStringToMBCS(GetProductNameAndVersion()).c_str()) ||
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
