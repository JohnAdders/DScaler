///////////////////////////////////////////////////////////////////////////////
// DS_ApiCommon.h
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
// Change Log
//
// Date          Developer             Changes
//
// 27 Mar 2001   John Adcock           Separated code to support plug-ins
//
/////////////////////////////////////////////////////////////////////////////

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
#define FEATURE_SSE				0x00002000
#define FEATURE_SSE2            0x00004000

#define USE_SSE		1		// Use Intel Streaming SIMD Extensions
#define USE_3DNOW	1		// Use AMD 3DNow! extensions
#define USE_SSE2	1		// Use Intel SSE version 2 (Pentium 4 and above)

///////////////////////////////////////////////////////////////////////////////
// Definitions for the settings and new UI code
///////////////////////////////////////////////////////////////////////////////

typedef enum
{
	NOT_PRESENT = 0,
	ONOFF,
	YESNO,
	ITEMFROMLIST,
	SLIDER,
	NUMBER,
	SUBMENU,

} SETTING_TYPE;

// Function called when setting value changes
// return value indicates whether.rest of screen needs to be
// refreshed
typedef BOOL (SETTING_ONCHANGE)(long NewValue);

typedef struct
{
	char* szDisplayName;
	SETTING_TYPE Type;
	long OriginalValue;
	long* pValue;
	long Default;
	long MinValue;
	long MaxValue;
	long StepValue;
	long OSDDivider;
	char** pszList;
	char* szIniSection;
	char* szIniEntry;
	SETTING_ONCHANGE* pfnOnChange;
} SETTING;

#define SETTINGS_PER_MENU 8

typedef struct
{
   char* szDisplayName;
   SETTING* Elements[SETTINGS_PER_MENU];
} UI_SUBMENU;

/////////////////////////////////////////////////////////////////////////////
// Describes the inputs to a deinterlacing or filter algorithm.

// Deinterlace functions return true if the overlay is ready to be displayed.
typedef void (MEMCPY_FUNC)(void* pOutput, void* pInput, size_t nSize);

#define MAX_FIELD_HISTORY 5

typedef struct
{
	// Data from the most recent several odd and even fields, from newest
	// to oldest, i.e., OddLines[0] is always the most recent odd field.
	// Pointers are NULL if the field in question isn't valid, e.g. because
	// the program just started or a field was skipped.
	short **OddLines[MAX_FIELD_HISTORY];
	short **EvenLines[MAX_FIELD_HISTORY];

	// Current overlay buffer pointer.
	BYTE *Overlay;

	// The part of the overlay that we actually show
	RECT SourceRect;

	// True if the most recent field is an odd one; false if it was even.
	BOOL IsOdd;

	// which frame are we on
	int CurrentFrame;

	// Overlay pitch (number of bytes between scanlines).
	DWORD OverlayPitch;

	// Number of bytes of actual data in each scanline.  May be less than
	// OverlayPitch since the overlay's scanlines might have alignment
	// requirements.  Generally equal to FrameWidth * 2.
	DWORD LineLength;

	// Number of pixels in each scanline.
	int FrameWidth;

	// Number of scanlines per frame.
	int FrameHeight;

	// Number of scanlines per field.  FrameHeight / 2, mostly for
	// cleanliness so we don't have to keep dividing FrameHeight by 2.
	int FieldHeight;

	// Results from the NTSC Field compare
	long FieldDiff;
	// Results of the PAL mode deinterlace detect
	long CombFactor;
	// Function pointer to optimized memcpy function
	MEMCPY_FUNC* pMemcpy;
	// What type of CPU are we running
	long CpuFeatureFlags;
	// Are we behind with processing
	BOOL bRunningLate;
	// Are we behind with processing
	BOOL bMissedFrame;
	// Do we want to flip accuratly
	BOOL bDoAccurateFlips;
	// How long should we sleep for if we get the chance
	long SleepInterval;
	// How many fields to skip before doing sleep interval
	long SleepSkipFields;
	// How big the source will end up
	RECT DestRect;
	long SleepSkipFields;
} DEINTERLACE_INFO;

// Deinterlace functions return true if the overlay is ready to be displayed.
typedef BOOL (DEINTERLACE_FUNC)(DEINTERLACE_INFO *info);


#endif
