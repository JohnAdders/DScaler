#ifndef _BT848_VOODOOTV_IFDEM_H_
#define _BT848_VOODOOTV_IFDEM_H_

#include "ITuner.h"
#include "BT848Card.h"

class CPreTuneVoodooFM : public IExternalIFDemodulator
{
public:
    CPreTuneVoodooFM(CBT848Card* pBT848Card);

    void TunerSet(bool bPreSet, eVideoFormat videoFormat);
    bool Detect();

    void Init(bool bPreInit, eVideoFormat videoFormat) { TunerSet(bPreInit, videoFormat); }
    eTunerAFCStatus GetAFCStatus(long &nFreqDeviation) { return TUNER_AFC_NOTSUPPORTED; }
protected:
    // from CI2CDevice
    virtual BYTE GetDefaultAddress() const { return 0; }
private:
    CBT848Card* m_pBT848Card;
};

#endif