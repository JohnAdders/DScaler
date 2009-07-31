/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Torbjörn Jansson.  All rights reserved.
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
//  GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////

/**
 * @file mediatypes.h AM_MEDIA_TYPE related stuff.
 */


#if !defined(AFX_MEDIATYPES_H__ADEEFF39_72C9_40F7_98D0_1F31E06419D4__INCLUDED_)
#define AFX_MEDIATYPES_H__ADEEFF39_72C9_40F7_98D0_1F31E06419D4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**
 * Copy class for atl enumerator classes.
 * This class is used as a template parameter to CComEnum<> when enumerating media types.
 * used in CDSRendInPin::EnumMediaTypes
 * @see CDSRendInPin
 */
class CopyMT
{
public:
    static HRESULT copy(AM_MEDIA_TYPE** p1, AM_MEDIA_TYPE** p2);
    static void init(AM_MEDIA_TYPE** p);
    static void destroy(AM_MEDIA_TYPE** p);
};

/** 
 * Function for copying mediatypes.
 * @param pDest destination mediatype
 * @param pSource source mediatype
 * @return true if successfull
 */
bool copyMediaType(AM_MEDIA_TYPE *pDest,const AM_MEDIA_TYPE *pSource);

//free and reset mediatype
//void initMediaType(AM_MEDIA_TYPE *pmt);

/**
 * Function for freeing the format block and pUnk pointer from a AM_MEDIA_TYPE struct.
 * @param pmt pointer to the mediatype to free
 */
void freeMediaType(AM_MEDIA_TYPE *pmt);

#endif // !defined(AFX_MEDIATYPES_H__ADEEFF39_72C9_40F7_98D0_1F31E06419D4__INCLUDED_)
