/////////////////////////////////////////////////////////////////////////////
// $Id: TreeSettingsGeneric.cpp,v 1.13 2003-04-28 13:21:46 laurentg Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Torbjörn Jansson.  All rights reserved.
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
// Change Log
//
// Date          Developer             Changes
//
// 22 Jun 2001   Torbjörn Jansson      Initial release
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.12  2003/04/26 23:19:15  laurentg
// Character string settings
//
// Revision 1.11  2003/01/10 17:38:38  adcockj
// Interrim Check in of Settings rewrite
//  - Removed SETTINGSEX structures and flags
//  - Removed Seperate settings per channel code
//  - Removed Settings flags
//  - Cut away some unused features
//
// Revision 1.10  2002/11/08 18:33:36  atnak
// Fixed a problem with the last change
//
// Revision 1.9  2002/11/08 17:39:41  atnak
// FIxed combo boxes not working properly with up/down arrows
//
// Revision 1.8  2002/10/24 12:03:18  atnak
// Added a constructor that takes an array SETTING pointers
//
// Revision 1.7  2002/10/23 09:46:46  adcockj
// Allow NULL entries in Settings lists sent to Generic settings dialog
//
// Revision 1.6  2002/10/19 15:15:42  tobbej
// Implemented new gradient header above active page.
// Changed resizing a bit to minimize flashing when repainting window
//
// Revision 1.5  2002/10/15 15:04:01  kooiman
// Resizable tree setting dialog and use ISetting/CSimpleSetting as setting interface.
//
// Revision 1.4  2002/09/28 13:34:36  kooiman
// Added sender object to events and added setting flag to treesettingsgeneric.
//
// Revision 1.3  2002/09/26 10:03:52  kooiman
// Small adaptation for setting interface.
//
// Revision 1.2  2002/09/02 19:06:10  kooiman
// It is now possible to modify CSimpleSetting style settings from the TreeSettingDialog
//
// Revision 1.1  2002/04/24 19:04:01  tobbej
// new treebased settings dialog
//
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file TreeSettingsGeneric.cpp Implementation for CTreeSettingsGeneric class
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "TreeSettingsGeneric.h"
#include "Settings.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTreeSettingsGeneric dialog

CTreeSettingsGeneric::CTreeSettingsGeneric(CString name,SETTING* settings,long count)
	:CTreeSettingsPage(name,CTreeSettingsGeneric::IDD),
	m_CurrentSetting(0),
    m_Settings(),
    m_DeleteSettingsOnExit(FALSE)
{
    //{{AFX_DATA_INIT(CTreeSettingsGeneric)
    //}}AFX_DATA_INIT

    m_SettingsCount = m_Settings.LoadSettingStructures(settings, 0, count);
    m_DeleteSettingsOnExit = TRUE;
}

CTreeSettingsGeneric::CTreeSettingsGeneric(CString name,SETTING** settings,long count)
	:CTreeSettingsPage(name,CTreeSettingsGeneric::IDD),
	m_CurrentSetting(0),
    m_Settings(),
    m_DeleteSettingsOnExit(FALSE)
{
    //{{AFX_DATA_INIT(CTreeSettingsGeneric)
    //}}AFX_DATA_INIT

    for (int i(0); i < count; i++)
    {
        m_Settings.AddSetting(settings[i]);
    }

    // This RegisterMe() function and its use seems
    // VERY hacky and redundant.  It's protected so
    // I can't call it here anyway.  AddSetting()
    // calls it once in every loop above so it's
    // really not necessary here.  --atnak 02-10-24
    // m_Settings.RegisterMe();

    m_SettingsCount = count;
    m_DeleteSettingsOnExit = TRUE;
}

CTreeSettingsGeneric::CTreeSettingsGeneric(CString name,vector<CSimpleSetting*> csettings)
	:CTreeSettingsPage(name,CTreeSettingsGeneric::IDD),
	m_CurrentSetting(0),
    m_Settings(),
    m_DeleteSettingsOnExit(FALSE)
{
    m_SettingsCount = csettings.size();
    for (int i = 0; i < m_SettingsCount; i++)
    {
        m_Settings.AddSetting(csettings[i]);
    }
}

CTreeSettingsGeneric::~CTreeSettingsGeneric()
{
    //Clear list, don't write settings
    m_Settings.ClearSettingList(m_DeleteSettingsOnExit, FALSE);
}

