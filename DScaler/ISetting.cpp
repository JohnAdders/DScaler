#include "stdafx.h"

#include "ISetting.h"

/** CSettingGroup class.
    Hold information like name & description.
    Used for structured lists in the GUI
*/
CSettingGroup::CSettingGroup(CSettingGroupList *pGroupList, LPCSTR szGroupName, LPCSTR szLongName, LPCSTR szInfoText, CSettingObject* pObject)
{
    m_pGroupList = pGroupList;
    m_pObject = pObject;

    m_sGroupName = (szGroupName == NULL) ? "" : szGroupName; 
    m_sDisplayName = (szLongName == NULL) ? "" : szLongName; 
    m_sInfoText = (szInfoText == NULL) ? "" : szInfoText; 		    
}

CSettingGroup::~CSettingGroup()
{
}

/** Set display name
*/
void CSettingGroup::SetLongName(LPCSTR szLongName)
{
    m_sDisplayName = (szLongName == NULL) ? "" : szLongName;
}

/** Set info text
*/
void CSettingGroup::SetInfoText(LPCSTR szInfoText)
{
    m_sInfoText = (szInfoText == NULL) ? "" : szInfoText; 		    
}

LPCSTR CSettingGroup::GetName() 
{
    return m_sGroupName.c_str();
}

LPCSTR CSettingGroup::GetLongName()
{
    return m_sDisplayName.c_str();
}

LPCSTR CSettingGroup::GetInfoText()
{
	return m_sInfoText.c_str();
}


/** 
    Adds group to pSetting    
*/
void CSettingGroup::AddSetting(ISetting *pSetting)
{
    pSetting->SetGroup(this);    
}

