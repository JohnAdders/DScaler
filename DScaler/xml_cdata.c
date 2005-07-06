/*
 *  XMLTV character data processing and storage
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License Version 2 as
 *  published by the Free Software Foundation. You find a copy of this
 *  license in the file COPYRIGHT in the root directory of this release.
 *
 *  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
 *  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
 *  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *
 *  Description:
 *
 *    This helper module manages buffers for character data.  Buffers
 *    will automatically grow when new data is appended.  For efficiency
 *    applications should just "reset" the buffer inbetween use, which
 *    just marks the buffer empty but keeps the buffer allocated.
 *
 *  Author: Tom Zoerner
 *
 *  $Id: xml_cdata.c,v 1.1 2005-07-06 19:42:39 laurentg Exp $
 */

#define DEBUG_SWITCH DEBUG_SWITCH_XMLTV
#define DPRINTF_OFF

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "xmltv_types.h"
#include "xmltv_debug.h"

#include "xml_cdata.h"

#define DEFAULT_BUF_SIZE_STEP    2048

// ----------------------------------------------------------------------------
// Initialize a string buffer
// - size hint is used as start size for the buffer when the first data is added
// - this call is optional; the user may also initialize the buffer state
//   by setting it to all zero
//
void XmlCdata_Init( XML_STR_BUF * pBuf, uint sizeHint )
{
   if (pBuf != NULL)
   {
      if (sizeHint == 0)
         sizeHint = DEFAULT_BUF_SIZE_STEP;

      memset(pBuf, 0, sizeof(XML_STR_BUF));
      pBuf->sizeHint = sizeHint;
   }
   else
      fatal0("XmlCdata-Init: illegal NULL ptr param");
}

// ----------------------------------------------------------------------------
// Discard data in string buffer
// - note: buffer is not freed (for efficiency it's re-used)
//
#ifndef XML_CDATA_INLINE
void XmlCdata_Reset( XML_STR_BUF * pBuf )
{
   if (pBuf != NULL)
   {
      assert((pBuf->pStrBuf == NULL) ? (pBuf->off == 0) : (pBuf->off + 1 <= pBuf->size));
      assert(pBuf->skip <= pBuf->off);

      if (pBuf->pStrBuf != NULL)
      {
         pBuf->off = 0;
         pBuf->skip = 0;
         pBuf->pStrBuf[0] = 0;
      }
   }
   else
      fatal0("XmlCdata-Reset: illegal NULL ptr param");
}
#endif

// ----------------------------------------------------------------------------
// Free resources allocated in string buffer
//
void XmlCdata_Free( XML_STR_BUF * pBuf )
{
   if (pBuf != NULL)
   {
      XmlCdata_Reset(pBuf);

      if (pBuf->pStrBuf != NULL)
      {
         xfree(pBuf->pStrBuf);
         pBuf->pStrBuf = NULL;
         pBuf->size = 0;
      }
   }
   else
      fatal0("XmlCdata-Free: illegal NULL ptr param");
}

// ----------------------------------------------------------------------------
// Grow a buffer - called in case the buffer overflows
//
void XmlCdata_Grow( XML_STR_BUF * pBuf, uint len )
{
   char * pNewbuf;
   uint newSize;

   assert((pBuf->pStrBuf == NULL) ? (pBuf->off == 0) : (pBuf->off + 1 <= pBuf->size));
   assert(pBuf->skip <= pBuf->off);

   // size hint may be zero if the caller doesn't call "init" above, but instead just uses memset(0)
   if (pBuf->sizeHint == 0)
      pBuf->sizeHint = DEFAULT_BUF_SIZE_STEP;

   // increase buffer size in steps of the hint
   newSize = pBuf->size + pBuf->sizeHint;
   if (newSize < pBuf->off - pBuf->skip + len + 1)
   {
      dprintf4("XmlCdata-Grow: buffer len increment %d-%d+%d exceeds size hint %d\n", pBuf->off, pBuf->skip, len, pBuf->sizeHint);
      newSize = pBuf->off - pBuf->skip + len + 1 + (pBuf->sizeHint / 2);
      pBuf->sizeHint = newSize;
   }

   // copy content into the new buffer and free the old one
   if ((pBuf->pStrBuf != NULL) && (pBuf->skip == 0))
   {
      pNewbuf = xrealloc(pBuf->pStrBuf, newSize);
   }
   else
   {
      pNewbuf = xmalloc(newSize);
      if (pBuf->pStrBuf != NULL)
      {
         memcpy(pNewbuf, pBuf->pStrBuf + pBuf->skip, pBuf->size - pBuf->skip);
         xfree(pBuf->pStrBuf);
      }
      pBuf->skip = 0;
   }
   pBuf->pStrBuf = pNewbuf;
   pBuf->size = newSize;
}

