#ifndef _TOOLBARS_H_
#define _TOOLBARS_H_

#include "Events.h"
#include "ToolbarWindow.h"


class CToolbarChannels : public CToolbarChild 
{
public:
    CToolbarChannels(CToolbarWindow *pToolbar);
    ~CToolbarChannels();

    void Reset();
    
    int LastChannel;
private:

    void UpdateControls(HWND hWnd,bool bInitDialog);
    LRESULT ToolbarChildProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    //LRESULT ButtonChildProc(string sID, HWND hWndParent, UINT MouseFlags, HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    static void ChannelChange(void *pThis, eEventType EventType, long OldValue, long NewValue, eEventType *ComingUp);
};



class CToolbarVolume : public CToolbarChild 
{
public:
    CToolbarVolume(CToolbarWindow *pToolbar);

    LRESULT ToolbarChildProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    
    void Update(int What, int Value);
    void Reset();

private:
    BOOL m_Mute;
    int  m_Volume;
        
    void UpdateControls(HWND hWnd, bool bInitDialog);
};


class CToolbarLogo : public CToolbarChild 
{
public:
    CToolbarLogo(CToolbarWindow *pToolbar);
    LRESULT ToolbarChildProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);        
    void Reset();    
};


//Toolbar management
void SetToolbars(HWND hWnd, LPCSTR szSkinName);
void UpdateToolbars(HWND hWnd, BOOL bRedraw);
void Toolbars_UpdateMenu(HMENU hMenu);
BOOL ProcessToolbar1Selection(HWND hWnd, UINT uItem);
void FreeToolbars();
void Toolbars_AdjustArea(LPRECT lpRect, int Crop);

#endif
