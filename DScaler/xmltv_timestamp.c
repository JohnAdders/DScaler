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

/*
** Note: optimized, added DTD-0.6 support and error detection [TZO]
*/

#define DEBUG_SWITCH DEBUG_SWITCH_XMLTV
#define DPRINTF_OFF

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "xmltv_types.h"

#include "xmltv_timestamp.h"

#define DATE_OFF_YEAR    0
#define DATE_OFF_MON     4
#define DATE_OFF_DAY     6
#define DATE_OFF_HOUR    8
#define DATE_OFF_MIN     10
#define DATE_OFF_SEC     12

#define ATOI_2(S)  (((S)[0]-'0')*10 + ((S)[1]-'0'))
#define ATOI_4(S)  (ATOI_2(S)*100 + ATOI_2(S + 2))


// when cross-compiling for WIN32 on Linux "timezone" is undefined
#if !defined(__NetBSD__) && !defined(__FreeBSD__)
# if defined(WIN32) && !defined(timezone)
#  define timezone _timezone
# endif
#endif


/**
 * Support the Date::Manip timezone names.  This code will hopefully
 * go away when all XMLTV providers drop these names.  Using names
 * is a bad idea since there is no unified standard for them, and the
 * XMLTV DTD does not define a set of standard names to use.
 */
typedef struct tz_map_s {
    const char *name;
    int offset;
} tz_map_t;

static const tz_map_t date_manip_timezones[] = {
    { "IDLW",    -1200, }, { "NT",      -1100, }, { "HST",     -1000, },
    { "CAT",     -1000, }, { "AHST",    -1000, }, { "AKST",     -900, },
    { "YST",      -900, }, { "HDT",      -900, }, { "AKDT",     -800, },
    { "YDT",      -800, }, { "PST",      -800, }, { "PDT",      -700, },
    { "MST",      -700, }, { "MDT",      -600, }, { "CST",      -600, },
    { "CDT",      -500, }, { "EST",      -500, }, { "ACT",      -500, },
    { "SAT",      -400, }, { "BOT",      -400, }, { "EDT",      -400, },
    { "AST",      -400, }, { "AMT",      -400, }, { "ACST",     -400, },
    { "NFT",      -330, }, { "BRST",     -300, }, { "BRT",      -300, },
    { "AMST",     -300, }, { "ADT",      -300, }, { "ART",      -300, },
    { "NDT",      -230, }, { "AT",       -200, }, { "BRST",     -200, },
    { "FNT",      -200, }, { "WAT",      -100, }, { "FNST",     -100, },
    { "GMT",         0, }, { "UT",          0, }, { "UTC",         0, },
    { "WET",         0, }, { "CET",       100, }, { "FWT",       100, },
    { "MET",       100, }, { "MEZ",       100, }, { "MEWT",      100, },
    { "SWT",       100, }, { "BST",       100, }, { "GB",        100, },
    { "WEST",        0, }, { "CEST",      200, }, { "EET",       200, },
    { "FST",       200, }, { "MEST",      200, }, { "MESZ",      200, },
    { "METDST",    200, }, { "SAST",      200, }, { "SST",       200, },
    { "EEST",      300, }, { "BT",        300, }, { "MSK",       300, },
    { "EAT",       300, }, { "IT",        330, }, { "ZP4",       400, },
    { "MSD",       300, }, { "ZP5",       500, }, { "IST",       530, },
    { "ZP6",       600, }, { "NOVST",     600, }, { "NST",       630, },
    { "JAVT",      700, }, { "CCT",       800, }, { "AWST",      800, },
    { "WST",       800, }, { "PHT",       800, }, { "JST",       900, },
    { "ROK",       900, }, { "ACST",      930, }, { "CAST",      930, },
    { "AEST",     1000, }, { "EAST",     1000, }, { "GST",      1000, },
    { "ACDT",     1030, }, { "CADT",     1030, }, { "AEDT",     1100, },
    { "EADT",     1100, }, { "IDLE",     1200, }, { "NZST",     1200, },
    { "NZT",      1200, }, { "NZDT",     1300, } };

#define num_timezones (sizeof( date_manip_timezones ) / sizeof( tz_map_t ))

/**
 * Timezone parsing code based loosely on the algorithm in
 * filldata.cpp of MythTV.
 */
