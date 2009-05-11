#pragma once

void RemoteRegister();
LONG Remote_HandleMsgs(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, BOOL* bDone);
