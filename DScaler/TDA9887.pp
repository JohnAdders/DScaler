#include "stdafx.h"

#include "TVFormats.h"
#include "I2CDevice.h"

class CTDA9887 : public CI2CDevice
{

};

void CTDA9887::PrepareTDA9887(bool bPrepare, eVideoFormat VideoFormat)
{
   static BYTE tda9887set_pal_bg[] =   {I2C_TDA9887_0, 0x00, 0x96, 0x70, 0x49};
   static BYTE tda9887set_pal_i[] =    {I2C_TDA9887_0, 0x00, 0x96, 0x70, 0x4a};
   static BYTE tda9887set_pal_dk[] =   {I2C_TDA9887_0, 0x00, 0x96, 0x70, 0x4b};
   static BYTE tda9887set_pal_l[] =    {I2C_TDA9887_0, 0x00, 0x86, 0x50, 0x4b};
   static BYTE tda9887set_ntsc[] =     {I2C_TDA9887_0, 0x00, 0x96, 0x70, 0x44};
   static BYTE tda9887set_ntsc_jp[] =  {I2C_TDA9887_0, 0x00, 0x96, 0x70, 0x40};
   static BYTE tda9887set_fm_radio[] = {I2C_TDA9887_0, 0x00, 0x8e, 0x0d, 0x77};

   if (
   
   BYTE *tda9887set = NULL;

   switch (VideoFormat)
   {
	case VIDEOFORMAT_PAL_B:    
    case VIDEOFORMAT_PAL_G:
    case VIDEOFORMAT_PAL_H:        
    case VIDEOFORMAT_PAL_N:
	case VIDEOFORMAT_SECAM_B:
    case VIDEOFORMAT_SECAM_G:
    case VIDEOFORMAT_SECAM_H:
		tda9887set = tda9887set_pal_bg;
		break;

	case VIDEOFORMAT_PAL_I:
		tda9887set = tda9887set_pal_i;
		break;

	case VIDEOFORMAT_PAL_D:
	case VIDEOFORMAT_SECAM_D:	
    case VIDEOFORMAT_SECAM_K:
    case VIDEOFORMAT_SECAM_K1:
		tda9887set = tda9887set_pal_dk;
		break;
	
	case VIDEOFORMAT_SECAM_L:
    case VIDEOFORMAT_SECAM_L1:
		tda9887set = tda9887set_pal_l;
		break;

    case VIDEOFORMAT_PAL_60:    
		///\todo Video bandwidth of PAL-60?
		break;

	case VIDEOFORMAT_PAL_M:
	case VIDEOFORMAT_PAL_N_COMBO:
	case VIDEOFORMAT_NTSC_M:        
		tda9887set = tda9887set_ntsc;
		break;

    case VIDEOFORMAT_NTSC_50:
    case VIDEOFORMAT_NTSC_M_Japan:
        tda9887set = tda9887set_ntsc_jp;
		break;

	case (VIDEOFORMAT_LASTONE+1):
		//radio
		tda9887set = tda9887set_fm_radio;
		break;
   }
      
   if (bPrepare)
   {
      if (tda9887set != NULL)
      {   
          LOG(2,"TDA9885/6/7: 0x%02x 0x%02x 0x%02x", tda9887set[2],tda9887set[3],tda9887set[4]);
          m_I2CBus->Write(tda9887set, 5);   
      }
   }
}