/** 
    Get a subgroup from this group using the grouplist
*/
CSettingGroup *CSettingGroup::GetGroup(LPCSTR szGroupName, LPCSTR szLongName, LPCSTR szInfoText)
{
    if (m_pGroupList != NULL)
    {
        return m_pGroupList->GetSubGroup(this, szGroupName, szLongName, szInfoText);
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////////
/// Group list ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////


CSettingGroupList::CSettingGroupList()
{    
	m_GroupList.pGroup = NULL;
}

CSettingGroupList::~CSettingGroupList()
{    
	Clear();
}


///////////////////////////////////////////////////////////////////////////////////

/** Recurse through (sub)tree. Delete groups on the way
*/
void CSettingGroupList::DeleteGroupsRecursive(CSettingGroupList::TSubGroupInfo* pGroupList)
{
	for (int i = 0; i < pGroupList->vSubGroups.size(); i++)
	{
		DeleteGroupsRecursive(&pGroupList->vSubGroups[i]);		
	}	
	pGroupList->vSubGroups.clear();

    if (pGroupList->pGroup != NULL) 
	{ 
        delete pGroupList->pGroup;
		pGroupList->pGroup = NULL;
		return;
	}	
}

/** Recurse through tree(sub). 
    Delete groups on the way
*/
CSettingGroupList::TSubGroupInfo* CSettingGroupList::FindAndCreateRecursive(TSubGroupInfo* pGroupList, CSettingObject* pObject, char** pszList, char** pszDisplayNameList, char** pszTooltips)
{        
    //End of list / error
    if ((pszList == NULL) || (*pszList==NULL) || (**pszList == 0) || (pGroupList == NULL))
    {
        return NULL;
    }    

    //Find group in list
    int i = 0;
    for (i = 0; i < pGroupList->vSubGroups.size(); i++)
    {
        CSettingGroup *pSubGroup = pGroupList->vSubGroups[i].pGroup;
        if ((pSubGroup != NULL) && ((pObject==NULL) || (pSubGroup->GetObject() == pObject)) && !strcmp(pSubGroup->GetName(),*pszList)) 
        {
            break;
        }
    }
    if (i >= pGroupList->vSubGroups.size())
    {
        //Not found. Create new group
        CSettingGroup *pGroup = new CSettingGroup(this,
                 *pszList, 
                 (pszDisplayNameList == NULL) ? NULL : *pszDisplayNameList, 
                 (pszTooltips == NULL) ? NULL : *pszTooltips, 
                 pObject);

        TSubGroupInfo sgi;
	    sgi.pGroup = pGroup;				
	    pGroupList->vSubGroups.push_back(sgi);	    
    }

    //Shift lists
    pszList= &pszList[1];
    pszDisplayNameList = (pszDisplayNameList == NULL) ? NULL : &pszDisplayNameList[1];
    pszTooltips = (pszTooltips == NULL) ? NULL : &pszTooltips[1];

    //End of list, return
    if ((*pszList==NULL) || (**pszList == 0))
    {
        return &pGroupList->vSubGroups[i];
    }    

    return FindAndCreateRecursive(&pGroupList->vSubGroups[i], pObject, pszList, pszDisplayNameList, pszTooltips);
}

/** Recurse through tree(sub). 
    Find the group 'pGroup'
*/
CSettingGroupList::TSubGroupInfo* CSettingGroupList::FindGroupRecursive(CSettingGroupList::TSubGroupInfo* pGroupList, CSettingGroup* pGroup)
{
	if (pGroupList->pGroup == pGroup) { return pGroupList; }
	TSubGroupInfo* pSubGroupInfo = NULL;
	for (int i = 0; i < pGroupList->vSubGroups.size(); i++)
	{
		pSubGroupInfo = FindGroupRecursive(&pGroupList->vSubGroups[i], pGroup);
		if (pSubGroupInfo != NULL)
		{
			return pSubGroupInfo;
		}	
	}
	return pSubGroupInfo;
}

/** Recurse through tree(sub). 
    Find the group of object 'pObject' and group and subgroup names in list pszGroupNames
*/
CSettingGroupList::TSubGroupInfo* CSettingGroupList::FindGroupRecursive(CSettingGroupList::TSubGroupInfo* pGroupList, CSettingObject *pObject, char** pszGroupNames)
{
	if (pszGroupNames == NULL)
    {
        return NULL;
    }
    
    if (pGroupList != &m_GroupList)
    {
        BOOL bNextSubGroup = FALSE;
        if ((pGroupList->pGroup != NULL) && (strcmp(pGroupList->pGroup->GetName(),*pszGroupNames)==0))
        {
            if ((pObject==NULL) || (pGroupList->pGroup->GetObject() == pObject))
            {
                bNextSubGroup = TRUE; 
            }
        }
                
        if (bNextSubGroup)
        {
            pszGroupNames = &pszGroupNames[1];
            if (*pszGroupNames == NULL)
            {
                return pGroupList;
            }
        }
        else
        {
            return NULL;
        }
    }

	TSubGroupInfo* pSubGroupInfo = NULL;
	for (int i = 0; i < pGroupList->vSubGroups.size(); i++)
	{		        
        pSubGroupInfo = FindGroupRecursive(&pGroupList->vSubGroups[i], pObject, pszGroupNames);
		if (pSubGroupInfo != NULL)
		{
			return pSubGroupInfo;
		}	
	}
	return pSubGroupInfo;
}


///////////////////////////////////////////////////////////////////////////////////

/** Delete entire group list
*/
void CSettingGroupList::Clear()
{
	DeleteGroupsRecursive(&m_GroupList);
	m_GroupList.vSubGroups.clear();
}

/** Get group at the end of the pszGroupList list.
    Create groups when necessary
*/
CSettingGroup* CSettingGroupList::GetGroup(CSettingObject* pObject, char** pszGroupList, char** pszDisplayNameList, char** pszTooltip)
{        
    TSubGroupInfo* pSubGroupInfo = FindAndCreateRecursive(&m_GroupList, pObject, pszGroupList, pszDisplayNameList, pszTooltip);
    if (pSubGroupInfo == NULL)
    {
        return NULL;
    }
    return pSubGroupInfo->pGroup;
}

/** Get root group.
    Create group when necessary
*/
CSettingGroup* CSettingGroupList::GetRootGroup(CSettingObject* pObject, LPCSTR szGroupName, LPCSTR szDisplayName, LPCSTR szTooltip)
{
	char* pszGroupList[2];
	char* pszDisplayName[2];
	char* pszTooltip[2];
	
	pszGroupList[0] = (char*)szGroupName;
    pszGroupList[1] = NULL;

	pszDisplayName[0] = (char*)szDisplayName;
    pszDisplayName[1] = NULL;
	
    pszTooltip[0] = (char*)szTooltip;	
	pszTooltip[1] = NULL;

	return GetGroup(pObject, (char**)pszGroupList, (char**)pszDisplayName, (char**)pszTooltip);
}

/** Get sub group.
    Finds group pMainGroup and 
    creates group when necessary
*/
CSettingGroup* CSettingGroupList::GetSubGroup(CSettingGroup* pMainGroup, LPCSTR szSubGroup, LPCSTR szDisplayName, LPCSTR szTooltip)
{
	TSubGroupInfo* pSubGroupInfo = FindGroupRecursive(&m_GroupList, pMainGroup);
	CSettingGroup* pGroup = NULL;

	if (pSubGroupInfo != NULL)
	{
		pGroup = new CSettingGroup(this, szSubGroup, szDisplayName, szTooltip, pSubGroupInfo->pGroup->GetObject());
		TSubGroupInfo sgi;
		sgi.pGroup = pGroup;				
		pSubGroupInfo->vSubGroups.push_back(sgi);
	}
	
	return pGroup;	
}


/** Find a group in the group list    
*/
CSettingGroup* CSettingGroupList::FindGroup(CSettingObject* pObject, char** pszGroupList)
{
    if (pszGroupList == NULL)
    {
        return 0;
    }
    
    TSubGroupInfo* pSubGroupInfo = FindGroupRecursive(&m_GroupList, pObject, pszGroupList);
	
	if (pSubGroupInfo != NULL)
	{
		return pSubGroupInfo->pGroup;
	}
	
	return NULL;
}

/** Find a group at the end of the tree indicated with Index[depth].
    The 'Index' list is terminated with a value less than 0.
*/
CSettingGroup* CSettingGroupList::Find(int* Index)
{
	TSubGroupInfo* pSubGroupInfo = &m_GroupList;		
	if (Index == NULL) { return NULL; }
	for (;;)
	{
	   if (*Index < 0)
	   {
		   return pSubGroupInfo->pGroup;
	   }
	   else
	   {
			if (*Index >= pSubGroupInfo->vSubGroups.size())
			{
				return NULL;
			}
			else
			{
				pSubGroupInfo = &pSubGroupInfo->vSubGroups[*Index];
			}
	   }
	   Index++;
	}
}

/** Get te number of subgroups in the group the end of the tree indicated with Index[depth].
    The 'Index' list is terminated with a value less than 0.
*/
int CSettingGroupList::NumGroups(int* Index)
{
	TSubGroupInfo* pSubGroupInfo = &m_GroupList;		
	if (Index == NULL) { return 0; }
	for (;;)
	{
	   if (*Index < 0)
	   {
		   return pSubGroupInfo->vSubGroups.size();
	   }
	   else
	   {
			if (*Index >= pSubGroupInfo->vSubGroups.size())
			{
				return 0;
			}
			else
			{
				pSubGroupInfo = &pSubGroupInfo->vSubGroups[*Index];
			}
	   }
	   Index++;
	}	
}


