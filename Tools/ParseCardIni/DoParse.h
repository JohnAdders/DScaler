/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 Torsten Seeboth. All rights reserved.
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

#if !defined(AFX_DOPARSE_H__BCC73107_96A6_47BA_A1E5_9E3D6B4B2A4A__INCLUDED_)
#define AFX_DOPARSE_H__BCC73107_96A6_47BA_A1E5_9E3D6B4B2A4A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "HierarchicalConfigParser.h"
#include "ParsingCommon.h"

class CDoParse  
{
protected:
    CDoParse(const char* pszFileName, CListBox* pErrorList, CTreeCtrl* pTreeCtrl, BOOL bSort);
    virtual ~CDoParse();

protected:
    virtual void DisplayCardsInTree() = 0;
    virtual void ParseIniFile() = 0;

protected:
    CTreeCtrl*   m_pTreeCtrl;
    CListBox*    m_pErrorList;
    const char*  m_pszFileName;
    BOOL         m_bDoSort;

    static const char*  m_TDAFormatNames[];


};

#endif // !defined(AFX_DOPARSE_H__BCC73107_96A6_47BA_A1E5_9E3D6B4B2A4A__INCLUDED_)
