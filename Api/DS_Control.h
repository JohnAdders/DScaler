/////////////////////////////////////////////////////////////////////////////
// $Id: DS_Control.h,v 1.77 2002-08-06 22:18:08 lindsey Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
// This header file is free software; you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details
/////////////////////////////////////////////////////////////////////////////
//
// Notes for writers of external apps
//
// To use control DScaler using an external app you can use SendMessage to
// perform both commands and to get/set all the settings
//
// To perform commands use the WM_COMMAND message
// e.g. SendMessage(hWndDScaler, WM_COMMAND, IDM_OSD_SHOW, 0);
//
// To get settings use the appropriate WM_XXX_GETVALUE
// e.g. Brightness = SendMessage(hWndDScaler, WM_BT848_GETVALUE, BRIGHTNESS, 0);
//
// To set settings use the appropriate WM_XXX_SETVALUE
// e.g. SendMessage(hWndDScaler, WM_BT848_SETVALUE, HUE, NewHueValue);
//
// To up settings use the appropriate WM_XXX_CHANGEVALUE
// e.g. SendMessage(hWndDScaler, WM_BT848_CHANGEVALUE, HUE, INCREMENTVALUE);
//
// To down settings use the appropriate WM_XXX_CHANGEVALUE
// e.g. SendMessage(hWndDScaler, WM_BT848_CHANGEVALUE, HUE, DECREMENTVALUE);
//
// To show settings use the appropriate WM_XXX_CHANGEVALUE
// e.g. SendMessage(hWndDScaler, WM_BT848_CHANGEVALUE, HUE, DISPLAYVALUE);
//
// To reset settings to default use the appropriate WM_XXX_CHANGEVALUE
// e.g. SendMessage(hWndDScaler, WM_BT848_CHANGEVALUE, HUE, RESETVALUE);
//
// To do the above operation without using the OSD use the ????VALUE_SILENT
// e.g. SendMessage(hWndDScaler, WM_BT848_CHANGEVALUE, HUE, INCREMENTVALUE_SILENT);
//
// The DScaler window handle can be obtained using
// hWndDScaler = FindWindow(DSCALER_APPNAME, NULL);
//
/////////////////////////////////////////////////////////////////////////////
//
// Notes for DScaler developers
//
// This is the place to add settings for any new file you create
// You should also update the LoadSettingsFromIni & SaveSettingsToIni
// functions in Settings.h so that your setttings get loaded
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 16 Jan 2001   John Adcock           Moved all parts that could be used to
//                                     Control DScaler externally to this file
//
// 20 Feb 2001   Michael Samblanet     Added new values for Aspect Control
//                                     (bounce & clipping Modes)
//
// 08 Jun 2001   Eric Schmidt          Added bounce amplitude to ini
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.76  2002/08/03 00:16:01  laurentg
// Value for WM_DI_TOMSMOCOMP_SETVALUE and WM_DI_TOMSMOCOMP_CHANGEVALUE updated to avoid conflicts
//
// Revision 1.75  2002/07/25 20:41:46  laurentg
// Setting added to take still always in the same file
//
// Revision 1.74  2002/07/24 21:37:00  laurentg
// Take cyclic stills
//
// Revision 1.73  2002/07/20 13:01:06  laurentg
// New setting for vertical mirror
//
// Revision 1.72  2002/07/20 10:26:34  laurentg
// New settings to select the wished OSD screens
//
// Revision 1.71  2002/07/19 15:25:09  laurentg
// New settings (other settings) added in the tree settings + related menu items deleted
//
// Revision 1.70  2002/07/08 17:44:58  adcockj
// Corrected Settings messages
//
// Revision 1.69  2002/07/08 13:53:35  trbarry
// First cut at motion compensated deinterlace
//
// Revision 1.68  2002/07/02 20:00:05  adcockj
// New setting for MSP input pin selection
//
// Revision 1.67  2002/06/24 21:45:42  laurentg
// New option to use or not WSS data when doing AR detection
//
// Revision 1.66  2002/06/20 20:00:42  robmuller
// Implemented videotext search highlighting.
//
// Revision 1.65  2002/06/18 19:46:05  adcockj
// Changed appliaction Messages to use WM_APP instead of WM_USER
//
// Revision 1.64  2002/06/13 11:24:32  robmuller
// Channel enter time is now configurable.
//
// Revision 1.63  2002/06/13 10:40:37  robmuller
// Made anti plop mute delay configurable.
//
// Revision 1.62  2002/06/06 12:48:18  robmuller
// Added settings for the Mirror filter.
//
// Revision 1.61  2002/06/05 20:53:49  adcockj
// Default changes and settings fixes
//
// Revision 1.60  2002/05/29 18:44:56  robmuller
// Added option to disable font anti-aliasing in Teletext.
//
// Revision 1.59  2002/05/27 20:17:05  robmuller
// Patch #561180  by PietOO:
// Autodetection of teletext code page.
//
// Revision 1.58  2002/05/26 09:21:48  robmuller
// Patch #560680 by PietOO:
// Added option to disable screensaver.
//
// Revision 1.57  2002/05/03 11:15:46  laurentg
// New settings added to define the size of the pattern
//
// Revision 1.56  2002/05/02 20:13:35  laurentg
// JPEG quality setting added
//
// Revision 1.55  2002/04/28 16:41:16  laurentg
// New setting for aspect ratio detect
//
// Revision 1.54  2002/04/27 16:06:15  laurentg
// Initial source
//
// Revision 1.53  2002/04/07 10:37:53  adcockj
// Made audio source work per input
//
// Revision 1.52  2002/02/27 20:41:08  laurentg
// Still settings
//
// Revision 1.51  2002/02/16 00:20:20  lindsey
// Added constants for FLT_Histogram
//
// Revision 1.50  2002/02/09 02:48:14  laurentg
// Overscan now stored in a setting of the source
//
// Revision 1.49  2002/02/01 20:25:24  robmuller
// Added new option to FLT_TNoise.
//
// Revision 1.48  2002/01/22 14:50:10  robmuller
// Added keyboard lock option.
//
// Revision 1.47  2002/01/05 23:05:39  lindsey
// Consolidated two settings into one from FLT_TemporalComb
//
// Revision 1.46  2001/12/31 00:55:55  lindsey
// Added settings for FLT_AdaptiveNoise
//
// Revision 1.45  2001/12/23 00:51:55  lindsey
// Added constants for Gradual Noise filter
//
// Revision 1.44  2001/12/16 17:04:37  adcockj
// Debug Log improvements
//
// Revision 1.43  2001/11/26 12:26:07  trbarry
// Missed update first commit?
//
// Revision 1.42  2001/11/02 10:45:29  adcockj
// Merge in code from Multiple card branch
//
// Revision 1.41  2001/10/18 16:20:39  adcockj
// Made Color of blanking adjustable
//
// Revision 1.40  2001/09/25 22:24:04  laurentg
// New control settings for calibration
//
// Revision 1.39  2001/09/12 14:32:45  tobbej
// fix for invalid DEINTERLACE_METHOD.nSettings (nSettings was more that the real number of settings)
//
// Revision 1.38  2001/08/30 10:06:29  adcockj
// Added support for extra settings in DI_OldGamne and FLT_TemporalComb
//
// Revision 1.37  2001/08/23 06:48:57  adcockj
// Fixed control header for TemporalComb filter
//
// Revision 1.36  2001/08/15 17:45:41  laurentg
// UseRGB ini parameter suppressed
//
// Revision 1.35  2001/08/14 11:36:03  adcockj
// Mixer change to allow restore of initial mixer settings
//
// Revision 1.34.2.3  2001/08/21 16:42:15  adcockj
// Per format/input settings and ini file fixes
//
// Revision 1.34.2.2  2001/08/20 16:14:18  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.34.2.1  2001/08/17 16:35:13  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
// Revision 1.34  2001/08/09 21:34:59  adcockj
// Fixed bugs raise by Timo and Keld
//
// Revision 1.33  2001/08/08 08:54:31  adcockj
// Added Delay option to film modes
// Switched comb modes to use greedy (low) on bad cadence instead of doings it's own thing
//
// Revision 1.32  2001/08/06 22:33:48  laurentg
// Little improvments for AR autodetection
//
// Revision 1.31  2001/08/03 12:27:41  adcockj
// Added sharpness filter
//
// Revision 1.30  2001/08/02 16:43:05  adcockj
// Added Debug level to LOG function
//
// Revision 1.29  2001/07/30 12:18:14  adcockj
// Added new OldGame plug-in
//
// Revision 1.28  2001/07/27 16:11:31  adcockj
// Added support for new Crash dialog
//
// Revision 1.27  2001/07/27 12:30:09  adcockj
// Added Overlay Color controls (Thanks to Muljadi Budiman)
//
// Revision 1.26  2001/07/26 21:59:15  laurentg
// New entry in OSD section of ini file
//
// Revision 1.25  2001/07/25 12:04:31  adcockj
// Moved Control stuff into DS_Control.h
// Added $Id and $Log to comment blocks as per standards
//
// Revision 1.24  2001/07/13 18:13:24  adcockj
// Changed Mute to not be persisted and to work properly
//
// Revision 1.23  2001/07/13 16:15:43  adcockj
// Changed lots of variables to match Coding standards
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __DS_CONTROL_H___
#define __DS_CONTROL_H___

