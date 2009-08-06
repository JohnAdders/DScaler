/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
//
// DScaler changes:
// * changed static casts to dynamic to allow
//   more natural syntax with downcasting checks
// * Renamed from yasper::ptr to SmartPtr in line with coding style
// * Removed a load of function and made severla operaions more explicit
//   to avoid wherever possible problems
//
/////////////////////////////////////////////////////////////////////////////


/*
 * yasper - A non-intrusive reference counted pointer.
 *      Version: 1.04
 *
 *  Many ideas borrowed from Yonat Sharon and
 *  Andrei Alexandrescu.
 *
 * (zlib license)
 * ----------------------------------------------------------------------------------
 * Copyright (C) 2005-2007 Alex Rubinsteyn
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * -----------------------------------------------------------------------------------
 *
 * Send all questions, comments and bug reports to:
 * Alex Rubinsteyn (alex.rubinsteyn {at-nospam} gmail {dot} com)
 */


#ifndef _SMART_PTR_H
#define _SMART_PTR_H

#include <exception>

struct NullPointerException : public std::exception
{
    NullPointerException(): std::exception("[SmartPtr Exception] Attempted to use NULL pointer") {}
};

struct Counter
{
    Counter(unsigned c = 1) : count(c) {}
    unsigned count;
};

template <class X>
class SmartPtr
{

public:
    typedef X element_type;

    /*
        default constructor
            - don't create Counter
    */
    SmartPtr() : rawPtr(0), counter(0) { }
    
    /*
        Copy constructor
    */
   
    SmartPtr(const SmartPtr< X >& otherPtr)
    {
        acquire( otherPtr.counter );
        rawPtr = otherPtr.rawPtr;
    }

    /*
        Construct from a raw pointer
    */
	explicit SmartPtr(X* raw, Counter* c = 0) : rawPtr(0), counter(0)
    {
        if (raw)
        {
            rawPtr = raw;
            if (c) acquire(c);
            else counter = new Counter;
        }
    }
    
    /*
        Destructor
    */
    ~SmartPtr()
    {
        Clear();
    }


	SmartPtr& operator=(const SmartPtr< X >& otherPtr)
	{
		if (this != &otherPtr)
		{
			Clear();
			acquire(otherPtr.counter);
			rawPtr = otherPtr.rawPtr;
		}
		return *this;
	}

	BOOL operator< (const SmartPtr < X >& rhs)
	{
		return rawPtr < rhs.rawPtr;
	}

	BOOL operator==(const SmartPtr< X >& rhs)
	{
		return rawPtr == rhs.rawPtr;
	}

	BOOL operator==(X* rhs)
	{
		return rawPtr == rhs;
	}

	BOOL operator!=(const SmartPtr< X >& rhs)
	{
		return rawPtr != rhs.rawPtr;
	}

	BOOL operator!=(X* rhs)
	{
		return rawPtr != rhs;
	}

	/*
		Assignment to raw pointers is really dangerous business.
		If the raw pointer is also being used elsewhere,
		we might prematurely delete it, causing much pain.
		Use sparingly/with caution.
	*/
	SmartPtr& operator=(X* raw)
	{

		Clear();
		if (raw)
		{
			counter = new Counter;
			rawPtr = raw;
		}
		return *this;
	}

	/*
		Member Access
	*/
    X* operator->() const
    {
        return GetRawPointer();
    }


	/*
		Dereference the pointer
	*/
    const X& operator* () const
    {
        return *GetRawPointer();
    }


	/*
		Conversion/casting operators
	*/
    template <typename Y>
    SmartPtr<Y> DynamicCast()
    {
        //new SmartPtr must also take our counter or else the reference counts
        //will go out of sync
		Y* newPtr = dynamic_cast<Y*>(rawPtr);
		if(newPtr)
		{
			return SmartPtr<Y>(newPtr, counter);
		}
		else
		{
			return SmartPtr<Y>();
		}
    }

	/*
		Provide access to the raw pointer
	*/
    X* GetRawPointer() const
    {
        if (rawPtr == 0) throw new NullPointerException;
        return const_cast<X*>(rawPtr);
    }

    
	/*
		Is there only one reference on the counter?
	*/
    BOOL IsUnique() const
    {
        if (counter && counter->count == 1) return TRUE;
        return FALSE;
    }
    
    BOOL IsValid() const
    {
        if (counter && rawPtr) return TRUE;
        return FALSE;
    }

    unsigned GetCount() const
    {
        if (counter) return counter->count;
        return 0;
    }

    Counter* GetCounter() const
    {
        if (counter) return counter;
        return 0;
    }

	// decrement the count, delete if it is 0
    void Clear()
    {
        if (counter)
        {
            (counter->count)--;

            if (counter->count == 0)
            {
                delete counter;
                delete rawPtr;
            }
        }
        counter = 0;
        rawPtr = 0;
    }

private:
    X* rawPtr;

    Counter* counter;

    // increment the count
    void acquire(Counter* c)
    {
        counter = c;
        if (c)
        {
            (c->count)++;
        }
    }

};

#endif

