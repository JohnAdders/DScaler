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
 *  Description: see C source file.
 *
 *  Author: Tom Zoerner
 *          L. Garnier for interfacing with DScaler
 *
 *  $Id$
 */

#ifndef __XMLTV_DB_H
#define __XMLTV_DB_H


// ----------------------------------------------------------------------------
// Interface to the element parser (callback functions)
//
void Xmltv_AboutSetSourceInfoUrl( XML_STR_BUF * pBuf );
void Xmltv_AboutSetSourceInfoName( XML_STR_BUF * pBuf );
void Xmltv_AboutSetSourceDataUrl( XML_STR_BUF * pBuf );
void Xmltv_SourceInfoOpen( void );
void Xmltv_SourceInfoClose( void );
void Xmltv_GenInfoOpen( void );
void Xmltv_GenInfoClose( void );
void Xmltv_AboutSetGenInfoName( XML_STR_BUF * pBuf );
void Xmltv_AboutSetGenInfoUrl( XML_STR_BUF * pBuf );
void Xmltv_LinkAddText( XML_STR_BUF * pBuf );
void Xmltv_LinkHrefSet( XML_STR_BUF * pBuf );
void Xmltv_LinkBlurbAddText( XML_STR_BUF * pBuf );

void Xmltv_ChannelCreate( void );
void Xmltv_ChannelClose( void );
void Xmltv_ChannelSetId( XML_STR_BUF * pBuf );
void Xmltv_ChannelAddName( XML_STR_BUF * pBuf );
void Xmltv_ChannelAddUrl( XML_STR_BUF * pBuf );

void Xmltv_TsOpen( void );
void Xmltv_TsClose( void );
Bool Xmltv_TsFilter( void );
void Xmltv_TsSetChannel( XML_STR_BUF * pBuf );
void Xmltv_TsSetStartTime( XML_STR_BUF * pBuf );
void Xmltv_TsSetStopTime( XML_STR_BUF * pBuf );
void Xmltv_TsSetFeatLive( XML_STR_BUF * pBuf );
void Xmltv_TsSetFeatCrypt( XML_STR_BUF * pBuf );
void Xmltv_TsCodeTimeOpen( void );
void Xmltv_TsCodeTimeClose( void );
void Xmltv_TsCodeTimeSetVps( XML_STR_BUF * pBuf );
void Xmltv_TsCodeTimeSetPdc( XML_STR_BUF * pBuf );
void Xmltv_TsCodeTimeSetSV( XML_STR_BUF * pBuf );
void Xmltv_TsCodeTimeSetVP( XML_STR_BUF * pBuf );
void Xmltv_TsCodeTimeSetStart( XML_STR_BUF * pBuf );
void Xmltv_TsCodeTimeSetSystem( XML_STR_BUF * pBuf );

void Xmltv_PiDescOpen( void );
void Xmltv_PiDescClose( void );
void Xmltv_PiTitleAdd( XML_STR_BUF * pBuf );
void Xmltv_PiEpisodeTitleAdd( XML_STR_BUF * pBuf );
void Xmltv_PiDateAdd( XML_STR_BUF * pBuf );
void Xmltv_PiCatOpen( void );
void Xmltv_PiCatClose( void );
void Xmltv_PiCatSetType( XML_STR_BUF * pBuf );
void Xmltv_PiCatSetSystem( XML_STR_BUF * pBuf );
void Xmltv_PiCatSetCode( XML_STR_BUF * pBuf );
void Xmltv_PiCatAddText( XML_STR_BUF * pBuf );
void Xmltv_PiVideoBwSet( void );
void Xmltv_PiVideoAspectOpen( void );
void Xmltv_PiVideoAspectClose( void );
void Xmltv_PiVideoAspectSetX( XML_STR_BUF * pBuf );
void Xmltv_PiVideoAspectSetY( XML_STR_BUF * pBuf );
void Xmltv_PiVideoAspectAddXY( XML_STR_BUF * pBuf );
void Xmltv_PiVideoQualityAdd( XML_STR_BUF * pBuf );
void Xmltv_PiAudioOpen( void );
void Xmltv_PiAudioClose( void );
void Xmltv_PiAudioMonoOpen( void );
void Xmltv_PiAudioStereoOpen( void );
void Xmltv_PiAudioStereoAdd( XML_STR_BUF * pBuf );
void Xmltv_PiAudioSurrOpen( void );
void Xmltv_PiSubtitlesOsd( void );
void Xmltv_PiSubtitlesTtx( void );
void Xmltv_PiSubtitlesSetType( XML_STR_BUF * pBuf );
void Xmltv_PiSubtitlesSetPage( XML_STR_BUF * pBuf );
void Xmltv_PiRatingSetSystem( XML_STR_BUF * pBuf );
void Xmltv_PiRatingAddText( XML_STR_BUF * pBuf );
void Xmltv_PiStarRatingOpen( void );
void Xmltv_PiStarRatingClose( void );
void Xmltv_PiStarRatingAddText( XML_STR_BUF * pBuf );
void Xmltv_PiStarRatingSetValue( XML_STR_BUF * pBuf );
void Xmltv_PiStarRatingSetMax( XML_STR_BUF * pBuf );
void Xmltv_ParagraphCreate( void );
void Xmltv_ParagraphClose( void );
void Xmltv_ParagraphAdd( XML_STR_BUF * pBuf );

void Xmltv_PiCreditsOpen( void );
void Xmltv_PiCreditsClose( void );
void Xmltv_PiCreditsAddDirector( XML_STR_BUF * pBuf );
void Xmltv_PiCreditsAddActor( XML_STR_BUF * pBuf );
void Xmltv_PiCreditsAddWriter( XML_STR_BUF * pBuf );
void Xmltv_PiCreditsAddAdapter( XML_STR_BUF * pBuf );
void Xmltv_PiCreditsAddProducer( XML_STR_BUF * pBuf );
void Xmltv_PiCreditsAddExecProducer( XML_STR_BUF * pBuf );
void Xmltv_PiCreditsAddPresenter( XML_STR_BUF * pBuf );
void Xmltv_PiCreditsAddCommentator( XML_STR_BUF * pBuf );
void Xmltv_PiCreditsAddNarrator( XML_STR_BUF * pBuf );
void Xmltv_PiCreditsAddCompany( XML_STR_BUF * pBuf );
void Xmltv_PiCreditsAddGuest( XML_STR_BUF * pBuf );

// ----------------------------------------------------------------------------
// Interface to main control module
//
#ifdef __EPGBLOCK_H
void XmltvDb_Init( XMLTV_DTD_VERSION dtd, Bool isPeek );
void XmltvDb_Destroy( void );
EPGDB_CONTEXT * XmltvDb_GetDatabase( void );
#endif


#endif // __XMLTV_DB_H
