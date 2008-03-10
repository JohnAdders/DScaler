/////////////////////////////////////////////////////////////////////////////
// $Id: RegLog.cpp,v 1.9 2008-03-10 17:41:47 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 John Adcock.  All rights reserved.
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.8  2002/12/05 17:11:11  adcockj
// Sound fixes
//
// Revision 1.7  2002/12/04 14:15:06  adcockj
// Fixed RegSpy Problems
//
// Revision 1.6  2002/11/29 17:19:09  adcockj
// extra logging
//
// Revision 1.5  2002/11/28 14:56:19  adcockj
// Fixed some register size issues
//
// Revision 1.4  2002/11/28 14:50:52  adcockj
// Added set directory so that files aways go to the exe location
//
// Revision 1.3  2002/11/28 14:47:21  adcockj
// Added bt848 logging
//
// Revision 1.2  2002/11/27 17:39:30  adcockj
// Added headers
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "GenericCard.h"
#include "../DScaler/CX2388X_Defines.h"
#include "../DScaler/BT848_Defines.h"

void SetDirectoryToExe()
{
    char szDriverPath[MAX_PATH];
    char* pszName;

    if (!GetModuleFileName(NULL, szDriverPath, sizeof(szDriverPath)))
    {
        ErrorBox("Cannot get module file name");
        return;
    }

    pszName = szDriverPath + strlen(szDriverPath);
    while (pszName >= szDriverPath && *pszName != '\\')
    {
        *pszName-- = 0;
    }

    SetCurrentDirectory(szDriverPath);
}


#define DumpBRegister(Reg) fprintf(hFile, #Reg "\t%02x\n", pCard->ReadByte(Reg))
#define DumpWRegister(Reg) fprintf(hFile, #Reg "\t%04x\n", pCard->ReadWord(Reg))
#define DumpDWRegister(Reg) fprintf(hFile, #Reg "\t%08x\n", pCard->ReadDword(Reg))

void __cdecl BT848RegLog(CGenericCard* pCard, LPCSTR Filename)
{
    FILE* hFile;

    hFile = fopen(Filename, "w");
    if(!hFile)
    {
        return;
    }

    fprintf(hFile, "SubSystemId\t%08x\n", pCard->GetSubSystemId());
    DumpBRegister(BT848_IFORM);
    DumpBRegister(BT848_FCNTR);
    DumpBRegister(BT848_PLL_F_LO);
    DumpBRegister(BT848_PLL_F_HI);
    DumpBRegister(BT848_PLL_XCI);
    DumpBRegister(BT848_TGCTRL);
    DumpBRegister(BT848_TDEC);
    DumpBRegister(BT848_E_CROP);
    DumpBRegister(BT848_O_CROP);
    DumpBRegister(BT848_E_VDELAY_LO);
    DumpBRegister(BT848_O_VDELAY_LO);
    DumpBRegister(BT848_E_VACTIVE_LO);
    DumpBRegister(BT848_O_VACTIVE_LO);
    DumpBRegister(BT848_E_HDELAY_LO);
    DumpBRegister(BT848_O_HDELAY_LO);
    DumpBRegister(BT848_E_HACTIVE_LO);
    DumpBRegister(BT848_O_HACTIVE_LO);
    DumpBRegister(BT848_E_HSCALE_HI);
    DumpBRegister(BT848_O_HSCALE_HI);
    DumpBRegister(BT848_E_HSCALE_LO);
    DumpBRegister(BT848_O_HSCALE_LO);
    DumpBRegister(BT848_BRIGHT);
    DumpBRegister(BT848_E_CONTROL);
    DumpBRegister(BT848_O_CONTROL);
    DumpBRegister(BT848_CONTRAST_LO);
    DumpBRegister(BT848_SAT_U_LO);
    DumpBRegister(BT848_SAT_V_LO);
    DumpBRegister(BT848_HUE);
    DumpBRegister(BT848_E_SCLOOP);
    DumpBRegister(BT848_O_SCLOOP);
    DumpBRegister(BT848_WC_UP);
    DumpBRegister(BT848_WC_DOWN);
    DumpBRegister(BT848_VTOTAL_LO);
    DumpBRegister(BT848_VTOTAL_HI);
    DumpBRegister(BT848_DVSIF);
    DumpBRegister(BT848_OFORM);
    DumpBRegister(BT848_E_VSCALE_HI);
    DumpBRegister(BT848_O_VSCALE_HI);
    DumpBRegister(BT848_E_VSCALE_LO);
    DumpBRegister(BT848_O_VSCALE_LO);
    DumpBRegister(BT848_ADC);
    DumpBRegister(BT848_E_VTC);
    DumpBRegister(BT848_O_VTC);
    DumpBRegister(BT848_COLOR_FMT);
    DumpBRegister(BT848_COLOR_CTL);
    DumpBRegister(BT848_CAP_CTL);
    DumpBRegister(BT848_VBI_PACK_SIZE);
    DumpBRegister(BT848_VBI_PACK_DEL);
    DumpDWRegister(BT848_INT_MASK);
    DumpDWRegister(BT848_GPIO_OUT_EN);
    DumpDWRegister(BT848_GPIO_OUT_EN_HIBYTE);
    DumpDWRegister(BT848_GPIO_DATA);
    DumpDWRegister(BT848_RISC_STRT_ADD);
    DumpWRegister(BT848_GPIO_DMA_CTL);

    fclose(hFile);
}

