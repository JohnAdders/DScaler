/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Common.cpp,v 1.6 2002-10-20 07:41:04 atnak Exp $
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
// Change Log
//
// Date          Developer             Changes
//
// 01 Oct 2002   Atsushi Nakagawa      Initial Release
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.5  2002/10/16 11:38:46  atnak
// cleaned up audio standard stuff
//
// Revision 1.4  2002/10/12 01:38:54  atnak
// VBI_DecodeLine() doesn't work properly with only 16 VBI lines
//
// Revision 1.3  2002/10/09 13:20:16  atnak
// fixed up field start lines
//
// Revision 1.2  2002/10/08 19:35:45  atnak
// various fixes, tweaks, cleanups
//
// Revision 1.1  2002/10/03 23:42:07  atnak
// SAA7134Common.h SAA7134Common.cpp SAA7134Card_Video.cpp added
//
//
//////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "SAA7134_Defines.h"
#include "SAA7134Common.h"

#include "DebugLog.h"


CSAA7134Common::TVideoStandardDefinition CSAA7134Common::m_VideoStandards[] =
{
    // VIDEOSTANDARD_AUTODETECT
    {
        288, 720, 0, 20, TRUE, 0, 18, 0x0,
    },
    // VIDEOSTANDARD_PAL_BGDHI
    {
        288, 720, 0, 19, TRUE, 0, 18, 0x0,
    },
    // VIDEOSTANDARD_PAL_N_COMBO
    {
        288, 720, 0, 19, TRUE, 0, 18, 0x2,
    },
    // VIDEOSTANDARD_PAL_60
    {
        240, 720, 0, 19, FALSE, 0, 18, 0x1,
    },
    // VIDEOSTANDARD_PAL_M
    {
        240, 720, 0, 19, FALSE, 0, 18, 0x3,
    },
    // VIDEOSTANDARD_SECAM
    {
        288, 720, 0, 20, TRUE, 0, 18, 0x5,
    },
    // VIDEOSTANDARD_NTSC_M
    {
        240, 720, 0, 20, FALSE, 0, 18, 0x0,
    },
    // VIDEOSTANDARD_NTSC_60 (NTSC 4.43)
    {
        240, 720, 0, 20, FALSE, 0, 18, 0x2,
    },
    // VIDEOSTANDARD_NTSC_Japan
    {
        240, 720, 0, 20, FALSE, 0, 18, 0x4,
    },
    // VIDEOSTANDARD_NTSC_50
    {
        288, 720, 0, 20, TRUE, 0, 18, 0x1,
    },
    // VIDEOSTANDARD_NTSC_N
    {
        288, 720, 0, 20, TRUE, 0, 18, 0x3,
    },
};

