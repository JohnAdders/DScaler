//
// $Id$
//
/////////////////////////////////////////////////////////////////////////////
//
// copyright 2003 MIDIMaker midimaker@yandex.ru
//  portions based on MT2032 by itt@myself.com
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

/**
 * @file MT2050.cpp CMT2050 Implementation
 */

#include "stdafx.h"
#include "MT2050.h"
#include "DebugLog.h"


CMT2050::CMT2050(eVideoFormat DefaultVideoFormat) :
    m_Initialized(FALSE),
    m_Frequency (0),
    m_Locked(FALSE)
{
    m_DefaultVideoFormat = DefaultVideoFormat;
}

BYTE CMT2050::GetDefaultAddress() const
{
    return 0xC2>>1;
}

eTunerId CMT2050::GetTunerId()
{
    return TUNER_MT2050;
}

eVideoFormat CMT2050::GetDefaultVideoFormat()
{
    return m_DefaultVideoFormat;
}

BOOL CMT2050::HasRadio() const
{
    return FALSE;
}


BYTE CMT2050::GetRegister(BYTE reg)
{
    BYTE result = 0;
    ReadFromSubAddress(reg, &result, sizeof(result));
    return result;
}

void CMT2050::SetRegister(BYTE reg, BYTE value)
{
    WriteToSubAddress(reg, &value, sizeof(value));
}

WORD CMT2050::GetVersion()
{
    WORD result = 0;
    ReadFromSubAddress(0x13, (BYTE*)&result, sizeof(result));
    return result;
}

WORD CMT2050::GetVendor()
{
    WORD result = 0;
    ReadFromSubAddress(0x11, (BYTE*)&result, sizeof(result));
    return result;
}

void CMT2050::Initialize()
{
    int             SRO, xok = 0;

    if (m_ExternalIFDemodulator)
    {
        m_ExternalIFDemodulator->Init(TRUE, m_DefaultVideoFormat);
    }

    // Get chip info
    BYTE rdbuf[22];
    BYTE wrbuf[] = { (BYTE)(m_DeviceAddress << 1), 0 };

    if (m_I2CBus->Read(wrbuf,2,rdbuf,21))
    {
        LOG(1,_T("MT2050: Companycode=%02x%02x Part=%02x Revision=%02x"),
                    rdbuf[0x11],rdbuf[0x12],rdbuf[0x13],rdbuf[0x14]);
    }

    /* Initialize Registers per spec. */
    SetRegister(1, 0x2F);
    SetRegister(2, 0x25);
    SetRegister(3, 0xC1);
    SetRegister(4, 0x00);
    SetRegister(5, 0x63);
    SetRegister(6, 0x11);
    SetRegister(10, 0x85);
    SetRegister(13, 0x28);
    SetRegister(15, 0x0F);
    SetRegister(16, 0x24);

    SRO = GetRegister(13);
    if ((SRO & 0x40) != 0)
    {
        LOG(1, _T("MT2050: SRO Crystal problem - tuner will not function!"));
    }

    if (m_ExternalIFDemodulator)
    {
        m_ExternalIFDemodulator->Init(FALSE, m_DefaultVideoFormat);
    }

    m_Initialized = TRUE;
}

int CMT2050::SpurCheck(int flos1, int flos2, int fifbw, int fout)
{
    int n1 = 1, n2, f, nmax = 11;
    long Band;

    flos1 = flos1 / 1000;     /* scale to kHz to avoid 32bit overflows */
    flos2 = flos2 / 1000;
    fifbw /= 1000;
    fout /= 1000;

    Band = fout + fifbw / 2;

    do {
        n2 = -n1;
        f = n1 * (flos1 - flos2);
        do {
            n2--;
            f = f - flos2;
            if (abs((abs(f) - fout)) < (fifbw >> 1))
            {
                return 1;
            }
        } while ((f > (flos2 - fout - (fifbw >> 1))) && (n2 > -nmax));
        n1++;
    } while (n1 < nmax);

    return 0;
}

