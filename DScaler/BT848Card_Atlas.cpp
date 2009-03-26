/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2006 Curtiss-Wright Controls, Inc.  All rights reserved.
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
//
// This code is based on BT848Card_PMS.cpp, which in turn, is based
// on a version of dTV modified by Michael Eskin and others at Connexant.
// Those parts are probably (c) Connexant 2002
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file BT848Card_Atlas.cpp CBT848Card Implementation (Atlas)
 */

#include "stdafx.h"

#ifdef WANT_BT8X8_SUPPORT

#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "BT848Card.h"
#include "BT848_Defines.h"
#include "DebugLog.h"
#include "Providers.h"
#include "IOutput.h"

#include "status.h"

#include "Source.h"

#include "DS_Filter.h"

// enum sequence aligned to the UI entry sequence!
enum eAtlasInputs
{
    ATLAS_SVIDEO,

    ATLAS_COMPOSITE1,
    ATLAS_COMPOSITE2,
    ATLAS_COMPOSITE3,
    ATLAS_COMPOSITE4,

    ATLAS_RGB_DVI,
    ATLAS_HIRESCOMP,
};


void CBT848Card::InitAtlas()
{
    m_AD9882 = new CAD9882();

    m_AD9882->SetI2CBus(m_I2CBus);

    // start with AD9882 disabled
    m_AD9882->Suspend();
}

void CBT848Card::AtlasInputSelect(int nInput)
{
    m_CurrentInput = nInput;

    if (IsSPISource(nInput))
    {
        // AD9882 will auto-select input source between RGB and DVI
        m_AD9882->Wakeup();
    }
    else
    {
        m_AD9882->Suspend();
        StandardBT848InputSelect(nInput);
    }
    #ifdef _DEBUG
    LOG(1, "AtlasInputSelect()");
    #endif
}


void CBT848Card::SetAtlasFormat(int nInput, eVideoFormat TVFormat)
{
    BOOL bOverlay = GetActiveOutput()->OverlayActive();

    if (!IsSPISource(nInput))
    {
        StandardSetFormat(nInput, TVFormat);
        // change overlay format to YUV, if needed
        if (bOverlay)
        {
            if (GetActiveOutput()->Overlay_GetRGB())
            {
                GetActiveOutput()->Overlay_Destroy();
                GetActiveOutput()->Overlay_SetRGB(FALSE);
                GetActiveOutput()->Overlay_Create();
            }
        }
        else
        {
            GetActiveOutput()->Overlay_SetRGB(FALSE);
        }

        return;
    }

    //SetupAD9882();

    #ifdef _DEBUG
    if(m_AD9882->GetVersion() > 0)
    {
        m_AD9882->DumpSettings("AD9882_Atlas.txt");
    }
    #endif
    // configure Bt878 to pass through the data
    WriteByte(BT848_COLOR_FMT, BT848_COLOR_FMT_YUY2);
    // SPI input mode
    MaskDataWord(BT848_GPIO_DMA_CTL, 2 << 11,
        (BT848_GPIO_DMA_CTL_GPIOMODE | BT848_GPIO_DMA_CTL_GPCLKMODE));
    // byte swap + no gamma correction removal
    MaskDataByte(BT848_COLOR_CTL, (BT848_COLOR_CTL_GAMMA |
            BT848_COLOR_CTL_BSWAP_ODD | BT848_COLOR_CTL_BSWAP_EVEN), 0x1f);
    // clear interrupt status bits and field counter
    WriteDword(BT848_INT_STAT, ReadDword(BT848_INT_STAT));
    WriteByte(BT848_FCNTR, 0);

    // change overlay format to RGB16, if needed
    if (bOverlay)
    {
        if (!GetActiveOutput()->Overlay_GetRGB())
        {
            GetActiveOutput()->Overlay_Destroy();
            GetActiveOutput()->Overlay_SetRGB(TRUE);
            GetActiveOutput()->Overlay_Create();
        }
    }
    else
    {
        GetActiveOutput()->Overlay_SetRGB(TRUE);
    }
}

void CBT848Card::SetAtlasContrastBrightness(WORD Contrast, WORD Brightness)
{
    if (!IsSPISource(m_CurrentInput))
    {
        SetAnalogContrastBrightness(Contrast, Brightness);
    }
}

void CBT848Card::SetAtlasHue(BYTE Hue)
{
    if (!IsSPISource(m_CurrentInput))
    {
        SetAnalogHue(Hue);
    }
}

void CBT848Card::SetAtlasSaturationU(WORD Saturation)
{
    if (!IsSPISource(m_CurrentInput))
    {
        SetAnalogSaturationU(Saturation);
    }
}

void CBT848Card::SetAtlasSaturationV(WORD Saturation)
{
    if (!IsSPISource(m_CurrentInput))
    {
        SetAnalogSaturationV(Saturation);
    }
}

#endif // WANT_BT8X8_SUPPORT