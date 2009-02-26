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
 * @file BitVector.cpp Bit value and mask storage class implementation
 */

#include "stdafx.h"
#include "BitVector.h"


//////////////////////////////////////////////////////////////////////////
// CBitBlock
//////////////////////////////////////////////////////////////////////////

CBitBlock::CBitBlock(unsigned long offset, unsigned long mask) :
    m_offset(offset), m_mask(mask)
{
}

CBitBlock::~CBitBlock()
{
}

unsigned long CBitBlock::offset() const
{
    return m_offset;
}

unsigned long CBitBlock::mask() const
{
    return m_mask;
}

CBitBlock CBitBlock::subset(unsigned long mask) const
{
    return CBitBlock(m_offset, m_mask & mask);
}

CBitBlock CBitBlock::subset(unsigned long offset, unsigned long mask) const
{
    return CBitBlock(m_offset + offset, (m_mask >> offset) & mask);
}

unsigned long CBitBlock::pack(unsigned long value) const
{
    return pack(m_offset, m_mask, value);
}

unsigned long CBitBlock::unpack(unsigned long bitVector) const
{
    return unpack(m_offset, m_mask, bitVector);
}

unsigned long CBitBlock::pack(unsigned long offset, unsigned long mask,
                              unsigned long value)
{
    return (value & mask) << offset;
}

unsigned long CBitBlock::unpack(unsigned long offset, unsigned long mask,
                                unsigned long bitVector)
{
    return (bitVector >> offset) & mask;
}


//////////////////////////////////////////////////////////////////////////
// CBitMask
//////////////////////////////////////////////////////////////////////////

CBitMask::CBitMask(unsigned long mask) :
    m_mask(mask)
{
}

CBitMask::CBitMask(unsigned long offset, unsigned long mask) :
    m_mask(mask << offset)
{
}

CBitMask::CBitMask(const CBitBlock& bitBlock) :
    m_mask(bitBlock.mask() << bitBlock.offset())
{
}

CBitMask::~CBitMask()
{
}

unsigned long CBitMask::mask() const
{
    return m_mask;
}

unsigned long CBitMask::mask(unsigned long value) const
{
    return value & m_mask;
}

CBitMask CBitMask::operator~ () const
{
    return CBitMask(~m_mask);
}

CBitMask& CBitMask::operator&= (const CBitMask& m)
{
    m_mask &= m.m_mask;
    return *this;
}

CBitMask CBitMask::operator& (const CBitMask& m) const
{
    return CBitMask(m_mask & m.m_mask);
}

CBitMask& CBitMask::operator|= (const CBitMask& m)
{
    m_mask |= m.m_mask;
    return *this;
}

CBitMask CBitMask::operator| (const CBitMask& m) const
{
    return CBitMask(m_mask | m.m_mask);
}


//////////////////////////////////////////////////////////////////////////
// CBitVector
//////////////////////////////////////////////////////////////////////////

CBitVector::CBitVector() :
    m_mask(0), m_value(0)
{
}

CBitVector::CBitVector(unsigned long value) :
    m_mask(~static_cast<unsigned long>(0)), m_value(value)
{
}

CBitVector::CBitVector(unsigned long mask, unsigned long value) :
    m_mask(mask), m_value(value & mask)
{
}

CBitVector::CBitVector(unsigned char offset, unsigned char mask, unsigned long value) :
    m_mask(mask << offset), m_value(CBitBlock::pack(offset, mask, value))
{
}

CBitVector::CBitVector(const CBitBlock& bitBlock, unsigned long value) :
    m_mask(bitBlock.mask() << bitBlock.offset()), m_value(bitBlock.pack(value))
{
}

CBitVector::~CBitVector()
{
}

unsigned long CBitVector::value() const
{
    return m_value;
}

unsigned long CBitVector::value(unsigned char offset, unsigned char mask) const
{
    return CBitBlock::unpack(offset, mask, m_value);
}

unsigned long CBitVector::value(const CBitBlock& bitBlock)
{
    return bitBlock.unpack(m_value);
}

unsigned long CBitVector::mask() const
{
    return m_mask;
}

CBitVector CBitVector::operator~ () const
{
    return CBitVector(m_mask, ~m_value);
}

CBitVector& CBitVector::operator&= (const CBitVector& b)
{
    m_mask |= b.m_mask;
    m_value &= b.m_value;
    return *this;
}

CBitVector CBitVector::operator& (const CBitVector& b) const
{
    return CBitVector(m_mask | b.m_mask, m_value & b.m_value);
}

CBitVector& CBitVector::operator|= (const CBitVector& b)
{
    m_mask |= b.m_mask;
    m_value |= b.m_value;
    return *this;
}

CBitVector CBitVector::operator| (const CBitVector& b) const
{
    return CBitVector(m_mask | b.m_mask, m_value | b.m_value);
}

CBitVector& CBitVector::operator+= (const CBitVector& b)
{
    m_mask |= b.m_mask;
    m_value = m_value & ~b.m_mask | b.m_value;
    return *this;
}

CBitVector CBitVector::operator+ (const CBitVector& b)
{
    return CBitVector(m_mask | b.m_mask, m_value & ~b.m_mask | b.m_value);
}

CBitVector& CBitVector::operator&= (const CBitMask& m)
{
    m_mask &= m.mask();
    m_value &= m_mask;
    return *this;
}

CBitVector CBitVector::operator& (const CBitMask& m) const
{
    return CBitVector(m_mask & m.mask(), m_value);
}


/*
CBitVector CPCICard::ReadData(DWORD registerOffset, DWORD registerMask)
{
    unsigned long value = 0;

    if (registerMask & 0xFFFF0000)
    {
        value = static_cast<unsigned long>(ReadDword(registerOffset));
    }
    else if (registerMask & 0xFF00)
    {
        value = static_cast<unsigned long>(ReadWord(registerOffset));
    }
    else if (registerMask & 0xFF)
    {
        value = static_cast<unsigned long>(ReadByte(registerOffset));
    }
    return CBitVector(registerMask, value);
}

void CPCICard::WriteData(DWORD registerOffset, DWORD registerMask, DWORD value)
{
    WriteData(registerOffset, registerMask, CBitVector(registerMask, value));
}

void CPCICard::WriteData(DWORD registerOffset, DWORD registerMask, const CBitVector& b)
{
    unsigned long mask = b.mask() & registerMask;
    unsigned long value = b.value();

    if (registerMask & 0xFFFF0000)
    {
        if (mask == 0xFFFFFFFF)
        {
            WriteDword(registerOffset, static_cast<BYTE>(value));
        }
        else
        {
            MaskDataDword(registerOffset, static_cast<BYTE>(value), static_cast<BYTE>(mask));
        }
    }
    else if (registerMask & 0xFF00)
    {
        if (mask == 0xFFFF)
        {
            WriteWord(registerOffset, static_cast<BYTE>(value));
        }
        else
        {
            MaskDataWord(registerOffset, static_cast<BYTE>(value), static_cast<BYTE>(mask));
        }
    }
    else if (registerMask & 0xFF)
    {
        if (mask == 0xFF)
        {
            WriteByte(registerOffset, static_cast<BYTE>(value));
        }
        else
        {
            MaskDataByte(registerOffset, static_cast<BYTE>(value), static_cast<BYTE>(mask));
        }
    }
}

*/