#define DSCALER_APPNAME "DScaler"

/////////////////////////////////////////////////////////////////////////////
// Control Messages passed using WM_COMMAND
/////////////////////////////////////////////////////////////////////////////
#ifdef DSCALER_EXTERNAL

#define IDM_SAVE_SETTINGS_NOW           100

#define IDM_VT_RESET                    261
#define IDM_RESET                       280
#define IDM_TAKESTILL                   485
#define IDM_OVERLAY_STOP                590
#define IDM_OVERLAY_START               591
#define IDM_HIDE_OSD                    592
#define IDM_SHOW_OSD                    593
// IDM_SET_OSD_TEXT the lParam must be the handle of a global atom
#define IDM_SET_OSD_TEXT                594
#define IDM_FAST_REPAINT                599
#define IDM_ASPECT_FULLSCREEN           701
#define IDM_ASPECT_LETTERBOX            702
#define IDM_ASPECT_ANAMORPHIC           703

// Messages for the Video Format Menu
#define IDM_TYPEFORMAT_PAL              1120
#define IDM_TYPEFORMAT_NTSC             1121
#define IDM_TYPEFORMAT_SECAM            1122
#define IDM_TYPEFORMAT_PAL_M            1123
#define IDM_TYPEFORMAT_PAL_N            1124
#define IDM_TYPEFORMAT_NTSC_JAPAN       1125
#define IDM_TYPEFORMAT_PAL60            1126

