/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Atsushi Nakagawa.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//  This file is subject to the terms of the GNU General Public License as
//  published by the Free Software Foundation.  A copy of this license is
//  included with this software distribution in the file COPYING.  If you
//  do not have a copy, you may obtain a copy by writing to the Free
//  Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Library General Public License for more details
/////////////////////////////////////////////////////////////////////////////
//  
//  This file was based on, and supercedes the functionality of,
//  VTConvTables.*.
//  Those portions are Copyright (c) 2002 Mike Temperton
//
/////////////////////////////////////////////////////////////////////////////
//
// Change Log
//
// Date          Developer             Changes
//
// 21 Dec 2002   Atsushi Nakagawa      Moved the functionality of
//                                     VTConvTables.* into these new files.
//                                     Charactersets are now recognized as
//                                     G0 charactersets and G0 Latin subsets.
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.3  2003/02/05 06:58:28  atnak
// Added Danish codepage submitted by Kristian Trenskow (trenskow)
//
// Revision 1.2  2003/01/01 20:40:08  atnak
// Added wrapping to previous long CVS log entry
//
// Revision 1.1  2003/01/01 20:37:52  atnak
// New class for handling videotext charatersets.  Adds Latin, Cyrillic-1,
// Cyrillic-2, Cyrillic-3, Greek, Arabic and Hebrew G0 charactersets and
// various Latin subsets.
//
//
/////////////////////////////////////////////////////////////////////////////


/** 
 * @file vtcharacterset.h vtcharacterset Header file
 */
 
#ifndef __VTCHARACTERSET_H__
#define __VTCHARACTERSET_H__

#include "stdafx.h"
#include "VTCommon.h"


class CVTCharacterSet : CVTCommon
{
private:
    /// G0 Character Sets
    enum eG0CharacterSet
    {
        VTG0CHARACTERSET_LATIN,
        VTG0CHARACTERSET_CYRILLIC_1,
        VTG0CHARACTERSET_CYRILLIC_2,
        VTG0CHARACTERSET_CYRILLIC_3,
        VTG0CHARACTERSET_GREEK,
        VTG0CHARACTERSET_ARABIC,
        VTG0CHARACTERSET_HEBREW,
        VTG0CHARACTERSET_LASTONE
    };

    // G0 Latin National Option Sub-sets
    enum eG0LatinSubset
    {
        VTG0LATINSUBSET_NA          = -1,
        VTG0LATINSUBSET_CZECH       = 0,
        VTG0LATINSUBSET_ENGLISH,
        VTG0LATINSUBSET_ESTONIAN,
        VTG0LATINSUBSET_FRENCH,
        VTG0LATINSUBSET_GERMAN,
        VTG0LATINSUBSET_ITALIAN,
        VTG0LATINSUBSET_LETTISH,
        VTG0LATINSUBSET_POLISH,
        VTG0LATINSUBSET_PORTUGUESE,
        VTG0LATINSUBSET_RUMANIAN,
        VTG0LATINSUBSET_SLOVENIAN,
        VTG0LATINSUBSET_SWEDISN,
        VTG0LATINSUBSET_TURKISH,
        VTG0LATINSUBSET_DANISH,
        VTG0LATINSUBSET_LASTONE
    };

    // Character set designation
    typedef struct
    {
        eG0CharacterSet     G0CharacterSet;
        eG0LatinSubset      G0LatinSubset;
    } TCharacterSetDesignation;

public:
    static WORD VTCharToUnicode(eVTCodepage VTCodepage, BYTE VTChar);
    static void GetCharacterSet(eVTCodepage VTCodepage, WORD pBuffer[96]);

private:
    // The list of G0 characterset and subset designation for all codepages
    static TCharacterSetDesignation m_CharacterSetDesignation[VTCODEPAGE_LASTONE];

    static WORD m_G0CharacterSet[VTG0CHARACTERSET_LASTONE][96];
    static WORD m_G0LatinSubset[VTG0LATINSUBSET_LASTONE][13];
};


#endif
