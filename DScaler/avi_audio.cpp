/* $Id: avi_audio.cpp,v 1.1 2008-03-11 10:07:38 adcockj Exp $ */

/** \file
 * Audio recording and compression functions
 * \author Nick Kochakian
 */

#include "stdafx.h"
#include "avi.h"
#include "avi_internal.h"

/** Calculates the number of ticks equal to a number of bytes
 * \param file  An open file
 * \param wfx   The audio format being used
 * \param bytes The number of bytes being examined using the format defined by
 *              \a wfx
 * \return The number of ticks that are equal to the specified number of bytes
 * \note nBytesPerSec = nSamplesPerSec * nBlockAlign\n
 *       (bytes / bytesPerSec) * ticksPerSec = ticks
 */

__inline aviTime_t bytesToTicks(AVI_FILE *file, WAVEFORMATEX *wfx, DWORD bytes)
{
    return (aviTime_t)(((double)bytes /
                       (double)(wfx->nSamplesPerSec * wfx->nBlockAlign)) *
                       (double)file->timer.freq);
}

/** Calculates the number of bytes equal to a number of ticks
 * \param file  An open file
 * \param wfx   The audio format being used
 * \param ticks The number of ticks being examined
 * \return The number of bytes in the audio format \a wfx that has a length
 *         equal to the number of specified ticks
 * \note (ticks / ticksPerSec) * bytesPerSec = bytes
 */

__inline DWORD ticksToBytes(AVI_FILE *file, WAVEFORMATEX *wfx, aviTime_t ticks)
{
    return (DWORD)(((double)ticks / (double)file->timer.freq) *
                   ((double)wfx->nSamplesPerSec * (double)wfx->nBlockAlign));
}

/** Converts the number of recorded bytes to the number of recorded samples
 * \param file  An open file
 * \param wfx   The format the data was recorded in
 * \param bytes The number of recorded bytes
 * \return The number of recorded samples
 * \note The number of recorded bytes is a multiple of the block alignment so
 *       there shouldn't be a need to ever worry about a fractional number
 *       resulting from the calculation
 */

__inline DWORD bytesToSamples(AVI_FILE *file, WAVEFORMATEX *wfx, DWORD bytes)
{
    return wfx->nBlockAlign > 0 ? bytes / wfx->nBlockAlign : bytes;
}

/** Callback function for the waveIn device
 * \note This doesn't start saving audio data to an open file until the video
 *       stream has started to be saved
 */

void CALLBACK waveInCallback(HWAVEIN hWaveIn, UINT uMsg, DWORD dwInstance,
                             DWORD dwParam1, DWORD dwParam2)
{
    WAVEHDR  *whdr;
    AVI_FILE *file;
    BOOL     save;
    DWORD    startOffset;
    DWORD    size;
    DWORD    samples;

    struct
    {
        aviTime_t start, current;
    } time;

    if (uMsg==WIM_DATA)
    {
        whdr = (WAVEHDR *)dwParam1;
        file = (AVI_FILE *)dwInstance;

        /* Stop here if no more data is supposed to be written */
        if (file->hold)
           return;

        if (whdr->dwBytesRecorded > 0)
        {
            save         = FALSE;
            startOffset  = 0;
            time.current = aviGetTimer();

            aviLockTimer(file);

            if (aviTimerStarted(file))
            {
                time.start = aviTimerGetStart(file);
                if (time.current > time.start)
                {
                    save = TRUE;

                    /* Make sure the starting time doesn't go below zero if the
                       total time of this data is subtracted from it */
                    time.start -= bytesToTicks(file, &file->audio.wfxIn,
                                               whdr->dwBytesRecorded);
                    if (time.start < 0)
                    {
                        /* Adjust the starting offset so that data before the
                           starting time is not saved */
                        startOffset = ticksToBytes(file, &file->audio.wfxIn,
                                                   -time.start);

                        /* If the number of bytes that would be saved ends up
                           being zero, then don't save anything */
                        if ((long)whdr->dwBytesRecorded - (long)startOffset <= 0)
                           save = FALSE;
                    }
                }
            }

            if (save)
               file->timer.elapsed += bytesToTicks(file, &file->audio.wfxIn,
                                                   whdr->dwBytesRecorded -
                                                   startOffset);

            aviUnlockTimer(file);

            if (save)
            {
                size    = whdr->dwBytesRecorded - startOffset;
                samples = bytesToSamples(file, &file->audio.wfxIn, size);

                aviLockAudio(file);

                /* Update the maximum and average chunk sizes for this
                   stream */
                aviAddStreamValue(&file->audio.values, size);

                file->audio.streamLength += samples;

                aviSaveAudio(file, whdr->lpData + startOffset, size,
                             samples, &file->audio.values);

                aviUnlockAudio(file);
            }
        }

        if (file->audio.recording)
        {
            /* Give this buffer back to the waveIn device */
            whdr->dwFlags = 0;

            waveInPrepareHeader(hWaveIn, whdr, sizeof(WAVEHDR));
            waveInAddBuffer(hWaveIn, whdr, sizeof(WAVEHDR));
        }
    }
}