// Indexes for the Video Deinterlace Algorithms
// to select a deinterlace method send a WM_COMMAND message
// with the relevant index added to this message
// e.g. To switch to adaptive
// SendMessage(hWndDScaler, WM_COMMAND, IDM_FIRST_DEINTMETHOD + INDEX_ADAPTIVE, 0);
#define IDM_FIRST_DEINTMETHOD             1900

/////////////////////////////////////////////////////////////////////////////
// For setting of certain values
//
typedef enum
{
    SOURCE_TUNER = 0,
    SOURCE_COMPOSITE,
    SOURCE_SVIDEO,
    SOURCE_OTHER1,
    SOURCE_OTHER2,
    SOURCE_COMPVIASVIDEO,
    SOURCE_CCIR656_1,
    SOURCE_CCIR656_2,
    SOURCE_CCIR656_3,
    SOURCE_CCIR656_4,
} VIDEOSOURCETYPE;

typedef enum
{
    FORMAT_PAL_BDGHI = 0,
    FORMAT_NTSC,
    FORMAT_SECAM,
    FORMAT_PAL_M,
    FORMAT_PAL_N,
    FORMAT_NTSC_J,
    FORMAT_PAL60,
    FORMAT_LASTONE,
} VIDEOFORMAT;

#endif

// Indexes for the Video Deinterlace Algorithms
#define INDEX_VIDEO_BOB                   0
#define INDEX_VIDEO_WEAVE                 1
#define INDEX_VIDEO_2FRAME                2
#define INDEX_WEAVE                       3
#define INDEX_BOB                         4
#define INDEX_SCALER_BOB                  5
#define INDEX_EVEN_ONLY                   13
#define INDEX_ODD_ONLY                    14
#define INDEX_BLENDED_CLIP                15
#define INDEX_ADAPTIVE                    16
#define INDEX_VIDEO_GREEDY                17
#define INDEX_VIDEO_GREEDY2FRAME          18
#define INDEX_VIDEO_GREEDYH               19
#define INDEX_OLD_GAME                    20
#define INDEX_VIDEO_TOMSMOCOMP            21

/////////////////////////////////////////////////////////////////////////////
// Allow callers to convert a WM code to do other operations
/////////////////////////////////////////////////////////////////////////////

#define WM_CONVERT_TO_GETVALUE(x)     (((x - WM_APP) % 100) + WM_APP)
#define WM_CONVERT_TO_SETVALUE(x)    ((((x - WM_APP) % 100) + 100) + WM_APP)
#define WM_CONVERT_TO_CHANGEVALUE(x) ((((x - WM_APP) % 100) + 200) + WM_APP)


/////////////////////////////////////////////////////////////////////////////
// Constants for WM_????_CHANGEVALUE messages
/////////////////////////////////////////////////////////////////////////////
typedef enum
{
    DISPLAY = 0,          // Display OSD Value.
    ADJUSTUP,             // Increase Value, with acceleration [display OSD]
    ADJUSTDOWN,           // Decrease Value, with acceleration [display OSD]
    INCREMENT,            // Increase Value by 1 [display OSD]
    DECREMENT,            // Decrease Value by 1 [display OSD]
    RESET,                // Reset Value to default [display OSD]
    TOGGLEBOOL,           // Toggle a boolean setting [display OSD]
    ADJUSTUP_SILENT,      // Same, but no OSD
    ADJUSTDOWN_SILENT,    // Same, but no OSD
    INCREMENT_SILENT,     // Same, but no OSD
    DECREMENT_SILENT,     // Same, but no OSD
    RESET_SILENT,         // Same, but no OSD
    TOGGLEBOOL_SILENT,    // Same, but no OSD
} eCHANGEVALUE;


/////////////////////////////////////////////////////////////////////////////
// Control settings contained in AspectRatio.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    SOURCE_ASPECT = 0,
    CUSTOM_SOURCE_ASPECT,
    TARGET_ASPECT,
    CUSTOM_TARGET_ASPECT,
    ASPECT_MODE,
    LUMINANCETHRESHOLD,
    IGNORENONBLACKPIXELS,
    AUTODETECTASPECT,
    ZOOMINFRAMECOUNT,
    ASPECTHISTORYTIME,
    ASPECTCONSISTENCYTIME,
    VERTICALPOS,
    HORIZONTALPOS,
    CLIPPING,
    BOUNCE,
    BOUNCEPERIOD,
    DEFERSETOVERLAY,
    BOUNCETIMERPERIOD,
    BOUNCEAMPLITUDE,
    ORBIT,
    ORBITPERIODX,
    ORBITPERIODY,
    ORBITSIZE,
    ORBITTIMERPERIOD,
    AUTOSIZEWINDOW,
    SKIPPERCENT,
    XZOOMFACTOR,
    YZOOMFACTOR,
    XZOOMCENTER,
    YZOOMCENTER,
    CHROMARANGE,
    WAITFORVERTBLANKINDRAW,
    ZOOMOUTFRAMECOUNT,
    ALLOWGREATERTHANSCREEN,
    MASKGREYSHADE,
    USEONLYWSS,
    USEWSS,
    ASPECT_SETTING_LASTONE,
} ASPECT_SETTING;