void CTreeSettingsGeneric::DoDataExchange(CDataExchange* pDX)
{
    CTreeSettingsPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CTreeSettingsGeneric)
    DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_CHOOSEFROMLIST, m_Combo);
    DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_CHECK, m_CheckBox);
    DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_SPIN, m_Spin);
    DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_SLIDER, m_Slider);
    DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_EDIT, m_Edit);
    DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_EDIT2, m_EditString);
    DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_DEFAULT, m_DefaultButton);
    DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_LIST, m_ListBox);
	DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_SAVEPER_GLOBAL, m_CheckGlobalBox);
	DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_SAVEPER_SOURCE, m_CheckSourceBox);
	DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_SAVEPER_VIDEOINPUT, m_CheckVideoInputBox);
	DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_SAVEPER_AUDIOINPUT, m_CheckAudioInputBox);
	DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_SAVEPER_VIDEOFORMAT, m_CheckVideoFormatBox);
	DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_SAVEPER_CHANNEL, m_CheckChannelBox);
	DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_SETTINGINFO, m_SavePerInfoBox);
	DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_TOPBOX, m_TopGroupBox);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTreeSettingsGeneric, CTreeSettingsPage)
    //{{AFX_MSG_MAP(CTreeSettingsGeneric)
    ON_LBN_SELCHANGE(IDC_TREESETTINGS_GENERIC_LIST, OnSelchangeList)
    ON_EN_CHANGE(IDC_TREESETTINGS_GENERIC_EDIT, OnChangeEdit)
    ON_EN_KILLFOCUS(IDC_TREESETTINGS_GENERIC_EDIT2, OnChangeEditString)
    ON_WM_HSCROLL()
    ON_BN_CLICKED(IDC_TREESETTINGS_GENERIC_DEFAULT, OnSettingsDefault)
    ON_BN_CLICKED(IDC_TREESETTINGS_GENERIC_CHECK, OnCheckClick)
    ON_CBN_SELCHANGE(IDC_TREESETTINGS_GENERIC_CHOOSEFROMLIST, OnSelchangeChoosefromlist)
    ON_NOTIFY(UDN_DELTAPOS, IDC_TREESETTINGS_GENERIC_SPIN, OnDeltaposSettingsSpin)
	ON_BN_CLICKED(IDC_TREESETTINGS_GENERIC_SAVEPER_GLOBAL, OnCheckGlobalClick)
	ON_BN_CLICKED(IDC_TREESETTINGS_GENERIC_SAVEPER_SOURCE, OnCheckSourceClick)
	ON_BN_CLICKED(IDC_TREESETTINGS_GENERIC_SAVEPER_VIDEOINPUT, OnCheckVideoInputClick)
	ON_BN_CLICKED(IDC_TREESETTINGS_GENERIC_SAVEPER_AUDIOINPUT, OnCheckAudioInputClick)
	ON_BN_CLICKED(IDC_TREESETTINGS_GENERIC_SAVEPER_VIDEOFORMAT, OnCheckVideoFormatClick)
	ON_BN_CLICKED(IDC_TREESETTINGS_GENERIC_SAVEPER_CHANNEL, OnCheckChannelClick)
    ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTreeSettingsGeneric message handlers

