//
// $Id: IAudioControls.h,v 1.1 2001-12-05 21:45:11 ittarnavsky Exp $
//
/////////////////////////////////////////////////////////////////////////////
//
// copyleft 2001 itt@myself.com
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
/////////////////////////////////////////////////////////////////////////////

#if !defined(__IAUDIOCONTROLS_H__)
#define __IAUDIOCONTROLS_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class IAudioControls
{
public:
    virtual void SetMute(bool mute=true) = 0;
    virtual bool IsMuted() = 0;
    virtual void SetVolume(WORD volume) = 0;
    virtual WORD GetVolume() = 0;
    virtual void SetBalance(WORD balance) = 0;
    virtual WORD GetBalance() = 0;
    virtual void SetBass(WORD level) = 0;
    virtual WORD GetBass() = 0;
    virtual void SetTreble(WORD level) = 0;
    virtual WORD GetTreble() = 0;
};

#endif // !defined(__IAUDIOCONTROLS_H__)
