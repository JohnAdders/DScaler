////////////////////////////////////////////////////////////////////////////
// $Id: DScaler.cpp,v 1.339 2003-08-14 19:38:14 laurentg Exp $
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
//
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           Original Release
//                                     Translated most code from German
//                                     Combined Header files
//                                     Cut out all decoding
//                                     Cut out digital hardware stuff
//
// 21 Dec 2000   John Adcock           Stopped Timer after ini write
//
// 26 Dec 2000   Eric Schmidt          Fixed remember-last-audio-input-at-start.
//
// 02 Jan 2001   John Adcock           Added pVBILines
//                                     Removed bTV plug-in
//                                     Added Scaled BOB method
//
// 03 Jan 2001   Michael Eskin         Added MSP muting
//
// 07 Jan 2001   John Adcock           Added Adaptive deinterlacing
//                                     Changed display and handling of
//                                     change deinterlacing method
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 20 Feb 2001   Michael Samblanet     Added bounce timer - see AspectRatio.c
//                                     Corrected bug in SaveWindowPos - length 
//                                     not set in placement structure
//
// 23 Feb 2001   Michael Samblanet     Added orbit timer, Expierementaly removed
//                                     2 lines from WM_PAINT which should not be
//                                     needed and may have caused flashing.
//
// 31 Mar 2001   Laurent Garnier       Single click replaced by double click
//
// 04 Apr 2001   Laurent Garnier       Automatic hide cursor
//
// 26 May 2001   Eric Schmidt          Added Custom Channel Order.
//
// 24 Jul 2001   Eric Schmidt          Added TimeShift stuff.
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.338  2003/08/12 19:11:33  laurentg
// Move some methods from CDSFileSource to CDSSourceBase
//
// Revision 1.337  2003/08/11 23:03:44  laurentg
// Time slider in the media player toolbar
//
// Revision 1.336  2003/08/09 20:18:37  laurentg
// Automatic show/hide the toolbar when in full screen mode
//
// Revision 1.335  2003/08/09 15:53:39  laurentg
// Bad refresh of the toolbar when in full screen mode corrected
//
// Revision 1.334  2003/08/09 13:03:09  laurentg
// Display of the toolbar in full screen mode
//
// Revision 1.333  2003/08/02 12:04:13  laurentg
// Two new settings to define how many channels to display in preview mode
//
// Revision 1.332  2003/07/29 13:33:06  atnak
// Overhauled mixer code
//
// Revision 1.331  2003/07/24 21:15:18  laurentg
// Hide the toolbar when starting in full screen mode
//
// Revision 1.330  2003/07/18 09:48:48  adcockj
// Added Timer clean up
//
// Revision 1.329  2003/07/18 09:41:23  adcockj
// Added PDI input to holo3d (doesn't yet work)
//
// Revision 1.328  2003/07/02 20:35:12  laurentg
// Allow virtual screen with origin different from (0,0)
//
// Revision 1.327  2003/06/14 19:38:10  laurentg
// Preview mode improved
//
// Revision 1.326  2003/06/14 12:05:21  laurentg
// Restore default position (on primary monitor) for the window if the old position was on a screen which is no more active
//
// Revision 1.325  2003/05/31 11:38:14  laurentg
// Load dynamic functions earlier to have splash screen on correct screen and to be able to start in full screen mode on the second monitor
//
// Revision 1.324  2003/04/28 12:41:19  laurentg
// PowerStrip settings access added
//
// Revision 1.323  2003/04/26 19:02:38  laurentg
// Character string settings and memory management
//
// Revision 1.322  2003/04/26 16:07:48  laurentg
// Character string settings
//
// Revision 1.321  2003/04/16 14:38:01  atnak
// Changed the double click hack to be less hacky
//
// Revision 1.320  2003/04/15 13:07:09  adcockj
// Fixed memory leak
//
// Revision 1.319  2003/04/12 15:23:22  laurentg
// Interface with PowerStrip when changing resolution (code from Olivier Borca)
//
// Revision 1.318  2003/03/29 13:39:33  laurentg
// Allow the display of DScaler to monitors other than the primary
//
// Revision 1.317  2003/03/23 09:24:27  laurentg
// Automatic leave preview mode when necessary
//
// Revision 1.316  2003/03/22 18:58:38  laurentg
// New key to switch to or from preview mode
// Spped up initial display of channels in preview mode
//
// Revision 1.315  2003/03/21 22:48:06  laurentg
// Preview mode (multiple frames) improved
//
// Revision 1.314  2003/03/19 23:56:35  laurentg
// Second step for the navigation through channels in preview mode
//
// Revision 1.313  2003/03/17 22:36:13  laurentg
// First step for the navigation through channels in preview mode
//
// Revision 1.312  2003/03/16 18:31:25  laurentg
// New multiple frames feature
//
// Revision 1.311  2003/03/08 20:50:58  laurentg
// Updated combobox to select the usage of DScaler
//
// Revision 1.310  2003/02/27 01:37:51  atnak
// Fixes skin problem introduced when paint code was changed.
//
// Revision 1.309  2003/02/08 13:16:47  laurentg
// Change resolution in full screen mode slightly updated
//
// Revision 1.308  2003/02/07 12:46:17  laurentg
// Change resolution correctly handled when DScaler is minimized and restored
//
// Revision 1.307  2003/02/07 11:28:23  laurentg
// Keep more ids for the output reso menus (100)
// New resolutions added (720x480 and 720x576)
//
// Revision 1.306  2003/02/06 12:22:56  laurentg
// Take the refresh rate when changing resolution (choice between 60, 72, 75, 100 and 120 Hz)
//
// Revision 1.305  2003/02/06 09:59:40  laurentg
// Change resolution in full screen
//
// Revision 1.304  2003/02/06 00:58:50  laurentg
// Change output resolution (first step)
//
// Revision 1.303  2003/02/05 19:57:50  laurentg
// New option to minimize DScaler when there is no signal and to restore it when a signal is detected
//
// Revision 1.302  2003/02/05 17:50:51  robmuller
// Add the systray icon again after the task bar has been restarted.
//
// Revision 1.301  2003/02/05 17:21:50  robmuller
// Hide systray menu when another window is activated.
//
// Revision 1.300  2003/02/05 16:40:17  laurentg
// New option to stop capture when DScaler is minimized
//
// Revision 1.299  2003/02/05 15:11:39  laurentg
// Channel name as tip for the DScaler icon in the systray (patch from Kristian Trenskow)
//
// Revision 1.298  2003/02/05 14:40:56  laurentg
// DScaler in Windows system tray (patch from Kristian Trenskow)
//
// Revision 1.297  2003/02/05 14:26:19  laurentg
// DScaler in systray (Patch from Kristian Trenskow)
//
// Revision 1.296  2003/01/27 22:06:39  laurentg
// Forbid key O or key Shift+O as soon as one of the overscans is at its minimum or maximum
//
// Revision 1.295  2003/01/27 16:40:13  adcockj
// Fixed maximize bug spotted by Atsushi
//
// Revision 1.294  2003/01/26 10:34:57  tobbej
// changed statusbar updates from output thread to be thread safe (PostMessage instead of SendMessage)
//
// Revision 1.293  2003/01/26 03:46:30  atnak
// Fixed no refresh after videotext setting change
//
// Revision 1.292  2003/01/25 12:03:45  atnak
// Changed OSD RECT from GetDestRect() to GetDisplayAreaRect(),
// re-implemented IDM_FAST_REPAINT
//
// Revision 1.291  2003/01/24 01:55:18  atnak
// OSD + Teletext conflict fix, offscreen buffering for OSD and Teletext,
// got rid of the pink overlay colorkey for Teletext.
//
// Revision 1.290  2003/01/18 12:10:48  laurentg
// Avoid double display in OSD (ADJUSTDOWN_SILENT and ADJUSTUP_SILENT instead of (ADJUSTDOWN and ADJUSTUP)
//
// Revision 1.289  2003/01/17 17:26:52  adcockj
// reverted writesettings change
//
// Revision 1.288  2003/01/17 14:40:33  adcockj
// Write all settings on exit
//
// Revision 1.287  2003/01/16 22:34:21  laurentg
// First step to add a new dialog box to adjust image size
//
// Revision 1.286  2003/01/16 16:55:44  adcockj
// Added new credits dialog
//
// Revision 1.285  2003/01/15 15:54:22  adcockj
// Fixed some keyboard focus issues
//
// Revision 1.284  2003/01/12 17:12:45  atnak
// Added hex pages display and goto dialog
//
// Revision 1.283  2003/01/12 16:19:34  adcockj
// Added SettingsGroup activity setting
// Corrected event sequence and channel change behaviour
//
// Revision 1.282  2003/01/11 15:22:25  adcockj
// Interim Checkin of setting code rewrite
//  - Remove CSettingsGroupList class
//  - Fixed bugs in format switching
//  - Some new CSettingGroup code
//
// Revision 1.281  2003/01/10 17:37:56  adcockj
// Interrim Check in of Settings rewrite
//  - Removed SETTINGSEX structures and flags
//  - Removed Seperate settings per channel code
//  - Removed Settings flags
//  - Cut away some unused features
//
// Revision 1.280  2003/01/09 21:43:14  laurentg
// Menu AspectRatio restored
//
// Revision 1.279  2003/01/08 22:46:57  laurentg
// OSD display when incrementing or decrementing overscan
//
// Revision 1.278  2003/01/08 22:01:33  robmuller
// Fixed problem with multi-line OSD messages at default size.
//
// Revision 1.277  2003/01/08 20:22:17  laurentg
// Display the 4 values for overscan in OSD
//
// Revision 1.276  2003/01/07 23:27:02  laurentg
// New overscan settings
//
// Revision 1.275  2003/01/07 13:40:59  robmuller
// Added Help menu entry to the popup menu.
//
// Revision 1.274  2003/01/05 16:09:45  atnak
// Updated TopText for new teletext
//
// Revision 1.273  2003/01/05 10:30:08  atnak
// Added Cursor_IsOurs()  --fixes cursor hiding/changing problems
//
// Revision 1.272  2003/01/02 20:06:20  atnak
// Fixed teletext menu.
//
// Revision 1.271  2003/01/02 19:34:09  robmuller
// Fixed teletext menu.
//
// Revision 1.270  2003/01/02 18:58:15  adcockj
// Removed comment no longer required
//
// Revision 1.269  2003/01/02 11:05:24  atnak
// Added missing InitialTextPage implementation
//
// Revision 1.268  2003/01/01 22:01:44  atnak
// Added OSD message for VideoText off
//
// Revision 1.267  2003/01/01 20:58:30  atnak
// New code for new videotext + videotext reorganziations, + fixes
// cursor not redrawing on Cursor_SetType()
//
// Revision 1.266  2002/12/15 15:19:21  adcockj
// Fixed a crash on exit
//
// Revision 1.265  2002/12/09 00:32:14  atnak
// Added new muting stuff
//
// Revision 1.264  2002/12/07 16:06:54  adcockj
// Tidy up muting code
//
// Revision 1.263  2002/12/07 15:59:06  adcockj
// Modified mute behaviour
//
// Revision 1.262  2002/12/02 17:06:15  adcockj
// Changed Events to use messages instead of timer
//
// Revision 1.261  2002/11/03 06:00:29  atnak
// Added redrawing the menu bar when it changes
//
// Revision 1.260  2002/10/30 13:37:52  atnak
// Added "Single key teletext toggle" option. (Enables mixed mode menu item)
//
// Revision 1.259  2002/10/29 11:05:28  adcockj
// Renamed CT2388x to CX2388x
//
// Revision 1.258  2002/10/27 12:18:51  laurentg
// New setting to define the number of consecutive stills
//
// Revision 1.257  2002/10/27 04:28:42  atnak
// Fixed cursor toggled hidden not showing for menus and dialogs
//
// Revision 1.256  2002/10/26 21:42:04  laurentg
// Take consecutive stills
//
// Revision 1.255  2002/10/26 17:51:52  adcockj
// Simplified hide cusror code and removed PreShowDialogOrMenu & PostShowDialogOrMenu
//
// Revision 1.254  2002/10/26 16:36:41  atnak
// Made DisableScreensaver disable monitor sleeping
//
// Revision 1.253  2002/10/24 12:10:39  atnak
// Fixed up Reverse Channel Scrolling in Other Settings
//
// Revision 1.252  2002/10/22 18:51:37  adcockj
// Added logging of windows messages at level 3
//
// Revision 1.251  2002/10/22 01:54:04  atnak
// fixed the places where I didn't call ReleaseDC after GetDC
//
// Revision 1.250  2002/10/21 07:29:52  adcockj
// Moved new scroll wheel option to end of list of settings
//
// Revision 1.249  2002/10/21 00:12:30  atnak
// Added comments for lastest change
//
// Revision 1.248  2002/10/20 23:51:12  atnak
// Added option to reverse mousewheel
//
// Revision 1.247  2002/10/20 21:51:30  laurentg
// Don't show window border when in full screen mode
//
// Revision 1.246  2002/10/15 18:14:36  kooiman
// Added 'use overlay controls' to Overlay settings dialog.
//
// Revision 1.245  2002/10/15 15:26:09  kooiman
// Added include for settingsmaster.h
//
// Revision 1.244  2002/10/15 11:53:38  atnak
// Added UI feedback for some videotext stuff
//
// Revision 1.243  2002/10/08 13:23:19  adcockj
// Actually reverted to old minimize behaviour
//
// Revision 1.242  2002/10/08 12:12:35  adcockj
// Changed minimize behaviour back to how it was
//
// Revision 1.241  2002/10/08 08:23:32  kooiman
// Fixed lost border buttons.
//
// Revision 1.240  2002/10/07 20:34:48  kooiman
// Fixed cursor hide problem & window region problems.
//
// Revision 1.239  2002/10/07 16:09:21  adcockj
// Stop processing and release overlay on minimize
//
// Revision 1.238  2002/10/04 11:40:08  adcockj
// Removed skin and toolbar create
//
// Revision 1.237  2002/10/02 19:31:05  adcockj
// Removed need to get Video Input menu
//
// Revision 1.236  2002/10/02 18:39:23  robmuller
// Fixed problem name issue.
//
// Revision 1.235  2002/09/30 16:23:44  adcockj
// Moved number handling code out to a routine
//
// Revision 1.234  2002/09/29 17:52:04  adcockj
// Try again with cursor fix
//
// Revision 1.233  2002/09/29 17:40:04  adcockj
// Fix for cursor not disappearing at startup
//
// Revision 1.232  2002/09/29 13:56:30  adcockj
// Fixed some cursor hide problems
//
// Revision 1.231  2002/09/28 18:20:28  robmuller
// Fixed a problem caused by the renaming of a menu entry.
// Added a check to detect this error earlier next time.
//
// Revision 1.230  2002/09/28 13:34:08  kooiman
// Added sender object to events and added setting flag to treesettingsgeneric.
//
// Revision 1.229  2002/09/27 14:11:35  kooiman
// Added audio standard detect event & implemented event scheduler.
//
// Revision 1.228  2002/09/26 16:34:19  kooiman
// Lots of toolbar fixes &added EVENT_VOLUME support.
//
// Revision 1.227  2002/09/26 06:11:57  kooiman
// Added toolbar, skin & event collector.
//
// Revision 1.226  2002/09/25 15:11:12  adcockj
// Preliminary code for format specific support for settings per channel
//
// Revision 1.225  2002/09/24 17:21:07  tobbej
// fixed osd flashing when changing volume
//
// Revision 1.224  2002/09/18 11:38:05  kooiman
// Preparations for skinned dscaler look.
//
// Revision 1.223  2002/09/17 17:28:24  tobbej
// updated crashloging to same version as in latest virtualdub
//
// Revision 1.222  2002/09/16 19:34:19  adcockj
// Fix for auto format change
//
// Revision 1.221  2002/09/11 18:19:38  adcockj
// Prelimainary support for CX2388x based cards
//
// Revision 1.220  2002/08/13 21:19:18  kooiman
// Moved settings per channel setup before first channel change.
//
// Revision 1.219  2002/08/12 19:59:04  laurentg
// Selection of video card to adjust DScaler settings
//
// Revision 1.218  2002/08/11 19:39:30  robmuller
// Corrected  menu item number in GetPatternsSubmenu().
//
// Revision 1.217  2002/08/11 16:14:36  laurentg
// New setting to choose between keep CPU for other applications or use full CPU for best results
//
// Revision 1.216  2002/08/11 13:52:02  laurentg
// Show automatically the general hardware setup dialog box the first time DScaler is started
//
// Revision 1.215  2002/08/11 12:12:10  laurentg
// Cut BT Card setup and general hardware setup in two different windows
//
// Revision 1.214  2002/08/09 13:33:24  laurentg
// Processor speed and trade off settings moved from BT source settings to DScaler settings
//
// Revision 1.213  2002/08/08 21:16:24  kooiman
// Add call to free memory for settings per channel.
//
// Revision 1.212  2002/08/08 12:23:18  kooiman
// Added channel settings setup calls.
//
// Revision 1.211  2002/08/08 10:34:23  robmuller
// Updated command line message example.
//
// Revision 1.210  2002/08/08 10:31:21  robmuller
// Fixed problem when command line OSD message contains quotes.
//
// Revision 1.209  2002/08/07 13:13:27  robmuller
// Press x to clear OSD.
//
// Revision 1.208  2002/08/07 12:43:40  robmuller
// Send messages to the OSD with the command line.
//
// Revision 1.207  2002/08/05 21:01:56  laurentg
// Square pixels mode updated
//
// Revision 1.206  2002/08/04 12:29:02  kooiman
// Fixed previous channel feature.
//
// Revision 1.205  2002/08/02 21:59:03  laurentg
// Hide the menu "Channels" from the menu bar when the source has no tuner or when the tuner is not the selected input
//
// Revision 1.204  2002/08/02 18:56:27  robmuller
// 'EasyMove' feature added.
//
// Revision 1.203  2002/07/31 20:23:54  laurentg
// no message
//
// Revision 1.202  2002/07/30 21:20:59  laurentg
// Merge of menus View, AspectRatio and OSD
//
// Revision 1.201  2002/07/29 21:33:06  laurentg
// "Show Video Method UI" feature restored
//
// Revision 1.200  2002/07/27 16:27:35  laurentg
// Deinterlace and Settings menus updated
//
// Revision 1.199  2002/07/27 15:20:34  laurentg
// Channels menu updated
//
// Revision 1.198  2002/07/27 13:52:06  laurentg
// Distinguish menu entries for filter settings, video modes settings and advanced settings
// Connect again the patterns menu
//
// Revision 1.197  2002/07/26 22:40:55  laurentg
// Menus updates
//
// Revision 1.196  2002/07/24 21:43:15  laurentg
// Take cyclic stills
//
// Revision 1.195  2002/07/20 13:07:36  laurentg
// New setting for vertical mirror
//
// Revision 1.194  2002/07/20 10:33:06  laurentg
// New settings to select the wished OSD screens
//
// Revision 1.193  2002/07/19 15:31:38  laurentg
// New settings (other settings) added in the tree settings + related menu items deleted
//
// Revision 1.192  2002/07/19 13:02:32  laurentg
// OSD menu simplified (one depth level less)
//
// Revision 1.191  2002/07/19 12:04:51  laurentg
// Auto hide (OSD) menu deleted
//
// Revision 1.190  2002/07/05 20:52:54  laurentg
// Thread priority settings
//
// Revision 1.189  2002/07/03 00:45:41  laurentg
// Add a new section in the Change Settings dialog box to set the thread priorities
//
// Revision 1.188  2002/06/30 20:10:15  laurentg
// Mouse wheel + CTRL key to go to previous and next playlist file
//
// Revision 1.187  2002/06/30 00:33:39  robmuller
// Change volume with the mousewheel when the right mouse button is down.
//
// Revision 1.186  2002/06/25 22:41:29  robmuller
// Fixed: entering digits in non-tuner mode causes weird behaviour.
//
// Revision 1.185  2002/06/23 20:51:13  laurentg
// Attachment of test patterns menu restored
//
// Revision 1.184  2002/06/22 21:50:47  robmuller
// Generate a valid dscaler.ini at startup.
//
// Revision 1.183  2002/06/22 15:06:30  laurentg
// New vertical flip mode
//
// Revision 1.182  2002/06/20 20:00:37  robmuller
// Implemented videotext search highlighting.
//
// Revision 1.181  2002/06/18 23:12:41  robmuller
// Fixed: context menu not working.
//
// Revision 1.180  2002/06/18 19:46:06  adcockj
// Changed appliaction Messages to use WM_APP instead of WM_USER
//
// Revision 1.179  2002/06/14 21:20:10  robmuller
// Enter zero's to switch video input.
// Fixed: direct switching to channel number > 99 is not possible.
//
// Revision 1.178  2002/06/13 12:10:21  adcockj
// Move to new Setings dialog for filers, video deint and advanced settings
//
// Revision 1.177  2002/06/13 11:24:32  robmuller
// Channel enter time is now configurable.
//
// Revision 1.176  2002/06/13 10:40:37  robmuller
// Made anti plop mute delay configurable.
//
// Revision 1.175  2002/06/13 09:23:02  robmuller
// Update name of the source in the context menu.
//
// Revision 1.174  2002/06/12 23:57:13  robmuller
// Fixed OSD entry on context menu. Added code to prevent similar errors in the future.
//
// Revision 1.173  2002/06/12 20:16:33  robmuller
// Mousewheel + shift changes volume.
//
// Revision 1.172  2002/06/06 21:40:00  robmuller
// Fixed: timeshifting and VBI data decoding was not done when minimized.
//
// Revision 1.171  2002/06/06 18:17:31  robmuller
// Change to prevent (un)installation with InnoSetup when DScaler is running.
//
// Revision 1.170  2002/06/01 22:24:36  laurentg
// New calibration mode to compute YUV range
//
// Revision 1.169  2002/05/30 21:47:21  robmuller
// Unmute sound on volume plus/minus.
//
// Revision 1.168  2002/05/30 19:48:04  robmuller
// Unhide cursor when moving outside client area.
//
// Revision 1.167  2002/05/30 19:09:46  robmuller
// Redraw screen after Videotext Reset.
//
// Revision 1.166  2002/05/30 13:06:41  robmuller
// Removed variable bIgnoreMouse.
//
// Revision 1.165  2002/05/30 12:58:28  robmuller
// Prevent bogus WM_MOUSEMOVE messages to unhide the cursor.
//
// Revision 1.164  2002/05/29 18:44:54  robmuller
// Added option to disable font anti-aliasing in Teletext.
//
// Revision 1.163  2002/05/28 08:54:07  robmuller
// Fixed broken OSD menu.
// Added ASSERTs to prevent similar errors in the future.
//
// Revision 1.162  2002/05/27 20:17:05  robmuller
// Patch #561180  by PietOO:
// Autodetection of teletext code page.
//
// Revision 1.161  2002/05/26 10:33:35  robmuller
// Screen redraw problem fixed.
//
// Revision 1.160  2002/05/26 09:21:48  robmuller
// Patch #560680 by PietOO:
// Added option to disable screensaver.
//
// Revision 1.159  2002/05/24 18:22:46  robmuller
// Turn off VideoText when VideoText capturing is disabled.
//
// Revision 1.158  2002/05/24 16:49:00  robmuller
// VideoText searching improved.
//
// Revision 1.157  2002/05/24 15:12:12  tobbej
// changed timer status updates to use strncpy insted of strcpy
//
// Revision 1.156  2002/05/24 10:52:58  robmuller
// Applied patch #559718 by PietOO.
// SleepTimer implementation.
//
// Revision 1.155  2002/05/23 18:45:03  robmuller
// Patch #559554 by PietOO.
// Teletext: + text search ctrl-F & next F3
//
// Revision 1.154  2002/05/20 16:41:16  robmuller
// Prevent channel changing when in videotext mode.
//
// Revision 1.153  2002/05/20 16:32:12  robmuller
// Instantaneous channel switching when entering digits.
//
// Revision 1.152  2002/05/19 22:19:20  robmuller
// Pause capture when minimized.
//
// Revision 1.151  2002/05/06 15:34:59  laurentg
// Key <i> to show source informations through OSD
//
// Revision 1.150  2002/05/01 20:34:10  tobbej
// generate crashlog if crashing in MainWndProc
//
// Revision 1.149  2002/04/27 16:02:59  laurentg
// Initial source
//
// Revision 1.148  2002/04/24 19:10:38  tobbej
// test of new tree based setting dialog
//
// Revision 1.147  2002/04/15 22:50:08  laurentg
// Change again the available formats for still saving
// Automatic switch to "square pixels" AR mode when needed
//
// Revision 1.146  2002/04/13 18:56:22  laurentg
// Checks added to manage case where the current source is not yet defined
//
// Revision 1.145  2002/04/06 11:46:46  laurentg
// Check that the current source is not NULL to avoid DScaler exits
//
// Revision 1.144  2002/03/24 18:56:45  adcockj
// Fix for comamnd line ini file
//
// Revision 1.143  2002/03/21 10:26:57  robmuller
// Don't show overlay adjustments dialog if the system does not support it.
//
// Revision 1.142  2002/03/21 08:34:07  robmuller
// Added last line ("Program exit") to log file.
//
// Revision 1.141  2002/03/12 23:29:45  robmuller
// Implemented functions VT_GetNextPage() and VT_GetPreviousPage().
//
// Revision 1.140  2002/03/11 22:25:56  robmuller
// Added hand cursor.
//
// Revision 1.139  2002/02/28 11:27:03  temperton
// Preserve WS_DISABLED window style in UpdateWindowState
//
// Revision 1.138  2002/02/27 20:47:21  laurentg
// Still settings
//
// Revision 1.137  2002/02/24 19:08:37  laurentg
// OSD text when resetting statistics
//
// Revision 1.136  2002/02/24 08:18:03  temperton
// TIMER_VTFLASHER set only when displayed page contains flashed elements and only in teletext modes.
//
// Revision 1.135  2002/02/23 16:43:13  laurentg
// Timer TIMER_STATUS killed when status bar is not displayed
//
// Revision 1.134  2002/02/23 00:37:15  laurentg
// AR statistics included in user's action to reset statistics
// AR statistics reseted at the startup of the decoding thread
//
// Revision 1.133  2002/02/19 16:03:36  tobbej
// removed CurrentX and CurrentY
// added new member in CSource, NotifySizeChange
//
// Revision 1.132  2002/02/18 23:28:05  laurentg
// Overlay settings dialog box updated
// New menu item to choose between DScaler overlay settings and external overlay settings
//
// Revision 1.131  2002/02/18 20:51:51  laurentg
// Statistics regarding deinterlace modes now takes into account the progressive mode
// Reset of the deinterlace statistics at each start of the decoding thread
// Reset action now resets the deinterlace statistics too
//
// Revision 1.130  2002/02/17 21:41:03  laurentg
// Action "Find and Lock Film mode" added
//
// Revision 1.129  2002/02/17 20:32:34  laurentg
// Audio input display suppressed from the OSD main screen
// GetStatus modified to display the video input name in OSD main screen even when there is no signal
//
// Revision 1.128  2002/02/11 21:28:19  laurentg
// Popup menu updated
//
// Revision 1.127  2002/02/10 21:38:04  laurentg
// Default value for "Autohide Cursor" is now ON
//
// Revision 1.126  2002/02/09 15:30:19  laurentg
// Card calibration menus revisited.
//
// Revision 1.125  2002/02/09 13:08:41  laurentg
// New menu items to access to UI for calibration and OSD settings
//
// Revision 1.124  2002/02/09 02:44:56  laurentg
// Overscan now stored in a setting of the source
//
// Revision 1.123  2002/02/08 08:14:21  adcockj
// Select saved channel on startup if in tuner mode
//
// Revision 1.122  2002/02/07 13:04:54  temperton
// Added Spanish and Polish teletext code pages. Thanks to Jazz (stawiarz).
//
// Revision 1.121  2002/02/03 22:48:21  robmuller
// Added command line parameters /driverinstall and /driveruninstall.
//
// Revision 1.120  2002/02/03 10:31:22  tobbej
// fixed so its posibel to open popup menu from keyboard
//
// Revision 1.119  2002/02/02 01:31:18  laurentg
// Access to the files of the playlist added in the menus
// Save Playlist added
// "Video Adjustments ..." restored in the popup menu
//
// Revision 1.118  2002/01/31 18:07:15  robmuller
// Fixed crash when using command line parameter /c.
//
// Revision 1.117  2002/01/31 13:02:46  robmuller
// Improved accuracy and reliability of the performance statistics.
//
// Revision 1.116  2002/01/24 00:00:13  robmuller
// Added bOptimizeFileAccess flag to WriteToIni from the settings classes.
//
// Revision 1.115  2002/01/22 14:50:10  robmuller
// Added keyboard lock option.
//
// Revision 1.114  2002/01/20 10:05:02  robmuller
// On channel setup prevent switch to tuner mode if already in tuner mode.
//
// Revision 1.113  2002/01/20 09:59:32  robmuller
// In tuner mode STATUS_TEXT in statusbar shows channel number if channel name is not available.
//
// Revision 1.112  2002/01/19 12:53:00  temperton
// Teletext pages updates at correct time.
// Teletext can use variable-width font.
//
// Revision 1.111  2002/01/16 19:02:17  adcockj
// Fixed window style and context menu fullscreen check
//
// Revision 1.110  2002/01/15 19:53:36  adcockj
// Fix for window creep with toolbar at top or left
//
// Revision 1.109  2002/01/15 11:16:03  temperton
// New teletext drawing code.
//
// Revision 1.108  2002/01/12 16:56:21  adcockj
// Series of fixes to bring 4.0.0 into line with 3.1.1
//
// Revision 1.107  2001/12/22 13:18:04  adcockj
// Tuner bugfixes
//
// Revision 1.106  2001/12/18 13:12:11  adcockj
// Interim check-in for redesign of card specific settings
//
// Revision 1.105  2001/12/16 18:40:28  laurentg
// Reset statistics
//
// Revision 1.104  2001/12/16 16:31:43  adcockj
// Bug fixes
//
// Revision 1.103  2001/12/16 13:13:34  laurentg
// New statistics
//
// Revision 1.102  2001/12/08 14:22:19  laurentg
// Bug fix regarding Sources submenu in the right mouse menu
//
// Revision 1.101  2001/12/08 13:43:20  adcockj
// Fixed logging and memory leak bugs
//
// Revision 1.100  2001/12/03 19:33:59  adcockj
// Bug fixes for settings and memory
//
// Revision 1.99  2001/12/03 17:14:42  adcockj
// Added command line patch from Arie van Wijngaarden
//
// Revision 1.98  2001/11/29 17:30:51  adcockj
// Reorgainised bt848 initilization
// More Javadoc-ing
//
// Revision 1.97  2001/11/29 14:04:06  adcockj
// Added Javadoc comments
//
// Revision 1.96  2001/11/28 16:04:50  adcockj
// Major reorganization of STill support
//
// Revision 1.95  2001/11/26 13:02:27  adcockj
// Bug Fixes and standards changes
//
// Revision 1.94  2001/11/26 12:48:01  temperton
// Teletext corrections
//
// Revision 1.93  2001/11/25 21:29:50  laurentg
// Take still, Open file, Close file callbacks updated
//
// Revision 1.92  2001/11/24 22:54:25  laurentg
// Close file added for still source
//
// Revision 1.91  2001/11/24 18:01:39  laurentg
// Still source
//
// Revision 1.90  2001/11/23 10:47:44  adcockj
// Added Hebrew and Hungarian codepages
//
// Revision 1.89  2001/11/22 13:32:03  adcockj
// Finished changes caused by changes to TDeinterlaceInfo - Compiles
//
// Revision 1.88  2001/11/19 14:02:48  adcockj
// Apply patches from Sandu Turcan
//
// Revision 1.87  2001/11/17 18:15:57  adcockj
// Bugfixes (including very silly performance bug)
//
// Revision 1.86  2001/11/14 11:28:03  adcockj
// Bug fixes
//
// Revision 1.85  2001/11/09 14:19:34  adcockj
// Bug fixes
//
// Revision 1.84  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.83  2001/11/02 16:30:07  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.82  2001/11/02 10:15:20  temperton
// Removed unnecessary painting of color key in middle part of screen in teletext mode.
//
// Revision 1.81  2001/10/25 12:59:48  temperton
// Fixed problem, when DScaler hangs on exit if we forgot to stop record.
//
// Revision 1.80  2001/10/22 05:55:07  temperton
// Teletext improvements
//
// Revision 1.79  2001/10/06 17:04:26  adcockj
// Fixed teletext crashing problems
//
// Revision 1.78  2001/10/06 12:36:10  laurentg
// New shortcut keys added to adjust left and right player cropping during calibration
//
// Revision 1.77  2001/10/04 12:39:16  adcockj
// Added Teletext colour buttons to UI and switch to using accelerator rather than keyup message
//
// Revision 1.76  2001/09/29 10:51:09  laurentg
// O and Shift+O to adjust specific overscan when in calibration mode
// Enter and Backspace to show and hide calibration OSD when in calibration mode
//
// Revision 1.75  2001/09/21 20:47:12  laurentg
// SaveStill modified to return the name of the written file
// Name of the file added in the OSD text when doing a snapshot
//
// Revision 1.74  2001/09/21 16:43:54  adcockj
// Teletext improvements by Mike Temperton
//
// Revision 1.73  2001/09/21 15:39:01  adcockj
// Added Russian and German code pages
// Corrected UK code page
//
// Revision 1.72  2001/09/11 12:03:52  adcockj
// Updated Help menu to go to help page
//
// Revision 1.71  2001/09/08 19:18:46  laurentg
// Added new specific dialog box to set the overlay settings
//
// Revision 1.70  2001/09/05 15:08:43  adcockj
// Updated Loging
//
// Revision 1.69  2001/09/05 06:59:12  adcockj
// Teletext fixes
//
// Revision 1.68  2001/09/03 13:46:06  adcockj
// Added PAL-NC thanks to Eduardo José Tagle
//
// Revision 1.67  2001/09/02 14:17:51  adcockj
// Improved teletext code
//
// Revision 1.66  2001/09/02 12:13:21  adcockj
// Changed dscaler webiste
// Tidied up resource spelling
//
// Revision 1.65  2001/08/24 21:36:46  adcockj
// Menu bug fix
//
// Revision 1.64  2001/08/23 18:54:21  adcockj
// Menu and Settings fixes
//
// Revision 1.63  2001/08/23 16:03:26  adcockj
// Improvements to dynamic menus to remove requirement that they are not empty
//
// Revision 1.62  2001/08/21 09:39:46  adcockj
// Added Greek teletext Codepage
//
// Revision 1.61  2001/08/16 21:17:34  laurentg
// Automatic calibration improved with a fine adjustment
//
// Revision 1.60  2001/08/15 17:50:11  laurentg
// UseRGB ini parameter suppressed
// OSD screen concerning card calibration fully modified
// Automatic calibration added (not finished)
//
// Revision 1.59.2.9  2001/08/24 12:35:09  adcockj
// Menu handling changes
//
// Revision 1.59.2.8  2001/08/23 16:04:57  adcockj
// Improvements to dynamic menus to remove requirement that they are not empty
//
// Revision 1.59.2.7  2001/08/21 09:43:01  adcockj
// Brought branch up to date with latest code fixes
//
// Revision 1.59.2.6  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.59.2.5  2001/08/18 17:09:30  adcockj
// Got to compile, still lots to do...
//
// Revision 1.59.2.4  2001/08/17 16:35:14  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
// Revision 1.59.2.3  2001/08/15 14:44:05  adcockj
// Starting to put some flesh onto the new structure
//
// Revision 1.59.2.2  2001/08/14 16:41:36  adcockj
// Renamed driver
// Got to compile with new class based card
//
// Revision 1.59.2.1  2001/08/14 09:40:19  adcockj
// Interim version of code for multiple card support
//
// Revision 1.59  2001/08/13 18:07:24  adcockj
// Added Czech code page for teletext
//
// Revision 1.58  2001/08/09 22:18:23  laurentg
// Improvments in relation with calibration
//
// Revision 1.57  2001/08/09 21:34:59  adcockj
// Fixed bugs raise by Timo and Keld
//
// Revision 1.56  2001/08/08 18:03:20  adcockj
// Moved status timer start till after hardware init
//
// Revision 1.55  2001/08/05 16:32:12  adcockj
// Added brackets
//
// Revision 1.54  2001/08/03 14:36:05  adcockj
// Added menu for sharpness filter
//
// Revision 1.53  2001/08/03 14:24:32  adcockj
// added extra info to splash screen and log
//
// Revision 1.52  2001/08/02 16:43:05  adcockj
// Added Debug level to LOG function
//
// Revision 1.51  2001/07/30 22:44:04  laurentg
// Bug fixed concerning saturation V accelerator
//
// Revision 1.50  2001/07/28 13:24:40  adcockj
// Added UI for Overlay Controls and fixed issues with SettingsDlg
//
// Revision 1.49  2001/07/27 16:11:32  adcockj
// Added support for new Crash dialog
//
// Revision 1.48  2001/07/26 22:26:24  laurentg
// New menu for card calibration
//
// Revision 1.47  2001/07/24 12:19:00  adcockj
// Added code and tools for crash logging from VirtualDub
//
// Revision 1.46  2001/07/23 20:52:07  ericschmidt
// Added TimeShift class.  Original Release.  Got record and playback code working.
//
// Revision 1.45  2001/07/16 18:07:50  adcockj
// Added Optimisation parameter to ini file saving
//
// Revision 1.44  2001/07/13 18:13:24  adcockj
// Changed Mute to not be persisted and to work properly
//
// Revision 1.43  2001/07/13 16:14:55  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.42  2001/07/13 07:04:43  adcockj
// Attemp 1 at fixing MSP muting
//
// Revision 1.41  2001/07/12 19:28:03  adcockj
// Limit VT display to valid pages
//
// Revision 1.40  2001/07/12 16:20:07  adcockj
// Fixed typo in $Id
//
// Revision 1.39  2001/07/12 16:16:39  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "Other.h"
#include "CPU.h"
#include "MixerDev.h"
#include "VBI_VideoText.h"
#include "AspectRatio.h"
#include "DScaler.h"
#include "Settings.h"
#include "ProgramList.h"
#include "Dialogs.h"
#include "OutThreads.h"
#include "OSD.h"
#include "Audio.h"
#include "Status.h"
#include "VBI.h"
#include "FD_60Hz.H"
#include "FD_50Hz.H"
#include "FD_Common.H"
#include "Filter.h"
#include "Splash.h"
#include "VideoSettings.h"
#include "VBI_CCdecode.h"
#include "VBI_VideoText.h"
#include "Deinterlace.h"
#include "FieldTiming.h"
#include "DebugLog.h"
#include "TimeShift.h"
#include "Crash.h"
#include "Calibration.h"
#include "Providers.h"
#include "OverlaySettings.h"
#include "Perf.h"
#include "hardwaredriver.h"
#include "StillSource.h"
#include "TreeSettingsDlg.h"
#include "SettingsPerChannel.h"
#include "HardwareSettings.h"
#include "Events.h"
#include "WindowBorder.h"
#include "ToolbarControl.h"
#include "SettingsMaster.h"
#include "Credits.h"
#include "SizeSettings.h"
#include "PaintingHDC.h"
#include "OutReso.h"
#include "MultiFrames.h"
#include "dshowsource/DSSourceBase.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


