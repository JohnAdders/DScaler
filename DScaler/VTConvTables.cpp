/////////////////////////////////////////////////////////////////////////////
// $Id: VTConvTables.cpp,v 1.2 2002-01-19 12:53:00 temperton Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Mike Temperton.  All rights reserved.
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.1  2002/01/15 11:16:03  temperton
// New teletext drawing code.
//
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VTConvTables.h"

WORD vtconv[VT_CODE_PAGE_LASTONE][96] = {
    //VT_UK_CODE_PAGE
    0x0020/* */,0x0021/*!*/,0x0022/*"*/,0x00A3/* */,0x0024/*$*/,0x0025/*%*/,0x0026/*&*/,0x0027/*'*/,
    0x0028/*(*/,0x0029/*)*/,0x002A/***/,0x002B/*+*/,0x002C/*,*/,0x002D/*-*/,0x002E/*.*/,0x002F/*/*/,
    0x0030/*0*/,0x0031/*1*/,0x0032/*2*/,0x0033/*3*/,0x0034/*4*/,0x0035/*5*/,0x0036/*6*/,0x0037/*7*/,
    0x0038/*8*/,0x0039/*9*/,0x003A/*:*/,0x003B/*;*/,0x003C/*<*/,0x003D/*=*/,0x003E/*>*/,0x003F/*?*/,
    0x0040/*@*/,0x0041/*A*/,0x0042/*B*/,0x0043/*C*/,0x0044/*D*/,0x0045/*E*/,0x0046/*F*/,0x0047/*G*/,
    0x0048/*H*/,0x0049/*I*/,0x004A/*J*/,0x004B/*K*/,0x004C/*L*/,0x004D/*M*/,0x004E/*N*/,0x004F/*O*/,
    0x0050/*P*/,0x0051/*Q*/,0x0052/*R*/,0x0053/*S*/,0x0054/*T*/,0x0055/*U*/,0x0056/*V*/,0x0057/*W*/,
    0x0058/*X*/,0x0059/*Y*/,0x005A/*Z*/,0x2190/* */,0x00BD/* */,0x2192/* */,0x2191/* */,0x0023/*#*/,
    0x002D/*-*/,0x0061/*a*/,0x0062/*b*/,0x0063/*c*/,0x0064/*d*/,0x0065/*e*/,0x0066/*f*/,0x0067/*g*/,
    0x0068/*h*/,0x0069/*i*/,0x006A/*j*/,0x006B/*k*/,0x006C/*l*/,0x006D/*m*/,0x006E/*n*/,0x006F/*o*/,
    0x0070/*p*/,0x0071/*q*/,0x0072/*r*/,0x0073/*s*/,0x0074/*t*/,0x0075/*u*/,0x0076/*v*/,0x0077/*w*/,
    0x0078/*x*/,0x0079/*y*/,0x007A/*z*/,0x00BC/* */,0x007C/*|*/,0x00BE/* */,0x00F7/* */,0x002A/***/,

    //VT_FRENCH_CODE_PAGE
    0x0020/* */,0x0021/*!*/,0x0022/*"*/,0x00E9/* */,0x00EF/* */,0x0025/*%*/,0x0026/*&*/,0x0027/*'*/,
    0x0028/*(*/,0x0029/*)*/,0x002A/***/,0x002B/*+*/,0x002C/*,*/,0x002D/*-*/,0x002E/*.*/,0x002F/*/*/,
    0x0030/*0*/,0x0031/*1*/,0x0032/*2*/,0x0033/*3*/,0x0034/*4*/,0x0035/*5*/,0x0036/*6*/,0x0037/*7*/,
    0x0038/*8*/,0x0039/*9*/,0x003A/*:*/,0x003B/*;*/,0x003C/*<*/,0x003D/*=*/,0x003E/*>*/,0x003F/*?*/,
    0x00E0/* */,0x0041/*A*/,0x0042/*B*/,0x0043/*C*/,0x0044/*D*/,0x0045/*E*/,0x0046/*F*/,0x0047/*G*/,
    0x0048/*H*/,0x0049/*I*/,0x004A/*J*/,0x004B/*K*/,0x004C/*L*/,0x004D/*M*/,0x004E/*N*/,0x004F/*O*/,
    0x0050/*P*/,0x0051/*Q*/,0x0052/*R*/,0x0053/*S*/,0x0054/*T*/,0x0055/*U*/,0x0056/*V*/,0x0057/*W*/,
    0x0058/*X*/,0x0059/*Y*/,0x005A/*Z*/,0x00EB/* */,0x00EA/* */,0x00F9/* */,0x005E/*^*/,0x005F/*_*/,
    0x00E8/* */,0x0061/*a*/,0x0062/*b*/,0x0063/*c*/,0x0064/*d*/,0x0065/*e*/,0x0066/*f*/,0x0067/*g*/,
    0x0068/*h*/,0x0069/*i*/,0x006A/*j*/,0x006B/*k*/,0x006C/*l*/,0x006D/*m*/,0x006E/*n*/,0x006F/*o*/,
    0x0070/*p*/,0x0071/*q*/,0x0072/*r*/,0x0073/*s*/,0x0074/*t*/,0x0075/*u*/,0x0076/*v*/,0x0077/*w*/,
    0x0078/*x*/,0x0079/*y*/,0x007A/*z*/,0x00E2/* */,0x00F4/* */,0x00FB/* */,0x00E7/* */,0x002A/***/,

    //VT_CZECH_CODE_PAGE
    0x0020/* */,0x0021/*!*/,0x0022/*"*/,0x0023/*#*/,0x016F/* */,0x0025/*%*/,0x0026/*&*/,0x0027/*'*/,
    0x0028/*(*/,0x0029/*)*/,0x002A/***/,0x002B/*+*/,0x002C/*,*/,0x002D/*-*/,0x002E/*.*/,0x002F/*/*/,
    0x0030/*0*/,0x0031/*1*/,0x0032/*2*/,0x0033/*3*/,0x0034/*4*/,0x0035/*5*/,0x0036/*6*/,0x0037/*7*/,
    0x0038/*8*/,0x0039/*9*/,0x003A/*:*/,0x003B/*;*/,0x003C/*<*/,0x003D/*=*/,0x003E/*>*/,0x003F/*?*/,
    0x010D/* */,0x0041/*A*/,0x0042/*B*/,0x0043/*C*/,0x0044/*D*/,0x0045/*E*/,0x0046/*F*/,0x0047/*G*/,
    0x0048/*H*/,0x0049/*I*/,0x004A/*J*/,0x004B/*K*/,0x004C/*L*/,0x004D/*M*/,0x004E/*N*/,0x004F/*O*/,
    0x0050/*P*/,0x0051/*Q*/,0x0052/*R*/,0x0053/*S*/,0x0054/*T*/,0x0055/*U*/,0x0056/*V*/,0x0057/*W*/,
    0x0058/*X*/,0x0059/*Y*/,0x005A/*Z*/,0x0165/* */,0x017E/* */,0x00FD/* */,0x00ED/* */,0x0159/*#*/,
    0x00E9/* */,0x0061/*a*/,0x0062/*b*/,0x0063/*c*/,0x0064/*d*/,0x0065/*e*/,0x0066/*f*/,0x0067/*g*/,
    0x0068/*h*/,0x0069/*i*/,0x006A/*j*/,0x006B/*k*/,0x006C/*l*/,0x006D/*m*/,0x006E/*n*/,0x006F/*o*/,
    0x0070/*p*/,0x0071/*q*/,0x0072/*r*/,0x0073/*s*/,0x0074/*t*/,0x0075/*u*/,0x0076/*v*/,0x0077/*w*/,
    0x0078/*x*/,0x0079/*y*/,0x007A/*z*/,0x00E1/* */,0x011B/* */,0x00FA/* */,0x0161/* */,0x002A/***/,

    //VT_GREEK_CODE_PAGE
    0x0020/* */,0x0021/*!*/,0x0022/*"*/,0x0023/*#*/,0x0024/*$*/,0x0025/*%*/,0x0026/*&*/,0x0027/*'*/,
    0x0028/*(*/,0x0029/*)*/,0x002A/***/,0x002B/*+*/,0x002C/*,*/,0x002D/*-*/,0x002E/*.*/,0x002F/*/*/,
    0x0030/*0*/,0x0031/*1*/,0x0032/*2*/,0x0033/*3*/,0x0034/*4*/,0x0035/*5*/,0x0036/*6*/,0x0037/*7*/,
    0x0038/*8*/,0x0039/*9*/,0x003A/*:*/,0x003B/*;*/,0x003C/*<*/,0x003D/*=*/,0x003E/*>*/,0x003F/*?*/,
    0x0040/*@*/,0x0391     ,0x0392     ,0x0393     ,0x0394     ,0x0395     ,0x0396     ,0x0397     ,
    0x0398     ,0x0399     ,0x039A     ,0x039B     ,0x039C     ,0x039D     ,0x039E     ,0x039F     ,
    0x03A0     ,0x03A1     ,0x0052/*R*/,0x03A3     ,0x03A4     ,0x03A5     ,0x03A6     ,0x03A7     ,
    0x03A8     ,0x03A9     ,0x005A/*Z*/,0x00C4     ,0x03AC     ,0x03AD     ,0x03AE     ,0x03AF     ,
    0x002D/*-*/,0x03B1     ,0x03B2     ,0x03B3     ,0x03B4     ,0x03B5     ,0x03B6     ,0x03B7     ,
    0x03B8     ,0x03B9     ,0x03BA     ,0x03BB     ,0x03BC     ,0x03BD     ,0x03BE     ,0x03BF     ,
    0x03C0     ,0x03C1     ,0x03C2     ,0x03C3     ,0x03C4     ,0x03C5     ,0x03C6     ,0x03C7     ,
    0x03C8     ,0x03C9     ,0x007A/*z*/,0x00E4     ,0x03CC     ,0x03CD     ,0x03CE     ,0x002A/***/,


    //VT_RUSSIAN_CODE_PAGE
    0x0020/* */,0x0021/*!*/,0x0022/*"*/,0x0023/*#*/,0x0024/*$*/,0x0025/*%*/,0x044B/*�*/,0x0027/*'*/,
    0x0028/*(*/,0x0029/*)*/,0x002A/***/,0x002B/*+*/,0x002C/*,*/,0x002D/*-*/,0x002E/*.*/,0x002F/*/*/,
    0x0030/*0*/,0x0031/*1*/,0x0032/*2*/,0x0033/*3*/,0x0034/*4*/,0x0035/*5*/,0x0036/*6*/,0x0037/*7*/,
    0x0038/*8*/,0x0039/*9*/,0x003A/*:*/,0x003B/*;*/,0x003C/*<*/,0x003D/*=*/,0x003E/*>*/,0x003F/*?*/,
    0x042E/*�*/,0x0410/*�*/,0x0411/*�*/,0x0426/*�*/,0x0414/*�*/,0x0415/*�*/,0x0424/*�*/,0x0413/*�*/,
    0x0425/*�*/,0x0418/*�*/,0x0419/*�*/,0x041A/*�*/,0x041B/*�*/,0x041C/*�*/,0x041D/*�*/,0x041E/*�*/,
    0x041F/*�*/,0x042F/*�*/,0x0420/*�*/,0x0421/*�*/,0x0422/*�*/,0x0423/*�*/,0x0416/*�*/,0x0412/*�*/,
    0x042C/*�*/,0x042A/*�*/,0x0417/*�*/,0x0428/*�*/,0x042D/*�*/,0x0429/*�*/,0x0427/*�*/,0x042B/*�*/,
    0x044E/*�*/,0x0430/*�*/,0x0431/*�*/,0x0446/*�*/,0x0434/*�*/,0x0435/*�*/,0x0444/*�*/,0x0433/*�*/,
    0x0445/*�*/,0x0438/*�*/,0x0439/*�*/,0x043A/*�*/,0x043B/*�*/,0x043C/*�*/,0x043D/*�*/,0x043E/*�*/,
    0x043F/*�*/,0x044F/*�*/,0x0440/*�*/,0x0441/*�*/,0x0442/*�*/,0x0443/*�*/,0x0436/*�*/,0x0432/*�*/,
    0x044C/*�*/,0x044A/*�*/,0x0437/*�*/,0x0448/*�*/,0x044D/*�*/,0x0449/*�*/,0x0447/*�*/,0x002A/***/,

    //VT_GERMAN_CODE_PAGE
    0x0020/* */,0x0021/*!*/,0x0022/*"*/,0x0023/*#*/,0x0024/*$*/,0x0025/*%*/,0x0026/*&*/,0x0027/*'*/,
    0x0028/*(*/,0x0029/*)*/,0x002A/***/,0x002B/*+*/,0x002C/*,*/,0x002D/*-*/,0x002E/*.*/,0x002F/*/*/,
    0x0030/*0*/,0x0031/*1*/,0x0032/*2*/,0x0033/*3*/,0x0034/*4*/,0x0035/*5*/,0x0036/*6*/,0x0037/*7*/,
    0x0038/*8*/,0x0039/*9*/,0x003A/*:*/,0x003B/*;*/,0x003C/*<*/,0x003D/*=*/,0x003E/*>*/,0x003F/*?*/,
    0x00A7/* */,0x0041/*A*/,0x0042/*B*/,0x0043/*C*/,0x0044/*D*/,0x0045/*E*/,0x0046/*F*/,0x0047/*G*/,
    0x0048/*H*/,0x0049/*I*/,0x004A/*J*/,0x004B/*K*/,0x004C/*L*/,0x004D/*M*/,0x004E/*N*/,0x004F/*O*/,
    0x0050/*P*/,0x0051/*Q*/,0x0052/*R*/,0x0053/*S*/,0x0054/*T*/,0x0055/*U*/,0x0056/*V*/,0x0057/*W*/,
    0x0058/*X*/,0x0059/*Y*/,0x005A/*Z*/,0x00C4/* */,0x00D6/* */,0x00DC/* */,0x005E/*^*/,0x005F/*_*/,
    0x00B0/* */,0x0061/*a*/,0x0062/*b*/,0x0063/*c*/,0x0064/*d*/,0x0065/*e*/,0x0066/*f*/,0x0067/*g*/,
    0x0068/*h*/,0x0069/*i*/,0x006A/*j*/,0x006B/*k*/,0x006C/*l*/,0x006D/*m*/,0x006E/*n*/,0x006F/*o*/,
    0x0070/*p*/,0x0071/*q*/,0x0072/*r*/,0x0073/*s*/,0x0074/*t*/,0x0075/*u*/,0x0076/*v*/,0x0077/*w*/,
    0x0078/*x*/,0x0079/*y*/,0x007A/*z*/,0x00E4/* */,0x00F6/* */,0x00FC/* */,0x00DF/* */,0x002A/***/,

    //VT_HUNGARIAN_CODE_PAGE
    0x0020/* */,0x0021/*!*/,0x0022/*"*/,0x00A3/* */,0x0024/*$*/,0x0025/*%*/,0x0026/*&*/,0x0027/*'*/,
    0x0028/*(*/,0x0029/*)*/,0x002A/***/,0x002B/*+*/,0x002C/*,*/,0x002D/*-*/,0x002E/*.*/,0x002F/*/*/,
    0x0030/*0*/,0x0031/*1*/,0x0032/*2*/,0x0033/*3*/,0x0034/*4*/,0x0035/*5*/,0x0036/*6*/,0x0037/*7*/,
    0x0038/*8*/,0x0039/*9*/,0x003A/*:*/,0x003B/*;*/,0x003C/*<*/,0x003D/*=*/,0x003E/*>*/,0x003F/*?*/,
    0x0116/* */,0x0041/*A*/,0x0042/*B*/,0x0043/*C*/,0x0044/*D*/,0x0045/*E*/,0x0046/*F*/,0x0047/*G*/,
    0x0048/*H*/,0x0049/*I*/,0x004A/*J*/,0x004B/*K*/,0x004C/*L*/,0x004D/*M*/,0x004E/*N*/,0x004F/*O*/,
    0x0050/*P*/,0x0051/*Q*/,0x0052/*R*/,0x0053/*S*/,0x0054/*T*/,0x0055/*U*/,0x0056/*V*/,0x0057/*W*/,
    0x0058/*X*/,0x0059/*Y*/,0x005A/*Z*/,0x2190/* */,0x00D6/* */,0x00C1/* */,0x00DC/* */,0x0023/*#*/,
    0x0117/* */,0x0061/*a*/,0x0062/*b*/,0x0063/*c*/,0x0064/*d*/,0x0065/*e*/,0x0066/*f*/,0x0067/*g*/,
    0x0068/*h*/,0x0069/*i*/,0x006A/*j*/,0x006B/*k*/,0x006C/*l*/,0x006D/*m*/,0x006E/*n*/,0x006F/*o*/,
    0x0070/*p*/,0x0071/*q*/,0x0072/*r*/,0x0073/*s*/,0x0074/*t*/,0x0075/*u*/,0x0076/*v*/,0x0077/*w*/,
    0x0078/*x*/,0x0079/*y*/,0x007A/*z*/,0x00BC/* */,0x00F6/* */,0x00E1/* */,0x00FC/* */,0x002A/***/,

    //VT_HEBREW_CODE_PAGE
    0x0020/* */,0x0021/*!*/,0x0022/*"*/,0x00A3/* */,0x0024/*$*/,0x0025/*%*/,0x0026/*&*/,0x0027/*'*/,
    0x0028/*(*/,0x0029/*)*/,0x002A/***/,0x002B/*+*/,0x002C/*,*/,0x002D/*-*/,0x002E/*.*/,0x002F/*/*/,
    0x0030/*0*/,0x0031/*1*/,0x0032/*2*/,0x0033/*3*/,0x0034/*4*/,0x0035/*5*/,0x0036/*6*/,0x0037/*7*/,
    0x0038/*8*/,0x0039/*9*/,0x003A/*:*/,0x003B/*;*/,0x003C/*<*/,0x003D/*=*/,0x003E/*>*/,0x003F/*?*/,
    0x0040/*@*/,0x0041/*A*/,0x0042/*B*/,0x0043/*C*/,0x0044/*D*/,0x0045/*E*/,0x0046/*F*/,0x0047/*G*/,
    0x0048/*H*/,0x0049/*I*/,0x004A/*J*/,0x004B/*K*/,0x004C/*L*/,0x004D/*M*/,0x004E/*N*/,0x004F/*O*/,
    0x0050/*P*/,0x0051/*Q*/,0x0052/*R*/,0x0053/*S*/,0x0054/*T*/,0x0055/*U*/,0x0056/*V*/,0x0057/*W*/,
    0x0058/*X*/,0x0059/*Y*/,0x005A/*Z*/,0x2190/* */,0x00BD/* */,0x2192/* */,0x2191/* */,0x0023/*#*/,
    0x05D0/* */,0x05D1/* */,0x05D2/* */,0x05D3/* */,0x05D4/* */,0x05D5/* */,0x05D6/* */,0x05D7/* */,
    0x05D8/* */,0x05D9/* */,0x05DA/* */,0x05DB/* */,0x05DC/* */,0x05DD/* */,0x05DE/* */,0x05DF/* */,
    0x05E0/* */,0x05E1/* */,0x05E2/* */,0x05E3/* */,0x05E4/* */,0x05E5/* */,0x05E6/* */,0x05E7/* */,
    0x05E8/* */,0x05E9/* */,0x05EA/* */,0x00BC/* */,0x007C/*|*/,0x00BE/* */,0x00F7/* */,0x002A/***/,

    //VT_SWEDISH_CODE_PAGE
    0x0020/* */,0x0021/*!*/,0x0022/*"*/,0x0023/*#*/,0x0024/*$*/,0x0025/*%*/,0x0026/*&*/,0x0027/*'*/,
    0x0028/*(*/,0x0029/*)*/,0x002A/***/,0x002B/*+*/,0x002C/*,*/,0x002D/*-*/,0x002E/*.*/,0x002F/*/*/,
    0x0030/*0*/,0x0031/*1*/,0x0032/*2*/,0x0033/*3*/,0x0034/*4*/,0x0035/*5*/,0x0036/*6*/,0x0037/*7*/,
    0x0038/*8*/,0x0039/*9*/,0x003A/*:*/,0x003B/*;*/,0x003C/*<*/,0x003D/*=*/,0x003E/*>*/,0x003F/*?*/,
    0x00A7/* */,0x0041/*A*/,0x0042/*B*/,0x0043/*C*/,0x0044/*D*/,0x0045/*E*/,0x0046/*F*/,0x0047/*G*/,
    0x0048/*H*/,0x0049/*I*/,0x004A/*J*/,0x004B/*K*/,0x004C/*L*/,0x004D/*M*/,0x004E/*N*/,0x004F/*O*/,
    0x0050/*P*/,0x0051/*Q*/,0x0052/*R*/,0x0053/*S*/,0x0054/*T*/,0x0055/*U*/,0x0056/*V*/,0x0057/*W*/,
    0x0058/*X*/,0x0059/*Y*/,0x005A/*Z*/,0x00C4/* */,0x00D6/* */,0x00C5/* */,0x005E/*^*/,0x005F/*_*/,
    0x00B0/* */,0x0061/*a*/,0x0062/*b*/,0x0063/*c*/,0x0064/*d*/,0x0065/*e*/,0x0066/*f*/,0x0067/*g*/,
    0x0068/*h*/,0x0069/*i*/,0x006A/*j*/,0x006B/*k*/,0x006C/*l*/,0x006D/*m*/,0x006E/*n*/,0x006F/*o*/,
    0x0070/*p*/,0x0071/*q*/,0x0072/*r*/,0x0073/*s*/,0x0074/*t*/,0x0075/*u*/,0x0076/*v*/,0x0077/*w*/,
    0x0078/*x*/,0x0079/*y*/,0x007A/*z*/,0x00E4/* */,0x00F6/* */,0x00E5/* */,0x00DF/* */,0x002A/***/,

    //VT_ITALIAN_CODE_PAGE
    0x0020/* */,0x0021/*!*/,0x0022/*"*/,0x00A3/* */,0x0024/*$*/,0x0025/*%*/,0x0026/*&*/,0x0027/*'*/,
    0x0028/*(*/,0x0029/*)*/,0x002A/***/,0x002B/*+*/,0x002C/*,*/,0x002D/*-*/,0x002E/*.*/,0x002F/*/*/,
    0x0030/*0*/,0x0031/*1*/,0x0032/*2*/,0x0033/*3*/,0x0034/*4*/,0x0035/*5*/,0x0036/*6*/,0x0037/*7*/,
    0x0038/*8*/,0x0039/*9*/,0x003A/*:*/,0x003B/*;*/,0x003C/*<*/,0x003D/*=*/,0x003E/*>*/,0x003F/*?*/,
    0x00E9/* */,0x0041/*A*/,0x0042/*B*/,0x0043/*C*/,0x0044/*D*/,0x0045/*E*/,0x0046/*F*/,0x0047/*G*/,
    0x0048/*H*/,0x0049/*I*/,0x004A/*J*/,0x004B/*K*/,0x004C/*L*/,0x004D/*M*/,0x004E/*N*/,0x004F/*O*/,
    0x0050/*P*/,0x0051/*Q*/,0x0052/*R*/,0x0053/*S*/,0x0054/*T*/,0x0055/*U*/,0x0056/*V*/,0x0057/*W*/,
    0x0058/*X*/,0x0059/*Y*/,0x005A/*Z*/,0x00B0/* */,0x00E7/* */,0x2192/* */,0x2191/* */,0x0023/*#*/,
    0x00F9/* */,0x0061/*a*/,0x0062/*b*/,0x0063/*c*/,0x0064/*d*/,0x0065/*e*/,0x0066/*f*/,0x0067/*g*/,
    0x0068/*h*/,0x0069/*i*/,0x006A/*j*/,0x006B/*k*/,0x006C/*l*/,0x006D/*m*/,0x006E/*n*/,0x006F/*o*/,
    0x0070/*p*/,0x0071/*q*/,0x0072/*r*/,0x0073/*s*/,0x0074/*t*/,0x0075/*u*/,0x0076/*v*/,0x0077/*w*/,
    0x0078/*x*/,0x0079/*y*/,0x007A/*z*/,0x00E0/* */,0x00F2/* */,0x00E8/* */,0x00EC/* */,0x002A/***/,
};


WORD VTCharToUnicode(eVTCodePage VTCharSet, int VTChar)
{
    return vtconv[VTCharSet][VTChar];
}


