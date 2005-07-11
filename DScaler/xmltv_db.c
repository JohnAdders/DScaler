/*
 *  XMLTV content processing
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License Version 2 as
 *  published by the Free Software Foundation. You find a copy of this
 *  license in the file COPYRIGHT in the root directory of this release.
 *
 *  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
 *  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
 *  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *
 *  Description:
 *
 *    This module contains the main entry point for parsing XMLTV files.
 *    The control flow is directly forwarded to the parser and scanner
 *    which in return invoke callback functions in this module for all
 *    recognized tokens, i.e. start (open) and close element tags,
 *    end of opening tag, attribute assignments and content data.
 *
 *    General explanation of callbacks:
 *
 *    Not all tags have the full set of callback functions, i.e. only those
 *    which are required to process the data are implemented. Which ones
 *    are required depends on the kind and organization of the data
 *    carried by the respective tag.  It also depends on the implementation
 *    of the callbacks below.  The callback functions are registered in
 *    the tag parser table in xmltv_tags.c; for unneeded callbacks a
 *    NULL pointer is registered.
 *
 *    The open function carries no data and just signals the start of
 *    processing of the respective tag.  It can be used to initialize the
 *    state machine which will process the data. Likewise, the close
 *    function just signals the end of processing for the tag, i.e. no
 *    more attributes or content will follow. At this point any temporarily
 *    stored data can be post-processed (if necessary) and committed to
 *    the database.
 *
 *    The attribute "set" functions are called in the order in which
 *    attribute assignments are found inside the tag in the XML file.
 *    (Note most of the attributes are declared optional in the XMLTV DTD,
 *    so the callbacks are not invoked for each tag; also the order of
 *    attributes inside a tag is undefined, i.e. depends on the generator
 *    of the XMLTV file; however an attribute may be defined at most once.)
 *
 *    The content data "add" function is called after all attributes have
 *    been processed. The parser already strips leading and trailing
 *    whitespace (for tags for which it's appropriate; in case of XMLTV
 *    that's all tags; trimming is enabled by a flag in the tables in
 *    xmltv_tags.c)  Also all escaped characters (&lt; for "<" etc.) are
 *    replaced.  Theoretically the data function could be called more than
 *    once if the content is interrupted by "child" tags; however the
 *    XMLTV DTD does not include such so-called mixed elements, so this
 *    will not occur.
 *
 *    The "attributes complete" callback is called when the opening tag
 *    is closed, i.e. inbetween processing attributes and content.  The
 *    callback function can return FALSE to indicate that the content and
 *    all child elements shuld be discarded at parser level. This feature
 *    can be used to skip over unwanted data; it's not used here though.
 *
 *    Note the same callbacks are used both for DTD version 0.5 and 0.6
 *    whenever applicable. In many cases information which was carried as
 *    attribute in DTD 0.5 is carries as content in 0.6 and vice versa.
 *    This explains some of the oddities below.
 *
 *    Building the programme database:
 *
 *    The callback functions fill temporary structures with programme
 *    parameters and data (or timeslot data respectively for XMLTV DTD 0.6)  
 *    The structures are forwarded to a database when the tag is closed.
 *    Also a channel table is generated and forwarded to the database at
 *    the end of the file.
 *
 *  Author: Tom Zoerner
 *          L. Garnier for interfacing with DScaler
 *
 *  $Id: xmltv_db.c,v 1.3 2005-07-11 16:00:52 laurentg Exp $
 */

#define DEBUG_SWITCH DEBUG_SWITCH_XMLTV
#define DPRINTF_OFF

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "xmltv_types.h"
#include "xmltv_debug.h"

#include "xml_cdata.h"
#include "xml_hash.h"
#include "xmltv_timestamp.h"
#include "xmltv_db.h"
#include "xmltv_tags.h"


extern int CheckProgrammeValidity(time_t StartTime, time_t EndTime, char * ChannelName);
extern void AddProgramme(time_t StartTime, time_t EndTime, char * Title, char * ChannelName, char * SubTitle, char * Category, char * Description);


//#define ALL_CONTENT


typedef struct
{
   char       * p_disp_name;
   time_t       pi_min_time;
   time_t       pi_max_time;
} XMLTV_CHN;

#ifdef ALL_CONTENT
typedef enum
{
   XMLTV_CAT_SYS_NONE,
   XMLTV_CAT_SYS_PDC,
   XMLTV_CAT_SYS_SORTCRIT
} XMLTV_CAT_SYS;

typedef enum
{
   XMLTV_CODE_NONE,
   XMLTV_CODE_VPS,
   XMLTV_CODE_PDC,
   XMLTV_CODE_SV,
   XMLTV_CODE_VP
} XMLTV_CODE_TIME_SYS;

typedef enum
{
   XMLTV_RATING_NONE,
   XMLTV_RATING_AGE,
   XMLTV_RATING_FSK,
   XMLTV_RATING_BBFC,
   XMLTV_RATING_MPAA
} XMLTV_RATING_SYS;

#define XMLTV_PI_CAT_MAX  40   // 3 languages
#endif

