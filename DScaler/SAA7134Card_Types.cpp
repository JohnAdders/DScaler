/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Card_Types.cpp,v 1.51 2004-11-20 17:58:03 atnak Exp $
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
// Change Log
//
// Date          Developer             Changes
//
// 09 Sep 2002   Atsushi Nakagawa      Initial Release
// 19 Nov 2004   Atsushi Nakagawa      Changed to INI card list and trimmed
//                                     the CVS Log by removing all card adding
//                                     related entries by me.
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.50  2004/11/20 14:20:09  atnak
// Changed the card list to an ini file.
//
// Revision 1.41  2004/02/18 06:39:47  atnak
// Changed Setup Card / Tuner so that only cards of the same device are
// shown in the card list.
//
// Revision 1.36  2004/02/14 04:03:44  atnak
// Put GPIO settings and AutoDetect IDs into the main card definition
// to remove the need for extra tables and custom functions.
//
// Revision 1.35  2003/10/27 10:39:53  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.34  2003/08/23 10:00:33  atnak
// Added a missing break in a switch statement
//
// Revision 1.29  2003/04/16 15:11:50  atnak
// Fixed Medion TV-Tuner 7134 MK2/3 audio clock and default tuner
//
// Revision 1.24  2003/02/06 21:30:44  ittarnavsky
// changes to support primetv 7133
//
// Revision 1.22  2003/01/30 07:19:47  ittarnavsky
// fixed the autodetect
//
// Revision 1.15  2002/12/22 03:52:11  atnak
// Fixed FlyVideo2000 GPIO settings for input change
//
// Revision 1.13  2002/12/10 11:05:46  atnak
// Fixed FlyVideo 3000 audio for external inputs
//
// Revision 1.12  2002/11/12 01:26:25  atnak
// Changed the define name of a card
//
// Revision 1.11  2002/10/28 11:10:13  atnak
// Various changes and revamp to settings
//
// Revision 1.10  2002/10/26 05:24:23  atnak
// Minor cleanups
//
// Revision 1.9  2002/10/26 04:41:44  atnak
// Clean up + added auto card detection
//
// Revision 1.8  2002/10/16 22:10:56  atnak
// fixed some cards to available FM1216ME_MK3 tuner
//
// Revision 1.6  2002/10/06 11:09:48  atnak
// SoundChannel function from TCardType
//
// Revision 1.5  2002/10/04 13:24:46  atnak
// Audio mux select through GPIO added (for 7130 cards)
//
// Revision 1.4  2002/10/03 23:36:23  atnak
// Various changes (major): VideoStandard, AudioStandard, CSAA7134Common, cleanups, tweaks etc,
//
// Revision 1.3  2002/09/14 19:40:48  atnak
// various changes
//
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file SAA7134Card.cpp CSAA7134Card Implementation (Types)
 */

#include "stdafx.h"
#include "DScaler.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "SAA7134Card.h"
#include "SAA7134_Defines.h"
#include "DebugLog.h"
#include "HierarchicalConfigParser.h"

using namespace HCParser;

static const char* k_SAA713xCardListFilename = "SAA713xCards.ini";


//////////////////////////////////////////////////////////////////////////
// Default generic prefab card for the card list
//////////////////////////////////////////////////////////////////////////
const CSAA7134Card::TCardType CSAA7134Card::m_SAA7134UnknownCard =
{
    // SAA7134CARDID_UNKNOWN - Unknown Card
    "*Unknown Card*",
    0x0000,
    4,
    {
        {
            "Tuner",
            INPUTTYPE_TUNER,
            VIDEOINPUTSOURCE_PIN1,
            AUDIOINPUTSOURCE_DAC,
        },
        {
            "Composite",
            INPUTTYPE_COMPOSITE,
            VIDEOINPUTSOURCE_PIN3,
            AUDIOINPUTSOURCE_LINE1,
        },
        {
            "S-Video",
            INPUTTYPE_SVIDEO,
            VIDEOINPUTSOURCE_PIN0,
            AUDIOINPUTSOURCE_LINE1,
        },
        {
            "Composite over S-Video",
            INPUTTYPE_COMPOSITE,
            VIDEOINPUTSOURCE_PIN0,
            AUDIOINPUTSOURCE_LINE1,
        },
    },
    TUNER_ABSENT,
    AUDIOCRYSTAL_32110Hz,
    0,
};

