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

std::string CDShowBaseCrossbar::GetInputName(long Index)
{
    long type=GetInputType(Index);

    switch(type)
    {
    case PhysConn_Video_Tuner:
        return "Video Tuner";
        break;
    case PhysConn_Video_Composite:
        return "Video Composite";
        break;
    case PhysConn_Video_SVideo:
        return "Video SVideo";
        break;
    case PhysConn_Video_RGB:
        return "Video RGB";
        break;
    case PhysConn_Video_YRYBY:
        return "Video YRYBY";
        break;
    case PhysConn_Video_SerialDigital:
        return "Video SerialDigital";
        break;
    case PhysConn_Video_ParallelDigital:
        return "Video ParallelDigital";
        break;
    case PhysConn_Video_SCSI:
        return "Video SCSI";
        break;
    case PhysConn_Video_AUX:
        return "Video AUX";
        break;
    case PhysConn_Video_1394:
        return "Video 1394";
        break;
    case PhysConn_Video_USB:
        return "Video USB";
        break;
    case PhysConn_Video_VideoDecoder:
        return "Video Decoder";
        break;
    case PhysConn_Video_VideoEncoder:
        return "Video Encoder";
        break;
    case PhysConn_Video_SCART:
        return "Video SCART";
        break;
    case PhysConn_Video_Black:
        return "Video Black"; //what is this ???

    case PhysConn_Audio_Tuner:
        return "Audio Tuner";
        break;
    case PhysConn_Audio_Line:
        return "Audio Line";
        break;
    case PhysConn_Audio_Mic:
        return "Audio Mic";
        break;
    case PhysConn_Audio_AESDigital:
        return "Audio AESDigital";
        break;
    case PhysConn_Audio_SPDIFDigital:
        return "Audio SPDIFDigital";
        break;
    case PhysConn_Audio_SCSI:
        return "Audio SCSI";
        break;
    case PhysConn_Audio_AUX:
        return "Audio AUX";
        break;
    case PhysConn_Audio_1394:
        return "Audio 1394";
        break;
    case PhysConn_Audio_USB:
        return "Audio USB";
        break;
    case PhysConn_Audio_AudioDecoder:
        return "Audio Decoder";
        break;
    default:
        return "Unknown";
        break;
    }
}
#endif