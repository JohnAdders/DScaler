/////////////////////////////////////////////////////////////////////////////
// TVCards.c
/////////////////////////////////////////////////////////////////////////////
// The structures where taken from bttv driver version 7.37
// bttv - Bt848 frame grabber driver
//
// Copyright (C) 1996,97,98 Ralph  Metzler (rjkm@thp.uni-koeln.de)
//                         & Marcus Metzler (mocm@thp.uni-koeln.de)
// (c) 1999,2000 Gerd Knorr <kraxel@goldbach.in-berlin.de>
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
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
void TVCard_WriteSettingsToIni();
void TVCard_SetMenu(HMENU hMenu);

typedef enum
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
	TVCARD_LASTONE,
} TVCARDID;

typedef enum
{
	TUNER_AUTODETECT = -2,
	TUNER_USER_SETUP = -1,
	TUNER_ABSENT = 0,			
	TUNER_PHILIPS_PAL_I,		
	TUNER_PHILIPS_NTSC,		
	TUNER_PHILIPS_SECAM,		
	TUNER_PHILIPS_PAL,		
	TUNER_TEMIC_PAL,
	TUNER_TEMIC_NTSC,		
	TUNER_TEMIC_PAL_I,		
	TUNER_TEMIC_4036FY5_NTSC,
	TUNER_ALPS_TSBH1_NTSC, 	
	TUNER_ALPS_TSBE1_PAL, 	
	TUNER_ALPS_TSBB5_PAL_I, 	
	TUNER_ALPS_TSBE5_PAL, 	
	TUNER_ALPS_TSBC5_PAL, 	
	TUNER_TEMIC_4006FH5_PAL,
	// MAE 5 Dec 2000 Start of change
	TUNER_PHILIPS_1236D_NTSC_INPUT1,
	TUNER_PHILIPS_1236D_NTSC_INPUT2,
	// MAE 5 Dec 2000 End of change
	TUNER_LASTONE,
} TVTUNERID;

typedef enum
{
	PLL_NONE = 0,
	PLL_28,
	PLL_35,
} PLLFREQ;

typedef enum
{
	NoTuner = 0,
	Philips,
	TEMIC,
	Sony,
	Alps,
} TUNERMFTR;

typedef enum
{
	NOTUNER = 0,
	PAL,
	PAL_I,
	NTSC,
	SECAM,
} TUNERTYPE;

typedef struct
{
	char *szName;
	int nVideoInputs;
	int nAudioInputs;
	int TunerInput;
	int SVideoInput;
	DWORD GPIOMask;
	DWORD MuxSelect[10];
	DWORD AudioMuxSelect[6]; /* Tuner, Radio, external, internal, mute, stereo */
	DWORD GPIOMuxMask;   /* GPIO MUX mask */

	/* look for these i2c audio chips */
	int msp34xx:1;
	int tda8425:1;
	int tda9840:1;
	int tda985x:1;
	int tea63xx:1;
	int tea64xx:1;
	int tda7432:1;
	int tda9875:1;

	/* other settings */
	PLLFREQ pll;
	TVTUNERID TunerId;
} TVCARDSETUP;

typedef struct
{
	DWORD ID;
	TVCARDID CardId;
	char *szName;
} TAutoDectect878;

typedef struct
{
	char* szName;
	TUNERMFTR Vendor;
	TUNERTYPE Type;
	WORD thresh1; /* frequency Range for UHF,VHF-L, VHF_H */   
	WORD thresh2;  
	BYTE VHF_L;
	BYTE VHF_H;
	BYTE UHF;
	BYTE config; 
	BYTE I2C;
	WORD IFPCoff;
	BYTE mode; /* mode change value (tested PHILIPS_SECAM only) */
			/* 0x01 -> ??? no change ??? */
			/* 0x02 -> PAL BDGHI / SECAM L */
			/* 0x04 -> ??? PAL others / SECAM others ??? */
} TVTUNERSETUP;

TVCARDID Card_AutoDetect();
int Card_AutoDetectTuner(TVCARDID CardId);
void Card_Init();
const TVCARDSETUP* GetCardSetup();
const TVTUNERSETUP* GetTunerSetup();
void TVCard_FirstTimeSetupHardware(HINSTANCE hInst, HWND hWnd);
LPCSTR TVCard_AutoDetectID();

BOOL APIENTRY SelectCardProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

#endif