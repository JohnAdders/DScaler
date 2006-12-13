/*
 *  General type definitions for tv_to_text
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License Version 2 as
 *  published by the Free Software Foundation. You find a copy of this
 *  license in the file COPYRIGHT in the root directory of this release.
 *
 *  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
 *  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
 *  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *
 *  Description:
 *
 *    Platform and compiler independent type definitions
 *    and global debug control.
 *
 *  Author: Tom Zoerner
 *
 *  $Id: xmltv_types.h,v 1.3 2006-12-13 01:10:01 robmuller Exp $
 */

#ifndef __XMLTV_TYPES_H
#define __XMLTV_TYPES_H


// include declaration of exact-width types for inter-process communication
// (e.g. uint16_t must have exactly 16 bits - no more, no less - so the basic ANSI C
// integer types should not be used, because they are specified by minimum width only)
#ifndef _MSC_VER
#include <sys/types.h>
#ifndef WIN32
#include <inttypes.h>
#else
#include <stdint.h>
#endif
#else
// Microsoft Visual C doesn't fully comply to the C99 standard, so we define the types here
#include <windef.h>
typedef BYTE           uint8_t;
typedef WORD           uint16_t;
typedef DWORD          uint32_t;
#endif

// required basic types
typedef unsigned char  bool;     // >=1 bit unsigned
typedef   signed char  schar;    // 8 bit signed
typedef unsigned char  uchar;    // 8 bit unsigned
typedef   signed int   sint;     // >=32 bit signed
typedef   signed short sshort;   // >=16 bit signed
#ifdef WIN32
typedef unsigned short ushort;   // >=16 bit unsigned
typedef unsigned int   uint;     // >=32 bit unsigned
#endif
#if defined(WIN32) || defined(__FreeBSD__)
// FreeBSD: required at least for 4.7-RC
typedef unsigned long  ulong;    // >=32 bit unsigned
#endif
typedef   signed long  slong;    // >=32 bit signed

// boolean values
#define FALSE 0
#define TRUE  1

// definitions for compile-time pre-processor switches
#define OFF FALSE
#define ON  TRUE

// en-/disable debugging
#define HALT_ON_FAILED_ASSERTION OFF
#define DEBUG_GLOBAL_SWITCH      OFF
#define DEBUG_SWITCH_XMLTV       OFF

// enable memory leak detection for debugging
#define CHK_MALLOC               OFF

// Macro to cast (void *) to (int) and backwards without compiler warning
// (note: 64-bit compilers warn when casting a pointer to an int)
#define  PVOID2INT(X)    ((int)((long)(X)))
#define  PVOID2UINT(X)   ((uint)((ulong)(X)))
#define  INT2PVOID(X)    ((void *)((long)(X)))
#define  UINT2PVOID(X)   ((void *)((ulong)(X)))

#ifdef WIN32
#define strcasecmp	_stricmp
#define strncasecmp	_strnicmp
#endif

#endif // __XMLTV_TYPES_H
