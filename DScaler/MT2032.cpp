//
// $Id: MT2032.cpp,v 1.10 2002-10-16 21:42:36 kooiman Exp $
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
// Revision 1.9  2002/10/11 13:38:14  kooiman
// Added support for VoodooTV IF demodulator. Improved TDA9887. Added interface for GPOE/GPDATA access to make this happen.
//
// Revision 1.8  2002/10/08 20:43:16  kooiman
// Added Automatic Frequency Control for tuners. Changed to Hz instead of multiple of 62500 Hz.
//
// Revision 1.7  2002/10/07 20:32:00  kooiman
// Added/fixed TDA9887 support for new Pinnacle cards
//
// Revision 1.6  2002/09/04 11:58:45  kooiman
// Added new tuners & fix for new Pinnacle cards with MT2032 tuner.
//
// Revision 1.5  2002/01/16 20:49:30  adcockj
// Added Rob Muller's fixes for Radio
//
// Revision 1.4  2001/12/05 21:45:11  ittarnavsky
// added changes for the AudioDecoder and AudioControls support
//
// Revision 1.3  2001/11/29 17:30:52  adcockj
// Reorgainised bt848 initilization
// More Javadoc-ing
//
// Revision 1.2  2001/11/26 13:02:27  adcockj
// Bug Fixes and standards changes
//
// Revision 1.1  2001/11/25 02:03:21  ittarnavsky
// initial checkin of the new I2C code
//
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MT2032.h"
#include "DebugLog.h"

#define OPTIMIZE_VCO 1   // perform VCO optimizations


CMT2032::CMT2032(eVideoFormat DefaultVideoFormat) :
    m_XOGC(0),
    m_Initialized(false),
    m_Frequency (0),
    m_Locked(false)      
{
    m_DefaultVideoFormat = DefaultVideoFormat;    
}

BYTE CMT2032::GetDefaultAddress() const
{
    return 0xC0>>1;
}
    
eTunerId CMT2032::GetTunerId()
{
    return TUNER_MT2032;
}

eVideoFormat CMT2032::GetDefaultVideoFormat()
{
    return m_DefaultVideoFormat;
}

bool CMT2032::HasRadio() const
{
    return true;
}


BYTE CMT2032::GetRegister(BYTE reg)
{
    BYTE result = 0;
    ReadFromSubAddress(reg, &result, sizeof(result));
    return result;
}

void CMT2032::SetRegister(BYTE reg, BYTE value)
{
    WriteToSubAddress(reg, &value, sizeof(value));
}

WORD CMT2032::GetVersion()
{
    WORD result = 0;
    ReadFromSubAddress(0x13, (BYTE*)&result, sizeof(result));
    return result;
}

WORD CMT2032::GetVendor()
{
    WORD result = 0;
    ReadFromSubAddress(0x11, (BYTE*)&result, sizeof(result));
    return result;
}

void CMT2032::Initialize()
{
    int             xogc, xok = 0;

    if (m_ExternalIFDemodulator != NULL)
    {
        m_ExternalIFDemodulator->Init(TRUE, m_DefaultVideoFormat);
    }

    // Get chip info
    BYTE rdbuf[22];
    BYTE wrbuf[] = { (BYTE)(m_DeviceAddress << 1), 0 };    
    
    if (m_I2CBus->Read(wrbuf,2,rdbuf,21))
    {
        LOG(1,"MT2032: Companycode=%02x%02x Part=%02x Revision=%02x\n",
                    rdbuf[0x11],rdbuf[0x12],rdbuf[0x13],rdbuf[0x14]);
    }

    /* Initialize Registers per spec. */
    SetRegister(2, 0xff);
    SetRegister(3, 0x0f);
    SetRegister(4, 0x1f);
    SetRegister(6, 0xe4);
    SetRegister(7, 0x8f);
    SetRegister(8, 0xc3);
    SetRegister(9, 0x4e);
    SetRegister(10, 0xec);
    SetRegister(13, 0x32);

    /* Adjust XOGC (register 7), wait for XOK */
    xogc = 7;
    do
    {
        Sleep(10);
        xok = GetRegister(0x0e) & 0x01;
        if (xok == 1)
        {
            break;
        }
        xogc--;
        if (xogc == 3)
        {
            xogc = 4;   /* min. 4 per spec */
            break;
        }
        SetRegister(7, 0x88 + xogc);
    } while (xok != 1);


    if (m_ExternalIFDemodulator != NULL)
    {
        m_ExternalIFDemodulator->Init(FALSE, m_DefaultVideoFormat);
    }
    
    m_XOGC = xogc;
    m_Initialized = true;
}

int CMT2032::SpurCheck(int f1, int f2, int spectrum_from, int spectrum_to)
{
    int n1 = 1, n2, f;

    f1 = f1 / 1000;     /* scale to kHz to avoid 32bit overflows */
    f2 = f2 / 1000;
    spectrum_from /= 1000;
    spectrum_to /= 1000;

    do
    {
        n2 = -n1;
        f = n1 * (f1 - f2);
        do
        {
            n2--;
            f = f - f2;
            if ((f > spectrum_from) && (f < spectrum_to))
            {
                return 1;
            }
        } while ((f > (f2 - spectrum_to)) || (n2 > -5));
        n1++;
    } while (n1 < 5);

    return 0;
}