#define WM_ASPECT_GETVALUE          (WM_APP + 1)
#define WM_ASPECT_SETVALUE          (WM_APP + 101)
#define WM_ASPECT_CHANGEVALUE       (WM_APP + 201)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in Bt848.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    BRIGHTNESS = 0,
    CONTRAST,
    HUE,
    SATURATION,
    SATURATIONU,
    SATURATIONV,
    BDELAY,
    BTAGCDISABLE,
    BTCRUSH,
    BTEVENCHROMAAGC,
    BTODDCHROMAAGC,
    BTEVENLUMAPEAK,
    BTODDLUMAPEAK,
    BTFULLLUMARANGE,
    BTEVENLUMADEC,
    BTODDLUMADEC,
    BTEVENCOMB,
    BTODDCOMB,
    BTCOLORBARS,
    BTGAMMACORRECTION,
    BTCORING,
    BTHORFILTER,
    BTVERTFILTER,
    BTCOLORKILL,
    BTWHITECRUSHUP,
    BTWHITECRUSHDOWN,
    CURRENTX,
    CUSTOMPIXELWIDTH,
    VIDEOSOURCE,
    TVFORMAT,
    HDELAY,
    VDELAY,
    REVERSEPOLARITY,
    CURRENTCARDTYPE,
    CURRENTTUNERTYPE,
    PROCESSORSPEED,
    TRADEOFF,
    AUDIOSOURCE1,
    MSPMODE,
    MSPMAJORMODE,
    MSPMINORMODE,
    MSPSTEREO,
    AUTOSTEREOSELECT,
    VOLUME,
    SPATIAL,
    LOUDNESS,
    BASS,
    TREBLE,
    BALANCE,
    SUPERBASS,
    MSPEQ1,
    MSPEQ2,
    MSPEQ3,
    MSPEQ4,
    MSPEQ5,
    BT848SAVEPERINPUT,
    BT848SAVEPERFORMAT,
    BT848SAVETVFORMATPERINPUT,
    AUDIOSOURCE2,
    AUDIOSOURCE3,
    AUDIOSOURCE4,
    AUDIOSOURCE5,
    AUDIOSOURCE6,
    USEINPUTPIN1,
    BT848_SETTING_LASTONE,
} BT848_SETTING;

#define WM_BT848_GETVALUE           (WM_APP + 2)
#define WM_BT848_SETVALUE           (WM_APP + 102)
#define WM_BT848_CHANGEVALUE        (WM_APP + 202)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in DScaler.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    STARTLEFT = 0,
    STARTTOP,
    STARTWIDTH,
    STARTHEIGHT,
    ALWAYSONTOP,
    ISFULLSCREEN,
    FORCEFULLSCREEN,
    SHOWSTATUSBAR,
    SHOWMENU,
    WINDOWPROCESSOR,
    THREADPROCESSOR,
    WINDOWPRIORITY,
    THREADPRIORITY,
    AUTOSAVESETTINGS,
    ALWAYSONTOPFULL,
    SHOWCRASHDIALOG,
    DISPLAYSPLASHSCREEN,
    AUTOHIDECURSOR,
    LOCKKEYBOARD,
    SCREENSAVEROFF,
    AUTOCODEPAGE,
    VTANTIALIAS,
    INITIALSOURCE,
    CHANNELENTERTIME,
    DSCALER_SETTING_LASTONE,
} DSCALER_SETTING;

#define WM_DSCALER_GETVALUE             (WM_APP + 3)
#define WM_DSCALER_SETVALUE             (WM_APP + 103)
#define WM_DSCALER_CHANGEVALUE          (WM_APP + 203)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in OutThreads.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    HURRYWHENLATE = 0,
    WAITFORFLIP,
    DOVERTICALFLIP,
    DOACCURATEFLIPS,
    AUTODETECT,
    WAITFORVSYNC,
    DOJUDDERTERMINATORONVIDEO,
    OUTTHREADS_SETTING_LASTONE,
} OUTTHREADS_SETTING;

#define WM_OUTTHREADS_GETVALUE      (WM_APP + 4)
#define WM_OUTTHREADS_SETVALUE      (WM_APP + 104)
#define WM_OUTTHREADS_CHANGEVALUE   (WM_APP + 204)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in Other.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    BACKBUFFERS = 0,
    OVERLAYCOLOR,
    USEOVERLAYCONTROLS,
    OVERLAYBRIGHTNESS,
    OVERLAYCONTRAST,
    OVERLAYHUE,
    OVERLAYSATURATION,
    OVERLAYGAMMA,
    OVERLAYSHARPNESS,
    OTHER_SETTING_LASTONE,
} OTHER_SETTING;

#define WM_OTHER_GETVALUE           (WM_APP + 5)
#define WM_OTHER_SETVALUE           (WM_APP + 105)
#define WM_OTHER_CHANGEVALUE        (WM_APP + 205)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in FD_50Hz.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    PULLDOWNTHRESHHOLDLOW = 0,
    PULLDOWNTHRESHHOLDHIGH,
    PALFILMFALLBACKMODE,
    PALFILMREPEATCOUNT,
    PALFILMREPEATCOUNT2,
    MAXCALLSTOPALCOMB,
    PALBADCADENCEMODE,
    FD50_SETTING_LASTONE,
} FD50_SETTING;

