/////////////////////////////////////////////////////////////////////////////
// $Id: TVCards.h,v 1.24 2001-11-29 17:30:52 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// The structures where taken from bttv driver version 7.37
// bttv - Bt848 frame grabber driver
//
// Copyright (C) 1996,97,98 Ralph  Metzler (rjkm@thp.uni-koeln.de)
//                         & Marcus Metzler (mocm@thp.uni-koeln.de)
// (c) 1999,2000 Gerd Knorr <kraxel@goldbach.in-berlin.de>
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
// 15 Aug 2000   John Adcock           Added structures from bttv
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __TVCARDS_H___
#define __TVCARDS_H___

#include "settings.h"

// Get Hold of the TVCard.c file settings
SETTING* TVCard_GetSetting(TVCARD_SETTING Setting);
void TVCard_ReadSettingsFromIni();
void TVCard_WriteSettingsToIni(BOOL bOptimizeFileAccess);
void TVCard_SetMenu(HMENU hMenu);

typedef void (AUDIOMODE_FUNC)(int StereoMode);

enum eTVCardId
{
    TVCARD_UNKNOWN = 0,
    TVCARD_MIRO,
    TVCARD_HAUPPAUGE,
    TVCARD_STB,           
    TVCARD_INTEL,         
    TVCARD_DIAMOND,        
    TVCARD_AVERMEDIA,      
    TVCARD_MATRIX_VISION,
    TVCARD_FLYVIDEO,      
    TVCARD_TURBOTV,       
    TVCARD_HAUPPAUGE878,  
    TVCARD_MIROPRO,       
    TVCARD_ADSTECH_TV,    
    TVCARD_AVERMEDIA98,   
    TVCARD_VHX,           
    TVCARD_ZOLTRIX,       
    TVCARD_PIXVIEWPLAYTV, 
    TVCARD_WINVIEW_601,   
    TVCARD_AVEC_INTERCAP, 
    TVCARD_LIFE_FLYKIT,   
    TVCARD_CEI_RAFFLES,   
    TVCARD_CONFERENCETV,  
    TVCARD_PHOEBE_TVMAS,  
    TVCARD_MODTEC_205,    
    TVCARD_MAGICTVIEW061, 
    TVCARD_VOBIS_BOOSTAR, 
    TVCARD_HAUPPAUG_WCAM, 
    TVCARD_MAXI,          
    TVCARD_TERRATV,       
    TVCARD_PXC200,        
    TVCARD_FLYVIDEO_98,   
    TVCARD_IPROTV,        
    TVCARD_INTEL_C_S_PCI, 
    TVCARD_TERRATVALUE,   
    TVCARD_WINFAST2000,   
    TVCARD_CHRONOS_VS2,   
    TVCARD_TYPHOON_TVIEW, 
    TVCARD_PXELVWPLTVPRO, 
    TVCARD_MAGICTVIEW063, 
    TVCARD_PINNACLERAVE,  
    TVCARD_STB2,          
    TVCARD_AVPHONE98,     
    TVCARD_PV951,         
    TVCARD_ONAIR_TV,      
    TVCARD_SIGMA_TVII_FM, 
    TVCARD_MATRIX_VISION2d,
    TVCARD_ZOLTRIX_GENIE, 
    TVCARD_TERRATVRADIO, 
    TVCARD_DYNALINK,
    // MAE 20 Nov 2000 Start of change
    TVCARD_CONEXANTNTSCXEVK,
    TVCARD_ROCKWELLNTSCXEVK,
    // MAE 20 Nov 2000 End of change
    // MAE 5 Dec 2000 Start of change
    TVCARD_CONEXANTFOGHORNREVA,
    TVCARD_CONEXANTFOGHORNREVB,
    TVCARD_CONEXANTFOGHORNREVC,
    // MAE 5 Dec 2000 End of change
    TVCARD_RS_BT,
    TVCARD_CYBERMAIL,
    TVCARD_VIEWCAST,
    TVCARD_ATI_TVWONDER,
    TVCARD_ATI_TVWONDERVE,
    TVCARD_GVBCTV3PCI,
    TVCARD_PROLINK,
    TVCARD_EAGLE,
    TVCARD_PINNACLEPRO,
    TVCARD_THYPHOON,
    TVCARD_LIFETEC,
    TVCARD_BESTBUY_OLD,
    TVCARD_FLYVIDEO_98FM,
    TVCARD_GRANDTEC,
    TVCARD_PHOEBE,
    TVCARD_TVCAPTURER,
    TVCARD_MM100PCTV,
    TVCARD_GMV1,
    TVCARD_BESTBUY_NEW,
    // Sasem 4 channel card has 2 jumper configurations.  s-video (1) or composite.(4)
    /// \todo  Add composite jumpered card settings
    TVCARD_SASEM4CHNLSVID,
    TVCARD_PINNACLESAT,
    TVCARD_VHXOLD,           
    TVCARD_VOODOOTV_200,
    TVCARD_VOODOOTV_FM,
    TVCARD_STANDARD_BT878_NO_INIT_MSP,
    TVCARD_TERRATVALUER,
    TVCARD_FLYVIDEO98EZ,
    TVCARD_AIMSS,
    TVCARD_FLYVIDEO2000,
    TVCARD_GVBCTV4PCI,
    TVCARD_PXELVWPLTVPAK,
    TVCARD_TVIEW_RDS_FM,
    TVCARD_LASTONE,
};

