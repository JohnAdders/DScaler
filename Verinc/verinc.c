/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2001 Avery Lee.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
/////////////////////////////////////////////////////////////////////////////
// This file was taken from VirtualDub
// VirtualDub - Video processing and capture application
// original comments
// 
// Ehhh....
//
// I think I'll just put this one in the public domain
// (with no warranty as usual).
//
// --Avery
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.2  2006/10/06 13:35:33  adcockj
// Added projects for .NET 2005 and fixed most of the warnings and errors
//
// Revision 1.1  2001/07/24 12:19:00  adcockj
// Added code and tools for crash logging from VirtualDub
//
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <memory.h>
#include <time.h>

typedef unsigned long ulong;

int main(void)
{
	FILE *f;
	ulong build=0,build_t;
	char s[25];
	time_t tm;

    printf("DScaler Version Incrementer\n");
    printf("(c) Avery Lee 2001\n\n");
    printf("This software comes with ABSOLUTELY NO WARRANTY\n");
    printf("This is free software, and you are welcome to redistribute it\n");
    printf("under certain conditions; See DScaler.chm for GPL licence\n");

	//////////////

	if (f=fopen("version.bin","rb"))
    {
		if (1==fread(&build_t,sizeof build_t,1,f))
        {
			build=build_t;
        }
	}

	++build;
	printf("Incrementing to build %d\n",build);

	time(&tm);
	memcpy(s ,asctime(localtime(&tm)), 24);
	s[24]=0;

	if (f=fopen("verstub.asm","w"))
    {
		fprintf(f,
			"\t"	".386\n"
			"\t"	".model\t"	"flat\n"
			"\t"	".data\n"
			"\n"
			"\t"	"public\t"	"_gBuildNum\n"
			"\t"	"public\t"	"_gBuildDateTime\n"
			"\n"
			"_gBuildNum\t"	"dd\t"	"%ld\n"
			"_gBuildDateTime\t"	"db\t"	"\"%s\",0\n"
			"\n"
			"\t"	"end\n"
			,build
			,s);
		fclose(f);
	}

	if (f=fopen("version.bin","wb"))
    {
		fwrite(&build,sizeof build,1,f);
		fclose(f);
	}

	return 0;
}