#define WM_FD50_GETVALUE            (WM_APP + 6)
#define WM_FD50_SETVALUE            (WM_APP + 106)
#define WM_FD50_CHANGEVALUE         (WM_APP + 206)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in FD_50Hz.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    NTSCFILMFALLBACKMODE = 0,
    NTSCPULLDOWNREPEATCOUNT,
    NTSCPULLDOWNREPEATCOUNT2,
    THRESHOLD32PULLDOWN,
    THRESHOLDPULLDOWNMISMATCH,
    THRESHOLDPULLDOWNCOMB,
    FALLBACKTOVIDEO,
    PULLDOWNSWITCHINTERVAL,
    PULLDOWNSWITCHMAX,
    MAXCALLSTOCOMB,
    NTSCBADCADENCEMODE,
    FD60_SETTING_LASTONE,
} FD60_SETTING;

#define WM_FD60_GETVALUE            (WM_APP + 7)
#define WM_FD60_SETVALUE            (WM_APP + 107)
#define WM_FD60_CHANGEVALUE         (WM_APP + 207)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in FD_Common.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    BITSHIFT = 0,
    EDGEDETECT,
    JAGGIETHRESHOLD,
    DIFFTHRESHOLD,
    USECHROMA,
    FILMFLIPDELAY,
    FD_COMMON_SETTING_LASTONE,
} FD_COMMON_SETTING;

#define WM_FD_COMMON_GETVALUE       (WM_APP + 8)
#define WM_FD_COMMON_SETVALUE       (WM_APP + 108)
#define WM_FD_COMMON_CHANGEVALUE    (WM_APP + 208)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in DI_Adaptive.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    LOWMOTIONFIELDCOUNT = 0,
    STATICIMAGEFIELDCOUNT,
    STATICIMAGEMODE,
    LOWMOTIONMODE,
    HIGHMOTIONMODE,
    ADAPTIVETHRESH32PULLDOWN,
    ADAPTIVETHRESHMISMATCH,
    DI_ADAPTIVE_SETTING_LASTONE,
} DI_ADAPTIVE_SETTING;

#define WM_DI_ADAPTIVE_GETVALUE     (WM_APP + 9)
#define WM_DI_ADAPTIVE_SETVALUE     (WM_APP + 109)
#define WM_DI_ADAPTIVE_CHANGEVALUE  (WM_APP + 209)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in DI_VideoBob.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    WEAVEEDGEDETECT = 0,
    WEAVEJAGGIETHRESHOLD,
    DI_VIDEOBOB_SETTING_LASTONE,
} DI_VIDEOBOB_SETTING;

#define WM_DI_VIDEOBOB_GETVALUE     (WM_APP + 10)
#define WM_DI_VIDEOBOB_SETVALUE     (WM_APP + 110)
#define WM_DI_VIDEOBOB_CHANGEVALUE  (WM_APP + 210)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in DI_BlendedClip.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    BLCMINIMUMCLIP = 0,
    BLCPIXELMOTIONSENSE,
    BLCRECENTMOTIONSENSE,
    BLCMOTIONAVGPERIOD,
    BLCPIXELCOMBSENSE,
    BLCRECENTCOMBSENSE,
    BLCCOMBAVGPERIOD,
    BLCHIGHCOMBSKIP,
    BLCLOWMOTIONSKIP,
    BLCVERTICALSMOOTHING,
    BLCUSEINTERPBOB,
    BLCBLENDCHROMA,
    BLCSHOWCONTROLS,
    DI_BLENDEDCLIP_SETTING_LASTONE,
} DI_BLENDEDCLIP_SETTING;

#define WM_DI_BLENDEDCLIP_GETVALUE      (WM_APP + 11)
#define WM_DI_BLENDEDCLIP_SETVALUE      (WM_APP + 111)
#define WM_DI_BLENDEDCLIP_CHANGEVALUE   (WM_APP + 211)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in DI_TwoFrame.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    TWOFRAMESPATIALTOLERANCE = 0,
    TWOFRAMETEMPORALTOLERANCE,
    DI_TWOFRAME_SETTING_LASTONE,
} DI_TWOFRAME_SETTING;

#define WM_DI_TWOFRAME_GETVALUE     (WM_APP + 12)
#define WM_DI_TWOFRAME_SETVALUE     (WM_APP + 112)
#define WM_DI_TWOFRAME_CHANGEVALUE  (WM_APP + 212)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in DI_Greedy.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    GREEDYMAXCOMB = 0,
    GREEDYUSETILTNWARP,
    GREEDYSIZERATIO,
    GREEDYLINEARSIZEPCNT,
    GREEDYLINEARLOCIN,
    GREEDYLINEARLOCOUT,
    GREEDYTOPWARPFACTOR,
    GREEDYBOTTOMWARPFACTOR,
    DI_GREEDY_SETTING_LASTONE,
} DI_GREEDY_SETTING;

#define WM_DI_GREEDY_GETVALUE       (WM_APP + 15)
#define WM_DI_GREEDY_SETVALUE       (WM_APP + 115)
#define WM_DI_GREEDY_CHANGEVALUE    (WM_APP + 215)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in FLT_TNoise.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    TEMPORALLUMINANCETHRESHOLD = 0,
    TEMPORALCHROMATHRESHOLD,
    LOCKTHRESHOLDSTOGETHER,
    USETEMPORALNOISEFILTER,
    FLT_TNOISE_SETTING_LASTONE,
} FLT_TNOISE_SETTING;

