#ifndef _TOOLBARS_H_
#define _TOOLBARS_H_

#include "Events.h"
#include "ToolbarWindow.h"


class CToolbarChannels : public CToolbarChild, public CEventObject 
{
public:
    CToolbarChannels(CToolbarWindow *pToolbar);
    ~CToolbarChannels();

	HWND CreateFromDialog(LPCTSTR lpTemplate, HINSTANCE hResourceInst);
    void Reset();
        
	void OnEvent(CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp);
	void UpdateWindow() { UpdateControls(NULL, FALSE); }

	LRESULT MyComboProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
private:
	int LastChannel;
	WNDPROC m_oldComboProc;

    void UpdateControls(HWND hWnd,bool bInitDialog);
    LRESULT ToolbarChildProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);    
	
	static LRESULT CALLBACK MyComboProcWrap(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};



class CToolbarVolume : public CToolbarChild, public CEventObject
{
public:
    CToolbarVolume(CToolbarWindow *pToolbar);
	~CToolbarVolume();

    LRESULT ToolbarChildProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    
    void Reset();

	void OnEvent(CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp);
	void UpdateWindow() { UpdateControls(NULL, FALSE); }
private:
    BOOL m_Mute;
    int  m_Volume;
        
    void UpdateControls(HWND hWnd, bool bInitDialog);	
};


class CToolbarLogo : public CToolbarChild, public CEventObject 
{
private:
	int OriginalLogoWidth;
	int OriginalLogoHeight;
public:
    CToolbarLogo(CToolbarWindow *pToolbar);	
	HWND CreateFromDialog(LPCTSTR lpTemplate, HINSTANCE hResourceInst);
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
