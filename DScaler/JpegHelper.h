/////////////////////////////////////////////////////////////////////////////
// $Id: JpegHelper.h,v 1.1 2002-05-01 12:57:19 laurentg Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Laurent Garnier.  All rights reserved.
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
// CVS Log
//
// $Log: not supported by cvs2svn $
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __JPEGHELPER_H___
#define __JPEGHELPER_H___

#include "StillSource.h"


/** A helper class that can read and write JPEG files
*/
class CJpegHelper : public CStillSourceHelper
{
public:
    CJpegHelper(CStillSource* pParent);
    BOOL OpenMediaFile(LPCSTR FileName);
    void SaveSnapshot(LPCSTR FilePath, int Height, int Width, BYTE* pOverlay, LONG OverlayPitch);
};

#endif