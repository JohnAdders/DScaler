
#ifndef __SAA7134I2CBUSINTERFACE_H__
#define __SAA7134I2CBUSINTERFACE_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SAA7134I2CInterface.h"
#include "I2CBus.h"


class CSAA7134I2CBusInterface : public CI2CBus
{
public:
    CSAA7134I2CBusInterface(ISAA7134I2CInterface *saa7134Interface);

public:
    /**
    @return true if sucessful
    */
    virtual bool Read(
                        const BYTE *writeBuffer,
                        size_t writeBufferSize,
                        BYTE *readBuffer,
                        size_t readBufferSize
                     );
    
    /**
    @return true if sucessful
    */
    virtual bool Write(const BYTE *writeBuffer, size_t writeBufferSize);

protected:
    virtual void Sleep();
	virtual void SetData(BYTE Data);
	virtual BYTE GetData();
	virtual bool I2CStart();
	virtual bool I2CContinue();
	virtual bool I2CStop();
	virtual bool BusyWait();
	virtual bool IsReady();

    virtual void Start() {};
    virtual void Stop() {};
    virtual bool Write(BYTE byte) { return FALSE; };
    virtual BYTE Read(bool last=true) { return 0x00; };
    virtual bool GetAcknowledge() { return FALSE; };
    virtual void SendACK() {};
    virtual void SendNAK() {};

private:
    ISAA7134I2CInterface *m_SAA7134Interface;
};


#endif
