///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// This file contains #define directives that control compilation of CPU-specific
// code, mostly deinterlacing functions.  Turning these directives on requires
// that you have Microsoft's "Processor Pack" patch installed on your build system.
// The Processor Pack is available from Microsoft for free:
//
// http://msdn.microsoft.com/vstudio/downloads/ppack/
//
// Note that compiling the code to use a processor-specific feature is safe even
// if your PC doesn't have the feature in question; DScaler detects processor types
// at startup and sets flags in the global "CpuFeatureFlags" (see cpu.h for
// the list of flags) which the code uses to determine whether or not to use
// each feature.
///////////////////////////////////////////////////////////////////////////////

#ifndef __DS_APICOMON_H__
#define __DS_APICOMON_H__ 1

// Symbolic constants for CpuFeatureFlags TRB 12/00
#define FEATURE_CPUID           0x00000001
#define FEATURE_STD_FEATURES    0x00000002
#define FEATURE_EXT_FEATURES    0x00000004
#define FEATURE_TSC             0x00000010
#define FEATURE_MMX             0x00000020
#define FEATURE_CMOV            0x00000040
#define FEATURE_3DNOW           0x00000080
#define FEATURE_3DNOWEXT        0x00000100
#define FEATURE_MMXEXT          0x00000200
#define FEATURE_SSEFP           0x00000400
#define FEATURE_K6_MTRR         0x00000800
#define FEATURE_P6_MTRR         0x00001000
#define FEATURE_SSE             0x00002000
#define FEATURE_SSE2            0x00004000

///////////////////////////////////////////////////////////////////////////////
// Definitions for the settings and new UI code
///////////////////////////////////////////////////////////////////////////////

/** type of setting
*/
typedef enum
{
    /// used when settings are depricated
    NOT_PRESENT = 0,
    // simple boolean setting
    ONOFF,
    // simple boolean setting
    YESNO,
    // select an item froma list
    ITEMFROMLIST,
    // select value using slider
    SLIDER,
    // character string
    CHARSTRING,
} SETTING_TYPE;

/** Function called when setting Value changes
    return Value indicates whether.rest of screen needs to be
    refreshed
*/
typedef BOOL (__cdecl SETTING_ONCHANGE)(long NewValue);

/** A Dscaler setting that may be manipulated
*/
typedef struct
{
    char* szDisplayName;
    SETTING_TYPE Type;
    long LastSavedValue;
    long* pValue;
    long Default;
    long MinValue;
    long MaxValue;
    long StepValue;
    long OSDDivider;
    const char** pszList;
    char* szIniSection;
    char* szIniEntry;
    SETTING_ONCHANGE* pfnOnChange;
} SETTING;

/** Deinterlace functions return true if the overlay is ready to be displayed.
*/
typedef void (_cdecl MEMCPY_FUNC)(void* pOutput, void* pInput, size_t nSize);

#define MAX_PICTURE_HISTORY 10


#define PICTURE_PROGRESSIVE 0
#define PICTURE_INTERLACED_ODD 1
#define PICTURE_INTERLACED_EVEN 2
#define PICTURE_INTERLACED_MASK (PICTURE_INTERLACED_ODD | PICTURE_INTERLACED_EVEN)

/** Structure containing a single field or frame
    from the source.

    This may be modified
*/

typedef struct
{
    // pointer to the start of data for this picture
    BYTE* pData;
    // see PICTURE_ flags
    DWORD Flags;
    // is this the first picture in a new series
    // use this flag to indicate changes to any of the
    // paramters that are assumed to be fixed like
    // timings or pixel width
    BOOL IsFirstInSeries;
} TPicture;


#define DEINTERLACE_INFO_CURRENT_VERSION 400

/** Structure used to transfer all the information used by plugins
    around in one chunk
*/
typedef struct
{
    /** set to version of this structure
        used to avoid crashing with incompatable versions
    */
    DWORD Version;

    /** The most recent pictures
        PictureHistory[0] is always the most recent.
        Pointers are NULL if the picture in question isn't valid, e.g. because
        the program just started or a picture was skipped.
    */
    TPicture* PictureHistory[MAX_PICTURE_HISTORY];

    /// Current overlay buffer pointer.
    BYTE *Overlay;

    /// The part of the overlay that we actually show
    RECT SourceRect;

    /** which frame are we on now
        \todo  remove this
    */
    int CurrentFrame;

    /// Overlay pitch (number of bytes between scanlines).
    DWORD OverlayPitch;

    /** Number of bytes of actual data in each scanline.  May be less than
        OverlayPitch since the overlay's scanlines might have alignment
        requirements.  Generally equal to FrameWidth * 2.
    */
    DWORD LineLength;

    /// Number of pixels in each scanline.
    int FrameWidth;

    /// Number of scanlines per frame.
    int FrameHeight;

    /** Number of scanlines per field.  FrameHeight / 2, mostly for
        cleanliness so we don't have to keep dividing FrameHeight by 2.
    */
    int FieldHeight;

    /// Results from the NTSC Field compare
    long FieldDiff;
    /// Results of the PAL Mode deinterlace detect
    long CombFactor;
    /// Function pointer to optimized memcpy function
    MEMCPY_FUNC* pMemcpy;
    /// What Type of CPU are we running
    long CpuFeatureFlags;
    /// Are we behind with processing
    BOOL bRunningLate;
    /// Are we behind with processing
    BOOL bMissedFrame;
    /// Do we want to flip accuratly
    BOOL bDoAccurateFlips;
    /// How big the source will end up
    RECT DestRect;

    /** distance between lines in image
        need not match the pixel width
    */
    long InputPitch;
} TDeinterlaceInfo;

#endif