#define WM_FLT_TNOISE_GETVALUE      (WM_APP + 14)
#define WM_FLT_TNOISE_SETVALUE      (WM_APP + 114)
#define WM_FLT_TNOISE_CHANGEVALUE   (WM_APP + 214)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in OSD.c
/////////////////////////////////////////////////////////////////////////////
typedef enum
{
    OSDB_TRANSPARENT = 0,
    OSDB_BLOCK,
    OSDB_SHADED,
    OSDBACK_LASTONE,
} eOSDBackground;

typedef enum
{
    OSD_OUTLINECOLOR = 0,
    OSD_TEXTCOLOR,
    OSD_PERCENTAGESIZE,
    OSD_PERCENTAGESMALLSIZE,
    OSD_ANTIALIAS,
    OSD_BACKGROUND,
    OSD_OUTLINE,
    OSD_AUTOHIDE_SCREEN,
    OSD_USE_GENERAL_SCREEN,
    OSD_USE_STATISTICS_SCREEN,
    OSD_USE_WSS_SCREEN,
    OSD_USE_DEVELOPER_SCREEN,
    OSD_SETTING_LASTONE,
} OSD_SETTING;
#define WM_OSD_GETVALUE     (WM_APP + 18)
#define WM_OSD_SETVALUE     (WM_APP + 118)
#define WM_OSD_CHANGEVALUE  (WM_APP + 218)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in FLT_Gamma.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    GAMMAVALUE = 0,
    USESTOREDTABLE,
    USEGAMMAFILTER,
    BLACKLEVEL,
    WHITELEVEL,
    FLT_GAMMA_SETTING_LASTONE,
} FLT_GAMMA_SETTING;

#define WM_FLT_GAMMA_GETVALUE       (WM_APP + 20)
#define WM_FLT_GAMMA_SETVALUE       (WM_APP + 120)
#define WM_FLT_GAMMA_CHANGEVALUE    (WM_APP + 220)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in VBI.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    CAPTURE_VBI = 0,
    CLOSEDCAPTIONMODE,
    DOTELETEXT,
    DOVPS,
    DOWSS,
    SEARCHHIGHLIGHT,
    VBI_SETTING_LASTONE,
} VBI_SETTING;

#define WM_VBI_GETVALUE     (WM_APP + 21)
#define WM_VBI_SETVALUE     (WM_APP + 121)
#define WM_VBI_CHANGEVALUE  (WM_APP + 221)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in DI_Greedy2Frame.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    GREEDYTWOFRAMETHRESHOLDLUMA = 0,
    GREEDYTWOFRAMETHRESHOLDCHROMA,
    DI_GREEDY2FRAME_SETTING_LASTONE,
} DI_GREEDY2FRAME_SETTING;

#define WM_DI_GREEDY2FRAME_GETVALUE     (WM_APP + 22)
#define WM_DI_GREEDY2FRAME_SETVALUE     (WM_APP + 122)
#define WM_DI_GREEDY2FRAME_CHANGEVALUE  (WM_APP + 222)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in DI_VideoWeave.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    TEMPORALTOLERANCE = 0,
    SPATIALTOLERANCE,
    SIMILARITYTHRESHOLD,
    DI_VIDEOWEAVE_SETTING_LASTONE,
} DI_VIDEOWEAVE_SETTING;

#define WM_DI_VIDEOWEAVE_GETVALUE       (WM_APP + 23)
#define WM_DI_VIDEOWEAVE_SETVALUE       (WM_APP + 123)
#define WM_DI_VIDEOWEAVE_CHANGEVALUE    (WM_APP + 223)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in FLT_LinearCorrection.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    USELINEARCORRFILTER = 0,
    DOONLYMASKING,
    MASKTYPE,
    MASKPARAM1,
    MASKPARAM2,
    MASKPARAM3,
    MASKPARAM4,
    FLT_LINEAR_CORR_SETTING_LASTONE,
} FLT_LINEAR_CORR_SETTING;

#define WM_FLT_LINEAR_CORR_GETVALUE     (WM_APP + 24)
#define WM_FLT_LINEAR_CORR_SETVALUE     (WM_APP + 124)
#define WM_FLT_LINEAR_CORR_CHANGEVALUE  (WM_APP + 224)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in MixerDev.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    USEMIXER = 0,
    MIXERINDEX,
    DESTINDEX,
    TUNERINDEX,
    COMPINDEX,
    SVIDEOINDEX,
    OTHER1INDEX,
    OTHER2INDEX,
    MIXERRESETONEXIT,
    MIXERDEV_SETTING_LASTONE,
} MIXERDEV_SETTING;

#define WM_MIXERDEV_GETVALUE        (WM_APP + 25)
#define WM_MIXERDEV_SETVALUE        (WM_APP + 125)
#define WM_MIXERDEV_CHANGEVALUE     (WM_APP + 225)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in FieldTiming.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    AUTOFORMATDETECT = 0,
    FIFTY_HZ_FORMAT,
    SIXTY_HZ_FORMAT,
    FORMATCHANGETHRESHOLD,
    SLEEPINTERVAL,
    SLEEPSKIPFIELDS,
    SLEEPSKIPFIELDSLATE,
    TIMING_SETTING_LASTONE,
} TIMING_SETTING;

