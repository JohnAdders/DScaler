/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Laurent Garnier.  All rights reserved.
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
 * @file tiffhelper.h tiffhelper Header file
 */
 
#ifndef __TIFFHELPER_H___
#define __TIFFHELPER_H___

#include "StillSource.h"


enum eTIFFClass {
    TIFF_CLASS_R = 0,
    TIFF_CLASS_R_JPEG,
    TIFF_CLASS_Y,
};


#define SQUARE_MARK "(square)"


/** A helper class that can read and write TIFF files
*/
class CTiffHelper : public CStillSourceHelper
{
public:
    CTiffHelper(CStillSource* pParent, eTIFFClass FormatSaving);
    BOOL OpenMediaFile(const std::string& FileName);
    void SaveSnapshot(const std::string& FilePath, int Height, int Width, BYTE* pOverlay, LONG OverlayPitch, const std::string& Context);

private:
    eTIFFClass  m_FormatSaving;
};

#endif