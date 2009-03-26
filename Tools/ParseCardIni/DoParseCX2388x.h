/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 Torsten Seeboth. All rights reserved.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_DOPARSECX2388X_H__663635B9_03D0_4F42_8167_D0243FF87D78__INCLUDED_)
#define AFX_DOPARSECX2388X_H__663635B9_03D0_4F42_8167_D0243FF87D78__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define CX_INPUTS_PER_CARD        9
#define CX_AUTODETECT_ID_PER_CARD 3

#include "DoParse.h"

class CDoParseCX2388x : public CDoParse
{
public:
    CDoParseCX2388x(const char* pszFileName, CListBox* pErrorList, CTreeCtrl* pTreeCtrl, BOOL bSort);
    virtual ~CDoParseCX2388x();

private:
    void ParseIniFile();
    void DisplayCardsInTree();


/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// Parsing                                                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

private:

    enum eCardMode
    {
        MODE_STANDARD = 0,
        MODE_H3D,
    };

    enum eInputType
    {
        INPUTTYPE_TUNER = 0,
        INPUTTYPE_COMPOSITE,
        INPUTTYPE_SVIDEO,
        INPUTTYPE_CCIR,
        INPUTTYPE_COLOURBARS,
        INPUTTYPE_FINAL,
    };

    typedef struct
    {
        DWORD GPIO_0;
        DWORD GPIO_1;
        DWORD GPIO_2;
        DWORD GPIO_3;
    } TGPIOSet;

    typedef struct
    {
        char       szName[64];
        eInputType InputType;
        BYTE       MuxSelect;
        TGPIOSet   GPIOSet;
    } TInputType;

    typedef struct
    {
        char            szName[128];
        int             NumInputs;
        TInputType      Inputs[CX_INPUTS_PER_CARD];
        eTunerId        TunerId;
        DWORD           AutoDetectId[CX_AUTODETECT_ID_PER_CARD];
        BOOL            bUseTDA9887;
        eCardMode       CardMode;
    } TCardType;

    class CCardTypeEx : public TCardType
    {
    public:
        std::vector<TTDA9887FormatModes> tda9887Modes;
        CCardTypeEx() { };
        CCardTypeEx(const TCardType& card) : TCardType(card) { };
    };

    typedef struct
    {
        std::vector<CCardTypeEx>*   pCardList;
        CCardTypeEx*                pCurrentCard;
        size_t                      nGoodCards;
        HCParser::CHCParser*        pHCParser;
        TParseTunerInfo             tunerInfo;
        TParseUseTDA9887Info        useTDA9887Info;
    } TParseCardInfo;

public:

    static void ReadCardInputInfoProc(int, const HCParser::CParseTag*, unsigned char, const HCParser::CParseValue*, void*);
    static void ReadCardInputProc(int, const HCParser::CParseTag*, unsigned char, const HCParser::CParseValue*, void*);
    static void ReadCardUseTDA9887Proc(int, const HCParser::CParseTag*, unsigned char, const HCParser::CParseValue*, void*);
    static void ReadCardDefaultTunerProc(int, const HCParser::CParseTag*, unsigned char, const HCParser::CParseValue*, void*);
    static void ReadCardInfoProc(int, const HCParser::CParseTag*, unsigned char, const HCParser::CParseValue*, void*);
    static void ReadCardAutoDetectIDProc(int, const HCParser::CParseTag*, unsigned char, const HCParser::CParseValue*, void*);
    static void ReadCardProc(int, const HCParser::CParseTag*, unsigned char, const HCParser::CParseValue*, void*);

private:
    static std::vector<CCardTypeEx> m_Cards;

    static const HCParser::CParseConstant k_parseInputTypeConstants[];
    static const HCParser::CParseConstant k_parseCardModeConstants[];

    static const HCParser::CParseTag k_parseCardGPIOSetConstants[];
    static const HCParser::CParseTag k_parseCardInput[];
    static const HCParser::CParseTag k_parseCardAutoDetectID[];
    static const HCParser::CParseTag k_parseCard[];
    static const HCParser::CParseTag k_parseCardList[];
};

#endif // !defined(AFX_DOPARSECX2388X_H__663635B9_03D0_4F42_8167_D0243FF87D78__INCLUDED_)
