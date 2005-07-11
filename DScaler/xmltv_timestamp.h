/**
 * Copyright (c) 2003 Billy Biggs <vektor@dumbterm.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __XMLTV_TIMESTAMP_H
#define __XMLTV_TIMESTAMP_H

#ifdef __cplusplus
extern "C" {
#endif

time_t parse_xmltv_date_v5( const char *date, unsigned int len );
time_t parse_xmltv_date_v6( const char *date, unsigned int len );

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif // __XMLTV_TIMESTAMP_H

