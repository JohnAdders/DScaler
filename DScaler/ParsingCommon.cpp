/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004 Atsushi Nakagawa.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ParsingCommon.h"
#include "HierarchicalConfigParser.h"

using namespace HCParser;

static void SetTDA9887ModeMaskAndBits(OUT TTDA9887Modes&, IN BYTE, IN BOOL);


//////////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////////

// It's not imperative that this list be updated when a new tuner
// is added but not updating it will mean the new tuner cannot be
// referred to by its name in the card list ini files.
const CParseConstant k_parseTunerConstants[] =
{
    PC( _T("AUTO"),                            TUNER_AUTODETECT                ),
    PC( _T("SETUP"),                        TUNER_USER_SETUP                ),
    PC( _T("ABSENT"),                        TUNER_ABSENT                    ),
    PC( _T("PHILIPS_PAL_I"),                TUNER_PHILIPS_PAL_I                ),
    PC( _T("PHILIPS_NTSC"),                    TUNER_PHILIPS_NTSC                ),
    PC( _T("PHILIPS_SECAM"),                TUNER_PHILIPS_SECAM                ),
    PC( _T("PHILIPS_PAL"),                    TUNER_PHILIPS_PAL                ),
    PC( _T("TEMIC_4002FH5_PAL"),            TUNER_TEMIC_4002FH5_PAL            ),
    PC( _T("TEMIC_4032FY5_NTSC"),            TUNER_TEMIC_4032FY5_NTSC        ),
    PC( _T("TEMIC_4062FY5_PAL_I"),            TUNER_TEMIC_4062FY5_PAL_I        ),
    PC( _T("TEMIC_4036FY5_NTSC"),            TUNER_TEMIC_4036FY5_NTSC        ),
    PC( _T("ALPS_TSBH1_NTSC"),                TUNER_ALPS_TSBH1_NTSC            ),
    PC( _T("ALPS_TSBE1_PAL"),                TUNER_ALPS_TSBE1_PAL            ),
    PC( _T("ALPS_TSBB5_PAL_I"),                TUNER_ALPS_TSBB5_PAL_I            ),
    PC( _T("ALPS_TSBE5_PAL"),                TUNER_ALPS_TSBE5_PAL            ),
    PC( _T("ALPS_TSBC5_PAL"),                TUNER_ALPS_TSBC5_PAL            ),
    PC( _T("TEMIC_4006FH5_PAL"),            TUNER_TEMIC_4006FH5_PAL            ),
    PC( _T("PHILIPS_1236D_NTSC_INPUT1"),    TUNER_PHILIPS_1236D_NTSC_INPUT1    ),
    PC( _T("PHILIPS_1236D_NTSC_INPUT2"),    TUNER_PHILIPS_1236D_NTSC_INPUT2    ),
    PC( _T("ALPS_TSCH6_NTSC"),                TUNER_ALPS_TSCH6_NTSC            ),
    PC( _T("TEMIC_4016FY5_PAL"),            TUNER_TEMIC_4016FY5_PAL            ),
    PC( _T("PHILIPS_MK2_NTSC"),                TUNER_PHILIPS_MK2_NTSC            ),
    PC( _T("TEMIC_4066FY5_PAL_I"),            TUNER_TEMIC_4066FY5_PAL_I        ),
    PC( _T("TEMIC_4006FN5_PAL"),            TUNER_TEMIC_4006FN5_PAL            ),
    PC( _T("TEMIC_4009FR5_PAL"),            TUNER_TEMIC_4009FR5_PAL            ),
    PC( _T("TEMIC_4039FR5_NTSC"),            TUNER_TEMIC_4039FR5_NTSC        ),
    PC( _T("TEMIC_4046FM5_MULTI"),            TUNER_TEMIC_4046FM5_MULTI        ),
    PC( _T("PHILIPS_PAL_DK"),                TUNER_PHILIPS_PAL_DK            ),
    PC( _T("PHILIPS_MULTI"),                TUNER_PHILIPS_MULTI                ),
    PC( _T("LG_I001D_PAL_I"),                TUNER_LG_I001D_PAL_I            ),
    PC( _T("LG_I701D_PAL_I"),                TUNER_LG_I701D_PAL_I            ),
    PC( _T("LG_R01F_NTSC"),                    TUNER_LG_R01F_NTSC                ),
    PC( _T("LG_B01D_PAL"),                    TUNER_LG_B01D_PAL                ),
    PC( _T("LG_B11D_PAL"),                    TUNER_LG_B11D_PAL                ),
    PC( _T("TEMIC_4009FN5_PAL"),            TUNER_TEMIC_4009FN5_PAL            ),
    PC( _T("MT2032"),                        TUNER_MT2032                    ),
    PC( _T("SHARP_2U5JF5540_NTSC"),            TUNER_SHARP_2U5JF5540_NTSC        ),
    PC( _T("LG_TAPCH701P_NTSC"),            TUNER_LG_TAPCH701P_NTSC            ),
    PC( _T("SAMSUNG_PAL_TCPM9091PD27"),        TUNER_SAMSUNG_PAL_TCPM9091PD27    ),
    PC( _T("TEMIC_4106FH5"),                TUNER_TEMIC_4106FH5                ),
    PC( _T("TEMIC_4012FY5"),                TUNER_TEMIC_4012FY5                ),
    PC( _T("TEMIC_4136FY5"),                TUNER_TEMIC_4136FY5                ),
    PC( _T("LG_TAPCNEW_PAL"),                TUNER_LG_TAPCNEW_PAL            ),
    PC( _T("PHILIPS_FM1216ME_MK3"),            TUNER_PHILIPS_FM1216ME_MK3        ),
    PC( _T("LG_TAPCNEW_NTSC"),                TUNER_LG_TAPCNEW_NTSC            ),
    PC( _T("MT2032_PAL"),                    TUNER_MT2032_PAL                ),
    PC( _T("PHILIPS_FI1286_NTSC_M_J"),        TUNER_PHILIPS_FI1286_NTSC_M_J    ),
    PC( _T("MT2050"),                        TUNER_MT2050                    ),
    PC( _T("MT2050_PAL"),                    TUNER_MT2050_PAL                ),
    PC( _T("PHILIPS_4IN1"),                    TUNER_PHILIPS_4IN1                ),
    PC( _T("TCL_2002N"),                    TUNER_TCL_2002N                  ),
    PC( _T("HITACHI_NTSC"),                    TUNER_HITACHI_NTSC              ),
    PC( _T("PHILIPS_PAL_MK"),                TUNER_PHILIPS_PAL_MK              ),
    PC( _T("PHILIPS_FM1236_MK3"),            TUNER_PHILIPS_FM1236_MK3          ),
    PC( _T("LG_NTSC_TAPE"),                    TUNER_LG_NTSC_TAPE              ),
    PC( _T("TNF_8831BGFF"),                    TUNER_TNF_8831BGFF              ),
    PC( _T("PHILIPS_FM1256_IH3"),            TUNER_PHILIPS_FM1256_IH3          ),
    PC( _T("PHILIPS_FQ1286"),                TUNER_PHILIPS_FQ1286              ),
    PC( _T("LG_PAL_TAPE"),                    TUNER_LG_PAL_TAPE                  ),
    PC( _T("TUNER_PHILIPS_FQ1216AME_MK4"),  TUNER_PHILIPS_FQ1216AME_MK4        ),
    PC( _T("PHILIPS_FQ1236A_MK4"),            TUNER_PHILIPS_FQ1236A_MK4          ),
    PC( _T("PHILIPS_TDA8275"),                TUNER_TDA8275                      ),
    PC( _T("YMEC_TVF_8531MF"),              TUNER_YMEC_TVF_8531MF           ),
    PC( _T("YMEC_TVF_5533MF"),              TUNER_YMEC_TVF_5533MF           ),
    PC( _T("TENA_9533_DI"),                 TUNER_TENA_9533_DI              ),
    PC( _T("PHILIPS_FMD1216ME_MK3"),        TUNER_PHILIPS_FMD1216ME_MK3     ),
    PC( NULL )
};

