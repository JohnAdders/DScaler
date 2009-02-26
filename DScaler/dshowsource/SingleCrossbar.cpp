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
 * @file SingleCrossbar.cpp implementation of the CDShowSingleCrossbar class.
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT

#include "SingleCrossbar.h"
#include "exception.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDShowSingleCrossbar::CDShowSingleCrossbar(CComPtr<IAMCrossbar> &pCrossbar,IGraphBuilder *pGraph)
:m_crossbar(pCrossbar),CDShowBaseCrossbar(pGraph)
{
    ASSERT(m_crossbar!=NULL);
}

CDShowSingleCrossbar::~CDShowSingleCrossbar()
{

}

void CDShowSingleCrossbar::GetPinCounts(long &cIn,long &cOut)
{    
    HRESULT hr=m_crossbar->get_PinCounts(&cOut,&cIn);
    if(FAILED(hr))
    {
        throw CCrossbarException("get_PinCounts failed",hr);
    }
}

PhysicalConnectorType CDShowSingleCrossbar::GetInputType(long Index)
{
    long cPinRelated,type;
    HRESULT hr=m_crossbar->get_CrossbarPinInfo(TRUE,Index,&cPinRelated,&type);
    if(FAILED(hr))
    {
        throw CCrossbarException("get_CrossbarPinInfo failed",hr);
    }
    return (PhysicalConnectorType)type;
}

void CDShowSingleCrossbar::SetInputIndex(long Index,bool bSetRelated)
{
    long cInputPinRelated,cOutputPinRelated,type;
    long cInput,cOutput;
    
    HRESULT hr=m_crossbar->get_PinCounts(&cOutput,&cInput);
    if(FAILED(hr))
    {
        throw CCrossbarException("get_PinCounts failed",hr);
    }

    //set the related pin too?
    if(bSetRelated==true)
    {
        hr=m_crossbar->get_CrossbarPinInfo(TRUE,Index,&cInputPinRelated,&type);
        if(FAILED(hr))
        {
            throw CCrossbarException("get_CrossbarPinInfo faile",hr);
        }
    }
    
    //check all outputs to see if its posibel to connect with selected input
    bool bInputRouted=false;
    for(int i=0;i<cOutput;i++)
    {
        //CanRoute returns S_FALSE if it cant route
        if(m_crossbar->CanRoute(i,Index)==S_OK)
        {
            hr=m_crossbar->Route(i,Index);
            if(FAILED(hr))
            {
                throw CCrossbarException("failed to route",hr);
            }
            
            //output pin for related pin
            hr=m_crossbar->get_CrossbarPinInfo(FALSE,i,&cOutputPinRelated,&type);
            
            bInputRouted=true;
            break;
        }
    }
    
    //successfull?
    if(bInputRouted==false)
    {
        throw CCrossbarException("Cant find route");
    }

    if(bSetRelated==true)
    {
        if(SUCCEEDED(m_crossbar->CanRoute(cOutputPinRelated,cInputPinRelated)))
        {
            if(FAILED(m_crossbar->Route(cOutputPinRelated,cInputPinRelated)))
            {
                throw CCrossbarException("failed to route related pin",hr);
            }

        }
    }
}

long CDShowSingleCrossbar::GetInputIndex(long OutIndex)
{
    long InputIndex=0;
    HRESULT hr=m_crossbar->get_IsRoutedTo(OutIndex,&InputIndex);
    if(FAILED(hr))
    {
        throw CCrossbarException("IAMCrossbar::get_IsRoutedTo failed",hr);
    }
    return InputIndex;
}

bool CDShowSingleCrossbar::IsInputSelected(long index)
{
    long cInput,cOutput;
    GetPinCounts(cInput,cOutput);

    for(int i=0;i<cOutput;i++)
    {
        long inputIndex=GetInputIndex(i);
        if(index==inputIndex)
        {
            return true;
        }
    }
    return false;
}
#endif