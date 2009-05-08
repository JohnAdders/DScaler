/*
 *  Helper module implementing a hash array for strings
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
 *  Description: see C source file.
 *
 *  Author: Tom Zoerner
 *
 *  $Id$
 */

#ifndef __XML_HASH_H
#define __XML_HASH_H


typedef void * XML_HASH_PTR;
typedef void * XML_HASH_PAYLOAD;
typedef void (* XML_HASH_FREE_CB)( XML_HASH_PTR pHash, TCHAR*  pPayload );

// ----------------------------------------------------------------------------
// Function interface

XML_HASH_PAYLOAD XmlHash_SearchEntry( XML_HASH_PTR pHash, const TCHAR*  pStr );
XML_HASH_PAYLOAD XmlHash_CreateEntry( XML_HASH_PTR pHash, const TCHAR*  pStr, Bool * pIsNew );

void XmlHash_Destroy( XML_HASH_PTR pHash, XML_HASH_FREE_CB pCb );
XML_HASH_PTR XmlHash_Init( void );

#endif /* __XML_HASH_H */
