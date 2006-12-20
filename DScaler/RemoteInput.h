#pragma once

void RemoteRegister();
LONG Remote_HandleMsgs(HWND hWnd, UINT message, UINT wParam, LONG lParam, BOOL* bDone);