BOOL CTreeSettingsGeneric::OnInitDialog()
{
    CTreeSettingsPage::OnInitDialog();

    //add relevant settings to listbox
    for(int i=0;i<m_SettingsCount;i++)
    {
        CSimpleSetting* pSetting = (CSimpleSetting*)m_Settings[i];
        if ((pSetting != NULL) && (pSetting->GetDisplayName() != NULL) && (pSetting->GetType() != NOT_PRESENT))
        {
            int index=m_ListBox.AddString(pSetting->GetDisplayName());
            if(index!=LB_ERR)
            {
                m_ListBox.SetItemData(index,i);
            }
        }
    }

    if(m_SettingsCount>0)
    {
        m_ListBox.SetCurSel(0);
        OnSelchangeList();
    }
    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTreeSettingsGeneric::OnSelchangeList()
{
    long idx = m_ListBox.GetItemData(m_ListBox.GetCurSel());
    m_CurrentSetting = idx;

    SETTING_TYPE Type = NOT_PRESENT;

    if (m_Settings[m_CurrentSetting] != NULL)
    {
        Type = m_Settings[m_CurrentSetting]->GetType();
    }

    switch(Type)
    {
    case YESNO:
    case ONOFF:
        m_DefaultButton.ShowWindow(SW_SHOWNA);
        m_CheckBox.ShowWindow(SW_SHOWNA);
        m_Edit.ShowWindow(SW_HIDE);
        m_EditString.ShowWindow(SW_HIDE);
        m_Slider.ShowWindow(SW_HIDE);
        m_Spin.ShowWindow(SW_HIDE);
        m_Combo.ShowWindow(SW_HIDE);
        break;

    case SLIDER:
        m_DefaultButton.ShowWindow(SW_SHOWNA);
        m_Edit.ShowWindow(SW_SHOWNA);
        if ( (m_Settings[m_CurrentSetting] != NULL) && (m_Settings[m_CurrentSetting]->GetMax() <= UD_MAXVAL) && (m_Settings[m_CurrentSetting]->GetMin() >= UD_MINVAL) )
        {
            m_Spin.ShowWindow(SW_SHOWNA);
        }
        else
        {
            m_Spin.ShowWindow(SW_HIDE);
        }
        m_Slider.ShowWindow(SW_SHOWNA);
        m_EditString.ShowWindow(SW_HIDE);
        m_CheckBox.ShowWindow(SW_HIDE);
        m_Combo.ShowWindow(SW_HIDE);
        break;

    case ITEMFROMLIST:
        m_CheckBox.ShowWindow(SW_HIDE);
        m_DefaultButton.ShowWindow(SW_SHOWNA);
        m_Edit.ShowWindow(SW_HIDE);
        m_EditString.ShowWindow(SW_HIDE);
        m_Slider.ShowWindow(SW_HIDE);
        m_Spin.ShowWindow(SW_HIDE);
        m_Combo.ShowWindow(SW_SHOWNA);
        break;

    case CHARSTRING:
        m_DefaultButton.ShowWindow(SW_SHOWNA);
        m_EditString.ShowWindow(SW_SHOWNA);
        m_CheckBox.ShowWindow(SW_HIDE);
        m_Edit.ShowWindow(SW_HIDE);
        m_Slider.ShowWindow(SW_HIDE);
        m_Spin.ShowWindow(SW_HIDE);
        m_Combo.ShowWindow(SW_HIDE);
        break;

    default:
        m_CheckBox.ShowWindow(SW_HIDE);
        m_DefaultButton.ShowWindow(SW_HIDE);
        m_Edit.ShowWindow(SW_HIDE);
        m_EditString.ShowWindow(SW_HIDE);
        m_Slider.ShowWindow(SW_HIDE);
        m_Spin.ShowWindow(SW_HIDE);
        m_Combo.ShowWindow(SW_HIDE);
        break;
    }


    char *szName = NULL;
    if (m_Settings[m_CurrentSetting] != NULL)
    {
        szName = (char*)((CSimpleSetting*)m_Settings[m_CurrentSetting])->GetDisplayName();
        if (szName == NULL)
        {
            szName = (char*)((CSimpleSetting*)m_Settings[m_CurrentSetting])->GetEntry();
        }
    }

    if (szName == NULL)
    {
        szName = "";
    }

    // JA 10/Jan/2003
    // supressed display of boxes to avoid user confusion
    // boxes are invisible in dialog
	BOOL bShowBoxes = FALSE;
	if (bShowBoxes)
	{
		m_SavePerInfoBox.ShowWindow(SW_HIDE);
		m_CheckGlobalBox.ShowWindow(SW_SHOW);
		m_CheckSourceBox.ShowWindow(SW_SHOW);
		m_CheckVideoInputBox.ShowWindow(SW_SHOW);
		m_CheckAudioInputBox.ShowWindow(SW_SHOW);
		m_CheckVideoFormatBox.ShowWindow(SW_SHOW);
		m_CheckChannelBox.ShowWindow(SW_SHOW);

		char szBuffer[200];

		if (szName[0] != 0)
		{
			sprintf(szBuffer,"Load and save \"%s\" per",szName);
		} else {
			szBuffer[0] = 0;
		}
		m_TopGroupBox.SetWindowText(szBuffer);
	}
	else
	{
		m_CheckGlobalBox.ShowWindow(SW_HIDE);
		m_CheckSourceBox.ShowWindow(SW_HIDE);
		m_CheckVideoInputBox.ShowWindow(SW_HIDE);
		m_CheckAudioInputBox.ShowWindow(SW_HIDE);
		m_CheckVideoFormatBox.ShowWindow(SW_HIDE);
		m_CheckChannelBox.ShowWindow(SW_HIDE);

		m_TopGroupBox.SetWindowText("");
		m_SavePerInfoBox.SetWindowText(szName);
		//m_SavePerInfoBox.ShowWindow(SW_SHOW);
        m_SavePerInfoBox.ShowWindow(SW_HIDE);
	}

    UpdateControls();
}

void CTreeSettingsGeneric::UpdateControls()
{
    ASSERT(m_CurrentSetting>=0 && m_CurrentSetting<m_SettingsCount);

    static bool bInUpdate = false;

    //if we dont do this check there will be a loop
    if(bInUpdate)
    {
        return;
    }
    if (m_Settings[m_CurrentSetting] == NULL)
    {
        return;
    }
    bInUpdate = true;


    if(m_Spin.GetStyle() & WS_VISIBLE)
    {
        m_Spin.SetRange32(m_Settings[m_CurrentSetting]->GetMin(),m_Settings[m_CurrentSetting]->GetMax());
        m_Spin.SetPos(m_Settings[m_CurrentSetting]->GetValue());
    }

    if(m_Edit.GetStyle() & WS_VISIBLE)
    {
        CString newValue;
	    newValue.Format("%d", m_Settings[m_CurrentSetting]->GetValue());
        m_Edit.SetWindowText(newValue);
    }

    if(m_EditString.GetStyle() & WS_VISIBLE)
    {
        CString newValue;
	    newValue.Format("%s", (char*)(m_Settings[m_CurrentSetting]->GetValue()));
        m_EditString.SetWindowText(newValue);
    }

    if(m_CheckBox.GetStyle() & WS_VISIBLE)
    {
        m_CheckBox.SetCheck(m_Settings[m_CurrentSetting]->GetValue());
        m_CheckBox.SetWindowText(((CSimpleSetting*)m_Settings[m_CurrentSetting])->GetDisplayName());
    }

    if((m_Slider.GetStyle() & WS_VISIBLE) && (m_Settings[m_CurrentSetting]->GetType() == SLIDER))
    {
        //Setting_SetupSlider(m_Settings[m_CurrentSetting], m_Slider.m_hWnd);
        CSliderSetting *pSetting = (CSliderSetting*)m_Settings[m_CurrentSetting];
        pSetting->SetupControl(m_Slider.m_hWnd);
    }

    if((m_Combo.GetStyle() & WS_VISIBLE) && (m_Settings[m_CurrentSetting]->GetType() == ITEMFROMLIST))
    {
        CListSetting *pSetting = (CListSetting*)m_Settings[m_CurrentSetting];
        char **pszList = (char**)pSetting->GetList();

        if(pszList != NULL)
        {
            char szString[128];         //\memory bound assumption
            int count=0;
            bool bListIsSame=false;

            //count the number of items
            for(int i(pSetting->GetMin()); i <= pSetting->GetMax(); ++i)
            {
                //is there any text for this item?
                if ( (pszList[i] != NULL) && (*pszList[i] != '\0') )
                {
                    count++;
                }
            }

            //make sure the number of items is the same
            if (m_Combo.GetCount() == count)
            {
                bListIsSame=true;
                //find the current value in the combo box
                for(int pos(0); pos < m_Combo.GetCount(); pos++)
                {
                    int i=m_Combo.GetItemData(pos);

                    //check item is the same
                    m_Combo.GetLBText(pos, szString);
                    if(strcmp(pszList[i], szString) != 0)
                    {
                        bListIsSame=false;
                        break;
                    }

                    //is this item the current Value?
                    if(i == pSetting->GetValue())
                    {
                        m_Combo.SetCurSel(pos);
                    }
                }
            }

            //is the list the same?
            if (bListIsSame==false)
            {
                //recreate the list
                m_Combo.ResetContent();
                for(int i(pSetting->GetMin()); i <= pSetting->GetMax(); ++i)
                {
                    //is there any text for this item?
                    if ( (pszList[i] != NULL) && (*pszList[i] != '\0') )
                    {
                        int pos=m_Combo.AddString(pszList[i]);

                        //store Value in itemdata
                        m_Combo.SetItemData(pos,i);

                        //is this item the current Value?
                        if(pSetting->GetValue() == i)
                        {
                            m_Combo.SetCurSel(pos);
                        }
                    }
                }
            }
        }
        else
        {
            //clear out the list
            m_Combo.ResetContent();
        }
    }

    bInUpdate = false;
}

void CTreeSettingsGeneric::OnChangeEdit()
{
    if(m_Edit.m_hWnd==NULL)
        return;

    CString Value;
    m_Edit.GetWindowText(Value);

    if (m_Settings[m_CurrentSetting] != NULL)
    {
		m_Settings[m_CurrentSetting]->SetValue(atol(Value));
    }

    UpdateControls();
}

void CTreeSettingsGeneric::OnChangeEditString()
{
    if(m_EditString.m_hWnd==NULL)
        return;

    CString Value;
    m_EditString.GetWindowText(Value);

    if (m_Settings[m_CurrentSetting] != NULL)
    {
	    m_Settings[m_CurrentSetting]->SetValue((long)Value.GetBuffer(255));
    }

    UpdateControls();
}

void CTreeSettingsGeneric::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    //slider has changed

    if ( (m_Settings[m_CurrentSetting] != NULL) && (m_Settings[m_CurrentSetting]->GetType() == SLIDER) )
    {
        CSliderSetting *pSetting = (CSliderSetting*)m_Settings[m_CurrentSetting];
        pSetting->SetFromControl(m_Slider.m_hWnd);
    }

    UpdateControls();
}

