/* $Id$ */

/** \file
 * OpenDML definition header
 * \author Nick Kochakian
 */

#ifndef __OPENDML_H
#define __OPENDML_H

typedef __int64 QUADWORD;

/* bIndexType codes */

/* when each entry in aIndex array points to an index chunk */
#define AVI_INDEX_OF_INDEXES 0x0

/* when each entry in aIndex array points to a chunk in the file */
#define AVI_INDEX_OF_CHUNKS 0x1

/* when each entry is aIndex is really the data bIndexSubtype codes for
   INDEX_OF_CHUNKS */
#define AVI_INDEX_IS_DATA 0x80

/* when fields within frames are also indexed */
#define AVI_INDEX_2FIELD 0x1

#pragma pack(push, 1)

/* Chunk: ix## */
typedef struct
{
    WORD     wLongsPerEntry; /* sizeof(aIndex[0]) / sizeof(DWORD) */
    BYTE     bIndexSubType;  /* 0 */
    BYTE     bIndexType;     /* AVI_INDEX_OF_CHUNKS */
    DWORD    nEntriesInUse;
    DWORD    dwChunkId;      /* ##db, ##wb, etc */
    QUADWORD qwBaseOffset;   /* Every offset in aIndex is offset by this */
    DWORD    dwReserved;
} AVISTDINDEX;

typedef struct
{
    DWORD dwOffset; /* Offset to the chunk's data */
    DWORD dwSize;   /* Bit 31 is set if this is NOT a key frame */
} AVISTDINDEX_ENTRY;

/* Chunk: ix## */
typedef struct
{
    WORD  wLongsPerEntry; /* 4 */
    BYTE  bIndexSubType;  /* 0 or AVI_INDEX_2FIELD */
    BYTE  bIndexType;     /* AVI_INDEX_OF_INDEXES */
    DWORD nEntriesInUse;
    DWORD dwChunkId;      /* ##db, ##wb, etc */
    DWORD dwReserved[3];
} AVISUPERINDEX;

typedef struct
{
    QUADWORD qwOffset;   /* Absolute file offset */
    DWORD    dwSize;     /* Size of the index chunk at this offset */
    DWORD    dwDuration; /* Time span in stream ticks */
} AVISUPERINDEX_ENTRY;

typedef struct
{
    DWORD dwTotalFrames;
} ODMLExtendedAVIHeader;

#pragma pack(pop)

#endif