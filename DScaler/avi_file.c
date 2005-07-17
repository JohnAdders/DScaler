/* $Id: avi_file.c,v 1.1 2005-07-17 20:38:34 dosx86 Exp $ */

/** \file
 * Async file writing functions
 * \author Nick Kochakian
 */

#include "avi.h"
#include "avi_internal.h"

#define ASYNC_LOCK(file)   EnterCriticalSection(&(file)->async.lock)
#define ASYNC_UNLOCK(file) LeaveCriticalSection(&(file)->async.lock)

typedef struct
{
    DWORD size;   /**< The length of data in bytes */
    int64 offset; /**< The offset in the file */
} ASYNC_DATA_HEADER;

/** FIFO buffer */
typedef struct
{
    /** Mutexes to block concurrent reads or writes */
    struct
    {
        CRITICAL_SECTION read, write;
    } mutex;

    DWORD start; /**< Data start offset */
    DWORD end;   /**< Data end offset */
    DWORD size;  /**< Size of the data */
    BYTE  flags; /**< Where the different FIFO flags are stored */
    BYTE  *data; /**< Data */
} FIFO;

/****************************
 * File accessing functions *
 ****************************/

/** Sets the file pointer to a specific offset in the file
 * \param file The file to seek in
 * \param offset An absolute offset in the file
 * \retval TRUE  The file pointer was modified
 * \retval FALSE The file pointer could not be modified because of an error
 */

BOOL __seek(AVI_FILE *file, int64 offset)
{
    LARGE_INTEGER dest;

    dest.QuadPart = offset;
    if (!SetFilePointerEx(file->async.f, dest, NULL, FILE_BEGIN))
    {
        aviSetError(file, AVI_ERROR_FILE, "Seek failed");
        return FALSE;
    }

    return TRUE;
}

/** Writes data to a file
 * \param file The file to write to
 * \param data A pointer to the data to write
 * \param size The number of bytes to write from \a data
 * \retval TRUE The data was written
 * \retval FALSE The data could not be written because of an error
 */

BOOL __write(AVI_FILE *file, void *data, DWORD size)
{
    DWORD written;

    if (!WriteFile(file->async.f, data, size, &written, NULL))
    {
        aviSetError(file, AVI_ERROR_FILE, "Could not write to the file. "
                                          "The disk is most likely full.");
        return FALSE;
    }

    return TRUE;
}

/*****************************************
 *   Specialized FIFO buffer functions   *
 *****************************************/

#define FIFO_POS(fifo, pos) ((pos) % ((fifo)->size))

DWORD fifoDistance(DWORD start, DWORD end, DWORD size)
{
    if (end < start)
       end += size;

    return end - start;
}

void fifoAdjustReadSize(FIFO *fifo, DWORD *start, DWORD *size)
{
    DWORD usedSpace;

    /* Get the FIFO buffer's start offset and only use that value of start for
     * the rest of the calculations
     */
    *start = fifo->start;

    usedSpace = fifoDistance(*start, fifo->end, fifo->size);
    if (*size > usedSpace)
    {
        /* Strict mode. Non-strict mode would set size to usedSpace. */
        *size = 0;
    }
}

void fifoAdjustWriteSize(FIFO *fifo, DWORD *end, DWORD *size)
{
    DWORD freeSpace;

    /* Get the FIFO buffer's end offset and only use that value for end for the
     * rest of the calculations
     */
    *end = fifo->end;

    freeSpace = (fifo->size - 1) -
                fifoDistance(fifo->start, *end, fifo->size);
    if (*size > freeSpace)
    {
        /* Strict mode. Non-strict mode would set size to freeSpace. */
        *size = 0;
    }
}

/** Creates a FIFO buffer
 * \param size The size of the FIFO buffer's data in bytes
 * \return A pointer to a new FIFO buffer on success or NULL on failure
 */

FIFO *fifoCreate(DWORD size)
{
    FIFO *fifo = NULL;

    fifo = (FIFO *)malloc(sizeof(FIFO));
    if (fifo)
    {
        memset(fifo, 0, sizeof(FIFO));

        /* One byte is lost to the empty buffer condition start==end */
        size++;

        fifo->size = size;
        fifo->data = (BYTE *)malloc(size);
        if (fifo->data)
        {
            InitializeCriticalSection(&fifo->mutex.read);
            InitializeCriticalSection(&fifo->mutex.write);
        } else
        {
            free(fifo);
            fifo = NULL;
        }
    }

    return fifo;
}

/** Destroys a FIFO buffer
 * \param fifo A pointer to the FIFO buffer to destroy
 */

