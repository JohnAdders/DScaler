/////////////////////////////////////////////////////////////////////////////
// VBI_CCdecode.c
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 Mike Baker.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//
//	GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
//
//  This is the CLOSED CAPTION DECODER.
//
// taken from ccdecode.c by Mike Baker (mbm@linux.com)
// (based on code by timecop@japan.co.jp)
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 29 Jan 2001   John Adcock           Took code from latest version of ccdecode
//                                     Made compatable with dTV
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VBI_CCdecode.h"
#include "DebugLog.h"
#include "DScaler.h"
#include "BT848.h"


/* #define XWIN 1   /* visual debugging */

#ifdef XWIN
	#include <X11/X.h>
	#include <X11/Xlib.h>
	#include <X11/Xutil.h>
	#include <X11/Xproto.h>
	Display *dpy;
	Window Win,Root;
	char dpyname[256] = "";
	GC WinGC;
	GC WinGC0;
	GC WinGC1;
	int x;
#endif


//XDSdecode
char	info[8][25][256]; 
char	newinfo[8][25][256];
char	*infoptr=newinfo[0][0];
int	mode,type;
char	infochecksum;

//ccdecode
char    *ratings[] = {"(NOT RATED)","TV-Y","TV-Y7","TV-G","TV-PG","TV-14","TV-MA","(NOT RATED)"};
int     rowdata[] = {10,0,0,1,2,3,11,12,13,14,4,5,6,7,8,9};
char	*specialchar[] = {"®","°","½","¿","™","¢","£","¶ ","à"," ","è","â","ê","î","ô","û"};
char	*modes[]={"current","future","channel","miscellanious","public service","reserved","invalid","invalid","invalid","invalid"};
int	lastcode;
int	ccmode=1;		//cc1 or cc2
char	ccbuf[3][256];		//cc is 32 columns per row, this allows for extra characters
int	keywords=0;
char	*keyword[32];


//args (this should probably be put into a structure later)
char rawline=-1;

int sen;
int inval;

int parityok(int n)	/* check parity for 2 bytes packed in n */
{
    int mask=0;
    int j, k;

    for (k = 1, j = 0; j < 7; j++)
	{
		if (n & (1<<j))
		{
			k++;
		}
	}
    if ((k & 1) == ((n>>7)&1))
	{
	    mask |= 0x00FF;
	}
    for (k = 1, j = 8; j < 15; j++)
	{
		if (n & (1<<j))
		{
			k++;
		}
	}
    if ((k & 1) == ((n>>15)&1))
	{
	    mask |= 0xFF00;
	}
   return mask;
}

int decodebit(unsigned char *data, int threshold, int NumPixels)
{
    int i, sum = 0;
    for (i = 0; i < NumPixels; i++)
	{
		sum += data[i];
	}
    return (sum > threshold * NumPixels);
}

int FindClock(unsigned char *vbiline, int ClockPixels)
{
	int i;
	DWORD MinTotal = 0;
	int MinCount = 0;
	DWORD MaxTotal = 0;
	int MaxCount = 0;
	int LastPixel = (int)(ClockPixels * 6.5); 
	int Remainder;

	for(i = 0; i < LastPixel; i++)
	{
		Remainder = i % ClockPixels;
		if(Remainder < ClockPixels / 4  || Remainder > 3 * ClockPixels / 4)
		{
			MinTotal += vbiline[i];
			MinCount++;
		}
		else
		{
			MaxTotal += vbiline[i];
			MaxCount++;
		}
	}
	if(MinCount == 0 || MaxCount == 0)
	{
		return 0;	
	}
	else
	{
		return (MaxTotal / MaxCount) - (MinTotal / MinCount);
	}
}

