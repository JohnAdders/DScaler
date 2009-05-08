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
 * @file JpegHelper.cpp Jpeg Helper functions
 */

#include "stdafx.h"
#include "JpegHelper.h"
#include "DebugLog.h"
#define XMD_H
extern "C"
{
#include "..\ThirdParty\LibJpeg\jerror.h"
#include "..\ThirdParty\LibJpeg\jpeglib.h"
}
#include <setjmp.h>

using namespace std;

/* Expanded data source object for stdio input */

typedef struct {
  struct jpeg_source_mgr pub;    /* public fields */

  FILE * infile;        /* source stream */
  JOCTET * buffer;        /* start of buffer */
  boolean start_of_file;    /* have we gotten any data yet? */
} my_source_mgr;

typedef my_source_mgr * my_src_ptr;

/* Expanded data destination object for stdio output */

typedef struct {
  struct jpeg_destination_mgr pub; /* public fields */

  FILE * outfile;        /* target stream */
  JOCTET * buffer;        /* start of buffer */
} my_destination_mgr;

typedef my_destination_mgr * my_dest_ptr;


#define LIMIT_Y(x)      (((x)<16)?16:((x)>235)?235:(x))
#define LIMIT_CbCr(x)   (((x)<16)?16:((x)>240)?240:(x))

#define INPUT_BUF_SIZE  4096    /* choose an efficiently fread'able size */
#define OUTPUT_BUF_SIZE 4096    /* choose an efficiently fwrite'able size */
#define SQUARE_MODE_ON  "DScaler:Square Pixels Mode:ON"
#define SQUARE_MODE_OFF "DScaler:Square Pixels Mode:OFF"

#define JFREAD(file,buf,sizeofbuf)  \
  ((size_t) fread((void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))
#define JFWRITE(file,buf,sizeofbuf)  \
  ((size_t) fwrite((const void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))


char msg_err[512];


/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */

METHODDEF(void)
init_source (j_decompress_ptr cinfo)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;

  /* We reset the empty-input-file flag for each image,
   * but we don't clear the input buffer.
   * This is correct behavior for reading a series of images from one source.
   */
  src->start_of_file = TRUE;
}


/*
 * Fill the input buffer --- called whenever buffer is emptied.
 *
 * In typical applications, this should read fresh data into the buffer
 * (ignoring the current state of next_input_byte & bytes_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been reloaded.  It is not necessary to
 * fill the buffer entirely, only to obtain at least one more byte.
 *
 * There is no such thing as an EOF return.  If the end of the file has been
 * reached, the routine has a choice of ERREXIT() or inserting fake data into
 * the buffer.  In most cases, generating a warning message and inserting a
 * fake EOI marker is the best course of action --- this will allow the
 * decompressor to output however much of the image is there.  However,
 * the resulting error message is misleading if the real problem is an empty
 * input file, so we handle that case specially.
 *
 * In applications that need to be able to suspend compression due to input
 * not being available yet, a FALSE return indicates that no more data can be
 * obtained right now, but more may be forthcoming later.  In this situation,
 * the decompressor will return to its caller (with an indication of the
 * number of scanlines it has read, if any).  The application should resume
 * decompression after it has loaded more data into the input buffer.  Note
 * that there are substantial restrictions on the use of suspension --- see
 * the documentation.
 *
 * When suspending, the decompressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_input_byte & bytes_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point must be rescanned after resumption, so move it to
 * the front of the buffer rather than discarding it.
 */

METHODDEF(Boolean)
fill_input_buffer (j_decompress_ptr cinfo)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;
  size_t nbytes;

  nbytes = JFREAD(src->infile, src->buffer, INPUT_BUF_SIZE);

  if (nbytes <= 0)
  {
    if (src->start_of_file)    /* Treat empty input file as fatal error */
    {
      ERREXIT(cinfo, JERR_INPUT_EMPTY);
    }
    WARNMS(cinfo, JWRN_JPEG_EOF);
    /* Insert a fake EOI marker */
    src->buffer[0] = (JOCTET) 0xFF;
    src->buffer[1] = (JOCTET) JPEG_EOI;
    nbytes = 2;
  }

  src->pub.next_input_byte = src->buffer;
  src->pub.bytes_in_buffer = nbytes;
  src->start_of_file = FALSE;

  return TRUE;
}