HWND hWnd = NULL;
HINSTANCE hResourceInst = NULL;
HINSTANCE hDScalerInst = NULL;
LPSTR lPStripTimingString = NULL;

HWND hPSWnd = NULL;

// Used to call MainWndOnInitBT
#define INIT_BT 1800

BOOL bDoResize = FALSE;

HWND VThWnd;

long WStyle;

BOOL    bShowMenu=TRUE;
HMENU   hMenu;
HMENU   hSubMenuChannels = NULL;
HMENU	hMenuTray;
HACCEL  hAccel;

char ChannelString[10] = "";

int MainProcessor=0;
int DecodeProcessor=0;
int PriorClassId = 0;
int ThreadClassId = 1;

//Cursor defines and vars
#define CURSOR_DEFAULT 0x0000
#define CURSOR_HAND    0x0001
HCURSOR hCursorDefault = NULL;
HCURSOR hCursorHand = NULL;

BOOL bShowCursor = TRUE;
BOOL bAutoHideCursor = TRUE;

long MainWndWidth = 649;
long MainWndHeight = 547;
long MainWndLeft = 10;
long MainWndTop = 10;

BOOL bAlwaysOnTop = FALSE;
BOOL bAlwaysOnTopFull = TRUE;
BOOL bDisplayStatusBar = TRUE;
BOOL bDisplaySplashScreen = TRUE;
BOOL bVTSingleKeyToggle = TRUE;
BOOL bIsFullScreen = FALSE;
BOOL bForceFullScreen = FALSE;
BOOL bUseAutoSave = FALSE;
BOOL bScreensaverOff = FALSE;
BOOL bVTAutoCodePage = FALSE;
BOOL bVTAntiAlias = FALSE;
BOOL bMinimized = FALSE;
BOOL bReverseChannelScroll = FALSE;

BOOL bMinToTray = FALSE;
BOOL bIconOn = FALSE;
int MinimizeHandling = 0;
BOOL BypassChgResoInRestore = FALSE;

BOOL bKeyboardLock = FALSE;
HHOOK hKeyboardHook = NULL;

HFONT hCurrentFont = NULL;

int ChannelEnterTime = 0;

int InitialChannel = -1;
int InitialTextPage = -1;

BOOL bInMenu = FALSE;
BOOL bShowCrashDialog = FALSE;
BOOL bIsRightButtonDown = FALSE;
BOOL bIgnoreNextRightButtonUpMsg = FALSE;

UINT MsgWheel;
UINT MsgOSDShow;
UINT MsgTaskbarRestart;

NOTIFYICONDATA nIcon;

// Current sleepmode timers (minutes)
TSMState SMState;
#define SMPeriodCount       7
int SMPeriods[SMPeriodCount] =
{
    0,
    1,
    15,
    30,
    60,
    90,
    120
};

HRGN DScalerWindowRgn = NULL;

char* szSkinName = NULL;
char szSkinDirectory[MAX_PATH+1];
vector<string> vSkinNameList;

CSettingsMaster* SettingsMaster = NULL;
CEventCollector* EventCollector = NULL;
CWindowBorder* WindowBorder = NULL;
CToolbarControl* ToolbarControl = NULL;

CPaintingHDC OffscreenHDC;

BOOL IsFullScreen_OnChange(long NewValue);
BOOL DisplayStatusBar_OnChange(long NewValue);
BOOL ScreensaverOff_OnChange(long NewValue);
BOOL Cursor_IsOurs();
void Cursor_UpdateVisibility();
void Cursor_SetVisibility(BOOL bVisible);
int Cursor_SetType(int type);
void Cursor_VTUpdate(int x = -1, int y = -1);
void MainWndOnDestroy();
void SetDirectoryToExe();
int ProcessCommandLine(char* commandLine, char* argv[], int sizeArgv);
void SetKeyboardLock(BOOL Enabled);
bool bScreensaverDisabled = false;
bool bPoweroffDisabled = false;
bool bLowpowerDisabled = false;
HMENU CreateDScalerPopupMenu();
BOOL IsStatusBarVisible();
BOOL IsToolBarVisible();
HRGN UpdateWindowRegion(HWND hWnd, BOOL bUpdateWindowState);
void SetWindowBorder(HWND hWnd, LPCSTR szSkinName, BOOL bShow);
void Skin_SetMenu(HMENU hMenu, BOOL bUpdateOnly);
LPCSTR GetSkinDirectory();
LONG OnChar(HWND hWnd, UINT message, UINT wParam, LONG lParam);
LONG OnSize(HWND hWnd, UINT wParam, LONG lParam);
void SetTray(BOOL Way);
int On_IconHandler(WPARAM wParam, LPARAM lParam);

static const char* UIPriorityNames[3] = 
{
    "Normal",
    "High",
    "Very High",
};

static const char* DecodingPriorityNames[5] = 
{
    "Low",
    "Normal",
    "Above Normal",
    "High",
    "Very High",
};

static const char* MinimizeHandlingLabels[3] = 
{
    "User control / Continue capture",
    "User control / Stop capture",
    "Automatic / Detect video signal",
};


static BOOL bTakingCyclicStills = FALSE;
static BOOL bIgnoreDoubleClick = FALSE;

static int ProcessorSpeed = 1;
static int TradeOff = 1;
static int FullCpu = 1;
static int VideoCard = 0;
static int ShowHWSetupBox;
static long m_EventTimerID = 0;

static int ChannelPreviewNbCols = 4;
static int ChannelPreviewNbRows = 4;

///**************************************************************************
//
// FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)
//
// PURPOSE: calls initialization function, processes message loop
//
///**************************************************************************

int APIENTRY WinMainOld(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc;
    MSG msg;
    HWND hPrevWindow;

    // used by the InnoSetup installer to prevent (un)installation when DScaler is running.
    CreateMutex(NULL, FALSE, "DScaler"); 

    hDScalerInst = hInstance;

    SetErrorMode(SEM_NOGPFAULTERRORBOX);
    SetUnhandledExceptionFilter(CrashHandler);
    VDCHECKPOINT;

    SetDirectoryToExe();

    CPU_SetupFeatureFlag();

    MsgOSDShow = RegisterWindowMessage("DScalerShowOSDMsgString");
    MsgWheel = RegisterWindowMessage("MSWHEEL_ROLLMSG");
    MsgTaskbarRestart = RegisterWindowMessage("TaskbarCreated");

    // make a copy of the command line since ProcessCommandLine() will replace the spaces with \0
    char OldCmdLine[1024];
    strncpy(OldCmdLine, lpCmdLine, sizeof(OldCmdLine));

    char* ArgValues[20];
    int ArgCount = ProcessCommandLine(lpCmdLine, ArgValues, sizeof(ArgValues) / sizeof(char*));

    // if we are already running then start up old version
    hPrevWindow = FindWindow((LPCTSTR) DSCALER_APPNAME, NULL);
    if (hPrevWindow != NULL)
    {
        if (IsIconic(hPrevWindow))
        {
            SendMessage(hPrevWindow, WM_SYSCOMMAND, SC_RESTORE, NULL);
        }
        SetFocus(hPrevWindow);
        SetActiveWindow(hPrevWindow);
        SetForegroundWindow(hPrevWindow);

        if(ArgCount > 1)
        {
            // Command line parameter to send messages to the OSD of a running copy of DScaler.
            //
            // Usage:
            // /m for a temporary message. /M for a persistent message.
            // /m or /M must be the first parameter.
            //
            // use \n to start a new line.
            // If DScaler is not running this copy exits silently.
            //
            // example:
            // dscaler /m This is a message.\nSecond line.
            //
            if((ArgValues[0][0] == '/' || ArgValues[0][0] == '-') && tolower(ArgValues[0][1]) == 'm')
            {
                HANDLE hMapFile = NULL;
                char* lpMsg = NULL;

                hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE,  // Use the system page file
                    NULL, PAGE_READWRITE, 0, 1024, "DScalerSendMessageFileMappingObject");                        
                
                if(hMapFile == NULL) 
                { 
                    // send error message to running copy of DScaler
                    SendMessage(hPrevWindow, MsgOSDShow, GetLastError(), 1);
                } 
                else
                {
                    lpMsg = (char*)MapViewOfFile(hMapFile, FILE_MAP_WRITE, 0, 0, 1024);              
                    
                    if (lpMsg == NULL) 
                    { 
                        // send error message to running copy of DScaler
                        SendMessage(hPrevWindow, MsgOSDShow, GetLastError(), 2);
                    }
                    else
                    {
                        // Find the command line after the /m or -m or /M or -M
                        // a simple strstr(OldCmdLine, ArgValues[1]) will not work with a leading quote
                        char* s;
                        s = strstr(OldCmdLine, ArgValues[0]);   // s points to first parameter
                        s = &s[strlen(ArgValues[0])];           // s points to char after first parm
                        if(s[0] == ' ')                         // point s to string after the space
                        {
                            s++;
                        }
                        strncpy(lpMsg, s, 1024);
                        SendMessage(hPrevWindow, MsgOSDShow, ArgValues[0][1] == 'm' ? 0 : 1, 0);
                        UnmapViewOfFile(lpMsg);
                    }
                    CloseHandle(hMapFile);
                }
            }
        }
        return FALSE;
    }

    // JA 07/01/2001
    // Required to use slider control
    InitCommonControls();

    // setup default ini file
	SetIniFileForSettings("");
    
	// Process the command line arguments.
    // The following arguments are supported
    // -i<inifile>      specification of the ini file.
    // -c<channel>      the starting channel (0-x).
    // -p<page>         the starting videotext page (100-y).
    // -driverinstall   (un)install the NT driver. These arguments are mainly intended to be
    // -driveruninstall used by the installation program. When the user has not enough rights to
    //                  complete (un)installation a messagebox is shown. With other errors there
    //                  is no feedback.
    // For backwards compatibility an argument not starting with a - or / is
    // processed as -i<parameter> (ini file specification).
    for (int i = 0; i < ArgCount; ++i)
    {
        if (ArgValues[i][0] != '-' && ArgValues[i][0] != '/')
        {
            SetIniFileForSettings(ArgValues[i]);
        }
        else if(tolower(ArgValues[i][1]) == 'm')
        {
            // DScaler was called to show a message in a running instance of DScaler.
            // At this time DScaler is not running so we can exit now.
            return 0;
        }
        else if(strlen(ArgValues[i]) > 2)
        {
            char* szParameter = &ArgValues[i][2];
            switch (tolower(ArgValues[i][1]))
            {
            case 'd':
                if(strcmp(szParameter, "riverinstall") == 0)
                {
                    DWORD result = 0;                  
                    CHardwareDriver* HardwareDriver = NULL;
                    
                    HardwareDriver = new CHardwareDriver();
                    if(!HardwareDriver->InstallNTDriver())
                    {
                        // if access denied
                        if(GetLastError() == 5)
                        {
                            RealErrorBox("You must have administrative rights to install the driver.");
                        }
                        result = 1;
                    }
                    
                    delete HardwareDriver;
                    
                    return result;
                }
                else if(strcmp(szParameter, "riveruninstall") == 0)
                {
                    DWORD result = 0;                  
                    CHardwareDriver* HardwareDriver = NULL;
                    
                    HardwareDriver = new CHardwareDriver();
                    if(!HardwareDriver->UnInstallNTDriver())
                    {
                        // if access denied
                        if(GetLastError() == 5)
                        {
                            RealErrorBox("You must have administrative rights to uninstall the driver.");
                        }
                        result = 1;
                    }
                    
                    delete HardwareDriver;
                    
                    return result;
                }
            case 'i':
                SetIniFileForSettings(szParameter);              
                break;
            case 'c':
                sscanf(szParameter, "%d", &InitialChannel);
                break;
            case 'p':
                sscanf(szParameter, "%x", &InitialTextPage);
                break;
            default:
                // Unknown
                break;
            }
        }
    }

    ShowHWSetupBox =    !Setting_ReadFromIni(DScaler_GetSetting(PROCESSORSPEED))
                     || !Setting_ReadFromIni(DScaler_GetSetting(TRADEOFF))
                     || !Setting_ReadFromIni(DScaler_GetSetting(FULLCPU))
                     || !Setting_ReadFromIni(DScaler_GetSetting(VIDEOCARD));

    
    // Event collector
    if (EventCollector == NULL)
    {
        EventCollector = new CEventCollector();        
    }
        
    // Master setting. Holds all settings in the future
    //  For now, only settings that are registered here respond to events 
    //  like source/input/format/channel changes
    if (SettingsMaster == NULL)
    {
        SettingsMaster = new CSettingsMaster();
    }
    SettingsMaster->IniFile(GetIniFileForSettings());
    
    // load up ini file settings after parsing parms as 
    // the ini file location may have changed
    LoadSettingsFromIni();
    //
    

    // make sure dscaler.ini exists with many of the options in it.
    // even if dscaler crashes a new user is able to make changes to dscaler.ini.
    WriteSettingsToIni(TRUE);

    Initialize_Mute();


    // load up the cursors we want to use
    // we load up arrow as the default and try and load up
    // the hand cursor if we are running NT 5
    hCursorDefault = LoadCursor(NULL, IDC_ARROW);

    OSVERSIONINFO version;
    version.dwOSVersionInfoSize = sizeof(version);
    GetVersionEx(&version);
    if ((version.dwPlatformId == VER_PLATFORM_WIN32_NT) && (version.dwMajorVersion >= 5))
    {
        hCursorHand = LoadCursor(NULL, IDC_HAND);
    }
    else
    {        
        hCursorHand = LoadCursor(hResourceInst, MAKEINTRESOURCE(IDC_CURSOR_HAND));
    }

    wc.style = CS_DBLCLKS;      // Allow double click
    wc.lpfnWndProc = (WNDPROC) MainWndProcSafe;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(LONG);
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DSCALER));
    wc.hCursor = hCursorDefault; 
    wc.hbrBackground = CreateSolidBrush(0);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = DSCALER_APPNAME;

    if (!RegisterClass(&wc))
    {
        return FALSE;
    }
    hMenu = LoadMenu(hResourceInst, MAKEINTRESOURCE(IDC_DSCALERMENU));

    hSubMenuChannels = GetSubMenuWithName(hMenu, 2, "&Channels");


    // 2000-10-31 Added by Mark: Changed to WS_POPUP for more cosmetic direct-to-full-screen startup,
    // let UpdateWindowState() handle initialization of windowed dTV instead.

	LoadDynamicFunctions();

    hWnd = CreateWindow(DSCALER_APPNAME, DSCALER_APPNAME, WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL, NULL, hInstance, NULL);
    if (!hWnd) return FALSE;
	// Always position the window to the last saved position even when starting in full screen mode
	// to be sure to have the display on the correct screen
	// Display in full screen mode is then done later when calling UpdateWindowState
    SetWindowPos(hWnd, 0, MainWndLeft, MainWndTop, MainWndWidth, MainWndHeight, SWP_SHOWWINDOW);

	// Restore the default positions for the window if the window was previously placed on a screen
	// which is no more active
	// The default position is on the primary monitor
	RECT screenRect;
	GetMonitorRect(hWnd, &screenRect);
	if ( (MainWndLeft > screenRect.right)
	  || ((MainWndLeft+MainWndWidth) < screenRect.left)
	  || (MainWndTop > screenRect.bottom)
	  || ((MainWndTop+MainWndHeight) < screenRect.top) )
	{
		Setting_SetDefault(DScaler_GetSetting(STARTLEFT));
		Setting_SetDefault(DScaler_GetSetting(STARTTOP));
		Setting_SetDefault(DScaler_GetSetting(STARTWIDTH));
		Setting_SetDefault(DScaler_GetSetting(STARTHEIGHT));
	    SetWindowPos(hWnd, 0, MainWndLeft, MainWndTop, MainWndWidth, MainWndHeight, SWP_SHOWWINDOW);
	}

	// Show the splash screen after creating the main window
	// to be sure to display it on the right monitor
    if(bDisplaySplashScreen)
    {
        ShowSpashScreen();
    }

    if (!StatusBar_Init()) return FALSE;

    if (bDisplayStatusBar == FALSE)
    {
        StatusBar_ShowWindow(FALSE);
    }

	if (bMinToTray)
	{
        SetTray(TRUE);
	}

    // 2000-10-31 Added by Mark Rejhon
    // Now show the window, directly to maximized or windowed right away.
    // That way, if the end user has configured dTV to startup maximized,
    // it won't flash a window right before maximizing.
    UpdateWindowState();

    PostMessage(hWnd, WM_SIZE, SIZENORMAL, MAKELONG(MainWndWidth, MainWndHeight));
    if ((hAccel = LoadAccelerators(hResourceInst, MAKEINTRESOURCE(IDA_DSCALER))) == NULL)
    {
        ErrorBox("Accelerators not Loaded");
    }

    // Initialize sleepmode
    SMState.State = SM_WaitMode;
    SMState.iPeriod = 0;
    SMState.Period = 0;
    SMState.SleepAt = 0;

    // trigger any error messages if the menu is corrupt