static time_t parse_xmltv_timezone( const char *tzstr )
{
    time_t result = 0;

    if( strlen( tzstr ) == 5 && (tzstr[ 0 ] == '+' || tzstr[ 0 ] == '-') ) {

        result = (3600 * ATOI_2(tzstr + 1)) + (60 * ATOI_2(tzstr + 3));

        if (tzstr[ 0 ] == '-')
            result = - result;
    } else {
        int i;

        for( i = 0; i < num_timezones; i++ ) {
            if( !STRCASECMP( tzstr, date_manip_timezones[ i ].name ) ) {
                result = 60 * ((date_manip_timezones[ i ].offset % 100) +
                              ((date_manip_timezones[ i ].offset / 100) * 60));
                break;
            }
        }
    }

    return result;
}

time_t parse_xmltv_date_v5( const char *date )
{
    const char * p;
    struct tm tm_obj;
    time_t tz;
    int len;
    int success = 1;
    time_t result = 0;

    /*
     * For some reason, mktime() accepts broken-time arguments as localtime,
     * and there is no corresponding UTC function. *Sigh*.
     * For this reason we have to calculate the offset from GMT to adjust the
     * argument given to mktime().
     */
    time_t now = time( 0 );
#ifndef WIN32
    long gmtoff = localtime( &now )->tm_gmtoff;
#else
    long gmtoff;
    struct tm * pTm = localtime( &now );
    gmtoff = 60*60 * pTm->tm_isdst - timezone;
#endif

    /*
     * according to the xmltv dtd:
     *
     * All dates and times in this [the xmltv] DTD follow the same format,
     * loosely based on ISO 8601.  They can be 'YYYYMMDDhhmmss' or some
     * initial substring, for example if you only know the year and month you
     * can have 'YYYYMM'.  You can also append a timezone to the end; if no
     * explicit timezone is given, UT is assumed.  Examples:
     * '200007281733 BST', '200209', '19880523083000 +0300'.  (BST == +0100.)
     *
     * thus:
     * example *date = "20031022220000 +0200"
     * type:            YYYYMMDDhhmmss ZZzzz"
     * position:        0         1         2          
     *                  012345678901234567890
     *
     * note: since part of the time specification can be omitted, we cannot
     *       hard-code the offset to the timezone!
     */

    /* Find where the timezone starts */
    p = date;
    while ((*p >= '0') && (*p <= '9'))
        p++;
    /* Calculate the length of the date */
    len = p - date;

    if (*p == ' ')
    {
        /* Parse the timezone, skipping the initial space */
        tz = parse_xmltv_timezone( p + 1 );
    } else if (*p == 0) {
        /* Assume UT */
        tz = 0;
    } else {
        /* syntax error */
        tz = 0;
        success = 0;
    }

    if (success)
    {
        if (len >= DATE_OFF_SEC + 2)
        {
            tm_obj.tm_sec = ATOI_2(date + DATE_OFF_SEC);
        }
        else
            tm_obj.tm_sec = 0;

        if (len >= DATE_OFF_MIN + 2)
        {
            tm_obj.tm_min = ATOI_2(date + DATE_OFF_MIN);
        }
        else
            tm_obj.tm_min = 0;

        if (len >= DATE_OFF_HOUR + 2)
        {
            tm_obj.tm_hour = ATOI_2(date + DATE_OFF_HOUR);
        }
        else
            tm_obj.tm_hour = 0;

        if (len >= DATE_OFF_DAY + 2)
        {
            tm_obj.tm_mday = ATOI_2(date + DATE_OFF_DAY);
            tm_obj.tm_mon = ATOI_2(date + DATE_OFF_MON);
            tm_obj.tm_year = ATOI_4(date + DATE_OFF_YEAR);

            tm_obj.tm_sec = tm_obj.tm_sec - tz + gmtoff;
            tm_obj.tm_mon -= 1;
            tm_obj.tm_year -= 1900;
            tm_obj.tm_isdst = -1;

            result = mktime( &tm_obj );
        }
    }
    return result;
}

// ----------------------------------------------------------------------------

time_t parse_xmltv_date_v6( const char *date )
{
   struct tm t;
   int nscan;
   int scan_pos;
   time_t tval;

   nscan = sscanf(date, "%4u-%2u-%2uT%2u:%2u:%2uZ%n",
                        &t.tm_year, &t.tm_mon, &t.tm_mday,
                        &t.tm_hour, &t.tm_min, &t.tm_sec, &scan_pos);
   if ((nscan >= 6) && (date[scan_pos] == 0))
   {
      t.tm_year -= 1900;
      t.tm_mon -= 1;
      t.tm_isdst = -1;
      t.tm_sec = 0;

      tval = mktime(&t);
      tval += 60*60 * t.tm_isdst - timezone;
   }
   else
      tval = 0;

   return tval;
}

