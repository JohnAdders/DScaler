/* $Id$ */

/** \file
 * Main AVI file interface functions
 * \author Nick Kochakian
 */

#include "stdafx.h"

#pragma

#include <io.h>
#include <conio.h>
#include "avi.h"
#include "avi_internal.h"

/************************************************
 *              Error functions                 *
 ************************************************/

const char *aviErrorLocations[AVI_MAX_ERROR] =
{
    /* AVI_ERROR_NONE */
    "in an unknown location",

    /* AVI_ERROR_BEGIN */
    "getting ready to write",

    /* AVI_ERROR_FILE_OPEN */
    "opening the file",

    /* AVI_ERROR_ASYNC */
    "writing data to the buffer",

    /* AVI_ERROR_FILE */
    "in the data writing thread",

    /* AVI_ERROR_FIFO */
    "reading from the FIFO buffer",

    /* AVI_ERROR_AUDIO_OPEN */
    "initializing the waveIn device",

    /* AVI_ERROR_VIDEO_OPEN */
    "initializing the video compressor",

    /* AVI_ERROR_BUILD_INDEX */
    "building the legacy index"
};

/** Sets a new error for a file
 * \param file    The file to set the error for
 * \param error   An AVI_ERROR_* constant
 * \param message The message associated with the error
 */

void aviSetError(AVI_FILE *file, aviError_t type, char *message)
{
    if (file && message)
    {
        EnterCriticalSection(&file->error.lock);
        file->error.type = type;
        strncpy(file->error.message, message, sizeof(file->error.message));
        LeaveCriticalSection(&file->error.lock);
    }
}

/** Determines if an error has been set
 * \param file The file to check for errors
 * \return TRUE if an error has been set for \a file or FALSE if no errors have
 *         been set for \a file
 */

BOOL aviHasError(AVI_FILE *file)
{
    if (!file)
       return TRUE;

    return file->error.type==AVI_ERROR_NONE ? FALSE : TRUE;
}

/** Displays the currently set error
 * \param file The file that the error should be displayed for
 * \param hWnd The window to use as the parent of the message box. Can be NULL.
 * \param post Text to append after the error message. Can be NULL.
 */

void aviDisplayError(AVI_FILE *file, HWND hWnd, char *post)
{
    char       *message;
    const char *pre   = "An error occurred while ";
    const char *title = "Recording Error";
    const char *location;
    const char *locationPost = ":\n";
    int        preLength;
    int        postLength;
    int        length;
    int        offset;

    if (file)
    {
        preLength = strlen(pre);

        if (post)
           postLength = strlen(post) + 1; /* Include a new line before the message */
           else
           postLength = 0;

        EnterCriticalSection(&file->error.lock);

        /* Get the string that describes where the error occurred */
        if (file->error.type >= 0 && file->error.type < AVI_MAX_ERROR)
           location = aviErrorLocations[file->error.type];
           else
           location = aviErrorLocations[AVI_ERROR_NONE];

        length = preLength + strlen(location) + strlen(locationPost) +
                 strlen(file->error.message) + postLength + 1;

        message = (char *)malloc(length);
        if (message)
        {
            offset = 0;

            memcpy(message, pre, preLength);
            offset += preLength;

            memcpy(&message[offset], location, strlen(location));
            offset += strlen(location);

            memcpy(&message[offset], locationPost, strlen(locationPost));
            offset += strlen(locationPost);

            memcpy(&message[offset], file->error.message,
                   strlen(file->error.message));
            offset += strlen(file->error.message);

            if (post)
            {
                message[offset] = '\n';
                offset++;

                memcpy(&message[offset], post, postLength);
                offset += postLength;
            }

            message[offset] = 0;

            MessageBox(hWnd, message, title, MB_OK | MB_ICONERROR);

            free(message);
        } else
          MessageBox(hWnd, file->error.message, title, MB_OK | MB_ICONERROR);

        LeaveCriticalSection(&file->error.lock);
    } else
      MessageBox(hWnd, "Unknown error", title, MB_OK | MB_ICONERROR);
}

/************************************************
 *            Interface functions               *
 ************************************************/

/** Gets the CC data for a stream
 * \param file The file containing the stream
 * \param type A *_STREAM constant
 * \return A pointer to the associated STREAM_CC structure
 */

STREAM_CC *aviGetStreamCC(AVI_FILE *file, stream_t type)
{
    assert(type==VIDEO_STREAM || type==AUDIO_STREAM);

    return type==VIDEO_STREAM ? &file->video.cc : &file->audio.cc;
}

