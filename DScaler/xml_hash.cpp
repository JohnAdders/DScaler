/*
 *  Helper module implementing a hash array for strings
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
 *    This helper module implements a hash array which is indexed by
 *    strings.  It associates a few bytes of payload data with each
 *    tstring which can be directly used to store data, or to store a
 *    pointer to a dynamically allocated buffer.  In the latter case
 *    a callback function must be provided during destruction of the
 *    array to free the buffers.
 *
 *    In the XMLTV parser this module is used to cache theme category
 *    mapping results and to store entity definitions.  The advantages
 *    are the fast lookup and that the array grows dynamically.
 *
 *    Note currently only the bucket array grows dynamically, however
 *    the map size is constant.  The statistics function should be used
 *    to provide a good value for the map size. Otherwise performance
 *    might suffer from a large number of key collisions (i.e. when
 *    multiple strings are mapped to the same list entry by the key
 *    function.)
 *
 *
 *  Author: Tom Zoerner
 *
 *  Principles derived from tcl 8.4.9 (tclHash.c,v 1.12.2.1 2004/11/11):
 *    Copyright (c) 1991-1993 The Regents of the University of California.
 *    Copyright (c) 1994 Sun Microsystems, Inc.
 *
 *  $Id$
 */

#define DEBUG_SWITCH DEBUG_SWITCH_XMLTV
#define DPRINTF_OFF

#include "stdafx.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "xmltv_types.h"
#include "xmltv_debug.h"

#include "xml_hash.h"


// ----------------------------------------------------------------------------
// Internal structures
//
#define HASH_MAP_SIZE       512
#define HASH_PAYLOAD_SIZE   8
#define HASH_MAP_FREE      -1

typedef struct HASH_CHAIN_struct
{
   sint         nextIdx;                        // next tstring with the same key, or -1
   TCHAR      * pStr;                           // pointer to malloc'ed copy of the tstring
   TCHAR        payload[HASH_PAYLOAD_SIZE];     // data associated with the tstring
} HASH_BUCKET;

typedef struct
{
   HASH_BUCKET   * pBuckets;
   uint            bucketUsed;
   uint            bucketCount;
   sint            map[HASH_MAP_SIZE];
} XML_HASH_STATE;


// ----------------------------------------------------------------------------
// Print statistics
//
#ifndef DPRINTF_OFF
static void XmlHash_Statistics( XML_HASH_STATE * pHash )
{
   uint  maxChainLen;
   uint  chainLen;
   uint  mapIdx;
   sint  walkIdx;
   uint  usedMapEntry;

   usedMapEntry = 0;
   maxChainLen = 0;
   for (mapIdx = 0; mapIdx < HASH_MAP_SIZE; mapIdx++)
   {
      usedMapEntry += 1;
      if (pHash->map[mapIdx] != HASH_MAP_FREE)
      {
         chainLen = 0;
         walkIdx = pHash->map[mapIdx];
         do
         {
            chainLen += 1;
            walkIdx = pHash->pBuckets[walkIdx].nextIdx;
         }
         while (walkIdx != HASH_MAP_FREE);

         if (chainLen > maxChainLen)
            maxChainLen = chainLen;
      }
   }

   _tprintf(_T("Hash: used buckets: %d of %d\n"), pHash->bucketUsed, pHash->bucketCount);
   _tprintf(_T("Hash: map entris: %d; max chain len: %d\n"), usedMapEntry, maxChainLen);
}
#endif

// ----------------------------------------------------------------------------
// Generate hash key for a tstring
//
static uint XmlHash_GetKey( const TCHAR*  pStr )
{
   uint  key;
   TCHAR c;

   key = 0;
   while ( (c = *(pStr++)) != 0 )
   {
      key += (key << 3) ^ c;
   }
   return key;
}

// ----------------------------------------------------------------------------
// Allocate a bucket from the list
// - automatically grow the list if no free buckets are available
//
static sint XmlHash_GetFreeBucket( XML_HASH_PTR pHashRef )
{
   XML_HASH_STATE * pHash = (XML_HASH_STATE*) pHashRef;
   HASH_BUCKET * pTmp;

   if (pHash->bucketUsed >= pHash->bucketCount)
   {
      dprintf1(_T("XmlHash-GetFreeBucket: grow bucket list to %d entries\n"), pHash->bucketCount * 2);
      assert(pHash->bucketUsed == pHash->bucketCount);

      // all buckets are used -> allocate larger list
      pTmp = (HASH_BUCKET*)xmalloc(pHash->bucketCount * 2 * sizeof(HASH_BUCKET));
      memcpy(pTmp, pHash->pBuckets, pHash->bucketCount * sizeof(HASH_BUCKET));

      xfree(pHash->pBuckets);
      pHash->pBuckets = pTmp;
      pHash->bucketCount *= 2;
   }
   // allocate the first free bucket
   pHash->bucketUsed += 1;

   return pHash->bucketUsed - 1;
}

