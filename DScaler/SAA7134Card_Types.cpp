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
//  GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
//
// This software was based on v4l2 device driver for philips
// saa7134 based TV cards.  Those portions are
// Copyright (c) 2001,02 Gerd Knorr <kraxel@bytesex.org> [SuSE Labs]
//
// This software was based on BT848Card_Types.cpp.  Those portions are
// Copyright (c) 2001 John Adcock.
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file SAA7134Card.cpp CSAA7134Card Implementation (Types)
 */

#include "stdafx.h"

#ifdef WANT_SAA713X_SUPPORT

#include "DScaler.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "SAA7134Card.h"
#include "SAA7134_Defines.h"
#include "DebugLog.h"
#include "HierarchicalConfigParser.h"
#include "ParsingCommon.h"

using namespace std;

using namespace HCParser;

static const TCHAR* k_SAA713xCardListFilename = _T("SAA713xCards.ini");


//////////////////////////////////////////////////////////////////////////
// Default generic prefab card for the card list
//////////////////////////////////////////////////////////////////////////
const CSAA7134Card::TCardType CSAA7134Card::m_SAA7134UnknownCard =
{
    // SAA7134CARDID_UNKNOWN - Unknown Card
    _T("*Unknown Card*"),
    0x0000,
    4,
    {
        {
            _T("Tuner"),
            INPUTTYPE_TUNER,
            VIDEOINPUTSOURCE_PIN1,
            AUDIOINPUTSOURCE_DAC,
        },
        {
            _T("Composite"),
            INPUTTYPE_COMPOSITE,
            VIDEOINPUTSOURCE_PIN3,
            AUDIOINPUTSOURCE_LINE1,
        },
        {
            _T("S-Video"),
            INPUTTYPE_SVIDEO,
            VIDEOINPUTSOURCE_PIN0,
            AUDIOINPUTSOURCE_LINE1,
        },
        {
            _T("Composite over S-Video"),
            INPUTTYPE_COMPOSITE,
            VIDEOINPUTSOURCE_PIN0,
            AUDIOINPUTSOURCE_LINE1,
        },
    },
    TUNER_ABSENT,
    AUDIOCRYSTAL_32110kHz,
    0,
};

std::vector<CSAA7134Card::CCardTypeEx> CSAA7134Card::m_SAA713xCards;


//////////////////////////////////////////////////////////////////////////
// SAA713x card list parsing constants
//////////////////////////////////////////////////////////////////////////
const CParseConstant CSAA7134Card::k_parseAudioPinConstants[] =
{
    PC(    _T("NONE"),            AUDIOINPUTSOURCE_NONE    ),
    PC(    _T("LINE1"),        AUDIOINPUTSOURCE_LINE1    ),
    PC(    _T("LINE2"),        AUDIOINPUTSOURCE_LINE2    ),
    PC(    _T("DAC"),            AUDIOINPUTSOURCE_DAC    ),
    PC(    NULL )
};

const CParseConstant CSAA7134Card::k_parseInputTypeConstants[] =
{
    PC(    _T("COMPOSITE"),    INPUTTYPE_COMPOSITE        ),
    PC(    _T("SVIDEO"),        INPUTTYPE_SVIDEO        ),
    PC(    _T("TUNER"),        INPUTTYPE_TUNER            ),
    PC(    _T("CCIR"),            INPUTTYPE_CCIR            ),
    PC(    _T("RADIO"),        INPUTTYPE_RADIO            ),
// MUTE    input was defined but not currently    used by    CSAA7134Card.
//    PC(    _T("MUTE"),            INPUTTYPE_MUTE            ),
// FINAL isn't necessary because Final() does the same thing.
//    PC(    _T("FINAL"),        INPUTTYPE_FINAL        ),
    PC(    NULL )
};

const CParseConstant CSAA7134Card::k_parseAudioCrystalConstants[] =
{
    PC(    _T("NONE"),            AUDIOCRYSTAL_NONE        ),
    PC(    _T("32MHz"),        AUDIOCRYSTAL_32110kHz    ),
    PC(    _T("24MHz"),        AUDIOCRYSTAL_24576kHz    ),
    PC(    NULL )
};