/** Creates a new AVI file structure
 * \return A pointer to a new AVI_FILE structure or NULL if there was not
 *         enough memory
 */

AVI_FILE *aviFileCreate(void)
{
    AVI_FILE *file;
    int      i;
    HMODULE  hLibrary;

    file = (AVI_FILE *)malloc(sizeof(AVI_FILE));
    if (!file)
       return NULL;

    memset(file, 0, sizeof(AVI_FILE));

    /* Initialize handles */
    file->async.f       = INVALID_HANDLE_VALUE;
    file->audio.hWaveIn = (HWAVEIN)INVALID_HANDLE_VALUE;

    /* Initialize the super index headers */
    for (i = 0; i < NUM_STREAMS; i++)
    {
        file->superIndex[i].header.wLongsPerEntry = 4;
        file->superIndex[i].header.bIndexType     = AVI_INDEX_OF_INDEXES;
    }

    /* Initialize the error data */
    InitializeCriticalSection(&file->error.lock);
    file->error.type = AVI_ERROR_NONE;

    /* Check for large file support */
    hLibrary = GetModuleHandle("kernel32.dll");
    if (hLibrary)
    {
        if (GetProcAddress(hLibrary, "SetFilePointerEx"))
           file->flags |= AVI_FLAG_LARGE_FILES;
    }

    /* If large file support isn't available then set the default file size
       limit to 4GiB */
    if (!(file->flags & AVI_FLAG_LARGE_FILES))
       aviSetLimit(file, AVI_4GiB_LIMIT);

    return file;
}

/**
 * Sets a type to have an absolute base offset equal to the current position of
 * the file pointer
 * \param type A *_BASE constant
 */

__inline void aviSetBaseOffset(AVI_FILE *file, int type)
{
    assert(type >= 0 && type < MAX_BASE_OFFSET);
    file->chunk.baseOffset[type] = fileTell(file);
}

/** Gets a base offset for a type that was set with aviSetBaseOffset
 * \param type A *_BASE constant
 * \return An absolute offset in the file
 */

__inline int64 aviGetBaseOffset(AVI_FILE *file, int type)
{
    assert(type >= 0 && type < MAX_BASE_OFFSET);
    return file->chunk.baseOffset[type];
}

/** Sets up compressors, opens a file for writing
 * \param file     A pointer to an AVI_FILE structure
 * \param fileName The name of the file to open for writing
 * \warning Only a single thread should call this function
 * \retval TRUE  If writing was started
 * \retval FALSE If writing could not start because of an error
 */