typedef struct
{
   uint         chn_count;
   uint         chn_tab_size;
   bool         chn_open;
   char       * p_chn_id_tmp;
   XMLTV_CHN  * p_chn_table;
   XML_HASH_PTR pChannelHash;

#ifdef ALL_CONTENT
   XML_STR_BUF  source_info_name;
   XML_STR_BUF  source_info_url;
   XML_STR_BUF  source_data_url;
   XML_STR_BUF  gen_info_name;
   XML_STR_BUF  gen_info_url;
   XML_STR_BUF  * link_text_dest;
   XML_STR_BUF  * link_href_dest;
#endif

   time_t       pi_start_time;
   time_t       pi_stop_time;
   uint         pi_netwop_no;
#ifdef ALL_CONTENT
   uint         pi_pil;
   ulong        pi_feature_flags;
#endif

   XML_STR_BUF  pi_title;
   XML_STR_BUF  pi_subtitle;
   XML_STR_BUF  pi_category;
   XML_STR_BUF  pi_desc;
#ifdef ALL_CONTENT
   XML_STR_BUF  pi_credits;
   XML_STR_BUF  pi_actors;

   uint         audio_cnt;
   uint         pi_aspect_x;
   uint         pi_aspect_y;
   uint         pi_star_rating_val;
   uint         pi_star_rating_max;
   XMLTV_CAT_SYS pi_cat_system;
   uint         pi_cat_code;
   uchar        pi_cat_count;
   uchar        pi_cats[XMLTV_PI_CAT_MAX];
   XMLTV_RATING_SYS pi_rating_sys;
   XMLTV_CODE_TIME_SYS pi_code_time_sys;
   XML_STR_BUF  pi_code_time_str;
   XML_STR_BUF  pi_code_sv;
   XML_STR_BUF  pi_code_vp;
#endif
   XMLTV_DTD_VERSION dtd;
   bool         isPeek;

} PARSE_STATE;

static PARSE_STATE xds;

#define PI_FEATURE_SUBTITLES       0x100
#define PI_FEATURE_REPEAT          0x080
#define PI_FEATURE_LIVE            0x040
#define PI_FEATURE_ENCRYPTED       0x020
#define PI_FEATURE_DIGITAL         0x010
#define PI_FEATURE_PAL_PLUS        0x008
#define PI_FEATURE_FMT_WIDE        0x004
#define PI_FEATURE_SOUND_MASK      0x003
#define PI_FEATURE_SOUND_MONO      0x000
#define PI_FEATURE_SOUND_2CHAN     0x001
#define PI_FEATURE_SOUND_STEREO    0x002
#define PI_FEATURE_SOUND_SURROUND  0x003

// ----------------------------------------------------------------------------
// Parse timestamp
//
static time_t Xmltv_ParseTimestamp( char * pStr, uint len )
{
   time_t  tval;

   if (xds.dtd == XMLTV_DTD_6)
   {
      tval = parse_xmltv_date_v6(pStr, len);
   }
   else
   {
      tval = parse_xmltv_date_v5(pStr, len);
   }

   ifdebug1(tval == 0, "Xmltv-ParseTimestamp: parse error '%s'", pStr);

   return tval;
}

#ifdef ALL_CONTENT
// ----------------------------------------------------------------------------
// Parse VPS/PDC timestamp
// - the value is in local time
// - the difference to regular timestamps is the returned format
//
static uint Xmltv_ParseVpsPdc( const char * pStr )
{
   uint  month, mday, hour, minute;
   uint  pil;
   int nscan;
   int scan_pos;

   pil = 0;
   if (xds.dtd == XMLTV_DTD_6)
   {
      scan_pos = 0;
      // note: trailing 'Z' is omitted since VPS/PDC is localtime
      nscan = sscanf(pStr, "%*4u-%2u-%2uT%2u:%2u:%*2u%n", &month, &mday, &hour, &minute, &scan_pos);
      if ((nscan >= 4 /*6?*/) && (pStr[scan_pos] == 0))
      {
         pil = (mday << 15) | (month << 11) | (hour << 6) | minute;
      }
      else
         debug3("Xmltv-ParseVpsPdc: parse error '%s' after %d tokens (around char #%d)", pStr, nscan, scan_pos);
   }
   else
   {
      scan_pos = 0;
      // note: ignoring timezone since VPS/PDC should always be localtime
      nscan = sscanf(pStr, "%*4u%2u%2u%2u%2u%*2u%n", &month, &mday, &hour, &minute, &scan_pos);
      if ((nscan >= 4 /*6?*/) && ((pStr[scan_pos] == 0) || (pStr[scan_pos] == ' ')))
      {
         pil = (mday << 15) | (month << 11) | (hour << 6) | minute;
      }
      else
         debug3("Xmltv-ParseVpsPdc: parse error '%s' after %d tokens (around char #%d)", pStr, nscan, scan_pos);
   }
   return pil;
}
#endif

// ----------------------------------------------------------------------------

// DTD 0.5 only (DTD 0.6 uses <link>)
void Xmltv_AboutSetSourceInfoUrl( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   XmlCdata_AssignOrAppend(&xds.source_info_name, pBuf);
#endif
}

// DTD 0.5 only (DTD 0.6 uses <link>)
void Xmltv_AboutSetSourceInfoName( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   XmlCdata_AssignOrAppend(&xds.source_info_url, pBuf);
#endif
}

void Xmltv_AboutSetSourceDataUrl( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   XmlCdata_AssignOrAppend(&xds.source_data_url, pBuf);
#endif
}

void Xmltv_SourceInfoOpen( void )
{
#ifdef ALL_CONTENT
   xds.link_text_dest = &xds.source_info_name;
   xds.link_href_dest = &xds.source_info_url;
#endif
}

