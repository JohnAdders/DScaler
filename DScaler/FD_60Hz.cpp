/////////////////////////////////////////////////////////////////////////////
// $Id: FD_60Hz.cpp,v 1.16 2001-08-08 08:54:32 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock. All rights reserved.
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
//
// Refinements made by Mark Rejhon and Steve Grimm
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           Original Release
//
// 17 Sep 2000   Mark Rejhon           Implemented Steve Grimm's changes
//                                     Some cleanup done.
//                                     Made refinements to Steve Grimm's changes
//
// 07 Jan 2001   John Adcock           Split code that did adaptive method
//                                     out of UpdateNTSCPulldownMode
//                                     Added gNTSCFilmFallbackMode setting
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 09 Jan 2001   John Adcock           Split out into new file
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.15  2001/08/02 16:43:05  adcockj
// Added Debug level to LOG function
//
// Revision 1.14  2001/07/16 18:07:50  adcockj
// Added Optimisation parameter to ini file saving
//
// Revision 1.13  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.12  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "OutThreads.h"
#include "FD_60Hz.h"
#include "FD_Common.h"
#include "DebugLog.h"
#include "SettingsDlg.h"

// Settings
// Default values which can be overwritten by the INI file
long NTSCFilmFallbackIndex = INDEX_ADAPTIVE;
long NTSCBadCadenceIndex = INDEX_VIDEO_GREEDY;
long Threshold32Pulldown = 15;
long ThresholdPulldownMismatch = 100;
long ThresholdPulldownComb = 300;
BOOL bFallbackToVideo = TRUE;
long PulldownRepeatCount = 4;
long PulldownRepeatCount2 = 2;
long PulldownSwitchMax = 4;
long PulldownSwitchInterval = 3000;
long MaxCallsToNTSCComb = 20;

// Module wide declarations
long NextPulldownRepeatCount = 0;    // for temporary increases of PullDownRepeatCount
DWORD ModeSwitchTimestamps[MAXMODESWITCHES];
DEINTERLACE_METHOD* ModeSwitchMethods[MAXMODESWITCHES];
int NumSwitches;

BOOL DidWeExpectWeave(DEINTERLACE_INFO* pInfo);

///////////////////////////////////////////////////////////////////////////////
// ResetModeSwitches
//
// Resets the memory used by TrackModeSwitches
///////////////////////////////////////////////////////////////////////////////
void ResetModeSwitches()
{
    memset(&ModeSwitchTimestamps[0], 0, sizeof(ModeSwitchTimestamps));
    memset(&ModeSwitchMethods[0], 0, sizeof(ModeSwitchMethods));
    NumSwitches = 0;
}