BOOL aviBeginWriting(AVI_FILE *file, char *fileName)
{
    MainAVIHeader         avih;
    AVIStreamHeader       strh;
    BITMAPINFOHEADER      *biOut;
    ODMLExtendedAVIHeader odmlAVIh;

    #define AVI_BEGIN_WRITING_ABORT()\
    {\
        aviEndWriting(file);\
        return FALSE;\
    }

    if (!file || !(file->flags & AVI_FLAG_VIDEO))
       return FALSE;

    #ifdef AVI_DEBUG
    AllocConsole();
    cprintf("----------------------\n");
    #endif

    /* Just to be safe */
    aviEndWriting(file);

    /**** Variable initialization ****/

    /* Clear any errors */
    aviSetError(file, AVI_ERROR_NONE, "No error");

    /* Make sure audio and video data can be written */
    file->hold = FALSE;

    /* Reset the legacy data */
    memset(&file->legacy, 0, sizeof(file->legacy));

    /* Clear the index counters */
    aviIndexClearCounters(file);

    /* Set up the timer stuff */
    memset(&file->timer, 0, sizeof(struct AVI_FILE::AVI_FILE_TIMER));
    memset(&file->video.timer, 0, sizeof(struct AVI_FILE::VIDEO_STREAM::AVI_FILE_VIDEO_TIMER));

    /* Clear the stream values */
    memset(&file->video.values, 0, sizeof(file->video.values));
    memset(&file->audio.values, 0, sizeof(file->audio.values));

    file->timer.freq = aviGetTimerFreq();
    if (!file->timer.freq)
    {
        aviSetError(file, AVI_ERROR_BEGIN, "Couldn't get the timer's "
                                           "frequency");
        AVI_BEGIN_WRITING_ABORT();
    }

    file->video.timer.step = (aviTime_t)((double)file->timer.freq / ((double)file->video.rate / (double)file->video.scale));

    if (!aviVideoCompressorOpen(file))
    {
        #ifdef AVI_DEBUG
        cprintf("Couldn't open the video compressor\n");
        #endif

        AVI_BEGIN_WRITING_ABORT();
    }

    if ((file->flags & AVI_FLAG_AUDIO) && !aviAudioBegin(file))
    {
        #ifdef AVI_DEBUG
        cprintf("Audio initialization failed\n");
        #endif

        AVI_BEGIN_WRITING_ABORT();
    }

    if (!fileOpen(file, fileName))
       AVI_BEGIN_WRITING_ABORT();

    /* Initialize critical sections */
    InitializeCriticalSection(&file->lock.file);
    InitializeCriticalSection(&file->lock.timer);
    InitializeCriticalSection(&file->lock.video);
    InitializeCriticalSection(&file->lock.audio);

    /**** Begin: First RIFF chunk ****/
    aviSetBaseOffset(file, RIFF_BASE);
    aviBeginChunk(file, RIFF_CHUNK);
    aviWriteFourCC(file, mmioFOURCC('A', 'V', 'I', ' '));

    /**** Begin: LIST avih chunk ****/
    aviBeginChunk(file, LIST_CHUNK);
    aviWriteFourCC(file, listtypeAVIHEADER);

    /* AVI header */
    aviBeginChunk(file, AVIH_CHUNK);
      memset(&avih, 0, sizeof(MainAVIHeader));

      aviSetBaseOffset(file, AVIH_BASE);

      /* The video stream is required */
      avih.dwStreams = 1;

      /* Check to see if the audio stream is going to be used */
      if (file->flags & AVI_FLAG_AUDIO)
         avih.dwStreams++;

      avih.dwFlags = AVIF_HASINDEX | AVIF_MUSTUSEINDEX;

      /* FPS = rate / scale */
      avih.dwMicroSecPerFrame = (DWORD)(1000000.0f / ((float)file->video.rate /
                                                      (float)file->video.scale));
      avih.dwWidth            = file->video.info.biWidth;
      avih.dwHeight           = file->video.info.biHeight;

      fileWrite(file, &avih, sizeof(MainAVIHeader));
    aviEndChunk(file);

    /* Get the output format */
    biOut = aviVideoGetOutputFormat(file);
    assert(biOut);

    /**** Begin: Video stream data ****/
    aviBeginChunk(file, LIST_CHUNK);
      aviWriteFourCC(file, mmioFOURCC('s', 't', 'r', 'l'));

      /* Stream header */
      aviBeginChunk(file, STRH_CHUNK);
        memset(&strh, 0, sizeof(AVIStreamHeader));

        file->video.strhOffset = (DWORD)fileTell(file);

        strh.fccType    = streamtypeVIDEO;
        strh.fccHandler = file->video.fccHandler;
        strh.dwRate     = file->video.rate;
        strh.dwScale    = file->video.scale;

        fileWrite(file, &strh, sizeof(AVIStreamHeader));
      aviEndChunk(file);

      /* Stream format */
      aviBeginChunk(file, STRF_CHUNK);
        fileWrite(file, biOut, biOut->biSize);
      aviEndChunk(file);

      /* Initial super index */
      aviBeginChunk(file, INDX_CHUNK);
        aviWriteInitialSuperIndex(file, VIDEO_STREAM);
      aviEndChunk(file);

    /**** End: Video stream data ****/
    aviEndChunk(file);

    if (file->flags & AVI_FLAG_AUDIO)
    {
        /**** Begin: Audio stream data ****/
        aviBeginChunk(file, LIST_CHUNK);
          aviWriteFourCC(file, mmioFOURCC('s', 't', 'r', 'l'));

          /* Stream header */
          aviBeginChunk(file, STRH_CHUNK);
            memset(&strh, 0, sizeof(AVIStreamHeader));

            file->audio.strhOffset = (DWORD)fileTell(file);

            strh.fccType      = streamtypeAUDIO;
            strh.dwRate       = file->audio.wfxOut.nSamplesPerSec;
            strh.dwScale      = 1;
            strh.dwSampleSize = file->audio.wfxOut.nBlockAlign;

            fileWrite(file, &strh, sizeof(AVIStreamHeader));
          aviEndChunk(file);

          /* Stream format */
          aviBeginChunk(file, STRF_CHUNK);
            fileWrite(file, &file->audio.wfxOut, sizeof(WAVEFORMATEX));
          aviEndChunk(file);

          /* Initial super index */
          aviBeginChunk(file, INDX_CHUNK);
            aviWriteInitialSuperIndex(file, AUDIO_STREAM);
          aviEndChunk(file);

        /**** End: Audio stream data ****/
        aviEndChunk(file);
    }

    /**** Begin: OpenDML chunk ****/
    aviBeginChunk(file, LIST_CHUNK);
    aviWriteFourCC(file, mmioFOURCC('o', 'd', 'm', 'l'));

      /* Extended AVI header */
      aviBeginChunk(file, mmioFOURCC('d', 'm', 'l', 'h'));
        memset(&odmlAVIh, 0, sizeof(ODMLExtendedAVIHeader));

        aviSetBaseOffset(file, DMLH_BASE);
        fileWrite(file, &odmlAVIh, sizeof(ODMLExtendedAVIHeader));
      aviEndChunk(file);

    /**** End: OpenDML chunk ****/
    aviEndChunk(file);

    /**** End: LIST avih chunk ****/
    aviEndChunk(file);

    /* Start the LIST movi chunk */
    aviSetBaseOffset(file, MOVI_BASE);
    aviBeginChunk(file, LIST_CHUNK);
    aviWriteFourCC(file, mmioFOURCC('m', 'o', 'v', 'i'));

    /* Reset the frame counter */
    file->video.numFrames = 0;

    /* Reset the missing frame data */
    file->video.missing.frames = 0;
    file->video.missing.step   = 0;

    if (file->flags & AVI_FLAG_AUDIO)
       aviAudioStartRecording(file);

    return TRUE;
}