enum eTunerId
{
    TUNER_AUTODETECT = -2,
    TUNER_USER_SETUP = -1,
    TUNER_ABSENT = 0,           
    TUNER_PHILIPS_PAL_I,        
    TUNER_PHILIPS_NTSC,     
    TUNER_PHILIPS_SECAM,        
    TUNER_PHILIPS_PAL,      
    TUNER_TEMIC_4002FH5_PAL,
    TUNER_TEMIC_4032FY5_NTSC,       
    TUNER_TEMIC_4062FY5_PAL_I,      
    TUNER_TEMIC_4036FY5_NTSC,
    TUNER_ALPS_TSBH1_NTSC,  
    TUNER_ALPS_TSBE1_PAL,   
    TUNER_ALPS_TSBB5_PAL_I,     
    TUNER_ALPS_TSBE5_PAL,   
    TUNER_ALPS_TSBC5_PAL,   
    TUNER_TEMIC_4006FH5_PAL,
    TUNER_PHILIPS_1236D_NTSC_INPUT1,
    TUNER_PHILIPS_1236D_NTSC_INPUT2,
    TUNER_ALPS_TSCH6_NTSC,
    TUNER_TEMIC_4016FY5_PAL,
    TUNER_PHILIPS_MK2_NTSC,
    TUNER_TEMIC_4066FY5_PAL_I,
    TUNER_TEMIC_4006FN5_PAL,
    TUNER_TEMIC_4009FR5_PAL,
    TUNER_TEMIC_4039FR5_NTSC,
    TUNER_TEMIC_4046FM5_MULTI,
    TUNER_PHILIPS_PAL_DK,
    TUNER_PHILIPS_MULTI,
    TUNER_LG_I001D_PAL_I,
    TUNER_LG_I701D_PAL_I,
    TUNER_LG_R01F_NTSC,
    TUNER_LG_B01D_PAL,
    TUNER_LG_B11D_PAL,
    TUNER_TEMIC_4009FN5_PAL,
    TUNER_MT2032,
    TUNER_SHARP_2U5JF5540_NTSC,
    TUNER_LASTONE,
};

enum ePLLFreq
{
    PLL_NONE = 0,
    PLL_28,
    PLL_35,
};

typedef struct
{
    char* szName;
    int nVideoInputs;
    int nAudioInputs;
    int TunerInput;
    int SVideoInput;
    DWORD GPIOMask;
    DWORD MuxSelect[10];
    DWORD AudioMuxSelect[6]; // Tuner, Radio, external, internal, mute, stereo 
    DWORD GPIOMuxMask;   // GPIO MUX mask 

    // other settings 
    ePLLFreq pll;
    eTunerId TunerId;
    AUDIOMODE_FUNC* pfnSetAudioMode;
} TCardSetup;

typedef struct
{
    DWORD ID;
    eTVCardId CardId;
    char* szName;
} TAutoDectect878;

eTVCardId Card_AutoDetect();
eTunerId Card_AutoDetectTuner(eTVCardId CardId);
void Card_Init();
const TCardSetup* GetCardSetup();
const TTunerSetup* GetTunerSetup();
void TVCard_FirstTimeSetupHardware();
LPCSTR TVCard_AutoDetectID();
long GetTunersTVFormat();


BOOL APIENTRY SelectCardProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

#endif