void __cdecl CX2388xRegLog(CGenericCard* pCard, LPCSTR Filename)
{
    FILE* hFile;

    hFile = fopen(Filename, "w");
    if(!hFile)
    {
        return;
    }

    fprintf(hFile, "SubSystemId\t%08x\n", pCard->GetSubSystemId());

    DumpDWRegister(CX2388X_DEVICE_STATUS);
    DumpDWRegister(CX2388X_VIDEO_INPUT);
    DumpDWRegister(CX2388X_TEMPORAL_DEC);
    DumpDWRegister(CX2388X_AGC_BURST_DELAY);
    DumpDWRegister(CX2388X_BRIGHT_CONTRAST); 
    DumpDWRegister(CX2388X_UVSATURATION);    
    DumpDWRegister(CX2388X_HUE);             
    DumpDWRegister(CX2388X_WHITE_CRUSH);
    DumpDWRegister(CX2388X_PIXEL_CNT_NOTCH);
    DumpDWRegister(CX2388X_HORZ_DELAY_EVEN);
    DumpDWRegister(CX2388X_HORZ_DELAY_ODD);
    DumpDWRegister(CX2388X_VERT_DELAY_EVEN);
    DumpDWRegister(CX2388X_VERT_DELAY_ODD);
    DumpDWRegister(CX2388X_VDELAYCCIR_EVEN);
    DumpDWRegister(CX2388X_VDELAYCCIR_ODD);
    DumpDWRegister(CX2388X_HACTIVE_EVEN);
    DumpDWRegister(CX2388X_HACTIVE_ODD);
    DumpDWRegister(CX2388X_VACTIVE_EVEN);    
    DumpDWRegister(CX2388X_VACTIVE_ODD);     
    DumpDWRegister(CX2388X_HSCALE_EVEN);     
    DumpDWRegister(CX2388X_HSCALE_ODD);      
    DumpDWRegister(CX2388X_VSCALE_EVEN);     
    DumpDWRegister(CX2388X_VSCALE_ODD);      
    DumpDWRegister(CX2388X_FILTER_EVEN);     
    DumpDWRegister(CX2388X_FILTER_ODD);      
    DumpDWRegister(CX2388X_FORMAT_2HCOMB);
    DumpDWRegister(CX2388X_PLL);
    DumpDWRegister(CX2388X_PLL_ADJUST);
    DumpDWRegister(CX2388X_SAMPLERATECONV);  
    DumpDWRegister(CX2388X_SAMPLERATEFIFO);  
    DumpDWRegister(CX2388X_SUBCARRIERSTEP);  
    DumpDWRegister(CX2388X_SUBCARRIERSTEPDR);
    DumpDWRegister(CX2388X_CAPTURECONTROL);  
    DumpDWRegister(CX2388X_VIDEO_COLOR_FORMAT);
    DumpDWRegister(CX2388X_VBI_SIZE);
    DumpDWRegister(CX2388X_FIELD_CAP_CNT);
    DumpDWRegister(CX2388X_VIP_CONFIG);
    DumpDWRegister(CX2388X_VIP_CONTBRGT);
    DumpDWRegister(CX2388X_VIP_HSCALE);
    DumpDWRegister(CX2388X_VIP_VSCALE);
    DumpDWRegister(CX2388X_VBOS);

    DumpDWRegister(MO_GP0_IO);
    DumpDWRegister(MO_GP1_IO);   
    DumpDWRegister(MO_GP2_IO);
    DumpDWRegister(MO_GP3_IO);
    DumpDWRegister(MO_GPIO);
    DumpDWRegister(MO_GPOE);

    DumpDWRegister(0x0320d01);
    DumpDWRegister(0x0320d02);
    DumpDWRegister(0x0320d03);
    DumpDWRegister(0x0320d04);
    DumpDWRegister(0x0320d2a);
    DumpDWRegister(0x0320d2b);

    DumpDWRegister(AUD_DN0_FREQ);
    DumpDWRegister(AUD_POLY0_DDS_CONSTANT);
    DumpDWRegister(AUD_IIR1_0_SEL);
    DumpDWRegister(AUD_IIR1_1_SEL);
    DumpDWRegister(AUD_IIR1_2_SEL);
    DumpDWRegister(AUD_IIR1_3_SEL);
    DumpDWRegister(AUD_IIR1_4_SEL);
    DumpDWRegister(AUD_IIR1_5_SEL);
    DumpDWRegister(AUD_IIR1_0_SHIFT);
    DumpDWRegister(AUD_IIR1_1_SHIFT);
    DumpDWRegister(AUD_IIR1_2_SHIFT);
    DumpDWRegister(AUD_IIR1_3_SHIFT);
    DumpDWRegister(AUD_IIR1_4_SHIFT);
    DumpDWRegister(AUD_IIR1_5_SHIFT);
    DumpDWRegister(AUD_IIR2_0_SEL);
    DumpDWRegister(AUD_IIR2_1_SEL);
    DumpDWRegister(AUD_IIR2_2_SEL);
    DumpDWRegister(AUD_IIR2_3_SEL);
    DumpDWRegister(AUD_IIR3_0_SEL);
    DumpDWRegister(AUD_IIR3_1_SEL);
    DumpDWRegister(AUD_IIR3_2_SEL);
    DumpDWRegister(AUD_IIR3_0_SHIFT);
    DumpDWRegister(AUD_IIR3_1_SHIFT);
    DumpDWRegister(AUD_IIR3_2_SHIFT);
    DumpDWRegister(AUD_IIR4_0_SEL);
    DumpDWRegister(AUD_IIR4_1_SEL);
    DumpDWRegister(AUD_IIR4_2_SEL);
    DumpDWRegister(AUD_IIR4_0_SHIFT);
    DumpDWRegister(AUD_IIR4_1_SHIFT);
    DumpDWRegister(AUD_IIR4_2_SHIFT);
    DumpDWRegister(AUD_IIR4_0_CA0);
    DumpDWRegister(AUD_IIR4_0_CA1);
    DumpDWRegister(AUD_IIR4_0_CA2);
    DumpDWRegister(AUD_IIR4_0_CB0);
    DumpDWRegister(AUD_IIR4_0_CB1);
    DumpDWRegister(AUD_IIR4_1_CA0);
    DumpDWRegister(AUD_IIR4_1_CA1);
    DumpDWRegister(AUD_IIR4_1_CA2);
    DumpDWRegister(AUD_IIR4_1_CB0);
    DumpDWRegister(AUD_IIR4_1_CB1);
    DumpDWRegister(AUD_IIR4_2_CA0);
    DumpDWRegister(AUD_IIR4_2_CA1);
    DumpDWRegister(AUD_IIR4_2_CA2);
    DumpDWRegister(AUD_IIR4_2_CB0);
    DumpDWRegister(AUD_IIR4_2_CB1);
    DumpDWRegister(AUD_HP_MD_IIR4_1);
    DumpDWRegister(AUD_HP_PROG_IIR4_1);
    DumpDWRegister(AUD_DN1_FREQ);
    DumpDWRegister(AUD_DN1_SRC_SEL);
    DumpDWRegister(AUD_DN1_SHFT);
    DumpDWRegister(AUD_DN1_AFC);
    DumpDWRegister(AUD_DN1_FREQ_SHIFT);
    DumpDWRegister(AUD_DN2_SRC_SEL);
    DumpDWRegister(AUD_DN2_SHFT);
    DumpDWRegister(AUD_DN2_AFC);
    DumpDWRegister(AUD_DN2_FREQ);
    DumpDWRegister(AUD_DN2_FREQ_SHIFT);
    DumpDWRegister(AUD_PDET_SRC);
    DumpDWRegister(AUD_PDET_SHIFT);
    DumpDWRegister(AUD_DEEMPH0_SRC_SEL);
    DumpDWRegister(AUD_DEEMPH1_SRC_SEL);
    DumpDWRegister(AUD_DEEMPH0_SHIFT);
    DumpDWRegister(AUD_DEEMPH1_SHIFT);
    DumpDWRegister(AUD_DEEMPH0_G0);
    DumpDWRegister(AUD_DEEMPH0_A0);
    DumpDWRegister(AUD_DEEMPH0_B0);
    DumpDWRegister(AUD_DEEMPH0_A1);
    DumpDWRegister(AUD_DEEMPH0_B1);
    DumpDWRegister(AUD_DEEMPH1_G0);
    DumpDWRegister(AUD_DEEMPH1_A0);
    DumpDWRegister(AUD_DEEMPH1_B0);
    DumpDWRegister(AUD_DEEMPH1_A1);
    DumpDWRegister(AUD_DEEMPH1_B1);
    DumpDWRegister(AUD_PLL_EN);
    DumpDWRegister(AUD_DMD_RA_DDS);
    DumpDWRegister(AUD_RATE_ADJ1);
    DumpDWRegister(AUD_RATE_ADJ2);
    DumpDWRegister(AUD_RATE_ADJ3);
    DumpDWRegister(AUD_RATE_ADJ4);
    DumpDWRegister(AUD_RATE_ADJ5);
    DumpDWRegister(AUD_C2_UP_THR);
    DumpDWRegister(AUD_C2_LO_THR);
    DumpDWRegister(AUD_C1_UP_THR);
    DumpDWRegister(AUD_C1_LO_THR);
    DumpDWRegister(AUD_CTL);
    DumpDWRegister(AUD_DCOC_0_SRC);
    DumpDWRegister(AUD_DCOC_1_SRC);
    DumpDWRegister(AUD_DCOC1_SHIFT);
    DumpDWRegister(AUD_DCOC_1_SHIFT_IN0);
    DumpDWRegister(AUD_DCOC_1_SHIFT_IN1);
    DumpDWRegister(AUD_DCOC_PASS_IN);
    DumpDWRegister(AUD_IIR4_0_SEL);
    DumpDWRegister(AUD_DN1_AFC);
    DumpDWRegister(AUD_DCOC_2_SRC);
    DumpDWRegister(AUD_IIR4_1_SEL);
    DumpDWRegister(AUD_CTL);
    DumpDWRegister(AUD_DN2_SRC_SEL);
    DumpDWRegister(AUD_DN2_FREQ);
    DumpDWRegister(AUD_POLY0_DDS_CONSTANT);
    DumpDWRegister(AUD_PHASE_FIX_CTL);
    DumpDWRegister(AUD_CORDIC_SHIFT_1);
    DumpDWRegister(AUD_PLL_PRESCALE);
    DumpDWRegister(AUD_PLL_INT);
    DumpDWRegister(AUD_AFE_12DB_EN);

    fclose(hFile);
}