int decode(unsigned char *vbiline)
{
    int i, tmp, packedbits = 0;
	int ClockMax = -1;
	int ClockPos = -1;
	DWORD Threshold = 0;
	int ClockCur;
    
    i=0;

    while (i < 120)
	{
		ClockCur = FindClock(vbiline + i, BT848_GetTVFormat()->CC_Clock);
		if(ClockCur > ClockMax)
		{
			ClockMax = ClockCur;
			ClockPos = i;
		}
		i++;
	}

	if(ClockMax < 10)
	{
		return -1;
	}

	tmp = ClockPos + BT848_GetTVFormat()->CC_Gap - 3 * BT848_GetTVFormat()->CC_Clock / 2;

	for(i = 0; i < BT848_GetTVFormat()->CC_Clock; i++)
	{
		Threshold += vbiline[tmp + i];
	}
	Threshold /= BT848_GetTVFormat()->CC_Clock;
	Threshold += ClockMax / 2; 
	tmp = ClockPos + BT848_GetTVFormat()->CC_Gap;

	if(!decodebit(&vbiline[tmp], Threshold, BT848_GetTVFormat()->CC_Clock / 2))
	{
		// no start bit
		return -1;
	}
    for (i = 0; i < 16; i++)
	{
		tmp += BT848_GetTVFormat()->CC_Clock;
		if(decodebit(&vbiline[tmp], Threshold, BT848_GetTVFormat()->CC_Clock / 2))
		{
			packedbits |= 1<<i;
		}
	}
	
    return packedbits & parityok(packedbits);
} /* decode */

int XDSdecode(int data)
{
	int b1, b2, length;
	if (data == -1)
		return -1;
	
	b1 = data & 0x7F;
	b2 = (data>>8) & 0x7F;

	if (b1 < 15) // start packet 
	{
		mode = b1;
		type = b2;
		infochecksum = b1 + b2 + 15;
		if (mode > 8 || type > 20)
		{
//			LOGD("%% Unsupported mode %s(%d) [%d]\n",modes[(mode-1)>>1],mode,type);
			mode=0; type=0;
		}
		infoptr = newinfo[mode][type];
	}
	else if (b1 == 15) // eof (next byte is checksum)
	{
#if 0 //debug
		if (mode == 0)
		{
			length=infoptr - newinfo[0][0];
			infoptr[1]=0;
			LOGD("LEN: %d\n",length);
			for (y=0;y<length;y++)
				LOGD(" %03d",newinfo[0][0][y]);
			LOGD(" --- %s\n",newinfo[0][0]);
		}
#endif
		if (mode == 0) return 0;
		if (b2 != 128-((infochecksum%128)&0x7F)) return 0;

		length = infoptr - newinfo[mode][type];

		//don't bug the user with repeated data
		//only parse it if it's different
		if (strncmp(info[mode][type],newinfo[mode][type],length-1))
		{
			infoptr = info[mode][type];
			memcpy(info[mode][type],newinfo[mode][type],length+1);
			LOGD("\33[33m%%");
			switch ((mode<<8) + type)
			{
				case 0x0101:
					LOGD(" TIMECODE: %d/%02d %d:%02d",
					infoptr[3]&0x0f,infoptr[2]&0x1f,infoptr[1]&0x1f,infoptr[0]&0x3f);
				case 0x0102:
					if ((infoptr[1]&0x3f)>5)
						break;
					LOGD("   LENGTH: %d:%02d:%02d of %d:%02d:00",
					infoptr[3]&0x3f,infoptr[2]&0x3f,infoptr[4]&0x3f,infoptr[1]&0x3f,infoptr[0]&0x3f);
					break;
				case 0x0103:
					infoptr[length] = 0;
					LOGD("    TITLE: %s",infoptr);
					break;
				case 0x0105:
					LOGD("   RATING: %s (%d)",ratings[infoptr[0]&0x07],infoptr[0]);
					if ((infoptr[0]&0x07)>0)
					{
						if (infoptr[0]&0x20) LOGD(" VIOLENCE");
						if (infoptr[0]&0x10) LOGD(" SEXUAL");
						if (infoptr[0]&0x08) LOGD(" LANGUAGE");
					}
					break;
				case 0x0501:
					infoptr[length] = 0;
					LOGD("  NETWORK: %s",infoptr);
					break;
				case 0x0502:
					infoptr[length] = 0;
					LOGD("     CALL: %s",infoptr);
					break;
				case 0x0701:
					LOGD(" CUR.TIME: %d:%02d %d/%02d/%04d UTC",infoptr[1]&0x1F,infoptr[0]&0x3f,infoptr[3]&0x0f,infoptr[2]&0x1f,(infoptr[5]&0x3f)+1990);
					break;
				case 0x0704: //timezone
					LOGD(" TIMEZONE: UTC-%d",infoptr[0]&0x1f);
					break;
				case 0x0104: //program genere
					break;
				case 0x0110:
				case 0x0111:
				case 0x0112:
				case 0x0113:
				case 0x0114:
				case 0x0115:
				case 0x0116:
				case 0x0117:
					infoptr[length+1] = 0;
					LOGD("     DESC: %s",infoptr);
					break;
			}
			LOGD("\33[0m\n");
		}
		mode = 0; type = 0;
	}
	else if( (infoptr - newinfo[mode][type]) < 250 ) // must be a data packet, check if we're in a supported mode
	{
		infoptr[0] = b1; infoptr++;
		infoptr[0] = b2; infoptr++;
		infochecksum += b1 + b2;
	}
	return 0;
}