/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 * Writers of suspendable-input applications must note that skip_input_data
 * is not granted the right to give a suspension return.  If the skip extends
 * beyond the data currently in the buffer, the buffer can be marked empty so
 * that the next read will cause a fill_input_buffer call that can suspend.
 * Arranging for additional bytes to be discarded before reloading the input
 * buffer is the application writer's problem.
 */

METHODDEF(void)
skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;

  /* Just a dumb implementation for now.  Could use fseek() except
   * it doesn't work on pipes.  Not clear that being smart is worth
   * any trouble anyway --- large skips are infrequent.
   */
  if (num_bytes > 0)
  {
    while (num_bytes > (long) src->pub.bytes_in_buffer)
    {
      num_bytes -= (long) src->pub.bytes_in_buffer;
      (void) fill_input_buffer(cinfo);
      /* note we assume that fill_input_buffer will never return FALSE,
       * so suspension need not be handled.
       */
    }
    src->pub.next_input_byte += (size_t) num_bytes;
    src->pub.bytes_in_buffer -= (size_t) num_bytes;
  }
}


/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.  Often a no-op.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

METHODDEF(void)
term_source (j_decompress_ptr cinfo)
{
  /* no work necessary here */
}


/*
 * Prepare for input from a stdio stream.
 * The caller must have already opened the stream, and is responsible
 * for closing it after finishing decompression.
 */

GLOBAL(void)
my_jpeg_stdio_src (j_decompress_ptr cinfo, FILE * infile)
{
  my_src_ptr src;

  /* The source object and input buffer are made permanent so that a series
   * of JPEG images can be read from the same file by calling jpeg_stdio_src
   * only before the first one.  (If we discarded the buffer at the end of
   * one image, we'd likely lose the start of the next one.)
   * This makes it unsafe to use this manager and a different source
   * manager serially with the same JPEG object.  Caveat programmer.
   */
  if (cinfo->src == NULL) /* first time for this JPEG object? */
  {
    cinfo->src = (struct jpeg_source_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                  (size_t) sizeof(my_source_mgr));
    src = (my_src_ptr) cinfo->src;
    src->buffer = (JOCTET *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                  INPUT_BUF_SIZE * (size_t) sizeof(JOCTET));
  }

  src = (my_src_ptr) cinfo->src;
  src->pub.init_source = init_source;
  src->pub.fill_input_buffer = fill_input_buffer;
  src->pub.skip_input_data = skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
  src->pub.term_source = term_source;
  src->infile = infile;
  src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
  src->pub.next_input_byte = NULL; /* until buffer loaded */
}


/*
 * Initialize destination --- called by jpeg_start_compress
 * before any data is actually written.
 */

METHODDEF(void)
init_destination (j_compress_ptr cinfo)
{
  my_dest_ptr dest = (my_dest_ptr) cinfo->dest;

  /* Allocate the output buffer --- it will be released when done with image */
  dest->buffer = (JOCTET *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
                  OUTPUT_BUF_SIZE * (size_t) sizeof(JOCTET));

  dest->pub.next_output_byte = dest->buffer;
  dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
}


/*
 * Empty the output buffer --- called whenever buffer fills up.
 *
 * In typical applications, this should write the entire output buffer
 * (ignoring the current state of next_output_byte & free_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been dumped.
 *
 * In applications that need to be able to suspend compression due to output
 * overrun, a FALSE return indicates that the buffer cannot be emptied now.
 * In this situation, the compressor will return to its caller (possibly with
 * an indication that it has not accepted all the supplied scanlines).  The
 * application should resume compression after it has made more room in the
 * output buffer.  Note that there are substantial restrictions on the use of
 * suspension --- see the documentation.
 *
 * When suspending, the compressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_output_byte & free_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point will be regenerated after resumption, so do not
 * write it out when emptying the buffer externally.
 */

METHODDEF(Boolean)
empty_output_buffer (j_compress_ptr cinfo)
{
  my_dest_ptr dest = (my_dest_ptr) cinfo->dest;

  if (JFWRITE(dest->outfile, dest->buffer, OUTPUT_BUF_SIZE) !=
      (size_t) OUTPUT_BUF_SIZE)
  {
    ERREXIT(cinfo, JERR_FILE_WRITE);
  }

  dest->pub.next_output_byte = dest->buffer;
  dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;

  return TRUE;
}


