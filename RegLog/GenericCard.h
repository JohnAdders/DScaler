/////////////////////////////////////////////////////////////////////////////
// $Id: GenericCard.h,v 1.1 2002-11-27 17:32:59 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 John Adcock.  All rights reserved.
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
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

#ifndef __GENERICCARD_H___
#define __GENERICCARD_H___

#include "../DScaler/PCICard.h"

class CGenericCard : public CPCICard
{
public:
    CGenericCard(CHardwareDriver* pDriver);
    ~CGenericCard();

protected:
    void ManageMyState() {};
    BOOL SupportsACPI() {return FALSE;};
    void ResetChip() {};

private:
};


#endif