/////////////////////////////////////////////////////////////////////////////
// $Id: CX2388xCard_Types.cpp,v 1.32 2006-10-06 13:35:28 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 John Adcock.  All rights reserved.
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
//
// This code is based on a version of dTV modified by Michael Eskin and
// others at Connexant.  Those parts are probably (c) Connexant 2002
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.31  2004/12/25 22:40:18  to_see
// Changed the card list to an ini file
//
// Revision 1.30  2004/09/29 20:36:02  to_see
// Added Card AverTV303, Thanks to Zbigniew Pluta
//
// Revision 1.29  2004/08/31 17:54:50  to_see
// New entry for PixelView PlayTV Ultra + on chip audio
// Minor fixes
//
// Revision 1.28  2004/08/27 13:12:40  to_see
// Added audio support for Ati Tv Wonder Pro
//
// Revision 1.27  2004/08/26 16:55:56  to_see
// WinTV - cleaned tuner detection
// ATI TV Wonder Pro - corrected GPIO's
//
// Revision 1.26  2004/08/19 20:17:46  to_see
// Changed write order GPIO's for Leadtek Expert card. It's importand to write first to MO_GP3_IO.
//
// Revision 1.25  2004/06/28 20:17:22  to_see
// Added Patch for INPUTTYPE_COMPOSITE in card "Prolink PlayTV HD" from jstwo@...
//
// Revision 1.24  2004/06/19 20:11:59  to_see
// Fix to reset GPIO Mode to Normal Mode
//
// Revision 1.23  2004/03/28 19:34:11  to_see
// added TVFormat for an call to CCX2388xCard::AudioInitxxx
// more on chip sound; BTSC and Nicam for PAL(I)
// set to LOG(2) when an tuner in CCX2388xCard::AutoDetectTuner was found
// added GPIO Settings for Leadtek WinFast TV2000 XP Expert
//
// Revision 1.22  2004/03/10 17:44:03  to_see
// corrected Card inf for "PixelView PlayTV Ultra"
//
// Revision 1.21  2004/03/07 17:34:49  to_see
// moved CCX2388xCard::AutoDetectTuner from CX2388xCard.cpp to CX2388xCard_Types.cpp
// to can use correct sizeof(m_Tuners_Hauppauge_CX2388x_Card)
//
// Revision 1.20  2004/03/07 12:20:12  to_see
// added 2 Cards
// working Nicam-Sound
// Submenus in CX-Card for Soundsettings
// Click in "Autodetect" in "Setup card CX2388x" is now working
// added "Automute if no Tunersignal" in CX2388x Advanced
//
// Revision 1.19  2004/02/21 21:47:06  to_see
// Added AutodetectTuner for Hauppauge
//
// Revision 1.18  2004/01/27 22:48:57  robmuller
// Use correct GPIO settings for TV@nywhere Master.
//
// Revision 1.17  2004/01/19 14:31:20  adcockj
// Test code for Hauppauge cards inspired by Torsten's diagram
//
// Revision 1.16  2004/01/07 10:08:12  adcockj
// Added setting GPIO pins for sound support and added ATI card
//
// Revision 1.15  2004/01/05 13:12:24  adcockj
// Added patch from Lavrenov Dmitrij (midimaker)
//
// Revision 1.14  2003/12/17 08:28:57  adcockj
// Added Leadtek Winfast Expert (Thanks to Tom Zoerner)
//
// Revision 1.13  2003/10/27 10:39:51  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.12  2003/07/18 09:41:23  adcockj
// Added PDI input to holo3d (doesn't yet work)
//
// Revision 1.11  2003/06/17 12:45:19  adcockj
// Added new card for use with PDI
//
// Revision 1.10  2003/03/10 17:43:19  adcockj
// Added support for Hauppauge WinTV PCI-FM : patch from Tom Zoerner
//
// Revision 1.9  2003/02/24 11:19:51  adcockj
// Futher fix for issues with PlayHD
//
// Revision 1.8  2003/02/03 17:25:23  adcockj
// Fixes for PlayHD
//
// Revision 1.7  2002/12/05 17:11:11  adcockj
// Sound fixes
//
// Revision 1.6  2002/12/04 17:43:49  adcockj
// Contrast and Brightness adjustments so that h3d card behaves in expected way
//
// Revision 1.5  2002/11/15 17:10:51  adcockj
// Setting of GPIO pins for MSI TV@nywhere
//
// Revision 1.4  2002/11/03 18:38:32  adcockj
// Fixes for cx2388x and PAL & NTSC
//
// Revision 1.3  2002/11/03 15:54:10  adcockj
// Added cx2388x register tweaker support
//
// Revision 1.2  2002/10/29 16:20:30  adcockj
// Added card setup for MSI TV@nywhere (no work done on sound)
//
// Revision 1.1  2002/10/29 11:05:28  adcockj
// Renamed CT2388x to CX2388x
//
// 
// CVS Log while file was called CT2388xCard_Types.cpp
//
// Revision 1.11  2002/10/27 19:17:25  adcockj
// Fixes for cx2388x - PAL & NTSC tested
//
// Revision 1.10  2002/10/25 14:44:26  laurentg
// filter setup updated to have something working even for SECAM
//
// Revision 1.9  2002/10/24 16:03:00  adcockj
// Minor tidy up
//
// Revision 1.8  2002/10/23 20:26:53  adcockj
// Bug fixes for cx2388x
//
// Revision 1.7  2002/10/23 16:10:50  adcockj
// Fixed some filter setting bugs and added SECAM tests for Laurent
//
// Revision 1.6  2002/10/21 16:07:26  adcockj
// Added H & V delay options for CX2388x cards
//
// Revision 1.5  2002/10/21 07:19:33  adcockj
// Preliminary Support for PixelView XCapture
//
// Revision 1.4  2002/10/17 13:31:37  adcockj
// Give Holo3d different menu and updated settings
//
// Revision 1.3  2002/09/22 17:47:04  adcockj
// Fixes for holo3d
//
// Revision 1.2  2002/09/11 19:33:06  adcockj
// a few tidy ups
//
// Revision 1.1  2002/09/11 18:19:37  adcockj
// Prelimainary support for CX2388x based cards
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file CX2388xCard.cpp CCX2388xCard Implementation (Types)
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "CX2388xCard.h"
#include "CX2388x_Defines.h"
#include "DScaler.h"
#include "DebugLog.h"
#include "HierarchicalConfigParser.h"
#include "ParsingCommon.h"