void CMT2050::SetIFFreq(int rfin, int if1, int if2, eVideoFormat videoFormat)
{
    unsigned char  buf[5];

    long flo1, flo2;
//3.1 Calculate LO frequencies
    flo1 = rfin + if1;
    flo1 = flo1 / 1000000;
    flo1 = flo1 * 1000000;
    flo2 = flo1 - rfin - if2;
//3.2 Avoid spurs
    int n = 0;
    long flos1, flos2, fifbw, fif1_bw;
    long ftest;
    char SpurInBand;
    flos1 = flo1;
    flos2 = flo2;
    fif1_bw = 16000000;
    if (IsNTSCVideoFormat(videoFormat))
    {
        fifbw = 6750000;
    }
    else
    {   /* PAL */
        fifbw = 8750000;
    }

    do {
        if ((n & 1) == 0)
        {
            flos1 = flos1 - 1000000 * n;
            flos2 = flos2 - 1000000 * n;
        }
        else
        {
            flos1 = flos1 + 1000000 * n;
            flos2 = flos2 + 1000000 * n;
        }
//check we are still in bandwidth
        ftest = abs(flos1 - rfin - if1 + (fifbw >> 1));
        if (ftest > (fif1_bw >> 1))
        {
            flos1 = flo1;
            flos2 = flo2;
            LOG(1, _T("No spur"));
            break;
        }
        n++;
        SpurInBand = SpurCheck(flos1, flos2, fifbw, if2);
    } while(SpurInBand != 0);

    flo1 = flos1;
    flo2 = flos2;
//3.3 Calculate LO registers
    long LO1I, LO2I, flo1step, flo2step;
    long flo1rem, flo2rem, flo1tune, flo2tune;
    int num1, num2, Denom1, Denom2;
    int div1a, div1b, div2a, div2b;

    flo1step = 1000000;
    flo2step = 50000;
    LO1I = (long)floor(flo1 / 4000000.0);
    LO2I = (long)floor(flo2 / 4000000.0);
    flo1rem = flo1 % 4000000;
    flo2rem = flo2 % 4000000;
    flo1tune = flo1step * (long)floor((flo1rem + flo1step / 2.0) / flo1step);
    flo2tune = flo2step * (long)floor((flo2rem + flo2step / 2.0) / flo2step);
    Denom1 = 4;
    Denom2 = 4095;
    num1 = (int)floor(flo1tune / (4000000.0 / Denom1) + 0.5);
    num2 = (int)floor(flo2tune / (4000000.0 / Denom2) + 0.5);
    if (num1 >= Denom1)
    {
        num1 = 0;
        LO1I++;
    }
    if (num2 >= Denom2)
    {
        num2 = 0;
        LO2I++;
    }
    div1a = (int)floor((double)(LO1I / 12.0)) - 1;
    div1b = LO1I % 12;
    div2a = (int)floor(double(LO2I / 8.0)) - 1;
    div2b = LO2I % 8;
//3.4 Writing registers
    if (rfin < 277000000)
    {
        buf[0] = 128 + 4 * div1b + num1;
    }
    else
    {
        buf [0] = 4 * div1b + num1;
    }
    buf [1] = div1a;
    buf [2] = 32 * div2b + (UCHAR)floor(num2 / 256.0);
    buf [3] = num2 % 256;
    if (num2 == 0)
    {
        buf [4] = div2a;
    }
    else
    {
        buf [4] = 64 + div2a;
    }

    if (m_ExternalIFDemodulator)
    {
        m_ExternalIFDemodulator->TunerSet(TRUE, videoFormat);
    }

    SetRegister(1, buf[0x00]);
    SetRegister(2, buf[0x01]);
    SetRegister(3, buf[0x02]);
    SetRegister(4, buf[0x03]);
    SetRegister(5, buf[0x04]);

//3.5 Allow LO to lock
    m_Locked = FALSE;
    Sleep(50);
    int nlock = 0, Status;
    do {
        Status = GetRegister(7);
        Status &= 0x88;
        if (Status == 0x88)
        {
            m_Locked = TRUE;
            break;
        }
        Sleep(2);
        nlock++;
    } while(nlock < 100);

    if (m_ExternalIFDemodulator)
    {
        m_ExternalIFDemodulator->TunerSet(FALSE, videoFormat);
    }
}

BOOL CMT2050::SetTVFrequency(long frequency, eVideoFormat videoFormat)
{
    if (!m_Initialized)
    {
        Initialize();
    }
    int if2;

    /* signal bandwidth and picture carrier */
    if (IsNTSCVideoFormat(videoFormat))
    {
        if2 = 45750 * 1000;
    }
    else
    {   /* PAL */
        if2 = 38900 * 1000;
    }

    m_Frequency = frequency;

    SetIFFreq(frequency, 1220 * 1000 * 1000, if2, videoFormat);
    return TRUE;
}

BOOL CMT2050::SetRadioFrequency(long nFrequency)
{
    return FALSE;
}

long CMT2050::GetFrequency()
{
    return m_Frequency;
}

eTunerLocked CMT2050::IsLocked()
{
    return TUNER_LOCK_ON;
}

eTunerAFCStatus CMT2050::GetAFCStatus(long &nFreqDeviation)
{
    eTunerAFCStatus AFCStatus = TUNER_AFC_NOTSUPPORTED;
    if (m_ExternalIFDemodulator)
    {
        AFCStatus = m_ExternalIFDemodulator->GetAFCStatus(nFreqDeviation);
    }

    return AFCStatus;
}
