//
// $Id: Bitmap.cpp,v 1.1 2002-09-25 22:31:48 kooiman Exp $
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Jeroen Kooiman.  All rights reserved.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: not supported by cvs2svn $
// Revision 1.0  2001/11/25 02:03:21  kooiman
// initial version
//
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Bitmap.h"
#include "DebugLog.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


////////////////////////////////////////////////////////////////////////////////
// Bitmap holder ///////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CBitmapHolder::CBitmapHolder(int DrawMode)
{
    this->DrawMode= DrawMode; //0=normal, 1=stretch to fit, 2=tile        
}

CBitmapHolder::~CBitmapHolder()
{
    for (int State = 0; State < States.size(); State++)
    {
        FreeState(State);
    }
    States.clear();
}

int CBitmapHolder::Width(int State)
{
    if (States.size()<=State) { return 0; }
    if (States[State].BmpWidth<0)
    {
       BITMAP bm;
	   ::GetObject (States[State].hBmp, sizeof (bm), & bm);
       States[State].BmpWidth = bm.bmWidth;
    }
    return States[State].BmpWidth;
}

int CBitmapHolder::Height(int State)
{
    if (States.size()<=State) { return 0; }
    if (States[State].BmpHeight<0)
    {
       BITMAP bm;
	   ::GetObject (States[State].hBmp, sizeof (bm), & bm);
       States[State].BmpHeight = bm.bmHeight;
    }
    return States[State].BmpHeight;
}

void CBitmapHolder::FreeState(int State)
{
    for (int i = 0; i < States[State].RegionList.size(); i++)
    {
        if (States[State].RegionList[i] != NULL) { delete States[State].RegionList[i]; }
    }
    States[State].RegionList.clear();
    States[State].hRegion = NULL;
    if (States[State].bDeleteBitmapOnExit)
    {
        if (States[State].hBmp != NULL) 
        {
            ::DeleteObject(States[State].hBmp);
        }
        if (States[State].hBmpMask != NULL) 
        {
            ::DeleteObject(States[State].hBmpMask);
        }    
    }
    States[State].hBmp = NULL;
    States[State].hBmpMask = NULL;
    States[State].BmpWidth = -1;
    States[State].BmpHeight = -1;
}

BOOL CBitmapHolder::Add(HBITMAP hBmp, HBITMAP hBmpMask, int State, BOOL bDeleteBitmapOnExit)
{
   if (State<0) {  return FALSE; }
   while (State >= States.size())
   {
      TState NewState;
      NewState.hBmp = NULL;
      NewState.hBmpMask = NULL;
      NewState.BmpWidth = -1;
      NewState.BmpHeight = -1; 
      NewState.hRegion = NULL;
      NewState.bDeleteBitmapOnExit = bDeleteBitmapOnExit;
      
      States.push_back(NewState);      
   }
   FreeState(State);
   
   States[State].hBmp = hBmp;
   States[State].hBmpMask = hBmpMask;
   States[State].BmpWidth = -1;
   States[State].BmpWidth = -1;
   return TRUE;
}

HBITMAP CBitmapHolder::GetBitmap(int State)
{
    if ((State>=0) && (State<States.size()))
    {
        return States[State].hBmp;
    }
    return NULL;
}

HBITMAP CBitmapHolder::GetBitmapMask(int State)
{
    if ((State>=0) && (State<States.size()))
    {
        return States[State].hBmpMask;
    }
    return NULL;
}

void CBitmapHolder::Draw(HDC hDC, POINT *pBmpStart, LPRECT r, int State)
{
    if ((State<0) || (State>=States.size()))
    {
        return;
    }
    if ((States[State].hBmp==NULL) || (Width(State)<=0) || (Height(State)<=0))
    {
        return;
    }
    RECT rc;
    if (r == NULL)
    {
        if (pBmpStart == NULL)
        {
            ::SetRect(&rc,0,0,Width(State), Height(State));
        }
        else
        {
            ::SetRect(&rc,0,0,Width(State)-pBmpStart->x, Height(State)-pBmpStart->y);
        }
        r = &rc;
    }
    BitmapDraw(hDC, States[State].hBmp, NULL /*States[State].hBmpMask*/, pBmpStart, r,DrawMode);
}