#ifdef _DEBUG
    CreateDScalerPopupMenu();
#endif
	
	VDCHECKPOINT;

    // catch any serious errors during message handling
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(hWnd, hAccel, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    LOG(1,"Program exit");

    return msg.wParam;
}

LONG APIENTRY MainWndProcSafe(HWND hWnd, UINT message, UINT wParam, LONG lParam)
{
    __try
    {
        LOG(3, "Got Windows Message %d Params %d %d", message, wParam, lParam);
        return MainWndProc(hWnd, message, wParam, lParam);
    }
    // if there is any exception thrown then exit the process
    __except (CrashHandler((EXCEPTION_POINTERS*)_exception_info())) 
    { 
        LOG(1, "Crash in MainWndProc");
        // try as best we can to unload everything
        // mostly we want to make sure that the driver is stopped
        // cleanly so that the machine doesn't blue screen
        MainWndOnDestroy();
        ExitProcess(1);
        return 0;
    }
}

HMENU CreateDScalerPopupMenu()
{
    HMENU hMenuPopup;
    hMenuPopup = LoadMenu(hResourceInst, MAKEINTRESOURCE(IDC_CONTEXTMENU));
    if (hMenuPopup != NULL)
    {
        hMenuPopup = GetSubMenu(hMenuPopup,0);
    }
    if (hMenuPopup != NULL && hMenu != NULL)
    {
        MENUITEMINFO MenuItemInfo;
        HMENU hSubMenu;
        char string[128];
        int reduc1;

        // update the name of the source
        if(GetMenuString(hMenu, 1, string, sizeof(string), MF_BYPOSITION) != 0)
        {
            ModifyMenu(hMenuPopup, 1, MF_BYPOSITION | MF_STRING, IDM_POPUP_SOURCES, string);
        }

        MenuItemInfo.cbSize = sizeof (MenuItemInfo);
        MenuItemInfo.fMask = MIIM_SUBMENU;

        hSubMenu = GetSubMenuWithName(hMenu, 0, "&Sources");
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,0,TRUE,&MenuItemInfo);
        }

        // The name of this menu item depends on the source so we don't check the name
        hSubMenu = GetSubMenu(hMenu, 1);
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,1,TRUE,&MenuItemInfo);
        }

        hSubMenu = GetChannelsSubmenu();
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,2, TRUE, &MenuItemInfo);
        }

        string[0] = '\0';
        GetMenuString(hMenu, 2, string, sizeof(string), MF_BYPOSITION);
        reduc1 = !strcmp(string, "&Channels") ? 0 : 1;

        hSubMenu = GetSubMenuWithName(hMenu, 3-reduc1, "&View");
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,3,TRUE,&MenuItemInfo);
        }

        hSubMenu = GetVideoDeinterlaceSubmenu();
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,4, TRUE, &MenuItemInfo);
        }

        hSubMenu = GetFiltersSubmenu();
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,5, TRUE, &MenuItemInfo);
        }

        hSubMenu = GetSubMenuWithName(hMenu, 6-reduc1, "&AspectRatio");
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,6, TRUE, &MenuItemInfo);
        }

        hSubMenu = GetSubMenuWithName(hMenu, 7-reduc1, "S&ettings");
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,7, TRUE, &MenuItemInfo);
        }

        hSubMenu = GetSubMenuWithName(hMenu, 8-reduc1, "Ac&tions");
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,8, TRUE, &MenuItemInfo);
        }

        hSubMenu = GetSubMenuWithName(hMenu, 9-reduc1, "&Datacasting");
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,9,TRUE,&MenuItemInfo);
        }

        hSubMenu = GetSubMenuWithName(hMenu, 10-reduc1, "&Help");
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,10,TRUE,&MenuItemInfo);
        }
    }
    return hMenuPopup;
}


BOOL WINAPI OnContextMenu(HWND hWnd, int x, int y)
{ 
    RECT rc;                    // client area of window
    POINT pt = {0,0};           // location of mouse click
    HMENU hMenuPopup = CreateDScalerPopupMenu();

    // Get the bounding rectangle of the client area.
    GetClientRect(hWnd, &rc);

    //if the context menu is opend with a keypress, x and y is -1
    if(x!=-1 && y!=-1)
    {
        pt.x=x;
        pt.y=y;

        // Convert the mouse position to client coordinates.
        ScreenToClient(hWnd, &pt);
    }

    // If the position is in the client area, display a
    // shortcut menu.
    if (PtInRect(&rc, pt))
    {
        ClientToScreen(hWnd, &pt);
        // Display the shortcut menu. Track the right mouse
        // button.
        return TrackPopupMenuEx(hMenuPopup, 
                                TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, 
                                pt.x, pt.y, hWnd, NULL); 
    }

    if (hMenuPopup != NULL)
    {
        DestroyMenu(hMenuPopup);
    }
 
    // Return FALSE if no menu is displayed.
    return FALSE; 
} 


BOOL ProcessVTMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    RECT Rect;
    BOOL bHandled = FALSE;
    BOOL bPageChanged = FALSE;

    HDC hWndDC = GetDC(hWnd);
    GetDestRect(&Rect);

    OffscreenHDC.UpdateGeometry(hWndDC, &Rect);
    HDC hDC = OffscreenHDC.GetBufferDC();

    VT_ResetPaintedRects();

    switch (uMsg)
    {
    case WM_COMMAND:
        if (VT_GetState() != VT_OFF)
        {
            switch (LOWORD(wParam))
            {
            case IDM_VT_PAGE_MINUS:
                VT_ClearInput();
                bPageChanged = VT_PageScroll(hDC, &Rect, FALSE);
                bHandled = TRUE;
                break;

            case IDM_VT_PAGE_PLUS:
                VT_ClearInput();
                bPageChanged = VT_PageScroll(hDC, &Rect, TRUE);
                bHandled = TRUE;
                break;

            case IDM_VT_PAGE_UP:
                VT_ClearInput();
                bPageChanged = VT_SubPageScroll(hDC, &Rect, FALSE);
                bHandled = TRUE;
                break;

            case IDM_VT_PAGE_DOWN:
                VT_ClearInput();
                bPageChanged = VT_SubPageScroll(hDC, &Rect, TRUE);
                bHandled = TRUE;
                break;

            case IDM_CHANNELPLUS:
            case IDM_CHANNELMINUS:
            case IDM_CHANNEL_PREVIOUS:
                VT_ClearInput();
                VT_ShowHeader(hDC, &Rect);
                bHandled = TRUE;
                break;

            case IDC_TOOLBAR_CHANNELS_LIST:
                bHandled = TRUE;
                break;

            case IDM_VT_SEARCH:
            case IDM_VT_SEARCHNEXT:
            case IDM_VT_SEARCHPREV:
                {
                    BOOL bInclusive = FALSE;
                    BOOL bReverse = LOWORD(wParam) == IDM_VT_SEARCHPREV;
                    bHandled = TRUE;

                    VT_ClearInput();

                    // Get search string with a dialog if it's not search-next
                    // or search-previous or if the search string doesn't exist.
                    if (LOWORD(wParam) == IDM_VT_SEARCH || !VT_IsSearchStringValid())
                    {
                        if (!DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_VTSEARCH),
                            hWnd, (DLGPROC)VTSearchProc))
                        {
                            break;
                        }

                        bInclusive = TRUE;
                    }

                    bPageChanged = VT_PerformSearch(hDC, &Rect, bInclusive, bReverse);
                }
                break;

            case IDM_VT_AUTOCODEPAGE:
                VT_SetAutoCodepage(hDC, &Rect, !VT_GetAutoCodepage());
                bHandled = TRUE;
                break;

            case IDM_VT_ANTIALIAS:
                VT_SetAntialias(hDC, &Rect, !VT_GetAntialias());
                bHandled = TRUE;
                break;

            case IDM_TELETEXT_KEY1:
            case IDM_TELETEXT_KEY2:
            case IDM_TELETEXT_KEY3:
            case IDM_TELETEXT_KEY4:
            case IDM_TELETEXT_KEY5:
                {
                    VT_ClearInput();
                    BYTE nFlofKey = LOWORD(wParam) - IDM_TELETEXT_KEY1;
                    bPageChanged = VT_PerformFlofKey(hDC, &Rect, nFlofKey);
                    bHandled = TRUE;
                }
                break;

            case IDM_TELETEXT_KEY6:
                VT_ClearInput();
                VT_SetShowHidden(hDC, &Rect, !VT_GetShowHidden());
                bHandled = TRUE;
                break;

            case IDM_CHARSET_TEST:
                VT_ClearInput();
                bPageChanged = VT_ShowTestPage(hDC, &Rect);
                bHandled = TRUE;
                break;
            }
        }
        break;

    case WM_LBUTTONDOWN:
        if (VT_GetState() != VT_OFF)
        {
            VT_ClearInput();
            bPageChanged = VT_ClickAtPosition(hDC, &Rect, LOWORD(lParam), HIWORD(lParam));
            if (bPageChanged != FALSE)
            {
                bHandled = TRUE;
            }
        }
        break;

    case WM_TIMER:
        {
            switch (LOWORD(wParam))
            {
            case TIMER_VTFLASHER:
                VT_RedrawFlash(hDC, &Rect);
                bHandled = TRUE;
                break;

            case TIMER_VTINPUT:
                VT_OnInputTimer(hDC, &Rect);
                bHandled = TRUE;
                break;
            }
        }
        break;

    case WM_CHAR:
        if (VT_GetState() != VT_OFF)
        {
            if (wParam >= '0' && wParam <= '9')
            {
                bPageChanged = VT_OnInput(hDC, &Rect, (char)wParam);
                bHandled = TRUE;
            }
        }
        break;

    case UWM_VIDEOTEXT:
        {
            switch(LOWORD(wParam))
            {
            case VTM_VTHEADERUPDATE:
                VT_ProcessHeaderUpdate(hDC, &Rect);
                bHandled = TRUE;
                break;

            case VTM_VTCOMMENTUPDATE:
                bPageChanged = VT_ProcessCommentUpdate(hDC, &Rect, lParam);
                bHandled = TRUE;
                break;

            case VTM_VTPAGEUPDATE:
                bPageChanged = VT_ProcessPageUpdate(hDC, &Rect, lParam);
                bHandled = TRUE;
                break;

            case VTM_VTPAGEREFRESH:
                bPageChanged = VT_ProcessPageRefresh(hDC, &Rect, lParam);
                bHandled = TRUE;
                break;
            }
        }
        break;
    }

    RECT PaintedRects[25];

    LONG nPaintedRects = VT_GetPaintedRects(PaintedRects, 25);

    if (nPaintedRects != 0)
    {
        RECT OSDDrawRect;
        GetDisplayAreaRect(hWnd, &OSDDrawRect);

        // Draw the OSD over the top
        OSD_Redraw(hDC, &OSDDrawRect);

        OffscreenHDC.BitBltRects(PaintedRects, nPaintedRects, hWndDC);
    }

    ReleaseDC(hWnd, hWndDC);

    if (bPageChanged != FALSE)
    {
        Cursor_VTUpdate();
    }

    return bHandled;
}


BOOL ProcessOSDMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    RECT Rect;
    BOOL bHandled = FALSE;
    HDC hDC;

    HDC hWndDC = GetDC(hWnd);
    GetDisplayAreaRect(hWnd, &Rect);

    if (OffscreenHDC.UpdateGeometry(hWndDC, &Rect))
    {
        // We should not use the offscreen buffer
        // until it is filled in WM_PAINT.
        hDC = hWndDC;
    }
    else
    {
        hDC = OffscreenHDC.GetBufferDC();
    }

    OSD_ResetPaintedRects();

    switch (uMsg)
    {
    case WM_COMMAND:
        if (LOWORD(wParam) == IDM_CLEAROSD)
        {
            OSD_Clear(hDC, &Rect);
            bHandled = TRUE;
        }
        break;

    case WM_TIMER:
        switch (LOWORD(wParam))
        {
        case OSD_TIMER_ID:
            OSD_Clear(hDC, &Rect);
            bHandled = TRUE;
            break;

        case OSD_TIMER_REFRESH_ID:
            OSD_RefreshInfosScreen(hDC, &Rect, 0);
            bHandled = TRUE;
            break;
        }
        break;

    case UWM_OSD:
        if (LOWORD(wParam) == OSDM_DISPLAYUPDATE)
        {
            OSD_ProcessDisplayUpdate(hDC, &Rect);
            bHandled = TRUE;
        }
        break;
    }

    if (hDC != hWndDC)
    {
        RECT PaintedRects[OSD_MAX_TEXT];

        LONG nPaintedRects = OSD_GetPaintedRects(PaintedRects, OSD_MAX_TEXT);

        if (nPaintedRects != 0)
        {
            OffscreenHDC.BitBltRects(PaintedRects, nPaintedRects, hWndDC);
        }
    }

    ReleaseDC(hWnd, hWndDC);

    return bHandled;            
}


LRESULT CALLBACK KeyboardHookProc(int code, UINT wParam, UINT lParam)
{
    if(code >= 0 && bKeyboardLock)
    {
        // if it is not Ctrl+Shift+L do not pass the message to the rest of the hook chain 
        // or the target window procedure
        if(!((char)wParam == 'L' && GetKeyState(VK_SHIFT) < 0 && GetKeyState(VK_CONTROL) < 0))
        {
            return 1;
        }
    }
   	return CallNextHookEx(hKeyboardHook, code, wParam, lParam);
}


void SetKeyboardLock(BOOL Enabled)
{
    if(Enabled)
    {
        bKeyboardLock = TRUE;
        if(hKeyboardHook == NULL)
        {
            hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)KeyboardHookProc, NULL, 
                                             GetCurrentThreadId());
        }
    }
    else
    {
        bKeyboardLock = FALSE;
        if(hKeyboardHook != NULL)
        {
            UnhookWindowsHookEx(hKeyboardHook);
            hKeyboardHook = NULL;
        }
    }
}


void SetScreensaverMode(BOOL bScreensaverOff)
{
    BOOL bScreensaverMode = false;
    BOOL bLowpowerMode = false;
    BOOL bPoweroffMode = false;

    if( bScreensaverOff )
    {
        // Disable screensaver if enabled
        SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &bScreensaverMode, 0);
        if( bScreensaverMode )
        {
            // Disable
            SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, 0 , NULL, 0);
            bScreensaverDisabled = true;
        }

        // Disable monitor sleeping
        SystemParametersInfo(SPI_GETLOWPOWERACTIVE, 0, &bLowpowerMode, 0);
        if( bLowpowerMode )
        {
            // BOTH low-power and power-off needs to be disabled
            // to stop monitor energy saver from kicking in.
            SystemParametersInfo(SPI_SETLOWPOWERACTIVE, 0 , NULL, 0);
            bLowpowerDisabled = true;
        }

        // Disable monitor sleeping
        SystemParametersInfo(SPI_GETPOWEROFFACTIVE, 0, &bPoweroffMode, 0);
        if( bPoweroffMode )
        {
            SystemParametersInfo(SPI_SETPOWEROFFACTIVE, 0 , NULL, 0);
            bPoweroffDisabled = true;
        }
    }
    else
    {
        // Enable if disabled by us
        if( bScreensaverDisabled )
        {
            SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, 1, NULL, 0);
            bScreensaverDisabled = false;    
        }

        if( bLowpowerDisabled )
        {
            SystemParametersInfo(SPI_SETLOWPOWERACTIVE, 1, NULL, 0);
            bLowpowerDisabled = false;
        }

        if( bPoweroffDisabled )
        {
            SystemParametersInfo(SPI_SETPOWEROFFACTIVE, 1, NULL, 0);
            bPoweroffDisabled = false;
        }
    }
}

void UpdateSleepMode(TSMState* SMState, char* Text)
{
    time_t curr = 0;
    struct tm* SMTime = NULL;
    UINT uiPeriod;

    switch( SMState->State )
    {
    case SM_WaitMode:
        // Called by press on delete key. 
        // Initial mode. (ie 1st press)
        
        // Show current sleep setting
        if( SMState->SleepAt != 0 ) 
        {
            KillTimer(hWnd, TIMER_SLEEPMODE);
            SMTime = localtime(&SMState->SleepAt);
            sprintf(Text, "Sleep %d (%02d:%02d)", SMState->Period, SMTime->tm_hour, SMTime->tm_min);
        }
        else
        {
            strcpy(Text, "Sleep OFF");
        }
        // Set update window
        SetTimer(hWnd, TIMER_SLEEPMODE, TIMER_SLEEPMODE_MS, NULL);

        // Goto change-mode (interprete next push on key as change-request)
        SMState->State = SM_ChangeMode;
        break;

    case SM_ChangeMode:
        // Called by press on delete key.
        // Subsequent presses within TIMER_SLEEPMODE_MS of previous press 
        
        // Next higher period (cycle)
        SMState->iPeriod = ++SMState->iPeriod % SMPeriodCount;
        SMState->Period = SMPeriods[SMState->iPeriod];

        // Set & show activation time
        if( SMState->Period != 0 ) 
        {
            curr = time(0);
            SMState->SleepAt = curr + SMState->Period * 60;
            SMTime = localtime(&SMState->SleepAt);
            sprintf(Text, "New sleep %d (%02d:%02d)", SMState->Period, SMTime->tm_hour, SMTime->tm_min);
        }
        else
        {   
            SMState->SleepAt = 0;
            strcpy(Text, "New sleep OFF");
        }
        
        // Restart change-mode timing
        KillTimer(hWnd, TIMER_SLEEPMODE);
        SetTimer(hWnd, TIMER_SLEEPMODE, TIMER_SLEEPMODE_MS, NULL);
        break;
        
    case SM_UpdateMode:
        // Mode set by WM_TIMER upon passing of TIMER_SLEEPMODE_MS
        curr = time(0);
        
        // Set timer to remainder of period if applicable
        if( SMState->SleepAt > curr )
        {
            uiPeriod = ( SMState->SleepAt - curr ) * 1000;
            SetTimer(hWnd, TIMER_SLEEPMODE, uiPeriod, NULL);
        }
        else
        {
            // Passed sleepat-time in SM_Show or ChangeMode, stop asap
            if( SMState->SleepAt != 0 )
            {
                uiPeriod = 10;
                SetTimer(hWnd, TIMER_SLEEPMODE, uiPeriod, NULL);
            }
            else
            {
                // Sleep OFF
                ; 
            }
        }

        // Return to wait-mode.
        SMState->State = SM_WaitMode;
        break;
        
    default:
        ; //NEVER_GET_HERE
    }
}


BOOL GetDisplayAreaRect(HWND hWnd, LPRECT lpRect, BOOL WithToolbar)
{
    BOOL result = GetClientRect(hWnd, lpRect);

    if(bIsFullScreen == TRUE) 
    {
        if (WithToolbar == FALSE && ToolbarControl != NULL)
        {
            ToolbarControl->AdjustArea(lpRect, 1);        
        }

        return result;
    }

    if (result)
    {
        if (IsStatusBarVisible())
        {
            lpRect->bottom -= StatusBar_Height();
        }

        if ((WindowBorder!=NULL) && WindowBorder->Visible())
        {
            WindowBorder->AdjustArea(lpRect,1);
        }

        if (ToolbarControl!=NULL)
        {
            ToolbarControl->AdjustArea(lpRect, 1);        
        }
    }
    return result;
}

void AddDisplayAreaRect(HWND hWnd, LPRECT lpRect)
{
    if (ToolbarControl!=NULL)
    {
        ToolbarControl->AdjustArea(lpRect, 0);    
    }

    if ((WindowBorder!=NULL) && WindowBorder->Visible())
    { 
       WindowBorder->AdjustArea(lpRect,0);
    }    
    
    if (IsStatusBarVisible())
    {
        lpRect->bottom += StatusBar_Height();
    }   
}

void InvalidateDisplayAreaRect(HWND hWnd, LPRECT lpRect, BOOL bErase)
{
    if (lpRect == NULL)
    {
        RECT rc;
        GetDisplayAreaRect(hWnd, &rc);
        InvalidateRect(hWnd,&rc,bErase);
    }
    else
    {
        InvalidateRect(hWnd,lpRect,bErase);
    }
}

BOOL BorderGetClientRect(HWND hWnd, LPRECT lpRect)
{
    BOOL result = GetClientRect(hWnd, lpRect);
    if (IsStatusBarVisible())
    {
       lpRect->bottom -= StatusBar_Height();
    }
    return result;
}

LRESULT BorderButtonProc(string sID, void* pThis, HWND hWndParent, UINT MouseFlags, HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (sID=="BUTTON_CLOSE")
    {
        switch(message)
        {
          case WM_LBUTTONUP:
              {
                  SendMessage(hWndParent,WM_CLOSE,0,0);                  
              }
              return TRUE;              
        }
    } 
    else if (sID=="BUTTON_MINIMIZE")
    {
        switch(message)
        {
          case WM_LBUTTONUP:
              {
                  ShowWindow(hWndParent,SW_MINIMIZE);
              }
              return TRUE;              
        }
    } 
    else if (sID=="BUTTON_MAXIMIZE")
    {
        switch(message)
        {
          case WM_LBUTTONUP:
              {
                  SendMessage(hWnd, WM_COMMAND, IDM_FULL_SCREEN, 0);                  
              }
              return TRUE;              
        }
    } 
    else if (sID=="BUTTON_SIZE")
    {
        switch(message)
        {
          case WM_NCHITTEST:
              {
                  if (MouseFlags & MK_LBUTTON)
                  {
                      return ::DefWindowProc(hWndParent, WM_NCLBUTTONDOWN, HTBOTTOMRIGHT, lParam);                                            
                  }
              }
              break;
          case WM_SETCURSOR:
            static HCURSOR hResizeCursor = NULL;
            if (hResizeCursor == NULL)
            {
                hResizeCursor = LoadCursor(NULL,IDC_SIZENWSE);
            }
            if (hResizeCursor != NULL)
            {
                SetCursor(hResizeCursor);
            }
            return TRUE;
        }
    }
    else if (sID=="BUTTON_SIDEBAR")
    {
        switch(message)
        {
          case WM_LBUTTONUP:
              {                  
              }
              return TRUE;              
        }
    }
    return FALSE;
}

LPCSTR GetSkinDirectory()
{
    if (szSkinDirectory[0] != 0)
    {
        return szSkinDirectory;
    }

    char szPath[MAX_PATH+1];
    char* s = NULL;
    int len = GetFullPathName(GetIniFileForSettings(), MAX_PATH, szPath, &s);
    if ((len > 0) && (s!=NULL))
    {
        *s = 0;            
    }
    else
    {
        GetCurrentDirectory(MAX_PATH, szPath);
        strcat(szPath,"\\");
    }
    strcpy(szSkinDirectory,szPath);        
    strcat(szSkinDirectory,"Skins\\");    

    return szSkinDirectory;
}

void SetWindowBorder(HWND hWnd, LPCSTR szSkinName, BOOL bShow)
{
    if (WindowBorder==NULL) 
    {                
        if ((szSkinName == NULL) || (szSkinName[0] == 0))
		{
			//Don't make the windowborder unless it is necessary
			return;
		}
		WindowBorder = new CWindowBorder(hWnd, hDScalerInst, BorderGetClientRect);

        //Test border (white)
        //WindowBorder->SolidBorder(10,10,10,10, 0xFFFFFF);        
    }

    if ((szSkinName != NULL) && (szSkinName[0] == 0))
    {
        WindowBorder->ClearSkin();
    }

    if ((szSkinName != NULL) && (szSkinName[0] != 0))
    {
        char szSkinIniFile[MAX_PATH*2];
        strcpy(szSkinIniFile,GetSkinDirectory());
        strcat(szSkinIniFile,szSkinName);
        strcat(szSkinIniFile,"\\skin.ini");
        ///\todo check if the ini file exists

        //Add border buttons
        WindowBorder->RegisterButton("BUTTON_CLOSE",BITMAPASBUTTON_PUSH,"ButtonClose","ButtonCloseMouseOver","ButtonCloseClick", BorderButtonProc);
        WindowBorder->RegisterButton("BUTTON_SIZE",BITMAPASBUTTON_PUSH,"ButtonSize","ButtonSizeMouseOver","ButtonSizeClick", BorderButtonProc);
        WindowBorder->RegisterButton("BUTTON_MINIMIZE",BITMAPASBUTTON_PUSH,"ButtonMinimize","ButtonMinimizeMouseOver","ButtonMinimizeClick", BorderButtonProc);
        WindowBorder->RegisterButton("BUTTON_MAXIMIZE",BITMAPASBUTTON_PUSH,"ButtonMaximize","ButtonMaximizeMouseOver","ButtonMaximizeClick", BorderButtonProc);
        //WindowBorder->RegisterButton("BUTTON_SIDEBAR",BITMAPASBUTTON_PUSH,"ButtonSideBar","ButtonSideBarMouseOver","ButtonSideBarClick", BorderButtonProc);        
        
        vector<int>Results;        
        WindowBorder->LoadSkin(szSkinIniFile,"Border",&Results);

        ///\todo Process errors    
    }
    
    if (bShow && !bIsFullScreen && ((szSkinName == NULL) || (szSkinName[0]!=0)))
    {
        WindowBorder->Show();                        
    }
    else
    {
        WindowBorder->Hide();
    }
}

