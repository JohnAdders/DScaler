/////////////////////////////////////////////////////////////////////////////
// $Id: Source.h,v 1.28 2003-01-07 23:27:04 laurentg Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
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

#ifndef __SOURCE_H___
#define __SOURCE_H___

#include "DS_ApiCommon.h"
#include "Events.h"
#include "Setting.h"
#include "TVFormats.h"
#include "ITuner.h"
#include "TreeSettingsGeneric.h"

enum eSourceInputType
{
    VIDEOINPUT = 0,
    AUDIOINPUT,
};


class ISetting;

/** Abstract interface for Source.
    This class abstracts a thing that produces images.
    It allows the control over some standard properties and access
    to other source specific ones.
    The key function is GetNextField which fills the info structure with
    the most recent data
*/
class CSource : public CSettingsHolder, public CEventObject
{
public:
    /// start capturing.  Perform any initilization here
    virtual void Start() = 0;
    /// start capturing.  Perform any clean-up here
    virtual void Stop() = 0;
    /// Reset to known state
    virtual void Reset() = 0;
    /** Fill the TDeinteralceInfo structure with the most recent data
        and update the history.  The Accuarate timing flag is used by
        JudderTermonator to tell the code to time the input
        Frequency exactly.
    */
    virtual void GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming) = 0;
    /// Returns TRUE is we process the command
    virtual BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam) = 0;
    /// Returns a string to be displayed in the UI
    virtual LPCSTR GetStatus() = 0;
    /// Get the exact freqency of the input
    double GetFieldFrequency();
    /// Turn off any sound
    virtual void Mute() = 0;
    /// Turn back on the sound
    virtual void UnMute() = 0;
    /// returns NULL if there is no control over this value
    virtual ISetting* GetVolume() = 0;
    /// returns NULL if there is no control over this value
    virtual ISetting* GetBalance() = 0;
    /// returns NULL if there is no control over this value
    virtual ISetting* GetBrightness() = 0;
    /// returns NULL if there is no control over this value
    virtual ISetting* GetContrast() = 0;
    /// returns NULL if there is no control over this value
    virtual ISetting* GetHue() = 0;
    /// returns NULL if there is no control over this value
    virtual ISetting* GetSaturation() = 0;
    /// returns NULL if there is no control over this value
    virtual ISetting* GetSaturationU() = 0;
    /// returns NULL if there is no control over this value
    virtual ISetting* GetSaturationV() = 0;
    /// returns NULL if there is no control over this value
    virtual ISetting* GetTopOverscan() = 0;
    /// returns NULL if there is no control over this value
    virtual ISetting* GetBottomOverscan() = 0;
    /// returns NULL if there is no control over this value
    virtual ISetting* GetLeftOverscan() = 0;
    /// returns NULL if there is no control over this value
    virtual ISetting* GetRightOverscan() = 0;
	/// Get the tuner Id - this method may (and should) be overriden
    virtual eTunerId GetTunerId() 
    {
        ITuner* tuner = GetTuner();
        if (NULL == tuner)
        {
            return TUNER_ABSENT;
        }
        return tuner->GetTunerId();
    }

    /// Get the original video format e.g. PAL, NTSC
    virtual eVideoFormat GetFormat() = 0;
    /// Are we currently showing the output from the tuner
    virtual BOOL IsInTunerMode() = 0;
    /// Gets the width of each picture in pixels
    virtual int GetWidth() = 0;
    /// Gets the height of each frame in pixels, for interlaced this is x2 FieldHeight
    virtual int GetHeight() = 0;
    /// Does this source have an anologue tuner
    //(kept for backward compatibility)
    /*virtual*/ BOOL HasTuner();    

    /// Update the content of the menu
    virtual void UpdateMenu() = 0;
    /// Update the menu with any settings
    virtual void SetMenu(HMENU hMenu) = 0;
    /// Handle any timers that you control
    virtual void HandleTimerMessages(int TimerId) = 0;
    /** Set the video format e.g. PAL, NTSC
        Will get called by auto forfat detect in the processing thread
        so do not stop the processing to habdle this command
        Normal processing is to post message to yourself using API

    */
    virtual void SetFormat(eVideoFormat NewFormat) = 0;
    /// Tune into channel
    virtual BOOL SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat) = 0;
    /// Are we capturing a proper video signal
    virtual BOOL IsVideoPresent() = 0;
    /// If VBI data is being captured, decode it.
    virtual void DecodeVBI(TDeinterlaceInfo* pInfo) = 0;
    /// Get string to display in source menu
    virtual LPCSTR GetMenuLabel() = 0;
    /// Open the following file, return TRUE if you understand the file
    virtual BOOL OpenMediaFile(LPCSTR FileName, BOOL NewPlayList) = 0;
    /// Returns the Source specific menu to display when this source is selected
    HMENU GetSourceMenu();
    // Is access to this source allowed
    virtual BOOL IsAccessAllowed() = 0;
    virtual void SetOverscan() = 0;
    virtual BOOL HasSquarePixels() = 0;
    virtual void ChangeSettingsBasedOnHW(int ProcessorSpeed, int TradeOff) = 0;
    virtual LPCSTR IDString() = 0;
    char* GetComments();
      
    virtual int  NumInputs(eSourceInputType InputType) = 0;
    virtual BOOL SetInput(eSourceInputType InputType, int Nr) = 0;
    virtual int  GetInput(eSourceInputType InputType) = 0;
    virtual const char* GetInputName(eSourceInputType InputType, int Nr) = 0;
    virtual BOOL InputHasTuner(eSourceInputType InputType, int Nr) = 0;

    virtual ITuner* GetTuner() = 0;

    /// returns NULL if there is no tree settings page
    virtual CTreeSettingsPage* GetTreeSettingsPage() = 0;

protected:
    CSource(long SetMessage, long MenuId);
    ~CSource();

	/**
	 * Notify dscaler that the input size has changed.
	 * Call this function when the width and/or height has changed.
	 */
    void NotifySizeChange();

    void NotifySquarePixelsCheck();

    double m_FieldFrequency;
    HMENU m_hMenu;
    std::string m_Comments;
};



#endif