using namespace HCParser;

static const char* k_CX2388xCardListFilename = "CX2388xCards.ini";

const CCX2388xCard::TCardType CCX2388xCard::m_CX2388xUnknownCard = 
{
    "*Unknown Card*",
	MODE_STANDARD,
    4,
    {
        {
            "Tuner",
            INPUTTYPE_TUNER,
            0,
            { NULL },
        },
        {
            "Composite",
            INPUTTYPE_COMPOSITE,
            1,
            { NULL },
        },
        {
            "S-Video",
            INPUTTYPE_SVIDEO,
            2,
            { NULL },
        },
        {
            "Colour Bars",
            INPUTTYPE_COLOURBARS,
            0,
            { NULL },
        },
    },
    TUNER_PHILIPS_NTSC,
	{ NULL },
	FALSE,
};

std::vector<CCX2388xCard::CCardTypeEx> CCX2388xCard::m_CX2388xCards;

//////////////////////////////////////////////////////////////////////////
// CX2388x card list parsing constants
//////////////////////////////////////////////////////////////////////////
const CParseConstant CCX2388xCard::k_parseInputTypeConstants[] =
{
    PC( "TUNER",      INPUTTYPE_TUNER      ),
    PC( "COMPOSITE",  INPUTTYPE_COMPOSITE  ),
    PC( "SVIDEO",     INPUTTYPE_SVIDEO     ),
    PC( "CCIR",       INPUTTYPE_CCIR       ),
    PC( "COLOURBARS", INPUTTYPE_COLOURBARS ),
    PC( NULL )
};

const CParseConstant CCX2388xCard::k_parseCardModeConstants[] =
{
    PC( "STANDARD",   MODE_STANDARD ),
    PC( "H3D",        MODE_H3D      ),
    PC( NULL )
};

