/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
//
// DScaler changes:
// * changed smart pointer class to support GDI handles
// This class supports anything that uses DeleteObject to delete
//
/////////////////////////////////////////////////////////////////////////////


/*
 * yasper - A non-intrusive reference counted pointer. 
 *	    Version: 1.04
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


#ifndef _SMART_HANDLE_H
#define _SMART_HANDLE_H

#include "SmartPtr.h"

struct EmptyHandleException : public std::exception
{
    EmptyHandleException() : std::exception("[SmartHandle Exception] Attempted to use empty handle") {}
};

template <typename X> 
class SmartHandle
{

public:
    typedef X element_type;

	/* 
		default constructor
			- don't create Counter
	*/
	SmartHandle() : rawHandle(0), counter(0) { }
	
	/*
		Construct from a raw handle 
	*/
	SmartHandle(X raw, Counter* c = 0) : rawHandle(0), counter(0)
	{
		if (raw)
		{
			rawHandle = raw; 
			if (c) acquire(c);
			else counter = new Counter;
		}
	}
	
	/*
		Copy constructor 
	*/
	SmartHandle(const SmartHandle< X >& otherPtr)
	{
		acquire( otherPtr.counter );
		rawHandle = otherPtr.rawHandle;
	}
	
	/* 
		Destructor 
	*/ 
	~SmartHandle()
	{
        release();
	}

/*
	Assignment to another SmartHandle 
*/

SmartHandle& operator=(const SmartHandle< X >& otherPtr)
{
	if (this != &otherPtr)
	{
		release();
		acquire(otherPtr.counter); 
		rawHandle = otherPtr.rawHandle; 
	}
	return *this; 
}

/*
	Assignment to raw pointers is really dangerous business.
	If the raw pointer is also being used elsewhere,
	we might prematurely delete it, causing much pain.
	Use sparingly/with caution.
*/

SmartHandle& operator=(X raw)
{

	if (raw)
	{
		release(); 
		counter = new Counter; 
		rawHandle = raw; 
	}
	return *this;
}




/*
	Conversion/casting operators
*/


	operator bool() const
	{
		return IsValid();
	}

	
	/*
	   implicit casts to base types of the
	   the pointer we're storing 
	*/
	
	operator X() const
	{
		return rawHandle;  
	}


/*
	Provide access to the raw handle
*/

	X GetRawHandle() const         
	{
		if (rawHandle == 0) throw new EmptyHandleException;
		return rawHandle;
	}

	
/* 
	Is there only one reference on the counter?
*/
	bool IsUnique() const
	{
		if (counter && counter->count == 1) return true; 
		return false; 
	}
	
	bool IsValid() const
	{
		if (counter && rawHandle) return true;
		return false; 
	}

	unsigned GetCount() const
	{
		if (counter) return counter->count;
		return 0;
	}

private:
	X rawHandle;

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

	// decrement the count, delete if it is 0
	void release()
	{ 
        if (counter) 
		{			
			(counter->count)--; 	

			if (counter->count == 0) 
			{
				delete counter;			
				::DeleteObject(rawHandle);
			}
		}
		counter = 0;
		rawHandle = 0; 

	}
};


template <typename X>
bool operator==(const SmartHandle< X >& lptr, const SmartHandle< X >& rptr) 
{
	return lptr.GetRawHandle() == rptr.GetRawHandle(); 
}

template <typename X>
bool operator==(const SmartHandle< X >& lptr, X raw) 
{
	return lptr.GetRawHandle() == raw ; 
}

template <typename X>
bool operator!=(const SmartHandle< X >& lptr, const SmartHandle< X >& rptr) 
{
	return ( !operator==(lptr, rptr) );
}

template <typename X>
bool operator!=(const SmartHandle< X >& lptr, X raw) 
{
	return ( !operator==(lptr, raw) );
}

template <typename X>
bool operator!(const SmartHandle< X >& p)
{
	return (!p.IsValid());
}


/* less than comparisons for storage in containers */
template <typename X>
bool operator< (const SmartHandle< X >& lptr, const SmartHandle < X >& rptr)
{
	return lptr.GetRawHandle() < rptr.GetRawHandle();
}

template <typename X>
bool operator< (const SmartHandle< X >& lptr, X raw)
{
	return lptr.GetRawHandle() < raw;
}

template <typename X>
bool operator< (X raw, const SmartHandle< X >& rptr)
{
	return raw < rptr.GetRawHandle();
}

#endif

