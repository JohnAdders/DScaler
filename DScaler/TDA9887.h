#ifndef _TDA_9887_H_
#define _TDA_9887_H_

#include "TVFormats.h"
#include "I2CDevice.h"
#include "ITuner.h"

#define I2C_TDA9887_0				0x86

class CTDA9887 : public IExternalIFDemodulator
{
public:
    CTDA9887();
    ~CTDA9887();

    bool Detect();

    void Init(bool bPreInit, eVideoFormat videoFormat);
    void TunerSet(bool bPreSet, eVideoFormat videoFormat);
    
    eTunerAFCStatus GetAFCStatus(long &nFreqDeviation);
protected:
    // from CI2CDevice
    virtual BYTE GetDefaultAddress() const { return I2C_TDA9887_0; }
};

class CTDA9887Pinnacle : public CTDA9887
{
public:
    CTDA9887Pinnacle(int CardId);
    ~CTDA9887Pinnacle();

    void Init(bool bPreInit, eVideoFormat videoFormat);
    void TunerSet(bool bPreSet, eVideoFormat videoFormat);    
private:
    int m_CardId;
    eVideoFormat m_LastVideoFormat;
};

#endif