void webtv_check(char * buf,int len)
{
	unsigned long   sum;
	unsigned long   nwords;
	unsigned short  csum=0;
	char temp[9];
	int nbytes=0;
	
	while (buf[0]!='<' && len > 6)  //search for the start
	{
		buf++; len--;
	}
	
	if (len == 6) //failure to find start
		return;
				
	
	while (nbytes+6 <= len)
	{
		//look for end of object checksum, it's enclosed in []'s and there shouldn't be any [' after
		if (buf[nbytes] == '[' && buf[nbytes+5] == ']' && buf[nbytes+6] != '[')
			break;
		else
			nbytes++;
	}
	if (nbytes+6>len) //failure to find end
		return;
	
	nwords = nbytes >> 1; sum = 0;

	//add up all two byte words
	while (nwords-- > 0) {
		sum += *buf++ << 8;
		sum += *buf++;
	}
	if (nbytes & 1) {
		sum += *buf << 8;
	}
	csum = (unsigned short)(sum >> 16);
	while(csum !=0) {
		sum = csum + (sum & 0xffff);
		csum = (unsigned short)(sum >> 16);
	}
	sprintf(temp,"%04X\n",(int)~sum&0xffff);
	buf++;
	if(!strncmp(buf,temp,4))
	{
		buf[5]=0;
		LOGD("\33[35mWEBTV: %s\33[0m\n",buf-nbytes-1);
	}
}