void Skin_SetMenu(HMENU hMenu, BOOL bUpdateOnly)
{
    if (!bUpdateOnly)
    {                
        vSkinNameList.clear();

        //Find sub directories with skin.ini files
        WIN32_FIND_DATA FindFileData;
        HANDLE hFind;                
        char szSearch[MAX_PATH+10];

        strcpy(szSearch,GetSkinDirectory());        
        strcat(szSearch,"*.*");
        hFind = FindFirstFile(szSearch, &FindFileData);
        if (hFind != INVALID_HANDLE_VALUE) 
        {
            do           
            {
                if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    && strcmp(FindFileData.cFileName,".") && strcmp(FindFileData.cFileName,"..") )
                {
                    char szSearchFile[MAX_PATH];
                    WIN32_FIND_DATA FindFileData2;
                    HANDLE hFind2;

                    strcpy(szSearchFile,GetSkinDirectory());
                    strcat(szSearchFile,FindFileData.cFileName);
                    strcat(szSearchFile,"\\skin.ini");
                    if ((hFind2=FindFirstFile(szSearchFile, &FindFileData2)) != INVALID_HANDLE_VALUE)
                    {
                        FindClose(hFind2);
                        vSkinNameList.push_back(FindFileData.cFileName);
                    }
                }
            } while (FindNextFile(hFind, &FindFileData));
            FindClose(hFind);       
        }        

        //Make menu

        // Find submenu
        char string[256];
        string[0] = '\0';
        GetMenuString(hMenu, 2, string, sizeof(string), MF_BYPOSITION);
        int reduc1 = !strcmp(string, "&Channels") ? 0 : 1;        
        HMENU hViewMenu = GetSubMenuWithName(hMenu, 3-reduc1, "&View");        
        HMENU hSkinMenu = NULL;
        for (int i = 0; i < GetMenuItemCount(hViewMenu); i++)
        {
            if (GetMenuItemID(GetSubMenu(hViewMenu, i), 0) == IDM_SKIN_NONE)
            {
                hSkinMenu = GetSubMenu(hViewMenu, i);
                break;
            }
        }

        if (hSkinMenu != NULL)
        {
            int num = GetMenuItemCount(hSkinMenu);
			int i;
            for (i = 2; i < num; i++)
            {
                DeleteMenu(hSkinMenu, 2, MF_BYPOSITION);
            }
            
            MENUITEMINFO MenuItemInfo;
            memset(&MenuItemInfo, 0, sizeof(MenuItemInfo));            
            for (i = 0; i < vSkinNameList.size(); i++)
            {
                MenuItemInfo.cbSize = sizeof (MenuItemInfo);
                MenuItemInfo.fMask = MIIM_TYPE | MIIM_ID;
                MenuItemInfo.fType = MFT_STRING;
                MenuItemInfo.cch = sizeof(vSkinNameList[i].c_str());
                MenuItemInfo.dwTypeData = (LPSTR)vSkinNameList[i].c_str();
                MenuItemInfo.wID = IDM_SKIN_FIRST + i;
                InsertMenuItem(hSkinMenu, i+2, TRUE, &MenuItemInfo);
            }
        }
    }

    CheckMenuItemBool(hMenu, IDM_SKIN_NONE, ((vSkinNameList.size()==0) || (szSkinName[0] == 0)));
        
    int Found = 0;
    for (int i = 0; i < vSkinNameList.size(); i++)
    {
        if ((Found == 0) && (vSkinNameList[i] == szSkinName))
        {
            Found = 1;
        }
        CheckMenuItemBool(hMenu, IDM_SKIN_FIRST+i, (Found==1));
        if (Found==1) 
        {
            Found=2;
        }
    }    
}

///**************************************************************************
//
//    FUNCTION: MainWndProc(HWND, unsigned, WORD, LONG)
//
//    PURPOSE:  Processes messages
//
//    MESSAGES:
//
//        WM_COMMAND    - application menu (About dialog box)
//        WM_CREATE     - create window and objects
//        WM_PAINT      - update window, draw objects
//        WM_DESTROY    - destroy window
//
//    COMMENTS:
//
//        Handles to the objects you will use are obtained when the WM_CREATE
//        message is received, and deleted when the WM_DESTROY message is
//        received.  The actual drawing is done whenever a WM_PAINT message is
//        received.
//
//
///**************************************************************************
LONG APIENTRY MainWndProc(HWND hWnd, UINT message, UINT wParam, LONG lParam)
{
    char Text[128];
    int i;
    BOOL bDone;
    ISetting* pSetting = NULL;
    ISetting* pSetting2 = NULL;
    ISetting* pSetting3 = NULL;
    ISetting* pSetting4 = NULL;

    if (message == MsgWheel)
    {
        // crack the mouse wheel delta
        // +ve is forward (away from user)
        // -ve is backward (towards user)
        if((short)wParam > 0)
        {
            if(GetKeyState(VK_SHIFT) < 0)
            {
                PostMessage(hWnd, WM_COMMAND, IDM_VOLUMEPLUS, 0);
            }
            else if(GetKeyState(VK_CONTROL) < 0)
            {
                PostMessage(hWnd, WM_COMMAND, IDM_PLAYLIST_PREVIOUS, 0);
            }
            else
            {
                PostMessage(hWnd, WM_COMMAND, IDM_CHANNELPLUS, 0);
            }
        }
        else
        {
            if(GetKeyState(VK_SHIFT) < 0)
            {
                PostMessage(hWnd, WM_COMMAND, IDM_VOLUMEMINUS, 0);
            }
            else if(GetKeyState(VK_CONTROL) < 0)
            {
                PostMessage(hWnd, WM_COMMAND, IDM_PLAYLIST_NEXT, 0);
            }
            else
            {
                PostMessage(hWnd, WM_COMMAND, IDM_CHANNELMINUS, 0);
            }
        }
    }
    else if(message == MsgOSDShow)
    {
        HANDLE hMapFile = NULL;
        char* lpMsg = NULL;
        char msg[1024];
        
        // show error message if an error occurred with the other instance of DScaler
        if(lParam != 0)
        {
            sprintf(msg, "Error processing incoming message. (#%i 0x%x)", lParam, wParam);
            LOG(0, msg);
            OSD_ShowTextPersistent(msg, 5);
        }
        else
        {
            hMapFile = OpenFileMapping(FILE_MAP_READ, FALSE, "DScalerSendMessageFileMappingObject");
            
            if(hMapFile == NULL) 
            { 
                sprintf(msg, "Error processing incoming message. (#10 0x%x)", GetLastError());
                LOG(0, msg); 
            } 
            else
            {
                lpMsg = (char*)MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 1024);              
                
                if (lpMsg == NULL) 
                { 
                    sprintf(msg, "Error processing incoming message. (#11 0x%x)", GetLastError());
                    LOG(0, msg); 
                }
                else
                {
                    strncpy(msg, lpMsg, sizeof(msg));
                }
            }
            // convert "\n" to newline characters
            char* s;
            while((s = strstr(msg,"\\n")) != NULL)
            {
                s[0] = '\n';
                strncpy(&s[1], &s[2], strlen(&s[1]));
            }
            
            if(wParam == 0 && hMapFile != NULL && lpMsg != NULL)
            {
                OSD_ShowText(msg, 5);
            }
            else
            {
                OSD_ShowTextPersistent(msg, 5);
            }
            
            if(lpMsg != NULL)
            {
                UnmapViewOfFile(lpMsg);
            }
            if(hMapFile != NULL)
            {
                CloseHandle(hMapFile);
            }
        }
        return 0;
    }
    else if(message == MsgTaskbarRestart)
    // the task bar has been restarted so the systray icon needs to be added again.
    {
        bIconOn = FALSE;
	    if (bMinToTray)
	    {
            SetTray(TRUE);
	    }
    }
    
    if (message == IDI_TRAYICON)
    {
        return On_IconHandler(wParam, lParam);
    }

    switch (message)
    {

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_MUTE:
            Audio_SetUserMute(!Audio_GetUserMute());
            ShowText(hWnd, Audio_GetUserMute() ? "MUTE" : "UNMUTE");
			break;

		case IDC_TOOLBAR_VOLUME_MUTE:
            Audio_SetUserMute(lParam);
            ShowText(hWnd, lParam ? "MUTE" : "UNMUTE");
            break;
            
        case IDM_VOLUMEPLUS:
            if (Audio_GetUserMute() == TRUE)
            {
                Audio_SetUserMute(FALSE);
                ShowText(hWnd, "UNMUTE");
            }
            else
            {
                if (!Mixer_IsEnabled())
                {
                    ISetting* pSetting = Providers_GetCurrentSource()->GetVolume();
                    if(pSetting != NULL)
                    {
                        pSetting->ChangeValue(ADJUSTUP);						
                    }
                    else
                    {
                        strcpy(Text, "Volume not supported");
                        ShowText(hWnd, Text);
                    }
                }
                else
                {
                    Mixer_Volume_Up();
                    sprintf(Text, "Mixer-Volume %d", Mixer_GetVolume());
                    ShowText(hWnd, Text);					
				}
            }
            break;

        case IDM_VOLUMEMINUS:
            if (Audio_GetUserMute() == TRUE)
            {
                Audio_SetUserMute(FALSE);
                ShowText(hWnd, "UNMUTE");
            }
            else
            {
                if (!Mixer_IsEnabled())
                {
                    ISetting* pSetting = Providers_GetCurrentSource()->GetVolume();
                    if(pSetting != NULL)
                    {
                        pSetting->ChangeValue(ADJUSTDOWN);
                    }
                    else
                    {
                        strcpy(Text, "Volume not supported");
                        ShowText(hWnd, Text);
                    }
                }
                else
                {
                    Mixer_Volume_Down();
                    sprintf(Text, "Mixer-Volume %d", Mixer_GetVolume());
                    ShowText(hWnd, Text);
                }
            }
            break;

		case IDC_TOOLBAR_VOLUME_SLIDER:
            if (Audio_GetUserMute() == TRUE)
            {
                Audio_SetUserMute(FALSE);
            }
            if (!Mixer_IsEnabled())
            {
                ISetting* pSetting = Providers_GetCurrentSource()->GetVolume();
                if(pSetting != NULL)
                {
                    pSetting->SetValue(lParam);
                    //sprintf(Text, "BT-Volume %d", pSetting->GetValue() / 10);
                }
                else
                {
                    strcpy(Text, "Volume not supported");
                    ShowText(hWnd, Text);
                }
            }
            else
            {
                extern void Mixer_SetVolume(long volume);
            
                Mixer_SetVolume(lParam);
                sprintf(Text, "Mixer-Volume %d", Mixer_GetVolume());
                ShowText(hWnd, Text);
            }
			break;
	
        case IDM_AUTO_FORMAT:
            Setting_SetValue(Timing_GetSetting(AUTOFORMATDETECT), 
                !Setting_GetValue(Timing_GetSetting(AUTOFORMATDETECT)));
            break;

        case IDM_VT_SEARCH:
        case IDM_VT_SEARCHNEXT:
        case IDM_VT_SEARCHPREV:
            ProcessVTMessage(hWnd, message, wParam, lParam);
            break;

        case IDM_VT_PAGE_MINUS:
			if (pMultiFrames)
			{
				pMultiFrames->HandleWindowsCommands(hWnd, wParam, lParam);
			}
			else if (!ProcessVTMessage(hWnd, message, wParam, lParam))
			{
				ProcessAspectRatioSelection(hWnd, LOWORD(wParam));
			}
            break;

        case IDM_VT_PAGE_PLUS:
			if (pMultiFrames)
			{
				pMultiFrames->HandleWindowsCommands(hWnd, wParam, lParam);
			}
			else if (!ProcessVTMessage(hWnd, message, wParam, lParam))
			{
				ProcessAspectRatioSelection(hWnd, LOWORD(wParam));
			}
            break;

        case IDM_VT_PAGE_UP:
			if (pMultiFrames)
			{
				pMultiFrames->HandleWindowsCommands(hWnd, wParam, lParam);
			}
			else if(!ProcessVTMessage(hWnd, message, wParam, lParam))
			{
				ProcessAspectRatioSelection(hWnd, LOWORD(wParam));
			}
            break;

        case IDM_VT_PAGE_DOWN:
			if (pMultiFrames)
			{
				pMultiFrames->HandleWindowsCommands(hWnd, wParam, lParam);
			}
			else if(!ProcessVTMessage(hWnd, message, wParam, lParam))
			{
				ProcessAspectRatioSelection(hWnd, LOWORD(wParam));
			}
            break;

        case IDM_CHANNEL_LIST:
            if(Providers_GetCurrentSource() && Providers_GetCurrentSource()->HasTuner())
            {
				if(!Providers_GetCurrentSource()->IsInTunerMode())
				{
					SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_INPUT1, 0);
				}
				if (pMultiFrames && pMultiFrames->IsActive())
				{
					pMultiFrames->RequestSwitch();
				}
				DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_CHANNELLIST), hWnd, (DLGPROC) ProgramListProc);
				Channels_UpdateMenu(hMenu);
			}
            break;

        case IDM_CHANNELPLUS:
            if (!ProcessVTMessage(hWnd, message, wParam, lParam))
            {
                if (Providers_GetCurrentSource()->IsInTunerMode())
                {
                    // IDM_CHANNELPLUS and IDM_CHANNELMINUS are used
                    // as the hub of all user interface channel up/down
                    // commands.  Put bReverseChannelScroll check here
                    // so mousewheel/page keys/toolbar arrows are taken
                    // care of.
                    if (!bReverseChannelScroll)
                    {
                        Channel_Increment();
                    }
                    else
                    {
                        Channel_Decrement();
                    }
					if (pMultiFrames && pMultiFrames->IsActive())
					{
						// We sleep to be sure that the channel is correctly displayed
						// in the output thread before acknowledging the change of content
						Sleep(250);
						pMultiFrames->AckContentChange();
					}
                }
            }
            break;

        case IDM_CHANNELMINUS:
            if (!ProcessVTMessage(hWnd, message, wParam, lParam))
            {
                if (Providers_GetCurrentSource()->IsInTunerMode())
                {
                    if (!bReverseChannelScroll)
                    {
                        Channel_Decrement();
                    }
                    else
                    {
                        Channel_Increment();
                    }
					if (pMultiFrames && pMultiFrames->IsActive())
					{
						// We sleep to be sure that the channel is correctly displayed
						// in the output thread before acknowledging the change of content
						Sleep(250);
						pMultiFrames->AckContentChange();
					}
                }
            }
            break;

        case IDM_CHANNEL_PREVIOUS:
            if (!ProcessVTMessage(hWnd, message, wParam, lParam))
            {
                if (Providers_GetCurrentSource()->IsInTunerMode())
                {
                    Channel_Previous();
                }
            }
            break;

		case IDM_CHANNEL_PREVIEW:
            if (pMultiFrames)
			{
				pMultiFrames->RequestSwitch();
			}
			else if (Providers_GetCurrentSource() && Providers_GetCurrentSource()->IsInTunerMode())
            {
				pMultiFrames = new CMultiFrames(PREVIEW_CHANNELS, ChannelPreviewNbCols, ChannelPreviewNbRows, Providers_GetCurrentSource());
				pMultiFrames->RequestSwitch();
            }
			else
			{
				SendMessage(hWnd, WM_COMMAND, IDM_PLAYLIST_PREVIEW, 0);
			}
			break;

		case IDM_CHANNEL_INDEX:
			if (Providers_GetCurrentSource()->IsInTunerMode() && pMultiFrames && pMultiFrames->IsActive())
			{
				Channel_Change(lParam, 1);
				if (pMultiFrames && pMultiFrames->IsActive())
				{
					// We sleep to be sure that the channel is correctly displayed
					// in the output thread before acknowledging the change of content
					Sleep(250);
					pMultiFrames->AckContentChange();
				}
			}
			break;

        case IDC_TOOLBAR_CHANNELS_LIST:
            if (!ProcessVTMessage(hWnd, message, wParam, lParam))
            {
                if (Providers_GetCurrentSource()->IsInTunerMode())
                {
                    Channel_Change(lParam);
                }
            }
            break;
            
        case IDM_PATTERN_SELECT:
            pCalibration->SelectTestPattern(lParam);
            break;

        case IDM_START_AUTO_CALIBRATION:
            pCalibration->Start(CAL_AUTO_FULL);
            break;

        case IDM_START_AUTO_CALIBRATION2:
            pCalibration->Start(CAL_AUTO_BRIGHT_CONTRAST);
            break;

        case IDM_START_AUTO_CALIBRATION3:
            pCalibration->Start(CAL_AUTO_COLOR);
            break;

        case IDM_START_MANUAL_CALIBRATION:
            pCalibration->Start(CAL_MANUAL);
            break;

        case IDM_START_YUV_RANGE:
            pCalibration->Start(CAL_CHECK_YUV_RANGE);
            break;

        case IDM_STOP_CALIBRATION:
            pCalibration->Stop();
            break;

        case IDM_RESET:
            Reset_Capture();
            Sleep(100);
            Providers_GetCurrentSource()->UnMute();
            break;

        case IDM_TOGGLE_MENU:
            bShowMenu = !bShowMenu;
            UpdateWindowState();
            WorkoutOverlaySize(TRUE);
            break;

        case IDM_SLEEPMODE:
            UpdateSleepMode(&SMState, Text);
            ShowText(hWnd, Text);
            break;
        
        case IDM_AUTODETECT:
            KillTimer(hWnd, TIMER_FINDPULL);
            if(Setting_GetValue(OutThreads_GetSetting(AUTODETECT)))
            {
                ShowText(hWnd, "Auto Pulldown Detect OFF");
                Setting_SetValue(OutThreads_GetSetting(AUTODETECT), FALSE);
            }
            else
            {
                ShowText(hWnd, "Auto Pulldown Detect ON");
                Setting_SetValue(OutThreads_GetSetting(AUTODETECT), TRUE);
            }
            // Set Deinterlace Mode to film fallback in
            // either case
            if(GetTVFormat(Providers_GetCurrentSource()->GetFormat())->Is25fps)
            {
                SetVideoDeinterlaceIndex(Setting_GetValue(FD50_GetSetting(PALFILMFALLBACKMODE)));
            }
            else
            {
                SetVideoDeinterlaceIndex(Setting_GetValue(FD60_GetSetting(NTSCFILMFALLBACKMODE)));
            }
            break;

        case IDM_FINDLOCK_PULL:
            if(!Setting_GetValue(OutThreads_GetSetting(AUTODETECT)))
            {
                Setting_SetValue(OutThreads_GetSetting(AUTODETECT), TRUE);
            }
            // Set Deinterlace Mode to film fallback in
            // either case
            if(GetTVFormat(Providers_GetCurrentSource()->GetFormat())->Is25fps)
            {
                SetVideoDeinterlaceIndex(Setting_GetValue(FD50_GetSetting(PALFILMFALLBACKMODE)));
            }
            else
            {
                SetVideoDeinterlaceIndex(Setting_GetValue(FD60_GetSetting(NTSCFILMFALLBACKMODE)));
            }
            SetTimer(hWnd, TIMER_FINDPULL, TIMER_FINDPULL_MS, NULL);
