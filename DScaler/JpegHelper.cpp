/////////////////////////////////////////////////////////////////////////////
// $Id: JpegHelper.cpp,v 1.1 2002-05-01 12:57:19 laurentg Exp $
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
//
//////////////////////////////////////////////////////////////////////////////

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



/* Expanded data source object for stdio input */

typedef struct {
  struct jpeg_source_mgr pub;	/* public fields */

  FILE * infile;		/* source stream */
  JOCTET * buffer;		/* start of buffer */
  boolean start_of_file;	/* have we gotten any data yet? */
} my_source_mgr;

typedef my_source_mgr * my_src_ptr;

#define INPUT_BUF_SIZE  4096	/* choose an efficiently fread'able size */

#define JFREAD(file,buf,sizeofbuf)  \
  ((size_t) fread((void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))

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

  if (nbytes <= 0) {
    if (src->start_of_file)	/* Treat empty input file as fatal error */
      ERREXIT(cinfo, JERR_INPUT_EMPTY);
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
  if (num_bytes > 0) {
    while (num_bytes > (long) src->pub.bytes_in_buffer) {
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
  if (cinfo->src == NULL) {	/* first time for this JPEG object? */
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

BOOL CJpegHelper::OpenMediaFile(LPCSTR FileName)
{
    int h, w, i, j;
    BYTE* pFrameBuf = NULL;
    BYTE* pDestBuf;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE* infile;
    JDIMENSION num_scanlines;
    JSAMPARRAY buffer;
    JDIMENSION buffer_height;
    jmp_buf jmp_mark;

    // Open the input stream (file)
	if ((infile = fopen(FileName, "rb")) == NULL)
    {
        return FALSE;
	}

    // Initialize the JPEG decompression object with default error handling
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

	jpeg_create_decompress(&cinfo);

	// Specify the source of the compressed data (eg, a file)
	my_jpeg_stdio_src(&cinfo, infile);

    // Call jpeg_read_header() to obtain image info
	(void)jpeg_read_header(&cinfo, TRUE);

    LOG(2, "image_height %d - image_width %d", cinfo.image_height, cinfo.image_width);
    LOG(2, "num_components %d", cinfo.num_components);
    LOG(2, "jpeg_color_space %d", cinfo.jpeg_color_space);
    LOG(2, "out_color_space %d", cinfo.out_color_space);
    LOG(2, "scale_num %u", cinfo.scale_num);
    LOG(2, "scale_denom %u", cinfo.scale_denom);
    LOG(2, "output_gamma %f", cinfo.output_gamma);

    // Set parameters for decompression
    cinfo.out_color_space = JCS_YCbCr;
    cinfo.scale_num = 1;
    cinfo.scale_denom = 1;
    cinfo.quantize_colors = FALSE;
    cinfo.dct_method = JDCT_ISLOW;
    cinfo.do_fancy_upsampling = TRUE;
    cinfo.do_block_smoothing = TRUE;

    // Begin decompression
	(void)jpeg_start_decompress(&cinfo);

    LOG(2, "output_height %d - output_width %d", cinfo.output_height, cinfo.output_width);
    LOG(2, "out_color_components %d", cinfo.out_color_components);
    LOG(2, "output_components %d", cinfo.output_components);
    LOG(2, "rec_outbuf_height %d", cinfo.rec_outbuf_height);

    h = cinfo.output_height;
    w = cinfo.output_width;

    buffer_height = cinfo.rec_outbuf_height;
    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, w * cinfo.output_components, buffer_height);

    // Allocate memory buffer to store the final YUYV values
    pFrameBuf = (BYTE*)malloc(w * 2 * h * sizeof(BYTE));
    if (pFrameBuf == NULL)
    {
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return FALSE;
    }

    // Process data
    pDestBuf = pFrameBuf;
    while (cinfo.output_scanline < h)
    {
        num_scanlines = jpeg_read_scanlines(&cinfo, buffer, buffer_height);

        // YUVYUV => YUYV
        for (i = 0 ; i < num_scanlines ; i++)
        {
            for (j = 0 ; j < w ; j++)
            {
                if (j%2)
                {
                    *pDestBuf = buffer[i][j * cinfo.output_components];
                    ++pDestBuf;
                    *pDestBuf = buffer[i][j * cinfo.output_components + 2];
                    ++pDestBuf;
                }
                else
                {
                    *pDestBuf = buffer[i][j * cinfo.output_components];
                    ++pDestBuf;
                    *pDestBuf = buffer[i][j * cinfo.output_components + 1];
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

    if (m_pParent->m_OriginalFrame.pData != NULL)
    {
        free(m_pParent->m_OriginalFrame.pData);
    }
    m_pParent->m_OriginalFrame.pData = pFrameBuf;
    m_pParent->m_Height = h;
    m_pParent->m_Width = w;
    m_pParent->m_SquarePixels = TRUE;

    return TRUE;
}


void CJpegHelper::SaveSnapshot(LPCSTR FilePath, int Height, int Width, BYTE* pOverlay, LONG OverlayPitch)
{
    return;
}