void Xmltv_SourceInfoClose( void )
{
#ifdef ALL_CONTENT
   xds.link_text_dest = NULL;
   xds.link_href_dest = NULL;
#endif
}

void Xmltv_GenInfoOpen( void )
{
#ifdef ALL_CONTENT
   xds.link_text_dest = &xds.gen_info_name;
   xds.link_href_dest = &xds.gen_info_url;
#endif
}

void Xmltv_GenInfoClose( void )
{
#ifdef ALL_CONTENT
   xds.link_text_dest = NULL;
   xds.link_href_dest = NULL;
#endif
}

void Xmltv_AboutSetGenInfoName( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   XmlCdata_AssignOrAppend(&xds.gen_info_name, pBuf);
#endif
}

void Xmltv_AboutSetGenInfoUrl( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   XmlCdata_AssignOrAppend(&xds.gen_info_url, pBuf);
#endif
}

void Xmltv_LinkAddText( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   if (xds.link_text_dest != NULL)
   {
      XmlCdata_AssignOrAppend(xds.link_text_dest, pBuf);
   }
#endif
}

void Xmltv_LinkHrefSet( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   if (xds.link_href_dest != NULL)
   {
      XmlCdata_AssignOrAppend(xds.link_href_dest, pBuf);
   }
#endif
}

void Xmltv_LinkBlurbAddText( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   if (xds.link_text_dest != NULL)
   {
      XmlCdata_AppendParagraph(xds.link_text_dest, TRUE);
      XmlCdata_AssignOrAppend(xds.link_text_dest, pBuf);
   }
#endif
}

// ----------------------------------------------------------------------------
// Add a new channel
//
void Xmltv_ChannelCreate( void )
{ 
   void * pTmp;
   uint size;

   assert(xds.chn_open == FALSE);

   if ((xds.p_chn_table == NULL) || (xds.chn_tab_size < xds.chn_count + 1))
   {
      size = (xds.chn_tab_size + 100) * sizeof(*xds.p_chn_table);
      pTmp = xmalloc(size);
      memset(pTmp, 0, size);
      if (xds.p_chn_table != NULL)
      {
         memcpy(pTmp, xds.p_chn_table, xds.chn_tab_size * sizeof(*xds.p_chn_table));
         xfree(xds.p_chn_table);
      }
      xds.p_chn_table = pTmp;
      // grow table in steps of 64 (64 should be enough for everyone)
      xds.chn_tab_size += 64;
   }
   xds.chn_open = TRUE;
}

void Xmltv_ChannelClose( void )
{
   bool   isNew;
   bool   result;
   uint * pChnIdx;

   assert(xds.chn_open);

   if (xds.chn_open)
   {
      result = FALSE;

      if ( (xds.p_chn_id_tmp != NULL) &&
           (xds.p_chn_table[xds.chn_count].p_disp_name != NULL) )
      {
         pChnIdx = XmlHash_CreateEntry(xds.pChannelHash, xds.p_chn_id_tmp, &isNew);
         if (isNew)
         {
            dprintf3("Xmltv-ChannelClose: add channel #%d '%s' (ID '%s')\n", xds.chn_count, xds.p_chn_table[xds.chn_count].p_disp_name, p_chn_id_tmp);

            xfree(xds.p_chn_id_tmp);
            xds.p_chn_id_tmp = NULL;

            *pChnIdx = xds.chn_count;
            xds.chn_count += 1;
            result = TRUE;
         }
         else
            debug2("Xmltv-ChannelClose: ID is not unique: '%s', display name '%s'", xds.p_chn_id_tmp, xds.p_chn_table[xds.chn_count].p_disp_name);
      }
      else
         debug2("Xmltv-ChannelClose: no ID or no display name (%lx,%lx)", (long)xds.p_chn_id_tmp, (long)xds.p_chn_table[xds.chn_count].p_disp_name);

      if (result == FALSE)
      {
         if (xds.p_chn_table[xds.chn_count].p_disp_name != NULL)
         {
            xfree(xds.p_chn_table[xds.chn_count].p_disp_name);
            xds.p_chn_table[xds.chn_count].p_disp_name = NULL;
         }
      }
      if (xds.p_chn_id_tmp != NULL)
      {
         xfree(xds.p_chn_id_tmp);
         xds.p_chn_id_tmp = NULL;
      }
      xds.chn_open = FALSE;
   }
   else
      fatal0("Xmltv-ChannelClose: not inside channel definition");
}

void Xmltv_ChannelSetId( XML_STR_BUF * pBuf )
{
   char * pStr = XML_STR_BUF_GET_STR(*pBuf);

   assert(xds.chn_open);

   if (xds.chn_open)
   {
      if (xds.p_chn_id_tmp == NULL)
      {
         xds.p_chn_id_tmp = xstrdup(pStr);
      }
      else
         debug2("Xmltv-ChannelSetId: 2 channel IDs: %s and %s", xds.p_chn_id_tmp, pStr);
   }
}

void Xmltv_ChannelAddName( XML_STR_BUF * pBuf )
{
   assert(xds.chn_open);

   if (xds.chn_open)
   {
      if (xds.p_chn_table[xds.chn_count].p_disp_name == NULL)
      {
         xds.p_chn_table[xds.chn_count].p_disp_name = xstrdup(XML_STR_BUF_GET_STR(*pBuf));
      }
      else
         debug2("Xmltv-ChannelAddName: 2 channel IDs: %s and %s", xds.p_chn_table[xds.chn_count].p_disp_name, XML_STR_BUF_GET_STR(*pBuf));
   }
}

