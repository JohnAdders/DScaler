/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004 Atsushi Nakagawa.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
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
/////////////////////////////////////////////////////////////////////////////

/**
 * @file BitVector.h Bit value and mask storage class implementation
 */

#ifndef __MASKBITS_H___
#define __MASKBITS_H___

#define _ltomask(length)                (~(~static_cast<unsigned long>(0) << (length)))

#define _M                                CBitMask
#define _B                                CBitVector


//////////////////////////////////////////////////////////////////////////
// CBitBlock
//////////////////////////////////////////////////////////////////////////

/**
 * This class describes a single block of bits in a larger 32-bit
 * bit vector.  (The block must be of contiguous bits.)
 */
class CBitBlock
{
public:
    // Creates a bit block with the given bit offset and mask.
    CBitBlock(unsigned long offset, unsigned long mask);
    virtual ~CBitBlock();

    // Gets the bit offset of the bit block.
    virtual unsigned long offset() const;
    // Gets the mask of the bit block.
    virtual unsigned long mask() const;

    // Returns a bit block that is a subset mask.
    virtual CBitBlock subset(unsigned long mask) const;
    // Returns a bit block that is an offset adjusted subset mask.
    virtual CBitBlock subset(unsigned long offset, unsigned long mask) const;

    // Returns the value shifted into a 32-bit space.
    virtual unsigned long pack(unsigned long value) const;
    // Returns the value of just the content of the block from the vector.
    virtual unsigned long unpack(unsigned long bitVector) const;

    // Returns the value, of the block described, shifted into a 32-bit space.
    static unsigned long pack(unsigned long offset, unsigned long mask, unsigned long value);
    // Returns the value of just the content of the block from the vector.
    static unsigned long unpack(unsigned long offset, unsigned long mask, unsigned long bitVector);

protected:
    unsigned long m_offset;
    unsigned long m_mask;
};


//////////////////////////////////////////////////////////////////////////
// CBitMask
//////////////////////////////////////////////////////////////////////////

/**
 * This class stores and provides operations for working with a simple
 * 32-bit bit mask value.
 */
class CBitMask
{
public:
    // Creates a mask with the given value.
    CBitMask(unsigned long mask);
    // Creates a mask with the value adjusted with the offset.
    CBitMask(unsigned long offset, unsigned long mask);
    // Creates a mask of the given bit block.
    CBitMask(const CBitBlock& bitBlock);
    virtual ~CBitMask();

    // Gets the mask value.
    virtual unsigned long mask() const;
    // Applies the mask to the given value and returns the result.
    virtual unsigned long mask(unsigned long value) const;

    // Returns an inverse mask object.
    virtual CBitMask operator~ () const;
    // Sets the left to the bitwise AND of two masks.
    virtual CBitMask& operator&= (const CBitMask& m);
    // Returns an object of the bitwise AND of two masks.
    virtual CBitMask operator& (const CBitMask& m) const;
    // Sets the left to the bitwise OR of two masks.
    virtual CBitMask& operator|= (const CBitMask& m);
    // Returns an object of the bitwise OR of two masks.
    virtual CBitMask operator| (const CBitMask& m) const;

protected:
    unsigned long    m_mask;
};


//////////////////////////////////////////////////////////////////////////
// CBitVector
//////////////////////////////////////////////////////////////////////////

class CBitVector
{
public:
    // Creates an empty vector.
    CBitVector();
    // Creates a full mask vector containing the value.
    CBitVector(unsigned long value);
    // Creates vector of the given mask containing the given value.
    CBitVector(unsigned long mask, unsigned long value);
    // Creates vector of the given mask containing the given value,
    // both adjusted with the offset.
    CBitVector(unsigned char offset, unsigned char mask, unsigned long value);
    // Creates a minimum vector required to store the value in the
    // given bit block.
    CBitVector(const CBitBlock& bitBlock, unsigned long value);
    virtual ~CBitVector();

    // Gets the straight bit vector value.
    virtual unsigned long value() const;
    // Gets a value adjusted by the offset and mask parameters.
    virtual unsigned long value(unsigned char offset, unsigned char mask) const;
    // Gets a value interpreted for the given bit block.
    virtual unsigned long value(const CBitBlock& bitBlock);
    // Gets the mask of the bit vector.
    virtual unsigned long mask() const;

    // No typecast operators because they obfuscate mistakes

    // Returns the vector where the value is bit inverted.
    virtual CBitVector operator~ () const;

    // Sets the left to a union vector with the values bitwise AND.
    virtual CBitVector& operator&= (const CBitVector& b);
    // Returns a unioned vector with the values bitwise AND.
    virtual CBitVector operator& (const CBitVector& b) const;
    // Sets the left to a union vector with the values bitwise OR.
    virtual CBitVector& operator|= (const CBitVector& b);
    // Returns a unioned vector with the values bitwise OR.
    virtual CBitVector operator| (const CBitVector& b) const;

    // Sets the left to a combination of the two vectors but with the
    // masked bits of the right wholly set by the value of the right.
    virtual CBitVector& operator+= (const CBitVector& b);
    // Returns the combination of the two vectors with the masked
    // bits of the right wholly set by the value of the right.
    virtual CBitVector operator+ (const CBitVector& b);

    // Sets the vector to a vector masked by the given mask.
    virtual CBitVector& operator&= (const CBitMask& m);
    // Masks the vector with the given mask.
    virtual CBitVector operator& (const CBitMask& m) const;

protected:
    unsigned long    m_mask;
    unsigned long    m_value;
};


#endif