/*
 * Terminate destination --- called by jpeg_finish_compress
 * after all data has been written.  Usually needs to flush buffer.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

METHODDEF(void)
term_destination (j_compress_ptr cinfo)
{
  my_dest_ptr dest = (my_dest_ptr) cinfo->dest;
  size_t datacount = OUTPUT_BUF_SIZE - dest->pub.free_in_buffer;

  /* Write any data remaining in the buffer */
  if (datacount > 0)
  {
    if (JFWRITE(dest->outfile, dest->buffer, datacount) != datacount)
    {
      ERREXIT(cinfo, JERR_FILE_WRITE);
    }
  }
  fflush(dest->outfile);
  /* Make sure we wrote the output file OK */
  if (ferror(dest->outfile))
  {
    ERREXIT(cinfo, JERR_FILE_WRITE);
  }
}


/*
 * Prepare for output to a stdio stream.
 * The caller must have already opened the stream, and is responsible
 * for closing it after finishing compression.
 */

GLOBAL(void)
my_jpeg_stdio_dest (j_compress_ptr cinfo, FILE * outfile)
{
  my_dest_ptr dest;

  /* The destination object is made permanent so that multiple JPEG images
   * can be written to the same file without re-executing jpeg_stdio_dest.
   * This makes it dangerous to use this manager and a different destination
   * manager serially with the same JPEG object, because their private object
   * sizes may be different.  Caveat programmer.
   */
  if (cinfo->dest == NULL)    /* first time for this JPEG object? */
  {
    cinfo->dest = (struct jpeg_destination_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                  (size_t) sizeof(my_destination_mgr));
  }

  dest = (my_dest_ptr) cinfo->dest;
  dest->pub.init_destination = init_destination;
  dest->pub.empty_output_buffer = empty_output_buffer;
  dest->pub.term_destination = term_destination;
  dest->outfile = outfile;
}


METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
    msg_err[0] = '\0';
    (*cinfo->err->format_message)(cinfo, msg_err);

    // Return control to the setjmp point
    longjmp(*(jmp_buf*)(cinfo->client_data), 1);
}


METHODDEF(void)
my_output_message (j_common_ptr cinfo)
{
    // Do nothing
}


METHODDEF(void)
my_emit_message (j_common_ptr cinfo, int msg_level)
{
    // Do nothing
}



CJpegHelper::CJpegHelper(CStillSource* pParent) :
    CStillSourceHelper(pParent)
{
}