#define WM_TIMING_GETVALUE          (WM_APP + 26)
#define WM_TIMING_SETVALUE          (WM_APP + 126)
#define WM_TIMING_CHANGEVALUE       (WM_APP + 226)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in Program.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    COUNTRYCODE = 0,
    CURRENTPROGRAM,
    CUSTOMCHANNELORDER,
    CHANNELS_SETTING_LASTONE,
} CHANNELS_SETTING;

#define WM_CHANNELS_GETVALUE            (WM_APP + 27)
#define WM_CHANNELS_SETVALUE            (WM_APP + 127)
#define WM_CHANNELS_CHANGEVALUE         (WM_APP + 227)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in Audio.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
	SYSTEMINMUTE = 0,
    AUDIO_SETTING_LASTONE,
} AUDIO_SETTING;

#define WM_AUDIO_GETVALUE               (WM_APP + 28)
#define WM_AUDIO_SETVALUE               (WM_APP + 128)
#define WM_AUDIO_CHANGEVALUE            (WM_APP + 228)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in Debug.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    DEBUGLOG = 0,
    DEBUGLEVEL,
    DEBUGFLUSHAFTERWRITE,
    DEBUG_SETTING_LASTONE,
} DEBUG_SETTING;

#define WM_DEBUG_GETVALUE               (WM_APP + 29)
#define WM_DEBUG_SETVALUE               (WM_APP + 129)
#define WM_DEBUG_CHANGEVALUE            (WM_APP + 229)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in FLT_LogoKill.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    LOGOKILL_TOP = 0,
    LOGOKILL_LEFT,
    LOGOKILL_WIDTH,
    LOGOKILL_HEIGHT,
    LOGOKILL_MODE,
    LOGOKILL_MAX,
    USELOGOKILL,
    FLT_LOGOKILL_SETTING_LASTONE,
} FLT_LOGOKILL_SETTING;

#define WM_FLT_LOGOKILL_GETVALUE        (WM_APP + 30)
#define WM_FLT_LOGOKILL_SETVALUE        (WM_APP + 130)
#define WM_FLT_LOGOKILL_CHANGEVALUE     (WM_APP + 230)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in VT.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    VTCODEPAGE = 0,
    VT_SETTING_LASTONE,
} VT_SETTING;

#define WM_VT_GETVALUE     (WM_APP + 31)
#define WM_VT_SETVALUE     (WM_APP + 131)
#define WM_VT_CHANGEVALUE  (WM_APP + 231)


/////////////////////////////////////////////////////////////////////////////
// Control settings contained in DI_GreedyH.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
	GR_MAX_COMB = 0,
	GR_MOTION_THRESHOLD,
	GR_MOTION_SENSE,
	GR_GOOD_PULLDOWN_LVL,
	GR_BAD_PULLDOWN_LVL,
	GR_H_SHARPNESS,			
	GR_V_SHARPNESS,			
	GR_MEDIAN_FILTER,
	GR_LOW_MOTION_PD_LVL,
	GR_USE_PULLDOWN,
	GR_USE_IN_BETWEEN,
	GR_USE_MEDIAN_FILTER,
	GR_USE_V_SHARPNESS,
	GR_USE_H_SHARPNESS,
//	GR_USE_LOW_MOTION_ONLY, 
	DI_GREEDYH_SETTING_LASTONE,
} DI_GREEDYH_SETTING;

#define WM_DI_GREEDYH_GETVALUE		(WM_APP + 32)
#define WM_DI_GREEDYH_SETVALUE		(WM_APP + 132)
#define WM_DI_GREEDYH_CHANGEVALUE	(WM_APP + 232)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in DI_OldGame.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    DI_OLDGAME_MAXCOMB = 0,
    DI_OLDGAME_COMPOSITEMODE,
    DI_OLDGAME_SETTING_LASTONE,
} DI_OLDGAME_SETTING;

#define WM_DI_OLDGAME_GETVALUE     (WM_APP + 33)
#define WM_DI_OLDGAME_SETVALUE     (WM_APP + 133)
#define WM_DI_OLDGAME_CHANGEVALUE  (WM_APP + 233)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in FLT_Sharpness.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    SHARPNESS = 0,
    SHARPNESS_PREFETCH,
    USESHARPNESS,
    FLT_SHARPNESS_SETTING_LASTONE,
} FLT_SHARPNESS_SETTING;

#define WM_FLT_SHARPNESS_GETVALUE     (WM_APP + 34)
#define WM_FLT_SHARPNESS_SETVALUE     (WM_APP + 134)
#define WM_FLT_SHARPNESS_CHANGEVALUE  (WM_APP + 234)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in FLT_TemporalComb.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    TCOMB_MAX_PHASE_COLOR_DIFF = 0,
    TCOMB_PREFETCH,
    TCOMB_SHIMMER_HIST_PERCENT,
    TCOMB_SHIMMER_PERCENT,
    TCOMB_ACTIVATE,
    TCOMB_TRADE_SPEED_FOR_ACCURACY,
    FLT_TCOMB_SETTING_LASTONE,
} FLT_TCOMB_SETTING;

