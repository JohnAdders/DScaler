#ifndef _BITMAP_H_
#define _BITMAP_H_

// Bitmaps for different states
//  -Can draw bitmap normal, tiled, stretched
//  -Can create a window region from bitmap

class CBitmapHolder
{
protected:    
    typedef struct
    {
        HBITMAP hBmp;
        HBITMAP hBmpMask;

        int BmpWidth;
        int BmpHeight;

        vector<LPRECT> RegionList;
        HRGN hRegion;

        BOOL bDeleteBitmapOnExit;
    } TState;

    vector<TState> States;

    int DrawMode; //0=normal, 1=stretch to fit, 2=tile

    BOOL MakeRegionList();
    void FreeState(int State);
    
public:
    CBitmapHolder(int DrawMode = 0);
    ~CBitmapHolder();
    void SetDrawMode(int Mode) { DrawMode = Mode; };
    
    BOOL Add(HBITMAP hBmp, HBITMAP hBmpMask, int State = 0, BOOL bDeleteBitmapOnExit = FALSE);    
    
    HBITMAP GetBitmap(int State = 0);
    HBITMAP GetBitmapMask(int State = 0);

    void Draw(HDC dc, POINT* bmstart, LPRECT r, int State = 0);
    
    int Width(int State = 0);
    int Height(int State = 0);
    
    vector<LPRECT> *GetRegionList(int State = 0);    
    HRGN GetWindowRegion(int State = 0);
    
    //void BitmapToRegionList(int State = 0);

    // Static functions    

    static void BitmapDraw(HDC dc, HBITMAP hbmp, HBITMAP hmask, POINT* bmstart, LPRECT r, int DrawMode);    
    static void BitmapDrawTiled(HDC hDC, HBITMAP hbmp, POINT *bmstart, LPRECT r);
    
    static HBITMAP BitmapCopyPieceRGB(HDC hdestDC, HDC hsrcDC, LPRECT lpRect);
    
    static HRGN CreateWindowRegion(RECT *rcBound, vector<LPRECT> *RegionList, POINT *pPosition = NULL);
    static BOOL BitmapToRegionList(HBITMAP hBmpMask, vector<LPRECT> *RegionList);
        
    static HBITMAP BitmapLoadFromFile(const char *szFile);
    //static void BitmapLoad_Free(HBITMAP hBm);
};



// Read bitmaps described in ini section
class CBitmapsFromIniSection
{
protected:
    string sLastIniFile;
    string sLastSection;
    HBITMAP hMainBmp;
    HBITMAP hMainBmpMask;

    typedef struct {
        string sName;
        HBITMAP hBmp;
        HBITMAP hBmpMask;
        string sExtraInfo;
        int	Result;
    } TBitmapIniInfo;
    
    vector<TBitmapIniInfo> BIList;
public:
    CBitmapsFromIniSection();
    ~CBitmapsFromIniSection();
    
    BOOL Register(string sName);
    void Clear(); //Free all, including loaded bitmaps

    int  Read(string sIniFile, string sSection, string sBitmapName, string sBitmapMaskName);
    void Finished(); //Free some memory

    HBITMAP Get(string sName);
    HBITMAP GetMask(string sName);
    string  GetInfo(string sName);
    int     GetResult(string sName);
};


#endif