// ----------------------------------------------------------------------------
// Looks up the entry for a given tstring
// - returns NULL if the tstring is not found
//
XML_HASH_PAYLOAD XmlHash_SearchEntry( XML_HASH_PTR pHashRef, const TCHAR*  pStr )
{
   XML_HASH_STATE * pHash = (XML_HASH_STATE*)pHashRef;
   sint  walkIdx;
   sint  prevIdx;
   uint  key;

   key = XmlHash_GetKey(pStr) & (HASH_MAP_SIZE - 1);
   prevIdx = HASH_MAP_FREE;
   walkIdx = pHash->map[key];

   if (walkIdx != HASH_MAP_FREE)
   {
      while (walkIdx != HASH_MAP_FREE)
      {
         if (_tcscmp(pHash->pBuckets[walkIdx].pStr, pStr) == 0)
         {
            break;
         }
         prevIdx = walkIdx;
         walkIdx = pHash->pBuckets[walkIdx].nextIdx;
      }
   }

   if (walkIdx != HASH_MAP_FREE)
      return (XML_HASH_PAYLOAD) (&pHash->pBuckets[walkIdx].payload);
   else
      return NULL;
}

// ----------------------------------------------------------------------------
// Add a tstring to the hash
//
XML_HASH_PAYLOAD XmlHash_CreateEntry( XML_HASH_PTR pHashRef, const TCHAR*  pStr, Bool * pIsNew )
{
   XML_HASH_STATE * pHash = (XML_HASH_STATE*)pHashRef;
   sint  walkIdx;
   sint  prevIdx;
   uint  key;

   key = XmlHash_GetKey(pStr) & (HASH_MAP_SIZE - 1);
   prevIdx = HASH_MAP_FREE;
   walkIdx = pHash->map[key];

   if (walkIdx != HASH_MAP_FREE)
   {
      while (walkIdx != HASH_MAP_FREE)
      {
         if (_tcscmp(pHash->pBuckets[walkIdx].pStr, pStr) == 0)
         {
            break;
         }
         prevIdx = walkIdx;
         walkIdx = pHash->pBuckets[walkIdx].nextIdx;
      }
   }

   if (walkIdx == HASH_MAP_FREE)
   {
      // tstring not found in hash
      *pIsNew = TRUE;

      walkIdx = XmlHash_GetFreeBucket(pHash);
      pHash->pBuckets[walkIdx].pStr = xstrdup(pStr);
      pHash->pBuckets[walkIdx].nextIdx = HASH_MAP_FREE;

      if (prevIdx == HASH_MAP_FREE)
      {  // allocate new key
         dprintf3(_T("XmlHash-CreateEntry: allocate map entry #%d: bucket #%d for '%s'\n"), key, walkIdx, pStr);
         pHash->map[key] = walkIdx;
      }
      else
      {  // key already known -> append to existing bucket chain
         dprintf3(_T("XmlHash-CreateEntry: append tp map entry #%d: bucket #%d for '%s'\n"), key, walkIdx, pStr);
         pHash->pBuckets[prevIdx].nextIdx = walkIdx;
      }
   }
   else
   {  // already in hash: just return pointer to the entry
      *pIsNew = FALSE;
   }

   return (XML_HASH_PAYLOAD) (&pHash->pBuckets[walkIdx].payload);
}

// ----------------------------------------------------------------------------
// Free resources in the hash
//
void XmlHash_Destroy( XML_HASH_PTR pHashRef, XML_HASH_FREE_CB pCb )
{
   XML_HASH_STATE * pHash = (XML_HASH_STATE*)pHashRef;
   HASH_BUCKET * pWalk;
   uint  idx;

   if (pHash != NULL)
   {
#ifndef DPRINTF_OFF
      XmlHash_Statistics(pHash);
#endif
      // free all strings referenced by buckets
      pWalk = pHash->pBuckets;
      for (idx = 0; idx < pHash->bucketUsed; idx++, pWalk++)
      {
         if (pCb != NULL)
         {
            pCb(pHash, pWalk->payload);
         }
         xfree(pWalk->pStr);
      }

      // free bucket array
      xfree(pHash->pBuckets);

      // free main struct
      xfree(pHash);
   }
   else
      fatal0(_T("XmlHash-Destroy: invalid NULL ptr param"));
}

// ----------------------------------------------------------------------------
// Initialize hash array
//
XML_HASH_PTR XmlHash_Init( void )
{
   XML_HASH_STATE * pHash;

   pHash = (XML_HASH_STATE*)xmalloc(sizeof(*pHash));
   memset(pHash, 0, sizeof(*pHash));

   pHash->bucketCount = 128;
   pHash->pBuckets = (HASH_BUCKET*)xmalloc(pHash->bucketCount * sizeof(HASH_BUCKET));

   memset(pHash->map, HASH_MAP_FREE, sizeof(pHash->map));

   return (XML_HASH_PTR) pHash;
}

