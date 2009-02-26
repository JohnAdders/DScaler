#undef WINVER
#undef _WIN32_WINNT
#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#include "stdafx.h"
#include "windows.h"
#include "RemoteInput.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"

#ifdef WM_INPUT
BOOL (WINAPI* lpRegisterRawInputDevices)(IN PCRAWINPUTDEVICE pRawInputDevices,IN UINT uiNumDevices,IN UINT cbSize) = NULL;
UINT (WINAPI* lpGetRawInputData)(IN HRAWINPUT hRawInput,IN UINT uiCommand,OUT LPVOID pData,IN OUT PUINT pcbSize,IN UINT cbSizeHeader) = NULL;
#endif


void RemoteRegister()
{
#ifdef WM_INPUT
    // we've got to load these functions dynamically 
    // so that we continue to run on NT 4
    HINSTANCE h = LoadLibrary("user32.dll");
    lpRegisterRawInputDevices = (BOOL (WINAPI *)(IN PCRAWINPUTDEVICE pRawInputDevices,IN UINT uiNumDevices,IN UINT cbSize))GetProcAddress(h,"RegisterRawInputDevices");
    lpGetRawInputData = (UINT (WINAPI*)(IN HRAWINPUT hRawInput,IN UINT uiCommand,OUT LPVOID pData,IN OUT PUINT pcbSize,IN UINT cbSizeHeader))GetProcAddress(h,"GetRawInputData");;

    // register for messages from MCE remote
    // this will mean we will get sent WM_INPUT messages
    if(lpRegisterRawInputDevices && lpGetRawInputData)
    {
        RAWINPUTDEVICE Rid[1];

        Rid[0].usUsagePage = 0xFFBC;
        Rid[0].usUsage = 0x88; 
        Rid[0].dwFlags = 0;
        Rid[0].hwndTarget = NULL;

        lpRegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));
    }
    // If the library was loaded by calling LoadLibrary(),
    // then you must use FreeLibrary() to let go of it.
    // Since we will already have an outstanding reference to user32.dll
    // it's OK to Free it here before we've even called the functions
    FreeLibrary(h);
#endif
}




#ifdef WM_APPCOMMAND
// Cope with new media commands 
// MCE remote sends these for the basic keys
LONG OnAppCommand(HWND hWnd, UINT wParam, LONG lParam)
{
    LONG RetVal = TRUE;

    switch(GET_APPCOMMAND_LPARAM(lParam))
    {
    case APPCOMMAND_BROWSER_BACKWARD:
        PostMessage(hWnd, WM_CLOSE, 0, 0);
        break;
    case APPCOMMAND_MEDIA_CHANNEL_DOWN:
        PostMessage(hWnd, WM_COMMAND, IDM_CHANNELMINUS, 0);
        break;
    case APPCOMMAND_MEDIA_CHANNEL_UP:
        PostMessage(hWnd, WM_COMMAND, IDM_CHANNELPLUS, 0);
        break;
    case APPCOMMAND_MEDIA_NEXTTRACK:
        PostMessage(hWnd, WM_COMMAND, IDM_PLAYLIST_NEXT, 0);
        break;
    case APPCOMMAND_MEDIA_PREVIOUSTRACK:
        PostMessage(hWnd, WM_COMMAND, IDM_PLAYLIST_PREVIOUS, 0);
        break;
    case APPCOMMAND_MEDIA_PLAY_PAUSE:
        PostMessage(hWnd, WM_COMMAND, IDM_CAPTURE_PAUSE, 0);
        break;
    case APPCOMMAND_VOLUME_DOWN:
        PostMessage(hWnd, WM_COMMAND, IDM_VOLUMEMINUS, 0);
        break;
    case APPCOMMAND_VOLUME_UP:
        PostMessage(hWnd, WM_COMMAND, IDM_VOLUMEPLUS, 0);
        break;
    case APPCOMMAND_VOLUME_MUTE:
        PostMessage(hWnd, WM_COMMAND, IDM_MUTE, 0);
        break;
    default:
        RetVal = FALSE;
        break;
    }
    return RetVal;
}
#endif

#ifdef WM_INPUT
LONG OnInput(HWND hWnd, UINT wParam, LONG lParam)
{
    if(GET_RAWINPUT_CODE_WPARAM(wParam) == RIM_INPUT)
    {
        UINT dwSize;
        BOOL bHandled = FALSE;

        lpGetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
        LPBYTE lpb = new BYTE[dwSize];

        if (lpb != NULL) 
        {
            if (lpGetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) == dwSize )
            {
                RAWINPUT* raw = (RAWINPUT*)lpb;

                if (raw->header.dwType == RIM_TYPEHID) 
                {
                    if(raw->data.hid.dwSizeHid == 2 && raw->data.hid.dwCount == 1)
                    {
                        if(raw->data.hid.bRawData[0] == 3)
                        {
                            switch(raw->data.hid.bRawData[1])
                            {
                            case 13:
                                // ehome (green button)
                                // just exit if the user want to get
                                // the media center home page
                                // (or any of the other TV ones)
                                PostMessage(hWnd, WM_CLOSE, 0, 0);
                                break;
                            case 90:
                                // teletext
                                PostMessage(hWnd, WM_COMMAND, IDM_CALL_VIDEOTEXT, 0);
                                bHandled = TRUE;
                                break;
                            case 91:
                                //red
                                PostMessage(hWnd, WM_COMMAND, IDM_TELETEXT_KEY1, 0);
                                bHandled = TRUE;
                                break;
                            case 92:
                                // green
                                PostMessage(hWnd, WM_COMMAND, IDM_TELETEXT_KEY2, 0);
                                bHandled = TRUE;
                                break;
                            case 93:
                                // yellow
                                PostMessage(hWnd, WM_COMMAND, IDM_TELETEXT_KEY3, 0);
                                bHandled = TRUE;
                                break;
                            case 94:
                                // blue
                                PostMessage(hWnd, WM_COMMAND, IDM_TELETEXT_KEY4, 0);
                                bHandled = TRUE;
                                break;
                            default:
                                break;
                            }
                        }
                    }
                } 
            }
            
            delete[] lpb; 
        }
        
        if(bHandled)
        {
            return FALSE;
        }
        else
        {
            DefWindowProc(hWnd, WM_INPUT, wParam, lParam);
            return TRUE;
        }
    }
    else
    {
        DefWindowProc(hWnd, WM_INPUT, wParam, lParam);
        return TRUE;
    }
}
#endif


LONG Remote_HandleMsgs(HWND hWnd, UINT message, UINT wParam, LONG lParam, BOOL* bDone)
{
#ifdef WM_INPUT
    if(message == WM_INPUT)
    {
        *bDone = TRUE;
        return OnInput(hWnd, wParam, lParam);
    }
#endif
#ifdef WM_APPCOMMAND
    if(message == WM_APPCOMMAND)
    {
        *bDone = TRUE;
        return OnAppCommand(hWnd, wParam, lParam);
    }
#endif
    return 0;
}