const CParseConstant k_parseTDAFormatConstants[] =
{
    PC( _T("GLOBAL"),                        TDA9887_FORMAT_NONE            ),
    PC( _T("PAL-BG"),                        TDA9887_FORMAT_PAL_BG        ),
    PC( _T("PAL-I"),                        TDA9887_FORMAT_PAL_I        ),
    PC( _T("PAL-DK"),                        TDA9887_FORMAT_PAL_DK        ),
    PC( _T("PAL-MN"),                        TDA9887_FORMAT_PAL_MN        ),
    PC( _T("SECAM-L"),                        TDA9887_FORMAT_SECAM_L        ),
    PC( _T("SECAM-DK"),                        TDA9887_FORMAT_SECAM_DK        ),
    PC( _T("NTSC-M"),                        TDA9887_FORMAT_NTSC_M        ),
    PC( _T("NTSC-JP"),                        TDA9887_FORMAT_NTSC_JP        ),
    PC( _T("Radio"),                        TDA9887_FORMAT_RADIO        ),
    PC( NULL )
};

const CParseConstant k_parseCarrierConstants[] =
{
    PC( _T("intercarrier"),                0 ),
    PC( _T("qss"),                        1 ),
    PC( NULL )
};

const CParseConstant k_parseYesNoConstants[] =
{
    PC( _T("yes"),                        1 ),
    PC( _T("no"),                        0 ),
    PC( _T("TRUE"),                        1 ),
    PC( _T("FALSE"),                    0 ),
    PC( _T("active"),                    1 ),
    PC( _T("inactive"),                    0 ),
    PC( _T("1"),                        1 ),
    PC( _T("0"),                        0 ),
    PC( NULL )
};