int CCdecode(int data, BOOL CaptionMode, int Channel)
{
	int b1, b2, len, x;
	static CC_Screen Screens[2];
	static int ScreenToWrite = 0;
	static int CursorPos = 0;
	static int CursorRow = 14;
	static char ForeColor = 0;
	static CAPTIONMODE Mode = TEXT;
	static int ModeRows = 0;
	static int LastIndent = -1;
	static CC_Char CurrentState =
		{ TRUE, ' ', CC_WHITE, CC_BLACK, FALSE, FALSE, FALSE,};
	static CC_Char ResetState =
		{ TRUE, ' ', CC_WHITE, CC_BLACK, FALSE, FALSE, FALSE,};
	static BOOL bCaptureText = FALSE;
	static int BadCount = 0;

	BOOL bPaintNow = FALSE;

	if (data == -1) //invalid data.
	{
		// if we get three in a row then reset the 
		// display
		BadCount++;
		if(BadCount == 3)
		{
			memset(&Screens[0],0,sizeof(CC_Screen));
			bPaintNow = TRUE;
			bCaptureText = FALSE;
		}
	}
	else
	{
		BadCount = 0;
		b1 = data & 0x7f;
		b2 = (data>>8) & 0x7f;

		if (b1&0x60 && data != lastcode) // text
		{
			if(bCaptureText)
			{
				// copy first character
				if(CursorPos < CC_CHARS_PER_LINE - 1)
				{
					memcpy(&Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos], &CurrentState, sizeof(CC_Char));
					Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].Text = b1;
					CursorPos++;
				}
				else 
				{
					for(x = 1; x < CC_CHARS_PER_LINE;x++)
					{
						memcpy(&Screens[ScreenToWrite].ScreenData[CursorRow][x - 1], 
							&Screens[ScreenToWrite].ScreenData[CursorRow][x], 
							sizeof(CC_Char));
					}
					memcpy(&Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos], &CurrentState, sizeof(CC_Char));
					Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].Text = b1;
				};
				
				if(b2 > 31)
				{
					// copy second character
					if(CursorPos < CC_CHARS_PER_LINE - 1)
					{
						memcpy(&Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos], &CurrentState, sizeof(CC_Char));
						Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].Text = b2;
						CursorPos++;
					}
					else 
					{
						for(x = 1; x < CC_CHARS_PER_LINE;x++)
						{
							memcpy(&Screens[ScreenToWrite].ScreenData[CursorRow][x - 1], 
								&Screens[ScreenToWrite].ScreenData[CursorRow][x], 
								sizeof(CC_Char));
						}
						memcpy(&Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos], &CurrentState, sizeof(CC_Char));
						Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].Text = b2;
					}
				}
				bPaintNow = (ScreenToWrite == 0);
			}
		}
		else if ((b1&0x10) && (b2>0x1F) && (data != lastcode)) //codes are always transmitted twice (apparently not, ignore the second occurance)
		{
			// if we have been asked for the other channel
			// need to ignore any text that follows
			if(((b1>>3)&1) != Channel)
			{
				bCaptureText = FALSE;
				return 0;
			}
			len = strlen(ccbuf[ccmode]);

			if (b2 & 0x40)	//preamble address code (row & indent)
			{
				if(bCaptureText == TRUE)
				{
					// reset state
					memcpy(&CurrentState, &ResetState, sizeof(CC_Char));

					// row inofrmation ignored in text mode
					if(Mode != ROLL_UP)
					{
						CursorRow = rowdata[((b1<<1)&14)|((b2>>5)&1)];
					}
					
					CursorPos = 0;

					// get underline flag
					CurrentState.bUnderline = b2&0x1;

					// get indent info
					if (b2&0x10)
					{
						//row contains indent flag
						CurrentState.ForeColor = CC_WHITE;
						CursorPos = b2&0x0E << 1; 
						// there seems to be a problem with the pop on captions
						// when you get the second or thrid line of data

						for(x=0; x<CursorPos; x++)
						{
							Screens[ScreenToWrite].ScreenData[CursorRow][x].bIsActive = FALSE;
						}
						LastIndent = CursorPos;
					}
					else
					{
						// get color and indent info
						if(b2&0x0E >> 1 == 0x8)
						{
							CurrentState.bItalics = TRUE;
						}
						else
						{
							CurrentState.ForeColor = (CC_Color)(b2&0x0E >> 1);
						}
					}
				}				
			}
			else
			{
				switch (b1 & 0x07)
				{
					case 0x00:	//attribute
						if(bCaptureText == TRUE)
						{
							CurrentState.BackColor = (CC_Color)(b2&0x0E >> 1);
							memcpy(&Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos], &CurrentState, sizeof(CC_Char));
							Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].Text = ' ';
							if(CursorPos < CC_CHARS_PER_LINE - 1) CursorPos++;
						}
						break;
					case 0x01:	//midrow or char
						if(bCaptureText == TRUE)
						{
							switch (b2&0x70)
							{
								case 0x20: //midrow attribute change
									switch (b2&0x0e)
									{
										case 0x0e: //italics on
											CurrentState.bItalics = TRUE;
											break;
										default:
											CurrentState.ForeColor = (CC_Color)(b2&0x0E >> 1);
											CurrentState.bItalics = FALSE;
											break;
									}
									if (b2&0x01) //underline
										CurrentState.bUnderline = TRUE;
									else
										CurrentState.bUnderline = FALSE;
									memcpy(&Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos], &CurrentState, sizeof(CC_Char));
									Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].Text = ' ';
									if(CursorPos < CC_CHARS_PER_LINE - 1) CursorPos++;
									break;
								case 0x30: //special character..
									memcpy(&Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos], &CurrentState, sizeof(CC_Char));
									Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].Text = *(specialchar[b2&0x0f]);
									if(CursorPos < CC_CHARS_PER_LINE - 1) CursorPos++;
									break;
							}
						}
						break;
					case 0x04:	//misc
					case 0x05:	//misc + F
						switch (b2)
						{
						// start pop on captioning
						case 0x20:
							if(CaptionMode)
							{
								ScreenToWrite = 1;
								Mode = POP_ON;
								memset(&Screens[1],0,sizeof(CC_Screen));
								memcpy(&CurrentState, &ResetState, sizeof(CC_Char));
								CursorPos = 0;
								CursorRow = 14;
								bPaintNow = FALSE;
								LastIndent = -1;
								bCaptureText = TRUE;
							}
							break;

						case 0x21: //backspace
							if(bCaptureText == TRUE)
							{
								if(CursorPos > 0)
								{
									CursorPos--;
								}
								Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].bIsActive = FALSE;
								bPaintNow = TRUE;
							}
							break;

						case 0x24: //delete to end of row
							if(bCaptureText == TRUE)
							{
								for(x = CursorPos; x < CC_CHARS_PER_LINE; x++)
								{
									Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].bIsActive = FALSE;
								}
								bPaintNow = TRUE;
							}
							break;

						// reset screen becase we've gone into roll up mode
						case 0x25: //2 row caption
						case 0x26: //3 row caption
						case 0x27: //4 row caption
							if(CaptionMode)
							{
								Mode = ROLL_UP;
								ModeRows = b2 - 0x23;
								ScreenToWrite = 0;
								memcpy(&CurrentState, &ResetState, sizeof(CC_Char));
								CursorPos = 0;
								CursorRow = 14;
								bPaintNow = TRUE;
								bCaptureText = TRUE;
							}
							break;

						case 0x28: //flash on
							if(bCaptureText == TRUE)
							{
								CurrentState.bFlash = TRUE;
								memcpy(&Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos], &CurrentState, sizeof(CC_Char));
								Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].Text = '\0';
								if(CursorPos < CC_CHARS_PER_LINE - 1) CursorPos++;
							}
							break;

						// reset screen becase we've gone into paint on mode
						case 0x29: //resume direct caption
							if(CaptionMode)
							{
								ScreenToWrite = 0;
								Mode = PAINT_ON;
								memset(&Screens[0],0,sizeof(CC_Screen));
								ScreenToWrite = 0;
								memcpy(&CurrentState, &ResetState, sizeof(CC_Char));
								CursorPos = 0;
								CursorRow = 14;
								bPaintNow = TRUE;
								bCaptureText = TRUE;
							}
							break;

						case 0x2C: //erase displayed memory
							if(CaptionMode)
							{
								memset(&Screens[0],0,sizeof(CC_Screen));
								bPaintNow = TRUE;
							}
							break;

						case 0x2E: //erase non-displayed memory
							if(CaptionMode)
							{
								memset(&Screens[1],0,sizeof(CC_Screen));
								LastIndent = -1;
							}
							break;

						case 0x2A: //text restart
							if(!CaptionMode)
							{
								Mode = TEXT;
								ModeRows = 9;
								memset(&Screens[0],0,sizeof(CC_Screen));
								ScreenToWrite = 0;
								memcpy(&CurrentState, &ResetState, sizeof(CC_Char));
								CursorPos = 0;
								CursorRow = 6;
								bPaintNow = TRUE;
								bCaptureText = TRUE;
							}
							break;

						case 0x2B: //resume text display
							if(!CaptionMode)
							{
								bCaptureText = TRUE;
							}
							break;
							
						case 0x2D: //carriage return
							if(bCaptureText == TRUE)
							{
								if(Mode == TEXT || Mode == ROLL_UP)
								{
									if(CursorRow < 14)
									{
										CursorRow++;
									}
									else
									{
										for(x = 14 - ModeRows; x < 14; x++)
										{
											memcpy(Screens[ScreenToWrite].ScreenData[x], Screens[ScreenToWrite].ScreenData[x + 1], CC_CHARS_PER_LINE * sizeof(CC_Char));
										}
										CursorRow = 14;
									}
									memset(Screens[ScreenToWrite].ScreenData[x], 0 ,CC_CHARS_PER_LINE * sizeof(CC_Char));
									memcpy(&CurrentState, &ResetState, sizeof(CC_Char));
									CursorPos = 0;
								}
							}
							break;
						
						case 0x2F: //end caption + swap memory
							if(CaptionMode)
							{
								memcpy(Screens, Screens + 1, sizeof(CC_Screen));
								memcpy(&CurrentState, &ResetState, sizeof(CC_Char));
								ScreenToWrite = 0;
								CursorPos = 0;
								CursorRow = 14;
								bPaintNow = TRUE;
								LastIndent = -1;
							}
							break;
					}
					break;
				case 0x07:	//misc (TAB)
					if(bCaptureText == TRUE)
					{
						switch(b2)
						{
						case 0x23:
							Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].bIsActive = FALSE;
							if(CursorPos < CC_CHARS_PER_LINE - 1) CursorPos++;
							/* FALL */
						case 0x22:
							Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].bIsActive = FALSE;
							if(CursorPos < CC_CHARS_PER_LINE - 1) CursorPos++;
							/* FALL */
						case 0x21:
							Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].bIsActive = FALSE;
							if(CursorPos < CC_CHARS_PER_LINE - 1) CursorPos++;
							break;

						case 0x2F:
							CurrentState.bUnderline = TRUE;
							/* FALL */
						case 0x2E:
							CurrentState.ForeColor = CC_BLACK;
							memcpy(&Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos], &CurrentState, sizeof(CC_Char));
							Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].Text = ' ';
							if(CursorPos < CC_CHARS_PER_LINE - 1) CursorPos++;
							break;
						}
					}
					break;
				}
			}
		}
	}
	if(bPaintNow)
	{
		PostMessage(hWnd, WM_COMMAND, IDM_OSD_CC_TEXT, (LPARAM)Screens);
	}
	lastcode=data;
	return 0;
}

