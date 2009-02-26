/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Torbjörn Jansson.  All rights reserved.
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

/**
 * @file DSTVTuner.h interface for the CDSTVTuner class.
 */

#ifndef _DSTVTUNER_H_
#define _DSTVTUNER_H_

#include "DSObject.h"
#include "exception.h"
#include "TVFormats.h"

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

    long GetCountryCode();
    void PutCountryCode(long CountryCode);
    TunerInputType GetInputType();
    void SetInputType(TunerInputType NewType);
    long GetChannel();
    BOOL SetChannel(long lChannel);

    BOOL SetTunerFrequency(long dwFrequency);
    long GetTunerFrequency();

    BOOL SetInputPin(long lInputPin);

    BOOL LoadFrequencyTable(int CountryCode, TunerInputType InputType);
private:
    /// Tv Tuner
    CComPtr<IAMTVTuner> m_TVTuner;
    long m_InputPin;

    int m_TunerInput;

    int m_CountryCode;

    long m_MinChannel;
    long m_MaxChannel;
    std::vector<long> m_FrequencyTable;

    int m_CustomFrequencyTable;
    long *m_AutoTuneInfo;
    long *m_AutoTuneTag;
    BOOL m_CountryOrInputChanged;

    static const long DSUniFreqTable[];
    int FrequencyToChannel(long lFrequency);
};

#endif