vector<LPRECT> *CBitmapHolder::GetRegionList(int State)
{
    if (State>=States.size())
    {
        return NULL;
    }
    
    if (States[State].RegionList.size()==0) 
    { 
        if (States[State].hBmpMask != NULL)
        {
            BitmapToRegionList(States[State].hBmpMask, (&States[State].RegionList));
        }    
    } 
    return &(States[State].RegionList);
}

HRGN CBitmapHolder::GetWindowRegion(int State) 
{ 
    vector<LPRECT> *RegionList = GetRegionList(State);
    if (RegionList == NULL) { return NULL; }

    if (States[State].hRegion == NULL) 
    { 
        RECT rc;
        rc.left = 0;
        rc.top = 0;
        rc.right = Width(State);
        rc.bottom = Height(State);
        States[State].hRegion = CreateWindowRegion(&rc,&States[State].RegionList,NULL);
    }
    return States[State].hRegion;
}




BOOL CBitmapHolder::BitmapToRegionList(HBITMAP hBmpMask, vector<LPRECT> *RegionList)
{
    BITMAP bm;
    ::GetObject(hBmpMask, sizeof(bm), &bm);

    HDC hDC =  ::CreateCompatibleDC(NULL);
    HBITMAP h = (HBITMAP)::SelectObject(hDC, hBmpMask);

    for (int y = 0; y < bm.bmHeight; y++)
    {
        // Scan each bitmap pixel from left to right
        for (int x = 0; x < bm.bmWidth; x++)
        {
            // Search for a continuous range of "non transparent pixels"
            int x0 = x;							
            while (x < bm.bmWidth)
            {
                if (GetPixel(hDC,x,y) == 0x00FFFFFF) 
                    break;                								
                x++;
            }

            if (x > x0)
            {
                // Add the pixels (x0, y) to (x, y+1) as a new rectangle in the region
                LPRECT pRc = new RECT;
                pRc->left=x0; 
                pRc->top=y;
                pRc->right=x;
                pRc->bottom=y+1;
                (*RegionList).push_back(pRc);
            }
        }

        // Go to next row (remember, the bitmap is inverted vertically)						
    }

    ::SelectObject(hDC, h);
    ::DeleteDC(hDC);
    ::DeleteObject(h);

    return TRUE;
}


HRGN CBitmapHolder::CreateWindowRegion(RECT *rcBound, vector<LPRECT> *RegionList, POINT *pPosition)
{
    HRGN hRgn = NULL;
    
    int NumRects = RegionList->size();
    int Num;

    if (NumRects == 0) return NULL;

    do {
        Num = 2000; //win98 gets confused if number of rects gets large
        if (NumRects < Num) { Num = NumRects; }

        HANDLE hData = ::GlobalAlloc(GMEM_MOVEABLE, sizeof(RGNDATAHEADER) + (sizeof(RECT) * Num));
        RGNDATA *pData = (RGNDATA *)::GlobalLock(hData);
        pData->rdh.dwSize = sizeof(RGNDATAHEADER);
        pData->rdh.iType = RDH_RECTANGLES;
        pData->rdh.nCount = Num;
        pData->rdh.nRgnSize = 0;
        pData->rdh.rcBound = *rcBound;
        //::SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);

        RECT *prDest = (RECT *)&pData->Buffer;
        for (int i = 0; i < Num; i++)
        {
            RECT *prSource = (*RegionList)[i];
            memcpy(prDest,prSource,sizeof(RECT));
            if (pPosition!=NULL)
            {
                prDest->left+=pPosition->x;
                prDest->right+=pPosition->x;
                prDest->top+=pPosition->y;
                prDest->bottom+=pPosition->y;
            }
            prDest++;
        }

        HRGN h = ::ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * Num), pData);
        
        if (hRgn != NULL)
    		{       
		      	::CombineRgn(hRgn, hRgn, h, RGN_OR);
				    ::DeleteObject(h);
		    }
		    else
				{
            hRgn = h;
        }

        ::GlobalFree(hData);

        NumRects -= Num;
    } while (NumRects>0);
    
    return hRgn;
}