CSAA7134Common::TAudioStandardDefinition CSAA7134Common::m_AudioStandards[] =
{
    // AUDIOSTANDARD_BG_DUAL_FM
    {
        "B/G-Dual FM-Stereo",
        AUDIO_CARRIER_5_5,
        AUDIO_CARRIER_5_7421875,
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_FM,
        AUDIOFMDEEMPHASIS_50_MICROS,
        AUDIOFMDEEMPHASIS_50_MICROS,
    },
    // AUDIOSTANDARD_DK1_DUAL_FM
    {
        "D/K1-Dual FM-Stereo",
        AUDIO_CARRIER_6_5,
        AUDIO_CARRIER_6_2578125,
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_FM,
        AUDIOFMDEEMPHASIS_50_MICROS,
        AUDIOFMDEEMPHASIS_50_MICROS,
    },
    // AUDIOSTANDARD_DK2_DUAL_FM
    {
        "D/K2-Dual FM-Stereo",
        AUDIO_CARRIER_6_5,
        AUDIO_CARRIER_6_7421875,
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_FM,
        AUDIOFMDEEMPHASIS_50_MICROS,
        AUDIOFMDEEMPHASIS_50_MICROS,
    },
    // AUDIOSTANDARD_DK_FM_MONO
    {
        "D/K-FM-Mono with HDEV3",
        AUDIO_CARRIER_6_5,
        AUDIO_CARRIER_6_5,
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_NONE,
        AUDIOFMDEEMPHASIS_50_MICROS,
        AUDIOFMDEEMPHASIS_50_MICROS,
    },
    // AUDIOSTANDARD_DK3_DUAL_FM
    {
        "D/K3-Dual FM-Stereo",
        AUDIO_CARRIER_6_5,
        AUDIO_CARRIER_5_7421875,
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_FM,
        AUDIOFMDEEMPHASIS_50_MICROS,
        AUDIOFMDEEMPHASIS_50_MICROS,
    },
    // AUDIOSTANDARD_M_DUAL_FM
    {
        "M-Dual FM-Stereo",
        AUDIO_CARRIER_4_5,
        AUDIO_CARRIER_4_724212,
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_FM_KOREA,
        AUDIOFMDEEMPHASIS_75_MICROS,
        AUDIOFMDEEMPHASIS_75_MICROS,
    },
    // AUDIOSTANDARD_BG_NICAM_FM
    {
        "B/G-NICAM-FM",
        AUDIO_CARRIER_5_5,
        AUDIO_CARRIER_5_85,
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_NICAM,
        AUDIOFMDEEMPHASIS_OFF,
        AUDIOFMDEEMPHASIS_OFF,
    },
    // AUDIOSTANDARD_L_NICAM_AM
    {
        "L-NICAM-AM",
        AUDIO_CARRIER_6_5,
        AUDIO_CARRIER_5_85,
        AUDIOCHANNELMODE_AM,
        AUDIOCHANNELMODE_NICAM,
        AUDIOFMDEEMPHASIS_OFF,
        AUDIOFMDEEMPHASIS_OFF,
    },
    // AUDIOSTANDARD_I_NICAM_FM
    {
        "I-NICAM-FM",
        AUDIO_CARRIER_6_0,
        AUDIO_CARRIER_6_552,
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_NICAM,
        AUDIOFMDEEMPHASIS_OFF,
        AUDIOFMDEEMPHASIS_OFF,
    },
    // AUDIOSTANDARD_DK_NICAM_FM
    {
        "D/K-NICAM-FM",
        AUDIO_CARRIER_6_5,
        AUDIO_CARRIER_5_85,
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_NICAM,
        AUDIOFMDEEMPHASIS_OFF,
        AUDIOFMDEEMPHASIS_OFF,
    },
    // AUDIOSTANDARD_DK_NICAM_FM_HDEV2
    {
        "D/K-NICAM-FM with HDEV2",
        AUDIO_CARRIER_6_5,
        AUDIO_CARRIER_5_85,
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_NICAM,
        AUDIOFMDEEMPHASIS_OFF,
        AUDIOFMDEEMPHASIS_OFF,
    },
    // AUDIOSTANDARD_DK_NICAM_FM_HDEV3
    {
        "D/K-NICAM-FM with HDEV3",
        AUDIO_CARRIER_6_5,
        AUDIO_CARRIER_5_85,
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_NICAM,
        AUDIOFMDEEMPHASIS_OFF,
        AUDIOFMDEEMPHASIS_OFF,
    },
    // AUDIOSTANDARD_M_BTSC
    {
        "M-BTSC-Stereo",
        AUDIO_CARRIER_4_5,
        AUDIO_CARRIER_4_5,
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_BTSC,
        AUDIOFMDEEMPHASIS_50_MICROS,
        AUDIOFMDEEMPHASIS_50_MICROS,
    },
    // AUDIOSTANDARD_M_BTSC_MONO
    {
        "M-BTSC-Mono + SAP",
        AUDIO_CARRIER_4_5,
        AUDIO_CARRIER_4_5,
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_BTSC,
        AUDIOFMDEEMPHASIS_50_MICROS,
        AUDIOFMDEEMPHASIS_50_MICROS,
    },
    // AUDIOSTANDARD_M_EIAJ
    {
        "M-EIA-J Japan Stereo",
        AUDIO_CARRIER_4_5,
        AUDIO_CARRIER_4_5,
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_EIAJ,
        AUDIOFMDEEMPHASIS_50_MICROS,
        AUDIOFMDEEMPHASIS_50_MICROS,
    },
    // AUDIOSTANDARD_FM_RADIO
    {
        "FM-Stereo Radio",
        AUDIO_CARRIER_10_7,
        AUDIO_CARRIER_10_7,
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_FM,
        AUDIOFMDEEMPHASIS_50_MICROS,
        AUDIOFMDEEMPHASIS_50_MICROS,
    },
    // AUDIOSTANDARD_SAT
    {
        "SAT-Stereo",
        AUDIO_CARRIER_7_02,
        AUDIO_CARRIER_7_20,
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_FM,
        AUDIOFMDEEMPHASIS_50_MICROS,
        AUDIOFMDEEMPHASIS_50_MICROS,
    },
    // AUDIOSTANDARD_SAT_MONO
    {
        "SAT-Mono",
        AUDIO_CARRIER_6_5,
        AUDIO_CARRIER_6_5,
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_NONE,
        AUDIOFMDEEMPHASIS_50_MICROS,
        AUDIOFMDEEMPHASIS_50_MICROS,
    },
    // AUDIOSTANDARD_SAT_ADR
    {
        "SAT ASTRA Digital Radio",
        AUDIO_CARRIER_6_12,
        AUDIO_CARRIER_6_12,
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_FM,
        AUDIOFMDEEMPHASIS_50_MICROS,
        AUDIOFMDEEMPHASIS_50_MICROS,
    },
};