//            ShowText(hWnd, "Searching Film mode ...");
            break;

        case IDM_FALLBACK:
            if(Setting_GetValue(FD60_GetSetting(FALLBACKTOVIDEO)))
            {
                ShowText(hWnd, "Fallback on Bad Pulldown OFF");
                Setting_SetValue(FD60_GetSetting(FALLBACKTOVIDEO), FALSE);
            }
            else
            {
                ShowText(hWnd, "Fallback on Bad Pulldown ON");
                Setting_SetValue(FD60_GetSetting(FALLBACKTOVIDEO), TRUE);
            }
            break;

        case IDM_22PULLODD:
        case IDM_22PULLEVEN:
        case IDM_32PULL1:
        case IDM_32PULL2:
        case IDM_32PULL3:
        case IDM_32PULL4:
        case IDM_32PULL5:
            KillTimer(hWnd, TIMER_FINDPULL);
            Setting_SetValue(OutThreads_GetSetting(AUTODETECT), FALSE);
            SetFilmDeinterlaceMode((eFilmPulldownMode)(LOWORD(wParam) - IDM_22PULLODD));
            ShowText(hWnd, GetDeinterlaceModeName());
            break;

        case IDM_ABOUT:
            DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutProc);
            break;

        case IDM_BRIGHTNESS_PLUS:
            if((pSetting = Providers_GetCurrentSource()->GetBrightness()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTUP_SILENT);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_BRIGHTNESS_CURRENT, 0);
            break;

        case IDM_BRIGHTNESS_MINUS:
            if((pSetting = Providers_GetCurrentSource()->GetBrightness()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTDOWN_SILENT);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_BRIGHTNESS_CURRENT, 0);
            break;

        case IDM_BRIGHTNESS_CURRENT:
            if((pSetting = Providers_GetCurrentSource()->GetBrightness()) != NULL)
            {
                pSetting->OSDShow();
            }
            else
            {
                ShowText(hWnd, "No Brightness Control");
            }
            break;

        case IDM_KONTRAST_PLUS:
            if((pSetting = Providers_GetCurrentSource()->GetContrast()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTUP_SILENT);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_KONTRAST_CURRENT, 0);
            break;

        case IDM_KONTRAST_MINUS:
            if((pSetting = Providers_GetCurrentSource()->GetContrast()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTDOWN_SILENT);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_KONTRAST_CURRENT, 0);
            break;

        case IDM_KONTRAST_CURRENT:
            if((pSetting = Providers_GetCurrentSource()->GetContrast()) != NULL)
            {
                pSetting->OSDShow();
            }
            else
            {
                ShowText(hWnd, "No Contrast Control");
            }
            break;

        case IDM_USATURATION_PLUS:
            if((pSetting = Providers_GetCurrentSource()->GetSaturationU()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTUP_SILENT);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_USATURATION_CURRENT, 0);
            break;
        
        case IDM_USATURATION_MINUS:
            if((pSetting = Providers_GetCurrentSource()->GetSaturationU()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTDOWN_SILENT);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_USATURATION_CURRENT, 0);
            break;

        case IDM_USATURATION_CURRENT:
            if((pSetting = Providers_GetCurrentSource()->GetSaturationU()) != NULL)
            {
                pSetting->OSDShow();
            }
            else
            {
                ShowText(hWnd, "No Saturation U Control");
            }
            break;
        
        case IDM_VSATURATION_PLUS:
            if((pSetting = Providers_GetCurrentSource()->GetSaturationV()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTUP_SILENT);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_VSATURATION_CURRENT, 0);
            break;

        case IDM_VSATURATION_MINUS:
            if((pSetting = Providers_GetCurrentSource()->GetSaturationV()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTDOWN_SILENT);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_VSATURATION_CURRENT, 0);
            break;

        case IDM_VSATURATION_CURRENT:
            if((pSetting = Providers_GetCurrentSource()->GetSaturationV()) != NULL)
            {
                pSetting->OSDShow();
            }
            else
            {
                ShowText(hWnd, "No Saturation V Control");
            }
            break;

        case IDM_COLOR_PLUS:
            if((pSetting = Providers_GetCurrentSource()->GetSaturation()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTUP_SILENT);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_COLOR_CURRENT, 0);
            break;

        case IDM_COLOR_MINUS:
            if((pSetting = Providers_GetCurrentSource()->GetSaturation()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTDOWN_SILENT);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_COLOR_CURRENT, 0);
            break;

        case IDM_COLOR_CURRENT:
            if((pSetting = Providers_GetCurrentSource()->GetSaturation()) != NULL)
            {
                pSetting->OSDShow();
            }
            else
            {
                ShowText(hWnd, "No Saturation Control");
            }
            break;

        case IDM_HUE_PLUS:
            if((pSetting = Providers_GetCurrentSource()->GetHue()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTUP_SILENT);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_HUE_CURRENT, 0);
            break;

        case IDM_HUE_MINUS:
            if((pSetting = Providers_GetCurrentSource()->GetHue()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTDOWN_SILENT);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_HUE_CURRENT, 0);
            break;

        case IDM_HUE_CURRENT:
            if((pSetting = Providers_GetCurrentSource()->GetHue()) != NULL)
            {
                pSetting->OSDShow();
            }
            else
            {
                ShowText(hWnd, "No Hue Control");
            }
            break;

        case IDM_OVERSCAN_PLUS:
			pSetting = Providers_GetCurrentSource()->GetTopOverscan();
			pSetting2 = Providers_GetCurrentSource()->GetBottomOverscan();
			pSetting3 = Providers_GetCurrentSource()->GetLeftOverscan();
			pSetting4 = Providers_GetCurrentSource()->GetRightOverscan();
			if ( ( (pSetting == NULL) || (pSetting->GetValue() != pSetting->GetMax()) )
			  && ( (pSetting2 == NULL) || (pSetting2->GetValue() != pSetting2->GetMax()) )
			  && ( (pSetting3 == NULL) || (pSetting3->GetValue() != pSetting3->GetMax()) )
			  && ( (pSetting4 == NULL) || (pSetting4->GetValue() != pSetting4->GetMax()) ) )
			{
				if(pSetting != NULL)
				{
					pSetting->ChangeValue(ADJUSTUP_SILENT);
				}
				if(pSetting2 != NULL)
				{
					pSetting2->ChangeValue(ADJUSTUP_SILENT);
				}
				if(pSetting3 != NULL)
				{
					pSetting3->ChangeValue(ADJUSTUP_SILENT);
				}
				if(pSetting4 != NULL)
				{
					pSetting4->ChangeValue(ADJUSTUP_SILENT);
				}
			}
            SendMessage(hWnd, WM_COMMAND, IDM_OVERSCAN_CURRENT, 0);
            break;

        case IDM_OVERSCAN_MINUS:
			pSetting = Providers_GetCurrentSource()->GetTopOverscan();
			pSetting2 = Providers_GetCurrentSource()->GetBottomOverscan();
			pSetting3 = Providers_GetCurrentSource()->GetLeftOverscan();
			pSetting4 = Providers_GetCurrentSource()->GetRightOverscan();
			if ( ( (pSetting == NULL) || (pSetting->GetValue() != pSetting->GetMin()) )
			  && ( (pSetting2 == NULL) || (pSetting2->GetValue() != pSetting2->GetMin()) )
			  && ( (pSetting3 == NULL) || (pSetting3->GetValue() != pSetting3->GetMin()) )
			  && ( (pSetting4 == NULL) || (pSetting4->GetValue() != pSetting4->GetMin()) ) )
			{
				if(pSetting != NULL)
				{
					pSetting->ChangeValue(ADJUSTDOWN_SILENT);
				}
				if(pSetting2 != NULL)
				{
					pSetting2->ChangeValue(ADJUSTDOWN_SILENT);
				}
				if(pSetting3 != NULL)
				{
					pSetting3->ChangeValue(ADJUSTDOWN_SILENT);
				}
				if(pSetting4 != NULL)
				{
					pSetting4->ChangeValue(ADJUSTDOWN_SILENT);
				}
			}
            SendMessage(hWnd, WM_COMMAND, IDM_OVERSCAN_CURRENT, 0);
            break;

        case IDM_OVERSCAN_CURRENT:
            if(Providers_GetCurrentSource()->GetTopOverscan() == NULL
            && Providers_GetCurrentSource()->GetBottomOverscan() == NULL
            && Providers_GetCurrentSource()->GetLeftOverscan() == NULL
            && Providers_GetCurrentSource()->GetRightOverscan() == NULL)
            {
                ShowText(hWnd, "No Overscan Control");
            }
            else
			{
				strcpy(Text, "Overscan");
				if((pSetting = Providers_GetCurrentSource()->GetTopOverscan()) != NULL)
				{
					sprintf(&Text[strlen(Text)], "\nTop %u", pSetting->GetValue());
				}
				if((pSetting = Providers_GetCurrentSource()->GetBottomOverscan()) != NULL)
				{
					sprintf(&Text[strlen(Text)], "\nBottom %u", pSetting->GetValue());
				}
				if((pSetting = Providers_GetCurrentSource()->GetLeftOverscan()) != NULL)
				{
					sprintf(&Text[strlen(Text)], "\nLeft %u", pSetting->GetValue());
				}
				if((pSetting = Providers_GetCurrentSource()->GetRightOverscan()) != NULL)
				{
					sprintf(&Text[strlen(Text)], "\nRight %u", pSetting->GetValue());
				}
				OSD_ShowText(Text, 0);
            }
            break;

        case IDM_TOGGLECURSOR:
            if(!bAutoHideCursor && bIsFullScreen == FALSE)
            {
                bShowCursor = !bShowCursor;
                Cursor_UpdateVisibility();
            }
            break;

        case IDM_END:
            ShowWindow(hWnd, SW_HIDE);
            PostMessage(hWnd, WM_DESTROY, wParam, lParam);
            break;

        case IDM_TRAYEND:
            SendMessage(hWnd, WM_COMMAND, IDM_END, NULL);
            break;

        case IDM_VBI_VT:
            if (VT_GetState() != VT_OFF)
            {
                VT_SetState(NULL, NULL, VT_OFF);
                Cursor_VTUpdate();
                InvalidateDisplayAreaRect(hWnd, NULL, FALSE);
            }
            Setting_SetValue(VBI_GetSetting(DOTELETEXT),
                !Setting_GetValue(VBI_GetSetting(DOTELETEXT)));
            break;

        case IDM_CCOFF:
        case IDM_CC1:
        case IDM_CC2:
        case IDM_CC3:
        case IDM_CC4:
        case IDM_TEXT1:
        case IDM_TEXT2:
        case IDM_TEXT3:
        case IDM_TEXT4:
            Setting_SetValue(VBI_GetSetting(CLOSEDCAPTIONMODE), 
                LOWORD(wParam) - IDM_CCOFF);
            break;

        case IDM_VBI_VPS:
            Setting_SetValue(VBI_GetSetting(DOVPS), 
                !Setting_GetValue(VBI_GetSetting(DOVPS)));
            break;

        case IDM_VBI_WSS:
            Setting_SetValue(VBI_GetSetting(DOWSS), 
                !Setting_GetValue(VBI_GetSetting(DOWSS)));
            break;

        case IDM_CALL_VIDEOTEXT:
            {
                eVTState NewState;

                switch (VT_GetState())
                {
                case VT_OFF:
                    NewState = VT_BLACK;
                    break;
                case VT_BLACK:
                    NewState = (bVTSingleKeyToggle ? VT_MIXED : VT_OFF);
                    break;
                case VT_MIXED:
                default:
                    NewState = VT_OFF;
                    break;
                }
            
                if (NewState != VT_OFF)
                {
                    if (!Setting_GetValue(VBI_GetSetting(CAPTURE_VBI)))
                    {
                        SendMessage(hWnd, WM_COMMAND, IDM_VBI, 0);
                    }
                    if (!Setting_GetValue(VBI_GetSetting(DOTELETEXT)))
                    {
                        SendMessage(hWnd, WM_COMMAND, IDM_VBI_VT, 0);
                    }
                }

                VT_SetState(NULL, NULL, NewState);
                Cursor_VTUpdate();
                WorkoutOverlaySize(TRUE);
                InvalidateDisplayAreaRect(hWnd, NULL, FALSE);

                if (NewState == VT_OFF)
                {
                    OSD_ShowText("VideoText OFF", 0);
                }
                else
                {
                    OSD_Clear();
                }
            }
            *ChannelString = '\0';
            break;

        case IDM_VT_MIXEDMODE:
            {
                eVTState NewState;

                if (VT_GetState() == VT_MIXED)
                {
                    NewState = VT_BLACK;
                }
                else
                {
                    NewState = VT_MIXED;
                }

                if (!Setting_GetValue(VBI_GetSetting(CAPTURE_VBI)))
                {
                    SendMessage(hWnd, WM_COMMAND, IDM_VBI, 0);
                }
                if (!Setting_GetValue(VBI_GetSetting(DOTELETEXT)))
                {
                    SendMessage(hWnd, WM_COMMAND, IDM_VBI_VT, 0);
                }

                VT_SetState(NULL, NULL, NewState);
                Cursor_VTUpdate();
                WorkoutOverlaySize(TRUE);
                InvalidateDisplayAreaRect(hWnd, NULL, FALSE);
            }
            *ChannelString = '\0';
            break;

        case IDM_VT_RESET:
            VT_ChannelChange();

            if (VT_GetState() != VT_OFF)
            {
                InvalidateDisplayAreaRect(hWnd, NULL, FALSE);
            }
            break;

        case IDM_VIDEOSETTINGS:
            DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_VIDEOSETTINGS), hWnd, VideoSettingProc);
            break;

        case IDM_SIZESETTINGS:
            DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_SIZESETTINGS), hWnd, SizeSettingProc);
            break;

        case IDM_VPS_OUT:
            DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_VPSSTATUS), hWnd, VPSInfoProc);
            break;

        case IDM_VT_OUT:
            DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_VTSTATUS), hWnd, VTInfoProc);
            break;

        case IDM_VT_GOTO:
            DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_VTGOTO), hWnd, VTGotoProc);
            break;

        case IDM_VBI:
            if (VT_GetState() != VT_OFF)
            {
                VT_SetState(NULL, NULL, VT_OFF);
                Cursor_VTUpdate();
                InvalidateDisplayAreaRect(hWnd, NULL, FALSE);
            }
            Stop_Capture();
            Setting_SetValue(VBI_GetSetting(CAPTURE_VBI),
                !Setting_GetValue(VBI_GetSetting(CAPTURE_VBI)));
            Start_Capture();
            break;

        case IDM_VT_SEARCHHIGHLIGHT:
            Setting_SetValue(VBI_GetSetting(SEARCHHIGHLIGHT), 
                !Setting_GetValue(VBI_GetSetting(SEARCHHIGHLIGHT)));
            InvalidateDisplayAreaRect(hWnd, NULL, FALSE);
            break;

        case IDM_CAPTURE_PAUSE:
            Pause_Toggle_Capture();
            break;

        case IDM_AUDIO_MIXER:
            Mixer_SetupDlg(hWnd);
            break;

        case IDM_STATUSBAR:
            DisplayStatusBar_OnChange(!bDisplayStatusBar);
            break;

        case IDM_ON_TOP:
            bAlwaysOnTop = !bAlwaysOnTop;
            WorkoutOverlaySize(FALSE);
            break;

        case IDM_ALWAYONTOPFULLSCREEN:
            bAlwaysOnTopFull = !bAlwaysOnTopFull;
            WorkoutOverlaySize(FALSE);
            break;

        case IDM_VT_AUTOCODEPAGE:
            if (!ProcessVTMessage(hWnd, message, wParam, lParam))
            {
                // IDM_VT_AUTOCODEPAGE won't be handled in
                // ProcessVTMessage if VTState is off
                VT_SetAutoCodepage(NULL, NULL, !VT_GetAutoCodepage());
            }
            bVTAutoCodePage = VT_GetAutoCodepage();
            break;
        
        case IDM_VT_ANTIALIAS:
            if (!ProcessVTMessage(hWnd, message, wParam, lParam))
            {
                // IDM_VT_ANTIALIAS won't be handled in
                // ProcessVTMessage if VTState is off
                VT_SetAntialias(NULL, NULL, !VT_GetAntialias());
            }
            bVTAntiAlias = VT_GetAntialias();
            break;

        case IDM_KEYBOARDLOCK:
            bKeyboardLock = !bKeyboardLock;
            SetKeyboardLock(bKeyboardLock);
            if(bKeyboardLock)
            {
                OSD_ShowText("Keyboard lock on", 0);
            }
            else
            {
                OSD_ShowText("Keyboard lock off", 0);
            }
            
            break;

        case IDM_TREADPRIOR_0:
        case IDM_TREADPRIOR_1:
        case IDM_TREADPRIOR_2:
        case IDM_TREADPRIOR_3:
        case IDM_TREADPRIOR_4:
            ThreadClassId = LOWORD(wParam) - IDM_TREADPRIOR_0;
            Stop_Capture();
            Start_Capture();
            break;

        case IDM_PRIORCLASS_0:
        case IDM_PRIORCLASS_1:
        case IDM_PRIORCLASS_2:
            PriorClassId = LOWORD(wParam) - IDM_PRIORCLASS_0;
            strcpy(Text, "Can't set Priority");
            if (PriorClassId == 0)
            {
                if (SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS) == TRUE)
                    strcpy(Text, "Normal Priority");
            }
            else if (PriorClassId == 1)
            {
                if (SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS) == TRUE)
                    strcpy(Text, "High Priority");
            }
            else
            {
                if (SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS) == TRUE)
                    strcpy(Text, "Real-Time Priority");
            }
            ShowText(hWnd, Text);
            break;

        case IDM_JUDDERTERMINATOR:
            Stop_Capture();
            Setting_SetValue(OutThreads_GetSetting(DOACCURATEFLIPS), 
                !Setting_GetValue(OutThreads_GetSetting(DOACCURATEFLIPS)));
            Start_Capture();
            break;

        case IDM_USECHROMA:
            Stop_Capture();
            Setting_SetValue(FD_Common_GetSetting(USECHROMA), 
                !Setting_GetValue(FD_Common_GetSetting(USECHROMA)));
            Start_Capture();
            break;

        case IDM_SPACEBAR:
            if(!Setting_GetValue(OutThreads_GetSetting(AUTODETECT)))
            {
                IncrementDeinterlaceMode();
                ShowText(hWnd, GetDeinterlaceModeName());
            }
            break;

        case IDM_SHIFT_SPACEBAR:
            if (!Setting_GetValue(OutThreads_GetSetting(AUTODETECT)))
            {
                DecrementDeinterlaceMode();
                ShowText(hWnd, GetDeinterlaceModeName());
            }
            break;

        case IDM_FULL_SCREEN:
            IsFullScreen_OnChange(!bIsFullScreen);
			WorkoutOverlaySize(TRUE);
            break;
        
        case IDM_RETURN_TO_WINDOW:
            if(bIsFullScreen)
            {
                IsFullScreen_OnChange(FALSE);
            }
            else
            {
                ShowWindow(hWnd, SW_MINIMIZE);
            }
            break;

        case IDM_MINTOTRAY:
            bMinToTray = !bMinToTray;
            SetTray(bMinToTray);
            break;

        case IDM_TRAYSHOW:
            SendMessage(hWnd, IDI_TRAYICON, 0, WM_LBUTTONDBLCLK);
            break;

        case IDM_TAKESTREAMSNAP:
            RequestStreamSnap();
            break;

        case IDM_TAKESTILL:
            RequestStill(1);
            break;

        case IDM_TAKECONSECUTIVESTILL:
			// Take cpnsecutive stills
            RequestStill((eStillFormat)Setting_GetValue(Still_GetSetting(NBCONSECUTIVESTILLS)));
            break;

        case IDM_TAKECYCLICSTILL:
            bTakingCyclicStills = !bTakingCyclicStills;
            KillTimer(hWnd, TIMER_TAKESTILL);
            if (bTakingCyclicStills)
            {
                RequestStill(1);
                SetTimer(hWnd, TIMER_TAKESTILL, Setting_GetValue(Still_GetSetting(DELAYBETWEENSTILLS)) * 1000, NULL);
            }
            break;

        case IDM_RESET_STATS:
            ResetDeinterlaceStats();
            ResetARStats();
            pPerf->Reset();
            ShowText(hWnd, "Statistics reset");
            break;

        case IDM_TSOPTIONS:
            CTimeShift::OnOptions();
            break;

        case IDM_TSRECORD:
            if (CTimeShift::OnRecord())
            {
                ShowText(hWnd, "Recording");
                CTimeShift::OnSetMenu(hMenu);
            }
            break;

        case IDM_TSSTOP:
            if (CTimeShift::OnStop())
            {
                ShowText(hWnd, "Stopped");
                CTimeShift::OnSetMenu(hMenu);
            }
            break;

        case IDM_TSPLAY:
            if (CTimeShift::OnPlay())
            {
                ShowText(hWnd, "Playing");
                CTimeShift::OnSetMenu(hMenu);
            }
            break;

        case IDM_TSPAUSE:
            if (CTimeShift::OnPause())
            {
                ShowText(hWnd, "Paused");
                CTimeShift::OnSetMenu(hMenu);
            }
            break;

        case IDM_TSFFWD:
            if (CTimeShift::OnFastForward())
            {
                ShowText(hWnd, "Scanning >>>");
                CTimeShift::OnSetMenu(hMenu);
            }
            break;

        case IDM_TSRWND:
            if (CTimeShift::OnFastBackward())
            {
                ShowText(hWnd, "Scanning <<<");
                CTimeShift::OnSetMenu(hMenu);
            }
            break;

        case IDM_TSNEXT:
            if (CTimeShift::OnGoNext())
            {
                ShowText(hWnd, "Next Clip");
                CTimeShift::OnSetMenu(hMenu);
            }
            break;

        case IDM_TSPREV:
            if (CTimeShift::OnGoPrev())
            {
                ShowText(hWnd, "Previous Clip");
                CTimeShift::OnSetMenu(hMenu);
            }
            break;

        case IDM_SHOW_INFOS:
            OSD_ShowSourceComments();
            break;

        case IDM_SET_OSD_TEXT:
            // Useful for external programs for custom control of dTV's OSD display
            // Such as macros in software such as Girder, etc.
            if (lParam)
            {
                lstrcpy(Text, "");
                GlobalGetAtomName((ATOM) lParam, Text, sizeof(Text));
                OSD_ShowTextOverride(Text, 0);
                GlobalDeleteAtom((ATOM) lParam);
            }
            else
            {
                OSD_ShowTextOverride("", 0);
            }
            break;

        case IDM_SAVE_SETTINGS_NOW:
            WriteSettingsToIni(FALSE);
            break;

        case IDM_OSD_CC_TEXT:
            {
                RECT winRect;
                RECT DestRect;
                PAINTSTRUCT sPaint;
                GetDisplayAreaRect(hWnd, &winRect);                
                InvalidateRect(hWnd, &winRect, FALSE);
                BeginPaint(hWnd, &sPaint);
                PaintColorkey(hWnd, TRUE, sPaint.hdc, &winRect);
                GetDestRect(&DestRect);
                CC_PaintScreen(hWnd, (TCCScreen*)lParam, sPaint.hdc, &DestRect);
                EndPaint(hWnd, &sPaint);
                ValidateRect(hWnd, &winRect);
            }
            break;

        case IDM_OVERLAY_STOP:
            Overlay_Stop(hWnd);
            break;

        case IDM_OVERLAY_START:
            Overlay_Start(hWnd);
            break;

        case IDM_OVERLAYSETTINGS:
            if(!CanDoOverlayColorControl())
            {
                MessageBox(hWnd, "Overlay color control is not supported by your video card.",
                           "DScaler", MB_OK);
            }
            else
            {
                DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_OVERLAYSETTINGS), hWnd, OverlaySettingProc);
            }
            break;

        case IDM_USE_DSCALER_OVERLAY:
            Setting_SetValue(Other_GetSetting(USEOVERLAYCONTROLS), !Setting_GetValue(Other_GetSetting(USEOVERLAYCONTROLS)));
            break;

        case IDM_FAST_REPAINT:
            {
                RECT Rect;

                GetDisplayAreaRect(hWnd, &Rect);

                HDC hWndDC = GetDC(hWnd);
                HDC hDC = OffscreenHDC.BeginPaint(hWndDC, &Rect);

                if (VT_GetState() != VT_OFF)
                {
                    RECT VTDrawRect;
                    GetDestRect(&VTDrawRect);

                    PaintColorkey(hWnd, TRUE, hDC, &Rect, TRUE);

                    VT_Redraw(hDC, &VTDrawRect);
                }
                else
                {
                    PaintColorkey(hWnd, TRUE, hDC, &Rect);
                }

                OSD_Redraw(hDC, &Rect);

                if (!bIsFullScreen && (WindowBorder!=NULL) && WindowBorder->Visible())
                {
                    WindowBorder->Paint(hWnd, hDC, &Rect);
                }

                OffscreenHDC.EndPaint();
                ReleaseDC(hWnd, hWndDC);

                ValidateRect(hWnd, NULL);
            }
            break;

        case IDM_HELP_HOMEPAGE:
            ShellExecute(hWnd, "open", "http://www.dscaler.org/", NULL, NULL, SW_SHOWNORMAL);
            break;

        case IDM_HELP_FAQ:
            HtmlHelp(hWnd, "DScaler.chm::/FAQ.htm", HH_DISPLAY_TOPIC, 0);
            break;

        case IDM_HELP_SUPPORT:
            HtmlHelp(hWnd, "DScaler.chm::/user_support.htm", HH_DISPLAY_TOPIC, 0);
            break;

        case IDM_HELP_KEYBOARD:
            HtmlHelp(hWnd, "DScaler.chm::/keyboard.htm", HH_DISPLAY_TOPIC, 0);
            break;

        case IDM_HELP_GPL:
            HtmlHelp(hWnd, "DScaler.chm::/COPYING.html", HH_DISPLAY_TOPIC, 0);
            break;

        case IDM_HELP_README:
            HtmlHelp(hWnd, "DScaler.chm::/Help.htm", HH_DISPLAY_TOPIC, 0);
            break;

        case IDM_CREDITS:
            {
                CCredits CreditsDlg;
                CreditsDlg.DoModal();
            }
            break;

        case IDM_TELETEXT_KEY1:
        case IDM_TELETEXT_KEY2:
        case IDM_TELETEXT_KEY3:
        case IDM_TELETEXT_KEY4:
        case IDM_TELETEXT_KEY5:
        case IDM_TELETEXT_KEY6:
            ProcessVTMessage(hWnd, message, wParam, lParam);
            break;

        case IDM_LEFT_CROP_PLUS:
            if ( pCalibration->IsRunning()
              && (pCalibration->GetType() == CAL_MANUAL) )
            {
                Setting_Up(Calibr_GetSetting(LEFT_SOURCE_CROPPING));
                SendMessage(hWnd, WM_COMMAND, IDM_LEFT_CROP_CURRENT, 0);
            }
            break;

        case IDM_LEFT_CROP_MINUS:
            if ( pCalibration->IsRunning()
              && (pCalibration->GetType() == CAL_MANUAL) )
            {
                Setting_Down(Calibr_GetSetting(LEFT_SOURCE_CROPPING));
                SendMessage(hWnd, WM_COMMAND, IDM_LEFT_CROP_CURRENT, 0);
            }
            break;

        case IDM_LEFT_CROP_CURRENT:
            if ( pCalibration->IsRunning()
              && (pCalibration->GetType() == CAL_MANUAL) )
            {
                Setting_OSDShow(Calibr_GetSetting(LEFT_SOURCE_CROPPING), hWnd);
            }
            break;

        case IDM_RIGHT_CROP_PLUS:
            if ( pCalibration->IsRunning()
              && (pCalibration->GetType() == CAL_MANUAL) )
            {
                Setting_Up(Calibr_GetSetting(RIGHT_SOURCE_CROPPING));
                SendMessage(hWnd, WM_COMMAND, IDM_RIGHT_CROP_CURRENT, 0);
            }
            break;

        case IDM_RIGHT_CROP_MINUS:
            if ( pCalibration->IsRunning()
              && (pCalibration->GetType() == CAL_MANUAL) )
            {
                Setting_Down(Calibr_GetSetting(RIGHT_SOURCE_CROPPING));
                SendMessage(hWnd, WM_COMMAND, IDM_RIGHT_CROP_CURRENT, 0);
            }
            break;

        case IDM_RIGHT_CROP_CURRENT:
            if ( pCalibration->IsRunning()
              && (pCalibration->GetType() == CAL_MANUAL) )
            {
                Setting_OSDShow(Calibr_GetSetting(RIGHT_SOURCE_CROPPING), hWnd);
            }
            break;

        case IDM_CHARSET_TEST:
            ProcessVTMessage(hWnd, message, wParam, lParam);
            break;
		
        case IDM_SETTINGS_CHANGESETTINGS:
            CTreeSettingsDlg::ShowTreeSettingsDlg(ADVANCED_SETTINGS_MASK);
            break;

        case IDM_SETTINGS_FILTERSETTINGS:
            CTreeSettingsDlg::ShowTreeSettingsDlg(FILTER_SETTINGS_MASK);
            break;

        case IDM_SETTINGS_DEINTERLACESETTINGS:
            CTreeSettingsDlg::ShowTreeSettingsDlg(DEINTERLACE_SETTINGS_MASK);
            break;

        case ID_SETTINGS_SAVESETTINGSPERCHANNEL:
            Setting_SetValue(SettingsPerChannel_GetSetting(SETTINGSPERCHANNEL_BYCHANNEL), !Setting_GetValue(SettingsPerChannel_GetSetting(SETTINGSPERCHANNEL_BYCHANNEL)));
            break;

        case ID_SETTINGS_SAVESETTINGSPERINPUT:
            Setting_SetValue(SettingsPerChannel_GetSetting(SETTINGSPERCHANNEL_BYINPUT), !Setting_GetValue(SettingsPerChannel_GetSetting(SETTINGSPERCHANNEL_BYINPUT)));
            break;

        case ID_SETTINGS_SAVESETTINGSPERFORMAT:
            Setting_SetValue(SettingsPerChannel_GetSetting(SETTINGSPERCHANNEL_BYFORMAT), !Setting_GetValue(SettingsPerChannel_GetSetting(SETTINGSPERCHANNEL_BYFORMAT)));
            break;

        case IDM_DEINTERLACE_SHOWVIDEOMETHODUI:
            ShowVideoModeUI();
            break;
        
        case IDM_CLEAROSD:
            ProcessOSDMessage(hWnd, message, wParam, lParam);
            break;

        case IDM_SETUPHARDWARE:
            // Stop and start capture because of possible pixel width chaange
            Stop_Capture();
            DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_HWSETUP), hWnd, (DLGPROC) HardwareSettingProc, (LPARAM)1);
            Start_Capture();
            break;

        case IDM_SKIN_NONE:
            szSkinName[0] = 0;
			Skin_SetMenu(hMenu, TRUE);
            SetWindowBorder(hWnd, szSkinName, FALSE);
            if (ToolbarControl!=NULL)
            {
                ToolbarControl->Set(hWnd, szSkinName, bIsFullScreen?1:0);
            }
            UpdateWindowState();
			WorkoutOverlaySize(FALSE);
            InvalidateRect(hWnd, NULL, FALSE);
            break;
        
		default:
            bDone = FALSE;

            if ((LOWORD(wParam)>=IDM_SKIN_FIRST) && (LOWORD(wParam)<=IDM_SKIN_LAST))
            {
                int n = LOWORD(wParam)-IDM_SKIN_FIRST;
                if (n<vSkinNameList.size())
                {
                    strcpy(szSkinName, vSkinNameList[n].c_str());
                }
                Skin_SetMenu(hMenu, TRUE);
                SetWindowBorder(hWnd, szSkinName, TRUE);  
                if (ToolbarControl!=NULL)
                {
                    ToolbarControl->Set(hWnd, szSkinName, bIsFullScreen?1:0);
                }
                UpdateWindowState();            
				WorkoutOverlaySize(FALSE);
                InvalidateRect(hWnd, NULL, FALSE);
                bDone = TRUE;
            }

			if (pMultiFrames && !bDone)
			{
				bDone = pMultiFrames->HandleWindowsCommands(hWnd, wParam, lParam);
			}
            // Check whether menu ID is an aspect ratio related item
            if (!bDone)
            {
                bDone = ProcessAspectRatioSelection(hWnd, LOWORD(wParam));
            }
	        if(!bDone)
            {
                bDone = ProcessDeinterlaceSelection(hWnd, LOWORD(wParam));
            }
            if(!bDone)
            {
                bDone = ProcessFilterSelection(hWnd, LOWORD(wParam));
            }
            if(!bDone)
            {
                bDone = ProcessProgramSelection(hWnd, LOWORD(wParam));
            }
            if(!bDone)
            {
                bDone = ProcessOSDSelection(hWnd, LOWORD(wParam));
            }
            if(!bDone)
            {
                bDone = ProcessVTCodepageSelection(hWnd, LOWORD(wParam));
            }
            if(!bDone)
            {
                bDone = ProcessOutResoSelection(hWnd, LOWORD(wParam));
            }
            if(!bDone && pCalibration != NULL)
            {
                bDone = pCalibration->ProcessSelection(hWnd, LOWORD(wParam));
            }
            if(!bDone && ToolbarControl != NULL)
            {
                bDone = ToolbarControl->ProcessToolbar1Selection(hWnd, LOWORD(wParam));
				if (bDone)
				{
					if (IsToolBarVisible())
					{
						SetTimer(hWnd, TIMER_TOOLBAR, TIMER_TOOLBAR_MS, NULL);
					}
					else
					{
						KillTimer(hWnd, TIMER_TOOLBAR);
					}
				}
            }
            if(!bDone)
            {
                bDone = Providers_HandleWindowsCommands(hWnd, wParam, lParam);
            }
            break;
        }

        //-------------------------------------------------------
        // The following code executes on all WM_COMMAND calls

        // Updates the menu checkbox settings
        SetMenuAnalog();
        if (ToolbarControl!=NULL)
        {
            ToolbarControl->UpdateMenu(hMenu);
        }

        if(bUseAutoSave)
        {
            // Set the configuration file autosave timer.
            // We use an autosave timer so that when the user has finished
            // making adjustments and at least a small delay has occured,
            // that the DTV.INI file is properly up to date, even if 
            // the system crashes or system is turned off abruptly.
            KillTimer(hWnd, TIMER_AUTOSAVE);
            SetTimer(hWnd, TIMER_AUTOSAVE, TIMER_AUTOSAVE_MS, NULL);
        }
        return 0;
        break;

    case WM_CREATE:
        MainWndOnCreate(hWnd);
        return 0;
        break;

    case INIT_BT:
        MainWndOnInitBT(hWnd);
        break;