void CTreeSettingsGeneric::OnSettingsDefault()
{
    if (m_Settings[m_CurrentSetting] != NULL)
    {
        m_Settings[m_CurrentSetting]->SetDefault();
    }

    UpdateControls();
}

void CTreeSettingsGeneric::OnCheckClick()
{
    if (m_Settings[m_CurrentSetting] != NULL)
    {
        m_Settings[m_CurrentSetting]->SetValue(m_CheckBox.GetCheck());
    }

    UpdateControls();
}

void CTreeSettingsGeneric::OnCheckGlobalClick()
{
	UpdateControls();
}

void CTreeSettingsGeneric::OnCheckSourceClick()
{
	UpdateControls();
}

void CTreeSettingsGeneric::OnCheckVideoInputClick()
{
	UpdateControls();
}

void CTreeSettingsGeneric::OnCheckAudioInputClick()
{
	UpdateControls();
}


void CTreeSettingsGeneric::OnCheckVideoFormatClick()
{
	UpdateControls();
}

void CTreeSettingsGeneric::OnCheckChannelClick()
{
	UpdateControls();
}

void CTreeSettingsGeneric::OnSelchangeChoosefromlist()
{
    if((m_Combo.GetCurSel()!=CB_ERR) && (m_Settings[m_CurrentSetting] != NULL))
    {
        m_Settings[m_CurrentSetting]->SetValue(m_Combo.GetItemData(m_Combo.GetCurSel()));
    }
    UpdateControls();
}