void CBitmapHolder::BitmapDrawTiled(HDC hDC, HBITMAP hbmp, POINT *bmstart, LPRECT r)
{
    if(!hbmp) return;
    HDC memdc;
    memdc = ::CreateCompatibleDC(hDC);
    ::SelectObject(memdc,hbmp);
    int startbmx = 0;
    int startbmy = 0;
    int startx;  
    int w = r->right - r->left;
    int	h = r->bottom - r->top;
    int x,y,z;

    BITMAP bm;
    ::GetObject (hbmp, sizeof (bm), & bm);
    int bx = bm.bmWidth;
    int by = bm.bmHeight;

    if (bmstart!=NULL)
    {
        startbmx = bmstart->x;
        startbmy = bmstart->y;   
        while (startbmx>=bx) { startbmx-=bx; }
        while (startbmy>=by) { startbmy-=by; }
    }

    for (y = 0; y < h ; y += by){
        if ((y+by)>h) by=h-y;		
        startx=startbmx;
        z=bx-startx;
        for (x = 0 ; x < w ; x += z){ 
            if ((x+z)>w) {
                z=w-x;      
                BitBlt(hDC, r->left + x, r->top + y, z, by, memdc, startx, startbmy, SRCCOPY); 
                //LOG(2,"DrawBM: (%d,%d,%dx%d) (bm: %d,%d)",r->left+x,r->top+y,z,by,startx,startbmy);
            } else {
                BitBlt(hDC, r->left + x, r->top + y, z, by, memdc, startx, startbmy, SRCCOPY);      
                //LOG(2,"DrawBM: (%d,%d,%dx%d) (bm: %d,%d)",r->left+x,r->top+y,z,by,startx,startbmy);
            }
            startx = 0;
        }
        startbmy = 0;
    }
    ::DeleteDC(memdc);
}
  
void CBitmapHolder::BitmapDraw(HDC hDC, HBITMAP hbmp, HBITMAP hmask, POINT * bmstart, LPRECT r, int DrawMode)
{
    //LOG(2,"DrawBM: to (%d,%d,%dx%d) mode %d",r->left,r->top,r->right-r->left,r->bottom-r->top,DrawMode);
    hmask=NULL;

    //	DrawMode: 0=Normal; 1=stretch; 2=tiled fill
    if(DrawMode==2){
        BitmapDrawTiled(hDC,hbmp,bmstart,r);
        return;
    }
    if(!hbmp) return;	//safe check

    int startbmx = 0;
    int startbmy = 0;
    if (bmstart!=NULL)
    {
        startbmx = bmstart->x;
        startbmy = bmstart->y;      
    }

    int cx=r->right  - r->left;
    int cy=r->bottom - r->top;
    HDC dcBmp, dcMask;
    dcBmp = ::CreateCompatibleDC(hDC);
    ::SelectObject(dcBmp, hbmp);

    if (hmask!=NULL){
        dcMask = ::CreateCompatibleDC(hDC);
        ::SelectObject(dcMask, hmask);

        HDC hdcMem;
        hdcMem = ::CreateCompatibleDC(hDC);
        HBITMAP hBitmap;
        
        hBitmap = ::CreateCompatibleBitmap(hDC,cx,cy);
        ::SelectObject(hdcMem, hBitmap);

        BitBlt(hdcMem,r->left,r->top,cx,cy,hDC,startbmx,startbmy,SRCCOPY);
        if(!DrawMode){
            BitBlt(hdcMem,r->left,r->top,cx,cy,dcBmp,startbmx,startbmy,SRCINVERT);
            BitBlt(hdcMem,r->left,r->top,cx,cy,dcMask,startbmx,startbmy,SRCAND);
            BitBlt(hdcMem,r->left,r->top,cx,cy,dcBmp,startbmx,startbmy,SRCINVERT);      
        } else {
            BITMAP bm;
            ::GetObject (hbmp, sizeof (bm), & bm);
            int bx = bm.bmWidth;
            int by = bm.bmHeight;

            //todo: correct startbmy/y and bx/by

            StretchBlt(hdcMem,r->left,r->top,cx,cy,dcBmp,startbmx,startbmy,bx,by,SRCINVERT);
            StretchBlt(hdcMem,r->left,r->top,cx,cy,dcMask,startbmx,startbmy,bx,by,SRCAND);
            StretchBlt(hdcMem,r->left,r->top,cx,cy,dcBmp,startbmx,startbmy,bx,by,SRCINVERT);
        }

        BitBlt(hDC, r->left,r->top,cx,cy,hdcMem,0,0,SRCCOPY);

        ::DeleteDC(hdcMem);
        ::DeleteObject(hBitmap);

        ::DeleteDC(dcMask);
    } else {
        //LOG(2,"DrawBM: to (%d,%d,%dx%d) (bm %d,%d)",r->left,r->top,cx,cy,startbmx,startbmy);
        if(!DrawMode){
            BitBlt(hDC, r->left,r->top,cx,cy,dcBmp,startbmx,startbmy,SRCCOPY);
        } else {
            BITMAP bm;
            ::GetObject (hbmp, sizeof (bm), & bm);
            int bx = bm.bmWidth;
            int by = bm.bmHeight;
            StretchBlt(hDC, r->left,r->top,cx,cy,dcBmp,startbmx,startbmy,bx,by,SRCCOPY);
        }
    }
    ::DeleteDC(dcBmp);
}

