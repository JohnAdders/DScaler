#ifndef _TOOLBARS_H_
#define _TOOLBARS_H_

#include "Events.h"
#include "ToolbarWindow.h"

//For eSoundChannel
#include "SoundChannel.h"


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
	HICON m_hIconChannelUp;
	HICON m_hIconChannelDown;
	HICON m_hIconChannelPrevious;

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
	int  m_VolumeMin;
	int  m_VolumeMax;
	BOOL m_UseMixer;
	eSoundChannel m_SoundChannel;

	HICON m_hIconMute;
	HICON m_hIconUnMute;

	HICON m_hIconMono;
	HICON m_hIconStereo;
	HICON m_hIconLang1;
	HICON m_hIconLang2;
        
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


class CToolbar1Bar : public CToolbarChild, public CEventObject 
{
private:
	int OriginalWidth;
	int OriginalHeight;
	int LeftMargin;
	int RightMargin;
	HWND hWndPicture;
	HBITMAP hBmp;	
public:
    CToolbar1Bar(CToolbarWindow *pToolbar);	
	~CToolbar1Bar();
	HWND Create(LPCSTR lpClassName, HINSTANCE hResourceInst);
    LRESULT ToolbarChildProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);	
	BOOL LoadSkin(const char *szSkinIniFile, const char *szSection, vector<int> *Results, CBitmapCache *pBitmapCache);
	void ClearSkin();
    void Reset();
    HWND GethWndPicture() { return hWndPicture; }
	void Margins(int l,int r);
};


//Toolbar management
void SetToolbars(HWND hWnd, LPCSTR szSkinName);
void UpdateToolbars(HWND hWnd, BOOL bRedraw);
void Toolbars_UpdateMenu(HMENU hMenu);
BOOL ProcessToolbar1Selection(HWND hWnd, UINT uItem);
void FreeToolbars();
void Toolbars_AdjustArea(LPRECT lpRect, int Crop);

#endif
