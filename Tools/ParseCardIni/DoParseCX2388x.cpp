/////////////////////////////////////////////////////////////////////////////
// $Id: DoParseCX2388x.cpp,v 1.1 2005-10-28 16:43:13 to_see Exp $
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
#include "DoParseCX2388x.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


using namespace std;
using namespace HCParser;

const char* g_CXInputTypeNamesConstants[] =
{
	"INPUTTYPE_TUNER",
	"INPUTTYPE_COMPOSITE",
	"INPUTTYPE_SVIDEO",
	"INPUTTYPE_CCIR",
	"INPUTTYPE_COLOURBARS",
	"INPUTTYPE_FINAL",
};

const char* g_CXCardModeConstants[] =
{
	"MODE_STANDARD",
	"MODE_H3D",
};

std::vector<CDoParseCX2388x::CCardTypeEx> CDoParseCX2388x::m_Cards;

const CParseTag CDoParseCX2388x::k_parseCardGPIOSetConstants[] =
{
    PT( "GPIO_0", PARSE_NUMERIC, 1, 16, NULL, ReadCardInputInfoProc ),
    PT( "GPIO_1", PARSE_NUMERIC, 0, 16, NULL, ReadCardInputInfoProc ),
    PT( "GPIO_2", PARSE_NUMERIC, 0, 16, NULL, ReadCardInputInfoProc ),
    PT( "GPIO_3", PARSE_NUMERIC, 0, 16, NULL, ReadCardInputInfoProc ),
    PT( NULL )
};

const CParseConstant CDoParseCX2388x::k_parseInputTypeConstants[] =
{
    PC( "TUNER",      INPUTTYPE_TUNER      ),
    PC( "COMPOSITE",  INPUTTYPE_COMPOSITE  ),
    PC( "SVIDEO",     INPUTTYPE_SVIDEO     ),
    PC( "CCIR",       INPUTTYPE_CCIR       ),
    PC( "COLOURBARS", INPUTTYPE_COLOURBARS ),
    PC( NULL )
};

const CParseConstant CDoParseCX2388x::k_parseCardModeConstants[] =
{
    PC( "STANDARD",   MODE_STANDARD ),
    PC( "H3D",        MODE_H3D      ),
    PC( NULL )
};

const CParseTag CDoParseCX2388x::k_parseCardInput[] =
{
    PT( "Name",       PARSE_STRING,   1, 63, NULL,                      ReadCardInputInfoProc ),
    PT( "Type",       PARSE_CONSTANT, 1, 16, k_parseInputTypeConstants, ReadCardInputInfoProc ),
    PT( "MuxSelect",  PARSE_NUMERIC,  1,  2, NULL,                      ReadCardInputInfoProc ),
    PT( "GPIOSet",    PARSE_CHILDREN, 0,  1, k_parseCardGPIOSetConstants,   NULL ),
    PT( NULL )
};

const CParseTag CDoParseCX2388x::k_parseCardAutoDetectID[] =
{
    PT( "0", PARSE_NUMERIC, 0, 16, NULL, ReadCardAutoDetectIDProc ),
    PT( "1", PARSE_NUMERIC, 0, 16, NULL, ReadCardAutoDetectIDProc ),
    PT( "2", PARSE_NUMERIC, 0, 16, NULL, ReadCardAutoDetectIDProc ),
    PT( NULL )
};

const CParseTag CDoParseCX2388x::k_parseCard[] =
{
    PT( "Name",           PARSE_STRING,                 1, 127,			       NULL,                            ReadCardInfoProc         ), 
    PT( "CardMode",       PARSE_CONSTANT,               0, 32,                 k_parseCardModeConstants, ReadCardInfoProc         ),
    PT( "DefaultTuner",   PARSE_CONSTANT|PARSE_NUMERIC, 0, 32,                 k_parseTunerConstants,           ReadCardDefaultTunerProc ),
    PT( "AutoDetectID",   PARSE_CHILDREN,               0, 1,				   k_parseCardAutoDetectID,         NULL                     ),
    PT( "Input",          PARSE_CHILDREN,               0, CX_INPUTS_PER_CARD, k_parseCardInput,                ReadCardInputProc        ),
    PT( "Final",          PARSE_CHILDREN,               0, CX_INPUTS_PER_CARD, k_parseCardInput+2,              ReadCardInputProc        ),
    PT( "UseTDA9887",     PARSE_CHILDREN,               0, 1,				   k_parseUseTDA9887,               ReadCardUseTDA9887Proc   ),
    PT( NULL )
};

const CParseTag CDoParseCX2388x::k_parseCardList[] =
{
    PT( "Card", PARSE_CHILDREN, 0, 1024, k_parseCard, ReadCardProc ),
    PT( NULL )
};

