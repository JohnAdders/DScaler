/////////////////////////////////////////////////////////////////////////////
// $Id: EPG.h,v 1.1 2005-03-20 09:48:58 laurentg Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 Laurent Garnier.  All rights reserved.
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
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 16 Mar 2005   Laurent Garnier       File created
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __EPG_H___
#define __EPG_H___


#include <stdio.h>
#include "Channels.h"


class CProgram
{
public:
	CProgram(time_t StartTime, time_t EndTime, LPCSTR Title, LPCSTR Channel);
	~CProgram();

	// Check whether the program matchs time and channel
	BOOL IsProgramMatching(time_t DateTime, LPCSTR Channel);

	// Get the program main data : start and end time + title
	void GetProgramMainData(string &Start, string &End, string &Title);

	// Dump the program main data : start and end time + channel + title
	void DumpProgramMainData();

private:
	time_t	m_StartTime;
	time_t	m_EndTime;
	string	m_Channel;
	string	m_Title;
	string	m_SubTitle;
	string	m_Category;
	string	m_Description;
	DWORD	m_Length;		// In minutes
};


class CEPG
{
public:
	CEPG();
	~CEPG();

	// Scan a XML file containing programs and generate the corresponding DScaler data files
	// The input file must be compatible with the XMLTV DTD
	// TODO Suppress the parameter delta_time as soon as XML API will be used
	int ScanXMLTVFile(LPCSTR file=NULL, int delta_time=0);

	// Load the DScaler EPG data for the programs between two dates
	// If DateMin and DateMax are not set, load the EPG data for
	// the interval [current time - 2 hours, current time + one day]
	// TODO Rewrite LoadEPGData as soon as XML API will be used
	int LoadEPGData(time_t DateMin=0, time_t DateMax=0);

	// Get the EPG data of the program matching time and channel
	int GetEPGData(string &StartTime, string &EndTime, string &Title, LPCSTR Channel=NULL, time_t Date=0);

	// Dump the EPG data
	void CEPG::DumpEPGData();

	void ClearPrograms();
	void AddProgram(time_t StartTime, time_t EndTime, LPCSTR Title, LPCSTR Channel);

private:
	// Convert the DScaler_tmp.txt file to the DScaler.txt final file
	// TODO Suppress ConvertXMLtoTXT as soon as XML API will be used
	int ConvertXMLtoTXT(int delta_time=0);
	
	// Execute a command using the Windows command interpreter
	int ExecuteCommand(string command);

	// Retrieve a line of text in a file (stream)
	BOOL GetFileLine(FILE *Stream, char *Buffer, int MaxLen);

	// Check whether a channel name belongs to the list of channels
	// defined in DScaler
	BOOL IsValidChannelName(LPCSTR Name);

    typedef vector<CProgram*> CPrograms;

    CPrograms	m_Programs;
	string		m_CMDExe;
	string		m_FilesDir;
	string		m_XMLTVExe;
};


extern CUserChannels MyChannels;
extern CEPG MyEPG;


#endif