//////////////////////////////////////////////////////////////////////////
// CX2388x card	list parsing values
//////////////////////////////////////////////////////////////////////////
const CParseTag CCX2388xCard::k_parseCardGPIOSet[] =
{
    PT( "GPIO_0", PARSE_NUMERIC, 1, 16, NULL, ReadCardInputInfoProc ),
    PT( "GPIO_1", PARSE_NUMERIC, 0, 16, NULL, ReadCardInputInfoProc ),
    PT( "GPIO_2", PARSE_NUMERIC, 0, 16, NULL, ReadCardInputInfoProc ),
    PT( "GPIO_3", PARSE_NUMERIC, 0, 16, NULL, ReadCardInputInfoProc ),
    PT( NULL )
};

const CParseTag CCX2388xCard::k_parseCardInput[] =
{
    PT( "Name",       PARSE_STRING,   1, 63, NULL,                      ReadCardInputInfoProc ),
    PT( "Type",       PARSE_CONSTANT, 1, 16, k_parseInputTypeConstants, ReadCardInputInfoProc ),
    PT( "MuxSelect",  PARSE_NUMERIC,  1,  1, NULL,                      ReadCardInputInfoProc ),
    PT( "GPIOSet",    PARSE_CHILDREN, 0,  1, k_parseCardGPIOSet,        NULL ),
    PT( NULL )
};

const CParseTag CCX2388xCard::k_parseCardAutoDetectID[] =
{
    PT( "0", PARSE_NUMERIC, 0, 16, NULL, ReadCardAutoDetectIDProc ),
    PT( "1", PARSE_NUMERIC, 0, 16, NULL, ReadCardAutoDetectIDProc ),
    PT( "2", PARSE_NUMERIC, 0, 16, NULL, ReadCardAutoDetectIDProc ),
    PT( NULL )
};

const CParseTag CCX2388xCard::k_parseCard[] =
{
    PT( "Name",           PARSE_STRING,                 1, 127,			       NULL,                     ReadCardInfoProc         ), 
    PT( "CardMode",       PARSE_CONSTANT,               0, 32,                 k_parseCardModeConstants, ReadCardInfoProc         ),
    PT( "DefaultTuner",   PARSE_CONSTANT|PARSE_NUMERIC, 0, 32,                 k_parseTunerConstants,    ReadCardDefaultTunerProc ),
    PT( "AutoDetectID",   PARSE_CHILDREN,               0, 1,				   k_parseCardAutoDetectID,  NULL                     ),
    PT( "Input",          PARSE_CHILDREN,               0, CX_INPUTS_PER_CARD, k_parseCardInput,         ReadCardInputProc        ),
    PT( "Final",          PARSE_CHILDREN,               0, CX_INPUTS_PER_CARD, k_parseCardInput+2,       ReadCardInputProc        ),
    PT( "UseTDA9887",     PARSE_CHILDREN,               0, 1,				   k_parseUseTDA9887,        ReadCardUseTDA9887Proc   ),
    PT( NULL )
};

const CParseTag CCX2388xCard::k_parseCardList[] =
{
    PT( "Card", PARSE_CHILDREN, 0, 1024, k_parseCard, ReadCardProc ),
    PT( NULL )
};

void CCX2388xCard::ReadCardInputInfoProc(int report, const CParseTag* tag, unsigned char,
                                         const CParseValue* value, void* context)
{
    if (report != REPORT_VALUE)
    {
        return;
    }

    TParseCardInfo* parseInfo = (TParseCardInfo*)context;
    TInputType* input = &parseInfo->pCurrentCard->Inputs[parseInfo->pCurrentCard->NumInputs];

    // Name
    if (tag == k_parseCardInput + 0)
    {
        if (*value->GetString() == '\0')
        {
            throw string("\"\" is not a valid name of an input");
        }
        strcpy(input->szName, value->GetString());
    }

    // Input Type
    else if (tag == k_parseCardInput + 1)
    {
        input->InputType = static_cast<eInputType>(value->GetNumber());
    }

    // Mux Select
    else if (tag == k_parseCardInput + 2)
    {
        // 0...3
		int n = value->GetNumber();
        if (n < 0 || n > 3)
        {
            throw string("MuxSelect must be between 0 and 3");
        }

		input->MuxSelect = n;
    }

    // GPIOSet->GPIO_0
    else if (tag == k_parseCardGPIOSet + 0)
    {
		input->GPIOSet.GPIO_0 = static_cast<DWORD>(value->GetNumber());
    }

    // GPIOSet->GPIO_1
    else if (tag == k_parseCardGPIOSet + 1)
    {
		input->GPIOSet.GPIO_1 = static_cast<DWORD>(value->GetNumber());
    }
    
	// GPIOSet->GPIO_2
    else if (tag == k_parseCardGPIOSet + 2)
    {
		input->GPIOSet.GPIO_2 = static_cast<DWORD>(value->GetNumber());
    }

    // GPIOSet->GPIO_3
    else if (tag == k_parseCardGPIOSet + 3)
    {
		input->GPIOSet.GPIO_3 = static_cast<DWORD>(value->GetNumber());
    }
}