typedef void (__cdecl REGLOGFUNC)(CGenericCard* pCard, LPCSTR Filename);

typedef struct
{
    WORD VendorId;
    WORD DeviceId;
    char* FileNameSpec;
    REGLOGFUNC* DumpFunction;
} TChip;

TChip Chips[] = 
{
    {
        0x109e,
        0x0350,
        "BT848%d.txt",
        BT848RegLog,
    },
    {
        0x109e,
        0x0351,
        "BT849%d.txt",
        BT848RegLog,
    },
    {
        0x109e,
        0x036e,
        "BT878%d.txt",
        BT848RegLog,
    },
    {
        0x109e,
        0x036f,
        "BT878A%d.txt",
        BT848RegLog,
    },
    {
        0x14F1, 
        0x8800, 
        "CX2388x%d.txt",
        CX2388xRegLog,
    },
};


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    // make sure the files go to whereever the exe is
    SetDirectoryToExe();

    CHardwareDriver* pHardwareDriver = new CHardwareDriver();
    if(pHardwareDriver->LoadDriver() == TRUE)
    {
        for(int i(0); i < sizeof(Chips)/sizeof(TChip); ++i)
        {
            int CardsFound(0);
            DWORD SubSystemId;

            while(pHardwareDriver->DoesThisPCICardExist( 
                                                            Chips[i].VendorId, 
                                                            Chips[i].DeviceId, 
                                                            CardsFound, 
                                                            SubSystemId
                                                       ) == TRUE)
            {
                CGenericCard* pNewCard = new CGenericCard(pHardwareDriver);
                if(pNewCard->OpenPCICard(Chips[i].VendorId, Chips[i].DeviceId, CardsFound))
                {
                    char Filename[MAX_PATH];
                    sprintf(Filename, Chips[i].FileNameSpec, CardsFound);
                    (Chips[i].DumpFunction)(pNewCard, Filename);
                }
                ++CardsFound;
                delete pNewCard;
            }
        }
    }
    else
    {
        MessageBox(NULL, "Can't open driver", "RegLog", MB_OK);
    }
    delete pHardwareDriver;
	return 0;
}

void __cdecl LOG(int , LPCSTR, ...)
{
}

HWND hWnd = NULL;

void __cdecl HideSplashScreen()
{
}

extern "C"
{
    long gBuildNum = 0;
}

void __cdecl OSD_Redraw(struct HWND__ *,struct HDC__ *)
{
}

void __cdecl OSD_ShowTextPersistent(struct HWND__ *,char const *,double)
{
}

void __cdecl OSD_ProcessDisplayUpdate(struct HDC__ *,struct tagRECT *)
{
}

int __cdecl GetDisplayAreaRect(struct HWND__ *,struct tagRECT *, BOOL)
{
    return FALSE;
}

void __cdecl OSD_ShowTextPersistent(char const *,double)
{
}