/** Finishes up writing, closes compressors, etc
 * \param file A pointer to a previously opened file
 * \warning Only a single thread should call this function. No other threads
 *          should be using \a file when this function is called.
 */

void aviEndWriting(AVI_FILE *file)
{
    DWORD largestChunk;

    if (file)
    {
        aviVideoCompressorClose(file);

        if (file->flags & AVI_FLAG_AUDIO)
           aviAudioEnd(file);

        if (fileIsOpen(file))
        {
            aviLockFile(file);

            aviIndexFlush(file);
            aviEndAllChunks(file);

            /* Update dwTotalFrames in the AVI header */
            fileSeek(file, aviGetBaseOffset(file, AVIH_BASE) +
                           offsetof(MainAVIHeader, dwTotalFrames));

            fileWrite(file, &file->legacy.frames, sizeof(DWORD));

            /* Update dwSuggestedBufferSize in the AVI header */
            fileSeek(file, aviGetBaseOffset(file, AVIH_BASE) +
                           offsetof(MainAVIHeader, dwSuggestedBufferSize));

            /* The suggested buffer size should be the size of the largest
               chunk in any of the streams written to the file. Determine which
               stream has the largest chunk then write that size to the
               file. */
            if (((file->flags & AVI_FLAG_AUDIO) &&
                 file->video.values.max > file->audio.values.max) ||
                !(file->flags & AVI_FLAG_AUDIO))
               largestChunk = file->video.values.max;
               else
               largestChunk = file->audio.values.max;

            fileWrite(file, &largestChunk, sizeof(DWORD));

            /* Update dwLength in the video stream header */
            fileSeek(file, file->video.strhOffset +
                           offsetof(AVIStreamHeader, dwLength));

            fileWrite(file, &file->video.numFrames, sizeof(DWORD));

            /* Update dwSuggestedBufferSize in the video stream header */
            fileSeek(file, file->video.strhOffset +
                           offsetof(AVIStreamHeader, dwSuggestedBufferSize));

            largestChunk = file->video.values.max;
            fileWrite(file, &largestChunk, sizeof(DWORD));

            if (file->flags & AVI_FLAG_AUDIO)
            {
                /* Update dwLength in the audio stream header */
                fileSeek(file, file->audio.strhOffset +
                               offsetof(AVIStreamHeader, dwLength));

                fileWrite(file, &file->audio.streamLength, sizeof(DWORD));

                /* Update dwSuggestedBufferSize in the audio stream header */
                fileSeek(file, file->audio.strhOffset +
                               offsetof(AVIStreamHeader,
                                        dwSuggestedBufferSize));

                largestChunk = file->audio.values.max;
                fileWrite(file, &largestChunk, sizeof(DWORD));
            }

            /* Update dwTotalFrames in the extended AVI header */
            fileSeek(file, aviGetBaseOffset(file, DMLH_BASE) +
                           offsetof(ODMLExtendedAVIHeader, dwTotalFrames));

            fileWrite(file, &file->video.numFrames, sizeof(DWORD));

            fileClose(file);

            aviUnlockFile(file);

            /* Delete critical sections */
            DeleteCriticalSection(&file->lock.file);
            DeleteCriticalSection(&file->lock.timer);
            DeleteCriticalSection(&file->lock.video);
            DeleteCriticalSection(&file->lock.audio);
        }
    }
}