// ----------------------------------------------------------------------------
// Append a chunk of text to a buffer
//
#ifndef XML_CDATA_INLINE
void XmlCdata_AppendRaw( XML_STR_BUF * pBuf, const char * pStr, uint len )
{
   if ( (pBuf->pStrBuf == NULL) ||
        (pBuf->off + len + 1 > pBuf->size) )
   {
      XmlCdata_Grow(pBuf, len);
   }

   memcpy(pBuf->pStrBuf + pBuf->off, pStr, len);
   pBuf->off += len;
   pBuf->pStrBuf[pBuf->off] = 0;
}

void XmlCdata_AppendString( XML_STR_BUF * pBuf, const char * pStr )
{
   XmlCdata_AppendRaw(pBuf, pStr, strlen(pStr));
}
#endif

// ----------------------------------------------------------------------------
// Assign one buffer's data to another buffer
// - the content of the two buffers are swapped
//
void XmlCdata_Assign( XML_STR_BUF * pDestBuf, XML_STR_BUF * pSrcBuf )
{
   char * pTmp;
   uint tmpSize;

   if ((pSrcBuf != NULL) && (pDestBuf != NULL))
   {
      if (pSrcBuf->pStrBuf != NULL)
      {
         // swap pointers and size between the two buffers
         pTmp = pDestBuf->pStrBuf;
         tmpSize = pDestBuf->size;

         pDestBuf->pStrBuf = pSrcBuf->pStrBuf;
         pDestBuf->size = pSrcBuf->size;
         pDestBuf->off = pSrcBuf->off;
         pDestBuf->skip = pSrcBuf->skip;

         pSrcBuf->pStrBuf = pTmp;
         pSrcBuf->size = tmpSize;
         pSrcBuf->off = 0;
         pSrcBuf->skip = 0;
         if (pSrcBuf->pStrBuf != NULL)
         {
            pSrcBuf->pStrBuf[0] = 0;
         }
      }
   }
   else
      fatal0("XmlCdata-Assign: invalid NULL ptr param");
}

#ifndef XML_CDATA_INLINE
void XmlCdata_AssignOrAppend( XML_STR_BUF * pDestBuf, XML_STR_BUF * pSrcBuf )
{
   if ((pDestBuf->off == 0) && (pSrcBuf->off > 128))
      XmlCdata_Assign(pDestBuf, pSrcBuf);
   else
      XmlCdata_AppendRaw(pDestBuf, XML_STR_BUF_GET_STR(*pSrcBuf), XML_STR_BUF_GET_STR_LEN(*pSrcBuf));
}
#endif

// ----------------------------------------------------------------------------
// Remove leading and trailing whitespace
// - leading whitespace is not really removed, but an offset is stored within
//   the buffer which is later added when retrieving the text
//
void XmlCdata_TrimWhitespace( XML_STR_BUF * pBuf )
{
   uint len;
   uint toff;
   char * p;

   if (pBuf != NULL)
   {
      if (pBuf->off > 0)
      {
         len = pBuf->off;
         p = pBuf->pStrBuf + len - 1;

         while ((len > 0) &&
                ((*p == ' ') || (*p == '\t') || (*p == '\n') || (*p == '\r')))
         {
            len--;
            p--;
         }
         pBuf->off = len;
         *(++p) = 0;

         p = pBuf->pStrBuf;
         toff = 0;
         while ((*p == ' ') || (*p == '\t') || (*p == '\n') || (*p == '\r'))
         {
            toff++;
            p++;
         }
         pBuf->skip = toff;
      }
   }
   else
      fatal0("XmlCdata-TrimWhitespace: invalid NULL ptr param");
}

// ----------------------------------------------------------------------------
// Append paragraph break to text buffer
//
void XmlCdata_AppendParagraph( XML_STR_BUF * pBuf, bool insertTwo )
{
   // if string is empty, do nothing
   if (pBuf->off > 0)
   {
      if (insertTwo)
      {
         if (pBuf->pStrBuf[pBuf->off - 1] == '\n')
         {
            if ( (pBuf->off > 2) &&
                 (pBuf->pStrBuf[pBuf->off - 2] != '\n') )
            {
               // single newline only (and string consists not of newline only) -> add one
               XmlCdata_AppendRaw(pBuf, "\n", 1);
            }
         }
         else
         {  // no newline at all at the end -> add double newline
            XmlCdata_AppendRaw(pBuf, "\n\n", 2);
         }
      }
      else
      {
         if ( (pBuf->off > 1) &&
              (pBuf->pStrBuf[pBuf->off - 1] != '\n') )
         {
            XmlCdata_AppendRaw(pBuf, "\n", 1);
         }
      }
   }
}