int CMT2032::ComputeFreq(
                            int             rfin,
                            int             if1,
                            int             if2,
                            int             spectrum_from,
                            int             spectrum_to,
                            unsigned char   *buf,
                            int             *ret_sel,
                            int             xogc
                        )   /* all in Hz */
{
    int fref, lo1, lo1n, lo1a, s, sel, lo1freq, desired_lo1, desired_lo2, lo2, lo2n, lo2a, lo2num,
        lo2freq;

    fref = 5250 * 1000; /* 5.25MHz */

    /* per spec 2.3.1 */
    desired_lo1 = rfin + if1;
    lo1 = (2 * (desired_lo1 / 1000) + (fref / 1000)) / (2 * fref / 1000);
    lo1freq = lo1 * fref;
    desired_lo2 = lo1freq - rfin - if2;

    /* per spec 2.3.2 */
    for (int nLO1adjust = 1; nLO1adjust < 3; nLO1adjust++)
    {
        if (!SpurCheck(lo1freq, desired_lo2, spectrum_from, spectrum_to))
        {
            break;
        }

        if (lo1freq < desired_lo1)
        {
            lo1 += nLO1adjust;
        }
        else
        {
            lo1 -= nLO1adjust;
        }

        lo1freq = lo1 * fref;
        desired_lo2 = lo1freq - rfin - if2;
    }

    /* per spec 2.3.3 */
    s = lo1freq / 1000 / 1000;

    if (OPTIMIZE_VCO)
    {
        if (s > 1890)
        {
            sel = 0;
        }
        else if (s > 1720)
        {
            sel = 1;
        }
        else if (s > 1530)
        {
            sel = 2;
        }
        else if (s > 1370)
        {
            sel = 3;
        }
        else
        {
            sel = 4;    /* >1090 */
        }
    }
    else
    {
        if (s > 1790)
        {
            sel = 0;    /* <1958 */
        }
        else if (s > 1617)
        {
            sel = 1;
        }
        else if (s > 1449)
        {
            sel = 2;
        }
        else if (s > 1291)
        {
            sel = 3;
        }
        else
        {
            sel = 4;    /* >1090 */
        }
    }

    *ret_sel = sel;

    /* per spec 2.3.4 */
    lo1n = lo1 / 8;
    lo1a = lo1 - (lo1n * 8);
    lo2 = desired_lo2 / fref;
    lo2n = lo2 / 8;
    lo2a = lo2 - (lo2n * 8);
    lo2num = ((desired_lo2 / 1000) % (fref / 1000)) * 3780 / (fref / 1000); /* scale to fit in 32bit arith */
    lo2freq = (lo2a + 8 * lo2n) * fref + lo2num * (fref / 1000) / 3780 * 1000;

    if (lo1a < 0 ||  
        lo1a > 7 ||  
        lo1n < 17 ||  
        lo1n > 48 ||  
        lo2a < 0 ||  
        lo2a > 7 ||  
        lo2n < 17 ||  
        lo2n > 30)
    {
        return -1;
    }

    /* set up MT2032 register map for transfer over i2c */
    buf[0] = lo1n - 1;
    buf[1] = lo1a | (sel << 4);
    buf[2] = 0x86;                  /* LOGC */
    buf[3] = 0x0f;                  /* reserved */
    buf[4] = 0x1f;
    buf[5] = (lo2n - 1) | (lo2a << 5);
    if (rfin < 400 * 1000 * 1000)
    {
        buf[6] = 0xe4;
    }
    else
    {
        buf[6] = 0xf4;              /* set PKEN per rev 1.2 */
    }

    buf[7] = 8 + xogc;
    buf[8] = 0xc3;                  /* reserved */
    buf[9] = 0x4e;                  /* reserved */
    buf[10] = 0xec;                 /* reserved */
    buf[11] = (lo2num & 0xff);
    buf[12] = (lo2num >> 8) | 0x80; /* Lo2RST */

    return 0;
}

int CMT2032::CheckLOLock(void)
{
    int t, lock = 0;
    for (t = 0; t < 10; t++)
    {
        lock = GetRegister(0x0e) & 0x06;
        if (lock == 6)
        {
            break;
        }
        Sleep(1);
    }
    return lock;
}

int CMT2032::OptimizeVCO(int sel, int lock)
{
    int tad1, lo1a;

    tad1 = GetRegister(0x0f) & 0x07;

    if (tad1 == 0)
    {
        return lock;
    }
    if (tad1 == 1)
    {
        return lock;
    }
    if (tad1 == 2)
    {
        if (sel == 0)
        {
            return lock;
        }
        else
        {
            sel--;
        }
    }
    else
    {
        if (sel < 4)
        {
            sel++;
        }
        else
        {
            return lock;
        }
    }
    lo1a = GetRegister(0x01) & 0x07;
    SetRegister(0x01, lo1a | (sel << 4));
    lock = CheckLOLock();
    return lock;
}

