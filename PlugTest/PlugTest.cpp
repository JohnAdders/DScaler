/////////////////////////////////////////////////////////////////////////////
// $Id: PlugTest.cpp,v 1.8 2001-07-16 18:27:28 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
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
// 10 Apr 2001   John Adcock           Initial Version
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.7  2001/07/13 16:15:44  adcockj
// Changed lots of variables to match Coding standards
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

char szIniFile[MAX_PATH];
void ReadFromIni(SETTING* pSetting, char* szIniFile);

void memcpyMMX(void *Dest, void *Src, size_t nBytes)
{
    __asm {
        mov     esi, dword ptr[Src]
        mov     edi, dword ptr[Dest]
        mov     ecx, nBytes
        shr     ecx, 6                      // nBytes / 64
align 8
CopyLoop:
        movq    mm0, qword ptr[esi]
        movq    mm1, qword ptr[esi+8*1]
        movq    mm2, qword ptr[esi+8*2]
        movq    mm3, qword ptr[esi+8*3]
        movq    mm4, qword ptr[esi+8*4]
        movq    mm5, qword ptr[esi+8*5]
        movq    mm6, qword ptr[esi+8*6]
        movq    mm7, qword ptr[esi+8*7]
        movq    qword ptr[edi], mm0
        movq    qword ptr[edi+8*1], mm1
        movq    qword ptr[edi+8*2], mm2
        movq    qword ptr[edi+8*3], mm3
        movq    qword ptr[edi+8*4], mm4
        movq    qword ptr[edi+8*5], mm5
        movq    qword ptr[edi+8*6], mm6
        movq    qword ptr[edi+8*7], mm7
        add     esi, 64
        add     edi, 64
        loop CopyLoop
        mov     ecx, nBytes
        and     ecx, 63
        cmp     ecx, 0
        je EndCopyLoop
align 8
CopyLoop2:
        mov dl, byte ptr[esi] 
        mov byte ptr[edi], dl
        inc esi
        inc edi
        dec ecx
        jne near CopyLoop2
EndCopyLoop:
    }
}

BOOL FillInfoStruct(DEINTERLACE_INFO* info, char* SnapshotFile)
{
    FILE *file;
    unsigned int NumRead;
    int i = 0;
    int j;

    file = fopen(SnapshotFile,"rb");
    if (!file)
    {
        printf("Could not open file %s\n", SnapshotFile);
        return FALSE;
    }
    NumRead = fread(info, 1, sizeof(DEINTERLACE_INFO), file);
    if(NumRead < sizeof(DEINTERLACE_INFO))
    {
        printf("Error reading file %s\n", SnapshotFile);
        fclose(file);      
        return FALSE;
    }
    // read in odd fields
    for (i = 0; i < MAX_FIELD_HISTORY; i++)
    {
        if (info->OddLines[i] == NULL)
        {
            printf("Odd field %d empty\n", i+1);
        }
        else
        {
            info->OddLines[i] = (short**)malloc(info->FieldHeight * sizeof(short*));
            for(j = 0; j < info->FieldHeight; ++j)
            {
                info->OddLines[i][j] = (short*)malloc(info->LineLength);
                NumRead = fread(info->OddLines[i][j], 1, info->LineLength, file);
                if(NumRead < info->LineLength)
                {
                    printf("Error reading file %s\n", SnapshotFile);
                    fclose(file);      
                    return FALSE;
                }
            }
        }
    }

    // read in even fields
    for (i = 0; i < MAX_FIELD_HISTORY; i++)
    {
        if (info->EvenLines[i] == NULL)
        {
            printf("Even field %d empty\n", i+1);
        }
        else
        {
            info->EvenLines[i] = (short**)malloc(info->FieldHeight * sizeof(short*));
            for(j = 0; j < info->FieldHeight; ++j)
            {
                info->EvenLines[i][j] = (short*)malloc(info->LineLength);
                NumRead = fread(info->EvenLines[i][j], 1, info->LineLength, file);
                if(NumRead < info->LineLength)
                {
                    printf("Error reading file %s\n", SnapshotFile);
                    fclose(file);      
                    return FALSE;
                }
            }
        }
    }

    info->Overlay = (BYTE*)malloc(info->OverlayPitch * info->FrameHeight);
    info->CpuFeatureFlags = 0;
    info->pMemcpy = memcpyMMX;
    fclose(file);      
    return TRUE;
}

