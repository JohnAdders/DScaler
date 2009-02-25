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
// Change Log
//
// Date          Developer             Changes
//
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.1  2001/12/17 19:22:33  tobbej
// new crossbar classes
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file BaseCrossbar.cpp implementation of the CDShowBaseCrossbar class.
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT

#include "BaseCrossbar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

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

char* CDShowBaseCrossbar::GetInputName(long Index)
{
	long type=GetInputType(Index);

	switch(type)
	{
	case PhysConn_Video_Tuner:
		return TEXT("Video Tuner");
		break;
	case PhysConn_Video_Composite:
		return TEXT("Video Composite");
		break;
	case PhysConn_Video_SVideo:
		return TEXT("Video SVideo");
		break;
	case PhysConn_Video_RGB:
		return TEXT("Video RGB");
		break;
	case PhysConn_Video_YRYBY:
		return TEXT("Video YRYBY");
		break;
	case PhysConn_Video_SerialDigital:
		return TEXT("Video SerialDigital");
		break;
	case PhysConn_Video_ParallelDigital:
		return TEXT("Video ParallelDigital");
		break;
	case PhysConn_Video_SCSI:
		return TEXT("Video SCSI");
		break;
	case PhysConn_Video_AUX:
		return TEXT("Video AUX");
		break;
	case PhysConn_Video_1394:
		return TEXT("Video 1394");
		break;
	case PhysConn_Video_USB:
		return TEXT("Video USB");
		break;
	case PhysConn_Video_VideoDecoder:
		return TEXT("Video Decoder");
		break;
	case PhysConn_Video_VideoEncoder:
		return TEXT("Video Encoder");
		break;
	case PhysConn_Video_SCART:
		return TEXT("Video SCART");
		break;
	case PhysConn_Video_Black:
		return TEXT("Video Black"); //what is this ???

	case PhysConn_Audio_Tuner:
		return TEXT("Audio Tuner");
		break;
	case PhysConn_Audio_Line:
		return TEXT("Audio Line");
		break;
	case PhysConn_Audio_Mic:
		return TEXT("Audio Mic");
		break;
	case PhysConn_Audio_AESDigital:
		return TEXT("Audio AESDigital");
		break;
	case PhysConn_Audio_SPDIFDigital:
		return TEXT("Audio SPDIFDigital");
		break;
	case PhysConn_Audio_SCSI:
		return TEXT("Audio SCSI");
		break;
	case PhysConn_Audio_AUX:
		return TEXT("Audio AUX");
		break;
	case PhysConn_Audio_1394:
		return TEXT("Audio 1394");
		break;
	case PhysConn_Audio_USB:
		return TEXT("Audio USB");
		break;
	case PhysConn_Audio_AudioDecoder:
		return TEXT("Audio Decoder");
		break;
	default:
		return TEXT("Unknown");
		break;
	}
}
#endif