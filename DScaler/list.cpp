/////////////////////////////////////////////////////////////////////////////
// $Id: list.cpp,v 1.1 2002-09-17 17:31:47 tobbej Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1998-2001 Avery Lee.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
/////////////////////////////////////////////////////////////////////////////
// This file was taken from VirtualDub
// VirtualDub 2.x (Nina) - Video processing and capture application
// Copyright (C) 1998-2001 Avery Lee, All Rights Reserved.
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//
//	For those of you who say this looks familiar... it should.  This is
//	the same linked-list style that the Amiga Exec uses, with dummy head
//	and tail nodes.  It's really a very convienent way to implement
//	doubly-linked lists.
//

#include "stdafx.h"
#include "list.h"

List::List()
{
	Init();
}

void List::Init()
{
	head.next = tail.prev = 0;
	head.prev = &tail;
	tail.next = &head;
}

ListNode *List::RemoveHead()
{
	if (head.prev->prev)
	{
		ListNode *t = head.prev;

		head.prev->Remove();
		return t;
	}

	return 0;
}

ListNode *List::RemoveTail()
{
	if (tail.next->next)
	{
		ListNode *t = tail.next;

		tail.next->Remove();
		return t;
	}

	return 0;
}

void List::Take(List &from)
{
	if (from.IsEmpty())
		return;

	head.prev = from.head.prev;
	tail.next = from.tail.next;
	head.prev->next = &head;
	tail.next->prev = &tail;

	from.Init();
}
