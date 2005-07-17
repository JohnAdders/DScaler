/////////////////////////////////////////////////////////////////////////////
// $Id: TimeShift.h,v 1.22 2005-07-17 20:43:23 dosx86 Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Eric Schmidt.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2001   Eric Schmidt          Original Release
//                                     Got 'record mode' working.
//                                     Got 'play mode' working.
//                                     Got 'prev/next' working.
//                                     More to come.
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.21  2004/08/12 16:27:48  adcockj
// added timeshift changes from emu
//
// Revision 1.20  2003/12/29 01:27:54  robmuller
// Added AVI file splitting.
//
// Revision 1.19  2003/10/27 10:39:54  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.18  2003/10/11 17:25:53  laurentg
// Comments updated
//
// Revision 1.17  2003/10/11 15:45:50  laurentg
// Saving of compression options fixed
//
// Revision 1.16  2003/09/13 13:59:09  laurentg
// half height mode removed - Some menu actions like play or pause disabled
//
// Revision 1.15  2003/08/04 23:48:24  laurentg
// Use extra buffer when recording DScaler output frames
//
// Revision 1.14  2003/07/08 21:04:59  laurentg
// New timeshift mode (full height) - experimental
//
// Revision 1.13  2003/07/05 12:59:51  laurentg
// Timeshift enabled + some improvments
//
// Revision 1.12  2003/07/02 21:44:19  laurentg
// TimeShift settings
//
// Revision 1.11  2002/02/09 11:09:50  temperton
// Frame rate of created AVI now depends on TV format.
//
// Revision 1.10  2001/11/29 17:30:52  adcockj
// Reorgainised bt848 initilization
// More Javadoc-ing
//
// Revision 1.9  2001/11/22 13:32:03  adcockj
// Finished changes caused by changes to TDeinterlaceInfo - Compiles
//
// Revision 1.8  2001/11/21 15:21:39  adcockj
// Renamed DEINTERLACE_INFO to TDeinterlaceInfo in line with standards
// Changed TDeinterlaceInfo structure to have history of pictures.
//
// Revision 1.7  2001/11/20 11:43:00  temperton
// Store wave-device names instead of indexes in ini
//
// Revision 1.6  2001/08/06 03:00:17  ericschmidt
// solidified auto-pixel-width detection
// preliminary pausing-of-live-tv work
//
// Revision 1.5  2001/07/27 15:52:26  ericschmidt
// P3-or-better handling.
// Preliminary pixel-width auto-setting on playback.
//
// Revision 1.4  2001/07/26 15:28:14  ericschmidt
// Added AVI height control, i.e. even/odd/averaged lines.
// Used existing cpu/mmx detection in TimeShift code.
//
// Revision 1.3  2001/07/24 12:25:49  adcockj
// Added copyright notice as per standards
//
// Revision 1.2  2001/07/24 12:24:25  adcockj
// Added Id to comment block
//
// Revision 1.1  2001/07/23 20:52:07  ericschmidt
// Added TimeShift class.  Original Release.  Got record and playback code working.
//
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
    tsMode_t         mode;
    tsFormat_t       format;
    int              recHeight;
    FOURCC           fccHandler;

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
bool TimeShiftGetWaveInDevice(char **ppszDevice);
bool TimeShiftGetWaveOutDevice(char **ppszDevice);
bool TimeShiftGetRecHeight(int *index);
bool TimeShiftSetRecHeight(int index);
bool TimeShiftGetRecFormat(tsFormat_t *format);
bool TimeShiftSetRecFormat(tsFormat_t format);
bool TimeShiftGetCompressionDesc(LPSTR dest, DWORD length, bool video);
bool TimeShiftOnOptions(void);
bool TimeShiftCompressionOptions(HWND hWndParent);
bool TimeShiftVideoCompressionOptions(HWND hWndParent);
bool TimeShiftOnSetMenu(HMENU hMenu);

SETTING *TimeShift_GetSetting(TIMESHIFT_SETTING Setting);
void    TimeShift_ReadSettingsFromIni(void);
void    TimeShift_WriteSettingsToIni(BOOL bOptimizeFileAccess);
CTreeSettingsGeneric* TimeShift_GetTreeSettingsPage(void);
void    TimeShift_FreeSettings(void);

#endif // __TIMESHIFT_H___