int CC_DecodeLine(BYTE* vbiline, CCMODE CCMode, BOOL IsOdd)
{
	if(!IsOdd)
	{
		switch(CCMode)
		{
		case CCMODE_CC1:
			CCdecode(decode(vbiline), TRUE, 0);
			break;
		case CCMODE_CC2:
			CCdecode(decode(vbiline), TRUE, 1);
			break;
		case CCMODE_TEXT1:
			CCdecode(decode(vbiline), FALSE, 0);
			break;
		case CCMODE_TEXT2:
			CCdecode(decode(vbiline), FALSE, 1);
			break;
		default:
			break;
		}
	}
	else
	{
		switch(CCMode)
		{
		case CCMODE_CC3:
			CCdecode(decode(vbiline), TRUE, 0);
			break;
		case CCMODE_CC4:
			CCdecode(decode(vbiline), TRUE, 1);
			break;
		case CCMODE_TEXT3:
			CCdecode(decode(vbiline), FALSE, 0);
			break;
		case CCMODE_TEXT4:
			CCdecode(decode(vbiline), FALSE, 1);
			break;
		default:
			break;
		}
	}
	return 0;
}

int XDS_DecodeLine(BYTE* vbiline)
{
	XDSdecode(decode(vbiline));
	return 0;
}