void EmptyInfoStruct(DEINTERLACE_INFO* info)
{
   int i, j;

    i = 0;
    while(i < MAX_FIELD_HISTORY && info->OddLines[i] != NULL)
    {
        for(j = 0; j < info->FieldHeight; ++j)
        {
            free(info->OddLines[i][j]);
        }
        free(info->OddLines[i]);
        i++;
    }

    i = 0;
    while(i < MAX_FIELD_HISTORY && info->EvenLines[i] != NULL)
    {
        for(j = 0; j < info->FieldHeight; ++j)
        {
            free(info->EvenLines[i][j]);
        }
        free(info->EvenLines[i]);
        i++;
    }

   free(info->Overlay);
}

BOOL LoadFilterPlugin(LPCSTR szFileName, FILTER_METHOD** FilterMethod)
{
    GETFILTERPLUGININFO* pfnGetFilterPluginInfo;
    FILTER_METHOD* pMethod;
    HMODULE hPlugInMod;
    int i;

    hPlugInMod = LoadLibrary(szFileName);
    if(hPlugInMod == NULL)
    {
        return FALSE;
    }
    
    pfnGetFilterPluginInfo = (GETFILTERPLUGININFO*)GetProcAddress(hPlugInMod, "GetFilterPluginInfo");
    if(pfnGetFilterPluginInfo == NULL)
    {
        return FALSE;
    }

    pMethod = pfnGetFilterPluginInfo(0);
    if(pMethod != NULL)
    {
        *FilterMethod = pMethod;
        pMethod->hModule = hPlugInMod;
        for (i = 0; i < pMethod->nSettings; i++)
        {
            ReadFromIni(&(pMethod->pSettings[i]), szIniFile);
        }
        if(pMethod->pfnPluginStart != NULL)
        {
            pMethod->pfnPluginStart();
        }
    }
    return TRUE;
}

void UnloadFilterPlugin(FILTER_METHOD* FilterMethod)
{
   if(FilterMethod->pfnPluginExit != NULL)
   {
      FilterMethod->pfnPluginExit();
   }
   FreeLibrary(FilterMethod->hModule);
}

