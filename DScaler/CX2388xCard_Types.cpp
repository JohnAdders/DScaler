/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file CX2388xCard.cpp CCX2388xCard Implementation (Types)
 */

#include "stdafx.h"

#ifdef WANT_CX2388X_SUPPORT

#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "CX2388xCard.h"
#include "CX2388x_Defines.h"
#include "DScaler.h"
#include "DebugLog.h"
#include "HierarchicalConfigParser.h"
#include "ParsingCommon.h"

using namespace std;

using namespace HCParser;

static const TCHAR* k_CX2388xCardListFilename = _T("CX2388xCards.ini");

const CCX2388xCard::TCardType CCX2388xCard::m_CX2388xUnknownCard =
{
    _T("*Unknown Card*"),
    MODE_STANDARD,
    4,
    {
        {
            _T("Tuner"),
            INPUTTYPE_TUNER,
            0,
            { NULL },
        },
        {
            _T("Composite"),
            INPUTTYPE_COMPOSITE,
            1,
            { NULL },
        },
        {
            _T("S-Video"),
            INPUTTYPE_SVIDEO,
            2,
            { NULL },
        },
        {
            _T("Colour Bars"),
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
    PC( _T("TUNER"),      INPUTTYPE_TUNER      ),
    PC( _T("COMPOSITE"),  INPUTTYPE_COMPOSITE  ),
    PC( _T("SVIDEO"),     INPUTTYPE_SVIDEO     ),
    PC( _T("CCIR"),       INPUTTYPE_CCIR       ),
    PC( _T("COLOURBARS"), INPUTTYPE_COLOURBARS ),
    PC( NULL )
};

const CParseConstant CCX2388xCard::k_parseCardModeConstants[] =
{
    PC( _T("STANDARD"),   MODE_STANDARD ),
    PC( _T("H3D"),        MODE_H3D      ),
    PC( NULL )
};

//////////////////////////////////////////////////////////////////////////
// CX2388x card    list parsing values
//////////////////////////////////////////////////////////////////////////
const CParseTag CCX2388xCard::k_parseCardGPIOSet[] =
{
    PT( _T("GPIO_0"), PARSE_NUMERIC, 1, 16, NULL, ReadCardInputInfoProc ),
    PT( _T("GPIO_1"), PARSE_NUMERIC, 0, 16, NULL, ReadCardInputInfoProc ),
    PT( _T("GPIO_2"), PARSE_NUMERIC, 0, 16, NULL, ReadCardInputInfoProc ),
    PT( _T("GPIO_3"), PARSE_NUMERIC, 0, 16, NULL, ReadCardInputInfoProc ),
    PT( NULL )
};

const CParseTag CCX2388xCard::k_parseCardInput[] =
{
    PT( _T("Name"),       PARSE_STRING,   1, 63, NULL,                      ReadCardInputInfoProc ),
    PT( _T("Type"),       PARSE_CONSTANT, 1, 16, k_parseInputTypeConstants, ReadCardInputInfoProc ),
    PT( _T("MuxSelect"),  PARSE_NUMERIC,  1,  1, NULL,                      ReadCardInputInfoProc ),
    PT( _T("GPIOSet"),    PARSE_CHILDREN, 0,  1, k_parseCardGPIOSet,        NULL ),
    PT( NULL )
};

const CParseTag CCX2388xCard::k_parseCardAutoDetectID[] =
{
    PT( _T("0"), PARSE_NUMERIC, 0, 16, NULL, ReadCardAutoDetectIDProc ),
    PT( _T("1"), PARSE_NUMERIC, 0, 16, NULL, ReadCardAutoDetectIDProc ),
    PT( _T("2"), PARSE_NUMERIC, 0, 16, NULL, ReadCardAutoDetectIDProc ),
    PT( NULL )
};

const CParseTag CCX2388xCard::k_parseCard[] =
{
    PT( _T("Name"),           PARSE_STRING,                 1, 127,                   NULL,                     ReadCardInfoProc         ),
    PT( _T("CardMode"),       PARSE_CONSTANT,               0, 32,                 k_parseCardModeConstants, ReadCardInfoProc         ),
    PT( _T("DefaultTuner"),   PARSE_CONSTANT|PARSE_NUMERIC, 0, 32,                 k_parseTunerConstants,    ReadCardDefaultTunerProc ),
    PT( _T("AutoDetectID"),   PARSE_CHILDREN,               0, 1,                   k_parseCardAutoDetectID,  NULL                     ),
    PT( _T("Input"),          PARSE_CHILDREN,               0, CX_INPUTS_PER_CARD, k_parseCardInput,         ReadCardInputProc        ),
    PT( _T("Final"),          PARSE_CHILDREN,               0, CX_INPUTS_PER_CARD, k_parseCardInput+2,       ReadCardInputProc        ),
    PT( _T("UseTDA9887"),     PARSE_CHILDREN,               0, 1,                   k_parseUseTDA9887,        ReadCardUseTDA9887Proc   ),
    PT( NULL )
};

const CParseTag CCX2388xCard::k_parseCardList[] =
{
    PT( _T("Card"), PARSE_CHILDREN, 0, 1024, k_parseCard, ReadCardProc ),
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
            throw std::exception("\"\" is not a valid name of an input");
        }
        _tcscpy(input->szName, value->GetString());
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
            throw std::exception("MuxSelect must be between 0 and 3");
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
            throw std::exception("\"\" is not a valid name of a card");
        }

        for (size_t i = 0; i < parseInfo->nGoodCards; i++)
        {
            if (_tcsicmp((*parseInfo->pCardList)[i].szName, value->GetString()) == 0)
            {
                throw std::exception("A card was already specified with this name");
            }
        }

        _tcscpy(parseInfo->pCurrentCard->szName, value->GetString());
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
    static TCardType cardDefault = { _T(""), MODE_STANDARD, 0, { 0 }, TUNER_ABSENT, 0, FALSE };
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
                throw std::exception("There can only be one input of type FINAL");
            }

            if (finalCount == 1)
            {
                int i = parseInfo->pCurrentCard->NumInputs - 1;
                if (parseInfo->pCurrentCard->Inputs[i].InputType != INPUTTYPE_FINAL)
                {
                    throw std::exception("The FINAL input must be after all other inputs");
                }
            }

            parseInfo->nGoodCards++;
            parseInfo->pCurrentCard = NULL;
        }
        break;
    }
}

