/////////////////////////////////////////////////////////////////////////////
// $Id: CRTL.CPP,v 1.2 2001-07-13 16:13:53 adcockj Exp $
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
//
// This software was based on hwiodrv from the FreeTV project Those portions are
// Copyright (C) Mathias Ellinger
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 15 Apr 1995   Mathias Ellinger      initial version
//
// 24 Jul 2000   John Adcock           Original dTV Release
//                                     Added Memory Alloc functions
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "basetype.h"

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void * operator
new(unsigned int size )
{
  return _HeapAllocate (size, 0) ;
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void operator
delete( void * ptr )
{
  _HeapFree( ptr, 0);
}

int __cdecl __vsnprintf(char *buffer, int Limit, const char *format, va_list next);

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
char * __cdecl strcat(char * dst, const char * src)
{
        char * cp = dst;

        while( *cp )
                cp++;                   // find end of dst 

        while( *cp++ = *src++ ) ;       // Copy src to end of dst 

        return( dst );                  // return dst 
}



//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
PCHAR __cdecl strncpy( char* s1, const char* s2, size_t c)
{
      _asm {
                mov     esi, [s2]
                mov     edi, [s1]
                cld
                mov     ecx, [c]
        }
next:
        _asm {
                dec     ecx
                jl      done
                lodsb
                stosb
                or      al, al
                jnz     next

        }
padout:
        _asm {
                dec     ecx
                jl      done
                stosb
                jmp     padout
        }
done:
        return s1;
}



int __cdecl
strncmp (
        const char * src,
        const char * dst,
        size_t len
        )
{
        int ret = 0 ;

        while( len && ! (ret = *(unsigned char *)src - *(unsigned char *)dst) && *dst)
        {
                ++src, ++dst;
                len--;
        }

        if ( ret < 0 )
                ret = -1 ;
        else if ( ret > 0 )
                ret = 1 ;

        return( ret );
}



//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
int __cdecl
strcmp (
        const char * src,
        const char * dst
        )
{
        int ret = 0 ;

        while( ! (ret = *(unsigned char *)src - *(unsigned char *)dst) && *dst)
                ++src, ++dst;

        if ( ret < 0 )
                ret = -1 ;
        else if ( ret > 0 )
                ret = 1 ;

        return( ret );
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
size_t __cdecl
strlen( const char* s1)
 {
  char * s = (char *)s1;
  int i = 0;
  while ( *s++ ) i++;
  return i;
 }


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
PCHAR __cdecl
strcpy( char* s1, const char* s2)
 {
  return strncpy( s1, s2, strlen(s2));
 }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
int __cdecl
_stricmp( const char* s1, const char* s2)
{
  int r;

  _asm {
                mov     ecx, [s1]
                mov     edx, [s2]
        }
next:
        _asm {
                movzx   eax, byte ptr [ecx]
                movzx   ebx, byte ptr [edx]

                cmp     al, 'A'
                jb      shift2
                cmp     al, 'Z'
                ja      shift2
                add     al, 'a'-'A'
        }
shift2:
        _asm {
                cmp     bl, 'A'
                jb      takeDiff
                cmp     bl, 'Z'
                ja      takeDiff
                add     bl, 'a'-'A'
        }
takeDiff:
        _asm {
                sub     eax, ebx
                jnz     notEqual

                inc     ecx
                inc     edx

                cmp     byte ptr [ecx-1], 0
                jnz     next

        }
notEqual:
        _asm {
                mov     r, eax
        }

        return r;
}

//---------------------------------------------------------------------------
// Finds the first occurrence of string2 in string1
//---------------------------------------------------------------------------
char * __cdecl
strstr(
        const char * str1,
        const char * str2
        )
 {
  char *cp = (char *) str1;
  char *s1, *s2;

  if ( !*str2 )  return((char *)str1);

  while (*cp)
   {
    s1 = cp;
    s2 = (char *) str2;

    while ( *s1 && *s2 && !(*s1-*s2) ) s1++, s2++;

    if (!*s2)  return(cp);

     cp++;
   }

  return(NULL);
 }




int __cdecl
__vsnprintf(char *buffer, int Limit, const char *format, va_list next);



//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
int
isDigit(CHAR ch)
 {
  return( ((ch >= '0') && (ch <= '9')) ? 1 : 0);
 }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void* _cdecl
memcpy( void *dest, const void *src, size_t count)
 {
  _asm {
    cld
    mov edi, [dest]
    mov esi, [src]
    mov ecx, [count]
    rep movsb
  }
 return dest;
 }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void* __cdecl
memset( void *dest, int c, size_t count)
 {
  _asm {
    cld
    mov     edi, [dest]
    mov     ecx, [count]
    mov     al, byte ptr c
    rep     stosb
  }

  return dest;
 }



//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
long __cdecl atol(const char* s)
{
        long r = 0;
        long sign = 0;

        while ( (*s == ' ') || (*s == '\t'))
                s++;


        if (*s == '+')
        {
                sign = 0;
                s++;
        }
        else if (*s == '-')
        {
                sign = -1;
                s++;
        }

        while ( ('0' <= *s) && (*s <= '9') )
        {
                r = r*10 + (*s - '0');
                s++;
        }

        return (r ^ sign) - sign;
}



//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

PCHAR __cdecl
_ltoa(long value, char* s, int radix)
 {
  long u;
  long pow, prevpow;
  int digit;
  char* p;

  if ( (radix < 2) || (36 < radix) )
   {
    *s = 0;
    return s;
   }

  if (value == 0)
   {
    s[0] = '0';
    s[1] = 0;
    return s;
   }

  p = s;

  if ( (radix == 10) && (value < 0) )
   {
    *p++ = '-';
    value = -value;
   }

  *(long*)&u = value;

  for (prevpow=0, pow=1; (u >= pow) && (prevpow < pow);  pow *= radix)
   {
    prevpow=pow;
   }

  pow = prevpow;

  while (pow != 0)
   {
    digit = u/pow;

    *p = (digit <= 9) ? ('0'+digit) : ( ('a'-10)+digit);
    p++;

    u -= digit*pow;
    pow /= radix;
   }

  *p = 0;

  return s;
 }


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
PCHAR __cdecl
_ultoa(unsigned long u, char* s, int radix)
 {

  long pow, prevpow;
  int digit;
  char* p;

  if ( (radix < 2) || (36 < radix) )
   {
    *s = 0;
    return s;
   }

   if (u == 0)
    {
     s[0] = '0';
     s[1] = 0;
     return s;
    }

   p = s;

   for (prevpow=0, pow=1; (u >= (unsigned long) pow) && (prevpow < pow);  pow *= radix)
                prevpow=pow;

        pow = prevpow;

        while (pow != 0)
        {
                digit = u/pow;

                *p = (digit <= 9) ? ('0'+digit) : ( ('a'-10)+digit);
                p++;

                u -= digit*pow;
                pow /= radix;
        }

        *p = 0;

        return s;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
char * __cdecl
_strupr ( char * string )
 {
  char * cp;

  for (cp=string; *cp; ++cp)
   {
    if ('a' <= *cp && *cp <= 'z') *cp += 'A' - 'a';
   }

   return(string);
 }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
int __cdecl vsprintf(char *buffer, const char *format, va_list next)
 {
  return __vsnprintf(buffer, -1, format, next);
 }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

int __cdecl
sprintf(char *buffer, const char *format, ...)
 {
  va_list next;

  va_start(next, format);
  return __vsnprintf(buffer, -1, format, next);
 }


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
int __cdecl __vsnprintf(char *buffer, int Limit, const char *format, va_list Next)
{
  int   InitLimit = Limit;    // Limit at entry point
  int   bMore;                // Loop control
  int   Width;                // Optional width
  int   Precision;            // Optional precision
  char  *str;                 // String
  char  strbuf[36];           // Constructed string
  int   len;                  // Length of string
  int   nLeadingZeros;        // Number of leading zeros required
  int   nPad;                 // Number of pad characters required
  char  cPad;                 // Current pad character ('0' or ' ')
  char  *sPrefix;             // Prefix string
  unsigned long val;          // Value of current number
  int   bLeftJustify;         // Justification
  int   bPlusSign;            // Show plus sign?
  int  bBlankSign;           // Blank for positives?
  int  bZeroPrefix;          // Want 0x for hex, 0 for octal?
  int  bIsShort;             // 1 if short
  int  bIsLong;              // 1 if long
  int  bIsWChar;             // 1 if 16 bit char

#define PUTONE(c) if (Limit) { --Limit; *buffer++ = c; } else c;

#define fLeftJustify    (1 << 0)
#define fPlusSign       (1 << 1)
#define fZeroPad        (1 << 2)
#define fBlankSign      (1 << 3)
#define fPrefixOX       (1 << 4)

#define fIsShort        (1 << 5)
#define fIsLong         (1 << 6)

 if (Limit == 0) return -1;
 Limit--;                    // Leave room for terminating NULL

 while (*format != '\0')
  {
   //
   // Everything but '%' is copied to buffer
   //

   if (*format != '%')
     {
      PUTONE(*format++)
     }
    else
     {
      // '%' gets special handling here
      // Set default flags, etc

      Width             = 0;
      Precision         = -1;
      cPad              = ' ';
      bLeftJustify      = 0;
      bPlusSign         = 0;
      bBlankSign        = 0;
      bZeroPrefix       = 0;
      bIsShort          = 0;
      bIsLong           = 0;
      bIsWChar          = 0;
      sPrefix           = "";

      format++;
      bMore = 1;
      while (bMore)
       {
        //
        // optional flags
        //

        switch (*format)
         {
          case '-':  bLeftJustify = 1; format++; break;
          case '+':  bPlusSign    = 1; format++; break;
          case '0':  cPad         = '0';  format++; break;
          case ' ':  bBlankSign   = 1; format++; break;
          case '#':  bZeroPrefix  = 1; format++; break;
          default:   bMore        = 0;
         }
       }

      //
      // optional width
      //
            if (*format == '*') {
                    Width = (int) va_arg(Next, int);
                    format++;
            }
            else if (isDigit(*format)) {
                    while (isDigit(*format)) {
                            Width *= 10;
                            Width += (*format++) - '0';
                    }
            }
// optional precision
            if (*format == '.') {
                    format++;
                    Precision = 0;
                    if (*format == '*') {
                            Precision = (int) va_arg(Next, int);
                            format++;
                    }
                    else while (isDigit(*format)) {
                            Precision *= 10;
                            Precision += (*format++) - '0';
                    }
            }
// optional size
            switch (*format) {
                case 'h':  bIsShort = 1; format++; break;
                case 'l':  bIsLong = 1;  format++; break;
                case 'w':  bIsWChar = 1; format++; break;
            }

// All controls are completed, dispatch on the conversion character

            switch (*format++) {
                case 'd':
                case 'i':
                        if (bIsLong)            // Signed long int
                            _ltoa( (long) va_arg(Next, long), strbuf, 10);
                        else                    // Signed int
                            _ltoa( (long) va_arg(Next, int), strbuf, 10);

                        if (strbuf[0] == '-') sPrefix = "-";
                        else {
                            if (bPlusSign) sPrefix = "+";
                            else if (bBlankSign) sPrefix = " ";
                        }
                        goto EmitNumber;


                case 'u':
                        if (bIsLong)            // Unsigned long int
                            _ultoa( (long) va_arg(Next, long), strbuf, 10);
                        else                    // Unsigned int
                            _ultoa( (long) (int) va_arg(Next, int), strbuf, 10);
                        goto EmitNumber;

// set sPrefix for these...
                case 'o':
                        if (bZeroPrefix) sPrefix = "0";
                        if (bIsLong)
                            val = (long) va_arg(Next, long);
                        else
                            val = (int) va_arg(Next, int);

                        _ultoa(val, strbuf, 8);
                        if (val == 0) sPrefix = "";
                        goto EmitNumber;

                case 'p':       // pointer
                case 'x':
                        if (bZeroPrefix) sPrefix = "0x";

                        if (bIsLong)
                            val = (unsigned long) va_arg(Next, long);
                        else
                            val = (unsigned int) va_arg(Next, int);

                        _ultoa(val, strbuf, 16);
                        if (val == 0) sPrefix = "";
                        goto EmitNumber;

                case 'X':
                        if (bZeroPrefix) sPrefix = "0X";

                        if (bIsLong)
                            val = (unsigned long) va_arg(Next, long);
                        else
                            val = (unsigned int) va_arg(Next, int);

                        _ultoa(val, strbuf, 16);
                        if (val == 0) sPrefix = "";
                        _strupr(strbuf);
                        goto EmitNumber;

                case 'c':
                        strbuf[0] = (char) va_arg(Next, char);
                        str = strbuf;
                        len = 1;
                        goto EmitString;

      case 's':
        str = (char *) va_arg(Next, char*);
        if (  bIsWChar )
          {
           PULONG ich = (PULONG) str;
           len        = 0;
           while ( *ich++ ) len++;
          }
         else
          {
           len =  strlen(str);
          }

        if ( (Precision != -1) && (Precision < len))
         {
          len = Precision;
         }

        goto EmitString;

                case 'n':
                        break;

                case '%':
                        strbuf[0] = '%';
                        str = strbuf;
                        len = 1;
                        goto EmitString;
                        break;

                case 'f':
                case 'e':
                case 'E':
                case 'g':
                case 'G':
                        str = "<float format not supported>";
                        len =  strlen(str);
                        goto EmitString;

                default:
                        str = "<bad format character>";
                        len =  strlen(str);
                        goto EmitString;
            }


EmitNumber:
            if (Precision == -1) Precision = 1;
            str = strbuf;
            if (*str == '-') str++;             // if negative, already have prefix
            len =  strlen(str);

            nLeadingZeros = Precision - len;
            if (nLeadingZeros < 0) nLeadingZeros = 0;

            nPad = Width - (len + nLeadingZeros +  strlen(sPrefix));
            if (nPad < 0) nPad = 0;
// nPad = 0;

            if (nPad && !bLeftJustify) {
                    // Left padding required
                    while (nPad--) {
                            PUTONE(cPad);
                    }
                    nPad = 0;           // Indicate padding completed
            }

            while (*sPrefix != '\0') PUTONE(*sPrefix++);

            while (nLeadingZeros-- > 0) PUTONE('0');

            while (len-- > 0) {
                    PUTONE(*str++);
            }

            if (nPad) {
                    // Right padding required
                    while (nPad--) PUTONE(' ');
            }

            goto Done;


EmitString:

// Here we have the string ready to emit.  Handle padding, etc.

            if (Width > len) nPad = Width - len;
            else nPad = 0;
// nPad = 0;
   if (nPad && !bLeftJustify)
    {
     // Left padding required
     while (nPad--) PUTONE(cPad);
     nPad = 0;
    }

   while (len-- > 0)
    {
     PUTONE(*str++);
     if ( bIsWChar ) str++;
    }

   if (nPad)
    {
     // Right padding required
     while (nPad--) PUTONE(' ');
    }

Done:   ;
        }
    }

 *buffer = '\0';
  return InitLimit - Limit - 1;               // Don't count terminating NULL
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
int __cdecl atexit(void (__cdecl *)(void))
{
  return 0;
}




