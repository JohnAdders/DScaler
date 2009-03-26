/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
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

/**
 * @file slider.h slider Header file
 */

#ifndef __SLIDER_H___
#define __SLIDER_H___

// windowsx.h style macros for accessing slider controls

#define Slider_GetPos(hwndCtl)            ((int)(DWORD)SNDMSG((hwndCtl), TBM_GETPOS, 0L, 0L))
#define Slider_SetPos(hwndCtl, pos)     ((void)SNDMSG((hwndCtl), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)(LONG)(pos)))
#define Slider_SetRange(hwndCtl, min, max)     ((void)SNDMSG((hwndCtl), TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG((int)min, (int)max)))
#define Slider_SetRangeMax(hwndCtl, max)     ((void)SNDMSG((hwndCtl), TBM_SETRANGEMAX, (WPARAM)TRUE, (LPARAM)(LONG)(max)))
#define Slider_GetRangeMax(hwndCtl)            ((int)(DWORD)SNDMSG((hwndCtl), TBM_GETRANGEMAX, 0L, 0L))
#define Slider_SetRangeMin(hwndCtl, min)     ((void)SNDMSG((hwndCtl), TBM_SETRANGEMIN, (WPARAM)TRUE, (LPARAM)(LONG)(min)))
#define Slider_GetRangeMin(hwndCtl)            ((int)(DWORD)SNDMSG((hwndCtl), TBM_GETRANGEMIN, 0L, 0L))
#define Slider_SetLineSize(hwndCtl, linesize)     ((void)SNDMSG((hwndCtl), TBM_SETLINESIZE, 0L, (LPARAM)(LONG)(linesize)))
#define Slider_SetPageSize(hwndCtl, pagesize)     ((void)SNDMSG((hwndCtl), TBM_SETPAGESIZE, 0L, (LPARAM)(LONG)(pagesize)))
#define Slider_SetTicFreq(hwndCtl, tickfreq)     ((void)SNDMSG((hwndCtl), TBM_SETTICFREQ, (WPARAM)(tickfreq), (LPARAM)0L))
#define Slider_SetTic(hwndCtl, tickpos)     ((void)SNDMSG((hwndCtl), TBM_SETTIC, 0L, (LPARAM)(LONG)(tickpos)))
#define Slider_ClearTicks(hwndCtl, redraw)     ((void)SNDMSG((hwndCtl), TBM_CLEARTICS, (WPARAM)(BOOL)redraw, 0L))
#define Slider_Enable(hwndCtl, fEnable)     EnableWindow((hwndCtl), (fEnable))

#endif