void CCX2388xCard::ReadCardInputProc(int report, const CParseTag* tag, unsigned char,
                                     const CParseValue*, void* context)
{
    TParseCardInfo* parseInfo = (TParseCardInfo*)context;

    switch (report)
    {
    case REPORT_OPEN:
        {
            TInputType* input = &parseInfo->pCurrentCard->Inputs[parseInfo->pCurrentCard->NumInputs];
            input->szName[0] = '\0';
            input->InputType = (tag == k_parseCard + 5) ? INPUTTYPE_FINAL : INPUTTYPE_COMPOSITE;
            input->GPIOSet.GPIO_0 = 0;
            input->GPIOSet.GPIO_1 = 0;
            input->GPIOSet.GPIO_2 = 0;
            input->GPIOSet.GPIO_3 = 0;
        }
        break;
    case REPORT_CLOSE:
        parseInfo->pCurrentCard->NumInputs++;
        break;
    }
}

void CCX2388xCard::ReadCardUseTDA9887Proc(int report, const CParseTag* tag, unsigned char type,
                                          const CParseValue* value, void* context)
{
    TParseCardInfo* parseInfo = (TParseCardInfo*)context;

    // Return TRUE means parseInfo->useTDA9887Info is ready.
    if (ReadUseTDA9887Proc(report, tag, type, value, &parseInfo->useTDA9887Info))
    {
        parseInfo->pCurrentCard->bUseTDA9887 = parseInfo->useTDA9887Info.useTDA9887;
        if (parseInfo->pCurrentCard->bUseTDA9887)
        {
            int count = 0;
            int i;
            // Count the number of non-zero masks.
            for (i = 0; i < TDA9887_FORMAT_LASTONE; i++)
            {
                if (parseInfo->useTDA9887Info.tdaModes[i].mask != 0)
                {
                    count++;
                }
            }
            // If there are any non-zero mask.
            if (count > 0)
            {
                // Pre-reserve the correct amount of blocks.
                parseInfo->pCurrentCard->tda9887Modes.clear();
                parseInfo->pCurrentCard->tda9887Modes.reserve(count);

                TTDA9887FormatModes modes;

                // Transfer all data to the vector.
                for (i = 0; i < TDA9887_FORMAT_LASTONE; i++)
                {
                    if (parseInfo->useTDA9887Info.tdaModes[i].mask != 0)
                    {
                        modes.format = (eTDA9887Format)i;
                        modes.mask = parseInfo->useTDA9887Info.tdaModes[i].mask;
                        modes.bits = parseInfo->useTDA9887Info.tdaModes[i].bits;
                        parseInfo->pCurrentCard->tda9887Modes.push_back(modes);
                    }
                }
            }
        }
    }
}

void CCX2388xCard::ReadCardDefaultTunerProc(int report, const CParseTag* tag, unsigned char type,
                                            const CParseValue* value, void* context)
{
    TParseCardInfo* parseInfo = (TParseCardInfo*)context;

    // Return TRUE means parseInfo->tunerInfo is ready.
    if (ReadTunerProc(report, tag, type, value, &parseInfo->tunerInfo))
    {
        parseInfo->pCurrentCard->TunerId = parseInfo->tunerInfo.tunerId;
    }
}

void CCX2388xCard::ReadCardInfoProc(int report, const CParseTag* tag, unsigned char,
                                    const CParseValue* value, void* context)
{
    if (report != REPORT_VALUE)
    {
        return;
    }

    TParseCardInfo* parseInfo = (TParseCardInfo*)context;

    // Name
    if (tag == k_parseCard + 0)
    {
        if (*value->GetString() == '\0')
        {
            throw string("\"\" is not a valid name of a card");
        }
        
        for (size_t i = 0; i < parseInfo->nGoodCards; i++)
        {
            if (stricmp((*parseInfo->pCardList)[i].szName, value->GetString()) == 0)
            {
                throw string("A card was already specified with this name");
            }
        }
		
        strcpy(parseInfo->pCurrentCard->szName, value->GetString());
    }

	// Card Mode
	else if (tag == k_parseCard + 1)
	{
		parseInfo->pCurrentCard->CardMode = static_cast<eCardMode>(value->GetNumber());
	}
}