void Xmltv_ChannelAddUrl( XML_STR_BUF * pBuf )
{
}

// ----------------------------------------------------------------------------

void Xmltv_TsOpen( void )
{
   dprintf0("Xmltv-TsOpen\n");
   xds.pi_start_time = 0;
   xds.pi_stop_time = 0;
   xds.pi_netwop_no = xds.chn_count;
#ifdef ALL_CONTENT
   xds.pi_pil = 0;
   xds.pi_feature_flags = 0;
   xds.audio_cnt = 0;
   xds.pi_cat_count = 0;
#endif

   XmlCdata_Reset(&xds.pi_title);
   XmlCdata_Reset(&xds.pi_subtitle);
   XmlCdata_Reset(&xds.pi_category);
   XmlCdata_Reset(&xds.pi_desc);
#ifdef ALL_CONTENT
   XmlCdata_Reset(&xds.pi_credits);
   XmlCdata_Reset(&xds.pi_actors);

   XmlCdata_Reset(&xds.pi_code_sv);
   XmlCdata_Reset(&xds.pi_code_vp);
#endif

   if (xds.isPeek)
   {
      XmlTags_ScanStop();
   }
}

bool Xmltv_TsFilter( void )
{
	if (   (xds.pi_start_time == 0)
		|| (xds.pi_stop_time == 0)
		|| (xds.pi_netwop_no >= xds.chn_count)
		|| !CheckProgrammeValidity(xds.pi_start_time, xds.pi_stop_time, xds.p_chn_table[xds.pi_netwop_no].p_disp_name) )
	{
		return FALSE;
	}

	return TRUE;
}

void Xmltv_TsClose( void )
{
   dprintf0("Xmltv_TsClose\n");

#ifdef ALL_CONTENT
   if (xds.pi_netwop_no < xds.chn_count)
   {
      if ((xds.pi_start_time < xds.p_chn_table[xds.pi_netwop_no].pi_min_time) ||
          (xds.p_chn_table[xds.pi_netwop_no].pi_min_time == 0))
         xds.p_chn_table[xds.pi_netwop_no].pi_min_time = xds.pi_start_time;
      if (xds.pi_start_time > xds.p_chn_table[xds.pi_netwop_no].pi_max_time)
         xds.p_chn_table[xds.pi_netwop_no].pi_max_time = xds.pi_start_time;
   }

   // append credits section (director, actors, crew)
   if ( (XML_STR_BUF_GET_STR_LEN(xds.pi_actors) > 0) ||
        (XML_STR_BUF_GET_STR_LEN(xds.pi_credits) > 0) )
   {
      XmlCdata_AppendParagraph(&xds.pi_desc, FALSE);
      XmlCdata_AppendString(&xds.pi_desc, "Credits: ");

      if (XML_STR_BUF_GET_STR_LEN(xds.pi_credits) > 0)
      {
         XmlCdata_AppendRaw(&xds.pi_desc, XML_STR_BUF_GET_STR(xds.pi_credits), XML_STR_BUF_GET_STR_LEN(xds.pi_credits));
         if (XML_STR_BUF_GET_STR_LEN(xds.pi_actors) > 0)
            XmlCdata_AppendRaw(&xds.pi_desc, "; ", 2);
      }
      if (XML_STR_BUF_GET_STR_LEN(xds.pi_actors) > 0)
         XmlCdata_AppendRaw(&xds.pi_desc, XML_STR_BUF_GET_STR(xds.pi_actors), XML_STR_BUF_GET_STR_LEN(xds.pi_actors));
   }

   // append ShowV*ew and Video+ codes (for programming VCR)
   if ( (XML_STR_BUF_GET_STR_LEN(xds.pi_code_sv) > 0) ||
        (XML_STR_BUF_GET_STR_LEN(xds.pi_code_vp) > 0) )
   {
      XmlCdata_AppendParagraph(&xds.pi_desc, FALSE);
      if (XML_STR_BUF_GET_STR_LEN(xds.pi_code_sv) > 0)
      {
         XmlCdata_AppendString(&xds.pi_desc, "SV-Code: ");
         XmlCdata_AppendRaw(&xds.pi_desc, XML_STR_BUF_GET_STR(xds.pi_code_sv), XML_STR_BUF_GET_STR_LEN(xds.pi_code_sv));

         if (XML_STR_BUF_GET_STR_LEN(xds.pi_code_vp) > 0)
            XmlCdata_AppendRaw(&xds.pi_desc, "; ", 2);
      }
      if (XML_STR_BUF_GET_STR_LEN(xds.pi_code_vp) > 0)
      {
         XmlCdata_AppendString(&xds.pi_desc, "V+ Code: ");
         XmlCdata_AppendRaw(&xds.pi_desc, XML_STR_BUF_GET_STR(xds.pi_code_vp), XML_STR_BUF_GET_STR_LEN(xds.pi_code_vp));
      }
   }
#endif

	AddProgramme(xds.pi_start_time, xds.pi_stop_time, XML_STR_BUF_GET_STR(xds.pi_title), xds.p_chn_table[xds.pi_netwop_no].p_disp_name, XML_STR_BUF_GET_STR(xds.pi_subtitle), XML_STR_BUF_GET_STR(xds.pi_category), XML_STR_BUF_GET_STR(xds.pi_desc));
}