std::vector<CSAA7134Card::TCardType> CSAA7134Card::m_SAA713xCards;


//////////////////////////////////////////////////////////////////////////
// SAA713x card list parsing constants
//////////////////////////////////////////////////////////////////////////
const ParseConstant CSAA7134Card::k_parseSAA713xInputAudioPinConstants[] =
{
    { "NONE",       "-1"            },
    { "LINE1",      "0"             },
    { "LINE2",      "1"             },
    { "DAC",        "2"             },
    { NULL,         NULL            },
};

const ParseConstant CSAA7134Card::k_parseSAA713xInputTypeConstants[] =
{
    { "COMPOSITE",  "0"             },
    { "SVIDEO",     "1"             },
    { "TUNER",      "2"             },
    { "CCIR",       "3"             },
    { "RADIO",      "4"             },
//  { "MUTE",       "5"             },
//  { "FINAL",      "6"             },
    { NULL,         NULL            },
};

const ParseConstant CSAA7134Card::k_parseSAA713xAudioCrystalConstants[] =
{
    { "NONE",       "0"             },
    { "32Hz",       "1"             },
    { "24Hz",       "2"             },
    { NULL,         NULL            },
};

const ParseConstant CSAA7134Card::k_parseSAA713xDefaultTunerConstants[] =
{
    // This list should really be generated at runtime but there
    // isn't another string list in DScaler that can be used as
    // the constants list.  I generated this code by running regex
    // replaces on the eTunerId enum list.
    { "ABSENT", "0" },
    { "PHILIPS_PAL_I", "1" },
    { "PHILIPS_NTSC", "2" },
    { "PHILIPS_SECAM", "3" },
    { "PHILIPS_PAL", "4" },
    { "TEMIC_4002FH5_PAL", "5" },
    { "TEMIC_4032FY5_NTSC", "6" },
    { "TEMIC_4062FY5_PAL_I", "7" },
    { "TEMIC_4036FY5_NTSC", "8" },
    { "ALPS_TSBH1_NTSC", "9" },
    { "ALPS_TSBE1_PAL", "10" },
    { "ALPS_TSBB5_PAL_I", "11" },
    { "ALPS_TSBE5_PAL", "12" },
    { "ALPS_TSBC5_PAL", "13" },
    { "TEMIC_4006FH5_PAL", "14" },
    { "PHILIPS_1236D_NTSC_INPUT1", "15" },
    { "PHILIPS_1236D_NTSC_INPUT2", "16" },
    { "ALPS_TSCH6_NTSC", "17" },
    { "TEMIC_4016FY5_PAL", "18" },
    { "PHILIPS_MK2_NTSC", "19" },
    { "TEMIC_4066FY5_PAL_I", "20" },
    { "TEMIC_4006FN5_PAL", "21" },
    { "TEMIC_4009FR5_PAL", "22" },
    { "TEMIC_4039FR5_NTSC", "23" },
    { "TEMIC_4046FM5_MULTI", "24" },
    { "PHILIPS_PAL_DK", "25" },
    { "PHILIPS_MULTI", "26" },
    { "LG_I001D_PAL_I", "27" },
    { "LG_I701D_PAL_I", "28" },
    { "LG_R01F_NTSC", "29" },
    { "LG_B01D_PAL", "30" },
    { "LG_B11D_PAL", "31" },
    { "TEMIC_4009FN5_PAL", "32" },
    { "MT2032", "33" },
    { "SHARP_2U5JF5540_NTSC", "34" },
    { "LG_TAPCH701P_NTSC", "35" },
    { "SAMSUNG_PAL_TCPM9091PD27", "36" },
    { "TEMIC_4106FH5", "37" },
    { "TEMIC_4012FY5", "38" },
    { "TEMIC_4136FY5", "39" },
    { "LG_TAPCNEW_PAL", "40" },
    { "PHILIPS_FM1216ME_MK3", "41" },
    { "LG_TAPCNEW_NTSC", "42" },
    { "MT2032_PAL", "43" },
    { "PHILIPS_FI1286_NTSC_M_J", "44" },
    { "MT2050", "45" },
    { "MT2050_PAL", "46" },
    { "PHILIPS_4IN1", "47" },
    { NULL,                 NULL    },
};