void fifoDestroy(FIFO *fifo)
{
    if (fifo)
    {
        if (fifo->data)
        {
            free(fifo->data);
            fifo->data = NULL;

            DeleteCriticalSection(&fifo->mutex.read);
            DeleteCriticalSection(&fifo->mutex.write);
        }

        free(fifo);
    }
}

/** Estimate the number of readable bytes in a FIFO buffer
 * \param fifo A pointer to a FIFO buffer
 * \return The approximate number of readable bytes
 */

DWORD fifoReadableSpace(FIFO *fifo)
{
    if (!fifo)
       return 0;

    return fifoDistance(fifo->start, fifo->end, fifo->size);
}

/** Estimate the number of bytes that can be written to a FIFO buffer
 * \param fifo A pointer to a FIFO buffer
 * \return The approximate number of writable bytes
 */

DWORD fifoWritableSpace(FIFO *fifo)
{
    if (!fifo)
       return 0;

    return (fifo->size - 1) - fifoReadableSpace(fifo);
}

/** Get the total usable space in the FIFO buffer
 * \param fifo A pointer to a FIFO buffer
 * \return The total usable space in bytes
 */

DWORD fifoTotalSpace(FIFO *fifo)
{
    if (!fifo)
       return 0;

    return fifo->size - 1;
}

/** Write data to the FIFO buffer
 * \param fifo   A pointer to a FIFO buffer
 * \param header A pointer to header data
 * \param src    A pointer to the data to copy
 * \return TRUE if the data was written or FALSE if there was not enough space
 *         to write the data
 */

BOOL fifoWrite(FIFO *fifo, ASYNC_DATA_HEADER *header, BYTE *src)
{
    /* This function only touches the end offset in the FIFO buffer */
    DWORD offset;
    DWORD thisBlock;
    DWORD size;
    DWORD wrote;
    BOOL  result = FALSE;

    /* Uses the local variables offset and wrote */
    #define FIFO_WRITE(FIFO, SRC, SIZE)\
        wrote = 0;\
\
        while (SIZE > 0)\
        {\
            if (offset + (SIZE) > (FIFO)->size)\
               thisBlock = (FIFO)->size - offset;\
               else\
               thisBlock = SIZE;\
\
            memcpy(&(FIFO)->data[offset], (BYTE *)(SRC) + wrote, thisBlock);\
\
            (SIZE) -= thisBlock;\
            offset = FIFO_POS(FIFO, offset + thisBlock);\
            wrote += thisBlock;\
        }

    if (!fifo)
       return FALSE;

    EnterCriticalSection(&fifo->mutex.write);

    /* Build the header and add its size to the number of bytes to write */
    size = header->size + sizeof(ASYNC_DATA_HEADER);

    /* Adjust the write size and set offset to the FIFO buffer's end offset */
    fifoAdjustWriteSize(fifo, &offset, &size);
    if (size > 0)
    {
        /* This means there's enough space to write the header and the data */

        /* Write the header first */
        size = sizeof(ASYNC_DATA_HEADER);
        FIFO_WRITE(fifo, header, size);

        /* Write the data */
        size = header->size;
        FIFO_WRITE(fifo, src, size);

        fifo->end = offset;

        result = TRUE;
    }

    LeaveCriticalSection(&fifo->mutex.write);

    return result;
}

/** Dumps data from a FIFO buffer to a file
 * \param fifo A pointer to a FIFO buffer
 * \param file The file to write the data to
 * \return TRUE if something was written or FALSE if there wasn't anything to
 *         write
 * \note This will set an error in \a file if something bad happens
 */

