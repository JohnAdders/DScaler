/////////////////////////////////////////////////////////////////////////////
// $Id: PtrList.h,v 1.2 2003-06-06 11:14:40 atnak Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 Atsushi Nakagawa.  All rights reserved.
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
// 05 Jun 2003   Atsushi Nakagawa      Initial Release
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.1  2003/06/05 08:38:09  atnak
// Initial release of PtrList template class.
//
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __PTRLIST_H___
#define __PTRLIST_H___

// Disable debug information truncated to '255 warning
#pragma warning(disable:4786)

#include <list>
#include <deque>
using namespace std;


//
// Description:
//
//  PtrList is a template class that inherits std::list and implements
//  a pointer list of type TPtr.  The class takes care of deleting the
//  objects pointed to by the pointer.  PtrList also provides an easy
//  to use iterating methodology.
//
// Declaration:
//
//    PtrList<MyClass*>  myClassList;
//
// Comments:
//
//  Note that the template is instantiated with a class pointer instead
//  of with a class name.  In this case with <MyClass*> instead of just
//  <MyClass>.
//
//  Also note that objects pushed and inserted into the list are to be
//  considered relinquished of ownership.  PtrList will take care of
//  deleting the object.  The release() function can be used to detach
//  an object from the list to regain ownership of it.
//


template <class TPtr>
class PtrList : protected list<TPtr>
{
public:
    PtrList() : list<TPtr>()
    {
        m_iterator = end();
    };

    virtual ~PtrList()
    {
        for (iterator it = begin(); it != end(); it++)
            delete *it;
    }

    //  Basic functions:
    //
    //  push_back(TPtr ptr);            - Push ptr to back of list*
    //  push_front(TPtr ptr);           - Push ptr to front of list*
    //  insert(TPtr ptr);               - Inserts before item at cursor*#
    //
    //  void clear();                   - Empty the list
    //  size_type size();               - Get items count of the list
    //
    //  * Caller relinquishes ownership of ptr.  Object must be
    //    delete'able.
    //
    //  # See "Iterating functions".
    //

    void push_back(TPtr ptr) { list<TPtr>::push_back(ptr); };
    void push_front(TPtr ptr) { list<TPtr>::push_front(ptr); };
    void insert(TPtr ptr) { list<TPtr>::insert(m_iterator, ptr); };

    void clear()
    {
        for (iterator it = begin(); it != end(); it++)
            delete *it;
        list<TPtr>::clear();

        m_iterator = end();
        istack_end_all();
    }

    size_type size() { return list<TPtr>::size(); }

    //
    //  Iterating functions:
    //
    //  TPtr first();                   - Move cursor to first item
    //  TPtr last();                    - Move cursor to last item
    //
    //  TPtr next();                    - Move the cursor to the next item
    //  TPtr next(const TPtr ptr);      - Move to next item after ptr
    //  TPtr prev();                    - Move the cursor to the previous item
    //  TPtr prev(const TPtr ptr);      - Move to previous item before ptr
    //
    //  TPtr current();                 - Return item at cursor
    //  TPtr current(const TPtr ptr);   - Move cursor to ptr
    //
    //  TPtr remove();                  - Remove item at cursor
    //  TPtr remove(const TPtr ptr);    - Remove ptr
    //
    //  TPtr release();                 - Release item at cursor
    //  TPtr release(const TPtr ptr);   - Release ptr
    //

    //
    //  Iterator stack functions:
    //  
    //  TPtr push_pos();                - Save the current iterator
    //  TPtr pop_pos();                 - Restore the previous iterator
    //


    TPtr first()
    {
        m_iterator = begin();
        return current();
    }


    TPtr last()
    {
        m_iterator = end();
        m_iterator--;
        return current();
    }


    TPtr next()
    {
        if (m_iterator != end())
        {
            m_iterator++;
        }
        return current();
    }


    TPtr next(const TPtr ptr)
    {
        return (current(ptr) ? next() : NULL);
    }


    TPtr prev()
    {
        if (m_iterator != end())
        {
            m_iterator--;
        }
        return current();
    }


    TPtr prev(const TPtr ptr)
    {
        return (current(ptr) ? prev() : NULL);
    }


    TPtr current()
    {
        return m_iterator == end() ? NULL : *m_iterator;
    }


    TPtr current(const TPtr ptr)
    {
        for (m_iterator = begin(); m_iterator != end(); m_iterator++)
            if (*m_iterator == ptr)
                return *m_iterator;
        return NULL;
    }


    TPtr remove()
    {
        if (m_iterator != end())
        {
            delete *m_iterator;
            istack_shift_current();
            m_iterator = erase(m_iterator);
        }
        return current();
    }


    TPtr remove(const TPtr ptr)
    {
        return (current(ptr) ? remove() : NULL);
    }


    TPtr release()
    {
        if (m_iterator != end())
        {
            TPtr ptr = *m_iterator;
            istack_shift_current();
            m_iterator = erase(m_iterator);
            return ptr;
        }
        return NULL;
    }


    TPtr release(const TPtr ptr)
    {
        return (current(ptr) ? release() : NULL);
    }


    TPtr push_pos()
    {
        m_istack.push_front(m_iterator);
        return current();
    }


    TPtr pop_pos()
    {
        if (!m_istack.empty())
        {
            m_iterator = m_istack.front();
            m_istack.pop_front();
        }
        return current();
    }

private:
    void istack_shift_current()
    {
        deque<iterator>::iterator it = m_istack.begin();
        for ( ; it != m_istack.end(); it++)
        {
            if (*it == m_iterator)
            {
                (*it)++;
            }
        }
    }

    void istack_end_all()
    {
        deque<iterator>::iterator it = m_istack.begin();
        for ( ; it != m_istack.end(); it++)
        {
            *it = end();
        }
    }

private:
    iterator            m_iterator;
    deque<iterator>     m_istack;
};


#endif

