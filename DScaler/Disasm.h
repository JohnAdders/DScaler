/////////////////////////////////////////////////////////////////////////////
// $Id: Disasm.h,v 1.1 2001-07-24 12:19:00 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1998-2001 Avery Lee.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
/////////////////////////////////////////////////////////////////////////////
// This file was taken from VirtualDub
// VirtualDub - Video processing and capture application
// Copyright (C) 1998-2001 Avery Lee.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////

#ifndef f_DISASM_H
#define f_DISASM_H

#include <windows.h>

class CodeDisassemblyWindow {
private:
	void parse();
	char *penalty_string(long f);
	static BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

	void *code, *rbase, *abase;
	long length;
	void *pFault;

	class lbent {
	public:
		unsigned char *ip_u;
		unsigned char *ip_v;
		long flags;
	} *lbents;
	int num_ents;

	HFONT hFontMono;

	char buf[256];

public:
	CodeDisassemblyWindow(void *code, long, void *, void *);
	~CodeDisassemblyWindow();

	void DoInitListbox(HWND hwndList);
	BOOL DoMeasureItem(LPARAM lParam);
	BOOL DoDrawItem(LPARAM lParam);
	BOOL post(HWND);
	long getInstruction(char *buf, long val);

	void setFaultAddress(void *_pFault) {
		pFault = _pFault;
	}
};

#endif