//////////////////////////////////////////////////////////////////////////
// SAA713x card list parsing values
//////////////////////////////////////////////////////////////////////////
const ParseTag CSAA7134Card::k_parseSAA713xCardInputGPIOSet[] =
{
    { "Bits",           PARSE_NUMERIC,                  1, 16, NULL, NULL, ReadCardInputInfoProc },
    { "Mask",           PARSE_NUMERIC,                  1, 16, NULL, NULL, ReadCardInputInfoProc },
    { NULL,             0,                              0, 0, NULL, NULL, NULL },
};

const ParseTag CSAA7134Card::k_parseSAA713xCardInput[] =
{
    { "Name",           PARSE_STRING,                   1, 63, NULL, NULL, ReadCardInputInfoProc },
    { "Type",           PARSE_CONSTANT,                 1, 16, NULL, k_parseSAA713xInputTypeConstants, ReadCardInputInfoProc },
    { "VideoPin",       PARSE_NUMERIC,                  0, 8, NULL, NULL, ReadCardInputInfoProc },
    { "AudioPin",       PARSE_CONSTANT|PARSE_NUMERIC,   0, 8, NULL, k_parseSAA713xInputAudioPinConstants, ReadCardInputInfoProc },
    { "GPIOSet",        PARSE_CHILDREN,                 0, 1, k_parseSAA713xCardInputGPIOSet, NULL, NULL },
    { NULL,             0,                              0, 0, NULL, NULL, NULL },
};

const ParseTag CSAA7134Card::k_parseSAA713xCardAutoDetectID[] =
{
    { "0",              PARSE_NUMERIC,                  0, 16, NULL, NULL, ReadCardAutoDetectIDProc },
    { "1",              PARSE_NUMERIC,                  0, 16, NULL, NULL, ReadCardAutoDetectIDProc },
    { "2",              PARSE_NUMERIC,                  0, 16, NULL, NULL, ReadCardAutoDetectIDProc },
    { NULL,             0,                              0, 0, NULL, NULL, NULL },
};

const ParseTag CSAA7134Card::k_parseSAA713xCard[] =
{
    { "Name",           PARSE_STRING,                   1, 127, NULL, NULL, ReadCardInfoProc }, 
    { "DeviceID",       PARSE_NUMERIC,                  1, 8, NULL, NULL, ReadCardInfoProc},
    { "DefaultTuner",   PARSE_CONSTANT|PARSE_NUMERIC,   0, 32, NULL, k_parseSAA713xDefaultTunerConstants, ReadCardInfoProc },
    { "AudioCrystal",   PARSE_CONSTANT,                 0, 8, NULL, k_parseSAA713xAudioCrystalConstants, ReadCardInfoProc },
    { "GPIOMask",       PARSE_NUMERIC,                  0, 16, NULL, NULL, ReadCardInfoProc },
    { "AutoDetectID",   PARSE_CHILDREN,                 0, 1, k_parseSAA713xCardAutoDetectID, NULL, NULL },
    { "Input",          PARSE_CHILDREN,                 0, 7, k_parseSAA713xCardInput, NULL, ReadCardInputProc },
    { "Final",          PARSE_CHILDREN,                 0, 7, k_parseSAA713xCardInput+2, NULL, ReadCardInputProc },
    { NULL,             0,                              0, 0, NULL, NULL, NULL },
};

const ParseTag CSAA7134Card::k_parseSAA713xCardList[] =
{
    { "Card",           PARSE_CHILDREN,                 0, 1024, k_parseSAA713xCard, NULL, ReadCardProc },
    { NULL,             0,                              0, 0, NULL, NULL, NULL },
};