void Xmltv_TsSetChannel( XML_STR_BUF * pBuf )
{
   char * pStr = XML_STR_BUF_GET_STR(*pBuf);
   uint * pChnIdx;

   pChnIdx = XmlHash_SearchEntry(xds.pChannelHash, pStr);
   if (pChnIdx != NULL)
   {
      xds.pi_netwop_no = *pChnIdx;
   }
   else
   {
      // note: the PI will be dropped if the channel isn't defined BEFORE it's referenced
      debug1("Xmltv-TsSetChannel: unknown channel ID '%s'", pStr);
   }
}

void Xmltv_TsSetStartTime( XML_STR_BUF * pBuf )
{
   char * pStr = XML_STR_BUF_GET_STR(*pBuf);
   uint len = XML_STR_BUF_GET_STR_LEN(*pBuf);

   xds.pi_start_time = Xmltv_ParseTimestamp(pStr, len);

   dprintf2("Xmltv-TsSetStartTime: start time %s: %d\n", pStr, (int)xds.pi.start_time);
}

void Xmltv_TsSetStopTime( XML_STR_BUF * pBuf )
{
   char * pStr = XML_STR_BUF_GET_STR(*pBuf);
   uint len = XML_STR_BUF_GET_STR_LEN(*pBuf);

   xds.pi_stop_time = Xmltv_ParseTimestamp(pStr, len);

   dprintf2("Xmltv-TsSetStopTime: stop time %s: %d\n", pStr, (int)xds.pi.stop_time);
}

// liveness    (live | joined | prerecorded) #IMPLIED
void Xmltv_TsSetFeatLive( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   char * pStr = XML_STR_BUF_GET_STR(*pBuf);

   if (strcasecmp(pStr, "live") == 0)
   {
      xds.pi_feature_flags |= PI_FEATURE_LIVE;
   }
#endif
}

void Xmltv_TsSetFeatCrypt( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   xds.pi_feature_flags |= PI_FEATURE_ENCRYPTED;
#endif
}

// ----------------------------------------------------------------------------

#ifdef ALL_CONTENT
static void Xmltv_TsCodeTimeAssign( XMLTV_CODE_TIME_SYS system, XML_STR_BUF * pBuf )
{
   char * pStr = XML_STR_BUF_GET_STR(*pBuf);
   uint len = XML_STR_BUF_GET_STR_LEN(*pBuf);

   switch (system)
   {
      case XMLTV_CODE_VPS:
      case XMLTV_CODE_PDC:
         xds.pi_pil = Xmltv_ParseVpsPdc(pStr);
         break;
      case XMLTV_CODE_SV:
         XmlCdata_Reset(&xds.pi_code_sv);
         XmlCdata_AppendRaw(&xds.pi_code_sv, pStr, len);
         break;
      case XMLTV_CODE_VP:
         XmlCdata_Reset(&xds.pi_code_vp);
         XmlCdata_AppendRaw(&xds.pi_code_vp, pStr, len);
         break;
      default:
         break;
   }
}
#endif

void Xmltv_TsCodeTimeOpen( void )
{
#ifdef ALL_CONTENT
   xds.pi_code_time_sys = XMLTV_CODE_NONE;
   XmlCdata_Reset(&xds.pi_code_time_str);
#endif
}

void Xmltv_TsCodeTimeClose( void )
{
#ifdef ALL_CONTENT
   Xmltv_TsCodeTimeAssign(xds.pi_code_time_sys, &xds.pi_code_time_str);
#endif
}

// DTD 0.5 only
void Xmltv_TsCodeTimeSetVps( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   Xmltv_TsCodeTimeAssign(XMLTV_CODE_VPS, pBuf);
#endif
}

void Xmltv_TsCodeTimeSetPdc( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   Xmltv_TsCodeTimeAssign(XMLTV_CODE_PDC, pBuf);
#endif
}

void Xmltv_TsCodeTimeSetSV( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   XmlCdata_AssignOrAppend(&xds.pi_code_sv, pBuf);
#endif
}

void Xmltv_TsCodeTimeSetVP( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   XmlCdata_AssignOrAppend(&xds.pi_code_vp, pBuf);
#endif
}

// DTD 0.6
void Xmltv_TsCodeTimeSetStart( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   if (xds.pi_code_time_sys == XMLTV_CODE_SV)
   {
      XmlCdata_Assign(&xds.pi_code_sv, pBuf);
   }
   else if (xds.pi_code_time_sys == XMLTV_CODE_VP)
   {
      XmlCdata_Assign(&xds.pi_code_vp, pBuf);
   }
   else
   {
      XmlCdata_Assign(&xds.pi_code_time_str, pBuf);
   }
#endif
}

void Xmltv_TsCodeTimeSetSystem( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   char * pStr = XML_STR_BUF_GET_STR(*pBuf);

   if (strcasecmp(pStr, "vps") == 0)
      xds.pi_code_time_sys = XMLTV_CODE_VPS;
   else if (strcasecmp(pStr, "pdc") == 0)
      xds.pi_code_time_sys = XMLTV_CODE_PDC;
   else if (strcasecmp(pStr, "showview") == 0)
      xds.pi_code_time_sys = XMLTV_CODE_SV;
   else if (strcasecmp(pStr, "videoplus") == 0)
      xds.pi_code_time_sys = XMLTV_CODE_VP;
   else
      debug1("Xmltv-TsCodeTimeSetSystem: unknown code-time systm '%s'", pStr);
#endif
}

