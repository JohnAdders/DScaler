#ifndef _TOOLBARCONTROL_H_
#define _TOOLBARCONTROL_H_

#include "Setting.h"
#include "ToolbarWindow.h"
#include "Toolbars.h"

class CToolbarControl : public CSettingsHolder
{
protected:
    CToolbarWindow *Toolbar1;                 //Main toolbar 1
    CToolbarChannels *Toolbar1Channels;;      //Childs of the toolbar
    CToolbarVolume *Toolbar1Volume;
    CToolbarLogo *Toolbar1Logo;

public:
    CToolbarControl(long SetMessage);
    ~CToolbarControl();

    void Set(HWND hWnd, LPCSTR szSkinName);
    void Adjust(HWND hWnd, BOOL bRedraw);
    void UpdateMenu(HMENU hMenu);
    void Free();
    void AdjustArea(LPRECT lpRect, int Crop);

    BOOL ProcessToolbar1Selection(HWND hWnd, UINT uItem);

private:
    void CreateSettings(LPCSTR IniSection);

    DEFINE_YESNO_CALLBACK_SETTING(CToolbarControl, ShowToolbar1);
    DEFINE_SLIDER_CALLBACK_SETTING(CToolbarControl, Toolbar1Position);

    DEFINE_SLIDER_CALLBACK_SETTING(CToolbarControl, Toolbar1Channels);
    DEFINE_SLIDER_CALLBACK_SETTING(CToolbarControl, Toolbar1Volume);
};


#endif
