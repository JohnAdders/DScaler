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
 *  $Id: xml_cdata.cpp,v 1.1 2008-03-11 10:07:38 adcockj Exp $
 */

#define DEBUG_SWITCH DEBUG_SWITCH_XMLTV
#define DPRINTF_OFF

#include "stdafx.h"
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
      pNewbuf = (char*) xrealloc(pBuf->pStrBuf, newSize);
   }
   else
   {
      pNewbuf = (char*) xmalloc(newSize);
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
// - this function is usually inlined as a macro (when switch XML_CDATA_INLINE
//   is defined) but it's also called via function pointer by the scanner when
//   no transcoding is required
//
void XmlCdata_AppendRawNOINLINE( XML_STR_BUF * pBuf, const char * pStr, uint len )
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

#ifndef XML_CDATA_INLINE
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
         pDestBuf->lang = pSrcBuf->lang;

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
   {
      XmlCdata_Assign(pDestBuf, pSrcBuf);
   }
   else
   {
      XmlCdata_AppendRaw(pDestBuf, XML_STR_BUF_GET_STR(*pSrcBuf), XML_STR_BUF_GET_STR_LEN(*pSrcBuf));
      pDestBuf->lang = pSrcBuf->lang;
   }
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
void XmlCdata_AppendParagraph( XML_STR_BUF * pBuf, Bool insertTwo )
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

// ----------------------------------------------------------------------------
// Append characters to a buffer while transcoding Latin-1 to UTF-8
// - the transcoding is very easy because Latin-1 characters have the same codes
//   in ISO8859-1 and Unicode; so here we only change the single-byte Latin-1
//   code into a 2-byte UTF-8 code
//
void XmlCdata_AppendLatin1ToUtf8( XML_STR_BUF * pBuf, const char * pStr, uint len )
{
   char * pDest;
   uchar code;

   // to save time, we allocate space for the worst case which is twice the original size
   // hence we don't have to keep track of the destination buffer size in the loop
   if ( (pBuf->pStrBuf == NULL) ||
        (pBuf->off + (2 * len) + 1 > pBuf->size) )
   {
      XmlCdata_Grow(pBuf, 2 * len);
   }

   pDest = pBuf->pStrBuf + pBuf->off;

   for ( ; len != 0; len-- )
   {
      code = *(pStr++);

      if (code < 0x80)
      {
         // ASCII range: passed through unmodified
         *(pDest++) = code;
      }
      else
      {
         // Latin-1 code page: char code is identical, but encoded as 2-byte sequence
         *(pDest++) = 0xC0 | (char) (code >> 6);
         *(pDest++) = 0x80 | (char) (code & 0x3F);
      }
   }
   *pDest = 0;

   pBuf->off = pDest - pBuf->pStrBuf;
}

// ----------------------------------------------------------------------------
// Append characters to a buffer while transcoding UTF-8 to Latin-1
//
void XmlCdata_AppendUtf8ToLatin1( XML_STR_BUF * pBuf, const char * pStr, uint len )
{
   char * pDest;
   uint code;
   uchar c1;
   uchar c2;

   // Latin-1 encoding is guaranteed to be at most as long as UTF-8
   if ( (pBuf->pStrBuf == NULL) ||
        (pBuf->off + len + 1 > pBuf->size) )
   {
      XmlCdata_Grow(pBuf, len);
   }

   pDest = pBuf->pStrBuf + pBuf->off;

   while (len != 0)
   {
      c1 = *(pStr++);
      len--;

      if (c1 < 0x80)
      {
         // ASCII range: passed through unmodified
         *(pDest++) = c1;
      }
      else if ((c1 & 0xE0) == 0xC0)
      {
         if (len != 0)
         {
            c2 = *(pStr++);
            len--;

            code = ((c1 & 0x1F) << 6) | (c2 & 0x3F);
            if ( ((c2 & 0xC0) == 0x80) && (code < 0x100) )
               *(pDest++) = code;
            else
               *(pDest++) = (char)0xA0;  // outside of ISO8859-1 range
         }
         else
         {
            dprintf1("XmlCdata-AppendUtf8ToLatin1: sequence error in input: string ends after 0x%02X\n", c1);
            *(pDest++) = (char)0xA0;
         }
      }
      else
      {  // >= 3-byte sequence -> Unicode character codes >= 0x800
         // not a Latin-1 character, replace with Latin-1 'NBSP' (non-breaking space)
         *(pDest++) = (char)0xA0;

         // skip the complete multi-byte code in the source string
         while ( (len != 0) && ((*pStr & 0xC0) == 0x80) )
         {
            pStr++;
            len--;
         }
      }
   }
   *pDest = 0;

   pBuf->off = pDest - pBuf->pStrBuf;
}

// ----------------------------------------------------------------------------
// Check if all characters in a Latin-1 encoded string are valid
// - valid means in the range defined by XML 1.0 3rd ed., ch. 2.2
// - note the character check functions are required because the scanner does
//   not check for invalid control codes for performance reasons
//
Bool XmlCdata_CheckLatin1( const char * pStr )
{
   uchar code;
   Bool result = TRUE;

   if (pStr != NULL)
   {
      while ( (code = *(pStr++)) != 0 )
      {
         // invalid codes are [0x0...0x31] except whitespace characters TAB, NL, CR
         if (code < 0x20)
         {
            if ((code != 0x09) && (code != 0x0A) && (code != 0x0D))
            {
               result = FALSE;
               break;
            }
         }
      }
   }
   return result;
}

// ----------------------------------------------------------------------------
// Check if all characters in a Latin-1 encoded string are letters
// - valid range for letters is defined by XML 1.0 3rd ed., Annex B
// - note the name check functions are required because the scanner just matches in
//   the ASCII range; all codes >= 0x80 are treated as letters and passed through
//
Bool XmlCdata_CheckLatin1Name( const char * pStr, Bool isNmtoken )
{
   uchar code;
   Bool result = TRUE;

   if (pStr != NULL)
   {
      while ( (code = *(pStr++)) != 0 )
      {
         if (code < 0x80)
         {
            if (code < 0x20)
            {
               if ((code != 0x09) && (code != 0x0A) && (code != 0x0D))
               {
                  result = FALSE;
                  break;
               }
            }
            // else: ASCII range already checked by scanner
         }
         else //if (code >= 0x80)
         {
            // [#x00C0-#x00D6] | [#x00D8-#x00F6] | [#x00F8-#x00FF]
            if (code >= 0xc0)
            {
               if ((code == 0xD7) && (code == 0xF7))
               {
                  result = FALSE;
                  break;
               }
            }
            else if (isNmtoken && (code == 0xB7))
            {
               // extender: OK
            }
            else
            {
               result = FALSE;
               break;
            }
         }
         isNmtoken = TRUE;
      }
   }
   return result;
}

// ----------------------------------------------------------------------------
// Check if a given string is valid UTF-8
// - to by valid it must firstly be syntactically correct, i.e. all sequence
//   headers must be followed by the correct amount of trailing bytes
// - secondly, all characters must be in the range allowed by XML 1.0, ed. 3
//
Bool XmlCdata_CheckUtf8( const char * pStr )
{
   uchar c1, c2, c3, c4;
   uint code;
   Bool result = TRUE;

   if (pStr != NULL)
   {
      while ( ((c1 = *(pStr++)) != 0) && result )
      {
         if (c1 < 0x80)
         {
            if (c1 < 0x20)
            {
               if ((c1 != 0x09) && (c1 != 0x0A) && (c1 != 0x0D))
               {
                  dprintf1("XmlCdata-CheckUtf8: invalid control code 0x%02X\n", c1);
                  result = FALSE;
               }
            }
            // else: ASCII range already checked by scanner
         }
         else
         {
            switch (c1 >> 4)
            {
               case 0xC:  // 2-byte code
               case 0xD:
                  c2 = *pStr++;
                  if ((c2 & ~0x3F) == 0x80)
                  {
                     // all codes in range 0x80..0x7FF are acceptable
                  }
                  else
                  {
                     dprintf2("XmlCdata-CheckUtf8: sequence error in 2-byte code: 0x%02X,%02X\n", c1, c2);
                     result = FALSE;
                  }
                  break;

               case 0xE:  // 3-byte code: range 0x0800..0xFFFF
                  if ( (((c2 = *(pStr++)) & ~0x3Fu) == 0x80) &&
                       (((c3 = *(pStr++)) & ~0x3Fu) == 0x80) )
                  {
                     code = ((c1 & 0x0F) << (2*6)) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);

                     if ( ((code >= 0xD800) && (code < 0xE000)) || (code >= 0xFFFE) )
                     {
                        dprintf4("XmlCdata-CheckUtf8: invalid code: 0x%X (3-byte 0x%02X,%02X,%02X)\n", code, c1, c2, c3);
                        result = FALSE;
                     }
                  }
                  else
                  {
                     dprintf2("XmlCdata-CheckUtf8: sequence error in 3-byte code: 0x%02X,%02X,...\n", c1, c2);
                     result = FALSE;
                  }
                  break;

               case 0xF:  // 4 or more byte: code >= 0x10000
                  if ((c1 & 0xF8) == 0xF0)
                  {
                     if ( (((c2 = *(pStr++)) & ~0x3Fu) == 0x80) &&
                          (((c3 = *(pStr++)) & ~0x3Fu) == 0x80) &&
                          (((c4 = *(pStr++)) & ~0x3Fu) == 0x80) )
                     {
                        code = ((c1 & 0x07) << (3*6)) | ((c2 & 0x3F) << (2*6)) |
                               ((c3 & 0x3F) << 6) | (c4 & 0x3F);

                        if (code > 0x10FFFF)
                        {
                           dprintf5("XmlCdata-CheckUtf8: invalid code: 0x%X (4-byte 0x%02X,%02X,%02X,%02X)\n", code, c1, c2, c3, c4);
                           result = FALSE;
                        }
                     }
                     else
                     {
                        dprintf2("XmlCdata-CheckUtf8: sequence error in 4-byte code: 0x%02X,%02X,...\n", c1, c2);
                        result = FALSE;
                     }
                  }
                  else
                  {
                     dprintf1("XmlCdata-CheckUtf8: >= 5-byte code: invalid in XML: 0x%02X\n", c1);
                     result = FALSE;  // 5 or more bytes
                  }
                  break;

               default:  // sequence error
                  result = FALSE;
                  break;
            }
         }
      }
   }
   return result;
}