/** Destroys an AVI_FILE structure
 * \param file An AVI_FILE structure created with aviFileCreate
 */

void aviFileDestroy(AVI_FILE *file)
{
    if (file)
    {
        aviEndWriting(file);

        DeleteCriticalSection(&file->error.lock);
        free(file);
    }
}

/** Determines if expected time for the next frame has arrived
 * \param file An open file
 * \retval TRUE  If the next frame is expected to be saved
 * \retval FALSE If the next frame is not expected yet
 */

BOOL aviFrameReady(AVI_FILE *file)
{
    /**** DScaler already does its own timing and doesn't need this ****/
    /*if (!aviTimerStarted(file) ||
        aviGetTimer() < file->video.timer.next)
       return TRUE;

    return FALSE;*/

    return TRUE;
}

/** Determines if the file size limit has been reached
 * \param file An open file
 * \retval TRUE  The file size limit has been reached and the file should be
 *               closed
 * \retval FALSE The file size limit has not been reached and data can still be
 *               written to the file
 */

BOOL aviLimitReached(AVI_FILE *file)
{
    if (!file)
       return FALSE;

    return file->hold;
}

/** Set the maximum size of a recorded file. Audio and video data stops being
 * written when the file size reaches or exceeds the limit. This isn't exact so
 * leave a few hundred mebibytes of space from your ideal value so there's
 * enough room to stop.
 * \warning Only call this function before recording begins
 * \param file     A valid file
 * \param limitMiB The size limit for the file in mebibytes. Using a value of 0
 *                 means there is no set limit to the size of the file.
 */

void aviSetLimit(AVI_FILE *file, DWORD limitMiB)
{
    if (file)
    {
        if (!(file->flags & AVI_FLAG_LARGE_FILES) &&
            (limitMiB > AVI_4GiB_LIMIT || !limitMiB))
           limitMiB = AVI_4GiB_LIMIT;

        file->limit = (int64)limitMiB * (int64)1048576;
    }
}

/**********
 * Chunks *
 **********/

/** Starts a new RIFF chunk
 * \param file An open file
 * \param cc   The CC to use for the chunk
 * \pre The file has been locked
 */

void aviBeginChunk(AVI_FILE *file, FOURCC cc)
{
    CHUNK *chunk;

    assert(file->chunk.current < MAX_CHUNK);

    chunk = &file->chunk.data[file->chunk.current];

    chunk->cc    = cc;
    chunk->begin = fileTell(file);
    chunk->size  = 0;

    fileWrite(file, &chunk->cc, sizeof(FOURCC));
    fileWrite(file, &chunk->size, sizeof(DWORD));

    file->chunk.current++;
}

/** Called when an AVI/X RIFF chunk is ending */
void aviRIFFChunkEnding(AVI_FILE *file)
{
    /* A base offset of 0 for the RIFF chunk means that the first RIFF chunk is
       ending */
    if (aviGetBaseOffset(file, RIFF_BASE)==0)
    {
        file->legacy.frames = file->video.numFrames;

        aviIndexSetLegacyCounters(file);

        /* Reserve space for the legacy index at the end of the first RIFF
           chunk */
        fileReserveLegacyIndex(file);
    }

    /* Reset the counters for the next RIFF chunk */
    aviIndexClearCounters(file);
}

/** Ends a previously started RIFF chunk
 * \param file An open file
 * \return The size of the chunk's data in bytes
 * \pre The file has been locked
 * \note This automatically detects when the main RIFF chunk is ending and
 *       takes actions based on that. The main RIFF chunk must be at index 0.
 */

DWORD aviEndChunk(AVI_FILE *file)
{
    int64 offset;
    CHUNK *chunk;
    BYTE  temp;

    assert(file->chunk.current > 0);

    if (file->chunk.current - 1 <= 0)
    {
        /* This should be the main RIFF chunk */
        aviRIFFChunkEnding(file);
    }

    file->chunk.current--;

    chunk = &file->chunk.data[file->chunk.current];

    offset = fileTell(file);
    fileSeek(file, chunk->begin + 4);

    chunk->size = (DWORD)(offset - (chunk->begin + 8));
    fileWrite(file, &chunk->size, sizeof(DWORD));

    fileSeek(file, offset);

    /* Write an extra byte after the end of the chunk if it contains data that
       has an odd number of bytes. This should rarely or never happen since
       most things that are saved have an even number of data bytes. */
    if (chunk->size % 2)
    {
        temp = 0;
        fileWrite(file, &temp, 1);
    }

    return chunk->size;
}