BOOL CSAA7134Card::InitializeSAA713xCardList()
{
    InitializeSAA713xUnknownCard();

    CHCParser hcParser(k_parseSAA713xCardList);

    TParseCardInfo parseInfo;
    parseInfo.pCardList = &m_SAA713xCards;
    parseInfo.pCurrentCard = NULL;
    parseInfo.nGoodCards = 1;
    parseInfo.pHCParser = &hcParser;

    // No need to use ParseLocalFile() because DScaler does SetExeDirectory()
    // at the beginning.
    while (!hcParser.ParseFile(k_SAA713xCardListFilename, (void*)&parseInfo))
    {
        LOG(0, "SAA713x cardlist: %s", hcParser.GetError().c_str());

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

    LOG(1, "SAA713x cardlist: %lu card(s) read", parseInfo.nGoodCards);

    return TRUE;
}


void CSAA7134Card::InitializeSAA713xUnknownCard()
{
    if (m_SAA713xCards.size() == 0)
    {
        m_SAA713xCards.push_back(m_SAA7134UnknownCard);
    }
}


void CSAA7134Card::ReadCardInputInfoProc(int reason, const ParseTag* tag, unsigned char,
                                         const char* value, void* context)
{
    if (reason != REPORT_VALUE)
    {
        return;
    }

    TParseCardInfo* parseInfo = (TParseCardInfo*)context;
    TInputType* input = &parseInfo->pCurrentCard->Inputs[parseInfo->pCurrentCard->NumInputs];

    // Name
    if (tag == k_parseSAA713xCardInput + 0)
    {
        if (*value == '\0')
        {
            throw string("\"\" is not a valid name of an input");
        }
        strcpy(input->szName, value);
    }
    // Type
    else if (tag == k_parseSAA713xCardInput + 1)
    {
        input->InputType = (eInputType)CHCParser::Str2Long(value);
    }
    // VideoPin
    else if (tag == k_parseSAA713xCardInput + 2)
    {
        long l = CHCParser::Str2Long(value);
        if (l < VIDEOINPUTSOURCE_NONE || l > VIDEOINPUTSOURCE_PIN4)
        {
            throw string("VideoPin must be between -1 and 4");
        }
        input->VideoInputPin = (eVideoInputSource)l;
    }
    // AudioPin
    else if (tag == k_parseSAA713xCardInput + 3)
    {
        long l = CHCParser::Str2Long(value);
        if (l < AUDIOINPUTSOURCE_NONE || l > AUDIOINPUTSOURCE_DAC)
        {
            throw string("AudioPin must be between -1 and 2");
        }
        input->AudioLineSelect = (eAudioInputSource)l;
    }
    // GPIOSet->Bits
    else if (tag == k_parseSAA713xCardInputGPIOSet + 0)
    {
        input->dwGPIOStatusBits = CHCParser::Str2Long(value);
    }
    // GPIOSet->Mask
    else if (tag == k_parseSAA713xCardInputGPIOSet + 1)
    {
        input->dwGPIOStatusMask = CHCParser::Str2Long(value);
    }
}


void CSAA7134Card::ReadCardInputProc(int reason, const ParseTag* tag, unsigned char,
                                     const char* value, void* context)
{
    TParseCardInfo* parseInfo = (TParseCardInfo*)context;

    switch (reason)
    {
    case REPORT_OPEN:
        {
            TInputType* input = &parseInfo->pCurrentCard->Inputs[parseInfo->pCurrentCard->NumInputs];
            input->szName[0] = '\0';
            input->InputType = (tag == k_parseSAA713xCard + 7) ? INPUTTYPE_FINAL : INPUTTYPE_COMPOSITE;
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


void CSAA7134Card::ReadCardInfoProc(int reason, const ParseTag* tag, unsigned char,
                                    const char* value, void* context)
{
    if (reason != REPORT_VALUE)
    {
        return;
    }

    TParseCardInfo* parseInfo = (TParseCardInfo*)context;

    // Name
    if (tag == k_parseSAA713xCard + 0)
    {
        if (*value == '\0')
        {
            throw string("\"\" is not a valid name of a card");
        }
        for (size_t i = 0; i < parseInfo->nGoodCards; i++)
        {
            if (stricmp((*parseInfo->pCardList)[i].szName, value) == 0)
            {
                throw string("A card was already specified with this name");
            }
        }
        strcpy(parseInfo->pCurrentCard->szName, value);
    }
    // DeviceID
    else if (tag == k_parseSAA713xCard + 1)
    {
        long l = CHCParser::Str2Long(value);
        if (l != 0x7130 && l != 0x7134 && l != 0x7133)
        {
            throw string("DeviceID needs to be either 0x7134 or 0x7130");
        }
        parseInfo->pCurrentCard->DeviceId = (WORD)l;
    }
    // DefaultTuner
    else if (tag == k_parseSAA713xCard + 2)
    {
        long l = CHCParser::Str2Long(value);
        if (l < 0 || l > 74)
        {
            throw string("TunerID must be between 0 and 74");
        }
        parseInfo->pCurrentCard->TunerId = (eTunerId)l;
    }
    // AudioCrystal
    else if (tag == k_parseSAA713xCard + 3)
    {
        parseInfo->pCurrentCard->AudioCrystal =
            (eAudioCrystal)(CHCParser::Str2Long(value)+AUDIOCRYSTAL_NONE);
    }
    // GPIOMask
    else if (tag == k_parseSAA713xCard + 4)
    {
        parseInfo->pCurrentCard->dwGPIOMode = CHCParser::Str2Long(value);
    }
}

void CSAA7134Card::ReadCardAutoDetectIDProc(int reason, const ParseTag* tag, unsigned char,
                                            const char* value, void* context)
{
    if (reason != REPORT_VALUE)
    {
        return;
    }

    TParseCardInfo* parseInfo = (TParseCardInfo*)context;

    int i = (int)(tag - k_parseSAA713xCardAutoDetectID);
    parseInfo->pCurrentCard->AutoDetectId[i] = CHCParser::Str2Long(value);
}


void CSAA7134Card::ReadCardProc(int reason, const ParseTag*, unsigned char, const char*, void* context)
{
    static TCardType cardDefaults = { "", 0x0000, 0, { 0 }, TUNER_ABSENT, AUDIOCRYSTAL_NONE, 0, { 0, 0, 0 } };
    TParseCardInfo* parseInfo = (TParseCardInfo*)context;

    switch (reason)
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
                    throw string("AudioCrystal for a 0x7130 card must be NONE");
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


BOOL APIENTRY CSAA7134Card::ParseErrorProc(HWND hDlg, UINT message, UINT wParam, LPARAM lParam)
{
    TParseCardInfo* parseInfo = (TParseCardInfo*)lParam;

    switch (message)
    {
    case WM_INITDIALOG:
        {
            SetWindowTextA(hDlg, "SAA713x Card List Parsing Error");

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
            oss << "An error occured while reading SAA713x cards from 'SAA713xCards.ini':";

            hItem = GetDlgItem(hDlg, IDC_TOP_STATIC);
            SetWindowTextA(hItem, oss.str().c_str());

            oss.str("");
            oss << (parseInfo->nGoodCards - 1) << " card(s) were successfully read before this "
                "error.  Although this error is not fatal, if a previously selected SAA713x "
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

//
// Notes:
//
// "Might req mode 6": S-Video is listed with VIDEOINPUTSOURCE_PIN0 but what
// is actually used is not mode 0 but mode 8.  --This is due to an old design
// decision that I no longer remember why.  Mode 6 is exactly the same as mode
// 8 except the C-channel gain control is set with a register instead of
// automatic gain control that is linked to the Y-channel. "Might req mode 6"
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
// - "Genius Video Wonder PRO III" also has the ID 0x01385168 and is 0x7134 but the
//   audio clock is different.
//


int CSAA7134Card::GetMaxCards()
{
    InitializeSAA713xUnknownCard();
    return m_SAA713xCards.size();
}


int CSAA7134Card::GetCardByName(LPCSTR cardName)
{
    int listSize = GetMaxCards();
    for (int i = 0; i < listSize; i++)
    {
        if (_stricmp(m_SAA713xCards[i].szName, cardName) == 0)
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
       LOG(0, "SAA713x: Autodetect found [0x00001131] *Unknown Card*.");
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
                LOG(0, "SAA713x: Autodetect found %s.", m_SAA713xCards[i].szName);
                return (eSAA7134CardId)i;
            }
        }
    }

    LOG(0, "SAA713x: Autodetect found an unknown card with the following");
    LOG(0, "SAA713x: properties.  Please email the author and quote the");
    LOG(0, "SAA713x: following numbers, as well as which card you have,");
    LOG(0, "SAA713x: so it can be added to the list:");
    LOG(0, "SAA713x: DeviceId: 0x%04x, AutoDetectId: 0x%08x",
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


LPCSTR CSAA7134Card::GetInputName(int nInput)
{
    if (nInput >= m_SAA713xCards[m_CardType].NumInputs || nInput < 0)
    {
        return "Error";
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


LPCSTR CSAA7134Card::GetCardName(eSAA7134CardId CardId)
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
        LOG(1, "Input Select Called for invalid input");
        nInput = m_SAA713xCards[m_CardType].NumInputs - 1;
    }
    if (nInput < 0)
    {
        LOG(1, "Input Select Called for invalid input");
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