INT_PTR CALLBACK CCX2388xCard::ParseErrorProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    TParseCardInfo* parseInfo = (TParseCardInfo*)lParam;

    switch (message)
    {
    case WM_INITDIALOG:
        {
            SetWindowText(hDlg, _T("CX2388x Card List Parsing Error"));

            HWND hItem;
            hItem = GetDlgItem(hDlg, IDC_ERROR_MESSAGE);
            SetWindowText(hItem, parseInfo->pHCParser->GetError().c_str());

            tostringstream oss;
            oss << _T("An error occured while reading CX2388x cards from 'CX2388xCards.ini':");

            hItem = GetDlgItem(hDlg, IDC_TOP_STATIC);
            SetWindowText(hItem, oss.str().c_str());

            oss.str(_T(""));
            oss << (parseInfo->nGoodCards - 1) << _T(" card(s) were successfully read before this ")
                _T("error.  Although this error is not fatal, if a previously selected CX2388x ")
                _T("card is not among those successfully read, and this error is ignored, a ")
                _T("different card will need to be selected.");

            hItem = GetDlgItem(hDlg, IDC_BOTTOM_STATIC);
            SetWindowText(hItem, oss.str().c_str());
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
        LOG(0, _T("CX2388x cardlist: %s"), hcParser.GetError().c_str());

        INT_PTR iRetVal = DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_PARSE_ERROR),
            NULL, ParseErrorProc, (LPARAM)&parseInfo);
        if (iRetVal == -1)
        {
            MessageBox(NULL, _T("DialogBoxParam(...) returned -1 for loading IDD_PARSE_ERROR."),
                _T("Critical Error"), MB_ICONEXCLAMATION|MB_OK);
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

    LOG(1, _T("CX2388x cardlist: %lu card(s) read"), parseInfo.nGoodCards);

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

int CCX2388xCard::GetCardByName(LPCTSTR cardName)
{
    int listSize = GetMaxCards();
    for (int i = 0; i < listSize; i++)
    {
        if (_tcsicmp(m_CX2388xCards[i].szName, cardName) == 0)
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

tstring CCX2388xCard::GetInputName(int nInput)
{
    if(nInput < m_CX2388xCards[m_CardType].NumInputs && nInput >= 0)
    {
        return m_CX2388xCards[m_CardType].Inputs[nInput].szName;
    }
    return _T("Error");
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
                    LOG(0, _T("CX2388x: Autodetect found %s."), m_CX2388xCards[i].szName);
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
        LOG(1, _T("Input Select Called for invalid input"));
        nInput = m_CX2388xCards[m_CardType].NumInputs - 1;
    }

    if(nInput < 0)
    {
        LOG(1, _T("Input Select Called for invalid input"));
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

#endif // WANT_CX2388X_SUPPORT
