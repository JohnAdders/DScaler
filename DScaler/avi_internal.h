/* $Id: avi_internal.h,v 1.2 2005-07-24 23:07:48 dosx86 Exp $ */

#ifndef __AVI_INTERNAL_H
#define __AVI_INTERNAL_H

#ifdef AVI_DEBUG
#include <conio.h>
#endif

/* From avi.c */
void aviSetError(AVI_FILE *file, aviError_t type, char *message);
BOOL aviHasError(AVI_FILE *file);
extern __inline int64 aviGetBaseOffset(AVI_FILE *file, int type);

STREAM_CC *aviGetStreamCC(AVI_FILE *file, stream_t type);
void      reserveSpace(AVI_FILE *file, DWORD size);
void      aviBeginChunk(AVI_FILE *file, FOURCC cc);
void      aviAlignChunk(AVI_FILE *file, DWORD boundary);
DWORD     aviEndChunk(AVI_FILE *file);
void      aviEndChunkWithIndex(AVI_FILE *file, stream_t type,
                               unsigned long replicate, BOOL keyFrame);
void      aviEndAllChunks(AVI_FILE *file);
void      aviWriteFourCC(AVI_FILE *file, FOURCC cc);
void      aviCheckFile(AVI_FILE *file);
BOOL      aviSaveAudio(AVI_FILE *file, void *src, DWORD size);

void aviLockFile(AVI_FILE *file);
void aviUnlockFile(AVI_FILE *file);
void aviLockTimer(AVI_FILE *file);
void aviUnlockTimer(AVI_FILE *file);
void aviLockVideo(AVI_FILE *file);
void aviUnlockVideo(AVI_FILE *file);

extern __inline int64 aviGetTimerFreq(void);
extern __inline int64 aviGetTimer(void);
extern __inline BOOL aviTimerStarted(AVI_FILE *file);
extern __inline void aviTimerSetStart(AVI_FILE *file, int64 value);
extern __inline int64 aviTimerGetStart(AVI_FILE *file);

/* From avi_index.c */
void aviWriteInitialSuperIndex(AVI_FILE *file, stream_t type);
void aviIndexFlush(AVI_FILE *file);
void aviIndexAddEntry(AVI_FILE *file, stream_t type, int64 begin, DWORD size,
                      BOOL keyFrame);
void aviIndexClearCounters(AVI_FILE *file);
void aviIndexSetLegacyCounters(AVI_FILE *file);

/* From avi_video.c */
void aviVideoDefineStream(AVI_FILE *file, DWORD rate, DWORD scale, DWORD w,
                          DWORD h);
void aviVideoSetHandler(AVI_FILE *file, FOURCC fccHandler, FOURCC inFormat,
                        WORD bitCount);
BOOL aviVideoCompressorOpen(AVI_FILE *file);
BOOL aviVideoCompressorClose(AVI_FILE *file);
void *aviVideoCompressData(AVI_FILE *file, void *src, BOOL *isKey,
                           DWORD *outSize);
BITMAPINFOHEADER *aviVideoGetOutputFormat(AVI_FILE *file);

/* From avi_audio.c */
void aviAudioDefineStream(AVI_FILE *file, UINT_PTR deviceID,
                          WAVEFORMATEX *wfxIn, WAVEFORMATEX *wfxOut);
BOOL aviAudioBegin(AVI_FILE *file);
void aviAudioEnd(AVI_FILE *file);
void aviAudioStartRecording(AVI_FILE *file);

/* From avi_file.c */
void  fileReserveLegacyIndex(AVI_FILE *file);
BOOL  fileOpen(AVI_FILE *file, char *fileName);
void  fileClose(AVI_FILE *file);
BOOL  fileSeek(AVI_FILE *file, int64 offset);
int64 fileTell(AVI_FILE *file);
BOOL  fileWrite(AVI_FILE *file, void *data, DWORD size);
BOOL  fileReserveSpace(AVI_FILE *file, DWORD size);
BOOL  fileIsOpen(AVI_FILE *file);

#endif