// ----------------------------------------------------------------------------

void Xmltv_PiTitleAdd( XML_STR_BUF * pBuf )
{
   XmlCdata_Assign(&xds.pi_title, pBuf);
}

void Xmltv_PiEpisodeTitleAdd( XML_STR_BUF * pBuf )
{
   char * pStr = XML_STR_BUF_GET_STR(*pBuf);

   if ( isalnum(*pStr) )
   {
      XmlCdata_AppendRaw(&xds.pi_desc, "\"", 1);
      XmlCdata_AppendRaw(&xds.pi_desc, pStr, XML_STR_BUF_GET_STR_LEN(*pBuf));
      XmlCdata_AppendRaw(&xds.pi_desc, "\"", 1);
   }
   else
      XmlCdata_AssignOrAppend(&xds.pi_desc, pBuf);

   XmlCdata_Assign(&xds.pi_subtitle, pBuf);
}

// ----------------------------------------------------------------------------

void Xmltv_PiCatOpen( void )
{
}

void Xmltv_PiCatClose( void )
{
}

void Xmltv_PiCatSetType( XML_STR_BUF * pBuf )
{
   // ignored
}

void Xmltv_PiCatSetSystem( XML_STR_BUF * pBuf )
{
}

void Xmltv_PiCatSetCode( XML_STR_BUF * pBuf )
{
}

// DTD 0.5: themes given as free text
// DTD 0.6: use text for unknown systems only
void Xmltv_PiCatAddText( XML_STR_BUF * pBuf )
{
   XmlCdata_Assign(&xds.pi_category, pBuf);
}

// ----------------------------------------------------------------------------

void Xmltv_PiVideoBwSet( void )
{
}

// DTD 0.6 only
void Xmltv_PiVideoAspectOpen( void )
{
#ifdef ALL_CONTENT
   xds.pi_aspect_x = 0;
   xds.pi_aspect_y = 0;
#endif
}

void Xmltv_PiVideoAspectClose( void )
{
#ifdef ALL_CONTENT
   if ( (xds.pi_aspect_x != 0) && (xds.pi_aspect_y != 0) &&
        (3 * xds.pi_aspect_x > 4 * xds.pi_aspect_y) )  // equiv: x/y > 4/3
   {
      xds.pi_feature_flags |= PI_FEATURE_FMT_WIDE;
   }
#endif
}

void Xmltv_PiVideoAspectSetX( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   char * pStr = XML_STR_BUF_GET_STR(*pBuf);

   xds.pi_aspect_x = atoi(pStr);
#endif
}

void Xmltv_PiVideoAspectSetY( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   char * pStr = XML_STR_BUF_GET_STR(*pBuf);

   xds.pi_aspect_y = atoi(pStr);
#endif
}

// DTD 0.5: aspect specified as PCDATA
void Xmltv_PiVideoAspectAddXY( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   char * pStr = XML_STR_BUF_GET_STR(*pBuf);
   int scan_pos;
   int nscan;
   int asp_x, asp_y;

   nscan = sscanf(pStr, "%u : %u%n", &asp_x, &asp_y, &scan_pos);
   if ((nscan >= 2) && (pStr[scan_pos] == 0))
   {
      xds.pi_aspect_x = asp_x;
      xds.pi_aspect_y = asp_y;
   }
   else
      debug1("Xmltv-VideoAspectAddXY: parse error '%s'", pStr);
#endif
}

void Xmltv_PiVideoQualityAdd( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   char * pStr = XML_STR_BUF_GET_STR(*pBuf);

   if ( (strcasecmp(pStr, "PAL+") == 0) ||
        (strcasecmp(pStr, "PALplus") == 0) ||
        (strcasecmp(pStr, "PAL-plus") == 0) )
   {
      xds.pi_feature_flags |= PI_FEATURE_PAL_PLUS;
   }
#endif
}

void Xmltv_PiAudioOpen( void )
{
}

void Xmltv_PiAudioClose( void )
{
#ifdef ALL_CONTENT
   xds.audio_cnt += 1;

   // two mono channels -> so-called "2-channel" (usually different languages)
   if ( (xds.audio_cnt > 1) &&
        ((xds.pi_feature_flags & PI_FEATURE_SOUND_MASK) == PI_FEATURE_SOUND_MONO) )
   {
      xds.pi_feature_flags &= ~PI_FEATURE_SOUND_MASK;
      xds.pi_feature_flags |= PI_FEATURE_SOUND_2CHAN;
   }
#endif
}

// DTD 0.5 only: "stereo-ness" specified as free text
void Xmltv_PiAudioStereoAdd( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   char * pStr = XML_STR_BUF_GET_STR(*pBuf);

   if (strcasecmp(pStr, "mono") == 0)
   {
      xds.pi_feature_flags &= ~PI_FEATURE_SOUND_MASK;
      xds.pi_feature_flags |= PI_FEATURE_SOUND_MONO;
   }
   else if (strcasecmp(pStr, "stereo") == 0)
   {
      xds.pi_feature_flags &= ~PI_FEATURE_SOUND_MASK;
      xds.pi_feature_flags |= PI_FEATURE_SOUND_STEREO;
   }
   else if ( (strcasecmp(pStr, "surround") == 0) ||
             (strncasecmp(pStr, "quad", 4) == 0) )
   {
      xds.pi_feature_flags &= ~PI_FEATURE_SOUND_MASK;
      xds.pi_feature_flags |= PI_FEATURE_SOUND_SURROUND;
   }
   else
      debug1("Xmltv-PiAudioStereoAdd: unknown keyword '%s'", pStr);
#endif
}