BOOL fifoDumpBlock(FIFO *fifo, AVI_FILE *file)
{
    /* This function only touches the start offset in the FIFO buffer */
    DWORD             offset;
    DWORD             thisBlock;
    DWORD             size;
    DWORD             read;
    ASYNC_DATA_HEADER header;
    BOOL              writeError;
    BOOL              fifoError;

    if (!fifo || !file)
       return FALSE;

    writeError = FALSE;
    fifoError  = FALSE;

    EnterCriticalSection(&fifo->mutex.read);

    if (fifoReadableSpace(fifo) < sizeof(ASYNC_DATA_HEADER))
    {
        LeaveCriticalSection(&fifo->mutex.read);
        return FALSE;
    }

    size = sizeof(ASYNC_DATA_HEADER);

    /* Adjust the read size and set offset to the FIFO buffer's start offset */
    fifoAdjustReadSize(fifo, &offset, &size);

    if (!size)
    {
        /* This shouldn't ever happen */
        aviSetError(file, AVI_ERROR_FIFO, "Internal write error");
        fifoError = TRUE;
    }

    /* Read the header */
    read = 0;
    while (size > 0)
    {
        if (offset + size > fifo->size)
           thisBlock = fifo->size - offset;
           else
           thisBlock = size;

        memcpy((BYTE *)&header + read, &fifo->data[offset], thisBlock);

        size -= thisBlock;
        offset = FIFO_POS(fifo, offset + thisBlock);
        read += thisBlock;
    }

    /* Write the data from the FIFO buffer to the file */
    size = header.size;

    if (fifoReadableSpace(fifo) < size + sizeof(ASYNC_DATA_HEADER))
    {
        /* This also shouldn't ever happen */
        aviSetError(file, AVI_ERROR_FIFO, "Internal write error");
        fifoError = TRUE;
    }

    if (!fifoError)
    {
        if (!__seek(file, header.offset))
           writeError = TRUE;

        while (size > 0)
        {
            if (offset + size > fifo->size)
               thisBlock = fifo->size - offset;
               else
               thisBlock = size;

            /* Keep writing data as long as there wasn't a write error */
            if (!writeError)
            {
                if (!__write(file, &fifo->data[offset], thisBlock))
                   writeError = TRUE;
            }

            size -= thisBlock;
            offset = FIFO_POS(fifo, offset + thisBlock);
        }
    } else
    {
        /* Try and recover from the FIFO error. Just make the start offset
           equal to the end offset so the bad data isn't read again. */
        offset = fifo->end;
    }

    fifo->start = offset;

    LeaveCriticalSection(&fifo->mutex.read);

    /* Always return true even if there was an error so anything that's waiting
       for a write to finish isn't blocked forever */
    return TRUE;
}

/*****************************************
 *         Async file functions          *
 *****************************************/

/** Async file writing thread
 * \note It's important that this thread doesn't terminate until the exit event
 *       is set. Terminating before then will cause any threads using fileWrite
 *       to sleep forever if they try to wait for free space in the buffer.
 */

DWORD WINAPI asyncThread(AVI_FILE *file)
{
    BOOL bExit;
    BOOL bFinishedWriting;

    bExit            = FALSE;
    bFinishedWriting = FALSE;
    while (!bExit || (bExit && !bFinishedWriting))
    {
        /* Attempt to write any data that might be in the FIFO buffer */
        while (fifoDumpBlock(file->async.fifo, file))
            SetEvent(file->async.hWriteEvent);

        if (!bExit)
        {
            /* There's nothing to write so wait until something new arrives */
            WaitForSingleObject(file->async.hThreadKick, INFINITE);

            /* Check the state of the exit event */
            if (WaitForSingleObject(file->async.hExit, 0)==WAIT_OBJECT_0)
               bExit = TRUE;
        } else
        {
            /* The FIFO buffer should be empty now */
            bFinishedWriting = TRUE;
        }
    }

    return 0;
}

/************************
 * Async file interface *
 ************************/

/** Opens a file for writing
 * \param fileName The path and file name of the file to open
 * \retval TRUE  The file was opened
 * \retval FALSE The file could not be opened because of an error
 */

BOOL fileOpen(AVI_FILE *file, char *fileName)
{
    DWORD temp;

    if (file->async.f != INVALID_HANDLE_VALUE)
       return TRUE;

    file->async.fifo = fifoCreate(ASYNC_MEM_LIMIT);
    if (!file->async.fifo)
    {
        aviSetError(file, AVI_ERROR_FILE_OPEN, "Out of memory");
        return FALSE;
    }

    file->async.hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!file->async.hWriteEvent)
    {
        fileClose(file);
        aviSetError(file, AVI_ERROR_FILE_OPEN, "Could not create an event");
        return FALSE;
    }

    file->async.hThreadKick = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!file->async.hThreadKick)
    {
        fileClose(file);
        aviSetError(file, AVI_ERROR_FILE_OPEN, "Could not create an event");
        return FALSE;
    }

    file->async.hExit = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!file->async.hExit)
    {
        fileClose(file);
        aviSetError(file, AVI_ERROR_FILE_OPEN, "Could not create an event");
        return FALSE;
    }

    /* Create the async thread but make sure it doesn't run until the file is
       opened and the lock is initialized */
    file->async.hThread = CreateThread(NULL, 0, asyncThread,
                                       (LPVOID)file, CREATE_SUSPENDED,
                                       &temp);
    if (!file->async.hThread)
    {
        fileClose(file);
        aviSetError(file, AVI_ERROR_FILE_OPEN,
                          "Failed to create the writing thread");
        return FALSE;
    }

    file->async.f = CreateFile(fileName, GENERIC_WRITE, 0, NULL,
                               CREATE_ALWAYS, 0, NULL);
    if (file->async.f==INVALID_HANDLE_VALUE)
    {
        fileClose(file);
        aviSetError(file, AVI_ERROR_FILE_OPEN,
                          "Could not open the file for writing");
        return FALSE;
    }

    if (!SetThreadPriority(file->async.hThread, THREAD_PRIORITY_ABOVE_NORMAL))
    {
        fileClose(file);
        aviSetError(file, AVI_ERROR_FILE_OPEN,
                          "Could not set the priority of the writing thread");
        return FALSE;
    }

    /* Reset the position in the file */
    file->async.filePos = 0;

    ResumeThread(file->async.hThread);

    return TRUE;
}

