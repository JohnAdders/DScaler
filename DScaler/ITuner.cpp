//
// $Id: ITuner.cpp,v 1.1 2001-11-25 02:03:21 ittarnavsky Exp $
//
/////////////////////////////////////////////////////////////////////////////
//
// copyleft 2001 itt@myself.com
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
//
// $Log: not supported by cvs2svn $
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ITuner.h"

extern "C" {

const char *TunerNames[TUNER_LASTONE] =
{
    // TUNER_ABSENT
        "NoTuner",
    // TUNER_PHILIPS_PAL_I
        "PHILIPS PAL_I",
    // TUNER_PHILIPS_NTSC
        "PHILIPS NTSC",
    // TUNER_PHILIPS_SECAM
        "PHILIPS SECAM",
    // TUNER_PHILIPSFY5_PAL
        "PHILIPS PAL",
    // TUNER_TEMIC_4002FH5_PAL
        "Temic 4002 FH5 PAL",
    // TUNER_TEMIC_4032FY5_NTSC
        "Temic 4036 FY5 NTSC",
    // TUNER_TEMIC_4062FY5_PAL_I
        "Temic PAL_I (4062 FY5)",
    // TUNER_TEMIC_4036FY5_NTSC
        "Temic 4036 FY5 NTSC",
    // TUNER_ALPS_TSBH1_NTSC
        "ALPS HSBH1",
    // TUNER_ALPS_TSBE1_PAL
        "ALPS TSBE1",
    // TUNER_ALPS_TSBB5_PAL_I
        "ALPS TSBB5",
    // TUNER_ALPS_TSBE5_PAL
        "ALPS TSBE5",
    // TUNER_ALPS_TSBC5_PAL
        "ALPS TSBC5",
    // TUNER_TEMIC_4006FH5_PAL
        "Temic 4006FH5",
    // TUNER_PHILIPS_1236D_NTSC_INPUT1
        "PHILIPS 1236D ATSC/NTSC Input 1",
    // TUNER_PHILIPS_1236D_NTSC_INPUT2
        "PHILIPS 1236D ATSC/NTSC Input 2",
    // TUNER_ALPS_TSCH6_NTSC
        "ALPS TSCH6",
    // TUNER_TEMIC_4016FY5_PAL
        "Temic PAL_DK (4016 FY5)",
    // TUNER_PHILIPS_MK2_NTSC
        "PHILIPS NTSC_M (MK2)",
    // TUNER_TEMIC_4066FY5_PAL_I
        "Temic PAL_I (4066 FY5)",
    // TUNER_TEMIC_4006FN5_PAL
        "Temic PAL* auto (4006 FN5)",
    // TUNER_TEMIC_4009FR5_PAL
        "Temic PAL (4009 FR5)",
    // TUNER_TEMIC_4039FR5_NTSC
        "Temic NTSC (4039 FR5)",
    // TUNER_TEMIC_4046FM5_MULTI
        "Temic PAL/SECAM multi (4046 FM5)",
    // TUNER_PHILIPS_PAL_DK
        "PHILIPS PAL_DK",
    // TUNER_PHILIPS_MULTI
        "PHILIPS PAL/SECAM multi (FQ1216ME)",
    // TUNER_LG_I001D_PAL_I
        "LG PAL_I+FM (TAPC-I001D)",
    // TUNER_LG_I701D_PAL_I
        "LG PAL_I (TAPC-I701D)",
    // TUNER_LG_R01F_NTSC
        "LG NTSC+FM (TPI8NSR01F)",
    // TUNER_LG_B01D_PAL
        "LG PAL_BG+FM (TPI8PSB01D)",
    // TUNER_LG_B11D_PAL
        "LG PAL_BG (TPI8PSB11D)",
    // TUNER_TEMIC_4009FN5_PAL
        "Temic PAL* auto + FM (4009 FN5)",
    // TUNER_MT2032
        "MT2032 universal",
    // TUNER_SHARP_2U5JF5540_NTSC
        "SHARP NTSC_JP (2U5JF5540)",
};

}