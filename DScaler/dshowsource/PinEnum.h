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
 * @file PinEnum.h interface for the CDShowPinEnum class.
 */

#if !defined(AFX_PINENUM_H__1446F155_3DEC_4476_805E_0B74863B7EC7__INCLUDED_)
#define AFX_PINENUM_H__1446F155_3DEC_4476_805E_0B74863B7EC7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "exception.h"

/**
 * Template based generic enumerator class
 */
template<typename EnumInterface,typename T>
class CDShowGenericEnum
{
public:
    /**
     * Constructor that dont set the enumerator. init must be called before use.
     */
    CDShowGenericEnum() : 
      m_pEnum(NULL)
    {
    
    }

    /**
     * Constructor that sets the enumerator.
     */
    CDShowGenericEnum(EnumInterface *pEnum) : 
      m_pEnum(pEnum)
    {
    
    }
    virtual ~CDShowGenericEnum()
    {
    
    }

    /**
     * @param pEnum enumerator interface to use
     * @exception throws CDShowException if enumerator is already set
     */
    void init(EnumInterface *pEnum)
    {
        if(m_pEnum!=NULL)
        {
            throw CDShowException("Enumerator already initialized");
        }
        m_pEnum=pEnum;
    }
    
    HRESULT next(T** ppItem)
    {
        if(m_pEnum==NULL ||ppItem==NULL)
            return E_POINTER;
        return m_pEnum->Next(1,ppItem,NULL);
    }

    /**
     * Skips specified amount of items.
     * 
     * @param c number of items to skip
     * @return HRESULT describing success or failiure
     */
    HRESULT skip(ULONG c)
    {
        if(m_pEnum==NULL)
            return E_POINTER;

        return m_pEnum->Skip(c);
    }

    /**
     * Resets the enumerator to the begining.
     * @return HRESULT describing success or failiure
     */
    HRESULT reset()
    {
        if(m_pEnum==NULL)
            return E_POINTER;
        return m_pEnum->Reset();
    }

    /**
     * Copys the enumerator.
     * @param ppEnum pointer to interface to return
     */
    HRESULT clone(EnumInterface **ppEnum)
    {
        if(m_pEnum==NULL || ppEnum==NULL)
            return E_POINTER;
        
        return m_pEnum->Clone(ppEnum);
    }
    
    /**
     * Index operator.
     *
     * @param index index of pin to return
     * @exception CDShowException is thrown if something goes wrong, for example if the enumerator has changed
     */
    T* operator[](int index)
    {
        T* item=NULL;
    
        HRESULT hr=reset();
        if(FAILED(hr))
            throw CDShowException("reset failed",hr);

        for(int i=0;i<=index;i++)
        {
            if(item!=NULL)
            {
                item->Release();
                item=NULL;
            }
            hr=next(&item);
            if(hr!=S_OK)
            {
                throw CDShowException("next failed",hr);
            }
        }
        ASSERT(item!=NULL);
        return item;
    }

public:
    ///enumerator interface
    CComPtr<EnumInterface> m_pEnum;
};


/**
 * Exception class for CDShowPinEnum
 * @see CDShowException
 * @see CDShowPinEnum
 */
class CDShowPinEnumException :public CDShowException
{
public:
    CDShowPinEnumException(const char* msg,HRESULT hr = S_OK):CDShowException(msg,hr) {};
};

/**
 * Class that enumerates a IBaseFilter's pins.
 * It can be locked to a specified direction (input or output pins)
 * @see CDShowGenericEnum
 */
class CDShowPinEnum :public CDShowGenericEnum<IEnumPins,IPin>
{
public:
    /**
     * Constructs a CPinEnum object that enumerates all pins in specified direction
     *
     * @exception CPinEnumException this is thrown if unable to create a enumerator for the filter
     * @param filter filter whos pins is to be enumerated
     * @param pinDir direction
     */
    CDShowPinEnum(CComPtr<IBaseFilter> filter,PIN_DIRECTION pinDir);    //endast viss riktning
    
    /**
     * Constructs a CPinEnum object that enumerates all pins.
     *
     * @exception CPinEnumException this is thrown if unable to create a enumerator for the filter
     * @param filter filter whos pins is to be enumerated
     */
    CDShowPinEnum(CComPtr<IBaseFilter> filter);
    
    /**
     * Destructor
     */
    virtual ~CDShowPinEnum();

    /**
     * Gets next pin.
     * Currently thers no exception thrown, insted a NULL is returned
     *
     * @return next pin or NULL if not found
     */
    CComPtr<IPin> next();

private:
    ///direction
    PIN_DIRECTION m_pinDir;    
    ///any direction or only specified?
    BOOL m_anydir;
};

#endif // !defined(AFX_PINENUM_H__1446F155_3DEC_4476_805E_0B74863B7EC7__INCLUDED_)
