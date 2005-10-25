/* $Id: avi.h,v 1.7 2005-10-25 08:17:59 adcockj Exp $ */

/** \file
 * Main AVI file header
 * \author Nick Kochakian
 */

#ifndef __AVI_H
#define __AVI_H

#ifdef __cplusplus
extern "C"
{
#endif

#define WINVER 0x0400
#define _WIN32_WINNT 0x0400

#include <windows.h>
#include <vfw.h>
#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include "opendml.h"

#pragma comment(lib, "vfw32.lib")
#pragma comment(lib, "msacm32.lib")
#pragma comment(lib, "winmm.lib")

#define RIFF_CHUNK mmioFOURCC('R', 'I', 'F', 'F')
#define LIST_CHUNK mmioFOURCC('L', 'I', 'S', 'T')
#define AVIH_CHUNK mmioFOURCC('a', 'v', 'i', 'h')
#define STRH_CHUNK mmioFOURCC('s', 't', 'r', 'h')
#define STRF_CHUNK mmioFOURCC('s', 't', 'r', 'f')
#define INDX_CHUNK mmioFOURCC('i', 'n', 'd', 'x')
#define IDX1_CHUNK mmioFOURCC('i', 'd', 'x', '1')
#define JUNK_CHUNK mmioFOURCC('J', 'U', 'N', 'K')

/** The maximum size of a RIFF chunk. DO NOT CHANGE. */
#define CHUNK_LIMIT 1027604480 /* About 1 giga byte */

/** The maximum amount of memory to allocate for the async data writer */
#define ASYNC_MEM_LIMIT (12 * 1024 * 1024) /* 12 mega bytes */

/** The size in mebibytes to limit a file to when files are limited to 4GiB.
 * This value should actually be a bit less than 4GiB.
 */
#define AVI_4GiB_LIMIT 3980

typedef __int64 int64;

/** Time data type
 * \warning This needs to be signed
 * \warning This needs to be at least 64 bits wide
 */

typedef int64 aviTime_t;

/* This gives the super index space to point to x groups of index chunks with
   a size of MAX_INDEX each. So, the total number of chunks that can be indexed
   are: SUPER_INDEX_ENTRIES * MAX_INDEX

   The defaults leave enough space for a total of 4,194,304 chunks.
   For the video data..
        At 30 frames/second, the video data will last for about 38 hours
        At 60 frames/second, the video data will last for about 19 hours

   It would have been better if OpenDML's super indexes could point to other
   super indexes, then the number of indexed chunks wouldn't have a fixed size
   and empty space wouldn't need to be preallocated.
*/
#define SUPER_INDEX_ENTRIES 16384

/* The main AVI_FILE flags */
#define AVI_FLAG_VIDEO       0x1  /**< A video stream is defined */
#define AVI_FLAG_AUDIO       0x2  /**< An audio stream is defined */
#define AVI_FLAG_LARGE_FILES 0x80 /**< Large files (> 4GiB) are supported */

#define MAX_CHUNK       8
#define MAX_SUPER_INDEX 64
#define MAX_STD_INDEX   256 /**< The number of standard index entries to store before writing */
#define MAX_WAVE_BUFFER 4   /**< The number of waveIn buffers to use */

typedef enum
{
    VIDEO_STREAM = 0,
    AUDIO_STREAM,
    NUM_STREAMS
} stream_t;

enum
{
    RIFF_BASE = 0,
    MOVI_BASE,
    AVIH_BASE, /**< Offset to the chunk data */
    DMLH_BASE, /**< Offset to the chunk data */
    MAX_BASE_OFFSET
};

/** Error types
 * \note Also update aviErrorLocations in avi.c if this is modified
 */
typedef enum
{
    AVI_ERROR_NONE = 0,     /**< No error */
    AVI_ERROR_BEGIN,        /**< Error while getting ready to write */
    AVI_ERROR_FILE_OPEN,    /**< File open error */
    AVI_ERROR_ASYNC,        /**< Async writing error */
    AVI_ERROR_FILE,         /**< File writing thread error */
    AVI_ERROR_FIFO,         /**< FIFO error */
    AVI_ERROR_AUDIO_OPEN,   /**< Audio initialization error */
    AVI_ERROR_VIDEO_OPEN,   /**< Video compression initialization error */
    AVI_ERROR_BUILD_INDEX,  /**< Error while building the legacy index */
    AVI_MAX_ERROR
} aviError_t;

/** Chunk data */
typedef struct
{
    FOURCC cc;    /**< The chunk's CC */
    DWORD  size;  /**< The size of the chunk's data in bytes */
    int64  begin; /**< The absolute offset in the file where the chunk begins */
} CHUNK;

/** Holds multiple OpenDML super index entries */
typedef struct
{
    int64         offset;  /**< Offset of the super index */
    AVISUPERINDEX header;  /**< Header of the super index */
    DWORD         current; /**< Current super index entry */

    /** Super index entries */
    AVISUPERINDEX_ENTRY entry[MAX_SUPER_INDEX];
} STREAM_SUPER_INDEX;

/** Holds multiple OpenDML standard index entries */
typedef struct
{
    DWORD             duration;             /**< Duration of the indices in stream ticks */
    DWORD             current;              /**< Current standard index entry */
    AVISTDINDEX_ENTRY entry[MAX_STD_INDEX]; /**< Standard index entries */
} STREAM_STD_INDEX;

/** Defines different CCs for a stream */
typedef struct
{
    FOURCC data;  /**< The CC used for the data chunks */
    FOURCC index; /**< The CC used for the index chunks */
} STREAM_CC;

/** Keeps track of average and maximum chunk sizes for each stream */
typedef struct
{
    int64 total;    /**< The sum of all the values entered */
    DWORD max;      /**< The maximum value that was entered */
    DWORD count;    /**< The number of values that were entered */
} STREAM_VALUES;

/** Video compressor data */
typedef struct
{
    HIC      hIC;         /**< Handle to a compressor. 0 if one's not open. */
    void     *format;     /**< Output format */
    BOOL     compressing; /**< TRUE if the compressor was started */
    COMPVARS vars;        /**< Compression variables */

    /* Compressor defaults */
    DWORD quality; /**< The compressor's default quality setting */
} VIDEO_COMP;

/** Main structure */
typedef struct
{
    BYTE flags; /**< Zero or more AVI_FLAG_* constants */

    /* These control the overall size of the file */
    BOOL  hold;  /**< Set to TRUE when no more audio or video data should be written */
    int64 limit; /**< The maximum size of the file in bytes. 0 mean there's no limit. */

    /** Error data */
    struct
    {
        CRITICAL_SECTION lock;
        aviError_t       type;          /**< The current type of error */
        char             message[128];  /**< A message associated with the error */
    } error;

    /** Async data */
    struct
    {
        /* This is only used by the async thread */
        HANDLE f;   /**< Handle to the file */

        /* This is shared between threads */
        void *fifo; /**< A FIFO buffer that contains the data to write */

        /* Events */
        HANDLE hWriteEvent; /**< This event is signaled after a write completes */
        HANDLE hThreadKick; /**< The async thread waits on this event when there's nothing else to do */

        /* This is protected by the external file lock */
        int64 filePos; /**< The current virtual file position */

        HANDLE hThread; /**< Handle to the async writing thread */
        HANDLE hExit;   /**< Signaled when the thread should exit */
    } async;

    /** Locks */
    struct
    {
        CRITICAL_SECTION file;  /**< Lock for the file/chunk data */
        CRITICAL_SECTION timer; /**< Lock for the timer data */
        CRITICAL_SECTION video; /**< Lock for the video data */
        CRITICAL_SECTION audio; /**< Lock for the audio data */
    } lock;

    /** Chunk data */
    struct
    {
        /** Base offsets to certain chunks */
        int64 baseOffset[MAX_BASE_OFFSET];

        CHUNK data[MAX_CHUNK]; /**< Data for active chunks */
        DWORD current;         /**< The number of active chunks */
    } chunk;

    /** Information about the timer */
    struct AVI_FILE_TIMER
    {
        aviTime_t freq;    /**< The timer's frequency */
        BOOL      flag;    /**< Set to TRUE if the timer has started to be used */
        aviTime_t start;   /**< The time that recording started */
        aviTime_t elapsed; /**< The amount of time that has elapsed in ticks */
    } timer;

    /** Video stream */
    struct
    {
        /** Video timing data */
        struct AVI_FILE_VIDEO_TIMER
        {
            int64  step;   /**< The time in between frames */
            int    frames; /**< Counts frames for each second */
            double expectedFrames;

            /* These are useful for debugging */
            int extraFrames; /**< The number of extra frames counted for the current second */
            int lostFrames;  /**< The number of lost frames counted for the current second */
        } timer;

        /** Keeps track of the number of lost frames to make up */
        struct
        {
            int frames;   /**< The total number of missing frames */
            int step;     /**< The number of frames to duplicate */
        } missing;

        STREAM_CC        cc;
        DWORD            strhOffset;   /**< Offset to the beginning of the stream header structure */
        BITMAPINFOHEADER info;         /**< Image input format */
        DWORD            rate;         /**< Rate of the video */
        DWORD            scale;        /**< Used in the equation FPS = rate / scale */
        FOURCC           fccHandler;   /**< The CC of the compressor */
        VIDEO_COMP       comp;         /**< Compression data */
        DWORD            numFrames;    /**< The number of frames that were saved */
        float            fps;          /**< The precalculated value of FPS for the video data */
        STREAM_VALUES    values;
    } video;

    /** Audio stream */
    struct
    {
        STREAM_CC    cc;
        WAVEFORMATEX wfxIn;        /**< Recording format */
        WAVEFORMATEX wfxOut;       /**< The format that's written in the AVI file */
        DWORD        strhOffset;   /**< Offset to the beginning of the stream header structure */
        HWAVEIN      hWaveIn;      /**< Handle to a waveIn device */
        BOOL         recording;    /**< Set to TRUE when audio is being recorded */
        UINT_PTR     deviceID;     /**< The ID of the waveIn device to use */

        /* These are protected by the audio lock */
        DWORD         streamLength; /**< The length of the audio stream */
        STREAM_VALUES values;

        /* Make sure the number of buffers doesn't go below four. It's a nice
           thing to have especially if the system becomes heavily loaded. If
           there's a buffer underrun, then it will completely throw the
           synchronization off. */

        WAVEHDR whdr[MAX_WAVE_BUFFER];    /**< Wave headers */
        void    *buffer[MAX_WAVE_BUFFER]; /**< Buffers used with the wave headers */
    } audio;

    /** Information about the first RIFF chunk */
    struct
    {
        int64 indexOffset;          /**< Offset to the legacy index chunk. 0 is invalid. */
        DWORD frames;               /**< The number of frames in the chunk */
        DWORD indices[NUM_STREAMS]; /**< The number of indices in the chunk */
    } legacy;

    /* Index data for both streams */
    DWORD              stdIndexCount[NUM_STREAMS]; /**< The number of standard indices written for the current chunk */
    STREAM_STD_INDEX   stdIndex[NUM_STREAMS];      /**< Standard index data for all streams */
    STREAM_SUPER_INDEX superIndex[NUM_STREAMS];    /**< Super index data for all streams */
} AVI_FILE;

/* From avi.c */
void     aviDisplayError(AVI_FILE *file, HWND hWnd, char *post);
AVI_FILE *aviFileCreate(void);
BOOL     aviBeginWriting(AVI_FILE *file, char *fileName);
void     aviEndWriting(AVI_FILE *file);
void     aviFileDestroy(AVI_FILE *file);
BOOL     aviFrameReady(AVI_FILE *file);
BOOL     aviLimitReached(AVI_FILE *file);
void     aviSetLimit(AVI_FILE *file, DWORD limitMiB);
BOOL     aviSaveFrame(AVI_FILE *file, void *src, aviTime_t inTime);

extern __inline aviTime_t aviGetTimer(void);

/* From avi_video.c */
void aviVideoDefineStream(AVI_FILE *file, DWORD rate, DWORD scale, DWORD w,
                          DWORD h);
void aviVideoSetHandler(AVI_FILE *file, FOURCC fccHandler, FOURCC inFormat,
                        WORD bitCount);

/* From avi_audio.c */
void aviAudioDefineStream(AVI_FILE *file, UINT_PTR deviceID,
                          WAVEFORMATEX *wfxIn, WAVEFORMATEX *wfxOut);

#ifdef __cplusplus
}
#endif

#endif