BOOL CJpegHelper::OpenMediaFile(const tstring& FileName)
{
    int h, w, i, j;
    BYTE* pFrameBuf = NULL;
    BYTE* pStartFrame;
    BYTE* pDestBuf;
    BYTE* pDestBuf2;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    jpeg_saved_marker_ptr pMarker;
    FILE* infile;
    JDIMENSION num_scanlines;
    JSAMPARRAY buffer;
    JDIMENSION buffer_height;
    BOOL Square = TRUE;
    int LinePitch;
    jmp_buf jmp_mark;

    // Open the input stream (file)
    if ((infile = _tfopen(FileName.c_str(), _T("rb"))) == NULL)
    {
        return FALSE;
    }

    // Error handling
    cinfo.err = jpeg_std_error(&jerr);
    cinfo.client_data = &jmp_mark;
    cinfo.err->error_exit = my_error_exit;
    cinfo.err->output_message = my_output_message;
    cinfo.err->emit_message = my_emit_message;
    cinfo.err->trace_level = 0;
    if (setjmp(jmp_mark))
    {
        // If we get here, the JPEG code has signaled an error.
        // We need to clean up the JPEG object, close the input file, and return.
        LOG(1, "libjpeg fatal error : %s", msg_err);
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        if (pFrameBuf != NULL)
        {
            free(pFrameBuf);
        }
        return FALSE;
    }

    // Initialize the JPEG decompression object
    jpeg_create_decompress(&cinfo);

    // Specify the source of the compressed data (eg, a file)
    my_jpeg_stdio_src(&cinfo, infile);

    // Read DScaler markers APP1 and APP2
    jpeg_save_markers(&cinfo, JPEG_APP0+1, 0xFFFF);
    jpeg_save_markers(&cinfo, JPEG_APP0+2, 0xFFFF);

    // Call jpeg_read_header() to obtain image info
    (void)jpeg_read_header(&cinfo, TRUE);

    LOG(2, _T("image_height %d - image_width %d"), cinfo.image_height, cinfo.image_width);
    LOG(2, _T("num_components %d"), cinfo.num_components);
    LOG(2, _T("jpeg_color_space %d"), cinfo.jpeg_color_space);
    LOG(2, _T("out_color_space %d"), cinfo.out_color_space);
    LOG(2, _T("scale_num %u"), cinfo.scale_num);
    LOG(2, _T("scale_denom %u"), cinfo.scale_denom);
    LOG(2, _T("output_gamma %f"), cinfo.output_gamma);

    m_pParent->m_Comments = _T("");
    pMarker = cinfo.marker_list;
    while (pMarker != NULL)
    {
        LOG(2, _T("marker %d"), pMarker->marker);
        // Check that the marker is a DScaler marker
        if (!strncmp((char*)pMarker->data, "DScaler", 7))
        {
            if (pMarker->marker == JPEG_APP0+1)
            {
                m_pParent->m_Comments.assign((char*)pMarker->data, (char*)pMarker->data + pMarker->data_length);
            }
            else if ( (pMarker->marker == JPEG_APP0+2)
              && ! strncmp((char*)pMarker->data, SQUARE_MODE_OFF, pMarker->data_length) )
            {
                Square = FALSE;
            }
        }
        pMarker = pMarker->next;
    }

    // Set parameters for decompression
    cinfo.out_color_space = JCS_YCbCr;
    cinfo.scale_num = 1;
    cinfo.scale_denom = 1;
    cinfo.quantize_colors = FALSE;
    cinfo.dct_method = JDCT_ISLOW;
    cinfo.do_fancy_upsampling = TRUE;
    cinfo.do_block_smoothing = FALSE;

    // Start decompression
    (void)jpeg_start_decompress(&cinfo);

    LOG(2, _T("output_height %d - output_width %d"), cinfo.output_height, cinfo.output_width);
    LOG(2, _T("out_color_components %d"), cinfo.out_color_components);
    LOG(2, _T("output_components %d"), cinfo.output_components);
    LOG(2, _T("rec_outbuf_height %d"), cinfo.rec_outbuf_height);

    buffer_height = cinfo.rec_outbuf_height;
    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, cinfo.output_width * cinfo.output_components, buffer_height);

    h = cinfo.output_height;
    w = cinfo.output_width;
    // The width must be even
    if (w%2)
    {
        w--;
    }

    // Allocate memory buffer to store the final YUYV values
    LinePitch = (w * 2 * sizeof(BYTE) + 15) & 0xfffffff0;
    pFrameBuf = (BYTE*)malloc(LinePitch * h + 16);
    if (pFrameBuf == NULL)
    {
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return FALSE;
    }
    pStartFrame = START_ALIGNED16(pFrameBuf);

    // Process data
    while (cinfo.output_scanline < h)
    {
        pDestBuf2 = pStartFrame + cinfo.output_scanline * LinePitch;
        num_scanlines = jpeg_read_scanlines(&cinfo, buffer, buffer_height);

        // In the input (file), Y range is [0,255]
        // In the output (overlay), Y range must be [16,235]
        // YUVYUV => YUYV with an Y compression from [0,255] to [16,235]
        for (i = 0 ; i < num_scanlines ; i++)
        {
            pDestBuf = pDestBuf2 + i * LinePitch;
            for (j = 0 ; j < w ; j++)
            {
                // Y compression from [0,255] to [16,235]
                *pDestBuf = LIMIT_Y(buffer[i][j * cinfo.output_components] * 219 / 255 + 16);
                ++pDestBuf;
                if (j%2)
                {
                    *pDestBuf = LIMIT_CbCr(buffer[i][j * cinfo.output_components + 2]);
                    ++pDestBuf;
                }
                else
                {
                    *pDestBuf = LIMIT_CbCr(buffer[i][j * cinfo.output_components + 1]);
                    ++pDestBuf;
                }
            }
        }
    }

    // Finish decompression
    (void)jpeg_finish_decompress(&cinfo);

    // Release the JPEG decompression object
    jpeg_destroy_decompress(&cinfo);

    // Close the input stream (file)
    fclose(infile);

    m_pParent->FreeOriginalFrameBuffer();
    m_pParent->m_OriginalFrameBuffer = pFrameBuf;
    m_pParent->m_OriginalFrame.pData = pStartFrame;
    m_pParent->m_LinePitch = LinePitch;
    m_pParent->m_InitialHeight = h;
    m_pParent->m_InitialWidth = w;
    m_pParent->m_Height = h;
    m_pParent->m_Width = w;
    m_pParent->m_SquarePixels = Square;

    return TRUE;
}


