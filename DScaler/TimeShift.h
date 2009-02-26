/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Eric Schmidt.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//    This file is subject to the terms of the GNU General Public License as
//    published by the Free Software Foundation.  A copy of this license is
//    included with this software distribution in the file COPYING.  If you
//    do not have a copy, you may obtain a copy by writing to the Free
//    Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//    This software is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details
//
/////////////////////////////////////////////////////////////////////////////

/** 
 * @file timeshift.h timeshift Header file
 */
 
#ifndef __TIMESHIFT_H___
#define __TIMESHIFT_H___

#include "DS_ApiCommon.h"
#include "DS_Control.h"
#include "TreeSettingsGeneric.h"
#include "avi.h"

// These match the radio group in TSOptionsDlg.
#define TS_FULLHEIGHT     0
#define TS_HALFHEIGHTEVEN 1
#define TS_HALFHEIGHTODD  2
#define TS_HALFHEIGHTAVG  3

#define NTSC_RATE  30
#define NTSC_SCALE 1

#define PAL_RATE  25
#define PAL_SCALE 1

#define NTSC_FPS ((float)NTSC_RATE / (float)NTSC_SCALE)
#define PAL_FPS  ((float)PAL_RATE / (float)PAL_SCALE)

#define RGB_FOURCC  BI_RGB
#define YUY2_FOURCC mmioFOURCC('Y', 'U', 'Y', '2')

/** The maximum size of a recorded file in MiB */
#define MAX_FILE_SIZE 1000000

/** This is the default path that's used if some error happened. Make sure this
 * is kept short.
 */
#define TS_DEFAULT_PATH "C:\\"

/** TimeShift modes */
typedef enum
{
    /** Not playing nor recording nor paused, default. */
    MODE_STOPPED = 0,

    /** Standard playback of avi file. */
    MODE_PLAYING,

    /** Standard recording of live capture. */
    MODE_RECORDING,

    /** Recording, and playing at a previous offset. */
    MODE_SHIFTING,

    /** Shifting, but playing is paused. */
    MODE_PAUSED,

    /** Scanning forward while playing or shifting. */
    MODE_FASTFWD,

    /** Scanning backward while playing or shifting. */
    MODE_REWIND
} tsMode_t;

/** Defines the only two color formats that are currently being used
 * \note Make sure the function makeFormatValid is updated if anything
 *       here changes
 */
typedef enum
{
    FORMAT_YUY2 = 0,
    FORMAT_RGB
} tsFormat_t;

typedef LPBYTE (*TSSCANLINECOPYPROC)(LPBYTE, LPBYTE, DWORD);
typedef LPBYTE (*TSSCANLINEAVGPROC)(LPBYTE, LPBYTE, LPBYTE, DWORD);

typedef struct
{
    CRITICAL_SECTION lock;
    tsMode_t         mode;          /**< Recording mode (state) */
    tsFormat_t       format;        /**< Recording format (color space) */
    int              recHeight;     /**< Recording height */
    FOURCC           fccHandler;    /**< Video compression codec FourCC */
    char             savingPath[MAX_PATH + 1];  /**< Path to save files to */
    DWORD            sizeLimit;     /**< Size limit of each file in MiB (0 = no limit) */

    struct
    {
        BYTE *record;
    } buffer;

    HWND hWnd;

    char waveInDevice[MAXPNAMELEN];     /**< Selected waveIn device name */
    char waveOutDevice[MAXPNAMELEN];    /**< Selected waveOut device name */

    BITMAPINFOHEADER bih;
    WAVEFORMATEX     waveFormat;

    TSSCANLINECOPYPROC lpbCopyScanline; /**< Copy YUV data -> recording format */
    TSSCANLINEAVGPROC  lpbAvgScanline;  /**< Average YUV data -> recording format */

    AVI_FILE *file;
} TIME_SHIFT;