//////////////////////////////////////////////////////////////////////////
// SAA713x card    list parsing values
//////////////////////////////////////////////////////////////////////////
const CParseTag    CSAA7134Card::k_parseInputGPIOSet[]    =
{
    PT(    _T("Bits"),                PARSE_NUMERIC,        1,    16,    NULL,                            ReadCardInputInfoProc        ),
    PT(    _T("Mask"),                PARSE_NUMERIC,        1,    16,    NULL,                            ReadCardInputInfoProc        ),
    PT(    NULL )
};

const CParseTag    CSAA7134Card::k_parseCardInput[] =
{
    PT(    _T("Name"),                PARSE_STRING,        1, 63,    NULL,                            ReadCardInputInfoProc        ),
    PT(    _T("Type"),                PARSE_CONSTANT,        1, 16,    k_parseInputTypeConstants,        ReadCardInputInfoProc        ),
    PT(    _T("VideoPin"),            PARSE_NUMERIC,        0, 8,    NULL,                            ReadCardInputInfoProc        ),
    PT(    _T("AudioPin"),            PARSE_NUM_OR_CONST,    0, 8,    k_parseAudioPinConstants,        ReadCardInputInfoProc        ),
    PT(    _T("GPIOSet"),            PARSE_CHILDREN,        0, 1,    k_parseInputGPIOSet,            NULL                        ),
    PT(    NULL )
};

const CParseTag    CSAA7134Card::k_parseAutoDetectID[]    =
{
    PT(    _T("0"),                PARSE_NUMERIC,        0, 16,    NULL,                            ReadCardAutoDetectIDProc    ),
    PT(    _T("1"),                PARSE_NUMERIC,        0, 16,    NULL,                            ReadCardAutoDetectIDProc    ),
    PT(    _T("2"),                PARSE_NUMERIC,        0, 16,    NULL,                            ReadCardAutoDetectIDProc    ),
    PT(    NULL )
};

const CParseTag    CSAA7134Card::k_parseCard[]    =
{
    PT(    _T("Name"),                PARSE_STRING,        1, 127,    NULL,                            ReadCardInfoProc            ),
    PT(    _T("DeviceID"),            PARSE_NUMERIC,        1, 8,    NULL,                            ReadCardInfoProc            ),
    PT(    _T("DefaultTuner"),        PARSE_NUM_OR_CONST,    0, 32,    k_parseTunerConstants,            ReadCardDefaultTunerProc    ),
    PT(    _T("AudioCrystal"),        PARSE_CONSTANT,        0, 8,    k_parseAudioCrystalConstants,    ReadCardInfoProc            ),
    PT(    _T("GPIOMask"),            PARSE_NUMERIC,        0, 16,    NULL,                            ReadCardInfoProc            ),
    PT(    _T("AutoDetectID"),        PARSE_CHILDREN,        0, 1,    k_parseAutoDetectID,            NULL                        ),
    PT(    _T("Input"),            PARSE_CHILDREN,        0, 7,    k_parseCardInput,                ReadCardInputProc            ),
    PT(    _T("Final"),            PARSE_CHILDREN,        0, 7,    k_parseCardInput+2,                ReadCardInputProc            ),
    PT(    _T("UseTDA9887"),        PARSE_CHILDREN,        0, 1,    k_parseUseTDA9887,                ReadCardUseTDA9887Proc        ),
    PT(    NULL )
};

const CParseTag    CSAA7134Card::k_parseCardList[]    =
{
    PT(    _T("Card"),                PARSE_CHILDREN,        0, 512,    k_parseCard,                    ReadCardProc                ),
    PT(    NULL )
};