void CJpegHelper::SaveSnapshot(const tstring& FilePath, int Height, int Width, BYTE* pOverlay, LONG OverlayPitch, const tstring& Context)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE * outfile;
    JSAMPROW row_pointer[1];
    JSAMPLE* pLineBuf;
    BYTE *pBufOverlay;
    int i, quality;
    jmp_buf jmp_mark;

    // Allocate memory buffer to store one line of data
    pLineBuf = (JSAMPLE*)malloc(Width * 3 * sizeof(JSAMPLE));
    if (pLineBuf == NULL)
    {
        return;
    }

    // Open the output stream (file)
    if ((outfile = _tfopen(FilePath.c_str(), _T("wb"))) == NULL)
    {
        free(pLineBuf);
        return;
    }

    // Error handling
    cinfo.err = jpeg_std_error(&jerr);
    cinfo.client_data = &jmp_mark;
    cinfo.err->error_exit = my_error_exit;
    cinfo.err->output_message = my_output_message;
    cinfo.err->emit_message = my_emit_message;
    cinfo.err->trace_level = 0;
    if (setjmp(jmp_mark))
    {
        // If we get here, the JPEG code has signaled an error.
        // We need to clean up the JPEG object, close the output file, and return.
        LOG(1, "libjpeg fatal error : %s", msg_err);
        jpeg_destroy_compress(&cinfo);
        fclose(outfile);
        free(pLineBuf);
        return;
    }

    // Initialize the JPEG compression object
    jpeg_create_compress(&cinfo);

    // Specify the destination of the compressed data (eg, a file)
    my_jpeg_stdio_dest(&cinfo, outfile);

    // Set parameters for compression
    cinfo.image_width = Width;
    cinfo.image_height = Height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_YCbCr;
    jpeg_set_defaults(&cinfo);
    jpeg_set_colorspace(&cinfo, JCS_YCbCr);
    quality = Setting_GetValue(WM_STILL_GETVALUE, JPEGQUALITY);
    jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
    cinfo.dct_method = JDCT_ISLOW;
    cinfo.optimize_coding = TRUE;

    // Start compression
    jpeg_start_compress(&cinfo, TRUE);

    // Write DScaler marker APP1 with the DScaler current context
    jpeg_write_marker(&cinfo, JPEG_APP0 + 1, (JOCTET*)Context.c_str(), Context.length());

    // Write DScaler marker APP2 to save if square pixels mode is on or off
    if (m_pParent->m_SquarePixels)
    {
        jpeg_write_marker(&cinfo, JPEG_APP0 + 2, (JOCTET*)SQUARE_MODE_ON, strlen(SQUARE_MODE_ON));
    }
    else
    {
        jpeg_write_marker(&cinfo, JPEG_APP0 + 2, (JOCTET*)SQUARE_MODE_OFF, strlen(SQUARE_MODE_OFF));
    }

    // Process data
    row_pointer[0] = pLineBuf;
    while (cinfo.next_scanline < Height)
    {
        pBufOverlay = pOverlay + cinfo.next_scanline * OverlayPitch;
        // In the input (overlay), Y range should be normally [16,235]
        // In the output (file), Y range must be [0,255]
        // YUYV => YUVYUV with an Y extension from [16,235] to [0,255]
        for (i = 0 ; i < Width ; i++)
        {
            // Y extension from [16,235] to [0,255]
            pLineBuf[i * 3] = (LIMIT_Y(pBufOverlay[i * 2]) - 16) * 255 / 219;
            if (i%2)
            {
                pLineBuf[i * 3 + 1] = LIMIT_CbCr(pBufOverlay[i * 2 - 1]);
                pLineBuf[i * 3 + 2] = LIMIT_CbCr(pBufOverlay[i * 2 + 1]);
            }
            else
            {
                pLineBuf[i * 3 + 1] = LIMIT_CbCr(pBufOverlay[i * 2 + 1]);
                if (i != (Width - 1))
                {
                    pLineBuf[i * 3 + 2] = LIMIT_CbCr(pBufOverlay[i * 2 + 3]);
                }
            }
        }
        (void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    // Finish compression
    jpeg_finish_compress(&cinfo);

    // Release the JPEG decompression object
    jpeg_destroy_compress(&cinfo);

    // Close the input stream (file)
    fclose(outfile);

    // Free the line buffer
    free(pLineBuf);

    return;
}