void CDoParseCX2388x::ReadCardProc(int report, const CParseTag*, unsigned char, const CParseValue*, void* context)
{
    static TCardType cardDefaults = { "", 0, { 0 }, TUNER_ABSENT, 0, FALSE, MODE_STANDARD };
    TParseCardInfo* parseInfo = (TParseCardInfo*)context;

    switch (report)
    {
    case REPORT_OPEN:
        parseInfo->pCardList->push_back(cardDefaults);
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

void CDoParseCX2388x::ReadCardInputInfoProc(int report, const CParseTag* tag, unsigned char,
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
    else if (tag == k_parseCardGPIOSetConstants + 0)
    {
		input->GPIOSet.GPIO_0 = static_cast<DWORD>(value->GetNumber());
    }

    // GPIOSet->GPIO_1
    else if (tag == k_parseCardGPIOSetConstants + 1)
    {
		input->GPIOSet.GPIO_1 = static_cast<DWORD>(value->GetNumber());
    }
    
	// GPIOSet->GPIO_2
    else if (tag == k_parseCardGPIOSetConstants + 2)
    {
		input->GPIOSet.GPIO_2 = static_cast<DWORD>(value->GetNumber());
    }

    // GPIOSet->GPIO_3
    else if (tag == k_parseCardGPIOSetConstants + 3)
    {
		input->GPIOSet.GPIO_3 = static_cast<DWORD>(value->GetNumber());
    }
}

void CDoParseCX2388x::ReadCardInfoProc(int report, const CParseTag* tag, unsigned char,
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

void CDoParseCX2388x::ReadCardInputProc(int report, const CParseTag* tag, unsigned char,
                                     const CParseValue*, void* context)
{
    TParseCardInfo* parseInfo = (TParseCardInfo*)context;

    switch (report)
    {
    case REPORT_OPEN:
        {
            TInputType* input = &parseInfo->pCurrentCard->Inputs[parseInfo->pCurrentCard->NumInputs];
            input->szName[0] = '\0';
            input->InputType = (tag == k_parseCard + 4) ? INPUTTYPE_FINAL : INPUTTYPE_COMPOSITE;
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

void CDoParseCX2388x::ReadCardAutoDetectIDProc(int report, const CParseTag* tag, unsigned char,
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

void CDoParseCX2388x::ReadCardUseTDA9887Proc(int report, const CParseTag* tag, unsigned char type,
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

void CDoParseCX2388x::ReadCardDefaultTunerProc(int report, const CParseTag* tag, unsigned char type,
                                            const CParseValue* value, void* context)
{
    TParseCardInfo* parseInfo = (TParseCardInfo*)context;

    // Return TRUE means parseInfo->tunerInfo is ready.
    if (ReadTunerProc(report, tag, type, value, &parseInfo->tunerInfo))
    {
        parseInfo->pCurrentCard->TunerId = parseInfo->tunerInfo.tunerId;
    }
}

CDoParseCX2388x::CDoParseCX2388x(const char* pszFileName, CListBox* pErrorList, CTreeCtrl* pTreeCtrl, BOOL bSort) :
	CDoParse(pszFileName, pErrorList, pTreeCtrl, bSort)
{
	ParseIniFile();
	DisplayCardsInTree();
}

CDoParseCX2388x::~CDoParseCX2388x()
{
}

void CDoParseCX2388x::ParseIniFile()
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

void CDoParseCX2388x::DisplayCardsInTree()
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

		// Card Handle Flag
		strTemp.Format("Card Mode: %s", g_CXCardModeConstants[pCurrentCard->CardMode]);
		m_pTreeCtrl->InsertItem(strTemp, 2, 2, hCard);

		// Default Tuner
		strTemp.Format("Default Tuner: %s", TunerNames[pCurrentCard->TunerId + 2]);
		m_pTreeCtrl->InsertItem(strTemp, 2, 2, hCard);

		// Auto Detect ID's
		HTREEITEM hAutoDetectID = NULL;
		for(int j = 0; j < CX_AUTODETECT_ID_PER_CARD; j++)
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
		
        for(int k = 0; k < CX_INPUTS_PER_CARD; k++)
		{
			TInputType* pInputType = &pCurrentCard->Inputs[k];

            if(pInputType->InputType == INPUTTYPE_FINAL)
            {
                strTemp = "Final";
            }
            else
            {
    			strTemp = pInputType->szName;
            }

            if(strTemp.IsEmpty())
            {
                continue;
            }

			HTREEITEM hInputItem = m_pTreeCtrl->InsertItem(strTemp, 1, 1, hInput);

			// Input Type
			strTemp.Format("Input Type: %s", g_CXInputTypeNamesConstants[pInputType->InputType]);
			m_pTreeCtrl->InsertItem(strTemp, 2, 2, hInputItem);
			
			// Mux Select
			strTemp.Format("Mux Select: %0d", pInputType->MuxSelect);
			m_pTreeCtrl->InsertItem(strTemp, 2, 2, hInputItem);
			
			// GPIO Set
			HTREEITEM hGPIO = m_pTreeCtrl->InsertItem("GPIO Set", 1, 1, hInputItem);
			TGPIOSet* pGPIOSet = &pInputType->GPIOSet;

			strTemp.Format("GPIO_0: 0x%08x", pGPIOSet->GPIO_0);
			m_pTreeCtrl->InsertItem(strTemp, 2, 2, hGPIO);
			
			strTemp.Format("GPIO_1: 0x%08x", pGPIOSet->GPIO_1);
			m_pTreeCtrl->InsertItem(strTemp, 2, 2, hGPIO);
			
			strTemp.Format("GPIO_2: 0x%08x", pGPIOSet->GPIO_2);
			m_pTreeCtrl->InsertItem(strTemp, 2, 2, hGPIO);
		
			strTemp.Format("GPIO_3: 0x%08x", pGPIOSet->GPIO_3);
			m_pTreeCtrl->InsertItem(strTemp, 2, 2, hGPIO);
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