COLORREF CC_GetColor(CC_Color Color)
{
	switch(Color)
	{
	case CC_GREEN:
		return RGB(0,255,0);
	case CC_BLUE:
		return RGB(0,0,255);
	case CC_CYAN:
		return RGB(0,255,255);
	case CC_RED:
		return RGB(255,0,0);
	case CC_YELLOW:
		return RGB(0,255,255);
	case CC_MAGENTA:
		return RGB(0,128,128);
	case CC_BLACK:
		return RGB(0,0,0);
	default:
	case CC_WHITE:
		return RGB(255,255,255);
	}
}


void CC_PaintChars(HWND hWnd, CC_Char* Char, char* szLine, HDC hDC, RECT* PaintRect, int nFontsize)
{
	SIZE sizeText;
	HFONT hTmp, hOSDfont;

	if (hDC != NULL)
	{
		hOSDfont = CreateFont(nFontsize, 0, 0, 0, 
								(Char->bFlash)? FW_BOLD:0, 
								Char->bItalics, 
								Char->bUnderline, 
								0, 0, 0, 0, 
								NONANTIALIASED_QUALITY, 
								DEFAULT_PITCH | FF_DONTCARE, 
								"Arial");
		if (!hOSDfont) return;

		hTmp = (HFONT)SelectObject(hDC, hOSDfont);
		
		if(hTmp)
		{
			GetTextExtentPoint32(hDC, szLine, strlen(szLine), &sizeText);

			SetBkMode(hDC, TRANSPARENT);
			SetTextColor(hDC, CC_GetColor(Char->BackColor));

			TextOut(hDC, PaintRect->left - 2, PaintRect->top, szLine, strlen(szLine));
			TextOut(hDC, PaintRect->left + 2, PaintRect->top, szLine, strlen(szLine));
			TextOut(hDC, PaintRect->left, PaintRect->top - 2, szLine, strlen(szLine));
			TextOut(hDC, PaintRect->left, PaintRect->top + 2, szLine, strlen(szLine));
			
			SetTextColor(hDC, CC_GetColor(Char->ForeColor));
			TextOut(hDC, PaintRect->left, PaintRect->top, szLine, strlen(szLine));

			PaintRect->left += sizeText.cx;

			SelectObject(hDC, hTmp);
			DeleteObject(hOSDfont);
		}
	}
}


