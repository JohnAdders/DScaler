/*
 *  XMLTV element parser
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
 *  $Id: xmltv_tags.h,v 1.1 2005-07-06 19:42:39 laurentg Exp $
 */

#ifndef __XMLTV_TAGS_H
#define __XMLTV_TAGS_H


// Interface to lexigraphical scanner (callback functions)
void XmltvTags_Open( const char * pTagName );
bool XmltvTags_Close( const char * pTagName );
void XmltvTags_Data( XML_STR_BUF * pBuf );
void XmltvTags_AttribIdentify( const char * pName );
void XmltvTags_AttribData( XML_STR_BUF * pBuf );
bool XmltvTags_AttribsComplete( void );
void Xmltv_SyntaxError( const char * pMsg, const char * pStr );

// Interface to prolog parser
void XmltvTags_DocType( const char * pName );
void XmltvTags_DocIntDtdClose( void );
void XmltvTags_PiTarget( const char * pName );
void XmltvTags_PiContent( const char * pValue );
void XmltvTags_Notation( int stepIdx, const char * pValue );

// Interface to main
void XmltvTags_StartScan( FILE * fp, XMLTV_DTD_VERSION dtdVersion );
XMLTV_DTD_VERSION XmltvTags_QueryVersion( void );

// Interface to xmltv.lex
void XmlScan_Init( void );
void XmlScan_Destroy( void );
void XmlScan_Stop( void );
void XmlScan_EntityDefName( const char * pName, int isParamEntity );
void XmlScan_EntityDefValue( const char * pValue );

#endif  // __XMLTV_TAGS_H
