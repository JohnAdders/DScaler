#ifndef _DSTVTUNER_H_
#define _DSTVTUNER_H_

#include "DSObject.h"
#include "exception.h"

class CDShowTvTunerException: public CDShowException
{
public:
	CDShowTvTunerException(CString msg,HRESULT hr):CDShowException(msg,hr) {};
	CDShowTvTunerException(CString msg):CDShowException(msg) {};
};


class CDShowTVTuner : public CDShowObject
{
public:
  CDShowTVTuner(IGraphBuilder *pGraph);
	CDShowTVTuner(CComPtr<IAMTVTuner> &pTvTuner,IGraphBuilder *pGraph);
	virtual ~CDShowTVTuner();
  
	eDSObjectType getObjectType() {return DSHOW_TYPE_TUNER;}

  long getCountryCode();
  void putCountryCode(long CountryCode);
  TunerInputType getInputType();
  void setInputType(TunerInputType NewType);
  long getChannel();
  BOOL setChannel(long lChannel);

  BOOL setTunerFrequency(long dwFrequency);
  long getTunerFrequency();
  
  BOOL setInput(long lInputPin);

  BOOL LoadFrequencyTable(int CountryCode, TunerInputType InputType);
private:
	/// Tv Tuner
	CComPtr<IAMTVTuner> m_TVTuner;
	long  m_InputPin;

  int   m_TunerInput;
  
  int   m_CountryCode;
  
  long  m_MinChannel;
  long  m_MaxChannel;
  std::vector<long> m_FrequencyTable;

  static const long DSUniFreqTable[];

  int   FrequencyToChannel(long lFrequency);
};

#endif