HBITMAP CBitmapHolder::BitmapCopyPieceRGB(HDC hdestDC, HDC hsrcDC, LPRECT lpRect)
{
    //HBITMAP hNewBmp = ::CreateCompatibleBitmap(hdestDC,rc.right-rc.left, rc.bottom-rc.top);
    BITMAPINFOHEADER bmi = {
        sizeof(BITMAPINFOHEADER),
        lpRect->right-lpRect->left,
        lpRect->bottom-lpRect->top,
        1, 32, BI_RGB, 0,0,0,0, 0};                    
    void *pvBits;

    HBITMAP hNewBmp = ::CreateDIBSection(hdestDC,(BITMAPINFO *)&bmi,DIB_RGB_COLORS,&pvBits,NULL,0);

    ::SelectObject(hdestDC, hNewBmp);                        
    ::BitBlt(hdestDC, 0,0, lpRect->right-lpRect->left, lpRect->bottom-lpRect->top, hsrcDC, lpRect->left, lpRect->top, SRCCOPY);    

    return hNewBmp;
}


///\todo: Doesn't work? fix it
// load *.bmp, *.jpg, *.gif, *.ico, *.emf, or *.wmf files
HBITMAP CBitmapHolder::BitmapLoadFromFile(const char *szFile)
{
    HBITMAP ret = NULL;

    LPPICTURE gpPicture;
    // open file
    HANDLE hFile = CreateFile(szFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE == hFile) return NULL;

    // get file size
    DWORD dwFileSize = GetFileSize(hFile, NULL);
    if (-1 != dwFileSize) 
    {
        LPVOID pvData = NULL;
        // alloc memory based on file size
        HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, dwFileSize);
        if (NULL != hGlobal)
        {

            pvData = GlobalLock(hGlobal);
            if (NULL != pvData)
            {
                DWORD dwBytesRead = 0;
                // read file and store in global memory
                BOOL bRead = ReadFile(hFile, pvData, dwFileSize, &dwBytesRead, NULL);
                if (bRead != FALSE)
                {
                    LPSTREAM pstm = NULL;
                    // create IStream* from global memory
                    HRESULT hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pstm);
                    if (SUCCEEDED(hr) && pstm)
                    {
                        // Create IPicture from image file
                        hr = ::OleLoadPicture(pstm, dwFileSize, FALSE, IID_IPicture, (LPVOID *)&gpPicture);
                        if (SUCCEEDED(hr) && gpPicture)
                        {
                            pstm->Release();

                            OLE_HANDLE m_picHandle;

                            gpPicture->get_Handle(&m_picHandle);

                            HDC hdc = CreateCompatibleDC(NULL);

                            SelectObject(hdc, (HGDIOBJ) m_picHandle);

                            ret = (HBITMAP)GetCurrentObject(hdc, OBJ_BITMAP);

                            DeleteDC(hdc);
                        }
                    }

                }
                GlobalUnlock(hGlobal);
            }
        }

    }
    CloseHandle(hFile);
    return ret;
}


////////////////////////////////////////////////////////////////////////////////
// Get bitmaps from ini section ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


