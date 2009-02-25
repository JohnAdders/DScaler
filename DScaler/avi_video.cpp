/* $Id$ */

/** \file
 * Video compression functions
 * \author Nick Kochakian
 */

#include "stdafx.h"
#include "avi.h"
#include "avi_internal.h"

#define FOURCC_DIB  mmioFOURCC('D', 'I', 'B', ' ')
#define FOURCC_YUY2 mmioFOURCC('Y', 'U', 'Y', '2')

/** Determines if a particular CC belongs to an uncompressed image format
 * \param fcc The CC to check
 * \retval TRUE  If the CC belongs to an uncompressed format
 * \retval FALSE If the CC does not belong to an uncompressed format
 */

__inline BOOL aviVideoFCCIsUncompressed(FOURCC fcc)
{
    return (fcc==0 || fcc==FOURCC_DIB) ? TRUE : FALSE;
}

/**
 * Sets the initial parameters that define the video stream. These are expected
 * to remain the same after they're set.
 * \note This function sets the default input format of 24-bit RGB
 */

void aviVideoDefineStream(AVI_FILE *file, DWORD rate, DWORD scale, DWORD w,
                          DWORD h)
{
    if (!file)
       return;

    memset(&file->video.info, 0, sizeof(BITMAPINFOHEADER));

    file->video.rate  = rate;
    file->video.scale = scale;

    file->video.info.biSize        = sizeof(BITMAPINFOHEADER);
    file->video.info.biWidth       = w;
    file->video.info.biHeight      = h;
    file->video.info.biPlanes      = 1;
    file->video.info.biCompression = BI_RGB;
    file->video.info.biBitCount    = 24;

    file->video.cc.data  = mmioFOURCC('0', '0', 'd', 'b');
    file->video.cc.index = mmioFOURCC('i', 'x', '0', '0');

    file->video.fccHandler = FOURCC_DIB;

    file->video.fps = (float)file->video.rate / (float)file->video.scale;

    file->flags |= AVI_FLAG_VIDEO;
}

/** Changes the video compressor and the input format of the video stream
 * \param file       The file to use
 * \param fccHandler The CC of the handler that will accept the input format
 *                   that you're planning to use
 * \param inFormat   The CC of the input format. Can be BI_RGB for plain RGB.
 * \param bitCount   The number of bits per pixel in the input image
 */

void aviVideoSetHandler(AVI_FILE *file, FOURCC fccHandler, FOURCC inFormat,
                        WORD bitCount)
{
    if (!file)
       return;

    file->video.fccHandler         = fccHandler;
    file->video.info.biCompression = inFormat;
    file->video.info.biBitCount    = bitCount;
}

/** Opens the video compressor and allocates memory for compression
 * \param file A file that has had its video stream defined
 * \retval TRUE  Success
 * \retval FALSE Failure
 */