const CParseConstant k_parseTakeoverPointConstants[] =
{
    PC( _T("min"),                        TDA9887_SM_TAKEOVERPOINT_MIN        ),
    PC( _T("max"),                        TDA9887_SM_TAKEOVERPOINT_MAX        ),
    PC( _T("default"),                    TDA9887_SM_TAKEOVERPOINT_DEFAULT    ),
    PC( NULL )
};

const CParseTag k_parseUseTDA9887SetOverride[] =
{
    PT( _T("Format"),            PARSE_CONSTANT,        0, 8,    k_parseTDAFormatConstants,        PASS_TO_PARENT    ),
    PT( _T("Intercarrier"),        0,                    0, 0,    NULL,                            PASS_TO_PARENT    ),
    PT( _T("QSS"),                0,                    0, 0,    NULL,                            PASS_TO_PARENT    ),
    PT( _T("Carrier"),            PARSE_CONSTANT,        0, 16,    k_parseCarrierConstants,        PASS_TO_PARENT    ),
    PT( _T("OutputPort1"),        PARSE_CONSTANT,        0, 8,    k_parseYesNoConstants,            PASS_TO_PARENT    ),
    PT( _T("OutputPort2"),        PARSE_CONSTANT,        0, 8,    k_parseYesNoConstants,            PASS_TO_PARENT    ),
    PT( _T("TakeOverPoint"),    PARSE_NUM_OR_CONST,    0, 8,    k_parseTakeoverPointConstants,    PASS_TO_PARENT    ),
    PT( NULL )
};

const CParseTag k_parseUseTDA9887[] =
{
    PT( _T("Use"),                PARSE_CONSTANT,        0, 8,    k_parseYesNoConstants,            PASS_TO_PARENT    ),
    PT( _T("SetModes"),            PARSE_CHILDREN,        0, 9,    k_parseUseTDA9887SetOverride,    PASS_TO_PARENT    ),
    PT( NULL )
};


//////////////////////////////////////////////////////////////////////////
// Interpreters
//////////////////////////////////////////////////////////////////////////

BOOL ReadTunerProc(IN int report, IN const CParseTag* tag, IN unsigned char type,
                   IN const CParseValue* value, IN OUT TParseTunerInfo* tunerInfo)
{
    if (report == REPORT_OPEN)
    {
        // Set the default tuner id for if there is no value.
        tunerInfo->tunerId = TUNER_ABSENT;
    }
    else if (report == REPORT_VALUE)
    {
        int n = value->GetNumber();
        if (n < TUNER_AUTODETECT || n >= TUNER_LASTONE)
        {
            throw std::exception("Invalid tuner Id");
        }
        tunerInfo->tunerId = static_cast<eTunerId>(n);
    }
    else if (report == REPORT_CLOSE)
    {
        // The value could be considered read at the end of REPORT_VALUE
        // but putting the 'return TRUE' here is also good.
        return TRUE;
    }
    return FALSE;
}


