//
// $Id: SoundChannel.h,v 1.2 2003-10-27 10:39:54 adcockj Exp $
//
/////////////////////////////////////////////////////////////////////////////
//
// copyleft 2003 itt@myself.com
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
//
// $Log: not supported by cvs2svn $
// Revision 1.1  2003/02/06 19:39:13  ittarnavsky
// contains definition of eSoundChannel
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file soundchannel.h soundchannel Header
 */

#if !defined(__SOUNDCHANNEL_H__)
#define __SOUNDCHANNEL_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum eSoundChannel
{
    SOUNDCHANNEL_MONO = 1,
    SOUNDCHANNEL_STEREO,
    SOUNDCHANNEL_LANGUAGE1,
    SOUNDCHANNEL_LANGUAGE2,
};

#endif // !defined(__SOUNDCHANNEL_H__)