CBitmapsFromIniSection::CBitmapsFromIniSection()
{
}

CBitmapsFromIniSection::~CBitmapsFromIniSection()
{
    Finished();
}

BOOL CBitmapsFromIniSection::Register(string sName)
{
    if (sName.length()==0)
    {
        return FALSE;
    }
    TBitmapIniInfo bii;
    bii.sName = sName;
    bii.hBmp = NULL;
    bii.hBmpMask = NULL;
    bii.Result = 1;    
    BIList.push_back(bii);
    return TRUE;
}

void CBitmapsFromIniSection::Finished()
{
    if (hMainBmp != NULL)
    {
        ::DeleteObject(hMainBmp);
        hMainBmp = NULL;
    }
    if (hMainBmpMask != NULL)
    {
        ::DeleteObject(hMainBmpMask);
        hMainBmpMask = NULL;
    }
    sLastIniFile = "";
}

void CBitmapsFromIniSection::Clear()
{
    Finished();

    for (int i = 0; i < BIList.size(); i++)
    {
        //Free bitmap2
        if (BIList[i].hBmp != NULL)
        {            
            ::DeleteObject(BIList[i].hBmp);
            BIList[i].hBmp = NULL;
        }
        if (BIList[i].hBmpMask != NULL)
        {            
            ::DeleteObject(BIList[i].hBmpMask);
            BIList[i].hBmpMask = NULL;
        }
    }
    BIList.clear();
}    

HBITMAP CBitmapsFromIniSection::Get(string sName)
{
    for (int i = 0; i < BIList.size(); i++)
    {
        if (BIList[i].sName == sName) 
        {
            return BIList[i].hBmp;
        }
    }
    return NULL;
}

HBITMAP CBitmapsFromIniSection::GetMask(string sName)
{
    for (int i = 0; i < BIList.size(); i++)
    {
        if (BIList[i].sName == sName) 
        {
            return BIList[i].hBmpMask;
        }
    }
    return NULL;
}
    
string CBitmapsFromIniSection::GetInfo(string sName)
{
    for (int i = 0; i < BIList.size(); i++)
    {
        if (BIList[i].sName == sName) 
        {
            return BIList[i].sExtraInfo;            
        }
    }
    return "";
}