/** Ends a chunk inside of a movi chunk and adds an index entry for the stream
 * \param file      An open file
 * \param type      A *_STREAM constant
 * \param replicate The number of times to replicate the index. If this value
 *                  is zero then only one index will be written for the chunk.
 * \param duration  The duration of this entry in stream ticks. This should NOT
 *                  include anything that's replicated since that's taken care
 *                  of automatically.
 * \param keyFrame  Determines if this entry references a key frame. Only
 *                  affects video streams.
 * \return The size of the chunk's data in bytes
 * \pre The file has been locked. This is being used to end a chunk inside of
 *      a LIST movi chunk.
 */

DWORD aviEndChunkWithIndex(AVI_FILE *file, stream_t type, DWORD replicate,
                           DWORD duration, BOOL keyFrame)
{
    int64 begin;
    DWORD size;

    assert(file->chunk.current > 0);

    /* Get the offset to the start of the chunk's data */
    begin = file->chunk.data[file->chunk.current - 1].begin + 8;

    /* End the current chunk and save its size */
    size = aviEndChunk(file);

    /* Add a new index for the chunk that was just ended */
    replicate++;

    while (replicate > 0)
    {
        aviIndexAddEntry(file, type, begin, size, duration, keyFrame);
        replicate--;
    }

    return size;
}

/** Ends all currently opened chunks
 * \param file An open file
 * \pre The file has been locked
 */

void aviEndAllChunks(AVI_FILE *file)
{
    while (file->chunk.current > 0)
        aviEndChunk(file);
}

/** Writes a FOURCC directly to the file
 * \param file An open file
 * \param cc   The CC to write
 * \pre The file has been locked
 */

void aviWriteFourCC(AVI_FILE *file, FOURCC cc)
{
    fileWrite(file, &cc, sizeof(FOURCC));
}

/** Checks the current RIFF AVI/X chunk to see if it's getting too large
 * \param file   An open file
 * \param values Values for one of the streams. This can be NULL.
 * \note This should be called after any new stream data is added
 * \note If \a values is being taken from the audio or video data then make
 *       sure the corresponding structures are locked first
 * \pre The file has been locked
 */

void aviCheckFile(AVI_FILE *file, STREAM_VALUES *values)
{
    int64 average;

    if (values)
       average = aviGetStreamValueAverage(values);
       else
       average = 0;

    if (file->limit)
    {
        /* Determine if the file is getting too large */
        if (fileTell(file) >= file->limit + average)
           file->hold = TRUE;
    }

    if (fileTell(file) - aviGetBaseOffset(file, RIFF_BASE) >=
        (int64)CHUNK_LIMIT + average)
    {
        /* Split the file */

        /* assert current chunk==LIST movi */

        aviIndexFlush(file);

        aviEndChunk(file);
        aviEndChunk(file);

        /* Start a new RIFF AVIX chunk */
        aviSetBaseOffset(file, RIFF_BASE);

        aviBeginChunk(file, RIFF_CHUNK);
        aviWriteFourCC(file, mmioFOURCC('A', 'V', 'I', 'X'));

        aviSetBaseOffset(file, MOVI_BASE);

        aviBeginChunk(file, LIST_CHUNK);
        aviWriteFourCC(file, mmioFOURCC('m', 'o', 'v', 'i'));
    }
}

/** Compresses and writes a frame to the file
 * \param src    A pointer to the uncompressed input frame
 * \param inTime The time that the frame actually arrived. Use aviGetTimer.
 * \retval TRUE  The frame was written
 * \retval FALSE The frame could not be written because of an error
 * \pre \a src points to a linear array of bytes that matches the format of the
 *      uncompressed input format
 */

