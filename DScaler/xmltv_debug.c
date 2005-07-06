/*
 *  Debug service module
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
 *    This is based on the file debug.c provided by Tom Zoerner in his
 *    XMLTV parser demo package. Only the minimum necessary to compile
 *    other files of the package has been kept.
 *
 *  $Id: xmltv_debug.c,v 1.1 2005-07-06 19:42:39 laurentg Exp $
 */


#include <windows.h>

#include "xmltv_types.h"
#include "xmltv_debug.h"


// ---------------------------------------------------------------------------
// Wrapper for malloc to check for error return
//
void * xmalloc( size_t size )
{
   void * ptr;

   assert(size > 0);

   ptr = malloc(size);
   if (ptr == NULL)
   {
      MessageBox(NULL, "Memory allocation failure - Terminating", "DScaler", MB_ICONSTOP | MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
      // force an exception that will be caught and shut down the process properly
      *(uchar *)ptr = 0;
   }
   return ptr;
}

// ---------------------------------------------------------------------------
// Wrapper for realloc to check for error return
//
void * xrealloc( void * ptr, size_t size )
{
   assert(size > 0);

   ptr = realloc(ptr, size);
   if (ptr == NULL)
   {
      MessageBox(NULL, "Memory allocation failure - Terminating", "DScaler", MB_ICONSTOP | MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
      // force an exception that will be caught and shut down the process properly
      *(uchar *)ptr = 0;
   }
   return ptr;
}