CSAA7134Common::eAudioCarrier CSAA7134Common::m_AudioCarrierList[] =
{
    { AUDIO_CARRIER_4_5,        },
    { AUDIO_CARRIER_4_724212,   },
    { AUDIO_CARRIER_5_5,        },
    { AUDIO_CARRIER_5_7421875,  },
    { AUDIO_CARRIER_5_85,       },
    { AUDIO_CARRIER_6_0,        },
    { AUDIO_CARRIER_6_12,       },
    { AUDIO_CARRIER_6_2578125,  },
    { AUDIO_CARRIER_6_5,        },
    { AUDIO_CARRIER_6_552,      },
    { AUDIO_CARRIER_6_7421875,  },
    { AUDIO_CARRIER_7_02,       },
    { AUDIO_CARRIER_7_20,       },
    { AUDIO_CARRIER_10_7,       },
};


char* CSAA7134Common::m_AudioFMDeemphasisName[] =
{
    { "OFF"      },
    { "50us"     },
    { "60us"     },
    { "75us"     },
    { "J17"      },
    { "Adaptive" },
};


BOOL CSAA7134Common::IsRegionIDVideo(eRegionID RegionID)
{
    if (RegionID == REGIONID_VIDEO_A || RegionID == REGIONID_VIDEO_B)
    {
        return TRUE;
    }
    return FALSE;
}

BOOL CSAA7134Common::IsRegionIDVBI(eRegionID RegionID)
{
    if (RegionID == REGIONID_VBI_A || RegionID == REGIONID_VBI_B)
    {
        return TRUE;
    }
    return FALSE;
}

CSAA7134Common::eTaskID CSAA7134Common::RegionID2TaskID(eRegionID RegionID)
{
    switch (RegionID)
    {
    case REGIONID_VIDEO_A:
    case REGIONID_VBI_A:
        return TASKID_A;

    case REGIONID_VIDEO_B:
    case REGIONID_VBI_B:
        return TASKID_B;
    }

    // NEVER_GET_HERE;
    return TASKID_A;
}

CSAA7134Common::eRegionID CSAA7134Common::TaskID2VideoRegion(eTaskID TaskID)
{
    if (TaskID == TASKID_A)
    {
        return REGIONID_VIDEO_A;
    }
    return REGIONID_VIDEO_B;
}

CSAA7134Common::eRegionID CSAA7134Common::TaskID2VBIRegion(eTaskID TaskID)
{
    if (TaskID == TASKID_A)
    {
        return REGIONID_VBI_A;
    }
    return REGIONID_VBI_B;
}


CSAA7134Common::eVideoStandard CSAA7134Common::TVFormat2VideoStandard(eVideoFormat TVFormat)
{
    switch (TVFormat)
    {
    case VIDEOFORMAT_PAL_B:
    case VIDEOFORMAT_PAL_D:
    case VIDEOFORMAT_PAL_G:
    case VIDEOFORMAT_PAL_H:
    case VIDEOFORMAT_PAL_I:
        return VIDEOSTANDARD_PAL_BGDHI;

    case VIDEOFORMAT_PAL_M:
        return VIDEOSTANDARD_PAL_M;

    case VIDEOFORMAT_PAL_N:
        return VIDEOSTANDARD_NTSC_N;    // guessed

    case VIDEOFORMAT_PAL_60:
        return VIDEOSTANDARD_PAL_60;

    case VIDEOFORMAT_PAL_N_COMBO:
        return VIDEOSTANDARD_PAL_N_COMBO;

    case VIDEOFORMAT_SECAM_B:
    case VIDEOFORMAT_SECAM_D:
    case VIDEOFORMAT_SECAM_G:
    case VIDEOFORMAT_SECAM_H:
    case VIDEOFORMAT_SECAM_K:
    case VIDEOFORMAT_SECAM_K1:
    case VIDEOFORMAT_SECAM_L:
    case VIDEOFORMAT_SECAM_L1:
        return VIDEOSTANDARD_SECAM;

    case VIDEOFORMAT_NTSC_M:
        return VIDEOSTANDARD_NTSC_M;

    case VIDEOFORMAT_NTSC_M_Japan:
        return VIDEOSTANDARD_NTSC_Japan;

    case VIDEOFORMAT_NTSC_50:
        return VIDEOSTANDARD_NTSC_50;

    default:
        // NEVER_GET_HERE;
        break;
    }

    return VIDEOSTANDARD_INVALID;
}