BOOL aviSaveFrame(AVI_FILE *file, void *src, aviTime_t inTime)
{
    DWORD     size;
    BOOL      isKey;
    BOOL      result = TRUE;
    void      *data;
    int       writeFrames;
    int       count;
    aviTime_t temp;

    if (!file || !src)
       return FALSE;

    /* Stop here if no more data is supposed to be written */
    if (file->hold)
       return TRUE;

    writeFrames = 1;

    aviLockVideo(file);
    aviLockTimer(file);

    if (!aviTimerStarted(file))
    {
        file->video.timer.frames         = 1;
        file->video.timer.extraFrames    = 0;
        file->video.timer.lostFrames     = 0;
        file->video.timer.expectedFrames = file->video.fps;
        file->video.missing.frames       = 0;

        aviTimerSetStart(file, inTime);
    } else
    {
        if (file->flags & AVI_FLAG_AUDIO)
        {
            /* Make sure that only FPS number of frames are written for each
               second */
            writeFrames = 1;
            if (file->video.timer.frames < (int)file->video.timer.expectedFrames)
               file->video.timer.frames++;
               else
               {
                   file->video.timer.extraFrames++;
                   writeFrames = 0;
               }

            /* Has a second passed? */
            if (file->timer.elapsed >= file->timer.freq)
            {
                /* Reset the extra frame counter */
                count = 0;

                /* Make sure that the expected frame count is met */
                if (file->video.timer.frames < (int)file->video.timer.expectedFrames)
                   count += (int)file->video.timer.expectedFrames -
                            file->video.timer.frames;

                /* Calculate the number of seconds that have passed */
                temp = file->timer.elapsed / file->timer.freq;

                /* Make up for any lost frames in other seconds */
                if (temp > 1)
                   count += (int)(file->video.fps * (float)(temp - 1));

                if (count > 0)
                {
                    file->video.timer.frames += count;
                    file->video.timer.lostFrames += count;

                    /* Add these lost frames into the total number of missing
                       frames */
                    file->video.missing.frames += count;

                    /* Keep spreading out the duplicate frames as long as the
                       number of missing frames stays below 1 second.
                       Otherwise just duplicate all of the missing frames at
                       once. */
                    if (file->video.missing.frames <= file->video.fps)
                       file->video.missing.step = (int)((float)file->video.missing.frames /
                                                        ((float)file->video.fps / 4.0f));
                       else
                       file->video.missing.step = file->video.missing.frames;

                    if (file->video.missing.step < 1)
                       file->video.missing.step = 1;
                }

                #ifdef AVI_DEBUG
                if (file->video.timer.extraFrames > 0 ||
                    file->video.timer.lostFrames > 0 ||
                    file->timer.elapsed - (temp * file->timer.freq) > 0)
                {
                   cprintf("%d second(s), %d frame(s), %d extra, %d lost",
                                                   (int)temp,
                                                   file->video.timer.frames,
                                                   file->video.timer.extraFrames,
                                                   file->video.timer.lostFrames);

                    cprintf(", elapsed: %u", (unsigned long)(file->timer.elapsed - (temp * file->timer.freq)));

                    if (file->video.timer.lostFrames > 0)
                       cprintf(", %u MB", (unsigned long)(fileTell(file) / (1024 * 1024)));

                    cprintf("\n");
                }
                #endif

                /* Calculate the number of frames that are expected for the
                   next second */
                file->video.timer.expectedFrames += (float)temp *
                                                    file->video.fps;

                /* Reset the frame counters */
                file->video.timer.extraFrames = 0;
                file->video.timer.lostFrames  = 0;
                file->timer.elapsed -= temp * file->timer.freq;
            }
        }
    }

    aviUnlockTimer(file);

    /* Update the number of frames to write if there are still missing
       frames */
    if (file->video.missing.frames > 0)
    {
        if (file->video.missing.step <= file->video.missing.frames)
           count = file->video.missing.step;
           else
           count = file->video.missing.frames;

        file->video.missing.frames -= count;
        writeFrames += count;
    }

    if (writeFrames > 0)
    {
        data = aviVideoCompressData(file, src, &isKey, &size);

        aviLockFile(file);
        aviBeginChunk(file, file->video.cc.data);

        if (data && size > 0)
        {
            if (!fileWrite(file, data, size))
               result = FALSE;
        }

        size = aviEndChunkWithIndex(file, VIDEO_STREAM, writeFrames - 1, 1,
                                    isKey);

        /* Update the maximum and average chunk sizes for this stream */
        aviAddStreamValue(&file->video.values, size);

        file->video.numFrames += writeFrames;

        aviCheckFile(file, &file->video.values);
        aviUnlockFile(file);
    }

    aviUnlockVideo(file);

    return result;
}

/** Compresses and writes a block of audio data to the file
 * \param src     A pointer to the uncompressed PCM data
 * \param size    The size of \a src in bytes
 * \param samples The size of \a src in samples
 * \param values  The values for the audio stream to use. If this is being
 *                taken from the audio structure then make sure it's locked
 *                first.
 * \retval TRUE  The data was written to the file
 * \retval FALSE The data could not be written because of an error
 * \pre The data pointed to by \a src matches the recording format
 */