void CCX2388xCard::ReadCardAutoDetectIDProc(int report, const CParseTag* tag, unsigned char,
                                            const CParseValue* value, void* context)
{
    if (report != REPORT_VALUE)
    {
        return;
    }

    TParseCardInfo* parseInfo = (TParseCardInfo*)context;

    int i = (int)(tag - k_parseCardAutoDetectID);
    parseInfo->pCurrentCard->AutoDetectId[i] = static_cast<DWORD>(value->GetNumber());
}

void CCX2388xCard::ReadCardProc(int report, const CParseTag*, unsigned char, const CParseValue*, void* context)
{
    static TCardType cardDefault = { "", MODE_STANDARD, 0, { 0 }, TUNER_ABSENT, 0, FALSE };
    TParseCardInfo* parseInfo = (TParseCardInfo*)context;

    switch (report)
    {
    case REPORT_OPEN:
        parseInfo->pCardList->push_back(cardDefault);
        parseInfo->pCurrentCard = &(*parseInfo->pCardList)[parseInfo->nGoodCards];
        break;
    case REPORT_CLOSE:
        {
            long finalCount = 0;
            for (int i = 0; i < parseInfo->pCurrentCard->NumInputs; i++)
            {
                if (parseInfo->pCurrentCard->Inputs[i].InputType == INPUTTYPE_FINAL)
                {
                    finalCount++;
                }
            }
            
            if (finalCount > 1)
            {
                throw string("There can only be one input of type FINAL");
            }
            
            if (finalCount == 1)
            {
                int i = parseInfo->pCurrentCard->NumInputs - 1;
                if (parseInfo->pCurrentCard->Inputs[i].InputType != INPUTTYPE_FINAL)
                {
                    throw string("The FINAL input must be after all other inputs");
                }
            }

            parseInfo->nGoodCards++;
            parseInfo->pCurrentCard = NULL;
        }
        break;
    }
}

BOOL APIENTRY CCX2388xCard::ParseErrorProc(HWND hDlg, UINT message, UINT wParam, LPARAM lParam)
{
    TParseCardInfo* parseInfo = (TParseCardInfo*)lParam;

    switch (message)
    {
    case WM_INITDIALOG:
        {
            SetWindowTextA(hDlg, "CX2388x Card List Parsing Error");

            HWND hItem;
            hItem = GetDlgItem(hDlg, IDC_ERROR_MESSAGE);
            if (CHCParser::IsUnicodeOS())
            {
                SetWindowTextW(hItem, parseInfo->pHCParser->GetErrorUnicode().c_str());
            }
            else
            {
                SetWindowTextA(hItem, parseInfo->pHCParser->GetError().c_str());
            }

            ostringstream oss;
            oss << "An error occured while reading CX2388x cards from 'CX2388xCards.ini':";

            hItem = GetDlgItem(hDlg, IDC_TOP_STATIC);
            SetWindowTextA(hItem, oss.str().c_str());

            oss.str("");
            oss << (parseInfo->nGoodCards - 1) << " card(s) were successfully read before this "
                "error.  Although this error is not fatal, if a previously selected CX2388x "
                "card is not among those successfully read, and this error is ignored, a "
                "different card will need to be selected.";

            hItem = GetDlgItem(hDlg, IDC_BOTTOM_STATIC);
            SetWindowTextA(hItem, oss.str().c_str());
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
        case IDCANCEL:
        case IDIGNORE:
            EndDialog(hDlg, IDIGNORE);
            break;
        case IDRETRY:
        case IDABORT:
            EndDialog(hDlg, LOWORD(wParam));
        }
        break;
    }

    return FALSE;
}


