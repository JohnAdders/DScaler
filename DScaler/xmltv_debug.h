/*
 *  Debug service module
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
 *    This is based on the file debug.h provided by Tom Zoerner in his
 *    XMLTV parser demo package. Only the minimum necessary to compile
 *    other files of the package has been kept.
 *
 *  $Id$
 */

#ifndef __XMLTV_DEBUG_H
#define __XMLTV_DEBUG_H

#include <stdio.h>

#ifdef DEBUG_SWITCH
#undef DEBUG_SWITCH
#endif
#define DEBUG_SWITCH OFF

#define assert(X)

#define debug0(S)
#define debug1(S,A)
#define debug2(S,A,B)
#define debug3(S,A,B,C)
#define ifdebug1(COND,S,A)
#define fatal0(S)
#define dprintf0(S)
#define dprintf1(S,A)
#define dprintf2(S,A,B)
#define dprintf3(S,A,B,C)
#define dprintf4(S,A,B,C,D)
#define dprintf5(S,A,B,C,D,E)


// memory allocation debugging
#define	xmalloc(SIZE)			malloc(SIZE)
#define	xrealloc(PTR, SIZE)		realloc(PTR, SIZE)
#define xfree(PTR)				free(PTR)
#define xstrdup(PTR)			_strdup(PTR)


#endif  /* not __XMLTV_DEBUG_H */