BOOL aviVideoCompressorOpen(AVI_FILE *file)
{
    DWORD      size;
    VIDEO_COMP *comp;
    COMPVARS   *vars;

    #define VIDEO_ERROR(file, message)\
    {\
        aviVideoCompressorClose(file);\
        aviSetError(file, AVI_ERROR_VIDEO_OPEN, message);\
        return FALSE;\
    }

    if (!file)
       return FALSE;

    comp = &file->video.comp;

    if (aviVideoFCCIsUncompressed(file->video.fccHandler))
    {
        /* Special case for uncompressed output */

        /* Check the input format */
        if (file->video.info.biCompression != BI_RGB &&
            file->video.info.biCompression != FOURCC_YUY2)
           VIDEO_ERROR(file, "The input format isn't supported");

        /* Make the output format equal to the input format */
        comp->format = malloc(sizeof(BITMAPINFOHEADER));
        if (!comp->format)
           VIDEO_ERROR(file, "Out of memory");

        memcpy(comp->format, &file->video.info, sizeof(BITMAPINFOHEADER));

        /* Make sure biSizeImage is set correctly in format */
        ((BITMAPINFOHEADER *)comp->format)->biSizeImage =
                file->video.info.biWidth * file->video.info.biHeight *
                (file->video.info.biBitCount >> 3);
    } else
    {
        /* The fourcc code doesn't seem to indicate an uncompressed format */

        if (comp->hIC)
           return TRUE; /* Already open */

        comp->hIC = ICOpen(ICTYPE_VIDEO, file->video.fccHandler,
                           ICMODE_COMPRESS);
        if (!comp->hIC)
           VIDEO_ERROR(file, "Could not open the selected compressor");

        /* Make sure the compressor supports the input format */
        if (ICCompressQuery(comp->hIC, &file->video.info, NULL) != ICERR_OK)
           VIDEO_ERROR(file, "Input format not supported");

        /* Get the output format */
        assert(!comp->format);
        size = ICCompressGetFormatSize(comp->hIC, &file->video.info);

        assert(size >= sizeof(BITMAPINFOHEADER));

        comp->format = malloc(size);
        if (!comp->format)
           VIDEO_ERROR(file, "Out of memory");

        if (ICCompressGetFormat(comp->hIC, &file->video.info,
                                comp->format) != ICERR_OK)
           VIDEO_ERROR(file, "Could not get the output format");

        /* Get the compressor's default quality setting */
        comp->quality = ICGetDefaultQuality(comp->hIC);

        /* Get the compressor ready to begin working on input data */
        vars = &comp->vars;
        memset(vars, 0, sizeof(COMPVARS));

        vars->cbSize     = sizeof(COMPVARS);
        vars->dwFlags    = ICMF_COMPVARS_VALID;
        vars->hic        = comp->hIC;
        vars->fccType    = ICTYPE_VIDEO;
        vars->fccHandler = file->video.fccHandler;
        vars->lpbiOut    = (LPBITMAPINFO)comp->format;
        vars->lQ         = comp->quality;

        if (!ICSeqCompressFrameStart(vars, (BITMAPINFO *)&file->video.info))
           VIDEO_ERROR(file, "Couldn't begin compressing data");
    }

    comp->compressing = TRUE;

    return TRUE;
}

/** Closes the video compressor and deallocates compression memory
 * \param file A file previously used in a call to aviVideoCompressorOpen
 * \retval TRUE  Success
 * \retval FALSE Failure
 */

BOOL aviVideoCompressorClose(AVI_FILE *file)
{
    VIDEO_COMP *comp;

    if (!file)
       return FALSE;

    comp = &file->video.comp;

    if (comp->hIC)
    {
        if (comp->compressing)
        {
            ICSeqCompressFrameEnd(&comp->vars);
            comp->compressing = FALSE;
        }

        ICClose(comp->hIC);
        comp->hIC = 0;
    }

    if (comp->format)
    {
        free(comp->format);
        comp->format = NULL;
    }

    comp->compressing = FALSE;

    return TRUE;
}

/** Compresses a frame
 * \param file    The file to use
 * \param src     A pointer to the beginning of the input frame
 * \param isKey   Specifies if the compressed frame was a key frame or not
 * \param outSize The number of bytes stored in the output data
 * \return A pointer to the output buffer or NULL if an error occurred.
 *         The pointer that's returned CAN be \a src in some cases.
 */

void *aviVideoCompressData(AVI_FILE *file, void *src, BOOL *isKey,
                           DWORD *outSize)
{
    VIDEO_COMP *comp;
    void       *result;

    if (!file || !file->video.comp.compressing || !outSize || !isKey)
       return NULL;

    comp     = &file->video.comp;
    *outSize = 0;

    if (aviVideoFCCIsUncompressed(file->video.fccHandler))
    {
        /* Special case for uncompressed data */
        result   = src;
        *outSize = ((BITMAPINFOHEADER *)comp->format)->biSizeImage;
        *isKey   = TRUE;
    } else
      result = ICSeqCompressFrame(&comp->vars, 0, src, isKey, (LONG*)outSize);

    return result;
}

/** Gets the info header for the current output format
 * \param file A file that was used in a call to aviVideoCompressorOpen
 * \return A pointer to the output format's BITMAPINFOHEADER structure or NULL
 *         if the compression has not been started
 */

BITMAPINFOHEADER *aviVideoGetOutputFormat(AVI_FILE *file)
{
    if (!file)
       return NULL;

    return (BITMAPINFOHEADER *)file->video.comp.format;
}