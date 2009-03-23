/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Torbjörn Jansson.  All rights reserved.
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
 * @file DShowBaseSource.h interface for the CDShowBaseSource class.
 */

#if !defined(AFX_DSHOWBASESOURCE_H__AB8F10EC_CF36_4398_8F9F_68144D830D0D__INCLUDED_)
#define AFX_DSHOWBASESOURCE_H__AB8F10EC_CF36_4398_8F9F_68144D830D0D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DSObject.h"

/**
 * Baseclass for direct show sources.
 */
class CDShowBaseSource : public CDShowObject  
{
public:
    CDShowBaseSource(IGraphBuilder *pGraph);
    virtual ~CDShowBaseSource();
    
    /**
     * Used to connect this source to a downstream filter (renderer)
     * @param VideoFilter video renderer filter to connect to
     * @param AudioFilter audio renderer filter to connect to, or null to let the source decide.
     */
    virtual void Connect(CComPtr<IBaseFilter> VideoFilter)=0;

    /**
     * Checks if this source is connected
     * @return TRUE if this source is connected
     */
    virtual BOOL IsConnected()=0;

    /**
     * Gets number of dropped frames.
     * @return number of dropped frames
     */
    virtual long GetNumDroppedFrames()=0;

protected:
    friend class CDShowGraph;
    void SetAudioDevice(std::string device);
    CComPtr<IBaseFilter> GetNewAudioRenderer();

private:
    std::string m_AudioDevice;
};

#endif // !defined(AFX_DSHOWBASESOURCE_H__AB8F10EC_CF36_4398_8F9F_68144D830D0D__INCLUDED_)
