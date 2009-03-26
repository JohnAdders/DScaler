///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009 John Adcock
///////////////////////////////////////////////////////////////////////////////
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
///////////////////////////////////////////////////////////////////////////////

#ifndef _COM_INITISLISE_H
#define _COM_INITISLISE_H


/// Use RAII to ensure that COM is correctly set up in a thread.
/// To use simple declare a variable of this type at the top of the thread
class ComInitialise
{
public:
    ComInitialise(DWORD Flags = COINIT_MULTITHREADED)
    {
        CoInitializeEx(NULL, Flags);
    }

    ~ComInitialise()
    {
        CoUninitialize();
    }
};

#endif