// ----------------------------------------------------------------------------
// Character code ranges of all letters in Unicode (acc. to XML 1.0 ed. 3, Annex B)
// - important note: the ranges must be sorted in increasing order because
//   a binary search algorithm is used on them
//
typedef struct
{
   uint low;
   uint high;
} XML_CDATA_UTF_RANGE;

// letters which can be encoded in 2-byte UTF-8
static const XML_CDATA_UTF_RANGE XmlCData_UtfLetterRange2[] =
{
   { 0x00C0, 0x00D6 }, { 0x00D8, 0x00F6 }, { 0x00F8, 0x00FF },
   { 0x0100, 0x0131 }, { 0x0134, 0x013E }, { 0x0141, 0x0148 }, { 0x014A, 0x017E }, { 0x0180, 0x01C3 },
   { 0x01CD, 0x01F0 }, { 0x01F4, 0x01F5 }, { 0x01FA, 0x0217 }, { 0x0250, 0x02A8 }, { 0x02BB, 0x02C1 },
   { 0x0386, 0x0386 }, { 0x0388, 0x038A }, { 0x038C, 0x038C }, { 0x038E, 0x03A1 }, { 0x03A3, 0x03CE },
   { 0x03D0, 0x03D6 }, { 0x03DA, 0x03DA }, { 0x03DC, 0x03DC }, { 0x03DE, 0x03DE }, { 0x03E0, 0x03E0 },
   { 0x03E2, 0x03F3 }, { 0x0401, 0x040C }, { 0x040E, 0x044F }, { 0x0451, 0x045C }, { 0x045E, 0x0481 },
   { 0x0490, 0x04C4 }, { 0x04C7, 0x04C8 }, { 0x04CB, 0x04CC }, { 0x04D0, 0x04EB }, { 0x04EE, 0x04F5 },
   { 0x04F8, 0x04F9 }, { 0x0531, 0x0556 }, { 0x0559, 0x0559 }, { 0x0561, 0x0586 }, { 0x05D0, 0x05EA },
   { 0x05F0, 0x05F2 }, { 0x0621, 0x063A }, { 0x0641, 0x064A }, { 0x0671, 0x06B7 }, { 0x06BA, 0x06BE },
   { 0x06C0, 0x06CE }, { 0x06D0, 0x06D3 }, { 0x06D5, 0x06D5 }, { 0x06E5, 0x06E6 },
};
#define XML_UTF_LETTER_RANGE2_COUNT  (sizeof(XmlCData_UtfLetterRange2)/sizeof(XmlCData_UtfLetterRange2[0]))
// letters which can be encoded in 3-byte UTF-8
static const XML_CDATA_UTF_RANGE XmlCData_UtfLetterRange3[] =
{
   { 0x0905, 0x0939 },
   { 0x093D, 0x093D }, { 0x0958, 0x0961 }, { 0x0985, 0x098C }, { 0x098F, 0x0990 }, { 0x0993, 0x09A8 },
   { 0x09AA, 0x09B0 }, { 0x09B2, 0x09B2 }, { 0x09B6, 0x09B9 }, { 0x09DC, 0x09DD }, { 0x09DF, 0x09E1 },
   { 0x09F0, 0x09F1 }, { 0x0A05, 0x0A0A }, { 0x0A0F, 0x0A10 }, { 0x0A13, 0x0A28 }, { 0x0A2A, 0x0A30 },
   { 0x0A32, 0x0A33 }, { 0x0A35, 0x0A36 }, { 0x0A38, 0x0A39 }, { 0x0A59, 0x0A5C }, { 0x0A5E, 0x0A5E },
   { 0x0A72, 0x0A74 }, { 0x0A85, 0x0A8B }, { 0x0A8D, 0x0A8D }, { 0x0A8F, 0x0A91 }, { 0x0A93, 0x0AA8 },
   { 0x0AAA, 0x0AB0 }, { 0x0AB2, 0x0AB3 }, { 0x0AB5, 0x0AB9 }, { 0x0ABD, 0x0ABD }, { 0x0AE0, 0x0AE0 },
   { 0x0B05, 0x0B0C }, { 0x0B0F, 0x0B10 }, { 0x0B13, 0x0B28 }, { 0x0B2A, 0x0B30 }, { 0x0B32, 0x0B33 },
   { 0x0B36, 0x0B39 }, { 0x0B3D, 0x0B3D }, { 0x0B5C, 0x0B5D }, { 0x0B5F, 0x0B61 }, { 0x0B85, 0x0B8A },
   { 0x0B8E, 0x0B90 }, { 0x0B92, 0x0B95 }, { 0x0B99, 0x0B9A }, { 0x0B9C, 0x0B9C }, { 0x0B9E, 0x0B9F },
   { 0x0BA3, 0x0BA4 }, { 0x0BA8, 0x0BAA }, { 0x0BAE, 0x0BB5 }, { 0x0BB7, 0x0BB9 }, { 0x0C05, 0x0C0C },
   { 0x0C0E, 0x0C10 }, { 0x0C12, 0x0C28 }, { 0x0C2A, 0x0C33 }, { 0x0C35, 0x0C39 }, { 0x0C60, 0x0C61 },
   { 0x0C85, 0x0C8C }, { 0x0C8E, 0x0C90 }, { 0x0C92, 0x0CA8 }, { 0x0CAA, 0x0CB3 }, { 0x0CB5, 0x0CB9 },
   { 0x0CDE, 0x0CDE }, { 0x0CE0, 0x0CE1 }, { 0x0D05, 0x0D0C }, { 0x0D0E, 0x0D10 }, { 0x0D12, 0x0D28 },
   { 0x0D2A, 0x0D39 }, { 0x0D60, 0x0D61 }, { 0x0E01, 0x0E2E }, { 0x0E30, 0x0E30 }, { 0x0E32, 0x0E33 },
   { 0x0E40, 0x0E45 }, { 0x0E81, 0x0E82 }, { 0x0E84, 0x0E84 }, { 0x0E87, 0x0E88 }, { 0x0E8A, 0x0E8A },
   { 0x0E8D, 0x0E8D }, { 0x0E94, 0x0E97 }, { 0x0E99, 0x0E9F }, { 0x0EA1, 0x0EA3 }, { 0x0EA5, 0x0EA5 },
   { 0x0EA7, 0x0EA7 }, { 0x0EAA, 0x0EAB }, { 0x0EAD, 0x0EAE }, { 0x0EB0, 0x0EB0 }, { 0x0EB2, 0x0EB3 },
   { 0x0EBD, 0x0EBD }, { 0x0EC0, 0x0EC4 }, { 0x0F40, 0x0F47 }, { 0x0F49, 0x0F69 }, { 0x10A0, 0x10C5 },
   { 0x10D0, 0x10F6 }, { 0x1100, 0x1100 }, { 0x1102, 0x1103 }, { 0x1105, 0x1107 }, { 0x1109, 0x1109 },
   { 0x110B, 0x110C }, { 0x110E, 0x1112 }, { 0x113C, 0x113C }, { 0x113E, 0x113E }, { 0x1140, 0x1140 },
   { 0x114C, 0x114C }, { 0x114E, 0x114E }, { 0x1150, 0x1150 }, { 0x1154, 0x1155 }, { 0x1159, 0x1159 },
   { 0x115F, 0x1161 }, { 0x1163, 0x1163 }, { 0x1165, 0x1165 }, { 0x1167, 0x1167 }, { 0x1169, 0x1169 },
   { 0x116D, 0x116E }, { 0x1172, 0x1173 }, { 0x1175, 0x1175 }, { 0x119E, 0x119E }, { 0x11A8, 0x11A8 },
   { 0x11AB, 0x11AB }, { 0x11AE, 0x11AF }, { 0x11B7, 0x11B8 }, { 0x11BA, 0x11BA }, { 0x11BC, 0x11C2 },
   { 0x11EB, 0x11EB }, { 0x11F0, 0x11F0 }, { 0x11F9, 0x11F9 }, { 0x1E00, 0x1E9B }, { 0x1EA0, 0x1EF9 },
   { 0x1F00, 0x1F15 }, { 0x1F18, 0x1F1D }, { 0x1F20, 0x1F45 }, { 0x1F48, 0x1F4D }, { 0x1F50, 0x1F57 },
   { 0x1F59, 0x1F59 }, { 0x1F5B, 0x1F5B }, { 0x1F5D, 0x1F5D }, { 0x1F5F, 0x1F7D }, { 0x1F80, 0x1FB4 },
   { 0x1FB6, 0x1FBC }, { 0x1FBE, 0x1FBE }, { 0x1FC2, 0x1FC4 }, { 0x1FC6, 0x1FCC }, { 0x1FD0, 0x1FD3 },
   { 0x1FD6, 0x1FDB }, { 0x1FE0, 0x1FEC }, { 0x1FF2, 0x1FF4 }, { 0x1FF6, 0x1FFC }, { 0x2126, 0x2126 },
   { 0x212A, 0x212B }, { 0x212E, 0x212E }, { 0x2180, 0x2182 }, { 0x3007, 0x3007 }, { 0x3021, 0x3029 },
   { 0x3041, 0x3094 }, { 0x30A1, 0x30FA }, { 0x3105, 0x312C }, { 0x4E00, 0x9FA5 }, { 0xAC00, 0xD7A3 }
};
#define XML_UTF_LETTER_RANGE3_COUNT  (sizeof(XmlCData_UtfLetterRange3)/sizeof(XmlCData_UtfLetterRange3[0]))
// digits, combining chars and extenders which can be encoded in 2-byte UTF-8
static const XML_CDATA_UTF_RANGE XmlCData_UtfDigiCombExtRange2[] =
{
   { 0x02D0, 0x02D0 }, { 0x02D1, 0x02D1 }, { 0x0300, 0x0345 }, { 0x0360, 0x0361 }, { 0x0387, 0x0387 },
   { 0x0483, 0x0486 }, { 0x0591, 0x05A1 }, { 0x05A3, 0x05B9 }, { 0x05BB, 0x05BD }, { 0x05BF, 0x05BF },
   { 0x05C1, 0x05C2 }, { 0x05C4, 0x05C4 }, { 0x0640, 0x0640 }, { 0x064B, 0x0652 }, { 0x0660, 0x0669 },
   { 0x0670, 0x0670 }, { 0x06D6, 0x06DC }, { 0x06DD, 0x06DF }, { 0x06E0, 0x06E4 }, { 0x06E7, 0x06E8 },
   { 0x06EA, 0x06ED }, { 0x06F0, 0x06F9 },
};
#define XML_UTF_DIGICOMBEXT_RANGE2_COUNT  (sizeof(XmlCData_UtfDigiCombExtRange2)/sizeof(XmlCData_UtfDigiCombExtRange2[0]))
// digits, combining chars and extenders which can be encoded in 3-byte UTF-8
static const XML_CDATA_UTF_RANGE XmlCData_UtfDigiCombExtRange3[] =
{
   { 0x0901, 0x0903 }, { 0x093C, 0x093C }, { 0x093E, 0x094C }, { 0x094D, 0x094D }, { 0x0951, 0x0954 },
   { 0x0962, 0x0963 }, { 0x0966, 0x096F }, { 0x0981, 0x0983 }, { 0x09BC, 0x09BC }, { 0x09BE, 0x09BE },
   { 0x09BF, 0x09BF }, { 0x09C0, 0x09C4 }, { 0x09C7, 0x09C8 }, { 0x09CB, 0x09CD }, { 0x09D7, 0x09D7 },
   { 0x09E2, 0x09E3 }, { 0x09E6, 0x09EF }, { 0x0A02, 0x0A02 }, { 0x0A3C, 0x0A3C }, { 0x0A3E, 0x0A3E },
   { 0x0A3F, 0x0A3F }, { 0x0A40, 0x0A42 }, { 0x0A47, 0x0A48 }, { 0x0A4B, 0x0A4D }, { 0x0A66, 0x0A6F },
   { 0x0A70, 0x0A71 }, { 0x0A81, 0x0A83 }, { 0x0ABC, 0x0ABC }, { 0x0ABE, 0x0AC5 }, { 0x0AC7, 0x0AC9 },
   { 0x0ACB, 0x0ACD }, { 0x0AE6, 0x0AEF }, { 0x0B01, 0x0B03 }, { 0x0B3C, 0x0B3C }, { 0x0B3E, 0x0B43 },
   { 0x0B47, 0x0B48 }, { 0x0B4B, 0x0B4D }, { 0x0B56, 0x0B57 }, { 0x0B66, 0x0B6F }, { 0x0B82, 0x0B83 },
   { 0x0BBE, 0x0BC2 }, { 0x0BC6, 0x0BC8 }, { 0x0BCA, 0x0BCD }, { 0x0BD7, 0x0BD7 }, { 0x0BE7, 0x0BEF },
   { 0x0C01, 0x0C03 }, { 0x0C3E, 0x0C44 }, { 0x0C46, 0x0C48 }, { 0x0C4A, 0x0C4D }, { 0x0C55, 0x0C56 },
   { 0x0C66, 0x0C6F }, { 0x0C82, 0x0C83 }, { 0x0CBE, 0x0CC4 }, { 0x0CC6, 0x0CC8 }, { 0x0CCA, 0x0CCD },
   { 0x0CD5, 0x0CD6 }, { 0x0CE6, 0x0CEF }, { 0x0D02, 0x0D03 }, { 0x0D3E, 0x0D43 }, { 0x0D46, 0x0D48 },
   { 0x0D4A, 0x0D4D }, { 0x0D57, 0x0D57 }, { 0x0D66, 0x0D6F }, { 0x0E31, 0x0E31 }, { 0x0E34, 0x0E3A },
   { 0x0E46, 0x0E46 }, { 0x0E47, 0x0E4E }, { 0x0E50, 0x0E59 }, { 0x0EB1, 0x0EB1 }, { 0x0EB4, 0x0EB9 },
   { 0x0EBB, 0x0EBC }, { 0x0EC6, 0x0EC6 }, { 0x0EC8, 0x0ECD }, { 0x0ED0, 0x0ED9 }, { 0x0F18, 0x0F19 },
   { 0x0F20, 0x0F29 }, { 0x0F35, 0x0F35 }, { 0x0F37, 0x0F37 }, { 0x0F39, 0x0F39 }, { 0x0F3E, 0x0F3E },
   { 0x0F3F, 0x0F3F }, { 0x0F71, 0x0F84 }, { 0x0F86, 0x0F8B }, { 0x0F90, 0x0F95 }, { 0x0F97, 0x0F97 },
   { 0x0F99, 0x0FAD }, { 0x0FB1, 0x0FB7 }, { 0x0FB9, 0x0FB9 }, { 0x20D0, 0x20DC }, { 0x20E1, 0x20E1 },
   { 0x3005, 0x3005 }, { 0x302A, 0x302F }, { 0x3031, 0x3035 }, { 0x3099, 0x3099 }, { 0x309A, 0x309A },
   { 0x309D, 0x309E }, { 0x30FC, 0x30FE },
};
#define XML_UTF_DIGICOMBEXT_RANGE3_COUNT  (sizeof(XmlCData_UtfDigiCombExtRange3)/sizeof(XmlCData_UtfDigiCombExtRange3[0]))