///////////////////////////////////////////////////////////////////////////////
// TrackModeSwitches
//
// Called whenever we switch to a new film Mode.  Keeps track of the frequency
// of Mode switches; if we switch too often, we want to drop down to video
// Mode since it means we're having trouble locking onto a particular film
// Mode.
//
// The settings PulldownSwitchInterval and PulldownSwitchMax control the
// sensitivity of this algorithm.  To trigger video Mode there need to be
// PulldownSwitchMax Mode switches in PulldownSwitchInterval milliseconds.
///////////////////////////////////////////////////////////////////////////////
BOOL TrackModeSwitches()
{
    if(IsFilmMode() &&
        GetCurrentDeintMethod() != ModeSwitchMethods[0])
    {
        // Scroll the list of timestamps.  Most recent is first in the list.
        memmove(&ModeSwitchTimestamps[1], &ModeSwitchTimestamps[0], sizeof(ModeSwitchTimestamps) - sizeof(DWORD));
        memmove(&ModeSwitchMethods[1], &ModeSwitchMethods[0], sizeof(ModeSwitchMethods) - sizeof(DEINTERLACE_METHOD*));
        ModeSwitchTimestamps[0] = GetTickCount();
        ModeSwitchMethods[0] = GetCurrentDeintMethod();
        if(NumSwitches < MAXMODESWITCHES)
        {
            NumSwitches++;
        }
    
        if (PulldownSwitchMax > 1 && PulldownSwitchInterval > 0 &&  // if the user wants to track switches
            PulldownSwitchMax <= NumSwitches &&
            ModeSwitchTimestamps[PulldownSwitchMax - 1] > 0)        // and there have been enough of them
        {
            int ticks = ModeSwitchTimestamps[0] - ModeSwitchTimestamps[PulldownSwitchMax - 1];
            if (ticks <= PulldownSwitchInterval)
                return TRUE;
        }
    }

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// UpdateNTSCPulldownMode
//
// This gem is where our software competes with the big boys - you know
// the expensive settop scaler boxes that detect 3:2 pulldown Mode!
//
// Original Programmer: JohnAd 
// Other maintainers: Mark Rejhon and Steve Grimm
//
// His attempt to implement Mark Rejhon's 3:2 pulldown code.
// This is an advanced descendant of Mark Rejhon's 3:2 algorithm designed 
// completely from scratch on paper in May 1999 at the following URL:
// http://www.avsforum.com/ubb/Forum12/HTML/000071.html
//
// There are numerous refinements contributed by Mark Rejhon and Steve Grimm.
// Mark Rejhon can be reached at dtv@marky.com
// Discussion forum is http://www.avsforum.com - AVSCIENCE HTPC Forum
//
// The algorithm and comments below are taken from Mark's post to the AVSCIENCE
// Home Theater Computer Forum reproduced in the file 32Spec.htm that should be
// with this source.  The key to getting this to work will be choosing the right 
// Value for the Threshold32Pulldown variable and others.  This should probably 
// vary depending on the input source, higher for video lower for TV and cable 
// and very low for laserdisk or DVD.
//
// In addition, we have a sanity check for poorly-transfered material.  If a
// field that will be woven with the previous field is dramatically different
// than the field before the previous one, we check the comb factor of the
// woven-together frame.  If it's too high, we immediately switch to video
// deinterlace Mode to prevent weave artifacts from appearing.
//
// This function normally gets called 60 times per second.
///////////////////////////////////////////////////////////////////////////////
void UpdateNTSCPulldownMode(DEINTERLACE_INFO* pInfo)
{
    boolean SwitchToVideo = FALSE;
    static long MISMATCH_COUNT = 0;
    static long MOVIE_FIELD_CYCLE = 0;
    static long MOVIE_VERIFY_CYCLE = 0;
    static DEINTERLACE_METHOD* OldPulldownMethod = NULL;
    static eFilmPulldownMode LastFilmMode = FILMPULLDOWNMODES_LAST_ONE;
    static long LastCombFactor = 0;

    // Call with pInfo == NULL is an initialization call.
    // This resets static variables when we start the thread each time.
    if(pInfo == NULL)
    {
        MOVIE_VERIFY_CYCLE = 0;
        MOVIE_FIELD_CYCLE = 0;
        MISMATCH_COUNT = 0;
        LastFilmMode = FILMPULLDOWNMODES_LAST_ONE;
        LastCombFactor = 0;
        ResetModeSwitches();
        return;
    }


    // If the field difference is bigger than the threshold, then
    // the current field is very different from the field two fields ago.
    // Threshold32Pulldown probably should be changed to be automatically
    // compensating depending on the material.
    
    //CompareFields(pInfo);
    if(pInfo->FieldDiff > Threshold32Pulldown)
    {
        if (IsFilmMode())
        {
            if ((MISMATCH_COUNT > PulldownRepeatCount2 * 5 && bFallbackToVideo) ||
                (MISMATCH_COUNT > PulldownRepeatCount * 5 && !bFallbackToVideo))
            {
                // There have been no duplicate fields lately.
                // It's probably video source.
                //
                // MAX_MISMATCH should be a reasonably high Value so
                // that we do not have an oversensitive hair-trigger
                // in switching to video source everytime there is
                // video noise or a single spurious field added/dropped
                // during a movie causing mis-synchronization problems. 
                LOG(2, " Back to Video, No field pairs");
                SetVideoDeinterlaceIndex(NTSCFilmFallbackIndex);
                MOVIE_VERIFY_CYCLE = 0;
                MOVIE_FIELD_CYCLE = 0;
            }
            else
            {
                // If we're in a film Mode and an incoming field would cause
                // weave artifacts, optionally switch to video Mode but make
                // it very easy to get back into film Mode in case this was
                // just a glitchy scene change.
                if (ThresholdPulldownMismatch > 0 &&            // only do video-force check if there's a threshold.
                    pInfo->FieldDiff >= ThresholdPulldownMismatch &&    // only force video if this field is very different,
                    DidWeExpectWeave(pInfo) &&              // and we would weave it with the previous field,
                    pInfo->CombFactor > (LastCombFactor + ThresholdPulldownComb) &&   // and it'd produce artifacts
                    pInfo->CombFactor > ThresholdPulldownComb)
                {
                    if (bFallbackToVideo)
                    {
                        SwitchToVideo = TRUE;
                        NextPulldownRepeatCount = 1;
                        LOG(2, " Back to Video, comb factor %d", pInfo->CombFactor);
                        SetVideoDeinterlaceIndex(NTSCFilmFallbackIndex);
                        MOVIE_VERIFY_CYCLE = 0;
                        MOVIE_FIELD_CYCLE = 0;
                    }
                    else
                    {
                        // Reset the paramters of the Comb method
                        FilmModeNTSCComb(NULL);
                        // go to Comb Mode but be ready to go back into film Mode
                        // as soon as we find a match
                        SetFilmDeinterlaceMode(FILM_32_PULLDOWN_COMB);
                        MOVIE_VERIFY_CYCLE = PulldownRepeatCount;
                        NextPulldownRepeatCount = 0;
                        LastFilmMode = FILM_32_PULLDOWN_COMB;
                        LOG(2, " Gone to Comb Method, comb factor %d", pInfo->CombFactor);
                    }
                    MISMATCH_COUNT++;
                }
                else
                {
                    MISMATCH_COUNT++;
                }
            }
        }
        else
        {
            MISMATCH_COUNT++;
        }
    }
    else
    {
        // It's either a stationary image OR a duplicate field in a movie
        if(MISMATCH_COUNT == 4)
        {
            eFilmPulldownMode NewFilmMode = GetFilmModeFromPosition(pInfo);
            // 3:2 pulldown is a cycle of 5 fields where there is only
            // one duplicate field pair, and 4 mismatching pairs.
            // We need to continue detection for at least PulldownRepeatCount
            // cycles to be very certain that it is actually 3:2 pulldown.
            // For a repeat Count of 2, this would mean a latency of 10
            // fields.
            //
            // If NextPulldownRepeatCount is nonzero, it's a temporary
            // repeat Count setting attempting to compensate for some kind
            // of anomaly in the sequence of fields, so use it instead.
            if(NewFilmMode == LastFilmMode || 
                (IsFilmMode() && LastFilmMode == FILM_32_PULLDOWN_COMB) ||
                LastFilmMode == FILMPULLDOWNMODES_LAST_ONE)
            {
                if(((NextPulldownRepeatCount > 0 && MOVIE_VERIFY_CYCLE >= NextPulldownRepeatCount) ||
                   (NextPulldownRepeatCount == 0 && MOVIE_VERIFY_CYCLE >= PulldownRepeatCount)))
                {
                    // If the pulldown repeat Count was temporarily changed, get
                    // rid of the temporary setting.
                    NextPulldownRepeatCount = 0;

                    // This executes regardless whether we've just entered or
                    // if we're *already* in 3:2 pulldown. Either way, we are
                    // currently now (re)synchronized to 3:2 pulldown and that
                    // we've now detected the duplicate field.
                    //
                    SetFilmDeinterlaceMode(NewFilmMode);

                    if (OldPulldownMethod != GetCurrentDeintMethod())
                    {
                        LOG(2, "Gone to film Mode %d", NewFilmMode - FILM_32_PULLDOWN_0); 
                        // A Mode switch.  If we've done a lot of them recently,
                        // force video Mode since it means we're having trouble
                        // locking onto a reliable film Mode.   
                        // 
                        // This stuff happens during these situations
                        // - Time compressed movies, which ruins 3:2 sequence 
                        // - Poorly telecined movies
                        // - Erratic framerates during transfers of old movies
                        // - TV commercials that vary framerates
                        // - Cartoons using nonstandard pulldown
                        // - Super-noisy video
                        //
                        // Therefore, we should switch to video Mode and ignore
                        // switching back to movie Mode for at least a certain
                        // amount of time.
                        //
                        // This is only triggered on switches between different
                        // film Modes, not switches between video and film Mode.
                        // Since we can drop down to video if we're not sure
                        // we should stay in pulldown Mode, we don't want to
                        // bail out of film Mode if we subsequently decide that
                        // the film Mode we just dropped out of was correct.
                        if (TrackModeSwitches())
                        {
                            if(bFallbackToVideo)
                            {
                                SetVideoDeinterlaceIndex(NTSCFilmFallbackIndex);
                                MOVIE_VERIFY_CYCLE = 0;
                                MOVIE_FIELD_CYCLE = 0;
                                LOG(2, " Too much film Mode cycling, switching to video");
                                
                                // Require pulldown Mode to be consistent for the
                                // rapid-Mode-switch interval before we'll lock onto
                                // film Mode again.  This is probably too long in most
                                // cases, but making it shorter than the interval would
                                // run the risk of switching back to film Mode just in
                                // time to hit a rapid sequence of Mode changes.
                                //
                                // 83 is (5 fields/cycle) / (60 fields/sec) * (1000 ms/sec).
                                //
                                // FIXME: Eliminate hardcoded values
                                //
                                //NextPulldownRepeatCount = PulldownSwitchInterval / 83;
                                NextPulldownRepeatCount = PulldownRepeatCount * 2;
                            }
                            else
                            {
                                LOG(2, " Rapid Cycling");
                            }
                        }
                    }

                    OldPulldownMethod = GetCurrentDeintMethod();
                }
                else
                {
                    // We've detected possible 3:2 pulldown.  However, we need
                    // to keep watching the 3:2 pulldown for at least a few 5-field
                    // cycles before jumping to conclusion that it's really 3:2
                    // pulldown and not a false alarm
                    //
                    MOVIE_VERIFY_CYCLE++;
                    LOG(2, " Found Pulldown Match");
                }
                LastFilmMode = NewFilmMode;
            }
            else
            {
                if(bFallbackToVideo)
                {
                    if(IsFilmMode())
                    {
                        SetFilmDeinterlaceMode(NewFilmMode);
                        if(TrackModeSwitches())
                        {
                            SetVideoDeinterlaceIndex(NTSCFilmFallbackIndex);
                            MOVIE_VERIFY_CYCLE = 0;
                            MOVIE_FIELD_CYCLE = 0;
                            LOG(2, " Too much film Mode cycling, switching to video");
                            NextPulldownRepeatCount = PulldownRepeatCount * 2;
                        }
                    }
                    else
                    {
                        MOVIE_VERIFY_CYCLE = 1;
                        LastFilmMode = NewFilmMode;
                    }
                }
                else
                {
                    SetFilmDeinterlaceMode(NewFilmMode);
                    LOG(2, "Gone to film Mode %d", NewFilmMode - FILM_32_PULLDOWN_0); 
                    if(TrackModeSwitches())
                    {
                        SetVideoDeinterlaceIndex(NTSCFilmFallbackIndex);
                        MOVIE_VERIFY_CYCLE = 0;
                        MOVIE_FIELD_CYCLE = 0;
                        LOG(2, " Too much film Mode cycling, switching to video");
                        NextPulldownRepeatCount = PulldownRepeatCount * 2;
                    }
                    LastFilmMode = NewFilmMode;
                }
            }
        }
        else
        {
            // If we've just seen rapid-fire pulldown Mode switches,
            // require all the duplicate fields to be at the same point
            // in the pulldown sequence.  Getting here means a duplicate
            // field happened in a different place than in the previous
            // cycle, so reset the Count of pulldown matches.
            //
            // As noted below, this will mean that it takes a long time to
            // switch back to film Mode if we hit a bunch of still frames
            // after a rapid-fire Mode switch sequence.
            if (NextPulldownRepeatCount > 0) MOVIE_VERIFY_CYCLE = 0;

            // Normally,
            // If execution point hits here, it is probably just
            // stationary video. That would produce lots of duplicate
            // field pairs. We can't determine whether it's video or
            // movie source. Therefore, we want to keep doing the
            // current algorithm from a prior detection. Therefore,
            // don't modify any variables EXCEPT the MISMATCH_COUNT
            // variable which will be reset to 0 below.
            // Also, occasionally we'll hit here during synchronization
            // problems during a movie, such as a spurious field added
            // or dropped. Reset MISMATCH_COUNT to 0 below and let
            // detection cycle happen again in order to re-synchronize.
            //
        }
        MISMATCH_COUNT = 0;
    }

    if(pInfo->CombFactor > 0)
    {
        LastCombFactor = pInfo->CombFactor;
    }
}

eFilmPulldownMode GetFilmModeFromPosition(DEINTERLACE_INFO* pInfo)
{
    if(pInfo->IsOdd == TRUE)
    {
        switch(pInfo->CurrentFrame)
        {
        case 0:  return FILM_32_PULLDOWN_2;  break;
        case 1:  return FILM_32_PULLDOWN_3;  break;
        case 2:  return FILM_32_PULLDOWN_4;  break;
        case 3:  return FILM_32_PULLDOWN_0;  break;
        case 4:  return FILM_32_PULLDOWN_1;  break;
        }
    }
    else
    {
        switch(pInfo->CurrentFrame)
        {
        case 0:  return FILM_32_PULLDOWN_4;  break;
        case 1:  return FILM_32_PULLDOWN_0;  break;
        case 2:  return FILM_32_PULLDOWN_1;  break;
        case 3:  return FILM_32_PULLDOWN_2;  break;
        case 4:  return FILM_32_PULLDOWN_3;  break;
        }
    }
    return FILM_32_PULLDOWN_0;
}


BOOL FlipNTSC1st(int CurrentFrame, BOOL bIsOdd)
{
    BOOL bFlipNow;
    // Film Mode.  If we have an entire new frame, display it.
    switch(CurrentFrame)
    {
    case 0:  bFlipNow = FALSE;    break;
    case 1:  bFlipNow = !bIsOdd;  break;
    case 2:  bFlipNow = !bIsOdd;  break;
    case 3:  bFlipNow = bIsOdd;   break;
    case 4:  bFlipNow = bIsOdd;   break;
    }
    return bFlipNow;
}

BOOL FlipNTSC2nd(int CurrentFrame, BOOL bIsOdd)
{
    BOOL bFlipNow;
    // Film Mode.  If we have an entire new frame, display it.
    switch(CurrentFrame)
    {
    case 0:  bFlipNow = bIsOdd;   break;
    case 1:  bFlipNow = FALSE;    break;
    case 2:  bFlipNow = !bIsOdd;  break;
    case 3:  bFlipNow = !bIsOdd;  break;
    case 4:  bFlipNow = bIsOdd;   break;
    }
    return bFlipNow;
}

BOOL FlipNTSC3rd(int CurrentFrame, BOOL bIsOdd)
{
    BOOL bFlipNow;
    // Film Mode.  If we have an entire new frame, display it.
    switch(CurrentFrame)
    {
    case 0:  bFlipNow = bIsOdd;   break;
    case 1:  bFlipNow = bIsOdd;   break;
    case 2:  bFlipNow = FALSE;    break;
    case 3:  bFlipNow = !bIsOdd;  break;
    case 4:  bFlipNow = !bIsOdd;  break;
    }
    return bFlipNow;
}

BOOL FlipNTSC4th(int CurrentFrame, BOOL bIsOdd)
{
    BOOL bFlipNow;
    // Film Mode.  If we have an entire new frame, display it.
    switch(CurrentFrame)
    {
    case 0:  bFlipNow = !bIsOdd;  break;
    case 1:  bFlipNow = bIsOdd;   break;
    case 2:  bFlipNow = bIsOdd;   break;
    case 3:  bFlipNow = FALSE;    break;
    case 4:  bFlipNow = !bIsOdd;  break;
    }
    return bFlipNow;
}

BOOL FlipNTSC5th(int CurrentFrame, BOOL bIsOdd)
{
    BOOL bFlipNow;
    // Film Mode.  If we have an entire new frame, display it.
    switch(CurrentFrame)
    {
    case 0:  bFlipNow = !bIsOdd;  break;
    case 1:  bFlipNow = !bIsOdd;  break;
    case 2:  bFlipNow = bIsOdd;   break;
    case 3:  bFlipNow = bIsOdd;   break;
    case 4:  bFlipNow = FALSE;    break;
    }
    return bFlipNow;
}

BOOL FilmModeNTSC1st(DEINTERLACE_INFO* pInfo)
{
    return SimpleFilmMode(pInfo, FlipNTSC1st);
}

BOOL FilmModeNTSC2nd(DEINTERLACE_INFO* pInfo)
{
    return SimpleFilmMode(pInfo, FlipNTSC2nd);
}

BOOL FilmModeNTSC3rd(DEINTERLACE_INFO* pInfo)
{
    return SimpleFilmMode(pInfo, FlipNTSC3rd);
}

BOOL FilmModeNTSC4th(DEINTERLACE_INFO* pInfo)
{
    return SimpleFilmMode(pInfo, FlipNTSC4th);
}

BOOL FilmModeNTSC5th(DEINTERLACE_INFO* pInfo)
{
    return SimpleFilmMode(pInfo, FlipNTSC5th);
}

BOOL FilmModeNTSCComb(DEINTERLACE_INFO* pInfo)
{
    static long NumCalls = 0;
    DEINTERLACE_METHOD* DeintMethod;
    
    if(pInfo == NULL)
    {
        NumCalls = 0;
        return FALSE;
    }

    ++NumCalls;
    if(NumCalls > MaxCallsToNTSCComb)
    {
        SetVideoDeinterlaceIndex(NTSCFilmFallbackIndex);
        LOG(2, " Gone back to video from Comb Mode");
    }
    DeintMethod = GetVideoDeintIndex(NTSCBadCadenceIndex);
    if(DeintMethod != NULL && DeintMethod->nMethodIndex == NTSCBadCadenceIndex)
    {
        return DeintMethod->pfnAlgorithm(pInfo);
    }
    else
    {
        return Bob(pInfo);
    }
}

BOOL DidWeExpectWeave(DEINTERLACE_INFO* pInfo)
{
    BOOL RetVal;
    switch(GetFilmMode())
    {
    case FILM_32_PULLDOWN_0:
        return FlipNTSC1st(pInfo->CurrentFrame, pInfo->IsOdd);
        break;
    case FILM_32_PULLDOWN_1:
        return FlipNTSC2nd(pInfo->CurrentFrame, pInfo->IsOdd);
        break;
    case FILM_32_PULLDOWN_2:
        return FlipNTSC3rd(pInfo->CurrentFrame, pInfo->IsOdd);
        break;
    case FILM_32_PULLDOWN_3:
        return FlipNTSC4th(pInfo->CurrentFrame, pInfo->IsOdd);
        break;
    case FILM_32_PULLDOWN_4:
        return FlipNTSC5th(pInfo->CurrentFrame, pInfo->IsOdd);
        break;
    default:
        RetVal = FALSE;
        break;
    }
    return RetVal;
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING FD60Settings[FD60_SETTING_LASTONE] =
{
    {
        "NTSC Video Mode", ITEMFROMLIST, 0, (long*)&NTSCFilmFallbackIndex,
        INDEX_ADAPTIVE, 0, 99, 1, 1,
        DeinterlaceNames,
        "Pulldown", "NTSCFilmFallbackMode", NULL,
    },
    {
        "NTSC Pulldown Repeat Count In", SLIDER, 0, (long*)&PulldownRepeatCount,
        4, 1, 10, 1, 1,
        NULL,
        "Pulldown", "PulldownRepeatCount", NULL,
    },
    {
        "NTSC Pulldown Repeat Count Out", SLIDER, 0, (long*)&PulldownRepeatCount2,
        2, 1, 10, 1, 1,
        NULL,
        "Pulldown", "PulldownRepeatCount2", NULL,
    },
    {
        "Threshold 3:2 Pulldown", SLIDER, 0, (long*)&Threshold32Pulldown,
        15, 1, 5000, 5, 1,
        NULL,
        "Pulldown", "Threshold32Pulldown", NULL,
    },
    {
        "Threshold 3:2 Pulldown Mismatch", SLIDER, 0, (long*)&ThresholdPulldownMismatch,
        100, 1, 10000, 10, 1,
        NULL,
        "Pulldown", "ThresholdPulldownMismatch", NULL,
    },
    {
        "Threshold 3:2 Pulldown Comb", SLIDER, 0, (long*)&ThresholdPulldownComb,
        300, 1, 5000, 10, 1,
        NULL,
        "Pulldown", "ThresholdPulldownComb", NULL,
    },
    {
        "Bad Pulldown Filter", ONOFF, 0, (long*)&bFallbackToVideo,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Pulldown", "bFallbackToVideo", NULL,
    },
    {
        "Pulldown Switch Interval", SLIDER, 0, (long*)&PulldownSwitchInterval,
        3000, 0, 10000, 10, 1,
        NULL,
        "Pulldown", "PulldownSwitchInterval", NULL,

    },
    {
        "Pulldown Switch Max", SLIDER, 0, (long*)&PulldownSwitchMax,
        4, 0, 100, 10, 1,
        NULL,
        "Pulldown", "PulldownSwitchMax", NULL,

    },
    {
        "Max Calls to Comb Method", SLIDER, 0, (long*)&MaxCallsToNTSCComb,
        20, 0, 1000, 10, 1,
        NULL,
        "Pulldown", "MaxCallsToComb", NULL,

    },
    {
        "NTSC Bad Cadence Mode", ITEMFROMLIST, 0, (long*)&NTSCBadCadenceIndex,
        INDEX_VIDEO_GREEDY, 0, 99, 1, 1,
        DeinterlaceNames,
        "Pulldown", "NTSCFilmFallbackMode", NULL,
    },
};

SETTING* FD60_GetSetting(FD60_SETTING Setting)
{
    if(Setting > -1 && Setting < FD60_SETTING_LASTONE)
    {
        return &(FD60Settings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void FD60_ReadSettingsFromIni()
{
    int i;
    for(i = 0; i < FD60_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(FD60Settings[i]));
    }
}

void FD60_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < FD60_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(FD60Settings[i]), bOptimizeFileAccess);
    }
}

void FD60_SetMenu(HMENU hMenu)
{
    CheckMenuItemBool(hMenu, IDM_FALLBACK, bFallbackToVideo);
}

void FD60_ShowUI()
{
    CSettingsDlg::ShowSettingsDlg("3:2 Pulldown Settings",FD60Settings, FD60_SETTING_LASTONE);
}