BOOL CSAA7134Common::IsPALVideoStandard(eVideoStandard VideoStandard)
{
    switch (VideoStandard)
    {
    case VIDEOSTANDARD_PAL_BGDHI:
    case VIDEOSTANDARD_PAL_N_COMBO:
    case VIDEOSTANDARD_PAL_60:
    case VIDEOSTANDARD_PAL_M:
        return TRUE;
    }
    return FALSE;
}

BOOL CSAA7134Common::IsSECAMVideoStandard(eVideoStandard VideoStandard)
{
    switch (VideoStandard)
    {
    case VIDEOSTANDARD_SECAM:
        return TRUE;
    }
    return FALSE;
}

BOOL CSAA7134Common::IsNTSCVideoStandard(eVideoStandard VideoStandard)
{
    switch (VideoStandard)
    {
    case VIDEOSTANDARD_NTSC_M:
    case VIDEOSTANDARD_NTSC_60:
    case VIDEOSTANDARD_NTSC_Japan:
    case VIDEOSTANDARD_NTSC_50:
    case VIDEOSTANDARD_NTSC_N:
        return TRUE;
    }
    return FALSE;
}


CSAA7134Common::eAudioStandard CSAA7134Common::TVFormat2AudioStandard(eVideoFormat VideoFormat)
{
    // Guess the correct format
    switch(VideoFormat)
    {
    case VIDEOFORMAT_PAL_B:
        return AUDIOSTANDARD_BG_DUAL_FM;

    case VIDEOFORMAT_PAL_D:
        return AUDIOSTANDARD_DK_NICAM_FM;

    case VIDEOFORMAT_PAL_G:
        return AUDIOSTANDARD_BG_NICAM_FM;

    case VIDEOFORMAT_PAL_H:
        return AUDIOSTANDARD_BG_DUAL_FM;

    case VIDEOFORMAT_PAL_I:
        return AUDIOSTANDARD_I_NICAM_FM;

    case VIDEOFORMAT_PAL_M:
        return AUDIOSTANDARD_M_DUAL_FM;

    case VIDEOFORMAT_PAL_N:
        return AUDIOSTANDARD_M_DUAL_FM;

    case VIDEOFORMAT_PAL_N_COMBO:
        return AUDIOSTANDARD_M_DUAL_FM;

    case VIDEOFORMAT_SECAM_B:
        return AUDIOSTANDARD_BG_DUAL_FM;

    case VIDEOFORMAT_SECAM_D:
        return AUDIOSTANDARD_DK_NICAM_FM;

    case VIDEOFORMAT_SECAM_G:
        return AUDIOSTANDARD_BG_NICAM_FM;

    case VIDEOFORMAT_SECAM_H:
        return AUDIOSTANDARD_DK_NICAM_FM;

    case VIDEOFORMAT_SECAM_K:
        return AUDIOSTANDARD_DK_NICAM_FM;

    case VIDEOFORMAT_SECAM_K1:
        return AUDIOSTANDARD_DK1_DUAL_FM;

    case VIDEOFORMAT_SECAM_L:
        return AUDIOSTANDARD_L_NICAM_AM;

    case VIDEOFORMAT_SECAM_L1:
        return AUDIOSTANDARD_L_NICAM_AM;

    case VIDEOFORMAT_NTSC_M:
        return AUDIOSTANDARD_M_DUAL_FM;

    case VIDEOFORMAT_NTSC_M_Japan:
        return AUDIOSTANDARD_M_DUAL_FM;

    default:
    case VIDEOFORMAT_PAL_60:
    case VIDEOFORMAT_NTSC_50:
        return AUDIOSTANDARD_BG_DUAL_FM;
    }

    return AUDIOSTANDARD_BG_DUAL_FM;
}


BOOL CSAA7134Common::IsDualFMAudioStandard(eAudioStandard AudioStandard)
{
    eAudioCarrierMode Carrier2Mode;

    Carrier2Mode = m_AudioStandards[AudioStandard].Carrier2Mode;

    if (Carrier2Mode == AUDIOCHANNELMODE_FM ||
        Carrier2Mode == AUDIOCHANNELMODE_FM_KOREA)
    {
        return TRUE;
    }
    return FALSE;
}

BOOL CSAA7134Common::IsNICAMAudioStandard(eAudioStandard AudioStandard)
{
    if (m_AudioStandards[AudioStandard].Carrier2Mode == AUDIOCHANNELMODE_NICAM)
    {
        return TRUE;
    }
    return FALSE;
}


int CSAA7134Common::GetMaxAudioCarrierNames()
{
    return sizeof(m_AudioCarrierList) / sizeof(eAudioCarrier);
}


int CSAA7134Common::GetMaxFMDeemphasisNames()
{
    return sizeof(m_AudioFMDeemphasisName) / sizeof(char*);
}
