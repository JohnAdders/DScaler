/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009 John Adcock.  All rights reserved.
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
 * @file PathHelpers.h
 */

#ifndef __PATH_HELPERS_H___
#define __PATH_HELPERS_H___

// returns the path location of the DScaler exe
tstring GetInstallationPath();

// returns the path location for settings and program created files
tstring GetUserFilePath();

// returns the path location for users pictures
tstring GetUserPicturePath();

// returns the path location for users pictures
tstring GetUserVideoPath();

// returns the full path of the DScaler exe
tstring GetMainExeName();

// gets the extension from a file name
tstring GetExtension(const tstring& FileName);

// strips the path from a file name
tstring StripPath(const tstring& FileName);

// gets just the the path from a file name
tstring StripFile(const tstring& FileName);

// gets just the the path from a file name
tstring AddFileToPath(const tstring& Path, const tstring& FileName);

#endif