/** Defines the audio stream
 * \param file     A pointer to an AVI_FILE structure
 * \param deviceID The ID of the recording device
 * \param wfxIn    The recording audio format
 * \param wfxOut   The audio format that's being saved to the file
 */

void aviAudioDefineStream(AVI_FILE *file, UINT_PTR deviceID,
                          WAVEFORMATEX *wfxIn, WAVEFORMATEX *wfxOut)
{
    WAVEFORMATEX *wfx;

    if (!file || !wfxIn || !wfxOut)
       return;

    /*wfx = &file->audio.wfxIn;

    memset(wfx, 0, sizeof(WAVEFORMATEX));
    wfx->wFormatTag      = WAVE_FORMAT_PCM;
    wfx->nChannels       = 2;
    wfx->nSamplesPerSec  = 44100;
    wfx->wBitsPerSample  = 16;
    wfx->nBlockAlign     = (wfx->nChannels * wfx->wBitsPerSample) / 8;
    wfx->nAvgBytesPerSec = wfx->nSamplesPerSec * wfx->nBlockAlign;

    memcpy(&file->audio.wfxOut, &file->audio.wfxIn, sizeof(WAVEFORMATEX));*/

    file->audio.deviceID = deviceID;

    memcpy(&file->audio.wfxIn, wfxIn, sizeof(WAVEFORMATEX));
    memcpy(&file->audio.wfxOut, wfxOut, sizeof(WAVEFORMATEX));

    /* Recalculate the block align and bytes per sec variables in both the
       input and output formats to make sure they're using the correct
       values */
    wfx = &file->audio.wfxIn;

    wfx->nBlockAlign     = (wfx->nChannels * wfx->wBitsPerSample) / 8;
    wfx->nAvgBytesPerSec = wfx->nSamplesPerSec * wfx->nBlockAlign;

    wfx = &file->audio.wfxOut;

    wfx->nBlockAlign     = (wfx->nChannels * wfx->wBitsPerSample) / 8;
    wfx->nAvgBytesPerSec = wfx->nSamplesPerSec * wfx->nBlockAlign;

    file->audio.cc.data  = mmioFOURCC('0', '1', 'w', 'b');
    file->audio.cc.index = mmioFOURCC('i', 'x', '0', '1');

    file->flags |= AVI_FLAG_AUDIO;
}

/** Opens the compressor and the waveIn device
 * \param file A pointer to an AVI_FILE structure
 * \retval TRUE  Recording was set up
 * \retval FALSE Recording could not be set up because of an error
 */