// DTD 0.6: separate tags
void Xmltv_PiAudioMonoOpen( void )
{
#ifdef ALL_CONTENT
   xds.pi_feature_flags &= ~PI_FEATURE_SOUND_MASK;
   xds.pi_feature_flags |= PI_FEATURE_SOUND_MONO;
#endif
}

void Xmltv_PiAudioStereoOpen( void )
{
#ifdef ALL_CONTENT
   xds.pi_feature_flags &= ~PI_FEATURE_SOUND_MASK;
   xds.pi_feature_flags |= PI_FEATURE_SOUND_STEREO;
#endif
}

void Xmltv_PiAudioSurrOpen( void )
{
#ifdef ALL_CONTENT
   xds.pi_feature_flags &= ~PI_FEATURE_SOUND_MASK;
   xds.pi_feature_flags |= PI_FEATURE_SOUND_SURROUND;
#endif
}

// DTD 0.5 only: attribute type set as attribute
void Xmltv_PiSubtitlesSetType( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   char * pStr = XML_STR_BUF_GET_STR(*pBuf);

   if ( (strcmp(pStr, "teletext") == 0) ||
        (strcmp(pStr, "onscreen") == 0) )
   {
      xds.pi_feature_flags |= PI_FEATURE_SUBTITLES;
   }
   else
      debug1("Xmltv-PiSubtitlesSetType: unknown subtitles type '%s'", pStr);
#endif
}

// DTD 0.6 only: separate tags for different subtitle types
void Xmltv_PiSubtitlesOsd( void )
{
#ifdef ALL_CONTENT
   xds.pi_feature_flags |= PI_FEATURE_SUBTITLES;
#endif
}

void Xmltv_PiSubtitlesTtx( void )
{
#ifdef ALL_CONTENT
   xds.pi_feature_flags |= PI_FEATURE_SUBTITLES;
#endif
}

void Xmltv_PiSubtitlesSetPage( XML_STR_BUF * pBuf )
{
   // not supported by database struct
}

void Xmltv_PiRatingSetSystem( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   char * pStr = XML_STR_BUF_GET_STR(*pBuf);

   if (strcasecmp(pStr, "age") == 0)
   {
      xds.pi_rating_sys = XMLTV_RATING_AGE;
   }
   else if (strcasecmp(pStr, "FSK") == 0)
   {
      xds.pi_rating_sys = XMLTV_RATING_FSK;
   }
   else if (strcasecmp(pStr, "BBFC") == 0)
   {
      xds.pi_rating_sys = XMLTV_RATING_BBFC;
   }
   else if (strcasecmp(pStr, "MPAA") == 0)
   {
      xds.pi_rating_sys = XMLTV_RATING_MPAA;
   }
   else
      debug1("Xmltv-PiRatingSetSystem: unknown system '%s'", pStr);
#endif
}

void Xmltv_PiRatingAddText( XML_STR_BUF * pBuf )
{
}

// DTD 0.5 only: rating in format "n / m"
void Xmltv_PiStarRatingAddText( XML_STR_BUF * pBuf )
{
}

void Xmltv_PiStarRatingOpen( void )
{
}

void Xmltv_PiStarRatingClose( void )
{
}

// DTD 0.6 only: rating value and max. values as attributes
void Xmltv_PiStarRatingSetValue( XML_STR_BUF * pBuf )
{
}

void Xmltv_PiStarRatingSetMax( XML_STR_BUF * pBuf )
{
}

void Xmltv_PiDescOpen( void )
{
}

void Xmltv_PiDescClose( void )
{
}

void Xmltv_ParagraphCreate( void )
{
}

void Xmltv_ParagraphClose( void )
{
}

void Xmltv_ParagraphAdd( XML_STR_BUF * pBuf )
{
   XmlCdata_AppendParagraph(&xds.pi_desc, FALSE);
   XmlCdata_AssignOrAppend(&xds.pi_desc, pBuf);
}

// ----------------------------------------------------------------------------

void Xmltv_PiCreditsOpen( void )
{
#ifdef ALL_CONTENT
   XmlCdata_Reset(&xds.pi_credits);
   XmlCdata_Reset(&xds.pi_actors);
#endif
}

void Xmltv_PiCreditsClose( void )
{
}

#ifdef ALL_CONTENT
static void Xmltv_PiCreditsAppend( const char * pKind, XML_STR_BUF * pBuf )
{
   if (XML_STR_BUF_GET_STR_LEN(xds.pi_credits) > 0)
      XmlCdata_AppendRaw(&xds.pi_credits, ", ", 2);
   XmlCdata_AppendString(&xds.pi_credits, pKind);
   XmlCdata_AssignOrAppend(&xds.pi_credits, pBuf);
}
#endif

void Xmltv_PiCreditsAddDirector( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   Xmltv_PiCreditsAppend("directed by: ", pBuf);
#endif
}

