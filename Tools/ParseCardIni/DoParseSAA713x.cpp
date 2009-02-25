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
// CVS Log
//
// $Log: not supported by cvs2svn $
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DoParseSAA713x.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


using namespace std;
using namespace HCParser;

const char* g_SAA_AudioLineNames[] =
{
	"NONE",
	"LINE1",
	"LINE2",
	"DAC",
};

const char* g_SAA_VideoPinNames[] =
{
	"VIDEOINPUTSOURCE_NONE",
	"VIDEOINPUTSOURCE_PIN0",
	"VIDEOINPUTSOURCE_PIN1",
	"VIDEOINPUTSOURCE_PIN2",
	"VIDEOINPUTSOURCE_PIN3",
	"VIDEOINPUTSOURCE_PIN4",
};

const char* g_SAA_InputTypeNames[] =
{
	"INPUTTYPE_COMPOSITE",
	"INPUTTYPE_SVIDEO",
	"INPUTTYPE_TUNER",
	"INPUTTYPE_CCIR",
	"INPUTTYPE_RADIO",
	"INPUTTYPE_MUTE",
	"INPUTTYPE_FINAL",
};

const char* g_SAA_AudioCrystalNames[] =
{
	"NONE",
	"32MHz",
	"24MHz",
};

std::vector<CDoParseSAA713x::CCardTypeEx> CDoParseSAA713x::m_Cards;

//////////////////////////////////////////////////////////////////////////
// SAA713x card list parsing constants
//////////////////////////////////////////////////////////////////////////
const CParseConstant CDoParseSAA713x::k_parseAudioPinConstants[] =
{
	PC(	"NONE",			AUDIOINPUTSOURCE_NONE	),
	PC(	"LINE1",		AUDIOINPUTSOURCE_LINE1	),
	PC(	"LINE2",		AUDIOINPUTSOURCE_LINE2	),
	PC(	"DAC",			AUDIOINPUTSOURCE_DAC	),
	PC(	NULL )
};

const CParseConstant CDoParseSAA713x::k_parseInputTypeConstants[] =
{
	PC(	"COMPOSITE",	INPUTTYPE_COMPOSITE		),
	PC(	"SVIDEO",		INPUTTYPE_SVIDEO		),
	PC(	"TUNER",		INPUTTYPE_TUNER			),
	PC(	"CCIR",			INPUTTYPE_CCIR			),
	PC(	"RADIO",		INPUTTYPE_RADIO			),
// MUTE	input was defined but not currently	used by	CSAA7134Card.
//	PC(	"MUTE",			INPUTTYPE_MUTE			),
// FINAL isn't necessary because Final() does the same thing.
//	PC(	"FINAL",		INPUTTYPE_FINAL		),
	PC(	NULL )
};

const CParseConstant CDoParseSAA713x::k_parseAudioCrystalConstants[] =
{
	PC(	"NONE",			AUDIOCRYSTAL_NONE		),
	PC(	"32MHz",		AUDIOCRYSTAL_32110kHz	),
	PC(	"24MHz",		AUDIOCRYSTAL_24576kHz	),
	PC(	NULL )
};

//////////////////////////////////////////////////////////////////////////
// SAA713x card	list parsing values
//////////////////////////////////////////////////////////////////////////
const CParseTag	CDoParseSAA713x::k_parseInputGPIOSet[]	=
{
	PT(	"Bits",				PARSE_NUMERIC,		1,	16,	NULL,							ReadCardInputInfoProc		),
	PT(	"Mask",				PARSE_NUMERIC,		1,	16,	NULL,							ReadCardInputInfoProc		),
	PT(	NULL )
};

const CParseTag	CDoParseSAA713x::k_parseCardInput[] =
{
	PT(	"Name",				PARSE_STRING,		1, 63,	NULL,							ReadCardInputInfoProc		),
	PT(	"Type",				PARSE_CONSTANT,		1, 16,	k_parseInputTypeConstants,		ReadCardInputInfoProc		),
	PT(	"VideoPin",			PARSE_NUMERIC,		0, 8,	NULL,							ReadCardInputInfoProc		),
	PT(	"AudioPin",			PARSE_NUM_OR_CONST,	0, 8,	k_parseAudioPinConstants,		ReadCardInputInfoProc		),
	PT(	"GPIOSet",			PARSE_CHILDREN,		0, 1,	k_parseInputGPIOSet,			NULL						),
	PT(	NULL )
};

