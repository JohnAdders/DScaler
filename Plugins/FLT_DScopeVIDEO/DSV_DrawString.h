/////////////////////////////////////////////////////////////////////////////
//  DSV_DrawString.h
/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2003 Michael Joubert   All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
/////////////////////////////////////////////////////////////////////////////
// Originally from theJam79's code from the IT 0.051AVISynth filter, with
//  trivial changes by Lindsey Dubb & Michael Joubert

#include "Font01.h"

/*----------------------------------------------------------------------------

    Draw Character

*/
void DrawChar(unsigned char* P_source, unsigned char* P_dest, int pitch_source, int FrameHeight_dest, int x, int y, int Character)
{
    // mods to select in/out pictures mj
    int             Column = 0;
    int             Row = 0;
    unsigned char*  P_dest_Start;
    static          char           strbuff[80];             // for debug text output

    x = x * 10;     // 10 Columns(pixels) width
    y = y * 20;     // 20 Rows   (lines)  height

// mj fix, added pitch for source and destination !!!!!!!!!!!!!!!!************!!!!!!!!!!!!!!

//      _ltoa ( P_dest, strbuff, 16 );         // MJ DEBUGGING
//      strcat( strbuff, " = P_dest" );
//      OutputDebugString   ( strbuff );


    P_dest_Start = P_dest;//quick fix only mj **********************8

    for (Column = 0; Column < 10; Column++)     // 10 Columns (pixels) across
    {
        for (Row = 0; Row < 20; Row++)          // 20 Rows    (lines)  down
        {
            P_dest = &(P_source[(x + Column) * 2 + (y + Row) * pitch_source]);

          //NOT COMPLETE to be fixed mj ******************************************************************************
           // if (P_dest < P_dest_Start + (FrameHeight_dest * pitch_source))// prevent over-run
           //     continue;
            if (font[Character][Row] & (1 << (15 - Column)))
            {
                if (Column & 1)
                {
                    P_dest[0]  = 250;
                    P_dest[-1] = 128;
                    P_dest[1]  = 128;
                }
                else
                {
                    P_dest[0]  = 250;
                    P_dest[1]  = 128;
                    P_dest[3]  = 128;
                }
            }
            else
            {
                if (Column & 1)
                {
                    P_dest[0]  = (P_dest[0]  * 3)   >> 2;
                    P_dest[-1] = (P_dest[-1] + 128) >> 1;
                    P_dest[1]  = (P_dest[1]  + 128) >> 1;
                }
                else
                {
                    P_dest[0]  = (P_dest[0] * 3)    >> 2;
                    P_dest[1]  = (P_dest[1] + 128)  >> 1;
                    P_dest[3]  = (P_dest[3] + 128)  >> 1;
                }
            }
        }
    }
}




/*----------------------------------------------------------------------------

    Draw String

*/

void DrawString(unsigned char* P_source, unsigned char* P_dest, int pitch_source, int FrameHeight_dest, int x, int y, const char *strBuffer)
{
    int xx = 0;

    for (xx = 0; *strBuffer; ++strBuffer, ++xx)
    {
        DrawChar(P_source, P_dest, pitch_source, FrameHeight_dest, x + xx, y, *strBuffer - ' ');
    }
}