void CTreeSettingsGeneric::OnDeltaposSettingsSpin(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

    if (m_Settings[m_CurrentSetting] != NULL)
    {
        long Value = m_Settings[m_CurrentSetting]->GetValue();

        if(pNMUpDown->iDelta > 0)
        {
            ((CSimpleSetting*)m_Settings[m_CurrentSetting])->Up();
        }
        else
        {
            ((CSimpleSetting*)m_Settings[m_CurrentSetting])->Down();
        }

        *pResult = m_Settings[m_CurrentSetting]->GetValue();
    }

    UpdateControls();
}

void CTreeSettingsGeneric::OnOK()
{
    //WriteSettingsToIni(TRUE);

    //Write settings & clear list
    m_Settings.ClearSettingList(m_DeleteSettingsOnExit, TRUE);
    CTreeSettingsPage::OnOK();
}

void CTreeSettingsGeneric::OnSize(UINT nType, int cx, int cy)
{
    CTreeSettingsPage::OnSize(nType,cx,cy);

    RECT rect;
    int TopBoxBottom = -1;
    int ValueBoxTop = -1;

    CWnd *pTopBox = GetDlgItem(IDC_TREESETTINGS_GENERIC_TOPBOX);
    if (pTopBox != NULL)
    {
        pTopBox->GetWindowRect(&rect);
        ScreenToClient(&rect);
        rect.right = cx;
        pTopBox->MoveWindow(&rect);
        TopBoxBottom = rect.bottom;
    }


    CWnd *pValueBox = GetDlgItem(IDC_TREESETTINGS_GENERIC_VALUEBOX);
    if (pValueBox != NULL)
    {
        int DefaultBtnLeft = 0;
        RECT rcBox;
        RECT rcBoxOrg;

        pValueBox->GetWindowRect(&rect);
        ScreenToClient(&rect);

        rcBoxOrg = rect;

        rect.right = cx;
        rect.top = cy - (rect.bottom-rect.top);
        rect.bottom = cy;
        pValueBox->MoveWindow(&rect);
        ValueBoxTop = rect.top;

        // Items in box
        rcBox = rect;

        // Default button
        CWnd *pDefaultBtn = GetDlgItem(IDC_TREESETTINGS_GENERIC_DEFAULT);
        if (pDefaultBtn != NULL)
        {
            pDefaultBtn->GetWindowRect(&rect);
            ScreenToClient(&rect);
            int Width = (rect.right-rect.left);
            int Height= (rect.bottom-rect.top);

            rect.right = rcBox.right - 5;
            rect.bottom = rcBox.bottom - 5;

            rect.left = rect.right - Width;
            rect.top = rect.bottom - Height;
            pDefaultBtn->MoveWindow(&rect);

            DefaultBtnLeft = rect.left;
        }

        // Check box
        CWnd *pCheckBtn = GetDlgItem(IDC_TREESETTINGS_GENERIC_CHECK);
        if (pCheckBtn != NULL)
        {
            pCheckBtn->GetWindowRect(&rect);
            ScreenToClient(&rect);
            int Height= (rect.bottom-rect.top);
            rect.top = (rect.top - rcBoxOrg.top) + rcBox.top;
            rect.bottom = rect.top + Height;
            rect.right = rcBox.right - 5;
            pCheckBtn->MoveWindow(&rect);
        }

        // Edit box
        CWnd *pEditBox = GetDlgItem(IDC_TREESETTINGS_GENERIC_EDIT);
        if (pEditBox != NULL)
        {
            pEditBox->GetWindowRect(&rect);
            ScreenToClient(&rect);
            int Height= (rect.bottom-rect.top);
            rect.top = (rect.top - rcBoxOrg.top) + rcBox.top;
            rect.bottom = rect.top + Height;
            pEditBox->MoveWindow(&rect);
        }

        // Spin
        CWnd *pSpinControl = GetDlgItem(IDC_TREESETTINGS_GENERIC_SPIN);
        if (pSpinControl != NULL)
        {
            pSpinControl->GetWindowRect(&rect);
            ScreenToClient(&rect);
            int Height= (rect.bottom-rect.top);
            rect.top = (rect.top - rcBoxOrg.top) + rcBox.top;
            rect.bottom = rect.top + Height;
            pSpinControl->MoveWindow(&rect);
        }

        //Slider
        CWnd *pSlider = GetDlgItem(IDC_TREESETTINGS_GENERIC_SLIDER);
        if (pSlider != NULL)
        {
            pSlider->GetWindowRect(&rect);
            ScreenToClient(&rect);
            int Height= (rect.bottom-rect.top);
            if (DefaultBtnLeft > 10)
            {
                rect.right = DefaultBtnLeft - 10;
            }
            rect.top = (rect.top - rcBoxOrg.top) + rcBox.top;
            rect.bottom = rect.top + Height;
            pSlider->MoveWindow(&rect);
        }

        //Combo box
        CWnd *pChooseFromList = GetDlgItem(IDC_TREESETTINGS_GENERIC_CHOOSEFROMLIST);
        if (pChooseFromList != NULL)
        {
            pChooseFromList->GetWindowRect(&rect);
            ScreenToClient(&rect);
            int Height= (rect.bottom-rect.top);
            if (DefaultBtnLeft > 10)
            {
                rect.right = DefaultBtnLeft - 10;
            }
            rect.top = (rect.top - rcBoxOrg.top) + rcBox.top;
            rect.bottom = rect.top + Height;
            pChooseFromList->MoveWindow(&rect);
        }
    }


    CWnd *pList = GetDlgItem(IDC_TREESETTINGS_GENERIC_LIST);
    if (pList != NULL)
    {
        pList->GetWindowRect(&rect);
        ScreenToClient(&rect);
        rect.right = cx;
        if (ValueBoxTop > rect.top)
        {
           rect.bottom = ValueBoxTop;
        }
        pList->MoveWindow(&rect);
    }
    Invalidate();

}