BOOL aviSaveAudio(AVI_FILE *file, void *src, DWORD size, DWORD samples,
                  STREAM_VALUES *values)
{
    BOOL result;

    if (!file || !src)
       return FALSE;

    aviLockFile(file);

    aviBeginChunk(file, file->audio.cc.data);
    result = fileWrite(file, src, size);
    aviEndChunkWithIndex(file, AUDIO_STREAM, 0, samples, FALSE);

    aviCheckFile(file, values);

    aviUnlockFile(file);

    return result;
}

/************************************************
 *              Timer functions                 *
 ************************************************/

/** Gets the number of ticks in one second for the timer
 * \retval 0        If an error occurred
 * \retval non-zero Success
 */

__inline aviTime_t aviGetTimerFreq(void)
{
    return 1000;
}

__inline aviTime_t aviGetTimer(void)
{
    return GetTickCount();
}

/** Determines if the start time has been set
 * \retval TRUE  If the start time has been set
 * \retval FALSE If the start time has not been set
 */

__inline BOOL aviTimerStarted(AVI_FILE *file)
{
    if (!file)
       return FALSE;

    return file->timer.flag;
}

/** Sets the start time
 * \param value The value of the timer to use as the start time
 * \pre The timer data is locked
 */

__inline void aviTimerSetStart(AVI_FILE *file, aviTime_t value)
{
    if (!file)
       return;

    file->timer.start = value;
    file->timer.flag  = TRUE;
}

/** Gets the starting time
 * \return The starting time
 * \pre The timer data is locked
 * \note The value returned is not defined if the start time hasn't been set
 */

__inline aviTime_t aviTimerGetStart(AVI_FILE *file)
{
    if (!file || !file->timer.flag)
       return 0;

    return file->timer.start;
}

/************************************************
 *              Locking/unlocking               *
 ************************************************/

/** Locks access to the file
 * \param file An open file
 * \pre Writing to the file has begun
 */

void aviLockFile(AVI_FILE *file)
{
    if (file)
       EnterCriticalSection(&file->lock.file);
}

/** Releases the lock on the file
 * \param file An open file
 * \pre Writing to the file has begun
 */

void aviUnlockFile(AVI_FILE *file)
{
    if (file)
       LeaveCriticalSection(&file->lock.file);
}

/** Locks the timer data
 * \param file An open file
 * \pre Writing to the file has begun
 */

void aviLockTimer(AVI_FILE *file)
{
    if (file)
       EnterCriticalSection(&file->lock.timer);
}

/** Releases the lock on the timer data
 * \param file An open file
 * \pre Writing to the file has begun
 */

void aviUnlockTimer(AVI_FILE *file)
{
    if (file)
       LeaveCriticalSection(&file->lock.timer);
}

/** Locks the video data
 * \param file An open file
 * \pre Writing to the file has begun
 */

void aviLockVideo(AVI_FILE *file)
{
    if (file)
       EnterCriticalSection(&file->lock.video);
}

/** Releases the lock on the video data
 * \param file An open file
 * \pre Writing to the file has begun
 */

void aviUnlockVideo(AVI_FILE *file)
{
    if (file)
       LeaveCriticalSection(&file->lock.video);
}

/** Locks the audio data
 * \param file An open file
 * \pre Writing to the file has begun
 */

void aviLockAudio(AVI_FILE *file)
{
    if (file)
       EnterCriticalSection(&file->lock.audio);
}

/** Releases the lock on the audio data
 * \param file An open file
 * \pre Writing to the file has begun
 */

void aviUnlockAudio(AVI_FILE *file)
{
    if (file)
       LeaveCriticalSection(&file->lock.audio);
}

/************************************************
 *            Stream value functions            *
 ************************************************/

/** Updates the total count and maximum values for a given amount
 * \param value  The values to update
 * \param amount The amount to add
 */

__inline void aviAddStreamValue(STREAM_VALUES *value, DWORD amount)
{
    value->total += (int64)amount;
    value->count++;

    if (amount > value->max)
       value->max = amount;
}

/** Gets the average of all the added amounts
 * \param value The values to average
 * \return The average of the amounts entered into \a value
 */

__inline int64 aviGetStreamValueAverage(STREAM_VALUES *value)
{
    return (value->count > 0) ? (value->total / (int64)value->count) : 0;
}