/** Closes a previously opened file */
void fileClose(AVI_FILE *file)
{
    if (file->async.hThread)
    {
        SetEvent(file->async.hExit);

        /* Wake up the thread and wait for it to finish */
        SetEvent(file->async.hThreadKick);
        WaitForSingleObject(file->async.hThread, INFINITE);
        CloseHandle(file->async.hThread);

        file->async.hThread = NULL;
    }

    if (file->async.f != INVALID_HANDLE_VALUE)
    {
        CloseHandle(file->async.f);
        file->async.f = INVALID_HANDLE_VALUE;
    }

    #ifdef AVI_DEBUG
    if (file->async.fifo && fifoReadableSpace(file->async.fifo))
       cprintf("Warning - The FIFO buffer isn't empty!\n");
    #endif

    if (file->async.hWriteEvent)
    {
        CloseHandle(file->async.hWriteEvent);
        file->async.hWriteEvent = NULL;
    }

    if (file->async.hThreadKick)
    {
        CloseHandle(file->async.hThreadKick);
        file->async.hThreadKick = NULL;
    }

    if (file->async.hExit)
    {
        CloseHandle(file->async.hExit);
        file->async.hExit = NULL;
    }

    if (file->async.fifo)
    {
        fifoDestroy(file->async.fifo);
        file->async.fifo = NULL;
    }
}

/** Sets the file pointer
 * \param file   An open file
 * \param offset The offset to set the file pointer to
 * \retval TRUE  If the file pointer was set
 * \retval FALSE If there was an error
 * \pre The file is locked
 */

BOOL fileSeek(AVI_FILE *file, int64 offset)
{
    if (!file || file->async.f==INVALID_HANDLE_VALUE)
       return FALSE;

    file->async.filePos = offset;

    return TRUE;
}

/** Gets the current value of the file pointer
 * \param file An open file
 * \return The current value of the file pointer, which is an absolute offset
 *         to the current write position in the file
 * \pre The file is locked
 */

int64 fileTell(AVI_FILE *file)
{
    if (!file || file->async.f==INVALID_HANDLE_VALUE)
       return 0;

    return file->async.filePos;
}

/** Writes data to the file at the current position
 * \param file An open file
 * \param data A pointer to the data to write
 * \param size The number of bytes from data to write
 * \retval TRUE  If the data was queued for writing
 * \retval FALSE If there was an error
 * \pre The file is locked
 */

BOOL fileWrite(AVI_FILE *file, void *data, DWORD size)
{
    ASYNC_DATA_HEADER header;

    if (!file || file->async.f==INVALID_HANDLE_VALUE || !data || !size)
       return FALSE;

    if (size > ASYNC_MEM_LIMIT + sizeof(ASYNC_DATA_HEADER))
    {
        aviSetError(file, AVI_ERROR_ASYNC,
                          "Tried to write a block of data that was larger than"
                          "the entire FIFO buffer");
        return FALSE;
    }

    /* Yes, keep trying to write data even if there's an error. Errors at this
       point usually happen when the disk runs out of space so it's possible
       that data will be written to parts of the file that are already on the
       disk. If there is an error then make sure to always return FALSE. */

    header.offset = file->async.filePos;
    header.size   = size;

    /* Try to write the data to the FIFO buffer. If that fails, wait until
       enough space is available. */
    while (!fifoWrite(file->async.fifo, &header, data))
    {
        #ifdef AVI_DEBUG
        cprintf("Mem limit reached\n");
        #endif

        WaitForSingleObject(file->async.hWriteEvent, INFINITE);
    }

    /* Move the file pointer forward */
    file->async.filePos += (int64)size;

    /* Notify the thread of the new data */
    SetEvent(file->async.hThreadKick);

    return aviHasError(file) ? FALSE : TRUE;
}

/** Determines if a file is currently opened
 * \param file The file to check
 * \retval TRUE  If a file is opened
 * \retval FALSE If a file is not opened
 */

BOOL fileIsOpen(AVI_FILE *file)
{
    if (!file || file->async.f==INVALID_HANDLE_VALUE)
       return FALSE;

    return TRUE;
}