void Xmltv_PiCreditsAddActor( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   if (XML_STR_BUF_GET_STR_LEN(xds.pi_actors) == 0)
      XmlCdata_AppendString(&xds.pi_actors, "With: ");
   else
      XmlCdata_AppendRaw(&xds.pi_actors, ", ", 2);
   XmlCdata_AppendRaw(&xds.pi_actors, XML_STR_BUF_GET_STR(*pBuf), XML_STR_BUF_GET_STR_LEN(*pBuf));
#endif
}

void Xmltv_PiCreditsAddWriter( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   Xmltv_PiCreditsAppend("written by: ", pBuf);
#endif
}

void Xmltv_PiCreditsAddAdapter( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   Xmltv_PiCreditsAppend("adapted by: ", pBuf);
#endif
}

void Xmltv_PiCreditsAddProducer( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   Xmltv_PiCreditsAppend("produced by: ", pBuf);
#endif
}

void Xmltv_PiCreditsAddExecProducer( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   Xmltv_PiCreditsAppend("exec. producer: ", pBuf);
#endif
}

void Xmltv_PiCreditsAddPresenter( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   Xmltv_PiCreditsAppend("presented by: ", pBuf);
#endif
}

void Xmltv_PiCreditsAddCommentator( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   Xmltv_PiCreditsAppend("commentator: ", pBuf);
#endif
}

void Xmltv_PiCreditsAddNarrator( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   Xmltv_PiCreditsAppend("narrator: ", pBuf);
#endif
}

void Xmltv_PiCreditsAddCompany( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   Xmltv_PiCreditsAppend("producer: ", pBuf);
#endif
}

void Xmltv_PiCreditsAddGuest( XML_STR_BUF * pBuf )
{
#ifdef ALL_CONTENT
   Xmltv_PiCreditsAppend("guest: ", pBuf);
#endif
}

void Xmltv_PiDateAdd( XML_STR_BUF * pBuf )
{
}

// ----------------------------------------------------------------------------
// Free resources
//
static void XmltvDb_Destroy( void )
{
   uint  idx;

   if (xds.p_chn_table != NULL)
   {
      // channel table list & dup'ed channel ID and display name strings
      // note: check one more than count in case there's an unclosed channel tag
      for (idx = 0; (idx < xds.chn_count + 1) && (idx < xds.chn_tab_size); idx++)
      {
         if (xds.p_chn_table[idx].p_disp_name != NULL)
            xfree(xds.p_chn_table[idx].p_disp_name);
      }
      xfree(xds.p_chn_table);
   }
   if (xds.p_chn_id_tmp != NULL)
   {
      xfree(xds.p_chn_id_tmp);
      xds.p_chn_id_tmp = NULL;
   }
   XmlHash_Destroy(xds.pChannelHash, NULL);

   // pi title and desc cache
   XmlCdata_Free(&xds.pi_title);
   XmlCdata_Free(&xds.pi_subtitle);
   XmlCdata_Free(&xds.pi_category);
   XmlCdata_Free(&xds.pi_desc);
#ifdef ALL_CONTENT
   XmlCdata_Free(&xds.pi_credits);
   XmlCdata_Free(&xds.pi_actors);

   XmlCdata_Free(&xds.pi_code_time_str);
   XmlCdata_Free(&xds.pi_code_sv);
   XmlCdata_Free(&xds.pi_code_vp);

   XmlCdata_Free(&xds.source_info_name);
   XmlCdata_Free(&xds.source_info_url);
   XmlCdata_Free(&xds.source_data_url);
   XmlCdata_Free(&xds.gen_info_name);
   XmlCdata_Free(&xds.gen_info_url);
#endif
}

// ----------------------------------------------------------------------------
// Initialize the local module state
//
static void XmltvDb_Init( void )
{
   memset(&xds, 0, sizeof(xds));
   xds.pChannelHash = XmlHash_Init();

   // set string buffer size hints
   XmlCdata_Init(&xds.pi_title, 256);
   XmlCdata_Init(&xds.pi_subtitle, 256);
   XmlCdata_Init(&xds.pi_category, 256);
#ifdef ALL_CONTENT
   XmlCdata_Init(&xds.pi_code_time_str, 256);
   XmlCdata_Init(&xds.pi_code_sv, 256);
   XmlCdata_Init(&xds.pi_code_vp, 256);
#endif
   XmlCdata_Init(&xds.pi_desc, 4096);
}

// ----------------------------------------------------------------------------
// Entry point
//
void XmltvParser_Start( const char * pFilename )
{
   FILE * fp;
   XMLTV_DETECTION detection;
   XMLTV_DTD_VERSION dtd;

   // initialize internal state
   memset(&xds, 0, sizeof(xds));

   fp = fopen(pFilename, "r");
   if (fp != NULL)
   {
     // start scanner in DTD version detection mode
     XmltvTags_StartScan(fp, XMLTV_DTD_UNKNOWN);

     dtd = XmltvTags_QueryVersion(&detection);
     if (dtd != XMLTV_DTD_UNKNOWN)
     {
        // rewind to the beginning after reading the header for the version check
        fseek(fp, 0, SEEK_SET);

        // initialize internal state
        XmltvDb_Init();
        xds.dtd = dtd;

        // parse the XMLTV file (will invoke the callbacks above to add programme data)
        XmltvTags_StartScan(fp, dtd);

        XmltvDb_Destroy();
	 }

      fclose(fp);
   }
}