BOOL LoadDeintPlugin(LPCSTR szFileName, DEINTERLACE_METHOD** DeintMethod)
{
    GETDEINTERLACEPLUGININFO* pfnGetDeinterlacePluginInfo;
    DEINTERLACE_METHOD* pMethod;
    HMODULE hPlugInMod;

    hPlugInMod = LoadLibrary(szFileName);
    if(hPlugInMod == NULL)
    {
        return FALSE;
    }

    pfnGetDeinterlacePluginInfo = (GETDEINTERLACEPLUGININFO*)GetProcAddress(hPlugInMod, "GetDeinterlacePluginInfo");
    if(pfnGetDeinterlacePluginInfo == NULL)
    {
        return FALSE;
    }

    pMethod = pfnGetDeinterlacePluginInfo(0);
    if(pMethod != NULL)
    {
        *DeintMethod = pMethod;
        pMethod->hModule = hPlugInMod;
        if(pMethod->pfnPluginInit != NULL)
        {
            pMethod->pfnPluginInit();
        }
        if(pMethod->pfnPluginStart != NULL)
        {
            pMethod->pfnPluginStart(0, NULL, NULL);
        }
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void UnloadDeintPlugin(DEINTERLACE_METHOD* DeintMethod)
{
   if(DeintMethod->pfnPluginExit != NULL)
   {
      DeintMethod->pfnPluginExit();
   }
   FreeLibrary(DeintMethod->hModule);
}

#define LIMIT(x) (((x)<0)?0:((x)>255)?255:(x))
#pragma pack(1)

// A TIFF image-file directory entry.  There are a bunch of
// these in a TIFF file.
struct TiffDirEntry {
    WORD tag;       // Entry type
    WORD type;      // 1=byte, 2=C string, 3=word, 4=dword (we always use dword)
    DWORD count;    // Number of units (of type specified by "type") in value
    DWORD value;
};

// Field data types.
enum TiffDataType {
    Byte = 1,
    String = 2,
    Short = 3,
    Long = 4
};

// A TIFF header with some hardwired fields.
struct TiffHeader {
    char byteOrder[2];
    WORD version;
    DWORD firstDirOffset;

    // TIFF files contain a bunch of extra information, each of which is a
    // tagged "directory" entry.  The entries must be in ascending numerical
    // order.

    WORD numDirEntries;
    struct TiffDirEntry fileType;       // What kind of file this is (tag 254)
    struct TiffDirEntry width;          // Width of image (tag 256)
    struct TiffDirEntry height;         // Height of image (tag 257)
    struct TiffDirEntry bitsPerSample;  // Number of bits per channel per pixel (tag 258)
    struct TiffDirEntry compression;    // Compression settings (tag 259)
    struct TiffDirEntry photometricInterpretation; // What kind of pixel data this is (tag 262)
    struct TiffDirEntry description;    // Image description (tag 270)
    struct TiffDirEntry make;           // "Scanner" maker, aka DScaler's URL (tag 271)
    struct TiffDirEntry model;          // "Scanner" model, aka DScaler version (tag 272)
    struct TiffDirEntry stripOffset;    // Offset to image data (tag 273)
    struct TiffDirEntry samplesPerPixel; // Number of color channels (tag 277)
    struct TiffDirEntry rowsPerStrip;   // Number of rows in a strip (tag 278)
    struct TiffDirEntry stripByteCounts; // Number of bytes per strip (tag 279)
    struct TiffDirEntry planarConfiguration; // Are channels interleaved? (tag 284)
    DWORD nextDirOffset;

    // We store a few strings in the file; include them in the structure so
    // it's easy to compute their offsets.  Yeah, this wastes a bit of disk
    // space, but an insignificant percentage of the overall file size.
    char descriptionText[80];
    char makeText[40];
    char modelText[16];
    WORD bitCounts[3];
};

#define STRUCT_OFFSET(s,f)  ((int)(((BYTE *) &(s)->f) - (BYTE *)(s)))

//-----------------------------------------------------------------------------
// Fill a TIFF directory entry with information.
static void FillTiffDirEntry(struct TiffDirEntry *entry, WORD tag, DWORD value, enum TiffDataType type)
{
    BYTE bValue;
    WORD wValue;

    entry->tag = tag;
    entry->count = 1;
    entry->type = (int) type;

    switch (type) {
    case Byte:
        bValue = (BYTE) value;
        memcpy(&entry->value, &bValue, 1);
        break;

    case Short:
        wValue = (WORD) value;
        memcpy(&entry->value, &wValue, 2);
        break;

    case String:    // in which case it's a file offset
    case Long:
        entry->value = value;
        break;
    }
}


//-----------------------------------------------------------------------------
// Fill a TIFF header with information about the current image.
static void FillTiffHeader(struct TiffHeader *head, char *description, char *make, char *model, int Height, DEINTERLACE_INFO* info)
{
    memset(head, 0, sizeof(struct TiffHeader));

    strcpy(head->byteOrder, "II");      // Intel byte order
    head->version = 42;                 // We're TIFF 5.0 compliant, but the version field is unused
    head->firstDirOffset = STRUCT_OFFSET(head, numDirEntries);
    head->numDirEntries = 14;
    head->nextDirOffset = 0;            // No additional directories

    strcpy(head->descriptionText, description);
    strcpy(head->makeText, make);
    strcpy(head->modelText, model);
    head->bitCounts[0] = head->bitCounts[1] = head->bitCounts[2] = 8;

    head->description.tag = 270;
    head->description.type = 2;
    head->description.count = strlen(description) + 1;
    head->description.value = STRUCT_OFFSET(head, descriptionText);

    head->make.tag = 271;
    head->make.type = 2;
    head->make.count = strlen(make) + 1;
    head->make.value = STRUCT_OFFSET(head, makeText);

    head->model.tag = 272;
    head->model.type = 2;
    head->model.count = strlen(model) + 1;
    head->model.value = STRUCT_OFFSET(head, modelText);
    
    head->bitsPerSample.tag = 258;
    head->bitsPerSample.type = Short;
    head->bitsPerSample.count = 3;
    head->bitsPerSample.value = STRUCT_OFFSET(head, bitCounts);

    FillTiffDirEntry(&head->fileType, 254, 0, Long);                        // Just the image, no thumbnails
    FillTiffDirEntry(&head->width, 256, info->FrameWidth, Short);
    FillTiffDirEntry(&head->height, 257, Height, Short);
    FillTiffDirEntry(&head->compression, 259, 1, Short);                    // No compression
    FillTiffDirEntry(&head->photometricInterpretation, 262, 2, Short);      // RGB image data
    FillTiffDirEntry(&head->stripOffset, 273, sizeof(struct TiffHeader), Long); // Image comes after header
    FillTiffDirEntry(&head->samplesPerPixel, 277, 3, Short);                // RGB = 3 channels/pixel
    FillTiffDirEntry(&head->rowsPerStrip, 278, Height, Short);          // Whole image is one strip
    FillTiffDirEntry(&head->stripByteCounts, 279, info->FrameWidth * Height * 3, Long); // Size of image data
    FillTiffDirEntry(&head->planarConfiguration, 284, 1, Short);            // RGB bytes are interleaved
}

//-----------------------------------------------------------------------------
// Save still image snapshot as TIFF format to disk
BOOL MakeTifFile(DEINTERLACE_INFO* info, char* TifFile, DEINTERLACE_METHOD* DeintMethod, int OddField, int EvenField)
{
    int y, cr, cb, r, g, b, i, j, n = 0;
    FILE *file;
    BYTE rgb[3];
    BYTE* buf;
    struct TiffHeader head;
    char description[] = "DScaler image";
    int NbLines;

    if (OddField >= 0 && info->OddLines[OddField] == NULL)
    {
        printf("Odd field missing => file %s not generated\n",TifFile);
        return FALSE;
    }
    if (EvenField >= 0 && info->EvenLines[EvenField] == NULL)
    {
        printf("Even field missing => file %s not generated\n",TifFile);
        return FALSE;
    }

    file = fopen(TifFile,"wb");
    if (!file)
    {
        printf("Could not open file %s\n", TifFile);
        return FALSE;
    }

    if (OddField >= 0 && EvenField >= 0)
        NbLines = info->FieldHeight * 2;
    else if (OddField >= 0 || EvenField >= 0)
        NbLines = info->FieldHeight;
    else if (DeintMethod->bIsHalfHeight)
        NbLines = info->FrameHeight / 2;
    else
        NbLines = info->FrameHeight;

    FillTiffHeader(&head, description, "http://deinterlace.sourceforge.net/", "PlugTest", NbLines, info);
    fwrite(&head, sizeof(head), 1, file);

    for (i = 0; i < NbLines; i++)
    {
        if (OddField >= 0 && EvenField < 0)
            buf = (BYTE*)info->OddLines[OddField][i];
        else if (EvenField >= 0 && OddField < 0)
            buf = (BYTE*)info->EvenLines[EvenField][i];
        else if (OddField >= 0 && i < info->FieldHeight)
            buf = (BYTE*)info->OddLines[OddField][i];
        else if (EvenField >= 0 && i >= info->FieldHeight)
            buf = (BYTE*)info->EvenLines[EvenField][i-info->FieldHeight];
        else
            buf = (BYTE*)info->Overlay + i * info->OverlayPitch;
        for (j = 0; j < info->FrameWidth ; j+=2)
        {
            cb = buf[1] - 128;
            cr = buf[3] - 128;
            y = buf[0] - 16;

            r = ( 76284*y + 104595*cr             )>>16;
            g = ( 76284*y -  53281*cr -  25624*cb )>>16;
            b = ( 76284*y             + 132252*cb )>>16;
            rgb[0] = LIMIT(r);
            rgb[1] = LIMIT(g);
            rgb[2] = LIMIT(b);

            fwrite(rgb,3,1,file) ;

            y = buf[2] - 16;
            r = ( 76284*y + 104595*cr             )>>16;
            g = ( 76284*y -  53281*cr -  25624*cb )>>16;
            b = ( 76284*y             + 132252*cb )>>16;
            rgb[0] = LIMIT(r);
            rgb[1] = LIMIT(g);
            rgb[2] = LIMIT(b);
            fwrite(rgb,3,1,file);

            buf += 4;
        }
    }
    fclose(file);
    return TRUE;
}

void ReadFromIni(SETTING* pSetting, char* szIniFile)
{
    long nValue;

    if(pSetting->szIniSection != NULL)
    {
        nValue = GetPrivateProfileInt(pSetting->szIniSection, pSetting->szIniEntry, pSetting->MinValue - 100, szIniFile);
        if(nValue < pSetting->MinValue)
        {
            nValue = pSetting->Default;
        }
        *pSetting->pValue = nValue;
        pSetting->LastSavedValue = *pSetting->pValue;
    }
}

int ProcessSnapShot(char* SnapshotFile, char* FilterPlugin, char* DeintPlugin, char* TifFile)
{
    DEINTERLACE_INFO info;
    DEINTERLACE_METHOD* DeintMethod = NULL;
    FILTER_METHOD* FilterMethod = NULL;
    LARGE_INTEGER EndTime;
    LARGE_INTEGER StartTime;
    LARGE_INTEGER TimerFrequency;
    int OddField = -1;
    int EvenField = -1;
    int i;

    GetCurrentDirectory(MAX_PATH, szIniFile);
    strcat(szIniFile, "\\DScaler.ini");

    // get the Frequency of the high resolution timer
    QueryPerformanceFrequency(&TimerFrequency);
    double TimerFreq = (double)TimerFrequency.QuadPart;

    if(!FillInfoStruct(&info, SnapshotFile))
    {
        return 1;
    }

    if(FilterPlugin != NULL)
    {
        if(!LoadFilterPlugin(FilterPlugin, &FilterMethod))
        {
            return 1;
        }
    }

    if(FilterMethod != NULL)
    {
        if(FilterMethod->bOnInput == TRUE)
        {
            QueryPerformanceCounter(&StartTime);
            FilterMethod->pfnAlgorithm(&info);
            QueryPerformanceCounter(&EndTime);
            double Ticks = (double)(EndTime.QuadPart - StartTime.QuadPart);
            printf("Input Filter %f microsecs\n", Ticks * 1000000 / TimerFreq);

            info.IsOdd = !info.IsOdd;
            QueryPerformanceCounter(&StartTime);
            FilterMethod->pfnAlgorithm(&info);
            QueryPerformanceCounter(&EndTime);
            Ticks = (double)(EndTime.QuadPart - StartTime.QuadPart);
            printf("Input Filter %f microsecs\n", Ticks * 1000000 / TimerFreq);
            info.IsOdd = !info.IsOdd;
        }
    }

    if (!strcmp(DeintPlugin, "odd1"))
    {
        OddField = 0;
    }
    else if (!strcmp(DeintPlugin, "odd2"))
    {
        OddField = 1;
    }
    else if (!strcmp(DeintPlugin, "odd3"))
    {
        OddField = 2;
    }
    else if (!strcmp(DeintPlugin, "odd4"))
    {
        OddField = 3;
    }
    else if (!strcmp(DeintPlugin, "odd5"))
    {
        OddField = 4;
    }
    else if (!strcmp(DeintPlugin, "even1"))
    {
        EvenField = 0;
    }
    else if (!strcmp(DeintPlugin, "even2"))
    {
        EvenField = 1;
    }
    else if (!strcmp(DeintPlugin, "even3"))
    {
        EvenField = 2;
    }
    else if (!strcmp(DeintPlugin, "even4"))
    {
        EvenField = 3;
    }
    else if (!strcmp(DeintPlugin, "even5"))
    {
        EvenField = 4;
    }
    else if (!strcmp(DeintPlugin, "oddeven1"))
    {
        OddField = 0;
        EvenField = 0;
    }
    else if (!strcmp(DeintPlugin, "oddeven2"))
    {
        OddField = 1;
        EvenField = 1;
    }
    else if (!strcmp(DeintPlugin, "oddeven3"))
    {
        OddField = 2;
        EvenField = 2;
    }
    else if (!strcmp(DeintPlugin, "oddeven4"))
    {
        OddField = 3;
        EvenField = 3;
    }
    else if (!strcmp(DeintPlugin, "oddeven5"))
    {
        OddField = 4;
        EvenField = 4;
    }
    else
    {
        if(!LoadDeintPlugin(DeintPlugin, &DeintMethod))
        {
            return 1;
        }
        for (i = 0; i < DeintMethod->nSettings; i++)
        {
            ReadFromIni(&(DeintMethod->pSettings[i]), szIniFile);
        }

        QueryPerformanceCounter(&StartTime);
        DeintMethod->pfnAlgorithm(&info);
        QueryPerformanceCounter(&EndTime);
        double Ticks = (double)(EndTime.QuadPart - StartTime.QuadPart);
        printf("Deint %f microsecs\n", Ticks * 1000000 / TimerFreq);

        if(FilterMethod != NULL)
        {
            if(FilterMethod->bOnInput == FALSE)
            {
                QueryPerformanceCounter(&StartTime);
                FilterMethod->pfnAlgorithm(&info);
                QueryPerformanceCounter(&EndTime);
                double Ticks = (double)(EndTime.QuadPart - StartTime.QuadPart);
                printf("Output Filter %f microsecs\n", Ticks * 1000000 / TimerFreq);
            }
        }
    }

    if(!MakeTifFile(&info, TifFile, DeintMethod, OddField, EvenField))
    {
        return 1;
    }

    if(FilterMethod != NULL)
    {
        UnloadFilterPlugin(FilterMethod);
    }

    if (OddField == -1 && EvenField == -1)
    {
        UnloadDeintPlugin(DeintMethod);
    }

    EmptyInfoStruct(&info);
    return 0;
}


int main(int argc, char* argv[])
{
    printf("PlugTest (c) 2001 John Adcock\n\n");
    printf("PlugTest comes with ABSOLUTELY NO WARRANTY\n");
    printf("This is free software, and you are welcome\n");
    printf("to redistribute it under certain conditions.\n");
    printf("See http://www.gnu.org/copyleft/gpl.html for details.\n\n");

    if(argc != 4 && argc != 5)
    {
        printf("Usage: PlugTest SnapFile.dtv [FilterPlugIn] DeintPlugIn OutputTifFile\n\n");
        printf("  FilterPlugIn is a DLL file\n");
        printf("  DeintPlugIn is either :\n");
        printf("    - a DLL file\n");
        printf("    - odd1|odd2|odd3|odd4|odd5 to save odd field\n");
        printf("    - even1|even2|even3|even4|even5 to save even field\n");
        printf("    - oddeven1|oddeven2|oddeven3|oddeven4|oddeven5 to save odd+even fields\n");
        return 1;
    }
    if(argc == 4)
    {
        return ProcessSnapShot(argv[1], NULL, argv[2], argv[3]);
    }
    else
    {
        return ProcessSnapShot(argv[1], argv[2], argv[3], argv[4]);
    }
}