/**
    Result:
     0 : all ok
     1 : never loaded
     2 : unknown name
    -1 : entry not found in .ini file
    -2 : syntax error in ini entry value
    -3 : invalid coordinates
int CBitmapsFromIniSection::GetResult(string sName)
{
    for (int i = 0; i < BIList.size(); i++)
    {
        if (BIList[i].sName == sName) 
        {
            return BIList[i].Result;
        }
    }
    return 2;
}

/**
    Return:
    -1: bitmap ini entry not found
    -2: can't open bitmap
    -3: can't open mask bitmap
    -4: size of mask bitmap not equal to size of main bitmap
    -100: at least one item failed. Use GetResult() to find out which one
     1: at least one item was not found in the ini file
     0: all ok
*/
int CBitmapsFromIniSection::Read(string sIniFile, string sSection, string sBitmapName, string sBitmapMaskName)
{
    char szBitmapName[100];
    char szBitmapMaskName[100];

    // Load main bitmap
    if ((sIniFile == sLastIniFile) && (sSection == sLastSection) && 
        (hMainBmp!=NULL))
    {
        
    }
    else
    {
        sLastIniFile = sIniFile;
        sLastSection = sSection;
        
        GetPrivateProfileString(sSection.c_str(),sBitmapName.c_str(),"",szBitmapName,100-1,sIniFile.c_str());    
        GetPrivateProfileString(sSection.c_str(),sBitmapMaskName.c_str(),"",szBitmapMaskName,100-1,sIniFile.c_str());

        if (szBitmapName[0] == 0)
        {            
            //Not found
            return -1;
        }
        
        string sFileName;
        string sPath;
        
        char szPath[MAX_PATH+1];
        char *s = NULL;
        int len = GetFullPathName(sIniFile.c_str(), MAX_PATH, szPath, &s);
        if ((len > 0) && (s!=NULL))
        {
            *s = 0;
            sPath = szPath;
        }
        sFileName = sPath + szBitmapName;        
        
        hMainBmp = CBitmapHolder::BitmapLoadFromFile(sFileName.c_str());
        if (hMainBmp == NULL)
        {
            hMainBmp = (HBITMAP)::LoadImage(NULL,sFileName.c_str(),IMAGE_BITMAP,0,0,LR_LOADFROMFILE|LR_DEFAULTSIZE);
        }

        if (hMainBmp == NULL)
        {
            //Can't open
            return -2;
        }
        
        if (szBitmapMaskName[0]!=0)
        {            
            sFileName = sPath + szBitmapMaskName;                    
            hMainBmpMask = CBitmapHolder::BitmapLoadFromFile(sFileName.c_str());
            if (hMainBmpMask == NULL)
            {
                hMainBmpMask = (HBITMAP)::LoadImage(NULL,sFileName.c_str(),IMAGE_BITMAP,0,0,LR_LOADFROMFILE|LR_DEFAULTSIZE);
            }

            if (hMainBmpMask == NULL)
            {
                //Can't open
                return -3;
            }
            BITMAP bm1;	
            BITMAP bm2;	
            ::GetObject (hMainBmp, sizeof (bm1), & bm1);
            ::GetObject (hMainBmpMask, sizeof (bm2), & bm2);
            if ((bm1.bmWidth != bm2.bmWidth) || (bm1.bmHeight != bm2.bmHeight))
            {
                //Mask bitmap size not equal to main bitmap size
                return -4;
            }
        }
        else
        {
            hMainBmpMask = NULL;
        }        
    }

    int Result = 0;
    // Extract bitmaps

    char szVal[200];
    char szVal2[200];    
    char szExtra[200];
    int left,top,right,bottom, x,y;

    HDC hsrcDC =  ::CreateCompatibleDC(NULL);
    HDC hdestDC = ::CreateCompatibleDC(NULL);    

    BITMAP bm;	
    ::GetObject (hMainBmp, sizeof (bm), & bm);

    for (int i = 0; i < BIList.size(); i++)
    {
        GetPrivateProfileString(sSection.c_str(),BIList[i].sName.c_str(),"",szVal,200-1,sIniFile.c_str());    
        char *pSrc = szVal;
        char *pDst = szVal2;
        while (*pSrc!=0) { if ((*pSrc == ' ') || (*pSrc=='\t')) { pSrc++; } else { *pDst++=*pSrc++; } }
        *pDst = 0;
        if (szVal2[0]!=0)  //Analyze string
        {            
            szExtra[0] = 0;
            x = 0;
            y = 0;
            if (sscanf(szVal2,"%d,%d,%d,%d,%s",&left,&top,&right,&bottom,szExtra) >= 4)
            {
                if (   (left>=0) && (top>=0) && (right<=bm.bmWidth) && (bottom<=bm.bmHeight)
                    && (right>left) && (bottom>top)
                   )
                {
                    //
                    RECT rc;
                    ::SetRect(&rc,left,top,right,bottom);
                    ::SelectObject(hsrcDC, hMainBmp);
                    BIList[i].hBmp = CBitmapHolder::BitmapCopyPieceRGB(hdestDC, hsrcDC, &rc);
                    if (hMainBmpMask != NULL)
                    {
                        ::SelectObject(hsrcDC, hMainBmpMask);
                        BIList[i].hBmpMask = CBitmapHolder::BitmapCopyPieceRGB(hdestDC, hsrcDC, &rc);
                    }
                    BIList[i].sExtraInfo = szExtra;
                    BIList[i].Result = 0;
                }
                else
                {
                    //Invalid coordinates
                    BIList[i].Result = -3;
                }   
            }
            else
            {
                //LOG(1,"Error in Ini entry '%s' in section '%s' of '%s'",BIList[i].sName.c_str(),sSection.c_str(),sIniFile.c_str());
                //Syntax error
                BIList[i].Result = -2;
            }
        }
        else
        {
            BIList[i].Result = -1;
        }
        if (BIList[i].Result == -1)
        {
            if (Result >=0 )
            {
                Result = 1;
            }
        }
        if (BIList[i].Result < -1)
        {
            Result = -100;
        }
    }   

    ::DeleteDC(hsrcDC);	 
    ::DeleteDC(hdestDC);

    return Result;
}
