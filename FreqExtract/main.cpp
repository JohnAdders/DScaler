////////////////////////////////////////////////////////////////////////////
// $Id: main.cpp,v 1.1 2002-10-30 18:04:16 tobbej Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Torbjörn Jansson.  All rights reserved.
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
// CVS Log
//
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

/**
 * @file FreqExtract.cpp Defines the entry point for the console application.
 */

#include "stdafx.h"

#pragma warning(disable : 4786)
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <strmif.h>

char szRCDATA[10];
HMODULE hDSTunerModule=NULL;

static void *GetRcData(LPCTSTR lpFileName, LPCTSTR lpName, LPCTSTR lpType, HMODULE &hDSTunerModule)
{
	if (hDSTunerModule == NULL)
	{
		hDSTunerModule = LoadLibrary(lpFileName);
	}

	HRSRC hDSTunerResource = FindResource(hDSTunerModule, lpName, lpType);
	if (hDSTunerResource == NULL)
	{
		return NULL;
	}

	HGLOBAL hDSTunerTable = LoadResource(hDSTunerModule, hDSTunerResource);
	if (hDSTunerTable == NULL)
	{
		return NULL;
	}

	return LockResource(hDSTunerTable);
}
//force the compiler to not try to aligne this struct
//if it does, Index++ will increase the pointer by 2 bytes too much
#pragma pack(push,1)
typedef struct
{
	WORD CountryCode;
	WORD CableFreqTable;
	WORD BroadcastFreqTable;
	DWORD VideoStandard;
} TRCCountryList;
#pragma pack(pop)

void usage(char* ProgName)
{
	std::cerr << "Usage: " << ProgName << " <options>" << std::endl;
	std::cerr << " -s Many countries in the same section (shorter format)" << std::endl;
	std::cerr << " -l Each country gets its own section (very long)" << std::endl;
	std::cerr << " -c <country code> Get frequency list for specified country only" << std::endl;
}

char* VideoStandardToString(DWORD VideoStandard)
{
	switch(VideoStandard)
	{
	case AnalogVideo_PAL_B:
		return "PAL-B";
	case AnalogVideo_PAL_D:
		return "PAL-D";
	case AnalogVideo_PAL_G:
		return "PAL-G";
	case AnalogVideo_PAL_H:
		return "PAL-H";
	case AnalogVideo_PAL_I:
		return "PAL-I";
	case AnalogVideo_PAL_M:
		return "PAL-M";
	case AnalogVideo_PAL_N:
		return "PAL-N";
	case AnalogVideo_PAL_60:
		return "PAL-60";
	case AnalogVideo_PAL_N_COMBO:
		return "PAL-N-COMBO";
	case AnalogVideo_SECAM_B:
		return "SECAM-B";
	case AnalogVideo_SECAM_D:
		return "SECAM-D";
	case AnalogVideo_SECAM_G:
		return "SECAM-G";
	case AnalogVideo_SECAM_H:
		return "SECAM-H";
	case AnalogVideo_SECAM_K:
		return "SECAM-K";
	case AnalogVideo_SECAM_K1:
		return "SECAM-K1";
	case AnalogVideo_SECAM_L:
		return "SECAM-L";
	case AnalogVideo_SECAM_L1:
		return "SECAM-L1";
	case AnalogVideo_NTSC_M:
		return "NTSC-M";
	case AnalogVideo_NTSC_M_J:
		return "NTSC-M-Japan";
	case AnalogVideo_NTSC_433:
		return "NTSC-50";
	default:
		return NULL;
	}
}

void PrintFrequencyList(TRCCountryList &CountryList,bool AirTable,bool CableTable)
{
	char lName[20];
	long *FreqTable = NULL;
	if(CableTable)
	{
		sprintf(lName,"#%d",CountryList.CableFreqTable);
		FreqTable = (long*)GetRcData("kstvtune.ax", lName, szRCDATA, hDSTunerModule);
		if(FreqTable!=NULL)
		{
			std::cout << "[Country " << CountryList.CountryCode << " Cable Frequency]" << std::endl;
			std::cout << "ChannelLow=" << FreqTable[0] << std::endl;
			std::cout << "ChannelHigh=" << FreqTable[1] << std::endl;
			if(VideoStandardToString(CountryList.VideoStandard)!=NULL)
			{
				std::cout << "Format=" << VideoStandardToString(CountryList.VideoStandard) << std::endl;
			}
			for(int i=FreqTable[0]+1;i<(FreqTable[1]+2);i++)
			{
				std::cout << FreqTable[i] << std::endl;
			}
		}
	}
	if(AirTable)
	{
		sprintf(lName,"#%d",CountryList.BroadcastFreqTable);
		FreqTable = (long*)GetRcData("kstvtune.ax", lName, szRCDATA, hDSTunerModule);
		if(FreqTable!=NULL)
		{
			std::cout << "[Country " << CountryList.CountryCode << " Air Frequency]" << std::endl;
			std::cout << "ChannelLow=" << FreqTable[0] << std::endl;
			std::cout << "ChannelHigh=" << FreqTable[1] << std::endl;
			if(VideoStandardToString(CountryList.VideoStandard)!=NULL)
			{
				std::cout << "Format=" << VideoStandardToString(CountryList.VideoStandard) << std::endl;
			}
			for(int i=FreqTable[0]+1;i<(FreqTable[1]+2);i++)
			{
				std::cout << FreqTable[i] << std::endl;
			}
		}
	}
}


