/////////////////////////////////////////////////////////////////////////////
// $Id: DoParseSAA713x.h,v 1.1 2005-10-28 16:43:13 to_see Exp $
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
// CVS Log
//
// $Log: not supported by cvs2svn $
//////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_DOPARSESAA713X_H__CF8E32C4_00C7_4C5E_9F09_0BFF92F06D5B__INCLUDED_)
#define AFX_DOPARSESAA713X_H__CF8E32C4_00C7_4C5E_9F09_0BFF92F06D5B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define SA_INPUTS_PER_CARD        7
#define SA_AUTODETECT_ID_PER_CARD 3

#include "DoParse.h"

class CDoParseSAA713x : public CDoParse  
{
public:
	CDoParseSAA713x(const char* pszFileName, CListBox* pErrorList, CTreeCtrl* pTreeCtrl, BOOL bSort);
	virtual ~CDoParseSAA713x();

private:
	void ParseIniFile();
	void DisplayCardsInTree();


/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// Parsing                                                                 //
//                                                                         // 
/////////////////////////////////////////////////////////////////////////////

private:

	enum eInputType
	{
		INPUTTYPE_COMPOSITE,
		INPUTTYPE_SVIDEO,
		INPUTTYPE_TUNER,
		INPUTTYPE_CCIR,
		INPUTTYPE_RADIO,
		INPUTTYPE_MUTE,
		INPUTTYPE_FINAL,
	};

	enum eVideoInputSource
	{
		VIDEOINPUTSOURCE_NONE =	-1,		// reserved	for	radio
		VIDEOINPUTSOURCE_PIN0 =	0,
		VIDEOINPUTSOURCE_PIN1,
		VIDEOINPUTSOURCE_PIN2,
		VIDEOINPUTSOURCE_PIN3,
		VIDEOINPUTSOURCE_PIN4,
	};

    enum eAudioInputSource
    {
        AUDIOINPUTSOURCE_NONE = -1,		// reserved, used in INPUTTYPE_FINAL
        AUDIOINPUTSOURCE_LINE1 = 0,
        AUDIOINPUTSOURCE_LINE2 = 1,
        AUDIOINPUTSOURCE_DAC = 2,
    };

	enum eAudioCrystal
	{
		AUDIOCRYSTAL_NONE =	0,
		AUDIOCRYSTAL_32110kHz,
		AUDIOCRYSTAL_24576kHz,
	};

	typedef	struct
	{
		char szName[64];
		eInputType InputType;
		eVideoInputSource VideoInputPin;
		eAudioInputSource AudioLineSelect;
		DWORD dwGPIOStatusMask;
		DWORD dwGPIOStatusBits;
	} TInputType;

    enum eSAA7134CardId
    {
        SAA7134CARDID_UNKNOWN = 0,		// The rest are no longer used
    };
	
	typedef	struct
	{
		char szName[128];
		WORD DeviceId;
		int	 NumInputs;
		TInputType Inputs[SA_INPUTS_PER_CARD];
		eTunerId TunerId;
		eAudioCrystal AudioCrystal;
		DWORD dwGPIOMode;
		DWORD AutoDetectId[SA_AUTODETECT_ID_PER_CARD];
		BOOL  bUseTDA9887;
	} TCardType;

	class CCardTypeEx :	public TCardType
	{
	public:
		std::vector<TTDA9887FormatModes> tda9887Modes;

		CCardTypeEx() {	};
		CCardTypeEx(const TCardType& card) : TCardType(card) { };
	};

	typedef	struct
	{
		WORD DeviceId;
		WORD SubSystemVendorId;
		WORD SubSystemId;
		eSAA7134CardId CardId;
	} TAutoDetectSAA7134;

	typedef	struct
	{
		std::vector<CCardTypeEx>*	pCardList;
		CCardTypeEx*				pCurrentCard;
		size_t						nGoodCards;
		HCParser::CHCParser*		pHCParser;
		TParseTunerInfo				tunerInfo;
		TParseUseTDA9887Info		useTDA9887Info;
	} TParseCardInfo;

public:
	static void	ReadCardInputInfoProc(int, const HCParser::CParseTag*, unsigned	char, const	HCParser::CParseValue*,	void*);
	static void	ReadCardInputProc(int, const HCParser::CParseTag*, unsigned	char, const	HCParser::CParseValue*,	void*);
	static void	ReadCardUseTDA9887Proc(int,	const HCParser::CParseTag*,	unsigned char, const HCParser::CParseValue*, void*);
	static void	ReadCardDefaultTunerProc(int, const	HCParser::CParseTag*, unsigned char, const HCParser::CParseValue*, void*);
	static void	ReadCardInfoProc(int, const	HCParser::CParseTag*, unsigned char, const HCParser::CParseValue*, void*);
	static void	ReadCardAutoDetectIDProc(int, const	HCParser::CParseTag*, unsigned char, const HCParser::CParseValue*, void*);
	static void	ReadCardProc(int, const	HCParser::CParseTag*, unsigned char, const HCParser::CParseValue*, void*);

private:
	static std::vector<CCardTypeEx>	m_Cards;

	static const HCParser::CParseConstant k_parseAudioPinConstants[];
	static const HCParser::CParseConstant k_parseInputTypeConstants[];
	static const HCParser::CParseConstant k_parseAudioCrystalConstants[];

	static const HCParser::CParseTag k_parseInputGPIOSet[];
	static const HCParser::CParseTag k_parseCardInput[];
	static const HCParser::CParseTag k_parseAutoDetectID[];
	static const HCParser::CParseTag k_parseCard[];
	static const HCParser::CParseTag k_parseCardList[];

};

#endif // !defined(AFX_DOPARSESAA713X_H__CF8E32C4_00C7_4C5E_9F09_0BFF92F06D5B__INCLUDED_)
