/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Common.cpp,v 1.1 2002-10-03 23:42:07 atnak Exp $
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
        288, 720, 0, 24, TRUE, 3, 21, 0x0,
    },
    // VIDEOSTANDARD_PAL_BGDHI
    {
        288, 720, 0, 22, TRUE, 3, 21, 0x0,
    },
    // VIDEOSTANDARD_PAL_N_COMBO
    {
        288, 720, 0, 22, TRUE, 3, 21, 0x2,
    },
    // VIDEOSTANDARD_PAL_60
    {
        240, 720, 0, 22, FALSE, 3, 21, 0x1,
    },
    // VIDEOSTANDARD_PAL_M
    {
        240, 720, 0, 22, FALSE, 3, 21, 0x3,
    },
    // VIDEOSTANDARD_SECAM
    {
        288, 720, 0, 24, TRUE, 3, 21, 0x5,
    },
    // VIDEOSTANDARD_NTSC_M
    {
        240, 720, 0, 24, FALSE, 3, 21, 0x0,
    },
    // VIDEOSTANDARD_NTSC_60 (NTSC 4.43)
    {
        240, 720, 0, 24, FALSE, 3, 21, 0x2,
    },
    // VIDEOSTANDARD_NTSC_Japan
    {
        240, 720, 0, 24, FALSE, 3, 21, 0x4,
    },
    // VIDEOSTANDARD_NTSC_50
    {
        288, 720, 0, 24, TRUE, 3, 21, 0x1,
    },
    // VIDEOSTANDARD_NTSC_N
    {
        288, 720, 0, 24, TRUE, 3, 21, 0x3,
    },
};

CSAA7134Common::TAudioStandardDefinition CSAA7134Common::m_AudioStandards[] =
{
    // AUDIOSTANDARD_BG_DUAL_FM
    { 
        "[PAL] B/G-Dual FM-Stereo",
        AUDIO_CARRIER_5_5,  
        AUDIO_CARRIER_5_7421875,
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_FM,
        AUDIOFMDEEMPHASIS_50_MICROS,
        AUDIOFMDEEMPHASIS_50_MICROS,
        FIR_BG_DK_DUAL_FM,
    },
    // AUDIOSTANDARD_DK1_DUAL_FM,         
    {
        "[PAL] D/K1-Dual FM-Stereo",
        AUDIO_CARRIER_6_5,  
        AUDIO_CARRIER_6_2578125,
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_FM,
        AUDIOFMDEEMPHASIS_50_MICROS,
        AUDIOFMDEEMPHASIS_50_MICROS,
        FIR_BG_DK_DUAL_FM,
    },
    // AUDIOSTANDARD_DK2_DUAL_FM,         
    {
        "[PAL] D/K2-Dual FM-Stereo",
        AUDIO_CARRIER_6_5,  
        AUDIO_CARRIER_6_7421875,  
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_FM,
        AUDIOFMDEEMPHASIS_50_MICROS,
        AUDIOFMDEEMPHASIS_50_MICROS,
        FIR_BG_DK_DUAL_FM,
    },
    // AUDIOSTANDARD_DK3_DUAL_FM,         
    { 
        "[PAL]D/K3-Dual FM-Stereo",
        AUDIO_CARRIER_6_5,  
        AUDIO_CARRIER_5_7421875,  
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_FM,
        AUDIOFMDEEMPHASIS_50_MICROS,
        AUDIOFMDEEMPHASIS_50_MICROS,
        FIR_BG_DK_DUAL_FM
    },
    // AUDIOSTANDARD_BG_NICAM_FM,         
    { 
        "[PAL] B/G-NICAM-FM",
        AUDIO_CARRIER_5_5,  
        AUDIO_CARRIER_5_85,       
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_NICAM,
        AUDIOFMDEEMPHASIS_OFF,
        AUDIOFMDEEMPHASIS_OFF,
        FIR_BG_DK_NICAM,
    },
    // AUDIOSTANDARD_I_NICAM_FM,          
    { 
        "[PAL] I-NICAM-FM",
        AUDIO_CARRIER_6_0,  
        AUDIO_CARRIER_6_552,      
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_NICAM,
        AUDIOFMDEEMPHASIS_OFF,
        AUDIOFMDEEMPHASIS_OFF,
        FIR_I_NICAM,
    },
    // AUDIOSTANDARD_DK_NICAM_FM,
    { 
        "[PAL] D/K-NICAM-FM",
        AUDIO_CARRIER_6_5,  
        AUDIO_CARRIER_5_85,       
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_NICAM,
        AUDIOFMDEEMPHASIS_OFF,
        AUDIOFMDEEMPHASIS_OFF,
        FIR_BG_DK_NICAM,
    },
    // AUDIOSTANDARD_L_NICAM_AM,
    { 
        "[SECAM] L-NICAM-AM",
        AUDIO_CARRIER_6_5,  
        AUDIO_CARRIER_5_85,       
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_NICAM,
        AUDIOFMDEEMPHASIS_OFF,
        AUDIOFMDEEMPHASIS_OFF,
        FIR_L_NICAM,
    },
    // AUDIOSTANDARD_NTCS_MONO,
    {
        "[NTSC] M",
        AUDIO_CARRIER_4_5,  
        AUDIO_CARRIER_4_5,       
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_AM,
        AUDIOFMDEEMPHASIS_50_MICROS,
        AUDIOFMDEEMPHASIS_50_MICROS,
        FIR_M_DUAL_FM,
    },
    // AUDIOSTANDARD_M_DUAL_FM,
    {
        "[NTSC] M/A2 FM-Stereo",
        AUDIO_CARRIER_4_5,  
        AUDIO_CARRIER_4_724212,       
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_AM,
        AUDIOFMDEEMPHASIS_50_MICROS,
        AUDIOFMDEEMPHASIS_50_MICROS,
        FIR_M_DUAL_FM,
    },
    // AUDIOSTANDARD_SATELLITE_DUAL_FM,
    {
        "Satellite FM-Stereo",
        AUDIO_CARRIER_7_02,  
        AUDIO_CARRIER_7_20,       
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_FM,
        AUDIOFMDEEMPHASIS_50_MICROS,
        AUDIOFMDEEMPHASIS_50_MICROS,
        FIR_SAT_DUAL_FM,
    }
    // A lot of standards snipped
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
    switch (AudioStandard)
    {
    case AUDIOSTANDARD_BG_DUAL_FM:
    case AUDIOSTANDARD_DK1_DUAL_FM:
    case AUDIOSTANDARD_DK2_DUAL_FM:
    case AUDIOSTANDARD_DK3_DUAL_FM:
        return TRUE;
    }
    return FALSE;
}

BOOL CSAA7134Common::IsNICAMAudioStandard(eAudioStandard AudioStandard)
{
    switch (AudioStandard)
    {
    case AUDIOSTANDARD_BG_NICAM_FM:
    case AUDIOSTANDARD_I_NICAM_FM:
    case AUDIOSTANDARD_DK_NICAM_FM:
    case AUDIOSTANDARD_L_NICAM_AM:
        return TRUE;
    }
    return FALSE;
}