void CMT2032::SetIFFreq(int rfin, int if1, int if2, int from, int to, eVideoFormat videoFormat)
{
    unsigned char   buf[21];
    int             lint_try, ret, sel, lock = 0;

    ret = ComputeFreq(rfin, if1, if2, from, to, &buf[0], &sel, m_XOGC);
    if (ret < 0)
    {
        return;
    }

    if (m_ExternalIFDemodulator != NULL)
    {
        m_ExternalIFDemodulator->TunerSet(TRUE, videoFormat);
    }

    /* send only the relevant registers per Rev. 1.2 */
    SetRegister(0, buf[0x00]);
    SetRegister(1, buf[0x01]);
    SetRegister(2, buf[0x02]);

    SetRegister(5, buf[0x05]);
    SetRegister(6, buf[0x06]);
    SetRegister(7, buf[0x07]);

    SetRegister(11, buf[0x0B]);
    SetRegister(12, buf[0x0C]);

    /* wait for PLLs to lock (per manual), retry LINT if not. */
    for (lint_try = 0; lint_try < 2; lint_try++)
    {
        lock = CheckLOLock();

        if (OPTIMIZE_VCO)
        {
            lock = OptimizeVCO(sel, lock);
        }

        if (lock == 6)
        {
            break;
        }

        /* set LINT to re-init PLLs */
        SetRegister(7, 0x80 + 8 + m_XOGC);
        Sleep(10);
        SetRegister(7, 8 + m_XOGC);
    }

    SetRegister(2, 0x20);

    m_Locked = (lock==6)?true:false;

    if (m_ExternalIFDemodulator != NULL)
    {
        m_ExternalIFDemodulator->TunerSet(FALSE, videoFormat);
    }
}

bool CMT2032::SetTVFrequency(long frequency, eVideoFormat videoFormat)
{
    if (!m_Initialized)
    {
        Initialize();
    }
    int if2, from, to;

    /* signal bandwidth and picture carrier */
    if (IsNTSCVideoFormat(videoFormat))
    {
        from = 40750 * 1000;
        to = 46750 * 1000;
        if2 = 45750 * 1000;
    }
    else
    {   /* PAL */
        from = 32900 * 1000;
        to = 39900 * 1000;
        if2 = 38900 * 1000;
    }

    m_Frequency = frequency;

    //SetIFFreq(frequency * 1000 / 16 * 1000, 1090 * 1000 * 1000, if2, from, to, videoFormat);
    SetIFFreq(frequency, 1090 * 1000 * 1000, if2, from, to, videoFormat);
    return true;
}

bool CMT2032::SetRadioFrequency(long nFrequency)
{
    if (!m_Initialized)
    {
        Initialize();
    }
    int if2, from, to;

    // from and to values are estimated. This should be no problem
    // since these values are only used in the spur checking routine.
    // According to the documentation spurs do not occur in the North
    // American FM band. (87-108MHz)
    // to - from = 0.2MHz, this is the bandwidth used for FM.
    from = 10600 * 1000;
    to = 10800 * 1000;
    if2 = 10700 * 1000;

    m_Frequency = nFrequency;

    //SetIFFreq(nFrequency * 1000 / 16 * 1000, 1085 * 1000 * 1000, if2, from, to, (eVideoFormat)(VIDEOFORMAT_LASTONE+1));

	SetIFFreq(nFrequency, 1085 * 1000 * 1000, if2, if2, if2, (eVideoFormat)(VIDEOFORMAT_LASTONE+1));
    return true;
}

long CMT2032::GetFrequency()
{
    return m_Frequency;
}

eTunerLocked CMT2032::IsLocked()
{
    return TUNER_LOCK_ON;
}

eTunerAFCStatus CMT2032::GetAFCStatus(long &nFreqDeviation)
{
    eTunerAFCStatus AFCStatus = TUNER_AFC_NOTSUPPORTED;
    if (m_ExternalIFDemodulator != NULL)
    {
        AFCStatus = m_ExternalIFDemodulator->GetAFCStatus(nFreqDeviation);
    }

    if (AFCStatus == TUNER_AFC_NOTSUPPORTED)
    {
        //Use internal AFC
        if (!m_Locked)
        {
            AFCStatus = TUNER_AFC_NOCARRIER;
        }
        else
        {            
            BYTE addr[] = { (BYTE)(m_DeviceAddress << 1), 0x0E };
            BYTE result;        
            if (m_I2CBus->Read(addr, 2, &result, 1))
            {
                result = (result>>4) & 7;
                nFreqDeviation = 100000 * ((int)result - 2);
                AFCStatus = TUNER_AFC_CARRIER;
            }
            AFCStatus = TUNER_AFC_NOTSUPPORTED;
        }
    }
    return AFCStatus;
}
