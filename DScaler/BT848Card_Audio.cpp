/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Card_Audio.cpp,v 1.6 2001-11-29 17:30:51 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
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
// Revision 1.5  2001/11/26 13:02:27  adcockj
// Bug Fixes and standards changes
//
// Revision 1.4  2001/11/25 01:58:34  ittarnavsky
// initial checkin of the new I2C code
//
// Revision 1.3  2001/11/23 10:49:16  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.2  2001/11/02 16:30:07  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.1.2.1  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "BT848Card.h"
#include "BT848_Defines.h"

BOOL CBT848Card::HasMSP()
{
    return m_bHasMSP;
}

void CBT848Card::InitMSP()
{
    m_MSP34x0->Attach(m_I2CBus);
    m_MSP34x0->Reset();
    ::Sleep(4);

    // setup version information
    int rev1 = m_MSP34x0->GetVersion();
    int rev2 = m_MSP34x0->GetProductCode();

    if (0 == rev1 && 0 == rev2)
    {
        m_bHasMSP = false;
        return;
    }
    m_bHasMSP = true;

    // set volume to Mute level
    m_MSP34x0->SetMute();

    sprintf(m_MSPVersion, "MSP34%02d%c-%c%d", (rev2 >> 8) & 0xff, (rev1 & 0xff) + '@', ((rev1 >> 8) & 0xff) + '@', rev2 & 0x1f);
}


void CBT848Card::Mute()
{
    if(m_bHasMSP)
    {
        m_MSP34x0->SetMute();
    }
    else
    {
        SetAudioSource(m_CardType, AUDIOMUX_MUTE);
    }
}

void CBT848Card::UnMute(long nVolume)
{
    if(m_bHasMSP)
    {
        // go back from mute to same volume as before
        m_MSP34x0->SetVolume(nVolume);
    }
    else
    {
        SetAudioSource(m_CardType, m_LastAudioSource);
    }
}


void CBT848Card::SetMSPVolume(long nVolume)
{
    if(m_bHasMSP)
    {
        m_MSP34x0->SetVolume(nVolume);
    }
}


void CBT848Card::SetMSPBalance(long nBalance)
{
    if(m_bHasMSP)
    {
        m_MSP34x0->SetBalance(nBalance);
    }
}

void CBT848Card::SetMSPBass(long nBass)
{
    if(m_bHasMSP)
    {
        m_MSP34x0->SetBass(nBass);
    }
}

void CBT848Card::SetMSPTreble(long nTreble)
{
    if(m_bHasMSP)
    {
        m_MSP34x0->SetTreble(nTreble);
    }
}

void CBT848Card::SetMSPSuperBassLoudness(long nLoudness, BOOL bSuperBass)
{
    if(m_bHasMSP)
    {
        m_MSP34x0->SetLoudnessAndSuperBass(nLoudness, bSuperBass ? true : false);
    }
}


void CBT848Card::SetMSPSpatial(long nSpatial)
{
    if(m_bHasMSP)
    {
        m_MSP34x0->SetSpatialEffects(nSpatial);
    }
}

void CBT848Card::SetMSPEqualizer(long EqIndex, long nLevel)
{
    if(m_bHasMSP)
    {
        m_MSP34x0->SetEqualizer(EqIndex, nLevel);
    }
}

void CBT848Card::SetMSPMode(long nMode)
{
    if(m_bHasMSP)
    {
        m_MSP34x0->SetMode(nMode);
    }
}

void CBT848Card::SetMSPStereo(eSoundChannel soundChannel)
{
    if(m_bHasMSP)
    {
        m_MSP34x0->SetStereo(soundChannel);
    }
    else
    {
        switch(m_CardType)
        {
        case TVCARD_GVBCTV3PCI:
        case TVCARD_GVBCTV4PCI:
            SetAudioGVBCTV3PCI(soundChannel);
            break;
        case TVCARD_LIFETEC:
            SetAudioLT9415(soundChannel);
            break;
        case TVCARD_TERRATV:
            SetAudioTERRATV(soundChannel);
            break;
        case TVCARD_AVERMEDIA:
            SetAudioAVER_TVPHONE(soundChannel);
            break;
        case TVCARD_WINFAST2000:
            SetAudioWINFAST2000(soundChannel);
            break;
        default:
            break;
        }
    }
}


