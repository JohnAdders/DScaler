/////////////////////////////////////////////////////////////////////////////
// $Id: TreeSettingsGeneric.cpp,v 1.5 2002-10-15 15:04:01 kooiman Exp $
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
    m_Settings((SETTINGHOLDERID)0),
    m_DeleteSettingsOnExit(FALSE)
{
    //{{AFX_DATA_INIT(CTreeSettingsGeneric)
    //}}AFX_DATA_INIT
    
    m_SettingsCount = count;
    m_Settings.LoadSettingStructures(settings,0,m_SettingsCount);    
    m_DeleteSettingsOnExit = TRUE;
}

CTreeSettingsGeneric::CTreeSettingsGeneric(CString name,vector<CSimpleSetting*> csettings)
	:CTreeSettingsPage(name,CTreeSettingsGeneric::IDD),
	m_CurrentSetting(0),
    m_Settings((SETTINGHOLDERID)0),
    m_DeleteSettingsOnExit(FALSE)
{  
    m_SettingsCount = csettings.size();      
    for (int i = 0; i < m_SettingsCount; i++)
    {
        m_Settings.AddSetting(csettings[i]);
    }  
}

CTreeSettingsGeneric::CTreeSettingsGeneric(CString name,SETTINGEX* settings,long count)
	:CTreeSettingsPage(name,CTreeSettingsGeneric::IDD),
	m_CurrentSetting(0),
    m_Settings((SETTINGHOLDERID)0),
    m_DeleteSettingsOnExit(FALSE)
{
    //{{AFX_DATA_INIT(CTreeSettingsGeneric)
    //}}AFX_DATA_INIT
    
    CSettingGroupList LocalGroupList;

    m_SettingsCount = count;
    m_Settings.LoadSettingStructuresEx(settings,0,m_SettingsCount,&LocalGroupList);        
    for (int i = 0; i < m_SettingsCount; i++)
    {
        if (m_Settings[i] != NULL)
        {
            m_Settings[i]->SetGroup(NULL);
        }
    }
    m_DeleteSettingsOnExit = TRUE;
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
        m_CheckBox.ShowWindow(SW_HIDE);
        m_Combo.ShowWindow(SW_HIDE);
        break;

    case ITEMFROMLIST:
        m_CheckBox.ShowWindow(SW_HIDE);
        m_DefaultButton.ShowWindow(SW_SHOWNA);
        m_Edit.ShowWindow(SW_HIDE);
        m_Slider.ShowWindow(SW_HIDE);
        m_Spin.ShowWindow(SW_HIDE);
        m_Combo.ShowWindow(SW_SHOWNA);
        break;

    default:
        m_CheckBox.ShowWindow(SW_HIDE);
        m_DefaultButton.ShowWindow(SW_HIDE);
        m_Edit.ShowWindow(SW_HIDE);
        m_Slider.ShowWindow(SW_HIDE);
        m_Spin.ShowWindow(SW_HIDE);
        m_Combo.ShowWindow(SW_HIDE);
    }

	BOOL bShowBoxes = FALSE;
	if ((m_Settings[m_CurrentSetting] != NULL) && (m_Settings[m_CurrentSetting]->GetFlags() != 0))
    {
		bShowBoxes = TRUE;	
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
		m_SavePerInfoBox.ShowWindow(SW_SHOW);
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
        
        m_Combo.ResetContent();
        if(pszList != NULL)
        {
            bool bFoundSetting=false;
            
            for(int i(pSetting->GetMin()); i <= pSetting->GetMax(); ++i)
            {                
                //is there any text for this item?
                if ( (pszList[i] != NULL) && (strlen(pszList[i])>0) )
                {
                    int pos=m_Combo.AddString(pszList[i]);

                    //store Value in itemdata
                    m_Combo.SetItemData(pos,i);

                    //is this item the current Value?
                    if(pSetting->GetValue() == i)
                    {
                        m_Combo.SetCurSel(pos);
                        bFoundSetting=true;
                    }
                }
            }
            if(bFoundSetting==false)
            {
                //clear selection since we didnt find pValue
                m_Combo.SetCurSel(-1);
            }
        }
    }
	
	if (m_Settings[m_CurrentSetting]->GetFlags() != 0)
	{	
		long Flags = m_Settings[m_CurrentSetting]->GetFlags();
		m_CheckGlobalBox.SetCheck(((Flags & SETTINGFLAG_GLOBAL)!=0));
		m_CheckSourceBox.SetCheck(((Flags & SETTINGFLAG_PER_SOURCE)!=0));
		m_CheckVideoInputBox.SetCheck(((Flags & SETTINGFLAG_PER_VIDEOINPUT)!=0));
		m_CheckAudioInputBox.SetCheck(((Flags & SETTINGFLAG_PER_AUDIOINPUT)!=0));
		m_CheckVideoFormatBox.SetCheck(((Flags & SETTINGFLAG_PER_VIDEOFORMAT)!=0));
		m_CheckChannelBox.SetCheck(((Flags & SETTINGFLAG_PER_CHANNEL)!=0));			        

		//Till it works:
		Flags&=~SETTINGFLAG_ALLOW_MASK;

		m_CheckGlobalBox.EnableWindow(((Flags & SETTINGFLAG_ALLOW_GLOBAL)!=0));
		m_CheckSourceBox.EnableWindow(((Flags & SETTINGFLAG_ALLOW_PER_SOURCE)!=0));
		m_CheckVideoInputBox.EnableWindow(((Flags & SETTINGFLAG_ALLOW_PER_VIDEOINPUT)!=0));
		m_CheckAudioInputBox.EnableWindow(((Flags & SETTINGFLAG_ALLOW_PER_AUDIOINPUT)!=0));
		m_CheckVideoFormatBox.EnableWindow(((Flags & SETTINGFLAG_ALLOW_PER_VIDEOFORMAT)!=0));
		m_CheckChannelBox.EnableWindow(((Flags & SETTINGFLAG_ALLOW_PER_CHANNEL)!=0));	
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
	if ( (m_Settings[m_CurrentSetting] != NULL) && (m_Settings[m_CurrentSetting]->GetFlags() != 0) )
	{
		long Flags = m_Settings[m_CurrentSetting]->GetFlags();
        if (Flags & SETTINGFLAG_ALLOW_GLOBAL)
        {
            m_Settings[m_CurrentSetting]->SetFlag(SETTINGFLAG_GLOBAL, (Flags&SETTINGFLAG_GLOBAL) ? FALSE : TRUE);		
        }
	}
	UpdateControls();
}