static Bool XmlCdata_SearchUtfCodeRange( uint code, const XML_CDATA_UTF_RANGE * pRange, uint count )
{
   uint  lowIdx  = 0;
   uint  highIdx = count - 1;
   uint  idx;
   Bool  result = FALSE;

   //assert(count > 0);

   while (1)
   {
      if (lowIdx == highIdx)
      {
         idx = lowIdx;
         result = ((code >= pRange[idx].low) && (code <= pRange[idx].high));
         break;
      }
      else
      {
         idx = (lowIdx + highIdx) / 2;
         if (code < pRange[idx].low)
         {
            highIdx = idx - 1;
         }
         else if (code > pRange[idx].high)
         {
            lowIdx = idx + 1;
         }
         else
         {
            result = TRUE;
            break;
         }
      }
   }

   return result;
}

// ----------------------------------------------------------------------------
// Check if all characters in a UTF-8 encoded string are letters
// - valid range for letters is defined by XML 1.0 3rd ed., Annex B
//
Bool XmlCdata_CheckUtf8Name( const char * pStr, Bool isNmtoken )
{
   uchar c1, c2, c3;
   uint code;
   Bool result = TRUE;

   if (pStr != NULL)
   {
      while ( ((c1 = *(pStr++)) != 0) && result )
      {
         if (c1 < 0x80)
         {
            if (c1 < 0x20)
            {
               if ((c1 != 0x09) && (c1 != 0x0A) && (c1 != 0x0D))
               {
                  dprintf1("XmlCdata-CheckUtf8Name: invalid control code 0x%02X\n", c1);
                  result = FALSE;
               }
            }
            // else: ASCII range already checked by scanner
         }
         else
         {
            switch (c1 >> 4)
            {
               case 0xC:  // 2-byte code
               case 0xD:
                  c2 = *pStr++;
                  if ((c2 & ~0x3Fu) == 0x80)
                  {
                     code = ((c1 & 0x1F) << 6) | (c2 & 0x3F);

                     if (  (XmlCdata_SearchUtfCodeRange(code, XmlCData_UtfLetterRange2, XML_UTF_LETTER_RANGE2_COUNT) == FALSE) &&
                          ((XmlCdata_SearchUtfCodeRange(code, XmlCData_UtfDigiCombExtRange2, XML_UTF_DIGICOMBEXT_RANGE2_COUNT) == FALSE) || (isNmtoken == FALSE)) )
                     {
                        dprintf3("XmlCdata-CheckUtf8Name: not a letter: 0x%X (2-byte code: 0x%02X,%02X)\n", code, c1, c2);
                        result = FALSE;
                     }
                  }
                  else
                  {
                     dprintf2("XmlCdata-CheckUtf8Name: sequence error in 2-byte code: 0x%02X,%02X\n", c1, c2);
                     result = FALSE;
                  }
                  break;

               case 0xE:  // 3-byte code
                  if ( (((c2 = *(pStr++)) & ~0x3Fu) == 0x80) &&
                       (((c3 = *(pStr++)) & ~0x3Fu) == 0x80) )
                  {
                     code = ((c1 & 0x0F) << (2*6)) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);

                     if (  (XmlCdata_SearchUtfCodeRange(code, XmlCData_UtfLetterRange3, XML_UTF_LETTER_RANGE3_COUNT) == FALSE) &&
                          ((XmlCdata_SearchUtfCodeRange(code, XmlCData_UtfDigiCombExtRange3, XML_UTF_DIGICOMBEXT_RANGE3_COUNT) == FALSE) || (isNmtoken == FALSE)) )
                     {
                        dprintf4("XmlCdata-CheckUtf8Name: not a letter: 0x%X (3-byte code: 0x%02X,%02X,%02X)\n", code, c1, c2, c3);
                        result = FALSE;
                     }
                  }
                  else
                  {
                     dprintf2("XmlCdata-CheckUtf8Name: sequence error in 3-byte code: 0x%02X,%02X,...\n", c1, c2);
                     result = FALSE;
                  }
                  break;

               case 0xF:  // 4 or more byte: code >= 0x10000
                  dprintf1("XmlCdata-CheckUtf8Name: 4-byte code: not a letter: 0x%02X\n", c1);
                  result = FALSE;
                  break;

               default:
                  dprintf1("XmlCdata-CheckUtf8Name: sequence error: unexpected trailing code 0x%02X\n", c1);
                  result = FALSE;
                  break;
            }
         }
         isNmtoken = TRUE;
      }
   }
   return result;
}

