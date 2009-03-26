////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Rob Muller.  All rights reserved.
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

#include "windows.h"

int ProcessCommandLine(char* CommandLine, char* ArgValues[], int SizeArgv);

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    UINT MsgOSDShow = RegisterWindowMessage("DScalerShowOSDMsgString");

    // make a copy of the command line since ProcessCommandLine() will replace the spaces with \0
    char OldCmdLine[1024];
    strncpy(OldCmdLine, lpCmdLine, sizeof(OldCmdLine));

    char* ArgValues[20];
    int ArgCount = ProcessCommandLine(lpCmdLine, ArgValues, sizeof(ArgValues) / sizeof(char*));

    if(ArgCount == 0)
    {
        MessageBox(NULL, "You can use this application to send messages to the OSD of DScaler.\n\
Note that DScaler itself can do the same, SendMsg is just a lot faster.\n\n\
Usage:\nSendMsg [/m] [/M] [message]\n\n\
/m: show a temporary message.\n\
/M: show a persistent message.\n\n\
Use \\n to start a new line.\n\
If DScaler is not running this program exits silently.\n\n\
Example:\n\
SendMsg /m This is a message.\\nSecond line.\
", "SendMsg", MB_OK | MB_ICONINFORMATION);
    }
    else
    {
        // if we are already running then start up old version
        HWND hPrevWindow = FindWindow((LPCTSTR) "DScaler", NULL);
        if (hPrevWindow != NULL)
        {
        /*        if (IsIconic(hPrevWindow))
        {
        SendMessage(hPrevWindow, WM_SYSCOMMAND, SC_RESTORE, NULL);
        }
        SetFocus(hPrevWindow);
        SetActiveWindow(hPrevWindow);
        SetForegroundWindow(hPrevWindow);
            */
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
        }
    }
    return 0;
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
