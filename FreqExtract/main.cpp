////////////////////////////////////////////////////////////////////////////
// $Id: main.cpp,v 1.2 2003-02-22 16:50:28 tobbej Exp $
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
// Revision 1.1  2002/10/30 18:04:16  tobbej
// new tool for extracting frequency tables from directshow to channel.txt format
//
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

char* CountryCodeToString(WORD CountryCode)
{
	typedef struct
	{
		WORD CountryCode;
		char *Name;
	}TCountryList;
	TCountryList list[]=
	{
		1,"USA",
		2,"Canada",
		7,"Russia",
		20,"Egypt",
		27,"South Africa",
		30,"Greece",
		31,"Netherlands",
		32,"Belgium",
		33,"France",
		34,"Spain",
		36,"Hungary",
		39,"Italy",
		39,"Vatican City",
		40,"Romania",
		41,"Liechtenstein",
		41,"Switzerland",
		43,"Austria",
		44,"United Kingdom",
		45,"Denmark",
		46,"Sweden",
		47,"Norway",
		48,"Poland",
		49,"Germany",
		51,"Peru",
		52,"Mexico",
		53,"Cuba",
		53,"Guantanamo Bay",
		54,"Argentina",
		55,"Brazil",
		56,"Chile",
		57,"Colombia",
		58,"Bolivarian Republic of Venezuela",
		60,"Malaysia",
		61,"Australia",
		61,"Cocos-Keeling Islands",
		62,"Indonesia",
		63,"Philippines",
		64,"New Zealand",
		65,"Singapore",
		66,"Thailand",
		81,"Japan",
		82,"Korea (South)",
		84,"Vietnam",
		86,"China",
		90,"Turkey",
		91,"India",
		92,"Pakistan",
		93,"Afghanistan",
		94,"Sri Lanka",
		95,"Myanmar",
		98,"Iran",
		212,"Morocco",
		213,"Algeria",
		216,"Tunisia",
		218,"Libya",
		220,"Gambia",
		221,"Senegal Republic",
		222,"Mauritania",
		223,"Mali",
		224,"Guinea",
		225,"Cote D'Ivoire",
		226,"Burkina Faso",
		227,"Niger",
		228,"Togo",
		229,"Benin",
		230,"Mauritius",
		231,"Liberia",
		232,"Sierra Leone",
		233,"Ghana",
		234,"Nigeria",
		235,"Chad",
		236,"Central African Republic",
		237,"Cameroon",
		238,"Cape Verde Islands",
		239,"Sao Tome and Principe",
		240,"Equatorial Guinea",
		241,"Gabon",
		242,"Congo",
		243,"Congo(DRC)",
		244,"Angola",
		245,"Guinea-Bissau",
		246,"Diego Garcia",
		247,"Ascension Island",
		248,"Seychelle Islands",
		249,"Sudan",
		250,"Rwanda",
		251,"Ethiopia",
		252,"Somalia",
		253,"Djibouti",
		254,"Kenya",
		255,"Tanzania",
		256,"Uganda",
		257,"Burundi",
		258,"Mozambique",
		260,"Zambia",
		261,"Madagascar",
		262,"Reunion Island",
		263,"Zimbabwe",
		264,"Namibia",
		265,"Malawi",
		266,"Lesotho",
		267,"Botswana",
		268,"Swaziland",
		269,"Comoros",
		269,"Mayotte Island",
		290,"St. Helena",
		291,"Eritrea",
		297,"Aruba",
		298,"Faroe Islands",
		299,"Greenland",
		350,"Gibraltar",
		351,"Portugal",
		352,"Luxembourg",
		353,"Ireland",
		354,"Iceland",
		355,"Albania",
		356,"Malta",
		357,"Cyprus",
		358,"Finland",
		359,"Bulgaria",
		370,"Lithuania",
		371,"Latvia",
		372,"Estonia",
		373,"Moldova",
		374,"Armenia",
		375,"Belarus",
		376,"Andorra",
		377,"Monaco",
		378,"San Marino",
		380,"Ukraine",
		381,"Serbia and Montenegro",
		385,"Croatia",
		386,"Slovenia",
		387,"Bosnia and Herzegovina",
		389,"F.Y.R.O.M. (Former Yugoslav Republic of Macedonia)",
		420,"Czech Republic",
		421,"Slovak Republic",
		500,"Falkland Islands (Islas Malvinas)",
		501,"Belize",
		502,"Guatemala",
		503,"El Salvador",
		504,"Honduras",
		505,"Nicaragua",
		506,"Costa Rica",
		507,"Panama",
		508,"St. Pierre and Miquelon",
		509,"Haiti",
		590,"French Antilles",
		590,"Guadeloupe",
		591,"Bolivia",
		592,"Guyana",
		593,"Ecuador",
		594,"French Guiana",
		595,"Paraguay",
		596,"Martinique",
		597,"Suriname",
		598,"Uruguay",
		599,"Netherlands Antilles",
		670,"Rota Island",
		670,"Saipan Island",
		670,"Tinian Island",
		671,"Guam",
		672,"Australian Antarctic Territory",
		672,"Christmas Island",
		672,"Norfolk Island",
		673,"Brunei",
		674,"Nauru",
		675,"Papua New Guinea",
		676,"Tonga",
		677,"Solomon Islands",
		678,"Vanuatu",
		679,"Fiji Islands",
		680,"Palau",
		681,"Wallis and Futuna Islands",
		682,"Cook Islands",
		683,"Niue",
		684,"Territory of American Samoa",
		685,"Samoa",
		686,"Kiribati Republic",
		687,"New Caledonia",
		688,"Tuvalu",
		689,"French Polynesia",
		690,"Tokelau",
		691,"Micronesia",
		692,"Marshall Islands",
		850,"Korea (North)",
		852,"Hong Kong SAR",
		853,"Macau SAR",
		855,"Cambodia",
		856,"Laos",
		871,"INMARSAT (Atlantic-East)",
		872,"INMARSAT (Pacific)",
		873,"INMARSAT (Indian)",
		874,"INMARSAT (Atlantic-West)",
		880,"Bangladesh",
		886,"Taiwan",
		960,"Maldives",
		961,"Lebanon",
		962,"Jordan",
		963,"Syria",
		964,"Iraq",
		965,"Kuwait",
		966,"Saudi Arabia",
		967,"Yemen",
		968,"Oman",
		971,"United Arab Emirates",
		972,"Israel",
		973,"Bahrain",
		974,"Qatar",
		975,"Bhutan",
		976,"Mongolia",
		977,"Nepal",
		994,"Azerbaijan",
		995,"Georgia",
		0,NULL
	};
	for(long index=0;list[index].Name!=NULL;index++)
	{
		if(list[index].CountryCode==CountryCode)
		{
			return list[index].Name;
		}
	}
	return NULL;
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
			char *CountryName=CountryCodeToString(CountryList.CountryCode);
			if(CountryName!=NULL)
			{
				std::cout << "[" << CountryName << " Cable Frequency]" << std::endl;
			}
			else
			{
				std::cout << "[Country " << CountryList.CountryCode << " Cable Frequency]" << std::endl;
			}
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
			char *CountryName=CountryCodeToString(CountryList.CountryCode);
			if(CountryName!=NULL)
			{
				std::cout << "[" << CountryName << " Air Frequency]" << std::endl;
			}
			else
			{
				std::cout << "[Country " << CountryList.CountryCode << " Air Frequency]" << std::endl;
			}
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