// 2000-10-31 Added by Mark Rejhon
// This was an attempt to allow dTV to run properly through
// computer resolution changes.  Alas, dTV still crashes and burns
// in a fiery dive if you try to change resolution while dTV is
// running.  We need to somehow capture a message that comes right
// before a resolution change, so we can destroy the video overlay
// on time beforehand.   This message seems to happen right after
// a resolution change.
//
//  case WM_DISPLAYCHANGE:
//      // Windows resolution changed while software running
//      Stop_Capture();
//      Overlay_Destroy();
//      Sleep(100);
//      Overlay_Create();
//      BT848_ResetHardware();
//      BT848_SetGeoSize();
//      WorkoutOverlaySize();
//      Start_Capture();
//      Sleep(100);
//      BT848_SetAudioSource(AudioSource);
//      break;

    case WM_POWERBROADCAST:
        // Handing to keep dTV running during computer suspend/resume
        switch ((DWORD) wParam)
        {
        case PBT_APMSUSPEND:
            // Stops video overlay upon suspend operation.
            Overlay_Stop(hWnd);
            break;
        case PBT_APMRESUMESUSPEND:
            // Restarts video overlay upon resume operation.
            // The following crashes unless we do either a HWND_BROADCAST
            // or a Sleep() operation.  To be on the safe side, I do both
            // here.  Perhaps the video overlay drivers needed to reinitialize.
            SendMessage(HWND_BROADCAST, WM_PAINT, 0, 0);
            Sleep(500);
            Overlay_Start(hWnd);
            break;
        }
        break;

    case WM_LBUTTONDBLCLK:
		if (bIgnoreDoubleClick == FALSE)
		{
	        SendMessage(hWnd, WM_COMMAND, IDM_FULL_SCREEN, 0);
		}
        return 0;
        break;

//  case WM_RBUTTONUP:
//      if(!bAutoHideCursor && bIsFullScreen == FALSE)
//      {
//          bShowCursor = !bShowCursor;
//          Cursor_UpdateVisibility();
//      }
//      break;

    case WM_LBUTTONDOWN:
        if (ProcessVTMessage(hWnd, message, wParam, lParam))
        {
			bIgnoreDoubleClick = TRUE;
        }
		else
		{
			bIgnoreDoubleClick = FALSE;
		}

        if((bShowMenu == FALSE || (GetKeyState(VK_CONTROL) < 0)) && bIsFullScreen == FALSE)
        {
            // pretend we are hitting the caption bar
            // this will allow the user to move the window
            // when the menu and title bar are hidden
            return DefWindowProc(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, lParam);
        }
        Cursor_UpdateVisibility();
        return 0;
        break;

    case WM_LBUTTONUP:
        Cursor_UpdateVisibility();
        return 0;
    case WM_RBUTTONDOWN:
        Cursor_UpdateVisibility();
        bIsRightButtonDown = TRUE;
        return 0;
    case WM_RBUTTONUP:
        Cursor_UpdateVisibility();
        bIsRightButtonDown = FALSE;
        break;

    case WM_MOUSEMOVE:
        {
            static int x = -1;
            static int y = -1;
            int newx = GET_X_LPARAM(lParam);
            int newy = GET_Y_LPARAM(lParam);
            
            if (x != newx || y != newy)
            {
                x = newx;
                y = newy;
                Cursor_UpdateVisibility();
                if (VT_GetState() != VT_OFF)
                {
                    Cursor_VTUpdate(newx, newy);
                }
				if (bIsFullScreen && ToolbarControl != NULL)
				{
					POINT Point;
					Point.x = x;
					Point.y = y;
					if (ToolbarControl->AutomaticDisplay(Point))
					{
						if (IsToolBarVisible())
						{
							SetTimer(hWnd, TIMER_TOOLBAR, TIMER_TOOLBAR_MS, NULL);
						}
						else
						{
							KillTimer(hWnd, TIMER_TOOLBAR);
						}
					}
				}
            }
        }
        return 0;
        break;

    case WM_NCMOUSEMOVE:
        Cursor_UpdateVisibility();
        return 0;
        break;

    case WM_ENTERMENULOOP:
        bInMenu = TRUE;
        Cursor_UpdateVisibility();
        return 0;
        break;

    case WM_EXITMENULOOP:
        bInMenu = FALSE;
        Cursor_UpdateVisibility();
        return 0;
        break;

    case WM_INITMENU: 
        SetMenuAnalog();
        return 0;
        break;

    case WM_CONTEXTMENU:
        if(bIgnoreNextRightButtonUpMsg)
        {
            bIgnoreNextRightButtonUpMsg = FALSE;
            return 0;
        }
        if (!OnContextMenu(hWnd, GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)))
            return DefWindowProc(hWnd, message, wParam, lParam);
        break;

    case WM_KILLFOCUS:
	    Cursor_UpdateVisibility();
        return 0;
        break;

    case WM_SETFOCUS:
	    Cursor_UpdateVisibility();
        return 0;
        break;

    case WM_TIMER:
        pPerf->Suspend();

        switch (LOWORD(wParam))
        {
        //-------------------------------
        case TIMER_STATUS:
            if (IsStatusBarVisible() && (Providers_GetCurrentSource() != NULL))
            {
                if (Setting_GetValue(Audio_GetSetting(SYSTEMINMUTE)) == TRUE)
	    		{
                    strcpy(Text, "Volume Mute");
			    }
                else if (!Providers_GetCurrentSource()->IsVideoPresent())
                {
                    strcpy(Text, "No Video Signal Found");
                }
                else
                {
					memset(Text,0,128);
                    strncpy(Text, Providers_GetCurrentSource()->GetStatus(),128);
                    if(Text[0] == 0x00)
                    {
                        if(Providers_GetCurrentSource()->IsInTunerMode())
                        {
                            strncpy(Text, Channel_GetName(),128);
                        }
                    }
                }
                StatusBar_ShowText(STATUS_TEXT, Text);

                sprintf(Text, "%d DF/S", pPerf->GetDroppedFieldsLastSecond());
                StatusBar_ShowText(STATUS_FPS, Text);
            }
            break;
        //-------------------------------
		case TIMER_TOOLBAR:
            if (IsToolBarVisible() && (Providers_GetCurrentSource() != NULL) && Providers_GetCurrentSource()->HasMediaControl())
            {
				EventCollector->RaiseEvent(NULL, EVENT_DURATION, -1, ((CDSSourceBase*)Providers_GetCurrentSource())->GetDuration());
				EventCollector->RaiseEvent(NULL, EVENT_CURRENT_POSITION, -1, ((CDSSourceBase*)Providers_GetCurrentSource())->GetCurrentPos());
			}
            break;
        //-------------------------------
        case TIMER_KEYNUMBER:
            KillTimer(hWnd, TIMER_KEYNUMBER);
    		i = atoi(ChannelString);
            // if only zero's are entered video input is switched.
            if(i == 0)
            {
                if(strcmp(ChannelString, "0") == 0)
                {
                    SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_INPUT1, 0);
                }
                else if(strcmp(ChannelString, "00") == 0)
                {
                    SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_INPUT2, 0);
                }
                else if(strcmp(ChannelString, "000") == 0)
                {
                    SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_INPUT3, 0);
                }
                else if(strcmp(ChannelString, "0000") == 0)
                {
                    SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_INPUT4, 0);
                }
                else if(strcmp(ChannelString, "00000") == 0)
                {
                    SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_INPUT5, 0);
                }
                else if(strcmp(ChannelString, "000000") == 0)
                {
                    SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_INPUT6, 0);
                }
                else if(strcmp(ChannelString, "0000000") == 0)
                {
                    SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_INPUT7, 0);
                }
                else if(strcmp(ChannelString, "00000000") == 0)
                {
                    SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_INPUT8, 0);
                }
                else if(strcmp(ChannelString, "000000000") == 0)
                {
                    SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_INPUT9, 0);
                }
            }
            ChannelString[0] = '\0';
            break;
        //-------------------------------
        case TIMER_AUTOSAVE:
            // JA 21/12/00 Added KillTimer so that settings are not
            // written repeatedly
            KillTimer(hWnd, TIMER_AUTOSAVE);
            WriteSettingsToIni(TRUE);
            break;
        //-------------------------------
        case OSD_TIMER_ID:
            ProcessOSDMessage(hWnd, message, wParam, lParam);
            break;
        //-------------------------------
        case OSD_TIMER_REFRESH_ID:
            ProcessOSDMessage(hWnd, message, wParam, lParam);
            break;
        //-------------------------------
        case TIMER_BOUNCE:
        case TIMER_ORBIT:
            // MRS 2-20-01 - Resetup the display for bounce and orbiting
            WorkoutOverlaySize(FALSE); // Takes care of everything...
            break;
        //-------------------------------
        case TIMER_HIDECURSOR:
            if (Cursor_IsOurs() != FALSE)
            {
                KillTimer(hWnd, TIMER_HIDECURSOR);
				if (ToolbarControl != NULL) 
				{
					POINT Point;		
					GetCursorPos(&Point);
					
					if (ToolbarControl->PtInToolbar(Point))
					{
						Cursor_SetVisibility(TRUE);
						break;
					}
				}				
                Cursor_SetVisibility(FALSE);
            }
            break;
        //-------------------------------
        case TIMER_VTFLASHER:
            ProcessVTMessage(hWnd, message, wParam, lParam);
            break;
        //---------------------------------
        case TIMER_VTINPUT:
            ProcessVTMessage(hWnd, message, wParam, lParam);
            break;
        //---------------------------------
        case TIMER_FINDPULL:
            {
                KillTimer(hWnd, TIMER_FINDPULL);
                Setting_SetValue(OutThreads_GetSetting(AUTODETECT), FALSE);
                ShowText(hWnd, GetDeinterlaceModeName());
            }
            break;
        //---------------------------------
        case TIMER_SLEEPMODE:
            {
                KillTimer(hWnd, TIMER_SLEEPMODE);
                switch( SMState.State )
                {
                case SM_WaitMode:
                    // End of main timing
                    ShowWindow(hWnd, SW_HIDE);
                    PostMessage(hWnd, WM_DESTROY, wParam, lParam);
                    break;
                case SM_ChangeMode:
                    // Update & Restart main timing
                    SMState.State = SM_UpdateMode;
                    UpdateSleepMode(&SMState, Text);
                    break;
                default:
                    ; //NEVER_GET_HERE;
                }
            }
            break;
        //---------------------------------
        case TIMER_TAKESTILL:
            RequestStill(1);
            break;
        //---------------------------------
        default:
            Provider_HandleTimerMessages(LOWORD(wParam));
            break;
        }
        pPerf->Resume();
        return 0;
        break;
    
    // support for mouse wheel
    // the WM_MOUSEWHEEL message is not defined but this is it's Value
    case WM_MOUSEWHEEL:
        // if shift or right mouse button down change volume
        if ((wParam & MK_SHIFT) != 0 || bIsRightButtonDown)
        {
            // crack the mouse wheel delta
            // +ve is forward (away from user)
            // -ve is backward (towards user)
            if((short)HIWORD(wParam) > 0)
            {
                PostMessage(hWnd, WM_COMMAND, IDM_VOLUMEPLUS, 0);
            }
            else
            {
                PostMessage(hWnd, WM_COMMAND, IDM_VOLUMEMINUS, 0);
            }
            // make sure the context menu is not shown when the right mouse button is released
            if(bIsRightButtonDown)
            {
                bIgnoreNextRightButtonUpMsg = TRUE;
            }
        }
        // if ctrl key down change playlist file
        else if ((wParam & MK_CONTROL) != 0)
        {
            // crack the mouse wheel delta
            // +ve is forward (away from user)
            // -ve is backward (towards user)
            if((short)HIWORD(wParam) > 0)
            {
                PostMessage(hWnd, WM_COMMAND, IDM_PLAYLIST_PREVIOUS, 0);
            }
            else
            {
                PostMessage(hWnd, WM_COMMAND, IDM_PLAYLIST_NEXT, 0);
            }
            // make sure the context menu is not shown when the right mouse button is released
            if(bIsRightButtonDown)
            {
                bIgnoreNextRightButtonUpMsg = TRUE;
            }
        }
        // else change the channel
        else if ((wParam & MK_CONTROL) == 0)
        {
            // crack the mouse wheel delta
            // +ve is forward (away from user)
            // -ve is backward (towards user)
            if((short)HIWORD(wParam) > 0)
            {
                PostMessage(hWnd, WM_COMMAND, IDM_CHANNELPLUS, 0);
            }
            else
            {
                PostMessage(hWnd, WM_COMMAND, IDM_CHANNELMINUS, 0);
            }
        }
        return 0;
        break;

    case WM_SYSCOMMAND:
        switch (wParam & 0xFFF0)
        {
        case SC_SCREENSAVE:
        case SC_MONITORPOWER:
            return FALSE;
            break;
        }
        break;

    case WM_SIZE:        
        return OnSize(hWnd, wParam, lParam);
        break;

    case WM_MOVE:
        StatusBar_Adjust(hWnd);
        if (bDoResize == TRUE && !IsIconic(hWnd) && !IsZoomed(hWnd))
        {
            WorkoutOverlaySize(FALSE);
        }
        return 0;
        break;

    case WM_CHAR:
        if (ProcessVTMessage(hWnd, message, wParam, lParam))
        {
            return 0;
        }
        else
        {
            return OnChar(hWnd, message, wParam, lParam);
        }
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT sPaint;
            RECT Rect;

            BeginPaint(hWnd, &sPaint);

            GetDisplayAreaRect(hWnd, &Rect);

            HDC hDC = OffscreenHDC.BeginPaint(sPaint.hdc, &Rect);

            if (VT_GetState() != VT_OFF)
            {
                RECT VTDrawRect;
                GetDestRect(&VTDrawRect);

                PaintColorkey(hWnd, TRUE, hDC, &sPaint.rcPaint, TRUE);

                // Paint the Teletext stuff
                VT_Redraw(hDC, &VTDrawRect);
            }
            else
            {
                PaintColorkey(hWnd, TRUE, hDC, &sPaint.rcPaint);
            }

            // Paint the OSD stuff
            OSD_Redraw(hDC, &Rect);

            OffscreenHDC.EndPaint();

            if (!bIsFullScreen && (WindowBorder!=NULL) && WindowBorder->Visible())
            {
                WindowBorder->Paint(hWnd, sPaint.hdc, &sPaint.rcPaint);
            }

            EndPaint(hWnd, &sPaint);
        }
        return 0;
        break;

    case UWM_VIDEOTEXT:
        ProcessVTMessage(hWnd, message, wParam, lParam);
        return FALSE;
        break;

    case UWM_OSD:
        ProcessOSDMessage(hWnd, message, wParam, lParam);
        return FALSE;
        break;

    case UWM_INPUTSIZE_CHANGE:
        //the input source has changed its size, update overlay.
        WorkoutOverlaySize(FALSE);
        return FALSE;
        break;
    
    case UWM_SQUAREPIXELS_CHECK:
        if (Providers_GetCurrentSource())
        {
            if (UpdateSquarePixelsMode(Providers_GetCurrentSource()->HasSquarePixels()))
            {
                WorkoutOverlaySize(TRUE);
            }
        }
        return FALSE;
        break;

    case UWM_DEINTERLACE_SETSTATUS:
        if(wParam!=NULL)
        {
            StatusBar_ShowText(STATUS_MODE,(LPCSTR)wParam);
            free((void*)wParam);
        }
        return 0;
        break;

    case UWM_EVENTADDEDTOQUEUE:
        if (EventCollector != NULL)
        {
            EventCollector->ProcessEvents();
        }
        return FALSE;
        break;

    case UWM_SWITCH_WINDOW:
        if (bMinimized == FALSE)
        {
            ShowWindow(hWnd, SW_MINIMIZE);
        }
        else
        {
            ShowWindow(hWnd, SW_SHOW);
            ShowWindow(hWnd, SW_RESTORE);
            SetForegroundWindow(hWnd);
        }
        return FALSE;
        break;

    //TJ 010506 make sure we dont erase the background
    //if we do, it will cause flickering when resizing the window
    //in future we migth need to adjust this to only erase parts not covered by overlay
    case WM_ERASEBKGND:
        return TRUE;
    
    case WM_QUERYENDSESSION:
        return TRUE;
        break;

    case WM_ENDSESSION:
    case WM_DESTROY:
        //Reset screensaver-mode if necessary
        SetScreensaverMode(false);

        MainWndOnDestroy();
        PostQuitMessage(0);
        return 0;
        break;

    default:
        {
            LONG RetVal = Settings_HandleSettingMsgs(hWnd, message, wParam, lParam, &bDone);
            if(!bDone)
            {
                RetVal = Deinterlace_HandleSettingsMsg(hWnd, message, wParam, lParam, &bDone);
            }
            if(!bDone)
            {
                RetVal = Filter_HandleSettingsMsg(hWnd, message, wParam, lParam, &bDone);
            }
            if(!bDone)
            {
                if(Providers_GetCurrentSource() != NULL)
                {
                    RetVal = Providers_GetCurrentSource()->HandleSettingsMessage(hWnd, message, wParam, lParam, &bDone);
                }
            }

            if(bDone)
            {
                // Updates the menu checkbox settings
                SetMenuAnalog();

                if(bUseAutoSave)
                {
                    // Set the configuration file autosave timer.
                    // We use an autosave timer so that when the user has finished
                    // making adjustments and at least a small delay has occured,
                    // that the DTV.INI file is properly up to date, even if 
                    // the system crashes or system is turned off abruptly.
                    KillTimer(hWnd, TIMER_AUTOSAVE);
                    SetTimer(hWnd, TIMER_AUTOSAVE, TIMER_AUTOSAVE_MS, NULL);
                }
                return RetVal;
            }
            else
            {
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        return 0;
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

//---------------------------------------------------------------------------
void SaveWindowPos(HWND hWnd)
{
    WINDOWPLACEMENT WndPlace;
    if(hWnd != NULL)
    {
        // MRS 2-20-01 - length must be set in WindowPlacement structure
        memset(&WndPlace,0,sizeof(WndPlace));
        WndPlace.length = sizeof(WndPlace);
        // End 2-20-01
        GetWindowPlacement(hWnd, &WndPlace);

        MainWndTop = WndPlace.rcNormalPosition.top;
        MainWndHeight = WndPlace.rcNormalPosition.bottom - WndPlace.rcNormalPosition.top;
        MainWndLeft = WndPlace.rcNormalPosition.left;
        MainWndWidth = WndPlace.rcNormalPosition.right - WndPlace.rcNormalPosition.left;

        // We need to adust these numbers by the workspace
        // offset so that we can later use them to set the
        // windows position
        RECT Workspace = {0,0,0,0};
        SystemParametersInfo(SPI_GETWORKAREA, 0, &Workspace, 0);
        MainWndTop += Workspace.top;
        MainWndLeft += Workspace.left;
    }
}


//---------------------------------------------------------------------------
void SaveActualPStripTiming(HWND hPSWnd)
{
	ATOM pStripTimingAtom = SendMessage(hPSWnd, UM_GETPSTRIPTIMING, 0, 0);
	if(lPStripTimingString == NULL)
	{
		lPStripTimingString = new char[PSTRIP_TIMING_STRING_SIZE];	
	}
	GlobalGetAtomName(pStripTimingAtom, lPStripTimingString, PSTRIP_TIMING_STRING_SIZE);
	GlobalDeleteAtom(pStripTimingAtom);
}

//---------------------------------------------------------------------------
void MainWndOnInitBT(HWND hWnd)
{
    int i;
    BOOL bInitOK = FALSE;

	// Initialise the PowerStrip window handler
	hPSWnd = FindWindow("TPShidden", NULL);

    AddSplashTextLine("Hardware Init");

    if (ShowHWSetupBox)
    {
        DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_HWSETUP), hWnd, (DLGPROC) HardwareSettingProc, (LPARAM)0);
    }

    if (Providers_Load(hMenu) > 0)
    {
        if (ShowHWSetupBox)
        {
            Providers_ChangeSettingsBasedOnHW(Setting_GetValue(DScaler_GetSetting(PROCESSORSPEED)), Setting_GetValue(DScaler_GetSetting(TRADEOFF)));
        }
        if(InitDD(hWnd) == TRUE)
        {
            if(Overlay_Create() == TRUE)
            {
                bInitOK = TRUE;
            }
        }
    }
    else
    {
        AddSplashTextLine("");
        AddSplashTextLine("No");
        AddSplashTextLine("Suitable");
        AddSplashTextLine("Hardware");
    }

    if (bInitOK)
    {
        AddSplashTextLine("Load Plugins");
        if(!LoadDeinterlacePlugins())
        {
            AddSplashTextLine("");
            AddSplashTextLine("No");
            AddSplashTextLine("Plug-ins");
            AddSplashTextLine("Found");
            bInitOK = FALSE;
        }
        else
        {
            LoadFilterPlugins();
        }
    }
    
    if (bInitOK)
    {
        AddSplashTextLine("Position Window");
        WStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
        if (bAlwaysOnTop == FALSE)
        {
            WStyle = WStyle ^ 8;
            i = SetWindowLong(hWnd, GWL_EXSTYLE, WStyle);
            SetWindowPos(hWnd, HWND_NOTOPMOST, 10, 10, 20, 20, SWP_NOMOVE | SWP_NOCOPYBITS | SWP_NOSIZE | SWP_SHOWWINDOW);
        }
        else
        {
            WStyle = WStyle | 8;
            i = SetWindowLong(hWnd, GWL_EXSTYLE, WStyle);
            SetWindowPos(hWnd, HWND_TOPMOST, 10, 10, 20, 20, SWP_NOMOVE | SWP_NOCOPYBITS | SWP_NOSIZE | SWP_SHOWWINDOW);
        }

        if (bShowMenu == FALSE)
        {
            bShowMenu = TRUE;
            SendMessage(hWnd, WM_COMMAND, IDM_TOGGLE_MENU, 0);
        }

		AddSplashTextLine("Load Toolbars");
        if (ToolbarControl == NULL)
        {
            ToolbarControl = new CToolbarControl(WM_TOOLBARS_GETVALUE);
			ToolbarControl->Set(hWnd, NULL);
        }

        if (szSkinName[0] != 0)
        {
            AddSplashTextLine("Load Skin");
            
            SetWindowBorder(hWnd, szSkinName, (szSkinName[0]!=0));  
            if (ToolbarControl!=NULL)
            {
                ToolbarControl->Set(hWnd, szSkinName);  
            }
        }

		// We must do two calls, the first one displaying the toolbar
		// in order to have the correct toolbar rectangle initialized,
		// and the second to hide the toolbar if in full scrren mode
        if (ToolbarControl == NULL)
        {
			ToolbarControl->Set(hWnd, NULL, bIsFullScreen?1:0);
        }

        AddSplashTextLine("Setup Mixer");
        Mixer_Init();

        AddSplashTextLine("Start Timers");
        if(bIsFullScreen == FALSE && bDisplayStatusBar == TRUE)
        {
            SetTimer(hWnd, TIMER_STATUS, TIMER_STATUS_MS, NULL);
        }

		if (IsToolBarVisible())
		{
	        SetTimer(hWnd, TIMER_TOOLBAR, TIMER_TOOLBAR_MS, NULL);
		}

        // do final setup routines for any files
        // basically where we need the hWnd to be set
        AddSplashTextLine("Setup Aspect Ratio");
        Aspect_FinalSetup();

        // OK we're ready to go
        WorkoutOverlaySize(FALSE);
        
        AddSplashTextLine("Update Menu");
        OSD_UpdateMenu(hMenu);
        pCalibration->UpdateMenu(hMenu);
        Channels_UpdateMenu(hMenu);
        VT_UpdateMenu(hMenu);
        OutReso_UpdateMenu(hMenu);
        SetMenuAnalog();
        if (ToolbarControl!=NULL)
        {
            ToolbarControl->UpdateMenu(hMenu);
        }
        Skin_SetMenu(hMenu, FALSE);

		if (bIsFullScreen)
		{
			if(hPSWnd)
			{
				// Save the actual PowerStrip timing string in lPStripTimingString
				SaveActualPStripTiming(hPSWnd);
			}
			OutReso_Change(hWnd, hPSWnd, FALSE, FALSE, NULL, FALSE);
			BypassChgResoInRestore = TRUE;
		}

        bDoResize = TRUE;

        if (Providers_GetCurrentSource())
        {
            // if we are in tuner mode
            // either set channel up as requested on the command line
            // or reset it to what is was last time
            if (Providers_GetCurrentSource()->IsInTunerMode())
            {
                if(InitialChannel >= 0)
                {
                    Channel_Change(InitialChannel);
                }
                else
                {
                    Channel_Reset();
                }
            }
        }
        
        if (InitialTextPage >= 0x100)
        {
            Setting_SetValue(VBI_GetSetting(CAPTURE_VBI), TRUE);
            Setting_SetValue(VBI_GetSetting(DOTELETEXT), TRUE);

            VT_SetState(NULL, NULL, VT_BLACK);
            VT_SetPage(NULL, NULL, InitialTextPage);
        }

        AddSplashTextLine("Start Video");
        Start_Capture();
		SetCurrentMonitor(hWnd);
        
    }
    else
    {
        Sleep(2000);
        PostQuitMessage(0);
    }
}

