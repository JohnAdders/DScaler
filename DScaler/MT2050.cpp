//
// $Id: MT2050.cpp,v 1.1 2003-12-18 15:57:41 adcockj Exp $
//
/////////////////////////////////////////////////////////////////////////////
//
// copyright 2003 Ralph Metzler, Gerd Knorr, Gunther Mayer
// Code adapted from video4linux tuner.c by John Adcock
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
/////////////////////////////////////////////////////////////////////////////

/**
 * @file MT2050.cpp CMT2050 Implementation
 */

#include "stdafx.h"
#include "MT2050.h"
#include "DebugLog.h"


CMT2050::CMT2050(eVideoFormat DefaultVideoFormat) :
    m_Initialized(false),
    m_Frequency (0)      
{
    m_DefaultVideoFormat = DefaultVideoFormat;    
}

BYTE CMT2050::GetDefaultAddress() const
{
    return 0xC0>>1;
}
    
eTunerId CMT2050::GetTunerId()
{
    return TUNER_MT2050;
}

eVideoFormat CMT2050::GetDefaultVideoFormat()
{
    return m_DefaultVideoFormat;
}

bool CMT2050::HasRadio() const
{
    return false;
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
    if (m_ExternalIFDemodulator != NULL)
    {
        m_ExternalIFDemodulator->Init(TRUE, m_DefaultVideoFormat);
    }

    // Get chip info
    BYTE rdbuf[22];
    BYTE wrbuf[] = { (BYTE)(m_DeviceAddress << 1), 0 };    
    
    if (m_I2CBus->Read(wrbuf,2,rdbuf,21))
    {
        LOG(1,"MT2050: Companycode=%02x%02x Part=%02x Revision=%02x\n",
                    rdbuf[0x11],rdbuf[0x12],rdbuf[0x13],rdbuf[0x14]);
    }

    

    /* Initialize Registers per spec. */
    //  power
    SetRegister(6, 0x10);
    // m1lo
    SetRegister(0x0f, 0x0f);

    BYTE Sro = GetRegister(0x0d);
    LOG(2,"MT2050: SRO = %02x\n", Sro);

    if (m_ExternalIFDemodulator != NULL)
    {
        m_ExternalIFDemodulator->Init(FALSE, m_DefaultVideoFormat);
    }
    
    m_Initialized = true;
}

int CMT2050::ComputeFreq(
                            int             rfin,
                            int             if2,
                            unsigned char   *buf
                        )   /* all in Hz */
{
	unsigned int if1=1218*1000*1000;
	unsigned int f_lo1,f_lo2,lo1,lo2,f_lo1_modulo,f_lo2_modulo,num1,num2,div1a,div1b,div2a,div2b;
	
	f_lo1=rfin+if1;
	f_lo1=(f_lo1/1000000)*1000000;
	
	f_lo2=f_lo1-rfin-if2;
	f_lo2=(f_lo2/50000)*50000;
	
	lo1=f_lo1/4000000;
	lo2=f_lo2/4000000;
	
	f_lo1_modulo= f_lo1-(lo1*4000000);
	f_lo2_modulo= f_lo2-(lo2*4000000);
	
	num1=4*f_lo1_modulo/4000000;
	num2=4096*(f_lo2_modulo/1000)/4000;
	
	// todo spurchecks
	
    div1a=(lo1/12)-1;
	div1b=lo1-(div1a+1)*12;
	
	div2a=(lo2/8)-1;
	div2b=lo2-(div2a+1)*8;
	
	buf[0]= 4*div1b + num1;
	if(rfin<275*1000*1000)
    {
        buf[0] = buf[0]|0x80;
    }
	
	buf[1]=div1a;
	buf[2]=32*div2b + num2/256;
	buf[3]=num2-(num2/256)*256;
	buf[4]=div2a;
	if(num2!=0)
    {
        buf[4]=buf[4]|0x40;
    }
    return 0;	

}

void CMT2050::SetIFFreq(int rfin, int if2, eVideoFormat videoFormat)
{
    unsigned char   buf[5];
    int             ret;

    ret = ComputeFreq(rfin, if2, &buf[0]);
    if (ret < 0)
    {
        return;
    }

    if (m_ExternalIFDemodulator != NULL)
    {
        m_ExternalIFDemodulator->TunerSet(TRUE, videoFormat);
    }

    /* send only the relevant registers per Rev. 1.2 */
    WriteToSubAddress(1, buf, 5);

    if (m_ExternalIFDemodulator != NULL)
    {
        m_ExternalIFDemodulator->TunerSet(FALSE, videoFormat);
    }
}

bool CMT2050::SetTVFrequency(long frequency, eVideoFormat videoFormat)
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

    SetIFFreq(frequency, if2, videoFormat);
    return true;
}

bool CMT2050::SetRadioFrequency(long nFrequency)
{
    return false;
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
    if (m_ExternalIFDemodulator != NULL)
    {
        AFCStatus = m_ExternalIFDemodulator->GetAFCStatus(nFreqDeviation);
    }
    return AFCStatus;
}