BOOL aviAudioBegin(AVI_FILE *file)
{
    MMRESULT result;
    WAVEHDR  *whdr;
    int      i;
    DWORD    bufferLength;

    if (!file)
       return FALSE;

    if (file->audio.hWaveIn != INVALID_HANDLE_VALUE)
       return TRUE;

    result = waveInOpen(&file->audio.hWaveIn, file->audio.deviceID,
                        &file->audio.wfxIn, (DWORD_PTR)waveInCallback,
                        (DWORD_PTR)file, CALLBACK_FUNCTION);

    if (result != MMSYSERR_NOERROR)
    {
        file->audio.hWaveIn = (HWAVEIN)INVALID_HANDLE_VALUE;

        if (result==WAVERR_BADFORMAT)
           aviSetError(file, AVI_ERROR_AUDIO_OPEN,
                             "The recording format is not supported");
           else
           aviSetError(file, AVI_ERROR_AUDIO_OPEN,
                             "The waveIn device could not be opened");

        return FALSE;
    }

    bufferLength = file->audio.wfxIn.nAvgBytesPerSec;

    /* Reset the pointers of the buffers */
    for (i = 0; i < MAX_WAVE_BUFFER; i++)
        file->audio.buffer[i] = NULL;

    /* Allocate the buffers and set up the wave headers */
    for (i = 0; i < MAX_WAVE_BUFFER; i++)
    {
        /* Allocate enough space for one second of audio */
        file->audio.buffer[i] = malloc(bufferLength);
        if (!file->audio.buffer[i])
        {
            aviSetError(file, AVI_ERROR_AUDIO_OPEN, "Out of memory");
            return FALSE;
        }

        whdr = &file->audio.whdr[i];

        whdr->lpData         = (LPSTR)file->audio.buffer[i];
        whdr->dwBufferLength = bufferLength;
        whdr->dwFlags        = 0;

        if (waveInPrepareHeader(file->audio.hWaveIn, whdr,
                                sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
        {
            aviSetError(file, AVI_ERROR_AUDIO_OPEN,
                              "waveInPrepareHeader failed");
            return FALSE;
        }

        /* Send this buffer to the waveIn device */
        if (waveInAddBuffer(file->audio.hWaveIn, whdr,
                            sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
        {
            aviSetError(file, AVI_ERROR_AUDIO_OPEN, "waveInAddBuffer failed");
            return FALSE;
        }
    }

    return TRUE;
}

/** Closes the compressor and the waveIn device
 * \param file A file used in a previous call to aviAudioBegin
 */

void aviAudioEnd(AVI_FILE *file)
{
    int i;

    if (file)
    {
        if (file->audio.hWaveIn != INVALID_HANDLE_VALUE)
        {
            file->audio.recording = FALSE;

            /* Stop recording */
            waveInReset(file->audio.hWaveIn);

            /* Deallocate the buffers and unprepare the headers */
            for (i = 0; i < MAX_WAVE_BUFFER; i++)
            {
                if (file->audio.buffer[i])
                {
                    if (waveInUnprepareHeader(file->audio.hWaveIn,
                                              &file->audio.whdr[i],
                                              sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
                    {
                        #ifdef AVI_DEBUG
                        cprintf("waveInUnprepareHeader error\n");
                        #endif
                    }

                    free(file->audio.buffer[i]);
                    file->audio.buffer[i] = NULL;
                }
            }

            waveInClose(file->audio.hWaveIn);
            file->audio.hWaveIn = (HWAVEIN)INVALID_HANDLE_VALUE;
        }
    }
}

/** Starts recording data from an open waveIn device
 * \param file A file used in a previous call to aviAudioBegin
 * \warning Don't call this until the file has been opened and everything else
 *          has been initialized
 */

void aviAudioStartRecording(AVI_FILE *file)
{
    if (file && file->audio.hWaveIn != INVALID_HANDLE_VALUE)
    {
        file->audio.recording = TRUE;

        /* Start grabbing audio */
        waveInStart(file->audio.hWaveIn);
    }
}