int main(int argc, char* argv[])
{
	long *FreqTable=NULL;
	int Index=0;
	
	if(argc<2)
	{
		usage(argv[0]);
		return 1;
	}
	
	sprintf(szRCDATA,"#%d",RT_RCDATA);
	TRCCountryList *RcCountryList = (TRCCountryList*)GetRcData("kstvtune.ax","#9999",szRCDATA, hDSTunerModule);

	if(strcmp(argv[1],"-s")==0)
	{
		//many coutries in one section
		std::map<long,std::vector<long> > Cable;
		std::map<long,std::vector<long> > Broadcast;
		while((RcCountryList != NULL) && (RcCountryList[Index].CountryCode != 0))
		{
			Cable[RcCountryList[Index].CableFreqTable].push_back(RcCountryList[Index].CountryCode);
			Broadcast[RcCountryList[Index].BroadcastFreqTable].push_back(RcCountryList[Index].CountryCode);
			Index++;
		}
		for(std::map<long,std::vector<long> >::iterator it=Cable.begin();it!=Cable.end();it++)
		{
			char lName[20];
			sprintf(lName,"#%d",it->first);
			FreqTable = (long*)GetRcData("kstvtune.ax", lName, szRCDATA, hDSTunerModule);
			if(FreqTable!=NULL)
			{
				std::cout << "[Countrys ";
				for(std::vector<long>::iterator vit=it->second.begin();vit!=it->second.end();vit++)
				{
					std::cout << *vit << ",";
				}
				std::cout << " Cable Frequency]" << std::endl;
				std::cout << "ChannelLow=" << FreqTable[0] << std::endl;
				std::cout << "ChannelHigh=" << FreqTable[1] << std::endl;
				for(int i=FreqTable[0]+1;i<(FreqTable[1]+2);i++)
				{
					std::cout << FreqTable[i] << std::endl;
				}
			}
		}
		for(it=Broadcast.begin();it!=Broadcast.end();it++)
		{
			char lName[20];
			sprintf(lName,"#%d",it->first);
			FreqTable = (long*)GetRcData("kstvtune.ax", lName, szRCDATA, hDSTunerModule);
			if(FreqTable!=NULL)
			{
				std::cout << "[Countrys ";
				for(std::vector<long>::iterator vit=it->second.begin();vit!=it->second.end();vit++)
				{
					std::cout << *vit << ",";
				}
				std::cout << " Air Frequency]" << std::endl;
				std::cout << "ChannelLow=" << FreqTable[0] << std::endl;
				std::cout << "ChannelHigh=" << FreqTable[1] << std::endl;
				for(int i=FreqTable[0]+1;i<(FreqTable[1]+2);i++)
				{
					std::cout << FreqTable[i] << std::endl;
				}
			}
		}
	}
	else if(strcmp(argv[1],"-l")==0)
	{
		//each country gets its own section (very long list)
		while((RcCountryList != NULL) && (RcCountryList[Index].CountryCode != 0))
		{
			PrintFrequencyList(RcCountryList[Index],true,true);
			Index++;
		}
	}
	else if(strcmp(argv[1],"-c")==0)
	{
		//specified country only
		if(argc!=3)
		{
			usage(argv[0]);
		}
		else
		{
			long Country=atol(argv[2]);
			bool CountryFound=false;
			while((RcCountryList != NULL) && (RcCountryList[Index].CountryCode != 0))
			{
				if(RcCountryList[Index].CountryCode==Country)
				{
					CountryFound=true;
					PrintFrequencyList(RcCountryList[Index],true,true);
					break;
				}
				Index++;
			}
			if(!CountryFound)
			{
				std::cerr << "Failed to find country " << Country << std::endl;
			}
		}
	}
	else
	{
		std::cout << "Unknown option '" << argv[1] << "'" << std::endl;
		usage(argv[0]);
	}

	if(hDSTunerModule != NULL)
	{
		FreeLibrary(hDSTunerModule);
	}

	return 0;
}