//---------------------------------------------------------------------------
void MainWndOnCreate(HWND hWnd)
{
    char Text[128];
    int i;
    int ProcessorMask;
    SYSTEM_INFO SysInfo;

    pCalibration = new CCalibration();

    pPerf = new CPerf();

    GetSystemInfo(&SysInfo);
    AddSplashTextLine("Table Build");
    AddSplashTextLine("VideoText");

    VBI_Init(); 
    OSD_Init();
    
    Load_Program_List_ASCII();

    AddSplashTextLine("System Analysis");

    sprintf(Text, "Processor %d ", SysInfo.dwProcessorType);
    AddSplashTextLine(Text);
    sprintf(Text, "Number %d ", SysInfo.dwNumberOfProcessors);
    AddSplashTextLine(Text);

    if (SysInfo.dwNumberOfProcessors > 1)
    {
        if (DecodeProcessor == 0)
        {
            if (SysInfo.dwNumberOfProcessors == 2)
            {
                MainProcessor = 0;
                DecodeProcessor = 1;
            }
            if (SysInfo.dwNumberOfProcessors == 3)
            {
                MainProcessor = 0;
                DecodeProcessor = 2;
            }
            if (SysInfo.dwNumberOfProcessors > 3)
            {
                DecodeProcessor = 3;
            }

        }

        AddSplashTextLine("Multi-Processor");
        sprintf(Text, "Main-CPU %d ", MainProcessor);
        AddSplashTextLine(Text);
        sprintf(Text, "DECODE-CPU %d ", DecodeProcessor);
        AddSplashTextLine(Text);
    }

    ProcessorMask = 1 << (MainProcessor);
    i = SetThreadAffinityMask(GetCurrentThread(), ProcessorMask);

    Cursor_UpdateVisibility();

    PostMessage(hWnd, INIT_BT, 0, 0);
}

void KillTimers()
{
    KillTimer(hWnd, TIMER_BOUNCE);
    KillTimer(hWnd, TIMER_ORBIT);
    KillTimer(hWnd, TIMER_AUTOSAVE);
    KillTimer(hWnd, TIMER_KEYNUMBER);
    KillTimer(hWnd, TIMER_STATUS);
    KillTimer(hWnd, OSD_TIMER_ID);
    KillTimer(hWnd, OSD_TIMER_REFRESH_ID);
    KillTimer(hWnd, TIMER_HIDECURSOR);
    KillTimer(hWnd, TIMER_VTFLASHER);
    KillTimer(hWnd, TIMER_VTINPUT);
    KillTimer(hWnd, TIMER_FINDPULL);
    KillTimer(hWnd, TIMER_SLEEPMODE);
    KillTimer(hWnd, TIMER_TAKESTILL);
    KillTimer(hWnd, TIMER_TOOLBAR);
}