BOOL ReadUseTDA9887Proc(IN int report, IN const CParseTag* tag, IN unsigned char type,
                        IN const CParseValue* value, IN OUT TParseUseTDA9887Info* useTDA9887Info)
{
    // Use
    if (tag == k_parseUseTDA9887 + 0)
    {
        if (report == REPORT_VALUE)
        {
            useTDA9887Info->useTDA9887 = value->GetNumber() != 0;
        }
    }
    // SetOverride
    else if (tag == k_parseUseTDA9887 + 1)
    {
        if (report == REPORT_OPEN)
        {
            useTDA9887Info->_readingFormat = TDA9887_FORMAT_NONE;
            useTDA9887Info->_readingModes.mask = 0;
            useTDA9887Info->_readingModes.bits = 0;
        }
        else if (report == REPORT_CLOSE)
        {
            // This should not fail because constants only provide 0 .. TDA9887_LASTFORMAT.
            _ASSERTE(useTDA9887Info->_readingFormat == TDA9887_FORMAT_NONE ||
                (useTDA9887Info->_readingFormat >= 0 &&
                useTDA9887Info->_readingFormat < TDA9887_FORMAT_LASTONE));

            // It is pointless copying if mask is zero.
            if (useTDA9887Info->_readingModes.mask != 0)
            {
                if (useTDA9887Info->_readingFormat == TDA9887_FORMAT_NONE)
                {
                    // Copy for all formats.
                    for (int i = 0; i < TDA9887_FORMAT_LASTONE; i++)
                    {
                        useTDA9887Info->tdaModes[i].bits &= ~useTDA9887Info->_readingModes.mask;
                        useTDA9887Info->tdaModes[i].bits |= useTDA9887Info->_readingModes.bits;
                        useTDA9887Info->tdaModes[i].mask |= useTDA9887Info->_readingModes.mask;
                    }
                }
                else
                {
                    int i = useTDA9887Info->_readingFormat;
                    useTDA9887Info->tdaModes[i].bits &= ~useTDA9887Info->_readingModes.mask;
                    useTDA9887Info->tdaModes[i].bits |= useTDA9887Info->_readingModes.bits;
                    useTDA9887Info->tdaModes[i].mask |= useTDA9887Info->_readingModes.mask;
                }
            }
        }
    }
    // Format
    else if (tag == k_parseUseTDA9887SetOverride + 0)
    {
        if (report == REPORT_VALUE)
        {
            useTDA9887Info->_readingFormat = static_cast<eTDA9887Format>(value->GetNumber());
        }
    }
    // Intercarrier
    else if (tag == k_parseUseTDA9887SetOverride + 1)
    {
        if (report == REPORT_TAG)
        {
            SetTDA9887ModeMaskAndBits(useTDA9887Info->_readingModes,
                TDA9887_SM_CARRIER_QSS, FALSE);
        }
    }
    // QSS
    else if (tag == k_parseUseTDA9887SetOverride + 2)
    {
        if (report == REPORT_TAG)
        {
            SetTDA9887ModeMaskAndBits(useTDA9887Info->_readingModes,
                TDA9887_SM_CARRIER_QSS, TRUE);
        }
    }
    // Carrier
    else if (tag == k_parseUseTDA9887SetOverride + 3)
    {
        if (report == REPORT_VALUE)
        {
            SetTDA9887ModeMaskAndBits(useTDA9887Info->_readingModes,
                TDA9887_SM_CARRIER_QSS, value->GetNumber() != 0);
        }
    }
    // OutputPort1
    else if (tag == k_parseUseTDA9887SetOverride + 4)
    {
        if (report == REPORT_VALUE)
        {
            SetTDA9887ModeMaskAndBits(useTDA9887Info->_readingModes,
                TDA9887_SM_OUTPUTPORT1_INACTIVE, value->GetNumber() == 0);
        }
    }
    // OutputPort2
    else if (tag == k_parseUseTDA9887SetOverride + 5)
    {
        if (report == REPORT_VALUE)
        {
            SetTDA9887ModeMaskAndBits(useTDA9887Info->_readingModes,
                TDA9887_SM_OUTPUTPORT2_INACTIVE, value->GetNumber() == 0);
        }
    }
    // TakeOverPoint
    else if (tag == k_parseUseTDA9887SetOverride + 6)
    {
        if (report == REPORT_VALUE)
        {
            BYTE point = static_cast<BYTE>(value->GetNumber());
            if (point & ~TDA9887_SM_TAKEOVERPOINT_MASK)
            {
                throw std::exception("Invalid value of TakeOverPoint");
            }

            useTDA9887Info->_readingModes.mask |= TDA9887_SM_TAKEOVERPOINT_MASK;
            useTDA9887Info->_readingModes.bits &= ~TDA9887_SM_TAKEOVERPOINT_MASK;
            useTDA9887Info->_readingModes.bits |= (point << TDA9887_SM_TAKEOVERPOINT_OFFSET);
        }
    }
    // This unknown tag is hopefully the parent tag.  There's no other way of
    // checking.  An incorrect tag here can only result from incorrect setup
    // outside ParsingCommon's control.
    else
    {
        if (report == REPORT_OPEN)
        {
            useTDA9887Info->useTDA9887 = TRUE;
            ZeroMemory(useTDA9887Info->tdaModes, sizeof(useTDA9887Info->tdaModes));
        }
        else if (report == REPORT_CLOSE)
        {
            // Everything about the UseTDA9887 tag is considered read and
            // ParseUseTDA9887Info considered fill at this point.
            return TRUE;
        }
    }
    return FALSE;
}

static void SetTDA9887ModeMaskAndBits(OUT TTDA9887Modes& specifics, IN BYTE bit, IN BOOL set)
{
    specifics.mask |= bit;
    if (set)
    {
        specifics.bits |= bit;
    }
    else
    {
        specifics.bits &= ~bit;
    }
}