/**
  Here are some suggestions for TODO tasks...

  NEXT:
  - For pause-then-playbackshifted code...
    - can't write frames once pGetFrame is created.
    - write our own compressor/decompressor?
    - use no compression for now with AVIStreamRead() instead of getframe?
    - actually, it works right now, but the file it creates is a corrupt AVI.

  LATER:
  - Add a 'Preferred Pixelwidth While Recording' edit as an option.
  - in options dialog, add waveform buffer count and size (advanced section).
  - enumerate saved avi file dynamic radio checked menu items.
  - make sure that if m_mode is STOPPED, memory & resource usage is minimized.
  - Remove 4GB filesize limitation.  Maybe try stop then immeditate rerecord.
  - BUG: During playback in windowed mode, writing too wide of lines.
  - TWEAK: You can see the live feed peek in between clips during play mode.
  - Add slowforward/backward feature.
  - Play audio (option) during fastforward/backward.
  - Use the standard DScaler Settings for ini file stuff?
  - Use -height for right-side-up AVI?  Matches odd/even field orientation.

  OPTIMIZE:
  - Is there a codec out there that takes YUY2, compress it, but don't convert
    to RGB in the process?  If so, we can get rid of our rgb<-->yuv routines.

  DOC:
  - Must setup Audio Mixer through its dialog.  You can then uncheck the box
    if you don't normally use it, but all the dropdowns should point to the
    input on your audio card that your capture card's audio is attached to.
  - pixel width affects DF/S.  Choose a low enough pixel width for 0 DF/S.
  - Currently, If you record with audio at, say, 44.1kHz, etc, make sure you
    have these same settings on playback.

  LATEST CHANGES:
  - Solidified the auto-pixelwidth-setting feature for AVIs of varying width.
  - Added preliminary pause feature.  Left it disabled since it's unstable.
  - Added use of mixerdev to mute, but still record audio during pause/shifting.
  - Made public static function pointers to the mmx/c yuv<->rgb converters.
  - Disabled all height control features.  They made pause/shifting difficult.
*/

/* From TimeShift.cpp */
DWORD GetMaximumVolumeFileSize(const char *path);
bool TimeShiftInit(HWND hWnd);
void TimeShiftShutdown(void);
bool TimeShiftPause(void);
bool TimeShiftRecord(void);
bool TimeShiftStop(void);
bool TimeShiftOnNewInputFrame(TDeinterlaceInfo *pInfo);
bool TimeShiftIsRunning(void);
bool TimeShiftWorkOnInputFrames(void);
bool TimeShiftCancelSchedule(void);
bool TimeShiftSetWaveInDevice(char *pszDevice);
bool TimeShiftSetWaveOutDevice(char *pszDevice);
bool TimeShiftSetSavingPath(char *newPath);
bool TimeShiftIsPathValid(const char *path);
bool TimeShiftSetFileSizeLimit(DWORD sizeLimit);
bool TimeShiftGetDimensions(BITMAPINFOHEADER *bih, int recHeight,
                            tsFormat_t format);
bool TimeShiftGetWaveInDevice(char **ppszDevice);
bool TimeShiftGetWaveOutDevice(char **ppszDevice);
bool TimeShiftGetRecHeight(int *index);
bool TimeShiftSetRecHeight(int index);
bool TimeShiftGetRecFormat(tsFormat_t *format);
bool TimeShiftSetRecFormat(tsFormat_t format);
bool TimeShiftGetFourCC(FOURCC *fcc);
bool TimeShiftSetFourCC(FOURCC fcc);
bool TimeShiftGetVideoCompressionDesc(LPSTR dest, DWORD length, int recHeight,
                                      tsFormat_t format);
bool TimeShiftGetAudioCompressionDesc(LPSTR dest, DWORD length);
bool TimeShiftOnOptions(void);
bool TimeShiftVideoCompressionOptions(HWND hWndParent, int recHeight,
                                      tsFormat_t format, FOURCC *fccHandler);
bool TimeShiftOnSetMenu(HMENU hMenu);

const char *TimeShiftGetSavingPath(void);
DWORD      TimeShiftGetFileSizeLimit(void);

#endif // __TIMESHIFT_H___