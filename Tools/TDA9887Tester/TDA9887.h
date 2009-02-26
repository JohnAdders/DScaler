/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004 Torsten Seeboth. All rights reserved.
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
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_TDA9887_H__2F8FE09E_95BA_4D91_9DE5_AA52287AB2CC__INCLUDED_)
#define AFX_TDA9887_H__2F8FE09E_95BA_4D91_9DE5_AA52287AB2CC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PCICard.h"

class CTDA9887  
{
public:
    bool WriteControlBytes(const BYTE* pControlBytes);
    const BYTE* GetI2CAdress();
    BOOL Detect();
    CTDA9887(CPCICard* pCard);
    virtual ~CTDA9887();

private:
    CPCICard*    m_pPCICard;
    BYTE        m_I2CAdress;

};

#endif // !defined(AFX_TDA9887_H__2F8FE09E_95BA_4D91_9DE5_AA52287AB2CC__INCLUDED_)