const CParseTag	CDoParseSAA713x::k_parseAutoDetectID[]	=
{
	PT(	"0",				PARSE_NUMERIC,		0, 16,	NULL,							ReadCardAutoDetectIDProc	),
	PT(	"1",				PARSE_NUMERIC,		0, 16,	NULL,							ReadCardAutoDetectIDProc	),
	PT(	"2",				PARSE_NUMERIC,		0, 16,	NULL,							ReadCardAutoDetectIDProc	),
	PT(	NULL )
};

const CParseTag	CDoParseSAA713x::k_parseCard[]	=
{
	PT(	"Name",				PARSE_STRING,		1, 127,	NULL,							ReadCardInfoProc			), 
	PT(	"DeviceID",			PARSE_NUMERIC,		1, 8,	NULL,							ReadCardInfoProc			),
	PT(	"DefaultTuner",		PARSE_NUM_OR_CONST,	0, 32,	k_parseTunerConstants,			ReadCardDefaultTunerProc	),
	PT(	"AudioCrystal",		PARSE_CONSTANT,		0, 8,	k_parseAudioCrystalConstants,	ReadCardInfoProc			),
	PT(	"GPIOMask",			PARSE_NUMERIC,		0, 16,	NULL,							ReadCardInfoProc			),
	PT(	"AutoDetectID",		PARSE_CHILDREN,		0, 1,	k_parseAutoDetectID,			NULL						),
	PT(	"Input",			PARSE_CHILDREN,		0, 7,	k_parseCardInput,				ReadCardInputProc			),
	PT(	"Final",			PARSE_CHILDREN,		0, 7,	k_parseCardInput+2,				ReadCardInputProc			),
	PT(	"UseTDA9887",		PARSE_CHILDREN,		0, 1,	k_parseUseTDA9887,				ReadCardUseTDA9887Proc		),
	PT(	NULL )
};

const CParseTag	CDoParseSAA713x::k_parseCardList[]	=
{
	PT(	"Card",				PARSE_CHILDREN,		0, 512,	k_parseCard,					ReadCardProc				),
	PT(	NULL )
};

