/////////////////////////////////////////////////////////////////////////////
// $Id: DSProvider.h,v 1.3 2002-08-20 16:22:59 tobbej Exp $
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
// Change Log
//
// Date          Developer             Changes
//
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.2  2001/12/17 19:36:45  tobbej
// renamed a few classes
//
// Revision 1.1  2001/12/09 22:01:48  tobbej
// experimental dshow support, doesnt work yet
// define WANT_DSHOW_SUPPORT if you want to try it
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file DSProvider.h interface for the CDSProvider class.
 */

#if !defined(AFX_DSPROVIDER_H__D8D191FB_3156_439A_9261_F81B9EB3B9E4__INCLUDED_)
#define AFX_DSPROVIDER_H__D8D191FB_3156_439A_9261_F81B9EB3B9E4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SourceProvider.h"
#include "DSSource.h"
#include "DSFileSource.h"

#include <map>

/**
 * Source provider for Direct show video capture devices.
 * @bug there will probably be problems if some devices is removed while dscaler is running
 * @see CDSCaptureSource
 * @see CDSFileSource
 */
class CDSProvider : public ISourceProvider  
{
public:
	CDSProvider();
	virtual ~CDSProvider();

    virtual int GetNumberOfSources();
    virtual CSource* GetSource(int SourceIndex);
	
	string getSourceName(int SourceIndex);

private:
	vector<CDSSourceBase*> m_DSSources;
	map<int,string> m_SourceNames;
	
};

#endif // !defined(AFX_DSPROVIDER_H__D8D191FB_3156_439A_9261_F81B9EB3B9E4__INCLUDED_)