// basically we want do make sure everything that needs to be done on exit gets 
// done even if one of the functions crashes we should just carry on with the rest
// of the functions
void MainWndOnDestroy()
{
    __try
    {
        KillTimers();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Kill Timers");}

    // stop capture before stopping timneshift to avoid crash
    __try
    {
        LOG(1, "Try Stop_Capture");
        Stop_Capture();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error Stop_Capture");}
      
    __try
    {
        LOG(1, "Try CTimeShift::OnStop");

        CTimeShift::OnStop();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error CTimeShift::OnStop");}

    
    // Kill timeshift before muting since it always exits unmuted on cleanup.
    __try
    {
        LOG(1, "Try CTimeShift::OnDestroy");

        CTimeShift::OnDestroy();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error CTimeShift::OnDestroy");}

    __try
    {
        LOG(1, "Try CleanUpMemory");
        CleanUpMemory();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error CleanUpMemory");}

    __try
    {
        if(bIsFullScreen == FALSE)
        {
            LOG(1, "Try SaveWindowPos");
			if(hPSWnd)
			{			
				// Save the actual PowerStrip timing string in lPStripTimingString
				SaveActualPStripTiming(hPSWnd);
			}
			SaveWindowPos(hWnd);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error SaveWindowPos");}
    
     __try
    {
        LOG(1, "Try free skinned border");
        if (WindowBorder != NULL)
        {
            delete WindowBorder;
            WindowBorder = NULL;
        }            
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error free skinned border");}

    __try
    {
        LOG(1, "Try free toolbars");
        if (ToolbarControl!=NULL)
        {
            delete ToolbarControl;
            ToolbarControl = NULL;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error free toolbars");}    

    __try
    {
        // save settings per cahnnel/input ets
        // must be done before providers are unloaded
        LOG(1, "SettingsMaster->SaveSettings");
        SettingsMaster->SaveSettings();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error SettingsMaster->SaveSettings");}
    
    __try
    {
        // write out setting with optimize on 
        // to avoid delay on flushing file
        // all the setting should be filled out anyway
        LOG(1, "WriteSettingsToIni");
        WriteSettingsToIni(TRUE);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error WriteSettingsToIni");}

    __try
    {
        LOG(1, "Try Providers_Unload");
        Providers_Unload();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error Providers_Unload");}

    
    __try
    {
        LOG(1, "Try free settings");
        if (SettingsMaster != NULL)
        {
            delete SettingsMaster;
            SettingsMaster = NULL;
        }                
		FreeSettings();
		// Free of filters settings is done later when calling UnloadFilterPlugins
		// Free of sources dependent settings is already done when calling Providers_Unload
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error free settings");}
    

    __try
    {
        LOG(1, "Try StatusBar_Destroy");
        StatusBar_Destroy();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error StatusBar_Destroy");}

    __try
    {
        LOG(1, "Try SetTray(FALSE)");
        if (bIconOn)
            SetTray(FALSE);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error SetTray(FALSE)");}

    __try
    {
        LOG(1, "Try free EventCollector");
        if (EventCollector != NULL)
        {
            delete EventCollector;
            EventCollector = NULL;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error free EventCollector");}
    
    __try
    {
        LOG(1, "Try ExitDD");
        ExitDD();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error ExitDD");}

    __try
    {
        if(bIsFullScreen == TRUE)
        {
			// Do this here after the ExitDD to be sure that the overlay is destroyed
            LOG(1, "Try restore display resolution");
			BypassChgResoInRestore = TRUE;
			OutReso_Change(hWnd, hPSWnd, TRUE, FALSE, lPStripTimingString, TRUE);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error restore display resolution");}
 
    __try
    {
        // unload plug-ins
        UnloadDeinterlacePlugins();
        UnloadFilterPlugins();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error Unload plug-ins");}

    __try
    {
        SetKeyboardLock(FALSE);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error SetKeyboardLock(FALSE)");}

    __try
    {
        Timing_CleanUp();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error Timing_CleanUp()");}

}

LONG OnChar(HWND hWnd, UINT message, UINT wParam, LONG lParam)
{
    char Text[128];
    int i;

    if (((char) wParam >= '0') && ((char) wParam <= '9'))
    {
        sprintf(Text, "%c", (char)wParam);
        // if something gets broken in the future
        if(strlen(ChannelString) >= sizeof(ChannelString)/sizeof(char) - 1)
        {
#ifdef _DEBUG
            // if this is a debug build show an error message
            MessageBox(hWnd, "dscaler.cpp: ChannelString out of bounds.", "Error", MB_ICONERROR);
#endif
            ChannelString[0] = '\0';
            return 0;
        }
        strcat(ChannelString, Text);

        // if the user is only typing zero's we are going to switch inputs
        if(atoi(ChannelString) == 0 && (char) wParam == '0')
        {
            int VideoInput = strlen(ChannelString) -1;

            if(Providers_GetCurrentSource() != NULL)
            {
                if(VideoInput < Providers_GetCurrentSource()->NumInputs(VIDEOINPUT))
                {
                    OSD_ShowText(Providers_GetCurrentSource()->GetInputName(VIDEOINPUT, VideoInput), 0);
                }
                else
                {
                    OSD_ShowText(ChannelString, 0);
                }

                if (strlen(ChannelString) >= 7)
                {
                    SetTimer(hWnd, TIMER_KEYNUMBER, 1, NULL);
                }
                else
                {
                    SetTimer(hWnd, TIMER_KEYNUMBER, ChannelEnterTime, NULL);
                }
            }
            else
            {
                OSD_ShowText("No Source", 0);
            }
        }
        // if in tuner mode or videotext mode
        else if (Providers_GetCurrentSource()->IsInTunerMode())
        {
            OSD_ShowText(ChannelString, 0);

            if(strlen(ChannelString) >= 3)
            {
                SetTimer(hWnd, TIMER_KEYNUMBER, 1, NULL);
            }
            else
            {
                SetTimer(hWnd, TIMER_KEYNUMBER, ChannelEnterTime, NULL);
            }

            i = atoi(ChannelString);
            if(i != 0)
            {
                Channel_ChangeToNumber(i,(strlen(ChannelString)>1)?1:0);
            }
        }
        // we don't know what to do with this keypress so we reset ChannelString
        else
        {
            ChannelString[0] = '\0';
        }
    }
    return 0;
}

LONG OnSize(HWND hWnd, UINT wParam, LONG lParam)
{
    StatusBar_Adjust(hWnd);
    if (ToolbarControl!=NULL)
    {
        ToolbarControl->Adjust(hWnd, FALSE);
    }
    UpdateWindowRegion(hWnd, FALSE);
    if (bDoResize == TRUE)
    {
        switch(wParam)
        {
        case SIZE_MAXIMIZED:
            if(bIsFullScreen == FALSE || bMinimized == TRUE)
            {
				bCheckSignalPresent = FALSE;
				bCheckSignalMissing = (MinimizeHandling == 2);
                IsFullScreen_OnChange(TRUE);
				if ((MinimizeHandling == 1) && bMinimized && !OverlayActive())
				{
					Overlay_Start(hWnd);
				}
				bMinimized = FALSE;
            }
            break;
        case SIZE_MINIMIZED:
			bMinimized = TRUE;
			if (OverlayActive() && (MinimizeHandling == 1))
			{
	            Overlay_Stop(hWnd);
			}
            if(bIsFullScreen)
			{
				OutReso_Change(hWnd, hPSWnd, TRUE, TRUE, lPStripTimingString, TRUE);
			}
			if (OverlayActive())
			{
		        Overlay_Update(NULL, NULL, DDOVER_HIDE);
			}
            if (bMinToTray)
			{
                ShowWindow(hWnd, SW_HIDE);
			}
			bCheckSignalPresent = (MinimizeHandling == 2);
			bCheckSignalMissing = FALSE;
            break;
        case SIZE_RESTORED:
            bMinimized = FALSE;
			bCheckSignalPresent = FALSE;
			bCheckSignalMissing = (MinimizeHandling == 2);
            InvalidateRect(hWnd, NULL, FALSE);
			if(bIsFullScreen)
			{
				if (BypassChgResoInRestore)
				{
					BypassChgResoInRestore = FALSE;
				}
				else
				{
					OutReso_Change(hWnd, hPSWnd, FALSE, TRUE, NULL, FALSE);
				}
			}
            if ((MinimizeHandling == 1) && !OverlayActive())
			{
                Overlay_Start(hWnd);
			}
			if (OverlayActive())
			{
	            WorkoutOverlaySize(FALSE);
			}
            SetMenuAnalog();
            break;
        default:
            break;
        }
    }
    return 0;
}



//---------------------------------------------------------------------------
void SetMenuAnalog()
{
    CheckMenuItem(hMenu, ThreadClassId + 1150, MF_CHECKED);
    CheckMenuItem(hMenu, PriorClassId + 1160, MF_CHECKED);

    CheckMenuItemBool(hMenu, IDM_TOGGLECURSOR, bShowCursor);
    EnableMenuItem(hMenu,IDM_TOGGLECURSOR, bAutoHideCursor?MF_GRAYED:MF_ENABLED);
    CheckMenuItemBool(hMenu, IDM_STATUSBAR, bDisplayStatusBar);
    CheckMenuItemBool(hMenu, IDM_TOGGLE_MENU, bShowMenu);
    CheckMenuItemBool(hMenu, IDM_ON_TOP, bAlwaysOnTop);
    CheckMenuItemBool(hMenu, IDM_ALWAYONTOPFULLSCREEN, bAlwaysOnTopFull);
    CheckMenuItemBool(hMenu, IDM_FULL_SCREEN, bIsFullScreen);
    CheckMenuItemBool(hMenu, IDM_MINTOTRAY, bMinToTray);
    CheckMenuItemBool(hMenu, IDM_VT_AUTOCODEPAGE, bVTAutoCodePage);
    CheckMenuItemBool(hMenu, IDM_VT_ANTIALIAS, bVTAntiAlias);

    CheckMenuItemBool(hMenu, IDM_USE_DSCALER_OVERLAY, Setting_GetValue(Other_GetSetting(USEOVERLAYCONTROLS)));
    //EnableMenuItem(hMenu,IDM_OVERLAYSETTINGS, Setting_GetValue(Other_GetSetting(USEOVERLAYCONTROLS))?MF_ENABLED:MF_GRAYED);

    EnableMenuItem(hMenu, IDM_OVERLAY_START, bMinimized ? MF_GRAYED : MF_ENABLED);
    EnableMenuItem(hMenu, IDM_OVERLAY_STOP, bMinimized ? MF_GRAYED : MF_ENABLED);
    CheckMenuItemBool(hMenu, IDM_TAKECYCLICSTILL, bTakingCyclicStills);

    SetMixedModeMenu(hMenu, !bVTSingleKeyToggle);

    AspectRatio_SetMenu(hMenu);
    FD60_SetMenu(hMenu);
    OutThreads_SetMenu(hMenu);
    Deinterlace_SetMenu(hMenu);
    Filter_SetMenu(hMenu);
    VBI_SetMenu(hMenu);
    Channels_SetMenu(hMenu);
    OSD_SetMenu(hMenu);
    FD_Common_SetMenu(hMenu);
    Timing_SetMenu(hMenu);
    MixerDev_SetMenu(hMenu);
    Audio_SetMenu(hMenu);
    VT_SetMenu(hMenu);
    OutReso_SetMenu(hMenu);
    Providers_SetMenu(hMenu);

    CTimeShift::OnSetMenu(hMenu);
    if(pCalibration)
    {
        pCalibration->SetMenu(hMenu);
    }

    CheckMenuItemBool(hMenu, ID_SETTINGS_SAVESETTINGSPERCHANNEL, SettingsPerChannel_IsPerChannel());
    CheckMenuItemBool(hMenu, ID_SETTINGS_SAVESETTINGSPERINPUT, SettingsPerChannel_IsPerInput());
    CheckMenuItemBool(hMenu, ID_SETTINGS_SAVESETTINGSPERFORMAT, SettingsPerChannel_IsPerFormat());
}

// This function checks the name of the menu item. A message box is shown with a debug build
// if the name is not correct.
HMENU GetSubMenuWithName(HMENU hMenu, int nPos, LPCSTR szMenuText)
{
#ifdef _DEBUG
    char name[128] = "\0";
    char msg[128] = "\0";

    GetMenuString(hMenu, nPos, name, sizeof(name), MF_BYPOSITION);

    if(strcmp(name, szMenuText) != 0)
    {
        sprintf(msg, "GetSubMenuWithName() error: \'%s\' != \'%s\'", name, szMenuText);
        MessageBox(hWnd, msg, "Error", MB_ICONERROR);
    }
#endif
    return GetSubMenu(hMenu, nPos);
}

HMENU GetOrCreateSubSubMenu(int SubId, int SubSubId, LPCSTR szMenuText)
{
    if(hMenu != NULL)
    {
        HMENU hSubMenu = GetSubMenu(hMenu, SubId);
        if(hSubMenu != NULL)
        {
            HMENU hSubSubMenu = GetSubMenu(hSubMenu, SubSubId);
            if(hSubSubMenu != NULL)
            {
                return hSubSubMenu;
            }
            else
            {
                if(ModifyMenu(hSubMenu, SubSubId, MF_STRING | MF_BYPOSITION | MF_POPUP, (UINT)CreatePopupMenu(), szMenuText))
                {
                    return GetSubMenu(hSubMenu, SubSubId);
                }
                else
                {
                    return NULL;
                }
            }
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
}

HMENU GetOrCreateSubSubSubMenu(int SubId, int SubSubId, int SubSubSubId, LPCSTR szMenuText)
{
    if(hMenu != NULL)
    {
        HMENU hSubMenu = GetSubMenu(hMenu, SubId);
        if(hSubMenu != NULL)
        {
            HMENU hSubSubMenu = GetSubMenu(hSubMenu, SubSubId);
            if(hSubSubMenu != NULL)
            {
                HMENU hSubSubSubMenu = GetSubMenu(hSubSubMenu, SubSubSubId);
                if(hSubSubSubMenu != NULL)
                {
                    return hSubSubSubMenu;
                }
                else
                {
                    if(ModifyMenu(hSubSubMenu, SubSubSubId, MF_STRING | MF_BYPOSITION | MF_POPUP, (UINT)CreatePopupMenu(), szMenuText))
                    {
                        return GetSubMenu(hSubSubMenu, SubSubSubId);
                    }
                    else
                    {
                        return NULL;
                    }
                }
            }
            else
            {
                return NULL;
            }
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
}

HMENU GetFiltersSubmenu()
{
    char string[128] = "\0";
    int reduc;

    GetMenuString(hMenu, 2, string, sizeof(string), MF_BYPOSITION);
    reduc = !strcmp(string, "&Channels") ? 0 : 1;

    HMENU hmenu = GetSubMenuWithName(hMenu, 5-reduc, "&Filters");
    ASSERT(hmenu != NULL);

    return hmenu;
}


HMENU GetVideoDeinterlaceSubmenu()
{
    char string[128] = "\0";
    int reduc;

    GetMenuString(hMenu, 2, string, sizeof(string), MF_BYPOSITION);
    reduc = !strcmp(string, "&Channels") ? 0 : 1;

    HMENU hmenu = GetSubMenuWithName(hMenu, 4-reduc, "Deinter&lace");
    ASSERT(hmenu != NULL);

    return hmenu;
}

HMENU GetChannelsSubmenu()
{
    ASSERT(hSubMenuChannels != NULL);

    return hSubMenuChannels;
}

HMENU GetOSDSubmenu()
{
    char string[128] = "\0";
    int reduc;

    GetMenuString(hMenu, 2, string, sizeof(string), MF_BYPOSITION);
    reduc = !strcmp(string, "&Channels") ? 0 : 1;

    HMENU hmenu = GetSubMenuWithName(hMenu, 3-reduc, "&View");
    ASSERT(hmenu != NULL);

    return hmenu;
}

HMENU GetPatternsSubmenu()
{
    char string[128] = "\0";
    int reduc;

    GetMenuString(hMenu, 2, string, sizeof(string), MF_BYPOSITION);
    reduc = !strcmp(string, "&Channels") ? 0 : 1;

    HMENU hmenu = GetOrCreateSubSubSubMenu(7-reduc, 2, 0, "Test &Patterns");
    ASSERT(hmenu != NULL);

    return hmenu;
}

HMENU GetVTCodepageSubmenu()
{
    char string[128] = "\0";
    int reduc;

    GetMenuString(hMenu, 2, string, sizeof(string), MF_BYPOSITION);
    reduc = !strcmp(string, "&Channels") ? 0 : 1;

    HMENU hmenu = GetSubMenuWithName(hMenu, 9-reduc, "&Datacasting");
    ASSERT(hmenu != NULL);

    GetMenuString(hmenu, 8, string, sizeof(string), MF_BYPOSITION);
    reduc = !strcmp(string, "Toggle &Mixed Mode\tShift-T") ? 0 : 1;

    hmenu = GetSubMenuWithName(hmenu, 9-reduc, "Teletext Code Page");
    ASSERT(hmenu != NULL);

    return hmenu;
}

HMENU GetOutResoSubmenu()
{
    char string[128] = "\0";
    int reduc;

    GetMenuString(hMenu, 2, string, sizeof(string), MF_BYPOSITION);
    reduc = !strcmp(string, "&Channels") ? 0 : 1;

    HMENU hmenu = GetOrCreateSubSubMenu(3-reduc, 10, "Switch Resolution in F&ull Screen");
    ASSERT(hmenu != NULL);

    return hmenu;
}

void SetMixedModeMenu(HMENU hMenu, BOOL bShow)
{
    static BOOL     bShown = TRUE;
    static char     szMenuName[64] = "";
    MENUITEMINFO    MenuItem;
    char            Buffer[64];

    if (bShow == bShown)
    {
        return;
    }

    MenuItem.cbSize     = sizeof(MENUITEMINFO);
    MenuItem.fMask      = MIIM_TYPE;
    MenuItem.dwTypeData = Buffer;
    MenuItem.cch        = sizeof(Buffer);

    // Get the Mixed Mode menu item name
    if (GetMenuItemInfo(hMenu, IDM_VT_MIXEDMODE, FALSE, &MenuItem))
    {
        // The menu item exists, delete it if necessary
        if (!bShow)
        {
            if (MenuItem.fType == MFT_STRING)
            {
                // Save the name so we can be put back
                strcpy(szMenuName, Buffer);
            }

            DeleteMenu(hMenu, IDM_VT_MIXEDMODE, MF_BYCOMMAND);
            bShown = FALSE;
        }
    }
    else
    {
        // The menu does not exists, add it if necessary
        if (bShow)
        {
            // To insert after, I insert before and delete the
            // previous then insert the previous before again
            GetMenuItemInfo(hMenu, IDM_CALL_VIDEOTEXT, FALSE, &MenuItem);

            MenuItem.fMask      = MIIM_TYPE | MIIM_ID;
            MenuItem.fType      = MFT_STRING;
            MenuItem.wID        = IDM_VT_MIXEDMODE;
            MenuItem.dwTypeData = szMenuName;
            MenuItem.cch        = strlen(szMenuName);

            // Put the mixed mode menu item back
            InsertMenuItem(hMenu, IDM_CALL_VIDEOTEXT, FALSE, &MenuItem);

            DeleteMenu(hMenu, IDM_CALL_VIDEOTEXT, MF_BYCOMMAND);

            MenuItem.wID        = IDM_CALL_VIDEOTEXT;
            MenuItem.dwTypeData = Buffer;
            MenuItem.cch        = strlen(Buffer);

            InsertMenuItem(hMenu, IDM_VT_MIXEDMODE, FALSE, &MenuItem);
            bShown = TRUE;
        }
    }
}

void RedrawMenuBar(HMENU)
{
    // We could use the 1st arg to make
    // sure the changed menu is on our
    // window, but it's not necessary.
    DrawMenuBar(hWnd);
}

//---------------------------------------------------------------------------
void CleanUpMemory()
{
    Mixer_Exit();
    VBI_Exit();
    OSD_Exit();
    if ((hMenu != NULL) && (GetMenu(hWnd) == NULL))
    {
        DestroyMenu(hMenu);
    }
    Channels_Exit();
    delete pCalibration;
    pCalibration = NULL;
    delete pPerf;
    pPerf = NULL;
}

//---------------------------------------------------------------------------
// Stops video overlay - 2000-10-31 Added by Mark Rejhon
// This ends the video overlay from operating, so that end users can
// write scripts that sends this special message to safely stop the video
// before switching computer resolutions or timings.
// This is also called during a Suspend operation
void Overlay_Stop(HWND hWnd)
{
    RECT winRect;
    HDC hDC;
    GetClientRect(hWnd, &winRect);
    hDC = GetDC(hWnd);
    PaintColorkey(hWnd, FALSE, hDC, &winRect);
    ReleaseDC(hWnd,hDC);
    Stop_Capture();
    Overlay_Destroy();
    InvalidateRect(hWnd, NULL, FALSE);
}

//---------------------------------------------------------------------------
// Restarts video overlay - 2000-10-31 Added by Mark Rejhon
// This reinitializes the video overlay to continue operation,
// so that end users can write scripts that sends this special message
// to safely restart the video after a resolution or timings change.
// This is also called during a Resume operation
void Overlay_Start(HWND hWnd)
{
    InvalidateRect(hWnd, NULL, FALSE);
    Overlay_Create();
    Reset_Capture();
}

//---------------------------------------------------------------------------
// Show text on both OSD and statusbar
void ShowText(HWND hWnd, LPCTSTR szText)
{
    StatusBar_ShowText(STATUS_TEXT, szText);
    OSD_ShowText(szText, 0);
}

//----------------------------------------------------------------------------
// Updates the window position/window state and enable/disable titlebar 
// as necessary.  This function should be globally used for everytime 
// you want to update the window everytime you have enabled/disabled the 
// statusbar, menus, full screen state, etc.
//
// This allows for more cosmetic handling - including the ability to 
// startup directly to maximized without any intermediate cosmetic
// glitches during startup.
//
void UpdateWindowState()
{
    if(bIsFullScreen == TRUE)
    {
		RECT ScreenRect;
        UpdateWindowRegion(hWnd, FALSE);
		SetWindowLong(hWnd, GWL_STYLE, WS_VISIBLE | (IsWindowEnabled(hWnd) ? 0 : WS_DISABLED));
        SetMenu(hWnd, NULL);
        StatusBar_ShowWindow(FALSE);
		GetMonitorRect(hWnd, &ScreenRect);
        SetWindowPos(hWnd,
                    bAlwaysOnTopFull?HWND_TOPMOST:HWND_NOTOPMOST,
                    ScreenRect.left,
                    ScreenRect.top,
                    ScreenRect.right  - ScreenRect.left,
                    ScreenRect.bottom - ScreenRect.top,
                    SWP_SHOWWINDOW | SWP_NOACTIVATE);
    }
    else
    {
		if(bShowMenu == TRUE)
		{
			SetWindowLong(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE | (IsWindowEnabled(hWnd) ? 0 : WS_DISABLED));
			SetMenu(hWnd, hMenu);
		}
		else
		{
			if ((WindowBorder!=NULL) && WindowBorder->Visible())
            {
                SetWindowLong(hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE | (IsWindowEnabled(hWnd) ? 0 : WS_DISABLED));				
            }
            else
            {
                SetWindowLong(hWnd, GWL_STYLE, WS_THICKFRAME | WS_POPUP | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | (IsWindowEnabled(hWnd) ? 0 : WS_DISABLED));
            }
			SetMenu(hWnd, NULL);            
		}
        StatusBar_ShowWindow(bDisplayStatusBar);
        if (ToolbarControl!=NULL)
        {
            ToolbarControl->Adjust(hWnd, FALSE);
        }
        if (UpdateWindowRegion(hWnd, FALSE) == NULL)
        {                
           if (!bShowMenu)
           {
               SetWindowLong(hWnd, GWL_STYLE, WS_THICKFRAME | WS_POPUP | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | (IsWindowEnabled(hWnd) ? 0 : WS_DISABLED));
           }
        }
        SetWindowPos(hWnd,bAlwaysOnTop?HWND_TOPMOST:HWND_NOTOPMOST,
                    0,0,0,0,
                    SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW | SWP_NOACTIVATE);            
	}	
}

HRGN UpdateWindowRegion(HWND hWnd, BOOL bUpdateWindowState)
{
    if (!bIsFullScreen && (WindowBorder!=NULL) && WindowBorder->Visible() && !bShowMenu)
    {   
        RECT rcExtra;
        if (IsStatusBarVisible())
        {
            ::SetRect(&rcExtra,0,0,0, StatusBar_Height());
        }
        else
        {
            ::SetRect(&rcExtra,0,0,0,0);
        }   
        HRGN hRgn = WindowBorder->MakeRegion(&rcExtra);
        if (hRgn != NULL)
        {
            if (bUpdateWindowState)
            {
                SetWindowLong(hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE | (IsWindowEnabled(hWnd) ? 0 : WS_DISABLED));
            }
            LOG(2,"DScaler: Set window region (0x%08x)",hRgn);
			if (hRgn != DScalerWindowRgn)
			{
				SetWindowRgn(hWnd,hRgn,TRUE);            
			}
            DScalerWindowRgn = hRgn;
        }
    }
    else
    {
        if (DScalerWindowRgn != NULL)
        {
            LOG(2,"DScaler: Set window region (0x%08x)",NULL);
			if (DScalerWindowRgn != NULL)
			{
				SetWindowRgn(hWnd,NULL,TRUE);
			}
            DScalerWindowRgn = NULL;
        }
    }
    return DScalerWindowRgn;
}

BOOL IsStatusBarVisible()
{
    return (bDisplayStatusBar == TRUE && bIsFullScreen == FALSE);
}

BOOL IsToolBarVisible()
{
    return (ToolbarControl != NULL && ToolbarControl->Visible());
}

///////////////////////////////////////////////////////////////////////////////
void SetThreadProcessorAndPriority()
{
    DWORD rc;
    int ProcessorMask;

    ProcessorMask = 1 << (DecodeProcessor);
    rc = SetThreadAffinityMask(GetCurrentThread(), ProcessorMask);
    
    if (ThreadClassId == 0)
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
    else if (ThreadClassId == 1)
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
    else if (ThreadClassId == 2)
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
    else if (ThreadClassId == 3)
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    else if (ThreadClassId == 4)
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
}

void Cursor_SetVisibility(BOOL bVisible)
{
    static int nCursorIndex = 1;
    if(bVisible)
    {
        while(nCursorIndex < 1)
        {
            ShowCursor(TRUE);
            nCursorIndex++;
        }
    }
    else
    {
        while(nCursorIndex > 0)
        {
            ShowCursor(FALSE);
            nCursorIndex--;
        }
    }
}

void Cursor_UpdateVisibility()
{
    KillTimer(hWnd, TIMER_HIDECURSOR);	

    if (Cursor_IsOurs() == FALSE)
    {
        Cursor_SetVisibility(TRUE);
        return;
    }

    if (!bAutoHideCursor)
    {
        if (bIsFullScreen)
        {
            Cursor_SetVisibility(FALSE);
        }
        else
        {
            Cursor_SetVisibility(bShowCursor);
        }
    }
    else
    {
        Cursor_SetVisibility(TRUE);
        SetTimer(hWnd, TIMER_HIDECURSOR, TIMER_HIDECURSOR_MS, NULL);
    }
}

int Cursor_SetType(int type)
{
    HCURSOR hCur;
    switch (type)
    {
    case CURSOR_HAND:
        hCur = hCursorHand;
        break;
    default:
        hCur = hCursorDefault;
        break;
    }

    // SetCursor changes and redraws the cursor.
    SetCursor(hCur);

    // SetClassLong makes the change permanent.
    SetClassLong(hWnd, GCL_HCURSOR, (LONG)hCur);

    return true;
}

BOOL Cursor_IsOurs()
{
    /*
     *  The cursor is ours (ie. We can hide it, change it and
     *  stuff like that) if all of these points are true:
     *
     *  1. Our menubar is not active.
     *
     *  2. Our window is enabled. (eg. no modal dialogs open)
     *
     *  3. No other window in the same thread has captured the
     *     mouse.
     *
     *
     *  And if any one of these points are true:
     *
     *  4. Our window is the foreground window and the cursor
     *     is within the bounds of the our window (or if the
     *     mouse button is down --but we don't want this.)
     *
     *  5. Our window is not in the foreground but the cursor
     *     is in the visible portion of our window and the
     *     mouse button is not down.
     *
     *  6. Our window has SetCapture() and the cursor is within
     *     the bounds of any of the windows owned by the same
     *     thread.
     *
     *
     *  How to check:
     *
     *  1: Track WM_ENTERMENULOOP and WM_EXITMENULOOP messages.
     *
     *  2: IsWindowEnabled() on our window will return false.
     *
     *  3: Check GetCapture() returns NULL or equal to our window.
     *
     *  4/5: Use WindowFromPoint() too check if the cursor is in
     *       the visible portion of the window.
     *       Use GetForegroundWindow() to determine if we are the
     *       foreground window. GetAsyncKeyState(VK_xBUTTON)
     *
     *  6: GetCapture(), WindowFromPoint(), GetWindowThreadProcessId()
     *
     */

    if (bInMenu != FALSE)
    {
        return FALSE;
    }

    if (IsWindowEnabled(hWnd) == FALSE)
    {
        return FALSE;
    }

    POINT Point;
    GetCursorPos(&Point);

    // Get the mouse over window
    HWND hPointWnd = WindowFromPoint(Point);

    // Check if our window is in the foreground
    if (GetForegroundWindow() == hWnd)
    {
        // Check if the cursor is over our bounds
        if (hPointWnd == hWnd)
        {
            return TRUE;
        }

        if (GetCapture() == hWnd)
        {
            // Check if the cursor is over a captured area
            if (GetWindowThreadProcessId(hPointWnd, NULL) ==
                GetWindowThreadProcessId(hWnd, NULL))
            {
                return TRUE;
            }
        }

        return FALSE;
    }

    // See if the cursor is not in our bounds
    if (hPointWnd != hWnd)
    {
        return FALSE;
    }

    // Get the mouse button state
    WORD wMouseButtonState = 0;
    wMouseButtonState |= GetAsyncKeyState(VK_LBUTTON);
    wMouseButtonState |= GetAsyncKeyState(VK_RBUTTON);
    wMouseButtonState |= GetAsyncKeyState(VK_MBUTTON);

    // See if the mouse button is down
    if (wMouseButtonState & 0x8000)
    {
        return FALSE;
    }

    // Get the front window in the same thread
    HWND hActiveWnd = GetActiveWindow();

    // Check if the front window has capture
    if (hActiveWnd != NULL && GetCapture() == hActiveWnd)
    {
        return FALSE;
    }

    return TRUE;
}

void Cursor_VTUpdate(int x, int y)
{
    if (Cursor_IsOurs() == FALSE)
    {
        return;
    }

    if (VT_GetState() == VT_OFF)
    {
        Cursor_SetType(CURSOR_DEFAULT);
    }
    else
    {
        RECT Rect;
        POINT Point;

        if (x == -1 || y == -1) 
        {
            GetCursorPos(&Point);
            ScreenToClient(hWnd, &Point);
        }
        else
        {
            Point.x = x;
            Point.y = y;
        }

        GetDestRect(&Rect);

        if (VT_IsPageNumberAtPosition(&Rect, &Point))
        {
            Cursor_SetType(CURSOR_HAND);
        }
        else
        {
            Cursor_SetType(CURSOR_DEFAULT);
        }
    }
}

void SetDirectoryToExe()
{
    char szDriverPath[MAX_PATH];
    char* pszName;

    if (!GetModuleFileName(NULL, szDriverPath, sizeof(szDriverPath)))
    {
        ErrorBox("Cannot get module file name");
        return;
    }

    pszName = szDriverPath + strlen(szDriverPath);
    while (pszName >= szDriverPath && *pszName != '\\')
    {
        *pszName-- = 0;
    }

    SetCurrentDirectory(szDriverPath);
}

/** Process command line parameters and return them

    Routine process the command line and returns them in argv/argc format.
    Modifies the incoming string
*/
int ProcessCommandLine(char* CommandLine, char* ArgValues[], int SizeArgv)
{
   int ArgCount = 0;
   char* pCurrentChar = CommandLine;

   while (*pCurrentChar && ArgCount < SizeArgv)
   {
      // Skip any preceeding spaces.
      while (*pCurrentChar && isspace(*pCurrentChar))
      {
         pCurrentChar++;
      }
      // If the parameter starts with a double quote, copy until
      // the end quote.
      if (*pCurrentChar == '"')
      {
         pCurrentChar++;  // Skip the quote
         ArgValues[ArgCount++] = pCurrentChar;  // Save the start in the argument list.
         while (*pCurrentChar && *pCurrentChar != '"')
         {
            pCurrentChar++;
         }
         if (*pCurrentChar)
         {
             *pCurrentChar++ = '\0';   // Replace the end quote
         }
      }
      else if (*pCurrentChar) // Normal parameter, continue until white found (or end)
      {
         ArgValues[ArgCount++] = pCurrentChar++;
         while (*pCurrentChar && !isspace(*pCurrentChar))
         {
             ++pCurrentChar;
         }
         if (*pCurrentChar)
         {
             *pCurrentChar++ = '\0';
         }
      }
   }
   return ArgCount;
}


void SetTray(BOOL Way)
{
    switch (Way)
    {
    case TRUE:
        if (bIconOn==FALSE)
        {
            nIcon.cbSize = sizeof(nIcon);
            nIcon.uID = 0;
            nIcon.hIcon = LoadIcon(hResourceInst, MAKEINTRESOURCE(IDI_TRAYICON));
            nIcon.hWnd = hWnd;
            nIcon.uCallbackMessage = IDI_TRAYICON;
            sprintf(nIcon.szTip, "DScaler");
            nIcon.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
            Shell_NotifyIcon(NIM_ADD, &nIcon);
            bIconOn = TRUE;
        }
        break;
    case FALSE:
        if (bIconOn==TRUE)
        {
            Shell_NotifyIcon(NIM_DELETE, &nIcon);
            bIconOn = FALSE;
        }
        break;
    }
}

void SetTrayTip(const char* ChannelName)
{
    if (bIconOn)
    {
        nIcon.uFlags = NIF_TIP;
        sprintf(nIcon.szTip, "DScaler - %s", ChannelName);
		SetWindowText(nIcon.hWnd, nIcon.szTip);
        Shell_NotifyIcon(NIM_MODIFY, &nIcon);
    }
}

static void Init_IconMenu()
{
    hMenuTray = LoadMenu(hResourceInst, MAKEINTRESOURCE(IDC_TRAYMENU));
    if (hMenuTray!=NULL)
    {
        MENUITEMINFO mInfo;

        hMenuTray = GetSubMenu(hMenuTray, 0);

        mInfo.cbSize = sizeof(mInfo);
        mInfo.fMask = MIIM_SUBMENU;
        mInfo.hSubMenu = CreateDScalerPopupMenu();

        SetMenuItemInfo(hMenuTray, 2, TRUE, &mInfo);
		
        SetMenuDefaultItem(hMenuTray, 4, TRUE);
	}
}

int On_IconHandler(WPARAM wParam, LPARAM lParam)
{
    UINT uID;
    UINT uMouseMsg;
    POINT mPoint;

    uID = (UINT) wParam;
    uMouseMsg = (UINT) lParam;

    switch (uID)
    {
    case 0:
        switch (uMouseMsg)
        {
        case WM_LBUTTONDBLCLK:
            if (bMinimized == FALSE)
            {
                ShowWindow(hWnd, SW_MINIMIZE);
            }
            else
            {
                ShowWindow(hWnd, SW_SHOW);
                ShowWindow(hWnd, SW_RESTORE);
                SetForegroundWindow(hWnd);
            }
            break;

        case WM_RBUTTONUP:
			if (hMenuTray==NULL)
			{
				Init_IconMenu();
			}
            GetCursorPos(&mPoint);
            SetForegroundWindow(hWnd); // To correct Windows errors. See KB Q135788
            TrackPopupMenuEx(hMenuTray, TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON, mPoint.x, mPoint.y, hWnd, NULL);
            // To correct Windows  errors. See KB Q135788. I am not really sure if this is necessary
            // with TrackPopupMenuEx() but it doesn't hurt either.
            PostMessage(hWnd, WM_NULL, 0, 0);
            break;

        }
        break;
    }
	
    return 0;
}



////////////////////////////////////////////////////////////////////////////
// On Change Functions for settings
/////////////////////////////////////////////////////////////////////////////

BOOL IsFullScreen_OnChange(long NewValue)
{
    bDoResize = FALSE;
    bIsFullScreen = (BOOL)NewValue;

    // make sure that the window has been created
    if(hWnd != NULL)
    {
        if(bIsFullScreen == FALSE)
        {
			if(lPStripTimingString != NULL)
			{
				OutReso_Change(hWnd, hPSWnd, TRUE, TRUE, lPStripTimingString, TRUE);
			}
			else
			{
				OutReso_Change(hWnd, hPSWnd, TRUE, TRUE, lPStripTimingString, FALSE);
			}
            SetWindowPos(hWnd, 0, MainWndLeft, MainWndTop, MainWndWidth, MainWndHeight, SWP_SHOWWINDOW);
            if (bDisplayStatusBar == TRUE)
            {
                SetTimer(hWnd, TIMER_STATUS, TIMER_STATUS_MS, NULL);
            }
        }
        else
        {
			if(hPSWnd)
			{
				// Save the actual PowerStrip timing string in lPStripTimingString
				SaveActualPStripTiming(hPSWnd);
			}
			SaveWindowPos(hWnd);				
            KillTimer(hWnd, TIMER_STATUS);
			OutReso_Change(hWnd, hPSWnd, FALSE, TRUE, NULL, FALSE);
        }
        if (WindowBorder!=NULL)
        {
            if (bIsFullScreen && WindowBorder->Visible())
            {                
                WindowBorder->Hide();
            }
            else if (!bIsFullScreen && szSkinName[0]!=0)
            {
                WindowBorder->Show();
            }
        }
        if (ToolbarControl!=NULL)
        {            
			ToolbarControl->Set(hWnd, NULL);
        }
        
        Cursor_UpdateVisibility();
        //InvalidateRect(hWnd, NULL, FALSE);
        UpdateWindowState();
		WorkoutOverlaySize(FALSE);

		// We must do two calls, the first one displaying the toolbar
		// in order to have the correct toolbar rectangle initialized,
		// and the second to hide the toolbar if in full scrren mode
        if (ToolbarControl!=NULL)
        {            
			ToolbarControl->Set(hWnd, NULL, bIsFullScreen?1:0);
        }
		if (IsToolBarVisible())
		{
	        SetTimer(hWnd, TIMER_TOOLBAR, TIMER_TOOLBAR_MS, NULL);
		}
		else
		{
	        KillTimer(hWnd, TIMER_TOOLBAR);
		}
    }
    bDoResize = TRUE;
    return FALSE;
}

BOOL AlwaysOnTop_OnChange(long NewValue)
{
    bAlwaysOnTop = (BOOL)NewValue;
    WorkoutOverlaySize(FALSE);
    return FALSE;
}

BOOL AlwaysOnTopFull_OnChange(long NewValue)
{
    bAlwaysOnTopFull = (BOOL)NewValue;
    WorkoutOverlaySize(FALSE);
    return FALSE;
}

BOOL ScreensaverOff_OnChange(long NewValue)
{
    bScreensaverOff = (BOOL)NewValue;
    SetScreensaverMode(bScreensaverOff);
    return FALSE;
}

BOOL DisplayStatusBar_OnChange(long NewValue)
{
    bDisplayStatusBar = (BOOL)NewValue;
    if(bIsFullScreen == FALSE)
    {
        if(bDisplayStatusBar == TRUE)
        {
            SetTimer(hWnd, TIMER_STATUS, TIMER_STATUS_MS, NULL);
        }
        else
        {
            KillTimer(hWnd, TIMER_STATUS);
        }
        if (ToolbarControl!=NULL)
        {
            ToolbarControl->Adjust(hWnd, TRUE);
        }        
        UpdateWindowState();
		WorkoutOverlaySize(TRUE);
    }
    return FALSE;
}

BOOL ShowMenu_OnChange(long NewValue)
{
    bShowMenu = (BOOL)NewValue;
    if(bIsFullScreen == FALSE)
    {
        WorkoutOverlaySize(TRUE);
    }
    return FALSE;
}

BOOL KeyboardLock_OnChange(long NewValue)
{
    bKeyboardLock = (BOOL)NewValue;
    SetKeyboardLock(bKeyboardLock);
    return FALSE;
}


BOOL MinimizeHandling_OnChange(long NewValue)
{
    MinimizeHandling = (int)NewValue;
	if (bMinimized)
	{
		bCheckSignalPresent = (MinimizeHandling == 2);
	}
	else
	{
		bCheckSignalMissing = (MinimizeHandling == 2);
	}
    return FALSE;
}


BOOL ChannelPreviewNbCols_OnChange(long NewValue)
{
    ChannelPreviewNbCols = (int)NewValue;
	if (pMultiFrames && (pMultiFrames->GetMode() == PREVIEW_CHANNELS) && pMultiFrames->IsActive())
	{
		pMultiFrames->RequestSwitch();
	}
    return FALSE;
}

BOOL ChannelPreviewNbRows_OnChange(long NewValue)
{
    ChannelPreviewNbRows = (int)NewValue;
	if (pMultiFrames && (pMultiFrames->GetMode() == PREVIEW_CHANNELS) && pMultiFrames->IsActive())
	{
		pMultiFrames->RequestSwitch();
	}
    return FALSE;
}


////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING DScalerSettings[DSCALER_SETTING_LASTONE] =
{
    {
        "Window Left", SLIDER, 0, (long*)&MainWndLeft,
        10, -2048, 2048, 1, 1,
        NULL,
        "MainWindow", "StartLeft", NULL,
    },
    {
        "Window Top", SLIDER, 0, (long*)&MainWndTop,
        10, -2048, 2048, 1, 1,
        NULL,
        "MainWindow", "StartTop", NULL,
    },
    {
        "Window Width", SLIDER, 0, (long*)&MainWndWidth,
        649, 0, 2048, 1, 1,
        NULL,
        "MainWindow", "StartWidth", NULL,
    },
    {
        "Window Height", SLIDER, 0, (long*)&MainWndHeight,
        547, 0, 2048, 1, 1,
        NULL,
        "MainWindow", "StartHeight", NULL,
    },
    {
        "Always On Top (Window)", YESNO, 0, (long*)&bAlwaysOnTop,
        FALSE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "AlwaysOnTop", AlwaysOnTop_OnChange,
    },
    {
        "Full Screen", YESNO, 0, (long*)&bIsFullScreen,
        FALSE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "bIsFullScreen", IsFullScreen_OnChange,
    },
    {
        "Force Full Screen", ONOFF, 0, (long*)&bForceFullScreen,
        FALSE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "AlwaysForceFullScreen", NULL,
    },
    {
        "Status Bar", ONOFF, 0, (long*)&bDisplayStatusBar,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Show", "StatusBar", DisplayStatusBar_OnChange,
    },
    {
        "Menu", ONOFF, 0, (long*)&bShowMenu,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Show", "Menu", ShowMenu_OnChange,
    },
    {
        "Window Processor", SLIDER, 0, (long*)&MainProcessor,
        0, 0, 3, 1, 1,
        NULL,
        "Threads", "WindowProcessor", NULL,
    },
    {
        "Thread Processor", SLIDER, 0, (long*)&DecodeProcessor,
        0, 0, 3, 1, 1,
        NULL,
        "Threads", "DecodeProcessor", NULL,
    },
    {
        "UI Thread", ITEMFROMLIST, 0, (long*)&PriorClassId,
        0, 0, 2, 1, 1,
        UIPriorityNames,
        "Threads", "WindowPriority", NULL,
    },
    {
        "Decoding / Output Thread", ITEMFROMLIST, 0, (long*)&ThreadClassId,
        1, 0, 4, 1, 1,
        DecodingPriorityNames,
        "Threads", "ThreadPriority", NULL,
    },
    {
        "Autosave settings", ONOFF, 0, (long*)&bUseAutoSave,
        FALSE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "UseAutoSave", NULL,
    },
    {
        "Always On Top (Full Screen)", YESNO, 0, (long*)&bAlwaysOnTopFull,
        TRUE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "AlwaysOnTopFull", AlwaysOnTopFull_OnChange,
    },
    {
        "Show Crash Dialog", ONOFF, 0, (long*)&bShowCrashDialog,
        FALSE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "ShowCrashDialog", NULL,
    },
    {
        "Splash Screen", ONOFF, 0, (long*)&bDisplaySplashScreen,
        TRUE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "DisplaySplashScreen", NULL,
    },
    {
        "Auto Hide Cursor", ONOFF, 0, (long*)&bAutoHideCursor,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Show", "AutoHideCursor", NULL,
    },
    {
        "Lock keyboard", ONOFF, 0, (long*)&bKeyboardLock,
        FALSE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "KeyboardLock", KeyboardLock_OnChange,
    },
    {
        "Disable Screensaver", YESNO, 0, (long*)&bScreensaverOff,
        TRUE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "ScreensaverOff", ScreensaverOff_OnChange,
    },
    {
        "Auto CodePage", YESNO, 0, (long*)&bVTAutoCodePage,
        TRUE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "AutoCodePage", NULL,
    },
    {
        "VT Anti-alias", YESNO, 0, (long*)&bVTAntiAlias,
        TRUE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "VTAntiAlias", NULL,
    },
    {
        "Initial source", SLIDER, 0, (long*)&InitSourceIdx,
        -1, -1, 100, 1, 1,
        NULL,
        "Show", "InitialSource", NULL,
    },
    {
        "Channel enter time", SLIDER, 0, (long*)&ChannelEnterTime,
        TIMER_KEYNUMBER_MS, 0, 5000, 1, 1,
        NULL,
        "MainWindow", "ChannelEnterTime", NULL,
    },
    {
        "Processor Speed", SLIDER, 0, (long*)&ProcessorSpeed,
        1, 0, 3, 1, 1,
        NULL,
        "MainWindow", "ProcessorSpeed", NULL,
    },
    {
        "Quality Trade Off", SLIDER, 0, (long*)&TradeOff,
        1, 0, 1, 1, 1,
        NULL,
        "MainWindow", "TradeOff", NULL,
    },
    {
        "Use Full CPU", SLIDER, 0, (long*)&FullCpu,
        1, 0, 2, 1, 1,
        NULL,
        "MainWindow", "FullCpu", NULL,
    },
    {
        "Video Card", SLIDER, 0, (long*)&VideoCard,
        0, 0, 0, 1, 1,
        NULL,
        "MainWindow", "VideoCard", NULL,
    },
    {
        "Reverse channel scrolling", ONOFF, 0, (long*)&bReverseChannelScroll,
        FALSE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "ReverseChannelScroll", NULL,
    },
    {
        "Single key teletext toggle", ONOFF, 0, (long*)&bVTSingleKeyToggle,
        TRUE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "SingleKeyTeletextToggle", NULL,
    },
    {
        "Minimize to the Windows system tray", ONOFF, 0, (long*)&bMinToTray,
        FALSE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "MinToTray", NULL,
    },
    {
        "Minimize handling", ITEMFROMLIST, 0, (long*)&MinimizeHandling,
        0, 0, 2, 1, 1,
        MinimizeHandlingLabels,
        "MainWindow", "MinimizeHandling", MinimizeHandling_OnChange,
    },
    {
        "Display Resolution in Full Screen", SLIDER, 0, (long*)&OutputReso,
        0, 0, MAX_NUMBER_RESO, 1, 1,
        NULL,
        "MainWindow", "ResoFullScreen", NULL,
    },
    {
        "PowerStrip resolution for 576i sources", CHARSTRING, 0, (long*)&PStrip576i,
        (long)"", 0, 0, 0, 0,
        NULL,
        "PStripOutResolution", "576i", NULL,
    },
    {
        "PowerStrip resolution for 480i sources", CHARSTRING, 0, (long*)&PStrip480i,
        (long)"", 0, 0, 0, 0,
        NULL,
        "PStripOutResolution", "480i", NULL,
    },
    {
        "Skin name", CHARSTRING, 0, (long*)&szSkinName,
        (long)"", 0, 0, 0, 0,
        NULL,
        "MainWindow", "SkinName", NULL,
    },
    {
        "Number of columns in preview mode", SLIDER, 0, (long*)&ChannelPreviewNbCols,
         4, 2, 10, 1, 1,
         NULL,
        "Still", "ChannelPreviewNbCols", ChannelPreviewNbCols_OnChange,
    },
    {
        "Number of rows in preview mode", SLIDER, 0, (long*)&ChannelPreviewNbRows,
         4, 2, 10, 1, 1,
         NULL,
        "Still", "ChannelPreviewNbRows", ChannelPreviewNbRows_OnChange,
    },
};

SETTING* DScaler_GetSetting(DSCALER_SETTING Setting)
{
    if(Setting > -1 && Setting < DSCALER_SETTING_LASTONE)
    {
        return &(DScalerSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void DScaler_ReadSettingsFromIni()
{
    int i;
    for(i = 0; i < DSCALER_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(DScalerSettings[i]));
    }

    if(bForceFullScreen)
    {
        bIsFullScreen = TRUE;
    }
    if(bKeyboardLock)
    {
        SetKeyboardLock(TRUE);
    }
    ScreensaverOff_OnChange(bScreensaverOff);

    VT_SetAutoCodepage(NULL, NULL, bVTAutoCodePage);
    VT_SetAntialias(NULL, NULL, bVTAntiAlias);
}

void DScaler_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < DSCALER_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(DScalerSettings[i]), bOptimizeFileAccess);
    }
}

void DScaler_FreeSettings()
{
    int i;
    for(i = 0; i < DSCALER_SETTING_LASTONE; i++)
    {
        Setting_Free(&(DScalerSettings[i]));
    }
}

CTreeSettingsGeneric* DScaler_GetTreeSettingsPage()
{
    return new CTreeSettingsGeneric("Threads Priority Settings", &DScalerSettings[WINDOWPRIORITY], AUTOSAVESETTINGS - WINDOWPRIORITY);
}

CTreeSettingsGeneric* DScaler_GetTreeSettingsPage2()
{
    // Other Settings
    SETTING* OtherSettings[6] =
    {
        &DScalerSettings[DISPLAYSPLASHSCREEN    ],
        &DScalerSettings[AUTOHIDECURSOR         ],
        &DScalerSettings[LOCKKEYBOARD           ],
        &DScalerSettings[SCREENSAVEROFF         ],
        &DScalerSettings[SINGLEKEYTELETEXTTOGGLE],
        &DScalerSettings[MINIMIZEHANDLING       ],
    };
    return new CTreeSettingsGeneric("Other Settings", OtherSettings, sizeof(OtherSettings) / sizeof(OtherSettings[0]));
}

CTreeSettingsGeneric* DScaler_GetTreeSettingsPage3()
{
    // Channel Settings
    SETTING* OtherSettings[3] =
    {
        &DScalerSettings[REVERSECHANNELSCROLLING],
        &DScalerSettings[CHANNELPREVIEWWNBCOLS],
        &DScalerSettings[CHANNELPREVIEWNBROWS],
    };
    return new CTreeSettingsGeneric("Channel Settings", OtherSettings, sizeof(OtherSettings) / sizeof(OtherSettings[0]));
}

CTreeSettingsGeneric* DScaler_GetTreeSettingsPage4()
{
    // PowerStrip Settings
    SETTING* OtherSettings[2] =
    {
        &DScalerSettings[PSTRIPRESO576I			],
        &DScalerSettings[PSTRIPRESO480I			],
    };
    return new CTreeSettingsGeneric("PowerStrip Settings", OtherSettings, sizeof(OtherSettings) / sizeof(OtherSettings[0]));
}