#define WM_FLT_TCOMB_GETVALUE     (WM_APP + 35)
#define WM_FLT_TCOMB_SETVALUE     (WM_APP + 135)
#define WM_FLT_TCOMB_CHANGEVALUE  (WM_APP + 235)


/////////////////////////////////////////////////////////////////////////////
// Control settings contained in Calibration.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    SOURCE_OVERSCAN = 0,
    LEFT_SOURCE_CROPPING,
    RIGHT_SOURCE_CROPPING,
    SHOW_RGB_DELTA,
    SHOW_YUV_DELTA,
    CALIBR_SETTING_LASTONE,
} CALIBR_SETTING;

#define WM_CALIBR_GETVALUE     (WM_APP + 36)
#define WM_CALIBR_SETVALUE     (WM_APP + 136)
#define WM_CALIBR_CHANGEVALUE  (WM_APP + 236)


/////////////////////////////////////////////////////////////////////////////
// Control settings contained in DVBT.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    DVBT_SETTING_LASTONE = 0,
} DVBT_SETTING;

#define WM_DVBT_GETVALUE     (WM_APP + 37)
#define WM_DVBT_SETVALUE     (WM_APP + 137)
#define WM_DVBT_CHANGEVALUE  (WM_APP + 237)


/////////////////////////////////////////////////////////////////////////////
// Control settings contained in FLT_GradualNoise.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    NOISE_REDUCTION = 0,
    GNOISE_PREFETCH,
    GNOISE_ACTIVATE,
    FLT_GNOISE_SETTING_LASTONE,
} FLT_GNOISE_SETTING;

#define WM_FLT_GNOISE_GETVALUE      (WM_APP + 38)
#define WM_FLT_GNOISE_SETVALUE      (WM_APP + 138)
#define WM_FLT_GNOISE_CHANGEVALUE   (WM_APP + 238)


/////////////////////////////////////////////////////////////////////////////
// Control settings contained in FLT_AdaptiveNoise.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    ANOISE_STABILITY = 0,
    ANOISE_PREFETCH,
    ANOISE_REDUCTION,
    ANOISE_LOCK_DOT,
    ANOISE_ACTIVATE,
    ANOISE_INDICATOR,
    ANOISE_MOTION_MEMORY,
    FLT_ANOISE_SETTING_LASTONE,
} FLT_ANOISE_SETTING;

#define WM_FLT_ANOISE_GETVALUE      (WM_APP + 39)
#define WM_FLT_ANOISE_SETVALUE      (WM_APP + 139)
#define WM_FLT_ANOISE_CHANGEVALUE   (WM_APP + 239)


/////////////////////////////////////////////////////////////////////////////
// Control settings contained in FLT_Histogram.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    HISTOGRAM_DISPLAY_MODE = 0,
    HISTOGRAM_PREFETCH,
    HISTOGRAM_ACTIVATE,
    HISTOGRAM_USE_COMB,
    FLT_HISTOGRAM_SETTING_LASTONE,
} FLT_HISTOGRAM_SETTING;

#define WM_FLT_HISTOGRAM_GETVALUE      (WM_APP + 40)
#define WM_FLT_HISTOGRAM_SETVALUE      (WM_APP + 140)
#define WM_FLT_HISTOGRAM_CHANGEVALUE   (WM_APP + 240)


/////////////////////////////////////////////////////////////////////////////
// Control settings contained in StillSource.cpp
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    FORMATSAVING = 0,
    SLIDESHOWDELAY,
    JPEGQUALITY,
    PATTERNHEIGTH,
    PATTERNWIDTH,
    DELAYBETWEENSTILLS,
    SAVEINSAMEFILE,
    STILL_SETTING_LASTONE,
} STILL_SETTING;

#define WM_STILL_GETVALUE     (WM_APP + 41)
#define WM_STILL_SETVALUE     (WM_APP + 141)
#define WM_STILL_CHANGEVALUE  (WM_APP + 241)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in FLT_Mirror.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    USEMIRROR,
    FLT_MIRROR_SETTING_LASTONE,
} FLT_MIRROR_SETTING;

#define WM_FLT_MIRROR_GETVALUE     (WM_APP + 42)
#define WM_FLT_MIRROR_SETVALUE     (WM_APP + 142)
#define WM_FLT_MIRROR_CHANGEVALUE  (WM_APP + 242)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in ProgramList.cpp
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    PRESWITCHMUTEDELAY,
    POSTSWITCHMUTEDELAY,
    ANTIPLOP_SETTING_LASTONE,
} ANTIPLOP_SETTING;

#define WM_ANTIPLOP_GETVALUE     (WM_APP + 43)
#define WM_ANTIPLOP_SETVALUE     (WM_APP + 143)
#define WM_ANTIPLOP_CHANGEVALUE  (WM_APP + 243)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in DI_TOMSMOCOMP.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
    TOMSMOCOMPSEARCHEFFORT = 0,
    DI_TOMSMOCOMP_SETTING_LASTONE,
} DI_TOMSMOCOMP_SETTING;

#define WM_DI_TOMSMOCOMP_GETVALUE     (WM_APP + 44)
#define WM_DI_TOMSMOCOMP_SETVALUE     (WM_APP + 144)
#define WM_DI_TOMSMOCOMP_CHANGEVALUE  (WM_APP + 244)



#endif
