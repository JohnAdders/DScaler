/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 John Adcock.  All rights reserved.
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

/**
 * @file Bitmap.h  Bitmap Header file
 */

#ifndef _BITMAP_H_
#define _BITMAP_H_

// Bitmaps for different states
//  -Can draw bitmap normal, tiled, stretched
//  -Can create a window region from bitmap

class CBitmapState
{
public:
    CBitmapState();
    CBitmapState(SmartHandle<HBITMAP> hBmp, SmartHandle<HBITMAP> hBmpMask);

    SmartHandle<HBITMAP> m_hBmp;
    SmartHandle<HBITMAP> m_hBmpMask;
    int m_BmpWidth;
    int m_BmpHeight;
    std::vector<RECT> m_RegionList;
    HRGN m_hRegion;
    tstring m_ExtraInfo;
};


class CBitmapHolder
{
public:
    CBitmapHolder(int DrawMode = 0);
    void SetDrawMode(int Mode) { m_DrawMode = Mode; };

    void Add(SmartPtr<CBitmapState> BitmapState, int State = 0);

    SmartHandle<HBITMAP> GetBitmap(int State = 0);

    void Draw(HDC dc, LPPOINT bmstart, LPRECT r, int State = 0);

    int Width(int State = 0);
    int Height(int State = 0);

    std::vector<RECT>& GetRegionList();
    HRGN GetWindowRegion();

    // Static functions

    static void BitmapDraw(HDC dc, SmartHandle<HBITMAP> hbmp, SmartHandle<HBITMAP> hmask, LPPOINT bmstart, LPRECT r, int DrawMode);
    static void BitmapDrawTiled(HDC hDC, SmartHandle<HBITMAP> hbmp, POINT *bmstart, LPRECT r);

    static SmartHandle<HBITMAP> BitmapCopyPieceRGB(HDC hdestDC, HDC hsrcDC, LPRECT lpRect);

    static HRGN CreateWindowRegion(RECT& rcBound, std::vector<RECT>& RegionList, POINT* pPosition = NULL);
    static BOOL BitmapToRegionList(SmartHandle<HBITMAP> hBmpMask, std::vector<RECT>& RegionList);

    static SmartHandle<HBITMAP> BitmapLoadFromFile(const TCHAR* szFile);
    //static void BitmapLoad_Free(HBITMAP hBm);

protected:
    BOOL MakeRegionList();

    std::vector< SmartPtr<CBitmapState> > m_States;
    int m_DrawMode; //0=normal, 1=stretch to fit, 2=tile

};

class CBitmapChangeInfo
{
public:
    ~CBitmapChangeInfo();
    HBITMAP hBmp;
    tstring sFileName;
    BOOL bFreeOnDestroy;
};


class CBitmapCache
{
public:
    SmartHandle<HBITMAP> Read(LPCTSTR szFileName);
private:
    std::map<tstring, SmartHandle<HBITMAP> > m_CacheList;
};


class CBitmapIniInfo
{
public:
    CBitmapIniInfo(tstring sName);
    tstring m_sName;
    SmartHandle<HBITMAP> m_hBmp;
    SmartHandle<HBITMAP> m_hBmpMask;
    tstring m_sExtraInfo;
    int m_Result;
};


// Read bitmaps described in ini section
class CBitmapsFromIniSection
{
public:
    CBitmapsFromIniSection();

    void Register(tstring sName);

    SmartPtr<CBitmapState> Get(tstring sName);

    int Read(tstring sIniFile, tstring sSection, tstring sBitmapName, tstring sBitmapMaskName);

protected:
    typedef std::map<tstring, SmartPtr<CBitmapState> > StateMap;
    SmartPtr<CBitmapCache> m_pBitmapCache;
    StateMap m_StateMap;
    tstring GetInfo(tstring sName);
};


#endif