void CC_PaintLine(HWND hWnd, CC_Char* Line, HDC hDC, RECT* PaintRect, int nFontsize)
{
	int i;
	BOOL bAnyText = FALSE;
	int Count = 0;
	int LineWidth = (PaintRect->right - PaintRect->left);
	char szString[CC_CHARS_PER_LINE + 1];
	CC_Char* LastChar = NULL;
	int StrPos = 0;

	for(i = 0; i < CC_CHARS_PER_LINE; i++)
	{
		if(Line[i].bIsActive)
		{
			if(!bAnyText && Count == 0)
			{
				bAnyText = TRUE;
				PaintRect->left = PaintRect->left + LineWidth * (i + 2) / (CC_CHARS_PER_LINE + 4);
			}
			
			if(LastChar != NULL &&
				(LastChar->BackColor != Line[i].BackColor ||
				 LastChar->ForeColor != Line[i].ForeColor ||
				 LastChar->bItalics != Line[i].bItalics ||
				 LastChar->bUnderline != Line[i].bUnderline ||
				 LastChar->bFlash != Line[i].bFlash))
			{
				szString[StrPos] = '\0';
				CC_PaintChars(hWnd, LastChar, szString, hDC, PaintRect, nFontsize);
				StrPos = 0;
			}
			
			// cope with changes from straight ASCII
			switch(Line[i].Text)
			{
			case '`':
				szString[StrPos++] = 'á';
				break;
			case '*':
				szString[StrPos++] = 'ú';
				break;
			case '\\':
				szString[StrPos++] = 'é';
				break;
			case '^':
				szString[StrPos++] = 'í';
				break;
			case '_':
				szString[StrPos++] = 'ó';
				break;
			case '{':
				szString[StrPos++] = 'ç';
				break;
			case '|':
				szString[StrPos++] = '÷';
				break;
			case '}':
				szString[StrPos++] = 'Ñ';
				break;
			case '~':
				szString[StrPos++] = 'ñ';
				break;
			default:
				szString[StrPos++] = Line[i].Text;
				break;
			}
			LastChar = Line + i;
			Count++;
		}
		else
		{
			if(StrPos > 0)
			{
				szString[StrPos] = '\0';
				CC_PaintChars(hWnd, LastChar, szString, hDC, PaintRect, nFontsize);
				StrPos = 0;
			}
			bAnyText = FALSE;
			LastChar = NULL;
		}
	}

	if(StrPos > 0)
	{
		szString[StrPos] = '\0';
		CC_PaintChars(hWnd, LastChar, szString, hDC, PaintRect, nFontsize);
	}
}

void CC_PaintScreen(HWND hWnd, CC_Screen* Screen, HDC hDC, RECT* PaintRect)
{
	int i;
	RECT LineRect;
	int nFontsize;
	
	if (hDC != NULL)
	{
		nFontsize = (PaintRect->bottom - PaintRect->top) / 17;
		if(nFontsize <= 1)
		{
			return;
		}

		for(i = 0; i < 15; i++)
		{
			LineRect.top = PaintRect->top + ((PaintRect->bottom - PaintRect->top) * (i + 1)) / 17;
			LineRect.bottom = PaintRect->top + ((PaintRect->bottom - PaintRect->top) * (i + 2)) / 17;
			LineRect.left = PaintRect->left;
			LineRect.right = PaintRect->right;
			CC_PaintLine(hWnd, &Screen->ScreenData[i][0], hDC, &LineRect, nFontsize);
		}
	}
}

