/////////////////////////////////////////////////////////////////////////////
// $Id: SettingValue.h,v 1.1 2004-08-06 17:12:10 atnak Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004 Atsushi Nakagawa.  All rights reserved.
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
//  This file is part of the SettingRepository module.  See
//  SettingRepository.h for more information.
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
//////////////////////////////////////////////////////////////////////////////

#ifndef __SETTINGVALUE_H__
#define __SETTINGVALUE_H__

#include <Windows.h>
#include <string>

#define SETTING_VALUE_USE_LAZYCOPY

enum eSettingValueType
{
	SETTING_VALUE_NULL,
	SETTING_VALUE_STRING,
	SETTING_VALUE_UINT,
	SETTING_VALUE_INT,
};

typedef class CSettingValue& RSETTINGVALUE;
typedef const class CSettingValue& RCSETTINGVALUE;


//////////////////////////////////////////////////////////////////////////
// CSettingValue
//////////////////////////////////////////////////////////////////////////
class CSettingValue
{
public:
	CSettingValue();
	CSettingValue(IN BYTE type);
	CSettingValue(IN const CSettingValue& src);
	virtual ~CSettingValue();

	// Sets or gets the type the instance is to become
	virtual void	SetType(IN BYTE type);
	virtual BYTE	GetType() const;

	// Setter and getter for working with type SETTING_VALUE_STRING
	virtual BOOL		SetString(IN std::string value);
	virtual std::string	GetString() const;
	// Setter and getter for working with type SETTING_VALUE_UINT
	virtual BOOL	SetUInt(IN UINT value);
	virtual UINT	GetUInt() const;
	// Setter and getter for working with type SETTING_VALUE_INT
	virtual BOOL	SetInt(IN INT value);
	virtual INT		GetInt() const;

	// Removes any existing values (sets type to SETTING_VALUE_NULL)
	virtual void	Reset();
	// Returns whether or not a value is set (type != SETTING_VALUE_NULL)
	virtual BOOL	IsSet() const;

	// Set the value from a string
	virtual void FromString(std::string from, BYTE type);
	// Converts the value to a string
	virtual std::string ToString() const;

	// Copies the passed value into itself
	virtual void	Copy(IN const CSettingValue& src);
	// Returns true if the value is semantically equal to itself
	virtual BOOL	IsEqual(IN const CSettingValue& value) const;

	// Assign operator override
	virtual CSettingValue& operator= (IN const CSettingValue& src);

protected:
	// Dev note: Lazy copy was initially implemented for SETTING_VALUE_STRING
	// and used to copy LPCSTR buffers but since std::string does lazy copy too,
	// just passing around a whole std::string objects should be more efficient
	// because that extends the lazy copy outside the bounds of this class.  As
	// such, the four functions below are not currently used.

	// Creates a buffer of size size taking into account extra field
	// requirements of lazy copy if it is enabled.
	virtual PVOID	CreateBuffer(IN size_t size);
	virtual PVOID	GetBuffer() const;

	// Performs a lazy copy of src into itself.  Lazy copy doesn't
	// duplicate any buffers in src and instead references the buffer.
	virtual void	LazyCopy(IN const CSettingValue& src);
	// Deletes a buffer referenced by LazyCopy.
	virtual BOOL	RefDelete();

private:
	BYTE	m_type;
	PVOID	m_value;
};


#endif