void CDoParseSAA713x::ReadCardInputInfoProc(int report, const CParseTag* tag, unsigned char,
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
            throw string("VideoPin must be between -1 and 4");
        }
        input->VideoInputPin = static_cast<eVideoInputSource>(n);
    }
    // AudioPin
    else if (tag == k_parseCardInput + 3)
    {
        int n = value->GetNumber();
        if (n < AUDIOINPUTSOURCE_NONE || n > AUDIOINPUTSOURCE_DAC)
        {
            throw string("AudioPin must be between -1 and 2");
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


void CDoParseSAA713x::ReadCardInputProc(int report, const CParseTag* tag, unsigned char,
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

void CDoParseSAA713x::ReadCardUseTDA9887Proc(int report, const CParseTag* tag, unsigned char type,
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
            for (int i = 0; i < TDA9887_FORMAT_LASTONE; i++)
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

void CDoParseSAA713x::ReadCardDefaultTunerProc(int report, const CParseTag* tag, unsigned char type,
                                            const CParseValue* value, void* context)
{
    TParseCardInfo* parseInfo = (TParseCardInfo*)context;

    // Return TRUE means parseInfo->tunerInfo is ready.
    if (ReadTunerProc(report, tag, type, value, &parseInfo->tunerInfo))
    {
        if (parseInfo->tunerInfo.tunerId == TUNER_AUTODETECT ||
            parseInfo->tunerInfo.tunerId == TUNER_USER_SETUP)
        {
            throw string("Tuner id \"auto\" and \"setup\" are not supported.");
        }
        parseInfo->pCurrentCard->TunerId = parseInfo->tunerInfo.tunerId;
    }
}


void CDoParseSAA713x::ReadCardInfoProc(int report, const CParseTag* tag, unsigned char,
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
    // DeviceID
    else if (tag == k_parseCard + 1)
    {
        int n = value->GetNumber();
        if (n != 0x7130 && n != 0x7134 && n != 0x7133)
        {
            throw string("DeviceID needs to be either 0x7134 or 0x7130");
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

void CDoParseSAA713x::ReadCardAutoDetectIDProc(int report, const CParseTag* tag, unsigned char,
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


void CDoParseSAA713x::ReadCardProc(int report, const CParseTag*, unsigned char, const CParseValue*, void* context)
{
    static TCardType cardDefaults = { "", 0x0000, 0, { 0 }, TUNER_ABSENT, AUDIOCRYSTAL_NONE, 0, { 0, 0, 0 }, FALSE };
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

CDoParseSAA713x::CDoParseSAA713x(const char* pszFileName, CListBox* pErrorList, CTreeCtrl* pTreeCtrl, BOOL bSort) :
	CDoParse(pszFileName, pErrorList, pTreeCtrl, bSort)
{
	ParseIniFile();
	DisplayCardsInTree();
}

CDoParseSAA713x::~CDoParseSAA713x()
{

}

void CDoParseSAA713x::ParseIniFile()
{
	m_Cards.resize(0);
	m_pErrorList->ResetContent();
    
	CHCParser hcParser(k_parseCardList);

    TParseCardInfo parseInfo;
    parseInfo.pCardList = &m_Cards;
    parseInfo.pCurrentCard = NULL;
    parseInfo.nGoodCards = 0;
    parseInfo.pHCParser = &hcParser;

    while (!hcParser.ParseFile(m_pszFileName, (void*)&parseInfo))
    {
		m_pErrorList->AddString(hcParser.GetError().c_str());
		int iRetVal = AfxMessageBox("Error while parsing.", MB_ABORTRETRYIGNORE|MB_ICONEXCLAMATION);

        if (iRetVal == IDIGNORE)
        {
            break;
        }
        
		if (iRetVal == IDABORT)
        {
            return;
        }

        parseInfo.pCurrentCard = NULL;
        parseInfo.nGoodCards = 0;
        parseInfo.pCardList->resize(0);
    }

    if (parseInfo.nGoodCards < parseInfo.pCardList->size())
    {
        parseInfo.pCardList->resize(parseInfo.nGoodCards);
    }
}

void CDoParseSAA713x::DisplayCardsInTree()
{
	m_pTreeCtrl->SetRedraw(FALSE);
	m_pTreeCtrl->DeleteAllItems();
	
	CString strTemp;

	for(int i = 0; i < m_Cards.size(); i++)
	{
		CCardTypeEx* pCurrentCard = &m_Cards[i];
		
		// Name
		HTREEITEM hCard = m_pTreeCtrl->InsertItem(pCurrentCard->szName, 0, 0);
		m_pTreeCtrl->SetItemState(hCard, TVIS_BOLD, TVIS_BOLD);

		// Device Id
		strTemp.Format("Device ID: 0x%04x", pCurrentCard->DeviceId);
		m_pTreeCtrl->InsertItem(strTemp, 2, 2, hCard);

		// Default Tuner
		strTemp.Format("Default Tuner: %s", TunerNames[pCurrentCard->TunerId + 2]);
		m_pTreeCtrl->InsertItem(strTemp, 2, 2, hCard);
		
		// Audio Crystal
		strTemp.Format("Audio Crystal: %s", g_SAA_AudioCrystalNames[pCurrentCard->AudioCrystal]);
		m_pTreeCtrl->InsertItem(strTemp, 2, 2, hCard);
		
		// GPIO Mode
		strTemp.Format("GPIO Mode: 0x%08x", pCurrentCard->dwGPIOMode);
		m_pTreeCtrl->InsertItem(strTemp, 2, 2, hCard);

		// Auto Detect ID's
		HTREEITEM hAutoDetectID = NULL;
		for(int j = 0; j < SA_AUTODETECT_ID_PER_CARD; j++)
		{
			if(pCurrentCard->AutoDetectId[j])
			{
				if(!hAutoDetectID)
				{
					hAutoDetectID = m_pTreeCtrl->InsertItem("Auto Detect ID(s)", 1, 1, hCard);
				}

				strTemp.Format("0x%08x", pCurrentCard->AutoDetectId[j]);
				m_pTreeCtrl->InsertItem(strTemp, 2, 2, hAutoDetectID);
			}
		}

		// Input Types
		strTemp.Format("Input Types [%d]", pCurrentCard->NumInputs);
		HTREEITEM hInput = m_pTreeCtrl->InsertItem(strTemp, 1, 1, hCard);

		for(int k = 0; k < SA_INPUTS_PER_CARD; k++)
		{
			TInputType* pInputType = &pCurrentCard->Inputs[k];

			strTemp = pInputType->szName;
			
			if(strTemp.IsEmpty())
			{
				continue;
			}

			// Name
			HTREEITEM hInputItem = m_pTreeCtrl->InsertItem(strTemp, 1, 1, hInput);

			// Type
			strTemp.Format("Type: %s", g_SAA_InputTypeNames[pInputType->InputType]);
			m_pTreeCtrl->InsertItem(strTemp, 2, 2, hInputItem);
			
			// Video Input Pin
			strTemp.Format("Video Input Pin: %s", g_SAA_VideoPinNames[pInputType->VideoInputPin]);
			m_pTreeCtrl->InsertItem(strTemp, 2, 2, hInputItem);
		
			// Audio Line Select
			strTemp.Format("Audio Line Select: %s", g_SAA_AudioLineNames[pInputType->AudioLineSelect]);
			m_pTreeCtrl->InsertItem(strTemp, 2, 2, hInputItem);
			
			// GPIO Status Mask
			strTemp.Format("GPIO Status Mask: 0x%08x", pInputType->dwGPIOStatusMask);
			m_pTreeCtrl->InsertItem(strTemp, 2, 2, hInputItem);

			// GPIO Status Bits
			strTemp.Format("GPIO Status Bits: 0x%08x", pInputType->dwGPIOStatusBits);
			m_pTreeCtrl->InsertItem(strTemp, 2, 2, hInputItem);
		}

		// TDA9887
		if(pCurrentCard->bUseTDA9887)
		{
			HTREEITEM hTDA9887 = m_pTreeCtrl->InsertItem("Use TDA9887", 1, 1, hCard);
			
			for(int m = 0; m < m_Cards[i].tda9887Modes.size(); m++)
			{
				TTDA9887FormatModes* pModes = &m_Cards[i].tda9887Modes[m];
				
				if (pModes->mask != 0)
				{
					HTREEITEM hTDA9887Format = m_pTreeCtrl->InsertItem(m_TDAFormatNames[pModes->format], 1, 1, hTDA9887);
					
					// Outport1
					if(pModes->mask & TDA9887_SM_OUTPUTPORT1_INACTIVE)
					{
						strTemp.Format("OutPort1: %s", pModes->bits & TDA9887_SM_OUTPUTPORT1_INACTIVE ? "Inactive" : "Active");
						m_pTreeCtrl->InsertItem(strTemp, 2, 2, hTDA9887Format);
					}
					
					// Outport2
					if(pModes->mask & TDA9887_SM_OUTPUTPORT2_INACTIVE)
					{
						strTemp.Format("OutPort2: %s", pModes->bits & TDA9887_SM_OUTPUTPORT2_INACTIVE ? "Inactive" : "Active");
						m_pTreeCtrl->InsertItem(strTemp, 2, 2, hTDA9887Format);
					}
					
					// Carrier
					if(pModes->mask & TDA9887_SM_CARRIER_QSS)
					{
						strTemp.Format("Carrier: %s", pModes->bits & TDA9887_SM_CARRIER_QSS ? "QSS" : "Intercarrier" );
						m_pTreeCtrl->InsertItem(strTemp, 2, 2, hTDA9887Format);
					}
					
					// Takeover Point
					if(pModes->mask & TDA9887_SM_TAKEOVERPOINT_MASK)
					{
						strTemp.Format("Takeover Point: 0x%02x", (pModes->bits & TDA9887_SM_TAKEOVERPOINT_MASK) >> TDA9887_SM_TAKEOVERPOINT_OFFSET );
						m_pTreeCtrl->InsertItem(strTemp, 2, 2, hTDA9887Format);
					}
				}
			}
		}
	}

    if(m_bDoSort)
    {
        m_pTreeCtrl->SortChildren(0);
    }

    m_pTreeCtrl->SetRedraw(TRUE);
}