void CTreeSettingsGeneric::OnCheckSourceClick()
{
	if ( (m_Settings[m_CurrentSetting] != NULL) && (m_Settings[m_CurrentSetting]->GetFlags() != 0) )
	{
		long Flags = m_Settings[m_CurrentSetting]->GetFlags();
        if (Flags & SETTINGFLAG_ALLOW_PER_SOURCE)
        {
            m_Settings[m_CurrentSetting]->SetFlag(SETTINGFLAG_PER_SOURCE, (Flags&SETTINGFLAG_PER_SOURCE) ? FALSE : TRUE);
        }
	}
	UpdateControls();
}

void CTreeSettingsGeneric::OnCheckVideoInputClick()
{
	if ( (m_Settings[m_CurrentSetting] != NULL) && (m_Settings[m_CurrentSetting]->GetFlags() != 0) )
	{
        long Flags = m_Settings[m_CurrentSetting]->GetFlags();
        if (Flags & SETTINGFLAG_ALLOW_PER_VIDEOINPUT)
        {
            m_Settings[m_CurrentSetting]->SetFlag(SETTINGFLAG_PER_VIDEOINPUT, (Flags&SETTINGFLAG_PER_VIDEOINPUT) ? FALSE : TRUE);		
        }
	}
	UpdateControls();
}

void CTreeSettingsGeneric::OnCheckAudioInputClick()
{
	if ( (m_Settings[m_CurrentSetting] != NULL) && (m_Settings[m_CurrentSetting]->GetFlags() != 0) )
	{
		long Flags = m_Settings[m_CurrentSetting]->GetFlags();
        if (Flags & SETTINGFLAG_ALLOW_PER_AUDIOINPUT)
        {
            m_Settings[m_CurrentSetting]->SetFlag(SETTINGFLAG_PER_AUDIOINPUT, (Flags&SETTINGFLAG_PER_AUDIOINPUT) ? FALSE : TRUE);		
        }
	}
	UpdateControls();
}


void CTreeSettingsGeneric::OnCheckVideoFormatClick()
{
	if ( (m_Settings[m_CurrentSetting] != NULL) && (m_Settings[m_CurrentSetting]->GetFlags() != 0) )
	{
		long Flags = m_Settings[m_CurrentSetting]->GetFlags();
        if (Flags & SETTINGFLAG_ALLOW_PER_VIDEOFORMAT)
        {
            m_Settings[m_CurrentSetting]->SetFlag(SETTINGFLAG_PER_VIDEOFORMAT, (Flags&SETTINGFLAG_PER_VIDEOFORMAT) ? FALSE : TRUE);		
        }
	}
	UpdateControls();
}

void CTreeSettingsGeneric::OnCheckChannelClick()
{
	if (m_Settings[m_CurrentSetting]->GetFlags() != 0)
	{
		long Flags = m_Settings[m_CurrentSetting]->GetFlags();
        if (Flags & SETTINGFLAG_ALLOW_PER_CHANNEL)
        {
            m_Settings[m_CurrentSetting]->SetFlag(SETTINGFLAG_PER_CHANNEL, (Flags&SETTINGFLAG_PER_CHANNEL) ? FALSE : TRUE);		
        }
	}
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
    RECT rect;
    int TopBoxBottom = -1;
    int ValueBoxTop = -1;

    CWnd *pTopBox = GetDlgItem(IDC_TREESETTINGS_GENERIC_TOPBOX); 
    if (pTopBox != NULL)
    {
        pTopBox->GetWindowRect(&rect);
        ScreenToClient(&rect);
        rect.right = cx;        
        pTopBox->MoveWindow(&rect,FALSE);        
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
        pValueBox->MoveWindow(&rect,FALSE);
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
            pDefaultBtn->MoveWindow(&rect,FALSE);

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
            pCheckBtn->MoveWindow(&rect,FALSE);
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
            pEditBox->MoveWindow(&rect,FALSE);
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
            pSpinControl->MoveWindow(&rect,FALSE);
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
            pSlider->MoveWindow(&rect,FALSE);
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
            pChooseFromList->MoveWindow(&rect,FALSE);
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
        pList->MoveWindow(&rect,FALSE);
    }
    
}