void CBT848Card::SetMSPMajorMinorMode(int majorMode, int minorMode)
{
    if(m_bHasMSP)
    {
        m_MSP34x0->SetMajorMinorMode(majorMode, minorMode);
    }
}



void CBT848Card::SetAudioGVBCTV3PCI(eSoundChannel soundChannel)
{
    OrDataDword(BT848_GPIO_DATA, 0x300);
    switch(soundChannel)
    {
    case STEREO:
        AndOrDataDword(BT848_GPIO_DATA, 0x200, ~0x300);
        break;
    case LANGUAGE2:
        AndOrDataDword(BT848_GPIO_DATA, 0x300, ~0x300);
        break;
    default:
    case LANGUAGE1:
        AndOrDataDword(BT848_GPIO_DATA, 0x000, ~0x300);
        break;
    }
}

void CBT848Card::SetAudioLT9415(eSoundChannel soundChannel)
{
    switch(soundChannel)
    {
    case STEREO:
        AndOrDataDword(BT848_GPIO_DATA, 0x0880, ~0x0880);
        break;
    case LANGUAGE2:
        AndOrDataDword(BT848_GPIO_DATA, 0x0080, ~0x0880);
        break;
    default:
    case LANGUAGE1:
        AndOrDataDword(BT848_GPIO_DATA, 0x0000, ~0x0880);
        break;
    }
}

void CBT848Card::SetAudioTERRATV(eSoundChannel soundChannel)
{
    OrDataDword(BT848_GPIO_DATA, 0x180000);
    switch(soundChannel)
    {
    case STEREO:
        AndOrDataDword(BT848_GPIO_DATA, 0x180000, ~0x180000);
        break;
    case LANGUAGE2:
        AndOrDataDword(BT848_GPIO_DATA, 0x080000, ~0x180000);
        break;
    default:
    case LANGUAGE1:
        AndOrDataDword(BT848_GPIO_DATA, 0x000000, ~0x180000);
        break;
    }
}

void CBT848Card::SetAudioAVER_TVPHONE(eSoundChannel soundChannel)
{
    OrDataDword(BT848_GPIO_DATA, 0x180000);
    switch(soundChannel)
    {
    case STEREO:
        AndOrDataDword(BT848_GPIO_DATA, 0x01, ~0x03);
        break;
    case LANGUAGE1:
        AndOrDataDword(BT848_GPIO_DATA, 0x02, ~0x03);
        break;
    default:
        break;
    }
}

void CBT848Card::SetAudioWINFAST2000(eSoundChannel soundChannel)
{
    OrDataDword(BT848_GPIO_DATA, 0x180000);
    switch(soundChannel)
    {
    case STEREO:
        AndOrDataDword(BT848_GPIO_DATA, 0x020000, ~0x430000);
        break;
    case LANGUAGE1:
        AndOrDataDword(BT848_GPIO_DATA, 0x420000, ~0x430000);
        break;
    case LANGUAGE2:
        AndOrDataDword(BT848_GPIO_DATA, 0x410000, ~0x430000);
        break;
    default:
        AndOrDataDword(BT848_GPIO_DATA, 0x420000, ~0x430000);
        break;
    }
}


void CBT848Card::GetMSPPrintMode(LPSTR Text)
{
    if (m_bHasMSP == FALSE)
    {
        strcpy(Text, "No MSP Audio Device");
    }
    else
    {
        m_MSP34x0->GetPrintMode(Text);
    }
}

eSoundChannel CBT848Card::GetMSPWatchMode(eSoundChannel desiredSoundChannel)
{
    if(!m_bHasMSP)
    {
        return desiredSoundChannel;
    }
    return m_MSP34x0->GetWatchMode(desiredSoundChannel); 
}