BOOL CCX2388xCard::InitializeCX2388xCardList()
{
    InitializeCX2388xUnknownCard();

    CHCParser hcParser(k_parseCardList);

    TParseCardInfo parseInfo;
    parseInfo.pCardList = &m_CX2388xCards;
    parseInfo.pCurrentCard = NULL;
    parseInfo.nGoodCards = 1;
    parseInfo.pHCParser = &hcParser;

    // No need to use ParseLocalFile() because DScaler does SetExeDirectory()
    // at the beginning.
    while (!hcParser.ParseFile(k_CX2388xCardListFilename, (void*)&parseInfo))
    {
        LOG(0, "CX2388x cardlist: %s", hcParser.GetError().c_str());

        INT_PTR iRetVal = DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_PARSE_ERROR),
            NULL, ParseErrorProc, (LPARAM)&parseInfo);
        if (iRetVal == -1)
        {
            MessageBoxA(NULL, "DialogBoxParam(...) returned -1 for loading IDD_PARSE_ERROR.",
                "Critical Error", MB_ICONEXCLAMATION|MB_OK);
            return FALSE;
        }
        if (iRetVal == IDIGNORE)
        {
            break;
        }
        if (iRetVal == IDABORT)
        {
            return FALSE;
        }

        parseInfo.pCurrentCard = NULL;
        parseInfo.nGoodCards = 1;
        parseInfo.pCardList->resize(1);
    }

    // These can be one extra card in the list than the number of nGoodCards.
    if (parseInfo.nGoodCards < parseInfo.pCardList->size())
    {
        parseInfo.pCardList->resize(parseInfo.nGoodCards);
    }

    LOG(1, "CX2388x cardlist: %lu card(s) read", parseInfo.nGoodCards);

    return TRUE;
}

void CCX2388xCard::InitializeCX2388xUnknownCard()
{
    if (m_CX2388xCards.size() == 0)
    {
        m_CX2388xCards.push_back(m_CX2388xUnknownCard);
    }
}

int CCX2388xCard::GetMaxCards()
{
    InitializeCX2388xUnknownCard();
    return m_CX2388xCards.size();
}

int CCX2388xCard::GetCardByName(LPCSTR cardName)
{
    int listSize = GetMaxCards();
    for (int i = 0; i < listSize; i++)
    {
        if (_stricmp(m_CX2388xCards[i].szName, cardName) == 0)
        {
            return i;
        }
    }
    return 0;
}

int CCX2388xCard::GetNumInputs()
{
    // There must be at most one input of type INPUTTYPE_FINAL in a card
    // and that input MUST always be the last.  GetNumInputs() is the
    // only function that compensates for the FINAL input to subtract
    // from the number of actual inputs.

    if (GetFinalInputNumber() != -1)
    {
        return m_CX2388xCards[m_CardType].NumInputs - 1;
    }
    return m_CX2388xCards[m_CardType].NumInputs;
}

LPCSTR CCX2388xCard::GetInputName(int nInput)
{
    if(nInput < m_CX2388xCards[m_CardType].NumInputs && nInput >= 0)
    {
        return m_CX2388xCards[m_CardType].Inputs[nInput].szName;
    }
    return "Error";
}

int CCX2388xCard::GetFinalInputNumber()
{
    if (m_CX2388xCards[m_CardType].NumInputs > 0)
    {
        int i = m_CX2388xCards[m_CardType].NumInputs - 1;
        if (m_CX2388xCards[m_CardType].Inputs[i].InputType == INPUTTYPE_FINAL)
        {
            return i;
        }
    }
    return -1;
}

BOOL CCX2388xCard::IsInputATuner(int nInput)
{
    if(nInput < m_CX2388xCards[m_CardType].NumInputs && nInput >= 0)
    {
        return (m_CX2388xCards[m_CardType].Inputs[nInput].InputType == INPUTTYPE_TUNER);
    }
    else
    {
        return FALSE;
    }
}

void CCX2388xCard::SetVideoSource(int nInput)
{
    if(IsCurCardH3D())
    {
        H3DInputSelect(nInput);
    }
    else
    {
        StandardInputSelect(nInput);
    }
}

const CCX2388xCard::TCardType* CCX2388xCard::GetCardSetup()
{
    return &(m_CX2388xCards[m_CardType]);
}

