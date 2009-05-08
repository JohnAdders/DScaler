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
 * @file BaseCrossbar.cpp implementation of the CDShowBaseCrossbar class.
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT

#include "BaseCrossbar.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDShowBaseCrossbar::CDShowBaseCrossbar(IGraphBuilder *pGraph)
:CDShowObject(pGraph)
{

}

CDShowBaseCrossbar::~CDShowBaseCrossbar()
{

}

tstring CDShowBaseCrossbar::GetInputName(long Index)
{
    long type=GetInputType(Index);

    switch(type)
    {
    case PhysConn_Video_Tuner:
        return _T("Video Tuner");
        break;
    case PhysConn_Video_Composite:
        return _T("Video Composite");
        break;
    case PhysConn_Video_SVideo:
        return _T("Video SVideo");
        break;
    case PhysConn_Video_RGB:
        return _T("Video RGB");
        break;
    case PhysConn_Video_YRYBY:
        return _T("Video YRYBY");
        break;
    case PhysConn_Video_SerialDigital:
        return _T("Video SerialDigital");
        break;
    case PhysConn_Video_ParallelDigital:
        return _T("Video ParallelDigital");
        break;
    case PhysConn_Video_SCSI:
        return _T("Video SCSI");
        break;
    case PhysConn_Video_AUX:
        return _T("Video AUX");
        break;
    case PhysConn_Video_1394:
        return _T("Video 1394");
        break;
    case PhysConn_Video_USB:
        return _T("Video USB");
        break;
    case PhysConn_Video_VideoDecoder:
        return _T("Video Decoder");
        break;
    case PhysConn_Video_VideoEncoder:
        return _T("Video Encoder");
        break;
    case PhysConn_Video_SCART:
        return _T("Video SCART");
        break;
    case PhysConn_Video_Black:
        return _T("Video Black"); //what is this ???

    case PhysConn_Audio_Tuner:
        return _T("Audio Tuner");
        break;
    case PhysConn_Audio_Line:
        return _T("Audio Line");
        break;
    case PhysConn_Audio_Mic:
        return _T("Audio Mic");
        break;
    case PhysConn_Audio_AESDigital:
        return _T("Audio AESDigital");
        break;
    case PhysConn_Audio_SPDIFDigital:
        return _T("Audio SPDIFDigital");
        break;
    case PhysConn_Audio_SCSI:
        return _T("Audio SCSI");
        break;
    case PhysConn_Audio_AUX:
        return _T("Audio AUX");
        break;
    case PhysConn_Audio_1394:
        return _T("Audio 1394");
        break;
    case PhysConn_Audio_USB:
        return _T("Audio USB");
        break;
    case PhysConn_Audio_AudioDecoder:
        return _T("Audio Decoder");
        break;
    default:
        return _T("Unknown");
        break;
    }
}
#endif