BOOL CSAA7134Card::InitializeSAA713xCardList()
{
    InitializeSAA713xUnknownCard();

    CHCParser hcParser(k_parseCardList);

    TParseCardInfo parseInfo;
    parseInfo.pCardList = &m_SAA713xCards;
    parseInfo.pCurrentCard = NULL;
    parseInfo.nGoodCards = 1;
    parseInfo.pHCParser = &hcParser;

    // No need to use ParseLocalFile() because DScaler does SetExeDirectory()
    // at the beginning.
    while (!hcParser.ParseFile(k_SAA713xCardListFilename, (void*)&parseInfo))
    {
        LOG(0, _T("SAA713x cardlist: %s"), hcParser.GetError().c_str());

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

    LOG(1, _T("SAA713x cardlist: %lu card(s) read"), parseInfo.nGoodCards);

    return TRUE;
}


void CSAA7134Card::InitializeSAA713xUnknownCard()
{
    if (m_SAA713xCards.size() == 0)
    {
        m_SAA713xCards.push_back(m_SAA7134UnknownCard);
    }
}


void CSAA7134Card::ReadCardInputInfoProc(int report, const CParseTag* tag, unsigned char,
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
    // Type
    else if (tag == k_parseCardInput + 1)
    {
        input->InputType = static_cast<eInputType>(value->GetNumber());
    }
    // VideoPin
    else if (tag == k_parseCardInput + 2)
    {
        int n = value->GetNumber();
        if (n < VIDEOINPUTSOURCE_NONE || n > VIDEOINPUTSOURCE_PIN4)
        {
            throw std::exception("VideoPin must be between -1 and 4");
        }
        input->VideoInputPin = static_cast<eVideoInputSource>(n);
    }
    // AudioPin
    else if (tag == k_parseCardInput + 3)
    {
        int n = value->GetNumber();
        if (n < AUDIOINPUTSOURCE_NONE || n > AUDIOINPUTSOURCE_DAC)
        {
            throw std::exception("AudioPin must be between -1 and 2");
        }
        input->AudioLineSelect = static_cast<eAudioInputSource>(n);
    }
    // GPIOSet->Bits
    else if (tag == k_parseInputGPIOSet + 0)
    {
        input->dwGPIOStatusBits = static_cast<DWORD>(value->GetNumber());
    }
    // GPIOSet->Mask
    else if (tag == k_parseInputGPIOSet + 1)
    {
        input->dwGPIOStatusMask = static_cast<DWORD>(value->GetNumber());
    }
}


void CSAA7134Card::ReadCardInputProc(int report, const CParseTag* tag, unsigned char,
                                     const CParseValue*, void* context)
{
    TParseCardInfo* parseInfo = (TParseCardInfo*)context;

    switch (report)
    {
    case REPORT_OPEN:
        {
            TInputType* input = &parseInfo->pCurrentCard->Inputs[parseInfo->pCurrentCard->NumInputs];
            input->szName[0] = '\0';
            input->InputType = (tag == k_parseCard + 7) ? INPUTTYPE_FINAL : INPUTTYPE_COMPOSITE;
            input->VideoInputPin = VIDEOINPUTSOURCE_NONE;
            input->AudioLineSelect = AUDIOINPUTSOURCE_NONE;
            input->dwGPIOStatusMask = 0;
            input->dwGPIOStatusBits = 0;
        }
        break;
    case REPORT_CLOSE:
        parseInfo->pCurrentCard->NumInputs++;
        break;
    }
}

void CSAA7134Card::ReadCardUseTDA9887Proc(int report, const CParseTag* tag, unsigned char type,
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
            // Count the number of non-zero masks.
            int i;
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

void CSAA7134Card::ReadCardDefaultTunerProc(int report, const CParseTag* tag, unsigned char type,
                                            const CParseValue* value, void* context)
{
    TParseCardInfo* parseInfo = (TParseCardInfo*)context;

    // Return TRUE means parseInfo->tunerInfo is ready.
    if (ReadTunerProc(report, tag, type, value, &parseInfo->tunerInfo))
    {
        if (parseInfo->tunerInfo.tunerId == TUNER_AUTODETECT ||
            parseInfo->tunerInfo.tunerId == TUNER_USER_SETUP)
        {
            throw std::exception("Tuner id \"auto\" and \"setup\" are not supported.");
        }
        parseInfo->pCurrentCard->TunerId = parseInfo->tunerInfo.tunerId;
    }
}


void CSAA7134Card::ReadCardInfoProc(int report, const CParseTag* tag, unsigned char,
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
    // DeviceID
    else if (tag == k_parseCard + 1)
    {
        int n = value->GetNumber();
        if (n != 0x7130 && n != 0x7134 && n != 0x7133)
        {
            throw std::exception("DeviceID needs to be either 0x7134 or 0x7130");
        }
        parseInfo->pCurrentCard->DeviceId = static_cast<WORD>(n);
    }
    // AudioCrystal
    else if (tag == k_parseCard + 3)
    {
        parseInfo->pCurrentCard->AudioCrystal =
            static_cast<eAudioCrystal>(value->GetNumber());
    }
    // GPIOMask
    else if (tag == k_parseCard + 4)
    {
        parseInfo->pCurrentCard->dwGPIOMode = static_cast<DWORD>(value->GetNumber());
    }
}

void CSAA7134Card::ReadCardAutoDetectIDProc(int report, const CParseTag* tag, unsigned char,
                                            const CParseValue* value, void* context)
{
    if (report != REPORT_VALUE)
    {
        return;
    }

    TParseCardInfo* parseInfo = (TParseCardInfo*)context;

    int i = (int)(tag - k_parseAutoDetectID);
    parseInfo->pCurrentCard->AutoDetectId[i] = static_cast<DWORD>(value->GetNumber());
}


void CSAA7134Card::ReadCardProc(int report, const CParseTag*, unsigned char, const CParseValue*, void* context)
{
    static TCardType cardDefaults = { _T(""), 0x0000, 0, { 0 }, TUNER_ABSENT, AUDIOCRYSTAL_NONE, 0, { 0, 0, 0 }, FALSE };
    TParseCardInfo* parseInfo = (TParseCardInfo*)context;

    switch (report)
    {
    case REPORT_OPEN:
        parseInfo->pCardList->push_back(cardDefaults);
        parseInfo->pCurrentCard = &(*parseInfo->pCardList)[parseInfo->nGoodCards];
        break;
    case REPORT_CLOSE:
        {
            if (parseInfo->pCurrentCard->DeviceId == 0x7130)
            {
                if (parseInfo->pCurrentCard->AudioCrystal != AUDIOCRYSTAL_NONE)
                {
                    throw std::exception("AudioCrystal for a 0x7130 card must be NONE");
                }
            }

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


BOOL APIENTRY CSAA7134Card::ParseErrorProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    TParseCardInfo* parseInfo = (TParseCardInfo*)lParam;

    switch (message)
    {
    case WM_INITDIALOG:
        {
            SetWindowText(hDlg, _T("SAA713x Card List Parsing Error"));

            HWND hItem;
            hItem = GetDlgItem(hDlg, IDC_ERROR_MESSAGE);
            SetWindowText(hItem, parseInfo->pHCParser->GetError().c_str());

            tostringstream oss;
            oss << _T("An error occured while reading SAA713x cards from 'SAA713xCards.ini':");

            hItem = GetDlgItem(hDlg, IDC_TOP_STATIC);
            SetWindowText(hItem, oss.str().c_str());

            oss.str(_T(""));
            oss << (parseInfo->nGoodCards - 1) << _T(" card(s) were successfully read before this ")
                _T("error.  Although this error is not fatal, if a previously selected SAA713x ")
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

//
// Notes:
//
// _T("Might req mode 6"): S-Video is listed with VIDEOINPUTSOURCE_PIN0 but what
// is actually used is not mode 0 but mode 8.  --This is due to an old design
// decision that I no longer remember why.  Mode 6 is exactly the same as mode
// 8 except the C-channel gain control is set with a register instead of
// automatic gain control that is linked to the Y-channel. _T("Might req mode 6")
// has been placed beside entries where the RegSpy dump showed the
// SAA7134_ANALOG_IN_CTRL1 register with xxxx0110(6) instead of xxxx1000(8).
//

//
// LifeView Clones:  (Actually, I don't know who supplies who)
//
//              0x7130                      0x7134                          0x7133
//
// 0x01384e42   Dazzle My TV                LifeView FlyVideo3000
//                                          Chronos Video Shuttle II
//
//
// 0x01385168   LifeView FlyVideo2000       LifeView PrimeTV 7134     PrimeTV 7133
//              Chronos Video Shuttle II
//
// 0x013819d0   V-Gear MyTV2 Radio
//
//
// Notes:
// - The auto detect ID 0x01384e42 is not used by LifeView FlyVideo3000 that I know
//   of.  This ID comes from the SAA7134 version of Chronos Video Shuttle II.
// - All cards above have an identical video input pin configuration.  They also use
//   the same 0x0018e700 GPIO mask.
// - _T("Genius Video Wonder PRO III") also has the ID 0x01385168 and is 0x7134 but the
//   audio clock is different.
//


int CSAA7134Card::GetMaxCards()
{
    InitializeSAA713xUnknownCard();
    return m_SAA713xCards.size();
}


int CSAA7134Card::GetCardByName(LPCTSTR cardName)
{
    int listSize = GetMaxCards();
    for (int i = 0; i < listSize; i++)
    {
        if (_tcsicmp(m_SAA713xCards[i].szName, cardName) == 0)
        {
            return i;
        }
    }
    return 0;
}


CSAA7134Card::eSAA7134CardId CSAA7134Card::AutoDetectCardType()
{
    WORD DeviceId           = GetDeviceId();
    DWORD SubSystemId        = GetSubSystemId();

    InitializeSAA713xUnknownCard();

    if (SubSystemId == 0x00001131)
    {
       LOG(0, _T("SAA713x: Autodetect found [0x00001131] *Unknown Card*."));
        return SAA7134CARDID_UNKNOWN;
    }

    int ListSize = GetMaxCards();

    for (int i = 0; i < ListSize; i++)
    {
        for (int j = 0; j < SA_AUTODETECT_ID_PER_CARD && m_SAA713xCards[i].AutoDetectId[j] != 0; j++)
        {
            if (m_SAA713xCards[i].DeviceId == DeviceId &&
                m_SAA713xCards[i].AutoDetectId[j] == SubSystemId)
            {
                LOG(0, _T("SAA713x: Autodetect found %s."), m_SAA713xCards[i].szName);
                return (eSAA7134CardId)i;
            }
        }
    }

    LOG(0, _T("SAA713x: Autodetect found an unknown card with the following"));
    LOG(0, _T("SAA713x: properties.  Please email the author and quote the"));
    LOG(0, _T("SAA713x: following numbers, as well as which card you have,"));
    LOG(0, _T("SAA713x: so it can be added to the list:"));
    LOG(0, _T("SAA713x: DeviceId: 0x%04x, AutoDetectId: 0x%08x"),
        DeviceId, SubSystemId);

    return SAA7134CARDID_UNKNOWN;
}


eTunerId CSAA7134Card::AutoDetectTuner(eSAA7134CardId CardId)
{
    return m_SAA713xCards[CardId].TunerId;
}


void CSAA7134Card::SetCardType(int CardType)
{
    InitializeSAA713xUnknownCard();

    if (m_CardType != CardType)
    {
        m_CardType = (eSAA7134CardId)CardType;
    }
}


CSAA7134Card::eSAA7134CardId CSAA7134Card::GetCardType()
{
    return m_CardType;
}


int CSAA7134Card::GetNumInputs()
{
    // There must be at most one input of type INPUTTYPE_FINAL in a card
    // and that input MUST always be the last.  GetNumInputs() is the
    // only function that compensates for the FINAL input to subtract
    // from the number of actual inputs.

    if (GetFinalInputNumber() != -1)
    {
        return m_SAA713xCards[m_CardType].NumInputs - 1;
    }
    return m_SAA713xCards[m_CardType].NumInputs;
}


tstring CSAA7134Card::GetInputName(int nInput)
{
    if (nInput >= m_SAA713xCards[m_CardType].NumInputs || nInput < 0)
    {
        return _T("Error");
    }
    return m_SAA713xCards[m_CardType].Inputs[nInput].szName;
}


int CSAA7134Card::GetFinalInputNumber()
{
    if (m_SAA713xCards[m_CardType].NumInputs > 0)
    {
        int i = m_SAA713xCards[m_CardType].NumInputs - 1;
        if (m_SAA713xCards[m_CardType].Inputs[i].InputType == INPUTTYPE_FINAL)
        {
            return i;
        }
    }
    return -1;
}


BOOL CSAA7134Card::IsInputATuner(int nInput)
{
    return (m_SAA713xCards[m_CardType].Inputs[nInput].InputType == INPUTTYPE_TUNER);
}


BOOL CSAA7134Card::IsCCIRSource(int nInput)
{
    return (m_SAA713xCards[m_CardType].Inputs[nInput].InputType == INPUTTYPE_CCIR);
}


tstring CSAA7134Card::GetCardName(eSAA7134CardId CardId)
{
    return m_SAA713xCards[CardId].szName;
}


WORD CSAA7134Card::GetCardDeviceId(eSAA7134CardId CardId)
{
    return m_SAA713xCards[CardId].DeviceId;
}


void CSAA7134Card::SetVideoSource(int nInput)
{
    StandardSAA7134InputSelect(nInput);
}


const CSAA7134Card::TCardType* CSAA7134Card::GetCardSetup()
{
    return &m_SAA713xCards[m_CardType];
}


/*
 *  LifeView's audio chip connected accross GPIO mask 0xE000.
 *  Used by FlyVideo3000, FlyVideo2000 and PrimeTV 7133.
 *  (Below information is an unverified guess --AtNak)
 *
 *  NNNx
 *  ^^^
 *  |||- 0 = Normal, 1 = BTSC processing on ?
 *  ||-- 0 = Internal audio, 1 = External line pass through
 *  |--- 0 = Audio processor ON, 1 = Audio processor OFF
 *
 *  Use Normal/Internal/Audio Processor ON for FM Radio
 */

void CSAA7134Card::StandardSAA7134InputSelect(int nInput)
{
    eVideoInputSource VideoInput;

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

    if (nInput >= m_SAA713xCards[m_CardType].NumInputs)
    {
        LOG(1, _T("Input Select Called for invalid input"));
        nInput = m_SAA713xCards[m_CardType].NumInputs - 1;
    }
    if (nInput < 0)
    {
        LOG(1, _T("Input Select Called for invalid input"));
        nInput = 0;
    }

    VideoInput = m_SAA713xCards[m_CardType].Inputs[nInput].VideoInputPin;

    /// There is a 1:1 correlation between (int)eVideoInputSource
    /// and SAA7134_ANALOG_IN_CTRL1_MODE
    BYTE Mode = (VideoInput == VIDEOINPUTSOURCE_NONE) ? 0x00 : VideoInput;

    switch (m_SAA713xCards[m_CardType].Inputs[nInput].InputType)
    {
    case INPUTTYPE_SVIDEO:
        OrDataByte(SAA7134_LUMA_CTRL, SAA7134_LUMA_CTRL_BYPS);

        switch (VideoInput)
        {
        // This new mode sets Y-channel with automatic
        // gain control and gain control for C-channel
        // linked to Y-channel
        case VIDEOINPUTSOURCE_PIN0: Mode = 0x08; break;
        case VIDEOINPUTSOURCE_PIN1: Mode = 0x09; break;
        default:
            // NEVER_GET_HERE;
            break;
        }
        break;
    case INPUTTYPE_TUNER:
    case INPUTTYPE_COMPOSITE:
    case INPUTTYPE_CCIR:
    default:
        AndDataByte(SAA7134_LUMA_CTRL, ~SAA7134_LUMA_CTRL_BYPS);
        break;
    }

    MaskDataByte(SAA7134_ANALOG_IN_CTRL1, Mode, 0x0F);

    // GPIO settings
    if (m_SAA713xCards[m_CardType].dwGPIOMode != 0)
    {
        MaskDataDword(SAA7134_GPIO_GPMODE, m_SAA713xCards[m_CardType].dwGPIOMode, 0x0EFFFFFF);
        MaskDataDword(SAA7134_GPIO_GPSTATUS,
            m_SAA713xCards[m_CardType].Inputs[nInput].dwGPIOStatusBits,
            m_SAA713xCards[m_CardType].Inputs[nInput].dwGPIOStatusMask);
    }
}

#endif//xxx