eCX2388xCardId CCX2388xCard::AutoDetectCardType()
{
    InitializeCX2388xUnknownCard();

    DWORD Id = m_SubSystemId;
    if (Id != 0 && Id != 0xffffffff)
    {
        int ListSize = GetMaxCards();

        for(int i = 0; i < ListSize; i++)
        {
            for(int j = 0; j < CX_AUTODETECT_ID_PER_CARD && m_CX2388xCards[i].AutoDetectId[j] != 0; j++)
            {
                if(m_CX2388xCards[i].AutoDetectId[j] == Id)
                {
                    LOG(0, "CX2388x: Autodetect found %s.", m_CX2388xCards[i].szName);
                    return (eCX2388xCardId)i;
                }
            }
        }
    }
    return CX2388xCARD_UNKNOWN;
}

void CCX2388xCard::StandardInputSelect(int nInput)
{
    // -1 for finishing clean up
    if (nInput == -1)
    {
        nInput = GetFinalInputNumber();
        if (nInput == -1)
        {
            // There are no cleanup specific changes
            return;
        }
    }

    if(nInput >= m_CX2388xCards[m_CardType].NumInputs)
    {
        LOG(1, "Input Select Called for invalid input");
        nInput = m_CX2388xCards[m_CardType].NumInputs - 1;
    }
    
    if(nInput < 0)
    {
        LOG(1, "Input Select Called for invalid input");
        nInput = 0;
    }
    
    m_CurrentInput = nInput;

    TInputType* pInput = &m_CX2388xCards[m_CardType].Inputs[nInput];

    if(pInput->InputType == INPUTTYPE_COLOURBARS)
    {
        // Enable color bars
        OrDataDword(CX2388X_VIDEO_COLOR_FORMAT, 0x00004000);
    }
    else
    {
        // disable color bars
        AndDataDword(CX2388X_VIDEO_COLOR_FORMAT, 0xFFFFBFFF);
    
        // Read and mask the video input register
        DWORD VideoInput = ReadDword(CX2388X_VIDEO_INPUT);
        // zero out mux and svideo bit
        // and force auto detect
        // also turn off CCIR input
        // also VERTEN & SPSPD
        VideoInput &= 0x0F;
        
        // set the Mux up from the card setup
        VideoInput |= (pInput->MuxSelect << CX2388X_VIDEO_INPUT_MUX_SHIFT);

        // set the comp bit for svideo
        switch (pInput->InputType)
        {
            case INPUTTYPE_SVIDEO: // SVideo
                VideoInput |= CX2388X_VIDEO_INPUT_SVID_C_SEL; 
                VideoInput |= CX2388X_VIDEO_INPUT_SVID;

                // Switch chroma DAC to chroma channel
                OrDataDword(MO_AFECFG_IO, 0x00000001);

                break;
            
            case INPUTTYPE_CCIR:
                VideoInput |= CX2388X_VIDEO_INPUT_PE_SRCSEL;
                VideoInput |= CX2388X_VIDEO_INPUT_SVID_C_SEL; 
                break;
        
            case INPUTTYPE_TUNER:
            case INPUTTYPE_COMPOSITE:
            default:
                // Switch chroma DAC to audio
                AndDataDword(MO_AFECFG_IO, 0xFFFFFFFE);
                break;
        }
        
        WriteDword(CX2388X_VIDEO_INPUT, VideoInput);
    }

    TGPIOSet* pGPIOSet = &pInput->GPIOSet;
    WriteDword(MO_GP3_IO, pGPIOSet->GPIO_3);
    WriteDword(MO_GP2_IO, pGPIOSet->GPIO_2);
    WriteDword(MO_GP1_IO, pGPIOSet->GPIO_1);
    WriteDword(MO_GP0_IO, pGPIOSet->GPIO_0);
}

HMENU CCX2388xCard::GetCardSpecificMenu()
{
    if(IsCurCardH3D())
    {
	    return LoadMenu(hResourceInst, MAKEINTRESOURCE(IDC_CX2388X_H3D));
    }
    else
    {
	    return LoadMenu(hResourceInst, MAKEINTRESOURCE(IDC_CX2388X));
    }
}

BOOL CCX2388xCard::IsCurCardH3D()
{
	if(m_CX2388xCards[m_CardType].CardMode == MODE_H3D)
	{
		return TRUE;
	}
    else
    {
        return FALSE;
    }
}

BOOL CCX2388xCard::IsThisCardH3D(eCX2388xCardId CardId)
{
	if(m_CX2388xCards[CardId].CardMode == MODE_H3D)
	{
		return TRUE;
	}
    else
    {
        return FALSE;
    }
}
